#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "util.h"
#include "wpa_ctrl.h"

#define SOCK_PATH "/var/run/wpa_supplicant"
#define BUFFER_SIZE 4096

#define SSID_SIZE 33 /* 32 characters plus terminator */

#define PLEVEL_LEN 3

#define TYPES_NUM 18

#define CE_CON           0
#define CE_DCON          1
#define CE_TERM          2
#define CE_PASS_CHANGED  3
#define CE_EAP_NOTIF     4
#define CE_EAP_START     5
#define CE_EAP_METHOD    6
#define CE_EAP_SUCCESS   7
#define CE_EAP_FAIL      8
#define CE_SCAN_RES      9
#define CE_BSS_ADD      10
#define CE_BSS_RM       11
#define CR_ID           12
#define CR_PASS         13
#define CR_NEW_PASS     14
#define CR_PIN          15
#define CR_OTP          16
#define CR_PASSPHRASE   17

typedef void (*wpa_action_t)(state_t *state, wpa_interface_t *iface, char *params);

static void
get_nid(char nid[4], char *params)
{
	while (*params != '-') {
		*nid++ = *params++;
	}
}

/* FIXME: Currently prints to stdout */
static void 
prompt_password(state_t *state, char *params)
{
	struct termios tp, save;
	char buf[BUFFER_SIZE] = "CTRL-RSP-";
	char pwd[512];
	char nid[4] = {0};

	get_nid(nid, params);

	if (tcgetattr(STDIN_FILENO, &tp) == -1) {
		perror("tcgetattr");
		return;
	}	

	save = tp;
	tp.c_lflag &= ~ECHO;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
		perror("tcsetattr");
		return;
	}

	printf("Password: ");
	fflush(stdout);

	if (fgets(pwd, 512, stdin) == NULL) {
		eprintf("EOF Error\n");
		return;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1) {
		perror("tcsetattr");
	}

	if (strcat(buf, nid) == NULL) {
		eprintf("strcat error\n");
		return;
	}

	if (strcat(buf, "-") == NULL) {
		eprintf("strcat error\n");
		return;
	}

	if (strcat(buf, pwd) == NULL) {
		eprintf("strcat error\n");
		return;
	}

	DEBUG("%s\n", buf);
}

static void
netman_exit(state_t *state, wpa_interface_t *iface, char *params)
{
	/* TODO: properly clean up or something */
	printf("%s\n", params);
	exit(0);
}

static char types[TYPES_NUM][128] = {
	[CE_CON]           = "CTRL-EVENT-CONNECTED",
	[CE_DCON]          = "CTRL-EVENT-DISCONNECTED",
	[CE_TERM]          = "CTRL-EVENT-TERMINATING",
	[CE_PASS_CHANGED]  = "CTRL-EVENT-PASSWORD_CHANGED",
	[CE_EAP_NOTIF]     = "CTRL-EVENT-EAP-NOTIFICATION",
	[CE_EAP_START]     = "CTRL-EVENT-EAP-STARTED",
	[CE_EAP_METHOD]    = "CTRL-EVENT-EAP-METHOD",
	[CE_EAP_SUCCESS]   = "CTRL-EVENT-EAP-SUCCESS",
	[CE_EAP_FAIL]      = "CTRL-EVENT-EAP-FAILURE",
	[CE_SCAN_RES]      = "CTRL-EVENT-SCAN-RESULTS",
	[CE_BSS_ADD]       = "CTRL-EVENT-BSS-ADDED",
	[CE_BSS_RM]        = "CTRL-EVENT-BSS-REMOVED",
	[CR_ID]            = "CTRL-REQ-IDENTITY",
	[CR_PASS]          = "CTRL-REQ-PASSWORD",
	[CR_NEW_PASS]      = "CTRL-REQ-NEW_PASSWORD",
	[CR_PIN]           = "CTRL-REQ-PIN",
	[CR_OTP]           = "CTRL-REQ-OTP",
	[CR_PASSPHRASE]    = "CTRL-REQ-PASSPHRASE",
};

static int
get_type(char buffer[BUFFER_SIZE])
{
	int i;
	for (i = 0; i < TYPES_NUM; i++) {
		/* Checks the beginning of the string */
		if (strncmp(buffer, types[i], strlen(types[i])) == 0) {
			return i;
		}
	}
	return -1;
}

static void
get_param_str(char buffer[BUFFER_SIZE], char params[512], int idx)
{
	char *type = types[idx];
	/* Assume the type is shorter than the actual message */
	while (*buffer == *type) {
		type++;
		buffer++;
	}

	/* Strip the - or the whitespace as documented in
	 * https://w1.fi/wpa_supplicant/devel/ctrl_iface_page.html
	 */
	buffer++;

	strcpy(params, buffer);
}

static int
wpa_ctrl_request(wpa_interface_t *iface, char *command, char *reply)
{
	char buffer[BUFFER_SIZE];
	int nbytes, res;

	strcpy(buffer, command);
	DEBUG("%s\n", buffer);
	res = write(iface->control.socket, buffer, strlen(buffer));
	if (res < 0) return -1;

	/* receive reply */
	nbytes = read(iface->control.socket, buffer, BUFFER_SIZE);
	buffer[nbytes] = 0;
	DEBUG("> %s\n", buffer);
	strcpy(reply, buffer);
	return nbytes;
}

static void
wpa_handle_messages(state_t *state, wpa_interface_t *iface)
{
	char buffer[BUFFER_SIZE];
	char params[512];
	int nbytes, idx = -1;
	wpa_action_t handle;

	wpa_action_t handles[TYPES_NUM][NETMAN_NUM_STATES] = {0};
	handles[CE_CON][NETMAN_STATE_CONNECTING] = netman_exit;

	wpa_ctrl_request(iface, "ATTACH", buffer);
	for (;;) {
		nbytes = read(iface->messages.socket, buffer, BUFFER_SIZE);
		/* remove trailing newline */
		buffer[nbytes] = 0;
		DEBUG("> %s\n", buffer);
		if (buffer[0] != '<') continue;
		idx = get_type(buffer+PLEVEL_LEN);
		if (idx < 0) continue;
		handle = handles[idx][state->state];
		if (handle) {
			get_param_str(buffer+PLEVEL_LEN, params, idx);
			handle(state, iface, params);
		}
	}
}

static void
wpa_configure_network(wpa_interface_t *iface, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	for (kv = network->options; kv; kv = kv->next) {
		if (snprintf(buffer, BUFFER_SIZE, "SET_NETWORK %d %s %s",
				network->id, kv->key, kv->value.str) < 0) {
			eprintf("Failed writing into buffer: SET_NETWORK %d %s %s\n",
					network->id, kv->key, kv->value.str);
			eprintf("Not calling the request\n");
			return;
		}
		wpa_ctrl_request(iface, buffer, buffer);
	}
}

static void
wpa_add_network(wpa_interface_t *iface, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	wpa_ctrl_request(iface, "ADD_NETWORK", buffer);
	sscanf(buffer, "%d", &network->id);
	wpa_configure_network(iface, network);
}

static void
wpa_enable_network(wpa_interface_t *iface, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	if (snprintf(buffer, BUFFER_SIZE, "ENABLE_NETWORK %d", network->id) < 0) {
		eprintf("Failed writing into buffer: ENABLE_NETWORK %d\n", network->id);
		eprintf("Not calling the request\n");
		return;
	}
	wpa_ctrl_request(iface, buffer, buffer);
}

static int
wpa_find_network(wpa_interface_t *iface, char *ssid)
{
	char buffer[BUFFER_SIZE], ssid_buffer[BUFFER_SIZE];
	int id;
	char *pos = buffer;

	wpa_ctrl_request(iface, "LIST_NETWORKS", buffer);
	while (pos = strchr(pos+1, '\n')) {
		if (sscanf(pos+1, "%d", &id) == 1) {
			snprintf(ssid_buffer, BUFFER_SIZE, "GET_NETWORK %d ssid", id);
			wpa_ctrl_request(iface, ssid_buffer, ssid_buffer);
			if (strcmp(ssid, ssid_buffer) == 0){
				return id;
			}
		}
	}
	return -1;
}

static int
wpa_socket_connect(wpa_socket_t *sock, char *socket_addr)
{
	int ret;

	sock->socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock->socket < 0) return 0;

	sock->local.sun_family = AF_UNIX;
	snprintf(sock->local.sun_path, sizeof(sock->local.sun_path),
			"/tmp/netman-%d", getpid());
	ret = bind(sock->socket, (struct sockaddr*) &sock->local,
			sizeof(struct sockaddr_un));
	if (ret < 0) return 0;

	sock->remote.sun_family = AF_UNIX;
	strcpy(sock->remote.sun_path, socket_addr);
	ret = connect(sock->socket, (struct sockaddr*) &sock->remote,
			sizeof(struct sockaddr_un));
	if (ret < 0) return 0;
	return 1;
}

static void
wpa_socket_close(wpa_socket_t *sock)
{
	unlink(sock->local.sun_path);
	close(sock->socket);
}

static int
wpa_interface_connect(wpa_interface_t *iface, char *socket_addr)
{
	int ret;
	if (!wpa_socket_connect(&iface->control, socket_addr))
		return 0;
	iface->messages = iface->control; /* for now, only use one socket */
	return 1;
}

wpa_interface_disconnect(wpa_interface_t *iface)
{
	wpa_socket_close(&iface->control);
	/* TODO: close messages when they have their own socket*/
}

/* TODO: eww. */
void
wpa_connect_to_network(state_t *state, char *interface, wpa_network_t *network)
{
	wpa_interface_t iface;
	char sock_addr[BUFFER_SIZE];
	int id;
	if (snprintf(sock_addr, BUFFER_SIZE, "%s/%s", SOCK_PATH, interface) < 0) {
		eprintf("Failed writing into buffer %s/%s\n", SOCK_PATH, interface);
		eprintf("Not connecting\n");
		return;
	}

	if (wpa_interface_connect(&iface, sock_addr)) {
		id = wpa_find_network(&iface, get_element("ssid", network->options).str);
		if (id > 0) {
			network->id = id;
		} else {
			wpa_add_network(&iface, network);
		}
		wpa_enable_network(&iface, network);
		wpa_handle_messages(state, &iface);
	} else {
		fprintf(stderr, "could not connect to wpa_suplicant\n");
	}
}

void
wpa_reconnect_to_network()
{
	/* char reply[BUFFER_SIZE]; */
	/* wpa_ctrl_t wpa_ctrl; */

	/* wpa_ctrl_connect(&wpa_ctrl, SOCK_PATH); */
	/* wpa_ctrl_request(&wpa_ctrl, "REASSOCIATE", reply); */

	/* wpa_ctrl_close(&wpa_ctrl); */
}

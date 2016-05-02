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

typedef void (*wpa_action_t)(state_t *state, char *params);

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
netman_exit(state_t *state, char *params)
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
wpa_ctrl_request(wpa_ctrl_t *wpa_ctrl, char *command, char *reply)
{
	char buffer[BUFFER_SIZE];
	int nbytes, res;

	strcpy(buffer, command);
	DEBUG("%s\n", buffer);
	res = write(wpa_ctrl->socket, buffer, strlen(buffer));
	if (res < 0) return -1;

	/* receive reply */
	nbytes = read(wpa_ctrl->socket, buffer, BUFFER_SIZE);
	/* remove trailing newline */
	buffer[nbytes-1] = 0;
	DEBUG("> %s\n", buffer);
	strcpy(reply, buffer);
	return nbytes;
}

static void
wpa_ctrl_handle_messages(state_t *state, wpa_ctrl_t *wpa_ctrl)
{
	char buffer[BUFFER_SIZE];
	char params[512];
	int nbytes, idx = -1;
	wpa_action_t handle;

	wpa_action_t handles[TYPES_NUM][NETMAN_NUM_STATES] = {0};
	handles[CE_CON][NETMAN_STATE_CONNECTING] = netman_exit;

	wpa_ctrl_request(wpa_ctrl, "ATTACH", buffer);
	for (;;) {
		nbytes = read(wpa_ctrl->socket, buffer, BUFFER_SIZE);
		/* remove trailing newline */
		buffer[nbytes] = 0;
		DEBUG("> %s\n", buffer);
		if (buffer[0] != '<') continue;
		idx = get_type(buffer+PLEVEL_LEN);
		if (idx < 0) continue;
		handle = handles[idx][state->state];
		if (handle) {
			get_param_str(buffer+PLEVEL_LEN, params, idx);
			handle(state, params);
		}
	}
}

static void
wpa_ctrl_configure_network(wpa_ctrl_t *wpa_ctrl, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	for (kv = network->options; kv; kv = kv->next) {
		if (snprintf(buffer, BUFFER_SIZE, "SET_NETWORK %s %s %s",
				network->id, kv->key, kv->value.str) < 0) {
			eprintf("Failed writing into buffer: SET_NETWORK %s %s %s\n",
					network->id, kv->key, kv->value.str);
			eprintf("Not calling the request\n");
			return;
		}
		wpa_ctrl_request(wpa_ctrl, buffer, buffer);
	}
}

static void
wpa_ctrl_register(wpa_ctrl_t *wpa_ctrl, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	wpa_ctrl_request(wpa_ctrl, "ADD_NETWORK", buffer);
	set_str(&network->id, buffer);
	wpa_ctrl_configure_network(wpa_ctrl, network);
}

static void
wpa_ctrl_enable(wpa_ctrl_t *wpa_ctrl, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	if (snprintf(buffer, BUFFER_SIZE, "ENABLE_NETWORK %s", network->id) < 0) {
		eprintf("Failed writing into buffer: ENABLE_NETWORK %s\n", network->id);
		eprintf("Not calling the request\n");
		return;
	}
	wpa_ctrl_request(wpa_ctrl, buffer, buffer);
}

static int
wpa_ctrl_connect(wpa_ctrl_t *wpa_ctrl, char* socket_addr)
{
	int ret;

	wpa_ctrl->socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (wpa_ctrl->socket < 0) return 0;

	wpa_ctrl->local.sun_family = AF_UNIX;
	snprintf(wpa_ctrl->local.sun_path, sizeof(wpa_ctrl->local.sun_path),
			"/tmp/netman-%d", getpid());
	ret = bind(wpa_ctrl->socket, (struct sockaddr*) &wpa_ctrl->local,
			sizeof(struct sockaddr_un));
	if (ret < 0) return 0;

	wpa_ctrl->remote.sun_family = AF_UNIX;
	strcpy(wpa_ctrl->remote.sun_path, socket_addr);
	ret = connect(wpa_ctrl->socket, (struct sockaddr*) &wpa_ctrl->remote,
			sizeof(struct sockaddr_un));
	if (ret < 0) return 0;
	return 1;
}

static void
wpa_ctrl_close(wpa_ctrl_t *wpa_ctrl)
{
	unlink(wpa_ctrl->local.sun_path);
	close(wpa_ctrl->socket);
}

void
wpa_connect_to_network(state_t *state, char *interface, wpa_network_t *network)
{
	wpa_ctrl_t wpa_ctrl;
	char sock_addr[BUFFER_SIZE];
	if (snprintf(sock_addr, BUFFER_SIZE, "%s/%s", SOCK_PATH, interface) < 0) {
		eprintf("Failed writing into buffer %s/%s\n", SOCK_PATH, interface);
		eprintf("Not connecting\n");
		return;
	}

	if (wpa_ctrl_connect(&wpa_ctrl, sock_addr)) {
		wpa_ctrl_register(&wpa_ctrl, network);
		wpa_ctrl_enable(&wpa_ctrl, network);
		wpa_ctrl_handle_messages(state, &wpa_ctrl);
	} else {
		fprintf(stderr, "could not connect to wpa_suplicant\n");
	}
}

void
wpa_reconnect_to_network()
{
	char reply[BUFFER_SIZE];
	wpa_ctrl_t wpa_ctrl;

	wpa_ctrl_connect(&wpa_ctrl, SOCK_PATH);
	wpa_ctrl_request(&wpa_ctrl, "REASSOCIATE", reply);

	wpa_ctrl_close(&wpa_ctrl);
}

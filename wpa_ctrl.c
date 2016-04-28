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

/* TODO:
 * - How are we going to handle knowing which handle was called?
 * - For example, CTRL-EVENT-CONNECTED, it's handle gets called
 *   should we set a flag or are we going to explicitly check for
 *   what the idx is?
 */

typedef int (*wpa_action_t)(char *);

static int
prompt_password(char *params)
{
	struct termios tp, save;
	char buf[512];

	if (tcgetattr(STDIN_FILENO, &tp) == -1) {
		perror("tcgetattr");
		return 0;
	}	

	save = tp;
	tp.c_lflag &= ~ECHO;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
		perror("tcsetattr");
		return 0;
	}

	printf("Password: ");
	fflush(stdout);

	if (fgets(buf, 512, stdin) == NULL) {
		eprintf("EOF Error\n");
		return 0;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1) {
		perror("tcsetattr");
	}
	
	strcpy(params, buf);
	return 1;
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

static wpa_action_t handles[TYPES_NUM] = {
	[CE_CON]           = NULL, /* CTRL-EVENT-CONNECTED */
	[CE_DCON]          = NULL, /* CTRL-EVENT-DISCONNECTED */
	[CE_TERM]          = NULL, /* CTRL-EVENT-TERMINATING */
	[CE_PASS_CHANGED]  = NULL, /* CTRL-EVENT-PASSWORD-CHANGED */
	[CE_EAP_NOTIF]     = NULL, /* CTRL-EVENT-EAP-NOTIFICATION */
	[CE_EAP_START]     = NULL, /* CTRL-EVENT-EAP-STARTED */
	[CE_EAP_METHOD]    = NULL, /* CTRL-EVENT-EAP-METHOD */
	[CE_EAP_SUCCESS]   = NULL, /* CTRL-EVENT-EAP-SUCCESS */
	[CE_EAP_FAIL]      = NULL, /* CTRL-EVENT-EAP-FAILURE */
	[CE_SCAN_RES]      = NULL, /* CTRL-EVENT-SCAN-RESULTS */
	[CE_BSS_ADD]       = NULL, /* CTRL-EVENT-BSS-ADDED */
	[CE_BSS_RM]        = NULL, /* CTRL-EVENT-BSS-REMOVED */
	[CR_ID]            = NULL, /* CTRL-REQ-IDENTITY */
	[CR_PASS]          = prompt_password, /* CTRL-REQ-PASSWORD */
	[CR_NEW_PASS]      = NULL, /* CTRL-REQ-NEW_PASSWORD */
	[CR_PIN]           = NULL, /* CTRL-REQ-PIN */
	[CR_OTP]           = NULL, /* CTRL-REQ-OTP */
	[CR_PASSPHRASE]    = NULL, /* CTRL-REQ-PASSPHRASE */
};

int
get_type(char buffer[BUFFER_SIZE])
{
	int i;
	for (i = 0; i < TYPES_NUM; i++) {
		if (strncmp(buffer, types[i], strlen(types[i])) == 0) {
			return i;
		}
	}
	return -1;
}

void
get_param_str(char buffer[BUFFER_SIZE], char params[512], int idx)
{
	char *type = types[idx];
	while (*buffer == *type) {
		type++;
		buffer++;
	}

	buffer++;

	strcpy(params, buffer);
}

static int
wpa_ctrl_request(wpa_ctrl_t *wpa_ctrl, char *command, char *reply)
{
	char buffer[BUFFER_SIZE];
	char params[512];
	int nbytes, res;
	int idx = -1;

	strcpy(buffer, command);
	DEBUG("%s\n", buffer);
	res = write(wpa_ctrl->socket, buffer, strlen(buffer));
	if (res < 0) return -1;

	/* receive reply */
	for (;;) {
		nbytes = read(wpa_ctrl->socket, buffer, BUFFER_SIZE);
		/* remove trailing newline */
		buffer[nbytes-1] = 0;
		DEBUG("> %s\n", buffer);
		if (buffer[0] == '<') {
			idx = get_type(buffer);
			get_param_str(buffer, params, idx);
			handles[idx](params);
		} else {
			strcpy(reply, buffer);
			return nbytes;
		}
	}
}

static void
wpa_ctrl_configure_network(wpa_ctrl_t *wpa_ctrl, wpa_network_t *network)
{
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	for (kv = network->options; kv; kv = kv->next) {
		snprintf(buffer, BUFFER_SIZE, "SET_NETWORK %s %s %s",
				network->id, kv->key, kv->value.str);
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
	snprintf(buffer, BUFFER_SIZE, "ENABLE_NETWORK %s", network->id);
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
wpa_connect_to_network(char *interface, wpa_network_t *network)
{
	wpa_ctrl_t wpa_ctrl;
	char sock_addr[BUFFER_SIZE];
	snprintf(sock_addr, BUFFER_SIZE, "%s/%s", SOCK_PATH, interface);

	if (wpa_ctrl_connect(&wpa_ctrl, sock_addr)) {
		wpa_ctrl_register(&wpa_ctrl, network);
		wpa_ctrl_enable(&wpa_ctrl, network);

		wpa_ctrl_close(&wpa_ctrl);
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

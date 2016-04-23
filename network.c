#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "util.h"

#define SOCK_PATH *PATH*
#define SSID *SSID*
#define PSK *PSK*
#define BUFFER_SIZE 4096

static int
wpa_ctrl_request(wpa_ctrl_t *wpa_ctrl, char *command, char *reply)
{
	char buffer[BUFFER_SIZE];
	int nbytes;

	strcpy(buffer, command);
	printf("%s\n", buffer);
	write(wpa_ctrl->socket, buffer, strlen(buffer));

	/* receive reply */
	for (;;) {
		nbytes = read(wpa_ctrl->socket, buffer, BUFFER_SIZE);
		/* remove trailing newline */
		buffer[nbytes-1] = 0;
		printf("> %s\n", buffer);
		if (buffer[0] == '<') {
			/* skip control messages for now */
		} else {
			strcpy(reply, buffer);
			return nbytes;
		}
	}
}

void
wpa_ctrl_configure_network(wpa_ctrl_t *wpa_ctrl, network_t *network)
{
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	for (kv = network->options; kv; kv = kv->next) {
		snprintf(buffer, BUFFER_SIZE, "SET_NETWORK %s %s %s",
				network->id, kv->key, kv->value);
		wpa_ctrl_request(wpa_ctrl, buffer, buffer);
	}
}

void
wpa_ctrl_register(wpa_ctrl_t *wpa_ctrl, network_t *network)
{
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	wpa_ctrl_request(wpa_ctrl, "ADD_NETWORK", buffer);
	set_str(&network->id, buffer);
	wpa_ctrl_configure_network(wpa_ctrl, network);
}

void
wpa_ctrl_enable(wpa_ctrl_t *wpa_ctrl, network_t *network)
{
	char buffer[BUFFER_SIZE];
	snprintf(buffer, BUFFER_SIZE, "ENABLE_NETWORK %s", network->id);
	wpa_ctrl_request(wpa_ctrl, buffer, buffer);
}

int
wpa_ctrl_connect(wpa_ctrl_t *wpa_ctrl, char* socket_addr)
{
	int ret;

	wpa_ctrl->socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (wpa_ctrl->socket < 0) return -1;

	wpa_ctrl->local.sun_family = AF_UNIX;
	snprintf(wpa_ctrl->local.sun_path, sizeof(struct sockaddr_un),
			"/tmp/netman-%d", getpid());
	ret = bind(wpa_ctrl->socket, (struct sockaddr*) &wpa_ctrl->local,
			sizeof(struct sockaddr_un));
	if (ret < 0) return -1;

	wpa_ctrl->remote.sun_family = AF_UNIX;
	strcpy(wpa_ctrl->remote.sun_path, socket_addr);
	ret = connect(wpa_ctrl->socket, (struct sockaddr*) &wpa_ctrl->remote,
			sizeof(struct sockaddr_un));
	if (ret < 0) return -1;
	return 0;
}

void
wpa_ctrl_close(wpa_ctrl_t *wpa_ctrl)
{
	unlink(wpa_ctrl->local.sun_path);
	close(wpa_ctrl->socket);
}

void
main()
{
	wpa_ctrl_t wpa_ctrl;
	network_t network;
	network.options = mk_keyvalue("ssid", SSID);
	network.options->next = mk_keyvalue("psk", PSK);

	wpa_ctrl_connect(&wpa_ctrl, SOCK_PATH);
	wpa_ctrl_register(&wpa_ctrl, &network);
	wpa_ctrl_enable(&wpa_ctrl, &network);

	wpa_ctrl_close(&wpa_ctrl);
}

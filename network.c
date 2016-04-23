#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "network.h"
#include "util.h"

#define SOCK_PATH *path*
#define SSID *ssid*
#define PSK *psk*
#define BUFFER_SIZE 4096

int
wpa_supplicant_request(int socket_fd, char* command, char* reply) {
	int nbytes;
	char buffer[BUFFER_SIZE];
	strcpy(buffer, command);
	write(socket_fd, buffer, strlen(buffer));
	nbytes = read(socket_fd, buffer, BUFFER_SIZE);
	/* remove trailing newline */
	buffer[nbytes-1] = 0;
	printf("GOT MESSAGE: %s\n", buffer);
	strcpy(reply, buffer);
	return nbytes;
}

void
connect_to_network(int socket_fd, network_t *network) {
	char buffer[BUFFER_SIZE];
	keyvalue_t *kv;
	wpa_supplicant_request(socket_fd, "ADD_NETWORK", buffer);
	set_str(&network->id, buffer);
	for (kv = network->options; kv; kv = kv->next) {
		snprintf(buffer, BUFFER_SIZE, "SET_NETWORK %s %s \"%s\"",
				network->id, kv->key, kv->value);
		printf("WRITING %s\n", buffer);
		wpa_supplicant_request(socket_fd, buffer, buffer);
	}
	snprintf(buffer, BUFFER_SIZE, "ENABLE_NETWORK %s", network->id);
	wpa_supplicant_request(socket_fd, buffer, buffer);
}

void
supplicant_test() {
	int socket_fd, nbytes;
	char reply[BUFFER_SIZE];

	network_t network;

	struct sockaddr_un addr;
	struct sockaddr_un local;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCK_PATH);
	local.sun_family = AF_UNIX;
	snprintf(local.sun_path, sizeof(struct sockaddr_un), "/tmp/netman-%d", getpid());

	socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	bind(socket_fd, (struct sockaddr*) &local, sizeof(local));
	connect(socket_fd, (struct sockaddr*) &addr, sizeof(addr));

	network.options = mk_keyvalue("ssid", SSID);
	network.options->next = mk_keyvalue("psk", PSK);

	connect_to_network(socket_fd, &network);

	close(socket_fd);
}

void
main() {
	supplicant_test();
}

/* void things(){ */
/* } */

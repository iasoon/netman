#ifndef WPA_CTRL_H
#define WPA_CTRL_H

#include <sys/socket.h>
#include <sys/un.h>

#include "netman_state.h"
#include "util.h"

struct wpa_network {
	int id;
	struct keyvalue *options;
};

struct wpa_socket {
	struct sockaddr_un local;
	struct sockaddr_un remote;
	int socket;
};

struct wpa_interface {
	struct wpa_socket control;
	struct wpa_socket messages;
};

typedef struct wpa_network wpa_network_t;
typedef struct wpa_socket wpa_socket_t;
typedef struct wpa_interface wpa_interface_t;

void wpa_connect_to_network(state_t *state, char *interface, wpa_network_t *network);
void wpa_reconnect_to_network();

#endif

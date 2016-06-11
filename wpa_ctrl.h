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
	struct wpa_network *current_network;
	/* map ssids to configured networks */
	HASHTABLE *networks;
};

typedef struct wpa_network WPA_NETWORK;
typedef struct wpa_socket WPA_SOCKET;
typedef struct wpa_interface WPA_INTERFACE;

__BEGIN_DECLS

size_t	wpa_request(const WPA_INTERFACE *iface, char *reply, const char *fmt, ...);
void	wpa_connect_to_network(STATE *state, char *interface, KEYVALUE *options);
void	wpa_reconnect_to_network();

__END_DECLS

#endif

#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <sys/un.h>

struct network {
	char *alias;
	char *id;
	struct keyvalue *options;
};

struct wpa_ctrl {
	struct sockaddr_un local;
	struct sockaddr_un remote;
	int socket;
};

typedef struct network network_t;
typedef struct wpa_ctrl wpa_ctrl_t;

#endif

#ifndef IF_CTRL_H_
#define IF_CTRL_H_

#include <errno.h>
#define __USE_MISC
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

struct if_ctrl {
	struct ifreq if_req; /* To access the name of the iface: if_req.ifr_name */
	int socket;
};

typedef struct if_ctrl if_ctrl_t;

int if_ctrl_check_link(if_ctrl_t *if_ctrl);
int wired_toggle_network(char *iface, int val);
int wired_connect_to_network(char *iface);
int wired_disconnect_from_network(char *iface);

/* TODO: Clean up function */

#endif

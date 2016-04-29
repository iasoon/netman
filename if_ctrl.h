#ifndef IF_CTRL_H_
#define IF_CTRL_H_

#include <errno.h>
#include <ifaddrs.h>
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

int if_up(char *iface);
int if_down(char *iface);
int if_reenable();

/* TODO: Clean up function */

#endif

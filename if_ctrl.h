#ifndef IF_CTRL_H_
#define IF_CTRL_H_

#include <errno.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

struct if_ctrl {
	struct ifreq if_req;
	struct iwreq iw_req;
	struct ifaddrs *ifaddr;
	int socket;
};

typedef struct if_ctrl if_ctrl_t;

int if_ctrl_check_link(if_ctrl_t *if_ctrl, const char *ifname);
int if_ctrl_check_wireless(if_ctrl_t *if_ctrl, const char *ifname, char *protocol);
int if_ctrl_populate_ifaddrs(if_ctrl_t *if_ctrl);
void if_ctrl_free_ifaddrs(if_ctrl_t *if_ctrl);
char *if_ctrl_get_default_wired(if_ctrl_t *if_ctrl);
char *if_ctrl_get_default_wireless(if_ctrl_t *if_ctrl, char protocol[IFNAMSIZ]);
void if_ctrl_print_addrs(if_ctrl_t *if_ctrl);

/* TODO: Clean up function */

#endif

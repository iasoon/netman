#ifndef IF_CTRL_H_
#define IF_CTRL_H_

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <ifaddrs.h>

#ifdef __linux__
#define __USE_MISC
#include <linux/if.h>
#else /* !__linux__ */
#include <net/if.h>
#endif /* __linux__ */

#include <netinet/in.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct if_ctrl {
	struct ifreq if_req; /* To access the name of the iface: if_req.ifr_name */
	int socket;
};

typedef struct if_ctrl IF_CTRL;

__BEGIN_DECLS

int	if_up(char *iface);
int	if_down(char *iface);
int	if_reenable();

/* TODO: Clean up function */

__END_DECLS

#endif

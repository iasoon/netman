#include "if_ctrl.h"

static if_ctrl_t default_if_ctrl = {
	.socket = -1,
};

static int
if_ctrl_connect_socket(if_ctrl_t *if_ctrl)
{
	if_ctrl->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (if_ctrl->socket < 0) {
		perror("Socket failed.");
		return 0;
	} 

	return 1;
}

static int
if_ctrl_disconnect_socket(if_ctrl_t *if_ctrl)
{
	if_ctrl->socket = close(if_ctrl->socket);
	if (if_ctrl->socket < 0) {
		perror("Socket closing failed.");
		return 0;
	}

	return 1;
}

static int
if_ctrl_set_flags(if_ctrl_t *if_ctrl, int val)
{
	int ret_val = -1;
	int flags = 0;
	
	ret_val = ioctl(if_ctrl->socket, SIOCGIFFLAGS, &if_ctrl->if_req);
	if (ret_val == -1) {
		perror("Ioctl failed.");
		return 0;
	}

	flags = if_ctrl->if_req.ifr_flags;

	if (val < 0) {
		val = -val;
		flags &= ~val;
	} else {
		flags |= val;
	}

	if_ctrl->if_req.ifr_flags = flags;

	ret_val = ioctl(if_ctrl->socket, SIOCSIFFLAGS, &if_ctrl->if_req);

	if (ret_val == -1) {
		perror("Ioctl failed.");
		return 0;
	}

	return 1;
}
	
static int
if_check_link(char *iface)
{
	int ret_val = -1;
	if_ctrl_t if_ctrl = default_if_ctrl;
	if_ctrl_connect_socket(&if_ctrl);

	if (strcpy(if_ctrl.if_req.ifr_name, iface) == NULL) {
		if_ctrl_disconnect_socket(&if_ctrl);
		return 0;
	}

	ret_val = ioctl(if_ctrl.socket, SIOCGIFFLAGS, &if_ctrl.if_req);
	if (ret_val == -1) {
		perror("Ioctl failed.");
		if_ctrl_disconnect_socket(&if_ctrl);
		return 0;
	}

	if_ctrl_disconnect_socket(&if_ctrl);

	return (if_ctrl.if_req.ifr_flags & IFF_UP) &&
		(if_ctrl.if_req.ifr_flags & IFF_RUNNING);
}


static int
if_toggle(char *iface, int val)
{
	if_ctrl_t if_ctrl = default_if_ctrl;
	if_ctrl_connect_socket(&if_ctrl);

	if (strcpy(if_ctrl.if_req.ifr_name, iface) == NULL) {
		if_ctrl_disconnect_socket(&if_ctrl);
		return 0;
	}

	if (if_ctrl_set_flags(&if_ctrl, val) == 0) {
		if_ctrl_disconnect_socket(&if_ctrl);
		return 0;
	}

	if_ctrl_disconnect_socket(&if_ctrl);

	return 1;
}

int
if_up(char *iface)
{
	return if_toggle(iface, IFF_UP);
}

int
if_down(char *iface)
{
	return if_toggle(iface, -IFF_UP);
}

int
if_reenable()
{
	struct ifaddrs *addrs = NULL, *addr = NULL;

	if (getifaddrs(&addrs) != 0) {
		perror("getifaddrs");
		return 0;
	}

	for (addr = addrs; addr; addr = addr->ifa_next) {
		/* Check only the interfaces that are up 
		 * This also includes the interfaces that might be:
		 * - unknown
		 * - dormant
		 * - up
		 */

		/* The problem:
		 * All the UNIX systems keep AF_LINK as the sa_family
		 * Linux keeps AF_PACKET.
		*/
#ifdef __linux__
		if (addr->ifa_addr && 
		    addr->ifa_addr->sa_family == AF_PACKET &&
		    strcmp(addr->ifa_name, "lo") != 0 &&
		    if_check_link(addr->ifa_name) != 0) {
#else /* !__linux__ */
		if (addr->ifa_addr &&
		    addr->ifa_addr->sa_family == AF_LINK &&
		    strcmp(addr->ifa_name, "lo") != 0 &&
		    if_check_link(addr->ifa_name) != 0) {
#endif /* __linux__ */

			if (if_down(addr->ifa_name) == 0) {
				eprintf("Failed to put the interface down: %s\n", 
				         addr->ifa_name);
				return 0;
			}

			if (if_up(addr->ifa_name) == 0) {
				eprintf("Failed to put the interface up: %s\n",
				         addr->ifa_name);
				return 0;
			}
		}
	}

	freeifaddrs(addrs);
	return 1;
}

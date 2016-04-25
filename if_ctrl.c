#include "if_ctrl.h"

int
if_ctrl_check_link(if_ctrl_t *if_ctrl, const char *ifname)
{
	int ret_val = -1;
	if_ctrl->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (if_ctrl->socket < 0) {
		perror("Socket failed.");
		return 0;
	}

	if (strncpy(if_ctrl->if_req.ifr_name, ifname, IFNAMSIZ) == NULL) {
		/* TODO: Handle error */
	}
	ret_val = ioctl(if_ctrl->socket, SIOCGIFFLAGS, &if_ctrl->if_req);

	close(if_ctrl->socket);

	if (ret_val == -1) {
		perror("Ioctl failed.");
		return 0;
	}

	return (if_ctrl->if_req.ifr_flags & IFF_UP) &&
		(if_ctrl->if_req.ifr_flags & IFF_RUNNING);
}

int
if_ctrl_check_wireless(if_ctrl_t *if_ctrl, const char *ifname, char *protocol)
{
	int ret_val = -1;
	memset(&if_ctrl->iw_req, 0, sizeof(if_ctrl->iw_req));
	if (strncpy(if_ctrl->iw_req.ifr_name, ifname, IFNAMSIZ) == NULL) {
		/* TODO: Handle error */
	}

	if_ctrl->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (if_ctrl->socket == -1) {
		perror("Socket failed.");
		return 0;
	}

	ret_val = ioctl(if_ctrl->socket, SIOCGIWNAME, &if_ctrl->iw_req);
	
	if (ret_val != -1) {
		if (strncpy(protocol, if_ctrl->iw_req.u.name, IFNAMSIZ) == NULL) {
			/* TODO: Handle error */
		}
		close(if_ctrl->socket);
		return 1;
	}

	close(if_ctrl->socket);

	return 0;
}

int
if_ctrl_populate_ifaddrs(if_ctrl_t *if_ctrl)
{
	if (getifaddrs(&if_ctrl->ifaddr) == -1) {
		perror("getifaddrs");
		return 0;
	}

	return 1;
}

void
if_ctrl_free_ifaddrs(if_ctrl_t *if_ctrl)
{
	freeifaddrs(if_ctrl->ifaddr);
}

char *
if_ctrl_get_default_wired(if_ctrl_t *if_ctrl)
{
	struct ifaddrs *addr = NULL;
	char protocol[IFNAMSIZ] = {0};

	for (addr = if_ctrl->ifaddr; addr != NULL; addr = addr->ifa_next) {
		if (addr->ifa_addr == NULL ||
				addr->ifa_addr->sa_family != AF_PACKET) continue;

		if (!if_ctrl_check_wireless(if_ctrl, addr->ifa_name, protocol) &&
				strcmp(addr->ifa_name, "lo") != 0)
			return addr->ifa_name;
	}
	return NULL;
}

char *
if_ctrl_get_default_wireless(if_ctrl_t *if_ctrl, char protocol[IFNAMSIZ])
{
	struct ifaddrs *addr = NULL;

	for (addr = if_ctrl->ifaddr; addr != NULL; addr = addr->ifa_next) {
		if (addr->ifa_addr == NULL ||
				addr->ifa_addr->sa_family != AF_PACKET) continue;

		if (if_ctrl_check_wireless(if_ctrl, addr->ifa_name, protocol))
			return addr->ifa_name;
	}

	return NULL;
}

void
if_ctrl_print_addrs(if_ctrl_t *if_ctrl)
{
	struct ifaddrs *addr;

	for (addr = if_ctrl->ifaddr; addr != NULL; addr = addr->ifa_next) {
		if (addr->ifa_addr == NULL ||
				addr->ifa_addr->sa_family != AF_PACKET) continue;

		printf("Interface %s\n", addr->ifa_name);
	}
}

#include "if_ctrl.h"

int
if_ctrl_connect_socket(if_ctrl_t *if_ctrl)
{
	if_ctrl->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (if_ctrl->socket < 0) {
		perror("Socket failed.");
		return 0;
	} 

	return 1;
}

int
if_ctrl_disconnect_socket(if_ctrl_t *if_ctrl)
{
	if_ctrl->socket = close(if_ctrl->socket);
	if (if_ctrl->socket < 0) {
		perror("Socket closing failed.");
		return 0;
	}

	return 1;
}

int
if_ctrl_check_link(if_ctrl_t *if_ctrl)
{
	int ret_val = -1;

	ret_val = ioctl(if_ctrl->socket, SIOCGIFFLAGS, &if_ctrl->if_req);
	if (ret_val == -1) {
		perror("Ioctl failed.");
		return 0;
	}

	return (if_ctrl->if_req.ifr_flags & IFF_UP) &&
		(if_ctrl->if_req.ifr_flags & IFF_RUNNING);
}

int
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

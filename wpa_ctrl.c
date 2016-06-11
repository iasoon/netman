#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "netman_config.h"
#include "util.h"
#include "wpa_ctrl.h"
#include "wpa_handlers.h"

#define SOCK_PATH "/var/run/wpa_supplicant"
#define BUFFER_SIZE 4096

#define SSID_SIZE 33 /* 32 characters plus terminator */

#define PLEVEL_LEN 3

enum {
	CE_CON = 0,
	CE_DCON,
	CE_TERM,
	CE_PASS_CHANGED,
	CE_EAP_NOTIF,
	CE_EAP_START,
	CE_EAP_METHOD,
	CE_EAP_SUCCESS,
	CE_EAP_FAIL,
	CE_SCAN_RES,
	CE_BSS_ADD,
	CE_BSS_RM,
	CR_ID,
	CR_PASS,
	CR_NEW_PASS,
	CR_PIN,
	CR_OTP,
	CR_PASSPHRASE,
	TYPES_NUM,
};

typedef void (*WPA_ACTION)(STATE *state, WPA_INTERFACE *iface, char *params);


static char types[TYPES_NUM][128] = {
	[CE_CON]           = "CTRL-EVENT-CONNECTED",
	[CE_DCON]          = "CTRL-EVENT-DISCONNECTED",
	[CE_TERM]          = "CTRL-EVENT-TERMINATING",
	[CE_PASS_CHANGED]  = "CTRL-EVENT-PASSWORD_CHANGED",
	[CE_EAP_NOTIF]     = "CTRL-EVENT-EAP-NOTIFICATION",
	[CE_EAP_START]     = "CTRL-EVENT-EAP-STARTED",
	[CE_EAP_METHOD]    = "CTRL-EVENT-EAP-METHOD",
	[CE_EAP_SUCCESS]   = "CTRL-EVENT-EAP-SUCCESS",
	[CE_EAP_FAIL]      = "CTRL-EVENT-EAP-FAILURE",
	[CE_SCAN_RES]      = "CTRL-EVENT-SCAN-RESULTS",
	[CE_BSS_ADD]       = "CTRL-EVENT-BSS-ADDED",
	[CE_BSS_RM]        = "CTRL-EVENT-BSS-REMOVED",
	[CR_ID]            = "CTRL-REQ-IDENTITY",
	[CR_PASS]          = "CTRL-REQ-PASSWORD",
	[CR_NEW_PASS]      = "CTRL-REQ-NEW_PASSWORD",
	[CR_PIN]           = "CTRL-REQ-PIN",
	[CR_OTP]           = "CTRL-REQ-OTP",
	[CR_PASSPHRASE]    = "CTRL-REQ-PASSPHRASE",
};

static int
get_type(char buffer[BUFFER_SIZE])
{
	int i;
	for (i = 0; i < TYPES_NUM; i++) {
		/* Checks the beginning of the string */
		if (strncmp(buffer, types[i], strlen(types[i])) == 0) {
			return i;
		}
	}
	return -1;
}

static void
get_param_str(char buffer[BUFFER_SIZE], char params[512], int idx)
{
	char *type = types[idx];
	/* Assume the type is shorter than the actual message */
	while (*buffer == *type) {
		type++;
		buffer++;
	}

	/* Strip the - or the whitespace as documented in
	 * https://w1.fi/wpa_supplicant/devel/ctrl_iface_page.html
	 */
	buffer++;

	strcpy(params, buffer);
}

/* returns: bytes read */
static size_t
_wpa_request(const WPA_INTERFACE *iface, char *reply, const char *fmt, va_list args)
{
	char buffer[BUFFER_SIZE];
	size_t nbytes;
	int res;

	if(vsnprintf(buffer, BUFFER_SIZE, fmt, args) < 0) {
		eprintf("Failed writing to buffer\n");
		return -1;
	}

	DEBUG("> %s\n", buffer);
	res = write(iface->control.socket, buffer, strlen(buffer));
	if (res < 0) return -1;

	/* receive reply */
	nbytes = read(iface->control.socket, reply, BUFFER_SIZE);
	reply[nbytes] = 0;
	DEBUG("%s\n", reply);
	return nbytes;
}

size_t
wpa_request(const WPA_INTERFACE *iface, char *reply, const char *fmt, ...)
{
	va_list args;
	size_t size;
	va_start(args, fmt);
	size = _wpa_request(iface, reply, fmt, args);
	va_end(args);
	return size;
}

static KEYVALUE *
wpa_request_kv(const WPA_INTERFACE *iface, const char *fmt, ...)
{
	va_list args;
	char buffer[BUFFER_SIZE];
	size_t size;
	FILE *handle;
	KEYVALUE *kv = 0;
	va_start(args, fmt);
	size = _wpa_request(iface, buffer, fmt, args);
	va_end(args);
	if (size > 0){
		handle = fmemopen(buffer, size, "r");
		kv = read_keyvalue(handle);
		fclose(handle);
	}
	return kv;
}

/* returns: success */
static int
wpa_command(const WPA_INTERFACE *iface, const char *fmt, ...)
{
	va_list args;
	char buffer[BUFFER_SIZE];
	int ret;
	va_start(args, fmt);
	ret = _wpa_request(iface, buffer, fmt, args);
	va_end(args);
	if (ret != 0 || strcmp(buffer, "OK\n") != 0) {
		return 0;
	}
	return 1;
}

static void
wpa_handle_messages(STATE *state, WPA_INTERFACE *iface)
{
	char buffer[BUFFER_SIZE];
	char params[512];
	int nbytes, idx = -1;
	WPA_ACTION handle;
	
	WPA_ACTION handles[TYPES_NUM][NETMAN_NUM_STATES] = {{0}};
	handles[CE_CON][NETMAN_STATE_CONNECTING] = netman_exit;
	handles[CR_PASS][NETMAN_STATE_CONNECTING] = prompt_password;
	
	wpa_command(iface, "ATTACH");
	for (;;) {
		nbytes = read(iface->messages.socket, buffer, BUFFER_SIZE);
		/* remove trailing newline */
		buffer[nbytes] = 0;
		DEBUG("> %s\n", buffer);
		if (buffer[0] != '<') continue;
		idx = get_type(buffer+PLEVEL_LEN);

		if (idx < 0) continue;
		
		handle = handles[idx][state->state];
		if (handle) {
			get_param_str(buffer+PLEVEL_LEN, params, idx);
			handle(state, iface, params);
		}
	}
}

static void
wpa_configure_network(WPA_INTERFACE *iface, WPA_NETWORK *network)
{
	KEYVALUE *kv;
	for (kv = network->options; kv; kv = kv->next) {
		/* TODO: failure */
		wpa_command(iface, "SET_NETWORK %d %s %s", network->id, kv->key, kv->value.str);
	}
}

static WPA_NETWORK *
wpa_add_network(WPA_INTERFACE *iface, KEYVALUE *options)
{
	char buffer[BUFFER_SIZE];
	WPA_NETWORK *net = malloc(sizeof(WPA_NETWORK));
	/* TODO: handle errors */
	wpa_request(iface, buffer, "ADD_NETWORK");
	sscanf(buffer, "%d", &net->id);
	net->options = options;
	wpa_configure_network(iface, net);
	hash_add(iface->networks, get_element("ssid", options).str, net);
	return net;
}

static void
wpa_enable_network(WPA_INTERFACE *iface, WPA_NETWORK *network)
{
	wpa_command(iface, "ENABLE_NETWORK %d", network->id);
}

static void
wpa_fetch_networks(WPA_INTERFACE *iface)
{
	char buffer[BUFFER_SIZE];
	char *id, *ssid, *bssid, *flags, *start = buffer;
	char *quoted_ssid, *end;
	WPA_NETWORK *net;

	wpa_request(iface, buffer, "LIST_NETWORKS");
	/* skip header line */
	start = strchr(buffer, '\n') + 1;
	while (strchr(start, '\n')) {
		id = start;
		ssid = strchr(id, '\t');
		*ssid++ = '\0';
		bssid = strchr(ssid, '\t');
		*bssid++ = '\0';
		flags = strchr(bssid, '\t');
		*flags++ = '\0';
		start = strchr(flags, '\n');
		*start++ = '\0';

		/* TODO: update networks if they already exist */
		if (*id && *ssid) {
			net = malloc(sizeof(WPA_NETWORK));
			/* TODO: fill properties */
			net->id = atoi(id);
			quoted_ssid = strdup(ssid);

			/* process flags */
			while (*flags) {
				start = strchr(flags, '[') + 1;
				flags = strchr(flags, ']');
				*flags++ = '\0';
				if (strcmp(start, "CURRENT") == 0){
					iface->current_network = net;
				}
			}

			hash_add(iface->networks, quoted_ssid, net);
		}
	}
}

static int
wpa_socket_connect(WPA_SOCKET *sock, char *socket_addr)
{
	int ret;

	sock->socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock->socket < 0) return 0;

	sock->local.sun_family = AF_UNIX;
	snprintf(sock->local.sun_path, sizeof(sock->local.sun_path),
	    "/tmp/netman-%d", getpid());
	ret = bind(sock->socket, (struct sockaddr*) &sock->local,
	    sizeof(struct sockaddr_un));
	if (ret < 0) return 0;

	sock->remote.sun_family = AF_UNIX;
	strcpy(sock->remote.sun_path, socket_addr);
	ret = connect(sock->socket, (struct sockaddr*) &sock->remote,
	    sizeof(struct sockaddr_un));
	if (ret < 0) return 0;
	return 1;
}

static void
wpa_socket_close(WPA_SOCKET *sock)
{
	unlink(sock->local.sun_path);
	close(sock->socket);
}

static int
wpa_interface_connect(WPA_INTERFACE *iface, char *socket_addr)
{
	if (wpa_socket_connect(&iface->control, socket_addr) == 0)
		return 0;
	iface->messages = iface->control; /* for now, only use one socket */
	return 1;
}

static void
wpa_interface_disconnect(WPA_INTERFACE *iface)
{
	wpa_socket_close(&iface->control);
	/* TODO: close messages when they have their own socket*/
}

static void
wpa_interface_init(WPA_INTERFACE *iface, char *interface)
{
	char sock_addr[BUFFER_SIZE];

	memset(&iface->control, 0, sizeof(WPA_SOCKET));
	memset(&iface->messages, 0, sizeof(WPA_SOCKET));
	iface->current_network = 0;
	iface->networks = mk_hashtable(64);

	snprintf(sock_addr, BUFFER_SIZE, "%s/%s", SOCK_PATH, interface);
	/* TODO: handle errors */
	wpa_interface_connect(iface, sock_addr);
	wpa_fetch_networks(iface);

}

/* TODO: eww. */
void
wpa_connect_to_network(STATE *state, char *interface, KEYVALUE *options)
{
	WPA_INTERFACE iface;
	WPA_NETWORK *net;
	wpa_interface_init(&iface, interface);
	net = hash_get_ptr(iface.networks, get_element("ssid", options).str);
	if (net == 0) {
		net = wpa_add_network(&iface, options);
	} else {
		/* TODO: merge options instead of overwriting? */
		net->options = options;
		wpa_configure_network(&iface, net);
	}
	if (net != iface.current_network){
		wpa_enable_network(&iface, net);
		wpa_handle_messages(state, &iface);
	}
	wpa_interface_disconnect(&iface);
}

void
wpa_reconnect_to_network()
{
	/* char reply[BUFFER_SIZE]; */
	/* wpa_ctrl_t wpa_ctrl; */

	/* wpa_ctrl_connect(&wpa_ctrl, SOCK_PATH); */
	/* wpa_ctrl_request(&wpa_ctrl, "REASSOCIATE", reply); */

	/* wpa_ctrl_close(&wpa_ctrl); */
}

#include "network.h"


/* TODO: Generalize this function, do not assume that there are no
 * more children in the config.
 * Example:
 *    eduroam = {
 *        wireless = {
 *            ssid     = "eduroam"
 *            password = "<password>"
 *            ... 
 *            1 = {
 *                bssid = 00:11:22:33:44:55
 *            }
 *
 *            2 = {
 *                bssid = 11:22:33:44:55:66
 *            }
 *            ...
 *        }
 *    }
 */
void
netman_connect(STATE *state)
{
	KEYVALUE *network_config = get_element(
	            state->options.network,
	            state->config).child;

	char *iface = get_element("interface", network_config).str;
	KEYVALUE *wpa_config = get_element("wireless", network_config).child;

	state->state = NETMAN_STATE_CONNECTING;
	
	if (wpa_config != NULL) {
		wpa_connect_to_network(state, iface, wpa_config);
	} else {
		if (if_up(iface) == 0) {
			eprintf("Failed to connect to network with interface: %s\n", iface);
		}
	}
}

void
netman_reconnect(OPTIONS *options)
{
	DEBUG("TODO: %s\n", options->network);
	if_reenable();
}

/* TODO: Implement blacklisting with wpa_supplicant - builtin
 * ENABLE_NETWORK - DISABLE_NETWORK
 * Config entry?
 */
void
netman_blacklist(OPTIONS *options)
{
	DEBUG("TODO: %s\n", options->network);
}

void
netman_scan(OPTIONS *options)
{
	DEBUG("TODO: %s\n", options->network);
}

void
netman_scan_networks(OPTIONS *options)
{
	DEBUG("TODO: %s\n", options->network);
}

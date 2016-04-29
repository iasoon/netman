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
netman_connect(options_t *options)
{
	wpa_network_t network;
	network.id = 0;
	keyvalue_t *network_config = get_element(options->name, options->config).child;
	char *iface = get_element("interface", network_config).str;
	keyvalue_t *wpa_config = get_element("wireless", network_config).child;
	if (wpa_config != NULL) {
		network.options = wpa_config;
		wpa_connect_to_network(iface, &network);
	} else {
		if (if_up(iface) == 0) {
			eprintf("Failed to connect to network with interface: %s\n", iface);
		}
	}
}

void
netman_reconnect(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
	if_reenable();	
}

/* TODO: Implement blacklisting with wpa_supplicant - builtin
 * ENABLE_NETWORK - DISABLE_NETWORK
 * Config entry?
 */
void
netman_blacklist(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
}

void
netman_scan(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
}

void
netman_scan_networks(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
}

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
netman_connect(state_t *state)
{
	keyvalue_t *network_config = get_element(
			state->options.network,
			state->config
		).child;
	char *iface = get_element("interface", network_config).str;
	keyvalue_t *wpa_config = get_element("wireless", network_config).child;

	char *psk = get_element("psk", wpa_config).str;
	char *pwd = (psk) ? psk : get_element("password", wpa_config).str;

	if (pwd) { 
		state->state = NETMAN_STATE_CONNECTING;
	} else {
		state->state = NETMAN_STATE_PROMPT_PW;
	}

	DEBUG("state: %d", state->state);
	
	if (wpa_config != NULL) {
		wpa_connect_to_network(state, iface, wpa_config);
	} else {
		if (if_up(iface) == 0) {
			eprintf("Failed to connect to network with interface: %s\n", iface);
		}
	}
}

void
netman_reconnect(options_t *options)
{
	DEBUG("TODO: %s\n", options->network);
	if_reenable();	
}

/* TODO: Implement blacklisting with wpa_supplicant - builtin
 * ENABLE_NETWORK - DISABLE_NETWORK
 * Config entry?
 */
void
netman_blacklist(options_t *options)
{
	DEBUG("TODO: %s\n", options->network);
}

void
netman_scan(options_t *options)
{
	DEBUG("TODO: %s\n", options->network);
}

void
netman_scan_networks(options_t *options)
{
	DEBUG("TODO: %s\n", options->network);
}

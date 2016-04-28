#include "network.h"

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
		if (wired_connect_to_network(iface) == 0) {
			eprintf("Failed to connect to network with interface: %s\n", iface);
		}
	}
}

void
netman_reconnect(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
	wpa_reconnect_to_network();	
}

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

#include "network.h"

void
netman_connect(options_t *options)
{
	wpa_network_t network;
	network.id = 0;
	keyvalue_t *network_config = get_element(options->name, options->config).child;
	keyvalue_t *wpa_config = get_element("wireless", network_config).child;
	network.options = wpa_config;
	connect_to_network(&network);
}

void
netman_reconnect(options_t *options)
{
	DEBUG("TODO: %s\n", options->name);
	reconnect_to_network();	
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

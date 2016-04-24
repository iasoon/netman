#include "network.h"

void
netman_connect(options_t *options)
{
	wpa_network_t network;
	network.name = options->name;
	network.options = options->kv_pair;
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

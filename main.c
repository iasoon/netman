#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "if_ctrl.h"
#include "netman_config.h"
#include "network.h"
#include "util.h"
#include "wpa_ctrl.h"

static if_ctrl_t default_if_ctrl = {
	.socket = -1,
};

static options_t default_opts = {
	.cfg_path  = NULL,
	.name      = NULL,
	.interface = NULL,
	.kv_pair   = NULL,
	.no_save   = 0,
	.verbose   = 0,
	.quiet     = 0,
	.config    = 0
};

static action_t default_action = {
	.cmd  = NULL,
	.opts = NULL
};

static int
arg_parse(int argc, char *argv[], action_t *config)
{
	int ret_code;
	int opt_idx = 0;
	char *ssid_str = NULL;
	char *psk_str = NULL;

	struct option long_options[] = {
		{ "no-save",       no_argument,       &config->opts->no_save,  0  },
		{ "verbose",       no_argument,       0,                      'v' },
		{ "quiet",         no_argument,       0,                      'q' },
		{ "connect",       required_argument, 0,                      'c' },
		{ "blacklist",     required_argument, 0,                      'b' },
		{ "reconnect",     no_argument,       0,                      'r' },
		{ "passphrase",    required_argument, 0,                      'p' },
		{ "scan",          no_argument,       0,                      's' },
		{ "scan-networks", no_argument,       0,                      'S' },
		{ "ssid",          required_argument, 0,                      'n' },
	};

	opterr = 0;

	while ((ret_code = getopt_long(argc, argv, "vqc:b:rp:sSn:", 
					long_options, &opt_idx)) != -1) {
		DEBUG("ret_code: %d\n", ret_code);
		switch (ret_code) {
			case 0:
				DEBUG("Option %s\n", long_options[opt_idx].name);
				if (long_options[opt_idx].flag != 0) break;
				break;
			case 'v':
				if (config->opts->quiet == 0)
					config->opts->verbose = 1;
				DEBUG("Verbose flag %d\n", config->opts->verbose);
				break;
			case 'q':
				if (config->opts->verbose == 0)
					config->opts->quiet = 1;
				DEBUG("Quiet flag %d\n", config->opts->quiet);
				break;
			case 'c':
				if (config->cmd == NULL) {
					config->cmd = netman_connect;
					set_str(&config->opts->name, optarg);
					DEBUG("Connect to %s\n", config->opts->name);
				}
				break;
			case 'b':
				if (config->cmd == NULL) {
					config->cmd = netman_blacklist;
					set_str(&config->opts->name, optarg);
					DEBUG("Blacklist %s\n", config->opts->name);
				}
				break;
			case 'r':
				if (config->cmd == NULL) {
					DEBUG("Reconnect\n");
					config->cmd = netman_reconnect;
				}
				break;
			case 'p': /* Keyvalue */
				set_str_quote(&psk_str, optarg);
				config->opts->kv_pair = mk_keyvalue("psk", psk_str, config->opts->kv_pair, VALUE_STR);
				DEBUG("Set the PSK: %s\n", psk_str);
				free(psk_str);
				break;
			case 's':
				if (config->cmd == NULL) {
					config->cmd = netman_scan;
					DEBUG("Scan\n");
				}
				break;
			case 'S':
				if (config->cmd == NULL) {
					config->cmd = netman_scan_networks;
					DEBUG("Scan networks\n");
				}
				break;
			case 'n': /* Keyvalue */
				set_str_quote(&ssid_str, optarg);
				config->opts->kv_pair = mk_keyvalue("ssid", ssid_str, config->opts->kv_pair, VALUE_STR);
				free(ssid_str);
				DEBUG("Set the SSID\n");
				break;
			default:
				eprintf("Sumting wong\n");
		}
	}
	return 0;
}

int
main(int argc, char *argv[])
{
	if_ctrl_t if_ctrl = default_if_ctrl;
	action_t action = default_action;

	if_ctrl_connect_socket(&if_ctrl);

	options_t opts = default_opts;
	opts.config = netman_get_config();

	action.opts = &opts;
	arg_parse(argc, argv, &action);
	action.cmd(action.opts);

	if_ctrl_disconnect_socket(&if_ctrl);
	free_kv(action.opts->kv_pair);
	return 0;
}

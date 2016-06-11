#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "if_ctrl.h"
#include "netman_config.h"
#include "netman_state.h"
#include "network.h"
#include "util.h"
#include "wpa_ctrl.h"

static OPTIONS default_opts = {
	.cfg_path    = NULL,
	.network     = NULL,
	.interface   = NULL,
	.wpa_options = NULL,
	.no_save     = 0,
	.verbose     = 0,
	.quiet       = 0,
};

static int
arg_parse(int argc, char *argv[], int *mode, OPTIONS *options)
{
	int ret_code;
	int opt_idx = 0;
	char *ssid_str = NULL;
	char *psk_str = NULL;

	struct option long_options[] = {
	    { "no-save",       no_argument,       &options->no_save,       0  },
	    { "verbose",       no_argument,       0,                      'v' },
	    { "quiet",         no_argument,       0,                      'q' },
	    { "connect",       required_argument, 0,                      'c' },
	    { "blacklist",     required_argument, 0,                      'b' },
	    { "reconnect",     no_argument,       0,                      'r' },
	    { "passphrase",    required_argument, 0,                      'p' },
	    { "scan",          no_argument,       0,                      's' },
	    { "ssid",          required_argument, 0,                      'n' },
	};

	opterr = 0;
	
	while ((ret_code = getopt_long(argc, argv, "vqc:b:rp:sSn:",
	        long_options, &opt_idx)) != -1) {
		switch (ret_code) {
		case 0:
			break;
		case 'v':
			if (options->quiet == 0)
				options->verbose = 1;
			break;
		case 'q':
			if (options->verbose == 0)
				options->quiet = 1;
			break;
		case 'c':
			if (*mode == NETMAN_MODE_NOP){
				*mode = NETMAN_MODE_CONNECT;
				options->network = strdup(optarg);
			}
			break;
		/*case 'b':
			if (*mode == NETMAN_MODE_NOP) {
				config->cmd = netman_blacklist;
				set_str(&config->opts->name, optarg);
				DEBUG("Blacklist %s\n", config->opts->name);
			}
			break; */
		case 'r':
			if (*mode == NETMAN_MODE_NOP) {
				*mode = NETMAN_MODE_RECONNECT;
			}
			break;
		case 'p':
			psk_str = quote_str(optarg);
			DEBUG("%s\n", psk_str);
			options->wpa_options = mk_keyvalue("psk",
			         psk_str, options->wpa_options, VALUE_STR);
			free(psk_str);
			break;
		case 's':
			if (*mode == NETMAN_MODE_NOP) {
				*mode = NETMAN_MODE_SCAN;
			}
			break;
		case 'n':
			ssid_str = quote_str(optarg);
			options->wpa_options = mk_keyvalue("ssid",
			         ssid_str, options->wpa_options, VALUE_STR);
			free(ssid_str);
			break;
		default:
			eprintf("Sumting wong\n");
		}
	}
	return 0;
}

typedef void (*ACTION)(STATE *state);

static ACTION actions[NETMAN_NUM_MODES] = {
	[NETMAN_MODE_NOP]     = NULL, /* TODO: print usage or something */
	[NETMAN_MODE_CONNECT] = netman_connect,
};

int
main(int argc, char *argv[])
{
	STATE state;
	state.state = NETMAN_STATE_IDLE;
	state.mode = NETMAN_MODE_NOP;
	state.config = NULL;
	state.options = default_opts;

	arg_parse(argc, argv, &state.mode, &state.options);
	state.config = netman_get_config();

	actions[state.mode](&state);
}

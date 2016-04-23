#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#include "network.h"
#include "util.h"

struct options {
	char *cfg_path;
	char *name;
	char *interface;
	keyvalue_t *kv_pair;
	int no_save;
	int verbose;
	int quiet;
};

typedef struct options options_t;
typedef void (*command_t)(options_t *);

struct netman_config {
	command_t cmd;
	options_t *opts;
};

typedef struct netman_config config_t;

static options_t default_opts = { 
	.cfg_path  = NULL,
	.name      = NULL,
	.interface = NULL,
	.kv_pair   = NULL,
	.no_save   = 0,
	.verbose   = 0,
	.quiet     = 0
};

static config_t default_conf = {
	.cmd  = NULL,
	.opts = NULL
};

static void 
netman_connect(options_t *options)
{
	DEBUG("PSK: %s\n", options->name);
}

static void 
netman_reconnect(options_t *options)
{
	DEBUG("SSID: %s\n", options->name);
}

static void 
netman_blacklist(options_t *options)
{
	DEBUG("Name: %s\n", options->name);
}

static void
netman_scan(options_t *options)
{
	DEBUG("Scan: %s\n", options->name); 
}

static void
netman_scan_networks(options_t *options)
{
	DEBUG("Scan networks: %s\n", options->name); 
}

static int
arg_parse(int argc, char *argv[], config_t *config)
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
			case 'p': /* Keyvalue */
				set_str_quote(&psk_str, optarg);
				config->opts->kv_pair = mk_keyvalue("psk", psk_str, config->opts->kv_pair);
				free(psk_str);
				DEBUG("Set the PSK\n");
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
				config->opts->kv_pair = mk_keyvalue("ssid", ssid_str, config->opts->kv_pair);
				free(ssid_str);
				DEBUG("Set the SSID\n");
				break;
			default:
				eprintf("Sumting wong\n");
		}
	}
	return 0;
}

void
print_keyvalues(config_t *config)
{
	keyvalue_t *tmp = config->opts->kv_pair;
	while (tmp) {
		DEBUG("%s - %s\n", tmp->key, tmp->value);
		tmp = tmp->next;
	}
}

int
main(int argc, char *argv[])
{
	config_t config = default_conf;
	options_t opts = default_opts;
	config.opts = &opts;
	arg_parse(argc, argv, &config);
	print_keyvalues(&config);
	config.cmd(config.opts);

	return 0;
}

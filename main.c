#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define eprintf(...) (fprintf(stderr, __VA_ARGS__))

#define DEBUG_MODE 1

#if DEBUG_MODE == 1
#define DEBUG(...) (printf(__VA_ARGS__))
#else
#define DEBUG(...)
#endif

struct options {
	char ssid[32];	
	char bssid[32];
	char *psk;
	char *cfg_path;
	char *name;
	char *interface;
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
	.ssid     = "",
	.bssid    = "",
	.psk      = NULL,
	.cfg_path = NULL,
	.name     = NULL,
	.no_save  = 0,
	.verbose  = 0,
	.quiet    = 0
};

static config_t default_conf = {
	.cmd  = NULL,
	.opts = NULL
};

static void 
netman_connect(options_t *options)
{
	DEBUG("PSK: %s\n", options->psk);
}

static void 
netman_reconnect(options_t *options)
{
	DEBUG("SSID: %s\n", options->ssid);
}

static void 
netman_blacklist(options_t *options)
{
	DEBUG("Name: %s\n", options->name);
}

static void
netman_scan(options_t *options)
{
	DEBUG("Scan: %s\n", options->interface);
}

static int
arg_parse(int argc, char *argv[], config_t *config)
{
	int ret_code;
	int opt_idx = 0;

	struct option long_options[] = {
		{ "no-save",    no_argument,       &config->opts->no_save,  0  },
		{ "verbose",    no_argument,       0,                      'v' },
		{ "quiet",      no_argument,       0,                      'q' },
		{ "connect",    required_argument, 0,                      'c' },
		{ "blacklist",  required_argument, 0,                      'b' },
		{ "reconnect",  no_argument,       0,                      'r' },
		{ "passphrase", required_argument, 0,                      'p' },
		{ "scan",       required_argument, 0,                      's' },
	};

	opterr = 0;

	while ((ret_code = getopt_long(argc, argv, "vqc:b:rp:s:", 
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
			case 'p':
				set_str(&config->opts->psk, optarg);
				DEBUG("PSK %s\n", config->opts->psk);
				break;
			case 's':
				if (config->cmd == NULL) {
					config->cmd = netman_scan;
					set_str(&config->opts->interface, optarg);
					DEBUG("Scan with interface: %s\n", config->opts->interface);
				}
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
	config_t config = default_conf;
	options_t opts = default_opts;
	config.opts = &opts;
	arg_parse(argc, argv, &config);
	return 0;
}

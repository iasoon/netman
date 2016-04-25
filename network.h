#ifndef NETWORK_H_
#define NETWORK_H_

#include "util.h"
#include "wpa_ctrl.h"

struct netman_options {
	char *cfg_path;
	char *name;
	char *interface;
	keyvalue_t *kv_pair;
	int no_save;
	int verbose;
	int quiet;
};

typedef struct netman_options options_t;
typedef void (*netman_command_t)(options_t *);

struct netman_action {
	netman_command_t cmd;
	options_t *opts;
};

typedef struct netman_action action_t;

void netman_connect(options_t *options);
void netman_reconnect(options_t *options);
void netman_blacklist(options_t *options);
void netman_scan(options_t *options);
void netman_scan_networks(options_t *options);

#endif

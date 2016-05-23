#ifndef NETMAN_STATE_H
#define NETMAN_STATE_H

#include "util.h"

enum {
	NETMAN_MODE_NOP = 0,
	NETMAN_MODE_CONNECT,
	NETMAN_MODE_SCAN,
	NETMAN_MODE_RECONNECT,
	NETMAN_NUM_MODES
};

enum {
	NETMAN_STATE_IDLE = 0,
	NETMAN_STATE_CONNECTING,
	NETMAN_STATE_SCANNING,
	NETMAN_NUM_STATES
};

struct netman_options {
	char *cfg_path;
	char *network;
	char *interface;
	keyvalue_t *wpa_options;
	int no_save;
	int verbose;
	int quiet;
    keyvalue_t *config;
};

typedef struct netman_options options_t;

struct netman_state {
    int state;
    int mode;
    options_t options;
    keyvalue_t *config;
};

typedef struct netman_state state_t;

#endif

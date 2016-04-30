#ifndef NETMAN_STATE_H
#define NETMAN_STATE_H

#include "util.h"

#define NETMAN_MODE_NOP         0
#define NETMAN_MODE_CONNECT     1
#define NETMAN_MODE_SCAN        2
#define NETMAN_MODE_RECONNECT   3
#define NETMAN_NUM_MODES        4

#define NETMAN_STATE_IDLE       0
#define NETMAN_STATE_CONNECTING 1
#define NETMAN_STATE_SCANNING   2
#define NETMAN_NUM_STATES       3

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

#ifndef NETMAN_CONFIG_H
#define NETMAN_CONFIG_H

#include <stdio.h>

#include "util.h"

#define NETMAN_CONFIG_LOCATION "netman.conf"

keyvalue_t *read_keyvalue(FILE *handle);
keyvalue_t *netman_get_config();

#endif

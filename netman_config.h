#ifndef NETMAN_CONFIG_H
#define NETMAN_CONFIG_H

#include <stdio.h>

#include "util.h"

#define NETMAN_CONFIG_LOCATION "netman.conf"

__BEGIN_DECLS

KEYVALUE	*read_keyvalue(FILE *handle);
KEYVALUE	*netman_get_config();

__END_DECLS

#endif

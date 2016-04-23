#ifndef NETWORK_H
#define NETWORK_H

struct network {
	char *alias;
	char *id;
	struct keyvalue *options;
};

typedef struct network network_t;

#endif

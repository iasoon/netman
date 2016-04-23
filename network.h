#ifndef NETWORK_H
#define NETWORK_H

struct keyvalue {
	char *key;
	char *value;
    struct keyvalue *next;
};

struct network {
	char *alias;
    char *id;
	struct keyvalue *options;
};

typedef struct network network_t;
typedef struct keyvalue keyvalue_t;

#endif

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eprintf(...) (fprintf(stderr, __VA_ARGS__))

#define DEBUG_MODE 1

#if DEBUG_MODE == 1
#define DEBUG(...) (printf(__VA_ARGS__))
#else
#define DEBUG(...)
#endif

#define VALUE_STR 0
#define VALUE_CHILD 1

struct keyvalue {
	char *key;
	union _vc {
		char *str;
		struct keyvalue *child;
	} value;
	struct keyvalue *next;
	uint8_t type;	
};

typedef struct keyvalue keyvalue_t;

size_t strlcpy(char *dest, const char *src, size_t size);
size_t strlcat(char *dest, const char *src, size_t size);
void set_str(char **dest, const char *src);
void set_str_quote(char **dest, const char *str);
void set_stripped(char **dest, char *begin, char *end);
keyvalue_t *mk_keyvalue(char *key, void *value, keyvalue_t *next, uint8_t type);
union _vc get_element(char *longkey, keyvalue_t *root);

#endif

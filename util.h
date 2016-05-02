#ifndef UTIL_H
#define UTIL_H

#include <limits.h>
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

struct hash_link {
	char *key;
	void *ptr;
	struct hash_link *next;
};

struct hashtable {
	int size;
	struct hash_link **table;
};

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
typedef struct hashtable hashtable_t;

char **separate_str(char **str, const char *delim, int *sz);
size_t strlcpy(char *dest, const char *src, size_t size);
size_t strlcat(char *dest, const char *src, size_t size);
void set_str(char **dest, const char *src);
void set_str_quote(char **dest, const char *str);
void set_stripped(char **dest, char *begin, char *end);
hashtable_t *mk_hashtable(int size);
void free_hashtable(hashtable_t *h);
void hash_add(hashtable_t *h, const char *key, void *ptr);
void *hash_get_ptr(hashtable_t *h, const char *key);
keyvalue_t *mk_keyvalue(char *key, void *value, keyvalue_t *next, uint8_t type);
union _vc get_element(const char *longkey, keyvalue_t *root);
void free_kv(keyvalue_t *root);

#endif

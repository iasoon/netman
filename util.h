#ifndef UTIL_H
#define UTIL_H

#define eprintf(...) (fprintf(stderr, __VA_ARGS__))

#define DEBUG_MODE 1

#if DEBUG_MODE == 1
#define DEBUG(...) (printf(__VA_ARGS__))
#else
#define DEBUG(...)
#endif


struct keyvalue {
	char *key;
	char *value;
	struct keyvalue *next;
};

typedef struct keyvalue keyvalue_t;

void set_str(char **dest, const char *src);
void set_str_quote(char **dest, const char *str);
keyvalue_t *mk_keyvalue(char *key, char *value, keyvalue_t *next);


#endif

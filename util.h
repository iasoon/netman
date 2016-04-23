#ifndef UTIL_H
#define UTIL_H

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

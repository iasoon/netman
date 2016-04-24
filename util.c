#include "util.h"

void
set_str(char **dest, const char *src)
{
	int src_len;
	int dest_len;

	if (src == NULL) return;
	src_len = strlen(src);

	if (*dest == NULL)  {
		if ((*dest = malloc(src_len)) == NULL) exit(1);	
		dest_len = src_len;
	} else {
		dest_len = strlen(*dest);
	}

	if (src_len <= dest_len) {
		if (strcpy(*dest, src) == NULL) { exit(1); }
	}
}

/* FIXME: Placeholder that just works, REFACTOR */
void
set_str_quote(char **dest, const char *src)
{
	int src_len;
	int dest_len;
	
	if (src == NULL) return;
	src_len = strlen(src);

	
	if (*dest == NULL) {
		if ((*dest = malloc(src_len+2)) == NULL) exit(1);
		dest_len = src_len+3;
	} else {
		dest_len = strlen(*dest);
	}	

	/* Dragons be here */
	if (src_len+2 <= dest_len) {
		(*dest)++;
		if (strcpy(*dest, src) == NULL) { exit(1); }
		(*dest)--;
		**dest = '\"';
		*dest = (*dest) + src_len+1;
		**dest = '\"';
		(*dest)++;
		**dest = '\0';
		*dest = (*dest) - src_len-2;
	}
}

/* FIXME: Placeholder that just works, REFACTOR */
void set_stripped(char **dest, char *begin, char *end)
{
	int size = 0;
	int i = 0;

	while (*begin == ' ' || *begin == '\t')
		begin++;

	while (*end == ' ' || *end == '\t' || *end == '\n')
		end--;
	
	size = end - begin;

	if (*dest == NULL) {
		if ((*dest = malloc(size)) == NULL) exit(1);
	} else {
		size = strlen(*dest);
	}

	if (end-begin <= size) {
		while (begin != end) {
			**dest = *begin;
			(*dest)++;
			begin++;
		}
		**dest = *end;
		for (i = 0; i < size; i++) {
			(*dest)--;
		}
	}
}

keyvalue_t *
mk_keyvalue(char *key, void *ptr, keyvalue_t *next, uint8_t type)
{
	keyvalue_t *kv = malloc(sizeof(keyvalue_t));
	set_str(&kv->key, key);
	if (type == VALUE_STR) {
		set_str(&kv->value.str, ptr);
	} else if (type == VALUE_CHILD) {
		kv->value.child = ptr;
	} else {
		free(kv->key);
		free(kv);
		return NULL;
	}
	kv->next = next;
	kv->type = type;
	return kv;
}


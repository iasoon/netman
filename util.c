#include "util.h"

void
set_str(char **dest, const char *src)
{
	int src_len;
	int dest_len;

	/* Do not copy if the src string is NULL */
	if (src == NULL) return;
	src_len = strlen(src);

	/* If the dest string is null, allocate src_len + 1 bytes for it
	 * in case of a missing '\0' terminator in the src string
	 */
	if (*dest == NULL)  {
		if ((*dest = malloc(src_len+1)) == NULL) exit(1);	
		dest_len = src_len;
	} else {
		dest_len = strlen(*dest);
	}

	/* Only copy if there is enough room */
	if (src_len <= dest_len) {
		if (strcpy(*dest, src) == NULL) { exit(1); }
	}
}

void
set_str_quote(char **dest, const char *src)
{
	int src_len;
	int dest_len;
	char *temp = NULL;
	
	/* Do not copy if the src string is NULL */
	if (src == NULL) return;
	src_len = strlen(src);

	/* src + " " + null terminator */
	dest_len = src_len + 3;

	/* No preallocated buffers allowed, function gets too dirty */
	if (*dest != NULL) {
		free(*dest);
	}

	if ((temp = malloc(dest_len)) == NULL)
		exit(1);
	strcpy(temp, "\"");
	strcat(temp, src);
	strcat(temp, "\"");

	*dest = temp;
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


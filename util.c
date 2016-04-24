#include "util.h"

void
set_str(char **dest, const char *src)
{
	int src_len;

	/* Do not copy if the src string is NULL */
	if (src == NULL) return;
	src_len = strlen(src);

	/* Disallow preallocated buffers */
	if (*dest != NULL) 
		free(*dest);
	
	/* In case of needed null termination */
	if ((*dest = malloc(src_len+1)) == NULL) exit(1);	

	if (strcpy(*dest, src) == NULL) {
		DEBUG("strcpy - set_str NULL\n");
		/* You get NULL */
		free(*dest);
		*dest = NULL;
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

	if (strcpy(temp, "\"") == NULL) {
		DEBUG("strcpy - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		temp = NULL;
		return;
	}

	if (strcat(temp, src) == NULL) {
		DEBUG("strcat - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		temp = NULL;
		return;	
	}

	if (strcat(temp, "\"") == NULL) {
		DEBUG("strcat - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		temp = NULL; 
		return;
	}

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


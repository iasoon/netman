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

	*dest = NULL;

	if ((temp = malloc(dest_len)) == NULL)
		exit(1);

	if (strcpy(temp, "\"") == NULL) {
		DEBUG("strcpy - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		return;
	}

	if (strcat(temp, src) == NULL) {
		DEBUG("strcat - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		return;	
	}

	if (strcat(temp, "\"") == NULL) {
		DEBUG("strcat - set_str_quote NULL\n");
		/* You get NULL */
		free(temp);
		return;
	}

	*dest = temp;
}

hashtable_t *
mk_hashtable(int size)
{
	hashtable_t *h = NULL;

	if (size < 1) {
		eprintf("Size cannot be lesser than 1\n");
		return NULL;
	}

	if ((h = malloc(sizeof(hashtable_t))) == NULL) {
		eprintf("Malloc failed\n");
		return NULL;
	}

	if ((h->table = calloc(size, sizeof(struct hash_link*))) == NULL) {
		eprintf("Malloc failed\n");
		return NULL;
	}

	h->size = size;

	return h;
}

void
free_hashtable(hashtable_t *h)
{
	int i;
	struct hash_link *tmp = NULL;
	struct hash_link *next = NULL;
	for (i = 0; i < h->size; i++) {
		tmp = h->table[i];
		while(tmp != NULL) {
			next = tmp->next;
			free(tmp->key);
			free(tmp);
			tmp = next;
		}
	}

	free(h->table);
}

static struct hash_link *
mk_hash_link(const char *key, void *ptr)
{
	struct hash_link *new = NULL;

	if (key == NULL) {
		eprintf("Key cannot be NULL\n");
		return NULL;
	}

	if (ptr == NULL) {
		eprintf("Function cannot be NULL\n");
		return NULL;
	}

	if ((new = malloc(sizeof(struct hash_link))) == NULL) {
		eprintf("Malloc failed\n");
		return NULL;
	}
	new->key = NULL;
	new->ptr = NULL;

	set_str(&new->key, key);

	if (new->key == NULL) {
		eprintf("set_str failed\n");
		return NULL;
	}

	new->ptr = ptr;
	new->next = NULL;
	return new;
}

static int
hash(hashtable_t *h, const char *key)
{
	unsigned long val = 0;
	unsigned i = 0;

	while (val < ULONG_MAX && i < strlen(key)) {
		val <<= 8;
		val += key[i];
		i++;	
	}

	return val % h->size;
}

void
hash_add(hashtable_t *h, const char *key, void *ptr)
{
	int bin = 0;
	struct hash_link *new = NULL;
	struct hash_link *next = NULL;
	struct hash_link *last = NULL;
	
	bin = hash(h, key);

	next = h->table[bin];

	while (next != NULL && next->key != NULL &&
			strcmp(key, next->key) > 0) {
		last = next;
		next = next->next;
	}

	if (next != NULL && next->key != NULL &&
			strcmp(key, next->key) == 0) {
		next->ptr = ptr;
	} else {
		new = mk_hash_link(key, ptr);

		if (next == h->table[bin]) {
			new->next = next;
			h->table[bin] = new;
		} else if (next == NULL) {
			last->next = new;
		} else {
			new->next = next;
			last->next = new;
		}
	}
}

void *
hash_get_ptr(hashtable_t *h, const char *key)
{
	int bin = 0;
	struct hash_link *pair;

	bin = hash(h, key);

	pair = h->table[bin];

	while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0)
		pair = pair->next;

	if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0)
		return NULL;
	else
		return pair->ptr;
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

union _vc
get_element(const char *key, keyvalue_t *root)
{
	static union _vc null = {
		.str = NULL
	};

	while (root) {
		if (strcmp(root->key, key) == 0)
			return root->value;

		root = root->next;
	}	

	return null;
}

void
free_kv(keyvalue_t *root)
{
	if (root == NULL) return;
	if (root->key) free(root->key);
	if (root->type == VALUE_STR) free(root->value.str);
	else free_kv(root->value.child);
	if (root->next) free_kv(root->next);
	free(root);
}

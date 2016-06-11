#include "util.h"

char *
strdup(const char *src)
{
	size_t length;
	char *dest;

	length = strlen(src) + 1;
	if ((dest = malloc(length)) == NULL)
		return NULL;

	return memcpy(dest, src, length);
}

char *
quote_str(const char *src)
{
	size_t length;
	char *dest;

	length = strlen(src) + 3;
	if ((dest = malloc(length)) == NULL)
		return NULL;

	*dest++ = '"';
	if (memcpy(dest, src, length) == NULL) {
		free(dest);
		return NULL;
	}
	dest--;
	
	return	strcat(dest, "\"");
}

HASHTABLE *
mk_hashtable(uint32_t size)
{
	HASHTABLE *h = NULL;

	if (size < 1) {
		eprintf("Size cannot be lesser than 1\n");
		return NULL;
	}

	if ((h = malloc(sizeof(HASHTABLE))) == NULL) {
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
free_hashtable(HASHTABLE *h)
{
	uint32_t i;
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

	new->key = strdup(key);

	if (new->key == NULL) {
		eprintf("strdup failed\n");
		return NULL;
	}

	new->ptr = ptr;
	new->next = NULL;
	return new;
}

static int
hash(HASHTABLE *h, const char *key)
{
	uint32_t val = 0;
	uint32_t i = 0;

	while (i < strlen(key)) {
		val <<= 8;
		val += key[i];
		i++;	
	}

	return val % h->size;
}

void
hash_add(HASHTABLE *h, const char *key, void *ptr)
{
	uint32_t bin = 0;
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
hash_get_ptr(HASHTABLE *h, const char *key)
{
	uint32_t bin = 0;
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

KEYVALUE *
mk_keyvalue(char *key, void *ptr, KEYVALUE *next, uint8_t type)
{
	KEYVALUE *kv = malloc(sizeof(KEYVALUE));
	kv->key = strdup(key);
	if (type == VALUE_STR) {
		kv->value.str = strdup(ptr);
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
get_element(const char *key, KEYVALUE *root)
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
free_kv(KEYVALUE *root)
{
	if (root == NULL) return;
	if (root->key) free(root->key);
	if (root->type == VALUE_STR) free(root->value.str);
	else free_kv(root->value.child);
	if (root->next) free_kv(root->next);
	free(root);
}

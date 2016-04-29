#include <string.h>
#include <stdlib.h>

#include "netman_config.h"
#include "util.h"

#define BUFFER_SIZE 512
#define STACK_SIZE 32

static inline int
read(char *dest, FILE *file)
{
	return (*dest = fgetc(file)) != EOF;
}

static inline size_t
read_until(char *dest, char *delims, FILE *file)
{
	char* d, *c = dest;
	for (;;) {
		for (d = delims; *d != '\0'; d++)
			if (*c == *d) return c - dest;
		if (!read(c+1, file)) return c - dest;
		c++;
	}
}

static inline void
skip(char *c, char *delims, FILE *file)
{
	char *d;
	for (;;) {
		for (d = delims; *d != '\0' && *c != *d; d++);
		if (*d == '\0') return;
		if (!read(c, file)) return;
	}
}

static char *
extract_token(char *buffer, int num)
{
	/* strip trailing whitespace */
	char *token, *c = buffer + num - 1;
	while (*c == ' ' || *c == '\t' || *c == '\n') c--;
	num = c - buffer + 1;
	if ((token = malloc(num+1)) == NULL) {
		eprintf("Malloc failed\n");
		exit(1);
	}

	memcpy(token, buffer, num);
	token[num] = '\0';
	return token;
}


keyvalue_t *
read_keyvalue(FILE *file)
{
	char buffer[BUFFER_SIZE];
	size_t num;
	keyvalue_t *kv, *root = 0;

	keyvalue_t *predecessors[STACK_SIZE] = {0};
	int depth = 0;

	while (read(buffer, file)) {
		skip(buffer, " \t\n", file);
		if (*buffer == '}') {
			/* ascend a level */
			predecessors[depth] = 0;
			depth--;
		} else {
			/* attach a new node to the tree */
			if ((kv = malloc(sizeof(keyvalue_t))) == NULL) {
				eprintf("Malloc failed\n");
				exit(1);
			}

			if (predecessors[depth])
				predecessors[depth]->next = kv;
			else if (depth > 0)
				predecessors[depth-1]->value.child = kv;
			else
				root = kv;
			predecessors[depth] = kv;

			kv->next = 0;

			/* read key */
			num = read_until(buffer, "=\n", file);
			kv->key = extract_token(buffer, num);

			/* read value */
			read(buffer, file);
			skip(buffer, " \t", file);

			if (*buffer == '{') {
				/* value is nested k/v; descend a level */
				kv->type = VALUE_CHILD;
				depth++;
			}  else {
				/* value is a string */
				kv->type = VALUE_STR;
				num = read_until(buffer, "\n", file);
				kv->value.str = extract_token(buffer, num);
			}
		}
	}
	return root;
}


void
write_keyvalue(FILE *file, keyvalue_t *root){
	keyvalue_t *pos[STACK_SIZE] = {0};
	int i, depth = 0;
	pos[0] = root;
	while (pos[0]) {
		if (!pos[depth]) {
			pos[depth] = 0;
			depth--;
			pos[depth] = pos[depth]->next;
			for (i = 0; i < depth; i++)
				fprintf(file, "  ");
			fprintf(file, "}\n");
		} else {
			for (i = 0; i < depth; i++)
				fprintf(file, "  ");
			if (pos[depth]->type == VALUE_CHILD) {
				fprintf(file, "%s = {\n", pos[depth]->key);
				pos[depth+1] = pos[depth]->value.child;
				depth++;
			} else {
				fprintf(file, "%s = %s\n", pos[depth]->key, pos[depth]->value.str);
				pos[depth] = pos[depth]->next;
			}
		}
	}
}

keyvalue_t *
netman_get_config()
{
	FILE *conf_file = fopen(NETMAN_CONFIG_LOCATION, "r");
	keyvalue_t *conf = read_keyvalue(conf_file);
	fclose(conf_file);
	return conf;
}

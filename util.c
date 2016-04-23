#include <stdio.h>

#include <stdlib.h>
#include <string.h>

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

/* TODO: This can be done better */
void
set_str_quote(char **dest, const char *src)
{
	int src_len;
	int dest_len;
	
	if (src == NULL) return;
	src_len = strlen(src);

	
	if (*dest == NULL) {
		if ((*dest = malloc(src_len+2)) == NULL) exit(1);
		dest_len = src_len+2;
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
		*dest = (*dest) - src_len-1;
	}
}

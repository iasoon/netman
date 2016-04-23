#include <stdlib.h>
#include <string.h>

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

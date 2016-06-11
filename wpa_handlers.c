#include "wpa_handlers.h"

#define BUFFER_SIZE 4096

static void
get_nid(char nid[4], char *params)
{
	while (*params != ':') {
		*nid++ = *params++;
	}
}


void 
prompt_password(STATE *state, WPA_INTERFACE *iface, char *params)
{
	struct termios tp, save;
	char buf[BUFFER_SIZE];
	char pwd[512];
	char nid[4] = {0};

	get_nid(nid, params);

	if (tcgetattr(STDIN_FILENO, &tp) == -1) {
		perror("tcgetattr");
		return;
	}	

	save = tp;
	tp.c_lflag &= ~ECHO;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
		perror("tcsetattr");
		return;
	}

	printf("Password: ");
	fflush(stdout);

	if (fgets(pwd, 512, stdin) == NULL) {
		eprintf("EOF Error\n");
		return;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1) {
		perror("tcsetattr");
	}

	pwd[strlen(pwd)-1] = '\0';
	wpa_request(iface, buf, "CTRL-RSP-PASSWORD-%s:%s", nid, pwd); 
}

void
netman_exit(STATE *state, WPA_INTERFACE *iface, char *params)
{
	/* TODO: properly clean up or something */
	printf("%s\n", params);
	exit(0);
}

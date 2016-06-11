
#ifndef WPA_HANDLERS_H_
#define WPA_HANDLERS_H_

#include <unistd.h>
#include <termios.h>

#include "util.h"
#include "wpa_ctrl.h"

void prompt_password(STATE *state, WPA_INTERFACE *iface, char *params);
void netman_exit(STATE *state, WPA_INTERFACE *iface, char *params);

#endif

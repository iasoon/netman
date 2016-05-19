
#ifndef WPA_HANDLERS_H_
#define WPA_HANDLERS_H_

#include <unistd.h>
#include <termios.h>

#include "util.h"
#include "wpa_ctrl.h"

void prompt_password(state_t *state, wpa_interface_t *iface, char *params);
void netman_exit(state_t *state, wpa_interface_t *iface, char *params);

#endif

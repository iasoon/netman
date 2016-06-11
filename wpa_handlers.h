
#ifndef WPA_HANDLERS_H_
#define WPA_HANDLERS_H_

#include <termios.h>
#include <unistd.h>

#include "util.h"
#include "wpa_ctrl.h"

__BEGIN_DECLS

void	prompt_password(STATE *state, WPA_INTERFACE *iface, char *params);
void	netman_exit(STATE *state, WPA_INTERFACE *iface, char *params);

__END_DECLS

#endif

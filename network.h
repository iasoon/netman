#ifndef NETWORK_H_
#define NETWORK_H_

#include "if_ctrl.h"
#include "netman_state.h"
#include "util.h"
#include "wpa_ctrl.h"

void netman_connect(state_t *state);
void netman_reconnect(options_t *options);
void netman_blacklist(options_t *options);
void netman_scan(options_t *options);
void netman_scan_networks(options_t *options);

#endif

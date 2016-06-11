#ifndef NETWORK_H_
#define NETWORK_H_

#include "if_ctrl.h"
#include "netman_state.h"
#include "util.h"
#include "wpa_ctrl.h"

void netman_connect(STATE *state);
void netman_reconnect(OPTIONS *options);
void netman_blacklist(OPTIONS *options);
void netman_scan(OPTIONS *options);
void netman_scan_networks(OPTIONS *options);

#endif

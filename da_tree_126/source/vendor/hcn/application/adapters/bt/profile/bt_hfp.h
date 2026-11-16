#ifndef BT_HFP_H
#define BT_HFP_H

#include "bt_app.h"

#include <stdbool.h>

int bt_hfp_init(void);
void bt_hfp_deinit(void);
void bt_hfp_connect_phone(const char *address);
void bt_hfp_disconnect_phone(const char *address);
void bt_hfp_dial(const char *number);
void bt_hfp_redial(void);
void bt_hfp_accept_call(void);
void bt_hfp_hangup_call(void);
void bt_hfp_hold_call(BtCallAction action);
void bt_hfp_switch_audio(bool phone_audio);
void bt_hfp_play_ringtone(void);
void bt_hfp_stop_ringtone(void);
void bt_hfp_send_dtmf(const char *code);

#endif

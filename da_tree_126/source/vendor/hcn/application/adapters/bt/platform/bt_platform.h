#ifndef BT_PLATFORM_H
#define BT_PLATFORM_H

#include "bt_app.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int bt_platform_init(void);
void bt_platform_deinit(void);
void bt_platform_poll(void);

int bt_platform_gap_set_power(bool enabled);
int bt_platform_gap_start_scan(void);
int bt_platform_gap_stop_scan(void);
int bt_platform_gap_pair(const char *address);
int bt_platform_gap_remove(const char *address);

int bt_platform_hfp_connect(const char *address);
int bt_platform_hfp_disconnect(const char *address);
int bt_platform_hfp_dial(const char *number);
int bt_platform_hfp_redial(void);
int bt_platform_hfp_accept(void);
int bt_platform_hfp_hangup(void);
int bt_platform_hfp_hold(BtCallAction action);
int bt_platform_hfp_switch_audio(bool phone_audio);
int bt_platform_hfp_play_ringtone(void);
int bt_platform_hfp_stop_ringtone(void);
int bt_platform_hfp_send_dtmf(const char *code);

int bt_platform_pbap_download_phonebook(void);
int bt_platform_pbap_download_call_records(void);
int bt_platform_pbap_abort(void);

int bt_platform_avrcp_play_pause(bool currently_playing);
int bt_platform_avrcp_pause(void);
int bt_platform_avrcp_prev(void);
int bt_platform_avrcp_next(void);

int bt_platform_a2dp_init(void);
int bt_platform_a2dp_set_audio_enabled(bool enabled);

int bt_platform_set_local_name(const char *name);
int bt_platform_set_pairing_pin(const char *pin);

#ifdef __cplusplus
}
#endif

#endif

#include "bt_hfp.h"

#include "bt_core.h"
#include "bt_platform.h"

int bt_hfp_init(void)
{
    return 0;
}

void bt_hfp_deinit(void)
{
}

void bt_hfp_connect_phone(const char *address)
{
    if (bt_platform_hfp_connect(address) != 0) {
        bt_core_notify_status("HFP connect request failed");
    }
}

void bt_hfp_disconnect_phone(const char *address)
{
    if (bt_platform_hfp_disconnect(address) != 0) {
        bt_core_notify_status("HFP disconnect request failed");
    }
}

void bt_hfp_dial(const char *number)
{
    if (bt_platform_hfp_dial(number) != 0) {
        bt_core_notify_status("HFP dial request failed");
    }
}

void bt_hfp_redial(void)
{
    if (bt_platform_hfp_redial() != 0) {
        bt_core_notify_status("HFP redial request failed");
    }
}

void bt_hfp_accept_call(void)
{
    if (bt_platform_hfp_accept() != 0) {
        bt_core_notify_status("HFP accept call request failed");
    }
}

void bt_hfp_hangup_call(void)
{
    if (bt_platform_hfp_hangup() != 0) {
        bt_core_notify_status("HFP hangup request failed");
    }
}

void bt_hfp_hold_call(BtCallAction action)
{
    if (bt_platform_hfp_hold(action) != 0) {
        bt_core_notify_status("HFP hold call request failed");
    }
}

void bt_hfp_switch_audio(bool phone_audio)
{
    if (bt_platform_hfp_switch_audio(phone_audio) != 0) {
        bt_core_notify_status("HFP switch audio request failed");
    }
}

void bt_hfp_play_ringtone(void)
{
    if (bt_platform_hfp_play_ringtone() != 0) {
        bt_core_notify_status("HFP play ringtone request failed");
    }
}

void bt_hfp_stop_ringtone(void)
{
    if (bt_platform_hfp_stop_ringtone() != 0) {
        bt_core_notify_status("HFP stop ringtone request failed");
    }
}

void bt_hfp_send_dtmf(const char *code)
{
    if (bt_platform_hfp_send_dtmf(code) != 0) {
        bt_core_notify_status("HFP DTMF request failed");
    }
}

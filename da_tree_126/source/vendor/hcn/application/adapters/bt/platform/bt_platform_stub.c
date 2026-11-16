#include "bt_platform.h"

#include "bt_event_queue.h"
#include "bt_types.h"

#include <string.h>

static bool s_power_on;
static bool s_music_playing;
static char s_connected_address[BT_DEVICE_TEXT_LEN];
static char s_bonded_address[BT_DEVICE_TEXT_LEN];
static char s_bonded_name[BT_DEVICE_TEXT_LEN];
static char s_local_name[BT_LOCAL_NAME_TEXT_LEN];
static char s_pairing_pin[20];

#define BT_STUB_DEVICE_ADDRESS "00:11:22:33:44:55"
#define BT_STUB_DEVICE_NAME "Demo Phone"

static bool copy_local_name(char *dst, size_t dst_size, const char *name)
{
    if (name == NULL || name[0] == '\0') {
        if (dst != NULL && dst_size > 0U) {
            dst[0] = '\0';
        }
        return false;
    }

    return bt_copy_valid_utf8_display_bytes(dst, dst_size, name, strlen(name));
}

static void post_status(const char *status)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_STATUS;
    bt_copy_string(event.text, sizeof(event.text), status);
    (void)bt_event_post(&event);
}

static void post_pbap_state(BtPbapState state, BtPbapDownloadType type, const char *status)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PBAP_STATE;
    event.state = state;
    event.pbap_download_type = type;
    event.pbap_current_index = 0;
    event.pbap_total_count = 0;
    bt_copy_string(event.text, sizeof(event.text), status);
    (void)bt_event_post(&event);
}

int bt_platform_init(void)
{
    BtEvent event;

    s_power_on = false;
    s_music_playing = false;
    s_connected_address[0] = '\0';
    s_bonded_address[0] = '\0';
    s_bonded_name[0] = '\0';
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_LOCAL_INFO;
    bt_copy_string(s_local_name, sizeof(s_local_name), "LVGL Stub BT");
    bt_copy_string(s_pairing_pin, sizeof(s_pairing_pin), "0000");
    bt_copy_string(event.local_name, sizeof(event.local_name), s_local_name);
    bt_copy_string(event.pairing_pin, sizeof(event.pairing_pin), s_pairing_pin);
    (void)bt_event_post(&event);
    post_status("Bluetooth stub backend enabled");
    return 0;
}

void bt_platform_deinit(void)
{
    s_power_on = false;
    s_music_playing = false;
    s_connected_address[0] = '\0';
}

void bt_platform_poll(void)
{
}

int bt_platform_gap_set_power(bool enabled)
{
    BtEvent event;

    s_power_on = enabled;
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_POWER_STATE;
    event.value = enabled;
    (void)bt_event_post(&event);
    if (enabled && s_bonded_address[0] != '\0') {
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_DEVICE_BONDED;
        bt_copy_string(event.address, sizeof(event.address), s_bonded_address);
        bt_copy_string(event.name, sizeof(event.name), s_bonded_name);
        (void)bt_event_post(&event);
    }
    return 0;
}

int bt_platform_gap_start_scan(void)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_SCAN_STATE;
    event.value = true;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_DEVICE_FOUND;
    bt_copy_string(event.address, sizeof(event.address), BT_STUB_DEVICE_ADDRESS);
    bt_copy_string(event.name, sizeof(event.name), BT_STUB_DEVICE_NAME);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_SCAN_STATE;
    event.value = false;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_gap_stop_scan(void)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_SCAN_STATE;
    event.value = false;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_gap_pair(const char *address)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_DEVICE_PAIRED;
    bt_copy_string(event.address, sizeof(event.address), address != NULL ? address : BT_STUB_DEVICE_ADDRESS);
    bt_copy_string(event.name, sizeof(event.name), BT_STUB_DEVICE_NAME);
    bt_copy_string(s_bonded_address, sizeof(s_bonded_address), event.address);
    bt_copy_string(s_bonded_name, sizeof(s_bonded_name), event.name);
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_gap_remove(const char *address)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_DEVICE_REMOVED;
    bt_copy_string(event.address, sizeof(event.address), address);
    if (event.address[0] == '\0' || strcmp(event.address, s_bonded_address) == 0) {
        s_bonded_address[0] = '\0';
        s_bonded_name[0] = '\0';
    }
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_connect(const char *address)
{
    BtEvent event;

    bt_copy_string(s_connected_address, sizeof(s_connected_address), address != NULL ? address : BT_STUB_DEVICE_ADDRESS);
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONE_CONNECTED;
    bt_copy_string(event.address, sizeof(event.address), s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MEDIA_CONNECTED;
    bt_copy_string(event.address, sizeof(event.address), s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_A2DP_CONNECTED;
    bt_copy_string(event.address, sizeof(event.address), s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_AVRCP_CONNECTED;
    bt_copy_string(event.address, sizeof(event.address), s_connected_address);
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_disconnect(const char *address)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONE_DISCONNECTED;
    bt_copy_string(event.address, sizeof(event.address), address != NULL ? address : s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MEDIA_DISCONNECTED;
    bt_copy_string(event.address, sizeof(event.address), address != NULL ? address : s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_A2DP_DISCONNECTED;
    bt_copy_string(event.address, sizeof(event.address), address != NULL ? address : s_connected_address);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_AVRCP_DISCONNECTED;
    bt_copy_string(event.address, sizeof(event.address), address != NULL ? address : s_connected_address);
    (void)bt_event_post(&event);
    s_connected_address[0] = '\0';
    return 0;
}

int bt_platform_hfp_dial(const char *number)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_NUMBER;
    bt_copy_string(event.number, sizeof(event.number), number);
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_STATE;
    event.state = BT_APP_CALL_STATE_OUTGOING;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_redial(void)
{
    return bt_platform_hfp_dial("");
}

int bt_platform_hfp_accept(void)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_STATE;
    event.state = BT_APP_CALL_STATE_SPEAKING;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_hangup(void)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_STATE;
    event.state = BT_APP_CALL_STATE_IDLE;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_hold(BtCallAction action)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_STATE;
    event.state = (action == BT_CALL_ACTION_MERGE) ? BT_APP_CALL_STATE_MULTI : BT_APP_CALL_STATE_HELD;
    (void)bt_event_post(&event);
    post_status("HFP hold action requested");
    return 0;
}

int bt_platform_hfp_switch_audio(bool phone_audio)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_AUDIO_STATE;
    event.value = phone_audio;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_hfp_play_ringtone(void)
{
    post_status("HFP local ringtone started");
    return 0;
}

int bt_platform_hfp_stop_ringtone(void)
{
    post_status("HFP local ringtone stopped");
    return 0;
}

int bt_platform_hfp_send_dtmf(const char *code)
{
    (void)code;
    post_status("DTMF sent");
    return 0;
}

int bt_platform_pbap_download_phonebook(void)
{
    BtEvent event;

    post_pbap_state(BT_PBAP_STATE_DOWNLOADING, BT_PBAP_DOWNLOAD_PHONEBOOK, "PBAP phonebook downloading");
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONEBOOK_RESET;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONEBOOK_RECORD;
    bt_copy_string(event.contact.name, sizeof(event.contact.name), "Demo Contact");
    bt_copy_string(event.contact.number, sizeof(event.contact.number), "1234567890");
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONEBOOK_RECORD;
    bt_copy_string(event.contact.name, sizeof(event.contact.name), "Bluetooth Test");
    bt_copy_string(event.contact.number, sizeof(event.contact.number), "10086");
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONEBOOK_COMPLETE;
    (void)bt_event_post(&event);

    post_pbap_state(BT_PBAP_STATE_FINISHED, BT_PBAP_DOWNLOAD_PHONEBOOK, "PBAP phonebook download finish");
    return 0;
}

int bt_platform_pbap_download_call_records(void)
{
    BtEvent event;

    post_pbap_state(BT_PBAP_STATE_DOWNLOADING, BT_PBAP_DOWNLOAD_CALL_RECORDS, "PBAP call records downloading");
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORDS_RESET;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORD;
    bt_copy_string(event.call_record.name, sizeof(event.call_record.name), "Demo Contact");
    bt_copy_string(event.call_record.number, sizeof(event.call_record.number), "1234567890");
    bt_copy_string(event.call_record.time, sizeof(event.call_record.time), "2026-04-28 10:00");
    event.call_record.type = BT_CALL_RECORD_OUTGOING;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORD;
    bt_copy_string(event.call_record.name, sizeof(event.call_record.name), "Bluetooth Test");
    bt_copy_string(event.call_record.number, sizeof(event.call_record.number), "10086");
    bt_copy_string(event.call_record.time, sizeof(event.call_record.time), "2026-04-28 10:05");
    event.call_record.type = BT_CALL_RECORD_MISSED;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORDS_COMPLETE;
    (void)bt_event_post(&event);

    post_pbap_state(BT_PBAP_STATE_FINISHED, BT_PBAP_DOWNLOAD_CALL_RECORDS, "PBAP call records download finish");
    return 0;
}

int bt_platform_pbap_abort(void)
{
    post_pbap_state(BT_PBAP_STATE_STOPPED, BT_PBAP_DOWNLOAD_NONE, "PBAP download stopped");
    return 0;
}

int bt_platform_avrcp_play_pause(bool currently_playing)
{
    BtEvent event;

    s_music_playing = !currently_playing;
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MUSIC_PLAYING;
    event.value = s_music_playing;
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MEDIA_INFO;
    bt_copy_string(event.media_title, sizeof(event.media_title), "Unknown");
    bt_copy_string(event.media_artist, sizeof(event.media_artist), "Unknown");
    bt_copy_string(event.media_album, sizeof(event.media_album), "Unknown");
    (void)bt_event_post(&event);

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MUSIC_POSITION;
    bt_copy_string(event.media_position, sizeof(event.media_position), s_music_playing ? "00:30" : "00:00");
    bt_copy_string(event.media_duration, sizeof(event.media_duration), "03:30");
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_avrcp_pause(void)
{
    BtEvent event;

    s_music_playing = false;
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MUSIC_PLAYING;
    event.value = false;
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_avrcp_prev(void)
{
    post_status("AVRCP previous requested");
    return 0;
}

int bt_platform_avrcp_next(void)
{
    post_status("AVRCP next requested");
    return 0;
}

int bt_platform_a2dp_init(void)
{
    return 0;
}

int bt_platform_a2dp_set_audio_enabled(bool enabled)
{
    (void)enabled;
    return 0;
}

int bt_platform_set_local_name(const char *name)
{
    BtEvent event;

    if (!copy_local_name(s_local_name, sizeof(s_local_name), name)) {
        return -1;
    }
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_LOCAL_INFO;
    bt_copy_string(event.local_name, sizeof(event.local_name), s_local_name);
    bt_copy_string(event.pairing_pin, sizeof(event.pairing_pin), s_pairing_pin);
    (void)bt_event_post(&event);
    return 0;
}

int bt_platform_set_pairing_pin(const char *pin)
{
    BtEvent event;

    bt_copy_string(s_pairing_pin, sizeof(s_pairing_pin), pin);
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_LOCAL_INFO;
    bt_copy_string(event.local_name, sizeof(event.local_name), s_local_name);
    bt_copy_string(event.pairing_pin, sizeof(event.pairing_pin), s_pairing_pin);
    (void)bt_event_post(&event);
    return 0;
}

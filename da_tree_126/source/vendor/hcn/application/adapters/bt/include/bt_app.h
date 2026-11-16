#ifndef BT_APP_H
#define BT_APP_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_APP_DEVICE_MAX 16

#define BT_LOCAL_NAME_MAX_BYTES 248
#define BT_LOCAL_NAME_TEXT_LEN (BT_LOCAL_NAME_MAX_BYTES + 1)

typedef enum BtAppCallState {
    BT_APP_CALL_STATE_IDLE = 0,
    BT_APP_CALL_STATE_INCOMING,
    BT_APP_CALL_STATE_OUTGOING,
    BT_APP_CALL_STATE_ALERTING,
    BT_APP_CALL_STATE_SPEAKING,
    BT_APP_CALL_STATE_HELD,
    BT_APP_CALL_STATE_WAITING,
    BT_APP_CALL_STATE_MULTI
} BtAppCallState;

typedef enum BtRingtoneType {
    BT_RINGTONE_TYPE_LOCAL = 0,
    BT_RINGTONE_TYPE_INBAND
} BtRingtoneType;

typedef enum BtCallAction {
    BT_CALL_ACTION_HOLD = 0,
    BT_CALL_ACTION_SWITCH,
    BT_CALL_ACTION_MERGE,
    BT_CALL_ACTION_TERMINATE_AND_ACCEPT
} BtCallAction;

typedef enum BtCallRecordType {
    BT_CALL_RECORD_ALL = 0,
    BT_CALL_RECORD_INCOMING,
    BT_CALL_RECORD_OUTGOING,
    BT_CALL_RECORD_MISSED
} BtCallRecordType;

typedef enum BtPbapState {
    BT_PBAP_STATE_IDLE = 0,
    BT_PBAP_STATE_DOWNLOADING,
    BT_PBAP_STATE_FINISHED,
    BT_PBAP_STATE_STOPPED,
    BT_PBAP_STATE_ERROR
} BtPbapState;

typedef enum BtPbapDownloadType {
    BT_PBAP_DOWNLOAD_NONE = 0,
    BT_PBAP_DOWNLOAD_PHONEBOOK,
    BT_PBAP_DOWNLOAD_CALL_RECORDS
} BtPbapDownloadType;

typedef struct BtAppConfig {
    const char *config_path;
} BtAppConfig;

typedef struct BtContact {
    char name[96];
    char number[64];
} BtContact;

typedef struct BtCallRecord {
    char name[96];
    char number[64];
    char time[32];
    BtCallRecordType type;
} BtCallRecord;

typedef struct BtAppSnapshot {
    bool power_on;
    bool scanning;
    bool phone_connected;
    bool media_connected;
    bool a2dp_connected;
    bool avrcp_connected;
    bool in_call;
    bool music_playing;
    bool music_playing_state;
    bool auto_connect;
    bool call_audio_in_phone;
    bool ringtone_requested;
    bool ringtone_type_known;
    BtAppCallState call_state;
    BtRingtoneType ringtone_type;
    BtPbapState pbap_state;
    BtPbapDownloadType pbap_download_type;
    size_t paired_count;
    size_t available_count;
    size_t phonebook_count;
    size_t phonebook_filtered_count;
    size_t call_record_count;
    unsigned int device_version;
    unsigned int phonebook_version;
    unsigned int call_record_version;
    int pbap_current_index;
    int pbap_total_count;
    char paired_devices[BT_APP_DEVICE_MAX][96];
    char available_devices[BT_APP_DEVICE_MAX][96];
    char paired_addresses[BT_APP_DEVICE_MAX][96];
    char available_addresses[BT_APP_DEVICE_MAX][96];
    char connected_address[96];
    char latest_address[96];
    char active_number[64];
    char active_contact_name[96];
    char media_title[96];
    char media_artist[96];
    char media_album[96];
    char media_position[16];
    char media_duration[16];
    char local_name[BT_LOCAL_NAME_TEXT_LEN];
    char pairing_pin[20];
    char status[128];
} BtAppSnapshot;

typedef void (*BtAppUiEnterMainPage)(void *user_data);
typedef void (*BtAppUiGoHome)(void *user_data);
typedef void (*BtAppUiInvalidate)(void *user_data);

typedef struct BtAppUiHooks {
    BtAppUiEnterMainPage enter_main_page;
    BtAppUiGoHome go_home;
    BtAppUiInvalidate invalidate;
    void *user_data;
} BtAppUiHooks;

int bt_app_init(const BtAppConfig *config);
void bt_app_init_async(const BtAppConfig *config);
bool bt_app_is_ready(void);
void bt_app_deinit(void);
void bt_app_poll(void);

void bt_app_set_ui_hooks(const BtAppUiHooks *hooks);
void bt_app_get_snapshot(BtAppSnapshot *snapshot);

void bt_app_enter_main_page(void);
void bt_app_go_home(void);

void bt_app_set_power(bool enabled);
void bt_app_start_scan(void);
void bt_app_stop_scan(void);
void bt_app_pair_device(const char *address);
void bt_app_remove_device(const char *address);
void bt_app_connect_phone(const char *address);
void bt_app_disconnect_phone(const char *address);

void bt_app_dial(const char *number);
void bt_app_redial(void);
void bt_app_accept_call(void);
void bt_app_hangup_call(void);
void bt_app_hold_call(BtCallAction action);
void bt_app_switch_call_audio(bool phone_audio);
void bt_app_play_call_ringtone(void);
void bt_app_stop_call_ringtone(void);
void bt_app_send_dtmf(const char *code);

void bt_app_download_phonebook(void);
void bt_app_download_call_records(void);
void bt_app_abort_pbap_download(void);
bool bt_app_get_contact(size_t index, BtContact *out);
bool bt_app_get_call_record(size_t index, BtCallRecordType filter, BtCallRecord *out);
void bt_app_search_phonebook(const char *keyword);
void bt_app_filter_call_records(BtCallRecordType type);

void bt_app_music_play_pause(void);
void bt_app_music_pause(void);
void bt_app_music_prev(void);
void bt_app_music_next(void);
void bt_app_set_music_audio_enabled(bool enabled);

void bt_app_set_auto_connect(bool enabled);
void bt_app_set_local_name(const char *name);
void bt_app_set_pairing_pin(const char *pin);

size_t bt_utf8_char_len(const unsigned char *text, size_t available);
bool bt_copy_utf8_display_prefix(char *dst, size_t dst_size, const char *src);
bool bt_copy_valid_utf8_display_bytes(char *dst, size_t dst_size, const char *src, size_t src_size);

#ifdef __cplusplus
}
#endif

#endif

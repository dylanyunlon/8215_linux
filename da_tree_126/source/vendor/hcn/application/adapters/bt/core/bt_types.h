#ifndef BT_TYPES_H
#define BT_TYPES_H

#include "bt_app.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_DEVICE_MAX BT_APP_DEVICE_MAX
#define BT_DEVICE_TEXT_LEN 96
#define BT_NUMBER_LEN 64
#define BT_MEDIA_TEXT_LEN 96
#define BT_STATUS_LEN 128
#define BT_CONTACT_NAME_LEN 96
#define BT_CALL_TIME_LEN 32
#define BT_PHONEBOOK_MAX_RECORDS 500U
#define BT_CALL_RECORD_MAX_RECORDS 200U
typedef enum BtEventType {
    BT_EVENT_NONE = 0,
    BT_EVENT_POWER_STATE,
    BT_EVENT_SCAN_STATE,
    BT_EVENT_DEVICE_FOUND,
    BT_EVENT_DEVICE_BONDED,
    BT_EVENT_DEVICE_PAIRED,
    BT_EVENT_DEVICE_REMOVED,
    BT_EVENT_PHONE_CONNECTED,
    BT_EVENT_PHONE_DISCONNECTED,
    BT_EVENT_MEDIA_CONNECTED,
    BT_EVENT_MEDIA_DISCONNECTED,
    BT_EVENT_A2DP_CONNECTED,
    BT_EVENT_A2DP_DISCONNECTED,
    BT_EVENT_AVRCP_CONNECTED,
    BT_EVENT_AVRCP_DISCONNECTED,
    BT_EVENT_CALL_STATE,
    BT_EVENT_RINGTONE_TYPE,
    BT_EVENT_RINGTONE,
    BT_EVENT_CALL_AUDIO_STATE,
    BT_EVENT_CALL_NUMBER,
    BT_EVENT_MUSIC_PLAYING,
    BT_EVENT_MUSIC_POSITION,
    BT_EVENT_MEDIA_INFO,
    BT_EVENT_PBAP_STATE,
    BT_EVENT_PHONEBOOK_RESET,
    BT_EVENT_PHONEBOOK_RECORD,
    BT_EVENT_PHONEBOOK_COMPLETE,
    BT_EVENT_CALL_RECORDS_RESET,
    BT_EVENT_CALL_RECORD,
    BT_EVENT_CALL_RECORDS_COMPLETE,
    BT_EVENT_LOCAL_INFO,
    BT_EVENT_STATUS
} BtEventType;

typedef struct BtEvent {
    BtEventType type;
    bool value;
    int state;
    int pbap_current_index;
    int pbap_total_count;
    BtPbapDownloadType pbap_download_type;
    char address[BT_DEVICE_TEXT_LEN];
    char name[BT_DEVICE_TEXT_LEN];
    char text[BT_STATUS_LEN];
    char number[BT_NUMBER_LEN];
    char media_title[BT_MEDIA_TEXT_LEN];
    char media_artist[BT_MEDIA_TEXT_LEN];
    char media_album[BT_MEDIA_TEXT_LEN];
    char media_position[16];
    char media_duration[16];
    char local_name[BT_LOCAL_NAME_TEXT_LEN];
    char pairing_pin[20];
    BtContact contact;
    BtCallRecord call_record;
} BtEvent;

typedef struct BtDevice {
    char address[BT_DEVICE_TEXT_LEN];
    char name[BT_DEVICE_TEXT_LEN];
} BtDevice;

typedef struct BtState {
    bool initialized;
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
    bool pending_auto_connect;
    uint64_t pending_auto_connect_ready_ms;
    bool suppress_auto_reconnect;
    bool call_audio_in_phone;
    bool phonebook_truncated;
    bool phonebook_bulk_loading;
    bool call_records_bulk_loading;
    bool call_records_truncated;
    bool ringtone_requested;
    bool ringtone_type_known;
    BtAppCallState call_state;
    BtRingtoneType ringtone_type;
    BtCallRecordType active_call_type;
    bool active_call_was_answered;
    bool active_call_seen;
    BtPbapState pbap_state;
    BtPbapDownloadType pbap_download_type;
    int pbap_current_index;
    int pbap_total_count;
    BtCallRecordType call_record_filter;
    char phonebook_filter_keyword[BT_CONTACT_NAME_LEN];
    char config_path[260];
    char connected_address[BT_DEVICE_TEXT_LEN];
    char latest_address[BT_DEVICE_TEXT_LEN];
    char active_number[BT_NUMBER_LEN];
    char active_contact_name[BT_CONTACT_NAME_LEN];
    char media_title[BT_MEDIA_TEXT_LEN];
    char media_artist[BT_MEDIA_TEXT_LEN];
    char media_album[BT_MEDIA_TEXT_LEN];
    char media_position[16];
    char media_duration[16];
    char local_name[BT_LOCAL_NAME_TEXT_LEN];
    char pairing_pin[20];
    char status[BT_STATUS_LEN];
    BtDevice paired_devices[BT_DEVICE_MAX];
    BtDevice available_devices[BT_DEVICE_MAX];
    BtContact *phonebook;
    size_t *phonebook_filtered_index;
    BtCallRecord *call_records;
    char paired_addresses[BT_DEVICE_MAX][24];
    char available_addresses[BT_DEVICE_MAX][24];
    size_t paired_count;
    size_t available_count;
    size_t phonebook_count;
    size_t phonebook_capacity;
    size_t phonebook_filtered_count;
    size_t phonebook_filtered_capacity;
    size_t call_record_count;
    size_t call_record_filtered_count;
    size_t call_record_capacity;
    unsigned int device_version;
    unsigned int phonebook_version;
    unsigned int call_record_version;
} BtState;

void bt_copy_string(char *dst, size_t dst_size, const char *src);
void bt_format_device(char *dst, size_t dst_size, const BtDevice *device);
size_t bt_utf8_char_len(const unsigned char *text, size_t available);
bool bt_copy_utf8_display_prefix(char *dst, size_t dst_size, const char *src);
bool bt_copy_valid_utf8_display_bytes(char *dst, size_t dst_size, const char *src, size_t src_size);

#ifdef __cplusplus
}
#endif

#endif

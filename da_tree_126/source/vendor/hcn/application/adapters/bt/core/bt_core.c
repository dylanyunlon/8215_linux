#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "bt_core_internal.h"

#include "bt_event_queue.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#define BT_DEFAULT_CONFIG_PATH "/data/bluetoothconfiguration.cfg"
#define BT_DEFAULT_AUTO_CONNECT true
#define BT_CORE_POLL_TIME_BUDGET_MS 2U
#define BT_CORE_POLL_TIME_CHECK_INTERVAL 16U

pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
BtState s_state;
BtAppUiHooks s_hooks;
bool s_pending_invalidate;
size_t s_seen_event_drop_count;
static bool s_event_budget_deferred_logged;

void reset_active_call_tracking_locked(void)
{
    s_state.active_call_type = BT_CALL_RECORD_ALL;
    s_state.active_call_was_answered = false;
    s_state.active_call_seen = false;
}

void bt_copy_string(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    len = strlen(src);
    if (len >= dst_size) {
        len = dst_size - 1U;
        while (len > 0U && ((unsigned char)src[len] & 0xC0U) == 0x80U) {
            len--;
        }
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

void bt_format_device(char *dst, size_t dst_size, const BtDevice *device)
{
    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (device == NULL || device->address[0] == '\0') {
        dst[0] = '\0';
        return;
    }

    bt_copy_string(dst, dst_size, device->name[0] != '\0' ? device->name : device->address);
}

size_t bt_utf8_char_len(const unsigned char *text, size_t available)
{
    unsigned char ch;

    if (text == NULL || available == 0U) {
        return 0U;
    }

    ch = text[0];
    if (ch < 0x80U) {
        return (ch >= 0x20U && ch != 0x7FU) ? 1U : 0U;
    }
    if (ch >= 0xC2U && ch <= 0xDFU) {
        return available >= 2U && (text[1] & 0xC0U) == 0x80U ? 2U : 0U;
    }
    if (ch == 0xE0U) {
        return available >= 3U && text[1] >= 0xA0U && text[1] <= 0xBFU &&
               (text[2] & 0xC0U) == 0x80U ? 3U : 0U;
    }
    if ((ch >= 0xE1U && ch <= 0xECU) || (ch >= 0xEEU && ch <= 0xEFU)) {
        return available >= 3U && (text[1] & 0xC0U) == 0x80U &&
               (text[2] & 0xC0U) == 0x80U ? 3U : 0U;
    }
    if (ch == 0xEDU) {
        return available >= 3U && text[1] >= 0x80U && text[1] <= 0x9FU &&
               (text[2] & 0xC0U) == 0x80U ? 3U : 0U;
    }
    if (ch == 0xF0U) {
        return available >= 4U && text[1] >= 0x90U && text[1] <= 0xBFU &&
               (text[2] & 0xC0U) == 0x80U && (text[3] & 0xC0U) == 0x80U ? 4U : 0U;
    }
    if (ch >= 0xF1U && ch <= 0xF3U) {
        return available >= 4U && (text[1] & 0xC0U) == 0x80U &&
               (text[2] & 0xC0U) == 0x80U && (text[3] & 0xC0U) == 0x80U ? 4U : 0U;
    }
    if (ch == 0xF4U) {
        return available >= 4U && text[1] >= 0x80U && text[1] <= 0x8FU &&
               (text[2] & 0xC0U) == 0x80U && (text[3] & 0xC0U) == 0x80U ? 4U : 0U;
    }

    return 0U;
}

static uint64_t monotonic_time_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }

    return ((uint64_t)ts.tv_sec * 1000U) + ((uint64_t)ts.tv_nsec / 1000000U);
}

bool bt_copy_utf8_display_prefix(char *dst, size_t dst_size, const char *src)
{
    size_t written = 0U;
    const unsigned char *p;
    size_t remaining;

    if (dst == NULL || dst_size == 0U) {
        return false;
    }
    if (src == NULL || src[0] == '\0') {
        dst[0] = '\0';
        return false;
    }

    p = (const unsigned char *)src;
    remaining = strlen(src);
    while (remaining > 0U && written + 1U < dst_size) {
        size_t len = bt_utf8_char_len(p, remaining);
        size_t i;

        if (len == 0U || written + len >= dst_size) {
            break;
        }
        for (i = 0U; i < len; ++i) {
            dst[written++] = (char)p[i];
        }
        p += len;
        remaining -= len;
    }
    dst[written] = '\0';
    return written > 0U;
}

bool bt_copy_valid_utf8_display_bytes(char *dst, size_t dst_size, const char *src, size_t src_size)
{
    size_t written = 0U;
    const unsigned char *p;
    size_t remaining;

    if (dst == NULL || dst_size == 0U) {
        return false;
    }
    dst[0] = '\0';
    if (src == NULL || src_size == 0U) {
        return false;
    }

    p = (const unsigned char *)src;
    remaining = src_size;
    while (remaining > 0U) {
        size_t len = bt_utf8_char_len(p, remaining);
        size_t i;

        if (len == 0U) {
            if (p[0] < 0x20U || p[0] == 0x7FU) {
                p++;
                remaining--;
                continue;
            }
            dst[0] = '\0';
            return false;
        }
        if (written + len >= dst_size) {
            break;
        }
        for (i = 0U; i < len; ++i) {
            dst[written++] = (char)p[i];
        }
        p += len;
        remaining -= len;
    }
    dst[written] = '\0';
    return written > 0U;
}

void queue_auto_connect_locked(unsigned int delay_ms)
{
    uint64_t now_ms;

    if (s_state.power_on &&
        s_state.auto_connect &&
        s_state.latest_address[0] != '\0' &&
        !s_state.phone_connected &&
        !s_state.pending_auto_connect) {
        now_ms = monotonic_time_ms();
        s_state.pending_auto_connect = true;
        s_state.pending_auto_connect_ready_ms = now_ms != 0U ? now_ms + delay_ms : 0U;
    }
}

void set_status_locked(const char *status)
{
    bt_copy_string(s_state.status, sizeof(s_state.status), status);
}

void request_invalidate_locked(void)
{
    s_pending_invalidate = true;
}

static void dispatch_invalidate_if_needed(void)
{
    BtAppUiHooks hooks;
    bool should_invalidate;

    pthread_mutex_lock(&s_state_mutex);
    hooks = s_hooks;
    should_invalidate = s_pending_invalidate;
    s_pending_invalidate = false;
    pthread_mutex_unlock(&s_state_mutex);

    if (should_invalidate && hooks.invalidate != NULL) {
        hooks.invalidate(hooks.user_data);
    }
}

int bt_core_init(const BtAppConfig *config)
{
    pthread_mutex_lock(&s_state_mutex);
    free_call_record_storage_locked();
    free_phonebook_storage_locked();
    memset(&s_state, 0, sizeof(s_state));
    bt_copy_string(s_state.config_path, sizeof(s_state.config_path),
                   (config != NULL && config->config_path != NULL) ? config->config_path : BT_DEFAULT_CONFIG_PATH);
    bt_copy_string(s_state.media_title, sizeof(s_state.media_title), "Unknown");
    bt_copy_string(s_state.media_artist, sizeof(s_state.media_artist), "Unknown");
    bt_copy_string(s_state.media_album, sizeof(s_state.media_album), "Unknown");
    s_state.call_state = BT_APP_CALL_STATE_IDLE;
    s_state.ringtone_type = BT_RINGTONE_TYPE_LOCAL;
    s_state.ringtone_requested = false;
    s_state.ringtone_type_known = false;
    s_state.pbap_state = BT_PBAP_STATE_IDLE;
    s_state.pbap_download_type = BT_PBAP_DOWNLOAD_NONE;
    s_state.call_record_filter = BT_CALL_RECORD_ALL;
    s_state.auto_connect = BT_DEFAULT_AUTO_CONNECT;
    reset_active_call_tracking_locked();
    set_status_locked("Bluetooth is idle");
    load_config_locked();
    s_state.initialized = true;
    s_pending_invalidate = true;
    s_seen_event_drop_count = 0U;
    s_event_budget_deferred_logged = false;
    pthread_mutex_unlock(&s_state_mutex);

    bt_event_queue_reset();
    return 0;
}

void bt_core_deinit(void)
{
    pthread_mutex_lock(&s_state_mutex);
    free_call_record_storage_locked();
    free_phonebook_storage_locked();
    memset(&s_state, 0, sizeof(s_state));
    memset(&s_hooks, 0, sizeof(s_hooks));
    s_pending_invalidate = false;
    s_seen_event_drop_count = 0U;
    s_event_budget_deferred_logged = false;
    pthread_mutex_unlock(&s_state_mutex);
    bt_event_queue_reset();
}

void bt_core_poll(void)
{
    BtEvent event;
    size_t dropped_count;
    size_t processed_count = 0U;
    uint64_t start_ms;
    bool budget_deferred = false;

    start_ms = monotonic_time_ms();
    while (bt_event_pop(&event)) {
        uint64_t now_ms;

        pthread_mutex_lock(&s_state_mutex);
        apply_event_locked(&event);
        pthread_mutex_unlock(&s_state_mutex);

        processed_count++;
        if (start_ms != 0U && (processed_count % BT_CORE_POLL_TIME_CHECK_INTERVAL) == 0U) {
            now_ms = monotonic_time_ms();
            if (now_ms != 0U && now_ms - start_ms >= BT_CORE_POLL_TIME_BUDGET_MS) {
                budget_deferred = true;
                break;
            }
        }
    }

    if (budget_deferred) {
        if (!s_event_budget_deferred_logged) {
            syslog(LOG_WARNING, "Bluetooth event processing exceeded %u ms; deferring remaining events",
                   (unsigned int)BT_CORE_POLL_TIME_BUDGET_MS);
            s_event_budget_deferred_logged = true;
        }
    } else {
        s_event_budget_deferred_logged = false;
    }

    dropped_count = bt_event_queue_dropped_count();
    if (dropped_count != s_seen_event_drop_count) {
        char status[BT_STATUS_LEN];

        pthread_mutex_lock(&s_state_mutex);
        snprintf(status, sizeof(status), "Bluetooth event queue overflow; dropped %zu event(s)", dropped_count);
        set_status_locked(status);
        request_invalidate_locked();
        pthread_mutex_unlock(&s_state_mutex);
        s_seen_event_drop_count = dropped_count;
    }

    dispatch_invalidate_if_needed();
}

void bt_core_set_ui_hooks(const BtAppUiHooks *hooks)
{
    pthread_mutex_lock(&s_state_mutex);
    if (hooks != NULL) {
        s_hooks = *hooks;
    } else {
        memset(&s_hooks, 0, sizeof(s_hooks));
    }
    pthread_mutex_unlock(&s_state_mutex);
}

void bt_core_get_snapshot(BtAppSnapshot *snapshot)
{
    size_t i;

    if (snapshot == NULL) {
        return;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    pthread_mutex_lock(&s_state_mutex);
    snapshot->power_on = s_state.power_on;
    snapshot->scanning = s_state.scanning;
    snapshot->phone_connected = s_state.phone_connected;
    snapshot->media_connected = s_state.media_connected;
    snapshot->a2dp_connected = s_state.a2dp_connected;
    snapshot->avrcp_connected = s_state.avrcp_connected;
    snapshot->in_call = s_state.in_call;
    snapshot->music_playing = s_state.music_playing;
    snapshot->music_playing_state = s_state.music_playing_state;
    snapshot->auto_connect = s_state.auto_connect;
    snapshot->call_audio_in_phone = s_state.call_audio_in_phone;
    snapshot->ringtone_requested = s_state.ringtone_requested;
    snapshot->ringtone_type_known = s_state.ringtone_type_known;
    snapshot->call_state = s_state.call_state;
    snapshot->ringtone_type = s_state.ringtone_type;
    snapshot->pbap_state = s_state.pbap_state;
    snapshot->pbap_download_type = s_state.pbap_download_type;
    snapshot->paired_count = s_state.paired_count;
    snapshot->available_count = s_state.available_count;
    snapshot->phonebook_count = s_state.phonebook_count;
    snapshot->phonebook_filtered_count = s_state.phonebook_filtered_count;
    snapshot->call_record_count = s_state.call_record_filtered_count;
    snapshot->device_version = s_state.device_version;
    snapshot->phonebook_version = s_state.phonebook_version;
    snapshot->call_record_version = s_state.call_record_version;
    snapshot->pbap_current_index = s_state.pbap_current_index;
    snapshot->pbap_total_count = s_state.pbap_total_count;
    for (i = 0U; i < s_state.paired_count && i < BT_DEVICE_MAX; ++i) {
        bt_format_device(snapshot->paired_devices[i], sizeof(snapshot->paired_devices[i]), &s_state.paired_devices[i]);
        bt_copy_string(snapshot->paired_addresses[i], sizeof(snapshot->paired_addresses[i]), s_state.paired_devices[i].address);
    }
    for (i = 0U; i < s_state.available_count && i < BT_DEVICE_MAX; ++i) {
        bt_format_device(snapshot->available_devices[i], sizeof(snapshot->available_devices[i]), &s_state.available_devices[i]);
        bt_copy_string(snapshot->available_addresses[i], sizeof(snapshot->available_addresses[i]), s_state.available_devices[i].address);
    }
    bt_copy_string(snapshot->connected_address, sizeof(snapshot->connected_address), s_state.connected_address);
    bt_copy_string(snapshot->latest_address, sizeof(snapshot->latest_address), s_state.latest_address);
    bt_copy_string(snapshot->active_number, sizeof(snapshot->active_number), s_state.active_number);
    bt_copy_string(snapshot->active_contact_name, sizeof(snapshot->active_contact_name), s_state.active_contact_name);
    bt_copy_string(snapshot->media_title, sizeof(snapshot->media_title), s_state.media_title);
    bt_copy_string(snapshot->media_artist, sizeof(snapshot->media_artist), s_state.media_artist);
    bt_copy_string(snapshot->media_album, sizeof(snapshot->media_album), s_state.media_album);
    bt_copy_string(snapshot->media_position, sizeof(snapshot->media_position), s_state.media_position);
    bt_copy_string(snapshot->media_duration, sizeof(snapshot->media_duration), s_state.media_duration);
    bt_copy_string(snapshot->local_name, sizeof(snapshot->local_name), s_state.local_name);
    bt_copy_string(snapshot->pairing_pin, sizeof(snapshot->pairing_pin), s_state.pairing_pin);
    bt_copy_string(snapshot->status, sizeof(snapshot->status), s_state.status);
    pthread_mutex_unlock(&s_state_mutex);
}

void bt_core_notify_status(const char *status)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_STATUS;
    bt_copy_string(event.text, sizeof(event.text), status);
    (void)bt_event_post(&event);
}

void bt_core_enter_main_page(void)
{
    BtAppUiHooks hooks;

    pthread_mutex_lock(&s_state_mutex);
    hooks = s_hooks;
    pthread_mutex_unlock(&s_state_mutex);

    if (hooks.enter_main_page != NULL) {
        hooks.enter_main_page(hooks.user_data);
    }
}

void bt_core_go_home(void)
{
    BtAppUiHooks hooks;

    pthread_mutex_lock(&s_state_mutex);
    hooks = s_hooks;
    pthread_mutex_unlock(&s_state_mutex);

    if (hooks.go_home != NULL) {
        hooks.go_home(hooks.user_data);
    }
}

void bt_core_set_auto_connect(bool enabled)
{
    pthread_mutex_lock(&s_state_mutex);
    s_state.auto_connect = enabled;
    if (enabled) {
        queue_auto_connect_locked(0U);
    } else {
        s_state.pending_auto_connect = false;
        s_state.pending_auto_connect_ready_ms = 0U;
    }
    save_config_locked();
    set_status_locked(enabled ? "Auto connect enabled" : "Auto connect disabled");
    request_invalidate_locked();
    pthread_mutex_unlock(&s_state_mutex);
}

bool bt_core_should_auto_connect(char *address, size_t address_size)
{
    bool should_connect;

    pthread_mutex_lock(&s_state_mutex);
    should_connect = s_state.auto_connect && s_state.latest_address[0] != '\0';
    if (should_connect) {
        bt_copy_string(address, address_size, s_state.latest_address);
    } else if (address != NULL && address_size > 0U) {
        address[0] = '\0';
    }
    pthread_mutex_unlock(&s_state_mutex);

    return should_connect;
}

bool bt_core_pending_auto_connect(char *address, size_t address_size)
{
    bool should_connect;
    uint64_t now_ms;

    pthread_mutex_lock(&s_state_mutex);
    should_connect = s_state.pending_auto_connect &&
                     s_state.power_on &&
                     s_state.auto_connect &&
                     s_state.latest_address[0] != '\0' &&
                     !s_state.phone_connected;
    if (should_connect) {
        now_ms = monotonic_time_ms();
        if (now_ms != 0U && now_ms < s_state.pending_auto_connect_ready_ms) {
            should_connect = false;
        }
    }
    if (should_connect && address != NULL && address_size > 0U) {
        bt_copy_string(address, address_size, s_state.latest_address);
    } else if (address != NULL && address_size > 0U) {
        address[0] = '\0';
    }
    if (should_connect ||
        !s_state.power_on ||
        !s_state.auto_connect ||
        s_state.latest_address[0] == '\0' ||
        s_state.phone_connected) {
        s_state.pending_auto_connect = false;
        s_state.pending_auto_connect_ready_ms = 0U;
    }
    pthread_mutex_unlock(&s_state_mutex);

    return should_connect;
}

void bt_core_notify_manual_disconnect(void)
{
    pthread_mutex_lock(&s_state_mutex);
    s_state.suppress_auto_reconnect = true;
    s_state.pending_auto_connect = false;
    s_state.pending_auto_connect_ready_ms = 0U;
    pthread_mutex_unlock(&s_state_mutex);
}

bool bt_core_get_contact(size_t index, BtContact *out)
{
    bool ok = false;

    if (out == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    pthread_mutex_lock(&s_state_mutex);
    if (index < s_state.phonebook_filtered_count &&
        s_state.phonebook != NULL && s_state.phonebook_filtered_index != NULL) {
        size_t contact_index = s_state.phonebook_filtered_index[index];
        if (contact_index < s_state.phonebook_count) {
            *out = s_state.phonebook[contact_index];
            ok = true;
        }
    }
    pthread_mutex_unlock(&s_state_mutex);

    return ok;
}

bool bt_core_get_call_record(size_t index, BtCallRecordType filter, BtCallRecord *out)
{
    bool ok = false;
    size_t i;
    size_t matched = 0U;

    if (out == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    pthread_mutex_lock(&s_state_mutex);
    if (filter != s_state.call_record_filter) {
        rebuild_call_record_filter_locked(filter);
    }
    for (i = 0U; i < s_state.call_record_count; ++i) {
        if (!call_record_matches_filter(&s_state.call_records[i], filter)) {
            continue;
        }
        if (matched == index) {
            *out = s_state.call_records[i];
            ok = true;
            break;
        }
        matched++;
    }
    pthread_mutex_unlock(&s_state_mutex);

    return ok;
}

void bt_core_search_phonebook(const char *keyword)
{
    pthread_mutex_lock(&s_state_mutex);
    rebuild_phonebook_filter_locked(keyword);
    s_state.phonebook_version++;
    set_status_locked(keyword != NULL && keyword[0] != '\0' ? "Phonebook search updated" : "Phonebook list restored");
    request_invalidate_locked();
    pthread_mutex_unlock(&s_state_mutex);
}

void bt_core_filter_call_records(BtCallRecordType type)
{
    if (type < BT_CALL_RECORD_ALL || type > BT_CALL_RECORD_MISSED) {
        type = BT_CALL_RECORD_ALL;
    }

    pthread_mutex_lock(&s_state_mutex);
    rebuild_call_record_filter_locked(type);
    s_state.call_record_version++;
    set_status_locked("Call record filter updated");
    request_invalidate_locked();
    pthread_mutex_unlock(&s_state_mutex);
}

#include "bt_core_internal.h"

#include <string.h>

static bool call_record_name_is_missing(const char *name)
{
    return name == NULL || name[0] == '\0' || strcmp(name, "Unknown") == 0;
}

static void resolve_call_record_display_name_locked(BtCallRecord *record)
{
    const char *contact_name;

    if (record == NULL) {
        return;
    }

    contact_name = find_contact_name_locked(record->number);
    if (contact_name != NULL && contact_name[0] != '\0') {
        bt_copy_string(record->name, sizeof(record->name), contact_name);
    } else if (call_record_name_is_missing(record->name)) {
        bt_copy_string(record->name, sizeof(record->name),
                       record->number[0] != '\0' ? record->number : "Unknown");
    }
}

static void update_latest_address_locked(const char *address)
{
    if (address == NULL || address[0] == '\0') {
        return;
    }

    bt_copy_string(s_state.latest_address, sizeof(s_state.latest_address), address);
}

static void clear_media_state_locked(bool clear_metadata)
{
    s_state.music_playing = false;
    s_state.music_playing_state = false;
    s_state.media_position[0] = '\0';
    s_state.media_duration[0] = '\0';
    if (clear_metadata) {
        bt_copy_string(s_state.media_title, sizeof(s_state.media_title), "Unknown");
        bt_copy_string(s_state.media_artist, sizeof(s_state.media_artist), "Unknown");
        bt_copy_string(s_state.media_album, sizeof(s_state.media_album), "Unknown");
    }
}

static BtDevice *find_device(BtDevice *devices, size_t count, const char *address)
{
    size_t i;

    if (address == NULL || address[0] == '\0') {
        return NULL;
    }

    for (i = 0U; i < count; ++i) {
        if (strcmp(devices[i].address, address) == 0) {
            return &devices[i];
        }
    }
    return NULL;
}

static bool has_device(BtDevice *devices, size_t count, const char *address)
{
    return find_device(devices, count, address) != NULL;
}

static void upsert_device(BtDevice *devices, size_t *count, const char *address, const char *name)
{
    BtDevice *device;

    if (devices == NULL || count == NULL || address == NULL || address[0] == '\0') {
        return;
    }

    device = find_device(devices, *count, address);
    if (device == NULL) {
        if (*count >= BT_DEVICE_MAX) {
            return;
        }
        device = &devices[*count];
        memset(device, 0, sizeof(*device));
        (*count)++;
    }

    bt_copy_string(device->address, sizeof(device->address), address);
    bt_copy_string(device->name, sizeof(device->name), name);
}

static size_t device_index(BtDevice *devices, size_t count, const char *address)
{
    size_t i;

    if (address == NULL || address[0] == '\0') {
        return count;
    }
    for (i = 0U; i < count; ++i) {
        if (strcmp(devices[i].address, address) == 0) {
            return i;
        }
    }
    return count;
}

static void sync_device_address_slot(BtDevice *devices, size_t count, char addresses[][24], const char *address)
{
    size_t idx;

    if (devices == NULL || addresses == NULL || address == NULL || address[0] == '\0') {
        return;
    }

    idx = device_index(devices, count, address);
    if (idx < BT_DEVICE_MAX) {
        bt_copy_string(addresses[idx], sizeof(addresses[idx]), address);
    }
}

static void remove_device(BtDevice *devices, size_t *count, const char *address)
{
    size_t i;

    if (devices == NULL || count == NULL || address == NULL || address[0] == '\0') {
        return;
    }

    for (i = 0U; i < *count; ++i) {
        if (strcmp(devices[i].address, address) == 0) {
            if (i + 1U < *count) {
                memmove(&devices[i], &devices[i + 1U], ((*count - i) - 1U) * sizeof(devices[0]));
            }
            (*count)--;
            memset(&devices[*count], 0, sizeof(devices[0]));
            return;
        }
    }
}

void apply_event_locked(const BtEvent *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->type) {
    case BT_EVENT_POWER_STATE:
    {
        bool was_power_on = s_state.power_on;

        s_state.power_on = event->value;
        if (!event->value) {
            s_state.scanning = false;
            s_state.phone_connected = false;
            s_state.media_connected = false;
            s_state.a2dp_connected = false;
            s_state.avrcp_connected = false;
            s_state.in_call = false;
            s_state.call_state = BT_APP_CALL_STATE_IDLE;
            s_state.ringtone_type = BT_RINGTONE_TYPE_LOCAL;
            s_state.ringtone_requested = false;
            s_state.ringtone_type_known = false;
            s_state.music_playing = false;
            s_state.music_playing_state = false;
            if (s_state.latest_address[0] == '\0' && s_state.connected_address[0] != '\0') {
                update_latest_address_locked(s_state.connected_address);
                save_config_locked();
            }
            s_state.connected_address[0] = '\0';
            s_state.active_number[0] = '\0';
            s_state.active_contact_name[0] = '\0';
            reset_active_call_tracking_locked();
            s_state.pending_auto_connect = false;
            s_state.pending_auto_connect_ready_ms = 0U;
            s_state.suppress_auto_reconnect = false;
            memset(s_state.paired_devices, 0, sizeof(s_state.paired_devices));
            memset(s_state.available_devices, 0, sizeof(s_state.available_devices));
            memset(s_state.paired_addresses, 0, sizeof(s_state.paired_addresses));
            memset(s_state.available_addresses, 0, sizeof(s_state.available_addresses));
            clear_phonebook_locked(true);
            s_state.paired_count = 0U;
            s_state.available_count = 0U;
            clear_call_records_locked(true);
            s_state.pbap_state = BT_PBAP_STATE_IDLE;
            s_state.pbap_download_type = BT_PBAP_DOWNLOAD_NONE;
            s_state.pbap_current_index = 0;
            s_state.pbap_total_count = 0;
            s_state.device_version++;
            s_state.phonebook_version++;
            s_state.call_record_version++;
        } else if (!was_power_on) {
            queue_auto_connect_locked(BT_AUTO_CONNECT_POWER_ON_DELAY_MS);
        }
        set_status_locked(event->value ? "Bluetooth power on" : "Bluetooth power off");
        break;
    }
    case BT_EVENT_SCAN_STATE:
        s_state.scanning = event->value;
        if (event->value) {
            memset(s_state.available_devices, 0, sizeof(s_state.available_devices));
            memset(s_state.available_addresses, 0, sizeof(s_state.available_addresses));
            s_state.available_count = 0U;
            s_state.device_version++;
        }
        set_status_locked(event->value ? "Scanning for Bluetooth devices" : "Bluetooth scan stopped");
        break;
    case BT_EVENT_DEVICE_FOUND:
        if (event->address[0] != '\0' &&
            has_device(s_state.paired_devices, s_state.paired_count, event->address)) {
            upsert_device(s_state.paired_devices, &s_state.paired_count, event->address, event->name);
            sync_device_address_slot(s_state.paired_devices, s_state.paired_count,
                                     s_state.paired_addresses, event->address);
            s_state.device_version++;
            set_status_locked("Bluetooth paired device updated");
            break;
        }
        if (event->address[0] != '\0' &&
            strcmp(s_state.connected_address, event->address) == 0) {
            break;
        }
        upsert_device(s_state.available_devices, &s_state.available_count, event->address, event->name);
        sync_device_address_slot(s_state.available_devices, s_state.available_count,
                                 s_state.available_addresses, event->address);
        s_state.device_version++;
        set_status_locked("Bluetooth device discovered");
        break;
    case BT_EVENT_DEVICE_BONDED:
        remove_device(s_state.available_devices, &s_state.available_count, event->address);
        upsert_device(s_state.paired_devices, &s_state.paired_count, event->address, event->name);
        sync_device_address_slot(s_state.paired_devices, s_state.paired_count,
                                 s_state.paired_addresses, event->address);
        if (s_state.latest_address[0] == '\0' && event->address[0] != '\0') {
            update_latest_address_locked(event->address);
            save_config_locked();
            queue_auto_connect_locked(BT_AUTO_CONNECT_POWER_ON_DELAY_MS);
        }
        s_state.device_version++;
        set_status_locked("Bluetooth bonded device restored");
        break;
    case BT_EVENT_DEVICE_PAIRED:
        remove_device(s_state.available_devices, &s_state.available_count, event->address);
        upsert_device(s_state.paired_devices, &s_state.paired_count, event->address, event->name);
        sync_device_address_slot(s_state.paired_devices, s_state.paired_count,
                                 s_state.paired_addresses, event->address);
        s_state.device_version++;
        set_status_locked("Bluetooth pair success");
        break;
    case BT_EVENT_DEVICE_REMOVED:
        remove_device(s_state.paired_devices, &s_state.paired_count, event->address);
        {
            size_t i;
            memset(s_state.paired_addresses, 0, sizeof(s_state.paired_addresses));
            for (i = 0U; i < s_state.paired_count && i < BT_DEVICE_MAX; ++i) {
                bt_copy_string(s_state.paired_addresses[i],
                               sizeof(s_state.paired_addresses[i]),
                               s_state.paired_devices[i].address);
            }
        }
        if (event->address[0] != '\0' && strcmp(s_state.latest_address, event->address) == 0) {
            s_state.latest_address[0] = '\0';
            save_config_locked();
        }
        if (event->address[0] != '\0' && strcmp(s_state.connected_address, event->address) == 0) {
            s_state.connected_address[0] = '\0';
        }
        s_state.device_version++;
        set_status_locked("Bluetooth pair removed");
        break;
    case BT_EVENT_PHONE_CONNECTED:
        remove_device(s_state.available_devices, &s_state.available_count, event->address);
        s_state.phone_connected = true;
        s_state.pending_auto_connect = false;
        s_state.pending_auto_connect_ready_ms = 0U;
        s_state.suppress_auto_reconnect = false;
        bt_copy_string(s_state.connected_address, sizeof(s_state.connected_address), event->address);
        update_latest_address_locked(event->address);
        save_config_locked();
        s_state.device_version++;
        set_status_locked("Phone audio connected");
        break;
    case BT_EVENT_PHONE_DISCONNECTED:
        if (s_state.connected_address[0] != '\0' &&
            event->address[0] != '\0' &&
            strcmp(s_state.connected_address, event->address) != 0) {
            break;
        }
        s_state.phone_connected = false;
        s_state.in_call = false;
        s_state.call_state = BT_APP_CALL_STATE_IDLE;
        s_state.ringtone_type = BT_RINGTONE_TYPE_LOCAL;
        s_state.ringtone_requested = false;
        s_state.ringtone_type_known = false;
        s_state.active_number[0] = '\0';
        s_state.active_contact_name[0] = '\0';
        reset_active_call_tracking_locked();
        clear_phonebook_locked(false);
        clear_call_records_locked(false);
        s_state.pbap_state = BT_PBAP_STATE_IDLE;
        s_state.pbap_download_type = BT_PBAP_DOWNLOAD_NONE;
        s_state.pbap_current_index = 0;
        s_state.pbap_total_count = 0;
        s_state.connected_address[0] = '\0';
        if (event->address[0] != '\0') {
            update_latest_address_locked(event->address);
            save_config_locked();
        }
        s_state.suppress_auto_reconnect = false;
        /* Keep power-on auto connect requests; disconnect events may arrive late. */
        s_state.device_version++;
        s_state.phonebook_version++;
        s_state.call_record_version++;
        set_status_locked("Phone audio disconnected");
        break;
    case BT_EVENT_MEDIA_CONNECTED:
        remove_device(s_state.available_devices, &s_state.available_count, event->address);
        s_state.media_connected = true;
        update_latest_address_locked(event->address);
        save_config_locked();
        s_state.device_version++;
        set_status_locked("Media connected");
        break;
    case BT_EVENT_MEDIA_DISCONNECTED:
        s_state.media_connected = false;
        s_state.a2dp_connected = false;
        s_state.avrcp_connected = false;
        clear_media_state_locked(true);
        set_status_locked("Media disconnected");
        break;
    case BT_EVENT_A2DP_CONNECTED:
        s_state.a2dp_connected = true;
        s_state.media_connected = true;
        set_status_locked("A2DP connected");
        break;
    case BT_EVENT_A2DP_DISCONNECTED:
        s_state.a2dp_connected = false;
        s_state.media_connected = s_state.avrcp_connected;
        clear_media_state_locked(!s_state.media_connected);
        set_status_locked("A2DP disconnected");
        break;
    case BT_EVENT_AVRCP_CONNECTED:
        s_state.avrcp_connected = true;
        s_state.media_connected = true;
        set_status_locked("AVRCP connected");
        break;
    case BT_EVENT_AVRCP_DISCONNECTED:
        s_state.avrcp_connected = false;
        s_state.media_connected = s_state.a2dp_connected;
        clear_media_state_locked(true);
        set_status_locked("AVRCP disconnected");
        break;
    case BT_EVENT_CALL_STATE:
        s_state.call_state = (BtAppCallState)event->state;
        s_state.in_call = (s_state.call_state != BT_APP_CALL_STATE_IDLE);
        if (s_state.call_state == BT_APP_CALL_STATE_IDLE) {
            bool call_record_failed = false;

            s_state.ringtone_requested = false;
            s_state.ringtone_type_known = false;
            s_state.ringtone_type = BT_RINGTONE_TYPE_LOCAL;
            if (s_state.active_call_seen) {
                BtCallRecord record;

                memset(&record, 0, sizeof(record));
                record.type = s_state.active_call_type;
                if (record.type == BT_CALL_RECORD_INCOMING && !s_state.active_call_was_answered) {
                    record.type = BT_CALL_RECORD_MISSED;
                }
                if (record.type == BT_CALL_RECORD_INCOMING ||
                    record.type == BT_CALL_RECORD_OUTGOING ||
                    record.type == BT_CALL_RECORD_MISSED) {
                    bt_copy_string(record.number, sizeof(record.number), s_state.active_number);
                    bt_copy_string(record.name, sizeof(record.name),
                                   s_state.active_contact_name[0] != '\0' ? s_state.active_contact_name : "Unknown");
                    resolve_call_record_display_name_locked(&record);
                    format_current_call_time(record.time, sizeof(record.time));
                    if (!insert_call_record_front_locked(&record)) {
                        call_record_failed = true;
                    }
                }
            }
            s_state.active_number[0] = '\0';
            s_state.active_contact_name[0] = '\0';
            reset_active_call_tracking_locked();
            set_status_locked(call_record_failed ? "Call record memory allocation failed" : "Call idle");
        } else if (s_state.call_state == BT_APP_CALL_STATE_INCOMING) {
            s_state.active_call_type = BT_CALL_RECORD_INCOMING;
            s_state.active_call_seen = true;
            if (s_state.active_number[0] == '\0') {
                bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name), "Unknown");
            }
            set_status_locked("Incoming call");
        } else if (s_state.call_state == BT_APP_CALL_STATE_SPEAKING) {
            s_state.ringtone_requested = false;
            s_state.ringtone_type_known = false;
            s_state.active_call_was_answered = true;
            if (s_state.active_number[0] == '\0') {
                bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name), "Unknown");
            }
            set_status_locked("Call speaking");
        } else if (s_state.call_state == BT_APP_CALL_STATE_HELD ||
                   s_state.call_state == BT_APP_CALL_STATE_WAITING ||
                   s_state.call_state == BT_APP_CALL_STATE_MULTI) {
            s_state.ringtone_requested = false;
            s_state.ringtone_type_known = false;
            if (s_state.call_state == BT_APP_CALL_STATE_WAITING) {
                s_state.active_call_type = BT_CALL_RECORD_INCOMING;
                s_state.active_call_seen = true;
            }
            if (s_state.active_number[0] == '\0') {
                bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name), "Unknown");
            }
            set_status_locked("Call held or waiting");
        } else {
            s_state.ringtone_requested = false;
            s_state.ringtone_type_known = false;
            s_state.active_call_type = BT_CALL_RECORD_OUTGOING;
            s_state.active_call_seen = true;
            if (s_state.active_number[0] == '\0') {
                bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name), "Unknown");
            }
            set_status_locked("Outgoing call");
        }
        break;
    case BT_EVENT_RINGTONE_TYPE:
        s_state.ringtone_type =
            event->state == BT_RINGTONE_TYPE_INBAND
                ? BT_RINGTONE_TYPE_INBAND
                : BT_RINGTONE_TYPE_LOCAL;
        s_state.ringtone_type_known = true;
        break;
    case BT_EVENT_RINGTONE:
        s_state.ringtone_requested = true;
        break;
    case BT_EVENT_CALL_AUDIO_STATE:
        s_state.call_audio_in_phone = event->value;
        set_status_locked(event->value ? "Call audio on phone" : "Call audio on car");
        break;
    case BT_EVENT_CALL_NUMBER:
        bt_copy_string(s_state.active_number, sizeof(s_state.active_number), event->number);
        {
            const char *contact_name = find_contact_name_locked(event->number);
            bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name),
                           (contact_name != NULL && contact_name[0] != '\0') ? contact_name : "Unknown");
            if (contact_name != NULL && contact_name[0] != '\0') {
                bt_copy_string(s_state.status, sizeof(s_state.status), contact_name);
            }
        }
        break;
    case BT_EVENT_MUSIC_PLAYING:
        s_state.music_playing = event->value;
        s_state.music_playing_state = event->value;
        set_status_locked(event->value ? "Music playing" : "Music paused");
        break;
    case BT_EVENT_MUSIC_POSITION:
        if (event->media_position[0] != '\0') {
            bt_copy_string(s_state.media_position, sizeof(s_state.media_position), event->media_position);
        }
        if (event->media_duration[0] != '\0') {
            bt_copy_string(s_state.media_duration, sizeof(s_state.media_duration), event->media_duration);
        }
        break;
    case BT_EVENT_MEDIA_INFO:
        bt_copy_string(s_state.media_title, sizeof(s_state.media_title),
                       event->media_title[0] != '\0' ? event->media_title : "Unknown");
        bt_copy_string(s_state.media_artist, sizeof(s_state.media_artist),
                       event->media_artist[0] != '\0' ? event->media_artist : "Unknown");
        bt_copy_string(s_state.media_album, sizeof(s_state.media_album),
                       event->media_album[0] != '\0' ? event->media_album : "Unknown");
        break;
    case BT_EVENT_PBAP_STATE:
        s_state.pbap_state = (BtPbapState)event->state;
        if (event->pbap_download_type != BT_PBAP_DOWNLOAD_NONE ||
            event->state == BT_PBAP_STATE_IDLE) {
            s_state.pbap_download_type = event->pbap_download_type;
        }
        if (event->pbap_current_index >= 0) {
            s_state.pbap_current_index = event->pbap_current_index;
        }
        if (event->pbap_total_count >= 0) {
            s_state.pbap_total_count = event->pbap_total_count;
        }
        /* Only bump UI versions on meaningful state transitions. Skipping
         * per-step DOWNLOADING bumps avoids hundreds of wasted redraws while
         * records stream in — the record events themselves update the versions. */
        if (event->state == BT_PBAP_STATE_IDLE ||
            event->state == BT_PBAP_STATE_FINISHED ||
            event->state == BT_PBAP_STATE_STOPPED ||
            event->state == BT_PBAP_STATE_ERROR) {
            s_state.phonebook_version++;
            s_state.call_record_version++;
        }
        set_status_locked(event->text);
        break;
    case BT_EVENT_PHONEBOOK_RESET:
        clear_phonebook_locked(false);
        s_state.phonebook_bulk_loading = true;
        s_state.phonebook_version++;
        set_status_locked("Phonebook cleared");
        break;
    case BT_EVENT_PHONEBOOK_RECORD:
        if (s_state.phone_connected) {
            bool appended = false;
            bool was_truncated = s_state.phonebook_truncated;

            if (!append_phonebook_contact_locked(&event->contact, &appended)) {
                set_status_locked("Out of memory loading contacts");
                break;
            }
            if (!appended) {
                if (!was_truncated) {
                    set_status_locked("Phonebook truncated to 500 records");
                }
                break;
            }
            /* Incrementally extend the filtered view; the full rebuild and call
             * record name sync are deferred to download completion to avoid
             * O(N^2) work while records stream in. */
            (void)append_phonebook_filter_match_locked(s_state.phonebook_count - 1U);
            if (!s_state.phonebook_bulk_loading) {
                s_state.phonebook_version++;
            }
        }
        break;
    case BT_EVENT_PHONEBOOK_COMPLETE:
        /* All records for this download have been applied. Do the deferred
         * one-shot full filter rebuild and call-record name sync once, instead
         * of per record. Rebuild reconciles the incrementally-built filter view
         * and recovers from any allocation failures that occurred mid-stream. */
        rebuild_phonebook_filter_locked(s_state.phonebook_filter_keyword);
        sync_call_record_names_locked();
        if (s_state.active_number[0] != '\0') {
            update_active_contact_name_locked();
        }
        s_state.phonebook_bulk_loading = false;
        s_state.phonebook_version++;
        s_state.call_record_version++;
        break;
    case BT_EVENT_CALL_RECORDS_RESET:
        clear_call_records_locked(false);
        s_state.call_records_bulk_loading = true;
        s_state.call_record_version++;
        set_status_locked("Call records cleared");
        break;
    case BT_EVENT_CALL_RECORD:
        {
            BtCallRecord record;

            if (!s_state.phone_connected) {
                break;
            }
            record = event->call_record;
            resolve_call_record_display_name_locked(&record);
            if (!insert_call_record_sorted_locked(&record)) {
                set_status_locked("Call record memory allocation failed");
            }
        }
        break;
    case BT_EVENT_CALL_RECORDS_COMPLETE:
        s_state.call_records_bulk_loading = false;
        rebuild_call_record_filter_locked(s_state.call_record_filter);
        s_state.call_record_version++;
        if (s_state.call_records_truncated) {
            set_status_locked("Call records truncated to 200 records");
        }
        break;
    case BT_EVENT_LOCAL_INFO:
        bt_copy_string(s_state.local_name, sizeof(s_state.local_name), event->local_name);
        bt_copy_string(s_state.pairing_pin, sizeof(s_state.pairing_pin), event->pairing_pin);
        set_status_locked("Bluetooth local information updated");
        break;
    case BT_EVENT_STATUS:
        set_status_locked(event->text);
        break;
    case BT_EVENT_NONE:
    default:
        break;
    }

    request_invalidate_locked();
}

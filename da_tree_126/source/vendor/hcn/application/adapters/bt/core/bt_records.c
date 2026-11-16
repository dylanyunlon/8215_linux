#include "bt_core_internal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BT_CALL_RECORD_INITIAL_CAPACITY 32U
#define BT_PHONEBOOK_INITIAL_CAPACITY 32U

void free_call_record_storage_locked(void)
{
    free(s_state.call_records);
    s_state.call_records = NULL;
    s_state.call_record_count = 0U;
    s_state.call_record_filtered_count = 0U;
    s_state.call_record_capacity = 0U;
    s_state.call_records_bulk_loading = false;
    s_state.call_records_truncated = false;
}

static bool ensure_call_record_capacity_locked(size_t needed)
{
    size_t old_capacity;
    size_t new_capacity;
    BtCallRecord *records;

    if (needed <= s_state.call_record_capacity) {
        return true;
    }

    old_capacity = s_state.call_record_capacity;
    new_capacity = old_capacity > 0U ? old_capacity : BT_CALL_RECORD_INITIAL_CAPACITY;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2U;
    }
    if (new_capacity > SIZE_MAX / sizeof(s_state.call_records[0])) {
        return false;
    }

    records = (BtCallRecord *)realloc(s_state.call_records, new_capacity * sizeof(s_state.call_records[0]));
    if (records == NULL) {
        return false;
    }
    s_state.call_records = records;

    if (new_capacity > old_capacity) {
        memset(&s_state.call_records[old_capacity], 0,
               (new_capacity - old_capacity) * sizeof(s_state.call_records[0]));
    }
    s_state.call_record_capacity = new_capacity;
    return true;
}

void clear_call_records_locked(bool release_storage)
{
    if (release_storage) {
        free_call_record_storage_locked();
        return;
    }

    if (s_state.call_records != NULL && s_state.call_record_capacity > 0U) {
        memset(s_state.call_records, 0,
               s_state.call_record_capacity * sizeof(s_state.call_records[0]));
    }
    s_state.call_record_count = 0U;
    s_state.call_record_filtered_count = 0U;
    s_state.call_records_bulk_loading = false;
    s_state.call_records_truncated = false;
}

void free_phonebook_storage_locked(void)
{
    free(s_state.phonebook);
    free(s_state.phonebook_filtered_index);
    s_state.phonebook = NULL;
    s_state.phonebook_filtered_index = NULL;
    s_state.phonebook_count = 0U;
    s_state.phonebook_capacity = 0U;
    s_state.phonebook_filtered_count = 0U;
    s_state.phonebook_filtered_capacity = 0U;
    s_state.phonebook_truncated = false;
    s_state.phonebook_bulk_loading = false;
    s_state.phonebook_filter_keyword[0] = '\0';
}

void clear_phonebook_locked(bool release_storage)
{
    if (release_storage) {
        free_phonebook_storage_locked();
        return;
    }

    s_state.phonebook_count = 0U;
    s_state.phonebook_filtered_count = 0U;
    s_state.phonebook_truncated = false;
    s_state.phonebook_bulk_loading = false;
    s_state.phonebook_filter_keyword[0] = '\0';
}

static bool ensure_phonebook_capacity_locked(size_t needed)
{
    size_t old_capacity;
    size_t new_capacity;
    BtContact *contacts;

    if (needed <= s_state.phonebook_capacity) {
        return true;
    }

    old_capacity = s_state.phonebook_capacity;
    new_capacity = old_capacity > 0U ? old_capacity : BT_PHONEBOOK_INITIAL_CAPACITY;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2U;
    }
    if (new_capacity > SIZE_MAX / sizeof(s_state.phonebook[0])) {
        return false;
    }

    contacts = (BtContact *)realloc(s_state.phonebook, new_capacity * sizeof(s_state.phonebook[0]));
    if (contacts == NULL) {
        return false;
    }
    s_state.phonebook = contacts;

    if (new_capacity > old_capacity) {
        memset(&s_state.phonebook[old_capacity], 0,
               (new_capacity - old_capacity) * sizeof(s_state.phonebook[0]));
    }
    s_state.phonebook_capacity = new_capacity;
    return true;
}

static bool ensure_phonebook_filter_capacity_locked(size_t needed)
{
    size_t old_capacity;
    size_t new_capacity;
    size_t *indices;

    if (needed <= s_state.phonebook_filtered_capacity) {
        return true;
    }

    old_capacity = s_state.phonebook_filtered_capacity;
    new_capacity = old_capacity > 0U ? old_capacity : BT_PHONEBOOK_INITIAL_CAPACITY;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2U;
    }
    if (new_capacity > SIZE_MAX / sizeof(s_state.phonebook_filtered_index[0])) {
        return false;
    }

    indices = (size_t *)realloc(s_state.phonebook_filtered_index,
                                new_capacity * sizeof(s_state.phonebook_filtered_index[0]));
    if (indices == NULL) {
        return false;
    }
    s_state.phonebook_filtered_index = indices;
    s_state.phonebook_filtered_capacity = new_capacity;
    return true;
}

bool append_phonebook_contact_locked(const BtContact *contact, bool *appended)
{
    if (contact == NULL) {
        return false;
    }
    if (appended != NULL) {
        *appended = false;
    }
    if (s_state.phonebook_count >= BT_PHONEBOOK_MAX_RECORDS) {
        s_state.phonebook_truncated = true;
        return true;
    }
    if (!ensure_phonebook_capacity_locked(s_state.phonebook_count + 1U)) {
        return false;
    }
    s_state.phonebook[s_state.phonebook_count] = *contact;
    s_state.phonebook_count++;
    if (appended != NULL) {
        *appended = true;
    }
    return true;
}

static bool string_contains_ignore_case(const char *text, const char *keyword)
{
    size_t text_len;
    size_t keyword_len;
    size_t i;

    if (keyword == NULL || keyword[0] == '\0') {
        return true;
    }
    if (text == NULL) {
        return false;
    }

    text_len = strlen(text);
    keyword_len = strlen(keyword);
    if (keyword_len > text_len) {
        return false;
    }

    for (i = 0U; i + keyword_len <= text_len; ++i) {
        size_t j;
        bool match = true;

        for (j = 0U; j < keyword_len; ++j) {
            char a = text[i + j];
            char b = keyword[j];
            if (a >= 'A' && a <= 'Z') {
                a = (char)(a - 'A' + 'a');
            }
            if (b >= 'A' && b <= 'Z') {
                b = (char)(b - 'A' + 'a');
            }
            if (a != b) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

void rebuild_phonebook_filter_locked(const char *keyword)
{
    size_t i;

    /* keyword may alias the stored keyword (e.g. the deferred rebuild at
     * download completion); only copy when it is a distinct buffer. */
    if (keyword != s_state.phonebook_filter_keyword) {
        bt_copy_string(s_state.phonebook_filter_keyword, sizeof(s_state.phonebook_filter_keyword),
                       keyword != NULL ? keyword : "");
    }

    s_state.phonebook_filtered_count = 0U;
    if (s_state.phonebook == NULL) {
        return;
    }
    if (!ensure_phonebook_filter_capacity_locked(s_state.phonebook_count)) {
        /* Fall back to whatever capacity we already have; still bounded below. */
        if (s_state.phonebook_filtered_capacity == 0U) {
            return;
        }
    }
    for (i = 0U; i < s_state.phonebook_count; ++i) {
        if (string_contains_ignore_case(s_state.phonebook[i].name, keyword) ||
            string_contains_ignore_case(s_state.phonebook[i].number, keyword)) {
            if (s_state.phonebook_filtered_count >= s_state.phonebook_filtered_capacity) {
                break;
            }
            s_state.phonebook_filtered_index[s_state.phonebook_filtered_count++] = i;
        }
    }
}

bool append_phonebook_filter_match_locked(size_t contact_index)
{
    const char *keyword;
    const BtContact *contact;

    if (contact_index >= s_state.phonebook_count || s_state.phonebook == NULL) {
        return true;
    }

    keyword = s_state.phonebook_filter_keyword;
    contact = &s_state.phonebook[contact_index];
    if (!string_contains_ignore_case(contact->name, keyword) &&
        !string_contains_ignore_case(contact->number, keyword)) {
        return true;
    }

    if (!ensure_phonebook_filter_capacity_locked(s_state.phonebook_filtered_count + 1U)) {
        return false;
    }
    s_state.phonebook_filtered_index[s_state.phonebook_filtered_count++] = contact_index;
    return true;
}

bool call_record_matches_filter(const BtCallRecord *record, BtCallRecordType filter)
{
    if (record == NULL) {
        return false;
    }
    return filter == BT_CALL_RECORD_ALL || record->type == filter;
}

void rebuild_call_record_filter_locked(BtCallRecordType filter)
{
    size_t i;

    s_state.call_record_filter = filter;
    s_state.call_record_filtered_count = 0U;
    if (s_state.call_record_count > 0U && s_state.call_records == NULL) {
        return;
    }
    for (i = 0U; i < s_state.call_record_count; ++i) {
        if (call_record_matches_filter(&s_state.call_records[i], filter)) {
            s_state.call_record_filtered_count++;
        }
    }
}

static bool call_record_time_has_minute(const char *time_text)
{
    size_t len;

    if (time_text == NULL) {
        return false;
    }

    len = strlen(time_text);
    return len >= 16U &&
           time_text[0] >= '0' && time_text[0] <= '9' &&
           time_text[1] >= '0' && time_text[1] <= '9' &&
           time_text[2] >= '0' && time_text[2] <= '9' &&
           time_text[3] >= '0' && time_text[3] <= '9' &&
           time_text[4] == '-' &&
           time_text[5] >= '0' && time_text[5] <= '9' &&
           time_text[6] >= '0' && time_text[6] <= '9' &&
           time_text[7] == '-' &&
           time_text[8] >= '0' && time_text[8] <= '9' &&
           time_text[9] >= '0' && time_text[9] <= '9' &&
           time_text[10] == ' ' &&
           time_text[11] >= '0' && time_text[11] <= '9' &&
           time_text[12] >= '0' && time_text[12] <= '9' &&
           time_text[13] == ':' &&
           time_text[14] >= '0' && time_text[14] <= '9' &&
           time_text[15] >= '0' && time_text[15] <= '9';
}

static bool call_record_exists_locked(const BtCallRecord *record)
{
    size_t i;

    if (record == NULL || record->number[0] == '\0') {
        return false;
    }

    for (i = 0U; i < s_state.call_record_count; ++i) {
        const BtCallRecord *existing = &s_state.call_records[i];

        if (existing->type != record->type || strcmp(existing->number, record->number) != 0) {
            continue;
        }
        if (strcmp(existing->time, record->time) == 0) {
            return true;
        }
        if (call_record_time_has_minute(existing->time) &&
            call_record_time_has_minute(record->time) &&
            strncmp(existing->time, record->time, 16U) == 0) {
            return true;
        }
    }

    return false;
}

static bool call_record_should_precede(const BtCallRecord *record, const BtCallRecord *existing)
{
    bool record_has_time;
    bool existing_has_time;

    if (record == NULL || existing == NULL) {
        return false;
    }

    record_has_time = call_record_time_has_minute(record->time);
    existing_has_time = call_record_time_has_minute(existing->time);
    if (record_has_time && !existing_has_time) {
        return true;
    }
    if (!record_has_time || !existing_has_time) {
        return false;
    }

    return strcmp(record->time, existing->time) > 0;
}

static void trim_call_records_to_max_locked(void)
{
    if (s_state.call_record_count <= BT_CALL_RECORD_MAX_RECORDS) {
        return;
    }

    s_state.call_records_truncated = true;
    if (s_state.call_records != NULL) {
        memset(&s_state.call_records[BT_CALL_RECORD_MAX_RECORDS], 0,
               (s_state.call_record_count - BT_CALL_RECORD_MAX_RECORDS) *
               sizeof(s_state.call_records[0]));
    }
    s_state.call_record_count = BT_CALL_RECORD_MAX_RECORDS;
}

bool insert_call_record_sorted_locked(const BtCallRecord *record)
{
    size_t index;

    if (record == NULL) {
        return false;
    }
    if (call_record_exists_locked(record)) {
        return true;
    }
    if (!ensure_call_record_capacity_locked(s_state.call_record_count + 1U)) {
        return false;
    }

    index = 0U;
    while (index < s_state.call_record_count &&
           !call_record_should_precede(record, &s_state.call_records[index])) {
        index++;
    }

    if (index < s_state.call_record_count) {
        memmove(&s_state.call_records[index + 1U],
                &s_state.call_records[index],
                (s_state.call_record_count - index) * sizeof(s_state.call_records[0]));
    }
    s_state.call_records[index] = *record;
    s_state.call_record_count++;
    trim_call_records_to_max_locked();
    if (!s_state.call_records_bulk_loading) {
        rebuild_call_record_filter_locked(s_state.call_record_filter);
        s_state.call_record_version++;
    }
    return true;
}

bool insert_call_record_front_locked(const BtCallRecord *record)
{
    if (record == NULL) {
        return false;
    }
    if (call_record_exists_locked(record)) {
        return true;
    }
    if (!ensure_call_record_capacity_locked(s_state.call_record_count + 1U)) {
        return false;
    }

    if (s_state.call_record_count > 0U) {
        memmove(&s_state.call_records[1],
                &s_state.call_records[0],
                s_state.call_record_count * sizeof(s_state.call_records[0]));
    }
    s_state.call_records[0] = *record;
    s_state.call_record_count++;
    trim_call_records_to_max_locked();
    rebuild_call_record_filter_locked(s_state.call_record_filter);
    s_state.call_record_version++;
    return true;
}

void format_current_call_time(char *dst, size_t dst_size)
{
    time_t now;
    struct tm time_info;
    struct tm *time_result;

    if (dst == NULL || dst_size == 0U) {
        return;
    }
    dst[0] = '\0';

    now = time(NULL);
    if (now == (time_t)-1) {
        return;
    }

#if defined(_WIN32)
    time_result = localtime_s(&time_info, &now) == 0 ? &time_info : NULL;
#else
    time_result = localtime_r(&now, &time_info);
#endif
    if (time_result == NULL) {
        return;
    }

    (void)strftime(dst, dst_size, "%Y-%m-%d %H:%M", time_result);
}

const char *find_contact_name_locked(const char *number)
{
    size_t i;

    if (number == NULL || number[0] == '\0') {
        return NULL;
    }
    for (i = 0U; i < s_state.phonebook_count; ++i) {
        if (strcmp(s_state.phonebook[i].number, number) == 0) {
            return s_state.phonebook[i].name;
        }
    }
    return NULL;
}

void update_active_contact_name_locked(void)
{
    const char *contact_name = find_contact_name_locked(s_state.active_number);

    bt_copy_string(s_state.active_contact_name, sizeof(s_state.active_contact_name),
                   (contact_name != NULL && contact_name[0] != '\0') ? contact_name : "Unknown");
}

void sync_call_record_names_locked(void)
{
    size_t i;
    bool changed = false;

    for (i = 0U; i < s_state.call_record_count; ++i) {
        const char *contact_name;

        if (s_state.call_records[i].number[0] == '\0') {
            continue;
        }

        contact_name = find_contact_name_locked(s_state.call_records[i].number);
        if (contact_name != NULL && contact_name[0] != '\0' &&
            strcmp(s_state.call_records[i].name, contact_name) != 0) {
            bt_copy_string(s_state.call_records[i].name, sizeof(s_state.call_records[i].name), contact_name);
            changed = true;
        }
    }
    rebuild_call_record_filter_locked(s_state.call_record_filter);
    if (changed) {
        s_state.call_record_version++;
    }
}

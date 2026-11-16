#ifndef BT_CORE_INTERNAL_H
#define BT_CORE_INTERNAL_H

#include "bt_core.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

extern pthread_mutex_t s_state_mutex;
extern BtState s_state;
extern BtAppUiHooks s_hooks;
extern bool s_pending_invalidate;
extern size_t s_seen_event_drop_count;

#define BT_AUTO_CONNECT_POWER_ON_DELAY_MS 2000U

/* CALLER MUST HOLD s_state_mutex. */
void free_call_record_storage_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void clear_call_records_locked(bool release_storage);
/* CALLER MUST HOLD s_state_mutex. */
void free_phonebook_storage_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void clear_phonebook_locked(bool release_storage);
/* CALLER MUST HOLD s_state_mutex. Returns false on allocation failure. */
bool append_phonebook_contact_locked(const BtContact *contact, bool *appended);
/* CALLER MUST HOLD s_state_mutex. Incrementally add one contact index to the
 * filtered view if it matches the active keyword. Returns false on alloc fail. */
bool append_phonebook_filter_match_locked(size_t contact_index);
/* CALLER MUST HOLD s_state_mutex. */
void reset_active_call_tracking_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void queue_auto_connect_locked(unsigned int delay_ms);
/* CALLER MUST HOLD s_state_mutex. */
void set_status_locked(const char *status);
/* CALLER MUST HOLD s_state_mutex. */
void request_invalidate_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void rebuild_phonebook_filter_locked(const char *keyword);
/* CALLER MUST HOLD s_state_mutex. */
bool call_record_matches_filter(const BtCallRecord *record, BtCallRecordType filter);
/* CALLER MUST HOLD s_state_mutex. */
void rebuild_call_record_filter_locked(BtCallRecordType filter);
/* CALLER MUST HOLD s_state_mutex. */
bool insert_call_record_sorted_locked(const BtCallRecord *record);
/* CALLER MUST HOLD s_state_mutex. */
bool insert_call_record_front_locked(const BtCallRecord *record);
void format_current_call_time(char *dst, size_t dst_size);
/* CALLER MUST HOLD s_state_mutex. */
const char *find_contact_name_locked(const char *number);
/* CALLER MUST HOLD s_state_mutex. */
void update_active_contact_name_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void sync_call_record_names_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void save_config_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void load_config_locked(void);
/* CALLER MUST HOLD s_state_mutex. */
void apply_event_locked(const BtEvent *event);

#ifdef __cplusplus
}
#endif

#endif

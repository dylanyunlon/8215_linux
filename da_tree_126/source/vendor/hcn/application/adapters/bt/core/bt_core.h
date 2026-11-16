#ifndef BT_CORE_H
#define BT_CORE_H

#include "bt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int bt_core_init(const BtAppConfig *config);
void bt_core_deinit(void);
void bt_core_poll(void);
void bt_core_set_ui_hooks(const BtAppUiHooks *hooks);
void bt_core_get_snapshot(BtAppSnapshot *snapshot);
void bt_core_notify_status(const char *status);
void bt_core_enter_main_page(void);
void bt_core_go_home(void);
void bt_core_set_auto_connect(bool enabled);
bool bt_core_should_auto_connect(char *address, size_t address_size);
bool bt_core_pending_auto_connect(char *address, size_t address_size);
void bt_core_notify_manual_disconnect(void);
bool bt_core_get_contact(size_t index, BtContact *out);
bool bt_core_get_call_record(size_t index, BtCallRecordType filter, BtCallRecord *out);
void bt_core_search_phonebook(const char *keyword);
void bt_core_filter_call_records(BtCallRecordType type);

#ifdef __cplusplus
}
#endif

#endif

#ifndef BT_PBAP_H
#define BT_PBAP_H

int bt_pbap_init(void);
void bt_pbap_deinit(void);
void bt_pbap_download_phonebook(void);
void bt_pbap_download_call_records(void);
void bt_pbap_abort_download(void);

#endif

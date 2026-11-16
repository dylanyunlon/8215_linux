#ifndef BT_GAP_H
#define BT_GAP_H

#include <stdbool.h>

int bt_gap_init(void);
void bt_gap_deinit(void);
void bt_gap_set_power(bool enabled);
void bt_gap_start_scan(void);
void bt_gap_stop_scan(void);
void bt_gap_pair_device(const char *address);
void bt_gap_remove_device(const char *address);

#endif

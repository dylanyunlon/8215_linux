#ifndef BT_AVRCP_H
#define BT_AVRCP_H

#include <stdbool.h>

int bt_avrcp_init(void);
void bt_avrcp_deinit(void);
void bt_avrcp_play_pause(bool currently_playing);
void bt_avrcp_pause(void);
void bt_avrcp_prev(void);
void bt_avrcp_next(void);

#endif

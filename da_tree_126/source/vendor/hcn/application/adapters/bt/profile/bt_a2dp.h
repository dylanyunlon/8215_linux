#ifndef BT_A2DP_H
#define BT_A2DP_H

#include <stdbool.h>

int bt_a2dp_init(void);
void bt_a2dp_deinit(void);
void bt_a2dp_set_audio_enabled(bool enabled);

#endif

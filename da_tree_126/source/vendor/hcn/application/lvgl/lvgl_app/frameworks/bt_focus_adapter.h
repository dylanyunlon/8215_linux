/**
 * @file bt_focus_adapter.h
 * @brief Bluetooth adapter for audio and video focus managers
 */

#ifndef BT_FOCUS_ADAPTER_H
#define BT_FOCUS_ADAPTER_H

#include "app_common.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_AUDIO_FOCUS_ID APP_BLUETOOTH

int bt_focus_adapter_init(void);
void bt_focus_adapter_deinit(void);
void bt_focus_adapter_poll(void);
void bt_focus_adapter_sync(void);
void bt_focus_adapter_set_bluetooth_music_page_selected(bool selected);

#ifdef __cplusplus
}
#endif

#endif /* BT_FOCUS_ADAPTER_H */

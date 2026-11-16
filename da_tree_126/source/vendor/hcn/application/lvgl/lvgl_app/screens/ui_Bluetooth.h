#ifndef UI_BLUETOOTH_H
#define UI_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

extern lv_obj_t * ui_Bluetooth;

void ui_Bluetooth_screen_init(lv_obj_t **screen);
void ui_Bluetooth_screen_destroy(void);
void ui_Bluetooth_screen_load(void);
void ui_Bluetooth_refresh(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef __CARPLAYAPP_UI_H__
#define __CARPLAYAPP_UI_H__

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_CarPlayApp_screen_init(lv_obj_t **screen);
void ui_CarPlayApp_screen_destroy();
void carplayapp_touch_event_handler(lv_event_t *e);

void carplayapp_show_first_connect_popup(char *device_name, char *device_id);
void carplayapp_show_passive_connect_popup(char *device_name, char *device_id);
void carplayapp_show_connect_fail_popup(const char *reason_text);

#ifdef __cplusplus
}
#endif

#endif
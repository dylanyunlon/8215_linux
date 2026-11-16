#ifndef __CARPLAYSETTING_UI_H__
#define __CARPLAYSETTING_UI_H__

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_CarPlaySetting_screen_init(lv_obj_t **screen);
void ui_CarPlaySetting_screen_destroy();

void carplaysetting_add_bonded_device(const char *name, const char *id);
void carplaysetting_remove_bonded_device(const char *id);
void carplaysetting_clear_bonded_devices();
void carplaysetting_update_bonded_device_status(const char *id, int status);
void carplaysetting_update_bonded_device_name(const char *id, const char *name);

void carplaysetting_add_available_device(const char *name, const char *id);
void carplaysetting_update_available_device_status(const char *id);
void carplaysetting_remove_available_device(const char *id);
void carplaysetting_clear_available_devices();

void carplaysetting_switch_on_off(int on);
void carplaysetting_discoverable_on_of(int on);

#ifdef __cplusplus
}
#endif

#endif
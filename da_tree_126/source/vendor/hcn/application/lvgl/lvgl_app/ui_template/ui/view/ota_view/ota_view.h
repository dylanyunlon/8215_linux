#ifndef OTA_VIEW_H__
#define OTA_VIEW_H__

#include "ui_init.h"
#include "aic_ui.h"

void ota_view_refresh_uuid(const char *str);

void ota_view_refresh_bluetooth(const char *str);

void ota_view_refresh_ota_passworld(const char *str);

void ota_view_refresh_sn(const char *str);

void ota_view_refresh_version(const char *str);

void ota_view_refresh_ota_state(const char *str );

void ota_view_set_progress(uint8_t percent);

void ota_view_update_status(const char *str);

void ota_view_update_error(const char *str);

#endif

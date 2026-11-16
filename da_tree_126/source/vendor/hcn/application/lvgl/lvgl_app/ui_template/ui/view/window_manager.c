/**
 * @file window_manager.c
 * @brief Compatibility facade from the template navigation API to displayfocusmanager.
 */

#include "window_manager.h"
#include "displayfocusmanager.h"

static app_id_t window_to_app(window_e window)
{
    switch (window) {
    case WINDOW_MAIN:   return APP_HOME;
    case WINDOW_MENU:   return APP_SETTINGS;
    case WINDOW_MOBILE: return APP_PHONE;
    case WINDOW_OTA:    return APP_OTA;
    case WINDOW_RADIO:  return APP_RADIO;
    case WINDOW_DVR:    return APP_DVR;
    default:            return APP_NONE;
    }
}

static window_e app_to_window(app_id_t app_id)
{
    switch (app_id) {
    case APP_HOME:     return WINDOW_MAIN;
    case APP_SETTINGS: return WINDOW_MENU;
    case APP_PHONE:    return WINDOW_MOBILE;
    case APP_OTA:      return WINDOW_OTA;
    case APP_RADIO:    return WINDOW_RADIO;
    case APP_DVR:      return WINDOW_DVR;
    default:           return WINDOW_NULL;
    }
}

bool window_manager_is_active(window_e window_id)
{
    return window_manager_get() == window_id;
}

void window_manager_set_target(window_e new_window)
{
    app_id_t app_id = window_to_app(new_window);

    if (app_id == APP_NONE) {
        return;
    }
    if (app_id == APP_HOME) {
        displayfocusmanager_go_home();
    } else {
        displayfocusmanager_open_app(app_id);
    }
}

window_e window_manager_get(void)
{
    return app_to_window(displayfocusmanager_get_current_app());
}

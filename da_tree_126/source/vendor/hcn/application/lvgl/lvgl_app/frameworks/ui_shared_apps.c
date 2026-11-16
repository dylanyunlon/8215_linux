/**
 * @file ui_shared_apps.c
 * @brief Registration and lifecycle for apps shared by both UI variants.
 */

#include "ui_shared_apps.h"
#include "displayfocusmanager.h"

#ifdef HCN_APP_HAVE_BT
#include "bt_app.h"
#include "frameworks/bt_focus_adapter.h"
#include "screens/ui_Bluetooth.h"
#endif

#ifdef LVGL_BUILD_CARPLAY
#include "carplayapp_if.h"
#include "carplayapp_ui.h"
#include "carplaysetting_ui.h"
#endif

#ifdef HCN_APP_HAVE_MUSIC
#include "music_adapter.h"
#include "frameworks/music_focus.h"
#include "screens/ui_Music.h"
#include <stdio.h>
#endif

#ifdef HCN_APP_HAVE_BT
static void shared_bt_enter_main_page(void *user_data)
{
    (void)user_data;
    displayfocusmanager_open_app(APP_BLUETOOTH);
}

static void shared_bt_go_home(void *user_data)
{
    (void)user_data;
    displayfocusmanager_go_home();
}

static void shared_bt_invalidate(void *user_data)
{
    (void)user_data;
    bt_focus_adapter_sync();
    ui_Bluetooth_refresh();
}

static void shared_bt_on_enter(void)
{
    bt_app_init_async(NULL);
}
#endif

int hcn_ui_shared_apps_register(void)
{
    int ret = APP_OK;

#ifdef HCN_APP_HAVE_MUSIC
    const app_desc_t music_desc = {
        .app_id = APP_MUSIC,
        .priority = APP_PRIORITY_LOW,
        .name = "Music",
        .init_cb = ui_Music_screen_init,
        .destroy_cb = ui_Music_screen_destroy,
        .screen_policy = APP_SCREEN_PERSISTENT,
        .on_enter = ui_Music_refresh,
    };
    ret = displayfocusmanager_register_app(&music_desc);
    if (ret != APP_OK) {
        return ret;
    }
#endif

#ifdef HCN_APP_HAVE_BT
    const app_desc_t bluetooth_desc = {
        .app_id = APP_BLUETOOTH,
        .priority = APP_PRIORITY_LOW,
        .name = "Bluetooth",
        .init_cb = ui_Bluetooth_screen_init,
        .destroy_cb = ui_Bluetooth_screen_destroy,
        .screen_policy = APP_SCREEN_PERSISTENT,
        .on_enter = shared_bt_on_enter,
    };
    ret = displayfocusmanager_register_app(&bluetooth_desc);
    if (ret != APP_OK) {
        return ret;
    }
#endif

#ifdef LVGL_BUILD_CARPLAY
    const app_desc_t carplay_desc = {
        .app_id = APP_CARPLAY,
        .priority = APP_PRIORITY_LOW,
        .name = "CarPlayApp",
        .init_cb = ui_CarPlayApp_screen_init,
        .destroy_cb = ui_CarPlayApp_screen_destroy,
        .screen_policy = APP_SCREEN_PERSISTENT,
    };
    ret = displayfocusmanager_register_app(&carplay_desc);
    if (ret != APP_OK) {
        return ret;
    }

    const app_desc_t carplay_settings_desc = {
        .app_id = APP_CARPLAY_SETTINGS,
        .priority = APP_PRIORITY_LOW,
        .name = "CarPlaySetting",
        .init_cb = ui_CarPlaySetting_screen_init,
        .destroy_cb = ui_CarPlaySetting_screen_destroy,
        .screen_policy = APP_SCREEN_PERSISTENT,
    };
    ret = displayfocusmanager_register_app(&carplay_settings_desc);
    if (ret != APP_OK) {
        return ret;
    }
#endif

    return ret;
}

int hcn_ui_shared_apps_init(void)
{
#ifdef HCN_APP_HAVE_BT
    BtAppUiHooks hooks = {
        .enter_main_page = shared_bt_enter_main_page,
        .go_home = shared_bt_go_home,
        .invalidate = shared_bt_invalidate,
        .user_data = NULL,
    };

    bt_app_set_ui_hooks(&hooks);
    bt_focus_adapter_init();
#endif
#ifdef HCN_APP_HAVE_MUSIC
    /* hooks run on the GUI thread, in this order: audio focus first, then the screen */
    music_focus_init();
    music_adapter_add_event_hook(music_focus_on_app_event);
    music_adapter_add_event_hook(ui_Music_on_app_event);
    if (music_adapter_init() != 0) {
        fprintf(stderr, "[ui] music adapter init failed, music screen stays empty\n");
    }
#endif
    return APP_OK;
}

void hcn_ui_shared_apps_open_bluetooth(void)
{
#ifdef HCN_APP_HAVE_BT
    bt_app_init_async(NULL);
    displayfocusmanager_open_app(APP_BLUETOOTH);
#endif
}

void hcn_ui_shared_apps_open_carplay(void)
{
#ifdef LVGL_BUILD_CARPLAY
    CarplayApp *app = carplayapp_get_instance();

    if (app == NULL) {
        lv_log("%s carplay instance is NULL", __func__);
        return;
    }

    if (carplayapp_is_connected(app) && !carplayapp_is_show_limit(app)) {
        if (displayfocusmanager_open_app(APP_CARPLAY) == APP_OK) {
            carplayapp_request_ui(app);
        }
    } else {
        displayfocusmanager_open_app(APP_CARPLAY_SETTINGS);
    }
#endif
}

void hcn_ui_shared_apps_open_music(void)
{
#ifdef HCN_APP_HAVE_MUSIC
    displayfocusmanager_open_app(APP_MUSIC);
#endif
}

void hcn_ui_shared_apps_deinit(void)
{
#ifdef HCN_APP_HAVE_MUSIC
    music_adapter_deinit();
    music_adapter_clear_event_hooks();
    music_focus_deinit();
#endif
#ifdef HCN_APP_HAVE_BT
    bt_focus_adapter_deinit();
    bt_app_set_ui_hooks(NULL);
#endif
}

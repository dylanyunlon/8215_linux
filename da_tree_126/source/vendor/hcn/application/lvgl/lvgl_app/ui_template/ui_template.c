/**
 * @file ui_template.c
 * @brief Template UI registration and page lifecycle adapters.
 */

#include "ui_template.h"
#include "ui_init.h"
#include "displayfocusmanager.h"
#include "frameworks/ui_shared_apps.h"

#include <string.h>

#include "aic_ui.h"
#include "aic_lang.h"
#include "custom/custom.h"
#include "logic.h"
#include "vehicle_argument.h"
#include "vehicle_data.h"
#include "animation_ctrl.h"
#include "window_manager.h"
#include "setting_menu.h"
#include "link_logic.h"
#include "ota_logic.h"
#include "radio_logic.h"
#include "dvr_logic.h"

ui_manager_t ui_manager;

static void template_home_create(lv_obj_t **screen)
{
    home_page_create(&ui_manager);
    *screen = home_page_get(&ui_manager)->obj;
}

static void template_home_enter(void)
{
    global_refresh_unit(vehicle_get_param_unit());
    if (!vehicle_get_mirror_navigation()) {
        home_refresh_animation_state(true);
    }
}

static void template_home_leave(void)
{
    home_refresh_animation_state(false);
}

static void template_settings_create(lv_obj_t **screen)
{
    setting_page_create(&ui_manager);
    *screen = setting_page_get(&ui_manager)->obj;
}

static void template_phone_create(lv_obj_t **screen)
{
    phone_page_create(&ui_manager);
    *screen = phone_page_get(&ui_manager)->obj;
}

static void template_ota_create(lv_obj_t **screen)
{
    ota_page_create(&ui_manager);
    *screen = ota_page_get(&ui_manager)->obj;
}

static void template_radio_create(lv_obj_t **screen)
{
    radio_page_create(&ui_manager);
    *screen = radio_page_get(&ui_manager)->obj;
}

static void template_dvr_create(lv_obj_t **screen)
{
    dvr_page_create(&ui_manager);
    *screen = dvr_page_get(&ui_manager)->obj;
}

static void template_settings_destroy(void)
{
    setting_menu_destory();
    memset(setting_page_get(&ui_manager), 0, sizeof(setting_page_t));
}

static void template_phone_destroy(void)
{
    on_phone_destory();
    memset(phone_page_get(&ui_manager), 0, sizeof(phone_page_t));
}

static void template_ota_destroy(void)
{
    on_ota_page_destory();
    memset(ota_page_get(&ui_manager), 0, sizeof(ota_page_t));
}

static void template_radio_destroy(void)
{
    on_radio_page_destory();
    memset(radio_page_get(&ui_manager), 0, sizeof(radio_page_t));
}

static void template_dvr_destroy(void)
{
    on_dvr_page_destory();
    memset(dvr_page_get(&ui_manager), 0, sizeof(dvr_page_t));
}

int hcn_ui_template_init(void)
{
    const app_desc_t home_desc = {
        .app_id = APP_HOME,
        .priority = APP_PRIORITY_SYSTEM,
        .name = "TemplateHome",
        .init_cb = template_home_create,
        .destroy_cb = NULL,
        .screen_policy = APP_SCREEN_PERSISTENT,
        .on_enter = template_home_enter,
        .on_leave = template_home_leave,
    };
    const app_desc_t settings_desc = {
        .app_id = APP_SETTINGS,
        .priority = APP_PRIORITY_LOW,
        .name = "TemplateSettings",
        .init_cb = template_settings_create,
        .destroy_cb = template_settings_destroy,
        .screen_policy = APP_SCREEN_TRANSIENT,
    };
    const app_desc_t phone_desc = {
        .app_id = APP_PHONE,
        .priority = APP_PRIORITY_LOW,
        .name = "TemplatePhone",
        .init_cb = template_phone_create,
        .destroy_cb = template_phone_destroy,
        .screen_policy = APP_SCREEN_TRANSIENT,
    };
    const app_desc_t ota_desc = {
        .app_id = APP_OTA,
        .priority = APP_PRIORITY_LOW,
        .name = "TemplateOTA",
        .init_cb = template_ota_create,
        .destroy_cb = template_ota_destroy,
        .screen_policy = APP_SCREEN_TRANSIENT,
    };
    const app_desc_t radio_desc = {
        .app_id = APP_RADIO,
        .priority = APP_PRIORITY_LOW,
        .name = "TemplateRadio",
        .init_cb = template_radio_create,
        .destroy_cb = template_radio_destroy,
        .screen_policy = APP_SCREEN_TRANSIENT,
    };
    const app_desc_t dvr_desc = {
        .app_id = APP_DVR,
        .priority = APP_PRIORITY_LOW,
        .name = "TemplateDVR",
        .init_cb = template_dvr_create,
        .destroy_cb = template_dvr_destroy,
        .screen_policy = APP_SCREEN_TRANSIENT,
    };

    aic_font_list_init();
    aic_set_language((lang_type_t)vehicle_get_param_language());
    ui_manager_init(&ui_manager, false);

    if (displayfocusmanager_register_app(&home_desc) != APP_OK ||
        displayfocusmanager_register_app(&settings_desc) != APP_OK ||
        displayfocusmanager_register_app(&phone_desc) != APP_OK ||
        displayfocusmanager_register_app(&ota_desc) != APP_OK ||
        displayfocusmanager_register_app(&radio_desc) != APP_OK ||
        displayfocusmanager_register_app(&dvr_desc) != APP_OK ||
        hcn_ui_shared_apps_register() != APP_OK) {
        return -1;
    }

    if (displayfocusmanager_open_app(APP_HOME) != APP_OK) {
        return -1;
    }

    custom_init();
    return 0;
}

void hcn_ui_template_deinit(void)
{
    memset(&ui_manager, 0, sizeof(ui_manager));
}

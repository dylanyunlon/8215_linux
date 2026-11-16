#include "carplaysetting_ui.h"
#include "carplayapp_if.h"

#include <string.h>
#include "videofocusmanager.h"


static lv_obj_t *carplaysetting_screen = NULL;
static lv_obj_t *bonded_devices_list = NULL;
static lv_obj_t *available_devices_list = NULL;
static lv_obj_t *popup_bg = NULL;
static lv_obj_t *popup_name_label = NULL;
static lv_obj_t *popup_disconnect_btn = NULL;
static lv_obj_t *popup_forget_btn = NULL;
static lv_obj_t *long_pressed_device = NULL;
static lv_obj_t *switch_btn = NULL;
static lv_obj_t *discoverable_btn = NULL;

static bool same_device(lv_obj_t *list, const char *id);
static void popup_show_for_device(const char *id, const char *name, bool disconnected);

static void home_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_log("%s", __func__);
        videofocusmanager_open_app(APP_HOME);
        carplayapp_stop_video(carplayapp_get_instance());
    }
}

static void back_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_log("%s", __func__);
        videofocusmanager_close_app(APP_CARPLAY_SETTINGS);
        carplayapp_stop_video(carplayapp_get_instance());
    }
}

static void carplay_switch_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *sw = lv_event_get_target(e);
        if (lv_obj_get_state(sw) & LV_STATE_CHECKED) {
            lv_log("%s LV_STATE_CHECKED", __func__);
            carplayapp_enable(carplayapp_get_instance());
        } else {
            lv_log("%s", __func__);
            carplayapp_disable(carplayapp_get_instance());
        }
    }
}

static void discoverable_switch_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *sw = lv_event_get_target(e);
        if (lv_obj_get_state(sw) & LV_STATE_CHECKED) {
            lv_log("%s LV_STATE_CHECKED", __func__);
            carplayapp_enable_discoverable(carplayapp_get_instance());
        } else {
            lv_log("%s", __func__);
            carplayapp_disable_discoverable(carplayapp_get_instance());
        }
    }
}

static void available_device_short_press_handler(lv_event_t *e)
{
    lv_log("%s", __func__);
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_log("%s LV_EVENT_SHORT_CLICKED", __func__);
        lv_obj_t *btn = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(btn);
        if (id != NULL) {
            carplayapp_connect(carplayapp_get_instance(), id);

            uint32_t child_cnt = lv_obj_get_child_cnt(btn);
            for (uint32_t j = 0; j < child_cnt; j++) {
                lv_obj_t *child = lv_obj_get_child(btn, j);
                if (lv_obj_check_type(child, &lv_label_class)) {
                    lv_obj_t *label = child;
                    const char *label_text = lv_label_get_text(label);
                    if (label_text != NULL && lv_strcmp(label_text, lv_list_get_button_text(available_devices_list, btn)) != 0) {
                        lv_label_set_text(label, "Connecting");
                        lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
                        break;
                    }
                }
            }
        }
    }
}

static void available_device_long_press_handler(lv_event_t *e)
{
    lv_log("%s", __func__);
    if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
        long_pressed_device = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(long_pressed_device);
        if (id != NULL) {
            lv_log("%s - popup", __func__);
            const char *name = lv_list_get_button_text(available_devices_list, long_pressed_device);

            bool connected = carplayapp_id_is_connected(carplayapp_get_instance(), id);
            bool connecting = carplayapp_id_is_connecting(carplayapp_get_instance(), id);
            popup_show_for_device(id, name, !(connected || connecting));
        }
    }
}

static void bonded_device_short_press_handler(lv_event_t *e)
{
    lv_log("%s", __func__);
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_log("%s LV_EVENT_SHORT_CLICKED", __func__);
        lv_obj_t *btn = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(btn);
        if (id != NULL) {
            if (!carplayapp_id_is_connected(carplayapp_get_instance(), id)) {
                carplayapp_connect(carplayapp_get_instance(), id);
            }
        }
    }
}

static void bonded_device_long_press_handler(lv_event_t *e)
{
    lv_log("%s", __func__);
    if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
        long_pressed_device = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(long_pressed_device);
        if (id != NULL) {
            lv_log("%s - popup", __func__);
            const char *name = lv_list_get_button_text(bonded_devices_list, long_pressed_device);
            
            bool connected = carplayapp_id_is_connected(carplayapp_get_instance(), id);
            bool connecting = carplayapp_id_is_connecting(carplayapp_get_instance(), id);
            popup_show_for_device(id, name, !(connected || connecting));

        }
    }
}

static void popup_disconnect_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_obj_add_flag(popup_bg, LV_OBJ_FLAG_HIDDEN);
        char *id = (char *)lv_obj_get_user_data(popup_bg);
        if (id != NULL) {
            carplayapp_disconnect(carplayapp_get_instance(), id);
        }
    }
}

static void popup_forget_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_obj_add_flag(popup_bg, LV_OBJ_FLAG_HIDDEN);
        char *id = (char *)lv_obj_get_user_data(popup_bg);
        if (id != NULL) {
            carplayapp_forget(carplayapp_get_instance(), id);
            carplaysetting_remove_bonded_device(id);
        }
    }
}

static void popup_bg_click_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_obj_add_flag(popup_bg, LV_OBJ_FLAG_HIDDEN);
    }
}

static void popup_show_for_device(const char *id, const char *name, bool disconnected)
{
    if (id == NULL || name == NULL || popup_bg == NULL) {
        return;
    }

    if (popup_name_label != NULL) {
        lv_label_set_text(popup_name_label, name);
    }

    bool bonded = (bonded_devices_list != NULL) && same_device(bonded_devices_list, id);

    if (popup_disconnect_btn != NULL) {
        if (!disconnected) {
            lv_obj_remove_flag(popup_disconnect_btn, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(popup_disconnect_btn, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (popup_forget_btn != NULL) {
        if (bonded) {
            lv_obj_remove_flag(popup_forget_btn, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(popup_forget_btn, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* Reposition visible buttons so the popup looks balanced. */
    if (popup_disconnect_btn != NULL && popup_forget_btn != NULL) {
        if (!disconnected && bonded) {
            lv_obj_align(popup_disconnect_btn, LV_ALIGN_TOP_MID, 0, 45);
            lv_obj_align(popup_forget_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
        } else if (!disconnected) {
            lv_obj_align(popup_disconnect_btn, LV_ALIGN_CENTER, 0, 20);
        } else if (bonded) {
            lv_obj_align(popup_forget_btn, LV_ALIGN_CENTER, 0, 20);
        }
    }

    if (bonded || !disconnected) {
        lv_obj_remove_flag(popup_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(popup_bg);
        lv_obj_set_user_data(popup_bg, (void *)id);
    }
}

static void refresh_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED) {
        lv_log("%s", __func__);
        carplaysetting_clear_available_devices();
        carplayapp_fresh(carplayapp_get_instance());
    }
}

static lv_obj_t *create_popup(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_add_flag(bg, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_style_bg_opa(bg, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(bg, popup_bg_click_handler, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t *panel = lv_obj_create(bg);
    lv_obj_set_size(panel, 320, 220);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x17202A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(panel, popup_bg_click_handler, LV_EVENT_SHORT_CLICKED, NULL);

    popup_name_label = lv_label_create(panel);
    lv_label_set_text(popup_name_label, "");
    lv_obj_set_width(popup_name_label, 280);
    lv_label_set_long_mode(popup_name_label, LV_LABEL_LONG_DOT);
    lv_obj_align(popup_name_label, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(popup_name_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(popup_name_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *disconnect_btn = lv_button_create(panel);
    lv_obj_set_size(disconnect_btn, 220, 48);
    lv_obj_align(disconnect_btn, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_radius(disconnect_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *disconnect_label = lv_label_create(disconnect_btn);
    lv_label_set_text(disconnect_label, "Disconnect");
    lv_obj_center(disconnect_label);
    lv_obj_set_style_text_font(disconnect_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(disconnect_btn, popup_disconnect_handler, LV_EVENT_SHORT_CLICKED, NULL);
    popup_disconnect_btn = disconnect_btn;

    lv_obj_t *forget_btn = lv_button_create(panel);
    lv_obj_set_size(forget_btn, 220, 48);
    lv_obj_align(forget_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_radius(forget_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *forget_label = lv_label_create(forget_btn);
    lv_label_set_text(forget_label, "Forget");
    lv_obj_center(forget_label);
    lv_obj_set_style_text_font(forget_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(forget_btn, popup_forget_handler, LV_EVENT_SHORT_CLICKED, NULL);
    popup_forget_btn = forget_btn;

    lv_obj_add_flag(bg, LV_OBJ_FLAG_HIDDEN);

    return bg;
}

static lv_obj_t *create_switch_with_label(lv_obj_t *parent, const char *label_text, lv_event_cb_t event_cb)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, lv_pct(100), 50);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw = lv_switch_create(container);
    if (lv_strcmp(label_text, "CarPlay Switch") == 0) {
        switch_btn = sw;
        if (carplayapp_is_enabled(carplayapp_get_instance())) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        }
    } else if (lv_strcmp(label_text, "Discoverable") == 0) {
        discoverable_btn = sw;
        if (carplayapp_is_discoverable(carplayapp_get_instance())) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        }
    }
    lv_obj_add_event_cb(sw, event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return container;
}

static lv_obj_t *create_section_label(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    return label;
}

static void style_list(lv_obj_t *list)
{
    lv_obj_set_style_bg_opa(list, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x17202A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(list, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(list, lv_color_hex(0x3E5267), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(list, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void style_list_item(lv_obj_t *item)
{
    lv_obj_set_style_bg_opa(item, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(item, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(item, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_CarPlaySetting_screen_init(lv_obj_t **screen)
{
    *screen = lv_obj_create(NULL);
    carplaysetting_screen = *screen;
    lv_obj_set_size(*screen, 1024, 600);
    lv_obj_remove_flag(*screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(*screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(*screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(*screen, lv_color_hex(0x111820), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(*screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(*screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *topbar = lv_obj_create(*screen);
    lv_obj_set_size(topbar, lv_pct(100), 56);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(topbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(0x202832), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(topbar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(topbar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(topbar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(topbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *btn_home = lv_button_create(topbar);
    lv_obj_set_size(btn_home, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_home, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_home, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(btn_home, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(btn_home, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Home");
    lv_obj_center(label_home);
    lv_obj_set_style_text_font(label_home, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_home, home_event_handler, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t *title_label = lv_label_create(topbar);
    lv_label_set_text(title_label, "CarPlaySetting");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *btn_back = lv_button_create(topbar);
    lv_obj_set_size(btn_back, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_back, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_back, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(btn_back, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(btn_back, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_center(label_back);
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_back, back_event_handler, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t *content = lv_obj_create(*screen);
    lv_obj_set_width(content, lv_pct(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x111820), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    create_switch_with_label(content, "CarPlay Switch", carplay_switch_handler);

    create_switch_with_label(content, "Discoverable", discoverable_switch_handler);

    create_section_label(content, "BondedDevices");

    bonded_devices_list = lv_list_create(content);
    lv_obj_set_size(bonded_devices_list, lv_pct(100), 150);
    style_list(bonded_devices_list);

    create_section_label(content, "AvailableDevices");

    available_devices_list = lv_list_create(content);
    lv_obj_set_size(available_devices_list, lv_pct(100), 150);
    style_list(available_devices_list);

    lv_obj_t *btn_refresh = lv_button_create(content);
    lv_obj_set_size(btn_refresh, 150, 48);
    lv_obj_set_style_radius(btn_refresh, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_margin_top(btn_refresh, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_refresh = lv_label_create(btn_refresh);
    lv_label_set_text(label_refresh, "Refresh");
    lv_obj_center(label_refresh);
    lv_obj_set_style_text_font(label_refresh, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_refresh, refresh_handler, LV_EVENT_SHORT_CLICKED, NULL);

    popup_bg = create_popup(*screen);

    if (carplayapp_is_enabled(carplayapp_get_instance())) {
        carplayapp_update_list(carplayapp_get_instance());
    }
}

void ui_CarPlaySetting_screen_destroy(void)
{
    if (carplaysetting_screen != NULL) {
        carplaysetting_screen = NULL;
        bonded_devices_list = NULL;
        available_devices_list = NULL;
        popup_bg = NULL;
        popup_name_label = NULL;
        popup_disconnect_btn = NULL;
        popup_forget_btn = NULL;
        long_pressed_device = NULL;
        switch_btn = NULL;
        discoverable_btn = NULL;
    }
}

static void device_delete_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_DELETE) {
        lv_obj_t *item = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(item);
        if (id != NULL) {
            lv_free(id);
        }
    }
}

static bool same_device(lv_obj_t *list, const char *id)
{
    bool result = false;

    uint32_t n = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            result = true;
            break;
        }
    }

    return result;
}

void carplaysetting_add_bonded_device(const char *name, const char *id)
{
    lv_log("%s name: %s, id: %s", __func__, name, id);

    if (bonded_devices_list == NULL || name == NULL || id == NULL) {
        return;
    }

    if (same_device(bonded_devices_list, id)) {
        return;
    }

    lv_log("lv_list_add_button");

    lv_obj_t *btn = lv_list_add_button(bonded_devices_list, NULL, name);
    style_list_item(btn);

    lv_obj_t *status_label = lv_label_create(btn);
    lv_label_set_text(status_label, "");
    lv_obj_add_flag(status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(status_label, LV_ALIGN_RIGHT_MID, -10, 0);

    char *id_copy = lv_malloc(strlen(id) + 1);
    if (id_copy == NULL) {
        lv_log("%s id_copy is NULL", __func__);
    } else {
        strcpy(id_copy, id);
        lv_obj_set_user_data(btn, (void *)id_copy);
        lv_obj_add_event_cb(btn, device_delete_handler, LV_EVENT_DELETE, NULL);
    }

    lv_obj_add_event_cb(btn, bonded_device_short_press_handler, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, bonded_device_long_press_handler, LV_EVENT_LONG_PRESSED, NULL);
}

void carplaysetting_remove_bonded_device(const char *id)
{
    if (bonded_devices_list == NULL || id == NULL) {
        return;
    }

    uint32_t n = lv_obj_get_child_cnt(bonded_devices_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(bonded_devices_list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            lv_obj_del(btn);
            return;
        }
    }
}

void carplaysetting_update_bonded_device_status(const char *id, int status)
{
    if (bonded_devices_list == NULL || id == NULL) {
        return;
    }

    uint32_t n = lv_obj_get_child_cnt(bonded_devices_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(bonded_devices_list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            uint32_t child_cnt = lv_obj_get_child_cnt(btn);
            for (uint32_t j = 0; j < child_cnt; j++) {
                lv_obj_t *child = lv_obj_get_child(btn, j);
                if (lv_obj_check_type(child, &lv_label_class)) {
                    lv_obj_t *label = child;
                    const char *label_text = lv_label_get_text(label);
                    if (label_text != NULL && lv_strcmp(label_text, lv_list_get_button_text(bonded_devices_list, btn)) != 0) {
                        switch (status) {
                        case 0:
                            lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
                            break;
                        case 1:
                            lv_label_set_text(label, "Connecting");
                            lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
                            break;
                        case 2:
                        case 3:
                        case 4:
                            lv_label_set_text(label, "Connected");
                            lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
                            break;
                        case 5:
                            lv_label_set_text(label, "Disconnecting");
                            lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
                            break;
                        }
                        return;
                    }
                }
            }
            return;
        }
    }
}

void carplaysetting_update_bonded_device_name(const char *id, const char *name)
{
    if (bonded_devices_list == NULL || id == NULL || name == NULL) {
        return;
    }

    uint32_t n = lv_obj_get_child_cnt(bonded_devices_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(bonded_devices_list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            lv_list_set_button_text(bonded_devices_list, btn, name);
            return;
        }
    }
}

void carplaysetting_clear_bonded_devices()
{
    if (bonded_devices_list == NULL) {
        return;
    }
    lv_obj_clean(bonded_devices_list);
}

void carplaysetting_add_available_device(const char *name, const char *id)
{
    lv_log("%s name: %s, id: %s", __func__, name, id);
    if (available_devices_list == NULL || name == NULL || id == NULL) {
        return;
    }

    if (same_device(available_devices_list, id)) {
        return;
    }

    if (bonded_devices_list != NULL && same_device(bonded_devices_list, id)) {
        return;
    }

    lv_log("lv_list_add_button");
    lv_obj_t *btn = lv_list_add_button(available_devices_list, NULL, name);
    style_list_item(btn);

    lv_obj_t *status_label = lv_label_create(btn);
    lv_label_set_text(status_label, "");
    lv_obj_add_flag(status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(status_label, LV_ALIGN_RIGHT_MID, -10, 0);

    char *id_copy = lv_malloc(strlen(id) + 1);
    if (id_copy == NULL) {
        lv_log("%s id_copy is NULL", __func__);
    } else {
        strcpy(id_copy, id);
        lv_obj_set_user_data(btn, (void *)id_copy);
        lv_obj_add_event_cb(btn, device_delete_handler, LV_EVENT_DELETE, NULL);
    }

    lv_obj_add_event_cb(btn, available_device_short_press_handler, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, available_device_long_press_handler, LV_EVENT_LONG_PRESSED, NULL);
}

void carplaysetting_update_available_device_status(const char *id)
{
    if (available_devices_list == NULL || id == NULL) {
        return;
    }

    uint32_t n = lv_obj_get_child_cnt(available_devices_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(available_devices_list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            uint32_t child_cnt = lv_obj_get_child_cnt(btn);
            for (uint32_t j = 0; j < child_cnt; j++) {
                lv_obj_t *child = lv_obj_get_child(btn, j);
                if (lv_obj_check_type(child, &lv_label_class)) {
                    lv_obj_t *label = child;
                    const char *label_text = lv_label_get_text(label);
                    if (label_text != NULL && lv_strcmp(label_text, lv_list_get_button_text(available_devices_list, btn)) != 0) {
                        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
                        return;
                    }
                }
            }
            return;
        }
    }
}

void carplaysetting_remove_available_device(const char *id)
{
    if (available_devices_list == NULL || id == NULL) {
        return;
    }

    uint32_t n = lv_obj_get_child_cnt(available_devices_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(available_devices_list, i);
        const char *device_id = (const char *)lv_obj_get_user_data(btn);
        if (device_id != NULL && lv_strcmp(id, device_id) == 0) {
            lv_obj_del(btn);
            return;
        }
    }
}

void carplaysetting_clear_available_devices()
{
    if (available_devices_list == NULL) {
        return;
    }
    lv_obj_clean(available_devices_list);
}

void carplaysetting_switch_on_off(int on)
{
    lv_log("%s", __func__);
    if (switch_btn != NULL) {
        if (on == 1) {
            lv_log("%s LV_STATE_CHECKED", __func__);
            lv_obj_add_state(switch_btn, LV_STATE_CHECKED);
            carplaysetting_clear_bonded_devices();
            carplayapp_update_list(carplayapp_get_instance());
        } else if (on == 0) {
            lv_obj_clear_state(switch_btn, LV_STATE_CHECKED);
            carplaysetting_clear_bonded_devices(carplayapp_get_instance());
            carplaysetting_clear_available_devices(carplayapp_get_instance());
        }
    }
}

void carplaysetting_discoverable_on_of(int on)
{
    if (discoverable_btn != NULL) {
        if (on == 1) {
            lv_obj_add_state(discoverable_btn, LV_STATE_CHECKED);
        } else if (on == 0) {
            lv_obj_clear_state(discoverable_btn, LV_STATE_CHECKED);
        }
    }
}

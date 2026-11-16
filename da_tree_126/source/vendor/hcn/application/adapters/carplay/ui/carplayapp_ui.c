#include "carplayapp_ui.h"
#include "carplayapp_if.h"
#include <string.h>

#include "app_common.h"
#include "videofocusmanager.h"

static lv_obj_t *carplayapp_screen = NULL;
static lv_obj_t *first_connect_popup = NULL;
static lv_obj_t *passive_connect_popup = NULL;
static lv_obj_t *passive_connect_list = NULL;
static lv_obj_t *connect_fail_popup = NULL;

void carplayapp_touch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL) {
        return;
    }

    CarplayApp* app = carplayapp_get_instance();
    if (app == NULL) {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(indev, &point);

    switch (code) {
    case LV_EVENT_PRESSED:
        carplayapp_on_ui_touched(app, 1, point.x, point.y);
        break;
    case LV_EVENT_PRESSING:
        carplayapp_on_ui_touched(app, 1, point.x, point.y);
        break;
    case LV_EVENT_RELEASED:
        carplayapp_on_ui_touched(app, 0, point.x, point.y);
        break;
    default:
        break;
    }
}

void on_video_focus_changed(app_id_t app_id, bool focused, void *user_data)
{
    lv_log("%s, app_id: %d, focused: %d", __func__, app_id, focused);
    if (app_id != APP_CARPLAY) {
        return;
    }

    if (focused) {
        carplayapp_start_video(carplayapp_get_instance());
    } else {
        CarplayApp *app = carplayapp_get_instance();
        lv_log("%s: focus lost, wait for video stopped then release surface", __func__);
        carplayapp_wait_for_video_stopped(app);
        lv_log("%s: wait done, release surface", __func__);
        carplayapp_release_surface(app);
    }
}

void ui_CarPlayApp_screen_init(lv_obj_t **screen)
{
    lv_log("%s", __func__);
    *screen = lv_obj_create(NULL);
    carplayapp_screen = *screen;

    lv_obj_set_size(*screen, 1024, 600);
    lv_obj_set_style_bg_color(*screen, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_obj_set_style_bg_opa(*screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(*screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(*screen, carplayapp_touch_event_handler, LV_EVENT_ALL, NULL);
    videofocusmanager_register_focus_callback(APP_CARPLAY, on_video_focus_changed, NULL);
}

void ui_CarPlayApp_screen_destroy(void)
{
    lv_log("%s", __func__);

    if (first_connect_popup != NULL) {
        lv_obj_delete(first_connect_popup);
        first_connect_popup = NULL;
    }
    if (passive_connect_popup != NULL) {
        lv_obj_delete(passive_connect_popup);
        passive_connect_popup = NULL;
    }
    passive_connect_list = NULL;
    if (connect_fail_popup != NULL) {
        lv_obj_delete(connect_fail_popup);
        connect_fail_popup = NULL;
    }

    if (carplayapp_screen != NULL) {
        lv_obj_remove_event_cb(carplayapp_screen, carplayapp_touch_event_handler);
        carplayapp_screen = NULL;
        videofocusmanager_unregister_focus_callback(APP_CARPLAY);
    }
}

static void first_connect_popup_delete_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_DELETE) {
        lv_obj_t *bg = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(bg);
        if (id != NULL) {
            lv_free(id);
        }
    }
}

static void first_connect_popup_bg_click_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *bg = lv_event_get_target(e);
        const char *device_id = (const char *)lv_obj_get_user_data(bg);
        if (device_id != NULL) {
            carplayapp_disconnect(carplayapp_get_instance(), device_id);
        }
        lv_obj_del(first_connect_popup);
        first_connect_popup = NULL;
    }
}

static void first_confirm_btn_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *panel = lv_obj_get_parent(btn);
        lv_obj_t *bg = lv_obj_get_parent(panel);
        const char *device_id = (const char *)lv_obj_get_user_data(bg);
        if (device_id != NULL) {
            carplayapp_confirmFirstConnect(carplayapp_get_instance(), device_id);
        }
        lv_obj_del(first_connect_popup);
        first_connect_popup = NULL;
    }
}

static void first_disconnect_btn_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *panel = lv_obj_get_parent(btn);
        lv_obj_t *bg = lv_obj_get_parent(panel);
        const char *device_id = (const char *)lv_obj_get_user_data(bg);
        if (device_id != NULL) {
            carplayapp_disconnect(carplayapp_get_instance(), device_id);
        }
        lv_obj_del(first_connect_popup);
        first_connect_popup = NULL;
    }
}

void carplayapp_show_first_connect_popup(char *device_name, char *device_id)
{
    if (passive_connect_popup != NULL) {
        lv_obj_del(passive_connect_popup);
        passive_connect_popup = NULL;
        passive_connect_list = NULL;
    }

    if (first_connect_popup != NULL) {
        lv_obj_del(first_connect_popup);
        first_connect_popup = NULL;
    }

    lv_obj_t *bg = lv_obj_create(lv_layer_top());
    first_connect_popup = bg;
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(bg, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bg, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bg, first_connect_popup_bg_click_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(bg, first_connect_popup_delete_handler, LV_EVENT_DELETE, NULL);

    char *id_copy = lv_malloc(strlen(device_id) + 1);
    if (id_copy != NULL) {
        strcpy(id_copy, device_id);
        lv_obj_set_user_data(bg, (void *)id_copy);
    }

    lv_obj_t *panel = lv_obj_create(bg);
    lv_obj_set_size(panel, 320, 200);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *name_label = lv_label_create(panel);
    lv_label_set_text(name_label, device_name);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(name_label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *connect_btn = lv_button_create(panel);
    lv_obj_set_size(connect_btn, 220, 48);
    lv_obj_align(connect_btn, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_set_style_bg_color(connect_btn, lv_color_hex(0x278FCB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(connect_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(connect_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *connect_label = lv_label_create(connect_btn);
    lv_label_set_text(connect_label, "Confirm");
    lv_obj_center(connect_label);
    lv_obj_set_style_text_font(connect_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(connect_btn, first_confirm_btn_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *disconnect_btn = lv_button_create(panel);
    lv_obj_set_size(disconnect_btn, 220, 48);
    lv_obj_align(disconnect_btn, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(disconnect_btn, lv_color_hex(0x278FCB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(disconnect_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(disconnect_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *disconnect_label = lv_label_create(disconnect_btn);
    lv_label_set_text(disconnect_label, "Disconnect");
    lv_obj_center(disconnect_label);
    lv_obj_set_style_text_font(disconnect_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(disconnect_btn, first_disconnect_btn_handler, LV_EVENT_CLICKED, NULL);
}

static void passive_item_delete_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_DELETE) {
        lv_obj_t *item = lv_event_get_target(e);
        char *id = (char *)lv_obj_get_user_data(item);
        if (id != NULL) {
            lv_free(id);
        }
    }
}

static void passive_connect_popup_bg_click_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        uint32_t n = lv_obj_get_child_cnt(passive_connect_list);
        for (uint32_t i = 0; i < n; i++) {
            lv_obj_t *item = lv_obj_get_child(passive_connect_list, i);
            const char *device_id = (const char *)lv_obj_get_user_data(item);
            if (device_id != NULL) {
                carplayapp_disconnect(carplayapp_get_instance(), device_id);
            }
        }
        lv_obj_del(passive_connect_popup);
        passive_connect_popup = NULL;
        passive_connect_list = NULL;
    }
}

static void passive_connect_btn_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *btn_container = lv_obj_get_parent(btn);
        lv_obj_t *item = lv_obj_get_parent(btn_container);
        const char *device_id = (const char *)lv_obj_get_user_data(item);
        if (device_id != NULL) {
            carplayapp_connect(carplayapp_get_instance(), device_id);
        }
        lv_obj_del(passive_connect_popup);
        passive_connect_popup = NULL;
        passive_connect_list = NULL;
    }
}

static void passive_disconnect_btn_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *btn_container = lv_obj_get_parent(btn);
        lv_obj_t *item = lv_obj_get_parent(btn_container);
        const char *device_id = (const char *)lv_obj_get_user_data(item);
        if (device_id != NULL) {
            carplayapp_disconnect(carplayapp_get_instance(), device_id);
        }
        lv_obj_del(item);
    }
}

void carplayapp_show_passive_connect_popup(char *device_name, char *device_id)
{
    if (first_connect_popup != NULL) {
        lv_obj_del(first_connect_popup);
        first_connect_popup = NULL;
    }

    if (passive_connect_popup == NULL) {
        lv_obj_t *bg = lv_obj_create(lv_layer_top());
        passive_connect_popup = bg;
        lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_opa(bg, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(bg, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(bg, passive_connect_popup_bg_click_handler, LV_EVENT_CLICKED, NULL);

        lv_obj_t *panel = lv_obj_create(bg);
        lv_obj_set_size(panel, 500, 350);
        lv_obj_center(panel);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        passive_connect_list = lv_obj_create(panel);
        lv_obj_set_size(passive_connect_list, lv_pct(100), lv_pct(100));
        lv_obj_set_flex_flow(passive_connect_list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(passive_connect_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(passive_connect_list, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(passive_connect_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(passive_connect_list, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(passive_connect_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_obj_t *item = lv_obj_create(passive_connect_list);
    lv_obj_set_size(item, lv_pct(100), 50);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(item, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(item, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    char *id_copy = lv_malloc(strlen(device_id) + 1);
    if (id_copy != NULL) {
        strcpy(id_copy, device_id);
        lv_obj_set_user_data(item, (void *)id_copy);
        lv_obj_add_event_cb(item, passive_item_delete_handler, LV_EVENT_DELETE, NULL);
    }

    lv_obj_t *name_label = lv_label_create(item);
    lv_label_set_text(name_label, device_name);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(name_label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *btn_container = lv_obj_create(item);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *connect_btn = lv_button_create(btn_container);
    lv_obj_set_size(connect_btn, 80, 36);
    lv_obj_set_style_bg_color(connect_btn, lv_color_hex(0x278FCB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(connect_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(connect_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *connect_label = lv_label_create(connect_btn);
    lv_label_set_text(connect_label, "Connect");
    lv_obj_center(connect_label);
    lv_obj_set_style_text_font(connect_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(connect_btn, passive_connect_btn_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *disconnect_btn = lv_button_create(btn_container);
    lv_obj_set_size(disconnect_btn, 90, 36);
    lv_obj_set_style_bg_color(disconnect_btn, lv_color_hex(0x278FCB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(disconnect_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(disconnect_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_margin_left(disconnect_btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *disconnect_label = lv_label_create(disconnect_btn);
    lv_label_set_text(disconnect_label, "Disconnect");
    lv_obj_center(disconnect_label);
    lv_obj_set_style_text_font(disconnect_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(disconnect_btn, passive_disconnect_btn_handler, LV_EVENT_CLICKED, NULL);
}

static void connect_fail_popup_bg_click_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_del(connect_fail_popup);
        connect_fail_popup = NULL;
    }
}

void carplayapp_show_connect_fail_popup(const char *reason_text)
{
    if (connect_fail_popup != NULL) {
        lv_obj_del(connect_fail_popup);
        connect_fail_popup = NULL;
    }

    lv_obj_t *bg = lv_obj_create(lv_layer_top());
    connect_fail_popup = bg;
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(bg, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bg, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bg, connect_fail_popup_bg_click_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *panel = lv_obj_create(bg);
    lv_obj_set_size(panel, 500, 300);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(panel);
    if (strncmp(reason_text, "DISCONNECT_REASON_NET_IFACE_CLIENT_DISCONNECTED",
            strlen("DISCONNECT_REASON_NET_IFACE_CLIENT_DISCONNECTED")) == 0) {
        lv_label_set_text(title_label, "Disconnect reason");
    } else {
        lv_label_set_text(title_label, "Connection Failed");
    }
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xE74C3C), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *reason_label = lv_label_create(panel);
    lv_label_set_text(reason_label, reason_text ? reason_text : "Unknown error");
    lv_obj_align(reason_label, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_text_font(reason_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(reason_label, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *hint_label = lv_label_create(panel);
    lv_label_set_text(hint_label, "Tap anywhere to dismiss");
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_text_font(hint_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hint_label, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
}

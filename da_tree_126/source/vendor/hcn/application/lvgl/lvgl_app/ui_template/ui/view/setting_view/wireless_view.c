#include "wireless_view.h"
#include <stdio.h>
#include <string.h>
#include "setting_menu.h"
#include "definition.h"
#include "aic_lang.h"
#include "ev1527.h"

#define DIALOG_WIDTH 316
#define DIALOG_HEIGHT 80
#define DIALOG_X 0
#define DIALOG_Y 40

#define WIRELESS_MAX_KEYS 2

typedef enum
{
    WL_MENU_LEARN,
    WL_MENU_DELETE,
    WL_MENU_NUM_MAX,
} wl_menu_e;

typedef enum
{
    WL_MODE_NONE,
    WL_MODE_LEARNING,
    WL_MODE_DELETE_LIST,
} wl_mode_e;

typedef enum
{
    WL_LEARN_NOT_LEARN = 0,
    WL_LEARN_LEARNING = 1,
    WL_LEARN_SUCCESS = 2,
    WL_LEARN_TIMEOUT = 3,
    WL_LEARN_DUPLICATE = 4,
} wl_learn_status_e;

static int focused_index = 0;
static int delete_list_focused = 0;
static int learned_count = 0;
static lv_obj_t *delete_list = NULL;
static lv_obj_t *learn_dialog = NULL;
static lv_obj_t *msgbox = NULL;
static wl_mode_e wl_mode = WL_MODE_NONE;
static lv_timer_t *learn_poll_timer = NULL;

static int delete_key_index_map[WIRELESS_MAX_KEYS];

static void wl_update_delete_list_selection(void);
static void wl_close_delete_list(void);
static void wl_rebuild_delete_list(void);
static void wl_msgbox_auto_close_cb(lv_timer_t *timer);

static void wl_view_deal_back(void)
{
    if (wl_mode == WL_MODE_DELETE_LIST)
    {
        wl_close_delete_list();
        return;
    }

    if (wl_mode == WL_MODE_LEARNING)
    {
        if (learn_poll_timer)
        {
            lv_timer_delete(learn_poll_timer);
            learn_poll_timer = NULL;
        }
        stop_key_learn();
        wl_mode = WL_MODE_NONE;
        if (learn_dialog)
        {
            lv_obj_delete(learn_dialog);
            learn_dialog = NULL;
        }
        printf("[WL] Learn cancelled by user\n");
        return;
    }

    if(!learn_dialog && !msgbox) //确认删除可返回
    {
        set_current_menu_level(MENU_LEVEL_1);
        // wireless_view_clean_state();
        setting_menu_set_focused_item(get_current_menu_index());
    }

}

static void wl_view_deal_up(void)
{
    if (wl_mode == WL_MODE_DELETE_LIST && learned_count > 0)
    {
        delete_list_focused = (delete_list_focused - 1 + learned_count) % learned_count;
        wl_update_delete_list_selection();
        return;
    }
    if (wl_mode != WL_MODE_NONE) return;
    if (WL_MENU_NUM_MAX == 0) return;
    focused_index = (focused_index - 1 + WL_MENU_NUM_MAX) % WL_MENU_NUM_MAX;
    wireless_view_set_focused_item(focused_index);
}

static void wl_view_deal_down(void)
{
    if (wl_mode == WL_MODE_DELETE_LIST && learned_count > 0)
    {
        delete_list_focused = (delete_list_focused + 1) % learned_count;
        wl_update_delete_list_selection();
        return;
    }
    if (wl_mode != WL_MODE_NONE) return;
    if (WL_MENU_NUM_MAX == 0) return;
    focused_index = (focused_index + 1) % WL_MENU_NUM_MAX;
    wireless_view_set_focused_item(focused_index);
}

static void wl_update_delete_list_selection(void)
{
    if (delete_list == NULL || learned_count == 0) return;
    for (int i = 0; i < learned_count; i++)
    {
        lv_obj_t *item = lv_obj_get_child(delete_list, i);
        if (item)
        {
            if (i == delete_list_focused)
                lv_obj_set_style_bg_image_src(item, LVGL_IMAGE_PATH(setting/setting_selected.png), 0);
            else
                lv_obj_remove_local_style_prop(item, LV_STYLE_BG_IMAGE_SRC, 0);
        }
    }
}

static void wl_close_delete_list(void)
{
    setting_page_t *page = setting_page_get(&ui_manager);
    delete_list_focused = 0;
    wl_mode = WL_MODE_NONE;
    if (page->wireless_list)
        lv_obj_remove_flag(page->wireless_list, LV_OBJ_FLAG_HIDDEN);
    if (delete_list)
    {
        lv_obj_delete(delete_list);
        delete_list = NULL;
    }
}

static void wl_rebuild_delete_list(void)
{
    if (delete_list == NULL) return;
    lv_obj_t *child = lv_obj_get_child(delete_list, delete_list_focused);
    if (child)
    {
        lv_obj_delete(child);
        if (lv_obj_get_child_count(delete_list) > 0)
        {
            lv_obj_set_style_bg_image_src(lv_obj_get_child(delete_list, 0),
                LVGL_IMAGE_PATH(setting/setting_selected.png), 0);
        }
        delete_list_focused = 0;
    }
}

static void wl_msgbox_auto_close_cb(lv_timer_t *timer)
{
    if (msgbox) { lv_obj_delete(msgbox); msgbox = NULL; }
    if (learn_dialog) { lv_obj_delete(learn_dialog); learn_dialog = NULL; }
    lv_timer_delete(timer);
}

static void wl_learn_poll_cb(lv_timer_t *timer)
{
    uint8_t status = get_wireless_key_state();
    printf("[WL] Learn poll status: %d\n", status);

    if (status == WL_LEARN_SUCCESS || status == WL_LEARN_TIMEOUT || status == WL_LEARN_DUPLICATE)
    {
        if (learn_poll_timer)
        {
            lv_timer_delete(learn_poll_timer);
            learn_poll_timer = NULL;
        }

        lv_obj_t *box = lv_obj_get_child(learn_dialog, 0);
        if (box == NULL) return;

        if (status == WL_LEARN_SUCCESS)
            lv_msgbox_add_text(box, aic_get_translation(wl_success));
        else if (status == WL_LEARN_TIMEOUT)
            lv_msgbox_add_text(box, aic_get_translation(wl_timeout));
        else if (status == WL_LEARN_DUPLICATE)
            lv_msgbox_add_text(box, aic_get_translation(wl_duplicate));

        lv_timer_create(wl_msgbox_auto_close_cb, 2000, NULL);

        wl_mode = WL_MODE_NONE;
    }
}

static void wl_view_deal_set(void)
{
    setting_page_t *page = setting_page_get(&ui_manager);

    if (wl_mode == WL_MODE_DELETE_LIST)
    {
        int bsp_idx = delete_key_index_map[delete_list_focused];
        uint32_t addr = 0;
        get_wireless_key_addr(bsp_idx, &addr);
        printf("[WL] Delete key, list idx %d, bsp idx %d, addr 0x%08lX\n",
               delete_list_focused, bsp_idx, (unsigned long)addr);
        delete_key(addr);

        learned_count = get_wireless_key_count();
        if (learned_count == 0)
            wl_close_delete_list();
        else
            wl_rebuild_delete_list();
        return;
    }

    if (wl_mode == WL_MODE_LEARNING) return;

    if (focused_index == WL_MENU_LEARN)
    {
        if (get_wireless_key_count() >= WIRELESS_MAX_KEYS)
        {
            if (msgbox == NULL)
            {
                msgbox = lv_obj_create(page->obj);
                lv_obj_set_size(msgbox, 327, 155);
                lv_obj_set_scrollbar_mode(msgbox, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_pos(msgbox, 137, 90);
                lv_obj_t *box = lv_msgbox_create(msgbox);
                lv_msgbox_add_title(box, aic_get_translation(wireless));
                lv_msgbox_add_text(box, aic_get_translation(wl_max_reached));
                lv_obj_set_size(box, 327, 155);
                lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);
                lv_obj_set_pos(box, 0, 0);
                lv_timer_create(wl_msgbox_auto_close_cb, 2000, msgbox);
            }
            return;
        }

        learn_dialog = lv_obj_create(page->obj);
        lv_obj_set_size(learn_dialog, 327, 155);
        lv_obj_set_pos(learn_dialog, 137, 90);
        lv_obj_t *box = lv_msgbox_create(learn_dialog);
        lv_obj_set_scrollbar_mode(learn_dialog, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(box, 327, 155);
        lv_obj_set_pos(box, 0, 0);
        lv_msgbox_add_title(box, aic_get_translation(wl_learning));
        lv_msgbox_add_text(box, aic_get_translation(wl_press_key));
        lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);

        start_key_learn();
        learn_poll_timer = lv_timer_create(wl_learn_poll_cb, 500, NULL);
        wl_mode = WL_MODE_LEARNING;
    }
    else if (focused_index == WL_MENU_DELETE)
    {
        learned_count = get_wireless_key_count();
        if (learned_count == 0)
        {
            if (msgbox == NULL)
            {
                msgbox = lv_obj_create(page->obj);
                lv_obj_set_size(msgbox, 327, 155);
                lv_obj_set_scrollbar_mode(msgbox, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_pos(msgbox, 137, 90);
                lv_obj_t *box = lv_msgbox_create(msgbox);
                lv_msgbox_add_title(box, aic_get_translation(wireless));
                lv_msgbox_add_text(box, aic_get_translation(wl_no_key));
                lv_obj_set_size(box, 327, 155);
                lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);
                lv_obj_set_pos(box, 0, 0);
                lv_timer_create(wl_msgbox_auto_close_cb, 2000, msgbox);
            }
            return;
        }

        wl_mode = WL_MODE_DELETE_LIST;
        lv_obj_add_flag(page->wireless_list, LV_OBJ_FLAG_HIDDEN);

        delete_list = lv_obj_create(page->obj);
        lv_obj_set_size(delete_list, DIALOG_WIDTH, 90);
        lv_obj_set_pos(delete_list, 144, 100);
        lv_obj_set_flex_flow(delete_list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scrollbar_mode(delete_list, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_pad_all(delete_list, 0, 0);
        lv_obj_set_style_pad_row(delete_list, 0, 0);
        lv_obj_set_style_pad_column(delete_list, 0, 0);
        lv_obj_set_style_bg_opa(delete_list, 0, 0);
        lv_obj_set_style_border_width(delete_list, 0, 0);

        int valid_count = 0;
        for (int i = 0; i < WIRELESS_MAX_KEYS; i++)
        {
            uint32_t addr = 0;
            if (get_wireless_key_addr(i, &addr) == 0)
            {
                delete_key_index_map[valid_count] = i;
                char key_name[32] = {0};
                snprintf(key_name, 32, "KEY%d-%08lX", valid_count + 1, (unsigned long)addr);

                lv_obj_t *label = lv_label_create(delete_list);
                lv_label_set_text(label, key_name);
                lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
                lv_obj_set_size(label, DIALOG_WIDTH, 43);
                lv_obj_set_style_bg_opa(label, 0, 0);
                lv_obj_set_style_text_font(label, aic_get_multi_lang_font(16), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_pad_top(label, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_center(label);
                valid_count++;
            }
        }

        delete_list_focused = 0;
        wl_update_delete_list_selection();
    }
}

static short_click_deal short_click[] = {
    [CLICK_SHORT_UP]   = wl_view_deal_up,
    [CLICK_SHORT_DOWN] = wl_view_deal_down,
    [CLICK_SHORT_SET]  = wl_view_deal_set,
    [CLICK_SHORT_BACK] = wl_view_deal_back,
};

void wireless_view_init(void)
{
    focused_index = 0;
    wireless_view_set_focused_item(focused_index);
    learned_count = get_wireless_key_count();
    printf("[WL] init: %d learned keys\n", learned_count);
}

void on_wireless_view_deal_short_key(click_e key)
{
    printf("on_wireless_view_deal_short_key = %d\n", key);
    if (key < sizeof(short_click) / sizeof(short_click_deal) && short_click[key])
        short_click[key]();
}

void wireless_view_set_focused_item(int index)
{
    setting_page_t *page = setting_page_get(&ui_manager);
    int count = lv_obj_get_child_count(page->wireless_list);
    if (count == 0 || index >= count) return;

    for (int i = 0; i < count; i++)
    {
        lv_obj_t *child = lv_obj_get_child(page->wireless_list, i);
        if (i == index)
            lv_obj_set_style_bg_image_src(child, LVGL_IMAGE_PATH(setting/setting_selected.png), 0);
        else
            lv_obj_remove_local_style_prop(child, LV_STYLE_BG_IMAGE_SRC, 0);
    }

    lv_obj_scroll_to_view(lv_obj_get_child(page->wireless_list, index), LV_ANIM_OFF);
    focused_index = index;
}

void wireless_view_clean_state(void)
{
    wl_mode = WL_MODE_NONE;
    if (learn_dialog) { lv_obj_delete(learn_dialog); learn_dialog = NULL; }
    if (delete_list)  { lv_obj_delete(delete_list);  delete_list  = NULL; }

    setting_page_t *page = setting_page_get(&ui_manager);
    int count = lv_obj_get_child_count(page->wireless_list);
    for (int i = 0; i < count; i++)
    {
        lv_obj_t *child = lv_obj_get_child(page->wireless_list, i);
        if (child)
            lv_obj_remove_local_style_prop(child, LV_STYLE_BG_IMAGE_SRC, 0);
    }
    focused_index = 0;
}

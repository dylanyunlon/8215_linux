#include "nfc_view.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>
#include "setting_menu.h"
#include "definition.h"
#include "aic_lang.h"

#include "nfc_manage.h"

#define DIALOG_WIDTH 316
#define DIALOG_HEIGHT 80
#define DIALOG_X 0  // 使用lv_obj_center + translate偏移
#define DIALOG_Y 40 // 向下偏移40px

#define NFC_MAX_CARDS 2

typedef enum
{
    NFC_MENU_LEARN,
    NFC_MENU_DELETE,
    NFC_MENU_NUM_MAX,
} nfc_menu_e;

// NFC工作模式状态
typedef enum
{
    NFC_MODE_NONE,
    NFC_MODE_LEARNING,    // 学习中
    NFC_MODE_DELETE_LIST, // 删除列表
} nfc_mode_e;

// 学习状态枚举
typedef enum
{
    NFC_LEARN_NOT_LEARN = 0,    // 未学习
    NFC_LEARN_LEARNING = 1,    // 学习中
    NFC_LEARN_SUCCESS = 2,     // 学习成功
    NFC_LEARN_TIMEOUT = 3,     // 学习超时
    NFC_LEARN_DUPLICATE = 4,   // NFC重复学习（已存在）
} nfc_learn_status_e;

// static char learned_cards[NFC_MENU_NUM_MAX][32] = {0};
static int focused_index = 0;
static int delete_list_focused = 0; // 删除列表索引
static int learned_count = 0;
static lv_obj_t *delete_list = NULL;
static lv_obj_t *learn_dialog = NULL;
static lv_obj_t *msgbox = NULL;
static nfc_mode_e nfc_mode = NFC_MODE_NONE;
static lv_timer_t *learn_poll_timer = NULL;

static int delete_card_index_map[NFC_MAX_CARDS];  // 删除列表下标卡片index的映射

static void nfc_update_delete_list_selection(void);
static void nfc_close_delete_list(void);
static void nfc_rebuild_delete_list(void);
static void nfc_msgbox_auto_close_cb(lv_timer_t *timer);

// 返回上一级菜单
static void nfc_view_deal_back(void)
{
    if (nfc_mode == NFC_MODE_DELETE_LIST)
    {
        nfc_close_delete_list();
        return;
    }

    if (nfc_mode == NFC_MODE_LEARNING)
    {
        // 学习模式下，BACK取消学习
        if (learn_poll_timer)
        {
            lv_timer_delete(learn_poll_timer);
            learn_poll_timer = NULL;
        }

        nfc_stop_cur();  // 停止学习

        nfc_mode = NFC_MODE_NONE;

        if (learn_dialog)
        {
            lv_obj_delete(learn_dialog);
            learn_dialog = NULL;
        }

        printf("[NFC] Learn cancelled by user\n");
        return;
    }

    if(!learn_dialog && !msgbox)  //确认删除可返回
    {
        set_current_menu_level(MENU_LEVEL_1);
        // nfc_view_clean_state();
        setting_menu_set_focused_item(get_current_menu_index());
    }
}

// 上键处理
static void nfc_view_deal_up(void)
{
    if (nfc_mode == NFC_MODE_DELETE_LIST && learned_count > 0)
    {
        delete_list_focused = (delete_list_focused - 1 + learned_count) % learned_count;
        nfc_update_delete_list_selection();
        return;
    }

    if (nfc_mode != NFC_MODE_NONE)
        return;

    if (NFC_MENU_NUM_MAX == 0)
        return;

    focused_index = (focused_index - 1 + NFC_MENU_NUM_MAX) % NFC_MENU_NUM_MAX;
    nfc_view_set_focused_item(focused_index);
}

// 下键处理
static void nfc_view_deal_down(void)
{
    if (nfc_mode == NFC_MODE_DELETE_LIST && learned_count > 0)
    {
        delete_list_focused = (delete_list_focused + 1) % learned_count;
        nfc_update_delete_list_selection();
        return;
    }

    if (nfc_mode != NFC_MODE_NONE)
        return;

    if (NFC_MENU_NUM_MAX == 0)
        return;

    focused_index = (focused_index + 1) % NFC_MENU_NUM_MAX;
    nfc_view_set_focused_item(focused_index);
}

// 更新删除列表选中状态
static void nfc_update_delete_list_selection(void)
{
    if (delete_list == NULL || learned_count == 0)
        return;

    for (int i = 0; i < learned_count; i++)
    {
        lv_obj_t *card_item = lv_obj_get_child(delete_list, i);
        if (card_item)
        {
            if (i == delete_list_focused){
                lv_obj_set_style_bg_image_src(card_item, LVGL_IMAGE_PATH(setting/setting_selected.png), 0);
            }
            else{
                lv_obj_remove_local_style_prop(card_item, LV_STYLE_BG_IMAGE_SRC, 0);
            }
        }
    }
}

// 关闭删除列表
static void nfc_close_delete_list(void)
{
    setting_page_t *page = setting_page_get(&ui_manager);

    delete_list_focused = 0;

    nfc_mode = NFC_MODE_NONE;

    // 恢复显示原有的nfc_list
    if (page->nfc_list){
        lv_obj_remove_flag(page->nfc_list, LV_OBJ_FLAG_HIDDEN);
    }

    if (delete_list){
        lv_obj_delete(delete_list);
        delete_list = NULL;
    }
}

//删除列表UI
static void nfc_rebuild_delete_list(void)
{
    // setting_page_t *page = setting_page_get(&ui_manager);

    if (delete_list == NULL)
        return;

    lv_obj_t *child = lv_obj_get_child(delete_list, delete_list_focused);

    if (child)
    {
        lv_obj_delete(child);
        lv_obj_set_style_bg_image_src(lv_obj_get_child(delete_list, 0), LVGL_IMAGE_PATH(setting/setting_selected.png), 0);

        //高亮第一个选项
        delete_list_focused = 0;
    }

}

// 弹窗自动关闭回调
static void nfc_msgbox_auto_close_cb(lv_timer_t *timer)
{
    if (msgbox)
    {
        lv_obj_delete(msgbox);
        msgbox = NULL;
    }

    if (learn_dialog)
    {
        lv_obj_delete(learn_dialog);
        learn_dialog = NULL;
    }

    lv_timer_delete(timer);
}

static void nfc_learn_poll_cb(lv_timer_t *timer)
{
    uint8_t status = nfc_manage_get_nfc_learn_status();
    printf("[NFC] Learn poll status: %d\n", status);

    if (status == NFC_LEARN_SUCCESS || status == NFC_LEARN_TIMEOUT || status == NFC_LEARN_DUPLICATE)
    {
        if (learn_poll_timer)
        {
            lv_timer_delete(learn_poll_timer);
            learn_poll_timer = NULL;
        }

        lv_obj_t* box = lv_obj_get_child(learn_dialog, 0);

        if(NULL == box) return ;

        if (status == NFC_LEARN_SUCCESS)
        {
            lv_msgbox_add_text(box, aic_get_translation(nfc_success));
            printf("[NFC] Learn success!\n");
        }
        else if (status == NFC_LEARN_TIMEOUT)
        {

            lv_msgbox_add_text(box, aic_get_translation(nfc_timeout));
            printf("[NFC] Learn timeout!\n");
        }
        else if (status == NFC_LEARN_DUPLICATE)
        {

            lv_msgbox_add_text(box, aic_get_translation(nfc_duplicate));
            printf("[NFC] Learn duplicate!\n");
        }

        // 2秒后关闭
        lv_timer_create(nfc_msgbox_auto_close_cb, 2000, NULL);

        nfc_mode = NFC_MODE_NONE;
    }

    // status == NFC_LEARN_LEARNING: 继续等待
}

// SET键处理
static void nfc_view_deal_set(void)
{
    setting_page_t *page = setting_page_get(&ui_manager);
    // VIEW_MEMBER_VALID(page, nfc_list);

    // 删除列表模式：SET删除选中卡片
    if (nfc_mode == NFC_MODE_DELETE_LIST)
    {
        // 使用映射获取卡片index
        int bsp_card_index = delete_card_index_map[delete_list_focused];
        printf("[NFC] Delete card, list index: %d, bsp index: %d\n", delete_list_focused, bsp_card_index);

        //删除卡片
        nfc_delete(bsp_card_index);

        printf("[NFC] Delete request sent\n");
        rt_thread_mdelay(120);

        // 删除成功后刷新列表
        //learned_count = nfc_manager_get_card_count();
        learned_count -= 1;
        if (learned_count == 0)
        {
            nfc_close_delete_list();
        }
        else
        {
            nfc_rebuild_delete_list();
        }
        return;
    }

    // 学习中不允许其他SET操作
    if (nfc_mode == NFC_MODE_LEARNING)
    {
        return;
    }

    if (focused_index == NFC_MENU_LEARN)
    {
        // 检查当前卡片数量是否已达最大值
        if (nfc_manager_get_card_count() >= NFC_MAX_CARDS)
        {
            if (msgbox == NULL)
            {
                msgbox = lv_obj_create(page->obj);
                lv_obj_set_size(msgbox, 327, 155);
                lv_obj_set_scrollbar_mode(msgbox, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_pos(msgbox, 137, 90);
                lv_obj_t *box = lv_msgbox_create(msgbox);
                lv_msgbox_add_title(box, "NFC");
                lv_msgbox_add_text(box, aic_get_translation(nfc_max_reached));
                lv_obj_set_size(box, 327, 155);
                lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);
                lv_obj_set_pos(box, 0, 0);
                // 2秒后自动关闭
                lv_timer_create(nfc_msgbox_auto_close_cb, 2000, msgbox);
            }
            return;
        }

        // 显示学习弹窗
        learn_dialog = lv_obj_create(page->obj);
        lv_obj_set_size(learn_dialog, 327, 155);

        lv_obj_set_pos(learn_dialog, 137, 90);
        lv_obj_t *box = lv_msgbox_create(learn_dialog);
        lv_obj_set_scrollbar_mode(learn_dialog, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(box, 327, 155);
        lv_obj_set_pos(box, 0, 0);

        lv_msgbox_add_title(box, aic_get_translation(nfc_learning));
        lv_msgbox_add_text(box, aic_get_translation(nfc_put_card));

        // 设置msgbox文字字体为c_default_16
        lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);

        //启动学习
        nfc_learn();

        learn_poll_timer = lv_timer_create(nfc_learn_poll_cb, 500, NULL);

        nfc_mode = NFC_MODE_LEARNING;
    }
    else if (focused_index == NFC_MENU_DELETE)
    {
        //获取已学习卡片数量
        learned_count = nfc_manager_get_card_count();

        if (learned_count == 0)
        {
            // 无已学习卡片，显示提示弹窗
            if (msgbox == NULL)
            {
                msgbox = lv_obj_create(page->obj);
                lv_obj_set_size(msgbox, 327, 155);
                lv_obj_set_scrollbar_mode(msgbox, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_pos(msgbox, 137, 90);
                lv_obj_t *box = lv_msgbox_create(msgbox);
                lv_msgbox_add_title(box, "NFC");
                lv_msgbox_add_text(box, aic_get_translation(nfc_no_card));
                lv_obj_set_size(box, 327, 155);
                lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);
                lv_obj_set_style_text_font(box, aic_get_multi_lang_font(16), LV_PART_MAIN);
                lv_obj_set_pos(box, 0, 0);
                // 2秒后自动关闭
                lv_timer_create(nfc_msgbox_auto_close_cb, 2000, msgbox);
            }
        }
        else
        {
            // 显示删除列表
            nfc_mode = NFC_MODE_DELETE_LIST;

            // 隐藏原有的nfc_list选项
            lv_obj_add_flag(page->nfc_list, LV_OBJ_FLAG_HIDDEN);

            // 在nfc_list位置创建删除列表，使用弹窗样式
            delete_list = lv_obj_create(page->obj);
            lv_obj_set_size(delete_list, DIALOG_WIDTH, 90);
            lv_obj_set_pos(delete_list, 144, 100);
            lv_obj_set_flex_flow(delete_list, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_scrollbar_mode(delete_list, LV_SCROLLBAR_MODE_OFF);

            lv_obj_set_style_pad_all(delete_list, 0, 0);
            lv_obj_set_style_pad_row(delete_list, 0, 0);
            lv_obj_set_style_pad_column(delete_list, 0, 0);
            // 设置弹窗样式背景

            lv_obj_set_style_bg_opa(delete_list, 0, 0);
            lv_obj_set_style_border_width(delete_list, 0, 0);

            // 添加卡片选项 - 使用BSP接口获取有效卡片
            int valid_count = 0;

            for (int i = 0; i < NFC_MAX_CARDS; i++)
            {
                if (nfc_manage_get_card_valid(i) == 0)
                {
                    delete_card_index_map[valid_count] = i;  // 记录映射

                    uint8_t uid[10] = {0};
                    uint8_t uid_len = 0;
                    char card_name[32] = {0};

                    if (nfc_manager_get_card_uid(i, uid, &uid_len) == 0)
                    {
                        snprintf(card_name, 32, "NFC_%02X%02X", uid[0], uid[1]);
                    }
                    else
                    {
                        snprintf(card_name, 32, "NFC_%d", i + 1);
                    }

                    lv_obj_t *label = lv_label_create(delete_list);
                    lv_label_set_text(label, card_name);
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

            // 高亮第一个选项
            delete_list_focused = 0;
            nfc_update_delete_list_selection();
        }
    }
}

static short_click_deal short_click[] = {
    [CLICK_SHORT_UP] = nfc_view_deal_up,
    [CLICK_SHORT_DOWN] = nfc_view_deal_down,
    [CLICK_SHORT_SET] = nfc_view_deal_set,
    [CLICK_SHORT_BACK] = nfc_view_deal_back,
};

void nfc_view_init(void)
{
    focused_index = 0;
    nfc_view_set_focused_item(focused_index);

    //获取卡片数量
    learned_count = nfc_manager_get_card_count();
    printf("[NFC] init: %d learned cards\n", learned_count);

    return;
}

void on_nfc_view_deal_short_key(click_e key)
{
    printf("on_nfc_view_deal_short_key = %d\n", key);
    if (key < sizeof(short_click) / sizeof(short_click_deal) && short_click[key])
    {
        short_click[key]();
    }
}

void nfc_view_set_focused_item(int index)
{
    setting_page_t *page = setting_page_get(&ui_manager);
    // VIEW_MEMBER_VALID(page, nfc_list);

    int count = lv_obj_get_child_count(page->nfc_list);
    if (count == 0 || index >= count)
        return;

    for (int i = 0; i < count; i++)
    {
        lv_obj_t *child = lv_obj_get_child(page->nfc_list, i);
        if (i == index)
        {
            lv_obj_set_style_bg_image_src(child, LVGL_IMAGE_PATH(setting/setting_selected.png), 0);
        }
        else
        {
            lv_obj_remove_local_style_prop(child, LV_STYLE_BG_IMAGE_SRC, 0);
        }
    }

    lv_obj_scroll_to_view(lv_obj_get_child(page->nfc_list, index), LV_ANIM_OFF);
    focused_index = index;
}

void nfc_view_clean_state(void)
{
    nfc_mode = NFC_MODE_NONE;

    if (learn_dialog)
    {
        lv_obj_delete(learn_dialog);
        learn_dialog = NULL;
    }

    if (delete_list)
    {
        lv_obj_delete(delete_list);
        delete_list = NULL;
    }

    setting_page_t *page = setting_page_get(&ui_manager);

    int count = lv_obj_get_child_count(page->nfc_list);
    for (int i = 0; i < count; i++)
    {
        lv_obj_t *child = lv_obj_get_child(page->nfc_list, i);
        if (child)
        {
            lv_obj_remove_local_style_prop(child, LV_STYLE_BG_IMAGE_SRC, 0);
        }
    }

    focused_index = 0;
}
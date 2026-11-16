/*
 * dvr_view.c --- DVR 页面视图层（数据 -> 控件）
 *
 * 只做搬运：数据由 dvr_logic.c 经 vs_dvr_* 门面取回，本文件不 include
 * vehicle_services.h、不调中间件。
 *
 * 控件由 dvr_page.c 建好，这里只改"内容/状态"：
 *   - 焦点：换图标 n/p、文字颜色（选中橙底白字，未选中米底深棕）
 *   - 内容：文件名、版本、容量、页签文案、弹窗标题/按钮
 *   - 预览：软解把 RGBA8888 贴进画布（RGBA -> LVGL ARGB8888 内存序，换 R/B 两字节）
 *
 * 注意：VIEW_MEMBER_VALID 会在函数里声明与控件成员同名的局部变量（如 row_name），
 * 所以本文件函数形参一律带 _ 后缀，避免重名。
 */
#include "dvr_view.h"
#include "definition.h"

#include <string.h>

/* 画布像素缓冲：lv_canvas 要求缓冲活到页面销毁，所以放文件级静态数组。
 * 用 uint32_t 而不只是 uint8_t 是为了保证 4 字节对齐（LVGL 的 draw buf 要求）。 */
static uint32_t s_canvas_buf[DVR_PREVIEW_W * DVR_PREVIEW_H];

/* 图片路径缓存：按字符串字面量指针存，同一个字面量只拼一次、字符串常驻
 * （LVGL 的 style 属性不复制字符串，指针必须长期有效）。 */
#define DVR_ASSET_MAX 48
static const char *s_asset_key[DVR_ASSET_MAX];
static char       *s_asset_val[DVR_ASSET_MAX];
static int         s_asset_cnt;

const char *dvr_view_asset(const char *rel)
{
    const char *root;
    char       *buf;
    size_t      n;
    int         i;

    for (i = 0; i < s_asset_cnt; i++) {
        if (s_asset_key[i] == rel) {
            return s_asset_val[i];
        }
    }

    root = ui_asset_root();
    n    = strlen(root) + strlen(rel) + 16;
    buf  = (char *)lv_malloc(n);
    if (buf == NULL) {
        return "";
    }
    lv_snprintf(buf, n, "%s/image/dvr/%s", root, rel);

    if (s_asset_cnt < DVR_ASSET_MAX) {
        s_asset_key[s_asset_cnt] = rel;
        s_asset_val[s_asset_cnt] = buf;
        s_asset_cnt++;
    }
    return buf;
}

void *dvr_view_preview_buffer(void)
{
    return s_canvas_buf;
}

void dvr_view_init(void)
{
    /* 目前没有需要初始化的状态：控件由 dvr_page.c 建，焦点由 dvr_logic.c 首刷时设 */
}

/* ---------- 小工具 ---------- */

static void img_src(lv_obj_t *obj, const char *rel)
{
    if (obj != NULL) {
        lv_image_set_src(obj, dvr_view_asset(rel));
    }
}

static void lab_color(lv_obj_t *obj, uint32_t color)
{
    if (obj != NULL) {
        lv_obj_set_style_text_color(obj, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

/* 选中态的一对图（n/p）+ 文字色（米底深棕 / 橙底白字） */
static void sel_img(lv_obj_t *obj, const char *n_rel, const char *p_rel, bool sel)
{
    img_src(obj, sel ? p_rel : n_rel);
}

static void sel_lab(lv_obj_t *obj, bool sel)
{
    lab_color(obj, sel ? DVR_COLOR_TEXT_SEL : DVR_COLOR_TEXT);
}

/* ---------- 子视图切换 ---------- */

void dvr_view_show_sub(dvr_sub_e sub)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    bool        in_main;

    VIEW_MEMBER_VALID(page, main_view);
    VIEW_MEMBER_VALID(page, main_bg);
    VIEW_MEMBER_VALID(page, main_dock);
    VIEW_MEMBER_VALID(page, cam_dock);
    VIEW_MEMBER_VALID(page, idle_view);
    VIEW_MEMBER_VALID(page, list_view);
    VIEW_MEMBER_VALID(page, set_view);
    VIEW_MEMBER_VALID(page, popup_view);

    /* 可见性矩阵 */
    in_main = (sub == DVR_SUB_MAIN || sub == DVR_SUB_CAM_SW || sub == DVR_SUB_PLAYBACK);

    if (in_main)               lv_obj_remove_flag(main_view, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(main_view, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_MAIN)   lv_obj_remove_flag(main_bg, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(main_bg, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_MAIN)   lv_obj_remove_flag(main_dock, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(main_dock, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_CAM_SW) lv_obj_remove_flag(cam_dock, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(cam_dock, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_LIST_SEL) lv_obj_remove_flag(idle_view, LV_OBJ_FLAG_HIDDEN);
    else                         lv_obj_add_flag(idle_view, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_LIST || sub == DVR_SUB_LIST_ACT)
                               lv_obj_remove_flag(list_view, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(list_view, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_SETTING || sub == DVR_SUB_SET_EDIT)
                               lv_obj_remove_flag(set_view, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(set_view, LV_OBJ_FLAG_HIDDEN);

    if (sub == DVR_SUB_POPUP)  lv_obj_remove_flag(popup_view, LV_OBJ_FLAG_HIDDEN);
    else                       lv_obj_add_flag(popup_view, LV_OBJ_FLAG_HIDDEN);
}

/* ---------- 焦点 ---------- */

void dvr_view_dock_focus(int index)
{
    static const char *n_rel[4] = { "icon_camera_n.png", "icon_playback_n.png",
                                    "icon_photo_n.png",  "icon_settings_n.png" };
    static const char *p_rel[4] = { "icon_camera_p.png", "icon_playback_p.png",
                                    "icon_photo_p.png",  "icon_settings_p.png" };
    dvr_page_t *page = dvr_page_get(&ui_manager);
    int i;

    for (i = 0; i < 4; i++) {
        sel_img(page->dock_icon[i], n_rel[i], p_rel[i], (i == index));
    }
}

void dvr_view_cam_focus(int index)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, cam_tab_sel);

    if (index < 0) index = 0;
    if (index > 2) index = 2;

    lv_obj_set_pos(cam_tab_sel, index * DVR_CAM_TAB_W, 0);
    sel_lab(page->cam_front, (index == 0));
    sel_lab(page->cam_rear,  (index == 1));
    sel_img(page->cam_snap, "icon_photo_n.png", "icon_photo_p.png", (index == 2));
}

void dvr_view_tab_focus(int right)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    if (page->idle_tab_sel != NULL) {
        lv_obj_set_pos(page->idle_tab_sel, right ? DVR_TAB_SEL_W : 0, 0);
    }
    if (page->list_tab_sel != NULL) {
        lv_obj_set_pos(page->list_tab_sel, right ? DVR_TAB_SEL_W : 0, 0);
    }
    sel_lab(page->idle_tab_left,  !right);
    sel_lab(page->idle_tab_right,  right);
    sel_lab(page->list_tab_left,  !right);
    sel_lab(page->list_tab_right,  right);
}

void dvr_view_row_focus(int index)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    int i;

    for (i = 0; i < DVR_LIST_ROW_N; i++) {
        /* 焦点行绿字，其余深棕；图标一律回到未选中 */
        lab_color(page->row_name[i], (i == index) ? DVR_COLOR_FOCUS : DVR_COLOR_TEXT);
        sel_img(page->row_view[i], "icon_view_n.png", "icon_view_p.png", false);
        sel_img(page->row_del[i],  "icon_delete_n.png", "icon_delete_p.png", false);
    }
}

void dvr_view_row_action(int index, int del)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    if (index < 0 || index >= DVR_LIST_ROW_N) {
        return;
    }
    sel_img(page->row_view[index], "icon_view_n.png", "icon_view_p.png", (del == 0));
    sel_img(page->row_del[index],  "icon_delete_n.png", "icon_delete_p.png", (del != 0));
}

void dvr_view_set_focus(int row)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    int i;

    for (i = 0; i < DVR_SET_ROW_NUM; i++) {
        sel_img(page->set_row_label_bg[i], "settings_btn1_n.png", "settings_btn1_p.png", (i == row));
        sel_lab(page->set_row_lab[i], (i == row));
    }
}

void dvr_view_loop_sel(int v)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    if (page->set_loop_sel != NULL) {
        lv_obj_set_pos(page->set_loop_sel, 278 + v * 220, 28);
    }
}

void dvr_view_fmt_sel(int f)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    if (page->set_fmt_sel != NULL) {
        lv_obj_set_pos(page->set_fmt_sel, f ? 615 : 285, 28);
    }
}

/* ---------- 文件列表内容 ---------- */

void dvr_view_list_fill(const dvr_view_item_t *items, int n)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    int i;

    for (i = 0; i < DVR_LIST_ROW_N; i++) {
        bool has = (items != NULL) && (i < n);

        if (page->row_name[i] != NULL) {
            lv_label_set_text(page->row_name[i], has ? items[i].name : "");
        }
        /* 锁定文件用另一套图标（原图两张一样，这里当"状态位"用） */
        sel_img(page->row_icon[i], "list_icon_video_n.png", "list_icon_video_p.png",
                has && items[i].locked);
        if (has) {
            lv_obj_remove_flag(page->row_view[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(page->row_del[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(page->row_view[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(page->row_del[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void dvr_view_list_no_sd(bool show)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, list_no_sd);

    if (show) {
        lv_obj_remove_flag(list_no_sd, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(list_no_sd, LV_OBJ_FLAG_HIDDEN);
    }
}

void dvr_view_tab_text(int mode)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    const char *left  = (mode == 0) ? aic_get_translation(dvr_front_video)
                                    : aic_get_translation(dvr_front_photo);
    const char *right = (mode == 0) ? aic_get_translation(dvr_rear_video)
                                    : aic_get_translation(dvr_rear_photo);

    if (page->idle_tab_left != NULL)  lv_label_set_text(page->idle_tab_left, left);
    if (page->idle_tab_right != NULL) lv_label_set_text(page->idle_tab_right, right);
    if (page->list_tab_left != NULL)  lv_label_set_text(page->list_tab_left, left);
    if (page->list_tab_right != NULL) lv_label_set_text(page->list_tab_right, right);
}

void dvr_view_set_version(const char *ver)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, set_ver_text);

    lv_label_set_text(set_ver_text, (ver != NULL && ver[0]) ? ver : "--");
}

void dvr_view_apply_language(void)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);
    static const key_id_t row_key[DVR_SET_ROW_NUM] = { dvr_version, dvr_loop_time,
                                                       dvr_format, dvr_sd_card };
    static const key_id_t loop_key[3] = { dvr_loop_1min, dvr_loop_2min, dvr_loop_3min };
    int i;

    if (page == NULL) {
        return;
    }
    if (page->cam_front != NULL)  lv_label_set_text(page->cam_front, aic_get_translation(dvr_front_video));
    if (page->cam_rear != NULL)   lv_label_set_text(page->cam_rear, aic_get_translation(dvr_rear_video));
    if (page->list_no_sd != NULL) lv_label_set_text(page->list_no_sd, aic_get_translation(dvr_no_sd));

    for (i = 0; i < DVR_SET_ROW_NUM; i++) {
        if (page->set_row_lab[i] != NULL) {
            lv_label_set_text(page->set_row_lab[i], aic_get_translation(row_key[i]));
        }
    }
    for (i = 0; i < 3; i++) {
        if (page->set_loop_lab[i] != NULL) {
            lv_label_set_text(page->set_loop_lab[i], aic_get_translation(loop_key[i]));
        }
    }
    if (page->set_fmt_sd_lab != NULL)  lv_label_set_text(page->set_fmt_sd_lab, aic_get_translation(dvr_quick_format));
    if (page->set_fmt_rst_lab != NULL) lv_label_set_text(page->set_fmt_rst_lab, aic_get_translation(dvr_factory_reset));
    if (page->popup_ok != NULL)        lv_label_set_text(page->popup_ok, aic_get_translation(dvr_confirm));
    if (page->popup_cancel != NULL)    lv_label_set_text(page->popup_cancel, aic_get_translation(dvr_cancel));
}

/* ---------- 弹窗 ---------- */

void dvr_view_popup(dvr_pop_e src, const char *title)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    if (page->popup_title != NULL) {
        lv_label_set_text(page->popup_title, (title != NULL) ? title : "");
    }
    if (src == DVR_POP_NO_SD) {
        /* 无 SD 卡：只有"确认"，没有"取消" */
        if (page->popup_cancel_bg != NULL) lv_obj_add_flag(page->popup_cancel_bg, LV_OBJ_FLAG_HIDDEN);
        if (page->popup_cancel != NULL)    lv_obj_add_flag(page->popup_cancel, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (page->popup_cancel_bg != NULL) lv_obj_remove_flag(page->popup_cancel_bg, LV_OBJ_FLAG_HIDDEN);
        if (page->popup_cancel != NULL)    lv_obj_remove_flag(page->popup_cancel, LV_OBJ_FLAG_HIDDEN);
    }
    if (page->popup_loading != NULL) {
        lv_obj_add_flag(page->popup_loading, LV_OBJ_FLAG_HIDDEN);
    }
}

void dvr_view_popup_focus(int f)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    sel_img(page->popup_ok_bg, "pop_up_btn_n.png", "pop_up_btn_p.png", (f == 0));
    sel_img(page->popup_cancel_bg, "pop_up_btn_n.png", "pop_up_btn_p.png", (f == 1));
    sel_lab(page->popup_ok, (f == 0));
    sel_lab(page->popup_cancel, (f == 1));
}

/* ---------- 预览 ---------- */

void dvr_view_show_preview_hint(const char *text)
{
    dvr_page_t *page = dvr_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, preview_hint);
    VIEW_MEMBER_VALID(page, preview_canvas);

    lv_label_set_text(preview_hint, (text != NULL) ? text : "");
    lv_obj_remove_flag(preview_hint, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(preview_canvas, LV_OBJ_FLAG_HIDDEN);
}

void dvr_view_draw_preview(const uint8_t *rgba, uint32_t w, uint32_t h)
{
    uint32_t x, y;
    uint8_t *dst = (uint8_t *)s_canvas_buf;
    dvr_page_t *page = dvr_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, preview_canvas);
    VIEW_MEMBER_VALID(page, preview_hint);

    if (rgba == NULL) {
        dvr_view_show_preview_hint("NO SIGNAL");
        return;
    }
    if (w != DVR_PREVIEW_W || h != DVR_PREVIEW_H) {
        LV_LOG_WARN("dvr preview size %ux%u != %dx%d", (unsigned)w, (unsigned)h,
                    (int)DVR_PREVIEW_W, (int)DVR_PREVIEW_H);
        return;
    }

    /* 源是 RGBA8888（R,G,B,A 字节序），LVGL 的 ARGB8888 在内存里是 B,G,R,A
     * （lv_color32_t = {blue,green,red,alpha}），所以换 R 与 B 两个字节，
     * alpha 原样带过去。整帧一次线性搬运，不做逐像素函数调用。 */
    for (y = 0; y < h; y++) {
        const uint8_t *src = rgba + (size_t)y * w * 4;
        uint8_t       *row = dst + (size_t)y * DVR_PREVIEW_W * 4;

        for (x = 0; x < w; x++) {
            row[x * 4 + 0] = src[x * 4 + 2];   /* B */
            row[x * 4 + 1] = src[x * 4 + 1];   /* G */
            row[x * 4 + 2] = src[x * 4 + 0];   /* R */
            row[x * 4 + 3] = src[x * 4 + 3];   /* A */
        }
    }

    lv_obj_add_flag(preview_hint, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(preview_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(preview_canvas);
}

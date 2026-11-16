/*
 * dvr_page.c --- DVR 页面容器（1024x600）
 *
 * 控件树/坐标/尺寸/图片沿用既有 dvr_ui 版式，不自行调整。
 *
 * 视图分 6 块（同一 screen 里切显示，不换 screen）：
 *   main_view   预览 1024x500 + dock 1024x100（4 个图标按钮）
 *   cam_dock    主视图底部的"摄头切换"条（前/后/抓拍），不单独占一个 view
 *   idle_view   待机页：选前/后摄头再进列表
 *   list_view   文件列表（6 行 600x60）+ 底部前/后页签
 *   set_view    设置 4 行 910x80
 *   popup_view  确认弹窗 500x270
 *
 * 字体：c_default_28/30/32/33 为包含本页中文的子集字体（tools/gen_font_subset.js 生成），
 * 注册在 aic_get_c_font()，中英文共用同一套字形。
 *
 * 文案：全部走 aic_get_translation()，中文/英文随系统语言切换。
 * 改了中文文案要重跑字体生成脚本。
 */
#include "ui_objects.h"
#include "aic_ui.h"
#include "aic_lang.h"
#include "dvr_view.h"
#include "dvr_logic.h"

/* ---------------- 字体 ---------------- */

/* 字号 -> 本工程可用字体 */
static const lv_font_t *dvr_font(int font_size)
{
    /* 28/30/32/33 为包含中文的子集字体（见 aic_get_c_font / tools/gen_font_subset.js） */
    const lv_font_t *f = aic_get_multi_lang_font(font_size);

    if (f != NULL) {
        return f;
    }
#if LV_FONT_MONTSERRAT_28 && LV_FONT_MONTSERRAT_30 && LV_FONT_MONTSERRAT_32
    /* 兜底：C 数组字体没编进来时（HCN_APPLICATION_USE_C_FONTS=OFF）用 LVGL 自带西文字体 */
    switch (font_size) {
    case 28: f = &lv_font_montserrat_28; break;
    case 30: f = &lv_font_montserrat_30; break;
    default: f = &lv_font_montserrat_32; break;
    }
#endif
    if (f == NULL) {
        f = aic_get_multi_lang_font(18);
    }
    if (f == NULL) {
        f = LV_FONT_DEFAULT;
    }
    return f;
}

/* ---------------- 建控件的小工具 ---------------- */

static void on_click(lv_event_t *e);

/* 透明布局容器：无边框/无内边距/不滚动 */
static lv_obj_t *dvr_panel(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *p = lv_obj_create(parent);

    lv_obj_set_pos(p, x, y);
    lv_obj_set_size(p, w, h);
    lv_obj_set_style_bg_opa(p, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(p, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(p, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(p, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);

    return p;
}

/* 图片（尺寸按 XML 给；图片原始尺寸不同的话由调用方另设 inner_align） */
static lv_obj_t *dvr_img(lv_obj_t *parent, int x, int y, int w, int h, const char *rel)
{
    lv_obj_t *o = lv_image_create(parent);

    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_image_set_src(o, dvr_view_asset(rel));

    return o;
}

/* 居中标签：按内容尺寸建控件，再按给定盒子水平居中。
 * 盒子只作为锚点，文本不裁剪。 */
static lv_obj_t *dvr_label_mid(lv_obj_t *parent, int x, int y, int w, int h,
                               const lv_font_t *font, uint32_t color, const char *text)
{
    lv_obj_t *lab = lv_label_create(parent);

    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lab, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lab, text);

    lv_obj_update_layout(lab);
    lv_obj_set_pos(lab, x + (w - lv_obj_get_width(lab)) / 2,
                        y + (h - lv_obj_get_height(lab)) / 2);
    return lab;
}

/* 固定盒子 + 水平垂直都居中（文字会变的场景，省掉每次重排）。
 * pad_top 做垂直居中：标签默认贴顶画。 */
static lv_obj_t *dvr_label_fix(lv_obj_t *parent, int x, int y, int w, int h,
                               const lv_font_t *font, uint32_t color, const char *text)
{
    lv_obj_t *lab = lv_label_create(parent);
    int       lh = lv_font_get_line_height(font);

    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lab, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(lab, w, h);
    lv_obj_set_pos(lab, x, y);
    if (h > lh) {
        lv_obj_set_style_pad_top(lab, (h - lh) / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_label_set_long_mode(lab, LV_LABEL_LONG_CLIP);
    lv_label_set_text(lab, text);

    return lab;
}

/* 左对齐 + 固定盒子 + 超出裁剪（文件名/版本号这类长度不可控的文本） */
static lv_obj_t *dvr_label_left(lv_obj_t *parent, int x, int y, int w, int h,
                                const lv_font_t *font, uint32_t color, const char *text)
{
    lv_obj_t *lab = lv_label_create(parent);
    int       lh = lv_font_get_line_height(font);

    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lab, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lab, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(lab, w, h);
    lv_obj_set_pos(lab, x, y + (h - lh) / 2);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_CLIP);
    lv_label_set_text(lab, text);

    return lab;
}

/* 点击回调：area/index 打包进 user_data */
static void dvr_clickable(lv_obj_t *obj, dvr_click_e area, int index)
{
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, on_click, LV_EVENT_CLICKED,
                        (void *)(lv_uintptr_t)(((int)area << 8) | (index & 0xff)));
}

static void on_click(lv_event_t *e)
{
    lv_uintptr_t v = (lv_uintptr_t)lv_event_get_user_data(e);

    lv_event_stop_bubbling(e);   /* 子控件点了不要再冒泡给整行/整页 */
    dvr_logic_click((dvr_click_e)((v >> 8) & 0xff), (int)(v & 0xff));
}

/* ---------------- 主视图 + 摄头切换 dock ---------------- */

static void dvr_build_main(dvr_page_t *s)
{
    static const char *dock_n[4] = { "icon_camera_n.png", "icon_playback_n.png",
                                     "icon_photo_n.png",  "icon_settings_n.png" };
    int i;

    s->main_view = dvr_panel(s->obj, 0, 0, DVR_PAGE_W, DVR_PAGE_H);

    /* 底图：没帧时显示（预览画布盖在它上面） */
    s->main_bg = dvr_img(s->main_view, 0, 0, DVR_PREVIEW_W, DVR_PREVIEW_H, "dvr_bg.png");

    /* 预览画布（软解路径贴像素用）；硬解是 overlay 直铺，不经过这里 */
    s->preview_canvas = lv_canvas_create(s->main_view);
    lv_canvas_set_buffer(s->preview_canvas, dvr_view_preview_buffer(),
                         DVR_PREVIEW_W, DVR_PREVIEW_H, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_pos(s->preview_canvas, 0, 0);
    lv_obj_add_flag(s->preview_canvas, LV_OBJ_FLAG_HIDDEN);
    dvr_clickable(s->preview_canvas, DVR_CLICK_PREVIEW, 0);

    /* 无帧时的一行提示（离线/启动中/硬解直铺），居中压在预览区上 */
    s->preview_hint = dvr_label_fix(s->main_view, 0, DVR_PREVIEW_H / 2 - 30,
                                    DVR_PREVIEW_W, 60, dvr_font(30), DVR_COLOR_TEXT, "");
    lv_obj_add_flag(s->preview_hint, LV_OBJ_FLAG_HIDDEN);

    /* 主 dock：4 个 160x100 图标 + 3 条 1x100 分隔线 */
    s->main_dock = dvr_panel(s->main_view, 0, DVR_DOCK_Y, DVR_PAGE_W, DVR_DOCK_H);
    dvr_img(s->main_dock, 0, 0, DVR_PAGE_W, DVR_DOCK_H, "dock_bg.png");
    for (i = 0; i < 4; i++) {
        s->dock_icon[i] = dvr_img(s->main_dock, DVR_DOCK_BTN_X(i), 0,
                                  DVR_DOCK_BTN_W, DVR_DOCK_BTN_H, dock_n[i]);
        dvr_clickable(s->dock_icon[i], DVR_CLICK_DOCK, i);
        if (i < 3) {
            dvr_img(s->main_dock, DVR_DOCK_LINE_X(i), 0, 1, DVR_DOCK_H, "line.png");
        }
    }

    /* 摄头切换 dock（占主视图底部同一块位置，与 main_dock 互斥显示） */
    s->cam_dock = dvr_panel(s->main_view, 0, DVR_DOCK_Y, DVR_PAGE_W, DVR_DOCK_H);
    dvr_img(s->cam_dock, 0, 0, DVR_PAGE_W, DVR_DOCK_H, "dock_bg.png");
    s->cam_tab_sel = dvr_img(s->cam_dock, 0, 0, DVR_CAM_TAB_W, DVR_DOCK_H, "dock_selected.png");
    dvr_img(s->cam_dock, DVR_CAM_TAB_W, 0, 1, DVR_DOCK_H, "line.png");
    dvr_img(s->cam_dock, DVR_CAM_TAB_W * 2, 0, 1, DVR_DOCK_H, "line.png");
    s->cam_front = dvr_label_mid(s->cam_dock, 86, 19, 168, 61, dvr_font(30),
                                 DVR_COLOR_WHITE, aic_get_translation(dvr_front_video));
    dvr_clickable(s->cam_front, DVR_CLICK_CAM, 0);
    s->cam_rear = dvr_label_mid(s->cam_dock, 427, 19, 168, 61, dvr_font(30),
                                DVR_COLOR_TEXT, aic_get_translation(dvr_rear_video));
    dvr_clickable(s->cam_rear, DVR_CLICK_CAM, 1);
    s->cam_snap = dvr_img(s->cam_dock, DVR_CAM_SNAP_X, DVR_CAM_SNAP_Y,
                          DVR_CAM_SNAP_W, DVR_CAM_SNAP_H, "icon_photo_n.png");
    /* 按比例缩放到 80x80 盒内 */
    lv_image_set_inner_align(s->cam_snap, LV_IMAGE_ALIGN_CONTAIN);
    dvr_clickable(s->cam_snap, DVR_CLICK_CAM, 2);

    lv_obj_add_flag(s->cam_dock, LV_OBJ_FLAG_HIDDEN);
}

/* ---------------- 待机页（选前/后摄头） ---------------- */

static void dvr_build_idle(dvr_page_t *s)
{
    s->idle_view = dvr_panel(s->obj, 0, 0, DVR_PAGE_W, DVR_PAGE_H);

    dvr_img(s->idle_view, 0, 0, DVR_PREVIEW_W, DVR_PREVIEW_H, "dvr_bg.png");

    s->idle_dock = dvr_panel(s->idle_view, 0, DVR_DOCK_Y, DVR_PAGE_W, DVR_DOCK_H);
    dvr_img(s->idle_dock, 0, 0, DVR_PAGE_W, DVR_DOCK_H, "dock_bg.png");
    s->idle_tab_sel = dvr_img(s->idle_dock, 0, 0, DVR_TAB_SEL_W, DVR_DOCK_H, "dock_selected.png");
    dvr_img(s->idle_dock, DVR_TAB_SEL_W, 0, 1, DVR_DOCK_H, "line.png");
    s->idle_tab_left = dvr_label_fix(s->idle_dock, 156, 19, 200, 61, dvr_font(30),
                                     DVR_COLOR_WHITE, aic_get_translation(dvr_front_video));
    dvr_clickable(s->idle_tab_left, DVR_CLICK_TAB_LEFT, 0);
    s->idle_tab_right = dvr_label_fix(s->idle_dock, 668, 19, 200, 61, dvr_font(30),
                                      DVR_COLOR_TEXT, aic_get_translation(dvr_rear_video));
    dvr_clickable(s->idle_tab_right, DVR_CLICK_TAB_RIGHT, 0);

    lv_obj_add_flag(s->idle_view, LV_OBJ_FLAG_HIDDEN);
}

/* ---------------- 文件列表 ---------------- */

static void dvr_build_list(dvr_page_t *s)
{
    int i;

    s->list_view = dvr_panel(s->obj, 0, 0, DVR_PAGE_W, DVR_PAGE_H);
    dvr_img(s->list_view, 0, 0, DVR_PAGE_W, DVR_PAGE_H, "dvr_list_bg.png");

    s->file_scroll = dvr_panel(s->list_view, DVR_LIST_X, DVR_LIST_Y, DVR_LIST_W, DVR_LIST_H);

    for (i = 0; i < DVR_LIST_ROW_N; i++) {
        lv_obj_t *row = dvr_panel(s->file_scroll, 0, i * DVR_LIST_ROW_STEP,
                                  DVR_LIST_ROW_W, DVR_LIST_ROW_H);

        dvr_clickable(row, DVR_CLICK_ROW, i);

        s->row_bg[i] = dvr_img(row, 0, 0, DVR_LIST_ROW_W, DVR_LIST_ROW_H, "list_bg_n.png");
        s->row_icon[i] = dvr_img(row, 10, 0, 60, 60, "list_icon_video_n.png");
        s->row_name[i] = dvr_label_left(row, 80, 12, 320, 35, dvr_font(28),
                                        DVR_COLOR_TEXT, "");
        s->row_view[i] = dvr_img(row, 390, 0, 100, 60, "icon_view_n.png");
        dvr_clickable(s->row_view[i], DVR_CLICK_ROW_PLAY, i);
        s->row_del[i] = dvr_img(row, 500, 0, 100, 60, "icon_delete_n.png");
        dvr_clickable(s->row_del[i], DVR_CLICK_ROW_DEL, i);
    }

    s->list_no_sd = dvr_label_mid(s->list_view, 362, 200, 300, 60, dvr_font(32),
                                  DVR_COLOR_WHITE, aic_get_translation(dvr_no_sd));
    lv_obj_add_flag(s->list_no_sd, LV_OBJ_FLAG_HIDDEN);

    s->list_dock = dvr_panel(s->list_view, 0, DVR_DOCK_Y, DVR_PAGE_W, DVR_DOCK_H);
    dvr_img(s->list_dock, 0, 0, DVR_PAGE_W, DVR_DOCK_H, "dock_bg.png");
    s->list_tab_sel = dvr_img(s->list_dock, 0, 0, DVR_TAB_SEL_W, DVR_DOCK_H, "dock_selected.png");
    dvr_img(s->list_dock, DVR_TAB_SEL_W, 0, 1, DVR_DOCK_H, "line.png");
    s->list_tab_left = dvr_label_fix(s->list_dock, 156, 19, 200, 61, dvr_font(30),
                                     DVR_COLOR_WHITE, aic_get_translation(dvr_front_video));
    dvr_clickable(s->list_tab_left, DVR_CLICK_TAB_LEFT, 0);
    s->list_tab_right = dvr_label_fix(s->list_dock, 668, 19, 200, 61, dvr_font(30),
                                      DVR_COLOR_TEXT, aic_get_translation(dvr_rear_video));
    dvr_clickable(s->list_tab_right, DVR_CLICK_TAB_RIGHT, 0);

    lv_obj_add_flag(s->list_view, LV_OBJ_FLAG_HIDDEN);
}

/* 设置行里的"标签列"底板：未选中 30% 棕，选中橙棕（两套图） */
static void dvr_set_row_label(dvr_page_t *s, int r, const char *text)
{
    lv_obj_t *row = dvr_panel(s->set_view, DVR_SET_X, DVR_SET_Y(r), DVR_SET_W, DVR_SET_H);

    dvr_clickable(row, DVR_CLICK_SET_ROW, r);

    s->set_row_label_bg[r] = dvr_img(row, 0, 0, 250, DVR_SET_H, "settings_btn1_n.png");
    s->set_row_lab[r] = dvr_label_mid(row, 65, 18, 120, 43, dvr_font(30), DVR_COLOR_TEXT, text);

    s->set_row[r] = row;
}

static void dvr_build_setting(dvr_page_t *s)
{
    int i;
    const char *loop_txt[3] = { aic_get_translation(dvr_loop_1min),
                                aic_get_translation(dvr_loop_2min),
                                aic_get_translation(dvr_loop_3min) };

    s->set_view = dvr_panel(s->obj, 0, 0, DVR_PAGE_W, DVR_PAGE_H);
    dvr_img(s->set_view, 0, 0, DVR_PAGE_W, DVR_PAGE_H, "dvr_list_bg.png");

    /* 行 0：版本 */
    dvr_set_row_label(s, DVR_SET_ROW_VERSION, aic_get_translation(dvr_version));
    dvr_img(s->set_row[0], 270, 0, 640, DVR_SET_H, "settings_btn3_n.png");
    s->set_ver_text = dvr_label_left(s->set_row[0], 310, 18, 560, 43, dvr_font(28),
                                     DVR_COLOR_TEXT, "--");

    /* 行 1：循环时长（3 个候选 + 选中标记） */
    dvr_set_row_label(s, DVR_SET_ROW_LOOP, aic_get_translation(dvr_loop_time));
    for (i = 0; i < 3; i++) {
        dvr_img(s->set_row[1], 270 + i * 220, 0, 200, DVR_SET_H, "settings_btn2_n.png");
        s->set_loop_lab[i] = dvr_label_mid(s->set_row[1], 303 + i * 220, 18, 80, 43,
                                           dvr_font(30), DVR_COLOR_TEXT, loop_txt[i]);
        dvr_clickable(s->set_loop_lab[i], DVR_CLICK_SET_LOOP, i);
    }
    s->set_loop_sel = dvr_img(s->set_row[1], 278, 28, 24, 24, "settings_s.png");

    /* 行 2：格式化 / 恢复出厂 */
    dvr_set_row_label(s, DVR_SET_ROW_FORMAT, aic_get_translation(dvr_format));
    dvr_img(s->set_row[2], 270, 0, 310, DVR_SET_H, "settings_btn2w_n.png");
    dvr_img(s->set_row[2], 600, 0, 310, DVR_SET_H, "settings_btn2w_n.png");
    s->set_fmt_sd_lab = dvr_label_mid(s->set_row[2], 340, 18, 170, 43, dvr_font(28),
                                      DVR_COLOR_TEXT, aic_get_translation(dvr_quick_format));
    dvr_clickable(s->set_fmt_sd_lab, DVR_CLICK_SET_FMT, 0);
    s->set_fmt_rst_lab = dvr_label_mid(s->set_row[2], 670, 18, 170, 43, dvr_font(28),
                                       DVR_COLOR_TEXT, aic_get_translation(dvr_factory_reset));
    dvr_clickable(s->set_fmt_rst_lab, DVR_CLICK_SET_FMT, 1);
    s->set_fmt_sel = dvr_img(s->set_row[2], 285, 28, 24, 24, "settings_s.png");

    /* 行 3：SD 卡容量（进度条 = 白底 storage_n + 蓝条 storage_p 按比例裁） */
    dvr_set_row_label(s, DVR_SET_ROW_ABOUT, aic_get_translation(dvr_sd_card));
    dvr_img(s->set_row[3], 270, 0, 640, DVR_SET_H, "settings_btn3_n.png");
    dvr_img(s->set_row[3], 300, 30, 360, 20, "storage_n.png");
    s->set_storage_fg = dvr_panel(s->set_row[3], 300, 30, 0, 20);   /* 宽度随用量变 */
    dvr_img(s->set_storage_fg, 0, 0, 360, 20, "storage_p.png");
    s->set_storage_text = dvr_label_left(s->set_row[3], 683, 19, 200, 43, dvr_font(28),
                                         DVR_COLOR_TEXT, "--/--");

    lv_obj_add_flag(s->set_view, LV_OBJ_FLAG_HIDDEN);
}

/* ---------------- 确认弹窗 ---------------- */

static void dvr_build_popup(dvr_page_t *s)
{
    lv_obj_t *row;

    s->popup_view = dvr_panel(s->obj, DVR_POPUP_X, DVR_POPUP_Y, DVR_POPUP_W, DVR_POPUP_H);
    dvr_img(s->popup_view, 0, 0, DVR_POPUP_W, DVR_POPUP_H, "pop_up_bg.png");

    s->popup_title = dvr_label_mid(s->popup_view, 170, 60, 160, 46, dvr_font(33),
                                   DVR_COLOR_TEXT, aic_get_translation(dvr_confirm_delete));
    s->popup_loading = dvr_img(s->popup_view, 210, 90, 80, 80, "dvr_loading_0.png");
    lv_obj_add_flag(s->popup_loading, LV_OBJ_FLAG_HIDDEN);

    row = s->popup_view;
    s->popup_ok_bg = dvr_img(row, 50, 170, 180, 60, "pop_up_btn_n.png");
    s->popup_ok = dvr_label_mid(row, 112, 179, 56, 41, dvr_font(30), DVR_COLOR_TEXT, aic_get_translation(dvr_confirm));
    s->popup_cancel_bg = dvr_img(row, 270, 170, 180, 60, "pop_up_btn_n.png");
    s->popup_cancel = dvr_label_mid(row, 332, 179, 56, 41, dvr_font(30), DVR_COLOR_TEXT, aic_get_translation(dvr_cancel));
    dvr_clickable(s->popup_ok_bg, DVR_CLICK_POPUP_OK, 0);
    dvr_clickable(s->popup_cancel_bg, DVR_CLICK_POPUP_CANCEL, 0);

    lv_obj_add_flag(s->popup_view, LV_OBJ_FLAG_HIDDEN);
}

/* ---------------- 对外接口 ---------------- */

void dvr_page_create(ui_manager_t *ui)
{
    dvr_page_t *scr = dvr_page_get(ui);

    if (!ui->auto_del && scr->obj) {
        dvr_page_update_current_language();
        return;
    }

    /* 页面比显示屏还大就必定被裁切（PC 模拟器忘了设 HCN_APPLICATION_SCREEN 时最容易踩） */
    {
        int32_t disp_w = lv_display_get_horizontal_resolution(NULL);
        int32_t disp_h = lv_display_get_vertical_resolution(NULL);

        if (disp_w < DVR_PAGE_W || disp_h < DVR_PAGE_H) {
            lv_log("[DVR-UI] WARN: display %dx%d < page %dx%d, "
                   "页面会被裁切；PC 上请设 HCN_APPLICATION_SCREEN=%dx%d\n",
                   (int)disp_w, (int)disp_h, DVR_PAGE_W, DVR_PAGE_H, DVR_PAGE_W, DVR_PAGE_H);
        }
    }

    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(scr->obj, DVR_PAGE_W, DVR_PAGE_H);
    lv_obj_set_pos(scr->obj, 0, 0);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(scr->obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(scr->obj, LV_OBJ_FLAG_SCROLLABLE);

    dvr_view_init();
    dvr_build_main(scr);
    dvr_build_idle(scr);
    dvr_build_list(scr);
    dvr_build_setting(scr);
    dvr_build_popup(scr);

    lv_log("[DVR-UI] page created at %dx%d\n", DVR_PAGE_W, DVR_PAGE_H);

    dvr_page_init();   /* 建定时器 + 拉状态 + 起预览（dvr_logic.c） */
}

void dvr_page_update_language(lang_type_t lang)
{
    aic_set_language(lang);
    dvr_page_update_current_language();
}

void dvr_page_update_current_language(void)
{
    dvr_page_t *scr = dvr_page_get(&ui_manager);

    if (!scr || !scr->obj) {
        return;
    }

    /* 本页静态文案（摄头页签/设置行/格式化/弹窗按钮/无卡提示）重刷；
     * 页签的视频/照片、打开中的弹窗标题这些"带状态"的文案在 dvr_logic 里重刷。
     * 实际上语言只在设置页改，而那时本页已经销毁重建，这里是防御性重刷。 */
    dvr_view_apply_language();
    dvr_logic_update_language();
}

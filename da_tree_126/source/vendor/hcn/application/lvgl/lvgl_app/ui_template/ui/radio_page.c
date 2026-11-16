/*
 * radio_page.c --- 收音机页面容器（1024x600，黑底）
 *
 * 设计尺寸 1024x600，分区如下：
 *   顶栏   : 标题 | 信号强度 | 搜台状态 | ST/MONO | RDS/TP/TA
 *   第二行 : 台号 | PS 台名 | RT 电台文本
 *   第三行 : 大字频率 + 单位(MHz/KHz)，左右各三个调谐按钮
 *   第四行 : 横向分页的预置台列表，整行居中（一屏 6 个 = 一页，18 个台分 3 页
 *            1-6/7-12/13-18；按页吸附翻页，当前台所在的页会自动显示）
 *   第五行 : 波段 / AS / PLAY / STOP / MUTE / LOCAL 六个控制按钮
 *
 * 字体：正文用 aic_get_multi_lang_font()（当前语言的 C 字体只有 10/11/12/16/18/41 号）；
 *       台号/频率是纯数字，用 aic_get_lang_font(SPEEDLABEL_TTF, n, true)（c_speedlabel_36/90），
 *       与仪表页的车速用同一套字体，数字更粗更醒目。
 *
 * 样式：全部用 LVGL 控件默认样式（默认主题是浅色，页面是黑底，所以文字颜色显式给白色系；
 * 配色宏见 radio_view.h）。
 *
 * 布局避让：custom.c 在 lv_layer_top() 上叠了 4 个模拟器虚拟按键
 * （上/下/左/右 四个边的中点，约 44x26），它们是"实体按键"的触点，
 * 所以本页在这四块区域留白：
 *   - 上：y<30 且 x 在 470~554 之间留空（顶栏右侧的图标从 x=560 才开始排）；
 *   - 左/右：y 在 270~330 之间，x<50 / x>974 留空（调谐按钮排在 56~964）；
 *   - 下：y>540 留空（控制按钮行结束于 536）。
 *
 * 模拟器窗口尺寸：默认 1024x600（见 port/hcn_application_entry.c 的 HCN_APPLICATION_SCREEN），
 * 要和本页一致才能看到整页；仪表页(480x272)用 HCN_APPLICATION_SCREEN=480x272 切回去看。
 */
#include "ui_objects.h"
#include "aic_ui.h"
#include "aic_lang.h"
#include "radio_view.h"
#include "radio_logic.h"

/* 页面设计尺寸 */
#define RADIO_PAGE_W  1024
#define RADIO_PAGE_H  600

/* 预置列表容器的左右内边距，与 RADIO_PRESET_VISIBLE 一起决定容器宽度：
 * 一屏正好放下 RADIO_PRESET_VISIBLE 个台 = 一页，其余台按页翻(宏见 radio_view.h) */
#define RADIO_PRESET_PAD  4
#define RADIO_PRESET_ROW_W                                       \
    (RADIO_PRESET_VISIBLE * RADIO_PRESET_ITEM_W +                \
     (RADIO_PRESET_VISIBLE - 1) * RADIO_PRESET_ITEM_GAP + 2 * RADIO_PRESET_PAD)

/* ---------------- 建控件的两个小工具 ---------------- */

static lv_obj_t *radio_label(lv_obj_t *parent, int x, int y, int w, int h,
                             const lv_font_t *font, uint32_t color, lv_text_align_t align)
{
    lv_obj_t *lab = lv_label_create(parent);

    lv_obj_set_pos(lab, x, y);
    lv_obj_set_size(lab, w, h);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(lab, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lab, align, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);

    return lab;
}

/* 数字专座：台号 / 频率用 speedLabel 字体（纯数字，和正文分开取字体） */
static lv_obj_t *radio_num_label(lv_obj_t *parent, int x, int y, int w, int h,
                                 int font_size, uint32_t color, lv_text_align_t align)
{
    return radio_label(parent, x, y, w, h,
                       aic_get_lang_font(SPEEDLABEL_TTF, font_size, true), color, align);
}

static void on_radio_button_clicked(lv_event_t *e)
{
    radio_action_e action = (radio_action_e)(lv_uintptr_t)lv_event_get_user_data(e);

    radio_logic_action(action);
}

/* out_label 非空时回传按钮里的文字控件（波段/静音/本地按钮的文案会变） */
static lv_obj_t *radio_button(lv_obj_t *parent, const char *text, int x, int y, int w, int h,
                              const lv_font_t *font, radio_action_e action, lv_obj_t **out_label)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *lab = NULL;

    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);

    lab = lv_label_create(btn);
    lv_label_set_text(lab, text);
    lv_obj_center(lab);
    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(btn, on_radio_button_clicked, LV_EVENT_CLICKED, (void *)(lv_uintptr_t)action);

    if (out_label) {
        *out_label = lab;
    }
    return btn;
}

void radio_page_create(ui_manager_t *ui)
{
    radio_page_t *scr = radio_page_get(ui);
    const lv_font_t *f12 = aic_get_multi_lang_font(12);
    const lv_font_t *f16 = aic_get_multi_lang_font(16);
    const lv_font_t *f18 = aic_get_multi_lang_font(18);

    if (!ui->auto_del && scr->obj) {
        radio_page_update_current_language();
        return;
    }

    /* 页面比显示屏还大就必定被裁切（PC 模拟器忘了设 HCN_APPLICATION_SCREEN 时最容易踩） */
    {
        int32_t disp_w = lv_display_get_horizontal_resolution(NULL);
        int32_t disp_h = lv_display_get_vertical_resolution(NULL);

        if (disp_w < RADIO_PAGE_W || disp_h < RADIO_PAGE_H) {
            lv_log("[RADIO-UI] WARN: display %dx%d < page %dx%d, " \
                   "页面会被裁切；PC 上请设 HCN_APPLICATION_SCREEN=%dx%d\n",
                   (int)disp_w, (int)disp_h, RADIO_PAGE_W, RADIO_PAGE_H, RADIO_PAGE_W, RADIO_PAGE_H);
        }
    }

    // Init scr->obj
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(scr->obj, RADIO_PAGE_W, RADIO_PAGE_H);
    lv_obj_set_pos(scr->obj, 0, 0);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(scr->obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ---------- 顶栏：标题 | 信号 | 搜台状态 | ST/MONO | RDS/TP/TA ----------
    // 中间 x=470..554 让给模拟器的"上键"虚拟按键，右侧图标从 x=560 才开始排。
    scr->title = radio_label(scr->obj, 12, 10, 140, 28, f18, RADIO_COLOR_TEXT, LV_TEXT_ALIGN_LEFT);
    scr->level_hint = radio_label(scr->obj, 160, 12, 40, 24, f12, RADIO_COLOR_DIM, LV_TEXT_ALIGN_LEFT);
    lv_label_set_text(scr->level_hint, "SIG");

    scr->level_bar = lv_bar_create(scr->obj);
    lv_obj_set_pos(scr->level_bar, 204, 16);
    lv_obj_set_size(scr->level_bar, 100, 16);
    lv_bar_set_range(scr->level_bar, 0, 100);
    lv_bar_set_value(scr->level_bar, 0, LV_ANIM_OFF);

    scr->level_value = radio_label(scr->obj, 312, 10, 76, 28, f12, RADIO_COLOR_TEXT, LV_TEXT_ALIGN_LEFT);
    scr->scan_state = radio_label(scr->obj, 560, 10, 170, 28, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_LEFT);
    scr->stereo = radio_label(scr->obj, 740, 10, 76, 28, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_CENTER);
    scr->rds = radio_label(scr->obj, 820, 10, 60, 28, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_CENTER);
    scr->tp = radio_label(scr->obj, 884, 10, 54, 28, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_CENTER);
    scr->ta = radio_label(scr->obj, 944, 10, 54, 28, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_CENTER);

    // ---------- 台号 | PS 台名 | RT 文本 ----------
    scr->station_no = radio_num_label(scr->obj, 200, 56, 88, 48, 36, RADIO_COLOR_STATION, LV_TEXT_ALIGN_CENTER);
    scr->ps = radio_label(scr->obj, 300, 62, 180, 36, f18, RADIO_COLOR_TEXT, LV_TEXT_ALIGN_LEFT);
    scr->rt = radio_label(scr->obj, 496, 66, 496, 32, f16, RADIO_COLOR_DIM, LV_TEXT_ALIGN_RIGHT);

    // ---------- 居中大字频率 + 单位 ----------
    scr->freq = radio_num_label(scr->obj, 180, 112, 664, 176, 90, RADIO_COLOR_TEXT, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(scr->freq, "--");
    scr->unit = radio_label(scr->obj, 452, 292, 120, 40, f18, RADIO_COLOR_DIM, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(scr->unit, "");

    // ---------- 调谐按钮：频率左右各三个（左列 x=56、右列 x=868，避开左右虚拟按键） ----------
    scr->btn_prev = radio_button(scr->obj, "PREV", 56, 120, 96, 48, f16, RADIO_ACTION_PREV, NULL);
    scr->btn_step_down = radio_button(scr->obj, "STEP-", 56, 184, 96, 48, f16, RADIO_ACTION_STEP_DOWN, NULL);
    scr->btn_seek_down = radio_button(scr->obj, "SEEK-", 56, 248, 96, 48, f16, RADIO_ACTION_SEEK_DOWN, NULL);
    scr->btn_next = radio_button(scr->obj, "NEXT", 868, 120, 96, 48, f16, RADIO_ACTION_NEXT, NULL);
    scr->btn_step_up = radio_button(scr->obj, "STEP+", 868, 184, 96, 48, f16, RADIO_ACTION_STEP_UP, NULL);
    scr->btn_seek_up = radio_button(scr->obj, "SEEK+", 868, 248, 96, 48, f16, RADIO_ACTION_SEEK_UP, NULL);

    // ---------- 预置台：横向分页列表（一屏 RADIO_PRESET_VISIBLE 个 = 一页，整行居中） ----------
    scr->preset_cont = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->preset_cont, (RADIO_PAGE_W - RADIO_PRESET_ROW_W) / 2, 414);
    lv_obj_set_size(scr->preset_cont, RADIO_PRESET_ROW_W, 56);
    lv_obj_set_style_bg_opa(scr->preset_cont, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->preset_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(scr->preset_cont, RADIO_PRESET_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(scr->preset_cont, RADIO_PRESET_ITEM_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(scr->preset_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scr->preset_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(scr->preset_cont, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(scr->preset_cont, LV_SCROLLBAR_MODE_OFF);
    /* 按页吸附：吸附点是每页第一个台(见 radio_view.c 只给 0/6/12 留 SNAPPABLE)，
     * 松手后落到最近的那一页 —— 拖过半页就翻页，不到半页自动回正。 */
    lv_obj_set_scroll_snap_x(scr->preset_cont, LV_SCROLL_SNAP_START);

    // ---------- 底部控制按钮：波段 / AS / PLAY / STOP / MUTE / LOCAL ----------
    scr->band_btn = radio_button(scr->obj, "FM-1", 8, 480, 160, 56, f16, RADIO_ACTION_BAND, &scr->band_lab);
    scr->btn_as = radio_button(scr->obj, "AS", 176, 480, 160, 56, f16, RADIO_ACTION_AS, NULL);
    scr->btn_ps_play = radio_button(scr->obj, "PLAY", 344, 480, 160, 56, f16, RADIO_ACTION_PS_PLAY, NULL);
    scr->btn_ps_stop = radio_button(scr->obj, "STOP", 512, 480, 160, 56, f16, RADIO_ACTION_PS_STOP, NULL);
    scr->btn_mute = radio_button(scr->obj, "MUTE:OFF", 680, 480, 160, 56, f16, RADIO_ACTION_MUTE, &scr->mute_lab);
    scr->btn_local = radio_button(scr->obj, "DX", 848, 480, 160, 56, f16, RADIO_ACTION_LOCAL, &scr->local_lab);

    lv_log("[RADIO-UI] page created at %dx%d\n", RADIO_PAGE_W, RADIO_PAGE_H);

    radio_page_init();
}

void radio_page_update_language(lang_type_t lang)
{
    aic_set_language(lang);
    radio_page_update_current_language();
}

void radio_page_update_current_language()
{
    radio_page_t *scr = radio_page_get(&ui_manager);
    if (!scr || !scr->obj)
        return;

    /* 本页文案目前用 ASCII：ui/font 下的 C 数组字体只包含既有 UI 用到的汉字子集，
     * 没有"收音机/上一台/搜台/波段/静音"等字形，直接显示中文会整串不渲染。
     * 重新生成字体子集后，可把这里换成 aic_get_translation(xxx)。 */
    lv_obj_set_style_text_font(scr->title, aic_get_multi_lang_font(18), LV_PART_MAIN | LV_STATE_DEFAULT);
    radio_view_refresh_title();
}

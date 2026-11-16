/*
 * radio_view.c --- 收音机页面视图层实现
 *
 * 只做"数据 -> 控件"的搬运。数据由 radio_logic.c 经 vehicle_services 的 vs_radio_*
 * 门面取回；本文件只 include vehicle_services.h，用其中的 VS_RADIO_BAND_IS_FM /
 * VS_RADIO_BAND_INDEX / VS_RADIO_ICON_* 契约宏做显示判断，不调任何中间件函数。
 *
 * 注意：VIEW_MEMBER_VALID 会在函数里声明与控件成员同名的局部变量（如 freq/unit），
 * 所以本文件的函数形参一律加 _text / _khz 之类的后缀，避免重名。
 */
#include "radio_view.h"
#include "definition.h"
#include "vehicle_services.h"

#include <stdio.h>
#include <string.h>

/* 预置台按钮尺寸/一屏可见个数见 radio_view.h 的 RADIO_PRESET_* 宏 */

static radio_view_preset_cb_t s_preset_cb = NULL;

/* 上次高亮的预置下标：只在高亮项变化时才滚入可视区，
 * 避免每 200ms 刷新都把用户手动滑走的列表拉回来。 */
static int s_highlight = -1;

/* 上次刷新的波段值：只在波段变化/异常时打一行日志，便于定位"波段文案不对"这类问题 */
static int s_last_band = -1;

void radio_view_set_preset_cb(radio_view_preset_cb_t cb)
{
    s_preset_cb = cb;
}

/* ---------- 小工具 ---------- */

static void set_text_color(lv_obj_t *obj, uint32_t color)
{
    lv_obj_set_style_text_color(obj, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* 频率格式化：FM 显示 "100.1" + MHz，AM 显示 "756" + KHz */
static void format_freq(int freq_khz, char *buf, size_t size, const char **unit)
{
    if (freq_khz <= 0) {
        snprintf(buf, size, "--");
        *unit = "";
    } else if (freq_khz > 60000) {
        snprintf(buf, size, "%d.%d", freq_khz / 1000, (freq_khz % 1000) / 100);
        *unit = "MHz";
    } else {
        snprintf(buf, size, "%d", freq_khz);
        *unit = "KHz";
    }
}

/* 预置台按钮点击 -> 通知 logic（user_data 里是按钮下标） */
static void on_preset_clicked(lv_event_t *e)
{
    int index = (int)(lv_uintptr_t)lv_event_get_user_data(e);

    if (s_preset_cb) {
        s_preset_cb(index);
    }
}

/* ---------- 顶栏 ---------- */

void radio_view_refresh_title(void)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, title);

    lv_label_set_text(title, "Radio");
}

void radio_view_refresh_level(int level)
{
    char text[16];
    radio_page_t *page = radio_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, level_bar);
    VIEW_MEMBER_VALID(page, level_value);

    if (level < 0)  level = 0;
    if (level > 15) level = 15;

    lv_bar_set_value(level_bar, level * 100 / 15, LV_ANIM_OFF);
    snprintf(text, sizeof(text), "%d/15", level);
    lv_label_set_text(level_value, text);
}

void radio_view_refresh_scan(bool scanning)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, scan_state);

    lv_label_set_text(scan_state, scanning ? "SCANNING" : "STANDBY");
    set_text_color(scan_state, scanning ? RADIO_COLOR_ON : RADIO_COLOR_DIM);
}

void radio_view_refresh_stereo(bool on)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, stereo);

    lv_label_set_text(stereo, on ? "ST" : "MONO");
    set_text_color(stereo, on ? RADIO_COLOR_ON : RADIO_COLOR_DIM);
}

void radio_view_refresh_rds(int rds_state)
{
    radio_page_t *page = radio_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, rds);
    VIEW_MEMBER_VALID(page, tp);
    VIEW_MEMBER_VALID(page, ta);

    lv_label_set_text(rds, (rds_state & VS_RADIO_ICON_RDS) ? "RDS" : "--");
    set_text_color(rds, (rds_state & VS_RADIO_ICON_RDS) ? RADIO_COLOR_ON : RADIO_COLOR_DIM);

    lv_label_set_text(tp, (rds_state & VS_RADIO_ICON_TP) ? "TP" : "--");
    set_text_color(tp, (rds_state & VS_RADIO_ICON_TP) ? RADIO_COLOR_ON : RADIO_COLOR_DIM);

    lv_label_set_text(ta, (rds_state & VS_RADIO_ICON_TA) ? "TA" : "--");
    set_text_color(ta, (rds_state & VS_RADIO_ICON_TA) ? RADIO_COLOR_ON : RADIO_COLOR_DIM);
}

/* ---------- 台号 / 台名 / 文本 ---------- */

void radio_view_refresh_station_no(int no)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, station_no);

    if (no > 0) {
        lv_label_set_text_fmt(station_no, "%d", no);
    } else {
        lv_label_set_text(station_no, "--");
    }
}

void radio_view_refresh_ps(const char *ps_text)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, ps);

    lv_label_set_text(ps, (ps_text != NULL && ps_text[0]) ? ps_text : "--");
}

void radio_view_refresh_rt(const char *rt_text)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, rt);

    lv_label_set_text(rt, (rt_text != NULL && rt_text[0]) ? rt_text : " ");
}

/* ---------- 频率 ---------- */

/* 波段值是否合法：radio_band_t 只有 8 个取值 FM1~FM4(0x0000~0x0003) / AM1~AM4(0x0100~0x0103)。
 * 为什么在这里判：band 是"编码值"(type<<8|index)，不是中间件内部的 0~7 槽位下标；
 * 上游一旦把槽位下标(0~7)或频率误当 band 传进来，按位解码就会印出 FM-8(下标 7) /
 * AM-81(低字节 80) 这种读不通的文案。非法值一律显示 "--" 并把原始值打进日志。 */
static bool band_is_valid(int band)
{
    int type  = VS_RADIO_BAND_TYPE(band);
    int index = VS_RADIO_BAND_INDEX(band);

    if (type == 0) {
        return index < VS_RADIO_BAND_FM_COUNT;
    }
    if (type == 1) {
        return index < VS_RADIO_BAND_AM_COUNT;
    }
    return false;
}

void radio_view_refresh_band(int band)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, band_lab);

    if (!band_is_valid(band)) {
        if (band != s_last_band) {
            s_last_band = band;
            LV_LOG_WARN("radio band 非法: band=0x%04x (type=%d index=%d), 非 radio_band_t 编码",
                        band, VS_RADIO_BAND_TYPE(band), VS_RADIO_BAND_INDEX(band));
        }
        lv_label_set_text(band_lab, "--");
        return;
    }

    if (band != s_last_band) {
        s_last_band = band;
        LV_LOG_USER("[RADIO-UI] band=0x%04x -> %s-%d", band,
                    VS_RADIO_BAND_IS_FM(band) ? "FM" : "AM", VS_RADIO_BAND_INDEX(band) + 1);
    }

    lv_label_set_text_fmt(band_lab, "%s-%d",
                          VS_RADIO_BAND_IS_FM(band) ? "FM" : "AM",
                          VS_RADIO_BAND_INDEX(band) + 1);
}

void radio_view_refresh_freq(int freq_khz)
{
    char text[24];
    const char *unit_text = NULL;
    radio_page_t *page = radio_page_get(&ui_manager);

    VIEW_MEMBER_VALID(page, freq);
    VIEW_MEMBER_VALID(page, unit);

    format_freq(freq_khz, text, sizeof(text), &unit_text);
    lv_label_set_text(freq, text);
    lv_label_set_text(unit, unit_text);
}

/* ---------- 控制按钮状态 ---------- */

void radio_view_refresh_mute(bool mute)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, mute_lab);

    lv_label_set_text(mute_lab, mute ? "MUTE:ON" : "MUTE:OFF");
    set_text_color(mute_lab, mute ? RADIO_COLOR_ON : RADIO_COLOR_TEXT);
}

void radio_view_refresh_local(bool local)
{
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, local_lab);

    lv_label_set_text(local_lab, local ? "LOCAL" : "DX");
    set_text_color(local_lab, local ? RADIO_COLOR_ON : RADIO_COLOR_TEXT);
}

/* ---------- 预置台列表 ---------- */

void radio_view_rebuild_presets(const int *freqs, int n)
{
    int i;
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, preset_cont);

    lv_obj_clean(preset_cont);
    s_highlight = -1;   /* 列表重建，下次刷新重新滚一次 */

    if (freqs == NULL || n <= 0) {
        return;
    }

    for (i = 0; i < n; i++) {
        char text[24];
        const char *unit_text = NULL;
        lv_obj_t *btn = lv_button_create(preset_cont);
        lv_obj_t *lab = NULL;

        lv_obj_set_size(btn, RADIO_PRESET_ITEM_W, RADIO_PRESET_ITEM_H);
        lv_obj_add_event_cb(btn, on_preset_clicked, LV_EVENT_CLICKED, (void *)(lv_uintptr_t)i);

        /* 分页吸附：只有每页第一个台是吸附点(0/6/12)，其余台清掉 SNAPPABLE 标志。
         * 这样滑动松手后 LVGL 只会吸附到这三条页边界（最近的页），
         * 即 "超过半页翻页、否则回正"，而不是逐台吸附。 */
        if ((i % RADIO_PRESET_VISIBLE) != 0) {
            lv_obj_remove_flag(btn, LV_OBJ_FLAG_SNAPPABLE);
        }

        lab = lv_label_create(btn);
        format_freq(freqs[i], text, sizeof(text), &unit_text);
        lv_label_set_text_fmt(lab, "%d %s", i + 1, text);
        lv_obj_center(lab);
        lv_obj_set_style_text_font(lab, aic_get_multi_lang_font(16), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void radio_view_highlight_preset(int index)
{
    uint32_t i;
    uint32_t n;
    bool moved = (index != s_highlight);   /* 高亮变化才滚，别打断用户手动滑动 */
    lv_obj_t *page_obj = NULL;
    radio_page_t *page = radio_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, preset_cont);

    s_highlight = index;
    n = lv_obj_get_child_count(preset_cont);
    for (i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(preset_cont, (int32_t)i);

        if ((int)i == index) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(RADIO_COLOR_ACTIVE), LV_PART_MAIN | LV_STATE_DEFAULT);
            page_obj = btn;
        } else {
            lv_obj_remove_local_style_prop(btn, LV_STYLE_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    /* 当前台不在第一页时，把列表整页翻到它所在的那一页（1-6 / 7-12 / 13-18）。
     * 注意是翻页不是逐像素滚动：目标位置就是那个页的吸附点，和手动滑动的落点一致。
     * 只在下标变化时翻，别打断用户手动滑动。 */
    if (page_obj != NULL && moved) {
        int page_n = (int)RADIO_PRESET_PAGE_N((int)n);
        int pg     = index / RADIO_PRESET_VISIBLE;

        if (pg < 0) pg = 0;
        if (pg > page_n - 1) pg = page_n - 1;

        lv_obj_scroll_to_x(preset_cont, pg * RADIO_PRESET_PAGE_STEP, LV_ANIM_ON);
        lv_log("[RADIO-UI] preset highlight -> #%d (page %d/%d)\n", index + 1, pg + 1, page_n);
    }
}

void radio_view_init(void)
{
    radio_view_refresh_title();
    /* 其余控件先留空，由 radio_logic 的首次 timer_refresh() 填初值 */
}
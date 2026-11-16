/*
 * radio_logic.c --- 收音机页面控制层
 *
 * 线程模型：
 *   中间件 radio_mw 在自己的 reader 线程里更新内部状态，事件回调只做日志；
 *   本页挂一个 200ms 定时器，在 LVGL 主线程里轮询 vs_radio_get_info() 并刷新控件，
 *   避免跨线程操作 UI。
 *
 * 中间件：一律经 vehicle_services 的 vs_radio_* 门面（ui/ 不 include <mw/...>）。
 * PC 模拟器上由 platform/pc/mock_radio_mw.c + mock_vehicle_services.c 顶替，
 * 上真机后同一份 ui/ 代码走 device 侧 vehicle_services.c 转发真 radio_mw。
 */
#include "radio_logic.h"
#include "radio_view.h"
#include "vehicle_services.h"

#include <string.h>

#define REFRESH_INTERVAL_MS (200)
#define PRESET_MAX          (128)   /* 单波段预置台上限，与中间件对齐 */

/* 可切换波段：只在 FM1/FM2/FM3/AM1 之间循环
 * （目标硬件不支持 AM 时，把 AM1 去掉只留 FM1~FM3 即可）。 */
static const int g_switchable_bands[] = {
    VS_RADIO_BAND_FM1, VS_RADIO_BAND_FM2, VS_RADIO_BAND_FM3, VS_RADIO_BAND_AM1,
};
#define SWITCHABLE_BAND_N ((int)(sizeof(g_switchable_bands) / sizeof(g_switchable_bands[0])))

static lv_timer_t     *s_radio_timer = NULL;
static bool            s_mw_inited   = false;
static int             s_band        = VS_RADIO_BAND_FM1;
static int             s_presets[PRESET_MAX];
static int             s_preset_n    = 0;
static vs_radio_info_t s_info;

/* 当前频率在预置表中的下标（-1 = 不在预置）：逐项比对，
 * 命中则用于显示台号 + 高亮预置项。 */
static int current_preset_index(void)
{
    int i;

    for (i = 0; i < s_preset_n; i++) {
        if (s_presets[i] == s_info.freq) {
            return i;
        }
    }
    return -1;
}

/* 预置台被点击：直接切到该频点 */
static void on_preset_click(int index)
{
    if (index < 0 || index >= s_preset_n) {
        return;
    }
    lv_log("[RADIO-UI] 点击预置台 %d -> %dKHz\n", index + 1, s_presets[index]);
    vs_radio_set_uiband_freq(s_band, index, s_presets[index]);
}

/* 取当前波段的预置表并重建横向列表 */
static void load_presets(void)
{
    s_preset_n = vs_radio_get_preset(s_band, s_presets, PRESET_MAX);
    if (s_preset_n < 0) {
        s_preset_n = 0;
    }
    radio_view_rebuild_presets(s_presets, s_preset_n);
}

/* 200ms 定时刷新（LVGL 主线程） */
static void timer_refresh(lv_timer_t *info)
{
    vs_radio_info_t cur;
    int index;

    (void)info;

    vs_radio_get_info(&cur);
    s_info = cur;

    /* 波段被中间件改过（比如上电恢复状态），跟着换预置表 */
    if (cur.band != s_band) {
        s_band = cur.band;
        load_presets();
    }

    radio_view_refresh_band(s_band);
    radio_view_refresh_freq(cur.freq);

    index = current_preset_index();
    radio_view_refresh_station_no(index + 1);   /* 未命中传 0 -> 显示 "--" */
    radio_view_highlight_preset(index);

    radio_view_refresh_ps(cur.ps);
    radio_view_refresh_rt(cur.rt);
    radio_view_refresh_stereo(cur.stereo);
    radio_view_refresh_rds(cur.rds_state);
    radio_view_refresh_level(cur.level);
    radio_view_refresh_scan(vs_radio_is_scanning());
    radio_view_refresh_mute(cur.mute);
    radio_view_refresh_local(cur.local);
}

void radio_logic_action(radio_action_e action)
{
    switch (action) {
    case RADIO_ACTION_PREV:
        vs_radio_prev();
        break;
    case RADIO_ACTION_NEXT:
        vs_radio_next();
        break;
    case RADIO_ACTION_STEP_UP:
        vs_radio_step_up();
        break;
    case RADIO_ACTION_STEP_DOWN:
        vs_radio_step_down();
        break;
    case RADIO_ACTION_SEEK_UP:
        /* 搜索中再按同键 = 取消搜索 */
        if (vs_radio_is_scanning()) {
            vs_radio_scan_stop();
        } else {
            vs_radio_scan(VS_RADIO_SCAN_UP);
        }
        break;
    case RADIO_ACTION_SEEK_DOWN:
        if (vs_radio_is_scanning()) {
            vs_radio_scan_stop();
        } else {
            vs_radio_scan(VS_RADIO_SCAN_DOWN);
        }
        break;
    case RADIO_ACTION_BAND:
    {
        /* 在可切换波段列表里循环；当前波段不在列表(如历史状态落在 FM4)时回绕到 FM1 */
        int cur = -1, i;
        for (i = 0; i < SWITCHABLE_BAND_N; i++) {
            if (g_switchable_bands[i] == s_band) {
                cur = i;
                break;
            }
        }
        s_band = g_switchable_bands[(cur + 1) % SWITCHABLE_BAND_N];
        vs_radio_set_band_restore(s_band);   /* 恢复到该波段上次的位置 */
        load_presets();
        break;
    }
    case RADIO_ACTION_AS:
        if (vs_radio_is_scanning()) {
            vs_radio_scan_stop();
        } else {
            vs_radio_scan_as(18);
        }
        break;
    case RADIO_ACTION_PS_PLAY:
        vs_radio_preset_play(5);
        break;
    case RADIO_ACTION_PS_STOP:
        vs_radio_preset_stop();
        break;
    case RADIO_ACTION_MUTE:
        vs_radio_set_mute(!s_info.mute);
        break;
    case RADIO_ACTION_LOCAL:
        vs_radio_set_local(!s_info.local);
        break;
    default:
        break;
    }

    timer_refresh(NULL);   /* 立即回读一次，不等下一个 200ms */
}

void radio_page_init(void)
{
    radio_view_init();

    if (!s_mw_inited) {
        /* 常见原因：dev_config.h 未开 MW_RADIO_ENABLE（radio_mw.h 退化为空转桩），
         * 或 /dev/fmradio 打不开。 */
        int r = vs_radio_init(VS_RADIO_AREA_CHINA);
        if (r != 0) {
            LV_LOG_WARN("radio init failed: %d (查 dev_config.h 的 MW_RADIO_ENABLE, 再查 /dev/fmradio)", r);
        }
        s_mw_inited = true;
    }

    radio_view_set_preset_cb(on_preset_click);

    s_band = vs_radio_get_band();
    load_presets();

    if (s_radio_timer == NULL) {
        s_radio_timer = lv_timer_create(timer_refresh, REFRESH_INTERVAL_MS, NULL);
    }

    timer_refresh(NULL);
}

void on_radio_page_destory(void)
{
    /* 回主页时 displayfocusmanager 会删除本页对象，定时器必须一起删掉，
     * 否则回调会去写已释放的控件。中间件本身不退（进程级资源）。 */
    if (s_radio_timer) {
        lv_timer_del(s_radio_timer);
        s_radio_timer = NULL;
    }
}

/*
 * dvr_logic.c --- DVR 页面控制层
 *
 * 中间件一律经 vehicle_services 的 vs_dvr_* 门面（ui/ 不 include <mw/...>）。
 * PC 模拟器上由 platform/pc/mock_dvr_mw.c + mock_vehicle_services.c 顶替；
 * 上真机后同一份 ui/ 代码走 device 侧 vehicle_services.c 转发真 dvr_mw。
 *
 * 状态机：
 *   MAIN(dvr_bg+dock) -> CAM_SW(前/后/抓拍) / LIST_SEL(选前/后摄头) / SETTING
 *   LIST_SEL -> SET -> LIST(列表) -> SET on file -> LIST_ACT(播放/删除)
 *   LIST_ACT -> SET(播放) -> PLAYBACK | SET(删除) -> POPUP(确认)
 *   SETTING -> SET(循环时长) -> SET_EDIT(1/2/3 min) | SET(格式化) -> SET_EDIT -> POPUP
 *   POPUP 分类：文件删除 / 格式化 / 恢复出厂 / 无 SD 卡(只有确认)
 *   进文件区停录像；退回 MAIN 恢复录像。
 */
#include "dvr_logic.h"
#include "dvr_view.h"
#include "vehicle_services.h"

#include <string.h>
#include <time.h>

#define REFRESH_INTERVAL_MS (100)   /* 预览要跟得上：10fps 轮询 */
#define LIST_WAIT_MAX_TICK  (30)    /* 30 x 100ms = 3s 收不到列表回复就放弃 */
#define DEL_VERIFY_TICK     (4)     /* 删除后 400ms 重拉同一 mode 的列表 */
#define STS_REQ_TICK        (5)     /* 每 500ms 发一次 GET_STS */
#define LOOP_MIN_MAX        (3)     /* 循环录像时长只有 1/2/3 分钟可选 */

/* ---------------- 状态 ---------------- */

static lv_timer_t *s_timer = NULL;

static int  s_sub        = DVR_SUB_MAIN;
static int  s_dock_focus = 0;              /* 0=预览 1=视频回放 2=照片回放 3=设置 */
static int  s_cam_focus  = 0;              /* 0=前 1=后 2=抓拍 */
static int  s_tab        = 0;              /* 0=前摄 1=后摄（待机页/列表页共用） */
static int  s_mode       = 0;              /* 0=视频 1=照片 */
static int  s_offset     = 0;              /* 列表首行对应的绝对下标 */
static int  s_focus      = 0;              /* 列表焦点行 0..5 */
static int  s_count      = 0;              /* 当前列表文件数 */
static int  s_act        = 0;              /* LIST_ACT：0=播放 1=删除 */
static int  s_set_focus  = DVR_SET_ROW_VERSION;
static int  s_loop       = 0;              /* 0..2 -> 1/2/3 min */
static int  s_fmt        = 0;              /* 0=SD格式化 1=恢复出厂 */
static int  s_pop_focus  = 1;
static int  s_pop_src    = DVR_POP_FILE_DEL;
static int  s_no_sd_back = DVR_SUB_MAIN;
static bool s_pb_paused  = false;

static bool s_list_wait  = false;          /* 已发 GET_LIST 等回复 */
static int  s_list_tick  = 0;
static int  s_del_tick   = 0;
static int  s_sts_tick   = 0;
static uint32_t s_last_seq = 0;
static bool s_hw_hint    = false;          /* 硬解提示只切一次 */
static bool s_preview_on = false;
static uint8_t *s_rgba   = NULL;           /* 软解取像素的缓冲（硬解路径用不上） */

/* ---------------- 工具 ---------------- */

static bool has_sd(void)
{
    vs_dvr_status_t st;

    memset(&st, 0, sizeof(st));
    vs_dvr_get_status(&st);
    return st.sd && !st.sd_err;
}

static bool preview_visible(void)
{
    return s_sub == DVR_SUB_MAIN || s_sub == DVR_SUB_CAM_SW || s_sub == DVR_SUB_PLAYBACK;
}

/* 当前焦点行对应的列表绝对下标 */
static int cur_index(void)
{
    return s_offset + s_focus;
}

/* ---------------- 视图切换 ---------------- */

static void show_sub(dvr_sub_e sub)
{
    bool was = preview_visible();

    s_sub = sub;
    dvr_view_show_sub(sub);

    /* 离开预览区只是不再画，取流不停
     * （列表/设置页也要发命令、收回复，停了读线程命令就发不出去）。
     * ⚠ 硬解(device)的画面是中间件直铺的**独立 v4l2 overlay 图层**（layer z=3），
     *   不吃 LVGL 的绘制指令：光"不再画"它照样盖在设置页/列表页上，必须显式隐藏。 */
    if (!was && preview_visible()) {
        s_preview_on = true;
        vs_dvr_set_preview(0, 0, DVR_PREVIEW_W, DVR_PREVIEW_H, VS_DVR_FMT_RGBA8888);
        vs_dvr_start();
        vs_dvr_set_preview_visible(1);
    } else if (was && !preview_visible()) {
        vs_dvr_set_preview_visible(0);
    }
}

/* ---------------- 列表 ---------------- */

static void list_fill(void)
{
    dvr_view_item_t items[DVR_LIST_ROW_N];
    vs_dvr_file_t   f;
    int i;

    memset(items, 0, sizeof(items));
    for (i = 0; i < DVR_LIST_ROW_N; i++) {
        int fi = s_offset + i;

        if (fi >= s_count) {
            continue;
        }
        if (vs_dvr_get_file(s_mode, (uint16_t)fi, &f) == 0) {
            lv_snprintf(items[i].name, sizeof(items[i].name), "%s", f.name);
            items[i].file_no = f.file_no;
            items[i].locked  = f.locked;
        }
    }
    dvr_view_list_fill(items, s_count - s_offset);
    dvr_view_list_no_sd(s_count == 0 && !has_sd());
}

/* 拉列表：进文件区前先停录像；结果为异步，由定时器 list_poll 取 */
static void request_list(int mode)
{
    vs_dvr_record_stop();

    s_mode   = (mode >= 2) ? 1 : 0;
    s_tab    = (mode == 1 || mode == 3) ? 1 : 0;
    s_offset = 0;
    s_focus  = 0;
    s_count  = 0;
    s_list_wait = true;
    s_list_tick = 0;

    dvr_view_tab_text(s_mode);
    dvr_view_tab_focus(s_tab);
    dvr_view_list_fill(NULL, 0);
    dvr_view_row_focus(-1);

    vs_dvr_get_list((int)mode);
    show_sub(DVR_SUB_LIST);
}

static void list_poll(void)
{
    if (!s_list_wait) {
        return;
    }
    if (!vs_dvr_is_filelist_ready()) {
        if (++s_list_tick > LIST_WAIT_MAX_TICK) {
            s_list_wait = false;
            vs_dvr_clear_skip_jpeg();     /* 没收到也要恢复 JPEG 解码，否则预览一直停 */
            s_count = 0;
            list_fill();
            dvr_view_row_focus(-1);
        }
        return;
    }

    s_list_wait = false;
    s_count = vs_dvr_get_file_count(s_mode);
    if (s_count < 0) s_count = 0;
    if (s_count > VS_DVR_FILELIST_MAX) s_count = VS_DVR_FILELIST_MAX;
    vs_dvr_clear_skip_jpeg();

    if (s_focus >= s_count) {
        s_focus = (s_count > 0) ? s_count - 1 : 0;
    }
    list_fill();
    dvr_view_row_focus((s_count > 0) ? s_focus : -1);
}

/* ---------------- 动作 ---------------- */

static void do_snapshot(void)
{
    char       path[256];
    char       stamp[64];
    time_t     now = time(NULL);
    struct tm  tm;

#if defined(_WIN32)
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tm);
    lv_snprintf(path, sizeof(path), "dvr_%s.jpg", stamp);

    if (vs_dvr_snapshot(path) == 0) {
        LV_LOG_USER("DVR snapshot -> %s", path);
    } else {
        LV_LOG_WARN("DVR snapshot failed: %s", path);
    }
}

static void do_play_file(void)
{
    vs_dvr_file_t f;
    uint16_t      file_no = (uint16_t)cur_index();

    if (vs_dvr_get_file(s_mode, (uint16_t)cur_index(), &f) == 0) {
        file_no = f.file_no;
    }
    vs_dvr_pb_start(s_mode, file_no);
    s_pb_paused = false;
    show_sub(DVR_SUB_PLAYBACK);
}

/* 弹窗标题 */
static const char *popup_title(dvr_pop_e src)
{
    switch (src) {
    case DVR_POP_FORMAT:      return aic_get_translation(dvr_format);
    case DVR_POP_FACTORY_RST: return aic_get_translation(dvr_factory_reset);
    case DVR_POP_NO_SD:       return aic_get_translation(dvr_no_sd);
    default:                  return aic_get_translation(dvr_confirm_delete);
    }
}

static void popup_open(dvr_pop_e src, const char *title, int focus)
{
    s_pop_src   = (int)src;
    s_pop_focus = focus;
    dvr_view_popup(src, title);
    dvr_view_popup_focus(focus);
    show_sub(DVR_SUB_POPUP);
}

static void show_no_sd_popup(int back)
{
    s_no_sd_back = back;
    popup_open(DVR_POP_NO_SD, popup_title(DVR_POP_NO_SD), 0);
}

static void enter_settings(void)
{
    vs_dvr_status_t st;

    vs_dvr_req_id();
    vs_dvr_req_status();

    memset(&st, 0, sizeof(st));
    vs_dvr_get_status(&st);
    if (st.loop_min >= 1 && st.loop_min <= LOOP_MIN_MAX) {
        s_loop = st.loop_min - 1;
    }
    dvr_view_set_version(st.version[0] ? st.version : "--");
    dvr_view_set_focus(s_set_focus);
    dvr_view_loop_sel(s_loop);
    dvr_view_fmt_sel(s_fmt);

    show_sub(DVR_SUB_SETTING);
}

/* ---------------- 键控（UP/DOWN/SET/BACK） ---------------- */

static void key_set(void)
{
    switch (s_sub) {
    case DVR_SUB_MAIN:
        if (s_dock_focus == 0) {
            vs_dvr_status_t st;
            memset(&st, 0, sizeof(st));
            vs_dvr_get_status(&st);
            s_cam_focus = (st.view_mode == VS_DVR_VIEW_REAR) ? 1 : 0;
            show_sub(DVR_SUB_CAM_SW);
            dvr_view_cam_focus(s_cam_focus);
        } else if (s_dock_focus == 1 || s_dock_focus == 2) {
            if (!has_sd()) {
                show_no_sd_popup(DVR_SUB_MAIN);
                break;
            }
            s_mode = (s_dock_focus == 2) ? 1 : 0;
            s_tab  = 0;
            dvr_view_tab_text(s_mode);
            dvr_view_tab_focus(s_tab);
            show_sub(DVR_SUB_LIST_SEL);
        } else {
            enter_settings();
        }
        break;

    case DVR_SUB_CAM_SW:
        if (s_cam_focus == 0 || s_cam_focus == 1) {
            vs_dvr_view_switch(s_cam_focus);
            dvr_view_cam_focus(s_cam_focus);
        } else {
            if (!has_sd()) {
                show_no_sd_popup(DVR_SUB_CAM_SW);
            } else {
                do_snapshot();
            }
        }
        break;

    case DVR_SUB_LIST_SEL:
        request_list(s_mode * 2 + s_tab);
        break;

    case DVR_SUB_LIST:
        if (cur_index() < s_count) {
            s_act = 0;
            show_sub(DVR_SUB_LIST_ACT);
            dvr_view_row_action(s_focus, 0);
        }
        break;

    case DVR_SUB_LIST_ACT:
        if (s_act == 0) {
            do_play_file();
        } else {
            popup_open(DVR_POP_FILE_DEL, popup_title(DVR_POP_FILE_DEL), 1);   /* 默认停在"取消" */
        }
        break;

    case DVR_SUB_POPUP:
        if (s_pop_src == DVR_POP_NO_SD) {
            show_sub((dvr_sub_e)s_no_sd_back);
            break;
        }
        if (s_pop_focus == 0) {
            if (s_pop_src == DVR_POP_FILE_DEL) {
                vs_dvr_file_t f;
                uint16_t      file_no = (uint16_t)cur_index();

                if (vs_dvr_get_file(s_mode, (uint16_t)cur_index(), &f) == 0) {
                    file_no = f.file_no;
                }
                vs_dvr_del_file(s_mode, file_no);
                s_del_tick = DEL_VERIFY_TICK;
                LV_LOG_USER("DVR del file_no=%u (idx=%d)", (unsigned)file_no, cur_index());
            } else if (s_pop_src == DVR_POP_FORMAT) {
                vs_dvr_record_stop();      /* 格式化前必须停录像 */
                vs_dvr_format();
            } else if (s_pop_src == DVR_POP_FACTORY_RST) {
                vs_dvr_record_stop();
                vs_dvr_restore_default();
            }
        }
        if (s_pop_src == DVR_POP_FORMAT || s_pop_src == DVR_POP_FACTORY_RST) {
            show_sub(DVR_SUB_SETTING);
            dvr_view_set_focus(s_set_focus);
        } else {
            show_sub(DVR_SUB_LIST);
            list_fill();
            dvr_view_row_focus((s_count > 0) ? s_focus : -1);
        }
        break;

    case DVR_SUB_SETTING:
        if (s_set_focus == DVR_SET_ROW_LOOP || s_set_focus == DVR_SET_ROW_FORMAT) {
            show_sub(DVR_SUB_SET_EDIT);
            if (s_set_focus == DVR_SET_ROW_LOOP) {
                dvr_view_loop_sel(s_loop);
            } else {
                dvr_view_fmt_sel(s_fmt);
            }
        }
        break;

    case DVR_SUB_SET_EDIT:
        if (s_set_focus == DVR_SET_ROW_LOOP) {
            vs_dvr_set_loop_time((uint8_t)(s_loop + 1));
            dvr_view_loop_sel(s_loop);
            show_sub(DVR_SUB_SETTING);
            dvr_view_set_focus(s_set_focus);
        } else if (s_set_focus == DVR_SET_ROW_FORMAT) {
            if (s_fmt == 0) {
                popup_open(DVR_POP_FORMAT, popup_title(DVR_POP_FORMAT), 1);
            } else {
                popup_open(DVR_POP_FACTORY_RST, popup_title(DVR_POP_FACTORY_RST), 1);
            }
        }
        break;

    case DVR_SUB_PLAYBACK:
        s_pb_paused = !s_pb_paused;
        vs_dvr_pb_pause();
        break;

    default:
        break;
    }
}

static void key_up(void)
{
    switch (s_sub) {
    case DVR_SUB_MAIN:
        if (s_dock_focus > 0) {
            s_dock_focus--;
            dvr_view_dock_focus(s_dock_focus);
        }
        break;
    case DVR_SUB_CAM_SW:
        if (s_cam_focus > 0) {
            s_cam_focus--;
            dvr_view_cam_focus(s_cam_focus);
        }
        break;
    case DVR_SUB_LIST:
        if (s_focus > 0) {
            s_focus--;
            dvr_view_row_focus(s_focus);
        } else if (s_offset > 0) {
            s_offset--;
            list_fill();
            dvr_view_row_focus(0);
        } else if (s_tab > 0) {
            request_list(s_mode * 2 + 0);        /* 顶行再上 -> 切前摄 tab */
        }
        break;
    case DVR_SUB_SETTING:
        if (s_set_focus > 0) {
            s_set_focus--;
            dvr_view_set_focus(s_set_focus);
        }
        break;
    case DVR_SUB_SET_EDIT:
        if (s_set_focus == DVR_SET_ROW_LOOP) {
            s_loop = (s_loop > 0) ? s_loop - 1 : 2;
            dvr_view_loop_sel(s_loop);
        } else {
            s_fmt = !s_fmt;
            dvr_view_fmt_sel(s_fmt);
        }
        break;
    case DVR_SUB_POPUP:
        if (s_pop_src != DVR_POP_NO_SD && s_pop_focus != 0) {
            s_pop_focus = 0;
            dvr_view_popup_focus(s_pop_focus);
        }
        break;
    case DVR_SUB_LIST_ACT:
        if (s_act != 0) {
            s_act = 0;
            dvr_view_row_action(s_focus, 0);
        }
        break;
    case DVR_SUB_LIST_SEL:
        if (s_tab > 0) {
            s_tab--;
            dvr_view_tab_focus(s_tab);
        }
        break;
    default:
        break;
    }
}

static void key_down(void)
{
    switch (s_sub) {
    case DVR_SUB_MAIN:
        if (s_dock_focus < 3) {
            s_dock_focus++;
            dvr_view_dock_focus(s_dock_focus);
        }
        break;
    case DVR_SUB_CAM_SW:
        if (s_cam_focus < 2) {
            s_cam_focus++;
            dvr_view_cam_focus(s_cam_focus);
        }
        break;
    case DVR_SUB_LIST:
        if (s_focus < DVR_LIST_ROW_N - 1 && (cur_index() + 1) < s_count) {
            s_focus++;
            dvr_view_row_focus(s_focus);
        } else if ((s_offset + DVR_LIST_ROW_N) < s_count) {
            s_offset++;
            list_fill();
            dvr_view_row_focus(DVR_LIST_ROW_N - 1);
        } else if (s_tab < 1) {
            request_list(s_mode * 2 + 1);        /* 末行再下 -> 切后摄 tab */
        }
        break;
    case DVR_SUB_SETTING:
        if (s_set_focus < DVR_SET_ROW_NUM - 1) {
            s_set_focus++;
            dvr_view_set_focus(s_set_focus);
        }
        break;
    case DVR_SUB_SET_EDIT:
        if (s_set_focus == DVR_SET_ROW_LOOP) {
            s_loop = (s_loop + 1) % LOOP_MIN_MAX;
            dvr_view_loop_sel(s_loop);
        } else {
            s_fmt = !s_fmt;
            dvr_view_fmt_sel(s_fmt);
        }
        break;
    case DVR_SUB_POPUP:
        if (s_pop_src != DVR_POP_NO_SD && s_pop_focus != 1) {
            s_pop_focus = 1;
            dvr_view_popup_focus(s_pop_focus);
        }
        break;
    case DVR_SUB_LIST_ACT:
        if (s_act != 1) {
            s_act = 1;
            dvr_view_row_action(s_focus, 1);
        }
        break;
    case DVR_SUB_LIST_SEL:
        if (s_tab < 1) {
            s_tab++;
            dvr_view_tab_focus(s_tab);
        }
        break;
    default:
        break;
    }
}

static bool key_back(void)
{
    switch (s_sub) {
    case DVR_SUB_CAM_SW:
        show_sub(DVR_SUB_MAIN);
        dvr_view_dock_focus(s_dock_focus);
        break;
    case DVR_SUB_LIST_SEL:
        show_sub(DVR_SUB_MAIN);
        dvr_view_dock_focus(s_dock_focus);
        vs_dvr_record_start(NULL);              /* 离开文件区回主预览即恢复录像 */
        break;
    case DVR_SUB_LIST:
        show_sub(DVR_SUB_LIST_SEL);
        dvr_view_tab_focus(s_tab);
        break;
    case DVR_SUB_LIST_ACT:
        show_sub(DVR_SUB_LIST);
        dvr_view_row_focus(s_focus);
        break;
    case DVR_SUB_SETTING:
        show_sub(DVR_SUB_MAIN);
        dvr_view_dock_focus(s_dock_focus);
        vs_dvr_record_start(NULL);
        break;
    case DVR_SUB_SET_EDIT:
        show_sub(DVR_SUB_SETTING);
        dvr_view_set_focus(s_set_focus);
        break;
    case DVR_SUB_POPUP:
        if (s_pop_src == DVR_POP_NO_SD) {
            show_sub((dvr_sub_e)s_no_sd_back);
        } else if (s_pop_src == DVR_POP_FORMAT || s_pop_src == DVR_POP_FACTORY_RST) {
            show_sub(DVR_SUB_SETTING);
            dvr_view_set_focus(s_set_focus);
        } else {
            show_sub(DVR_SUB_LIST);
            dvr_view_row_focus((s_count > 0) ? s_focus : -1);
        }
        break;
    case DVR_SUB_PLAYBACK:
        vs_dvr_pb_stop();
        s_pb_paused = false;
        show_sub(DVR_SUB_LIST);
        list_fill();
        dvr_view_row_focus((s_count > 0) ? s_focus : -1);
        break;
    case DVR_SUB_MAIN:
    default:
        return false;                            /* 主视图再退 -> 退出本页 */
    }
    return true;
}

/* ---------------- 定时器 ---------------- */

static void preview_poll(void)
{
    uint32_t seq;

    if (s_rgba == NULL || !s_preview_on || !preview_visible()) {
        return;
    }
    seq = vs_dvr_get_frame_seq();
    if (seq == s_last_seq) {
        return;
    }
    s_last_seq = seq;

    if (vs_dvr_get_preview_rgba(s_rgba, DVR_PREVIEW_W * 4,
                                DVR_PREVIEW_W, DVR_PREVIEW_H) == 0) {
        s_hw_hint = false;
        dvr_view_draw_preview(s_rgba, DVR_PREVIEW_W, DVR_PREVIEW_H);
    } else if (!s_hw_hint) {
        /* 硬解：像素由中间件直铺 surface overlay，UI 只留一行提示（只切一次） */
        s_hw_hint = true;
        dvr_view_show_preview_hint("HW DECODE (directrender overlay)");
    }
}

static void status_poll(void)
{
    vs_dvr_status_t st;

    if (++s_sts_tick >= STS_REQ_TICK) {
        s_sts_tick = 0;
        vs_dvr_req_status();
    }

    memset(&st, 0, sizeof(st));
    vs_dvr_get_status(&st);
    dvr_view_set_version(st.version[0] ? st.version : "--");
    /* 固件没有容量查询命令，容量条保持"空条 + --/--"。 */
}

static void timer_cb(lv_timer_t *t)
{
    (void)t;

    status_poll();
    preview_poll();
    list_poll();

    /* 删除后延时重拉同一 mode 的列表 */
    if (s_sub == DVR_SUB_LIST && s_del_tick > 0 && --s_del_tick == 0) {
        s_list_wait = true;
        s_list_tick = 0;
        vs_dvr_get_list(s_mode * 2 + s_tab);
    }
}

/* ---------------- 对外接口 ---------------- */

void dvr_logic_action(dvr_action_e action)
{
    switch (action) {
    case DVR_ACTION_UP:     key_up();     break;
    case DVR_ACTION_DOWN:   key_down();   break;
    case DVR_ACTION_SELECT: key_set();    break;
    default: break;
    }
}

void dvr_logic_click(dvr_click_e area, int index)
{
    /* 触摸 = "先把焦点移过去，再按 SET"（与实体键同一套行为，避免两套逻辑） */
    switch (area) {
    case DVR_CLICK_DOCK:
        if (index >= 0 && index < 4) {
            s_dock_focus = index;
            dvr_view_dock_focus(index);
            if (s_sub == DVR_SUB_MAIN) {
                key_set();
            }
        }
        break;
    case DVR_CLICK_CAM:
        s_cam_focus = index;
        dvr_view_cam_focus(index);
        if (s_sub == DVR_SUB_CAM_SW) {
            key_set();
        }
        break;
    case DVR_CLICK_TAB_LEFT:
    case DVR_CLICK_TAB_RIGHT:
        s_tab = (area == DVR_CLICK_TAB_RIGHT) ? 1 : 0;
        dvr_view_tab_focus(s_tab);
        if (s_sub == DVR_SUB_LIST_SEL) {
            key_set();                      /* 待机页点页签 = 进列表 */
        } else if (s_sub == DVR_SUB_LIST) {
            request_list(s_mode * 2 + s_tab);
        }
        break;
    case DVR_CLICK_ROW:
        s_focus = index;
        dvr_view_row_focus(index);
        if (s_sub == DVR_SUB_LIST) {
            key_set();                      /* 进 LIST_ACT */
        }
        break;
    case DVR_CLICK_ROW_PLAY:
        s_focus = index;
        dvr_view_row_focus(index);
        do_play_file();
        break;
    case DVR_CLICK_ROW_DEL:
        s_focus = index;
        dvr_view_row_focus(index);
        popup_open(DVR_POP_FILE_DEL, popup_title(DVR_POP_FILE_DEL), 1);
        break;
    case DVR_CLICK_SET_ROW:
        s_set_focus = index;
        dvr_view_set_focus(index);
        key_set();
        break;
    case DVR_CLICK_SET_LOOP:
        s_set_focus = DVR_SET_ROW_LOOP;
        s_loop = index;
        dvr_view_set_focus(s_set_focus);
        dvr_view_loop_sel(s_loop);
        key_set();                          /* 进 SET_EDIT 后立刻确认（一步到位） */
        break;
    case DVR_CLICK_SET_FMT:
        s_set_focus = DVR_SET_ROW_FORMAT;
        s_fmt = index;
        dvr_view_set_focus(s_set_focus);
        dvr_view_fmt_sel(s_fmt);
        key_set();
        break;
    case DVR_CLICK_POPUP_OK:
        s_pop_focus = 0;
        dvr_view_popup_focus(0);
        key_set();
        break;
    case DVR_CLICK_POPUP_CANCEL:
        s_pop_focus = 1;
        dvr_view_popup_focus(1);
        key_set();
        break;
    case DVR_CLICK_PREVIEW:
        if (s_sub == DVR_SUB_PLAYBACK) {
            key_set();                      /* 回放画面：点一下暂停/继续 */
        }
        break;
    default:
        break;
    }
}

bool dvr_logic_back(void)
{
    return key_back();
}

void dvr_page_init(void)
{
    /* 软解取像素的缓冲：硬解路径用不上（get_preview_rgba 会返回负值），
     * 但没法先验判断，所以统一按最大尺寸备一块。 */
    if (s_rgba == NULL) {
        s_rgba = (uint8_t *)lv_malloc(DVR_PREVIEW_BYTES);
    }

    dvr_view_show_sub(DVR_SUB_MAIN);
    dvr_view_dock_focus(0);
    dvr_view_cam_focus(0);
    dvr_view_tab_text(0);
    dvr_view_tab_focus(0);
    dvr_view_set_focus(DVR_SET_ROW_VERSION);
    dvr_view_loop_sel(0);
    dvr_view_fmt_sel(0);
    dvr_view_row_focus(-1);
    dvr_view_list_fill(NULL, 0);
    dvr_view_popup_focus(1);

    s_sub = DVR_SUB_MAIN;
    s_preview_on = true;

    /* 中间件必须先 init（真机上 = 打开 elene 设备、起 reader/decode 线程）。
     * 没 init 时 dvr_start() 直接返回 -1、帧序号不动，预览就是一张静止底图。 */
    if (!vs_dvr_is_active()) {
        int r = vs_dvr_init();
        if (r != 0) {
            /* 常见原因：dev_config.h 未开 MW_DVR_ENABLE（dvr_mw.h 退化为空转桩），
             * 或 USB-DVR(elene) 未插入/未挂载。 */
            LV_LOG_WARN("DVR init failed: %d (查 dev_config.h 的 MW_DVR_ENABLE, 再查 elene 挂载)", r);
        }
    }

    vs_dvr_set_preview(0, 0, DVR_PREVIEW_W, DVR_PREVIEW_H, VS_DVR_FMT_RGBA8888);
    vs_dvr_start();
    vs_dvr_req_id();
    vs_dvr_set_preview_visible(1);
    vs_dvr_req_status();

    s_timer = lv_timer_create(timer_cb, REFRESH_INTERVAL_MS, NULL);
    lv_log("[DVR-UI] logic started\n");
}

void on_dvr_page_destory(void)
{
    if (s_timer != NULL) {
        lv_timer_delete(s_timer);
        s_timer = NULL;
    }
    vs_dvr_pb_stop();
    /* 必须先撤 overlay 图层再停流水线：hal_dvr_stop 会 release() 掉 surface，
     * surface 没了就没东西可 hide 了(而且若这版 .so 的 release() 不负责把画面从屏上
     * 撤下来，最后一帧会留在屏上盖住主页)。 */
    vs_dvr_set_preview_visible(0);
    vs_dvr_stop();

    if (s_rgba != NULL) {
        lv_free(s_rgba);
        s_rgba = NULL;
    }
    s_preview_on = false;
    lv_log("[DVR-UI] page destroyed\n");
}

void dvr_logic_update_language(void)
{
    /* 页签文案带"视频/照片"状态，得用当前模式重刷 */
    dvr_view_tab_text(s_mode);

    /* 正开着的弹窗标题（删除确认/格式化/无 SD 卡）跟着语言走 */
    if (s_sub == DVR_SUB_POPUP) {
        dvr_view_popup((dvr_pop_e)s_pop_src, popup_title((dvr_pop_e)s_pop_src));
        dvr_view_popup_focus(s_pop_focus);
    }
}

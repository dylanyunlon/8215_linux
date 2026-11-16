#ifndef RADIO_VIEW_H__
#define RADIO_VIEW_H__

#include "ui_init.h"
#include "aic_ui.h"

/*
 * 收音机页面视图层：只负责"把数据画到控件上"，不做任何业务判断。
 * 数据由 radio_logic.c 的 200ms 定时器拉取后调这里的 refresh_xxx()。
 *
 * 配色（页面是黑底，控件默认主题是浅色，文字颜色必须显式给）：
 */
#define RADIO_COLOR_TEXT     0xffffff   /* 正文 */
#define RADIO_COLOR_DIM      0x808080   /* 次要/关闭态 */
#define RADIO_COLOR_ACTIVE   0xff9800   /* 高亮：当前所在的那个预置台
                                         * (默认主题的按钮底色就是蓝的，高亮必须换个色系才看得出来) */
#define RADIO_COLOR_ON       0x00ff00   /* 状态打开(ST/RDS/TP/TA) */
#define RADIO_COLOR_STATION  0xffd700   /* 台号 */

/* 预置台按钮尺寸与分页：页面按 1024x600 设计，预置行一屏正好 RADIO_PRESET_VISIBLE(6) 个
 * = 一页（容器宽度由 radio_page.c 按这几个宏算出）。预置表 18 个台分成 3 页：
 * 1-6 / 7-12 / 13-18，横向滑动按页翻（ViewPager 思路：松手后吸附到最近的页边界，
 * 拖过半页就翻页、不到半页回正），当前台落在别的页时会自动跳到那一页显示
 * （见 radio_view_highlight_preset）。
 * RADIO_PRESET_PAGE_STEP = 相邻两页第一个台之间的像素距离，同时也是吸附点间距，
 * 必须与 radio_page.c 里容器的 pad/间距一致（那里只让每页第一个台可吸附）。 */
#define RADIO_PRESET_ITEM_W    150
#define RADIO_PRESET_ITEM_H    48
#define RADIO_PRESET_ITEM_GAP  6
#define RADIO_PRESET_VISIBLE   6
#define RADIO_PRESET_PAGE_STEP (RADIO_PRESET_VISIBLE * (RADIO_PRESET_ITEM_W + RADIO_PRESET_ITEM_GAP))
#define RADIO_PRESET_PAGE_N(n) (((n) + RADIO_PRESET_VISIBLE - 1) / RADIO_PRESET_VISIBLE)

/* 预置台点击回调：由 radio_logic 注册，view 不反向依赖 logic 的其它符号 */
typedef void (*radio_view_preset_cb_t)(int index);

void radio_view_init(void);
void radio_view_set_preset_cb(radio_view_preset_cb_t cb);

/* ---------------- 顶栏 ---------------- */
void radio_view_refresh_title(void);
void radio_view_refresh_level(int level);        /* 0~15，同时刷新进度条和 "n/15" */
void radio_view_refresh_scan(bool scanning);     /* SCANNING / STANDBY */
void radio_view_refresh_stereo(bool on);         /* ST / MONO */
void radio_view_refresh_rds(int rds_state);      /* VS_RADIO_ICON_TP|TA|RDS 位组合 */

/* ---------------- 台号 / 台名 / 文本 ---------------- */
void radio_view_refresh_station_no(int station_no);   /* <=0 显示 "--" */
void radio_view_refresh_ps(const char *ps);
void radio_view_refresh_rt(const char *rt);

/* ---------------- 频率 ---------------- */
void radio_view_refresh_band(int band);          /* 波段按钮文案 "FM-1"; band 非 radio_band_t 编码时显示 "--" */
void radio_view_refresh_freq(int freq_khz);      /* 大字频率 + 单位(MHz/KHz) */

/* ---------------- 控制按钮状态 ---------------- */
void radio_view_refresh_mute(bool mute);
void radio_view_refresh_local(bool local);

/* ---------------- 预置台列表 ---------------- */
/* 按当前波段的预置频点重建分页列表（会清掉旧按钮；每 RADIO_PRESET_VISIBLE 个一页） */
void radio_view_rebuild_presets(const int *freqs, int n);
/* 高亮当前频率命中的预置台；index < 0 表示都不高亮 */
void radio_view_highlight_preset(int index);

#endif /* RADIO_VIEW_H__ */
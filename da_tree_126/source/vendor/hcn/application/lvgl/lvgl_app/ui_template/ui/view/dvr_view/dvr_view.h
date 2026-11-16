#ifndef DVR_VIEW_H__
#define DVR_VIEW_H__

#include "ui_init.h"
#include "aic_ui.h"

/*
 * DVR 页面视图层：坐标、尺寸、图片沿用既有 dvr_ui 版式，不自行发明。
 *
 * 下面前 4 组宏与版式里的 x/y/w/h 一一对应；要动先确认版式，再同步这里，
 * 不要就地"看着差不多"调数值。
 */

#define DVR_PAGE_W  1024
#define DVR_PAGE_H  600

/* 主视图：预览 1024x500 + 底部 dock 1024x100 */
#define DVR_PREVIEW_W      1024
#define DVR_PREVIEW_H      500
#define DVR_PREVIEW_BYTES  (DVR_PREVIEW_W * DVR_PREVIEW_H * 4)   /* RGBA8888 */
#define DVR_DOCK_Y         500
#define DVR_DOCK_H         100

/* dock 4 个按钮：160x100，中心 x = 128/384/640/896；分隔线 1x100 */
#define DVR_DOCK_BTN_W      160
#define DVR_DOCK_BTN_H      100
#define DVR_DOCK_BTN_X(i)   (48 + (i) * 256)
#define DVR_DOCK_LINE_X(i)  (256 + (i) * 256)

/* 摄头切换 dock：选中块 341x100 平移；抓拍图标 80x80 */
#define DVR_CAM_TAB_W       341
#define DVR_CAM_SNAP_X      812
#define DVR_CAM_SNAP_Y      10
#define DVR_CAM_SNAP_W      80
#define DVR_CAM_SNAP_H      80

/* 待机页（选前/后摄头）与列表页底部 dock：选中块 512x100 平移 */
#define DVR_TAB_SEL_W       512

/* 文件列表：scroll(44,10,936,480)，行 600x60，行距 70，一屏 6 行 */
#define DVR_LIST_X          44
#define DVR_LIST_Y          10
#define DVR_LIST_W          936
#define DVR_LIST_H          480
#define DVR_LIST_ROW_N      6
#define DVR_LIST_ROW_W      600
#define DVR_LIST_ROW_H      60
#define DVR_LIST_ROW_STEP   70

/* 设置：4 行 910x80，起点 (57,110)，行距 100 */
#define DVR_SET_X           57
#define DVR_SET_W           910
#define DVR_SET_H           80
#define DVR_SET_Y(r)        (110 + (r) * 100)

/* 确认弹窗 500x270 @ (262,165) */
#define DVR_POPUP_X         262
#define DVR_POPUP_Y         165
#define DVR_POPUP_W         500
#define DVR_POPUP_H         270

/* 设置行的行序号（顺序与 XML / dvr_logic 一致） */
enum { DVR_SET_ROW_VERSION = 0, DVR_SET_ROW_LOOP, DVR_SET_ROW_FORMAT, DVR_SET_ROW_ABOUT, DVR_SET_ROW_NUM };

/*
 * 配色：直接取图片像素采样值，别凭感觉调
 *   底图 dvr_bg / dvr_list_bg   #FDDBC1（米色）
 *   dock_bg                     #E9B082（浅棕）
 *   未选中按钮底色              #4DB06311（30% 棕，叠在底图上）
 *   选中按钮底色                #FFB06412（橙棕）
 *   图标深蓝 #1C2E4D；进度条前景 #0E9AFF
 */
#define DVR_COLOR_TEXT      0x573508   /* 米底上的正文 */
#define DVR_COLOR_TEXT_SEL  0xffffff   /* 橙底上的白字 */
#define DVR_COLOR_NAVY      0x1c2e4d   /* 未选中导航/图标深蓝 */
#define DVR_COLOR_FOCUS     0x00ff00   /* 列表焦点行绿字 */
#define DVR_COLOR_WHITE     0xffffff
#define DVR_COLOR_DIM       0x8a6a4a

/* 子视图 */
typedef enum {
    DVR_SUB_MAIN = 0,     /* 预览 + 主 dock */
    DVR_SUB_CAM_SW,       /* 摄头切换 dock（前/后/抓拍） */
    DVR_SUB_LIST_SEL,     /* 待机页：选前/后摄头再进列表 */
    DVR_SUB_LIST,         /* 文件列表 */
    DVR_SUB_LIST_ACT,     /* 列表上选"播放/删除" */
    DVR_SUB_SETTING,      /* 设置 */
    DVR_SUB_SET_EDIT,     /* 设置子项编辑（循环时长 / 格式化） */
    DVR_SUB_POPUP,        /* 确认弹窗 / 无 SD 卡提示 */
    DVR_SUB_PLAYBACK,     /* 回放（全屏预览，无 dock） */
} dvr_sub_e;

/* 弹窗来源 */
typedef enum {
    DVR_POP_FILE_DEL = 0,
    DVR_POP_FORMAT,
    DVR_POP_FACTORY_RST,
    DVR_POP_NO_SD,
} dvr_pop_e;

/* 触摸点击区域；index 的含义随 area 而定 */
typedef enum {
    DVR_CLICK_DOCK = 0,      /* index 0..3：预览 / 视频回放 / 照片回放 / 设置 */
    DVR_CLICK_CAM,           /* index 0=前摄 1=后摄 2=抓拍 */
    DVR_CLICK_TAB_LEFT,      /* 待机页/列表页 左页签 */
    DVR_CLICK_TAB_RIGHT,     /* 右页签 */
    DVR_CLICK_ROW,           /* index 0..5：选中该行 */
    DVR_CLICK_ROW_PLAY,      /* index 0..5：播放该行 */
    DVR_CLICK_ROW_DEL,       /* index 0..5：删除该行 */
    DVR_CLICK_SET_ROW,       /* index 0..3：设置行 */
    DVR_CLICK_SET_LOOP,      /* index 0..2：循环时长候选 */
    DVR_CLICK_SET_FMT,       /* index 0=SD格式化 1=恢复出厂 */
    DVR_CLICK_POPUP_OK,
    DVR_CLICK_POPUP_CANCEL,
    DVR_CLICK_PREVIEW,       /* 回放画面：暂停/继续 */
} dvr_click_e;

/* 文件列表一屏一项（logic 把 vs_dvr_file_t 转成这个再交给 view） */
typedef struct {
    char     name[32];   /* MOVI0001.avi */
    char     time[20];   /* "09-28 10:00"，保留字段 */
    uint16_t file_no;    /* 固件文件号，回放/删除用 */
    bool     locked;
} dvr_view_item_t;

/* 资源路径：rel 是 ui/assets/image/dvr/ 下的相对名，如 "dock_bg.png"。
 * 传字符串字面量（按指针缓存，同一字面量只拼一次、字符串常驻）。 */
const char *dvr_view_asset(const char *rel);

/* 软解画布的像素缓冲（dvr_page.c 建 lv_canvas 时用它 set_buffer） */
void *dvr_view_preview_buffer(void);

void dvr_view_init(void);

/* ---------------- 子视图 ---------------- */
/* 切视图，并在需要时开启预览 */
void dvr_view_show_sub(dvr_sub_e sub);

/* ---------------- 焦点 ---------------- */
void dvr_view_dock_focus(int index);      /* 0..3 */
void dvr_view_cam_focus(int index);       /* 0=前 1=后 2=抓拍 */
void dvr_view_tab_focus(int right);       /* 0=左 1=右（待机页/列表页共用同一状态） */
void dvr_view_row_focus(int index);       /* 列表焦点行 0..5，<0 = 无 */
void dvr_view_row_action(int index, int del);  /* LIST_ACT：index 行，del=0 播放 1 删除 */
void dvr_view_set_focus(int row);         /* 设置页焦点行 0..3 */
void dvr_view_loop_sel(int v);            /* 循环时长选中标记 0..2 */
void dvr_view_fmt_sel(int f);             /* 格式化/恢复出厂选中标记 0/1 */

/* ---------------- 内容 ---------------- */
void dvr_view_list_fill(const dvr_view_item_t *items, int n);  /* n<=0 清空 */
void dvr_view_list_no_sd(bool show);
void dvr_view_tab_text(int mode);         /* mode 0=视频 1=照片：页签文案跟着变 */
void dvr_view_set_version(const char *ver);
/* 语言切换后重刷本页静态文案（文字来自 aic_get_translation） */
void dvr_view_apply_language(void);
void dvr_view_popup(dvr_pop_e src, const char *title);
void dvr_view_popup_focus(int f);

/* ---------------- 预览 ---------------- */
/* 没有像素可画时在预览区居中显示一行提示（离线 / 启动中 / 硬解直铺 overlay） */
void dvr_view_show_preview_hint(const char *text);
/* 软解路径：把一帧 RGBA8888 像素贴进画布（每行 w*4 字节） */
void dvr_view_draw_preview(const uint8_t *rgba, uint32_t w, uint32_t h);

#endif /* DVR_VIEW_H__ */

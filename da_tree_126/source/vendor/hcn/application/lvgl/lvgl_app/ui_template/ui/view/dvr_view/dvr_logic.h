#ifndef DVR_LOGIC_H__
#define DVR_LOGIC_H__

#include "ui_init.h"
#include "aic_ui.h"
#include "dvr_view.h"

/*
 * DVR 页面控制层：维护子视图状态，处理 UP/DOWN/SET/BACK 与触摸动作。
 * 所有数据经 vehicle_services 的 vs_dvr_* 接口获取。
 *
 *
 * 线程模型：中间件 dvr_mw 内部有自己的 reader/decode 线程取流解码并维护状态快照；
 * 本层在 LVGL 主线程挂 100ms 定时器轮询快照、取新帧，避免跨线程操作 UI。
 */

/* 实体键动作（dvr_page_key.c 只发这三个 + dvr_logic_back） */
typedef enum {
    DVR_ACTION_UP = 0,
    DVR_ACTION_DOWN,
    DVR_ACTION_SELECT,
} dvr_action_e;

void dvr_page_init(void);        /* 建完控件后调用：挂定时器 + 拉状态 + 起预览 */
void on_dvr_page_destory(void);  /* 页面销毁：停定时器/回放/取流 */

void dvr_logic_action(dvr_action_e action);        /* 实体键 UP/DOWN/SET */
void dvr_logic_update_language(void);              /* 语言切换后重刷本页动态文案 */
void dvr_logic_click(dvr_click_e area, int index); /* 触摸点击（dvr_page.c 转发） */

/* 实体键 BACK：页面内部处理掉了返回 true（不退出本页）；
 * 已经在主视图（无路可退）返回 false，由调用方退出页面。 */
bool dvr_logic_back(void);

#endif /* DVR_LOGIC_H__ */

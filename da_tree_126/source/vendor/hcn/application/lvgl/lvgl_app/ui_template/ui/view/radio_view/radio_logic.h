#ifndef RADIO_LOGIC_H__
#define RADIO_LOGIC_H__

#include "ui_init.h"
#include "aic_ui.h"

/*
 * 收音机页面控制层：拉中间件状态 -> 调 view 刷新；接收按钮动作 -> 调中间件。
 *
 * 中间件一律经 vehicle_services 的 vs_radio_* 门面（ui/ 不 include <mw/...>）。
 * PC 模拟器上由 platform/pc/mock_radio_mw.c + mock_vehicle_services.c 顶替，
 * 上真机后同一份 ui/ 代码走 device 侧 vehicle_services.c 转发真 radio_mw。
 */

/* 页面上的按钮动作（radio_page.c 建按钮时按这个 id 挂到 user_data） */
typedef enum {
    RADIO_ACTION_PREV = 0,   /* 上一台 */
    RADIO_ACTION_NEXT,       /* 下一台 */
    RADIO_ACTION_STEP_UP,    /* 步进+ */
    RADIO_ACTION_STEP_DOWN,  /* 步进- */
    RADIO_ACTION_SEEK_UP,    /* 搜台↑ */
    RADIO_ACTION_SEEK_DOWN,  /* 搜台↓ */
    RADIO_ACTION_BAND,       /* 切换波段 */
    RADIO_ACTION_AS,         /* AS 自动搜台 */
    RADIO_ACTION_PS_PLAY,    /* 预置循环播放 */
    RADIO_ACTION_PS_STOP,    /* 停止 */
    RADIO_ACTION_MUTE,       /* 静音开关 */
    RADIO_ACTION_LOCAL,      /* 本地/远地 */
} radio_action_e;

void radio_page_init(void);
void on_radio_page_destory(void);

/* 按钮动作派发（radio_page.c 的点击回调统一从这里进） */
void radio_logic_action(radio_action_e action);

#endif /* RADIO_LOGIC_H__ */
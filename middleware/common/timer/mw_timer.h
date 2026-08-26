#ifndef __MW_TIMER_H__
#define __MW_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"

/**
 * @file mw_timer.h
 * @brief 软件定时器服务 -- 薄封装，委托 OSAL 定时器原语
 *
 * 改造后 mw_timer_t 别名 osal_timer_t，所有 API 转调 osal_timer_*。
 * daemon 线程由 osal_init()（Linux）启动，RTOS 由内核定时器托管。
 * 原 mw_timer.c 的单链表/cond 实现整体上移至 osal_linux.c，逻辑不变。
 * 接口与旧版完全一致，现有调用方无需修改。
 */
typedef osal_timer_t mw_timer_t;
typedef osal_timer_cb_t mw_timer_cb_t;

/** 启动定时器服务（等价 osal_init，幂等）。通常由 mw_init() 调用一次。返回 0
 * 成功。 */
int mw_timer_init(void);

/** 停止定时器服务。OSAL 版本无独立 deinit，兼容空实现。 */
void mw_timer_deinit(void);

mw_timer_t* mw_timer_create(mw_timer_cb_t cb, void* param, bool auto_reload);
int mw_timer_start(mw_timer_t* t, uint32_t period_ms);
int mw_timer_stop(mw_timer_t* t);
int mw_timer_reset(mw_timer_t* t);
int mw_timer_change_period(mw_timer_t* t, uint32_t period_ms);
void mw_timer_delete(mw_timer_t* t);

#ifdef __cplusplus
}
#endif

#endif /* __MW_TIMER_H__ */

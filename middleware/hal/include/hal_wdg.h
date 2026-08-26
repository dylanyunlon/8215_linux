#ifndef __HAL_WDG_H__
#define __HAL_WDG_H__

/**
 * @file hal_wdg.h
 * @brief 看门狗 HAL -- 极简接口（初始化/喂狗），板级 ops 覆盖优先
 *
 * 后端：Linux /dev/watchdog；RT-Thread wdt 设备(RT_USING_WDT)；
 *       FreeRTOS 仅板级 ops。硬件狗不可用时（无驱动/未注册 ops）
 *       init 返回 -1、feed 空转 -- 上层线程健康监测照常工作，
 *       仅失去硬件复位兜底。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** 板级看门狗操作表（覆盖默认后端路径） */
typedef struct {
    int (*init)(uint32_t timeout_ms); /**< 启动并设置超时 */
    int (*feed)(void);                /**< 喂狗 */
} hal_wdg_ops_t;

/** 启动看门狗并设置超时（返回 0 成功；无硬件狗返回 -1，可忽略） */
int hal_wdg_init(uint32_t timeout_ms);

/** 喂狗（未初始化/无硬件狗时空转返回 -1） */
int hal_wdg_feed(void);

/** 板级操作表注册（hal_bsp_register 聚合调用，亦可直接用） */
void hal_wdg_register_board(const hal_wdg_ops_t* ops);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_WDG_H__ */

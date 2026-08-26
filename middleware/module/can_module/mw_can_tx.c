/**
 * @file mw_can_tx.c
 * @brief CAN 周期发送模块 -- 周期报文调度（1ms 粒度轮询，同原版）
 *
 * 改造说明（原 FreeRTOS 版 -> OSAL + HAL 三系统同源，业务逻辑不变）：
 *   - xTaskCreate/vTaskDelay -> osal_thread_create / osal_delay_ms
 *   - xTaskGetTickCount      -> osal_tick_ms（ms 单调时钟，回绕安全比较）
 *   - iCanWrite              -> hal_can_send
 *   - CanMsg                 -> struct hal_can_frame
 *   - 原版 HCN_CAN_TX_ENABLE 裁剪宏并入模块级 MW_CAN_MODULE_ENABLE
 */

#include <stdint.h>
#include <stdbool.h>

#include "mw_log.h"
#include "osal.h"
#include "hal.h"
#include "mw_can_tx.h"

/** 周期报文发送回调 */
typedef void (*can_tx_fn)(hal_can_t* can, void* param);

/** 周期报文描述：ID + 下次发送时刻 + 周期 + 发送函数 */
typedef struct {
    uint32_t id;      ///< 报文 ID（诊断用）
    uint32_t next_ms; ///< 下次发送时刻（ms，开机后延迟启动）
    uint32_t cycle_ms; ///< 发送周期（ms）
    can_tx_fn fn;     ///< 组帧+发送函数
    void* param;      ///< 发送函数私有参数
} can_tx_cycle_t;

static void tx_450_msg(hal_can_t* can, void* param);

/** 周期报文表（★新增周期报文在此追加一行） */
static can_tx_cycle_t s_tx_cycle[] = {
    /* id,    next_ms, cycle_ms, fn,        param */
    {0x450, 5, 100, tx_450_msg, NULL},
};

#define TX_CYCLE_COUNT (sizeof(s_tx_cycle) / sizeof(s_tx_cycle[0]))

static void tx_450_msg(hal_can_t* can, void* param) {
    (void)param;
    static uint8_t tcs_on_off_status = 0;

    struct hal_can_frame frame = {0};
    frame.id = 0x450;
    frame.dlc = 8;

    tcs_on_off_status = !tcs_on_off_status;
    frame.data[0] = tcs_on_off_status;

    hal_can_send(can, &frame, 1, OSAL_WAIT_FOREVER);
}

static void can_tx_cycle_proc(hal_can_t* can, uint32_t now) {
    for (unsigned i = 0; i < TX_CYCLE_COUNT; i++) {
        can_tx_cycle_t* p = &s_tx_cycle[i];
        if ((int32_t)(now - p->next_ms) >= 0) { /* 回绕安全 */
            p->next_ms = now + p->cycle_ms;
            p->fn(can, p->param);
        }
    }
}

static void* can_tx_thread(void* arg) {
    hal_can_t* can = (hal_can_t*)arg;
    uint32_t last_ms = 0;

    for (;;) {
        uint32_t now = (uint32_t)osal_tick_ms();
        if (now != last_ms) { /* 同 1ms 内不重复调度（同原版） */
            last_ms = now;
            can_tx_cycle_proc(can, now);
        }
        osal_delay_ms(1);
    }
    return NULL;
}

int mw_can_tx_start(hal_can_t* can) {
    if (!can) {
        mw_log_error("can port is null!\n");
        return -1;
    }

    if (osal_thread_create(NULL, "can_tx", can_tx_thread, can, 0, 0) !=
        OSAL_OK) {
        mw_log_error("Failed to create CAN cyclic task\n");
        return -1;
    }

    return 0;
}

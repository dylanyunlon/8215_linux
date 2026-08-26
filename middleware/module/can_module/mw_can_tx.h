#ifndef __MW_CAN_TX_H__
#define __MW_CAN_TX_H__

/**
 * @file mw_can_tx.h
 * @brief CAN 周期发送内部接口（mw_can_rx.c 的 init 调用，非对外 API）
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_can.h"

/**
 * @brief  启动周期发送线程（1ms 粒度调度，同原版）
 * @param  can 已打开的 CAN 通道句柄（can_module_init 打开后传入）
 * @return 0:成功  -1:失败
 */
int mw_can_tx_start(hal_can_t* can);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //__MW_CAN_TX_H__

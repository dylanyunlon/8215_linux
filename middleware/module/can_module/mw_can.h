#ifndef __MW_CAN_H__
#define __MW_CAN_H__

/**
 * @file mw_can.h
 * @brief CAN 业务模块对外接口（模块层，基于 hal_can + OSAL）
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief  CAN 模块初始化：打开通道、拉 STB、拉起收/发线程
 * @return 0:成功  -1:失败
 * @note   由 mw_init() 在 MW_CAN_MODULE_ENABLE 时调用
 */
int can_module_init(void);

/**
 * @brief  CAN 通信是否正常（3s 内收到过有效帧）
 * @return true:正常  false:无通信
 */
bool can_get_communication_status(void);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //__MW_CAN_H__

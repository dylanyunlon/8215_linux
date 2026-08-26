#ifndef __HAL_H__
#define __HAL_H__

/**
 * @file hal.h
 * @brief HAL 入口 -- 统一转包 OSAL（系统抽象）与 HAL（硬件 IO 抽象）两层
 *
 *   OSAL 层 (hal/osal/osal.h)：抽象 线程/延时/互斥/信号量/定时器/日志，
 *                              兼容 Linux(pthread) / RT-Thread / FreeRTOS。
 *   HAL  层 (hal/include/hal_gpio.h / hal_key.h / hal_adc.h / hal_pwm.h /
 *                              hal_i2c.h / hal_can.h / hal_wdg.h)：抽象 硬件 IO（GPIO、按键/PWM/
 *                              I2C/CAN/看门狗、ADC 原始值），极简接口。
 *
 * 两层互相独立：OSAL 不依赖 HAL，HAL 不依赖 OSAL。应用模块只需包含本头，
 * 即获得跨系统的统一原语与设备 IO 接口，源码无需为不同 OS 修改。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "osal.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hal_adc.h"
#include "hal_pwm.h"
#include "hal_i2c.h"
#include "hal_can.h"
#include "hal_wdg.h"

#ifdef __cplusplus
}
#endif
#endif /* __HAL_H__ */

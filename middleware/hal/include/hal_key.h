#ifndef __HAL_KEY_H__
#define __HAL_KEY_H__

/**
 * @file hal_key.h
 * @brief 极简按键采集 HAL -- ADC 按键所需的唯一新原语
 *
 * ADC 按键（电压梯复用一路 ADC）跨系统只差"读一路 ADC 原始值"：
 *   - Linux     : iio sysfs（后端默认，可板级回调覆盖）
 *   - FreeRTOS  : 板级回调注册（无统一 ADC 框架，同 hal_gpio_freertos.c 模式）
 *   - RT-Thread : 板级回调优先（如 AIC 触摸屏 RTP 复用 ADC 场景），
 *                 无回调时兜底 rt_device ADC 框架
 *
 * GPIO 按键（每键一个独立引脚）直接复用 hal_gpio.h（gpio_get/gpio_read），
 * 不经过本层。电压窗口匹配/滤波/长按状态机均在 key_module，保持单一实现。
 *
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int (*hal_key_adc_read_fn)(uint8_t channel);

/**
 * 读一路 ADC 原始值（同步、立即返回，不做滤波）。
 * @param channel  ADC 通道号（板级语义）
 * @return         >=0 原始值（范围由板级决定，如 12bit: 0~4095）；<0 出错
 */
int hal_key_adc_read(uint8_t channel);

/**
 * 板级注册 ADC 读取回调（覆盖后端默认实现；传 NULL 恢复默认）。
 * 适用：无 ADC 框架的 OS、或特殊复用通道（如 AIC 触摸屏 RTP 复用的 ADC，
 * 由 BSP 从触摸设备 control 读取后注册进来）。
 */
void hal_key_adc_register_board(hal_key_adc_read_fn fn);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_KEY_H__ */

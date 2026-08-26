#ifndef __HAL_ADC_H__
#define __HAL_ADC_H__

/**
 * @file hal_adc.h
 * @brief 极简 ADC HAL -- "读一路 ADC 原始值"的统一原语
 *
 * 消费者：key_module(电压梯按键) / light_sensor(环境光)。跨系统只差
 * 这一个原语：
 *   - Linux     : iio sysfs（后端默认，可板级回调覆盖）
 *   - FreeRTOS  : 板级回调注册（无统一 ADC 框架，同 hal_gpio_freertos.c 模式）
 *   - RT-Thread : 板级回调优先（特殊复用通道场景），无回调时兜底
 *                 rt_device ADC 框架
 *
 * 滤波/窗口匹配/业务判定均在各模块内，保持单一实现。
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件。
 * 注：hal_key.h 的 hal_key_adc_read 与本接口语义相同（历史先行），
 * 新代码一律使用本头。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int (*hal_adc_read_fn)(uint8_t channel);

/**
 * 读一路 ADC 原始值（同步、立即返回，不做滤波）。
 * @param channel  ADC 通道号（板级语义）
 * @return         >=0 原始值（范围由板级决定，如 12bit: 0~4095）；<0 出错
 */
int hal_adc_read(uint8_t channel);

/**
 * 板级注册 ADC 读取回调（覆盖后端默认实现；传 NULL 恢复默认）。
 * 适用：无 ADC 框架的 OS、或特殊复用通道（BSP 从专用驱动读回后注入，
 * 如 AC83xx 内核无 iio ADC 驱动时由 BSP 封装后注册进来）。
 */
void hal_adc_register_board(hal_adc_read_fn fn);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_ADC_H__ */

#ifndef __MW_LIGHT_SENSOR_H__
#define __MW_LIGHT_SENSOR_H__

/**
 * @file mw_light_sensor.h
 * @brief 环境光 ADC 采集 -- hal_adc 之上的无线程取值器
 *
 * 旧版(FreeRTOS/裸机)自带 chip.h 的 adc_get_channel_value 与共享缓存
 * (set/get 两步)；迁移后平台差异全部收敛到 hal/adc 后端（iio 或板级
 * 回调），本模块不自建线程：采样节拍由上层(display_mode 周期线程)
 * 驱动，read 即触发一次硬件读取，无共享状态、无需加锁。
 * 通道号与裁剪宏见 dev_config.h（MW_LIGHT_ADC_CHANNEL /
 * MW_LIGHT_SENSOR_ENABLE）。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dev_config.h"

#if MW_LIGHT_SENSOR_ENABLE

/**
 * @brief  光感初始化（幂等）。探测一次通道可读性并打印结果，
 *         不因硬件暂缺而失败（板级回调可晚于本调用注册）。
 * @return 0:成功
 */
int light_sensor_init(void);

/**
 * @brief  读一次光感 ADC 原始值（同步、无滤波）
 * @return >=0 原始值（范围由板级决定）；<0 读取失败（上层应保留旧值）
 */
int light_sensor_read(void);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __MW_LIGHT_SENSOR_H__ */

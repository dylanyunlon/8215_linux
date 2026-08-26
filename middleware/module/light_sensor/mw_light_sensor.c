/**
 * @file mw_light_sensor.c
 * @brief 环境光 ADC 采集 -- 纯 hal_adc 取值器（无线程/无共享缓存）
 */
#include "mw_light_sensor.h"
#include "hal_adc.h"
#include "mw_log.h"

#if MW_LIGHT_SENSOR_ENABLE

int light_sensor_init(void) {
    int v = hal_adc_read(MW_LIGHT_ADC_CHANNEL);
    if (v < 0) {
        mw_log_warn("light sensor ch%u not ready (board fn later?)\n",
                    (unsigned)MW_LIGHT_ADC_CHANNEL);
    } else {
        mw_log_info("light sensor ch%u init, raw=%d\n",
                    (unsigned)MW_LIGHT_ADC_CHANNEL, v);
    }
    return 0;
}

int light_sensor_read(void) { return hal_adc_read(MW_LIGHT_ADC_CHANNEL); }

#endif

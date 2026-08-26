/** 极简按键采集 HAL -- RT-Thread 后端
 * 板级回调优先（AIC 触摸屏 RTP 复用 ADC 等特殊通道由 BSP 注册回调，
 * 中间件不含任何厂商头文件）；无回调时兜底 rt_device ADC 框架。 */
#include "hal_key.h"

#include <rtthread.h>
#include <rtdevice.h>

#ifndef RT_ADC_CMD_ENABLE
#define RT_ADC_CMD_ENABLE 0 /* rt_adc 框架命令字（框架头未包含时兜底） */
#endif

static const char* s_dev_name = "adc0";
static rt_device_t s_dev = RT_NULL;
static hal_key_adc_read_fn s_board_fn = NULL;

/** 板级指定 rt_device ADC 设备名（默认 "adc0"）。仅本后端有效。 */
void hal_key_adc_rtthread_set_device(const char* name) {
    if (!name || !name[0]) return;
    s_dev_name = name;
    s_dev = RT_NULL; /* 换设备名后强制下次重新 find/open */
}

void hal_key_adc_register_board(hal_key_adc_read_fn fn) {
    s_board_fn = fn;
    if (fn) { /* 切回板级回调，释放框架设备引用 */
        s_dev = RT_NULL;
    }
}

int hal_key_adc_read(uint8_t channel) {
    if (s_board_fn) return s_board_fn(channel);

    if (s_dev == RT_NULL) {
        s_dev = rt_device_find(s_dev_name);
        if (s_dev == RT_NULL) return -1;
        if (rt_device_open(s_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
            s_dev = RT_NULL;
            return -1;
        }
        rt_device_control(s_dev, RT_ADC_CMD_ENABLE,
                          (void*)(rt_uint32_t)channel);
    }
    rt_uint32_t v = (rt_uint32_t)rt_device_read(s_dev, channel, RT_NULL, 0);
    return (int)v;
}

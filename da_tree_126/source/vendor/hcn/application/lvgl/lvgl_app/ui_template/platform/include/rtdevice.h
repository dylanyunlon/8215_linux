/*
 * rtdevice.h —— RT-Thread 设备框架在 PC 上的最小替身
 * AiUIBuilder 生成代码只用到其中的少量类型；硬件访问由中间件层，不在 PC 上编译。
 */
#ifndef RTDEVICE_H_SHIM
#define RTDEVICE_H_SHIM

#include <rtthread.h>

#define PIN_LOW           0
#define PIN_HIGH          1
#define PIN_MODE_OUTPUT   0
#define PIN_MODE_INPUT    1

struct rt_device {
    int dummy;
};

#endif /* RTDEVICE_H_SHIM */
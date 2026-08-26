/** 看门狗 HAL -- RT-Thread 后端：板级 ops 覆盖优先；
 * RT_USING_WDT 启用时兜底 wdt 设备（默认设备名 "wdt"），
 * 未启用框架时本后端仅板级 ops（编译不破）。 */
#include "hal_wdg.h"

#ifdef RT_USING_WDT
#include <rtthread.h>
#include <rtdevice.h>

#ifndef RT_DEVICE_CTRL_WDT_GET_TIMEOUT
#define RT_DEVICE_CTRL_WDT_GET_TIMEOUT 0x11 /* 与 rt_hw_watchdog.h 一致 */
#define RT_DEVICE_CTRL_WDT_SET_TIMEOUT 0x12
#define RT_DEVICE_CTRL_WDT_START      0x13
#define RT_DEVICE_CTRL_WDT_STOP       0x14
#define RT_DEVICE_CTRL_WDT_KEEPALIVE  0x15
#endif
#endif /* RT_USING_WDT */

static const hal_wdg_ops_t* s_board_ops = NULL;
#ifdef RT_USING_WDT
static rt_device_t s_wdt = RT_NULL;
#endif

void hal_wdg_register_board(const hal_wdg_ops_t* ops) { s_board_ops = ops; }

int hal_wdg_init(uint32_t timeout_ms) {
    if (s_board_ops && s_board_ops->init)
        return s_board_ops->init(timeout_ms);

#ifdef RT_USING_WDT
    if (s_wdt == RT_NULL) {
        s_wdt = rt_device_find("wdt");
        if (s_wdt == RT_NULL) return -1;
    }
    rt_err_t err = rt_device_open(s_wdt, RT_DEVICE_OFLAG_RDWR);
    if (err != RT_EOK && err != RT_EBUSY && err != -RT_EBUSY) return -1;
    int sec = (int)((timeout_ms + 999) / 1000);
    if (sec < 1) sec = 1;
    rt_device_control(s_wdt, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &sec);
    rt_device_control(s_wdt, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    return 0;
#else
    (void)timeout_ms;
    return -1; /* 无框架无板级 ops：无硬件狗 */
#endif
}

int hal_wdg_feed(void) {
    if (s_board_ops && s_board_ops->feed) return s_board_ops->feed();

#ifdef RT_USING_WDT
    if (s_wdt == RT_NULL) return -1;
    return rt_device_control(s_wdt, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
#else
    return -1;
#endif
}

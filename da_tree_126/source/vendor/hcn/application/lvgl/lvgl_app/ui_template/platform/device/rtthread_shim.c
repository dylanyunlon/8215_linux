/*
 * rtthread_shim.c —— RT-Thread 内核 API 在 Linux 上的 POSIX 替身（device 后端）
 *
 * ui/ 里只有两处用到 RT-Thread：
 *   ui/logic/mileage_calc.c   rt_thread_create/startup/delay_until, rt_tick_get, rt_kprintf
 *   ui/view/setting_view/nfc_view.c   rt_thread_mdelay
 *
 * 声明沿用 platform/include/rtthread.h（两个后端共用同一份契约头）。
 * 实现在 platform/pc/mock_rtthread.c 里已有一份 POSIX 分支，这里是同一套逻辑
 * 去掉 Windows 分支后的版本。
 *
 * ⚠ rt_thread_create 必须**异步**拉起线程：mileage_calc 的 calc_task() 是
 *   while(1) 周期任务，同步调用入口会让 ui_init() 永不返回，主线程到不了
 *   lv_timer_handler() 循环 —— 表现就是黑屏 + 关不掉窗口。
 */
#include <rtthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

static void posix_sleep_ms(unsigned int ms)
{
    usleep((useconds_t)ms * 1000u);
}

static rt_uint32_t posix_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (rt_uint32_t)(ts.tv_sec * 1000u + (rt_uint32_t)(ts.tv_nsec / 1000000L));
}

/* 线程控制块：create 只登记，startup 才真正拉起（与 RT-Thread 两段式语义一致） */
typedef struct {
    const char *name;
    void      (*entry)(void *parameter);
    void       *parameter;
    pthread_t   hdl;
    int         started;
} rt_shim_cb_t;

static void *rt_shim_trampoline(void *arg)
{
    rt_shim_cb_t *cb = (rt_shim_cb_t *)arg;
    if (cb != NULL && cb->entry != NULL) cb->entry(cb->parameter);
    return NULL;
}

rt_tick_t rt_tick_get(void)
{
    /* 用墙钟毫秒（不是 CPU 时间），否则周期任务节拍会失真 */
    return (rt_tick_t)posix_now_ms();
}

void rt_kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
}

rt_thread_t rt_thread_create(const char *name, void (*entry)(void *parameter), void *parameter,
                             rt_uint32_t stack_size, rt_uint8_t priority, rt_uint32_t tick)
{
    rt_shim_cb_t *cb;

    (void)stack_size; (void)priority; (void)tick;

    cb = (rt_shim_cb_t *)calloc(1, sizeof(*cb));
    if (cb == NULL) return RT_NULL;

    cb->name      = name;
    cb->entry     = entry;
    cb->parameter = parameter;
    return (rt_thread_t)cb;
}

rt_err_t rt_thread_startup(rt_thread_t thread)
{
    rt_shim_cb_t *cb = (rt_shim_cb_t *)thread;
    if (cb == NULL || cb->started) return RT_ERROR;
    if (pthread_create(&cb->hdl, NULL, rt_shim_trampoline, cb) != 0) return RT_ERROR;
    cb->started = 1;
    return RT_EOK;
}

rt_err_t rt_thread_mdelay(rt_int32_t ms)
{
    if (ms > 0) posix_sleep_ms((unsigned int)ms);
    return RT_EOK;
}

rt_err_t rt_thread_delay(rt_tick_t tick)
{
    if (tick > 0) posix_sleep_ms((unsigned int)tick);
    return RT_EOK;
}

rt_err_t rt_thread_delay_until(rt_tick_t *prev, rt_tick_t inc)
{
    rt_tick_t  target;
    rt_int32_t remain;

    if (prev == NULL || inc == 0) return RT_EOK;

    target = *prev + inc;
    remain = (rt_int32_t)(target - (rt_tick_t)posix_now_ms());
    if (remain > 0) posix_sleep_ms((unsigned int)remain);

    *prev = target;
    return RT_EOK;
}

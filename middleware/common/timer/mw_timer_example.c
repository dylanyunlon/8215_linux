/**
 * mw_timer_example.c -- mw_timer 使用示例
 *
 * 演示：周期定时器、单次定时器、回调内操作其他定时器（stop）、reset
 * 重启、delete、deinit。
 *
 * ⚠ 本文件含 main()，仅为使用示范，请勿加入 lib_mw.a 的 SRCS。
 *
 * 独立编译验证（在中间件根目录 source/vendor/hcn/middleware 下执行）：
 *   arm-buildroot-linux-gnueabi-gcc -std=gnu11 -Wall -O2 -pthread \
 *       -I common/lock -I common/log -I common/timer \
 *       common/timer/mw_timer.c \
 *       common/timer/mw_timer_example.c \
 *       -o mw_timer_example -lpthread
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> /* sleep */

#include "mw_timer.h"

static mw_timer_t* s_periodic; /* 周期定时器句柄 */

/** 周期回调：每秒打印一次 */
static void on_periodic(void* p) {
    (void)p;
    printf("[periodic] tick\n");
}

/** 单次回调：3.5s 时触发，停掉周期定时器（在回调内操作其他定时器，安全）*/
static void on_oneshot(void* p) {
    (void)p;
    printf("[one-shot] fire -> stop periodic\n");
    mw_timer_stop(s_periodic);
}

int main(void) {
    mw_timer_t* one;

    if (mw_timer_init() != 0) {
        fprintf(stderr, "timer init failed\n");
        return 1;
    }

    /* 周期定时器：每 1000ms */
    s_periodic = mw_timer_create(on_periodic, NULL, true);
    mw_timer_start(s_periodic, 1000);

    /* 单次定时器：3500ms 后停掉周期定时器 */
    one = mw_timer_create(on_oneshot, NULL, false);
    mw_timer_start(one, 3500);

    printf("run 6s (expect 3 ticks, then stopped)...\n");
    sleep(6);

    /* 演示重新启用：再跑 2s（应打印 2 个 tick）*/
    printf("restart periodic for 2s...\n");
    mw_timer_start(s_periodic, 1000);
    sleep(2);

    mw_timer_delete(s_periodic);
    mw_timer_delete(one);
    mw_timer_deinit();

    printf("done\n");
    return 0;
}

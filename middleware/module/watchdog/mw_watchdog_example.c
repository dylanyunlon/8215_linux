/** mw_watchdog 使用示例 -- 仪表关键线程的典型接入方式（模板，不参与编译）
 *
 * 场景：两轮仪表的两个典型线程
 *   - sensor  快循环（传感器/总线采集），200ms 一帧，心跳超时 1s
 *   - display 慢循环（UI 刷新/慢速轮询），1s 一次，用默认超时 3s
 * 任意线程超过自己的超时没有 kick -> 监测线程停喂硬件狗 -> 整机复位。
 *
 * 接入纪律（防误杀）：
 *   1. kick 周期 <= timeout/3（留 2 个周期的调度抖动余量）
 *   2. 阻塞调用（fgets/select/长自旋）不得持有心跳：拆线程或换超时版本
 *   3. 注册放线程入口处，失败立即让线程退出（别裸奔）
 *
 * 启用方法：
 *   1. 本文件加入 Makefile SRCS（或工程直接编译）
 *   2. mw_init 之后、创建业务线程之前调用 mw_watchdog_example()
 *      （真实工程去掉示例线程，把 register/kick 放进你的线程）
 *   3. 演示卡死检测：编译加 -DWDG_EXAMPLE_INJECT_STALE
 *      -- 硬件狗存在时 5s 内整机复位（这正是设计兜底，勿在量产固件开）
 */
#include "mw_watchdog.h"
#include "mw_pthread.h"
#include "mw_log.h"
#include "osal.h"
#include <stdio.h>

/* ---------------- 1. 快循环线程：传感器/总线采集 ----------------
 * 200ms 一帧，注册超时 1000ms（kick 周期的 5 倍，抖动余量充足） */
static void* sensor_thread(void* arg) {
    (void)arg;
    int id = mw_watchdog_register("sensor", 1000);
    if (id < 0) {
        mw_log_error("[example] sensor 心跳槽注册失败，线程退出\n");
        return NULL;
    }
    while (1) {
        /* ... 采集一帧（CAN/ADC/串口）... */
        mw_watchdog_kick(id);      /* 干完一票活就喂一次心跳 */
        osal_delay_ms(200);
    }
    return NULL;
}

/* ---------------- 2. 慢循环线程：显示/慢速轮询 ----------------
 * 1s 一次，timeout 传 0 用默认值 MW_WDG_DEFAULT_TIMEOUT_MS(3s) */
static void* display_thread(void* arg) {
    (void)arg;
    int id = mw_watchdog_register("display", 0);
    if (id < 0) {
        mw_log_error("[example] display 心跳槽注册失败，线程退出\n");
        return NULL;
    }
    while (1) {
        /* ... 刷新/轮询 ... */
        mw_watchdog_kick(id);
        osal_delay_ms(1000);
    }
    return NULL;
}

#ifdef WDG_EXAMPLE_INJECT_STALE
/* ---------------- 3.（可选）卡死注入：演示故障路径 ----------------
 * 注册成功但从不 kick：约 2s 后监测线程打出 stale 错误日志，
 * modules 表 watchdog 行 chk 变 FAULT；硬件狗存在且超时(2s+2x0.5s+1s=4s)
 * 到点后整机复位。 */
static void* stuck_thread(void* arg) {
    (void)arg;
    mw_watchdog_register("stuck", 2000);
    mw_log_warn("[example] STALE 注入：本线程不再 kick，等待判死\n");
    while (1) {
        osal_delay_ms(60000); /* 模拟死等/死循环/优先级反转 */
    }
    return NULL;
}
#endif

/* ---------------- 入口：mw_init 之后调用 ---------------- */
int mw_watchdog_example(void) {
    /* 幂等：重复调用返回 0；硬件狗不可用时自动退化为纯软件监测 */
    if (mw_watchdog_init() != 0) {
        mw_log_error("[example] watchdog init failed\n");
        return -1;
    }

    mw_pthread_create("sensor", NULL, sensor_thread);
    mw_pthread_create("display", NULL, display_thread);
#ifdef WDG_EXAMPLE_INJECT_STALE
    mw_pthread_create("stuck", NULL, stuck_thread);
#endif

    /* 观测面（应用任意位置可查）：
     *   mw_watchdog_healthy()  -- 0=全部新鲜；注册表 self_check 同款判定
     *   mw_watchdog_dump()     -- 心跳全表（CLI `wdg` 命令同款）
     *   mw_module_dump()       -- `modules` 命令，watchdog 行 chk=ok/FAULT
     */
    osal_delay_ms(3000); /* 等线程跑起来 */
    printf("[example] healthy = %d (0=OK)\n", mw_watchdog_healthy());
    mw_watchdog_dump();

    /* 常驻进程不需要 deinit（喂狗应持续到断电）；
     * 仅独立测试程序收尾时调用：
     *   mw_watchdog_deinit();
     */
    return 0;
}

#ifndef __MW_WATCHDOG_H__
#define __MW_WATCHDOG_H__

/**
 * @file mw_watchdog.h
 * @brief 看门狗 + 线程心跳健康监测
 *
 * 机制：关键线程注册心跳槽并周期 kick；监测线程每
 * MW_WDG_CHECK_PERIOD_MS 巡检一次，全部线程心跳新鲜才喂硬件狗
 * （任一线程卡死 -> 停止喂狗 -> 硬件复位整机）。硬件狗不可用时
 * （无 /dev/watchdog、未注册板级 ops）退化为纯软件健康监测。
 *
 * 启动初期 MW_WDG_BOOT_GRACE_MS 内无条件喂狗（开机动画/参数加载
 * 阶段线程可能尚未开始心跳）。
 *
 * 健康状态经注册表暴露：mw_watchdog_init() 自动向模块注册表挂
 * self_check（MW_MOD_WATCHDOG），`modules` 命令可见 ok/FAULT。
 *
 * 用法：
 *   mw_watchdog_init();
 *   int id = mw_watchdog_register("can_rx", 2000); // 0=默认超时
 *   ... 线程循环内：mw_watchdog_kick(id);
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dev_config.h"

#if MW_WATCHDOG_ENABLE

#define MW_WDG_MAX_THREADS 8           ///< 最大心跳线程数
#define MW_WDG_DEFAULT_TIMEOUT_MS 3000 ///< 默认心跳超时
#define MW_WDG_CHECK_PERIOD_MS 500     ///< 巡检周期
#define MW_WDG_BOOT_GRACE_MS 15000     ///< 启动宽限（此窗口内无条件喂狗）

/** 启动监测线程 + 初始化硬件狗（幂等，重复调用返回 0）。失败返回 -1 */
int mw_watchdog_init(void);

/** 停止监测线程（进程退出前调用；喂狗随线程停止） */
int mw_watchdog_deinit(void);

/** 注册心跳槽：name 诊断用，timeout_ms=0 用默认值。
 *  返回槽 id（>=0）；未 init/表满返回 -1。
 *  注册更慢的线程时硬件狗超时自动升级（保证 > 最慢心跳）。 */
int mw_watchdog_register(const char* name, uint32_t timeout_ms);

/** 心跳喂狗（id 非法时忽略）。热路径：锁内仅一次时间戳写 */
void mw_watchdog_kick(int id);

/** 距该线程上次心跳的毫秒数（id 非法/未注册返回 UINT32_MAX） */
uint32_t mw_watchdog_since_ms(int id);

/** 全部线程心跳是否新鲜：0=健康，-1=有线程超时（注册表 self_check 同款） */
int mw_watchdog_healthy(void);

/** 打印心跳健康表（CLI/诊断用） */
void mw_watchdog_dump(void);

#else /* MW_WATCHDOG_ENABLE=0：空转内联桩，调用点无需 #if 包裹 */

static inline int mw_watchdog_init(void) { return -1; }
static inline int mw_watchdog_deinit(void) { return 0; }
static inline int mw_watchdog_register(const char* name,
                                       uint32_t timeout_ms) {
    (void)name;
    (void)timeout_ms;
    return -1;
}
static inline void mw_watchdog_kick(int id) { (void)id; }
static inline uint32_t mw_watchdog_since_ms(int id) {
    (void)id;
    return (uint32_t)-1;
}
static inline int mw_watchdog_healthy(void) { return 0; }
static inline void mw_watchdog_dump(void) {}

#endif /* MW_WATCHDOG_ENABLE */

#ifdef __cplusplus
}
#endif
#endif /* __MW_WATCHDOG_H__ */

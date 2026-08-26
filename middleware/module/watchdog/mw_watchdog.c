/** 看门狗 + 线程心跳健康监测
 * 全线程心跳新鲜才喂硬件狗；卡死即停喂、由硬件复位兜底。
 * 硬件狗不可用时退化为纯软件健康监测（FAULT 仍可在注册表看到）。 */
#include "mw_watchdog.h"

#if MW_WATCHDOG_ENABLE

#include "mw_modules.h"
#include "hal_wdg.h"
#include "mw_lock.h"
#include "mw_pthread.h"
#include "mw_log.h"
#include "osal.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int used;
    char name[16];
    uint32_t timeout_ms;
    uint64_t last_kick; /* osal_tick_ms 快照 */
    int stale;          /* 状态迁移日志用：0->1 报超时，1->0 报恢复 */
} wdg_slot_t;

static wdg_slot_t s_slots[MW_WDG_MAX_THREADS];
static osal_mutex_t s_lock = OSAL_MUTEX_INIT;
static volatile int s_running = 0;
static osal_sem_t* s_done = NULL;
static uint32_t s_hw_timeout_ms = 0; /* 0=硬件狗不可用 */
static uint64_t s_grace_until = 0;

/** 硬件狗超时 = 最慢线程心跳 + 2 巡检周期 + 1s 余量；只升不降 */
static int ensure_hw_timeout(uint32_t need_ms) {
    uint32_t want = need_ms + 2 * MW_WDG_CHECK_PERIOD_MS + 1000;
    if (want <= s_hw_timeout_ms) return 0;
    if (hal_wdg_init(want) != 0) {
        s_hw_timeout_ms = 0;
        mw_log_warn("[mw_wdg] hw watchdog unavailable, software-only\n");
        return -1;
    }
    s_hw_timeout_ms = want;
    return 0;
}

static void* monitor_thread(void* arg) {
    (void)arg;
    while (s_running) {
        uint64_t now = osal_tick_ms();
        int feed;

        { /* 快照巡检 + 状态迁移日志（锁内） */
            MW_MUTEX_GUARD(&s_lock);
            int stale_cnt = 0;
            for (int i = 0; i < MW_WDG_MAX_THREADS; i++) {
                wdg_slot_t* s = &s_slots[i];
                if (!s->used) continue;
                uint64_t since = now - s->last_kick;
                if (since > s->timeout_ms) {
                    stale_cnt++;
                    if (!s->stale) {
                        s->stale = 1;
                        mw_log_error("[mw_wdg] thread '%s' stale %llu ms "
                                     "(timeout %u)\n",
                                     s->name, (unsigned long long)since,
                                     s->timeout_ms);
                    }
                } else if (s->stale) {
                    s->stale = 0;
                    mw_log_warn("[mw_wdg] thread '%s' recovered\n", s->name);
                }
            }
            feed = (stale_cnt == 0) || (now < s_grace_until);
        } /* 锁在此释放 */

        /* 全部新鲜（或宽限期内）才喂狗；卡死则停喂，等硬件复位 */
        if (feed) hal_wdg_feed();

        osal_delay_ms(MW_WDG_CHECK_PERIOD_MS);
    }
    if (s_done) osal_sem_post(s_done);
    return NULL;
}

int mw_watchdog_init(void) {
    if (s_running) return 0;

    memset(s_slots, 0, sizeof(s_slots));
    s_hw_timeout_ms = 0;
    s_grace_until = osal_tick_ms() + MW_WDG_BOOT_GRACE_MS;
    int hw = ensure_hw_timeout(MW_WDG_DEFAULT_TIMEOUT_MS);

    s_done = osal_sem_create(0, 1);
    if (!s_done) return -1;
    s_running = 1;
    if (mw_pthread_create("mw_wdg", NULL, monitor_thread) != 0) {
        s_running = 0;
        osal_sem_delete(s_done);
        s_done = NULL;
        return -1;
    }

    mw_module_register_check(MW_MOD_WATCHDOG, mw_watchdog_healthy);
    mw_log_info("[mw_wdg] init OK (hw watchdog %s)\n",
                hw == 0 ? "on" : "off, software-only");
    return 0;
}

int mw_watchdog_deinit(void) {
    if (!s_running) return 0;
    s_running = 0;
    if (s_done) {
        osal_sem_timedwait(s_done, MW_WDG_CHECK_PERIOD_MS * 4);
        osal_sem_delete(s_done);
        s_done = NULL;
    }
    return 0;
}

int mw_watchdog_register(const char* name, uint32_t timeout_ms) {
    if (!name || !s_running) return -1;
    if (timeout_ms == 0) timeout_ms = MW_WDG_DEFAULT_TIMEOUT_MS;

    /* 硬件狗超时升级放在锁外（hal_wdg_init 可能 open 设备）；
     * 硬件狗不可用也继续注册：纯软件监测 */
    ensure_hw_timeout(timeout_ms);

    MW_MUTEX_GUARD(&s_lock);
    for (int i = 0; i < MW_WDG_MAX_THREADS; i++) {
        if (!s_slots[i].used) {
            s_slots[i].used = 1;
            snprintf(s_slots[i].name, sizeof(s_slots[i].name), "%s", name);
            s_slots[i].timeout_ms = timeout_ms;
            s_slots[i].last_kick = osal_tick_ms();
            s_slots[i].stale = 0;
            return i;
        }
    }
    return -1; /* 表满 */
}

void mw_watchdog_kick(int id) {
    if (id < 0 || id >= MW_WDG_MAX_THREADS) return;
    MW_MUTEX_GUARD(&s_lock);
    if (s_slots[id].used) s_slots[id].last_kick = osal_tick_ms();
}

uint32_t mw_watchdog_since_ms(int id) {
    if (id < 0 || id >= MW_WDG_MAX_THREADS) return (uint32_t)-1;
    MW_MUTEX_GUARD(&s_lock);
    if (!s_slots[id].used) return (uint32_t)-1;
    return (uint32_t)(osal_tick_ms() - s_slots[id].last_kick);
}

int mw_watchdog_healthy(void) {
    uint64_t now = osal_tick_ms();
    MW_MUTEX_GUARD(&s_lock);
    for (int i = 0; i < MW_WDG_MAX_THREADS; i++) {
        if (s_slots[i].used &&
            now - s_slots[i].last_kick > s_slots[i].timeout_ms)
            return -1;
    }
    return 0;
}

void mw_watchdog_dump(void) {
    uint64_t now = osal_tick_ms();
    int n = 0;
    int healthy = 1;

    printf("%-16s %10s %10s %s\n", "thread", "since_ms", "timeout", "state");
    MW_MUTEX_GUARD(&s_lock);
    for (int i = 0; i < MW_WDG_MAX_THREADS; i++) {
        wdg_slot_t* s = &s_slots[i];
        if (!s->used) continue;
        n++;
        uint32_t since = (uint32_t)(now - s->last_kick);
        int stale = since > s->timeout_ms;
        if (stale) healthy = 0;
        printf("%-16s %10u %10u %s\n", s->name, since, s->timeout_ms,
               stale ? "STALE" : "fresh");
    }
    printf("hw watchdog : %s%s\n", s_hw_timeout_ms ? "on" : "off",
           s_hw_timeout_ms ? "" : " (software-only)");
    printf("threads: %d, health: %s\n", n, healthy ? "OK" : "FAULT");
}

#else /* MW_WATCHDOG_ENABLE=0：空翻译单元 */

#endif /* MW_WATCHDOG_ENABLE */

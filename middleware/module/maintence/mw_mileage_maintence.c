/**
 * @brief mw_mileage_maintence.c -- 里程保养提醒(轮询计算剩余保养里程)
 * worker 线程周期读 vehicle_param 总里程(米)折算 km，与 set_param 保存的
 * 上次保养总里程求差；剩余量发布 VEH_MAINT_REMAIN_MILEAGE，行驶里程达到
 * 间隔置 VEH_MAINT_REMINDER=1。范式同 msg_manage(worker + done-sem + self_check)。
 */
#include "mw_mileage_maintence.h"

#if MW_MILEAGE_MAINTENCE_ENABLE

#include <string.h>
#include "mw_modules.h"
#include "mw_lock.h"
#include "mw_pthread.h"
#include "mw_log.h"
#include "osal.h"
#include "set_param.h"
#include "vehicle_param.h"

/* 内部状态(互斥保护)：里程类字段一律 km(set_param 存储约定) */
typedef struct {
    uint16_t count;          ///< 保养次数(set_param 镜像)
    uint16_t interval;       ///< 本轮保养间隔 km
    uint32_t last_km;        ///< 上次保养时总里程 km
    int32_t remain_km;       ///< 剩余保养里程 km(未就绪=-1)
    int32_t last_odo_m;      ///< 上次轮询总里程(米, 去重)
    uint64_t last_poll_tick; ///< 末次 poll 时间戳(self_check 用)
} maintence_state_t;

static maintence_state_t s_state;
static osal_mutex_t s_lock = OSAL_MUTEX_INIT;

static volatile int s_running = 0;
static osal_sem_t* s_done = NULL;

/** 米 -> km 四舍五入(总里程单位为米, 见 vehicle_param.h) */
static int32_t m_to_km(int32_t odo_m) {
    return (odo_m + ((odo_m >= 0) ? 500 : -500)) / 1000;
}

/** 读保养参数并计算剩余里程(锁内调用, 结果记入 s_state)。返回 0=已计算 -1=参数未就绪 */
static int maintence_calc(uint32_t cur_km) {
    uint16_t count = 0;
    uint16_t interval = 0;
    uint32_t last_km = 0;

    if (!get_usr_param(USR_PARAM_MAINTAIN_COUNTS, &count) ||
        !get_usr_param(USR_PARAM_LAST_MAINTAIN_MILEAGE, &last_km) ||
        !get_usr_param(USR_PARAM_CUR_MAINTAIN_MILEAGE, &interval)) {
        return -1;
    }

    /* 间隔：cur_maintain_mileage 参数优先(0=未设置)，否则首保/常规默认 */
    if (interval == 0) {
        interval = (count == 0) ? MW_MAINT_FIRST_KM : MW_MAINT_NEXT_KM;
    }

    uint32_t travel = (cur_km > last_km) ? (cur_km - last_km) : 0;
    int32_t remain = (int32_t)interval - (int32_t)travel;
    if (remain < 0) remain = 0; /* 已超期 */

    s_state.count = count;
    s_state.interval = interval;
    s_state.last_km = last_km;
    s_state.remain_km = remain;
    return 0;
}

static void* poll_thread(void* arg) {
    (void)arg;
    mw_log_info("[maintence] poll thread started, period=%ums", MW_MAINT_POLL_MS);
    while (s_running) {
        int32_t odo_m = vehicle_get_data(VEH_MILEAGE_TOTAL);
        int32_t publish_remain = -2; /* -2=本轮不发布 */
        int reminder = -1;           /* -1=不动作 */

        if (odo_m > 0) {
            MW_MUTEX_GUARD(&s_lock);
            s_state.last_poll_tick = osal_tick_ms();
            if (odo_m != s_state.last_odo_m) {
                s_state.last_odo_m = odo_m;
                if (get_recovery_usr_param() &&
                    maintence_calc((uint32_t)m_to_km(odo_m)) == 0) {
                    /* 到达/超过间隔置提醒(清除只由 mw_maintence_reset 做) */
                    uint32_t cur_km = (uint32_t)m_to_km(odo_m);
                    uint32_t travel =
                        (cur_km > s_state.last_km) ? (cur_km - s_state.last_km) : 0;
                    if (travel >= s_state.interval) reminder = 1;
                    publish_remain = s_state.remain_km;
                }
            }
        } /* 锁释放，发布移到锁外(vehicle_param 自带互斥) */

        if (reminder == 1) {
            vehicle_set_data(VEH_MAINT_REMINDER, 1);
        }
        if (publish_remain >= -1) {
            vehicle_set_data(VEH_MAINT_REMAIN_MILEAGE, publish_remain);
        }

        osal_delay_ms(MW_MAINT_POLL_MS);
    }
    if (s_done) osal_sem_post(s_done);
    mw_log_info("[maintence] poll thread exit");
    return NULL;
}

/** self_check：worker 存活且末次 poll 距今 < 3 倍周期 -> 0，否则 -1 */
static int maintence_selfcheck(void) {
    if (!s_running) return -1;
    uint64_t now = osal_tick_ms();
    uint64_t last;
    {
        MW_MUTEX_GUARD(&s_lock);
        last = s_state.last_poll_tick;
    }
    return (now - last) < (3ull * MW_MAINT_POLL_MS) ? 0 : -1;
}

int mw_maintence_init(void) {
    if (s_running) return 0;
    memset(&s_state, 0, sizeof(s_state));
    s_state.remain_km = -1;
    s_state.last_odo_m = -1;
    s_state.last_poll_tick = osal_tick_ms();

    s_done = osal_sem_create(0, 1);
    if (!s_done) {
        mw_log_error("[maintence] sem create fail");
        return -1;
    }
    s_running = 1;
    if (mw_pthread_create("maintence", NULL, poll_thread) != 0) {
        s_running = 0;
        osal_sem_delete(s_done);
        s_done = NULL;
        mw_log_error("[maintence] thread create fail");
        return -1;
    }
    mw_module_register_check(MW_MOD_MAINTENCE, maintence_selfcheck);
    mw_log_info("[maintence] init OK");
    return 0;
}

int mw_maintence_deinit(void) {
    if (!s_running) return 0;
    s_running = 0;
    if (s_done) {
        osal_sem_timedwait(s_done, MW_MAINT_POLL_MS * 4);
        osal_sem_delete(s_done);
        s_done = NULL;
    }
    return 0;
}

void mw_maintence_reset(void) {
    int32_t odo_m = vehicle_get_data(VEH_MILEAGE_TOTAL);
    if (odo_m <= 0) {
        mw_log_warn("[maintence] reset: invalid odo %d", odo_m);
        return;
    }
    uint32_t cur_km = (uint32_t)m_to_km(odo_m);

    MW_MUTEX_GUARD(&s_lock);
    uint16_t count = 0;
    if (!get_usr_param(USR_PARAM_MAINTAIN_COUNTS, &count)) {
        mw_log_error("[maintence] reset: param not ready");
        return;
    }
    if (count < 0xfffe) {
        count++; /* 0xffff 为 flash 擦除态标记(usr_param_init 归零)，封顶避开 */
    }

    /* 保养次数/上次保养总里程写回并立即落盘(用户触发事件) */
    if (!set_usr_param(USR_PARAM_MAINTAIN_COUNTS, &count) ||
        !set_usr_param(USR_PARAM_LAST_MAINTAIN_MILEAGE, &cur_km)) {
        mw_log_error("[maintence] reset: set usr param fail");
        return;
    }
    if (save_usr_param() != 0) {
        mw_log_error("[maintence] reset: save usr param fail");
    }

    /* 下一轮间隔(次数>0 -> 常规/参数值)，剩余量重置并清除提醒 */
    uint16_t interval = 0;
    (void)get_usr_param(USR_PARAM_CUR_MAINTAIN_MILEAGE, &interval);
    if (interval == 0) interval = MW_MAINT_NEXT_KM;

    s_state.count = count;
    s_state.interval = interval;
    s_state.last_km = cur_km;
    s_state.remain_km = (int32_t)interval;

    vehicle_set_data(VEH_MAINT_REMINDER, 0);
    vehicle_set_data(VEH_MAINT_REMAIN_MILEAGE, (int32_t)interval);
    mw_log_info("[maintence] reset: count=%u last=%ukm interval=%ukm",
                count, cur_km, interval);
}

int mw_maintence_get_remain_km(void) {
    MW_MUTEX_GUARD(&s_lock);
    return s_state.remain_km;
}

#endif /* MW_MILEAGE_MAINTENCE_ENABLE */

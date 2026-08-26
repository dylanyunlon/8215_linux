/** OSAL —— RT-Thread 后端实现（真实参考代码，需 RT-Thread SDK） */
#include "osal.h"
#include <stdarg.h>
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>

/* ==================== 日志 ==================== */
static osal_log_level_t s_log_level = OSAL_LOG_DEBUG;
void osal_log_set_level(osal_log_level_t lvl) { s_log_level = lvl; }
void osal_log(osal_log_level_t lvl, const char* fmt, ...) {
    static const char* tag[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    if ((int)lvl > (int)s_log_level) return;
    char buf[128];
    rt_kprintf("[%s] ", tag[lvl]);
    va_list ap;
    va_start(ap, fmt);
    rt_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    rt_kprintf("%s", buf);
}

/* ==================== 线程 ==================== */
int osal_thread_create(osal_thread_t* t, const char* name, osal_thread_fn fn,
                       void* arg, uint32_t stack_size, uint32_t prio) {
    if (stack_size == 0) stack_size = 2048;
    rt_thread_t th = rt_thread_create(name ? name : "osal", (void (*)(void*))fn,
                                      arg, stack_size, prio ? prio : 15, 10);
    if (!th) return OSAL_ERR;
    rt_thread_startup(th);
    if (t) t->handle = th;
    return OSAL_OK;
}

/* ==================== 延时 / 时钟 ==================== */
void osal_delay_ms(uint32_t ms) { rt_thread_mdelay(ms); }
void osal_delay_us(uint32_t us) { rt_thread_mdelay(us / 1000 + 1); }
uint64_t osal_tick_ms(void) {
    return (uint64_t)rt_tick_get() * 1000 / RT_TICK_PER_SECOND;
}

/* ==================== 互斥锁 ==================== */
/** 句柄型；静态初值 NULL，首次 lock 惰性创建为普通互斥（推荐显式 init） */
int osal_mutex_init(osal_mutex_t* m, bool recursive) {
    rt_mutex_t h = rt_mutex_create(
        "osal", (recursive ? RT_IPC_FLAG_FIFO : RT_IPC_FLAG_FIFO));
    if (!h) return OSAL_ERR;
    *m = h;
    return OSAL_OK;
}

static void mutex_lazy(osal_mutex_t* m) {
    if (*m == NULL) {
        rt_mutex_t h = rt_mutex_create("osal", RT_IPC_FLAG_FIFO);
        if (h) *m = h;
    }
}

int osal_mutex_lock(osal_mutex_t* m) {
    mutex_lazy(m);
    return (rt_mutex_take((rt_mutex_t)*m, RT_WAITING_FOREVER) == RT_EOK)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_mutex_unlock(osal_mutex_t* m) {
    return (rt_mutex_release((rt_mutex_t)*m) == RT_EOK) ? OSAL_OK : OSAL_ERR;
}
int osal_mutex_destroy(osal_mutex_t* m) {
    return (rt_mutex_delete((rt_mutex_t)*m) == RT_EOK) ? OSAL_OK : OSAL_ERR;
}

/* ==================== 信号量 ==================== */
osal_sem_t* osal_sem_create(uint32_t init_count, uint32_t max_count) {
    osal_sem_t* o = (osal_sem_t*)malloc(sizeof(osal_sem_t));
    if (!o) return NULL;
    rt_sem_t h = rt_sem_create("osal", init_count, RT_IPC_FLAG_FIFO);
    if (!h) {
        free(o);
        return NULL;
    }
    o->handle = h;
    (void)max_count;
    return o;
}
int osal_sem_wait(osal_sem_t* s) {
    return (rt_sem_take((rt_sem_t)s->handle, RT_WAITING_FOREVER) == RT_EOK)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_sem_timedwait(osal_sem_t* s, uint32_t ms) {
    rt_tick_t to = rt_tick_from_millisecond(ms);
    return (rt_sem_take((rt_sem_t)s->handle, to) == RT_EOK) ? OSAL_OK
                                                            : OSAL_ERR;
}
int osal_sem_post(osal_sem_t* s) {
    return (rt_sem_release((rt_sem_t)s->handle) == RT_EOK) ? OSAL_OK : OSAL_ERR;
}
void osal_sem_delete(osal_sem_t* s) {
    if (!s) return;
    rt_sem_delete((rt_sem_t)s->handle);
    free(s);
}

/* ==================== 定时器（RT-Thread 软定时器） ==================== */
typedef struct osal_timer_wrap {
    osal_timer_cb_t cb;
    void* param;
    rt_timer_t h;
} osal_timer_wrap_t;

static void timer_rt_entry(void* p) {
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)p;
    if (w && w->cb) w->cb(w->param);
}

osal_timer_t* osal_timer_create(osal_timer_cb_t cb, void* param,
                                bool auto_reload) {
    osal_timer_t* t = (osal_timer_t*)malloc(sizeof(osal_timer_t));
    if (!t) return NULL;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)malloc(sizeof(*w));
    if (!w) {
        free(t);
        return NULL;
    }
    w->cb = cb;
    w->param = param;

    w->h = rt_timer_create(
        "osal", timer_rt_entry, w, 1, /* 周期由 start 设置 */
        auto_reload ? RT_TIMER_FLAG_PERIODIC : RT_TIMER_FLAG_ONE_SHOT);
    if (!w->h) {
        free(w);
        free(t);
        return NULL;
    }
    t->handle = w;
    return t;
}
int osal_timer_start(osal_timer_t* t, uint32_t period_ms) {
    if (!t) return OSAL_ERR;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    rt_tick_t tk = rt_tick_from_millisecond(period_ms);
    rt_timer_control(w->h, RT_TIMER_CTRL_SET_TIME, &tk);
    return (rt_timer_start(w->h) == RT_EOK) ? OSAL_OK : OSAL_ERR;
}
int osal_timer_stop(osal_timer_t* t) {
    return (rt_timer_stop(((osal_timer_wrap_t*)t->handle)->h) == RT_EOK)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_timer_reset(osal_timer_t* t) {
    rt_timer_stop(((osal_timer_wrap_t*)t->handle)->h);
    return (rt_timer_start(((osal_timer_wrap_t*)t->handle)->h) == RT_EOK)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_timer_change_period(osal_timer_t* t, uint32_t p) {
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    rt_timer_stop(w->h);
    rt_tick_t tk = rt_tick_from_millisecond(p);
    rt_timer_control(w->h, RT_TIMER_CTRL_SET_TIME, &tk);
    return (rt_timer_start(w->h) == RT_EOK) ? OSAL_OK : OSAL_ERR;
}
void osal_timer_delete(osal_timer_t* t) {
    if (!t) return;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    rt_timer_delete(w->h);
    free(w);
    free(t);
}

/* ==================== 消息队列 ==================== */
/** rt_mq_send_wait/rt_mq_recv 需要消息长度，故用 wrapper 保存 msg_size
 * 与本地计数。*/
typedef struct osal_mq_wrap {
    rt_mq_t h;
    uint32_t msg_size;
    uint32_t count;
} osal_mq_wrap_t;

static rt_int32_t osal_mq_ticks(uint32_t ms) {
    return (ms == OSAL_WAIT_FOREVER) ? RT_WAITING_FOREVER
                                     : rt_tick_from_millisecond(ms);
}
osal_mq_t* osal_mq_create(uint32_t msg_size, uint32_t max_msgs) {
    if (msg_size == 0 || max_msgs == 0) return NULL;
    osal_mq_t* mq = (osal_mq_t*)malloc(sizeof(osal_mq_t));
    if (!mq) return NULL;
    osal_mq_wrap_t* w = (osal_mq_wrap_t*)malloc(sizeof(*w));
    if (!w) {
        free(mq);
        return NULL;
    }
    w->msg_size = msg_size;
    w->count = 0;
    w->h = rt_mq_create("osal", msg_size, max_msgs, RT_IPC_FLAG_FIFO);
    if (!w->h) {
        free(w);
        free(mq);
        return NULL;
    }
    mq->handle = w;
    return mq;
}
int osal_mq_send(osal_mq_t* mq, const void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    osal_mq_wrap_t* w = (osal_mq_wrap_t*)mq->handle;
    rt_err_t r = rt_mq_send_wait(w->h, (const void*)msg, w->msg_size,
                                 osal_mq_ticks(timeout_ms));
    if (r == RT_EOK) {
        w->count++;
        return OSAL_OK;
    }
    return (timeout_ms == OSAL_WAIT_FOREVER) ? OSAL_ERR : OSAL_TIMEOUT;
}
int osal_mq_recv(osal_mq_t* mq, void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    osal_mq_wrap_t* w = (osal_mq_wrap_t*)mq->handle;
    rt_err_t r = rt_mq_recv(w->h, msg, w->msg_size, osal_mq_ticks(timeout_ms));
    if (r == RT_EOK) {
        w->count--;
        return OSAL_OK;
    }
    return (timeout_ms == OSAL_WAIT_FOREVER) ? OSAL_ERR : OSAL_TIMEOUT;
}
uint32_t osal_mq_count(osal_mq_t* mq) {
    if (!mq) return 0;
    return ((osal_mq_wrap_t*)mq->handle)->count;
}
void osal_mq_delete(osal_mq_t* mq) {
    if (!mq) return;
    osal_mq_wrap_t* w = (osal_mq_wrap_t*)mq->handle;
    rt_mq_delete(w->h);
    free(w);
    free(mq);
}

/* ==================== 初始化 ==================== */
int osal_init(void) {
    return OSAL_OK;
} /* RT-Thread 定时器由内核托管，无需守护线程 */

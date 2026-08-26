/** OSAL —— Linux/pthread 后端实现 */
#define _GNU_SOURCE
#include "osal.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/* ==================== 日志 ==================== */
static osal_log_level_t s_log_level = OSAL_LOG_DEBUG;

void osal_log_set_level(osal_log_level_t lvl) { s_log_level = lvl; }

void osal_log(osal_log_level_t lvl, const char* fmt, ...) {
    static const char* tag[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    if ((int)lvl > (int)s_log_level) return;
    fprintf(stderr, "[%s] ", tag[lvl]);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

/* ==================== 线程 ==================== */
int osal_thread_create(osal_thread_t* t, const char* name, osal_thread_fn fn,
                       void* arg, uint32_t stack_size, uint32_t prio) {
    (void)stack_size; /* pthread 由系统分配栈 */
    (void)prio;
    pthread_t tid;
    if (pthread_create(&tid, NULL, fn, arg) != 0) return OSAL_ERR;
    pthread_detach(tid);
    if (name) {
#ifdef __GLIBC__
        pthread_setname_np(tid, name);
#endif
    }
    if (t) t->handle = (void*)(uintptr_t)tid;
    return OSAL_OK;
}

/* ==================== 延时 / 时钟 ==================== */
void osal_delay_ms(uint32_t ms) {
    struct timespec ts = {ms / 1000, (long)(ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

void osal_delay_us(uint32_t us) {
    struct timespec ts = {us / 1000000, (long)(us % 1000000) * 1000};
    nanosleep(&ts, NULL);
}

uint64_t osal_tick_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

/* ==================== 互斥锁 ==================== */
int osal_mutex_init(osal_mutex_t* m, bool recursive) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (recursive) pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int r = pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);
    return (r == 0) ? OSAL_OK : OSAL_ERR;
}

int osal_mutex_lock(osal_mutex_t* m) {
    return (pthread_mutex_lock(m) == 0) ? OSAL_OK : OSAL_ERR;
}
int osal_mutex_unlock(osal_mutex_t* m) {
    return (pthread_mutex_unlock(m) == 0) ? OSAL_OK : OSAL_ERR;
}
int osal_mutex_destroy(osal_mutex_t* m) {
    return (pthread_mutex_destroy(m) == 0) ? OSAL_OK : OSAL_ERR;
}

/* ==================== 信号量 ==================== */
osal_sem_t* osal_sem_create(uint32_t init_count, uint32_t max_count) {
    (void)max_count;
    sem_t* s = (sem_t*)malloc(sizeof(sem_t));
    if (!s) return NULL;
    if (sem_init(s, 0, (unsigned)init_count) != 0) {
        free(s);
        return NULL;
    }

    osal_sem_t* o = (osal_sem_t*)malloc(sizeof(osal_sem_t));
    if (!o) {
        sem_destroy(s);
        free(s);
        return NULL;
    }
    o->handle = s;
    return o;
}

int osal_sem_wait(osal_sem_t* s) {
    if (!s) return OSAL_ERR;
    return (sem_wait((sem_t*)s->handle) == 0) ? OSAL_OK : OSAL_ERR;
}

int osal_sem_timedwait(osal_sem_t* s, uint32_t ms) {
    if (!s) return OSAL_ERR;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (long)(ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    return (sem_timedwait((sem_t*)s->handle, &ts) == 0) ? OSAL_OK : OSAL_ERR;
}

int osal_sem_post(osal_sem_t* s) {
    if (!s) return OSAL_ERR;
    return (sem_post((sem_t*)s->handle) == 0) ? OSAL_OK : OSAL_ERR;
}

void osal_sem_delete(osal_sem_t* s) {
    if (!s) return;
    sem_destroy((sem_t*)s->handle);
    free(s->handle);
    free(s);
}

/* ==================== 软件定时器（守护线程实现，移植自 mw_timer）
 * ==================== */
typedef struct osal_timer_node {
    osal_timer_cb_t cb;
    void* param;
    bool auto_reload;
    uint32_t period_ms;
    uint64_t expires_ms;
    bool active;
    bool running;
    bool rearm;
    bool deleted;
    struct osal_timer_node* next;
} osal_timer_node_t;

static pthread_t s_timer_tid;
static pthread_mutex_t s_timer_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_timer_cond;
static bool s_timer_inited = false;
static bool s_timer_running = false;
static osal_timer_node_t* s_timer_head = NULL;

static uint64_t timer_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void timer_list_insert(osal_timer_node_t* t) {
    osal_timer_node_t** pp = &s_timer_head;
    while (*pp && (*pp)->expires_ms <= t->expires_ms) pp = &(*pp)->next;
    t->next = *pp;
    *pp = t;
}

static void timer_list_remove(osal_timer_node_t* t) {
    osal_timer_node_t** pp = &s_timer_head;
    while (*pp) {
        if (*pp == t) {
            *pp = t->next;
            t->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

static void* timer_daemon(void* arg) {
    (void)arg;
    pthread_mutex_lock(&s_timer_mtx);
    while (s_timer_running) {
        uint64_t now = timer_now_ms();
        if (s_timer_head && s_timer_head->expires_ms <= now) {
            osal_timer_node_t* t = s_timer_head;
            s_timer_head = t->next;
            t->next = NULL;
            t->running = true;
            pthread_mutex_unlock(&s_timer_mtx);
            if (t->cb) t->cb(t->param);
            pthread_mutex_lock(&s_timer_mtx);
            t->running = false;
            if (t->deleted) {
                /* node 由 osal_timer_delete 的等待者释放；
                   唤醒正在等待回调结束的 delete 调用方 */
                pthread_cond_broadcast(&s_timer_cond);
            } else if (t->rearm) {
                t->rearm = false;
                timer_list_insert(t);
            } else if (t->active && t->auto_reload) {
                uint64_t nx = t->expires_ms + t->period_ms;
                if (nx <= now) nx = now + t->period_ms;
                t->expires_ms = nx;
                timer_list_insert(t);
            } else {
                t->active = false;
            }
            continue;
        }
        if (s_timer_head) {
            struct timespec ts;
            ts.tv_sec = s_timer_head->expires_ms / 1000;
            ts.tv_nsec = (long)(s_timer_head->expires_ms % 1000) * 1000000L;
            pthread_cond_timedwait(&s_timer_cond, &s_timer_mtx, &ts);
        } else {
            pthread_cond_wait(&s_timer_cond, &s_timer_mtx);
        }
    }
    pthread_mutex_unlock(&s_timer_mtx);
    return NULL;
}

osal_timer_t* osal_timer_create(osal_timer_cb_t cb, void* param,
                                bool auto_reload) {
    osal_timer_node_t* n = (osal_timer_node_t*)calloc(1, sizeof(*n));
    if (!n) return NULL;
    n->cb = cb;
    n->param = param;
    n->auto_reload = auto_reload;

    osal_timer_t* t = (osal_timer_t*)malloc(sizeof(osal_timer_t));
    if (!t) {
        free(n);
        return NULL;
    }
    t->handle = n;
    return t;
}

int osal_timer_start(osal_timer_t* t, uint32_t period_ms) {
    if (!t) return OSAL_ERR;
    osal_timer_node_t* n = (osal_timer_node_t*)t->handle;
    pthread_mutex_lock(&s_timer_mtx);
    n->period_ms = period_ms;
    n->expires_ms = timer_now_ms() + period_ms;
    n->active = true;
    n->rearm = false;
    timer_list_remove(n);
    timer_list_insert(n);
    pthread_cond_signal(&s_timer_cond);
    pthread_mutex_unlock(&s_timer_mtx);
    return OSAL_OK;
}

int osal_timer_stop(osal_timer_t* t) {
    if (!t) return OSAL_ERR;
    osal_timer_node_t* n = (osal_timer_node_t*)t->handle;
    pthread_mutex_lock(&s_timer_mtx);
    n->active = false;
    timer_list_remove(n);
    pthread_mutex_unlock(&s_timer_mtx);
    return OSAL_OK;
}

int osal_timer_reset(osal_timer_t* t) {
    if (!t) return OSAL_ERR;
    osal_timer_node_t* n = (osal_timer_node_t*)t->handle;
    pthread_mutex_lock(&s_timer_mtx);
    if (n->active) {
        n->expires_ms = timer_now_ms() + n->period_ms;
        timer_list_remove(n);
        timer_list_insert(n);
        pthread_cond_signal(&s_timer_cond);
    }
    pthread_mutex_unlock(&s_timer_mtx);
    return OSAL_OK;
}

int osal_timer_change_period(osal_timer_t* t, uint32_t period_ms) {
    if (!t) return OSAL_ERR;
    osal_timer_node_t* n = (osal_timer_node_t*)t->handle;
    pthread_mutex_lock(&s_timer_mtx);
    n->period_ms = period_ms;
    if (n->active) {
        n->expires_ms = timer_now_ms() + period_ms;
        timer_list_remove(n);
        timer_list_insert(n);
        pthread_cond_signal(&s_timer_cond);
    }
    pthread_mutex_unlock(&s_timer_mtx);
    return OSAL_OK;
}

void osal_timer_delete(osal_timer_t* t) {
    if (!t) return;
    osal_timer_node_t* n = (osal_timer_node_t*)t->handle;
    pthread_mutex_lock(&s_timer_mtx);
    n->active = false;
    n->rearm = false;
    n->deleted = true;
    /* 等待在途回调返回后再释放 node：保证 delete 返回后回调绝不再
       访问 param（C++ 封装 ~Timer 析构 cb_ 的安全前提）。
       回调契约要求快速返回 -> 等待时间上界 = 单次回调时长。
       注意：不得在回调内 delete 自身定时器（会死锁等待自己）。 */
    while (n->running) {
        pthread_cond_wait(&s_timer_cond, &s_timer_mtx);
    }
    timer_list_remove(n); /* 幂等：node 已被守护线程摘链时无操作 */
    pthread_mutex_unlock(&s_timer_mtx);
    free(n);
    free(t);
}

/* ==================== 消息队列（环形缓冲 + 双计数信号量，定长拷贝）
 * ==================== */
typedef struct osal_mq_impl {
    void* buf;  ///< max_msgs * msg_size 字节
    uint32_t msg_size;
    uint32_t max_msgs;
    uint32_t head;   ///< 取消息下标
    uint32_t tail;   ///< 存消息下标
    uint32_t count;  ///< 当前消息条数（head==tail 时区分空/满）
    osal_mutex_t lock;
    osal_sem_t* sem_free;   ///< 空闲槽计数（初值=max_msgs）
    osal_sem_t* sem_avail;  ///< 可用消息计数（初值=0）
} osal_mq_impl_t;

osal_mq_t* osal_mq_create(uint32_t msg_size, uint32_t max_msgs) {
    if (msg_size == 0 || max_msgs == 0) return NULL;

    osal_mq_impl_t* q = (osal_mq_impl_t*)calloc(1, sizeof(*q));
    if (!q) return NULL;
    q->msg_size = msg_size;
    q->max_msgs = max_msgs;
    q->buf = calloc(max_msgs, msg_size);
    if (!q->buf) {
        free(q);
        return NULL;
    }
    if (osal_mutex_init(&q->lock, false) != OSAL_OK) {
        free(q->buf);
        free(q);
        return NULL;
    }
    q->sem_free = osal_sem_create(max_msgs, max_msgs);
    if (!q->sem_free) {
        osal_mutex_destroy(&q->lock);
        free(q->buf);
        free(q);
        return NULL;
    }
    q->sem_avail = osal_sem_create(0, max_msgs);
    if (!q->sem_avail) {
        osal_sem_delete(q->sem_free);
        osal_mutex_destroy(&q->lock);
        free(q->buf);
        free(q);
        return NULL;
    }
    osal_mq_t* mq = (osal_mq_t*)malloc(sizeof(osal_mq_t));
    if (!mq) {
        osal_sem_delete(q->sem_free);
        osal_sem_delete(q->sem_avail);
        osal_mutex_destroy(&q->lock);
        free(q->buf);
        free(q);
        return NULL;
    }
    mq->handle = q;
    return mq;
}

int osal_mq_send(osal_mq_t* mq, const void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    osal_mq_impl_t* q = (osal_mq_impl_t*)mq->handle;
    /* 等空闲槽：sem_free 计数 = 空槽数。GUARD 不能跨越阻塞等待，故用显式
     * lock。*/
    int r = (timeout_ms == OSAL_WAIT_FOREVER)
                ? osal_sem_wait(q->sem_free)
                : osal_sem_timedwait(q->sem_free, timeout_ms);
    if (r != OSAL_OK) return OSAL_TIMEOUT; /* 无空闲槽 = 超时 */

    osal_mutex_lock(&q->lock);
    memcpy((char*)q->buf + (size_t)q->tail * q->msg_size, msg, q->msg_size);
    q->tail = (q->tail + 1) % q->max_msgs;
    q->count++;
    osal_mutex_unlock(&q->lock);

    osal_sem_post(q->sem_avail);
    return OSAL_OK;
}

int osal_mq_recv(osal_mq_t* mq, void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    osal_mq_impl_t* q = (osal_mq_impl_t*)mq->handle;
    int r = (timeout_ms == OSAL_WAIT_FOREVER)
                ? osal_sem_wait(q->sem_avail)
                : osal_sem_timedwait(q->sem_avail, timeout_ms);
    if (r != OSAL_OK) return OSAL_TIMEOUT; /* 无消息 = 超时 */

    osal_mutex_lock(&q->lock);
    memcpy(msg, (char*)q->buf + (size_t)q->head * q->msg_size, q->msg_size);
    q->head = (q->head + 1) % q->max_msgs;
    q->count--;
    osal_mutex_unlock(&q->lock);

    osal_sem_post(q->sem_free);
    return OSAL_OK;
}

uint32_t osal_mq_count(osal_mq_t* mq) {
    if (!mq) return 0;
    osal_mq_impl_t* q = (osal_mq_impl_t*)mq->handle;
    osal_mutex_lock(&q->lock);
    uint32_t n = q->count;
    osal_mutex_unlock(&q->lock);
    return n;
}

void osal_mq_delete(osal_mq_t* mq) {
    if (!mq) return;
    osal_mq_impl_t* q = (osal_mq_impl_t*)mq->handle;
    osal_sem_delete(q->sem_free);
    osal_sem_delete(q->sem_avail);
    osal_mutex_destroy(&q->lock);
    free(q->buf);
    free(q);
    free(mq);
}

/* ==================== 初始化 ==================== */
int osal_init(void) {
    if (s_timer_inited) return OSAL_OK;

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&s_timer_cond, &attr);
    pthread_condattr_destroy(&attr);

    s_timer_running = true;
    if (pthread_create(&s_timer_tid, NULL, timer_daemon, NULL) != 0) {
        s_timer_running = false;
        return OSAL_ERR;
    }
    pthread_detach(s_timer_tid);
#ifdef __GLIBC__
    pthread_setname_np(s_timer_tid, "osal_timer");
#endif
    s_timer_inited = true;
    return OSAL_OK;
}

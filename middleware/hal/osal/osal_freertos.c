/** OSAL —— FreeRTOS 后端实现（真实参考代码，需 FreeRTOS SDK） */
#include "osal.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"

/* ==================== 日志 ==================== */
static osal_log_level_t s_log_level = OSAL_LOG_DEBUG;
void osal_log_set_level(osal_log_level_t lvl) { s_log_level = lvl; }
void osal_log(osal_log_level_t lvl, const char* fmt, ...) {
    static const char* tag[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    if ((int)lvl > (int)s_log_level) return;
    printf("[%s] ", tag[lvl]);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
}

/* ==================== 线程 ==================== */
typedef struct osal_thread_wrap {
    TaskHandle_t task;
    void* arg;
    osal_thread_fn fn;
} osal_thread_wrap_t;
static void osal_task_entry(void* arg) {
    osal_thread_wrap_t* w = (osal_thread_wrap_t*)arg;
    if (w && w->fn) w->fn(w->arg);
    free(w);
    vTaskDelete(NULL);
}
int osal_thread_create(osal_thread_t* t, const char* name, osal_thread_fn fn,
                       void* arg, uint32_t stack_size, uint32_t prio) {
    if (stack_size == 0) stack_size = 2048;
    if (prio == 0) prio = tskIDLE_PRIORITY + 2;
    osal_thread_wrap_t* w = (osal_thread_wrap_t*)malloc(sizeof(*w));
    if (!w) return OSAL_ERR;
    w->fn = fn;
    w->arg = arg;
    BaseType_t r =
        xTaskCreate(osal_task_entry, name ? name : "osal",
                    stack_size / sizeof(StackType_t), w, prio, &w->task);
    if (r != pdPASS) {
        free(w);
        return OSAL_ERR;
    }
    if (t) t->handle = w->task;
    return OSAL_OK;
}

/* ==================== 延时 / 时钟 ==================== */
void osal_delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
void osal_delay_us(uint32_t us) { vTaskDelay(pdMS_TO_TICKS(us / 1000 + 1)); }
uint64_t osal_tick_ms(void) {
    return (uint64_t)xTaskGetTickCount() * portTICK_PERIOD_MS;
}

/* ==================== 互斥锁 ==================== */
struct fr_mtx {
    SemaphoreHandle_t h;
    bool recursive;
};
int osal_mutex_init(osal_mutex_t* m, bool recursive) {
    SemaphoreHandle_t h =
        recursive ? xSemaphoreCreateRecursiveMutex() : xSemaphoreCreateMutex();
    if (!h) return OSAL_ERR;
    struct fr_mtx* s = (struct fr_mtx*)malloc(sizeof(*s));
    if (!s) {
        vSemaphoreDelete(h);
        return OSAL_ERR;
    }
    s->h = h;
    s->recursive = recursive;
    *m = s;
    return OSAL_OK;
}
static void mutex_lazy(osal_mutex_t* m) {
    if (*m == NULL) (void)osal_mutex_init(m, false);
}
int osal_mutex_lock(osal_mutex_t* m) {
    mutex_lazy(m);
    struct fr_mtx* s = (struct fr_mtx*)*m;
    return (xSemaphoreTake(s->h, portMAX_DELAY) == pdTRUE) ? OSAL_OK : OSAL_ERR;
}
int osal_mutex_unlock(osal_mutex_t* m) {
    struct fr_mtx* s = (struct fr_mtx*)*m;
    if (s->recursive)
        xSemaphoreGiveRecursive(s->h);
    else
        xSemaphoreGive(s->h);
    return OSAL_OK;
}
int osal_mutex_destroy(osal_mutex_t* m) {
    struct fr_mtx* s = (struct fr_mtx*)*m;
    vSemaphoreDelete(s->h);
    free(s);
    *m = NULL;
    return OSAL_OK;
}

/* ==================== 信号量 ==================== */
osal_sem_t* osal_sem_create(uint32_t init_count, uint32_t max_count) {
    osal_sem_t* o = (osal_sem_t*)malloc(sizeof(osal_sem_t));
    if (!o) return NULL;
    SemaphoreHandle_t h =
        xSemaphoreCreateCounting(max_count ? max_count : 0xFFFF, init_count);
    if (!h) {
        free(o);
        return NULL;
    }
    o->handle = h;
    return o;
}
int osal_sem_wait(osal_sem_t* s) {
    return (xSemaphoreTake((SemaphoreHandle_t)s->handle, portMAX_DELAY) ==
            pdTRUE)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_sem_timedwait(osal_sem_t* s, uint32_t ms) {
    return (xSemaphoreTake((SemaphoreHandle_t)s->handle, pdMS_TO_TICKS(ms)) ==
            pdTRUE)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_sem_post(osal_sem_t* s) {
    xSemaphoreGive((SemaphoreHandle_t)s->handle);
    return OSAL_OK;
}
void osal_sem_delete(osal_sem_t* s) {
    if (!s) return;
    vSemaphoreDelete((SemaphoreHandle_t)s->handle);
    free(s);
}

/* ==================== 定时器（FreeRTOS 软定时器） ==================== */
static void osal_timer_entry(TimerHandle_t ht) {
    osal_timer_t* t = (osal_timer_t*)pvTimerGetTimerID(ht);
    if (!t) return;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    if (w && w->cb) w->cb(w->param);
}
typedef struct osal_timer_wrap {
    osal_timer_cb_t cb;
    void* param;
    TimerHandle_t tmr;
} osal_timer_wrap_t;

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
    t->handle = w;
    w->tmr =
        xTimerCreate("osal", pdMS_TO_TICKS(1), auto_reload ? pdTRUE : pdFALSE,
                     (void*)t, osal_timer_entry);
    if (!w->tmr) {
        free(w);
        free(t);
        return NULL;
    }
    return t;
}
int osal_timer_start(osal_timer_t* t, uint32_t period_ms) {
    if (!t) return OSAL_ERR;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    xTimerChangePeriod(w->tmr, pdMS_TO_TICKS(period_ms), 0);
    return (xTimerStart(w->tmr, 0) == pdPASS) ? OSAL_OK : OSAL_ERR;
}
int osal_timer_stop(osal_timer_t* t) {
    return (xTimerStop(((osal_timer_wrap_t*)t->handle)->tmr, 0) == pdPASS)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_timer_reset(osal_timer_t* t) {
    return (xTimerReset(((osal_timer_wrap_t*)t->handle)->tmr, 0) == pdPASS)
               ? OSAL_OK
               : OSAL_ERR;
}
int osal_timer_change_period(osal_timer_t* t, uint32_t p) {
    return (xTimerChangePeriod(((osal_timer_wrap_t*)t->handle)->tmr,
                               pdMS_TO_TICKS(p), 0) == pdPASS)
               ? OSAL_OK
               : OSAL_ERR;
}
void osal_timer_delete(osal_timer_t* t) {
    if (!t) return;
    osal_timer_wrap_t* w = (osal_timer_wrap_t*)t->handle;
    xTimerDelete(w->tmr, 0);
    free(w);
    free(t);
}

/* ==================== 消息队列 ==================== */
static TickType_t osal_mq_ticks(uint32_t ms) {
    return (ms == OSAL_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(ms);
}
osal_mq_t* osal_mq_create(uint32_t msg_size, uint32_t max_msgs) {
    if (msg_size == 0 || max_msgs == 0) return NULL;
    osal_mq_t* mq = (osal_mq_t*)malloc(sizeof(osal_mq_t));
    if (!mq) return NULL;
    QueueHandle_t h = xQueueCreate(max_msgs, msg_size);
    if (!h) {
        free(mq);
        return NULL;
    }
    mq->handle = h;
    return mq;
}
int osal_mq_send(osal_mq_t* mq, const void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    if (xQueueSend((QueueHandle_t)mq->handle, msg, osal_mq_ticks(timeout_ms)) ==
        pdPASS)
        return OSAL_OK;
    return (timeout_ms == OSAL_WAIT_FOREVER) ? OSAL_ERR : OSAL_TIMEOUT;
}
int osal_mq_recv(osal_mq_t* mq, void* msg, uint32_t timeout_ms) {
    if (!mq || !msg) return OSAL_ERR;
    if (xQueueReceive((QueueHandle_t)mq->handle, msg,
                      osal_mq_ticks(timeout_ms)) == pdPASS)
        return OSAL_OK;
    return (timeout_ms == OSAL_WAIT_FOREVER) ? OSAL_ERR : OSAL_TIMEOUT;
}
uint32_t osal_mq_count(osal_mq_t* mq) {
    if (!mq) return 0;
    return (uint32_t)uxQueueMessagesWaiting((QueueHandle_t)mq->handle);
}
void osal_mq_delete(osal_mq_t* mq) {
    if (!mq) return;
    vQueueDelete((QueueHandle_t)mq->handle);
    free(mq);
}

/* ==================== 初始化 ==================== */
int osal_init(void) {
    return OSAL_OK;
} /* FreeRTOS 定时器由定时器服务任务托管 */

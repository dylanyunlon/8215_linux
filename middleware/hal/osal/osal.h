#ifndef __OSAL_H__
#define __OSAL_H__

/**
 * OSAL —— 操作系统抽象层 (OS Abstraction Layer)
 *
 * 目标：为中间件提供一套与具体 OS 无关的原语（线程/延时/互斥/信号量/
 *      定时器/日志），使 HAL 层与应用模块只依赖 OSAL，不再直接依赖
 *      pthread / FreeRTOS / RT-Thread。
 *
 * 后端选择：构建时定义下列宏之一（由 Makefile 的 MW_OS 注入）
 *     OSAL_OS_LINUX       -> osal_linux.c      (pthread, 本工程默认)
 *     OSAL_OS_RTTTHREAD   -> osal_rtthread.c   (RT-Thread)
 *     OSAL_OS_FREERTOS    -> osal_freertos.c   (FreeRTOS)
 * 不定义任何宏时默认 Linux。
 *
 * 互斥锁：Linux 下 osal_mutex_t 直接别名 pthread_mutex_t，支持
 *         OSAL_MUTEX_INIT 静态初始化；RTOS 下为句柄，静态初值 NULL，
 *         首次 lock 时惰性创建（推荐显式 osal_mutex_init 初始化）。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- 后端选择 ---- */
#if defined(OSAL_OS_RTTTHREAD)
#define OSAL_BACKEND_RTTHREAD 1
#elif defined(OSAL_OS_FREERTOS)
#define OSAL_BACKEND_FREERTOS 1
#else
#define OSAL_BACKEND_LINUX 1
#if !defined(OSAL_OS_LINUX)
#define OSAL_OS_LINUX 1
#endif
#endif

#define OSAL_OK 0
#define OSAL_ERR (-1)

#define OSAL_TIMEOUT (-2)             /* 阻塞操作超时（消息队列 send/recv 等）*/
#define OSAL_WAIT_FOREVER 0xFFFFFFFFU /* 超时参数：永久等待；0 = 不等待 */

/* ==================== 日志 ==================== */
typedef enum {
    OSAL_LOG_ERROR = 0,
    OSAL_LOG_WARN,
    OSAL_LOG_INFO,
    OSAL_LOG_DEBUG,
} osal_log_level_t;

void osal_log_set_level(osal_log_level_t lvl);
void osal_log(osal_log_level_t lvl, const char* fmt, ...);

#define osal_log_error(...) osal_log(OSAL_LOG_ERROR, __VA_ARGS__)
#define osal_log_warn(...) osal_log(OSAL_LOG_WARN, __VA_ARGS__)
#define osal_log_info(...) osal_log(OSAL_LOG_INFO, __VA_ARGS__)
#define osal_log_debug(...) osal_log(OSAL_LOG_DEBUG, __VA_ARGS__)

/* ==================== 线程 ==================== */
typedef void* (*osal_thread_fn)(void* arg);

typedef struct osal_thread {
    void* handle;  ///< 后端线程句柄 (Linux: pthread_t 等)
} osal_thread_t;

/**
 * 创建并启动一个线程。
 * @param t      输出句柄，可为 NULL（表示不需要句柄）
 * @param name   线程名（调试用，Linux 限制 <=15 字节）
 * @param fn     线程入口
 * @param arg    入口参数
 * @param stack_size 栈大小（字节），0 表示使用后端默认值
 * @param prio   优先级（0=默认；RTOS 下为系统优先级）
 * @return OSAL_OK/OSAL_ERR
 */
int osal_thread_create(osal_thread_t* t, const char* name, osal_thread_fn fn,
                       void* arg, uint32_t stack_size, uint32_t prio);

/* ==================== 延时 / 时钟 ==================== */
void osal_delay_ms(uint32_t ms);
void osal_delay_us(uint32_t us);
uint64_t osal_tick_ms(void); /* 单调时钟，单位 ms */

/* ==================== 互斥锁 ==================== */
#if OSAL_BACKEND_LINUX
#include <pthread.h>
typedef pthread_mutex_t osal_mutex_t;
#define OSAL_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#else
typedef void* osal_mutex_t; /* 句柄，静态初值 NULL */
#define OSAL_MUTEX_INIT NULL
#endif

int osal_mutex_init(osal_mutex_t* m, bool recursive);
int osal_mutex_lock(osal_mutex_t* m);
int osal_mutex_unlock(osal_mutex_t* m);
int osal_mutex_destroy(osal_mutex_t* m);

/* ==================== 信号量（计数） ==================== */
typedef struct osal_sem {
    void* handle;
} osal_sem_t;

osal_sem_t* osal_sem_create(uint32_t init_count, uint32_t max_count);
int osal_sem_wait(osal_sem_t* s);
int osal_sem_timedwait(osal_sem_t* s, uint32_t ms); /* 超时返回 OSAL_ERR */
int osal_sem_post(osal_sem_t* s);
void osal_sem_delete(osal_sem_t* s);

/* ==================== 软件定时器 ==================== */
typedef void (*osal_timer_cb_t)(void* param);

typedef struct osal_timer {
    void* handle;
} osal_timer_t;

/**
 * 创建定时器（创建后未启动）。
 * @param cb          到期回调（在定时器任务上下文执行，须快速返回）
 * @param param       回调参数
 * @param auto_reload true=周期定时器, false=单次定时器
 */
osal_timer_t* osal_timer_create(osal_timer_cb_t cb, void* param,
                                bool auto_reload);
int osal_timer_start(osal_timer_t* t, uint32_t period_ms);
int osal_timer_stop(osal_timer_t* t);
int osal_timer_reset(osal_timer_t* t); /* 重新开始当前周期 */
int osal_timer_change_period(osal_timer_t* t, uint32_t period_ms);
/**
 * 销毁定时器。Linux 后端语义：等待在途回调返回后才释放并返回 --
 * 返回后回调保证绝不再触发（param 生命周期安全）。
 * 因此禁止在回调内 delete 自身定时器（死锁）；等待时长上界 = 单次回调。
 */
void osal_timer_delete(osal_timer_t* t);

/* ==================== 消息队列（定长、拷贝语义） ==================== */
/**
 * 定长消息队列：每条消息大小固定（创建时指定），send/recv 均按值拷贝。
 * 语义对齐 FreeRTOS xQueue / RT-Thread rt_mq；Linux 后端用环形缓冲 +
 * 双计数信号量实现。timeout_ms：OSAL_WAIT_FOREVER=永久阻塞, 0=不等待,
 * 其它=最长等待毫秒数。
 */
typedef struct osal_mq {
    void* handle;
} osal_mq_t;

/**
 * 创建消息队列。
 * @param msg_size  单条消息字节数（>0）
 * @param max_msgs  队列最大消息条数（>0）
 * @return 队列句柄，失败返回 NULL
 */
osal_mq_t* osal_mq_create(uint32_t msg_size, uint32_t max_msgs);

/** 投递一条消息到队尾（拷贝 msg_size 字节）。返回 OSAL_OK / OSAL_TIMEOUT /
 * OSAL_ERR */
int osal_mq_send(osal_mq_t* mq, const void* msg, uint32_t timeout_ms);

/** 从队头取出一条消息（拷贝 msg_size 字节到 msg）。返回 OSAL_OK / OSAL_TIMEOUT
 * / OSAL_ERR */
int osal_mq_recv(osal_mq_t* mq, void* msg, uint32_t timeout_ms);

/** 查询当前队列内消息条数（ informational，可能瞬时不精确）*/
uint32_t osal_mq_count(osal_mq_t* mq);

/** 销毁队列并释放资源（销毁后不得再使用该句柄）*/
void osal_mq_delete(osal_mq_t* mq);

/* ==================== 初始化 ==================== */
/**
 * OSAL 全局初始化（幂等）。mw_init() 调用一次。
 * Linux 后端会启动软件定时器守护线程。
 */
int osal_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __OSAL_H__ */

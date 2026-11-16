/*
 * rtthread.h — RT-Thread 内核 API 在 PC 上的最小替身
 *
 * 只声明 AiUIBuilder 生成代码 / 中间件头用到的类型与函数，
 * 实现在 port/mock_rtthread.c。目标：让设备端代码在 PC 上编译通过，
 * 不追求 RTOS 语义（线程用 Windows 线程/休眠模拟）。
 */
#ifndef RTTHREAD_H_SHIM
#define RTTHREAD_H_SHIM

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t   rt_int8_t;
typedef uint8_t  rt_uint8_t;
typedef int16_t  rt_int16_t;
typedef uint16_t rt_uint16_t;
typedef int32_t  rt_int32_t;
typedef uint32_t rt_uint32_t;
typedef int64_t  rt_int64_t;
typedef uint64_t rt_uint64_t;
typedef int32_t  rt_base_t;
typedef uint32_t rt_ubase_t;
typedef uint32_t rt_tick_t;
typedef uint32_t rt_size_t;
typedef int32_t  rt_err_t;
typedef int      rt_bool_t;

struct rt_thread;
typedef struct rt_thread *rt_thread_t;

/* 消息队列：设备端中间件（如 nfc_manage.h）把它当结构体成员按值使用，
 * 因此这里必须给出完整定义。字段与 RT-Thread 同名同义，PC 下不会真正被驱动。 */
struct rt_messagequeue
{
    void        *msg_pool;          /* start address of message queue */
    rt_uint16_t  msg_size;          /* message size of each message */
    rt_uint16_t  max_msgs;          /* max number of messages */
    rt_uint16_t  entry;             /* index of messages in the queue */
    rt_uint16_t  msg_queue_head;    /* list of message */
    rt_uint16_t  msg_queue_tail;    /* list of message */
    rt_uint16_t  msg_queue_free;    /* list of empty message */
    rt_uint8_t   parent_flag;       /* flag of whether there is a suspended thread */
};
typedef struct rt_messagequeue rt_mq_t;

/* RT-Thread 默认优先级数量（Cortex-M 平台为 32） */
#define RT_THREAD_PRIORITY_MAX  (32)

#define RT_NULL                 (0)
#define RT_EOK                  (0)
#define RT_ERROR                (1)
#define RT_ETIMEOUT             (2)
#define RT_WAITING_FOREVER      (-1)
#define RT_WAITING_NO           (0)
#define rt_tick_from_millisecond(ms) ((rt_tick_t)(ms))

rt_tick_t   rt_tick_get(void);
void        rt_kprintf(const char *fmt, ...);
rt_thread_t rt_thread_create(const char *name, void (*entry)(void *parameter), void *parameter,
                             rt_uint32_t stack_size, rt_uint8_t priority, rt_uint32_t tick);
rt_err_t    rt_thread_startup(rt_thread_t thread);
rt_err_t    rt_thread_delay_until(rt_tick_t *prev, rt_tick_t inc);
rt_err_t    rt_thread_mdelay(rt_int32_t ms);
rt_err_t    rt_thread_delay(rt_tick_t tick);

#endif /* RTTHREAD_H_SHIM */
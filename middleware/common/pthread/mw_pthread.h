#ifndef __MW_PTHREAD_H__
#define __MW_PTHREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "osal.h"

/**
 * @file mw_pthread.h
 * @brief 兼容封装：mw_pthread_create 委托 OSAL 线程原语
 *
 * 改造后不再直接依赖 pthread；底层由 osal_thread_create 实现于各 OS 后端。
 * tid 类型由 pthread_t* 改为 osal_thread_t*（Linux 下二者底层一致）。
 * 现有调用方均传 NULL（不取句柄），故签名变更向后兼容。
 */
#define create_pthread_info()             \
    do {                                  \
        osal_log_info("Create Thread\n"); \
    } while (0)

typedef void* (*mw_thread_fun)(void*);

/**
 * @brief  创建分离线程（创建即 detach，资源自动回收，不可 join）
 * @param  pthread_name 线程名字，不超过16个字符，包含\0
 * @param  tid  输出参数，返回创建的线程句柄；不需要可传 NULL
 * @param  fun  线程处理函数
 * @return 0：创建成功  -1:创建失败
 */
int mw_pthread_create(const char* pthread_name, osal_thread_t* tid,
                      mw_thread_fun fun);

#ifdef __cplusplus
}
#endif
#endif /* __MW_PTHREAD_H__ */

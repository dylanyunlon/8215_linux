#ifndef __MW_LOG_H__
#define __MW_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "osal.h"

/**
 * @file mw_log.h
 * @brief 日志宏 -- 路由到 OSAL 日志原语 osal_log
 *
 * 改造后输出统一经 osal_log（Linux->stderr, RT-Thread->rt_kprintf,
 * FreeRTOS->printf），级别过滤由 osal_log_set_level 控制（默认 DEBUG）。
 * 保留 [func:line] 前缀与原有 \n 约定（调用方在 fmt 末尾带 \n）。
 */

///< Assert enable
#define MW_USE_ASSERT 1
///< Assert with return enable
#define MW_USE_ASSERT_WITH_RETURN 1
///< Global log enable
#define G_LOG_EN 1

#if G_LOG_EN
#define mw_log_raw(...) osal_log(OSAL_LOG_DEBUG, __VA_ARGS__)
#define mw_log_error(fmt, ...)                                       \
    osal_log(OSAL_LOG_ERROR, "[%s:%d] " fmt, __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define mw_log_warn(fmt, ...)                                       \
    osal_log(OSAL_LOG_WARN, "[%s:%d] " fmt, __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define mw_log_info(fmt, ...)                                       \
    osal_log(OSAL_LOG_INFO, "[%s:%d] " fmt, __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#define mw_log_debug(fmt, ...)                                       \
    osal_log(OSAL_LOG_DEBUG, "[%s:%d] " fmt, __FUNCTION__, __LINE__, \
             ##__VA_ARGS__)
#else
#define mw_log_raw(...)
#define mw_log_error(...)
#define mw_log_warn(...)
#define mw_log_info(...)
#define mw_log_debug(...)
#endif

#if MW_USE_ASSERT
#define MW_ASSERT_PARAM(expr)                  \
    do {                                       \
        if (!(expr)) {                         \
            osal_log_error("assert failed\n"); \
            while (1);                         \
        }                                      \
    } while (0)
#else
#define MW_ASSERT_PARAM(expr)
#endif

#if MW_USE_ASSERT_WITH_RETURN
#define MW_ASSERT_WITH_VALUE(expr, value) \
    {                                     \
        if (!(expr)) return value;        \
    }
#else
#define MW_ASSERT_WITH_VALUE(expr, value)
#endif

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // __MW_LOG_H__

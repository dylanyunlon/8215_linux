#ifndef __MW_LOCK_H__
#define __MW_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "osal.h"

/**
 * @brief  RAII 式自动互斥锁（header-only，基于 GCC __attribute__((cleanup))）
 *
 * 改造后作用于 osal_mutex_t（Linux 下别名 pthread_mutex_t，行为不变；
 * RTOS 下为句柄型，首次 lock 惰性创建）。出作用域（含 return/break/goto）
 * 自动解锁，绝不遗漏；lock 失败时 guard 变量为 NULL，cleanup 跳过 unlock。
 *
 *   static osal_mutex_t s_lock = OSAL_MUTEX_INIT;    // Linux 静态初始化
 *   void foo(void) {
 *       MW_MUTEX_GUARD(&s_lock);
 *       g_data = 1;
 *       if (err) return;            // return 也会自动解锁
 *   }
 *
 * 注意：依赖 GCC cleanup 扩展；多个 GUARD 同作用域按声明逆序解锁；
 *       非递归锁（同线程二次加锁死锁），递归需求请用 osal_mutex_init(true)。
 */

static inline void _mw_mutex_cleanup(osal_mutex_t** pm) {
    if (pm != NULL && *pm != NULL) {
        osal_mutex_unlock(*pm);
    }
}

#define MW_MUTEX_GUARD(m)                       \
    __attribute__((cleanup(_mw_mutex_cleanup))) \
    osal_mutex_t* _mw_g_##__COUNTER__ =         \
        (osal_mutex_lock(m) == OSAL_OK ? (m) : NULL)

#ifdef __cplusplus
}
#endif

#endif /* __MW_LOCK_H__ */

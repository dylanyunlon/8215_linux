#include "mw_timer.h"
#include "osal.h"

int mw_timer_init(void)
{
    return (osal_init() == OSAL_OK) ? 0 : -1;
}

void mw_timer_deinit(void)
{
    /* OSAL 不提供独立 deinit（中间件生命周期内常驻）；保留空实现以兼容旧调用 */
}

mw_timer_t *mw_timer_create(mw_timer_cb_t cb, void *param, bool auto_reload)
{
    if (cb == NULL)
        return NULL;
    return osal_timer_create(cb, param, auto_reload);
}

int mw_timer_start(mw_timer_t *t, uint32_t period_ms)
{
    if (t == NULL || period_ms == 0)
        return -1;
    return (osal_timer_start(t, period_ms) == OSAL_OK) ? 0 : -1;
}

int mw_timer_stop(mw_timer_t *t)
{
    if (t == NULL)
        return -1;
    return (osal_timer_stop(t) == OSAL_OK) ? 0 : -1;
}

int mw_timer_reset(mw_timer_t *t)
{
    if (t == NULL)
        return -1;
    return (osal_timer_reset(t) == OSAL_OK) ? 0 : -1;
}

int mw_timer_change_period(mw_timer_t *t, uint32_t period_ms)
{
    if (t == NULL || period_ms == 0)
        return -1;
    return (osal_timer_change_period(t, period_ms) == OSAL_OK) ? 0 : -1;
}

void mw_timer_delete(mw_timer_t *t)
{
    osal_timer_delete(t);
}

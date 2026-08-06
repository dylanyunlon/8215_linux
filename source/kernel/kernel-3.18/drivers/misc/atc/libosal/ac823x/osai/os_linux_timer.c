/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/



#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include "x_assert.h"
#include "x_os.h"

#ifdef KERNEL_STANDARD_API
#include <linux/types.h>



typedef struct os_timer_light
{
    struct timer_list timer;
    x_os_timer_cb_fct pf_callback;
    void *pv_tag;
    bool fg_active;
    unsigned long interval;
} OS_TIMER_LIGHT_T;


static const u32 s_timer_res = (u32)((u32)1000 / (u32)HZ);


static void TimerProc(unsigned long id)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if (id == NULL) {
		return;
	}
	
    pt_timer = (OS_TIMER_LIGHT_T *)(id);
    pt_timer->pf_callback((u32)(pt_timer), pt_timer->pv_tag);

    if (pt_timer->interval != 0)
    {
        pt_timer->timer.expires += pt_timer->interval;
        add_timer(&pt_timer->timer);
    }
    else
    {
        pt_timer->fg_active = false;
    }
}


s32 x_timer_create (u32 *ph_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

    if (ph_timer == NULL)
    {
        return OSR_INV_ARG;
    }

    pt_timer = (OS_TIMER_LIGHT_T *)kcalloc((size_t)1, sizeof(OS_TIMER_LIGHT_T), GFP_KERNEL);
    if (pt_timer == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_timer);

    init_timer(&(pt_timer->timer));

    *ph_timer = (u32)(pt_timer);
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_create);


s32 x_timer_start (u32           h_timer,
                     u32             ui4_delay,
                     TIMER_FLAG_T       e_flags,
                     x_os_timer_cb_fct  pf_callback,
                     void*              pv_tag)
{
    OS_TIMER_LIGHT_T *pt_timer;
    unsigned long j;

    if ((pf_callback == NULL) || (ui4_delay == 0) || (h_timer == NULL) ||
        ((e_flags != X_TIMER_FLAG_ONCE) && (e_flags != X_TIMER_FLAG_REPEAT)))
    {
        return OSR_INV_ARG;
    }

    j = jiffies;
    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    pt_timer->timer.expires = j + ui4_delay / s_timer_res;
    pt_timer->timer.function = &TimerProc;
    pt_timer->timer.data = (unsigned long)pt_timer;
    if (e_flags == X_TIMER_FLAG_ONCE)
    {
        pt_timer->interval = 0;
    }
    else
    {
        pt_timer->interval = ui4_delay / s_timer_res;
    }

    pt_timer->pf_callback = pf_callback;
    pt_timer->pv_tag = pv_tag;
    pt_timer->fg_active = true;
    add_timer(&pt_timer->timer);

    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_start);


s32 x_timer_stop (u32  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if (h_timer == NULL) {
		return OSR_INV_ARG;
	}
    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (pt_timer->fg_active)
    {
        //del_timer(&pt_timer->timer);
		del_timer_sync(&pt_timer->timer);
        pt_timer->fg_active = false;
        pt_timer->timer.expires -= jiffies;
    }
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_stop);


s32 x_timer_delete (u32  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;
	
	if (h_timer == NULL) {
		return OSR_INV_ARG;
	}

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (pt_timer->fg_active)
    {
        //del_timer(&pt_timer->timer);
		del_timer_sync(&pt_timer->timer);
    }
    kfree(pt_timer);
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_delete);


s32 x_timer_resume (u32  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if (h_timer == NULL) {
		return OSR_INV_ARG;
	}

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (!pt_timer->fg_active)
    {
        pt_timer->timer.expires += jiffies;
        pt_timer->fg_active = true;
        add_timer(&pt_timer->timer);
    }

    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_resume);


u32 x_os_get_sys_tick (void)
{
    return jiffies;
}

EXPORT_SYMBOL(x_os_get_sys_tick);


u32 x_os_get_sys_tick_period (void)
{
    return s_timer_res;
}

EXPORT_SYMBOL(x_os_get_sys_tick_period);


s32 os_timer_init (void)
{
    return OSR_OK;
}

#else //old api

typedef struct os_timer_light
{
    struct timer_list timer;
    x_os_timer_cb_fct pf_callback;
    void *pv_tag;
    bool fg_active;
    unsigned long interval;
} OS_TIMER_LIGHT_T;


static const __u32 s_timer_res = 1000 / HZ;


static void TimerProc(unsigned long id)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if ((void *)id == NULL) {
		return;
	}
	
    pt_timer = (OS_TIMER_LIGHT_T *)(id);
    pt_timer->pf_callback((uintptr_t)(pt_timer), pt_timer->pv_tag);

    if (pt_timer->interval != 0)
    {
        pt_timer->timer.expires += pt_timer->interval;
        add_timer(&pt_timer->timer);
    }
    else
    {
        pt_timer->fg_active = FALSE;
    }
}


__s32 x_timer_create (uintptr_t *ph_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

    if (ph_timer == NULL)
    {
        return OSR_INV_ARG;
    }

    pt_timer = kcalloc(1, sizeof(OS_TIMER_LIGHT_T), GFP_KERNEL);
    if (pt_timer == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_timer);

    init_timer(&pt_timer->timer);

    *ph_timer = (uintptr_t)(pt_timer);
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_create);


__s32 x_timer_start (uintptr_t           h_timer,
                     u32             ui4_delay,
                     TIMER_FLAG_T       e_flags,
                     x_os_timer_cb_fct  pf_callback,
                     void*              pv_tag)
{
    OS_TIMER_LIGHT_T *pt_timer;
    unsigned long j;

    if ((pf_callback == NULL) || (ui4_delay == 0) || ((void *)h_timer == NULL) ||
        ((e_flags != X_TIMER_FLAG_ONCE) && (e_flags != X_TIMER_FLAG_REPEAT)))
    {
        return OSR_INV_ARG;
    }

    j = jiffies;
    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    pt_timer->timer.expires = j + ui4_delay / s_timer_res;
    pt_timer->timer.function = &TimerProc;
    pt_timer->timer.data = (unsigned long)pt_timer;
    if (e_flags == X_TIMER_FLAG_ONCE)
    {
        pt_timer->interval = 0;
    }
    else
    {
        pt_timer->interval = ui4_delay / s_timer_res;
    }

    pt_timer->pf_callback = pf_callback;
    pt_timer->pv_tag = pv_tag;
    pt_timer->fg_active = TRUE;
    add_timer(&pt_timer->timer);

    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_start);


__s32 x_timer_stop (uintptr_t  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if ((void *)h_timer == NULL) {
		return OSR_INV_ARG;
	}
    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (pt_timer->fg_active)
    {
        //del_timer(&pt_timer->timer);
		del_timer_sync(&pt_timer->timer);
        pt_timer->fg_active = FALSE;
        pt_timer->timer.expires -= jiffies;
    }
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_stop);


__s32 x_timer_delete (uintptr_t  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;
	
	if ((void *)h_timer == NULL) {
		return OSR_INV_ARG;
	}

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (pt_timer->fg_active)
    {
        //del_timer(&pt_timer->timer);
		del_timer_sync(&pt_timer->timer);
    }
    kfree(pt_timer);
    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_delete);


__s32 x_timer_resume (uintptr_t  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

	if ((void *)h_timer == NULL) {
		return OSR_INV_ARG;
	}

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    if (!pt_timer->fg_active)
    {
        pt_timer->timer.expires += jiffies;
        pt_timer->fg_active = TRUE;
        add_timer(&pt_timer->timer);
    }

    return OSR_OK;
}

EXPORT_SYMBOL(x_timer_resume);


__u32 x_os_get_sys_tick (void)
{
    return jiffies;
}

EXPORT_SYMBOL(x_os_get_sys_tick);


__u32 x_os_get_sys_tick_period (void)
{
    return s_timer_res;
}

EXPORT_SYMBOL(x_os_get_sys_tick_period);


__s32 os_timer_init (void)
{
    return OSR_OK;
}

#endif



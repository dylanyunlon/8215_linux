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



#include <asm/current.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#include "x_assert.h"
#include "x_os.h"

#include <linux/types.h>


#define INVALID_TASK ((struct task_struct *)(NULL))


typedef struct os_sema_light
{
    SEMA_TYPE_T e_type;
    struct semaphore sem;
    struct task_struct *task;
    s16 i2_selfcount;
} OS_SEMA_LIGHT_T;


__s32 x_sema_create (uintptr_t    *ph_sema_hdl,
                            SEMA_TYPE_T  e_types,
                            __u32       ui4_init_value)
{
    OS_SEMA_LIGHT_T *pt_sema;

    /* check arguments */
    if ((ph_sema_hdl == NULL) ||
        ((e_types != X_SEMA_TYPE_BINARY) && (e_types != X_SEMA_TYPE_MUTEX) &&
         (e_types != X_SEMA_TYPE_COUNTING)))
    {
        return OSR_INV_ARG;
    }

    if (((e_types == X_SEMA_TYPE_BINARY) || (e_types == X_SEMA_TYPE_MUTEX)) &&
         (ui4_init_value != X_SEMA_STATE_LOCK) &&
         (ui4_init_value != X_SEMA_STATE_UNLOCK))
    {
        return OSR_INV_ARG;
    }

    if ((e_types == X_SEMA_TYPE_COUNTING) &&
        (((__s32) ui4_init_value) < ((__s32) X_SEMA_STATE_LOCK)))
    {
        return OSR_INV_ARG;
    }

    pt_sema = (OS_SEMA_LIGHT_T *)kcalloc((size_t)1, sizeof(OS_SEMA_LIGHT_T), GFP_KERNEL);
    if (pt_sema == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_sema);

    pt_sema->e_type = e_types;
    sema_init(&(pt_sema->sem), (int)ui4_init_value);

    if (e_types == X_SEMA_TYPE_MUTEX)
    {
        if (ui4_init_value == X_SEMA_STATE_LOCK)
        {
            pt_sema->task = current;
            pt_sema->i2_selfcount++;
        }
        else
        {
            pt_sema->task = INVALID_TASK;
            pt_sema->i2_selfcount = 0;
        }
    }

    *ph_sema_hdl = (uintptr_t)(pt_sema);
    return OSR_OK;
}

EXPORT_SYMBOL(x_sema_create);


__s32 x_sema_delete (uintptr_t  h_sema_hdl)
{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);

    kfree(pt_sema);
    return OSR_OK;
}

EXPORT_SYMBOL(x_sema_delete);


__s32 x_sema_lock (uintptr_t       h_sema_hdl,
					SEMA_OPTION_T e_options)

{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);

    if ((e_options != X_SEMA_OPTION_WAIT) && (e_options != X_SEMA_OPTION_NOWAIT))
    {
        return OSR_INV_ARG;
    }

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (pt_sema->task != current)
        {
            if (e_options == X_SEMA_OPTION_NOWAIT)
            {
                if (down_trylock(&pt_sema->sem) != 0)
                {
                    return OSR_WOULD_BLOCK;
                }
            }
            else
            {
                down(&pt_sema->sem);
            }
            pt_sema->task = current;
        }
        pt_sema->i2_selfcount++;
        return OSR_OK;
    }
    else
    {
        if (e_options == X_SEMA_OPTION_NOWAIT)
        {
            if (down_trylock(&pt_sema->sem) != 0)
            {
                return OSR_WOULD_BLOCK;
            }
        }
        else
        {
            down(&pt_sema->sem);
        }

        return OSR_OK;
    }
}

EXPORT_SYMBOL(x_sema_lock);


__s32 x_sema_lock_timeout (uintptr_t  h_sema_hdl,
                                  __u32    ui4_time)

{
    static const __s32 quantum_ms = (__s32)((__s32)1000 / (__s32)HZ);
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);
    __s32 ret;

    if (ui4_time == 0)
    {
        __s32 i4 = x_sema_lock(h_sema_hdl, X_SEMA_OPTION_NOWAIT);
        return i4 != OSR_WOULD_BLOCK ? i4 : OSR_TIMEOUT;
    }

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (pt_sema->task != current)
        {
            ret = down_timeout(&(pt_sema->sem), (long)((__u32)ui4_time / (__u32)quantum_ms));
            if (ret != 0)
            {
                goto err;
            }
            pt_sema->task = current;
        }
        pt_sema->i2_selfcount++;
        return OSR_OK;
    }
    else
    {
        ret = down_timeout(&(pt_sema->sem), (long)((__u32)ui4_time / (__u32)quantum_ms));
        if (ret != 0)
        {
            goto err;
        }

        return OSR_OK;
    }

err:
    switch (ret)
    {
    case -ETIME:
        return OSR_TIMEOUT;

    default:
        return OSR_FAIL;
    }
}

EXPORT_SYMBOL(x_sema_lock_timeout);


__s32 x_sema_unlock (uintptr_t  h_sema_hdl)

{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (pt_sema->task != current)
        {
            return OSR_FAIL;
        }
        pt_sema->i2_selfcount--;
        if (pt_sema->i2_selfcount != 0)
        {
            return OSR_OK;
        }
        pt_sema->task = INVALID_TASK;

        up(&pt_sema->sem);
        return OSR_OK;
    }
    else
    {
        up(&pt_sema->sem);
        return OSR_OK;
    }
}

EXPORT_SYMBOL(x_sema_unlock);


__s32 os_sema_init (void)
{
    return OSR_OK;
}


__s32 os_cli_show_sema_all (__s32        i4_argc,
                            const char   **pps_argv)
{
    return OSR_OK;
}

EXPORT_SYMBOL(os_cli_show_sema_all);



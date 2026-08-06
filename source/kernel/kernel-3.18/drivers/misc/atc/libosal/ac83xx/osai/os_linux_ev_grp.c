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


#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/wait.h>
#include "x_assert.h"
#include "x_os.h"
#include <linux/spinlock_types.h>


static DEFINE_SPINLOCK(ac83xx_ev_grp_lock);

#define EV_GRP_NAME_LEN 16

typedef struct os_ev_grp_light
{
    struct os_ev_grp_light *previous;
    struct os_ev_grp_light *next;
    wait_queue_head_t wq;
    EV_GRP_EVENT_T e_events;
    __s16 i2_refcount;
    char s_name[EV_GRP_NAME_LEN + 1];
} OS_EV_GRP_LIGHT_T;


static OS_EV_GRP_LIGHT_T *s_ev_grp_list;


static void ev_grp_list_add(OS_EV_GRP_LIGHT_T *pt_ev_grp)
{
    if (s_ev_grp_list != NULL)
    {
        pt_ev_grp->previous = s_ev_grp_list->previous;
        pt_ev_grp->next = s_ev_grp_list;
        s_ev_grp_list->previous->next = pt_ev_grp;
        s_ev_grp_list->previous = pt_ev_grp;
    }
    else
    {
        s_ev_grp_list = pt_ev_grp->next = pt_ev_grp->previous = pt_ev_grp;
    }
}


static void ev_grp_list_remove(OS_EV_GRP_LIGHT_T *pt_ev_grp)
{
    if (pt_ev_grp->previous == pt_ev_grp)
    {
        s_ev_grp_list = NULL;
    }
    else
    {
        pt_ev_grp->previous->next = pt_ev_grp->next;
        pt_ev_grp->next->previous = pt_ev_grp->previous;
        if (s_ev_grp_list == pt_ev_grp)
        {
            s_ev_grp_list = pt_ev_grp->next;
        }
    }
}


static OS_EV_GRP_LIGHT_T *ev_grp_find_obj(const char *ps_name)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp = s_ev_grp_list;
    if (pt_ev_grp == NULL)
    {
        return NULL;
    }
    do
    {
        if (strncmp(pt_ev_grp->s_name, ps_name, EV_GRP_NAME_LEN) == 0)
        {
            return pt_ev_grp;
        }
        pt_ev_grp = pt_ev_grp->next;
    } while (pt_ev_grp != s_ev_grp_list);

    return NULL;
}


__s32
x_ev_group_create(uintptr_t          *ph_hdl,
                  const char         *ps_name,
                  EV_GRP_EVENT_T     e_init_events)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    unsigned long flags;

    /* check arguments */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_hdl == NULL))
    {
        return OSR_INV_ARG;
    }

    pt_ev_grp = kcalloc(1, sizeof(OS_EV_GRP_LIGHT_T), GFP_KERNEL);
    if (pt_ev_grp == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_ev_grp);

    pt_ev_grp->e_events = e_init_events;
    pt_ev_grp->i2_refcount = 1;
    strncpy(pt_ev_grp->s_name, ps_name, EV_GRP_NAME_LEN);
    init_waitqueue_head(&pt_ev_grp->wq);

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
    if (ev_grp_find_obj(ps_name) != NULL)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
        kfree(pt_ev_grp);
        return OSR_EXIST;
    }

    ev_grp_list_add(pt_ev_grp);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);

    *ph_hdl = (u32)(pt_ev_grp);
    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_create);


__s32
x_ev_group_attach(uintptr_t     *ph_hdl,
                  const char   *ps_name)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    unsigned long flags;

    /* arguments check */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_hdl == NULL))
    {
        return OSR_INV_ARG;
    }

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
    pt_ev_grp = ev_grp_find_obj(ps_name);
    if (pt_ev_grp == NULL)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
        return OSR_NOT_EXIST;
    }

    pt_ev_grp->i2_refcount++;
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);

    *ph_hdl = (u32)(pt_ev_grp);
    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_attach);


s32
x_ev_group_delete(uintptr_t    h_hdl)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    __s16 i2_refcount;
    unsigned long flags;
	if((void *)h_hdl == NULL)return OSR_INV_ARG;

    pt_ev_grp = (OS_EV_GRP_LIGHT_T *)(h_hdl);

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
    i2_refcount = --pt_ev_grp->i2_refcount;
    if (i2_refcount > 0)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
        return OSR_OK;
    }

    ev_grp_list_remove(pt_ev_grp);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);

    kfree(pt_ev_grp);
    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_delete);


s32
x_ev_group_set_event(uintptr_t           h_hdl,
                     EV_GRP_EVENT_T     e_events,
                     EV_GRP_OPERATION_T e_op)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    unsigned long flags;

    if (e_op == X_EV_OP_CLEAR)
    {
        e_events = (EV_GRP_EVENT_T)(0);
        e_op = X_EV_OP_AND;
    }
    else if (e_op != X_EV_OP_AND && e_op != X_EV_OP_OR)
    {
        return OSR_INV_ARG;
    }

	if((void *)h_hdl == NULL)return OSR_INV_ARG;

    pt_ev_grp = (OS_EV_GRP_LIGHT_T *)(h_hdl);

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
    if (e_op == X_EV_OP_AND)
    {
        pt_ev_grp->e_events &= e_events;
    }
    else
    {
        pt_ev_grp->e_events |= e_events;
    }
    wake_up_all(&pt_ev_grp->wq);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);

    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_set_event);


static __s32
x_ev_group_wait_event_impl(uintptr_t              h_hdl,
                           EV_GRP_EVENT_T        e_events_req,
                           EV_GRP_EVENT_T        *pe_events_got,
                           EV_GRP_OPERATION_T    e_op,
                           bool                  wait)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    EV_GRP_EVENT_T e_events;
    unsigned long flags;

    if (pe_events_got == NULL ||
        (e_op != X_EV_OP_AND && e_op != X_EV_OP_OR && e_op != X_EV_OP_AND_CONSUME && e_op != X_EV_OP_OR_CONSUME))
    {
        return OSR_INV_ARG;
    }
	if((void *)h_hdl == NULL)return OSR_INV_ARG;

    pt_ev_grp = (OS_EV_GRP_LIGHT_T *)(h_hdl);
    *pe_events_got = 0;

    if (wait)
    {
        while (TRUE)
        {
            if (e_op == X_EV_OP_AND || e_op == X_EV_OP_AND_CONSUME)
            {
                wait_event(pt_ev_grp->wq, (pt_ev_grp->e_events & e_events_req) == e_events_req);
            }
            else
            {
                wait_event(pt_ev_grp->wq, (pt_ev_grp->e_events & e_events_req) != (EV_GRP_EVENT_T)(0));
            }
            //local_irq_save(flags);
            spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
            e_events = pt_ev_grp->e_events & e_events_req;
            if (e_op == X_EV_OP_AND || e_op == X_EV_OP_AND_CONSUME)
            {
                if (e_events == e_events_req)
                {
                    if (e_op == X_EV_OP_AND_CONSUME)
                        pt_ev_grp->e_events &= ~e_events_req;
                    //local_irq_restore(flags);
                    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                    break;
                }
            }
            else
            {
                if (e_events != (EV_GRP_EVENT_T)(0))
                {
                    if (e_op == X_EV_OP_OR_CONSUME)
                    {
                        u32 u4Idx = 0;
                        for (u4Idx = 0; u4Idx < 32; u4Idx++)
                        {
                            if (0 != ((1 << u4Idx) & e_events))
                            {
                                break;
                            }
                        }
                        if (u4Idx < 32)
                        {
                            e_events = (EV_GRP_EVENT_T)(1 << u4Idx);
                            pt_ev_grp->e_events &= ~e_events;
                        }
                        else
                        {
                            pt_ev_grp->e_events &= ~e_events_req;
                        }
                    }
                    //local_irq_restore(flags);
                    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                    break;
                }
            }
            //local_irq_restore(flags);
            spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
        }
    }
    else
    {
        //local_irq_save(flags);
        spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
        if (e_op == X_EV_OP_AND || e_op == X_EV_OP_AND_CONSUME)
        {
            if (!((e_events = pt_ev_grp->e_events & e_events_req) == e_events_req))
            {
                //local_irq_restore(flags);
                spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                return OSR_TIMEOUT;
            }
        }
        else
        {
            if (!((e_events = pt_ev_grp->e_events & e_events_req) != (EV_GRP_EVENT_T)(0)))
            {
                //local_irq_restore(flags);
                spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                return OSR_TIMEOUT;
            }
        }
        if (e_op == X_EV_OP_AND_CONSUME)
        {
            pt_ev_grp->e_events &= ~e_events_req;
        }
        else if (e_op == X_EV_OP_OR_CONSUME)
        {
            __u32 u4Idx = 0;
            for (u4Idx = 0; u4Idx < 32; u4Idx++)
            {
                if (0 != ((1 << u4Idx) & e_events))
                {
                    break;
                }
            }
            if (u4Idx < 32)
            {
                e_events = (EV_GRP_EVENT_T)(1 << u4Idx);
                pt_ev_grp->e_events &= ~e_events;
            }
            else
            {
                pt_ev_grp->e_events &= ~e_events_req;
            }
        }
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
    }

    *pe_events_got = e_events;
    return OSR_OK;
}


__s32
x_ev_group_wait_event(uintptr_t              h_hdl,
                      EV_GRP_EVENT_T        e_events_req,
                      EV_GRP_EVENT_T        *pe_events_got,
                      EV_GRP_OPERATION_T    e_op)
{
    return x_ev_group_wait_event_impl(h_hdl, e_events_req, pe_events_got, e_op, TRUE);
}

EXPORT_SYMBOL(x_ev_group_wait_event);


__s32
x_ev_group_wait_event_timeout(uintptr_t              h_hdl,
                              EV_GRP_EVENT_T        e_events_req,
                              EV_GRP_EVENT_T        *pe_events_got,
                              EV_GRP_OPERATION_T    e_op,
                              u32                ui4_time)
{
    static const int quantum_ms = 1000 / HZ;
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    EV_GRP_EVENT_T e_events;
    unsigned long flags;
    int ret;

    if (ui4_time == 0)
    {
        return x_ev_group_wait_event_impl(h_hdl, e_events_req, pe_events_got, e_op, FALSE);
    }
    if (ui4_time == 0xFFFFFFFFU)
    {
        return x_ev_group_wait_event_impl(h_hdl, e_events_req, pe_events_got, e_op, TRUE);
    }
    if (pe_events_got == NULL ||
        (e_op != X_EV_OP_AND && e_op != X_EV_OP_OR && e_op != X_EV_OP_AND_CONSUME && e_op != X_EV_OP_OR_CONSUME))
    {
        return OSR_INV_ARG;
    }
	if((void *)h_hdl == NULL)return OSR_INV_ARG;


    pt_ev_grp = (OS_EV_GRP_LIGHT_T *)(h_hdl);
    *pe_events_got = 0;

    while (TRUE)
    {
        if (e_op == X_EV_OP_AND || e_op == X_EV_OP_AND_CONSUME)
        {
            ret = wait_event_timeout(pt_ev_grp->wq, (pt_ev_grp->e_events & e_events_req) == e_events_req, ui4_time / quantum_ms);
        }
        else
        {
            ret = wait_event_timeout(pt_ev_grp->wq, (pt_ev_grp->e_events & e_events_req) != (EV_GRP_EVENT_T)(0), ui4_time / quantum_ms);
        }
        if (ret == 0)
        {
            return OSR_TIMEOUT;
        }
        //local_irq_save(flags);
        spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
        e_events = pt_ev_grp->e_events & e_events_req;
        if (e_op == X_EV_OP_AND || e_op == X_EV_OP_AND_CONSUME)
        {
            if (e_events == e_events_req)
            {
                if (e_op == X_EV_OP_AND_CONSUME)
                {
                    pt_ev_grp->e_events &= ~e_events_req;
                }
                //local_irq_restore(flags);
                spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                break;
            }
        }
        else
        {
            if (e_events != (EV_GRP_EVENT_T)(0))
            {
                if (e_op == X_EV_OP_OR_CONSUME)
                {
                    __u32 u4Idx = 0;
                    for (u4Idx = 0; u4Idx < 32; u4Idx++)
                    {
                        if (0 != ((1 << u4Idx) & e_events))
                        {
                            break;
                        }
                    }
                    if (u4Idx < 32)
                    {
                        e_events = (EV_GRP_EVENT_T)(1 << u4Idx);
                        pt_ev_grp->e_events &= ~e_events;
                    }
                    else
                    {
                        pt_ev_grp->e_events &= ~e_events_req;
                    }
                }
                //local_irq_restore(flags);
                spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
                break;
            }
        }
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);
    }

    *pe_events_got = e_events;
    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_wait_event_timeout);


__s32
x_ev_group_get_info(uintptr_t          h_hdl,
                    EV_GRP_EVENT_T    *pe_cur_events,
                    u8             *pui1_num_thread_waiting,
                    char              *ps_ev_group_name,
                    char              *ps_first_wait_thread)
{
    OS_EV_GRP_LIGHT_T *pt_ev_grp;
    unsigned long flags;

    if ((pe_cur_events == NULL) ||
        (pui1_num_thread_waiting == NULL) ||
        (ps_ev_group_name == NULL) ||
        (ps_first_wait_thread == NULL))
    {
        return OSR_INV_ARG;
    }
	if((void *)h_hdl == NULL)return OSR_INV_ARG;

    pt_ev_grp = (OS_EV_GRP_LIGHT_T *)(h_hdl);
    *pe_cur_events = 0;

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_ev_grp_lock, flags);
    *pe_cur_events = pt_ev_grp->e_events;
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_ev_grp_lock, flags);

    // TODO: fill pui1_num_thread_waiting, ps_ev_group_name, ps_first_wait_thread

    return OSR_OK;
}

EXPORT_SYMBOL(x_ev_group_get_info);


__s32
ev_grp_init(void)
{
    return OSR_OK;
}




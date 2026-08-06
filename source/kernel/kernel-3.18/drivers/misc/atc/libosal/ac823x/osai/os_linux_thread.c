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



#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "x_assert.h"
#include "x_os.h"
#include <linux/spinlock_types.h>


#include <linux/types.h>


static DEFINE_SPINLOCK(ac83xx_thread_lock);

//---------------------------------------------------------------------


#define MEMORY_ALIGNMENT        8

#define THREAD_NAME_LEN         16
#define THREAD_PRI_RANGE_LOW    (u8) 1
#define THREAD_PRI_RANGE_HIGH   (u8) 254
#define DEFAULT_TASK_SLICE      5


typedef struct os_thread_pvt_light
{
    struct os_thread_pvt_light *previous;
    struct os_thread_pvt_light *next;

    u32  ui4_key;
    x_os_thread_pvt_del_fct  pf_pvt_del;
    void *pv_pvt;
} OS_THREAD_PVT_LIGHT_T;


typedef struct os_thread_light
{
    struct os_thread_light *previous;
    struct os_thread_light *next;
    struct task_struct *task;
    //void *pv_stack;
    //u32 z_stack_size;
    char s_name[THREAD_NAME_LEN + 1];
    u8 ui1_priority;
    x_os_thread_main_fct pf_main_rtn;
    OS_THREAD_PVT_LIGHT_T *pt_pvt;
    u32 z_arg_size;
    u8 au1_arg_local[1];
} OS_THREAD_LIGHT_T;


//---------------------------------------------------------------------


static __u8 from_sched_priority(__s32 sched_priority)
{
    return (__u8)((100 - sched_priority) * 256 / 100);
}


static __s32 to_sched_priority(__u8 ui1_priority)
{
    __s32 sched_priority;
    sched_priority = (__s32)100 - (__s32)ui1_priority * (__s32)100 / (__s32)256;
    if (sched_priority < 1) sched_priority = 1;
    if (sched_priority > 99) sched_priority = 99;
    return sched_priority;
}


//---------------------------------------------------------------------


static OS_THREAD_LIGHT_T *s_thread_list;


static void thread_list_add(OS_THREAD_LIGHT_T *pt_thread)
{
    if (s_thread_list != NULL)
    {
    	if ((pt_thread->s_name[THREAD_NAME_LEN] == (char)0) || (pt_thread->s_name[THREAD_NAME_LEN-1] == (char)0)) {
    		printk("[OSAL] ++ thread_list_add, add thread [%s], addr [0x%x] \n", pt_thread->s_name, pt_thread);
    	} else {
			printk("[OSAL] ++ thread_list_add, name not right \n");
    	}
		pt_thread->previous = s_thread_list->previous;
        pt_thread->next = s_thread_list;
        s_thread_list->previous->next = pt_thread;
        s_thread_list->previous = pt_thread;
    }
    else
    {
    	//printk("++ thread_list_add, create pt_thread [0x%x] \n", pt_thread);
        s_thread_list = pt_thread->next = pt_thread->previous = pt_thread;
    }
}


static void thread_list_remove(OS_THREAD_LIGHT_T *pt_thread)
{
    if (pt_thread->previous == pt_thread)
    {
    	printk("[OSAL] ++ thread_list_remove, No thread to be remove \n");
        s_thread_list = NULL;
    }
    else
    {
        if ((pt_thread->s_name[THREAD_NAME_LEN] == (char)0) || (pt_thread->s_name[THREAD_NAME_LEN-1] == (char)0)) {
    		printk("[OSAL] ++ thread_list_remove, remove thread [%s], addr [0x%x] \n", pt_thread->s_name, pt_thread);
    	} else {
			printk("[OSAL] ++ thread_list_remove, name not right \n");
    	}

        pt_thread->previous->next = pt_thread->next;
        pt_thread->next->previous = pt_thread->previous;
        if (s_thread_list == pt_thread)
        {
        	//printk("++ thread_list_remove, s_thread_list = pt_thread \n");
            s_thread_list = pt_thread->next;
        }
    }
}


static OS_THREAD_LIGHT_T *thread_find_handle(uintptr_t h_th_hdl)
{
    OS_THREAD_LIGHT_T *pt_thread = s_thread_list;
    if (pt_thread == NULL)
    {
        return NULL;
    }
    do
    {
        if (pt_thread == (OS_THREAD_LIGHT_T *)(h_th_hdl))
        {
            return pt_thread;
        }
        pt_thread = pt_thread->next;
    } while (pt_thread != s_thread_list);

    return NULL;
}


static OS_THREAD_LIGHT_T *thread_find_obj(const char *ps_name)
{
    OS_THREAD_LIGHT_T *pt_thread = s_thread_list;
    if (pt_thread == NULL)
    {
    	printk("[OSAL] ++ thread_find_obj, s_thread_list is NULL \n");
        return NULL;
    }
    do
    {
        if (strncmp(pt_thread->s_name, ps_name, (size_t)THREAD_NAME_LEN) == 0)
        {
        	if ((pt_thread->s_name[THREAD_NAME_LEN] == (char)0) || (pt_thread->s_name[THREAD_NAME_LEN-1] == (char)0)) {
    			printk("[OSAL] ++ thread_find_obj, s_name [%s]\n", pt_thread->s_name);
    		} else {
				printk("[OSAL] ++ thread_find_obj, name not right \n");
    		}
            return pt_thread;
        }
        pt_thread = pt_thread->next;
    } while (pt_thread != s_thread_list);

    return NULL;
}


static void ThreadExit(void)
{
    unsigned long flags;
    OS_THREAD_LIGHT_T *pt_thread;
    struct task_struct *task;

    ASSERT(x_thread_self((uintptr_t *)&pt_thread) == OSR_OK);

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    thread_list_remove(pt_thread);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    task = pt_thread->task;

    //kfree(pt_thread->pv_stack);
    kfree(pt_thread);
    pt_thread = NULL;
    complete_and_exit((struct completion *)NULL, (long)0);
}


static __s32 ThreadProc(void *arg)
{
    OS_THREAD_LIGHT_T *pt_thread =  (OS_THREAD_LIGHT_T *)arg;
    ASSERT(pt_thread != NULL);

    // Invoke the original thread function
    pt_thread->pf_main_rtn(pt_thread->z_arg_size != 0 ? pt_thread->au1_arg_local : NULL);

    // Terminate thread
    ThreadExit();

    return 0;
}

__s32 x_thread_create (uintptr_t            *ph_th_hdl,
                              const char*           ps_name,
                              size_t                z_stack_size,
                              __u8                 ui1_priority,
                              x_os_thread_main_fct  pf_main_rtn,
                              size_t                z_arg_size,
                              void*                 pv_arg)

{
    OS_THREAD_LIGHT_T *pt_thread;
    //void *pv_stack;
    unsigned long flags;

    if (pv_arg == NULL)
    {
        z_arg_size = 0;
    }

    /* check arguments */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_th_hdl == NULL) ||
        //(z_stack_size == 0) ||
        (pf_main_rtn == NULL) ||
        (ui1_priority < THREAD_PRI_RANGE_LOW) || (ui1_priority > THREAD_PRI_RANGE_HIGH) ||
        ((pv_arg != NULL) && (z_arg_size == 0)) ||
        ((pv_arg == NULL) && (z_arg_size != 0)))
    {
        return OSR_INV_ARG;
    }

    // Make sure the stack size is aligned
    //z_stack_size = (z_stack_size + MEMORY_ALIGNMENT - 1) & (~(MEMORY_ALIGNMENT - 1));

    pt_thread = kcalloc((size_t)1, (sizeof(OS_THREAD_LIGHT_T) - sizeof(__u8) + (size_t)z_arg_size), GFP_KERNEL);
    //pv_stack = kcalloc(1, z_stack_size);
    if (pt_thread == NULL /*|| pv_stack == NULL*/)
    {
        //kfree(pv_stack);
        kfree(pt_thread);
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_thread);
    //FILL_CALLER(pv_stack);

    //pt_thread->pv_stack = pv_stack;
    strncpy(pt_thread->s_name, ps_name, (size_t)THREAD_NAME_LEN);
    pt_thread->pf_main_rtn = pf_main_rtn;
    pt_thread->z_arg_size = z_arg_size;
    if (z_arg_size != 0)
    {
        memcpy(pt_thread->au1_arg_local, pv_arg, z_arg_size);
    }

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    if (thread_find_obj(ps_name) != NULL)
    {
        //local_irq_restore(flags);
        if ((ps_name[THREAD_NAME_LEN] == (char)0) || (ps_name[THREAD_NAME_LEN-1] == (char)0)) {
			printk("[OSAL] ++ x_thread_create, name [%s] \n", ps_name);
        } else {
			printk("[OSAL] ++ x_thread_create, name not right \n");
        }
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        //kfree(pv_stack);
        kfree(pt_thread);
        return OSR_EXIST;
    }

    thread_list_add(pt_thread);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    pt_thread->task = kthread_create(&ThreadProc, pt_thread, ps_name);
    if (pt_thread->task == ERR_PTR(-ENOMEM))
    {
        //local_irq_save(flags);
        spin_lock_irqsave(&ac83xx_thread_lock, flags);
        thread_list_remove(pt_thread);
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        //kfree(pv_stack);
        kfree(pt_thread);
        return OSR_NO_RESOURCE;
    }
    else if (IS_ERR(pt_thread->task))
    {
        //local_irq_save(flags);
        spin_lock_irqsave(&ac83xx_thread_lock, flags);
        thread_list_remove(pt_thread);
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        //kfree(pv_stack);
        kfree(pt_thread);
        return OSR_FAIL;
    }
    else
    {
        struct sched_param param;
        __s32 ret;

        param.sched_priority = to_sched_priority(ui1_priority);
        ret = (__s32)sched_setscheduler_nocheck(pt_thread->task, (int)SCHED_RR, &param);
        ASSERT(ret == 0);
        pt_thread->ui1_priority = from_sched_priority(param.sched_priority);
    }

    wake_up_process(pt_thread->task);

    *ph_th_hdl = (uintptr_t)(pt_thread);
    return OSR_OK;
}

EXPORT_SYMBOL(x_thread_create);


void x_thread_exit (void)
{
    ThreadExit();
}

EXPORT_SYMBOL(x_thread_exit);


void x_thread_delay (__u32 ui4_delay)
{
	if (in_interrupt() != 0) 
	{
		mdelay((unsigned long)ui4_delay);
		return ;
	}

    ASSERT(in_interrupt() == 0);

    if (ui4_delay == 0)
    {
        yield();
    }
    else
    {
        msleep((unsigned int)ui4_delay);
    }
}

EXPORT_SYMBOL(x_thread_delay);


__s32 x_thread_set_pri (uintptr_t  h_th_hdl,
                        __u8     ui1_new_pri)
{
    OS_THREAD_LIGHT_T *pt_thread;
    unsigned long flags;

    if ((ui1_new_pri < THREAD_PRI_RANGE_LOW) || (ui1_new_pri > THREAD_PRI_RANGE_HIGH))
    {
        return OSR_INV_ARG;
    }

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        return OSR_INV_HANDLE;
    }
    {
        struct sched_param param;
        __s32 ret;

        param.sched_priority = to_sched_priority(ui1_new_pri);
        ret = sched_setscheduler_nocheck(pt_thread->task, SCHED_RR, &param);
        ASSERT(ret == 0);
        pt_thread->ui1_priority = from_sched_priority(param.sched_priority);
    }
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    return OSR_OK;
}

EXPORT_SYMBOL(x_thread_set_pri);


__s32 x_thread_get_pri (uintptr_t  h_th_hdl,
                        __u8*    pui1_pri)
{
    OS_THREAD_LIGHT_T *pt_thread;
    unsigned long flags;

    if (pui1_pri == NULL)
    {
        return OSR_INV_ARG;
    }

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        return OSR_INV_HANDLE;
    }
    *pui1_pri = pt_thread->ui1_priority;
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    return OSR_OK;
}

EXPORT_SYMBOL(x_thread_get_pri);


__s32 x_thread_get_name (uintptr_t  h_th_hdl,
                        __u32* s_name)
{
    OS_THREAD_LIGHT_T *pt_thread;
    unsigned long flags;

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        //local_irq_restore(flags);
        spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        return OSR_INV_HANDLE;
    }
    *s_name =(__u32) &pt_thread->s_name[0];
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    return OSR_OK;
}

EXPORT_SYMBOL(x_thread_get_name);


void x_thread_suspend (void)
{
    ASSERT(0);
}

EXPORT_SYMBOL(x_thread_suspend);


__s32 x_thread_resume (uintptr_t  h_th_hdl)
{
    return OSR_NOT_SUPPORT;
}

EXPORT_SYMBOL(x_thread_resume);


__s32 x_thread_self (uintptr_t *ph_th_hdl)
{
    OS_THREAD_LIGHT_T *pt_thread;
    unsigned long flags;

    if (ph_th_hdl == NULL)
    {
        return OSR_INV_ARG;
    }

    //local_irq_save(flags);
    spin_lock_irqsave(&ac83xx_thread_lock, flags);
    pt_thread = s_thread_list;
    if (pt_thread == NULL)
    {
    	spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
        return OSR_NOT_EXIST;
    }
    do
    {
        if (pt_thread->task == current)
        {
            //local_irq_restore(flags);
            spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
            *ph_th_hdl = (uintptr_t)(pt_thread);
            return OSR_OK;
        }
        pt_thread = pt_thread->next;
    } while (pt_thread != s_thread_list);
    //local_irq_restore(flags);
    spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

    return OSR_NOT_EXIST;
}

EXPORT_SYMBOL(x_thread_self);


__s32 x_thread_stack_stats (uintptr_t  h_th_hdl,
						   size_t*	 pz_alloc_stack,
						   size_t*	 pz_max_used_stack)
{
    return OSR_NOT_SUPPORT;
}

EXPORT_SYMBOL(x_thread_stack_stats);


__s32 x_thread_set_pvt (__u32                   ui4_key,
                        x_os_thread_pvt_del_fct  pf_pvt_del,
                        void*                    pv_pvt)
{
    return OSR_NOT_SUPPORT;
}

EXPORT_SYMBOL(x_thread_set_pvt);


__s32 x_thread_get_pvt (__u32  ui4_key,
                        void**  ppv_pvt)
{
    return OSR_NOT_SUPPORT;
}

EXPORT_SYMBOL(x_thread_get_pvt);


__s32 x_thread_del_pvt (__u32  ui4_key)
{
    return OSR_NOT_SUPPORT;
}

EXPORT_SYMBOL(x_thread_del_pvt);


__s32 os_thread_init(void)
{
    return OSR_OK;
}


__s32
os_cli_show_thread_all(__s32        i4_argc,
                       const char   **pps_argv)
{
    return OSR_OK;
}

EXPORT_SYMBOL(os_cli_show_thread_all);

uintptr_t  x_thread_find_obj(const char *ps_name)
{
	OS_THREAD_LIGHT_T *pt_thread;
	unsigned long flags;
	
	if (ps_name == NULL)
	{
	    return (uintptr_t)OSR_INV_ARG;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_thread_lock, flags);
	pt_thread = thread_find_obj(ps_name);
	if (pt_thread == NULL)
	{
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_thread_lock, flags);
		return (uintptr_t)OSR_INV_HANDLE;
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_thread_lock, flags);

	return (uintptr_t)pt_thread;
}

EXPORT_SYMBOL(x_thread_find_obj);


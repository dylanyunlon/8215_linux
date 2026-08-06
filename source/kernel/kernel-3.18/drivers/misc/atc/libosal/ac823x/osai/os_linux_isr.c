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



#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/string.h>
#include "x_os.h"
#include <linux/spinlock_types.h>

#ifdef KERNEL_STANDARD_API
#include <linux/types.h>


static DEFINE_SPINLOCK(ac83xx_isr_lock);

#define MAX_VECTOR_ID       256                  // Max vector ID


// ISR control block of OS driver
typedef struct
{
    x_os_isr_fct        pf_isr;
    char                devname[8];
} OS_DRV_ISR_T;

static OS_DRV_ISR_T     s_isr_list[MAX_VECTOR_ID];


irqreturn_t IsrProc(s32 irq, void *dev_id)
{
    if (s_isr_list[irq].pf_isr != NULL)
    {
        s_isr_list[irq].pf_isr((__u16)irq);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}


s32
x_reg_isr_ex(__u16         ui2_vec_id,
             x_os_isr_fct   pf_isr,
             x_os_isr_fct   *ppf_old_isr,
             ISR_FLAG_T     e_flags)
{
    x_os_isr_fct pf_old_isr;
    s32 ret;
	unsigned long flags;

    pf_old_isr = s_isr_list[ui2_vec_id].pf_isr;
    if (pf_old_isr != NULL)
    {
        free_irq((unsigned int)ui2_vec_id, NULL);
    }
	spin_lock_irqsave(&ac83xx_isr_lock,flags);
    s_isr_list[ui2_vec_id].pf_isr = pf_isr;
    if (pf_isr != NULL)
    {
        ret = request_irq((unsigned int)ui2_vec_id, IsrProc, e_flags, s_isr_list[ui2_vec_id].devname, NULL);
        if (ret != 0)
        {
            return OSR_FAIL;
        }
    }
    *ppf_old_isr = pf_old_isr;
	spin_unlock_irqrestore(&ac83xx_isr_lock,flags);
    return OSR_OK;
}

EXPORT_SYMBOL(x_reg_isr_ex);


__s32
x_reg_isr(__u16         ui2_vec_id,
          x_os_isr_fct   pf_isr,
          x_os_isr_fct   *ppf_old_isr)
{
    return x_reg_isr_ex(ui2_vec_id, pf_isr, ppf_old_isr, 0);
}

EXPORT_SYMBOL(x_reg_isr);


s32
isr_init(void)
{
    s32 i;

    memset((void *)s_isr_list, 0, sizeof(s_isr_list));
    for (i = 0; i < MAX_VECTOR_ID; i++)
    {
        snprintf((char *)(s_isr_list[i].devname), (size_t)8, "ISR_%02d", i);
    }

    return OSR_OK;
}
EXPORT_SYMBOL(isr_init);

#else  //old api

static DEFINE_SPINLOCK(ac83xx_isr_lock);

#define MAX_VECTOR_ID       256                  // Max vector ID


// ISR control block of OS driver
typedef struct
{
    x_os_isr_fct        pf_isr;
    char                devname[8];
} OS_DRV_ISR_T;

static OS_DRV_ISR_T     s_isr_list[MAX_VECTOR_ID];


irqreturn_t IsrProc(int irq, void *dev_id)
{
    if (s_isr_list[irq].pf_isr != NULL)
    {
        s_isr_list[irq].pf_isr((__u16)irq);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}


__s32
x_reg_isr_ex(__u16         ui2_vec_id,
             x_os_isr_fct   pf_isr,
             x_os_isr_fct   *ppf_old_isr,
             ISR_FLAG_T     e_flags)
{
    x_os_isr_fct pf_old_isr;
    int ret;
	unsigned long flags;

    pf_old_isr = s_isr_list[ui2_vec_id].pf_isr;
    if (pf_old_isr != NULL)
    {
        free_irq(ui2_vec_id, NULL);
    }
	spin_lock_irqsave(&ac83xx_isr_lock,flags);
    s_isr_list[ui2_vec_id].pf_isr = pf_isr;
    if (pf_isr != NULL)
    {
        ret = request_irq(ui2_vec_id, IsrProc, e_flags, s_isr_list[ui2_vec_id].devname, NULL);
        if (ret != 0)
        {
            return OSR_FAIL;
        }
    }
    *ppf_old_isr = pf_old_isr;
	spin_unlock_irqrestore(&ac83xx_isr_lock,flags);
    return OSR_OK;
}

EXPORT_SYMBOL(x_reg_isr_ex);


__s32
x_reg_isr(__u16         ui2_vec_id,
          x_os_isr_fct   pf_isr,
          x_os_isr_fct   *ppf_old_isr)
{
    return x_reg_isr_ex(ui2_vec_id, pf_isr, ppf_old_isr, 0);
}

EXPORT_SYMBOL(x_reg_isr);


__s32
isr_init(VOID)
{
    int i;

    memset(s_isr_list, 0, sizeof(s_isr_list));
    for (i = 0; i < MAX_VECTOR_ID; i++)
    {
        snprintf(s_isr_list[i].devname, 8, "ISR_%02d", i);
    }

    return OSR_OK;
}
EXPORT_SYMBOL(isr_init);

#endif


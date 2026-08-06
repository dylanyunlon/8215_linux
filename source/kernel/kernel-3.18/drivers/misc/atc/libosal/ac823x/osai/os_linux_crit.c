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


#include <linux/irqflags.h>
#include <linux/module.h>
#include "x_os.h"
#include <linux/spinlock_types.h>

#include <linux/types.h>


#if 1
u32 x_crit_start(void)
{
    unsigned long flags;
    local_irq_save(flags);
    return (u32)(flags);
}


void x_crit_end(u32 t_old_level)
{
    unsigned long flags = (unsigned long)(t_old_level);
    local_irq_restore(flags);
}
#else
void x_crit_start(spinlock_t *lock, unsigned long *flags)
{
	spin_lock_irqsave(lock, (*flags));
}


void x_crit_end(spinlock_t *lock, unsigned long *flags)
{
	spin_unlock_irqrestore(lock, (*flags));
}
#endif


EXPORT_SYMBOL(x_crit_start);
EXPORT_SYMBOL(x_crit_end);



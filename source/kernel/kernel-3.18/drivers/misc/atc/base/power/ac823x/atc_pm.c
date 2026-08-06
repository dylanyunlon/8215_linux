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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#include <asm/suspend.h>


static int ac823x_pm_valid(suspend_state_t state)
{
	int ret;
	ret = ((state == PM_SUSPEND_STANDBY) || (state == PM_SUSPEND_MEM));
	printk(KERN_INFO "PM: %s, %s (%u) %s\n", __func__,
			(!state)   ? "PM_SUSPEND_ON":
			(1==state) ? "PM_SUSPEND_STANDBY":
			(3==state) ? "PM_SUSPEND_MEM":"UNKNOW",
			state, ret ? "support" : "unsupport");

	return ret;
}

static int ac823x_pm_begin(suspend_state_t state)
{
	printk(KERN_INFO "PM: %s\n", __func__);
	
	return 0;
}

static int ac823x_pm_prepare( void)
{
	printk(KERN_INFO "PM: %s\n", __func__);

	return 0;
}

static int ac823x_pm_prepare_late(void)
{
	printk(KERN_INFO "PM: %s\n", __func__);

	return 0;
}

static int ac823x_pm_enter(suspend_state_t state)
{
	printk(KERN_INFO "PM: %s, %s (%u)\n", __func__,
			(!state)   ?"PM_SUSPEND_ON":
			(1==state) ?"PM_SUSPEND_STANDBY":
			(3==state) ?"PM_SUSPEND_MEM":"UNKNOW", state);

	switch(state)
	{
		case PM_SUSPEND_MEM:
			cpu_suspend(2);
			printk("ac823x_pm_enter exit\n");
			break;
		case PM_SUSPEND_STANDBY:
			break;
		case PM_SUSPEND_ON:
			break;
		default:
			break;
	}
	return 0;
}

static void ac823x_pm_wake(void)
{
	printk(KERN_INFO "PM: %s\n", __func__);
}

static void ac823x_pm_finish(void)
{
	printk(KERN_INFO "PM: %s\n", __func__);
}

static void ac823x_pm_end(void)
{
	printk(KERN_INFO "PM: %s\n", __func__);
}

static void ac823x_pm_recover(void)
{
	printk(KERN_INFO "PM: %s\n", __func__);
}

// begin->*prepare->*prepare_late->*enter->*wake->*finish->end
static struct platform_suspend_ops ac823x_pm_ops =
{
	.valid        = ac823x_pm_valid,
	.begin        = ac823x_pm_begin,
	.prepare      = ac823x_pm_prepare,
	.prepare_late = ac823x_pm_prepare_late,
	.enter        = ac823x_pm_enter,
	.wake         = ac823x_pm_wake,
	.finish       = ac823x_pm_finish,
	.end          = ac823x_pm_end,
	.recover      = ac823x_pm_recover,
};

static int __init ac823x_pm_init(void)
{	
	printk(KERN_INFO "PM: %s\n", __func__);
	suspend_set_ops(&ac823x_pm_ops);
	
	return 0;
}

late_initcall(ac823x_pm_init);
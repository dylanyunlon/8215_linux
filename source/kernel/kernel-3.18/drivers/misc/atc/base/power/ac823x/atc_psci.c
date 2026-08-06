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
#include <linux/smp.h>
#include <linux/delay.h>

#include <asm/cpu_ops.h>
#include <asm/psci.h>
#include "atc_cpu_psci_ops.h"

#ifdef CONFIG_SMP

/* Debugging */
#undef TAG
#define TAG	 "[psci] "

#define psci_err(fmt, args...)	   \
	pr_err(TAG"[ERROR]"fmt, ##args)
#define psci_warn(fmt, args...)	  \
	pr_warn(TAG"[WARNING]"fmt, ##args)
#define psci_info(fmt, args...)	  \
	pr_warn(TAG""fmt, ##args)
#define psci_dbg(fmt, args...)	   \
	pr_debug(TAG""fmt, ##args)


static int __init atc_psci_cpu_init(struct device_node *dn, unsigned int cpu)
{
	return 0;
}

static int __init atc_psci_cpu_prepare(unsigned int cpu)
{
	return cpu_psci_ops.cpu_prepare(cpu);
}

static int atc_psci_cpu_boot(unsigned int cpu)
{
	int ret;

	ret = cpu_psci_ops.cpu_boot(cpu);
	if (ret < 0)
		return ret;

    spm_table_cpu_boot(cpu);

    psci_dbg("CPU boot , spm done \n");
	return ret; //(cpu, STA_POWER_ON, 1);
}

#ifdef CONFIG_HOTPLUG_CPU
static int atc_psci_cpu_disable(unsigned int cpu)
{
	return cpu_psci_ops.cpu_disable(cpu);
}

static void atc_psci_cpu_die(unsigned int cpu)
{
	cpu_psci_ops.cpu_die(cpu);
}

extern int spm_table_cpu_kill(unsigned int cpu);
static int atc_psci_cpu_kill(unsigned int cpu)
{
	int ret;

	ret = cpu_psci_ops.cpu_kill(cpu);
	if (!ret)
		psci_err("CPU%d may not have shut down cleanly\n", cpu);
	
	msleep(10);
	ret =  spm_table_cpu_kill(cpu);
	return ret;
}
#endif

#ifdef CONFIG_CPU_IDLE
static int atc_psci_cpu_init_idle(struct device_node *cpu_node,
				 unsigned int cpu)
{
	return cpu_psci_ops.cpu_init_idle(cpu_node, cpu);
}

static int atc_psci_cpu_suspend(unsigned long index)
{
	return cpu_psci_ops.cpu_suspend(index);
}
#endif

const struct cpu_operations atc_cpu_psci_ops = {
	.name = "atc-boot",
#ifdef CONFIG_CPU_IDLE
	.cpu_init_idle	= atc_psci_cpu_init_idle,
	.cpu_suspend	= atc_psci_cpu_suspend,
#endif
	.cpu_init = atc_psci_cpu_init,
	.cpu_prepare = atc_psci_cpu_prepare,
	.cpu_boot = atc_psci_cpu_boot,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable = atc_psci_cpu_disable,
	.cpu_die = atc_psci_cpu_die,
	.cpu_kill = atc_psci_cpu_kill,
#endif
};

#endif

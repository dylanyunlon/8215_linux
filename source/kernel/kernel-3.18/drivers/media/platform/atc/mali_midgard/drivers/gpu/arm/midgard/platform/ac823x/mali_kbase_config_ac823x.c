/*
 *
 * (C) COPYRIGHT 2011-2014 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



#include <linux/ioport.h>
#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <mali_kbase_config.h>

#ifndef MALI_CONFIG_OF
static struct kbase_io_resources io_resources = {
	.job_irq_number = 104+36,
	.mmu_irq_number = 104+35,
	.gpu_irq_number = 104+34,
	.io_memory_region = {
			     .start = 0x10060000,
			     .end = 0x10060000 + (4096 * 4) - 1}
};
#endif

#include "spm_vfy_cmd.h"

static int pm_callback_power_on(struct kbase_device *kbdev)
{
	//pr_info("%s.\n", __func__);
	//g3d_power_on();
	return 1;
}

static void pm_callback_power_off(struct kbase_device *kbdev)
{
	//pr_info("%s.\n", __func__);
	//g3d_power_off();
}


static void pm_callback_resume(struct kbase_device *kbdev)
{
	pr_info("%s.\n", __func__);
	g3d_power_on();
}

static void pm_callback_suspend(struct kbase_device *kbdev)
{
	pr_info("%s.\n", __func__);
	g3d_power_off();
}

struct kbase_pm_callback_conf pm_callbacks = {
	.power_on_callback = pm_callback_power_on,
	.power_off_callback = pm_callback_power_off,
	.power_suspend_callback  = pm_callback_suspend,
	.power_resume_callback = pm_callback_resume
};

static struct kbase_platform_config versatile_platform_config = {
#ifndef MALI_CONFIG_OF
	.io_resources = &io_resources
#endif
};

struct kbase_platform_config *kbase_get_platform_config(void)
{
	return &versatile_platform_config;
}

int kbase_platform_early_init(void)
{
	g3d_power_on();

	return 0;
}

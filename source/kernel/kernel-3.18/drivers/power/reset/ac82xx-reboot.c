/*
 * Generic AC82xx Reboot Driver
 *
 * Copyright (c) 2017 AutoChips Inc.
 * Author: Shouzhi.Chen@autochips.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>

#include <asm/system_misc.h>

#define	REG_RW_RESRV1	0x160
#define REG_RW_WDT	0x008
#define REG_RW_WDTSET	0x004

#define __io(a)	((void __iomem *)(a))

static void __iomem *watchdog_reg_base;

static inline u32 watchdog_readl(u64 regaddr)
{
	return __raw_readl(__io(watchdog_reg_base + regaddr));
}

static inline void watchdog_writel(u32 regval32, u64 regaddr)
{
	__raw_writel(regval32, __io(watchdog_reg_base + regaddr));
}


void ac82xx_arch_reset(enum reboot_mode mode, const char *cmd)
{
	/*
	 * use powerdown watch dog to reset system
	 */
	uint32_t u4Test;
	pr_info("System reset\n");
	watchdog_writel(0x33653365, REG_RW_RESRV1);
	watchdog_writel(0xff000000, REG_RW_WDT);
	for(u4Test = 0; u4Test < 10000; u4Test++) {
	}
	watchdog_writel(1, REG_RW_WDTSET);
	msleep(200);
	while(1);

}
void ac82xx_power_off(void)
{
	/* ac82xx_arch_reset(0,NULL); */
	pr_info("System power off\n");
	msleep(200);
	while(1);
}

static const struct of_device_id reset_of_match[] = {
	{.compatible = "Autochips,ac82xx-reset",},
	{},
};

MODULE_DEVICE_TABLE(of, reset_of_match);

static int ac82xx_reset_probe(struct platform_device *dev)
{
	watchdog_reg_base = of_iomap(dev->dev.of_node, 0);
	if (watchdog_reg_base == 0) {
		return -ENODEV;
	}
	arm_pm_restart =  ac82xx_arch_reset;
	pm_power_off = ac82xx_power_off;

	return 0;
}


static struct platform_driver ac82xx_reset_driver = {
	.probe = ac82xx_reset_probe,
	.driver = {
		.name = "ac82xx-reset",
		.of_match_table = reset_of_match,
	},
};


static int __init ac82xx_reset_init(void)
{
	return platform_driver_register(&ac82xx_reset_driver);
}

arch_initcall(ac82xx_reset_init);


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

#include <generated/atc_project.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include "oal.h"
#include "winutil.h"
#include "x_ver.h"
#include "ybr_vga_drv_if.h"
#include "vga_hal_api.h"
#include "ybr_vga_errcode.h"
#include "ybr_vga_hw_reg.h"
#include "ybr_vga_common.h"


#define LOG_TAG "drv_mod"

#define NO_IRQ ((unsigned int) -1)

unsigned int ybr_irq;
struct clk *g_clk_ybr = NULL;

unsigned long IO_BASE_REG_VA;

static int ybr_vga_open(struct inode *inode, struct file *filp)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return 0;
}

static int ybr_vga_release(struct inode *inode, struct file *filp)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return 0;
}

static long ybr_vga_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return 0;
}

const struct file_operations ybr_vga_fops = {
	.open = &ybr_vga_open,
	.release = &ybr_vga_release,
	.unlocked_ioctl = &ybr_vga_ioctl,
};

static struct miscdevice ybr_vga_dev = {
	MISC_DYNAMIC_MINOR,
	"ybr_vga",
	&ybr_vga_fops
};

static int  ybr_vga_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *dn;
	void __iomem *reg_addr = NULL;

#ifdef CONFIG_ATC_PLATFORM_ac823x
	IO_BASE_REG_VA = ioremap(IO_PHY_ADDR, IO_PHYS_SIZE);
	YBRVGA_DEBUG(LOG_TAG, "IO_BASE_REG_VA:0x%lx\n", IO_BASE_REG_VA);
#endif

	platform_set_drvdata(pdev, &ybr_vga_dev);
	dn = pdev->dev.of_node;

	g_clk_ybr = devm_clk_get(&pdev->dev, "ybr-device");
	if (NULL == g_clk_ybr) {
		YBRVGA_ERROR(LOG_TAG, "get ybr clk error!\n");
		return -1;
	}

	ybr_irq = (unsigned int)irq_of_parse_and_map(dn, 0);
	if (NO_IRQ == ybr_irq) {
		YBRVGA_ERROR(LOG_TAG, "get ybr irq error!\n");
		return -1;
	}
#ifdef CONFIG_ATC_PLATFORM_ac823x
	ybr_irq -= 32;
#endif
	YBRVGA_DEBUG(LOG_TAG, "get ybr irq(%d) success\n", ybr_irq);

	reg_addr = of_iomap(dn, 0);
	if (NULL == reg_addr) {
		YBRVGA_ERROR(LOG_TAG, "get ybr reg_addr error!\n");
		return -1;
	}
	YBRVGA_DEBUG(LOG_TAG, "get ybr reg_addr(0x%08x) success\n", reg_addr);

	ret = misc_register(&ybr_vga_dev);
	if (ret) {
		YBRVGA_ERROR(LOG_TAG, "misc_register error!\n");
		return ret;
	}
	YBRVGA_INFO(LOG_TAG, "success!\n");

	return 0;
}

static int ybr_vga_remove(struct platform_device *pdev)
{
	misc_deregister(&(ybr_vga_dev));

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int ybr_vga_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}
static int ybr_vga_legacy_resume(struct platform_device *dev)
{
	return 0;
}
#endif

#ifdef CONFIG_PM
static int ybr_vga_suspend(struct device *dev)
{
	//vDrvVideoSuspend();
	return 0;
}

static int ybr_vga_resume(struct device *dev)
{
	//vDrvVideoResume();
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int ybr_vga_runtime_suspend(struct device *dev)
{
	return 0;
}

static int ybr_vga_runtime_resume(struct device *dev)
{
	return 0;
}

static int ybr_vga_runtime_idle(struct device *dev)
{
	return 0;
}
#endif

static const struct dev_pm_ops ybr_vga_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(&ybr_vga_suspend, &ybr_vga_resume)
#ifdef CONFIG_PM_RUNTIME
	SET_RUNTIME_PM_OPS(&ybr_vga_runtime_suspend, &ybr_vga_runtime_resume, &ybr_vga_runtime_idle)
#endif
};
#endif

static const struct of_device_id ybr_of_ids[] = {
	{ .compatible = "atc,ybr", },
	{}
};

static struct platform_driver ybr_vga_driver = {
	.driver = {
		.name = "ybr_vga",
		.owner = THIS_MODULE,
		.of_match_table = ybr_of_ids,
#ifdef CONFIG_PM
		.pm = &ybr_vga_dev_pm_ops,
#endif
	},
	.probe  = &ybr_vga_probe,
	.remove = &ybr_vga_remove,
#ifdef CONFIG_PM_SLEEP
	.suspend = &ybr_vga_legacy_suspend,
	.resume = &ybr_vga_legacy_resume,
#endif
};

static int __init ybr_vga_init(void)
{
	int ret = 0;

	ret = os_driver_register(&ybr_vga_driver);
	if (ret) {
		YBRVGA_ERROR(LOG_TAG, "os_driver_register failed!\n");
		return ret;
	}

	/*output version info*/
	MOD_VERSION_INFO(YBR_VGA_MOD_NAME, YBR_VGA_VER_MAIN, YBR_VGA_VER_MINOR, YBR_VGA_VER_REV);
	YBRVGA_INFO(LOG_TAG, "success!\n");

	return ret;
}
module_init(ybr_vga_init);

static void __exit ybr_vga_exit(void)
{
	os_driver_unregister(&ybr_vga_driver);
}
module_exit(ybr_vga_exit);



MODULE_DESCRIPTION("ATC YPbPr/VGA Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ATC");


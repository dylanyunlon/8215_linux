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
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "x_ver.h"
#include "windev.h"
#include "oal.h"
#include "tvd_drv_if.h"
#include "tvd_log.h"



/**
Revision Control
*/
#define TVD_MOD_NAME    "TVD"
#define TVD_VER_MAIN    1
#define TVD_VER_MINOR   0
#define TVD_VER_REV     0


struct clk *clk_ac8317_tvd1;
struct clk *clk_ac8317_tvd2;
u32 tvd_irq[4] = {110, 116, 125, 148};
unsigned long  tvd_base[4] = {0};
unsigned long  IO_VBASE_VA = 0;
extern unsigned long TDC_DRAM_BASE;
extern unsigned int TDC_DRAM_SIZE;

struct platform_device t_dev = {
	.name = "tvddrv",
	.id   = -1 ,
};


#define __TVD_PM

#ifdef __TVD_PM
#define D0 0
#define D4 4
unsigned int  tvd_power_state = D0;
#endif


#ifndef NR_MEASURE_ENABLE
#define NR_MEASURE_ENABLE
#endif



static long tvd_ioctl(struct file *filp, unsigned int cmd, unsigned long data)
{
	int ret = -EINVAL;
	void __user *arg = (void __user *)data;

	switch (cmd) {
	case TVD_CONTROL_CODE_INIT: {
		TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T tvd_init_info;

		if (copy_from_user(&tvd_init_info, arg, sizeof(tvd_init_info))) {
			ret = -EFAULT;
			break;
		}

		break;
	}

	case TVD_CONTROL_CODE_CONFIG: {
		TVD_DRV_CAMERA_PREVIEW_CFG_T tvd_config_info;

		if (copy_from_user(&tvd_config_info, arg, sizeof(tvd_config_info))) {
			ret = -EFAULT;
			break;
		}

		break;
	}

	case TVD_CONTROL_CODE_START:
		break;

	case TVD_CONTROL_CODE_STOP:
		break;

	default:
		break;
	}

	return  ret;
}


static const struct file_operations tvd_fops = {
	.unlocked_ioctl = tvd_ioctl,
};


static struct miscdevice tvd_dev = {
	MISC_DYNAMIC_MINOR,
	"tvd",
	&tvd_fops
};

#if 0 /*NR_MEASURE_ENABLE  pending*/
/*Export the param of RF threshold.*/
/*extern unsigned int g_u4RFThreshold;*/
static ssize_t tvd_rfthreshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;

	size = sprintf(buf, "g_u4RFThreshold = %u\r\n", g_u4RFThreshold);
	return size;
}
static ssize_t tvd_rfthreshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/*sscanf(buf, "%u", &g_u4RFThreshold);*/
	return count;
}

static DEVICE_ATTR(rfthreshold, S_IWUSR | S_IRUGO, tvd_rfthreshold_show, tvd_rfthreshold_store);
#endif


#ifdef __TVD_PM
static ssize_t tvd_powlevel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;

	if (tvd_power_state == D0) {
		size = sprintf(buf, "tvd power state :D0 \r\n");
	} else {
		size = sprintf(buf, "tvd power state :D4 \r\n");
	}

	return size;
}

static ssize_t tvd_powlevel_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static DEVICE_ATTR(powlevel, S_IWUSR | S_IRUGO, tvd_powlevel_show, tvd_powlevel_store);
#endif


static int  tvd_probe(struct platform_device *pdev)
{
	int ret = 0;
	int result = 0;
	u32 regs[4] = {0};
	void __iomem *reg_addr = NULL;
	struct device_node *dn = NULL;
	struct device_node *node = NULL;
    void __iomem *tvd_sysreg_base = NULL;
	dn = pdev->dev.of_node;
	pr_info("tvd_probe\n");
	/* get tvdclock struct */
	clk_ac8317_tvd1 = devm_clk_get(&pdev->dev, "tvd-clock1");

	if (clk_ac8317_tvd1 == NULL) {
		pr_err("get tvd clk one error!\n");
		//return -1;
	}

	clk_ac8317_tvd2 = devm_clk_get(&pdev->dev, "tvd-clock2");

	if (clk_ac8317_tvd2 == NULL) {
		pr_err("get tvd clk two error!\n");
		//return -1;
	}

	/* get tvdirq */
	//tvd_irq = irq_of_parse_and_map(dn, 0);
	tvd_irq[0] = 110;
	tvd_irq[1] = 116;
	tvd_irq[2] = 125;
	tvd_irq[3] = 148;
	//110 116 125 148
	/*if (tvd_irq[0] == NO_IRQ) {
		pr_err("get tvd irq error!\n");
		return -1;
	}*/


	/* get reg */
	node = of_find_compatible_node(NULL, NULL, "atc,tvd");
	if (node) {
		tvd_base[0] = (unsigned long)of_iomap(node, 0);
		tvd_base[1] = (unsigned long)of_iomap(node, 1);
		tvd_base[2] = (unsigned long)of_iomap(node, 2);
		tvd_base[3] = (unsigned long)of_iomap(node, 3);
		if (tvd_base[0] == 0) {
            pr_err("[GPIO]can't find io virtual base address");
			return -1;
		}
		pr_info("tvd_base[0]=%lx tvd_base[1]=%lx tvd_base[2]=%lx tvd_base[3]=%lx\n", tvd_base[0],tvd_base[1],tvd_base[2],tvd_base[3]);
	}else {
        pr_err("[TVD]can't find compatible node\n");
	}
	IO_VBASE_VA = ioremap(0x10000000,0x00400000);
	pr_info("tvd_ana_base = 0x%lx\n", IO_VBASE_VA);
	
	node = of_find_compatible_node(NULL, NULL, "atc-tvd");
	if (node) {
		result = of_property_read_u32_array(node, "reg", regs, 4);
		if (0 != result) {
			pr_err("of_property_read_u32_array reg Fail\r\n");
			return -1;
		}

		pr_info(" probe find reg(0x%08x, 0x%08x, 0x%08x, 0x%08x)!!\r\n",
			regs[0], regs[1], regs[2], regs[3]);
		
		TDC_DRAM_BASE = regs[1];
		pr_info("get TDC_DRAM_BASE = 0x%x \r\n", TDC_DRAM_BASE);

		TDC_DRAM_SIZE = regs[3];
		pr_info("get TDC_DRAM_SIZE = 0x%x \r\n", TDC_DRAM_SIZE);
		if (TDC_DRAM_BASE == 0 || TDC_DRAM_SIZE == 0) {
			pr_err("can't find tdc reserved memory base address");
			return -1;
		}
	}else {
        pr_err("[TVD]can't find compatible node\n");
	}
	platform_set_drvdata(pdev, &tvd_dev);

	ret = misc_register(&(tvd_dev));

	if (ret) {
		pr_err("tvd_probe misc_register error!\n");
		return ret;
	}

	if (ret) {
		pr_err("tvd_probe create device file error!\n");
		return ret;
	}

#ifdef __TVD_PM
	ret = os_device_create_file(&(pdev->dev), &dev_attr_powlevel);
#endif
	return ret;
}

static int  tvd_remove(struct platform_device *pdev)
{
	misc_deregister(&(tvd_dev));

#ifdef __TVD_PM
	os_device_remove_file(tvd_dev.this_device, &dev_attr_powlevel);
#endif

	return 0;
}

static int tvd_resume(struct platform_device *device)
{
	return 0;
}

static int tvd_suspend(struct platform_device *device, pm_message_t state)
{
	return 0;
}

#ifdef __TVD_PM
static int tvd_pm_ops_suspend(struct device *dev)
{
	if (tvd_power_state == D0) {
		tvd_power_mgr(4);
		tvd_power_state = D4;
	}

	return 0;
}

static int tvd_pm_ops_resume(struct device *dev)
{
	if (tvd_power_state == D4) {
		tvd_power_mgr(0);
		tvd_power_state = D0;
	}

	return 0;
}
#endif

static const struct dev_pm_ops tvd_pm_ops = {
#ifdef __TVD_PM
	.suspend    = tvd_pm_ops_suspend,
	.resume     = tvd_pm_ops_resume,
#endif
};

static const struct of_device_id tvd_of_ids[] = {
	{ .compatible = "atc,tvd", },
	{}
};

static struct platform_driver tvd_driver = {
	.probe      = tvd_probe,
	.remove     = tvd_remove,
	.resume     = tvd_resume,
	.suspend    = tvd_suspend,
	.driver     = {
		.name    = "tvddrv",
		.owner  = THIS_MODULE,
		.pm     = &tvd_pm_ops,
		.of_match_table = tvd_of_ids,
	},
};


static int __init tvd_init(void)
{
	void __iomem *tvd_sysreg_base = NULL;
	int ret;
	pr_debug("tvd_init enter");
	MOD_VERSION_INFO(TVD_MOD_NAME, TVD_VER_MAIN, TVD_VER_MINOR, TVD_VER_REV);
	ret = os_driver_register(&tvd_driver);
	if(ret){
		printk("tvd platform_driver_register err\n");
	}
	return ret;
}

static void __exit tvd_exit(void)
{
	pr_debug("tvd_init exit");
	os_driver_unregister(&tvd_driver);
}

module_init(tvd_init);
module_exit(tvd_exit);

MODULE_DESCRIPTION("atc tvd driver");
MODULE_LICENSE("GPL");

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
u32 tvd_irq;
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
	void __iomem *reg_addr = NULL;
	struct device_node *dn = NULL;

	dn = pdev->dev.of_node;

	/* get tvdclock struct */
	clk_ac8317_tvd1 = devm_clk_get(&pdev->dev, "tvd-clock1");
	if (clk_ac8317_tvd1 == NULL) {
		TVD_LOG(TVD_LOG_LVL_ERR, "get tvd-clock1 error!\n");
		return -1;
	}

	clk_ac8317_tvd2 = devm_clk_get(&pdev->dev, "tvd-clock2");
	if (clk_ac8317_tvd2 == NULL) {
		TVD_LOG(TVD_LOG_LVL_ERR, "get tvd-clock2 error!\n");
		return -1;
	}

	/* get tvdirq */
	tvd_irq = irq_of_parse_and_map(dn, 0);
	if (tvd_irq == NO_IRQ) {
		TVD_LOG(TVD_LOG_LVL_ERR, "get tvd irq error!\n");
		return -1;
	}

	/* get reg */
	reg_addr = of_iomap(dn, 0);
	if (reg_addr == NULL) {
		TVD_LOG(TVD_LOG_LVL_ERR, "get tvd reg base addr error!\n");
		return -1;
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "got tvd reg base addr = [%x]\n", (unsigned int)reg_addr);

	platform_set_drvdata(pdev, &tvd_dev);

	ret = misc_register(&(tvd_dev));
	if (ret) {
		TVD_LOG(TVD_LOG_LVL_ERR, "tvd_probe misc_register error!\n");
		return ret;
	}

#ifdef __TVD_PM
	ret = os_device_create_file(&(pdev->dev), &dev_attr_powlevel);
	if (ret) {
		TVD_LOG(TVD_LOG_LVL_ERR, "tvd_probe create device file error!\n");
		return ret;
	}
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
	.probe        = tvd_probe,
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
	TVD_LOG(TVD_LOG_LVL_DBG, "tvd_init enter\n");
	MOD_VERSION_INFO(TVD_MOD_NAME, TVD_VER_MAIN, TVD_VER_MINOR, TVD_VER_REV);
	return os_driver_register(&tvd_driver);
}

static void __exit tvd_exit(void)
{
	TVD_LOG(TVD_LOG_LVL_DBG, "tvd_exit enter!\n");
	os_driver_unregister(&tvd_driver);
}

module_init(tvd_init);
module_exit(tvd_exit);

MODULE_DESCRIPTION("atc tvd driver");
MODULE_LICENSE("GPL");











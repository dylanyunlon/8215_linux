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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-03-29
 */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/clk.h>
#include "oal.h"
#include "x_ver.h"

#include "cp_log.h"
#include "cp_if.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define ATC_PP_DRIVER_INFO	"ATC color process driver"

#define MMISC_MODE_NAME                   "VCP"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                     00

static struct vcp_device *g_vcpdev;

static DEFINE_MUTEX(vcp_mutex);

u32 u4VcpEnableReg;
u32 u4VcpHueReg;
u32 u4VcpYGainReg;
u32 u4VcpUVGainReg;
u32 u4VcpBrightReg;
u32 u4VcpSaturationReg;

static long atc_vcp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	return -EFAULT;
}


static int atc_vcp_open(struct inode *inode, struct file *file)
{
	CP_LOG(CP_LOG_LVL_INFO, "vcp_open\n");

	return 0;
}

static int atc_vcp_release(struct inode *inode, struct file *file)
{
	CP_LOG(CP_LOG_LVL_INFO, "vcp_release\n");

	return 0;
}

static const struct file_operations vcp_fops = {
	.owner = THIS_MODULE,
	.open = atc_vcp_open,
	.release = atc_vcp_release,
	.unlocked_ioctl = atc_vcp_ioctl,
};

void __iomem *vcp_front_base = NULL;
void __iomem *vcp_rear_base = NULL;

static int vcp_probe(struct platform_device *pdev)
{
	struct vcp_device *vcpdev;
	struct device_node *nd = pdev->dev.of_node;
	int result;
	unsigned long io_base = 0x10000000;
	unsigned int io_size = 0x100000;
	CP_LOG(CP_LOG_LVL_INFO, "vcp_probe--->\n");

	_IO_BASE_ = ioremap(io_base, io_size);
	if (!_IO_BASE_) {
		CP_LOG(CP_LOG_LVL_ERR, "get io base address failed = %p \r\n",
					_IO_BASE_);
		return -EFAULT;
	}
	CP_LOG(CP_LOG_LVL_INFO, "get io base address _IO_BASE_ = %lx \r\n",_IO_BASE_);

	vcp_front_base = of_iomap(nd, 0);
	if (!vcp_front_base) {
		CP_LOG(CP_LOG_LVL_ERR, " get vcp register base address failed\n");
		return -1;
	}
	CP_LOG(CP_LOG_LVL_INFO, "vcp register base address = %x\n", (unsigned int)vcp_front_base);

	vcp_rear_base = of_iomap(nd, 1);
	if (!vcp_rear_base) {
		CP_LOG(CP_LOG_LVL_ERR, " get vcp register base address failed\n");
		return -1;
	}
	CP_LOG(CP_LOG_LVL_INFO, "vcp register base address = %x\n", (unsigned int)vcp_rear_base);

	vcpdev = kzalloc(sizeof(struct vcp_device), GFP_KERNEL);
	if (vcpdev == NULL) {
		/* dev_err(&pdev->dev, "[vcp]: vcp_probe: malloc device failed\n");  */
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, vcpdev);

	vcpdev->cdev.name = "vcp";
	vcpdev->cdev.minor = MISC_DYNAMIC_MINOR;
	vcpdev->cdev.fops = &vcp_fops;
	vcpdev->cdev.parent = &(pdev->dev);

	result = misc_register(&(vcpdev->cdev));

	if (result == 0) {
		CP_LOG(CP_LOG_LVL_INFO, "vcp init successes\n");
	} else {
		CP_LOG(CP_LOG_LVL_ERR, "vcp misc device register error\n");
		kfree(vcpdev);
		return result;
	}
	g_vcpdev = vcpdev;

	return 0;
}
	 
/* static int __devexit vcp_remove(struct platform_device *pdev) */
static int vcp_remove(struct platform_device *pdev)
{
	struct vcp_device *vcpdev = platform_get_drvdata(pdev);

	if (vcpdev == NULL) {
		CP_LOG(CP_LOG_LVL_ERR, " %s No device when vcp remove!\n",__func__);
		return -ENODEV;
	}

	/* todo:release something when remove */
	/* i4vcpUninit(); */

	misc_deregister(&(vcpdev->cdev));

	kfree(g_vcpdev);
	g_vcpdev = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int atc_vcp_suspend(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);

	vcpdev = platform_get_drvdata(pdev);

	if (vcpdev == NULL)
		return -ENODEV;

	CP_LOG(CP_LOG_LVL_INFO, " %s\r\n", __func__);

	return 0;
}

static int atc_vcp_resume(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);

	vcpdev = platform_get_drvdata(pdev);
	if (vcpdev == NULL)
		return -ENODEV;

	CP_LOG(CP_LOG_LVL_INFO, " %s\r\n", __func__);

	return 0;
}
#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops atc_vcp_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(atc_vcp_suspend, atc_vcp_resume)
};
#endif

static const struct of_device_id vcp_of_ids[] = {
	{.compatible = "Autochips,colorprocess",},
	{}
};

static struct platform_driver vcp_plt_drv = {
	.driver = {
		.name = "ac823x-vcp",
		.owner = THIS_MODULE,
		.of_match_table = vcp_of_ids,
#ifdef CONFIG_PM
		.pm = &atc_vcp_dev_pm_ops,
#endif
	},
	.probe = vcp_probe,
	.remove = vcp_remove,
};

static int __init vcp_init(void)
{
	int ret;

	CP_LOG(CP_LOG_LVL_INFO, "vcp_init--->\n");
	ret = platform_driver_register(&vcp_plt_drv);
	if (ret)
		CP_LOG(CP_LOG_LVL_ERR, " %s: register  driver failed\n", __func__);
	return ret;
}
module_init(vcp_init);

static void __exit vcp_exit(void)
{
	CP_LOG(CP_LOG_LVL_INFO, "vcp_exit--->\n");
	platform_driver_unregister(&vcp_plt_drv);
}
module_exit(vcp_exit);

MODULE_DESCRIPTION(ATC_PP_DRIVER_INFO);
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Ziran Xu <ziran.xu@autochips.com>");


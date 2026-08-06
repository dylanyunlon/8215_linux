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

 
/************************************************************************************************
 *
 * Filename:
 * ---------
 *   $Workfile:$
 *
 * Project:
 * --------
 * AC83XX Android Prototype
 *
 * Description:
 * ------------
 * video decode driver kernel module 
 *
 * Author:
 * -------
 * mtk94039 : 2012-09, draft
 *
 * $Modtime: $
 *
 * $Revision: #1 $
 ************************************************************************************************/
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h> 
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <asm/page.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pm.h>

#include "os_rpm.h"

struct file_operations sample_fops;
static struct miscdevice misc_dev = {
	MISC_DYNAMIC_MINOR,
	"sample",
	&sample_fops
};

static long sample_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct device *pdevice = misc_dev.parent;

	pm_runtime_get_sync(pdevice);
	// TODO
	pm_runtime_put_sync(pdevice);

	return 0;
}


static int sample_open(struct inode *inode, struct file *file)
{
	struct device *pdevice = misc_dev.parent;

	pm_runtime_get_sync(pdevice);
	// TODO
	pm_runtime_put_sync(pdevice);

	return 0;
}

static int sample_release(struct inode *inode, struct file *file)
{
	struct device *pdevice = misc_dev.parent;

	pm_runtime_get_sync(pdevice);
	// TODO
	pm_runtime_put_sync(pdevice);

	return 0;
}

static int sample_mmap(struct file *fp, struct vm_area_struct *vma)
{
	struct device *pdevice = misc_dev.parent;

	pm_runtime_get_sync(pdevice);
	// TODO
	pm_runtime_put_sync(pdevice);

	return 0;
}

struct file_operations sample_fops = {
	.release = sample_release,
	.open = sample_open,
	.mmap = sample_mmap,
	.unlocked_ioctl = sample_ioctl,
};
static int sample_runtime_idle(struct device *dev)
{
	return 0;
}

static int sample_runtime_suspend(struct device *dev)
{
	return 0;
}

static int sample_runtime_resume(struct device *dev)
{
	return 0;
}

static struct dev_pm_ops sample_pm_ops = {
	.runtime_idle		=	sample_runtime_idle,
	.runtime_suspend	=	sample_runtime_suspend,
	.runtime_resume		=	sample_runtime_resume,
};

static int _sample_init(struct device *parent)
{
	misc_dev.parent = parent;

	misc_register(&misc_dev);

	return 0;
}

static void _sample_exit(void)
{
	misc_unregister(&misc_dev);
}

static struct os_rpm_driver rpm_driver = {
	.name = "sample",
	.ppm_ops = &sample_pm_ops,
	.init = _sample_init,
	.exit = _sample_exit,
};

static int __init sample_init(void)
{
	return os_rpm_driver_register(&rpm_driver);
}

static void __exit sample_exit(void)
{
	return os_rpm_driver_unregister();
}

module_init(sample_init);
module_exit(sample_exit);




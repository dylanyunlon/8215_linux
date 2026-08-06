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

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysrq.h>

#include <linux/sysfs.h>

#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>

#include "../inc/oal.h"
#include <linux/irq.h>
#include "../inc/x_ver.h"

#define MTK_KERNEL_LINUX_LICENSE     "GPL"
#define MTK_LICENSE_IS_GPL 1



struct bus_type os_bus_type;

struct irq_data *os_irq_get_irq_data(unsigned int irq)
{
	return irq_get_irq_data(irq);
}

int os_device_create_file(struct device *dev,
		       const struct device_attribute *attr)
{
	return device_create_file(dev, attr);

}




void os_device_remove_file(struct device *dev,
			const struct device_attribute *attr)
{
	return device_remove_file(dev, attr);
}



int os_device_init_wakeup(struct device *dev, bool val)
{
	return device_init_wakeup(dev, val);
}

void os_sysfs_notify(struct kobject *k, const char *dir, const char *attr)
{
	return sysfs_notify(k, dir, attr);
}

void os_remove_file(struct device *dev,
			const struct device_attribute *attr)
{
	return device_remove_file(dev, attr);
}


int os_create_file(struct device *dev,
		       const struct device_attribute *attr)
{

	return device_create_file(dev, attr);
}



int os_device_register(struct platform_device *pdev)
{
	return platform_device_register(pdev);
}



void os_device_destroy(struct class *class, dev_t devt)
{
	return device_destroy(class, devt);

}

void os_class_destroy(struct class *cls)
{
	return class_destroy(cls);

}

struct class * __must_check os_class_create(struct module *owner,
						  const char *name)
{
	return class_create(owner, name);

}

struct device *os_device_create(struct class *class, struct device *parent,
			     dev_t devt, void *drvdata, const char *fmt)
{
	return device_create(class, parent, devt, drvdata, fmt);
}

struct platform_device *os_device_register_simple(
		const char *name, int id,
		const struct resource *res, unsigned int num)
{
	return platform_device_register_simple(name, id, res, num);
}


void os_driver_unregister(struct platform_driver *drv)
{
	return platform_driver_unregister(drv);
}

int os_driver_register(struct platform_driver *drv)
{

	return platform_driver_register(drv);
}

void os_device_unregister(struct platform_device *pdev)
{
	return platform_device_unregister(pdev);
}

int os_create_fs_group(struct kobject *kobj,
		       const struct attribute_group *grp)
{
	return sysfs_create_group(kobj, grp);
}

void os_remove_fs_group(struct kobject * kobj, 
			const struct attribute_group * grp)
{
	return sysfs_remove_group(kobj,grp);

}
struct kobject *os_create_and_add_node(const char *name, struct kobject *parent)
{
   return kobject_create_and_add(name, parent);
}

 long ac83xx_oal_ioctl(struct file *file, unsigned int cmd,
			  unsigned long arg)
{
	
	 return 0;
}

static const struct file_operations ac83xx_oal_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= ac83xx_oal_ioctl,
};

static struct miscdevice ac83xx_oal_dev = {
	MISC_DYNAMIC_MINOR,
	"ac83xx_oal",
	&ac83xx_oal_fops
};


#define OAL_VER_MAIN	1
#define OAL_VER_MINOR	00
#define OAL_VER_REV	00

static int __init oal_init(void)
{
	int ret;

	MOD_VERSION_INFO("ac83xx_oal",OAL_VER_MAIN,OAL_VER_MINOR,OAL_VER_REV);

	ret = misc_register(&ac83xx_oal_dev);
	if(ret)
    {
        printk(KERN_ERR "Unable to register \"ac83xx_oal\" misc device\n");
    }
    else
    {
        printk(KERN_ERR "oal init success\n"); 
    }

	return ret;
}


static void __exit oal_exit(void)
{		

	misc_deregister(&ac83xx_oal_dev);	
	printk(KERN_ERR "\"ac83xx_oal_dev\" deinit success\n");
}
	



int os_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
    return queue_work(wq, work);
}


struct workqueue_struct * os_create_workqueue(const char *name)
{
	return create_workqueue(name);
}

module_init(oal_init);
module_exit(oal_exit);


EXPORT_SYMBOL(os_irq_get_irq_data);
EXPORT_SYMBOL(os_device_create_file);
EXPORT_SYMBOL(os_device_remove_file);
EXPORT_SYMBOL(os_sysfs_notify);
EXPORT_SYMBOL(os_remove_file);
EXPORT_SYMBOL(os_create_file);
EXPORT_SYMBOL(os_device_init_wakeup);
EXPORT_SYMBOL(os_device_register);
EXPORT_SYMBOL(os_device_destroy);
EXPORT_SYMBOL(os_class_destroy);
EXPORT_SYMBOL(os_class_create);
EXPORT_SYMBOL(os_device_create);
EXPORT_SYMBOL(os_device_register_simple);
EXPORT_SYMBOL(os_driver_unregister);
EXPORT_SYMBOL(os_driver_register);
EXPORT_SYMBOL(os_device_unregister);
EXPORT_SYMBOL(os_create_fs_group);
EXPORT_SYMBOL(os_remove_fs_group);
EXPORT_SYMBOL(os_create_and_add_node);
EXPORT_SYMBOL(os_queue_work);
EXPORT_SYMBOL(os_create_workqueue);



MODULE_LICENSE("GPL");




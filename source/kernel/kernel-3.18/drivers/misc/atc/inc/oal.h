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
/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

#ifndef __OAL_H__
#define __OAL_H__
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/interrupt.h>




extern struct bus_type os_bus_type;
#define platform_bus_type os_bus_type


#define OS_DECLARE_WORK(n, f)  \
	          struct work_struct n = __WORK_INITIALIZER(n, f)

extern struct irq_data *os_irq_get_irq_data(unsigned int irq);


extern int os_device_create_file(struct device *dev,
		       const struct device_attribute *attr);


extern void os_device_remove_file(struct device *dev,
			const struct device_attribute *attr);

extern int os_device_init_wakeup(struct device *dev, bool val);

extern void os_sysfs_notify(struct kobject *k, const char *dir, const char *attr);

 
extern void os_remove_file(struct device *dev, const struct device_attribute *attr);

extern int os_create_file(struct device *dev, const struct device_attribute *attr);
extern int os_device_register(struct platform_device *pdev);
extern void os_device_destroy(struct class *class, dev_t devt);
extern void os_class_destroy(struct class *cls);
extern struct class * __must_check os_class_create(struct module *owner,
							   const char *name);

extern struct device *os_device_create(struct class *class, struct device *parent,
			     dev_t devt, void *drvdata, const char *fmt);
extern struct platform_device *os_device_register_simple(
			 const char *name, int id,
			 const struct resource *res, unsigned int num);


extern void os_driver_unregister(struct platform_driver *drv);

extern int os_driver_register(struct platform_driver *drv);

extern void os_device_unregister(struct platform_device *pdev);

extern int __must_check os_create_fs_group(struct kobject *kobj,
				    const struct attribute_group *grp);

extern void os_remove_fs_group(struct kobject *kobj,
			const struct attribute_group *grp);

extern struct kobject *os_create_and_add_node(const char *name, struct kobject *parent);

extern int os_queue_work(struct workqueue_struct *wq, struct work_struct *work);

extern struct workqueue_struct* os_create_workqueue(const char *name);

#endif


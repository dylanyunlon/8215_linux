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

#ifndef __OS_RPM_H__
#define __OS_RPM_H__

#include <linux/device.h>

struct platform_device;
struct platform_driver;

struct os_rpm_driver {
	char *name,
	struct dev_pm_ops *ppm_ops;
	int (*init)(struct device *parent);
	void (*exit)(void);
};

static struct os_rpm_driver *prpm_driver;

static int __devinit sample_probe(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;

	pm_runtime_enable(pdevice);
	pm_runtime_get_sync(pdevice);

	if (prpm_driver->init)
		prpm_driver->init(pdevice);

	pm_runtime_put_sync(pdevice);

	return 0;
}

static int __devexit sample_remove(struct platform_device *pdev)
{
	struct device *pdevice = &pdev->dev;

	if (prpm_driver->exit)
		prpm_driver->exit();

	return 0;
}

static struct platform_driver sample_drv = {
	.probe		= sample_probe,
	.remove		= __devexit_p(sample_remove),
};

static struct platform_device sample_dev = {
	.id			= 0;
};

static int __init os_rpm_driver_register(struct os_rpm_driver *pdriver)
{
	sample_dev.name = pdriver->name;
	sample_drv.driver.name = pdriver->name;
	sample_drv.driver.pm = pdriver->ppm_ops;
	sample_drv.driver.owner = THIS_MODULE;

	prpm_driver = pdriver;

	platform_device_register(&sample_dev);
	return platform_driver_register(&sample_drv);
}

static void __exit os_rpm_driver_unregister(void)
{
	platform_device_unregister(&sample_dev);
	return platform_driver_unregister(&sample_drv);
}


#endif

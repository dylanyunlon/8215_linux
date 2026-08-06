/*
 * Copyright (C) 2013 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/trusty/smcall.h>
#include <linux/trusty/sm_err.h>
#include <linux/trusty/trusty.h>

static struct platform_device *trusty_atc_dev;
static unsigned int g_sw_resume_entry = 0;

s32 trusty_atc_std_call32(u32 smcnr, u32 a0, u32 a1, u32 a2)
{
#if 1
	return trusty_std_call32(trusty_atc_dev->dev.parent,
				 smcnr, a0, a1, a2);
#endif
}

static int trusty_atc_probe(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s\n", __func__);
	s32 res = 0;

	trusty_atc_dev = pdev;
	
	res = (s32)trusty_atc_std_call32(ATC_GET_SW_RESUME_ENTRY, 0xFFFF, 0xFFFF, 0xFFFF);
	if (res > 0)
		g_sw_resume_entry = (unsigned int) res;

	return 0;
}

static int trusty_atc_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s\n", __func__);

	return 0;
}

#define MODULE_NAME "trusty-atc"
static const struct of_device_id trusty_atc_of_match[] = {
	{ .compatible = "atc,trusty-atc-tee", },
};
MODULE_DEVICE_TABLE(of, trusty_atc_of_match);

static struct platform_driver trusty_atc_driver = {
	.probe = trusty_atc_probe,
	.remove = trusty_atc_remove,
	.driver	= {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = trusty_atc_of_match,
	},
};

static int __init register_trusty_atc_driver(void)
{
	int ret = 0;

	if (platform_driver_register(&trusty_atc_driver)) {
		ret = -ENODEV;
		pr_warn("[%s] could not register device for the device, ret:%d\n",
			MODULE_NAME,
			ret);
		return ret;
	}

	return ret;
}

static int __init trusty_atc_init(void)
{
	int ret = 0;

	ret = register_trusty_atc_driver();
	if (ret) {
		pr_warn("[%s] register device/driver failed, ret:%d\n",
			MODULE_NAME,
			ret);
		return ret;
	}

	return 0;
}

subsys_initcall(trusty_atc_init);


unsigned int trusty_get_resume_entry(void)
{
	return g_sw_resume_entry;
}
EXPORT_SYMBOL(trusty_get_resume_entry);


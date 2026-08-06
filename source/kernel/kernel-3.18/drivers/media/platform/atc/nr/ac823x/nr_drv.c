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
 *Date: 2017-03-10
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_fdt.h>

#include "nr_log.h"
#include "nr_if.h"
#include "oal.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define ATC_NR_DRIVER_INFO	"ATC noise reduce driver"

/*set bypass nr begin*/
static ssize_t nr_bypass_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t nr_bypass_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0, fgBypass;

	NR_LOG(NR_LOG_LVL_INFO, "	Parameters: [fgBypass]\n");
	NR_LOG(NR_LOG_LVL_INFO, "		fgBypass:\n");
	NR_LOG(NR_LOG_LVL_INFO, "			0:enable NR; 1:bypass NR \n");

	ret = sscanf(buf, "%d", (unsigned int *)&fgBypass);
	if (ret < 0) {
		NR_LOG(NR_LOG_LVL_ERR, "nr_bypass_store sscanf error!\n");
		return -EINVAL;
	}

	NrBypass(fgBypass);

	return count;

}
static DEVICE_ATTR(nr4bypass, S_IWUSR | S_IRUGO, nr_bypass_show, nr_bypass_store);
/*set bypass nr end*/

/*set enable 3dnr begin*/
static ssize_t nr_en3d_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t nr_en3d_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0, fgBypass;

	NR_LOG(NR_LOG_LVL_INFO, "	Parameters: [fgBypass]\n");
	NR_LOG(NR_LOG_LVL_INFO, "		fgBypass:\n");
	NR_LOG(NR_LOG_LVL_INFO, "			0:disable 3dNR; 1:enable 3dNR \n");

	ret = sscanf(buf, "%d", (unsigned int *)&fgBypass);
	if (ret < 0) {
		NR_LOG(NR_LOG_LVL_ERR, "nr_en3d_store sscanf error!\n");
		return -EINVAL;
	}

	NrEnable3dFunc(fgBypass);

	return count;

}
static DEVICE_ATTR(nr4en3d, S_IWUSR | S_IRUGO, nr_en3d_show, nr_en3d_store);
/*set enable 3dnr end*/

/*set swap mode begin*/
static ssize_t nr_swap_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t nr_swap_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0, swapMode;

	NR_LOG(NR_LOG_LVL_INFO, "	Parameters: [swapMode]\n");
	NR_LOG(NR_LOG_LVL_INFO, "		swapMode:\n");
	NR_LOG(NR_LOG_LVL_INFO, "			level:[0~6] \n");

	ret = sscanf(buf, "%d", (unsigned int *)&swapMode);
	if (ret < 0) {
		NR_LOG(NR_LOG_LVL_ERR, "nr_swap_store sscanf error!\n");
		return -EINVAL;
	}

	NrSetSwapMode(swapMode);

	return count;

}
static DEVICE_ATTR(nr4swapmode, S_IWUSR | S_IRUGO, nr_swap_show, nr_swap_store);
/*set swap mode end*/

/*set nr level begin*/
static ssize_t nr_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t nr_level_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0, u4Strength, u4FNRStrength, u4MNRStrength, u4BNRStrength;

	NR_LOG(NR_LOG_LVL_INFO, "	Parameters: [u4Strength, u4FNRStrength, u4MNRStrength, u4BNRStrength]\n");
	NR_LOG(NR_LOG_LVL_INFO, "		all value:\n");
	NR_LOG(NR_LOG_LVL_INFO, "			level:[0~3] \n");

	ret = sscanf(buf, "%d %d %d %d",
		(unsigned int *)&u4Strength, (unsigned int *)&u4FNRStrength, (unsigned int *)&u4MNRStrength, (unsigned int *)&u4BNRStrength);
	if (ret < 0) {
		NR_LOG(NR_LOG_LVL_ERR, "nr_level_store sscanf error!\n");
		return -EINVAL;
	}

	NrSetLevel(u4Strength, u4FNRStrength, u4MNRStrength, u4BNRStrength);

	return count;

}
static DEVICE_ATTR(nr4level, S_IWUSR | S_IRUGO, nr_level_show, nr_level_store);
/*set nr level end*/

static long nr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	NR_LOG(NR_LOG_LVL_INFO, "nr_ioctl not be implemented \n");
	/*to do*/

	/*****/
}

struct file_operations const nr_fops = {
	.unlocked_ioctl = nr_ioctl,
};

static struct miscdevice nr_dev = {
	MISC_DYNAMIC_MINOR,
	"nr",
	&nr_fops
};

extern unsigned long _IO_BASE_;
static int nr_probe(struct platform_device *pdev)
{
	unsigned int ret = -EINVAL;
	struct device_node *node;
	struct device_node *nd = pdev->dev.of_node;
	unsigned int property[4];
	unsigned long base_pa, base_va;
	unsigned int size;
	unsigned long io_base = 0x10000000;
	unsigned int io_size = 0x100000;
	NR_LOG(NR_LOG_LVL_INFO, "pp_probe--->\n");

	_IO_BASE_ = ioremap(io_base, io_size);
	if (!_IO_BASE_) {
		NR_LOG(NR_LOG_LVL_ERR, "get io base address failed = %p \r\n", _IO_BASE_);
		goto err;
	}
	NR_LOG(NR_LOG_LVL_INFO, "get io base address _IO_BASE_ = %lx \r\n",_IO_BASE_);

	node = of_find_compatible_node(NULL,NULL,"atc-noisereduce");
	if (node) {
		base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			NR_LOG(NR_LOG_LVL_ERR, "noisereduce buffer of_iomap fail\r\n");
			goto err;
		}
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 4)) {
			NR_LOG(NR_LOG_LVL_ERR, "get noisereduce buffer reserved memory node reg info fail\r\n");
			goto err;
		}
		base_pa = (unsigned long)property[0] << 32 | property[1];
		size = property[3];
		NR_LOG(NR_LOG_LVL_INFO, "noisereduce buffer base va:%x, size:%x, pa:%x\n", base_va, size, base_pa);
		setNrReservemem(base_pa, base_va, size);
	} else {
		NR_LOG(NR_LOG_LVL_ERR, "can not find noisereduce buffer reserved memory node!!\r\n");
		goto err;
	}

	ret = misc_register(&nr_dev);
	if (ret) {
		NR_LOG(NR_LOG_LVL_ERR, "nr_probe: misc_register error %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(nr_dev.this_device, &dev_attr_nr4bypass);
	if (ret) {
		NR_LOG(NR_LOG_LVL_ERR, "cannot create bypass dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(nr_dev.this_device, &dev_attr_nr4en3d);
	if (ret) {
		NR_LOG(NR_LOG_LVL_ERR, "cannot create enable 3d dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(nr_dev.this_device, &dev_attr_nr4swapmode);
	if (ret) {
		NR_LOG(NR_LOG_LVL_ERR, "cannot create swap dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(nr_dev.this_device, &dev_attr_nr4level);
	if (ret) {
		NR_LOG(NR_LOG_LVL_ERR, "cannot create swap dev file %d\r\n", ret);
		goto err;
	}

	//Register NR interrupt
	NrIsrInit();

err:
	return ret;
}

static int nr_remove(struct platform_device *pdev)
{
	NR_LOG(NR_LOG_LVL_INFO, "nr_remove<---\n");

	os_device_remove_file(nr_dev.this_device, &dev_attr_nr4bypass);
	os_device_remove_file(nr_dev.this_device, &dev_attr_nr4en3d);
	os_device_remove_file(nr_dev.this_device, &dev_attr_nr4swapmode);
	os_device_remove_file(nr_dev.this_device, &dev_attr_nr4level);

	misc_deregister(&nr_dev);

	return 0;
}

static const struct of_device_id nr_of_ids[] = {
	{.compatible = "Autochips,noisereduce",},
	{}
};

static struct platform_driver nr_plt_drv = {
	.driver = {
		   .name = "Autochips-noisereduce",
		   .owner = THIS_MODULE,
		   .of_match_table = nr_of_ids,
		   },
	.probe = nr_probe,
	.remove = nr_remove,
};

static int __init nr_init(void)
{
	int ret;

	NR_LOG(NR_LOG_LVL_INFO, "[NR]:nr_init--->\n");
	ret = platform_driver_register(&nr_plt_drv);
	if (ret)
		NR_LOG(NR_LOG_LVL_ERR, "[NR]: %s: register  driver failed\n", __func__);

	NrKernelThreadInit();
}

static void __exit nr_exit(void)
{
	NR_LOG(NR_LOG_LVL_INFO, "nr_exit<---\n");

	platform_driver_unregister(&nr_plt_drv);
}
module_init(nr_init);
module_exit(nr_exit);

MODULE_DESCRIPTION(ATC_NR_DRIVER_INFO);
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);

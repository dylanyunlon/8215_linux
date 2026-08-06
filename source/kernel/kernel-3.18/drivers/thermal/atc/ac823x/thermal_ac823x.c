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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/bcd.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/completion.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <asm/uaccess.h>
#include <linux/spinlock_types.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/device.h>
#include <linux/thermal.h>
#include "mach/mt3365_irqs_vector.h"
#include "thermal.h" 


/* define */
#define CREATE_THERMAL_ATTRIBUTE_FILES	1

/* variables */
unsigned int thermal_irq = 0;

struct clk *thermal_clk_select_slow = NULL;
struct clk *thermal_clk_select_ex = NULL;
struct clk *thermal_clk_gate_slow = NULL;
struct clk *thermal_clk_gate = NULL;
struct clk *thermal_clk_parent = NULL;

void __iomem *reg_addr_base = NULL;
unsigned long thm_reg_base = 0;
unsigned long pdwnc_reg_base = 0;
unsigned long ckgen_reg_base = 0;
unsigned long efuse_reg_base = 0;

spinlock_t thermal_lock;
unsigned long thermal_reg_base0 = 0;
unsigned long thermal_reg_base1 = 0;

#if CREATE_THERMAL_ATTRIBUTE_FILES
volatile int temp_flag = 1;
volatile int temp_interval = 0;
volatile int to_temp = 0;
volatile int to_index = 0;
#endif


/* extern */
extern int init_thermal_hw(void);
extern void vDrvThermal_Cal_Prepare(void);
extern void vDrvThermal_Cal_Prepare_2(void);

/* function */


/*
 * IRQ handler for the Thermal
 */
static irqreturn_t ac823x_thermal_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct device *dev = &(pdev->dev);
	//unsigned int rtsr;
	//unsigned long events = 0;

	if (dev == NULL) {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

#if 0
	rtsr = RTC_READ32(RTC_IRQ_STA);
	if (rtsr) {		/* this interrupt is shared!  Is it ours? */
		RTCProtect(0);
		RTCClearIrqSta();

		if (irq >= 0) {
			mt33xx_mask_ack_bim_irq((uint32_t)irq);  // zplee
		}

		if (rtsr & ALSTA) {
			events |= (RTC_AF | RTC_IRQF);
			dev_dbg(dev, "mark alarm, disable alarm interrupt\n");
			RTCAlarmAllCMPR(0);
			RTCEnableAlarmInt(0);
		}

		if (rtsr & TCSTA) {
			events |= (RTC_IRQF);
			dev_dbg(dev, "disable tc interrupt\n");
			RTCEnableTCInt(0);
		}

		RTCProtect(1);
		rtc_update_irq(rtc, 1, events);

		dev_dbg(dev, "num=%ld, events=0x%02lx\n",
			events >> 8, events & 0x000000FF);

		return IRQ_HANDLED;
	}
#endif

	return IRQ_NONE;		/* not handled */
}


#if CREATE_THERMAL_ATTRIBUTE_FILES
struct thermal_zone_device *g_tz;
extern void start_monitor_thermal_zone(struct thermal_zone_device *tz, int flag);

#if 1
static ssize_t ac823x_thermal_sysfs_show_flag(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (dev != NULL) {
		pr_debug("[thermal] show temp_flag [%d]\n", temp_flag);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	return sprintf(buf, "%d\n", temp_flag);
}

static ssize_t ac823x_thermal_sysfs_store_flag(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		pr_debug("[thermal] store temp_flag [%s], count %d\n", buf, count);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	/*ret = sscanf(buf, "%i", &adj);*/

	if (ret != 0) {
		return -EINVAL;
	}

	if (adj == 0) {
		temp_flag = 0;
		start_monitor_thermal_zone(g_tz, temp_flag);
	} else if (adj == 1) {
		temp_flag = 1;
		start_monitor_thermal_zone(g_tz, temp_flag);
	} else {
		temp_flag = adj;
	}

	return count;
}

static DEVICE_ATTR(temp_flag, S_IRUGO | S_IWUSR,
		   ac823x_thermal_sysfs_show_flag,
		   ac823x_thermal_sysfs_store_flag);

static int ac823x_thermal_sysfs_register_temp_flag(struct device *dev)
{
	return device_create_file(dev, &dev_attr_temp_flag);
}

static void ac823x_thermal_sysfs_unregister_temp_flag(struct device *dev)
{
	device_remove_file(dev, &dev_attr_temp_flag);
}
#endif

#if 1
/* temp_interval */
static ssize_t ac823x_thermal_sysfs_show_temp_interval(struct device *dev,
						   struct device_attribute *attr,
						   char *buf)
{
	if (dev != NULL) {
		pr_debug("[thermal] show passive_delay [%d]\n", temp_interval);
	} else {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

	temp_interval = g_tz->passive_delay / 100;

	return sprintf(buf, "%d\n", temp_interval);
}

static ssize_t ac823x_thermal_sysfs_store_temp_interval(struct device *dev,
						    struct device_attribute *attr,
						    const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		pr_debug("[thermal] store passive_delay [%s], count %d\n", buf, count);
	} else {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	/*ret = sscanf(buf, "%i", &adj);*/

	if (ret != 0) {
		return -EINVAL;
	}

	#if 0
	if (adj == 0) {
		log_time = 0;
	} else if (adj == 1) {
		log_time = 1;
	} else {
		log_time = 1;
	}
	#else
	if (adj >= 0 && adj <= 100) {
		temp_interval = adj;
		g_tz->passive_delay = temp_interval * 100;
	}
	#endif

	return count;
}

static DEVICE_ATTR(temp_interval, S_IRUGO | S_IWUSR,
		   ac823x_thermal_sysfs_show_temp_interval,
		   ac823x_thermal_sysfs_store_temp_interval);

static int ac823x_thermal_sysfs_register_temp_interval(struct device *dev)
{
	return device_create_file(dev, &dev_attr_temp_interval);
}

static void ac823x_thermal_sysfs_unregister_temp_interval(struct device *dev)
{
	device_remove_file(dev, &dev_attr_temp_interval);
}
#endif

#if 1 // for temp transfrom
extern UINT32 tz_raw_to_temp(UINT32 raw, UINT32 ts_idx);

static ssize_t ac823x_thermal_sysfs_show_to_temp(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (dev != NULL) {
		pr_debug("[thermal] show temp [%d]\n", to_temp);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	return sprintf(buf, "%d\n", to_temp);
}

static ssize_t ac823x_thermal_sysfs_store_to_temp(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int adj;
	int ret;
	UINT32 temp = 0;

	if (dev != NULL) {
		pr_debug("[thermal] store temp [%s], count %d\n", buf, count);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	if (ret != 0) {
		return -EINVAL;
	}

	temp = tz_raw_to_temp(adj, to_index);
	to_temp = temp;

	return count;
}

static DEVICE_ATTR(to_temp, S_IRUGO | S_IWUSR,
		   ac823x_thermal_sysfs_show_to_temp,
		   ac823x_thermal_sysfs_store_to_temp);

static int ac823x_thermal_sysfs_register_to_temp(struct device *dev)
{
	return device_create_file(dev, &dev_attr_to_temp);
}

static void ac823x_thermal_sysfs_unregister_to_temp(struct device *dev)
{
	device_remove_file(dev, &dev_attr_to_temp);
}

/////
/* index for to_temp */
static ssize_t ac823x_thermal_sysfs_show_to_index(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (dev != NULL) {
		pr_debug("[thermal] show to_index [%d]\n", to_index);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	return sprintf(buf, "%d\n", to_index);
}

static ssize_t ac823x_thermal_sysfs_store_to_index(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		pr_debug("[thermal] store to_index [%s], count %d\n", buf, count);
	} else {
		pr_err("[thermal] got NULL device \n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	if (ret != 0) {
		return -EINVAL;
	}

	if (adj <= 3 && adj >=0 ) {
		to_index = adj;
	} else {
		to_index = 0;
	}

	return count;
}

static DEVICE_ATTR(to_index, S_IRUGO | S_IWUSR,
		   ac823x_thermal_sysfs_show_to_index,
		   ac823x_thermal_sysfs_store_to_index);

static int ac823x_thermal_sysfs_register_to_index(struct device *dev)
{
	return device_create_file(dev, &dev_attr_to_index);
}

static void ac823x_thermal_sysfs_unregister_to_index(struct device *dev)
{
	device_remove_file(dev, &dev_attr_to_index);
}

#endif


#if 0
static ssize_t ac823x_thermal_sysfs_show_passive_delay(struct device *dev,
						   struct device_attribute *attr,
						   char *buf)
{
	if (dev != NULL) {
		pr_debug("[thermal] show passive_delay [%d]\n", temp_interval);
	} else {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

	temp_interval = g_tz->passive_delay / 100;

	return sprintf(buf, "%d\n", temp_interval);
}

static ssize_t ac823x_thermal_sysfs_store_temp_interval(struct device *dev,
						    struct device_attribute *attr,
						    const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		pr_debug("[thermal] store passive_delay [%s], count %d\n", buf, count);
	} else {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	/*ret = sscanf(buf, "%i", &adj);*/

	if (ret != 0) {
		return -EINVAL;
	}

	#if 0
	if (adj == 0) {
		log_time = 0;
	} else if (adj == 1) {
		log_time = 1;
	} else {
		log_time = 1;
	}
	#else
	if (adj >= 0 && adj <= 100) {
		temp_interval = adj;
		g_tz->passive_delay = temp_interval * 100;
	}
	#endif

	return count;
}

static DEVICE_ATTR(temp_interval, S_IRUGO | S_IWUSR,
		   ac823x_thermal_sysfs_show_temp_interval,
		   ac823x_thermal_sysfs_store_temp_interval);

static int ac823x_thermal_sysfs_register_temp_interval(struct device *dev)
{
	return device_create_file(dev, &dev_attr_temp_interval);
}

static void ac823x_thermal_sysfs_unregister_temp_interval(struct device *dev)
{
	device_remove_file(dev, &dev_attr_temp_interval);
}
#endif

#endif

/*
 * Initialize and install Thermal driver
 */
extern int test_get_temp(void);

extern long tz_get_temp(void);
static int of_thermal_get_ts_temp(void *dev, int *temp)
{
	int temperature = 0;

    temperature = tz_get_temp();
	*temp = temperature;

	return 0;
}

static struct thermal_zone_of_device_ops ac823x_zone_ops = {
	.get_temp		= of_thermal_get_ts_temp,
};


static int ac823x_thermal_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct device *dev = &(pdev->dev);
	struct thermal_zone_device *tz;
	
	int ret = -1;

	/* check device */
	if (dev != NULL) {
		pr_info("[thermal] thermal probe ...\n");
	} else {
		pr_err("[thermal] got NULL thermal device \n");
		return -1;
	}

#if 1
	/* get thermal clock struct */
	thermal_clk_select_slow = devm_clk_get(&pdev->dev, "thermal-select-slow");
	if (thermal_clk_select_slow == NULL) {
		pr_err("[thermal] can not get clk SELECT_SLOW from DTS!\n");
		return -1;
	}
	thermal_clk_select_ex = devm_clk_get(&pdev->dev, "thermal-select");
	if (thermal_clk_select_ex == NULL) {
		pr_err("[thermal] can not get clk SELECT from DTS!\n");
		return -1;
	}
	thermal_clk_gate_slow = devm_clk_get(&pdev->dev, "thermal-gate-slow");
	if (thermal_clk_gate_slow == NULL) {
		pr_err("[thermal] can not get clk GATE_SLOW from DTS!\n");
		return -1;
	}
	thermal_clk_gate = devm_clk_get(&pdev->dev, "thermal-gate");
	if (thermal_clk_gate == NULL) {
		pr_err("[thermal] can not get clk GATE from DTS!\n");
		return -1;
	}
	pr_debug("[thermal] from DTS, got clk [%s]\n", thermal_clk_select_slow->name);
	pr_debug("[thermal] from DTS, got clk [%s]\n", thermal_clk_select_ex->name);
	pr_debug("[thermal] from DTS, got clk [%s]\n", thermal_clk_gate_slow->name);
	pr_debug("[thermal] from DTS, got clk [%s]\n", thermal_clk_gate->name);

	/* select slow clock */
	/* therm_slow_ck: 0 ->clk27m_ck, 1->clk27m_d512, 2->clk27m_d1024, 3->clk27m_d2048 */
	pr_debug("[thermal] select parent for SELECT_SLOW \n");
	thermal_clk_parent = clk_get(NULL, "clk27m_d512");
	if (thermal_clk_parent != NULL && thermal_clk_parent->name != NULL) {
		pr_debug("[thermal] got target parent[%s]\n", thermal_clk_parent->name);
	}

	ret = clk_set_parent(thermal_clk_select_slow, thermal_clk_parent);
	if (ret != 0) {
		pr_err("[thermal] set SELECT_SLOW parent error \n");
	}

	thermal_clk_parent = clk_get_parent(thermal_clk_select_slow);
	pr_debug("[thermal] re-get SELECT_SLOW parent, name [%s]\n", thermal_clk_parent->name);


	/* select clock*/
	/* therm_ck: 0 -> clk27m_ck, 1-> usbpll_d8, 2->syspll_d12, 3->syspll_d16 */
	pr_debug("[thermal] select parent for SELECT \n");
	thermal_clk_parent = clk_get(NULL, "usbpll_d8");
	if (thermal_clk_parent != NULL && thermal_clk_parent->name != NULL) {
		pr_debug("[thermal] got target parent[%s]\n", thermal_clk_parent->name);
	}

	ret = clk_set_parent(thermal_clk_select_ex, thermal_clk_parent);
	if (ret != 0) {
		pr_err("[thermal] set SELECT parent error \n");
	}

	thermal_clk_parent = clk_get_parent(thermal_clk_select_ex);
	pr_debug("[thermal] re-get SELECT parent, name [%s]\n", thermal_clk_parent->name);

	/* on gate slow clock*/
	clk_prepare_enable(thermal_clk_gate_slow);

	/* on gate clock*/
	clk_prepare_enable(thermal_clk_gate);

#endif

#if 1
	/* get rtc irq */
	thermal_irq = irq_of_parse_and_map(dn, 0);
	if (thermal_irq != 0) {
		pr_info("[thermal] got irq = [%d]\n", thermal_irq);

	} else {
		pr_err("[thermal] got irq error !\n");
		return -1;
	}
	pr_info("[thermal] thermal irq, defined[%d], dts[%d] \n", VECTOR_PTP_THERM, VECTOR_PTP1_THERM);
#endif

#if 1
	/* get reg base addr */
	reg_addr_base = of_iomap(dn, 0);
	if (reg_addr_base != NULL) {
		pr_debug("[thermal] got reg base addr 0 = [0x%lx]\n", reg_addr_base);
	} else {
		pr_err("[thermal] get reg base addr 0 error!\n");
		return -1;
	}
	thm_reg_base = (unsigned long)reg_addr_base;

	reg_addr_base = of_iomap(dn, 1);
	if (reg_addr_base != NULL) {
		pr_debug("[thermal] got reg base addr 1 = [0x%lx]\n", reg_addr_base);
	} else {
		pr_err("[thermal] get reg base addr 1 error!\n");
		return -1;
	}
	pdwnc_reg_base = (unsigned long)reg_addr_base;


	reg_addr_base = of_iomap(dn, 2);
	if (reg_addr_base != NULL) {
		pr_debug("[thermal] got reg base addr 2 = [0x%lx]\n", reg_addr_base);
	} else {
		pr_err("[thermal] get reg base addr 2 error!\n");
		return -1;
	}
	ckgen_reg_base = (unsigned long)reg_addr_base;


	reg_addr_base = of_iomap(dn, 3);
	if (reg_addr_base != NULL) {
		pr_debug("[thermal] got reg base addr 3 = [0x%lx]\n", reg_addr_base);
	} else {
		pr_err("[thermal] get reg base addr 3 error!\n");
		return -1;
	}
	efuse_reg_base = (unsigned long)reg_addr_base;

#endif

#if CREATE_THERMAL_ATTRIBUTE_FILES
	/* reg file temp_flag */
	ret = ac823x_thermal_sysfs_register_temp_flag(&pdev->dev);
	if (ret) {
		pr_err("[thermal] creat sys file 'temp_flag' error !\n");
	}

	/* reg file temp_interval */
	ret = ac823x_thermal_sysfs_register_temp_interval(&pdev->dev);

	if (ret) {
		pr_err("[thermal] creat sys file 'temp_interval' error !\n");
	}

	/* reg file to_temp */
	ret = ac823x_thermal_sysfs_register_to_temp(&pdev->dev);
	if (ret) {
		pr_err("[thermal] creat sys file 'to_temp' error !\n");
	}

	/* reg file to_index */
	ret = ac823x_thermal_sysfs_register_to_index(&pdev->dev);
	if (ret) {
		pr_err("[thermal] creat sys file 'to_index' error !\n");
	}
#endif


	/* calibration for temp */
	vDrvThermal_Cal_Prepare();
	vDrvThermal_Cal_Prepare_2();

	/* init thermal hw */
	ret = init_thermal_hw();
	if (ret == -1) {
		pr_err("[thermal] init thermal HW error!\n");
	}

	tz = thermal_zone_of_sensor_register(dev,
						   0,
						   dev,
						   &ac823x_zone_ops);

	if (IS_ERR(tz)) {
		pr_err("[thermal] reg sensor to zone fail \n");
		tz = NULL;
	}

	#if CREATE_THERMAL_ATTRIBUTE_FILES
	g_tz = tz;
	#endif

	/* start polling thermal */
	thermal_zone_device_update(tz, THERMAL_EVENT_UNSPECIFIED);

	/* read temp */
	//test_get_temp();

#if 0
	rtc = rtc_device_register("ac823x-rtc", &pdev->dev,
				  &ac823x_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		return PTR_ERR(rtc);
	}

	platform_set_drvdata(pdev, rtc);
	if (ret == 0) {
		ret = request_irq(VECTOR_RTC, ac823x_rtc_interrupt,
				  0,
				  "ac823x_rtc", pdev);

		if (ret) {
			dev_err(dev, "IRQ %d already in use.\n", VECTOR_RTC);
			rtc_device_unregister(rtc);
			platform_set_drvdata(pdev, NULL);
			return ret;
		}
	}
#endif

	pr_info("[thermal] thermal probe ok.\n");

	return 0;
}

/*
 * Disable and remove the RTC driver
 */
static int ac823x_thermal_remove(struct platform_device *pdev)
{
	// struct rtc_device *rtc = platform_get_drvdata(pdev);
	struct device *dev = &(pdev->dev);

	if (dev != NULL) {
		pr_info("[thermal] remove thermal \n");
	} else {
		pr_err("[thermal] got NULL device !\n");
		return -1;
	}

#if CREATE_THERMAL_ATTRIBUTE_FILES
	/* for thermal sysfs file */
	ac823x_thermal_sysfs_unregister_temp_flag(&pdev->dev);
	ac823x_thermal_sysfs_unregister_temp_interval(&pdev->dev);

	/* unreg file to_temp */
	ac823x_thermal_sysfs_unregister_to_temp(&pdev->dev);

	/* unreg file to_index */
	ac823x_thermal_sysfs_unregister_to_index(&pdev->dev);
#endif

#if 0
	free_irq(VECTOR_RTC, pdev);
	rtc_device_unregister(rtc);
	platform_set_drvdata(pdev, NULL);
#endif

	/* off  gate slow clock*/
    if (thermal_clk_gate_slow != NULL) {
		clk_disable_unprepare(thermal_clk_gate_slow);
    }

	/* off  gate clock */
	if (thermal_clk_gate != NULL) {
		clk_disable_unprepare(thermal_clk_gate);
	}



	return 0;
}

static int ac823x_thermal_suspend(struct platform_device *pdev, pm_message_t state)
{
	pr_info("ac823x_thermal_suspend \n");

	return 0;
}

static int ac823x_thermal_resume(struct platform_device *pdev)
{
	int ret;

	pr_info("ac823x_thermal_resume \n");

	/* init thermal hw */
	ret = init_thermal_hw();
	if (ret == -1) {
		pr_err("[thermal] init thermal HW error!\n");
	}
	
	return 0;
}

static const struct of_device_id thermal_of_ids[] = {
	{ .compatible = "atc,tsensor", },
	{}
};

static struct platform_driver ac823x_thermal_driver = {
	.probe		= ac823x_thermal_probe,
	.remove		= ac823x_thermal_remove,
	.suspend    = ac823x_thermal_suspend,
	.resume		= ac823x_thermal_resume,
	.driver		= {
		.name	= "ac823x_thermal",
		.owner	= THIS_MODULE,
		.of_match_table = thermal_of_ids,
	},
};

static int __init ac823x_thermal_init(void)
{
	return platform_driver_register(&ac823x_thermal_driver);
}

static void __exit ac823x_thermal_exit(void)
{
	platform_driver_unregister(&ac823x_thermal_driver);
}

module_init(ac823x_thermal_init);
module_exit(ac823x_thermal_exit);

MODULE_AUTHOR("ATC Inc.");
MODULE_DESCRIPTION("Thermal driver for AC823X");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ac823x_thermal");


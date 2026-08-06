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
#include <linux/rtc.h>
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
#include "mach/mt3365_irqs_vector.h"
#include "rtc_hal.h"


#define CREATE_RTC_ATTRIBUTE_FILES	0

#if CREATE_RTC_ATTRIBUTE_FILES
volatile int enable_output = 0;
volatile int log_time = 2;
#endif

spinlock_t ac823x_rtc_lock;
unsigned long rtc_reg_base0 = 0;
unsigned long rtc_reg_base1 = 0;

#define RTC_YEAR_DATUM (uint16_t)(2000)
#define RTC_YEAR_END (uint16_t)(2127)

/*
 *Function:  VerifyDateTime
 *This function verify real timer data
*/
int VerifyDateTime(struct device *dev, struct rtc_time *tm)
{
	int rc = 0;
	int32_t MaxDayOfMonth;
	int32_t DayOfMonthArray[12] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if (dev == NULL) {
		dev_err(dev, "gor NULL dev \n");
		rc = -1;
		goto cleanUp;
	}

	if (tm == NULL) {
		dev_err(dev, "gor NULL tm \n");
		rc = -1;
		goto cleanUp;
	}

	//if (((tm->tm_year + 1900) < 2000) || ((tm->tm_year + 1900) > 2127)) {
	if (((tm->tm_year + 1900) < 2000) || ((tm->tm_year + 1900) > 2037)) {
		dev_err(dev, "Year out of range: Year [2000(rtc)-2037(kernel)]\n");
		rc = -1;
		goto cleanUp;
	}

	if (((tm->tm_mon + 1) < 1) || ((tm->tm_mon + 1) > 12)) {
		dev_err(dev, "Month out of range: Month [1-12]\n");
		rc = -1;
		goto cleanUp;
	}

	#if 0
	if ((tm->tm_wday < 0) || (tm->tm_wday >= 7)) {
		dev_err(dev, "Parameter 3 out of range: Day of Week [1-7]\n");
		rc = -1;
		goto cleanUp;
	}
	#endif

	if ((((tm->tm_year + 1900) % 400) == 0)
	    || ((((tm->tm_year + 1900) % 4) == 0)
		&& (((tm->tm_year + 1900) % 100) != 0))) {
		DayOfMonthArray[1] = 29;
	} else {
		DayOfMonthArray[1] = 28;
	}

	MaxDayOfMonth = DayOfMonthArray[tm->tm_mon];

	if ((tm->tm_mday < 1) || (tm->tm_mday > MaxDayOfMonth)) {
		dev_err(dev, "The Max day of %d-%d is %d\n", (tm->tm_year + 1900), (tm->tm_mon + 1), MaxDayOfMonth);
		dev_err(dev, "DayM out of range: Day of Mon [1-%d]\n", MaxDayOfMonth);
		rc = -1;
		goto cleanUp;
	}

	if ((tm->tm_hour < 0) || (tm->tm_hour > 23)) {
		dev_err(dev, "Hour out of range: Hour [0-23]\n");
		rc = -1;
		goto cleanUp;
	}

	if ((tm->tm_min < 0) || (tm->tm_min) > 59) {
		dev_err(dev, "Min out of range: Min [0-59]\n");
		rc = -1;
		goto cleanUp;
	}

	if ((tm->tm_sec < 0) || (tm->tm_sec > 59)) {
		dev_err(dev, "Sec out of range: Sec [0-59]\n");
		rc = -1;
		goto cleanUp;
	}

	rc = 0;

cleanUp:
	return rc;
}

static int ac823x_rtc_output_32k(struct device *dev)
{
	int rc = -1;
	unsigned int out_32k = 0;

	if (dev == NULL) {
		dev_err(dev, "gor NULL dev \n");
		goto cleanUp;
	}

	out_32k = RTC_IO_READ32(0x64);
	out_32k = out_32k | 0x400;
	RTC_IO_WRITE32(0x64, out_32k);

	if (dev != NULL) {
		dev_dbg(dev, "output 32k, set [0x%x] to reg(0x64) \r\n", out_32k);
	}

	rc = 0;

cleanUp:
	return rc;
}


/*
 *Function:  ac823x_rtc_readtime
 *Reads the current RTC value and returns a system time.
*/
static int ac823x_rtc_readtime(struct device *dev, struct rtc_time *tm)
{
	int rc = -1;

	if (dev == NULL) {
		pr_err("got NULL dev\n");
		goto cleanUp;
	}

	if (!tm) {
		dev_err(dev, "got NULL tm\n");
		goto cleanUp;
	}

	if (RTCHWInit() == -1) {
		dev_err(dev, "rtc init fail\n");
		goto cleanUp;
	}

	RTCTOCORE();

	tm->tm_sec = RTCSecRead();
	tm->tm_sec = tm->tm_sec & 0x3F;

	tm->tm_min = RTCMinRead();
	tm->tm_min = tm->tm_min & 0x3F;

	tm->tm_hour = RTCHourRead();
	tm->tm_hour = tm->tm_hour & 0x1F;

	tm->tm_mday = RTCDOMRead();
	tm->tm_mday = tm->tm_mday & 0x1F;

	tm->tm_mon  = RTCMonthRead();
	tm->tm_mon = tm->tm_mon & 0x0F;
	tm->tm_mon = tm->tm_mon - 1;

	/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
	tm->tm_wday = RTCDOWRead();
	tm->tm_wday = tm->tm_wday & 0x07;
	tm->tm_wday = tm->tm_wday % 7;

	tm->tm_year = RTCYearRead();
	tm->tm_year = tm->tm_year & 0x7F;
	tm->tm_year = tm->tm_year + RTC_YEAR_DATUM - 1900;

	if ((rc = VerifyDateTime(dev, tm)) == -1) {
		dev_info(dev, "read time: got error time %4d-%02d-%02d %02d:%02d:%02d\n",
		 	1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
		 	tm->tm_hour, tm->tm_min, tm->tm_sec);
		dev_info(dev, "read time: reset RTC time to 2000/1/1 0:0:0 !\n");

		tm->tm_year = 0;
		tm->tm_mon = 0;
		tm->tm_mday = 1;
		tm->tm_hour = 0;
		tm->tm_min = 0;
		tm->tm_sec = 0;
		tm->tm_wday = 6;

		RTCProtect(0);
		RTCSecSet(tm->tm_sec);
		RTCMinSet(tm->tm_min);
		RTCHourSet(tm->tm_hour);
		RTCDOMSet(tm->tm_mday);
		RTCMonthSet(tm->tm_mon + 1);

		/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
		RTCDOWSet((tm->tm_wday == 0) ? 7 : tm->tm_mday);
		RTCYearSet(tm->tm_year);
		RTCProtect(1); 

		tm->tm_year = tm->tm_year + RTC_YEAR_DATUM - 1900;
	}

	tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon, tm->tm_year);

	if (tm != NULL) {
		dev_info(dev, "read time: %4d-%02d-%02d %02d:%02d:%02d\n",
		 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	rc = 0;

cleanUp:
	return rc;
}


/*
 *Function:  ac823x_rtc_settime
 *Updates the RTC with the specified system time.
*/
static int ac823x_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	int rc = -1;

	if (dev == NULL) {
		dev_err(dev, "got NULL dev\n");
		goto cleanUp;
	}

	if (!tm) {
		dev_err(dev, "got NULL tm\n");
		goto cleanUp;
	} else {
		dev_info(dev, "set time: %4d-%02d-%02d %02d:%02d:%02d, W:%d\n",
			 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
			 tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_wday);
	}

	if ((rc = VerifyDateTime(dev, tm)) == -1) {
		dev_err(dev, "time is not right!\n");
		goto cleanUp;
	}

	if (RTCHWInit() == -1) {
		dev_err(dev, "rtc init fail\n");
		goto cleanUp;
	}

	RTCProtect(0);
	RTCSecSet(tm->tm_sec);
	RTCMinSet(tm->tm_min);
	RTCHourSet(tm->tm_hour);
	RTCDOMSet(tm->tm_mday);
	RTCMonthSet(tm->tm_mon + 1);

	/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
	RTCDOWSet((tm->tm_wday == 0) ? 7 : tm->tm_mday);
	RTCYearSet(tm->tm_year + 1900 - RTC_YEAR_DATUM);
	RTCProtect(1);

	rc = 0;

cleanUp:
	return rc;
}


/*
 *Function:  ac823x_rtc_setalarm
 *Set the RTC alarm time.
*/
static int ac823x_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	int rc = -1;
	struct rtc_time *tm = &alrm->time;

	if (dev == NULL) {
		dev_err(dev, "got NULL dev\n");
		goto cleanUp;
	}

	if (!tm) {
		dev_err(dev, "got NULL tm\n");
		goto cleanUp;
	} else {
		dev_info(dev, "set alarm: %4d-%02d-%02d %02d:%02d:%02d\n",
			 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
			 tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	if ((rc = VerifyDateTime(dev, tm)) == -1) {
		dev_err(dev, "alarm time is not right! \n");
		goto cleanUp;
	}

	if (RTCHWInit() == -1) {
		dev_err(dev, "rtc init fail\n");
		goto cleanUp;
	}

	/* To avoid the interrupt to affect the rtc time*/
	RTCProtect(0);

	/* 1. Set the alarm time register*/
	RTCAlarmSecSet(tm->tm_sec);
	RTCAlarmMinSet(tm->tm_min);
	RTCAlarmHourSet(tm->tm_hour);
	RTCAlarmDOMSet(tm->tm_mday);
	RTCAlarmMonthSet(tm->tm_mon + 1);

	/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
	RTCAlarmDOWSet((tm->tm_wday == 0) ? 7 : tm->tm_mday);
	RTCAlarmYearSet(tm->tm_year + 1900 - RTC_YEAR_DATUM);

	/* 2. Clear ALARM_HIT*/
	/*RTC_READ32(RTC_IRQ_STA);  //oeminterrupter clear*/

	/* 3. Enable alarm controller*/
	if (alrm->enabled) {
		dev_info(dev, "set alarm: enabled alarm interrupt.\n");
		RTCAlarmAllCMPR(1);
		RTCEnableAlarmInt(1);
		RTCIntAutoReset(1);
	}

	RTCProtect(1);
	rc = 0;

cleanUp:
	return rc;
}

/*
 * Read alarm time and date in RTC
 */
static int ac823x_rtc_readalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	int rc = -1;
	struct rtc_time *tm = &alrm->time;

	if (dev == NULL) {
		dev_err(dev, "got NULL dev\n");
		goto cleanUp;
	}

	if (!tm) {
		dev_err(dev, "got NULL tm\n");
		goto cleanUp;
	}

	if (RTCHWInit() == -1) {
		dev_err(dev, "rtc init fail\n");
		goto cleanUp;
	}

	/* To avoid the interrupt to affect the rtc time*/
	RTCProtect(0);

	/* Set the alarm time register*/
	tm->tm_sec = RTCAlarmSecRead();
	tm->tm_sec = tm->tm_sec & 0x3F;

	tm->tm_min = RTCAlarmMinRead();
	tm->tm_min = tm->tm_min & 0x3F;

	tm->tm_hour = RTCAlarmHourRead();
	tm->tm_hour = tm->tm_hour & 0x1F;

	tm->tm_mday = RTCAlarmDOMRead();
	tm->tm_mday = tm->tm_mday & 0x1F;

	tm->tm_mon = RTCAlarmMonthRead();
	tm->tm_mon = tm->tm_mon & 0x0F;
	tm->tm_mon = tm->tm_mon - 1;

	/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
	tm->tm_wday = RTCAlarmDOWRead();
	tm->tm_wday = tm->tm_wday & 0x07;
	tm->tm_wday = tm->tm_wday % 7;

	tm->tm_year = RTCAlarmYearRead();
	tm->tm_year = tm->tm_year & 0x7F;
	tm->tm_year = tm->tm_year + RTC_YEAR_DATUM - 1900;
	
	if ((rc = VerifyDateTime(dev, tm)) == -1) {
	//if (0) {
		dev_info(dev, "read alarm: got error alarm %4d-%02d-%02d %02d:%02d:%02d\n",
			 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
			 tm->tm_hour, tm->tm_min, tm->tm_sec);
		dev_info(dev, "read alarm: reset alarm time to 2000/1/1 0:0:0 !\n");

		tm->tm_year = 37;
		tm->tm_mon = 0;
		tm->tm_mday = 1;
		tm->tm_hour = 0;
		tm->tm_min = 0;
		tm->tm_sec = 0;
		tm->tm_wday = 6;

		/* 1. Set the alarm time register*/
		RTCAlarmSecSet(tm->tm_sec);
		RTCAlarmMinSet(tm->tm_min);
		RTCAlarmHourSet(tm->tm_hour);
		RTCAlarmDOMSet(tm->tm_mday);
		RTCAlarmMonthSet(tm->tm_mon + 1);
		
		/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
		RTCAlarmDOWSet((tm->tm_wday == 0) ? 7 : tm->tm_mday);
		RTCAlarmYearSet(tm->tm_year);

		tm->tm_year = tm->tm_year + RTC_YEAR_DATUM - 1900;
	}

	tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon, tm->tm_year);
	alrm->enabled = (RTC_READ32(RTC_IRQ_STA) & ALSTA) ? 1 : 0;
	RTCProtect(1);

	dev_info(dev, "read alarm: alrm enable flag [%d] \n", alrm->enabled);

	if (tm != NULL) {
		dev_info(dev, "read alarm: %4d-%02d-%02d %02d:%02d:%02d\n",
			 1900 + tm->tm_year, tm->tm_mon+1, tm->tm_mday,
			 tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	rc = 0;

cleanUp:
	return rc;
}

/*
 * Handle commands from user-space
 */
static int ac823x_rtc_ioctl(struct device *dev, unsigned int cmd,
			    unsigned long arg)
{
	int ret = 0;

	if (dev != NULL) {
		dev_dbg(dev, "cmd=%08x, arg=%08lx.\n", cmd, arg);
	} else {
		pr_err("got NULL dev\n");
		return -1;
	}

	/* important:  scrub old status before enabling IRQs */
	switch (cmd) {
	case RTC_AIE_OFF:	/* alarm off */
		spin_lock_irq(&ac823x_rtc_lock);
		RTCEnableAlarmInt(0);
		spin_unlock_irq(&ac823x_rtc_lock);
		break;

	case RTC_AIE_ON:	/* alarm on */
		spin_lock_irq(&ac823x_rtc_lock);
		RTCEnableAlarmInt(1);
		spin_unlock_irq(&ac823x_rtc_lock);
		break;

	case RTC_UIE_OFF:	/* update off */
		break;

	case RTC_UIE_ON:	/* update on */
		break;

	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}

static const struct rtc_class_ops ac823x_rtc_ops = {
	.ioctl		= ac823x_rtc_ioctl,
	.read_time	= ac823x_rtc_readtime,
	.set_time	= ac823x_rtc_settime,
	.read_alarm	= ac823x_rtc_readalarm,
	.set_alarm	= ac823x_rtc_setalarm,
};

/*
 * IRQ handler for the RTC
 */
static irqreturn_t ac823x_rtc_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct rtc_device *rtc = platform_get_drvdata(pdev);
	struct device *dev = &(pdev->dev);
	unsigned int rtsr;
	unsigned long events = 0;

	if (dev == NULL) {
		pr_err("got NULL dev\n");
		return -1;
	}

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

	return IRQ_NONE;		/* not handled */
}

#if CREATE_RTC_ATTRIBUTE_FILES
static ssize_t ac823x_rtc_sysfs_show_flag(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	if (dev != NULL) {
		dev_dbg(dev, "show output flag [%d]\n", enable_output);
	} else {
		pr_err("got NULL dev\n");
		return -1;
	}

	return sprintf(buf, "%d\n", enable_output);
}

static ssize_t ac823x_rtc_sysfs_store_flag(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		dev_dbg(dev, "store output flag [%s], count %d\n", buf, count);
	} else {
		pr_err("got NULL dev\n");
		return -1;
	}

	ret = kstrtoint(buf, 0, &adj);
	/*ret = sscanf(buf, "%i", &adj);*/

	if (ret != 0) {
		return -EINVAL;
	}

	if (adj == 0) {
		enable_output = 0;
	} else if (adj == 1) {
		enable_output = 1;
	} else {
		enable_output = 1;
	}

	return count;
}

static DEVICE_ATTR(output_flag, S_IRUGO | S_IWUSR,
		   ac823x_rtc_sysfs_show_flag,
		   ac823x_rtc_sysfs_store_flag);

static int ac823x_rtc_sysfs_register(struct device *dev)
{
	return device_create_file(dev, &dev_attr_output_flag);
}

static void ac823x_rtc_sysfs_unregister(struct device *dev)
{
	device_remove_file(dev, &dev_attr_output_flag);
}

static ssize_t ac823x_rtc_sysfs_show_log_time_flag(struct device *dev,
						   struct device_attribute *attr,
						   char *buf)
{
	if (dev != NULL) {
		dev_dbg(dev, "show log time flag [%d]\n", log_time);
	} else {
		pr_err("got NULL dev\n");
		return -1;
	}

	return sprintf(buf, "%d\n", log_time);
}

static ssize_t ac823x_rtc_sysfs_store_log_time_flag(struct device *dev,
						    struct device_attribute *attr,
						    const char *buf, size_t count)
{
	int adj;
	int ret;

	if (dev != NULL) {
		dev_dbg(dev, "store log time flag [%s], count %d\n", buf, count);
	} else {
		pr_err("got NULL dev\n");
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
	if (adj >= 0 && adj <= 10) {
		log_time = adj;
	}
	#endif

	return count;
}

static DEVICE_ATTR(log_time_flag, S_IRUGO | S_IWUSR,
		   ac823x_rtc_sysfs_show_log_time_flag,
		   ac823x_rtc_sysfs_store_log_time_flag);

static int ac823x_rtc_sysfs_logtime_register(struct device *dev)
{
	return device_create_file(dev, &dev_attr_log_time_flag);
}

static void ac823x_rtc_sysfs_logtime_unregister(struct device *dev)
{
	device_remove_file(dev, &dev_attr_log_time_flag);
}
#endif

/*
 * Initialize and install RTC driver
 */
#if 0
struct clk *clk_ac8317_rtc = NULL;
struct clk *clk_ac8317_rtc_parent = NULL;
#endif

extern int thermal_get_temp(void);

static int ac823x_rtc_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct rtc_device *rtc = NULL;
	struct device *dev = &(pdev->dev);
	int ret = -1;
	unsigned int rtc_irq = 0;
	void __iomem *reg_addr = NULL;

	if (dev != NULL) {
		dev_info(dev, "rtc probe: \n");
	} else {
		dev_err(dev, "got NULL dev \n");
		return -1;
	}

#if 0
	/* get rtc clock struct */
	clk_ac8317_rtc = devm_clk_get(&pdev->dev, "rtc-device");

	if (clk_ac8317_rtc == NULL) {
		pr_err("get rtc clk error!\n");
		return -1;
	}

#if 0
	/* select clock*/
	pr_info("======== rtc: got clk, name [%s]\n", clk_ac8317_rtc->name);
	clk_ac8317_rtc_parent = clk_get(NULL, "syspll_d2");

	if (clk_ac8317_rtc_parent != NULL && clk_ac8317_rtc_parent->name != NULL) {
		pr_info("got parent name [%s] 2\n", clk_ac8317_rtc_parent->name);
	}

	ret = clk_set_parent(clk_ac8317_rtc, clk_ac8317_rtc_parent);

	if (clk_ac8317_rtc_parent->name != NULL) {
		pr_info("got parent name [%s] 3\n", clk_ac8317_rtc_parent->name);
	}

	clk_ac8317_rtc_parent = clk_get_parent(clk_ac8317_rtc);
	pr_info("regot parent clk, name [%s] 4\n", clk_ac8317_rtc_parent->name);
#else
	/* on/off clock*/
	/*clk_prepare_enable(clk_ac8317_rtc);*/
	/*clk_disable_unprepare(clk_ac8317_rtc);*/
	pr_info("ac823x rtc probe, clk_set_rate...\n");
	clk_set_rate(clk_ac8317_rtc, 0x02);
#endif
	pr_info("======================================\n");
#endif

	/* get rtc irq */
	rtc_irq = irq_of_parse_and_map(dn, 0);

	if (rtc_irq != 0) {
		//dev_info(dev, "rtc probe: got irq = [%d]\n", rtc_irq);
		printk("rtc probe: got irq = [%d]\n", rtc_irq);

	} else {
		//dev_err(dev, "got irq error !\n");
		printk("got irq error !\n");
		return -1;
	}

	/* get reg */
	reg_addr = of_iomap(dn, 0);

	if (reg_addr != NULL) {
		//dev_info(dev, "rtc probe: got reg base addr = [0x%x]\n", reg_addr);
		printk("rtc probe: got reg base addr = [0x%lx]\n", reg_addr);
	} else {
		//dev_err(dev, "get reg base addr error!\n");
		printk("get reg base addr error!\n");
		return -1;
	}
	
	rtc_reg_base0 = ioremap(0x10000000, 0x100);
	rtc_reg_base1 = ioremap(0x10051000, 0x100);

	printk("rtc probe: rtc_reg_base0 = [0x%lx]\n", rtc_reg_base0);
	printk("rtc probe: rtc_reg_base1 = [0x%lx]\n", rtc_reg_base1);

	ret = RTCHWInit();

	if (ret == -1) {
		dev_err(dev, "rtc HW init error!\n");
	}

	/* Disable all interrupts */
	if (ret == 0) {
		RTCEnableAlarmInt(0);
		RTCEnableTCInt(0);

		/* output 32k */
		ac823x_rtc_output_32k(dev);
	}

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

	dev_info(dev, "rtc probe ok.\n");

#if CREATE_RTC_ATTRIBUTE_FILES
	ret = ac823x_rtc_sysfs_register(&pdev->dev);

	if (ret) {
		dev_err(dev, "creat sys file error!\n");
	}

	/* for output log to sd card */
	ret = ac823x_rtc_sysfs_logtime_register(&pdev->dev);

	if (ret) {
		dev_err(dev, "creat sys logtime file error\n");
	}

	thermal_get_temp();
#endif

	return 0;
}

/*
 * Disable and remove the RTC driver
 */
static int ac823x_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);
	struct device *dev = &(pdev->dev);

	if (dev != NULL) {
		dev_info(dev, "rtc remove\n");
	} else {
		pr_err("got NULL dev\n");
		return -1;
	}

	/* Disable all interrupts */
	RTCEnableAlarmInt(0);
	RTCEnableTCInt(0);

#if CREATE_RTC_ATTRIBUTE_FILES
	/* for output thermal temp */
	ac823x_rtc_sysfs_unregister(&pdev->dev);
	ac823x_rtc_sysfs_logtime_unregister(&pdev->dev);
#endif

	free_irq(VECTOR_RTC, pdev);
	rtc_device_unregister(rtc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int ac823x_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	//dev_dbg(&pdev->dev, "ac823x_rtc_suspend \n");
	pr_debug("ac823x_rtc_suspend \n");
	g_bFirstBooting = TRUE;

	return 0;
}

static int ac823x_rtc_resume(struct platform_device *pdev)
{
	int ret;

	pr_debug("ac823x_rtc_resume \n");
	
	ret = RTCHWInit();
	if (ret != 0)
		dev_err(&pdev->dev, "rtc hw init err! \n");
	
	/* Disable all interrupts */
	if (ret == 0) {
		RTCEnableAlarmInt(0);
		RTCEnableTCInt(0);
		pr_info("ac823x_rtc_resume, output 32k \n");
		/* output 32k */
		ac823x_rtc_output_32k(&pdev->dev);
	}

	return 0;
}

static const struct of_device_id rtc_of_ids[] = {
	{ .compatible = "atc,rtc", },
	{}
};

static struct platform_driver ac823x_rtc_driver = {
	.probe		= ac823x_rtc_probe,
	.remove		= ac823x_rtc_remove,
	.suspend    = ac823x_rtc_suspend,
	.resume		= ac823x_rtc_resume,
	.driver		= {
		.name	= "ac823x_rtc",
		.owner	= THIS_MODULE,
		.of_match_table = rtc_of_ids,
	},
};

static int __init ac823x_rtc_init(void)
{
	return platform_driver_register(&ac823x_rtc_driver);
}

static void __exit ac823x_rtc_exit(void)
{
	platform_driver_unregister(&ac823x_rtc_driver);
}

module_init(ac823x_rtc_init);
module_exit(ac823x_rtc_exit);

MODULE_AUTHOR("ATC Inc.");
MODULE_DESCRIPTION("RTC driver for AC823X");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ac823x_rtc");


/*
* Copyright (c) 2016 AutoChips Inc.
*
* Implementation of the GPS driver.
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

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


/*******************************************************************************
* Dependency
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/version.h>
//#include <mach/hardware.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include "x_ver.h"
#include "oal.h"
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#include <linux/pinctrl/consumer.h>

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[%s] %s: " fmt, KBUILD_MODNAME, __func__

/**
Version Control
**/
#define GPS_MOD_NAME "GPS"
#define GPS_VER_MAJOR  1
#define GPS_VER_MINOR  0
#define GPS_VER_REV    0


struct mtk_gps_hardware{
	int (*ext_power_on)(int);
	int (*ext_power_off)(int);
};
int mt3332_gps_ext_power_on(int state);
int mt3332_gps_ext_power_off(int state);


struct mtk_gps_hardware mt3332_gps_hw = {
    .ext_power_on = mt3332_gps_ext_power_on,
    .ext_power_off = mt3332_gps_ext_power_off,
};
//extern int gpio_direction_output(unsigned gpio, int value);
//extern int gpio_request(unsigned gpio,const char *label);
//extern void gpio_free(unsigned gpio);



#define GPS_CKGEN_VIRT_READ32(REG)            __raw_readl(CKGEN_VIRT + REG)
#define  GPS_CKGEN_VIRT_WRITE32(VAL, REG)      __raw_writel(VAL, CKGEN_VIRT + REG)
#ifdef  GPS_BOARD_POWER_PIN_EXIST
//#define GPS_PIN_PWR  55
#endif

//#define GPS_PIN_STBY 30
//#define GPS_PIN_RST  31


/******************************************************************************
 * Function Configuration
******************************************************************************/
//#define FAKE_DATA
#define GPS_SUSPEND_RESUME
#define GPS_CONFIGURABLE_RESET_DELAY
/******************************************************************************
 * Definition
******************************************************************************/
/* device name and major number */
#define GPS_DEVNAME            "mt3332-gps"
#if 0
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define GPS_DBG_NONE(fmt, arg...)    do {} while (0)
#define GPS_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define GPS_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define GPS_WARN(fmt, arg...)        printk(KERN_WARNING PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define GPS_NOTICE(fmt, arg...)      printk(KERN_NOTICE PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define GPS_INFO(fmt, arg...)        printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define GPS_TRC_FUNC(f)              printk(PFX "<%s>\n", __FUNCTION__);
#define GPS_TRC_VERBOSE(fmt, arg...) printk(PFX fmt, ##arg)

#define PFX "GPS: "
#define GPS_DBG GPS_DBG_FUNC
#define GPS_TRC GPS_DBG_NONE //GPS_TRC_FUNC
#define GPS_VER GPS_DBG_NONE //GPS_TRC_VERBOSE
#define IH_DBG GPS_DBG_NONE
#endif
/*******************************************************************************
* structure & enumeration
*******************************************************************************/
enum {
    GPS_PWRCTL_UNSUPPORTED  = 0xFF,
    GPS_PWRCTL_OFF          = 0x00,
    GPS_PWRCTL_ON           = 0x01,
    GPS_PWRCTL_RST          = 0x02,
    GPS_PWRCTL_OFF_FORCE    = 0x03,
    GPS_PWRCTL_RST_FORCE    = 0x04,
    GPS_PWRCTL_MAX          = 0x05,
};
enum {
    GPS_PWR_UNSUPPORTED     = 0xFF,
    GPS_PWR_RESUME          = 0x00,
    GPS_PWR_SUSPEND         = 0x01,
    GPS_PWR_MAX             = 0x02,
};
enum {
    GPS_STATE_UNSUPPORTED   = 0xFF,
    GPS_STATE_OFF           = 0x00, /*cleanup/power off, default state*/
    GPS_STATE_INIT          = 0x01, /*init*/
    GPS_STATE_START         = 0x02, /*start navigating*/
    GPS_STATE_STOP          = 0x03, /*stop navigating*/
    GPS_STATE_DEC_FREQ      = 0x04,
    GPS_STATE_SLEEP         = 0x05,
    GPS_STATE_MAX           = 0x06,
};
enum {
    GPS_PWRSAVE_UNSUPPORTED = 0xFF,
    GPS_PWRSAVE_DEC_FREQ    = 0x00,
    GPS_PWRSAVE_SLEEP       = 0x01,
    GPS_PWRSAVE_OFF         = 0x02,
    GPS_PWRSAVE_MAX         = 0x03,
};
/*---------------------------------------------------------------------------*/
struct gps_data{
    int  dat_len;
    int  dat_pos;
    char dat_buf[4096];
    spinlock_t lock;
    wait_queue_head_t read_wait;
    struct semaphore sem;
};
/*---------------------------------------------------------------------------*/
struct gps_sta_itm {    /*gps status record*/
    unsigned char year;     /*current year - 1900*/
    unsigned char month;    /*1~12*/
    unsigned char day;      /*1~31*/
    unsigned char hour;     /*0~23*/

    unsigned char minute;      /*0~59*/
    unsigned char sec;      /*0~59*/
    unsigned char count;    /*reborn count*/
    unsigned char reason;   /*reason: 0: timeout; 1: force*/
};
/*---------------------------------------------------------------------------*/
struct gps_sta_obj {
    int index;
    struct gps_sta_itm items[32];
};
/*---------------------------------------------------------------------------*/
struct gps_drv_obj {
    unsigned char pwrctl;
    unsigned char suspend;
    unsigned char state;
    unsigned char pwrsave;
    int rdelay;   /*power reset delay*/
    struct kobject *kobj;
    struct mutex sem;
    struct gps_sta_obj status;
    struct mtk_gps_hardware *hw;
};
/*---------------------------------------------------------------------------*/
struct gps_dev_obj {
    struct class    *cls;
    struct device   *dev;
    dev_t           devno;
    struct cdev     chdev;
    struct mtk_gps_hardware *hw;
};
/******************************************************************************
 * local variables
******************************************************************************/
static struct gps_data gps_private= {0};
struct gpio_desc *resetgpio;
const char *const resetgpiofct[] = {"reset"};

#if defined(FAKE_DATA)
static char fake_data[] = {
"$GPGGA,135036.000,2446.3713,N,12101.3605,E,1,5,1.61,191.1,M,15.1,M,,*51\r\n"
"$GPGSA,A,3,22,18,14,30,31,,,,,,,,1.88,1.61,0.98*09\r\n"
"$GPGSV,2,1,6,18,83,106,32,22,58,324,35,30,45,157,35,14,28,308,32*44\r\n"
"$GPGSV,2,2,6,40,21,254,,31,17,237,29*42\r\n"
"$GPRMC,135036.000,A,2446.37125,N,12101.36054,E,0.243,56.48,140109,,A*46\r\n"
"$GPVTG,56.48,T,,M,0.243,N,0.451,K,A*07\r\n"
};
#endif //FAKE_DATA

/*this should be synchronous with mnld.c
enum {
    MNL_RESTART_NONE            = 0x00, //recording the 1st of mnld
    MNL_RESTART_TIMEOUT_INIT    = 0x01, //restart due to timeout
    MNL_RESTART_TIMEOUT_MONITOR = 0x02, //restart due to timeout
    MNL_RESTART_TIMEOUT_WAKEUP  = 0x03, //restart due to timeout
    MNL_RESTART_TIMEOUT_TTFF    = 0x04, //restart due to TTFF timeout
    MNL_RESTART_FORCE           = 0x04, //restart due to external command
};
*/
/*---------------------------------------------------------------------------*/
static char *str_reason[] = {"none", "init", "monitor", "wakeup", "TTFF", "force", "unknown"};
/******************************************************************************
 * Functions
******************************************************************************/
int mt3332_gps_ext_power_on(int state)
{
   int err = 0;
   if(state == 1)
   {
       //pr_debug("mt3332_gps_ext_power_on enter\n");
       //gpio_direction_output(GPS_PIN_STBY,1);
        err = gpiod_direction_output(resetgpio, 0);
        if (err)
        {
            pr_err("gpio_direction_output fail\n");
        }
        msleep(200);
        err = gpiod_direction_output(resetgpio, 1);
        if (err)
        {
            pr_err("gpio_direction_output fail\n");
        }
       //pr_debug("mt3332_gps_ext_power_on exit\n");
   }

   return err;

}


int mt3332_gps_ext_power_off(int state)
{
   //pr_debug("mt3332_gps_ext_power_off enter\n");
   int err = 0;
   //gpio_direction_output(GPS_PIN_STBY,0);

   //pr_debug("mt3332_gps_ext_power_off exit\n");
   return err;
}

static inline void mt3332_gps_power(struct mtk_gps_hardware *hw,
                                    unsigned int on, unsigned int force)
{
    /*FIX ME: PM_api should provide a function to get current status*/
    static unsigned int power_on = 1;
    int err;

    //pr_debug("Switching GPS device %s\n", on ? "on" : "off");
    if (!hw) {
        pr_err("null pointer!!\n");
        return;
    }

    if (power_on == on) {
        pr_debug("ignore power control: %d\n", on);
    } else if (on) {
		/*power on*/
        if (hw->ext_power_on) {
            err = hw->ext_power_on(0);
            if (err)
                pr_err("ext_power_on fail\n");
        }

        if (hw->ext_power_on) {
            err = hw->ext_power_on(1);
            if (err)
                pr_err("ext_power_on fail\n");
        }

        mdelay(120);
    } else {

        if (hw->ext_power_off) {
                err = hw->ext_power_off(force);
            if (err)
                pr_err("ext_power_off fail\n");
        }
    }
    power_on = on;
}

/*****************************************************************************/
static inline void mt3332_gps_reset(struct mtk_gps_hardware *hw, int delay, int force)
{
    mt3332_gps_power(hw, 1, FALSE);
    mdelay(delay);
    mt3332_gps_power(hw, 0, force);
    mdelay(delay);
    mt3332_gps_power(hw, 1, FALSE);
}

/******************************************************************************/

static inline int mt3332_gps_set_suspend(struct gps_drv_obj* obj,
                                          unsigned char suspend)
{
    if (!obj)
        return -1;
    mutex_lock(&obj->sem);
    if (obj->suspend != suspend) {
        pr_debug("issue sysfs_notify : %p\n", obj->kobj->sd);
        sysfs_notify(obj->kobj, NULL, "suspend");
    }
    obj->suspend = suspend;
    mutex_unlock(&obj->sem);
    return 0;
}
/******************************************************************************/
static inline int mt3332_gps_set_pwrctl(struct gps_drv_obj* obj,
                                         unsigned char pwrctl)
{
    int err = 0;

    if (!obj)
        return -1;
    mutex_lock(&obj->sem);

    if ((pwrctl == GPS_PWRCTL_ON) || (pwrctl == GPS_PWRCTL_OFF)) {
        obj->pwrctl = pwrctl;
        mt3332_gps_power(obj->hw, pwrctl, FALSE);
    } else if (pwrctl == GPS_PWRCTL_OFF_FORCE) {
        obj->pwrctl = pwrctl;
        mt3332_gps_power(obj->hw, pwrctl, TRUE);
    } else if (pwrctl == GPS_PWRCTL_RST) {
        mt3332_gps_reset(obj->hw, obj->rdelay, FALSE);
        obj->pwrctl = GPS_PWRCTL_ON;
    } else if (pwrctl == GPS_PWRCTL_RST_FORCE) {
        mt3332_gps_reset(obj->hw, obj->rdelay, TRUE);
        obj->pwrctl = GPS_PWRCTL_ON;
    } else {
        err = -1;
    }
    mutex_unlock(&obj->sem);
    return err;
}
 /******************************************************************************/
static inline int mt3332_gps_set_status(struct gps_drv_obj* obj,
                                        const char* buf, size_t count)
{
    int err = 0;
    int year, mon, day, hour, minute, sec, cnt, reason, idx;

    if (!obj)
        return -1;

    mutex_lock(&obj->sem);
    if (sscanf(buf, "(%d/%d/%d %d:%d:%d) - %d/%d", &year, &mon, &day,
               &hour, &minute, &sec, &cnt, &reason) == 8) {
        int number = (int)(sizeof(obj->status.items)/sizeof(obj->status.items[0]));
        idx = obj->status.index % number;
        obj->status.items[idx].year  = (unsigned char)year;
        obj->status.items[idx].month = (unsigned char)mon;
        obj->status.items[idx].day   = (unsigned char)day;
        obj->status.items[idx].hour  = (unsigned char)hour;
        obj->status.items[idx].minute= (unsigned char)minute;
        obj->status.items[idx].sec   = (unsigned char)sec;
        obj->status.items[idx].count = (unsigned char)cnt;
        obj->status.items[idx].reason= (unsigned char)reason;
        obj->status.index ++;
    } else {
        err = -1;
    }
    mutex_unlock(&obj->sem);
    return err;
}
/******************************************************************************/
static inline int mt3332_gps_set_state(struct gps_drv_obj* obj,
                                       unsigned char state)
{
    int err = 0;

    if (!obj)
        return -1;
    mutex_lock(&obj->sem);
    if (state < GPS_STATE_MAX)
        obj->state = state;
    else
        err = -1;
    mutex_unlock(&obj->sem);
    return err;
}
/******************************************************************************/
static inline int mt3332_gps_set_pwrsave(struct gps_drv_obj* obj,
                                         unsigned char pwrsave)
{
    int err = 0;

    if (!obj)
        return -1;
    mutex_lock(&obj->sem);
    if (pwrsave < GPS_PWRSAVE_MAX)
        obj->pwrsave = pwrsave;
    else
        err = -1;
    mutex_unlock(&obj->sem);
    return err;
}
/******************************************************************************/
static inline int mt3332_gps_dev_suspend(struct gps_drv_obj *obj)
{
#if defined(GPS_SUSPEND_RESUME)
    int err;

    if ((err = mt3332_gps_set_suspend(obj, GPS_PWR_SUSPEND)))
        pr_err("set suspend fail: %d\n", err);
    if ((err = mt3332_gps_set_pwrctl(obj, GPS_PWRCTL_OFF)))
        pr_err("set pwrctl fail: %d\n", err);
    return err;
#endif
}
/******************************************************************************/
static inline int mt3332_gps_dev_resume(struct gps_drv_obj *obj)
{
#if defined(GPS_SUSPEND_RESUME)
    int err;

    if ((err = mt3332_gps_set_suspend(obj, GPS_PWR_RESUME)))
        pr_err("set suspend fail: %d\n", err);
    /*don't power on device automatically*/
    return err;
#endif
}
/******************************************************************************/
static ssize_t mt3332_show_pwrctl(struct device* dev,
                                 struct device_attribute *attr, char *buf)
{
    struct gps_drv_obj *obj;
    ssize_t res;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    res = snprintf(buf, PAGE_SIZE, "%d\n", obj->pwrctl);
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_pwrctl(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    if ((count == 1) ||
        ((count == 2) && (buf[1] == '\n'))) {
        unsigned char pwrctl = buf[0] - '0';
        if (!mt3332_gps_set_pwrctl(obj, pwrctl))
            return count;
     }
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
static ssize_t mt3332_show_suspend(struct device* dev,
                                   struct device_attribute *attr, char *buf)
{
    struct gps_drv_obj *obj;
    ssize_t res;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    res = snprintf(buf, PAGE_SIZE, "%d\n", obj->suspend);
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_suspend(struct device* dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    if ((count == 1) ||
        ((count == 2) && (buf[1] == '\n'))) {
        unsigned char suspend = buf[0] - '0';
        if (suspend == GPS_PWR_SUSPEND) {
            if (!mt3332_gps_dev_suspend(obj))
                return count;
        } else if (suspend == GPS_PWR_RESUME) {
            if (!mt3332_gps_dev_resume(obj))
                return count;
        }
    }
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
static ssize_t mt3332_show_status(struct device* dev,
                                  struct device_attribute *attr, char *buf)
{
    int res, idx, num, left, cnt, len;
    struct gps_drv_obj *obj;
    char *reason = NULL;
    int reason_max = (int)(sizeof(str_reason)/sizeof(str_reason[0]));

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    num = (int)(sizeof(obj->status.items)/sizeof(obj->status.items[0]));
    left = PAGE_SIZE;
    cnt = 0;
    len = 0;
    for (idx = 0; idx < num; idx++) {
        if (obj->status.items[idx].month == 0)
            continue;
        if (obj->status.items[idx].reason >= reason_max)
            reason = str_reason[reason_max-1];
        else
            reason = str_reason[obj->status.items[idx].reason];
        cnt = snprintf(buf+len, left, "[%d] %.4d/%.2d/%.2d %.2d:%.2d:%.2d - %d, %s\n", idx,
              obj->status.items[idx].year + 1900, obj->status.items[idx].month,
              obj->status.items[idx].day, obj->status.items[idx].hour,
              obj->status.items[idx].minute, obj->status.items[idx].sec,
              obj->status.items[idx].count, reason);
        left -= cnt;
        len += cnt;
    }
    res = PAGE_SIZE - left;
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_status(struct device* dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    if (!mt3332_gps_set_status(obj, buf, count))
        return count;
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
static ssize_t mt3332_show_state(struct device* dev,
                                 struct device_attribute *attr, char *buf)
{
    ssize_t res;
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    res = snprintf(buf, PAGE_SIZE, "%d\n", obj->state);
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_state(struct device* dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    if ((count == 1) ||
        ((count == 2) && (buf[1] == '\n'))) {   /*To Do: dynamic change according to input*/
        unsigned char state = buf[0] - '0';
        if (!mt3332_gps_set_state(obj, state))
            return count;
    }
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
static ssize_t mt3332_show_pwrsave(struct device* dev,
                                   struct device_attribute *attr, char *buf)
{
    ssize_t res;
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    res = snprintf(buf, PAGE_SIZE, "%d\n", obj->pwrsave);
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_pwrsave(struct device* dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    if ((count == 1) ||
        ((count == 2) && (buf[1] == '\n'))) {
        unsigned char pwrsave = buf[0] - '0';
        if (!mt3332_gps_set_pwrsave(obj, pwrsave))
            return count;
    }
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
#if defined(GPS_CONFIGURABLE_RESET_DELAY)
/******************************************************************************/
static ssize_t mt3332_show_rdelay(struct device* dev,
                                   struct device_attribute *attr, char *buf)
{
    ssize_t res;
    struct gps_drv_obj *obj;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    mutex_lock(&obj->sem);
    res = snprintf(buf, PAGE_SIZE, "%d\n", obj->rdelay);
    mutex_unlock(&obj->sem);
    return res;
}
/******************************************************************************/
static ssize_t mt3332_store_rdelay(struct device* dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    struct gps_drv_obj *obj;
    int rdelay;
    char *end;

    if (!dev) {
        pr_err("dev is null!!\n");
        return 0;
    } else if (!(obj = (struct gps_drv_obj*)dev_get_drvdata(dev))) {
        pr_err("drv data is null!!\n");
        return 0;
    }
    end = (char*)buf+count;
    rdelay = (int)simple_strtol(buf, &end, 10);
    if (rdelay < 2000) {
        mutex_lock(&obj->sem);
        obj->rdelay = rdelay;
        mutex_unlock(&obj->sem);
        return count;
    }
    pr_debug("invalid content: '%s', length = %d\n", buf, count);
    return count;
}
/******************************************************************************/
#endif
/******************************************************************************/
DEVICE_ATTR(pwrctl,     S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_pwrctl,  mt3332_store_pwrctl);
DEVICE_ATTR(suspend,    S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_suspend,mt3332_store_suspend);
DEVICE_ATTR(status,     S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_status, mt3332_store_status);
DEVICE_ATTR(state,      S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_state,  mt3332_store_state);
DEVICE_ATTR(pwrsave,    S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_pwrsave,mt3332_store_pwrsave);
#if defined(GPS_CONFIGURABLE_RESET_DELAY)
DEVICE_ATTR(rdelay,     S_IWUSR | S_IWGRP | S_IRUGO, mt3332_show_rdelay ,mt3332_store_rdelay);
#endif
static struct device_attribute *gps_attr_list[] = {
    &dev_attr_pwrctl,
    &dev_attr_suspend,
    &dev_attr_status,
    &dev_attr_state,
    &dev_attr_pwrsave,
#if defined(GPS_CONFIGURABLE_RESET_DELAY)
    &dev_attr_rdelay,
#endif
};
/******************************************************************************/
static int mt3332_gps_create_attr(struct device *dev)
{
    int idx, err = 0;
    int num = (int)(sizeof(gps_attr_list)/sizeof(gps_attr_list[0]));

    if (!dev)
        return -EINVAL;

    pr_debug("\r\n");
    for (idx = 0; idx < num; idx++) {
        if ((err = device_create_file(dev, gps_attr_list[idx]))) {
            pr_err("device_create_file (%s) = %d\n", gps_attr_list[idx]->attr.name, err);
            break;
        }
    }

    return err;
}
/******************************************************************************/
static int mt3332_gps_delete_attr(struct device *dev)
{
    int idx ,err = 0;
    int num = (int)(sizeof(gps_attr_list)/sizeof(gps_attr_list[0]));

    if (!dev)
        return -EINVAL;

    for (idx = 0; idx < num; idx++)
        device_remove_file(dev, gps_attr_list[idx]);

    return err;
}
/******************************************************************************/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int mt3332_gps_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    //pr_debug("mt3332_gps_ioctl!!\n");
    return -ENOIOCTLCMD;
}
#else
long mt3332_gps_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //pr_debug("mt3332_gps_unlocked_ioctl!!\n");
    return -ENOIOCTLCMD;
}
#endif
/*****************************************************************************/
static int mt3332_gps_open(struct inode *inode, struct file *file)
{
	//pr_debug("[3332&33336][kernel]mt3332_gps_open\n\r");
    file->private_data = &gps_private;  //all files share the same buffer
    return nonseekable_open(inode, file);
}
/*****************************************************************************/
static int mt3332_gps_release(struct inode *inode, struct file *file)
{
    struct gps_data *dev = file->private_data;

    //pr_debug("\r\n");

    if (dev)
        file->private_data = NULL;

    return 0;
}
/******************************************************************************/
static ssize_t mt3332_gps_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct gps_data *dev = file->private_data;
    ssize_t ret = 0;
    int copy_len = 0;

    //pr_debug("\r\n");

    if (!dev)
        return -EINVAL;

    if (signal_pending(current))
        return -ERESTARTSYS;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (dev->dat_len == 0){ /*no data to be read*/
        up(&dev->sem);
        if (file->f_flags & O_NONBLOCK) /*non-block mode*/
            return -EAGAIN;
        do {/*block mode*/
            ret = wait_event_interruptible(dev->read_wait, (dev->dat_len > 0));
            if (ret == -ERESTARTSYS)
                return -ERESTARTSYS;
        }while(ret == 0);
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }

    /*data is available*/
    copy_len = (dev->dat_len < (int)count) ? (dev->dat_len) : (int)(count);
    if (copy_to_user(buf, dev->dat_buf+dev->dat_pos, (unsigned long)copy_len)){
        pr_err("copy_to_user error: 0x%X 0x%X, %d\n", (unsigned int)buf, (unsigned int)dev->dat_buf, dev->dat_len);
        ret = -EFAULT;
    } else {
        //pr_debug("mt3332_gps_read(%d,%d,%d) = %d\n", count, dev->dat_pos, dev->dat_len, copy_len);
        if (dev->dat_len > (copy_len+dev->dat_pos)) {
            dev->dat_pos += copy_len;
        } else {
            dev->dat_len = 0;
            dev->dat_pos = 0;
        }
        ret = copy_len;
    }

    up(&dev->sem);
    //pr_debug("%s return %d bytes\n", __FUNCTION__, ret);
    return ret;
}
/******************************************************************************/
static ssize_t mt3332_gps_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct gps_data *dev = file->private_data;
    ssize_t ret = 0;

    //pr_debug("\r\n");

    if (!dev)
        return -EINVAL;

    if (!count)     /*no data written*/
        return 0;

    if (signal_pending(current))
        return -ERESTARTSYS;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (copy_from_user(dev->dat_buf, buf, count)) {
        pr_err("copy_from_user error");
        ret = -EFAULT;
    } else {
        dev->dat_len = count;
        dev->dat_pos = 0;
        ret = count;
    }
    up(&dev->sem);
    wake_up_interruptible(&dev->read_wait);
    //pr_debug("%s: write %d bytes\n", __FUNCTION__, dev->dat_len);
    return ret;
}
/******************************************************************************/
static unsigned int mt3332_gps_poll(struct file *file, poll_table *wait)
{
    struct gps_data *dev = file->private_data;
    unsigned int mask = 0;

    //pr_debug("\r\n");

    if (!dev)
        return 0;

    down(&dev->sem);
    poll_wait(file, &dev->read_wait, wait);
    if (dev->dat_len != 0) /*readable if data is available*/
        mask = (POLLIN|POLLRDNORM) | (POLLOUT|POLLWRNORM);
    else /*always writable*/
        mask = (POLLOUT|POLLWRNORM);
    up(&dev->sem);
    //pr_debug("%s: mask : 0x%X\n", __FUNCTION__, mask);
    return mask;
}
/*****************************************************************************/
/* Kernel interface */
static struct file_operations mt3332_gps_fops = {
    .owner      = THIS_MODULE,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
    .ioctl      = mt3332_gps_ioctl,
#else
    .unlocked_ioctl      = mt3332_gps_unlocked_ioctl,
#endif
    .open       = mt3332_gps_open,
    .read       = mt3332_gps_read,
    .write      = mt3332_gps_write,
    .release    = mt3332_gps_release,
    .poll       = mt3332_gps_poll,
};
/*****************************************************************************/
extern unsigned int reset_state;
/*****************************************************************************/
static void mt3332_gps_hw_init(struct mtk_gps_hardware *hw)
{
    mt3332_gps_power(hw, 1, FALSE);
}
/*****************************************************************************/
static void mt3332_gps_hw_exit(struct mtk_gps_hardware *hw)
{
    mt3332_gps_power(hw, 0, FALSE);
}
/*****************************************************************************/
static int mt3332_gps_probe(struct platform_device *dev)
{
    int ret = 0, err = 0;
    struct gps_drv_obj *drvobj = NULL;
    //struct mtk_gps_hardware *hw = (struct mtk_gps_hardware*)dev->dev.platform_data;
    struct gps_dev_obj *devobj = NULL;

    if (!(devobj = kzalloc(sizeof(*devobj), GFP_KERNEL)))
    {
        pr_err("-ENOMEM\n");
        err = -ENOMEM;
        goto error;
    }

    resetgpio = __gpiod_get(&(dev->dev), resetgpiofct[0], GPIOD_ASIS);
    if (IS_ERR(resetgpio))
    {
        pr_err("__gpiod_get fail\n");
    }

    mt3332_gps_hw_init(&mt3332_gps_hw);

    pr_info("Registering chardev\n");
    ret = alloc_chrdev_region(&devobj->devno, 0, 1, GPS_DEVNAME);
    if (ret) {
        pr_err("alloc_chrdev_region fail: %d\n", ret);
        goto error;
    } else {
        pr_debug("major: %d, minor: %d\n", MAJOR(devobj->devno), MINOR(devobj->devno));
    }
    cdev_init(&devobj->chdev, &mt3332_gps_fops);
    devobj->chdev.owner = THIS_MODULE;
    err = cdev_add(&devobj->chdev, devobj->devno, 1);
    if (err) {
        pr_err("cdev_add fail: %d\n", err);
        goto error;
    }

    if (!(drvobj = kmalloc(sizeof(*drvobj), GFP_KERNEL))) {
        err = -ENOMEM;
        goto error;
    }
    memset(drvobj, 0 ,sizeof(*drvobj));

    devobj->cls = class_create(THIS_MODULE, "gpsdrv");
    if (IS_ERR(devobj->cls)) {
        pr_err("Unable to create class, err = %d\n", (int)PTR_ERR(devobj->cls));
        goto error;
    }
    devobj->dev = device_create(devobj->cls, NULL, devobj->devno, drvobj, "gps");
    drvobj->hw      = &mt3332_gps_hw;
    drvobj->pwrctl  = 0;
    drvobj->suspend = 0;
    drvobj->state   = GPS_STATE_UNSUPPORTED;
    drvobj->pwrsave = GPS_PWRSAVE_UNSUPPORTED;
    drvobj->rdelay  = 50;
    drvobj->kobj    = &devobj->dev->kobj;
    mutex_init(&drvobj->sem);

    if ((err = mt3332_gps_create_attr(devobj->dev)))
        goto error;

    /*initialize members*/
    spin_lock_init(&gps_private.lock);
    init_waitqueue_head(&gps_private.read_wait);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
    init_MUTEX(&gps_private.sem);
#else
    sema_init(&gps_private.sem, 1);
#endif
    gps_private.dat_len = 0;
    gps_private.dat_pos = 0;
    memset(gps_private.dat_buf, 0x00, sizeof(gps_private.dat_buf));

    /*set platform data:
      a new device created for gps */
    platform_set_drvdata(dev, devobj);

    //pr_debug("Done\n");
    return 0;

error:
    if (err == 0)
        cdev_del(&devobj->chdev);
    if (ret == 0)
        unregister_chrdev_region(devobj->devno, 1);
    return -1;
}
/*****************************************************************************/
static int mt3332_gps_remove(struct platform_device *dev)
{
    struct gps_dev_obj *devobj = (struct gps_dev_obj*)platform_get_drvdata(dev);
    struct gps_drv_obj *drvobj = (struct gps_drv_obj*)dev_get_drvdata(devobj->dev);
    int err;

    if (!devobj || !drvobj) {
        pr_err("null pointer: %p, %p\n", devobj, drvobj);
        return -1;
    }

    //pr_debug("Unregistering chardev\n");
    kfree(devobj);

    cdev_del(&devobj->chdev);
    unregister_chrdev_region(devobj->devno, 1);

    mt3332_gps_hw_exit(devobj->hw);
    if ((err = mt3332_gps_delete_attr(devobj->dev)))
        pr_err("delete attr fails: %d\n", err);
    device_destroy(devobj->cls, devobj->devno);
    class_destroy(devobj->cls);
    //pr_debug("Done\n");
    return 0;
}
/*****************************************************************************/
static void mt3332_gps_shutdown(struct platform_device *dev)
{
    struct gps_dev_obj *devobj = (struct gps_dev_obj*)platform_get_drvdata(dev);
    //pr_debug("Shutting down\n");
    mt3332_gps_hw_exit(devobj->hw);
}
/*****************************************************************************/
#ifdef CONFIG_PM
/*****************************************************************************/
static int mt3332_gps_suspend(struct platform_device *dev, pm_message_t state)
{
    int err = 0;
    struct gps_dev_obj *devobj = (struct gps_dev_obj*)platform_get_drvdata(dev);
    struct gps_drv_obj *drvobj = (struct gps_drv_obj*)dev_get_drvdata(devobj->dev);

    if (!devobj || !drvobj) {
        pr_err("null pointer: %p, %p\n", devobj, drvobj);
        return -1;
    }

    pr_debug("dev = %p, event = %u,", dev, state.event);
    if (state.event == PM_EVENT_SUSPEND) {
        err = mt3332_gps_dev_suspend(drvobj);
    }
    return err;
}
/*****************************************************************************/
static int mt3332_gps_resume(struct platform_device *dev)
{
    struct gps_dev_obj *devobj = (struct gps_dev_obj*)platform_get_drvdata(dev);
    struct gps_drv_obj *drvobj = (struct gps_drv_obj*)dev_get_drvdata(devobj->dev);

    //pr_debug("");
    return mt3332_gps_dev_resume(drvobj);
}
/*****************************************************************************/
#endif /* CONFIG_PM */
/*****************************************************************************/
static const struct of_device_id ac83xx_gps_of_match[] = {
	{
		.compatible = "Autochips,ac83xx-gps",
	},
	{ },
};

static struct platform_driver mt3332_gps_driver =
{
    .probe      = mt3332_gps_probe,
    .remove     = mt3332_gps_remove,
    .shutdown   = mt3332_gps_shutdown,
#if defined(CONFIG_PM)
    .suspend    = mt3332_gps_suspend,
    .resume     = mt3332_gps_resume,
#endif
    .driver     = {
        .name = GPS_DEVNAME,
        .of_match_table = ac83xx_gps_of_match,
//        .bus    = &platform_bus_type,
    },
};

/*struct platform_device mt3332_device_gps = {
        .name          = "mt3332-gps",
        .id            = -1,
        .dev = {
        .platform_data = &mt3332_gps_hw,
    },
};*/


/*****************************************************************************/
static void __init mtk_gps_register(void)
{
	int ret = 0;

        ret = platform_driver_register(&mt3332_gps_driver);
        if (ret)
		pr_err("mtk: failed to register gps driver: %d\n", ret);

	//ret = platform_device_register(&mt3332_device_gps);
	//if (ret)
	//	pr_err("mtk: failed to register gps device: %d\n", ret);
}


/*****************************************************************************/
void mtk_gps_register_test(void)
{
	mtk_gps_register();
}
/*****************************************************************************/
static int __init mt3332_gps_mod_init(void)
{
    int ret = 0;
	//int iRegTemp = 0;

	pr_info("[3332&33336][kernel]mt3332_gps_mod_init\n\r");
#ifdef  GPS_BOARD_POWER_PIN_EXIST
//	gpio_direction_output(GPS_PIN_PWR,1);
#endif
    //ret = driver_register(&mt3326_gps_driver);
    //ret = os_device_register(&mt3332_device_gps);
    //ret = platform_device_register(&mt3332_device_gps);
	//if(ret != 0)
	{
	   // pr_debug("mt3326_device_gps register failure \n\r");
	}
	mtk_gps_register();
    //ret = platform_driver_register(&mt3332_gps_driver);
    //ret = os_driver_register(&mt3332_gps_driver);

	//pr_debug("gps_mod_init set gpio\n\r");

    /*generate  32kHz clock  from GPIO74 to gps  */
	//iRegTemp = GPS_CKGEN_VIRT_READ32(0x64);
    //iRegTemp |= 0x400;
    //GPS_CKGEN_VIRT_WRITE32(iRegTemp, 0x64);

    MOD_VERSION_INFO(GPS_MOD_NAME,GPS_VER_MAJOR,GPS_VER_MINOR,GPS_VER_REV);
    return ret;
}

/*****************************************************************************/
static void __exit mt3332_gps_mod_exit(void)
{
    gpio_free(GPS_PIN_RST);
    platform_driver_unregister(&mt3332_gps_driver);
    //platform_device_unregister(&mt3332_device_gps);
}
/*****************************************************************************/
module_init(mt3332_gps_mod_init);
module_exit(mt3332_gps_mod_exit);
/*****************************************************************************/
MODULE_DESCRIPTION("MT3332 GPS Driver");
MODULE_LICENSE("GPL");




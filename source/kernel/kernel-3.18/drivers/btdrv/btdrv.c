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
#include <mach/hardware.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#include <linux/gpio/consumer.h>
#include <linux/wakelock.h>
//#include "x_typedef.h"
//#include "windev.h"
#include "oal.h"
#include "btdrv.h"

#include "x_ver.h" //add_version_info

#define BTDRV_MODE_NAME                   "BTDRV"
#define BTDRV_VER_MAJOR                   01
#define BTDRV_VER_MINOR                   00
#define BTDRV_VER_REV                     00


#define BTDRV_DEBUG   1

#define BT_DEVNAME "btdrv"
#define DBG_TAG "BTDRV"


#define ATC_KERNEL_LINUX_LICENSE          "GPL"

#define PREFX "BTDRV: "

#define BTDRV_DBG(fmt, arg...)       printk(KERN_INFO PREFX "%s: " fmt, __FUNCTION__ ,##arg)
#define BTDRV_ERR(fmt, arg...)       printk(KERN_ERR PREFX "%s: " fmt, __FUNCTION__ ,##arg)

int bt_ext_power_on(int state);
int bt_ext_power_off(int state);

extern int gpio_configure(unsigned gpio, int dir, int value);
extern int gpio_request(unsigned gpio, const char *label);
extern void gpio_free(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);


int number_of_devices = 1;
struct cdev btdrv_dev;
dev_t dev = 0;
struct class *btdrv_class;

struct bt_data{
    int  power_state;
    unsigned int capture[4];
    unsigned char mac_id[6];

    spinlock_t lock;
    wait_queue_head_t read_wait;
    struct semaphore sem;
    struct fasync_struct *async_queue;
};


static struct bt_data bt_private= {0};

static struct wake_lock wakelock;
static bool is_wakelock = false;

struct bt_hardware {
    int (*ext_power_on)(int);
    int (*ext_power_off)(int);
};

struct bt_hardware bt_hw = {
    .ext_power_on = bt_ext_power_on ,
    .ext_power_off = bt_ext_power_off ,
};


struct platform_device device_bt = {

    .name = "btdrv",
    .id   = -1 ,
    .dev ={
         .platform_data = &bt_hw ,
        },
};

enum {
    BT_PWRSAVE_UNSUPPORTED = 0xFF,
    BT_PWRSAVE_DEC_FREQ    = 0x00,
    BT_PWRSAVE_SLEEP       = 0x01,
    BT_PWRSAVE_OFF         = 0x02,
    BT_PWRSAVE_MAX         = 0x03,
};


/*---------------------------------------------------------------------------*/
struct bt_drv_obj {
    unsigned char pwrctl;
    unsigned char suspend;
    unsigned char state;
    unsigned char pwrsave;
    int rdelay;   /*power reset delay*/
    struct kobject *kobj;
    struct mutex sem;
   // struct bt_sta_obj status;
    struct bt_hardware *hw;
};
/*---------------------------------------------------------------------------*/
struct bt_dev_obj {
    struct class    *cls;
    struct device   *dev;
    dev_t           devno;
    struct cdev     chdev;
    struct bt_hardware *hw;
};

struct _bt_gpio {
    struct gpio_desc *desc;
    const char *name;
};

enum {
    PWN_PIN,
    RST_PIN,
    PIN_NUM
};
struct _bt_gpio bt_gpio[PIN_NUM] = {
    {NULL, "pwn"},
    {NULL, "rst"}
};


static int bt_wake_lock_ctrl(bool lock)
{
    int ret = 0;
    if (is_wakelock == lock) {
        return ret;
    }

    if (lock) {
        wake_lock(&wakelock);
    } else {
        wake_unlock(&wakelock);
    }
    BTDRV_DBG("lock(%s).\n", (lock?"true":"false"));
    is_wakelock = lock;
    return ret;
}

int bt_ext_power_on(int force)
{
    int err = 0;

    if ((force || bt_private.power_state == 0)) {
        bt_wake_lock_ctrl(true);

        err = gpiod_direction_output(bt_gpio[PWN_PIN].desc, 1);

        //6622 data sheet ask for that delay time from PWN high to RST high at least 10ms
        msleep(50);

        err = gpiod_direction_output(bt_gpio[RST_PIN].desc, 1);

        bt_private.power_state = (err == 0 ? 1 : 0);

        //empirical value
        msleep(500);
    }

    return err;

}


int bt_ext_power_off(int force)
{
    int err = 0;

    if (force || bt_private.power_state == 1) {

        err = gpiod_direction_output(bt_gpio[PWN_PIN].desc, 0);

        err = gpiod_direction_output(bt_gpio[RST_PIN].desc, 0);

        bt_private.power_state = (err == 0 ? 0 : 1);

        bt_wake_lock_ctrl(false);
    }

    return err;
}


static int btdrv_open(struct inode *inode, struct file *file)
{
#if BTDRV_DEBUG
    printk(KERN_INFO "btdrv open start");
#endif

    file->private_data = &bt_private;

    return 0;
}

static long btdrv_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
#if BTDRV_DEBUG
    printk(KERN_INFO "btdrv ioctl: command = %u,param = %x \n", cmd,(int)arg);
#endif

    if (BTDRV_IOCTL_GET_CFG == cmd ||
        BTDRV_IOCTL_SET_CFG == cmd ||
        BTDRV_IOCTL_GET_POWER == cmd)
    {

    }

    switch(cmd)
    {
    case BTDRV_IOCTL_POWER_OFF:
        bt_ext_power_off(1);
        break;

    case BTDRV_IOCTL_POWER_ON:
        bt_ext_power_on(1);
        break;

    case BTDRV_IOCTL_GET_CFG:

        break;

    case BTDRV_IOCTL_SET_CFG:

        break;

    case BTDRV_IOCTL_GET_POWER:

        break;

    default:
        return - EINVAL;
    }



    return 0;
}

/*
 * fasync file op
 */

static int btdrv_fasync(int fd, struct file *file, int mode)
{
    struct bt_data *bt_dev = file->private_data;

    return fasync_helper(fd, file, mode, &bt_dev->async_queue);
}

static inline void bt_power(struct bt_hardware *hw,
                                unsigned int on, unsigned int force)
{
    int err;
    BTDRV_DBG("Switching BT device %s\n", on ? "on" : "off");
    if (!hw) {
        BTDRV_ERR("null pointer!!\n");
        return;
    }

    switch (on) {
    case 0:
        /*power off*/
        if (hw->ext_power_off) {
            err = hw->ext_power_off(force);
            if (err)
                BTDRV_ERR("ext_power_off fail\n");
        }
        break;

    case 1:
        /*power on*/
        if (hw->ext_power_off && hw->ext_power_on) {
            err = hw->ext_power_off(force);
            msleep(200);
            err = hw->ext_power_on(force);
            if (err)
                BTDRV_ERR("ext_power_on fail\n");
        }
        break;
    }
}



/*****************************************************************************/

static void bt_hw_init(struct bt_hardware *hw)
{
    //init gpio
    if (NULL == bt_gpio[PWN_PIN].desc)
    {
       bt_gpio[PWN_PIN].desc = __gpiod_get(&(device_bt.dev), bt_gpio[PWN_PIN].name, GPIOD_ASIS);
    }
    if (NULL == bt_gpio[RST_PIN].desc)
    {
       bt_gpio[RST_PIN].desc = __gpiod_get(&(device_bt.dev), bt_gpio[RST_PIN].name, GPIOD_ASIS);
    }

    wake_lock_init(&wakelock, WAKE_LOCK_SUSPEND, "btdrvFuncCtrl");

    bt_power(hw, 0, true);
}
/*****************************************************************************/
static void bt_hw_exit(struct bt_hardware *hw)
{
    bt_power(hw, 0, true);

    if (NULL != bt_gpio[PWN_PIN].desc)
    {
        gpiod_put(bt_gpio[PWN_PIN].desc);
    }
    if (NULL != bt_gpio[RST_PIN].desc)
    {
        gpiod_put(bt_gpio[RST_PIN].desc);
    }

    wake_lock_destroy(&wakelock);
}

/*****************************************************************************/



static const struct file_operations btdrv_fops =
{
    .owner = THIS_MODULE,
    .open = btdrv_open,
    .unlocked_ioctl = btdrv_ioctl,
    .fasync =   btdrv_fasync,
};


static int btdrv_probe(struct platform_device *dev)
{
    int ret = 0, err = 0;
    struct bt_drv_obj *drvobj = NULL;
    //struct bt_hardware *hw = (struct bt_hardware*)dev->dev.platform_data;
    struct bt_dev_obj *devobj = NULL;

    dev->dev.platform_data = &bt_hw;
    memcpy(&device_bt, dev, sizeof(struct platform_device));

    if (!(devobj = kzalloc(sizeof(*devobj), GFP_KERNEL)))
    {
        BTDRV_ERR("-ENOMEM\n");
        err = -ENOMEM;
        goto error;
    }

    bt_hw_init(&bt_hw);

    BTDRV_DBG("Registering chardev\n");
    ret = alloc_chrdev_region(&devobj->devno, 0, 1, BT_DEVNAME);
    if (ret) {
        BTDRV_ERR("alloc_chrdev_region fail: %d\n", ret);
        goto error;
    } else {
        BTDRV_DBG("major: %d, minor: %d\n", MAJOR(devobj->devno), MINOR(devobj->devno));
    }
    cdev_init(&devobj->chdev, &btdrv_fops);
    devobj->chdev.owner = THIS_MODULE;
    err = cdev_add(&devobj->chdev, devobj->devno, 1);
    if (err) {
        BTDRV_ERR("cdev_add fail: %d\n", err);
        goto error;
    }

    if (!(drvobj = kmalloc(sizeof(*drvobj), GFP_KERNEL))) {
        err = -ENOMEM;
        goto error;
    }
    memset(drvobj, 0 ,sizeof(*drvobj));

    devobj->cls = os_class_create(THIS_MODULE, "btdrv_class");
    if (IS_ERR(devobj->cls)) {
        BTDRV_ERR("Unable to create class, err = %d\n", (int)PTR_ERR(devobj->cls));
        goto error;
    }
    devobj->dev = os_device_create(devobj->cls, NULL, devobj->devno, drvobj, BT_DEVNAME);
    drvobj->hw      = &bt_hw;
    drvobj->pwrctl  = 0;
    drvobj->suspend = 0;
    drvobj->pwrsave = BT_PWRSAVE_UNSUPPORTED;
    drvobj->rdelay  = 50;
    drvobj->kobj    = &devobj->dev->kobj;
    mutex_init(&drvobj->sem);


    /*initialize members*/
    spin_lock_init(&bt_private.lock);
    init_waitqueue_head(&bt_private.read_wait);
    init_MUTEX(&bt_private.sem);
    memset(bt_private.capture, 0x00, sizeof(bt_private.capture));


    /*set platform data:
      a new device created for bt */
    platform_set_drvdata(dev, devobj);

    BTDRV_DBG("Done\n");
    return 0;

error:
    if (err == 0)
        cdev_del(&devobj->chdev);
    if (ret == 0)
        unregister_chrdev_region(devobj->devno, 1);
    return -1;
}



static int btdrv_remove(struct platform_device *dev)
{
    struct bt_dev_obj *devobj = (struct bt_dev_obj*)platform_get_drvdata(dev);
    struct bt_dev_obj *drvobj = (struct bt_dev_obj*)dev_get_drvdata(devobj->dev);

    if (!devobj || !drvobj) {
        BTDRV_ERR("null pointer: %p, %p\n", devobj, drvobj);
        return -1;
    }

    BTDRV_DBG("Unregistering chardev\n");
    kfree(devobj);

    cdev_del(&devobj->chdev);
    unregister_chrdev_region(devobj->devno, 1);

    bt_hw_exit(devobj->hw);

    os_device_destroy(btdrv_class, (dev_t)dev);
    os_class_destroy(btdrv_class);
    BTDRV_DBG("Done\n");
    return 0;
}


/*****************************************************************************/
#if 0
static struct platform_driver bt_driver =
{
    .probe      = btdrv_probe,
    .remove     = btdrv_remove,
    .shutdown   = btdrv_shutdown,
#if defined(CONFIG_PM)
//    .suspend    = btdrv_suspend,
//    .resume     = btdrv_resume,
#endif
    .driver     = {
        .name = BT_DEVNAME,
      //  .bus    = &platform_bus_type,
    },
};

#else
static const struct of_device_id ac83xx_bluetooth_of_ids[] = {
        { .compatible = "Autochips,ac83xx-CNNdynamic", },
        {}
};

static struct platform_driver bt_driver = {
    .probe      = btdrv_probe,
    .remove     = btdrv_remove,
    .driver     = {
        .name   = BT_DEVNAME,
        .owner  = THIS_MODULE,
        .of_match_table = ac83xx_bluetooth_of_ids,
    },
};
#endif

/*****************************************************************************/
static int __init btdrv_mod_init(void)
{
    int ret = 0;

    MOD_VERSION_INFO(BTDRV_MODE_NAME, BTDRV_VER_MAJOR, BTDRV_VER_MINOR, BTDRV_VER_REV);//add_version_info

    BTDRV_DBG("btdrv_mod_init\n");
#if 0
    ret = os_device_register(&device_bt);

    if(ret != 0)
    {
        BTDRV_DBG("bt drv register failure \n");
    }
#else
#endif
    ret = os_driver_register(&bt_driver);
    BTDRV_DBG("btdrv_mod_init finish\n");

    return ret;
}
/*****************************************************************************/
static void __exit btdrv_mod_exit(void)
{
    BTDRV_DBG("btdrv_mod_exit\n\r");
    os_driver_unregister(&bt_driver);
}

module_init(btdrv_mod_init);
module_exit(btdrv_mod_exit);

MODULE_AUTHOR("Autochips");
MODULE_DESCRIPTION("bluetooth driver for Autochips");
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);


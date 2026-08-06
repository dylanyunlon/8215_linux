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

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/sched.h>
/*#include <linux/smp_lock.h>*/
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <linux/proc_fs.h>   /* proc file use */
#include <linux/types.h>
#include <generated/atc_project.h>

#include "x_typedef.h"
#include "drv_dual.h"
#include "backcar_msg.h"
#include "pmx_hal.h"
#include "wch_if.h"
//#include "ac83xx_memory.h"
#include "windev.h"
#include "oal.h"
#include "x_ver.h"

#ifdef CONFIG_ATC_PLATFORM_ac823x
#include <linux/gpio/consumer.h>
#define ARM2_RES_MEMORY_RECYCLE 0
#else
#include <mach/dma.h>
#include "drv_av_d.h"
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "pinmux_reg.h"
#include <linux/gpio.h>
#define ARM2_RES_MEMORY_RECYCLE 1
#endif

#if ARM2_RES_MEMORY_RECYCLE
#include <linux/of.h>
#include <linux/of_address.h>
#include <memory.h>
#include <linux/kthread.h>

// ARM2 RESOURCE is located at arm2.bin, offset is 0x200000.
/*  using offset to recycle memory -- not obvious , not good.
     should use a obsolated memory section for arm2 resource.

     this value is from arm2/arm2_83xx_Linux.lds
     the address of section track_res
   eg:
     . = 0x200000;
     track_res :
     {
      *(.track_res)
      }
*/
#define ARM2_RES_OFFSET    0X200000
//Resmem: start = arm2mem.start  + ARM2_RES_OFFSET;
//            size = arm2mem.size   - ARM2_RES_OFFSET;
#endif

#define  FASTCAMERAUI_ENABLE      1
#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"
#define BC_LOG_TAG "[BackCar]"

/**
Revision Control
*/
#define DBC_MOD_NAME    "DualArmBackCar"
#define DBC_VER_MAIN    1
#define DBC_VER_MINOR   0
#define DBC_VER_REV     0

MODULE_LICENSE("GPL");

/*extern void  HideBackVideo(void);*/

#if ARM2_RES_MEMORY_RECYCLE

/* add for arm2 resource (memory) recycle */
extern int free_memblock_runtime(phys_addr_t phy_addr,phys_addr_t size);

struct RES_MEM {
    u32 start;
    u32 size;
};

static struct RES_MEM UImem = {0,0};

#ifdef CONFIG_ATC_OS_linux
static struct RES_MEM resmem = {0,0};
#endif

static struct device_node *node = NULL;

#endif

/************  [ IOCTL Code ]  ***************/

#define IOCTL_FSC_NOTIFY_APP_READY         _IOR('M', 0x1, unsigned)
#define IOCTL_FSC_NOTIFY_ARM2_STOP         _IOR('M', 0x2, unsigned)
#define IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET  _IOR('M', 0x3, unsigned)
#define IOCTL_FSC_VIDEO_BLACKSCREEN         _IOWR('M', 0x4, WCH_BUFF_INFO_T*)
#define IOCTL_FSC_NOTIFY_ARM1_READY        _IOR('M', 0x5, unsigned)
#define IOCTL_FSC_GET_ARM2_STATUS          _IOR('M', 0x8, unsigned)

#define IOCTL_BC_GPIOINIT _IOR('M', 0x6, unsigned)
#define IOCTL_BC_DETECTGPIO _IOR('M', 0x7, unsigned)

/* for MCU Test */
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define IOCTL_MCU_BC_START_SUCCESS         _IOR('M', 0x11, unsigned)
#define IOCTL_MCU_BC_STOP                  _IOR('M', 0x12, unsigned)
#define BACKCAR_NOTIFY_MCU_GPIO_PIN        126
#define BACKCAR_NOTIFY_MCU_GPIO_HIGH       1
#define BACKCAR_NOTIFY_MCU_GPIO_LOW        0
static int MCUTestCount = 0;
#endif

static long u4UsingGPIOPinNum;
static wait_queue_head_t *g_ioctlwaitq;
static unsigned  g_arm2backcarstatus;
#ifdef CONFIG_ATC_OS_linux
#define SYSTEM_INFO_SIZE 2
static char system_status[SYSTEM_INFO_SIZE] = {0};
#endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
struct gpio_desc *pbackcarGPIO;
const char *const backcargpiofct[] = {"backcar"};

void GPIO_Init(void)
{
    pr_info(BC_LOG_TAG "GPIO Init enter!\r\n");

    int err = gpiod_direction_input(pbackcarGPIO);
    if (IS_ERR(err)) {
        pr_info(BC_LOG_TAG "GPIO_Init(): can not set backcar gpio direction to input\r\n");
    }
    pr_info(BC_LOG_TAG "GPIO Init leave!\r\n");
}
#else
void GPIO_Init(unsigned long u4GPIOPinNum)
{
    u4UsingGPIOPinNum = u4GPIOPinNum;

    GPIO_MultiFun_Set(u4GPIOPinNum, PINMUX_LEVEL_GPIO_END_FLAG);
    gpio_request(u4GPIOPinNum, "BackCar_Init_GPIO");
    gpio_direction_input(u4GPIOPinNum);
    pr_info(BC_LOG_TAG "GPIO %ld Init Success!\r\n", u4UsingGPIOPinNum);
}
#endif

int GPIO_Get_value(void)
{
#ifdef CONFIG_ATC_PLATFORM_ac823x
    int u8Val = gpiod_get_value(pbackcarGPIO);
#else
    int u8Val = gpio_get_value(u4UsingGPIOPinNum);
#endif
    return u8Val;
}

#if ARM2_RES_MEMORY_RECYCLE

/* ---- add for arm2 resource (memory) recycle ----- */

DECLARE_WAIT_QUEUE_HEAD(backcarqueue);
static bool fgrelease = false;
static struct task_struct *ts1;

static int arm2ResRecycle_thread(void *arg)
{
    unsigned int ret = 0;
    pr_info(BC_LOG_TAG "arm2ResRecycle_thread start\r\n");
    ret = wait_event_interruptible(backcarqueue, (fgrelease == true));
    pr_info(BC_LOG_TAG "[UImem] receive event, free memblock start  start is %x,size is %x\r\n",UImem.start,UImem.size);
    free_memblock_runtime((phys_addr_t)UImem.start,(phys_addr_t)UImem.size);
#ifdef CONFIG_ATC_OS_linux
    pr_info(BC_LOG_TAG "[resmem] receive event, free memblock start  start is %x,size is %x\r\n",resmem.start,resmem.size);
    free_memblock_runtime((phys_addr_t)resmem.start,(phys_addr_t)resmem.size);
#endif
    pr_info(BC_LOG_TAG "receive event, free memblock end\r\n");
    return ret;
}

int do_arm2ResRecycle(void)
{
    pr_info(BC_LOG_TAG "do_arm2ResRecycle: start is %x,size is %x\r\n",UImem.start,UImem.size);
    if((UImem.start != 0) && (UImem.size !=0))
    {
    #ifdef CONFIG_ATC_OS_linux
        if((resmem.start != 0) && (resmem.size !=0))
    #endif
        {
            fgrelease = true;
            wake_up_interruptible(&backcarqueue);
            return 0;
        }
    #ifdef CONFIG_ATC_OS_linux
        else{
            pr_err(BC_LOG_TAG "do_arm2ResRecycle:[resmem] error - invalid memory region for recycle !\r\n");
            return -1;
        }
    #endif
    } else {
        pr_err(BC_LOG_TAG "do_arm2ResRecycle:[UImem] error - invalid memory region for recycle !\r\n");
        return -1;
    }
}
/* ---- end  arm2 resource (memory) recycle ----- */
#endif
static irqreturn_t backcar_isr_handler(int irq, void *dev_id)
{
    UINT32 g_u4Param1, g_u4Param2, g_u4Param3;
    UINT32 u4ModuleID, u4MessageID;

    HWGetMessage(MODULE_BCAR, &u4ModuleID, &g_u4Param1, &g_u4Param2, &g_u4Param3);
    u4MessageID = GETMESSAGEID(u4ModuleID);

    if (u4MessageID == MSG_ARM2_RESPONSE) {
        pr_info(BC_LOG_TAG "[FSC][INFO]Rev ARM2 Responese\r\n");

        if (g_u4Param1 == ARM2_STATUS_NO_BACK_CAR) {
            #if ARM2_RES_MEMORY_RECYCLE
            pr_info(BC_LOG_TAG "[FSC][INFO]Rev ARM2 Responese : ARM2_STATUS_NO_BACK_CAR\r\n");
            if (do_arm2ResRecycle() != 0) {//arm2  memory recycle
                pr_info(BC_LOG_TAG "[FSC][INFO]wake up arm2 res recycle thread fail!\r\n");
            }
            #endif
        } else {
            pr_info(BC_LOG_TAG "[FSC][INFO]Rev ARM2 Responese : other responese\r\n");
        }
    }

    g_arm2backcarstatus = g_u4Param1;
    if (NULL != g_ioctlwaitq) {
        wake_up(g_ioctlwaitq);
    }

    pr_info(BC_LOG_TAG "[FSC][INFO]g_u4Param1 = %d\r\n", (unsigned int)g_u4Param1);
    return IRQ_HANDLED;
}

static long backcar_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    u_long size;
    int ret = 0;
    wait_queue_head_t waitq;
    WCH_BUFF_INFO_T rWchbuffers;
    DECLARE_WAITQUEUE(wait, current);

    if (cmd != IOCTL_BC_DETECTGPIO) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): enter! cmd = %u\r\n", cmd);
    }

    size = (cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT;

    if (cmd & IOC_IN) {
        if (!access_ok(VERIFY_READ, argp, size)) {
            pr_info(BC_LOG_TAG "backcar_ioctl(): error 1! \r\n");
            return -EFAULT;
        }
    }

    if (cmd & IOC_OUT) {
        if (!access_ok(VERIFY_WRITE, argp, size)) {
            pr_info(BC_LOG_TAG "backcar_ioctl(): error 2! \r\n");
            return -EFAULT;
        }
    }

    if (cmd == IOCTL_FSC_NOTIFY_APP_READY) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): IOCTL_FSC_NOTIFY_APP_READY\r\n");

        init_waitqueue_head(&waitq);
        add_wait_queue(&waitq, &wait);
        g_ioctlwaitq = &waitq;
        set_current_state(TASK_INTERRUPTIBLE);

        pr_info(BC_LOG_TAG "backcar_ioctl(): Inform Arm2, Arm1 Backcar apk is ready!\r\n");
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ANDROID_APP_READY), 0, 0, 0);

        pr_info(BC_LOG_TAG "backcar_ioctl(): wait arm2 interrupt!\r\n");
        schedule();
        pr_info(BC_LOG_TAG "backcar_ioctl(): wait arm2 interrupt after schedule!\r\n");
        #ifdef CONFIG_ATC_OS_linux
        snprintf(system_status, sizeof(system_status), "%d", 1);
        #endif

        if (signal_pending(current)) {
            pr_info(BC_LOG_TAG "backcar_ioctl(): signal wakeup task\r\n");

            ret = -ERESTARTSYS;
        } else {
            pr_info(BC_LOG_TAG "backcar_ioctl(): wait arm2 interrupt done!\r\n");

            #ifdef CONFIG_ATC_PLATFORM_ac823x
                vPmxMixPlane(0, PRIMARY_SURF_PLANE); /*for enable primary osd*/
            #else
                vPmxHalMixPlane(0, PRIMARY_SURF_PLANE); /*for enable primary osd*/
            #endif
        }

        remove_wait_queue(&waitq, &wait);
        set_current_state(TASK_RUNNING);
        copy_to_user(argp, &g_arm2backcarstatus, sizeof(unsigned));
        g_ioctlwaitq = NULL;

    } else if (cmd == IOCTL_FSC_NOTIFY_ARM2_STOP) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): fsc notify arm2 stop!\r\n");
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_NOTIFY_ARM2_STOP), 0, 0, 0);

    } else if (cmd == IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): fsc notify arm2 buff memset");
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_NOTIFY_ARM2_BUFF_MEMSET), 0, 0, 0);

    } else if (cmd == IOCTL_FSC_VIDEO_BLACKSCREEN) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): fsc video blackscreen!\r\n");

        if (copy_from_user(&rWchbuffers, (WCH_BUFF_INFO_T *)arg, sizeof(WCH_BUFF_INFO_T))) {
            pr_info(BC_LOG_TAG "backcar_ioctl(): copy_from_user wchbuffers error!\r\n");
            return -EFAULT;
        }

        //for (i = 0; i < rWchbuffers.u4BufCnt; i++) {
            //memset((void *)(MEMRSV_PHY_TO_VIRT(rWchbuffers.u4YBuf[i])), 0x10, WCH_SD_YBUF_SIZE);
            //memset((void *)(MEMRSV_PHY_TO_VIRT(rWchbuffers.u4CBuf[i])), 0x80, WCH_SD_CBUF_SIZE);
        //}

    } else if (cmd == IOCTL_FSC_NOTIFY_ARM1_READY) {
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ANDROID_APP_READY), 0, 0, 0);

    } else if (cmd == IOCTL_BC_GPIOINIT) {
        #ifdef CONFIG_ATC_PLATFORM_ac823x
            GPIO_Init();
        #else
            GPIO_Init(arg);
        #endif
        pr_info(BC_LOG_TAG "backcar_ioctl(): Init GPIO success!\r\n");

    } else if (cmd == IOCTL_BC_DETECTGPIO) {
        unsigned u4Val = GPIO_Get_value();
        /*pr_info(BC_LOG_TAG "backcar_ioctl(): Get GPIO value %d!\r\n", u4Val);*/
        copy_to_user(argp, &u4Val, sizeof(unsigned));

    } else if (cmd == IOCTL_FSC_GET_ARM2_STATUS) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): Get ARM2 Backcar Status!\r\n");
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ANDROID_APP_READY), 0, 0, 0);
        copy_to_user(argp, &g_arm2backcarstatus, sizeof(unsigned));

/* for MCU Test */
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    } else if (cmd == IOCTL_MCU_BC_START_SUCCESS) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: Notify MCU Backcar start success!\r\n");
        unsigned starttime = 0;
        if (0 != copy_from_user(&starttime, argp, sizeof(unsigned))) {
            pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: get backcar start time from native fail!\r\n");
        } else {
            pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: get backcar start time from native sucess! start time = %dms\r\n", starttime);
        }

        GPIO_MultiFun_Set(BACKCAR_NOTIFY_MCU_GPIO_PIN, PINMUX_LEVEL_GPIO_END_FLAG);
        gpio_direction_output(BACKCAR_NOTIFY_MCU_GPIO_PIN, BACKCAR_NOTIFY_MCU_GPIO_HIGH);
        MCUTestCount++;
        pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: current gpio126 value = %d ,test count = %d\r\n",gpio_get_value(126), MCUTestCount);

    } else if (cmd == IOCTL_MCU_BC_STOP) {
        pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: Notify MCU Backcar stop!\r\n");
        GPIO_MultiFun_Set(126, PINMUX_LEVEL_GPIO_END_FLAG);
        gpio_direction_output(BACKCAR_NOTIFY_MCU_GPIO_PIN, BACKCAR_NOTIFY_MCU_GPIO_LOW);
        pr_info(BC_LOG_TAG "backcar_ioctl(): MCU Test: current gpio126 value = %d\r\n", gpio_get_value(126));
#endif
    }

    return ret;
}

static int backcar_open(struct inode *inode, struct file *file)
{
    MOD_VERSION_INFO(DBC_MOD_NAME, DBC_VER_MAIN, DBC_VER_MINOR, DBC_VER_REV);
    pr_info(BC_LOG_TAG "backcar_open\r\n");
    return 0;
}

static int backcar_release(struct inode *inode, struct file *file)
{
    pr_info(BC_LOG_TAG "backcar_release\r\n");
    return 0;
}

#ifdef CONFIG_ATC_PLATFORM_ac823x
static const struct of_device_id ac823x_backcar_of_match[] = {
    {
        .compatible = "Autochips,ac823x-backcar",
    },
    { },
};
#endif

static const struct file_operations backcardrv_fops = {
    .owner            = THIS_MODULE,
    .open = backcar_open,
    .release = backcar_release,
    .unlocked_ioctl   = backcar_ioctl,
};

static struct miscdevice backcar_misc_dev = {
    MISC_DYNAMIC_MINOR,
    "backcardrv",
    &backcardrv_fops
};


static int backcar_probe(struct platform_device * pdev)
{
    int ret = 0;
    pr_info(BC_LOG_TAG "backcar_probe\r\n");
    ret = misc_register(&backcar_misc_dev);
    if (ret) {
        pr_err(BC_LOG_TAG "Unable to register \"backcar\" misc device\r\n");
        return ret;
    }

#ifdef CONFIG_ATC_PLATFORM_ac823x
    pbackcarGPIO = __gpiod_get(&(pdev->dev), backcargpiofct[0], GPIOD_ASIS);
    if (IS_ERR(pbackcarGPIO)) {
        pr_info(BC_LOG_TAG "can not get gpio_desc %s \r\n", backcargpiofct);
        ret = PTR_ERR(pbackcarGPIO);
        pbackcarGPIO = NULL;
        return ret;
    }
#endif
    return ret;
}

static int backcar_remove(struct platform_device *dev)
{
    pr_info(BC_LOG_TAG "backcar_remove\r\n");
    misc_deregister(&backcar_misc_dev);
    return 0;
}

static struct platform_driver backcar_driver = {

    .driver = {
        .name = "backcar-dev",
        .owner = THIS_MODULE,
#ifdef CONFIG_ATC_PLATFORM_ac823x
        .of_match_table = ac823x_backcar_of_match,
#endif
    },
    .probe = backcar_probe,
    .remove = backcar_remove,
};

static struct platform_device backcar_device = {

    .name = "backcar-dev",
    .id = -1,

};

#ifdef CONFIG_ATC_OS_linux
/* system status */
static int system_status_read(struct seq_file *m, void *v)
{
    pr_info("system status is %s\n", system_status);
    seq_printf(m, "%s\n", system_status);
    return 0;
};

static int proc_system_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, system_status_read, NULL);
};


static  struct file_operations fbackcar_proc_fops = {
    .owner = THIS_MODULE,
    .open  = proc_system_info_open,
    .read  = seq_read,
    .release = single_release,
};
#endif

static int __init backcar_drv_init(void)
{
    int ret = 0;
    pr_info(BC_LOG_TAG "backcar_drv_init enter\r\n");

#ifndef CONFIG_ATC_PLATFORM_ac823x
    ret = platform_device_register(&backcar_device);
    if (ret) {
        pr_err(BC_LOG_TAG "Failed to register backcar device: %d\r\n", ret);
        return ret;
    }
#endif

    ret = platform_driver_register(&backcar_driver);
    if (ret) {
        pr_err(BC_LOG_TAG "Failed to register backcar dirver: %d\r\n", ret);
    #ifndef CONFIG_ATC_PLATFORM_ac823x
        platform_device_unregister(&backcar_device);
    #endif
        return ret;
    }

    #ifdef CONFIG_ATC_OS_linux
    memset(system_status, '0', SYSTEM_INFO_SIZE - 1);
    proc_create("system_ready", 0, NULL, &fbackcar_proc_fops);
    #endif

    ret = request_dualarm_irq(MODULE_BCAR, backcar_isr_handler,
                  0, "BACKCAR_ISR", NULL);
    if (ret) {
        pr_err(BC_LOG_TAG "Failed to request dualarm irq: %d\r\n", ret);
    #ifndef CONFIG_ATC_PLATFORM_ac823x
        platform_device_unregister(&backcar_device);
    #endif
        platform_driver_unregister(&backcar_driver);
        return ret;
    }

#if ARM2_RES_MEMORY_RECYCLE
    /*so far, android recycle reserved UI mem; Linux recycle UI mem and track image mem(resmem)*/
    node =of_find_compatible_node(NULL,NULL,"atc-arm2-backcar-ui");
    if (node) {
        if (of_property_read_u32_array(node,"reg",(u32 *)&UImem,2)) {
            pr_err(BC_LOG_TAG "arm2memoryrecycle:[UImem] failed to get size and start\r\n");
        }
        pr_info(BC_LOG_TAG "arm2memoryrecycle:[UImem] memory start is %x,size is %x\r\n",UImem.start,UImem.size);
    } else {
        pr_err(BC_LOG_TAG "arm2memoryrecycle:[UImem] failed to get node\r\n");
    }

    #ifdef CONFIG_ATC_OS_linux
    /*for linux, recycle memory occupied by arm2 backcar track image resource, also*/
    node =of_find_compatible_node(NULL,NULL,"atc-arm2-reserved");
    if (node) {
        if (of_property_read_u32_array(node,"reg",(u32 *)&resmem,2)) {
            pr_err(BC_LOG_TAG "arm2memoryrecycle:[resmem] failed to get size and start\r\n");
        }
        pr_info(BC_LOG_TAG "arm2memoryrecycle:[resmem] memory start is %x,size is %x\r\n",resmem.start,resmem.size);
    } else {
        pr_err(BC_LOG_TAG "arm2memoryrecycle:[resmem] failed to get node\r\n");
    }

    resmem.start  = resmem.start + ARM2_RES_OFFSET;
    resmem.size   = resmem.size  - ARM2_RES_OFFSET;
    #endif

    ts1 = kthread_create(arm2ResRecycle_thread, NULL, "arm2RecyThr");
    if (IS_ERR(ts1)) {
        pr_err(BC_LOG_TAG "arm2memoryrecycle: kthread_create fail\r\n");
        ret = PTR_ERR(ts1);
        ts1 = NULL;
    #ifndef CONFIG_ATC_PLATFORM_ac823x
        platform_device_unregister(&backcar_device);
    #endif
        platform_driver_unregister(&backcar_driver);
        return ret;
    }

    wake_up_process(ts1);
#endif

#if !FASTCAMERAUI_ENABLE
    g_ioctlwaitq = NULL;
    HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ANDROID_APP_READY), 0, 0, 0);
#endif

    pr_info(BC_LOG_TAG "backcar_drv_init leave\r\n");
    return ret;
}

static void __exit backcar_drv_exit(void)
{
    pr_info(BC_LOG_TAG "backcar_drv_exit enter\r\n");
    platform_driver_unregister(&backcar_driver);

#ifndef CONFIG_ATC_PLATFORM_ac823x
    platform_device_unregister(&backcar_device);
#endif
    pr_info(BC_LOG_TAG "backcar_drv_exit leave\r\n");

}


module_init(backcar_drv_init);
module_exit(backcar_drv_exit);



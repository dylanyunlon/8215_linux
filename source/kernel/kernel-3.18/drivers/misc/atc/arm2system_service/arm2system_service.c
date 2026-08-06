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

#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/mrdump.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/notifier.h>

#include "drv_dual.h"
#include "arm2system_service.h"

MODULE_LICENSE("GPL");
static int arm2system_stopheart = 0;
/*extern void  HideBackVideo(void);*/
//#define ARM2SYSTEM_SERVICE "ARM2SYSTEM_SERVICE" 

static int arm2system_service_heartbeat(void *arg)
{
    unsigned int ret = 0;
    HWSendMessage(MSG_COMBINE(MODULE_ARM2SYSTEMSERVICE , ARM2SYSTEM_SERVICE_HEARTBEAT_START), 0, 0, 0);
    while(!arm2system_stopheart)
    {
        HWSendMessage(MSG_COMBINE(MODULE_ARM2SYSTEMSERVICE , ARM2SYSTEM_SERVICE_HEARTBEAT), 0, 0, 0);
        msleep(300);
    }
    return ret;
}

static int arm2system_service_kernel_panic(struct notifier_block *this, unsigned long event, void *ptr)
{
    pr_info("ARM2SYSTEM_SERVICE_KERNEL_PANIC\r\n");
    arm2system_stopheart = 1;
    return NOTIFY_OK; 
}

static int arm2system_service_reboot(struct notifier_block *this, unsigned long event, void *ptr)
{
    pr_info("ARM2SYSTEM_SERVICE_REBOOT\r\n");
    HWSendMessage(MSG_COMBINE(MODULE_ARM2SYSTEMSERVICE , ARM2SYSTEM_SERVICE_REBOOT), event, 0, 0);
    return NOTIFY_OK;
}

static struct notifier_block arm2system_service_panic_blk = {
    .notifier_call = arm2system_service_kernel_panic,
    .priority = 100,
};

static struct notifier_block arm2system_service_reboot_blk = {
    .notifier_call = arm2system_service_reboot,
    .priority = 100,
};

static irqreturn_t arm2system_service_isr_handler(int irq,void* dev_id)
{
	UINT32 g_u4Param1, g_u4Param2, g_u4Param3;
	UINT32 u4ModuleID, u4MessageID;
	HWGetMessage(MODULE_ARM2SYSTEMSERVICE, &u4ModuleID, &g_u4Param1, &g_u4Param2, &g_u4Param3);
    u4MessageID = GETMESSAGEID(u4ModuleID);
	pr_info("arm2system_service_drv_init enter:%d\r\n", u4MessageID);
	if (u4MessageID == ARM2SYSTEM_SERVIC_SHUTDOWN)
	{
		pr_info("orderly_poweroff\n");
		orderly_poweroff(true);
	}
    return IRQ_HANDLED;
}

static int __init arm2system_service_drv_init(void)
{
    int ret = 0;
    static struct task_struct *ts1;
    pr_info("arm2system_service_drv_init enter\r\n");
    ts1 = kthread_create(arm2system_service_heartbeat, NULL, "arm2system_service_heartbeat");
    if (IS_ERR(ts1)) {
        pr_err("arm2system_service: kthread_create fail\r\n");
        return ret;
    }

    wake_up_process(ts1);
    request_dualarm_irq(MODULE_ARM2SYSTEMSERVICE, arm2system_service_isr_handler,0,"ARM2SYSTEM_SERVICE",NULL);
    atomic_notifier_chain_register(&panic_notifier_list, &arm2system_service_panic_blk);
    register_reboot_notifier(&arm2system_service_reboot_blk);
    pr_info("arm2system_service_drv_init leave\r\n");
    return ret;
}

static void __exit arm2system_service_drv_exit(void)
{
    pr_info("arm2system_service_drv_exit enter\r\n");
}

module_init(arm2system_service_drv_init);
module_exit(arm2system_service_drv_exit);



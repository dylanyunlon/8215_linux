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

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include "types.h"
#include "x_os.h"
#include "drv_win32_if.h"
#include "inc/winutil.h"
#include "x_ver.h"
#include "windev.h"
#include <linux/spinlock_types.h>

#define MULTIPULE_EVENT_NUM   8

#include <linux/types.h>


extern spinlock_t ac83xx_event_lock;

#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"

#define DLOG(x...)  printk(x)

typedef struct
{
	struct miscdevice cdev;   /* Char device structure */
} osal_dev;

static osal_dev *_prOSALDev;

static int osaldev_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	return ret;
}

static int osaldev_release(struct inode *inode, struct file *file)
{
	return 0;
}

#include <asm/uaccess.h>

#define _get_usrdata(hdl, ptr) do { \
	if (copy_from_user((void *)&(hdl), (void *)(ptr), sizeof(hdl))) { \
		printk("[OSAL] createvent copy err, line %d\r\n", __LINE__); \
		return -1; \
	} \
} while (0)

static long osaldev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 ret = 0;
    void* hdl;
    WIN32_IOCTL_DATA win_ioctl;
    void* phandle[MULTIPULE_EVENT_NUM] = {0};

//  printk("enter osaldev_ioctl cmd = %d\n", cmd);

    switch (cmd)
    {
        case WIN32_IOCTL_CREATE_EVENT:
            {
                char *name;
                void* hRet;
                EVENT_PARA_T eventPara;

                _get_usrdata(win_ioctl, arg);
                memset(&eventPara, 0, sizeof(EVENT_PARA_T));
                name = (char *)win_ioctl.pInBuf;
                if (name) {
                    if (copy_from_user((void *)(&eventPara), (void *)name, sizeof(EVENT_PARA_T))) {
                        printk("[OSAL] createvent copy err, line %d\r\n", __LINE__);
                        return NULL;
                    }
                }
                //printk("createvent, driver, name %s, bManualReset %d, bInitialState %d\r\n", eventPara.name, eventPara.bManualReset, eventPara.bInitialState);
                hRet = x_event_create(NULL, eventPara.bManualReset, eventPara.bInitialState, (LPCSTR)eventPara.name);
                ret = (u32)hRet;
            }
            break;

        case WIN32_IOCTL_OPEN_EVENT:
            {
                void* hRet;

                WIN_EVENT_OPEN eData;
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(eData, win_ioctl.pInBuf);

                hRet = x_event_open(eData.dwDesiredAccess, eData.bInheritHandle, eData.szName);
                ret = (u32)hRet;
            }
            break;

        case WIN32_IOCTL_RESET_EVENT:
            {
                bool v;
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(hdl, win_ioctl.pInBuf);

                v = x_event_reset(hdl);
                ret = (u32)v;
            }
            break;

        case WIN32_IOCTL_SET_EVENT:
            {
                bool v;
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(hdl, win_ioctl.pInBuf);

                v = x_event_set(hdl);
                ret = (u32)v;
            }
            break;

        case WIN32_IOCTL_SET_EVENT_DATA:
            {
                WIN_EVENT_DATA eData;
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(eData, win_ioctl.pInBuf);

                ret = (u32)x_event_set_data(eData.handle, eData.dwData);
            }
            break;

        case WIN32_IOCTL_GET_EVENT_DATA:
            {
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(hdl, win_ioctl.pInBuf);
                ret = (u32)x_event_get_data(hdl);
            }
            break;

        case WIN32_IOCTL_DELETE_EVENT:
            {
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(hdl, win_ioctl.pInBuf);
                ret = (u32)x_event_destroy(hdl);
            }
            break;

        case WIN32_IOCTL_WAIT_EVENT:
            {
                WIN_EVENT_WAIT_FOR_DATA edata;
                _get_usrdata(win_ioctl, arg);
                _get_usrdata(edata, win_ioctl.pInBuf);
                if (edata.nCount > MULTIPULE_EVENT_NUM)
                {
                    printk("[OSAL] now we can only support at most %d event for syswaitformultipleobjects, line %d\r\n", MULTIPULE_EVENT_NUM, __LINE__);
                    ret = WAIT_FAILED;
                    break;
                }
                copy_from_user((void *)(phandle), (void *)(edata.lpHandles), edata.nCount*sizeof(void*));
                ret = (u32)x_event_wait_for_objects( edata.nCount, phandle,
                                                    FALSE, edata.dwMilliseconds );
            }
            break;
        default:
            ret = (u32)(-1);
            break;
    }

       return ret;
}


struct file_operations osaldev_fops = {
    .release = osaldev_release,
    .open = osaldev_open,
    .unlocked_ioctl = osaldev_ioctl,
};

//====== add sysfs node begin===================
#define SYSEVENT_SYSFS_NODE 0

#if SYSEVENT_SYSFS_NODE
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "oal.h"
#include "x_ver.h"

#include <linux/list.h>


//----------New KERNEL Standard API----------------

#define OS_LINUX_EVENT_MAX_NAME_LEN	(32)

typedef struct _OS_LINUX_EVENT_T {
     u32 magic;
     char name[OS_LINUX_EVENT_MAX_NAME_LEN];
     int refcnt;
     bool bSignaled;
     unsigned long data;
     bool bManualReset;
     bool bWaitingForFree;
     struct list_head wq_head;
     struct list_head ev_self;
     int mPID;
 } OS_LINUX_EVENT_T;

 extern struct list_head g_event_list_head;

static ssize_t list_event_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	char *s = buf;

	WIN_EVENT *pEvent;
	unsigned long flags;
	struct list_head *pglobal_list = &g_event_list_head;

	if (!list_empty(pglobal_list)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock,flags);
		list_for_each_entry(pEvent, pglobal_list, ev_self) {
			s += sprintf(s,"%s pid=%d\r\n",pEvent->name,pEvent->mPID);
		}
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock,flags);
	}

	return (s-buf);
}

static ssize_t list_event_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t n)
{
	return 0;
}

static struct kobj_attribute list_attr = {
	.attr = {
		.name = "list_event",
		.mode = 0644,
	},
	.show = list_event_show,
	.store = list_event_store,
};

//-------------------------------------------------




static struct attribute * g[] = {
	&list_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

struct kobject *gSysEvent_kobj;

#endif

#define DRVOSAL_VER_MAIN	1
#define DRVOSAL_VER_MINOR	00
#define DRVOSAL_VER_REV	00

#if 0//ndef KERNEL_STANDARD_API
extern s32 isr_init(VOID);
#endif

static int __init drvosal_init(void)
{
    int result;

	//printk(KERN_ERR "Init osal driver\r\n");

	MOD_VERSION_INFO("drvosal",DRVOSAL_VER_MAIN,DRVOSAL_VER_MINOR,DRVOSAL_VER_REV);

#if 0//ndef KERNEL_STANDARD_API
	if (!InitWin32Envrionment())
	{
		printk(KERN_ERR "Init win32 environment error\r\n");
		return -1 ;
	}
#endif
    _prOSALDev = kmalloc(sizeof(osal_dev), GFP_KERNEL);
    if (!_prOSALDev) {
        DLOG("[OSAL]: malloc device failed\r\n");
        return -ENOMEM;
    }
    memset((void *)_prOSALDev, 0, sizeof(osal_dev));

    _prOSALDev->cdev.name = "osaldev";
    _prOSALDev->cdev.minor = MISC_DYNAMIC_MINOR;
    _prOSALDev->cdev.fops = &osaldev_fops;

    result = misc_register(&(_prOSALDev->cdev));
    if (result == 0)
    {
        DLOG("[OSAL] osal dev init successes\r\n");
    }
    else
    {
        DLOG("[OSAL] osal dev init error\r\n");
        kfree(_prOSALDev);
        _prOSALDev = NULL;
    }

#if SYSEVENT_SYSFS_NODE
	gSysEvent_kobj = os_create_and_add_node("SysEvent", NULL);
	if(!gSysEvent_kobj){
		DLOG("[OSAL] gSysEvent_kobj init error\r\n");
        if (_prOSALDev) {
            misc_deregister(&(_prOSALDev->cdev));
            kfree(_prOSALDev);
            _prOSALDev = NULL;
        }
		return -1;//return what err code?
	}
	os_create_fs_group(gSysEvent_kobj,&attr_group);
#endif

#if 0//ndef KERNEL_STANDARD_API
	isr_init();
#endif

	return 0;
}

static void __exit drvosal_exit(void)
{
#if 0//ndef KERNEL_STANDARD_API
    DeinitWin32Envrionment();
#endif

	if (_prOSALDev)
	{
		misc_deregister(&_prOSALDev->cdev);
		kfree(_prOSALDev);
        _prOSALDev = NULL;
	}
#if SYSEVENT_SYSFS_NODE
	//TODO need del kobj?
#endif
}

module_init(drvosal_init);
module_exit(drvosal_exit);

MODULE_LICENSE("GPL");


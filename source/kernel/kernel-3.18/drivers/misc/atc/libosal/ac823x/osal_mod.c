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
#include "osal_compat.h"
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

#ifdef CONFIG_COMPAT

//EXTERN long OsalDev_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg);

long osaldev_ioctl_compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	WIN32_IOCTL_DATA win32_ioctl;
	void *pIn = NULL;
	void *pOut = NULL;
    long ret = 0;

	if (NULL == file) {
		printk("[OSAL] %s fail for file is NULL, ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -EINVAL;
	}

	if (0 == arg) {
		printk("[OSAL] %s fail for no arg, ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -EINVAL;
	}
	
	if (!file->f_op->unlocked_ioctl) {
		printk("[OSAL] %s fail for no unlocked_ioctl, ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
  }

	if (_IOC_TYPE(cmd) != OSAL_DRV_MAGIC) {
	printk("[OSAL] %s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
	  __func__, __LINE__,
	  _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
  }

	ret = OsalDev_IOControl_Compat(file, cmd, arg);
  if (0 != ret) {
		printk("[OSAL] %s line %d fail in proc ioctl '%c', dir=%d, #%d (0x%08x)\n",
		  __func__, __LINE__,
	  _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -EPERM;
  }
	
	return 0;
}
	
EXPORT_SYMBOL(osaldev_ioctl_compat);	
#endif     //CONFIG_COMPAT


static u64 osaldev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
//  printk("enter osaldev_ioctl cmd = %d\n", cmd);

    switch (cmd)
    {
        case WIN32_IOCTL_CREATE_EVENT:
            {
                void* hRet = NULL;
                OSAL_CREATE_EVENT_T rCreateEvt;
								OSAL_CREATE_EVENT_T __user *usr_ptr = (OSAL_CREATE_EVENT_T __user *)arg;

                memset(&rCreateEvt, 0, sizeof(OSAL_CREATE_EVENT_T));
                _get_usrdata(rCreateEvt, usr_ptr);
                //printk("createvent, driver, name %s, bManualReset %d, bInitialState %d\r\n", eventPara.name, eventPara.bManualReset, eventPara.bInitialState);
                hRet = x_event_create(NULL, rCreateEvt.bManualReset, rCreateEvt.bInitialState,
	                (const char *)rCreateEvt.name);
								if (0 != copy_to_user(&(usr_ptr->hEvent), &hRet, sizeof(__u64))) {
                    pr_err("%s line %d fail in copy_to_user, WIN32_IOCTL_CREATE_EVENT\r\n",
                        __func__, __LINE__);
                    return -1;
							  }
                return 0;
            }    
		
        case WIN32_IOCTL_OPEN_EVENT:
            {
                void *hRet = NULL;
                OSAL_OPEN_EVENT_T rOpenEvt;
								OSAL_OPEN_EVENT_T __user *usr_ptr = (OSAL_OPEN_EVENT_T __user *)arg;

                _get_usrdata(rOpenEvt, arg);

                hRet = x_event_open(rOpenEvt.dwDesiredAccess, rOpenEvt.bInheritHandle,
									rOpenEvt.szName);
								if (0 != copy_to_user(&(usr_ptr->hEvent), &hRet, sizeof(__u64))) {
                    pr_err("%s line %d fail in copy_to_user(%p), WIN32_IOCTL_OPEN_EVENT\r\n",
                        __func__, __LINE__, hRet);
                    return -1;
							  }
                return 0;
            }    

        case WIN32_IOCTL_RESET_EVENT:
            {
								void *hdl = NULL;
                _get_usrdata(hdl, arg);
                if (!x_event_reset(hdl)) {
									pr_err("%s line %d fail in x_event_reset(%p), WIN32_IOCTL_RESET_EVENT\r\n",
											__func__, __LINE__, hdl);
									return -1;
                }
	            	return 0;
						}    

        case WIN32_IOCTL_SET_EVENT:
            {
							void *hdl = NULL;
							_get_usrdata(hdl, arg);
							if (!x_event_set(hdl)) {
								pr_err("%s line %d fail in x_event_set(%p), WIN32_IOCTL_SET_EVENT\r\n",
										__func__, __LINE__, hdl);
								return -1;
							}
							return 0;
					  }    

        case WIN32_IOCTL_SET_EVENT_DATA:
            {
                OSAL_GSET_EVENT_DATA_T rData;
                _get_usrdata(rData, arg);

							if (!x_event_set_data(rData.hEvent, rData.ulData)) {
								pr_err("%s line %d fail in x_event_set_data(%p, 0x%llx), WIN32_IOCTL_SET_EVENT_DATA\r\n",
										__func__, __LINE__, rData.hEvent, rData.ulData);
								return -1;
							}
							return 0;
						}    

        case WIN32_IOCTL_GET_EVENT_DATA:
            {
							OSAL_GSET_EVENT_DATA_T rData;
							OSAL_GSET_EVENT_DATA_T __user *usr_ptr = (OSAL_GSET_EVENT_DATA_T __user *)arg;
							unsigned long data = 0;
							_get_usrdata(rData, arg);
							data = x_event_get_data(rData.hEvent);
							if (0 != copy_to_user(&(usr_ptr->ulData), &data, sizeof(unsigned long))) {
									pr_err("%s line %d fail in copy_to_user(0%llx), WIN32_IOCTL_GET_EVENT_DATA\r\n",
											__func__, __LINE__, data);
									return -1;
							}
							return 0;
					  }    

        case WIN32_IOCTL_DELETE_EVENT:
            {
							void *hdl = NULL;
							_get_usrdata(hdl, arg);
							if (!x_event_destroy(hdl)) {
								pr_err("%s line %d fail in x_event_destroy(%p), WIN32_IOCTL_DELETE_EVENT\r\n",
										__func__, __LINE__, hdl);
								return -1;
							}
							return 0;
					  }    
        case WIN32_IOCTL_WAIT_EVENT:
						{
							OSAL_WAIT_EVENT_T rWaitEvents;
							OSAL_WAIT_EVENT_T __user *usr_ptr = (OSAL_WAIT_EVENT_T __user *)arg;
							void *phandle[MULTIPULE_EVENT_NUM] = {0};
							void **lpHandles = NULL;
							__u32 i = 0;
							unsigned long ret = 0; 

							memset(&rWaitEvents, 0, sizeof(rWaitEvents));
							_get_usrdata(rWaitEvents, arg);
							lpHandles = (void **)rWaitEvents.lpHandles;
							for (i = 0; i < rWaitEvents.nCount; i++) {
								phandle[i] = (void *)lpHandles[i];
						  }
							ret = x_event_wait_for_objects( rWaitEvents.nCount, phandle,
																									FALSE, rWaitEvents.dwMilliseconds);
							if (0 != copy_to_user(&(usr_ptr->u4WaitResult), &ret, sizeof(unsigned long))) {
									pr_err("%s line %d fail in copy_to_user(0%llx), WIN32_IOCTL_WAIT_EVENT\r\n",
											__func__, __LINE__, ret);
									return -1;
							}

							return 0;
					  }
        default:
						{
							u64 ret = 0;
	            ret = (u64)(-1);
	            return ret;
		        }
    }
}

struct file_operations osaldev_fops = {
    .release = osaldev_release,
    .open = osaldev_open,
    .unlocked_ioctl = osaldev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = osaldev_ioctl_compat
#endif

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


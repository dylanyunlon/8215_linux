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

/*!
 * @file 
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer Os interface layer, demuxer ioctrl definitions
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */


#ifndef __OSAL_COMPAT__
#define __OSAL_COMPAT__
 
#include "x_typedef.h"
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "drv_win32_if.h"

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

/*
==========================COMPAT 32 BIT=========================
*/

struct _OSAL_CREATE_EVENT_T32
{
	char name[32];
	__u32 bManualReset;
	__u32 bInitialState;
	__u64 hEvent;
} __attribute__ ((__packed__));
typedef struct _OSAL_CREATE_EVENT_T32 OSAL_CREATE_EVENT_T32;

struct _OSAL_OPEN_EVENT_T32
{
	__u32 dwDesiredAccess;
	__u32  bInheritHandle;
	char szName[32];
	__u64 hEvent;
} __attribute__ ((__packed__));
typedef struct _OSAL_OPEN_EVENT_T32 OSAL_OPEN_EVENT_T32;

struct _OSAL_WAIT_EVENT_T32
{
	__u32 nCount;
	compat_caddr_t lpHandles;
	__u32 dwMilliseconds;
	__u32 u4WaitResult;
} __attribute__ ((__packed__));
typedef struct _OSAL_WAIT_EVENT_T32 OSAL_WAIT_EVENT_T32;

struct _OSAL_GSET_EVENT_DATA_T32
{
	__u64 hEvent;
	compat_ulong_t ulData;
} __attribute__ ((__packed__));
typedef struct _OSAL_GSET_EVENT_DATA_T32 OSAL_GSET_EVENT_DATA_T32;

#define WIN32_IOCTL_CREATE_EVENT32	_IOWR(OSAL_DRV_MAGIC, 1, OSAL_CREATE_EVENT_T32)
#define WIN32_IOCTL_OPEN_EVENT32	    _IOWR(OSAL_DRV_MAGIC, 2, OSAL_OPEN_EVENT_T32)
#define WIN32_IOCTL_SET_EVENT32		  _IOW(OSAL_DRV_MAGIC, 3, compat_uptr_t)
#define WIN32_IOCTL_RESET_EVENT32		_IOW(OSAL_DRV_MAGIC, 4, compat_uptr_t)
#define WIN32_IOCTL_WAIT_EVENT32		_IOWR(OSAL_DRV_MAGIC, 5, OSAL_WAIT_EVENT_T32)
#define WIN32_IOCTL_DELETE_EVENT32	_IOW(OSAL_DRV_MAGIC, 6, compat_uptr_t)
#define WIN32_IOCTL_SET_EVENT_DATA32	_IOW(OSAL_DRV_MAGIC, 7, OSAL_GSET_EVENT_DATA_T32)
#define WIN32_IOCTL_GET_EVENT_DATA32	_IOWR(OSAL_DRV_MAGIC, 8, OSAL_GSET_EVENT_DATA_T32)



/*===============================================================*/
static long osal_native_ioctl(struct file *file, unsigned int cmd,
  unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	if (file->f_op->unlocked_ioctl)
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);

	return ret;
}

static unsigned int osaldev_ioctl_cmd_switch(unsigned int cmd)
{
	unsigned int rCmd = 0;
	#if 0
	pr_err("%s WIN32_IOCTL_CREATE_EVENT32: 0x%08x/0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n",
		__func__,
		WIN32_IOCTL_CREATE_EVENT32, WIN32_IOCTL_CREATE_EVENT,
		sizeof(OSAL_CREATE_EVENT_T32),
		sizeof(BOOL),
		sizeof(compat_caddr_t));
	pr_err("%s WIN32_IOCTL_OPEN_EVENT32: 0x%08x/0x%08x\r\n", __func__,
		WIN32_IOCTL_OPEN_EVENT32, WIN32_IOCTL_OPEN_EVENT);
	pr_err("%s WIN32_IOCTL_SET_EVENT32: 0x%08x/0x%08x\r\n", __func__,
		WIN32_IOCTL_SET_EVENT32, WIN32_IOCTL_SET_EVENT);
	pr_err("%s WIN32_IOCTL_RESET_EVENT32: 0x%08x/0x%08x\r\n", __func__,
		WIN32_IOCTL_RESET_EVENT32, WIN32_IOCTL_RESET_EVENT);
	pr_err("%s WIN32_IOCTL_DELETE_EVENT32: 0x%08x/0x%08x\r\n", __func__,
		WIN32_IOCTL_DELETE_EVENT32, WIN32_IOCTL_DELETE_EVENT);
	pr_err("%s WIN32_IOCTL_WAIT_EVENT32: 0x%08x/0x%08x\r\n", __func__,
		WIN32_IOCTL_WAIT_EVENT32, WIN32_IOCTL_WAIT_EVENT);
	#endif
	switch(cmd){
	case WIN32_IOCTL_CREATE_EVENT32: rCmd = WIN32_IOCTL_CREATE_EVENT; break;
	case WIN32_IOCTL_OPEN_EVENT32: rCmd = WIN32_IOCTL_OPEN_EVENT; break;
	case WIN32_IOCTL_SET_EVENT32: rCmd = WIN32_IOCTL_SET_EVENT; break;
	case WIN32_IOCTL_RESET_EVENT32: rCmd = WIN32_IOCTL_RESET_EVENT; break;
	case WIN32_IOCTL_WAIT_EVENT32: rCmd = WIN32_IOCTL_WAIT_EVENT; break;
	case WIN32_IOCTL_DELETE_EVENT32: rCmd = WIN32_IOCTL_DELETE_EVENT; break;
	case WIN32_IOCTL_SET_EVENT_DATA32: rCmd = WIN32_IOCTL_SET_EVENT_DATA; break;
	case WIN32_IOCTL_GET_EVENT_DATA32: rCmd = WIN32_IOCTL_GET_EVENT_DATA; break;
	default:
		pr_err("%s Fail for Unsupport dwCode = 0x%x\r\n", __func__, cmd);
		break;
	}

	return rCmd;
}

static long get_osal_create_event32(OSAL_CREATE_EVENT_T *kernel_ptr,
  OSAL_CREATE_EVENT_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(OSAL_CREATE_EVENT_T32)) ||
		copy_from_user(kernel_ptr->name, usr_ptr->name, (sizeof(char) * 32)) ||
		get_user(kernel_ptr->bInitialState, &usr_ptr->bInitialState) ||
		get_user(kernel_ptr->bManualReset, &usr_ptr->bManualReset)) {
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}

static long get_osal_open_event32(OSAL_OPEN_EVENT_T *kernel_ptr,
  OSAL_OPEN_EVENT_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(OSAL_OPEN_EVENT_T32)) ||
		copy_from_user(kernel_ptr->szName, usr_ptr->szName, (sizeof(char) * 32)) ||
		get_user(kernel_ptr->bInheritHandle, &usr_ptr->bInheritHandle) ||
		get_user(kernel_ptr->dwDesiredAccess, &usr_ptr->dwDesiredAccess)) {
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}
static long get_osal_wait_event32(OSAL_WAIT_EVENT_T *kernel_ptr,
  OSAL_WAIT_EVENT_T32 __user *usr_ptr)
{
  void __user *lpHandles = NULL;
	compat_caddr_t tmpHandles = 0;
	__u32 i = 0;
  compat_caddr_t *lpHandles32 = NULL;
	void **ppHandles = NULL;
	long ret = 0;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(OSAL_WAIT_EVENT_T32)) ||
		get_user(kernel_ptr->nCount, &usr_ptr->nCount) ||
		get_user(kernel_ptr->dwMilliseconds, &usr_ptr->dwMilliseconds) ||
		get_user(tmpHandles, &usr_ptr->lpHandles)){
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	lpHandles32 = (compat_caddr_t *)compat_ptr(tmpHandles);
  if (NULL == lpHandles32){
		pr_err("%s line %d fail for invalid lpHandles32\r\n",
			__func__, __LINE__);
		return -EFAULT;
	}
  lpHandles = (void __user *)compat_alloc_user_space(
    sizeof(void *) * kernel_ptr->nCount);
  kernel_ptr->lpHandles = lpHandles;
  if (NULL == lpHandles){
		pr_err("%s line %d fail in compat_alloc_user_space\r\n",
			__func__, __LINE__);
		return -EFAULT;
	}

	ppHandles = (__u32 *)(kernel_ptr->lpHandles);

	for (i = 0; i < kernel_ptr->nCount; i++) {
		ret = get_user(tmpHandles, &lpHandles32[i]);
		if (0 != ret) {
			pr_err("%s line %d fail in get_user(lpHandles32[%d]: 0x%08x)\r\n",
				__func__, __LINE__, i, lpHandles32[i]);
		}
		ppHandles[i] = (void *)compat_ptr(tmpHandles);
		
		if (NULL == ppHandles[i]){
			pr_err("%s line %d fail for ppHandles[%d] is NULL, tmpHandles: 0x%08x\r\n",
				__func__, __LINE__,i , tmpHandles);
			return -EFAULT;
		}
  }
	return 0;
}

static long get_osal_gset_event_data32(OSAL_GSET_EVENT_DATA_T *kernel_ptr,
  OSAL_GSET_EVENT_DATA_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(OSAL_GSET_EVENT_DATA_T32)) ||
		get_user(kernel_ptr->ulData, &usr_ptr->ulData) ||
		get_user(kernel_ptr->hEvent, &usr_ptr->hEvent)) {
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}

static int put_osal_create_event32(OSAL_CREATE_EVENT_T *kernel_ptr,
  OSAL_CREATE_EVENT_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(OSAL_CREATE_EVENT_T32))) {
		pr_err("%s line %d fail in access_ok\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	
	if (0 != copy_to_user(&(usr_ptr->hEvent), &kernel_ptr->hEvent, sizeof(__u64))) {
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}

static int put_osal_open_event32(OSAL_OPEN_EVENT_T *kernel_ptr,
  OSAL_OPEN_EVENT_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(OSAL_OPEN_EVENT_T32))) {
		pr_err("%s line %d fail in access_ok\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	
	if (0 != copy_to_user(&(usr_ptr->hEvent), &kernel_ptr->hEvent, sizeof(__u64))){
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}
static int put_osal_get_event_data32(OSAL_GSET_EVENT_DATA_T *kernel_ptr,
  OSAL_GSET_EVENT_DATA_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(OSAL_GSET_EVENT_DATA_T32)) ||
		put_user(kernel_ptr->ulData, &(usr_ptr->ulData))) {
		pr_err("%s line %d fail \r\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}
static int put_osal_wait_event32(OSAL_WAIT_EVENT_T *kernel_ptr,
  OSAL_WAIT_EVENT_T32 __user *usr_ptr)
{
	compat_uptr_t tmp = 0;

	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(OSAL_WAIT_EVENT_T32)) ||
		put_user(kernel_ptr->u4WaitResult, &(usr_ptr->u4WaitResult))) {
		pr_err("%s line %d fail\r\n", __func__, __LINE__);
		return -EFAULT;
	}

	return 0;
}

long OsalDev_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	union {
		OSAL_CREATE_EVENT_T rCreateEvt;
		OSAL_OPEN_EVENT_T rOpenEvt;
		OSAL_GSET_EVENT_DATA_T rGSetEvtData;
		OSAL_WAIT_EVENT_T rWaitEvts;
		uintptr_t rPtr;
		u32 rUint;
	} rArg;
	
	void __user *usr_ptr = compat_ptr(arg);
	bool fgCompatible = TRUE;
	long err = 0;
	compat_uptr_t usr_tmp;
	uintptr_t kernel_tmp;

	unsigned int u4CompCmd = osaldev_ioctl_cmd_switch(cmd);
	switch (u4CompCmd) {
		case WIN32_IOCTL_CREATE_EVENT:
			err = get_osal_create_event32(&rArg.rCreateEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_OPEN_EVENT:
			err = get_osal_open_event32(&rArg.rOpenEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_SET_EVENT_DATA:
		case WIN32_IOCTL_GET_EVENT_DATA:
			err = get_osal_gset_event_data32(&rArg.rOpenEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_SET_EVENT:
		case WIN32_IOCTL_RESET_EVENT:
		case WIN32_IOCTL_DELETE_EVENT:
			err = get_user(rArg.rPtr, (__u64 __user *)usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_WAIT_EVENT:
			err = get_osal_wait_event32(&rArg.rWaitEvts, usr_ptr);
			fgCompatible = FALSE;
			break;
	  default:
			break;
	}

	if (0 != err) {
    pr_err("[DMX] %s line %d fail for get compat data, err(%ld)\r\n",
      __func__, __LINE__, err);
		return err;
  }

	if (fgCompatible)
		err = osal_native_ioctl(file, u4CompCmd, (unsigned long)usr_ptr);
	else {
		mm_segment_t old_fs = get_fs();

		set_fs(KERNEL_DS);
		err = osal_native_ioctl(file, u4CompCmd, (unsigned long)&rArg);
		set_fs(old_fs);
	}

	switch (u4CompCmd) {
		case WIN32_IOCTL_CREATE_EVENT:
			err = put_osal_create_event32(&rArg.rCreateEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_OPEN_EVENT:
			err = put_osal_open_event32(&rArg.rOpenEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_SET_EVENT_DATA:
			break;
		case WIN32_IOCTL_GET_EVENT_DATA:
			err = put_osal_get_event_data32(&rArg.rOpenEvt, usr_ptr);
			fgCompatible = FALSE;
			break;
		case WIN32_IOCTL_SET_EVENT:
		case WIN32_IOCTL_RESET_EVENT:
		case WIN32_IOCTL_DELETE_EVENT:
			break;
		case WIN32_IOCTL_WAIT_EVENT:
			err = put_osal_wait_event32(&rArg.rWaitEvts, usr_ptr);
			fgCompatible = FALSE;
			break;
	  default:
			break;
	}

	return err;
}

#endif    //CONFIG_COMPAT


/*****************************************************************************************/

#endif	


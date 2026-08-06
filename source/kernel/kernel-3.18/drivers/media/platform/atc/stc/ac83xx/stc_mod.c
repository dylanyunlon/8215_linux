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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/uaccess.h>
#include <media/atc/ioctl_stc.h>
#include "stc_os.h"
#include "stc_hal.h"
#include "x_ver.h"

#define MTK_KERNEL_LINUX_LICENSE     "GPL"

#define STCDRV_MODE_NAME             "STC"
#define STCDRV_VER_MAJOR             01
#define STCDRV_VER_MINOR             00
#define STCDRV_VER_REV               00

#define pr_fmt(fmt) "[MM][STC]" fmt


typedef struct {
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;
} stcdev;

static int stcdev_open(struct inode *inode, struct file *file)
{
	void *pvInst = NULL;

	pr_info("%s enter!\r\n", __func__);
	pvInst = stc_inst_create();

	if (NULL == pvInst) {
		pr_err("%s:%s:%d. fail in create stc instance!\r\n", FILE_ONLY, __func__, __LINE__);
		return -ENOMEM;
	}

	file->private_data = pvInst;
	pr_info("%s success, Instance is %p\r\n", __func__, pvInst);

	return 0;
}

static int stcdev_release(struct inode *inode, struct file *file)
{
	void *pvInst = NULL;
	long err = 0;

	if (NULL == file) {
		pr_err("%s:%s:%d. fail for file is NULL!\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	pvInst = (void *)(file->private_data);
	pr_info("%s Instance is %p\r\n", __func__,pvInst);

	err = stc_inst_release(pvInst);

	if (err != 0) {
		pr_err("%s:%s:%d. fail in release stc instance(%p), err: %ld!\r\n",
		       FILE_ONLY, __func__, __LINE__, pvInst, err);
		return err;
	}

	file->private_data = NULL;

	return 0;
}

static long stcdev_acquiredev_ioctl(void *pvInst, void __user *arg)
{
	__u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long ret = 0;

	if (!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(__u32))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_WRITE), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	ret = stc_inst_acquire_dev(pvInst, &u4DevId);
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stc instance acquire device(id: %d), err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId, ret);
		return ret;
	}

	ret = copy_to_user((void __user *)arg, (void *)&u4DevId, sizeof(__u32));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy to user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	pr_debug("%s success!\r\n", __func__);

	return ret;
}

static long stcdev_releasedev_ioctl(void *pvInst, void __user *arg)
{
	__u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long ret = 0;

	if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(__u32))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	ret = copy_from_user((void *)&u4DevId, (void __user *)arg, sizeof(__u32));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_release_dev(pvInst, u4DevId);
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stc instance release device(id: %d), err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId, ret);
		return ret;
	}

	return ret;
}

static long stcdev_settime_ioctl(void *pvInst, void __user *arg)
{
	STC_SGET_TIME_T rTimeVal;
	long ret = 0;

	pr_debug("%s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(STC_SGET_TIME_T))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rTimeVal, 0, sizeof(rTimeVal));
	ret = copy_from_user((void *)&rTimeVal, (void __user *)arg, sizeof(STC_SGET_TIME_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_set_time(pvInst, &rTimeVal);
	if (ret != 0) {
		pr_err
		    ("%s:%s:%d. fail in stc instance(%p) set time(devid: %d, time: %lld), err: %ld!\r\n",
		     FILE_ONLY, __func__, __LINE__, pvInst, rTimeVal.u4DevId, rTimeVal.u8TimeVal, ret);
		return ret;
	} else {
		pr_info("%s, %p set time(devid: %d) %lldms\r\n",
			FILE_ONLY,pvInst,rTimeVal.u4DevId, rTimeVal.u8TimeVal/90000);
	}

	pr_debug("%s success!\r\n", __func__);

	return ret;
}

static long stcdev_gettime_ioctl(void *pvInst, void __user *arg)
{
	STC_SGET_TIME_T rTimeVal;
	long ret = 0;

	pr_debug("%s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(STC_SGET_TIME_T))) {
		pr_err("%s:%s:%d. %s line %d fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rTimeVal, 0, sizeof(rTimeVal));
	ret = copy_from_user((void *)&rTimeVal, (void __user *)arg, sizeof(STC_SGET_TIME_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_get_time(pvInst, &rTimeVal);
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stc instance(%p) get time(devid: %d), err: %ld!\r\n",
		       FILE_ONLY, __func__, __LINE__, pvInst, rTimeVal.u4DevId, ret);
		return ret;
	}

	ret = copy_to_user((void __user *)arg, (void *)&rTimeVal, sizeof(STC_SGET_TIME_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy to user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	pr_debug("%s success!\r\n", __func__);

	return ret;
}

static long stcdev_setstatus_ioctl(void *pvInst, void __user *arg)
{
	STC_SGET_STATUS_T rStatus;
	long ret = 0;

	pr_debug("%s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(STC_SGET_STATUS_T))) {
		pr_err("%s:%s:%d. %s line %d fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rStatus, 0, sizeof(rStatus));
	ret = copy_from_user((void *)&rStatus, (void __user *)arg, sizeof(STC_SGET_STATUS_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_set_status(pvInst, &rStatus);
	if (0 != ret) {
		pr_err
		    ("%s:%s:%d. fail in stc instance(%p) set status(devid: %d, status: %d), err: %ld!\r\n",
		     FILE_ONLY, __func__, __LINE__, pvInst, rStatus.u4DevId, rStatus.eStatus, ret);
		return ret;
	} else {
		pr_info("%s, %p %s stc(devid: %d)",
			FILE_ONLY,pvInst, rStatus.eStatus?"start":"pause",rStatus.u4DevId);
	}

	pr_debug("%s success!\r\n", __func__);

	return ret;
}

static long stcdev_getstatus_ioctl(void *pvInst, void __user *arg)
{
	STC_SGET_STATUS_T rStatus;
	long ret = 0;

	pr_debug("%s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(STC_SGET_STATUS_T))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rStatus, 0, sizeof(rStatus));
	ret = copy_from_user((void *)&rStatus, (void __user *)arg, sizeof(STC_SGET_STATUS_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_get_status(pvInst, &rStatus);
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stc instance(%p) get status(devid: %d), err: %ld!\r\n",
		       FILE_ONLY, __func__, __LINE__, pvInst, rStatus.u4DevId, ret);
		return ret;
	}

	ret = copy_to_user((void __user *)arg, (void *)&rStatus, sizeof(STC_SGET_STATUS_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy to user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	pr_debug("  %s success!\r\n", __func__);

	return ret;
}

static long stcdev_setrate_ioctl(void *pvInst, void __user *arg)
{
	STC_SGET_RATE_T rRate;
	long ret = 0;

	pr_debug("  %s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(STC_SGET_RATE_T))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rRate, 0, sizeof(rRate));
	ret = copy_from_user((void *)&rRate, (void __user *)arg, sizeof(STC_SGET_RATE_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_set_rate(pvInst, &rRate);
	if (0 != ret) {
		pr_err
		    ("%s:%s:%d. fail in stc instance(%p) set rate(devid: %d, rate: %d, is_low_down: %s), err: %ld!\r\n",
		     FILE_ONLY, __func__, __LINE__, pvInst, rRate.u4DevId, rRate.u4Rate,
		     (rRate.is_slow_down ? "true" : "false"), ret);
		return ret;
	}

	pr_debug("  %s success!\r\n", __func__);

	return ret;
}

static long stcdev_update_ioctl(void *pvInst, void __user *arg)
{
	STC_UPDATE_INFO_T rUpdateInfo;
	long ret = 0;

	pr_debug("  %s enter!\r\n", __func__);

	if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(STC_UPDATE_INFO_T))) {
		pr_err("%s:%s:%d. fail in access_ok(VERIFY_READ), arg: %p\r\n",
		       FILE_ONLY, __func__, __LINE__, arg);
		return -EACCES;
	}

	memset(&rUpdateInfo, 0, sizeof(rUpdateInfo));
	ret = copy_from_user((void *)&rUpdateInfo, (void __user *)arg, sizeof(STC_UPDATE_INFO_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	ret = stc_inst_update(pvInst, &rUpdateInfo);
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stc instance(%p) update(devid: %d), err: %ld!\r\n",
		       FILE_ONLY, __func__, __LINE__, pvInst, rUpdateInfo.u4DevId, ret);
		return ret;
	}

	ret = copy_to_user((void __user *)arg, (void *)&rUpdateInfo, sizeof(STC_UPDATE_INFO_T));
	if (0 != ret) {
		pr_err("%s:%s:%d. fail in copy from user, arg: %p, err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	pr_debug("  %s success!\r\n", __func__);

	return ret;
}

static long stcdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void *pvInst = NULL;
	long ret = 0;

	pvInst = file->private_data;

	if (NULL == pvInst) {
		pr_err
		    ("%s:%s:%d. fail for no private_data for this file=%p, ioctl '%c', dir=%d, #%d (0x%08x)\n",
		     FILE_ONLY, __func__, __LINE__, file, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -EPERM;
	}

	switch (cmd) {
	case IOCTL_STCDEV_ACQUIRE:
		ret = stcdev_acquiredev_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_RELEASE:
		ret = stcdev_releasedev_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_GETTIME:
		ret = stcdev_gettime_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_SETTIME:
		ret = stcdev_settime_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_SETSTATUS:
		ret = stcdev_setstatus_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_GETSTATUS:
		ret = stcdev_getstatus_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_SETRATE:
		ret = stcdev_setrate_ioctl(pvInst, (void __user *)arg);
		break;
	case IOCTL_STCDEV_UPDATE:
		ret = stcdev_update_ioctl(pvInst, (void __user *)arg);
		break;
	default:
		pr_err("%s:%s:%d. fail for invalid ioctl '%c', dir=%d, #%d (0x%08x), file=%p\n",
		       FILE_ONLY, __func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, file);
		ret = -ENOIOCTLCMD;
		break;
	}

	if (ret != 0) {
		pr_err("%s:%s:%d. fail in ioctl '%c', dir=%d, #%d (0x%08x), file=%p, err=%d\n",
		       FILE_ONLY, __func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, file, ret);
		return ret;
	}

	return 0;
}

#if CONFIG_COMPAT
typedef struct _stc_update_ininfo32
{
  __s64    i8BaseTime;
  __s64    i8StartTime;
  __u64    u8StartMediaTime;
} STC_UPDATE_IN_INFO_T32;

typedef struct stc_sget_time32
{
  __u32 u4DevId;
  __u64 u8TimeVal;
} STC_SGET_TIME_T32;

typedef struct _stc_update_outinfo32
{
  __s64    i8BaseTime;
  __u64    u8STCTime;
} STC_UPDATE_OUT_INFO_T32;

typedef struct stc_sget_status32
{
  __u32 u4DevId;
  STC_STATUS_T eStatus;
} STC_SGET_STATUS_T32;

typedef struct stc_sget_rate32
{
  __u32 u4DevId;
  __u32 u4Rate;
  bool  is_slow_down;
} STC_SGET_RATE_T32;

typedef struct stc_update_info32
{
  __u32 u4DevId;
  STC_UPDATE_IN_INFO_T  rIn;
  STC_UPDATE_OUT_INFO_T rOut;
} STC_UPDATE_INFO_T32;

#define IOCTL_STCDEV_GETTIME32   _IOR(STC_IOCTL_MAGIC, 2, STC_SGET_TIME_T32)
#define IOCTL_STCDEV_SETTIME32   _IOW(STC_IOCTL_MAGIC, 3, STC_SGET_TIME_T32)
#define IOCTL_STCDEV_GETSTATUS32 _IOR(STC_IOCTL_MAGIC, 4, STC_SGET_STATUS_T32)
#define IOCTL_STCDEV_SETSTATUS32 _IOW(STC_IOCTL_MAGIC, 5, STC_SGET_STATUS_T32)
#define IOCTL_STCDEV_SETRATE32   _IOW(STC_IOCTL_MAGIC, 6, STC_SGET_RATE_T32)
#define IOCTL_STCDEV_UPDATE32    _IOWR(STC_IOCTL_MAGIC, 7, STC_UPDATE_INFO_T32)

static inline int get_stc_settime32(STC_SGET_TIME_T *kp,
  STC_SGET_TIME_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_SGET_TIME_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId) ||
		get_user(kp->u8TimeVal, &up->u8TimeVal))
			return -EFAULT;

	return 0;
}

static inline int get_stc_gettime32(STC_SGET_TIME_T *kp,
  STC_SGET_TIME_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_SGET_TIME_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId))
			return -EFAULT;

	return 0;
}

static inline int put_stc_gettime32(STC_SGET_TIME_T *kp,
  STC_SGET_TIME_T32 __user *up)
{
	if (!access_ok(VERIFY_WRITE, up, sizeof(STC_SGET_TIME_T32)) ||
		put_user(kp->u8TimeVal, &up->u8TimeVal))
			return -EFAULT;

	return 0;
}

static inline int get_stc_setstatus32(STC_SGET_STATUS_T *kp,
  STC_SGET_STATUS_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_SGET_STATUS_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId) ||
		get_user(kp->eStatus, &up->eStatus))
			return -EFAULT;

	return 0;
}

static inline int get_stc_getstatus32(STC_SGET_STATUS_T *kp,
  STC_SGET_STATUS_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_SGET_STATUS_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId))
			return -EFAULT;

	return 0;
}

static inline int put_stc_getstatus32(STC_SGET_STATUS_T *kp,
  STC_SGET_STATUS_T32 __user *up)
{
	if (!access_ok(VERIFY_WRITE, up, sizeof(STC_SGET_STATUS_T32)) ||
		put_user(kp->eStatus, &up->eStatus))
			return -EFAULT;

	return 0;
}

static inline int get_stc_setrate32(STC_SGET_RATE_T *kp,
  STC_SGET_RATE_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_SGET_STATUS_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId) ||
		get_user(kp->u4Rate, &up->u4Rate) ||
		get_user(kp->is_slow_down, &up->is_slow_down))
			return -EFAULT;

	return 0;
}

static inline int get_stc_updateinfo32(STC_UPDATE_INFO_T *kp,
  STC_UPDATE_INFO_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_UPDATE_INFO_T32)) ||
		get_user(kp->u4DevId, &up->u4DevId) ||
		get_user(kp->rIn.i8BaseTime, &up->rIn.i8BaseTime) ||
		get_user(kp->rIn.i8StartTime, &up->rIn.i8StartTime) ||
		get_user(kp->rIn.u8StartMediaTime, &up->rIn.u8StartMediaTime))
			return -EFAULT;

	return 0;
}

static inline int put_stc_updateinfo32(STC_UPDATE_INFO_T *kp,
  STC_UPDATE_INFO_T32 __user *up)
{
	if (!access_ok(VERIFY_READ, up, sizeof(STC_UPDATE_INFO_T32)) ||
		put_user(kp->rOut.i8BaseTime, &up->rOut.i8BaseTime) ||
		put_user(kp->rOut.u8STCTime, &up->rOut.u8STCTime))
			return -EFAULT;

	return 0;
}

static long stcdev_native_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	if (file->f_op->unlocked_ioctl)
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);

	return ret;
}

static long stcdev_compat32_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  union {
    STC_SGET_TIME_T rTime;
    STC_SGET_STATUS_T rStatus;
    STC_SGET_RATE_T rRate;
    STC_UPDATE_INFO_T rUpdateInfo;
    __u32 u4DevId;
  } karg;

	void __user *up = compat_ptr(arg);
	long ret = 0;
	int compatible_arg = 1;

	if (!file->f_op->unlocked_ioctl)
		return ret;

	if (_IOC_TYPE(cmd) != 'S') {
		pr_err("%s:%s:%d. fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
            FILE_ONLY, __func__, __LINE__,
		       _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return ret;
	}

	switch (cmd) {
  case IOCTL_STCDEV_ACQUIRE:
  case IOCTL_STCDEV_RELEASE:
    break;
	case IOCTL_STCDEV_GETTIME32:
		cmd = IOCTL_STCDEV_GETTIME;
		break;
	case IOCTL_STCDEV_SETTIME32:
    cmd = IOCTL_STCDEV_SETTIME;
    break;
	case IOCTL_STCDEV_GETSTATUS32:
		cmd = IOCTL_STCDEV_GETSTATUS;
		break;
	case IOCTL_STCDEV_SETSTATUS32:
    cmd = IOCTL_STCDEV_SETSTATUS;
    break;
	case IOCTL_STCDEV_SETRATE32:
		cmd = IOCTL_STCDEV_SETRATE;
		break;
	case IOCTL_STCDEV_UPDATE32:
    cmd = IOCTL_STCDEV_UPDATE;
    break;
	default:
		pr_err("%s:%s:%d. fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
            FILE_ONLY, __func__, __LINE__,
		       _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case IOCTL_STCDEV_ACQUIRE:
	case IOCTL_STCDEV_RELEASE:
		break;
	case IOCTL_STCDEV_GETTIME:
		ret = get_stc_gettime32(&(karg.rTime), up);
		compatible_arg = 0;
    break;
  case IOCTL_STCDEV_SETTIME:
		ret = get_stc_settime32(&(karg.rTime), up);
		compatible_arg = 0;
    break;
	case IOCTL_STCDEV_GETSTATUS:
    ret = get_stc_getstatus32(&(karg.rStatus), up);
		compatible_arg = 0;
    break;
	case IOCTL_STCDEV_SETSTATUS:
    ret = get_stc_setstatus32(&(karg.rStatus), up);
		compatible_arg = 0;
    break;
	case IOCTL_STCDEV_SETRATE:
    ret = get_stc_setrate32(&(karg.rRate), up);
		compatible_arg = 0;
    break;
	case IOCTL_STCDEV_UPDATE:
    ret = get_stc_updateinfo32(&(karg.rUpdateInfo), up);
		compatible_arg = 0;
    break;
  default:
		pr_err("%s:%s:%d. fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
            FILE_ONLY, __func__, __LINE__,
		    _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	if (0 != ret) {
		pr_err("%s:%s:%d. fail in get args for ioctl '%c', dir=%d, #%d (0x%08x), ret = %ld\n",
            FILE_ONLY, __func__, __LINE__,
		    _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, ret);
		return ret;
	}

	if (compatible_arg)
		ret = stcdev_native_ioctl(file, cmd, (unsigned long)up);
	else {
		mm_segment_t old_fs = get_fs();

		set_fs(KERNEL_DS);
		ret = stcdev_native_ioctl(file, cmd, (unsigned long)&karg);
		set_fs(old_fs);
	}

	if (0 != ret) {
		pr_err("%s:%s:%d. fail in stcdev_native_ioctl for ioctl '%c', dir=%d, #%d (0x%08x), ret = %ld\n",
            FILE_ONLY, __func__, __LINE__,
		    _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, ret);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case IOCTL_STCDEV_ACQUIRE:
	case IOCTL_STCDEV_RELEASE:
    break;
  case IOCTL_STCDEV_GETTIME:
    ret = put_stc_gettime32(&(karg.rTime), up);
    break;
  case IOCTL_STCDEV_SETTIME:
    break;
  case IOCTL_STCDEV_GETSTATUS:
    ret = put_stc_getstatus32(&(karg.rStatus), up);
    break;
  case IOCTL_STCDEV_SETSTATUS:
    break;
  case IOCTL_STCDEV_SETRATE:
    break;
  case IOCTL_STCDEV_UPDATE:
    ret = put_stc_updateinfo32(&(karg.rUpdateInfo), up);
    break;
  default:
		pr_err("%s:%s:%d. fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
            FILE_ONLY, __func__, __LINE__,
		    _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	if (0 != ret) {
		pr_err("%s:%s:%d. fail in pet args for ioctl '%c', dir=%d, #%d (0x%08x), ret = %ld\n",
            FILE_ONLY, __func__, __LINE__,
		    _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, ret);
		return ret;
	}

	return ret;
}

#endif				/* CONFIG_COMPAT */

static int stcdev_mmap(struct file *fp, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return 0;
}

const struct file_operations stcdev_fops = {
	.release = stcdev_release,
	.open = stcdev_open,
	.mmap = stcdev_mmap,
	.unlocked_ioctl = stcdev_ioctl,
#if CONFIG_COMPAT
	.compat_ioctl = stcdev_compat32_ioctl,
#endif
};

static int stc_remove(struct platform_device *pdev);
static int stc_probe(struct platform_device *pdev);

int stc_suspend(struct device *dev)
{
	return 0;
}

int stc_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops stc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(stc_suspend, stc_resume)
};

static int stc_probe(struct platform_device *pdev)
{
	stcdev *stc_dev;
	int result = -1;

	pr_debug("stc_probe enter!!\r\n");

	stc_dev = kzalloc(sizeof(stcdev), GFP_KERNEL);
	if (!stc_dev) {
		result = -ENOMEM;
		goto err_free_mem;
	}
	memset(stc_dev, 0, sizeof(stcdev));

	stc_dev->dev = &(pdev->dev);
	stc_dev->cdev.name = "stcdev";
	stc_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	stc_dev->cdev.fops = &stcdev_fops;

	platform_set_drvdata(pdev, stc_dev);

	if (!stcdevs_init()) {
		pr_err("%s:%s:%d. stc_probe fail in stcdevs_init!\r\n",FILE_ONLY, __func__, __LINE__);
		result = -1;
		goto err_unset_drvdata;
	}

	result = misc_register(&(stc_dev->cdev));

	if (result == 0)
		pr_debug("stc dev init successes\n");
	else {
		pr_err("%s:%s:%d. stc dev init error\n",FILE_ONLY, __func__, __LINE__);
		goto err_unset_drvdata;
	}

	pr_info("stc_probe ok!!\r\n");

	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_mem:
	kfree(stc_dev);

	return result;
}

static int stc_remove(struct platform_device *pdev)
{
	stcdev *stc_dev = platform_get_drvdata(pdev);

	stcdevs_deinit();

	misc_deregister(&(stc_dev->cdev));

	platform_set_drvdata(pdev, NULL);

	kfree(stc_dev);

	return 0;
}

void __iomem *stc_base_regs = NULL;

static const struct of_device_id stc_of_ids[] = {
	{.compatible = "atc,stc",},
	{}
};

static struct platform_driver stc_of_driver = {
	.driver = {.name = "ac83xx_stc",
		   .owner = THIS_MODULE,
		   .pm = &stc_pm_ops,
		   .of_match_table = stc_of_ids,
		   },
	.probe = stc_probe,
	.remove = stc_remove,
};

static int __init stc_init(void)
{
	struct device_node *node = NULL;
	int result = 0;

	pr_debug("stc init enter!!\r\n");

	node = of_find_compatible_node(NULL, NULL, "atc,stc");
	if (!node) {
		pr_debug("stc_init fail in get stc driver dts compatible node!!\r\n");
		result = -ENOMEM;
		return result;
	}

	stc_base_regs = of_iomap(node, 0);
	if (!stc_base_regs) {
		pr_debug("stc_init fail in unable to iomap stc base registers!!\r\n");
		result = -ENOMEM;
		return result;
	}

	result = platform_driver_register(&stc_of_driver);
	if (result) {
		pr_err("%s:%s:%d. stc_init fail in platform_driver_register, error = %d\r\n",
            FILE_ONLY, __func__, __LINE__, result);
		return result;
	}

	MOD_VERSION_INFO(STCDRV_MODE_NAME, STCDRV_VER_MAJOR, STCDRV_VER_MINOR, STCDRV_VER_REV);

	pr_info("stc device init success!!\r\n");

	return 0;
}

static void __exit stc_exit(void)
{
	pr_info("stc_exit enter!!\r\n");
	platform_driver_unregister(&stc_of_driver);
	pr_info("stc_exit exit!!\r\n");
}
module_init(stc_init);
module_exit(stc_exit);

MODULE_AUTHOR("Autochips");
MODULE_DESCRIPTION("ATC ac83xx Demuxer Driver");
MODULE_LICENSE("GPL");

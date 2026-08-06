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

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <media/atc/ioctl_stc.h>
#include "stc_os.h"
#include "stc_hal.h"


typedef struct stchwdev {
	u32 id;
	unsigned long flags;
	u32 refcount;
	spinlock_t lock;
} STC_HW_DEV_T;

typedef struct stchwdevman {
	STC_HW_DEV_T arDevs[MAX_OF_STC_DEV_CNT];
	struct semaphore sema;
} STC_HW_DEV_MAN_T;

typedef struct {
	u32 u4DevId;
	bool is_active_acquire;
	struct semaphore sema;
} STC_HW_INST_T;

STC_HW_DEV_MAN_T g_rStcDevManager;

bool stcdevs_init(void)
{
	u32 u4Idx = 0;

	if (!STC_HalInit()) {
		pr_err("%s:%s:%d.fail for stc hal initialization\r\n",FILE_ONLY, __func__, __LINE__);
		return false;
	}

	init_MUTEX(&(g_rStcDevManager.sema));

	for (u4Idx = 0; u4Idx < MAX_OF_STC_DEV_CNT; u4Idx++) {
		g_rStcDevManager.arDevs[u4Idx].id = u4Idx;
		g_rStcDevManager.arDevs[u4Idx].refcount = 0;
		g_rStcDevManager.arDevs[u4Idx].flags = 0;
		spin_lock_init(&(g_rStcDevManager.arDevs[u4Idx].lock));
	}
	pr_info("%s success\r\n", __func__);

	return true;
}


bool stcdevs_deinit(void)
{
	STC_HalDeinit();
	pr_info("%s success\r\n", __func__);

	return true;
}

u32 stcdevs_acquiredev(void)
{
	u32 u4Idx = 0;

	down(&(g_rStcDevManager.sema));
	for (u4Idx = 0; u4Idx < MAX_OF_STC_DEV_CNT; u4Idx++) {
        pr_info("%s [%d]ref count is %d\r\n",
            __func__, u4Idx, g_rStcDevManager.arDevs[u4Idx].refcount);
		if (0 == g_rStcDevManager.arDevs[u4Idx].refcount) {
			g_rStcDevManager.arDevs[u4Idx].refcount++;
			pr_info("%s in line %d dev %d ref count is %d\r\n",
        		__func__, __LINE__, u4Idx, g_rStcDevManager.arDevs[u4Idx].refcount);
			break;
		}
	}
	up(&(g_rStcDevManager.sema));

    pr_info("%s acquire %d\r\n", __func__, u4Idx);

	return u4Idx;
}

bool stcdevs_occupydev(u32 u4DevId)
{
	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. exit for stc device id is invalid\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return false;
	}
	down(&(g_rStcDevManager.sema));
	g_rStcDevManager.arDevs[u4DevId].refcount++;
	pr_info("%s in line %d dev %d ref count is %d\r\n",
        __func__, __LINE__, u4DevId, g_rStcDevManager.arDevs[u4DevId].refcount);
	up(&(g_rStcDevManager.sema));
	return true;
}

long stcdevs_releasedev(u32 u4DevId)
{
	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. exit for stc device id is invalid\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return 0;
	}

	down(&(g_rStcDevManager.sema));
	if (0 == g_rStcDevManager.arDevs[u4DevId].refcount) {
		pr_err("%s:%s:%d. fail for stc device(id: %d) has been already released\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId);
		up(&(g_rStcDevManager.sema));
		return -EINVAL;
	}

	g_rStcDevManager.arDevs[u4DevId].refcount--;
    pr_info("%s success and dev %d ref count is %d\r\n",
        __func__, u4DevId, g_rStcDevManager.arDevs[u4DevId].refcount);

	up(&(g_rStcDevManager.sema));

	return 0;
}

void stcdevs_lock(u32 u4DevId)
{
	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid args\r\n",FILE_ONLY, __func__, __LINE__);
		return;
	}

	spin_lock_irqsave(&(g_rStcDevManager.arDevs[u4DevId].lock),
			  g_rStcDevManager.arDevs[u4DevId].flags);
}

void stcdevs_unlock(u32 u4DevId)
{
	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return;
	}

	spin_unlock_irqrestore(&(g_rStcDevManager.arDevs[u4DevId].lock),
			       g_rStcDevManager.arDevs[u4DevId].flags);
}

void *stc_inst_create(void)
{
	STC_HW_INST_T *prInst = vmalloc(sizeof(STC_HW_INST_T));

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for no memory\r\n", FILE_ONLY, __func__, __LINE__);
		return NULL;
	}

	memset(prInst, 0, sizeof(STC_HW_INST_T));

	prInst->u4DevId = (u32) MAX_OF_STC_DEV_CNT;
	prInst->is_active_acquire = false;

	init_MUTEX(&(prInst->sema));
	pr_info("%s exit, Instance is %p\r\n", __func__, prInst);

	return prInst;
}

long stc_inst_release(void *pvInst)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	long err = 0;

	pr_info("%s enter, Instance is %p\r\n", __func__, prInst);

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid args\r\n",FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	err = stc_inst_release_dev(prInst, prInst->u4DevId);
	if (err != 0) {
		pr_err("%s:%s:%d. fail in release stc device, err: %ld\r\n",
		       FILE_ONLY, __func__, __LINE__, err);
	}

	vfree(prInst);
	pr_info("%s exit\r\n", __func__);

	return err;
}

void stc_inst_lock(STC_HW_INST_T *prInst)
{
	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return;
	}

	down(&(prInst->sema));
}

void stc_inst_unlock(STC_HW_INST_T *prInst)
{
	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return;
	}

	up(&(prInst->sema));
}

long stc_inst_acquire_dev(void *pvInst, __u32 *pu4DevId)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long err = 0;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == pu4DevId) {
		pr_err("%s:%s:%d. fail for invalid args\r\n",FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	stc_inst_lock(prInst);

	if (prInst->u4DevId < MAX_OF_STC_DEV_CNT) {
		pr_err("%s:%s:%d. exit for this stc instance already obtain dev(id: %p)\r\n",
		       FILE_ONLY, __func__, __LINE__, prInst->u4DevId);
		stc_inst_unlock(prInst);
		return 0;
	}
	stc_inst_unlock(prInst);

	u4DevId = stcdevs_acquiredev();

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail in stcdevs acquire device\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EBUSY;
	}
	stc_inst_lock(prInst);
	prInst->is_active_acquire = true;
	prInst->u4DevId = u4DevId;
	*pu4DevId = u4DevId;
	stc_inst_unlock(prInst);
	pr_info("%s exit success and dev id is %d, Instance is %p\r\n", __func__, u4DevId, prInst);

	return 0;
}

long stc_inst_release_dev(void *pvInst, __u32 u4DevId)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	long err = 0;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. Fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
	pr_info("%s enter,Instance is %p\r\n", __func__,prInst);

	stc_inst_lock(prInst);
	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		pr_debug("%s:%s:%d. exit for this stc instance hasn't occupied any device\r\n",
		     FILE_ONLY, __func__, __LINE__);
		stc_inst_unlock(prInst);
		return 0;
	}

	if (prInst->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId, prInst->u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	prInst->u4DevId = MAX_OF_STC_DEV_CNT;
	prInst->is_active_acquire = false;
	stc_inst_unlock(prInst);

	err = stcdevs_releasedev(u4DevId);
	if (err != 0) {
		pr_err("%s:%s:%d. fail in stcdevs release device(id: %d), err: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId, err);
		return err;
	}
	pr_info("%s exit success, Instance is %p\r\n", __func__,prInst);

	return err;
}

long stc_inst_get_time(void *pvInst, STC_SGET_TIME_T *prTimeVal)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long err = 0;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == prTimeVal) {
		pr_err("%s:%s:%d. fail for invalid args\r\n",FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prTimeVal->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prTimeVal->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prTimeVal->u4DevId);
		stc_inst_lock(prInst);
	}

	u4DevId = prInst->u4DevId;

	if (prTimeVal->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prTimeVal->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}
	stc_inst_unlock(prInst);
#else
	u4DevId = prTimeVal->u4DevId;
#endif
	stcdevs_lock(u4DevId);

	if (!STC_HalGetTime(u4DevId, &(prTimeVal->u8TimeVal))) {
		pr_err("%s:%s:%d. fail in stc hal get time, devid: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId);
		stcdevs_unlock(u4DevId);
		return -EACCES;
	}

	stcdevs_unlock(u4DevId);

	return 0;
}

long stc_inst_set_time(void *pvInst, STC_SGET_TIME_T *prTimeVal)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long err = 0;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == prTimeVal) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prTimeVal->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prTimeVal->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prTimeVal->u4DevId);
		stc_inst_lock(prInst);
	}

	u4DevId = prInst->u4DevId;

	if (prTimeVal->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prTimeVal->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	stc_inst_unlock(prInst);
#else
	u4DevId = prTimeVal->u4DevId;
#endif
	stcdevs_lock(u4DevId);

	if (!STC_HalSetTime(u4DevId, prTimeVal->u8TimeVal)) {
		pr_err("%s:%s:%d. fail in stc hal set time, devid: %d, time: 0x%llx\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId, prTimeVal->u8TimeVal);
		stcdevs_unlock(u4DevId);
		return -EACCES;
	}

	stcdevs_unlock(u4DevId);

	return 0;
}

long stc_inst_get_status(void *pvInst, STC_SGET_STATUS_T *prStatus)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	STC_STATUS_T eStcStatus = STC_PAUSE;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;
	long err = 0;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == prStatus) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prStatus->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n",FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prStatus->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prStatus->u4DevId);
		stc_inst_lock(prInst);
  	}

	u4DevId = prInst->u4DevId;

	if (prStatus->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prStatus->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	stc_inst_unlock(prInst);
#else
	u4DevId = prStatus->u4DevId;
#endif

	stcdevs_lock(u4DevId);

	if (!STC_HalGetStatus(u4DevId, &(prStatus->eStatus))) {
		pr_err("%s:%s:%d. fail in stc hal get status, devid: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, u4DevId);
		stcdevs_unlock(u4DevId);
		return -EACCES;
	}

	stcdevs_unlock(u4DevId);

	return 0;
}

long stc_inst_set_status(void *pvInst, STC_SGET_STATUS_T *prStatus)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
	if (NULL == prStatus) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prStatus->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prStatus->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prStatus->u4DevId);
		stc_inst_lock(prInst);
	}

	u4DevId = prInst->u4DevId;

	if (prStatus->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       __func__, __LINE__, prStatus->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	stc_inst_unlock(prInst);
#else
	u4DevId = prStatus->u4DevId;
#endif

	stcdevs_lock(u4DevId);

	switch (prStatus->eStatus) {
	case STC_RUN:
		if (!STC_HalStart(u4DevId)) {
			pr_err("%s:%s:%d. fail in stc hal start, devid: %d\r\n",
			       FILE_ONLY, __func__, __LINE__, u4DevId);
			stcdevs_unlock(u4DevId);
			return -EACCES;
		}
		pr_debug("%s:%s:%d. stc hal start success, devid: %d\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		break;
	case STC_PAUSE:
		if (!STC_HalPause(u4DevId)) {
			pr_err("%s:%s:%d. fail in stc hal pause, devid: %d\r\n",
			       FILE_ONLY, __func__, __LINE__, u4DevId);
			stcdevs_unlock(u4DevId);
			return -EACCES;
		}
		pr_debug("%s:%s:%d. stc hal pause success, devid: %d\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		break;
	default:
		pr_err("%s:%s:%d. fail for invalid stc status(%d) to set, devid: %d\r\n",
		       FILE_ONLY, __func__, __LINE__, prStatus->eStatus, u4DevId);
		stcdevs_unlock(u4DevId);
		return -EINVAL;
	}

	stcdevs_unlock(u4DevId);

	return 0;
}

long stc_inst_set_rate(void *pvInst, STC_SGET_RATE_T *prRate)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == prRate) {
		pr_err("%s:%s:%d.fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prRate->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prRate->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prRate->u4DevId);
		stc_inst_lock(prInst);
	}

	u4DevId = prInst->u4DevId;

	if (prRate->u4DevId != u4DevId) {
		pr_err("%s:%s:%d.fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prRate->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	stc_inst_unlock(prInst);
#else
	u4DevId = prRate->u4DevId;
#endif

	stcdevs_lock(u4DevId);

	switch (prRate->u4Rate) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
		break;
	default:
		pr_err("%s:%s:%d. fail for invalid rate(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prRate->u4Rate);
		stcdevs_unlock(u4DevId);
		return -EINVAL;
	}

	if (!STC_HalSetRate(u4DevId, prRate->u4Rate, prRate->is_slow_down)) {
		pr_err
		    ("%s:%s:%d. fail in stc hal set rate, devid: %d, rate: %d, is_slow_down: %s\r\n",
		        FILE_ONLY, __func__, __LINE__, u4DevId, prRate->u4Rate,
		     (prRate->is_slow_down ? "true" : "false"));
		stcdevs_unlock(prInst);
		return -EACCES;
	}

	stcdevs_unlock(u4DevId);
	return 0;
}

long stc_inst_update(void *pvInst, STC_UPDATE_INFO_T *prInfo)
{
	STC_HW_INST_T *prInst = (STC_HW_INST_T *) pvInst;
	u32 u4DevId = MAX_OF_STC_DEV_CNT;

	if (NULL == prInst) {
		pr_err("%s:%s:%d. fail for invalid stc instance\r\n",
		       FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (NULL == prInfo) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}

	if (MAX_OF_STC_DEV_CNT <= prInfo->u4DevId) {
		pr_err("%s:%s:%d. fail for invalid device id\r\n",
            FILE_ONLY, __func__, __LINE__);
		return -EINVAL;
	}
#if 0
	stc_inst_lock(prInst);

	if (MAX_OF_STC_DEV_CNT <= prInst->u4DevId) {
		prInst->u4DevId = prInfo->u4DevId;
		stc_inst_unlock(prInst);
		pr_info("%s occupydev, Instance is %p\r\n", __func__, prInst);
		stcdevs_occupydev(prInfo->u4DevId);
		stc_inst_lock(prInst);
	}

	u4DevId = prInst->u4DevId;

	if (prInfo->u4DevId != u4DevId) {
		pr_err("%s:%s:%d. fail for device id(%d) != inst's stc device id(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, prInfo->u4DevId, u4DevId);
		stc_inst_unlock(prInst);
		return -EINVAL;
	}

	stc_inst_unlock(prInst);
#else
	u4DevId = prInfo->u4DevId;
#endif

	stcdevs_lock(u4DevId);

	if (!STC_HalUpdate(u4DevId, &(prInfo->rIn), &(prInfo->rOut))) {
		pr_err("%s:%s:%d. fail in stc hal set rate, devid: %d\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		stcdevs_unlock(u4DevId);
		return -EACCES;
	}

	stcdevs_unlock(u4DevId);

	return 0;
}

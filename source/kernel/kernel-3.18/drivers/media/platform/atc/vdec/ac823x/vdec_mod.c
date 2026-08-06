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

#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/list.h>

#include <media/atc/ose_mem.h>
#include <media/atc/mm_debug.h>
#include <media/atc/vdec_init.h>
#include <media/atc/drv_vdec.h>
//#include "irqs.h"

#include "assert.h"

#include "x_os.h"
//#include "x_ckgen.h"

#include "windows.h"
#include "x_debug.h"
#include "x_ver.h"

#include "soc_cfg.h"
#include "mmisc.h"
#include "oal.h"
#include "vdec_mod.h"
#include "ioctl_vdec.h"

#ifdef CONFIG_COMPAT
#include <linux/uaccess.h>
#include <linux/compat.h>
#endif

#ifdef MPV_MAX_ES
#undef MPV_MAX_ES
#define MPV_MAX_ES 2
#endif
VDEC_PM_STATE g_VdecPmState = PM_POWER_ON;
struct vdec_dev_info *vdec_dev = NULL;

static HANDLE_T _arVDecEventInfo[MPV_MAX_ES];
static s32 i4Driver_Open_Count;         /* mutex : DriverOpenCountLock */
static struct semaphore HwStatusSemphore;

#define INVALID_VDEC_HW_ID (u32)(-1)
static bool VdecHWUsed[MPV_MAX_ES];

extern void mt33xx_mask_ack_bim_irq(unsigned int virq);


typedef struct vdec_reserved_memory_resource
{
	struct list_head list;
	VAL_MEMORY_T rMemInfo;    
} vdec_reserved_memory_resource;

// lock
static DEFINE_MUTEX(DriverOpenCountLock);
static DEFINE_MUTEX(MemoryTableLock);
static DEFINE_MUTEX(HwStatusLock);
static LIST_HEAD(vdec_reserved_memory_resource_list);

static bool RequestHW(u32 *pIdleHwID)
{
	u32 u4HwId = 0;

	if (NULL == pIdleHwID)
	{
		return FALSE;
	}
	
	down(&HwStatusSemphore);
	
	mutex_lock(&HwStatusLock);
	while(u4HwId < MPV_MAX_ES)
	{
		if (VdecHWUsed[u4HwId] == FALSE)
		{
			VdecHWUsed[u4HwId] = TRUE;
			*pIdleHwID = u4HwId;
			mutex_unlock(&HwStatusLock);
			return TRUE;
		}
        u4HwId++;
	}
	*pIdleHwID = INVALID_VDEC_HW_ID;
	mutex_unlock(&HwStatusLock);
	
	return FALSE;
}

static void ReleaseHW(u32 u4HwId)
{
	up(&HwStatusSemphore);
	
	if (u4HwId < MPV_MAX_ES)
	{
		mutex_lock(&HwStatusLock);
		VdecHWUsed[u4HwId] = FALSE;
		mutex_unlock(&HwStatusLock);
	}
	else
	{
		pr_alert("[%s]HW ID %d is error.\r\n", __FUNCTION__, u4HwId);
	}
}

static long videodecoder_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned char *user_data_addr = NULL;
	long ret = 0;
	EV_GRP_EVENT_T Vdec_Ev;
	s32 i4Ret = OSR_OK;
	VAL_MEMORY_T rTempMem;
	VDEC_DRV_INST *prVdecInst = NULL;

	if (_IOC_TYPE(cmd) != 'V') {
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
		       __func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -EFAULT;
	}
	
	prVdecInst =  (VDEC_DRV_INST*)(filp->private_data);

	switch (cmd) {
	case VCODEC_ALLOC_NON_CACHE_BUFFER:
		{
			vdec_reserved_memory_resource *pMemoryTable;
            
			user_data_addr = (unsigned char *)arg;
			ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
			if (ret) {
				pr_err("VCODEC_ALLOC_NON_CACHE_BUFFER, copy_from_user failed: %lu\r\n", ret);
				return -EFAULT;
			}

			rTempMem.pvMemVa = OSE_MemAllocCustom(OSE_VDEC,
							      rTempMem.u4MemSize,
							      rTempMem.u4Alignment,
							      (uintptr_t *) &(rTempMem.pvMemPa));
			rTempMem.u8RealMemPa = (uintptr_t)(rTempMem.pvMemPa); 
			if ((NULL == rTempMem.pvMemVa) || (NULL == rTempMem.pvMemPa)) {
				pr_err("VCODEC_ALLOC_NON_CACHE_BUFFER alloc memory fail. \r\n");
				return -ENOMEM;
			}

			pMemoryTable = kzalloc(sizeof(vdec_reserved_memory_resource), GFP_KERNEL);
			if (NULL == pMemoryTable)
			{
				return -ENOMEM;
			}
			memcpy(&pMemoryTable->rMemInfo, &rTempMem, sizeof(VAL_MEMORY_T));
			INIT_LIST_HEAD(&pMemoryTable->list);

			/* Add resource to global list */
			mutex_lock(&MemoryTableLock);
			list_add_tail(&pMemoryTable->list, &vdec_reserved_memory_resource_list);          
			mutex_unlock(&MemoryTableLock);

			pr_info("alloc kernel va = %p, kernel pa = %p, u8RealMemPa:0x%llx, memory size = 0x%x .\r\n",
			     rTempMem.pvMemVa, rTempMem.pvMemPa, rTempMem.u8RealMemPa,
			     (unsigned int)rTempMem.u4MemSize);

			ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
			if (ret) {
				pr_err("VCODEC_ALLOC_NON_CACHE_BUFFER, copy_to_user failed: %lu\r\n", ret);
				return -EFAULT;
			}
		}
		break;

	case VCODEC_FREE_NON_CACHE_BUFFER:
		{
			struct list_head *pos = NULL, *q = NULL;
			vdec_reserved_memory_resource *pMemoryTableEntry = NULL;
            
			user_data_addr = (unsigned char *)arg;

			ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
			if (ret) {
				pr_err("VCODEC_FREE_NON_CACHE_BUFFER, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}

			rTempMem.pvMemVa = OSE_PAToVA((uintptr_t)rTempMem.u8RealMemPa);
			pr_info("free kernel va = %p, kernel pa = %p, u8RealMemPa:0x%llx, memory size = 0x%x .\r\n",
			     rTempMem.pvMemVa, rTempMem.pvMemPa,rTempMem.u8RealMemPa,
			     (unsigned int)rTempMem.u4MemSize);
			OSE_MemFreeCustom(OSE_VDEC, rTempMem.pvMemVa);

			/* Delete resource from global list */
			mutex_lock(&MemoryTableLock);
			list_for_each_safe(pos, q, &vdec_reserved_memory_resource_list){
				pMemoryTableEntry = list_entry(pos, vdec_reserved_memory_resource, list);
				if (rTempMem.pvMemVa == pMemoryTableEntry->rMemInfo.pvMemVa)
				{
					list_del(pos);
					pMemoryTableEntry->rMemInfo.pvMemVa = NULL;
					pMemoryTableEntry->rMemInfo.pvMemPa = NULL;
					pMemoryTableEntry->rMemInfo.u8RealMemPa = NULL;
					kfree(pMemoryTableEntry);
					pMemoryTableEntry = NULL;
				}
			}
			mutex_unlock(&MemoryTableLock);

			rTempMem.pvMemVa = NULL;
			rTempMem.pvMemPa = NULL;
			rTempMem.u8RealMemPa = 0;

			ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
			if (ret) {
				pr_err("VCODEC_FREE_NON_CACHE_BUFFER, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}
		}
		break;

	case VCODEC_WAITISR:
		{
			/* wait isr */
			i4Ret = x_ev_group_wait_event_timeout(_arVDecEventInfo[prVdecInst->u4HwId],
							      VDEC_EVENT_DEC_END, &Vdec_Ev,
							      X_EV_OP_OR_CONSUME, 1000);
			if (i4Ret == OSR_TIMEOUT) {
				pr_debug("wait event timeout \r\n");
				return -EFAULT;
			}
		}
		break;

	case VCODEC_MB:
		{
			mb();
		}
		break;

	case VCODEC_FLUSH_DCACHE:
		{
			user_data_addr = (unsigned char *)arg;
			ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
			if (ret) {
				pr_err("VCODEC_FLUSH_DCACHE, copy_from_user failed: %lu\r\n", ret);
				return -EFAULT;
			}
			rTempMem.pvMemVa = OSE_PAToVA((uintptr_t)(rTempMem.u8RealMemPa));
			pr_debug("flush kernel va = %p, kernel pa = %p, u8RealMemPa:0x%llx, memory size = 0x%x .\r\n",
			     rTempMem.pvMemVa, rTempMem.pvMemPa,rTempMem.u8RealMemPa,
			     (unsigned int)rTempMem.u4MemSize);
			__flush_dcache_area(rTempMem.pvMemVa, rTempMem.u4MemSize);
		}
		break;
	case VCODEC_GET_DPB_SIZE:
		{
			unsigned int  u4Size = OSE_GetVdecBufSize();
			
			user_data_addr = (unsigned char *)arg;
			ret = copy_to_user(user_data_addr, &u4Size, sizeof(u4Size));
			if (ret) {
				pr_err("VCODEC_GET_DPB_SIZE, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}
		}
		break;

	case VCODEC_HW_INIT:
		{
			VDEC_CODEC_INFO_T rCodecInfo;

			memset(&rCodecInfo, 0, sizeof(VDEC_CODEC_INFO_T));

			user_data_addr = (unsigned char *)arg;

			ret = copy_from_user(&rCodecInfo, user_data_addr, sizeof(VDEC_CODEC_INFO_T));
			if (ret) {
				pr_err("VCODEC_HW_INIT, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			featureInit(rCodecInfo.u4ChipFeature, rCodecInfo.u4VDecID, rCodecInfo.u4CodeType, 0);
		}
		break;

		case VCODEC_POWERON_HW:
			{
				unsigned int u4HwId = 0;

				user_data_addr = (unsigned char *)arg;

				ret = copy_from_user(&u4HwId, user_data_addr, sizeof(u4HwId));
				if (ret) {
					pr_err("[%s:%s:%d]VCODEC_INIT_HW, copy_from_user failed: %lu\n", basename(__FILE__), __FUNCTION__, __LINE__, ret);
					return -EFAULT;
				}

				clk_prepare(vdec_dev->vdec_full_clk);
				clk_enable(vdec_dev->vdec_full_clk);
				clk_prepare(vdec_dev->vdec_full_clk1);
				clk_enable(vdec_dev->vdec_full_clk1);
				clk_set_parent(vdec_dev->vdec_topselect_clk, vdec_dev->vdec_topselect_clk_parent);
				//spm_power_vdec(1, 1);
				enable_irq(vdec_dev->vdful_irq);
				enable_irq(vdec_dev->vdful_irq1);
				pr_info("vdec power on ok");
			}
			break;

		case VCODEC_POWEROFF_HW:
			{
				unsigned int u4HwId = 0;

				user_data_addr = (unsigned char *)arg;

				ret = copy_from_user(&u4HwId, user_data_addr, sizeof(u4HwId));
				if (ret) {
					pr_err("[%s:%s:%d]VCODEC_UNINIT_HW, copy_from_user failed: %lu\n", basename(__FILE__), __FUNCTION__, __LINE__, ret);
					return -EFAULT;
				}
				disable_irq(vdec_dev->vdful_irq);
				disable_irq(vdec_dev->vdful_irq1);
				clk_disable(vdec_dev->vdec_full_clk);
				clk_unprepare(vdec_dev->vdec_full_clk);
				clk_disable(vdec_dev->vdec_full_clk1);
				clk_unprepare(vdec_dev->vdec_full_clk1);
				//spm_power_vdec(1, 0);
				pr_info("vdec power off ok");
			}
			break;
	case VCODEC_GET_SUPPORT_CODEC:
		{
			unsigned int u4Feature = 0, u4Support = 0;

			user_data_addr = (unsigned char *)arg;
			ret = copy_from_user(&u4Feature, user_data_addr, sizeof(u4Feature));
			if (ret) {
				pr_err("VCODEC_GET_SUPPORT_CODEC, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			u4Support = fgGetChipFeature(u4Feature);
			pr_debug("codec u4Feature: %u, u4Support:%u. \r\n", u4Feature, u4Support);

			ret = copy_to_user(user_data_addr, &u4Support, sizeof(u4Support));
			if (ret) {
				pr_err("VCODEC_GET_SUPPORT_CODEC, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}
		}
		break;
	case VCODEC_REQUEST_HW:
			{
				bool bRetOK = FALSE;
				u32  u4HwId = INVALID_VDEC_HW_ID;
				user_data_addr = (unsigned char *)arg;
		
				bRetOK = RequestHW(&u4HwId);
				if (bRetOK)
				{
					prVdecInst->u4HwId = u4HwId;
				}
		
				pr_debug("request idle hw id %u. \r\n", u4HwId);
		
				ret = copy_to_user(user_data_addr, &(prVdecInst->u4HwId), sizeof(prVdecInst->u4HwId));
				if (ret) {
					return -EFAULT;
				}
			}
			break;

	case VCODEC_RELEASE_HW:
			{
				ReleaseHW(prVdecInst->u4HwId);
				pr_debug("release hw id %u. \r\n", prVdecInst->u4HwId);
				prVdecInst->u4HwId = INVALID_VDEC_HW_ID;
			}
			break;

	default:
		pr_err("videodecoder_unlocked_ioctl vdec receive unknown ioctl code: 0x%x. \r\n", cmd);
		break;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
typedef enum {
	VAL_RPR_INFO = 0,
	VAL_MEMORY_TYPE
} STRUCT_TYPE;

typedef enum {
	COPY_FROM_USER = 0,
	COPY_TO_USER,
} COPY_DIRECTION;

typedef struct COMPAT_VAL_MEMORY {
	/* /< [IN]     The allocation memory type */
	compat_uint_t       eMemType;
	/* /< [IN]     The size of memory allocation */
	compat_uint_t       u4MemSize;
	/* /< [IN/OUT] The memory virtual address */
	compat_uptr_t       pvMemVa;
	/* /< [IN/OUT] The memory physical address */
	compat_uptr_t       pvMemPa;
	/* /< [IN]     The memory byte alignment setting */
	compat_uint_t       u4Alignment;
	compat_u64          u8RealMemPa;                   ///< [IN/OUT] The memory physical address
} COMPAT_VAL_MEMORY_T;

typedef struct _COMPAT_VDEC_CODEC_INFO {
    compat_uint_t u4VDecID;
    compat_uint_t u4ChipFeature;
    compat_uint_t u4CodeType;
    compat_uint_t u4Reserved;
} COMPAT_VDEC_CODEC_INFO_T;

#if 1
#define COMPAT_VCODEC_ALLOC_NON_CACHE_BUFFER      _IOWR(VCODEC_IOCTL_MAGIC, 0, COMPAT_VAL_MEMORY_T)
#define COMPAT_VCODEC_FREE_NON_CACHE_BUFFER       _IOWR(VCODEC_IOCTL_MAGIC, 1, COMPAT_VAL_MEMORY_T)
#define COMPAT_VCODEC_WAITISR                     _IO(VCODEC_IOCTL_MAGIC, 3)
#define COMPAT_VCODEC_MB                          _IO(VCODEC_IOCTL_MAGIC, 4)
#define COMPAT_VCODEC_GET_DPB_SIZE                _IOR(VCODEC_IOCTL_MAGIC, 5, __u32)
#define COMPAT_VCODEC_HW_INIT                     _IOR(VCODEC_IOCTL_MAGIC, 6, COMPAT_VDEC_CODEC_INFO_T)
#define COMPAT_VCODEC_GET_SUPPORT_CODEC           _IOWR(VCODEC_IOCTL_MAGIC, 7, __u32)
#define COMPAT_VCODEC_REQUEST_HW                  _IOR(VCODEC_IOCTL_MAGIC, 12, __u32)
#define COMPAT_VCODEC_RELEASE_HW                  _IO(VCODEC_IOCTL_MAGIC, 13)
#define COMPAT_VCODEC_FLUSH_DCACHE                _IOWR(VCODEC_IOCTL_MAGIC, 14, COMPAT_VAL_MEMORY_T)
#endif

static int get_uptr_to_32(compat_uptr_t *p, void __user **uptr)
{
	void __user *p2p;
	int err = get_user(p2p, uptr);
	*p = ptr_to_compat(p2p);
	return err;
}

static int compat_copy_struct(
	STRUCT_TYPE eType,
	COPY_DIRECTION eDirection,
	void __user *data32,
	void __user *data)
{
	compat_uint_t u;
	compat_uptr_t p;
	compat_u64    ll;
	int err = 0;

	switch (eType) {
	case VAL_MEMORY_TYPE: {
		if (eDirection == COPY_FROM_USER) {
			COMPAT_VAL_MEMORY_T __user *from32 = (COMPAT_VAL_MEMORY_T *)data32;
			VAL_MEMORY_T __user *to = (VAL_MEMORY_T *)data;

			err = get_user(u, &(from32->eMemType));
			err |= put_user(u, &(to->eMemType));
			err |= get_user(u, &(from32->u4MemSize));
			err |= put_user(u, &(to->u4MemSize));
			err |= get_user(p, &(from32->pvMemVa));
			err |= put_user(compat_ptr(p), &(to->pvMemVa));
			err |= get_user(p, &(from32->pvMemPa));
			err |= put_user(compat_ptr(p), &(to->pvMemPa));
			err |= get_user(u, &(from32->u4Alignment));
			err |= put_user(u, &(to->u4Alignment));
			err |= get_user(ll, &(from32->u8RealMemPa));
			err |= put_user(ll, &(to->u8RealMemPa));
			return err;
		} else {
			COMPAT_VAL_MEMORY_T __user *to32 = (COMPAT_VAL_MEMORY_T *)data32;
			VAL_MEMORY_T __user *from = (VAL_MEMORY_T *)data;

			err = get_user(u, &(from->eMemType));
			err |= put_user(u, &(to32->eMemType));
			err |= get_user(u, &(from->u4MemSize));
			err |= put_user(u, &(to32->u4MemSize));
			err |= get_uptr_to_32(&p, &(from->pvMemVa));
			err |= put_user(p, &(to32->pvMemVa));
			err |= get_uptr_to_32(&p, &(from->pvMemPa));
			err |= put_user(p, &(to32->pvMemPa));
			err |= get_user(u, &(from->u4Alignment));
			err |= put_user(u, &(to32->u4Alignment));
			err |= get_user(ll, &(from->u8RealMemPa));
			err |= put_user(ll, &(to32->u8RealMemPa));
		}
	}
	break;

	default:
		break;
	}

	return err;
}

static unsigned int vdec_cmd_switch(unsigned int cmd)
{
	unsigned int newCmd = 0;
	
	switch(cmd){
		case COMPAT_VCODEC_ALLOC_NON_CACHE_BUFFER:
			newCmd = VCODEC_ALLOC_NON_CACHE_BUFFER;
			break;
		case COMPAT_VCODEC_FREE_NON_CACHE_BUFFER:
			newCmd = VCODEC_FREE_NON_CACHE_BUFFER;
			break;
		case COMPAT_VCODEC_FLUSH_DCACHE:
			newCmd = VCODEC_FLUSH_DCACHE;
			break;
	default:
		newCmd = cmd;
		break;
	}

	return newCmd;
}

static long videodecoder_unlocked_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	unsigned int compat_cmd=0;

	if (!filp->f_op->unlocked_ioctl) {
		pr_err("unlocked_ioctl not implemented\n");
		return -ENOIOCTLCMD;
	}

	if (_IOC_TYPE(cmd) != 'V') {
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
	      __func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	pr_debug("videodecoder_unlocked_compat_ioctl: 0x%x\n", cmd);

	compat_cmd = vdec_cmd_switch(cmd);
	switch (compat_cmd) {
	case VCODEC_ALLOC_NON_CACHE_BUFFER:
    case VCODEC_FREE_NON_CACHE_BUFFER:
	case VCODEC_FLUSH_DCACHE:
		{
			COMPAT_VAL_MEMORY_T __user *data32;
			VAL_MEMORY_T __user *data;
			int err;

			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(VAL_MEMORY_T));
			if (data == NULL) {
				/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
				return -EFAULT;
			}

			err = compat_copy_struct(VAL_MEMORY_TYPE, COPY_FROM_USER, (void *)data32, (void *)data);
			if (err) {
				/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
				return err;
			}

			ret = filp->f_op->unlocked_ioctl(filp, compat_cmd, (unsigned long)data);
			
			err = compat_copy_struct(VAL_MEMORY_TYPE, COPY_TO_USER, (void *)data32, (void *)data);
			
			if (err) {
				/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
				return err;
			}
			return ret;
		}
		break;

	default:
		return videodecoder_unlocked_ioctl(filp, compat_cmd, arg);
	}

	return 0;
}
#endif

static int videodecoder_open(struct inode *inode, struct file *filp)
{
	VDEC_DRV_INST *prVdecInst = NULL;

	prVdecInst = (VDEC_DRV_INST *)vmalloc(sizeof(VDEC_DRV_INST));
	if(NULL != prVdecInst)
	{
	    memset(prVdecInst, 0, sizeof(VDEC_DRV_INST));
		prVdecInst->u4HwId = INVALID_VDEC_HW_ID;
	}
	else
	{
		pr_err(":videodecoder_open alloc memory fail! \r\n");
		return -ENOMEM;
	}

	mutex_lock(&DriverOpenCountLock);
	i4Driver_Open_Count++;

	pr_debug("videodecoder_open pid = %d, i4Driver_Open_Count %d\n", current->pid, i4Driver_Open_Count);
	mutex_unlock(&DriverOpenCountLock);

	pr_debug("video decoder open ok!\r\n");

	filp->private_data = (void*)prVdecInst;

	return 0;
}

static int videodecoder_flush(struct file *filp, fl_owner_t id)
{
	pr_debug("video decoder flush ok!\r\n");
	return 0;
}

static int videodecoder_release(struct inode *inode, struct file *filp)
{
	struct list_head *pos = NULL, *q = NULL;
	vdec_reserved_memory_resource *pMemoryTableEntry = NULL;
	VDEC_DRV_INST *prVdecInst = NULL;
	
	prVdecInst =  (VDEC_DRV_INST*)(filp->private_data);

	mutex_lock(&DriverOpenCountLock);

	pr_debug("videodecoder_release pid = %d, i4Driver_Open_Count %d\n", current->pid, i4Driver_Open_Count);
	i4Driver_Open_Count--;
	if (0 == i4Driver_Open_Count)
	{

		/* Delete resource from global list */
		mutex_lock(&MemoryTableLock);
		list_for_each_safe(pos, q, &vdec_reserved_memory_resource_list){
		    pMemoryTableEntry = list_entry(pos, vdec_reserved_memory_resource, list);
		    if (0 != pMemoryTableEntry->rMemInfo.pvMemVa)
		    {
				OSE_MemFreeCustom(OSE_VDEC, pMemoryTableEntry->rMemInfo.pvMemVa);
				pMemoryTableEntry->rMemInfo.pvMemVa = NULL;
				pMemoryTableEntry->rMemInfo.pvMemPa = NULL;
				pMemoryTableEntry->rMemInfo.u8RealMemPa = 0;
				list_del(pos);
				kfree(pMemoryTableEntry);
				pMemoryTableEntry = NULL;
		    }
		}
		mutex_unlock(&MemoryTableLock);
	}
	mutex_unlock(&DriverOpenCountLock);

    if (prVdecInst)
    {
        vfree(prVdecInst);
    }
	pr_debug("video decoder release ok!\r\n");

	return 0;
}

static void vdec_vma_open(struct vm_area_struct *vma)
{
	pr_debug("vdec_vma_open, virt 0x%x, phys 0x%x.\r\n",
		 (unsigned int)vma->vm_start, (unsigned int)(vma->vm_pgoff << PAGE_SHIFT));
}

static void vdec_vma_close(struct vm_area_struct *vma)
{
	pr_debug("vdec_vma_close, virt 0x%x, phys 0x%x.\r\n",
		 (unsigned int)vma->vm_start, (unsigned int)(vma->vm_pgoff << PAGE_SHIFT));
}

static struct vm_operations_struct vdec_remap_vm_ops = {
	.open = vdec_vma_open,
	.close = vdec_vma_close,
};

static int videodecoder_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long length = 0;

	length = vma->vm_end - vma->vm_start;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, length, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	vma->vm_ops = &vdec_remap_vm_ops;
	vdec_vma_open(vma);

	return 0;
}

const struct file_operations videodecoder_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = videodecoder_unlocked_ioctl,
	.open = videodecoder_open,
	.flush = videodecoder_flush,
	.release = videodecoder_release,
	.mmap = videodecoder_mmap,
#ifdef CONFIG_COMPAT
	.compat_ioctl = videodecoder_unlocked_compat_ioctl,
#endif
};

#if VDEC_PM_SUPPORT
static VDEC_PM_STATE set_vdec_state(VDEC_PM_STATE pm_state, struct vdec_dev_info *pvdec_dev)
{
	if (NULL == pvdec_dev){
		pr_err("[%s:%s:%d]invalid parameter  pvdec_dev", basename(__FILE__), __FUNCTION__, __LINE__);
		return INVALIED_PM_STATE;
	}

	pr_info("[%s:%s:%d] set_vdec_state enter\r\n", basename(__FILE__), __FUNCTION__, __LINE__);
	if(VALID_DX(pm_state))
	{
		switch (pm_state)
		{
			case PM_POWER_ON:        // Power Up
			{
				pr_info("[%s:%s:%d] pm_state: PM_POWER_ON\r\n", basename(__FILE__), __FUNCTION__, __LINE__);   
				clk_prepare(pvdec_dev->vdec_full_clk);
				clk_enable(pvdec_dev->vdec_full_clk);
				clk_prepare(pvdec_dev->vdec_full_clk1);
				clk_enable(pvdec_dev->vdec_full_clk1);
				clk_set_parent(pvdec_dev->vdec_topselect_clk, pvdec_dev->vdec_topselect_clk_parent);
				//spm_power_vdec(1, 1);
				enable_irq(pvdec_dev->vdful_irq);
				enable_irq(pvdec_dev->vdful_irq1);
				pr_info("[%s:%s:%d] vdec resume ok\r\n", basename(__FILE__), __FUNCTION__, __LINE__);
			}
				break;
			case PM_POWER_OFF:       // Power Off
			{
				pr_info("[%s:%s:%d] pm_state: PM_POWER_OFF\r\n", basename(__FILE__), __FUNCTION__, __LINE__);
				disable_irq(pvdec_dev->vdful_irq);
				disable_irq(pvdec_dev->vdful_irq1);
				clk_disable(pvdec_dev->vdec_full_clk);
				clk_unprepare(pvdec_dev->vdec_full_clk);
				clk_disable(pvdec_dev->vdec_full_clk1);
				clk_unprepare(pvdec_dev->vdec_full_clk1);
				//spm_power_vdec(1, 0);
				pr_info("[%s:%s:%d] vdec suspend ok\r\n", basename(__FILE__), __FUNCTION__, __LINE__);
			}
				break;
			default:
			pm_state = INVALIED_PM_STATE;
			break;
		}
		return pm_state;
	}
	pr_info("[vdec][%s:%s:%d]vdec power state error!\n", basename(__FILE__), __FUNCTION__, __LINE__);
	return INVALIED_PM_STATE;
}

static int videodecoder_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct vdec_dev_info *pvdec_dev = platform_get_drvdata(pdev);

	pr_info("[vdec][%s:%s:%d] videodecoder_suspend enter!\n", basename(__FILE__), __FUNCTION__, __LINE__);

	if (g_VdecPmState == PM_POWER_OFF){
	    return 0;
	}

	set_vdec_state(PM_POWER_OFF, pvdec_dev);
	g_VdecPmState = PM_POWER_OFF;

	return 0;
}

static int videodecoder_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct vdec_dev_info *pvdec_dev = platform_get_drvdata(pdev);

	pr_info("[vdec][%s:%s:%d] videodecoder_resume enter!\n", basename(__FILE__), __FUNCTION__, __LINE__);

	if (g_VdecPmState == PM_POWER_ON) {
	    return 0;
	}

	set_vdec_state(PM_POWER_ON, pvdec_dev);
	g_VdecPmState = PM_POWER_ON;

	return 0;
}

static struct dev_pm_ops videodecoder_pm_fops = {
	SET_SYSTEM_SLEEP_PM_OPS(videodecoder_suspend, videodecoder_resume)
};
#endif
static irqreturn_t ac83xx_vdec_decode_interrupt_hw(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct vdec_dev_info *vdec_dev = platform_get_drvdata(pdev);

	if (NULL == vdec_dev) {
		pr_err("fail for invalid drv data!\r\n");
		return IRQ_NONE;
	}

	x_ev_group_set_event(_arVDecEventInfo[0], VDEC_EVENT_DEC_END, X_EV_OP_OR);

	//ac83xx_mask_ack_bim_irq(vdec_dev->vdful_irq);
	mt33xx_mask_ack_bim_irq(vdec_dev->vdful_irq);

	return IRQ_HANDLED;
}

static irqreturn_t ac83xx_vdec_decode_interrupt_hw_1(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct vdec_dev_info *vdec_dev = platform_get_drvdata(pdev);

	if (NULL == vdec_dev) {
		pr_err("fail for invalid drv data!\r\n");
		return IRQ_NONE;
	}

	x_ev_group_set_event(_arVDecEventInfo[1], VDEC_EVENT_DEC_END, X_EV_OP_OR);

	//ac83xx_mask_ack_bim_irq(vdec_dev->vdful_irq1);
	mt33xx_mask_ack_bim_irq(vdec_dev->vdful_irq1);

	return IRQ_HANDLED;
}


static int videodecoder_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct vdec_dev_info *tempvdec_dev;
	int result = -1;

	pr_debug("enter video decoder probe\r\n");

	if (!node) {
		pr_err("video decoder probe fail because of no vdec device compatible dts node!\r\n");
		return -1;
	}

	tempvdec_dev = kzalloc(sizeof(struct vdec_dev_info), GFP_KERNEL);
	if (!tempvdec_dev) {
		result = -ENOMEM;
		pr_err("video decoder probe fail because of get alloc memory fail!\n");
		goto err_free_mem;
	}
	vdec_dev = tempvdec_dev;

	/* vdec full clk 0*/
	vdec_dev->vdec_full_clk = devm_clk_get(&pdev->dev, "vdec-fullclk");
	if (IS_ERR(vdec_dev->vdec_full_clk)) {
		pr_err("video decoder probe fail because of get vdec full clk 0 error!\n");
		goto err_free_mem;
	}
	clk_prepare(vdec_dev->vdec_full_clk);
	clk_enable(vdec_dev->vdec_full_clk);

	/* vdec full clk 1*/
	vdec_dev->vdec_full_clk1 = devm_clk_get(&pdev->dev, "vdec-fullclk1");
	if (IS_ERR(vdec_dev->vdec_full_clk1)) {
		pr_err("video decoder probe fail because of get vdec full clk 1 error!\n");
		goto err_free_mem;
	}
	clk_prepare(vdec_dev->vdec_full_clk1);
	clk_enable(vdec_dev->vdec_full_clk1);
	
	/* vdec top select clk 0*/
	vdec_dev->vdec_topselect_clk = devm_clk_get(&pdev->dev, "vdec-topselclk");
	if (IS_ERR(vdec_dev->vdec_topselect_clk)) {
		pr_err("video decoder probe fail because of get vdec top select clk 0 error!\n");
		goto err_free_mem;
	}

	/* vdec top select clk 1*/
	vdec_dev->vdec_topselect_clk1 = devm_clk_get(&pdev->dev, "vdec-topselclk1");
	if (IS_ERR(vdec_dev->vdec_topselect_clk1)) {
		pr_err("video decoder probe fail because of get vdec top select clk 1 error!\n");
		goto err_free_mem;
	}

	/* vdec top select parent setting */
	//vdec_dev->vdec_topselect_clk_parent = clk_get(NULL, "clk_apll2");
	vdec_dev->vdec_topselect_clk_parent = clk_get(NULL, "apll2_ck");
	result = clk_set_parent(vdec_dev->vdec_topselect_clk, vdec_dev->vdec_topselect_clk_parent);
	if (0 != result) {
		pr_err("video decoder probe fail because of clk_set_parent vdec top select clk 0 error!\n");
		goto err_free_mem;
	}

	result = clk_set_parent(vdec_dev->vdec_topselect_clk1, vdec_dev->vdec_topselect_clk_parent);
	if (0 != result) {
		pr_err("video decoder probe fail because of clk_set_parent vdec top select clk 1 error!\n");
		goto err_free_mem;
	}

	/* irq 0 */
	vdec_dev->vdful_irq = irq_of_parse_and_map(node, 0);
	if (vdec_dev->vdful_irq == -1) {
		pr_err("video decoder probe fail because of get vdec decode irq id!\n");
		goto err_free_mem;
	}

	/* irq 1 */
	vdec_dev->vdful_irq1 = irq_of_parse_and_map(node, 1);
	if (vdec_dev->vdful_irq1 == -1) {
		pr_err("video decoder probe fail because of get vdec decode irq id!\n");
		goto err_free_mem;
	}

	vdec_dev->dev = &(pdev->dev);
	vdec_dev->cdev.name = VIDEODECODER_DEVNAME;
	vdec_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	vdec_dev->cdev.fops = &videodecoder_fops;

	platform_set_drvdata(pdev, vdec_dev);

	result = misc_register(&(vdec_dev->cdev));
	if (result != 0) {
		pr_err("video decoder probe fail because of misc_register, error = %d\r\n", result);
		goto err_unset_drvdata;
	}

	result = request_irq(vdec_dev->vdful_irq, ac83xx_vdec_decode_interrupt_hw, 0, VIDEODECODER_DEVNAME, pdev);
	if (result) {
		pr_err("video decoder probe fail because of request decode irq(id: %d)\r\n", vdec_dev->vdful_irq);
		misc_deregister(&(vdec_dev->cdev));
		goto err_unset_drvdata;
	}
	disable_irq(vdec_dev->vdful_irq);

	result = request_irq(vdec_dev->vdful_irq1, ac83xx_vdec_decode_interrupt_hw_1, 0, VIDEODECODER_DEVNAME, pdev);
	if (result) {
		pr_err("video decoder probe fail because of request decode irq(id: %d)\r\n", vdec_dev->vdful_irq1);
		misc_deregister(&(vdec_dev->cdev));
		goto err_unset_drvdata;
	}
	disable_irq(vdec_dev->vdful_irq1);

	if (OSR_OK != x_ev_group_create(&_arVDecEventInfo[0], "VDEC0", 0)) {
		pr_err("video decoder create event hw 0 fail\r\n");
		free_irq(vdec_dev->vdful_irq, pdev);
		free_irq(vdec_dev->vdful_irq1, pdev);
		misc_deregister(&(vdec_dev->cdev));
		goto err_unset_drvdata;
	}

	if (OSR_OK != x_ev_group_create(&_arVDecEventInfo[1], "VDEC1", 0)) {
		pr_err("video decoder create event hw 1 fail\r\n");
		free_irq(vdec_dev->vdful_irq, pdev);
		free_irq(vdec_dev->vdful_irq1, pdev);
		misc_deregister(&(vdec_dev->cdev));
		goto err_unset_drvdata;
	}

	pr_alert("=====================================================================\r\n");
	MOD_VERSION_INFO(VDEC_MODE_NAME, VDEC_VER_MAJOR, VDEC_VER_MINOR, VDEC_VER_REV);
	pr_alert("=====================================================================\r\n");

	pr_debug("video decoder probe ok\r\n");
	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_mem:
	kfree(vdec_dev);
	vdec_dev = NULL;
	pr_err("video decoder probe fail\r\n");
	return result;
}

static int videodecoder_remove(struct platform_device *pdev)
{
	struct vdec_dev_info *vdec_dev = platform_get_drvdata(pdev);

	free_irq(vdec_dev->vdful_irq, pdev);
	free_irq(vdec_dev->vdful_irq1, pdev);

	clk_unprepare(vdec_dev->vdec_full_clk);
	clk_unprepare(vdec_dev->vdec_full_clk1);

	misc_deregister(&(vdec_dev->cdev));

	platform_set_drvdata(pdev, NULL);

	kfree(vdec_dev);
	vdec_dev = NULL;

	return 0;
}

static const struct of_device_id vdec_of_ids[] = {
	{.compatible = "atc,vdec",},
	{}
};

MODULE_DEVICE_TABLE(of, vdec_of_ids);

static struct platform_driver vdec_driver = {
	.driver = {.name = "ac83xx_videodecoder",
		   .owner = THIS_MODULE,
		   .pm = &videodecoder_pm_fops,
		   .of_match_table = vdec_of_ids,
		   },
	.probe = videodecoder_probe,
	.remove = videodecoder_remove,
};

static int __init videodecoder_init(void)
{
	struct device_node *node = NULL;
	int result = 0;

	pr_debug("enter video decoder init\r\n");

	mutex_lock(&DriverOpenCountLock);
	i4Driver_Open_Count = 0;
	mutex_unlock(&DriverOpenCountLock);
	
	sema_init(&HwStatusSemphore, 1);

	node = of_find_compatible_node(NULL, NULL, "atc,vdec");
	if (!node) {
		pr_err("video decoder init fail in find dts compatible node!!\r\n");
		result = -ENOMEM;
		goto err_node;
	}

	result = platform_driver_register(&vdec_driver);
	if (result) {
		pr_err("video decoder init fail in platform_driver_register, error = %d\r\n", result);
		goto err_node;
	}

	pr_debug("video decoder init ok!.\r\n");
	return 0;

err_node:

	return result;
}

static void __exit videodecoder_exit(void)
{
	pr_debug("enter video decoder exit!\r\n");

	platform_driver_unregister(&vdec_driver);

	pr_debug("video decoder exit ok!\r\n");
}
module_init(videodecoder_init);
module_exit(videodecoder_exit);

MODULE_AUTHOR("Autochips Inc");
MODULE_DESCRIPTION("ATC AC83xx Video Decode Driver");
MODULE_LICENSE("GPL");

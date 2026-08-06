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
#include <linux/compat.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <asm/uaccess.h>
#include <media/atc/ioctl_mmisc.h>
#include <media/atc/ose_mem.h>
#include "mmisc_compat32_ioctl.h"
#include "memmanager.h"

#if CONFIG_COMPAT

#define MMISC_IOCTL_MAGIC  'M'

typedef union {
  compat_caddr_t pvVirAddr;
  __u64 u8PhyAddr;
  __u32 size;
} OSE_MEM_GET_OUTINFO_T32;

typedef struct {
  E_OSE_MEM_GET_TYPE_T eType;
  OSE_MEM_GET_OUTINFO_T32 rOut;
} OSE_MEM_GET_INFO_T32;

typedef struct {
  compat_caddr_t pvAddr;
} MM_MEMDBG_INSTNODE_T32;

typedef struct {
  compat_caddr_t pvAddr;
  compat_caddr_t pvOutAddr;
} MM_MEMDBG_RMNODE_T32;

typedef struct {
  compat_caddr_t pvAddr;
  __u32 u4Size;
} MM_MEMDBG_GETNODESZ_T32;

#define IOCTL_MMISC_GET_OSEMEMIFNO32  _IOW(MMISC_IOCTL_MAGIC, 0, OSE_MEM_GET_INFO_T32)

#define IOCTL_MMISC_MEMDBG_INSTND32   _IOWR(MMISC_IOCTL_MAGIC, 10, MM_MEMDBG_INSTNODE_T32)
#define IOCTL_MMISC_MEMDBG_RMND32     _IOWR(MMISC_IOCTL_MAGIC, 11, MM_MEMDBG_RMNODE_T32)
#define IOCTL_MMISC_MEMDBG_GETSZ32    _IOWR(MMISC_IOCTL_MAGIC, 14, MM_MEMDBG_GETNODESZ_T32)

static long mmiscdev_getosememinfo32(void __user *arg)
{
	OSE_MEM_GET_INFO_T rInfo;
	OSE_MEM_GET_INFO_T32 *prArg32 = NULL;
	long ret = 0;

	if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(OSE_MEM_GET_INFO_T32))) {
		pr_err("%s line %d fail in access_ok(VERIFY_READ | VERIFY_WRITE),"
			"arg: %p\r\n", __func__, __LINE__, arg);
		return -EACCES;
	}

	prArg32 = (OSE_MEM_GET_INFO_T32 *) arg;

	if (get_user(rInfo.eType, &prArg32->eType)) {
		pr_err("%s line %d fail in get_user(eType), arg: %p\r\n",
			__func__, __LINE__, arg);
		return -EACCES;
	}

	switch (rInfo.eType) {
	case OSE_MEM_GET_MM_RESERVED_SIZE:
		rInfo.rOut.size = OSE_GetMMReservedMemSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_MM_RESERVED_PHY_SA:
		rInfo.rOut.u8PhyAddr =
		    (__u64) OSE_VAToPA((void *)OSE_GetMMReservedMemStartAddr());
		if (put_user(rInfo.rOut.u8PhyAddr, &(prArg32->rOut.u8PhyAddr))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_AVPBBUF:
		rInfo.rOut.size = OSE_GetPbbufInterleaveSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_APBBUF:
		rInfo.rOut.size = OSE_GetPbbufBadInterleaveSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_AVSLOTSIZE:
		rInfo.rOut.size = OSE_GetPbbufAVSlotSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_SPPBBUF:
		rInfo.rOut.size = OSE_GetSPPbbufSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_SPSLOTSIZE:
		rInfo.rOut.size = OSE_GetSPPbbufSlotSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
			       __func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_VFIFO:
		rInfo.rOut.size = OSE_GetVFIFOSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_MM_AFIFO:
		rInfo.rOut.size = OSE_GetAFIFOSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_HBVFIFO:
		rInfo.rOut.size = OSE_GetHighBitrateVFIFOSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	case OSE_MEM_GET_SPFIFO:
		rInfo.rOut.size = OSE_GetSPFIFOSize();
		if (put_user(rInfo.rOut.size, &(prArg32->rOut.size))) {
			pr_err("%s line %d fail in put_user(eType: %d), arg: %p\r\n",
				__func__, __LINE__, prArg32->eType, arg);
			return -EACCES;
		}
		break;
	default:
		pr_err("%s line %d fail for invalid Ose Mem Get Type: %d\n",
			__func__, __LINE__, rInfo.eType);
		return -EINVAL;
	}

	return 0;
}

typedef struct _alloc_node32 {
	compat_caddr_t lptr;
	compat_caddr_t rptr;
	compat_caddr_t pvAddr;
	compat_size_t	 len;
	char		file[256];
	__u32	  line;
} alloc_node32;

static long mmiscdev_memdbg32(unsigned int cmd, void __user *arg)
{
	long ret = 0;

	switch (cmd) {
	case IOCTL_MMISC_MEMDBG_INSTNODE:
		{
			MM_MEMDBG_INSTNODE_T rInsertNodeInfo;
			MM_MEMDBG_INSTNODE_T32 *prArg32 = NULL;
			compat_caddr_t cmpataddr32 = 0;
			alloc_node32 rAllocNode32;
			alloc_node rNode;

			memset(&rInsertNodeInfo, 0, sizeof(rInsertNodeInfo));
			if (!access_ok(VERIFY_READ, (void __user *)arg,
			  sizeof(MM_MEMDBG_INSTNODE_T32))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE),"
					" arg: %p\r\n", __func__, __LINE__, arg);
				return -EACCES;
			}

			prArg32 = (OSE_MEM_GET_INFO_T32 *) arg;

			if (get_user(cmpataddr32, &(prArg32->pvAddr))) {
				pr_err("%s line %d fail in get_user(INSTNODE),"
					"pvAddr: %p, arg: %p\r\n",
					__func__, __LINE__, prArg32->pvAddr, arg);
				return -EACCES;
			}

			rInsertNodeInfo.pvAddr = (__force void *)compat_ptr(cmpataddr32);

			memset(&rAllocNode32, 0, sizeof(rAllocNode32));

			if (0 != copy_from_user(&rAllocNode32, rInsertNodeInfo.pvAddr, sizeof(rAllocNode32))) {
				pr_err("%s line %d fail in copy_from_user(INSTNODE), pvAddr: %p, arg: %p\r\n",
					__func__, __LINE__, rInsertNodeInfo.pvAddr, arg);
				return -EACCES;
			}

			memset(&rNode, 0, sizeof(rNode));
			rNode.pvAddr = rAllocNode32.pvAddr;
			strcpy(rNode.file, rAllocNode32.file);
			rNode.line = rAllocNode32.line;
			rNode.len = (size_t)rAllocNode32.len;

			if (!MM_MemDbg_InsertNode(&rNode)) {
				pr_err("%s line %d fail in mm memdbg insert node,"
					" insert_node: %p, err: %d\r\n",
					__func__, __LINE__, rAllocNode32.pvAddr, ret);
				return -EPERM;
			}
		}
		break;
	case IOCTL_MMISC_MEMDBG_RMNODE:
		{
			MM_MEMDBG_RMNODE_T rRmNodeInfo;
			MM_MEMDBG_RMNODE_T32 *prArg32 = NULL;
			compat_caddr_t cmpataddr32 = 0;
			compat_caddr_t outcmpataddr32 = 0;
			memset(&rRmNodeInfo, 0, sizeof(rRmNodeInfo));

			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg,
				sizeof(MM_MEMDBG_RMNODE_T))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE),"
					" arg: %p\r\n",
					__func__, __LINE__, arg);
				return -EACCES;
			}

			prArg32 = (OSE_MEM_GET_INFO_T32 *) arg;

			if (get_user(cmpataddr32, &(prArg32->pvAddr))) {
				pr_err("%s line %d fail in get_user(INSTNODE), "
				"pvAddr: %p, arg: %p\r\n",
				__func__, __LINE__, prArg32->pvAddr, arg);
				return -EACCES;
			}

			rRmNodeInfo.pvAddr = (__force void *)compat_ptr(cmpataddr32);

			if (!MM_MemDbg_RemoveNode32(rRmNodeInfo.pvAddr,
				&(rRmNodeInfo.pvOutAddr))) {
				pr_err("%s line %d fail in mm memdbg insert node,"
				" remove node: %p, err: %d\r\n",
				__func__, __LINE__, rRmNodeInfo.pvAddr, ret);
				return -EPERM;
			}

			outcmpataddr32 = (compat_caddr_t)ptr_to_compat(rRmNodeInfo.pvOutAddr);

			if (put_user(outcmpataddr32, &(prArg32->pvOutAddr))) {
				pr_err("%s line %d fail in put_user(INSTNODE), "
					"pvAddr: %p, pvOutAddr: %p, arg: %p\r\n",
					__func__, __LINE__, prArg32->pvAddr,
					rRmNodeInfo.pvOutAddr, arg);
				return -EACCES;
			}
		}
		break;
	case IOCTL_MMISC_MEMDBG_DUMP:
		{
			if (!MM_MemDbg_Dump()) {
				pr_err("%s line %d fail in mm memdbg dump\r\n",
					__func__, __LINE__);
				return -EPERM;
			}
		}
		break;
	case IOCTL_MMISC_MEMDBG_FLUSH:
		{
			if (!MM_MemDbg_Flush()) {
				pr_err("%s line %d fail in mm memdbg flush\r\n",
					__func__, __LINE__);
				return -EPERM;
			}
		}
		break;
	case IOCTL_MMISC_MEMDBG_GETSZ:
		{
			MM_MEMDBG_GETNODESZ_T rGetNodeSzInfo;
			MM_MEMDBG_GETNODESZ_T32 *prArg32 = NULL;
			compat_caddr_t cmpataddr32 = 0;

			memset(&rGetNodeSzInfo, 0, sizeof(rGetNodeSzInfo));
			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg,
			sizeof(MM_MEMDBG_GETNODESZ_T32))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), "
					"arg: %p\r\n", __func__, __LINE__, arg);
				return -EACCES;
			}
			prArg32 = (MM_MEMDBG_GETNODESZ_T32 *) arg;
			if (get_user(cmpataddr32, &(prArg32->pvAddr))) {
				pr_err("%s line %d fail in get_user(GETSZ), pvAddr: %p, "
					"arg: %p\r\n", __func__, __LINE__, prArg32->pvAddr, arg);
				return -EACCES;
			}
			rGetNodeSzInfo.pvAddr = (__force void *)compat_ptr(cmpataddr32);
			if (!MM_MemDbg_GetNodeSize(rGetNodeSzInfo.pvAddr,
			&(rGetNodeSzInfo.u4Size))) {
				pr_err("%s line %d fail in mm memdbg insert node, "
					"remove node: %p, err: %d\r\n",
					__func__, __LINE__, rGetNodeSzInfo.pvAddr, ret);
				return -EPERM;
			}

			if (put_user(rGetNodeSzInfo.u4Size, &(prArg32->u4Size))) {
				pr_err("%s line %d fail in put_user(GETSZ), pvAddr: %p, "
					"size: 0x%x, arg: %p\r\n",
					__func__, __LINE__,
					prArg32->pvAddr, rGetNodeSzInfo.u4Size, arg);
				return -EACCES;
			}
		}
		break;
	default:
		pr_err("[STC] %s fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}

static long mmiscdev_osemem32(unsigned int cmd, void __user *arg)
{
	long ret = 0;

	switch (cmd) {
	case IOCTL_MMISC_ALLOC_HW_MEM:
		{
			OSE_MEM_HWMEM_INFO_T rInfo;
			void *pvVirAddr = NULL;
			void *pvMMResrVirAddr = NULL;
			uintptr_t ptrPhyAddr = 0;
			__u32 u4Align = 0;

			memset(&rInfo, 0, sizeof(rInfo));

			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(OSE_MEM_HWMEM_INFO_T))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: 0x%p\r\n",
							 __func__, __LINE__, arg);
				return -EACCES;
			}

			ret = copy_from_user((void *)&rInfo, (void __user *)arg, sizeof(OSE_MEM_HWMEM_INFO_T));
			if (0 != ret) {
				pr_err("%s line %d fail in copy from user, arg: 0x%p, err: %d\r\n",
							 __func__, __LINE__, arg, ret);
				return -EACCES;
			}
			if (0 == rInfo.u4Size) {
				pr_err("%s line %d fail for invalid size: %d, arg: 0x%p\r\n",
							 __func__, __LINE__, rInfo.u4Size, arg);
				return -EACCES;
		  }

			u4Align = rInfo.u4Align;

			if (0 == rInfo.u4Align) {
				u4Align = (__u32)(1 << 12); //4
		  }

			pvMMResrVirAddr = (void *)OSE_GetMMReservedMemStartAddr();
			pvVirAddr = OSE_MemAllocCustom(OSE_VDEC,
							      rInfo.u4Size,
							      u4Align,
							      &ptrPhyAddr);
			if ((NULL == pvVirAddr) || (0 == (uintptr_t)ptrPhyAddr)) {
				pr_err("%s line %d fail in OSE_MemAllocCustom, size: %d, align: %d, arg: 0x%p\r\n",
							 __func__, __LINE__, rInfo.u4Size, u4Align, arg);
				return -ENOMEM;
		  }

			if ((uintptr_t)pvVirAddr >= (uintptr_t)pvMMResrVirAddr) {
				rInfo.u4HwVirAddr = (__u32)((uintptr_t)pvVirAddr - (uintptr_t)OSE_GetMMReservedMemStartAddr());
			} else {
				pr_err("%s line %d fail for invalid virtual addr(0x%p) which is larger than mm reserve memory addr(0x%p)\r\n",
							 __func__, __LINE__, pvVirAddr, pvMMResrVirAddr);
				OSE_MemFreeCustom(OSE_VDEC, pvVirAddr);
				return -EACCES;
			}
			rInfo.u8PhyAddr = (__u64)ptrPhyAddr;

			ret = copy_to_user((void __user *)arg, (void *)&rInfo, sizeof(OSE_MEM_GET_INFO_T));
			if (0 != ret) {
				pr_err("%s line %d fail in copy to user, arg: 0x%p, err: %d\r\n",
							 __func__, __LINE__, arg, ret);
				return -EACCES;
			}
		}
		break;
	case IOCTL_MMISC_RELEASE_HW_MEM:
		{
			__u32 u4HwVirAddr = 0;
			void *pvHwVirAddr = NULL;
			void *pvMMResrVirAddr = NULL;

			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(__u32))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: 0x%p\r\n",
							 __func__, __LINE__, arg);
				return -EACCES;
			}

			ret = copy_from_user((void *)&u4HwVirAddr, (void __user *)arg, sizeof(__u32));
			if (0 != ret) {
				pr_err("%s line %d fail in copy from user, arg: 0x%p, err: %d\r\n",
							 __func__, __LINE__, arg, ret);
				return -EACCES;
			}
			if (0 == u4HwVirAddr) {
				pr_err("%s line %d fail for invalid hw virtual memory addr ofst: 0x%08x, arg: 0x%p, err: %d\r\n",
							 __func__, __LINE__, u4HwVirAddr, arg, ret);
				return -EINVAL;
		  }
			pvMMResrVirAddr = (void *)OSE_GetMMReservedMemStartAddr();
			pvHwVirAddr = (void *)((uintptr_t)pvMMResrVirAddr + (uintptr_t)u4HwVirAddr);
			if (0 == pvHwVirAddr) {
				pr_err("%s line %d fail for invalid hw virtual memory addr: 0x%p, arg: 0x%p, err: %d\r\n",
							 __func__, __LINE__, pvHwVirAddr, arg, ret);
				return -EINVAL;
		  }
			OSE_MemFreeCustom(OSE_VDEC, pvHwVirAddr);
		}
		break;
	default:
		pr_err("[STC] %s fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
		       __func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		ret = -ENOIOCTLCMD;
		break;
	}

	return 0;
}

long mmiscdev_compat32_ioctl(struct file *file, unsigned int cmd,
  unsigned long arg)
{
	long ret = 0;
	void __user *up = compat_ptr(arg);

	if (!file->f_op->unlocked_ioctl)
		return ret;

	if (_IOC_TYPE(cmd) != 'M') {
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case IOCTL_MMISC_GET_OSEMEMIFNO32:
		cmd = IOCTL_MMISC_GET_OSE_MEM_IFNO;
		break;
	case IOCTL_MMISC_ALLOC_HW_MEM:
	case IOCTL_MMISC_RELEASE_HW_MEM:
		break;
	case IOCTL_MMISC_MEMDBG_INSTND32:
		cmd = IOCTL_MMISC_MEMDBG_INSTNODE;
		break;
	case IOCTL_MMISC_MEMDBG_RMND32:
		cmd = IOCTL_MMISC_MEMDBG_RMNODE;
		break;
	case IOCTL_MMISC_MEMDBG_DUMP:
	case IOCTL_MMISC_MEMDBG_FLUSH:
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);
		if (ret != 0) {
			pr_err("%s line %d fail in ioctl '%c', dir=%d, #%d (0x%08x), err: %d\n",
				__func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd),
			  cmd, ret);
		}
		return ret;
	case IOCTL_MMISC_MEMDBG_GETSZ32:
		cmd = IOCTL_MMISC_MEMDBG_GETSZ;
		break;
	default:
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case IOCTL_MMISC_GET_OSE_MEM_IFNO:
		ret = mmiscdev_getosememinfo32((void __user *)arg);
		break;
	case IOCTL_MMISC_ALLOC_HW_MEM:
	case IOCTL_MMISC_RELEASE_HW_MEM:
		ret = mmiscdev_osemem32(cmd, (void __user *)arg);
		break;
	case IOCTL_MMISC_MEMDBG_INSTNODE:
	case IOCTL_MMISC_MEMDBG_RMNODE:
	case IOCTL_MMISC_MEMDBG_GETSZ:
		ret = mmiscdev_memdbg32(cmd, (void __user *)arg);
		break;
	default:
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
			__func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return -ENOIOCTLCMD;
	}

	if (ret != 0)
		pr_err("%s line %d fail in ioctl '%c', dir=%d, #%d (0x%08x), err: %d\n",
			__func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd, ret);

	return ret;
}

#endif				/* CONFIG_COMPAT */


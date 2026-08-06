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
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <media/atc/ioctl_mmisc.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/ose_mem.h>
#include <asm/uaccess.h>

#include "x_ver.h"
#include "memmanager.h"
#include "mmisc.h"
#include "mmisc_osemem.h"
#include "mmisc_compat32_ioctl.h"
/* #include "winutil.h" */
/* #include "windev.h" */
/* #include "windows.h" */


#define MTK_KERNEL_LINUX_LICENSE     "GPL"

#define pr_fmt(fmt) "[MM][MMISC]" fmt

#define MMISC_MODE_NAME              "MMISC"
#define MMISC_VER_MAJOR              02
#define MMISC_VER_MINOR              00
#define MMISC_VER_REV                00

typedef struct mmisc_dev_info {
	/* TODO */
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;
	struct res_mem_info reserved_mem_info;
} mmiscdev_info;

struct res_mem_info *g_rsvmem_info = NULL;

static int mmiscdev_open(struct inode *inode, struct file *file)
{
	int ret = 0;
#if MEMCHK_DEBUG
	MM_MemDbg_Open();
#endif
	return ret;
}

static int mmiscdev_release(struct inode *inode, struct file *file)
{
#if MEMCHK_DEBUG
	MM_MemDbg_Close();
#endif
	return 0;
}

static long mmiscdev_getosememinfo_ioctl(void __user *arg)
{
	OSE_MEM_GET_INFO_T rInfo;
	long ret = 0;

	if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(rInfo))) {
		pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: %p\r\n",
		       __func__, __LINE__, arg);
		return -EACCES;
	}

	ret = copy_from_user((void *)&rInfo, (void __user *)arg, sizeof(rInfo));
	if (0 != ret) {
		pr_err("%s line %d fail in copy from user, arg: %p, err: %d\r\n",
		       __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	switch (rInfo.eType) {
	case OSE_MEM_GET_MM_RESERVED_SIZE:
		rInfo.rOut.size = OSE_GetMMReservedMemSize();
		break;

	case OSE_MEM_GET_MM_RESERVED_PHY_SA:
		rInfo.rOut.u8PhyAddr =
		    (__u64) OSE_VAToPA((void *)OSE_GetMMReservedMemStartAddr());
		break;

	case OSE_MEM_GET_AVPBBUF:
		rInfo.rOut.size = OSE_GetPbbufInterleaveSize();
		break;

	case OSE_MEM_GET_APBBUF:
		rInfo.rOut.size = OSE_GetPbbufBadInterleaveSize();
		break;

	case OSE_MEM_GET_AVSLOTSIZE:
		rInfo.rOut.size = OSE_GetPbbufAVSlotSize();
		break;

	case OSE_MEM_GET_SPPBBUF:
		rInfo.rOut.size = OSE_GetSPPbbufSize();
		break;

	case OSE_MEM_GET_SPSLOTSIZE:
		rInfo.rOut.size = OSE_GetSPPbbufSlotSize();
		break;

	case OSE_MEM_GET_VFIFO:
		rInfo.rOut.size = OSE_GetVFIFOSize();
		break;

	case OSE_MEM_GET_MM_AFIFO:
		rInfo.rOut.size = OSE_GetAFIFOSize();
		break;

	case OSE_MEM_GET_HBVFIFO:
		rInfo.rOut.size = OSE_GetHighBitrateVFIFOSize();
		break;

	case OSE_MEM_GET_SPFIFO:
		rInfo.rOut.size = OSE_GetSPFIFOSize();
		break;

	default:
		pr_err("%s line %d fail for invalid Ose Mem Get Type: %d\n",
		       __func__, __LINE__, rInfo.eType);
		return -EINVAL;
	}

	ret = copy_to_user((void __user *)arg, (void *)&rInfo, sizeof(rInfo));
	if (0 != ret) {
		pr_err("%s line %d fail in copy to user, arg: %p, err: %d\r\n",
		       __func__, __LINE__, arg, ret);
		return -EACCES;
	}

	return 0;
}

static long mmiscdev_osemem_ioctl(unsigned int cmd, void __user *arg)
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

			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg, sizeof(rInfo))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: 0x%p\r\n",
							 __func__, __LINE__, arg);
				return -EACCES;
			}

			ret = copy_from_user((void *)&rInfo, (void __user *)arg, sizeof(rInfo));
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
				rInfo.u4HwVirAddr = (__u32)((uintptr_t)pvVirAddr - (uintptr_t)pvMMResrVirAddr);
			} else {
				pr_err("%s line %d fail for invalid virtual addr(0x%p) which is larger than mm reserve memory addr(0x%p)\r\n",
							 __func__, __LINE__, pvVirAddr, pvMMResrVirAddr);
				OSE_MemFreeCustom(OSE_VDEC, pvVirAddr);
				return -EACCES;
			}
			rInfo.u8PhyAddr = (__u64)ptrPhyAddr;

			ret = copy_to_user((void __user *)arg, (void *)&rInfo, sizeof(rInfo));
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


static long mmiscdev_memdbg_ioctl(unsigned int cmd, void __user *arg)
{
	long ret = 0;

	switch (cmd) {
	case IOCTL_MMISC_MEMDBG_INSTNODE:
		{
			void *pvInsertNode = (void *)NULL;

			if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(void *))) {
				pr_err
				    ("%s line %d fail in access_ok(VERIFY_WRITE), arg: %p\r\n",
				     __func__, __LINE__, arg);
				return -EACCES;
			}

			ret =
			    copy_from_user((void *)&pvInsertNode, (void __user *)arg,
					   sizeof(void *));
			if (0 != ret) {
				pr_err
				    ("%s line %d fail in copy from user, arg: %p, err: %d\r\n",
				     __func__, __LINE__, arg, ret);
				return -EACCES;
			}

			if (!MM_MemDbg_InsertNode(pvInsertNode)) {
				pr_err
				    ("%s line %d fail in mm memdbg insert node, insert_node: %p, err: %d\r\n",
				     __func__, __LINE__, pvInsertNode, ret);
				return -EPERM;
			}
		}
		break;
	case IOCTL_MMISC_MEMDBG_RMNODE:
		{
			MM_MEMDBG_RMNODE_T rRmNodeInfo;

			memset(&rRmNodeInfo, 0, sizeof(rRmNodeInfo));
			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg,
				       sizeof(rRmNodeInfo))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: %p\r\n",
				     __func__, __LINE__, arg);
				return -EACCES;
			}

			ret = copy_from_user((void *)&rRmNodeInfo, (void __user *)arg,
					     sizeof(rRmNodeInfo));
			if (0 != ret) {
				pr_err("%s line %d fail in copy from user, arg: %p, err: %d\r\n",
				     __func__, __LINE__, arg, ret);
				return -EACCES;
			}

			if (!MM_MemDbg_RemoveNode(rRmNodeInfo.pvAddr, &(rRmNodeInfo.pvOutAddr))) {
				pr_err("%s line %d fail in mm memdbg insert node, remove node: %p, err: %d\r\n",
				     __func__, __LINE__, rRmNodeInfo.pvAddr, ret);
				return -EPERM;
			}
			ret = copy_to_user((void __user *)arg, (void *)&rRmNodeInfo,
					   sizeof(rRmNodeInfo));
			if (0 != ret) {
				pr_err("%s line %d fail in copy to user, arg: %p, err: %d\r\n",
				     __func__, __LINE__, arg, ret);
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

			memset(&rGetNodeSzInfo, 0, sizeof(rGetNodeSzInfo));
			if (!access_ok(VERIFY_READ | VERIFY_WRITE, (void __user *)arg,
				       sizeof(rGetNodeSzInfo))) {
				pr_err("%s line %d fail in access_ok(VERIFY_WRITE), arg: %p\r\n",
				     __func__, __LINE__, arg);
				return -EACCES;
			}

			ret = copy_from_user((void *)&rGetNodeSzInfo, (void __user *)arg,
					     sizeof(rGetNodeSzInfo));
			if (0 != ret) {
				pr_err("%s line %d fail in copy from user, arg: %p, err: %d\r\n",
				     __func__, __LINE__, arg, ret);
				return -EACCES;
			}

			if (!MM_MemDbg_GetNodeSize(rGetNodeSzInfo.pvAddr, &(rGetNodeSzInfo.u4Size))) {
				pr_err("%s line %d fail in mm memdbg insert node, remove node: %p, err: %d\r\n",
				     __func__, __LINE__, rGetNodeSzInfo.pvAddr, ret);
				return -EPERM;
			}
			ret = copy_to_user((void __user *)arg, (void *)&rGetNodeSzInfo,
					   sizeof(rGetNodeSzInfo));
			if (0 != ret) {
				pr_err("%s line %d fail in copy to user, arg: %p, err: %d\r\n",
				     __func__, __LINE__, arg, ret);
				return -EACCES;
			}
		}
		break;
	default:
		pr_err("%s fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
		       __func__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}

static long mmiscdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if (_IOC_TYPE(cmd) != MMISC_IOCTL_MAGIC) {
		pr_err("%s line %d fail for invalid ioctl '%c', dir=%d, #%d (0x%08x)\n",
		       __func__, __LINE__, _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
		return ret;
	}

	switch (cmd) {
	case IOCTL_MMISC_GET_OSE_MEM_IFNO:
		{
			ret = mmiscdev_getosememinfo_ioctl((void __user *)arg);
			break;
		}

	case IOCTL_MMISC_ALLOC_HW_MEM:
	case IOCTL_MMISC_RELEASE_HW_MEM:
		{
			ret = mmiscdev_osemem_ioctl(cmd, (void __user *)arg);
			break;
		}

	case IOCTL_MMISC_MEMDBG_INSTNODE:
	case IOCTL_MMISC_MEMDBG_RMNODE:
	case IOCTL_MMISC_MEMDBG_DUMP:
	case IOCTL_MMISC_MEMDBG_FLUSH:
	case IOCTL_MMISC_MEMDBG_GETSZ:
		{
			ret = mmiscdev_memdbg_ioctl(cmd, (void __user *)arg);
			break;
		}

	default:
		pr_err("Get error ioctl: %d\n", cmd);
		ret = -1;
		break;
	}

	return ret;
}

static void mmisc_vma_open(struct vm_area_struct *vma)
{
	pr_debug("mmisc_vma_open, virt 0x%x, phys 0x%x.\r\n",
		 (unsigned int)vma->vm_start, (unsigned int)(vma->vm_pgoff << PAGE_SHIFT));
}

static void mmisc_vma_close(struct vm_area_struct *vma)
{
	pr_debug("mmisc_vma_close, virt 0x%x, phys 0x%x.\r\n",
		 (unsigned int)vma->vm_start, (unsigned int)(vma->vm_pgoff << PAGE_SHIFT));
}

static struct vm_operations_struct mmisc_remap_vm_ops = {
	.open = mmisc_vma_open,
	.close = mmisc_vma_close,
};

static int mmiscdev_mmap(struct file *fp, struct vm_area_struct *vma)
{
	unsigned long length = 0;

	length = vma->vm_end - vma->vm_start;

	//vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, length, vma->vm_page_prot))
		return -EAGAIN;

	vma->vm_ops = &mmisc_remap_vm_ops;
	mmisc_vma_open(vma);

	return 0;
}

const struct file_operations mmiscdev_fops = {
	.release = mmiscdev_release,
	.open = mmiscdev_open,
	.mmap = mmiscdev_mmap,
	.unlocked_ioctl = mmiscdev_ioctl,
#if CONFIG_COMPAT
	.compat_ioctl = mmiscdev_compat32_ioctl,
#endif
};

static int mmiscdev_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct reserved_mem *mmisc_reserved_mem = NULL;
	mmiscdev_info *mmisc_dev = NULL;
	int result = -1;

	if (!node) {
		pr_err
		    (" mmisc probe fail because of no mmisc device compatible dts node!\r\n");
		return -1;
	}

	mmisc_dev = kzalloc(sizeof(mmiscdev_info), GFP_KERNEL);
	if (!mmisc_dev) {
		result = -ENOMEM;
		pr_err(" mmisc probe fail because of get alloc memory fail!\n");
		return result;
	}

#if 1
	{
		struct device_node *node = NULL;
		int result = 0;
		u32 regs[4] = {0};
		__u64 addr = 0;

		node = of_find_compatible_node(NULL, NULL, "atc,mmreservedmem");
		if (!node) {
			pr_err(" mmisc probe fail in find dts compatible node -- atc,mmreservedmem!!\r\n");
			result = -ENOMEM;
			goto err_free_mem;
		}

		result = of_property_read_u32_array(node, "reg", regs, 4);
		if (0 != result) {
			pr_err(" mmisc probe fail in find reg in dts compatible node -- atc,mmreservedmem, result: %d!!\r\n",
				result);
			goto err_free_mem;
		}

		pr_info(" mmisc probe find reg(0x%08x, 0x%08x, 0x%08x, 0x%08x)!!\r\n",
			regs[0], regs[1], regs[2], regs[3]);
		addr = (((__u64)regs[0]) << 32) | ((__u64)regs[1]);
		mmisc_dev->reserved_mem_info.phys_addr = addr;
		pr_info(" mmisc probe line %d (0x%llx, phys_addr: 0x%llx)!\r\n",
			__LINE__, addr, mmisc_dev->reserved_mem_info.phys_addr);
		addr = (((__u64)regs[2]) << 32) | ((__u64)regs[3]);
		mmisc_dev->reserved_mem_info.size = addr;
		pr_info(" mmisc probe line %d (0x%llx, size: 0x%llx)!\r\n",
			__LINE__, addr, mmisc_dev->reserved_mem_info.size);
		if ((0 == mmisc_dev->reserved_mem_info.phys_addr) ||
			(0 == mmisc_dev->reserved_mem_info.size)) {
			pr_err(" mmisc probe fail for phyaddr(0x%llx) or size(0x%llx) is 0!!\r\n",
				mmisc_dev->reserved_mem_info.phys_addr, mmisc_dev->reserved_mem_info.size);
			goto err_free_mem;
		}
		mmisc_dev->reserved_mem_info.virt_addr = phys_to_virt(mmisc_dev->reserved_mem_info.phys_addr);
		pr_info(" mmisc probe (phys_addr: 0x%llx, size: 0x%llx, virt_addr: 0x%llx)!!\r\n",
			mmisc_dev->reserved_mem_info.phys_addr, mmisc_dev->reserved_mem_info.size,
			mmisc_dev->reserved_mem_info.virt_addr);
		if (0 == mmisc_dev->reserved_mem_info.virt_addr) {
			pr_err(" mmisc probe fail in phys_to_virt, phyaddr: 0x%llx, size: 0x%llx!!\r\n",
				mmisc_dev->reserved_mem_info.phys_addr, mmisc_dev->reserved_mem_info.size);
			goto err_free_mem;
		}
	}

#else
	of_reserved_mem_device_init(&(pdev->dev));
	mmisc_reserved_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!mmisc_reserved_mem) {
		pr_err(" mmisc reserved memory get error!\r\n");
		goto err_free_mem;
	}

	pr_info("%s reserved memory base:0x%x, size:0x%x. \r\n",
		mmisc_reserved_mem->name, mmisc_reserved_mem->base, mmisc_reserved_mem->size);

	mmisc_dev->reserved_mem_info.phys_addr = mmisc_reserved_mem->base;
	mmisc_dev->reserved_mem_info.size = mmisc_reserved_mem->size;
	mmisc_dev->reserved_mem_info.virt_addr = ioremap_nocache(mmisc_reserved_mem->base,
							 mmisc_reserved_mem->size);

	if (0 == mmisc_dev->reserved_mem_info.virt_addr) {
		pr_err(" mmisc fail in ioremap(phyaddr: 0x%08x, size: 0x%08x)!\r\n",
		       mmisc_reserved_mem->base, mmisc_reserved_mem->size);
		goto err_free_mem;
	}
#endif


	mmisc_dev->dev = &(pdev->dev);
	mmisc_dev->cdev.name = "mmiscdev";
	mmisc_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	mmisc_dev->cdev.fops = &mmiscdev_fops;

	platform_set_drvdata(pdev, mmisc_dev);

	result = misc_register(&(mmisc_dev->cdev));
	if (result != 0) {
		pr_err(" mmisc probe fail because of misc_register, error = %d\r\n",
		       result);
		goto err_unset_drvdata;
	}

	g_rsvmem_info = &(mmisc_dev->reserved_mem_info);

	pr_info(" mmisc probe ok\r\n");
	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_mem:
	if (0 != mmisc_dev->reserved_mem_info.virt_addr)
		iounmap(mmisc_dev->reserved_mem_info.virt_addr);
	kfree(mmisc_dev);
	pr_info(" mmisc probe fail\r\n");
	return result;
}

static int mmiscdev_remove(struct platform_device *pdev)
{
	mmiscdev_info *mmisc_dev = platform_get_drvdata(pdev);

	if (0 != mmisc_dev->reserved_mem_info.virt_addr)
		iounmap(mmisc_dev->reserved_mem_info.virt_addr);

	misc_deregister(&(mmisc_dev->cdev));

	platform_set_drvdata(pdev, NULL);

	kfree(mmisc_dev);

	return 0;
}

static const struct of_device_id mmisc_of_ids[] = {
	{.compatible = "atc,mmisc",},
	{}
};

static struct platform_driver mmisc_plt_drv = {
	.driver = {
		   .name = "atc-mmisc",
		   .owner = THIS_MODULE,
		   .of_match_table = mmisc_of_ids,
		   },
	.probe = mmiscdev_probe,
	.remove = mmiscdev_remove,
};

static int __init mmisc_init(void)
{

	struct device_node *node = NULL;
	int result = 0;

	node = of_find_compatible_node(NULL, NULL, "atc,mmisc");
	if (!node) {
		pr_err(" mmisc init fail in find dts compatible node!!\r\n");
		result = -ENOMEM;
		goto err_node;
	}

	result = platform_driver_register(&mmisc_plt_drv);
	if (result) {
		pr_err("mmisc init fail in platform_driver_register, error = %d\r\n",
		       result);
		result = -EPERM;
		goto err_node;
	}

	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

#if MEMCHK_DEBUG
	MM_MemDbg_Init();
#endif

	OSE_Init();

	pr_info("mmisc driver init ok!.\r\n");
	return 0;

err_node:
	return result;
}

static void __exit mmisc_exit(void)
{
	OSE_Uninit();

#if MEMCHK_DEBUG
	MM_MemDbg_Deinit();
#endif

	platform_driver_unregister(&mmisc_plt_drv);

	pr_info("mmmisc driver exit ok!\r\n");
}
module_init(mmisc_init);
module_exit(mmisc_exit);

MODULE_LICENSE(MTK_KERNEL_LINUX_LICENSE);

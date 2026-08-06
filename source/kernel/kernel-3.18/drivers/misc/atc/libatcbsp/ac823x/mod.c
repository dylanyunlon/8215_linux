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

/************************************************************************************************
 *
 * Filename:
 * ---------
 *   $Workfile:$
 *
 * Project:
 * --------
 * AC83XX Android Prototype
 *
 * Description:
 * ------------
 * video decode driver kernel module
 *
 * Author:
 * -------
 * mtk40505 : 2011-04, draft
 *
 * $Modtime: $
 *
 * $Revision: #1 $
 ************************************************************************************************/
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <asm/page.h>

#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"



//#define g_u4PhysicalAddress  SUBTITLE_MEM_PA
static int libatcbsp_mmap(struct file *fp, struct vm_area_struct *vma)
{
  #if 0
    //printk("[Libatcbsp] enter libatcbsp_mmap vma->vm_pgoff=0x%lx\n", vma->vm_pgoff);
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    /* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
    if (remap_pfn_range(vma,vma->vm_start,g_u4PhysicalAddress >> PAGE_SHIFT,
                        SUBTITLE_MEM_SIZE,vma->vm_page_prot))

        return -EAGAIN;
        
  #endif
    pr_info("[Libatcbsp] [%s][%d] this function is empty for linux reserve memory modify, mtk68080\r\n", __func__, __LINE__);
    return 0;
}

static const struct file_operations libatcbsp_fops = {
	.owner			= THIS_MODULE,
	.mmap			= libatcbsp_mmap,
};

static struct miscdevice libatcbsp_dev = {
	/*
	 * We don't care what minor number we end up with, so tell the
	 * kernel to just pick one.
	 */
	MISC_DYNAMIC_MINOR,
	/*
	 * Name ourselves /dev/vdp.
	 */
	"libatcbsp",
	/*
	 * What functions to call when a program performs file
	 * operations on the device.
	 */
	&libatcbsp_fops
};


int __init mt_sys_init(void)
{
    int ret;
    ret = misc_register(&libatcbsp_dev);
	if (ret)
		pr_err("[Libatcbsp] [mod.c][%s][%d] Unable to register \"libatcbsp\" misc device\n", __func__, __LINE__);

	return ret;
}

void __exit mt_sys_exit(void)
{
    misc_deregister(&libatcbsp_dev);
}

MODULE_LICENSE("GPL");



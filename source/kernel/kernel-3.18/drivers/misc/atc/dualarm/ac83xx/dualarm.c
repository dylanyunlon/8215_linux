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
#include <linux/interrupt.h>

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include "x_bim.h"
#include "x_ver.h"
#include "x_ckgen.h"
#include "base_regs.h"
#include "drv_dual.h"
#include "irqs_vector.h"
#include "oal.h"
#include <ac83xx_gpio_pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <mach/pinmux.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <generated/atc_project.h>

#include "dualarm.h"

#define DUALARM_DEV_DRIVER_NAME "dualarm-dev"
#define DEVICE_NAME "dualarm-dev"

#define DUALARM_IOC_MAGIC 'd'
#define DUALARM_IOC_MAXNR 2
//#define DUALARM_IOC_SENDMESSAGE _IOW(DUALARM_IOC_MAGIC, 0, int)
//#define DUALARM_IOC_GETMESSAGE _IOW(DUALARM_IOC_MAGIC, 1, int)

#define MAX_DULAARM_MODULE   (4)
typedef struct _module_info{

    int module_index;
    irq_handler_t handler;
    const char *name;

}module_info;

static module_info g_arm2_modules[3];




#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
UINT32 u4ARM2Start(void)
{
    UINT32 tmp;
    pr_info("[Dualarm] [%s][%d] start ARM2!\n", __func__, __LINE__);
//    UINT32 *pMemAddr=  (UINT32 *)(MEMRSV_PHY_TO_VIRT(0x16880000 + 0x1000));
    void* pVMemAddr = ioremap(0x16880000,16);
    *(UINT32 *)(pVMemAddr) = 0xea00fffe;
    iounmap(pVMemAddr);
//    *(UINT32 *)(pMemAddr - 0x200) = 0x40000000;   //memroy size 1G
#if trace_debug
    tmp = HAL_READ32(0xFD038088);
    tmp |= 0x1;
    HAL_WRITE32(0xFD038088,tmp);
    tmp = HAL_READ32(0xFD000058);
    tmp |= 0x2000000;
    HAL_WRITE32(0xFD000058,tmp);
#endif
    HAL_WRITE32(0xFD04501C,0x00000001);
    HAL_WRITE32(0xFD045020,0x16880000);
    HAL_WRITE32(0xFD0381B8,0x00000003);
    return 0;
}
#endif
#endif

static void fgDualGetMessage(unsigned int GroupID, unsigned int *pu4MessageHeader)
{
    if (GroupID == 0)
    {
        *pu4MessageHeader = BIM2_READ32(REG_RW_SINFO0_REG);
    }
    else if (GroupID == 1)
    {
        *pu4MessageHeader = BIM2_READ32(REG_RW_SINFO4_REG);
    }
    else if(GroupID == 2)
    {
        *pu4MessageHeader = BIM2_READ32(REG_RW_SINFO8_REG);
    }
    else if(GroupID == 3)
    {
        *pu4MessageHeader = BIM2_READ32(REG_RW_SINFOC_REG);
    }
}

static unsigned int vGetModuleID(void)
{
    unsigned int u4Tmp1;
    unsigned int u4MessageHeader, u4MoudleID;

    for(u4Tmp1=0; u4Tmp1<DUAL_MAX_TASK; u4Tmp1++)
    {
        fgDualGetMessage(u4Tmp1, &u4MessageHeader);

        if(GETMESSAGEDIR(u4MessageHeader) == ARM2TOARM1)
        {
            break;
        }
    }

    u4MoudleID = GETMODULEID(u4MessageHeader);



    return(u4MoudleID);
}


int  request_dualarm_irq(unsigned int module, irq_handler_t handler, unsigned long flags,
        const char *name, void *dev)
{
    if(module > (MAX_DULAARM_MODULE -1))
        return -1;

    g_arm2_modules[module].module_index = module;
    g_arm2_modules[module].handler = handler;
    g_arm2_modules[module].name = name;

    return 0;

}
EXPORT_SYMBOL(request_dualarm_irq);

extern void ac83xx_mask_ack_bim_irq(uint32_t irq);

static irqreturn_t dualarm_isr_handler(int irq, void *dev_id)
{

    unsigned int moduleid = vGetModuleID();
	pr_debug("[Dualarm] [%s][%s] dualarm_isr_handler call,moudle is %d\n", __func__, __LINE__,moduleid);

    if(g_arm2_modules[moduleid].handler != NULL){
		pr_debug("[Dualarm] [%s][%d] call %d handler\n", __func__, __LINE__,moduleid);
        g_arm2_modules[moduleid].handler(moduleid,dev_id);}

	else {
		pr_err("[Dualarm] [dualarm.c][%s][%d] moudle is %dhandler is null\n", __func__, __LINE__,moduleid);
		}
    ac83xx_mask_ack_bim_irq(VECTOR_TOCORISC);

    return IRQ_HANDLED;
}

static int dualarm_mmap(struct file *fp, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static int dualarm_open(struct inode* inode, struct file *file)
{
	pr_info("[Dualarm] [%s][%d] dualarm open[kernel space]\n", __func__, __LINE__);
	return 0;
}

static ssize_t dualarm_read(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	int err = 0;
	int ret = 0;
	UINT32 module_id=0;
	UINT32 user_buf[4] = {0};
	int i = 0;
	pr_info("[Dualarm] [%s][%d]dualarm read[kernel space]\n", __func__, __LINE__);
	ret = copy_from_user((void *)module_id, in, 1 * sizeof(UINT32));
	HWGetMessage(module_id, &user_buf[0], &user_buf[1], &user_buf[2], &user_buf[3]);
	ret = copy_to_user((void *)in, user_buf, 4 * sizeof(UINT32));
	for(i = 0; i < 4; i++)
	{
		pr_info("[Dualarm] [%s][%d] user_buf[%d] = %d\n", __func__, __LINE__, i, user_buf[i]);
	}
	return ret;
}

static ssize_t dualarm_write(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	int ret = 0;
	UINT32 user_buf[4] = {0};
	int i = 0;
	pr_info("[Dualarm] [%s][%d] dualarm write[kernel space]\n", __func__, __LINE__);
	ret = copy_from_user(user_buf, in, 4 * sizeof(UINT32));
	for(i = 0; i < size; i++)
	{
		pr_info("[Dualarm] [%s][%d] user_buf[%d] = %d\n", __func__, __LINE__, i, user_buf[i]);
	}
	HWSendMessage(user_buf[0], user_buf[1], user_buf[2], user_buf[3]);
	return 0;
}

static int dualarm_ioctl(struct file *filep,unsigned int cmd, unsigned long arg)
{
	int err = 0;
	BOOL ret = 0;
	UINT32 module_id=0;
	UINT32 user_buf[4] = {0};
	int i = 0;
	u32 bootLogoPhyAdr;
	LOGO_BUF_INFO_T rLogoBufInfo;
	/*if(_IOC_TYPE(cmd) != DUALARM_IOC_MAGIC)
	{
		return -EINVAL;
	}
	if(_IOC_NR(cmd) > DUALARM_IOC_MAXNR)
	{
		return -EINVAL;
	}
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	}
	if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = ! access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	}
	if(err != 0)
	{
		return -EFAULT;
	}*/
	pr_info("[Dualarm] [%s][%d] dualarm ioctl[kernel space 0x%08x] %d,%d\n", __func__, __LINE__, cmd, (DUALARM_IOC_GETMESSAGE == cmd), (DUALARM_IOC_SENDMESSAGE == cmd));

	switch(cmd)
	{
		case DUALARM_IOC_GETMESSAGE:
			{
				copy_from_user((void *)module_id, (void *)arg, 1 * sizeof(UINT32));
				pr_info("[Dualarm] [%s][%d] dualarm iotcl[kernel space] module_id=%d\n", __func__, __LINE__,module_id);
				ret = HWGetMessage(module_id, &user_buf[0], &user_buf[1], &user_buf[2], &user_buf[3]);
				pr_info("[Dualarm] [%s][%d] dualarm iotcl[kernel space] &user_buf[0]=%d,&user_buf[1]=%d,&user_buf[2]=%d,&user_buf[3]=%d\n", __func__, __LINE__,user_buf[0],user_buf[1],user_buf[2],user_buf[3]);
				copy_to_user((void *)arg, user_buf, 4 * sizeof(UINT32));
			}
		break;
		case DUALARM_IOC_SENDMESSAGE:
			{
				copy_from_user(user_buf, (void *)arg, 4 * sizeof(UINT32));
				ret = HWSendMessage(user_buf[0], user_buf[1], user_buf[2], user_buf[3]);
				pr_info("[Dualarm] [%s][%d] dualarm iotcl[kernel space] &user_buf[0]=%d,&user_buf[1]=%d,&user_buf[2]=%d,&user_buf[3]=%d\n", __func__, __LINE__,user_buf[0],user_buf[1],user_buf[2],user_buf[3]);
			}
		break;
		default:
				{
					return -EINVAL;
				}
		break;
	}
	return ret;
}

struct file_operations dualarm_fops = {
	.owner = THIS_MODULE,
	.open = dualarm_open,
	.read = dualarm_read,
	.write = dualarm_write,
	.mmap = dualarm_mmap,
	.unlocked_ioctl = dualarm_ioctl,

};

static struct miscdevice dualarm_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dualarm_fops,
};




static int dualarm_probe(struct platform_device * pdev)
{
	int ret = 0;
    pr_info("[Dualarm] [%s][%d] dualarm probe\n", __func__, __LINE__);
	ret = misc_register(&dualarm_misc_dev);
	

    return 0;
}
static int /*__devexit*/ dualarm_remove(struct platform_device *dev)
{

    pr_info("[Dualarm] [%s][%d] dualarm remove\n", __func__, __LINE__);
	misc_deregister(&dualarm_misc_dev);

    return 0;
}
#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
static int dualarm_suspend(struct device *dev)
{

    //struct platform_device *pdev = to_platform_device(dev);
    pr_info("[Dualarm] [%s][%d] dualarm suspend\n", __func__, __LINE__);
    return 0;

}
static int dualarm_resume(struct device *dev)
{

    //struct  platform_device *pdev = to_platform_device(dev);
    pr_info("[Dualarm] [%s][%d] dualarm resume\n", __func__, __LINE__);
    u4ARM2Start();
    return 0;
}

    static const struct dev_pm_ops dualarm_dev_pm_ops = {
        SET_SYSTEM_SLEEP_PM_OPS(dualarm_suspend,dualarm_resume)
    };
#endif 
#endif

static struct platform_driver dualarm_driver = {

    .driver = {
        .name = "dualarm-dev",
        .owner = THIS_MODULE,
#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
        .pm = &dualarm_dev_pm_ops,
#endif
#endif
    },
    .probe = dualarm_probe,
    //.remove = __devexit_p(dualarm_remove),
		.remove = dualarm_remove,


};


static struct platform_device dualarm_device = {

    .name = "dualarm-dev",
    .id = -1,

};

#define DUALARM_VER_MAIN	1
#define DUALARM_VER_MINOR	00
#define DUALARM_VER_REV	00

static int __init dualarm_init(void)
{
    int ret = 0;
    int res;

    res = os_device_register(&dualarm_device);
    res = os_driver_register(&dualarm_driver);

    memset((void *)g_arm2_modules,0x0,sizeof(g_arm2_modules));
    ret = request_irq(VECTOR_TOCORISC, dualarm_isr_handler,
            0,"DUALARM_ISR", NULL);

	MOD_VERSION_INFO("dualarmdrv",DUALARM_VER_MAIN,DUALARM_VER_MINOR,DUALARM_VER_REV);

    if(0 == ret)
    {
        pr_info("[Dualarm] [%s][%d] dualarm_init success\r\n", __func__, __LINE__);
    }
	else
        pr_err("[Dualarm] [dualarm.c][%s][%d] dualarm_init failed\r\n", __func__, __LINE__);

    return ret;
}

static void __exit dualarm_exit(void)
{
    free_irq(VECTOR_TOCORISC, NULL);
}




module_init(dualarm_init);
module_exit(dualarm_exit);
EXPORT_SYMBOL(fgDualHALInit);
EXPORT_SYMBOL(fgDualHALStart);
EXPORT_SYMBOL(fgDualHALStop);
EXPORT_SYMBOL(fgDualHALSetRemap);
EXPORT_SYMBOL(fgDualHALSetOffset);
EXPORT_SYMBOL(u4DualHALOffsetAlignment);
EXPORT_SYMBOL(fgDualHALSetBootUpParameter);

EXPORT_SYMBOL(fgDualHALSetSendCommandParameter);
EXPORT_SYMBOL(fgDualHALINTEachOther);
EXPORT_SYMBOL(fgDualHALGetReturnParameter);
EXPORT_SYMBOL(HWSendMessage);

EXPORT_SYMBOL(HWGetMessage);


MODULE_AUTHOR("ATC");
MODULE_DESCRIPTION("dualarm Driver");
MODULE_LICENSE("GPL");



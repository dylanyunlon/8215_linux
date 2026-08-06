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
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include "x_ver.h"
#include "drv_dual.h"
//#include "x_bim_823x.h"
#include "mach/mt3365_irqs_vector.h"
#include "oal.h"
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <asm/io.h>
#include <generated/atc_project.h>
#include "dualarm.h"
#include "share_registers.h"
#define DUALARM_DEV_DRIVER_NAME "dualarm-dev"
#define DEVICE_NAME "dualarm-dev"

#define DUALARM_IOC_MAGIC 'd'
#define DUALARM_IOC_MAXNR 2
//#define DUALARM_IOC_SENDMESSAGE _IOW(DUALARM_IOC_MAGIC, 0, int)
//#define DUALARM_IOC_GETMESSAGE _IOW(DUALARM_IOC_MAGIC, 1, int)

#define MAX_DULAARM_MODULE   (3)
typedef struct _module_info{

    int module_index;
    irq_handler_t handler;
    const char *name;

}module_info;

static module_info g_arm2_modules[3];

static struct device_node *node=NULL;


#define __io(a) ((void __iomem *)(a))
static void __iomem *io_reg_base;
static void __iomem *bim_reg_base;
static void __iomem *dualarm_reg_base;

u32 __bim_readl(u64 regaddr)
{
        return __raw_readl(__io(bim_reg_base + regaddr));
}
void __bim_writel(u64 regaddr, u32 regval32)
{
        __raw_writel(regval32, __io(bim_reg_base + regaddr));
}
u32 __bim2_readl(u64 regaddr)
{
        return __raw_readl(__io(dualarm_reg_base + regaddr));
}
void __bim2_writel(u64 regaddr, u32 regval32)
{
        __raw_writel(regval32, __io(dualarm_reg_base + regaddr));
}
#if 0  //SBIM only accessing in security mode
u32 __io_readl(u64 regaddr)
{
        return __raw_readl(__io(io_reg_base + regaddr));
}
void __io_writel(u64 regaddr, u32 regval32)
{
        __raw_writel(regval32, __io(io_reg_base + regaddr));
}
#endif

#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac823x)
#if 0
UINT32 u4ARM2Start(void)
{
	UINT32 tmp;
	pr_info("[Dualarm] [%s][%d] start ARM2!\n", __func__, __LINE__);
    	//UINT32 *pARM2phyadd = (UINT32 *)(ARM2_RESERVED_MEM_PA);
    	//UINT32 *pMemAddr=  (UINT32 *)(MEMRSV_PHY_TO_VIRT(0x19e80000 + 0x1000));
	void* pVMemAddr = ioremap(0x10FC00000,16);
    	*(UINT32 *)(pVMemAddr) = 0xea00fffe;
    	iounmap(pVMemAddr);
    	//*(UINT32 *)(pMemAddr - 0x400) = 0xea0003fe;
    	//*(UINT32 *)(pMemAddr - 0x200) = 0x40000000;   //memroy size 1G

        tmp = __io_readl(0xA0);
        tmp |= 0x1000000;
        __io_writel(0xA0,tmp);
	__io_writel(0x8780,0x00002000);
	__io_writel(0x45020,(u32)(0xfc00000));
	__io_writel(0x381B8,0x00000002);

    	return 0;
}
#endif
#endif
#endif

static void fgDualGetMessage(unsigned int GroupID, unsigned int *pu4MessageHeader)
{
    if (GroupID == 0)
    {
        *pu4MessageHeader = __bim2_readl(REG_RW_SINFO0_REG);
    }
    else if (GroupID == 1)
    {
        *pu4MessageHeader = __bim2_readl(REG_RW_SINFO4_REG);
    }
    else if(GroupID == 2)
    {
        *pu4MessageHeader = __bim2_readl(REG_RW_SINFO8_REG);
    }
    else if(GroupID == 3)
    {
        *pu4MessageHeader = __bim2_readl(REG_RW_SINFOC_REG);
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

//extern void ac83xx_mask_ack_bim_irq(uint32_t irq);
extern void mt33xx_mask_ack_bim_irq(uint32_t irq);

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
    //ac83xx_mask_ack_bim_irq(VECTOR_TOCORISC);
    mt33xx_mask_ack_bim_irq(VECTOR_ARM9_INT);

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
	MISC_DYNAMIC_MINOR,
	"dualarm-dev",
	&dualarm_fops,
};




static const struct of_device_id dualarm_of_match[] = {
        {.compatible = "Autochips,dualarm-dev",},
        {},
};
MODULE_DEVICE_TABLE(of, dualarm_of_match);
static int dualarm_probe(struct platform_device * pdev)
{
	int ret = 0;
	ret = misc_register(&dualarm_misc_dev);

	if (ret) {
		pr_err("[Dualarm] [dualarm.c][%s][%d] Unable to register \"dualarm\" misc device\n", __func__, __LINE__);
		return ret;
	}

        node =of_find_compatible_node(NULL,NULL,"Autochips,dualarm-dev");
        if(node){
 		dualarm_reg_base = of_iomap(node, 0);
                pr_info("[Dualarm] [%s][%d] dualarm_reg_base start is 0x%p\n", __func__, __LINE__, dualarm_reg_base);
        }
        else {
                pr_err("[Dualarm] [dualarm.c][%s][%d] failed to get dualarm-dev node\n", __func__, __LINE__);
        }
//	dualarm_reg_base = phys_to_virt((unsigned long)(0x10045000));//ioremap(0x10045000, 0x1000);//of_iomap(pdev->dev.of_node, 0);
	if(dualarm_reg_base == 0){
		pr_err("[Dualarm] [dualarm.c][%s][%d] dualarm probe is failed cause by get dualarm_reg_base\n", __func__, __LINE__);
		ret = -ENODEV;
	}

        node =of_find_compatible_node(NULL,NULL,"mediatek,mt33xx-bim");
        if(node){
                bim_reg_base = of_iomap(node, 0);
                pr_info("[Dualarm] [%s][%d] bim_reg_base start is 0x%p\n", __func__, __LINE__, bim_reg_base);
        }
        else {
                pr_err("[Dualarm] [dualarm.c][%s][%d] failed to get bim node\n", __func__, __LINE__);
        }
//      bim_reg_base = phys_to_virt((unsigned long)(0x10045000));//ioremap(0x10045000, 0x1000);//of_iomap(pdev->dev.of_node, 0);
        if(bim_reg_base == 0){
                pr_err("[Dualarm] [dualarm.c][%s][%d] dualarm probe is failed cause by get bim_reg_base\n", __func__, __LINE__);
                ret = -ENODEV;
        }
#if 0
	node =of_find_compatible_node(NULL,NULL,"mediatek,mt33xx-iodev");
        if(node){
                io_reg_base = of_iomap(node, 0);
                pr_info("[Dualarm] [%s][%d] io_reg_base start is 0x%p\n", __func__, __LINE__, io_reg_base);
        }
        else {
                pr_err("[Dualarm] [dualarm.c][%s][%d] failed to get iodev node\n", __func__, __LINE__);
        }
//      io_reg_base = phys_to_virt((unsigned long)(0x10045000));//ioremap(0x10045000, 0x1000);//of_iomap(pdev->dev.of_node, 0);
        if(io_reg_base == 0){
                pr_err("[Dualarm] [dualarm.c][%s][%d] dualarm probe is failed cause by get io_reg_base\n", __func__, __LINE__);
                ret = -ENODEV;
        }
#endif
	pr_info("[Dualarm] [%s][%d] dualarm probe is called\n", __func__, __LINE__);
    return ret;
}
static int /*__devexit*/ dualarm_remove(struct platform_device *dev)
{

    pr_info("[Dualarm] [%s][%d] dualarm remove\n", __func__, __LINE__);
	misc_deregister(&dualarm_misc_dev);

    return 0;
}

#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac823x)
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
#if 0
    u4ARM2Start();
#endif
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
	.of_match_table = dualarm_of_match,
#if defined(CONFIG_ATC_OS_android)
#if defined(CONFIG_ATC_PLATFORM_ac823x)
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
    res = platform_driver_register(&dualarm_driver);
    //res = os_device_register(&dualarm_device);
    //res = os_driver_register(&dualarm_driver);
    memset((void *)g_arm2_modules,0x0,sizeof(g_arm2_modules));
    ret = request_irq(VECTOR_ARM9_INT, dualarm_isr_handler,
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
    free_irq(VECTOR_ARM9_INT, NULL);
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



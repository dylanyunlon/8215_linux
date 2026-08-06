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

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/types.h>

#include <linux/miscdevice.h>
#include <linux/ioctl.h>
//#include <asm/system.h>
#include "x_bim_83xx.h"
#include "x_ckgen.h"
#include "oal.h"
#include "irtdma_drv.h"
#include "irt_dma_hw.h"
#include "irtdma_log.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"

typedef struct irtdma_device {
        struct miscdevice cdev;   /* Char device structure */
        uint32_t dwirtdmaInst;
        #ifdef CONFIG_PM
        struct device * dev;
        #endif
}irtdma_device;

//static struct irtdma_device *g_irtdmadev = NULL;
extern void __iomem *irt_dma_base ;
extern unsigned int irtdmairq ;
extern struct clk* irtdmaclk ;
unsigned int Begin_call_arm1 =0;


#define WriteREG(arg, val) *(volatile UINT32 *)(IO_BASE_VA + (arg)) = val
#define ReadREG(arg)       *(volatile UINT32 *)(IO_BASE_VA + (arg))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

UINT32 _u4IRD_DBG_LVL = IRD_LOG_LVL_ERR;
UCHAR* _pcIrtDmaLogLevel[] = {
    "[IRTDMA OFF]",
    "[IRTDMA ERR]",
    "[IRTDMA WARN]",
    "[IRTDMA INFO]",
    "[IRTDMA HAL]",
    "[IRTDMA DBG]",
    "[IRTDMA IRQ]",
    "[IRTDMA REGRW]",
};

IRT_DMA_SRC_MODE_T g_rIrtSrcMode = {0, 0, 1, 16, 32,0};

static INT32 IrtDmaCheckSrcMode(UINT32 *pu4ScanLineMode, UINT32 *pu45351Mode, UINT32 *u4BlockBurstRead)
{

    IRD_LOG(IRD_LOG_LVL_DBG, "IrtDmaCheckSrcMode\n");

    g_rIrtSrcMode.fgScanLineMode = (BOOL)(*pu4ScanLineMode);
    g_rIrtSrcMode.fg5351Mode     = (BOOL)(*pu45351Mode);

    if(g_rIrtSrcMode.fgScanLineMode)
    {
        EV_IRTDMA_W_ALIGN = SCANLINE_MODE_WIDTH_AGLINE;
        EV_IRTDMA_H_ALIGN = SCANLINE_MODE_HEIGHT_AGLINE;
        g_rIrtSrcMode.uc5351ModeSel    = 0;
        g_rIrtSrcMode.fg5351Mode       = FALSE;
        g_rIrtSrcMode.fgBlockBurstRead = 0; //add@jgao ,only blcok mode support burst read ,others MUST be 0.
        *pu45351Mode                   = 0;
        *u4BlockBurstRead              = 0; //add@jgao
    }
    else if(g_rIrtSrcMode.fg5351Mode)
    {
        EV_IRTDMA_W_ALIGN = B5351_MODE_WIDTH_AGLINE;
        EV_IRTDMA_H_ALIGN = B5351_MODE_HEIGHT_AGLINE;
        g_rIrtSrcMode.fgBlockBurstRead = 0; //add@jgao ,only blcok mode support burst read,others MUST be 0.
        g_rIrtSrcMode.uc5351ModeSel    = 2; // only support mb 64x32.
        g_rIrtSrcMode.fgScanLineMode   = FALSE;
        *pu4ScanLineMode               = 0;     
        *u4BlockBurstRead              = 0; //add@jgao
    }
    else
    {
        EV_IRTDMA_W_ALIGN = BLOCK_MODE_WIDTH_AGLINE;
        EV_IRTDMA_H_ALIGN = BLOCK_MODE_HEIGHT_AGLINE;
        *u4BlockBurstRead = (*u4BlockBurstRead) & 0x1; //add@jgao       
        g_rIrtSrcMode.fgBlockBurstRead = *u4BlockBurstRead; //add@jgao ,only blcok mode support burst read.     
        g_rIrtSrcMode.uc5351ModeSel    = 0;
    }
    return (IRT_DMA_ERR_NONE);
}

static INT32 IrtDmaHwRotateMirror(UINT32 *u4SrcYSa,UINT32 *u4SrcCSa,UINT32 *u4DstYSa, UINT32 *u4DstCSa,UINT32 u4Width, UINT32 u4Height, IRT_DMA_MODE_T eMode)
{
    VERIFY(u4SrcYSa != NULL);
    VERIFY(u4SrcCSa != NULL);
    VERIFY(u4DstYSa != NULL);
    VERIFY(u4DstCSa != NULL);

    if(0 != (u4Width % IRT_DMA_MODE_AGLINE))
    {
        u4Width = ((u4Width / IRT_DMA_MODE_AGLINE + 1) * IRT_DMA_MODE_AGLINE);
    }

    if(0 != (u4Height % IRT_DMA_MODE_AGLINE))
    {
        u4Height = ((u4Height / IRT_DMA_MODE_AGLINE + 1) * IRT_DMA_MODE_AGLINE);
    }

    eMode = (eMode % IRT_DMA_MODE_MAX); 

    IRT_DMA_HwSrcModeSet(g_rIrtSrcMode.fg5351Mode, g_rIrtSrcMode.uc5351ModeSel, g_rIrtSrcMode.fgScanLineMode, g_rIrtSrcMode.fgBlockBurstRead);

    if(IRT_DMA_HwSrcSa(u4SrcYSa,u4SrcCSa))
    {
        IRD_LOG(IRD_LOG_LVL_ERR, "IrtDma_SrcSa Failed \r\n");
    }
    if(IRT_DMA_HwDstSa(u4DstYSa,u4DstCSa))
    {
        IRD_LOG(IRD_LOG_LVL_ERR, "IrtDma_DstSa Failed \r\n");
    }
    if (IRT_DMA_HwHVSize(u4Width,u4Height))
    {
        IRD_LOG(IRD_LOG_LVL_ERR, "IRT_DMA_HwWROffset Failed \r\n");
    }
    if(IRT_DMA_HwYCAndRotMode(eMode))
    {
        IRD_LOG(IRD_LOG_LVL_ERR, "IRT_DMA_HwRotMode Failed \r\n");
    }
    if(IRT_DMA_HwTrig())
    {
        IRD_LOG(IRD_LOG_LVL_ERR, "IrtDma_Trig Failed \r\n");
    }
    return (IRT_DMA_ERR_NONE);
}

static void IrtDma_FlterPara_H_V_M(UINT32 *pu4FrameWidth, UINT32 *pu4FrameHeight, UINT32 *pu4ModeOpt)
{   
    UINT32 u4Width  = *pu4FrameWidth;
    UINT32 u4Height = *pu4FrameHeight;
    UINT32 u4Mode   = *pu4ModeOpt;

    if(0 != (u4Width % EV_IRTDMA_W_ALIGN))
    {
        u4Width = ((u4Width / EV_IRTDMA_W_ALIGN + 1) * EV_IRTDMA_W_ALIGN);

        if (u4Width > EV_IRT_MAX_WIDTH)
        {
            u4Width -= EV_IRTDMA_W_ALIGN;
        }
    }
    *pu4FrameWidth = u4Width;

    if(0 != (u4Height % EV_IRTDMA_H_ALIGN))
    {
        u4Height = ((u4Height / EV_IRTDMA_H_ALIGN + 1) * EV_IRTDMA_H_ALIGN);

        if (u4Height > EV_IRT_MAX_HEIGHT)
        {
            u4Height -= EV_IRTDMA_H_ALIGN;
        }
    }
    *pu4FrameHeight = u4Height;

    *pu4ModeOpt = (u4Mode % IRT_DMA_MODE_MAX);
}

UINT32 IrtDma_Rotate(VOID * pCtrlPara)
{
	if(Begin_call_arm1 == 0)
		Begin_call_arm1 = 1;
    PIRT_DMA_APP_INFO_T pIrtDmaInfo = (PIRT_DMA_APP_INFO_T)pCtrlPara;    
    UINT32 *pu4SrcYBufAddr = pIrtDmaInfo->pu4SrcYBufAddr;
    UINT32 *pu4SrcCBufAddr = pIrtDmaInfo->pu4SrcCBufAddr;
    UINT32 *pu4DstYBufAddr = pIrtDmaInfo->pu4DstYBufAddr;
    UINT32 *pu4DstCBufAddr = pIrtDmaInfo->pu4DstCBufAddr;
    UINT32 u4FrameWidth   = pIrtDmaInfo->u4FrameWidth;
    UINT32 u4FrameHeight  = pIrtDmaInfo->u4FrameHeight;
    UINT32 u4ModeOpt      = pIrtDmaInfo->eModeOpt;
    UINT32 u4ScanLineMode = pIrtDmaInfo->fgScanLineMode;
    UINT32 u45351Mode     = pIrtDmaInfo->fg5351Mode;
    UINT32 u4BlockBurstRead = pIrtDmaInfo->fgBlockBurstRead; 
    clk_prepare_enable(irtdmaclk);  
    IRT_DMA_HwReset();
    IrtDmaCheckSrcMode(&u4ScanLineMode, &u45351Mode,&u4BlockBurstRead);
    IrtDma_FlterPara_H_V_M(&u4FrameWidth, &u4FrameHeight, &u4ModeOpt);

    IRD_LOG(IRD_LOG_LVL_DBG, "Paras: ScanLine=%d, 5351Mode=%d, W=%d, H=%d, Mode=%d..Align=%dx%d ,u4BlockBurstRead=%d,\
        SrcY:%x, SrcC:%x, DstY:%x, DstC:%x\r\n",u4ScanLineMode, u45351Mode, u4FrameWidth, u4FrameHeight, u4ModeOpt,
        EV_IRTDMA_W_ALIGN, EV_IRTDMA_H_ALIGN, u4BlockBurstRead, pu4SrcYBufAddr,pu4SrcCBufAddr, pu4DstYBufAddr, pu4DstCBufAddr);
    
    IrtDmaHwRotateMirror(pu4SrcYBufAddr, pu4SrcCBufAddr, pu4DstYBufAddr, pu4DstCBufAddr, u4FrameWidth, u4FrameHeight, u4ModeOpt); 
	clk_disable_unprepare(irtdmaclk);
    return (IRT_DMA_ERR_NONE);
}
EXPORT_SYMBOL(IrtDma_Rotate);

//#define PERFTEST
static int atc_irtdma_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0;

    return err;
}


static int atc_irtdma_open(struct inode *inode, struct file *file)
{
    IRD_LOG(IRD_LOG_LVL_DBG, "irtdma_open\n");

    /*printk(KERN_INFO "current pgd %p init_mm pgd %p\n",
            (unsigned long *)((current->mm)->pgd),
            (unsigned long *)init_mm.pgd);
    */
    IRD_LOG(IRD_LOG_LVL_DBG, "current pgd %p\n",(unsigned long *)((current->mm)->pgd));
    return 0;
}

static int atc_irtdma_release(struct inode *inode, struct file *file)
{
    IRD_LOG(IRD_LOG_LVL_DBG, "irtdma_release\n");

    return 0;
}

static struct file_operations irtdma_fops = {
    .owner          = THIS_MODULE,
    .open           = atc_irtdma_open,
    .release        = atc_irtdma_release,
    .unlocked_ioctl = atc_irtdma_ioctl,
};

static struct miscdevice irtdma_dev = {
	MISC_DYNAMIC_MINOR,
	"irtdma",
	&irtdma_fops
};


static int __init irtdma_init(void);

static int  irtdma_probe(struct platform_device *pdev)
{
   // struct irtdma_device *irtdmadev;
    int result;
    struct device_node *nd = pdev->dev.of_node;
    
   /* irtdmadev = kzalloc(sizeof(struct irtdma_device), GFP_KERNEL);
    if (irtdmadev == NULL) {
        dev_err(&pdev->dev, "[irtdma]: irtdma_probe: malloc device failed\n");
        return -ENOMEM;
    }

    platform_set_drvdata(pdev, irtdmadev);
 
    irtdmadev->cdev.name = "irtdma";
    irtdmadev->cdev.minor = MISC_DYNAMIC_MINOR;
    irtdmadev->cdev.fops = &irtdma_fops;
    #ifdef CONFIG_PM
    irtdmadev->dev = &(pdev->dev);
    #endif
    irtdmadev->cdev.parent = &(pdev->dev);

    result = misc_register(&(irtdmadev->cdev));

    if ( result == 0 )
    {
        printk(KERN_INFO "irtdma init successes\n");
    } 
    else 
    {
        printk(KERN_ERR "irtdma misc device register error\n");
        kfree(irtdmadev);
        return result;
    }
    g_irtdmadev = irtdmadev;
    #ifdef CONFIG_PM
    pm_runtime_enable(g_irtdmadev->dev);
    #endif

   // irtdma_init();*/
   irt_dma_base = of_iomap(nd, 0);

	if(!irt_dma_base) {
		IRD_LOG(IRD_LOG_LVL_ERR,"failed to get irtdma reg base %x\n",irt_dma_base);
		return -1;
	}
	IRD_LOG(IRD_LOG_LVL_ERR,"get irtdma reg base %x\n",irt_dma_base);

	irtdmairq = irq_of_parse_and_map(nd, 0);

	if(irtdmairq == NO_IRQ) {
		IRD_LOG(IRD_LOG_LVL_ERR,"failed to get irtdma irq %d\n",irtdmairq);
		return -1;
	}
	IRD_LOG(IRD_LOG_LVL_ERR,"get irtdma irq %d\n",irtdmairq);

	irtdmaclk = devm_clk_get(&pdev->dev, "irtdma-device");

	if(!irtdmaclk) {
		IRD_LOG(IRD_LOG_LVL_ERR,"failed to get irtdma clock %x\n",irtdmaclk);
		return -1;
	}
	IRD_LOG(IRD_LOG_LVL_ERR,"get irtdma clock %x\n",irtdmaclk);

	result = misc_register(&irtdma_dev);

    if ( result == 0 )
    {
        IRD_LOG(IRD_LOG_LVL_ERR,"irtdma init successes,12211\n");
    } 
    else 
    {
        IRD_LOG(IRD_LOG_LVL_ERR,"irtdma misc device register error\n");
       // kfree(irtdmadev);
        return result;
    }
    //g_irtdmadev = irtdmadev;
    #ifdef CONFIG_PM
    //pm_runtime_enable(g_irtdmadev->dev);
    #endif
	
    IRT_DMA_HwInit();    
    return 0;
}

static int irtdma_remove(struct platform_device *pdev)
{   
    /*struct irtdma_device *irtdmadev = platform_get_drvdata(pdev);
    
    if (irtdmadev == NULL)
    {
        printk(KERN_ERR "No device when irtdma remove!\n");
        return -ENODEV;
    }*/

    IRT_DMA_HwUnInit();
    
    misc_deregister(&irtdma_dev);
    
   // kfree(g_irtdmadev);
   // g_irtdmadev = NULL;
    
    return 0;
}


static int atc_irtdma_suspend(struct device *dev)
{
    //struct irtdma_device *irtdmadev;
   // struct platform_device *pdev = to_platform_device(dev);
    
    //irtdmadev = platform_get_drvdata(pdev);
    
    //if (irtdmadev == NULL)
       // return -ENODEV;
    //IRT_DMA_HwUnInit();
    clk_disable_unprepare(irtdmaclk);  
    IRD_LOG(IRD_LOG_LVL_DBG, "[irtdma]: %s\n", __func__);
    
    return 0;  
}

static int atc_irtdma_resume(struct device *dev)
{
    //struct irtdma_device *irtdmadev;
    //struct platform_device *pdev = to_platform_device(dev);
    
    //irtdmadev = platform_get_drvdata(pdev);
    //if (irtdmadev == NULL)
     //   return -ENODEV; 
    //IRT_DMA_HwInit();
    clk_prepare_enable(irtdmaclk);  
    IRD_LOG(IRD_LOG_LVL_DBG, "[irtdma]: %s\n", __func__);
    
    return 0;
}



static int atc_irtdma_runtime_suspend(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);
     return 0;
}
static int atc_irtdma_runtime_resume(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}

static int atc_irtdma_runtime_idle(struct device *dev)
{   
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}




static int atc_irtdma_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
    IRD_LOG(IRD_LOG_LVL_DBG, "[irtdma]: %s\n", __func__);
    return 0;
}
static int atc_irtdma_legacy_resume(struct platform_device *dev)
{
    IRD_LOG(IRD_LOG_LVL_DBG,"[irtdma]: %s\n", __func__);
    return 0;
}




static const struct dev_pm_ops atc_irtdma_dev_pm_ops = {
    .suspend= atc_irtdma_suspend,
	.resume= atc_irtdma_resume
//#ifdef CONFIG_PM_RUNTIME        
  //  SET_RUNTIME_PM_OPS(atc_irtdma_runtime_suspend, atc_irtdma_runtime_resume, atc_irtdma_runtime_idle)
//#endif  
};


static const struct of_device_id irtdma_of_ids[] = {
	{.compatible = "Autochips,ac83xx-irtdma",},
	{}
};


static struct platform_driver irtdma_plt_drv = {
    .driver = {
        .name = "ac83xx-irtdma",
        .owner = THIS_MODULE,
        .of_match_table = irtdma_of_ids,
        .pm = &atc_irtdma_dev_pm_ops,
            },
    .probe  = irtdma_probe,
    .remove = (irtdma_remove),
     .suspend = atc_irtdma_legacy_suspend,
     .resume = atc_irtdma_legacy_resume,

};

static void irtdma_plt_dev_release(struct device *dev)
{
    IRD_LOG(IRD_LOG_LVL_DBG,"irtdma_plt_dev_release called!\n");
}   

/*static struct platform_device irtdma_plt_dev = {
    .name = "ac83xx-irtdma",
    .id = 0,
    .dev.release = irtdma_plt_dev_release,
};*/

static int __init irtdma_init(void)
{
    int ret;

    ret = platform_driver_register(&irtdma_plt_drv);
    if (ret) {
        IRD_LOG(IRD_LOG_LVL_ERR, "[irtdma]: %s: register  driver failed\n", __func__);
        goto fail1;
    }   
    
    return ret;
    
fail1:
   // platform_device_unregister(&irtdma_plt_dev);
fail0:
    return ret;
}
module_init(irtdma_init);

static void __exit irtdma_exit(void)
{
    platform_driver_unregister(&irtdma_plt_drv);
}
module_exit(irtdma_exit);


MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("xzr");


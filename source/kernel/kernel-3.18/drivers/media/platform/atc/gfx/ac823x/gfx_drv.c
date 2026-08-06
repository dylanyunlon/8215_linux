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
#include <linux/clk.h>
#include <linux/clk-private.h>

#include "x_bim_83xx.h"
#include <linux/of.h>
#include <linux/of_address.h>

#include <linux/of_irq.h>

//#include "gfx.h"
#include "x_ckgen.h"
#include "oal.h"
#include "drv_gfx.h"
#include "gfx_hal_if.h"
#include "gfx_if.h"
#include "gfx_dif.h"
#include "x_ver.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define MMISC_MODE_NAME                   "GFX"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                     00
#define NO_IRQ 0
//static struct gfx_device *g_gfxdev = NULL;

 void __iomem * gfx_addr;
 unsigned long gfx_base;
 unsigned int gfxirq;
 struct clk* gfxclk;
 struct clk* gfxselect;

extern void GFX_EnableClk(void);

static struct mutex gfx_mutex;

//#define PERFTEST
static int atc_gfx_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err;
	uint32_t u4TTB = 0;
	gfx_ioc_paras gfx_paras;
	#ifdef PERFTEST
	UINT8 *pu1DstAddr = NULL;
	UINT8 *pu1DstAddrUser = NULL;
	uint32_t u4DstPitch,u4DstX,u4DstY,u4Width,u4Height;
	static struct timeval start,end;
	#endif
	
	mutex_lock(&gfx_mutex);

	//printk("current pid is 0x%08X\n",current->pid);
	if (0 != copy_from_user(&gfx_paras, (void __user *)arg, sizeof(gfx_ioc_paras)))
	{
		mutex_unlock(&gfx_mutex);
		return -EFAULT;
	}

	if((current->pid != gfx_paras.pid) && (current->tgid != gfx_paras.pid))
	{
		pr_debug("[GFX]pid changed\r\n");
		mutex_unlock(&gfx_mutex);
		return -EFAULT;
	}
	//u4TTB = (uint32_t)u4HalGetTTB0();
		
	switch(cmd)
	{
		case GFX_IOC_BITBLT:
			err = gfx_bitblt(u4TTB,&gfx_paras);
			break;
		case GFX_IOC_FILLRECT:
			#ifdef PERFTEST
			pu1DstAddrUser = (UINT8 *)((gfx_paras.dstBuffer).u4Addr);
			u4DstPitch 	= (gfx_paras.dstBuffer).u4Pitch;
			u4DstX		= (gfx_paras.dstBuffer).u4X;
			u4DstY		= (gfx_paras.dstBuffer).u4Y;
			u4Width		= (gfx_paras.dstBuffer).u4Width;
			u4Height	= (gfx_paras.dstBuffer).u4Height;
			pu1DstAddr = (UINT8 *)kmalloc(u4DstPitch*(u4DstY+u4Height), GFP_KERNEL);
			if(NULL == pu1DstAddr)
				return -EFAULT;

			pr_debug("[GFX]dst addr :%x \r\n",pu1DstAddr);
			gfx_paras.dstBuffer.u4Addr = (unsigned long)((unsigned long)pu1DstAddr & 0x3fffffff);
			
        	do_gettimeofday(&start);
			err = gfx_fillrect(u4TTB,&gfx_paras);
			if(!err)
			{
				do_gettimeofday(&end);
				pr_debug("[GFX]hw costtime:%d",(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
				if(copy_to_user((void*)(pu1DstAddrUser + u4DstPitch*u4DstY + u4DstX),
					(void*)(pu1DstAddr+ u4DstPitch*u4DstY + u4DstX),u4Width*u4Height))
				{
					kfree(pu1DstAddr);
					pr_debug("[GFX]:copy to user failed\r\n");
				}
			}
			kfree(pu1DstAddr);
			
			#else
			err = gfx_fillrect(u4TTB,&gfx_paras);	
			#endif
			break;
		case GFX_IOC_STRETCHBLT:
			err = gfx_stretchblt(u4TTB,&gfx_paras);
			break;
		case GFX_IOC_ALPHABLEND:
			err = gfx_alphablend(u4TTB,&gfx_paras);
			break;
		default:
			pr_err( "[GFX]:No handler for ioctl 0x%08X 0x%08lX\n", cmd, arg);
			err = -ENOTTY;
	}
	mutex_unlock(&gfx_mutex);
	
	return err;
}


static int atc_gfx_open(struct inode *inode, struct file *file)
{

   // pr_info( "gfx_open\n");

	/*pr_info( "current pgd %p init_mm pgd %p\n",
			(unsigned long *)((current->mm)->pgd),
			(unsigned long *)init_mm.pgd);
	*/
	//pr_info( "current pgd %p\n",(unsigned long *)((current->mm)->pgd));
    return 0;
}

static int atc_gfx_release(struct inode *inode, struct file *file)
{
	
    //pr_info( "gfx_release\n");

    return 0;
}

static struct file_operations gfx_fops = {
	.owner			= THIS_MODULE,
    .open 			= atc_gfx_open,
    .release 		= atc_gfx_release,
    .unlocked_ioctl = atc_gfx_ioctl,
};
static struct miscdevice gfx_dev = {
	MISC_DYNAMIC_MINOR,
	"gfx",
	&gfx_fops
};


static int  gfx_probe(struct platform_device *pdev)
{
	int result;
    struct device_node * nd = pdev->dev.of_node;

	if(nd == NULL) {
		printk(KERN_ERR "[GFX]:nd is NULL\n");
		return -1;
	}
   // irtdma_init();*/
   gfx_addr= of_iomap(nd, 0);

	if(!gfx_addr) {
		printk(KERN_ERR "[GFX]:failed to get gfx reg base %x\n",gfx_addr);
		return -1;
	}
	gfx_base = (unsigned long)gfx_addr;
	printk(KERN_ERR "[GFX]:get gfx base addr %lx\n",gfx_base);

	gfxirq = irq_of_parse_and_map(nd, 0);
	gfxirq = gfxirq - 32;

	if(gfxirq == NO_IRQ) {
		printk(KERN_ERR "[GFX]:failed to get gfx irq %d\n",gfxirq);
		return -1;
	}
	printk(KERN_ERR "[GFX]:get gfx irq %d\n",gfxirq);

	gfxclk = devm_clk_get(&pdev->dev, "gfx-device");

	if(!gfxclk) {
		printk(KERN_ERR "[GFX]:failed to get gfx clock %x\n",gfxclk);
		return -1;
	}
	printk(KERN_ERR "[GFX]:get gfx clock %x\n",gfxclk);

	gfxselect= devm_clk_get(&pdev->dev, "gfx-select");

	if(!gfxselect) {
		printk(KERN_ERR "[GFX]:failed to get gfx selsect clock %x\n",gfxselect);
		return -1;
	}
	printk(KERN_ERR "[GFX]:get gfxclock %x\n",gfxselect);

	result = misc_register(&gfx_dev);

    if ( result == 0 )
    {
        printk(KERN_INFO "[GFX]:gfx register successes\n");
    } 
    else 
    {
        printk(KERN_ERR "[GFX]:gfx device register error\n");
       // kfree(irtdmadev);
        return result;
    }
    
	
    result = GFX_Init(); 

	  if ( result == (INT32)E_GFX_OK)
    {
        printk(KERN_INFO "[GFX]:gfx register successes\n");
    } 
    else 
    {
        printk(KERN_ERR "[GFX]:gfx device register error\n");
       // kfree(irtdmadev);
        return result;
    }
	mutex_init(&gfx_mutex);
    return 0;
}

static int  gfx_remove(struct platform_device *pdev)
{	
	
	misc_deregister(&gfx_dev);
	i4GFXUninit();
	return 0;
}



static int atc_gfx_suspend(struct device *dev)
{
	

	//mutex_lock(&gfx_mutex);

	
	clk_disable_unprepare(gfxclk);
	
	//mutex_unlock(&gfx_mutex);
	pr_info( "[GFX]: suspend ,%s\n", __func__);
	
    return 0;  
}

static int atc_gfx_resume(struct device *dev)
{
	

	//mutex_lock(&gfx_mutex);

	GFX_EnableClk();
	//mutex_unlock(&gfx_mutex);
	
    pr_info( "[GFX]: resume %s\n", __func__);
	
    return 0;
}



static int atc_gfx_runtime_suspend(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);
     return 0;
}
static int atc_gfx_runtime_resume(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}

static int atc_gfx_runtime_idle(struct device *dev)
{ 
  
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}




static int atc_gfx_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
    pr_info( "[GFX]: %s\n", __func__);
    return 0;
}
static int atc_gfx_legacy_resume(struct platform_device *dev)
{
    pr_info( "[GFX]: %s\n", __func__);
    return 0;
}




static const struct dev_pm_ops atc_gfx_dev_pm_ops = {
	.suspend = atc_gfx_suspend,
	.resume = atc_gfx_resume
};

static const struct of_device_id gfx_of_ids[] = {
	{.compatible = "Autochips,ac83xx-gfx",},
	{}
};




static struct platform_driver gfx_plt_drv = {
	.driver = {
		.name = "ac83xx-gfx",
		.owner = THIS_MODULE,
		.of_match_table = gfx_of_ids,
		.pm = &atc_gfx_dev_pm_ops,	
			},
	.probe  = gfx_probe,
	.remove = gfx_remove,
     .suspend = atc_gfx_legacy_suspend,
	 .resume = atc_gfx_legacy_resume,

};

static int __init gfx_init(void)
{
	int ret;
	ret = platform_driver_register(&gfx_plt_drv);
	if (ret) {
		pr_err( "[GFX]: %s: register  driver failed\n", __func__);
		goto fail1;
	}	
	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);
	return ret;
	
fail1:
	//platform_device_unregister(&gfx_plt_dev);
fail0:
	return ret;
}
module_init(gfx_init);

static void __exit gfx_exit(void)
{
	platform_driver_unregister(&gfx_plt_drv);
	
}
module_exit(gfx_exit);


MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Ziran Xu <ziran.xu@autochips.com>");


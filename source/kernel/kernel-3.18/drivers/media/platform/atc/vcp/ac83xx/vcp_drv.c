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
#ifndef VCP_FOR_ANDROID

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
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/clk.h>

#include "x_bim_83xx.h"
#include "oal.h"
#include "atc/drv_vcp.h"
#include "atc/cp.h"
#include "hal/cp_def.h"
#include "hal/cp_reg.h"
#include "x_ver.h"


#define ATC_KERNEL_LINUX_LICENSE     "GPL"

#define MMISC_MODE_NAME                   "VCP"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                     00

static struct vcp_device *g_vcpdev;

static DEFINE_MUTEX(vcp_mutex);
/* UINT32 */

u32 u4VcpEnableReg;
u32 u4VcpHueReg;
u32 u4VcpYGainReg;
u32 u4VcpUVGainReg;
u32 u4VcpBrightReg;
u32 u4VcpSaturationReg;

static long atc_vcp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;

	mutex_lock(&vcp_mutex);

	switch (cmd) {
	case (unsigned int)VCP_IOC_PG:
		{
			vcp_pg_paras karg;

			if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_pg_paras)))
				goto ERR;
			vCPPatterGenerate(karg.i4Channel, karg.i4En, karg.i4Step, karg.i4Prec);
		}
		break;

	case (unsigned int)VCP_IOC_ON_OFF:
		{
			vcp_onoff_para karg;

			if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_onoff_para)))
				goto ERR;
			if (!VcpOnOff(karg.u4VcpIdx, karg.fgOnOrOff))
				goto ERR;
		}
		break;

	case (unsigned int)VCP_IOC_SET_HUE:
		{
			vcp_hue_para karg;
        
			if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_hue_para)))
				goto ERR;
			VcpSetHue(karg.u4VcpIdx, karg.u4Hue);
		}
		break;

	case (unsigned int)VCP_IOC_GET_HUE:
		{
			vcp_hue_para karg;
        
			if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_hue_para)))
				goto ERR;
			karg.u4Hue = VcpGetHue(karg.u4VcpIdx);
			if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_hue_para)))
				goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_YGAIN:
		{
            vcp_ygain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_ygain_para)))
                goto ERR;
            VcpSetYGain(karg.u4VcpIdx, karg.u4YGain);
		}
        break;

    case (unsigned int)VCP_IOC_GET_YGAIN:
		{
            vcp_ygain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_ygain_para)))
                goto ERR;
            karg.u4YGain = VcpGetYGain(karg.u4VcpIdx);
            if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_ygain_para)))
                goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_UGAIN:
		{
            vcp_ugain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_ugain_para)))
                goto ERR;
            VcpSetUGain(karg.u4VcpIdx, karg.u4UGain);
		}
		break;

    case (unsigned int)VCP_IOC_GET_UGAIN:
		{
            vcp_ugain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_ugain_para)))
                goto ERR;
            karg.u4UGain = VcpGetUGain(karg.u4VcpIdx);
            if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_ugain_para)))
                goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_VGAIN:
		{
            vcp_vgain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_vgain_para)))
                goto ERR;
            VcpSetVGain(karg.u4VcpIdx, karg.u4VGain);
		}
		break;

    case (unsigned int)VCP_IOC_GET_VGAIN:
		{
            vcp_vgain_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_vgain_para)))
                goto ERR;
            karg.u4VGain = VcpGetVGain(karg.u4VcpIdx);
            if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_vgain_para)))
                goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_CONTR:
		{
            vcp_contr_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_contr_para)))
                goto ERR;
            VcpSetContrast(karg.u4VcpIdx, karg.u4Contrast);
		}
		break;

    case (unsigned int)VCP_IOC_GET_CONTR:
		{
            vcp_contr_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_contr_para)))
                goto ERR;
            karg.u4Contrast = VcpGetContrast(karg.u4VcpIdx);
            if (0 !=copy_to_user((void __user *)arg, &karg, sizeof(vcp_contr_para)))
                goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_BRIGH:
		{
            vcp_brigh_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_brigh_para)))
                goto ERR;
            VcpSetBrightness(karg.u4VcpIdx, karg.u4Brightness);
		}
		break;

    case (unsigned int)VCP_IOC_GET_BRIGH:
		{
            vcp_brigh_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_brigh_para)))
                goto ERR;
            karg.u4Brightness = VcpGetBrightness(karg.u4VcpIdx);
            if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_brigh_para)))
                goto ERR;
		}
		break;

    case (unsigned int)VCP_IOC_SET_SATUR:
		{
            vcp_satur_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_satur_para)))
                goto ERR;
            VcpSetSaturation(karg.u4VcpIdx, karg.u4Saturation);
		}
		break;

    case (unsigned int)VCP_IOC_GET_SATUR:
		{
            vcp_satur_para karg;

            if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_satur_para)))
                goto ERR;
            karg.u4Saturation = VcpGetSaturation(karg.u4Saturation);
            if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_satur_para)))
                goto ERR;
		}
		break;

	default:
		pr_err("[VCP] No handler for ioctl 0x%08X 0x%08lX\n", cmd, arg);
		pr_err
		    ("[VCP] Now only support VCP Bypass/pattern generate/contrast brightness saturation!\n");
		err = -ENOTTY;
	}
	mutex_unlock(&vcp_mutex);

	return err;

ERR:
	mutex_unlock(&vcp_mutex);
	return -EFAULT;
}


static int atc_vcp_open(struct inode *inode, struct file *file)
{

	pr_info("[VCP] vcp_open\n");

	return 0;
}

static int atc_vcp_release(struct inode *inode, struct file *file)
{
	pr_info("[VCP] vcp_release\n");

	return 0;
}

static const struct file_operations vcp_fops = {
	.owner = THIS_MODULE,
	.open = atc_vcp_open,
	.release = atc_vcp_release,
	.unlocked_ioctl = atc_vcp_ioctl,
};

void __iomem *vcp_sysreg_base = NULL;
struct clk *clk_ac8317_vcp = NULL;

static int vcp_probe(struct platform_device *pdev)
{
	struct vcp_device *vcpdev;
	struct device_node *nd = pdev->dev.of_node;
	int result;
	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);
	
	vcp_sysreg_base = of_iomap(nd, (int)0);
	if (!vcp_sysreg_base) {
		pr_err("[VCP] get vcp register base address failed\n");
		return -1;
	}
	pr_info("[VCP]  vcp register base address = %x\n", (unsigned int)vcp_sysreg_base);

	clk_ac8317_vcp = devm_clk_get(&pdev->dev, "vcp-device");
	if (!clk_ac8317_vcp) {
		pr_err("[VCP] get vcp clk failed %p\r\n", clk_ac8317_vcp);
		return -1;
	}
	pr_info("[VCP] get vcp clk success %p\r\n", clk_ac8317_vcp);

	vcpdev = kzalloc(sizeof(struct vcp_device), GFP_KERNEL);
	if (vcpdev == NULL) {
		/* dev_err(&pdev->dev, "[vcp]: vcp_probe: malloc device failed\n");  */
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, vcpdev);

	vcpdev->cdev.name = "vcp";
	vcpdev->cdev.minor = MISC_DYNAMIC_MINOR;
	vcpdev->cdev.fops = &vcp_fops;
#ifdef CONFIG_PM
	vcpdev->dev = &(pdev->dev);
#endif
	vcpdev->cdev.parent = &(pdev->dev);

	result = misc_register(&(vcpdev->cdev));

	if (result == 0) {
		pr_info("[VCP] vcp init successes\n");
	} else {
		pr_err("[VCP] vcp misc device register error\n");
		kfree(vcpdev);
		return result;
	}
	g_vcpdev = vcpdev;
#ifdef CONFIG_PM
	pm_runtime_enable(g_vcpdev->dev);
#endif
	/* call vcp HW init function */
	/* vcp_Init();vcp HW dont need open clock... */
	return 0;
}

/* static int __devexit vcp_remove(struct platform_device *pdev) */
static int vcp_remove(struct platform_device *pdev)
{
	struct vcp_device *vcpdev = platform_get_drvdata(pdev);

	if (vcpdev == NULL) {
		pr_err("[VCP] %s No device when vcp remove!\n",__func__);
		return -ENODEV;
	}

	/* todo:release something when remove */
	/* i4vcpUninit(); */

	misc_deregister(&(vcpdev->cdev));

	kfree(g_vcpdev);
	g_vcpdev = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int atc_vcp_suspend(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);

	vcpdev = platform_get_drvdata(pdev);

	if (vcpdev == NULL)
		return -ENODEV;

	mutex_lock(&vcp_mutex);

	pr_info("[VCP] atc_vcp_suspend:backup vcp 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 0x1F080,
		RW_PCLRP_HUE_SCECTRL, RW_PCLRP_GAIN_Y, RW_PCLRP_GAIN_UV, RW_PCLRP_BRIGHT_CONT,
		RW_PCLRP_SATURATION);
	memcpy((void *)&u4VcpEnableReg, (u32 *) (IO_BASE_ADDRESS + 0x1F080), (size_t)0x4);
	memcpy((void *)&u4VcpHueReg, (u32 *) (IO_BASE_ADDRESS + RW_PCLRP_HUE_SCECTRL), (size_t)0x4);
	memcpy((void *)&u4VcpYGainReg, (u32 *) (IO_BASE_ADDRESS + RW_PCLRP_GAIN_Y), (size_t)0x4);
	memcpy((void *)&u4VcpUVGainReg, (u32 *) (IO_BASE_ADDRESS + RW_PCLRP_GAIN_UV), (size_t)0x4);
	memcpy((void *)&u4VcpBrightReg, (u32 *) (IO_BASE_ADDRESS + RW_PCLRP_BRIGHT_CONT), (size_t)0x4);
	memcpy((void *)&u4VcpSaturationReg, (u32 *) (IO_BASE_ADDRESS + RW_PCLRP_SATURATION), (size_t)0x4);
	clk_disable_unprepare(clk_ac8317_vcp);

	mutex_unlock(&vcp_mutex);
	pr_info("[VCP] : %s\r\n", __func__);

	return 0;
}

static int atc_vcp_resume(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);

	vcpdev = platform_get_drvdata(pdev);
	if (vcpdev == NULL)
		return -ENODEV;

	mutex_lock(&vcp_mutex);

	clk_prepare_enable(clk_ac8317_vcp);
	pr_info("[VCP] atc_vcp_resume:backup vcp 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 0x1F080,
		RW_PCLRP_HUE_SCECTRL, RW_PCLRP_GAIN_Y, RW_PCLRP_GAIN_UV, RW_PCLRP_BRIGHT_CONT,
		RW_PCLRP_SATURATION);
	memcpy((u32 *) (IO_BASE_ADDRESS + 0x1F080), &u4VcpEnableReg, (size_t)0x4);
	memcpy((u32 *) (IO_BASE_ADDRESS + RW_PCLRP_HUE_SCECTRL), &u4VcpHueReg, (size_t)0x4);
	memcpy((u32 *) (IO_BASE_ADDRESS + RW_PCLRP_GAIN_Y), &u4VcpYGainReg, (size_t)0x4);
	memcpy((u32 *) (IO_BASE_ADDRESS + RW_PCLRP_GAIN_UV), &u4VcpUVGainReg, (size_t)0x4);
	memcpy((u32 *) (IO_BASE_ADDRESS + RW_PCLRP_BRIGHT_CONT), &u4VcpBrightReg, (size_t)0x4);
	memcpy((u32 *) (IO_BASE_ADDRESS + RW_PCLRP_SATURATION), &u4VcpSaturationReg, (size_t)0x4);

	mutex_unlock(&vcp_mutex);

	pr_info("[VCP]: %s\r\n", __func__);

	return 0;
}


#ifdef CONFIG_PM_RUNTIME
static int atc_vcp_runtime_suspend(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}

static int atc_vcp_runtime_resume(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}

static int atc_vcp_runtime_idle(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}
#endif
#endif

#ifdef CONFIG_PM_SLEEP
static int atc_vcp_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
	pr_info("[VCP]: %s\n", __func__);
	return 0;
}

static int atc_vcp_legacy_resume(struct platform_device *dev)
{
	pr_info("[VCP]: %s\n", __func__);
	return 0;
}
#endif


#ifdef CONFIG_PM
static const struct dev_pm_ops atc_vcp_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(atc_vcp_suspend, atc_vcp_resume)
#ifdef CONFIG_PM_RUNTIME
	    SET_RUNTIME_PM_OPS(atc_vcp_runtime_suspend, atc_vcp_runtime_resume,
			       atc_vcp_runtime_idle)
#endif
};
#endif

static const struct of_device_id vcp_of_ids[] = {
	{.compatible = "Autochips,colorprocess",},
	{}
};

static struct platform_driver vcp_plt_drv = {
	.driver = {
		   .name = "atc3363-vcp",
		   .owner = THIS_MODULE,
		   .of_match_table = vcp_of_ids,
#ifdef CONFIG_PM
		   .pm = &atc_vcp_dev_pm_ops,
#endif
		   },
	.probe = vcp_probe,
	/* .remove = __devexit_p(vcp_remove), */
	.remove = vcp_remove,
#ifdef CONFIG_PM_SLEEP
	.suspend = atc_vcp_legacy_suspend,
	.resume = atc_vcp_legacy_resume,
#endif
};

/*
static void vcp_plt_dev_release(struct device *dev)
{
	printk("vcp_plt_dev_release called!\n");
}

static struct platform_device vcp_plt_dev = {
    .name = "atc3363-vcp",
	.id = 0,
	.dev.release = vcp_plt_dev_release,
};
*/
static int __init vcp_init(void)
{
	int ret;

	pr_info("[VCP]vcp_init--->\n");
	ret = platform_driver_register(&vcp_plt_drv);
	if (ret)
		pr_err("[VCP]: %s: register  driver failed\n", __func__);
	return ret;
}
module_init(vcp_init);

static void __exit vcp_exit(void)
{
	pr_info("[VCP]vcp_exit--->\n");
	platform_driver_unregister(&vcp_plt_drv);
}
module_exit(vcp_exit);


MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Ziran Xu <ziran.xu@autochips.com>");

#else

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
#include "x_bim_83xx.h"
#include "oal.h"
#include "drv_vcp.h"
#include "atc/cp.h"
#include "hal/cp_def.h"
#include "hal/cp_reg.h"
//#include "x_bim_83xx.h"
//#include "oal.h"
//#include "drv_vcp.h"
//#include "cp.h"
//#include "cp_def.h"
//#include "cp_reg.h"



#define ATC_KERNEL_LINUX_LICENSE     "GPL"

static struct vcp_device *g_vcpdev = NULL;

static DEFINE_MUTEX(vcp_mutex);
//UINT32 

UINT32 u4VcpEnableReg;
UINT32 u4VcpHueReg;
UINT32 u4VcpYGainReg;
UINT32 u4VcpUVGainReg;
UINT32 u4VcpBrightReg;
UINT32 u4VcpSaturationReg;

static long atc_vcp_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
    INT32 tempvalue;
	
	mutex_lock(&vcp_mutex);
		
	switch(cmd)
	{
		case VCP_IOC_BYPASS:
			{
				int karg;
				if (0 != get_user(karg, (int __user *)arg))
				{
					goto ERR;
				}
				vCPBypass(karg);
			}
			break;

		case VCP_IOC_PG:
			{
				vcp_pg_paras karg;
				if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_pg_paras)))
				{
					goto ERR;
				}
				vCPPatterGenerate(karg.i4Channel,karg.i4En,karg.i4Step,karg.i4Prec);
			}
			break;

		case VCP_IOC_SET_CBS:
			{
				vcp_cbs_paras karg;
				if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_cbs_paras)))
				{
					goto ERR;
				}
				vCPAppSetContrBritSatr(karg.i4Contr, karg.i4Brit, karg.i4Satr, karg.srctype);
			}
			break;
		case VCP_IOC_GET_CBS:
			{
				vcp_cbs_paras karg;
                vCPGetContrBritSatr(&karg.i4Contr, &karg.i4Brit, &karg.i4Satr);
				if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_cbs_paras)))
				{
					goto ERR;
				}
			}
			break;      
            
		case VCP_IOC_ON:
			{
				int karg;
				if (0 != get_user(karg, (int __user *)arg))
				{
					goto ERR;
				}
				vCPOn(karg);
			}
			break;
		case VCP_IOC_OFF:
			{
				int karg;
				if (0 != get_user(karg, (int __user *)arg))
				{
					goto ERR;
				}
				vCPOff(karg);
			}
			break;
        case VCP_IOC_SET_YUV:
            {
                vcp_yuv_paras karg;                
                if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_yuv_paras)))
				{
					goto ERR;
				}
                vCPAppSetYUVGain(karg.i4YGain, karg.i4UGain, karg.i4VGain, karg.srctype);
            }
            break;
        case VCP_IOC_GET_YUV:
            {

                vcp_yuv_paras karg;
                vCPGetYUVGain(&karg.i4YGain, &karg.i4UGain, &karg.i4VGain);
                if (0 != copy_to_user((void __user *)arg, &karg, sizeof(vcp_yuv_paras)))
				{
					goto ERR;
				}
            }
            break;
        case VCP_IOC_SET_HUE:
            {

                vcp_hue_paras karg;
                if (0 != copy_from_user(&karg, (void __user *)arg, sizeof(vcp_hue_paras)))
                {
					goto ERR;
				}
                vCPAppSetGlobalHue(karg.i4hue, karg.srctype);
            }
            break;
        case VCP_IOC_GET_HUE:
            {

                tempvalue = vCPGetGlobalHue();
                if (0 != put_user(tempvalue, (int __user *)arg))
				{
					goto ERR;
				}
            }
            break;               
 
		default:
			printk(KERN_ERR "[VCP][ERR]No handler for ioctl 0x%08X 0x%08lX\n", cmd, arg);
			printk(KERN_WARNING "[VCP][WARN]Now only support VCP Bypass/pattern generate/contrast brightness saturation!\n");
			err = -ENOTTY;
	}
	mutex_unlock(&vcp_mutex);

	return err;

ERR:
	mutex_unlock(&vcp_mutex);
	return -EFAULT;
}


static int atc_vcp_open(struct inode *inode, struct file *file)
{

    printk(KERN_INFO "[VCP][INFO]vcp_open\n");

    return 0;
}

static int atc_vcp_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[VCP][INFO]vcp_release\n");

    return 0;
}

static struct file_operations vcp_fops = {
	.owner			= THIS_MODULE,
    .open 			= atc_vcp_open,
    .release 		= atc_vcp_release,
    .unlocked_ioctl = atc_vcp_ioctl,
};

static int vcp_probe(struct platform_device *pdev)
{
	struct vcp_device *vcpdev;
	int result;
	
	vcpdev = kzalloc(sizeof(struct vcp_device), GFP_KERNEL);
	if (vcpdev == NULL) {
		dev_err(&pdev->dev, "[VCP][ERR]: vcp_probe: malloc device failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, vcpdev);

 
    vcpdev->cdev.name = "vcp";
    vcpdev->cdev.minor = MISC_DYNAMIC_MINOR;
    vcpdev->cdev.fops = &vcp_fops;
  	#ifdef CONFIG_PM
	vcpdev->dev = &(pdev->dev);
	#endif
	vcpdev->cdev.parent = &(pdev->dev);

    result = misc_register(&(vcpdev->cdev));

    if ( result == 0 )
    {
        printk(KERN_INFO "[VCP][ERR]vcp init successes\n");
    } 
	else 
    {
        printk(KERN_ERR "[VCP][ERR]vcp misc device register error\n");
        kfree(vcpdev);
		return result;
    }
	g_vcpdev = vcpdev;
	#ifdef CONFIG_PM
	pm_runtime_enable(g_vcpdev->dev);
	#endif
    //call vcp HW init function 
    //vcp_Init();vcp HW dont need open clock...
	return 0;
}

static int vcp_remove(struct platform_device *pdev)
{	
	struct vcp_device *vcpdev = platform_get_drvdata(pdev);
	
	if (vcpdev == NULL)
	{
		printk(KERN_ERR "[VCP][ERR]No device when vcp remove!\n");
		return -ENODEV;
	}

	
	//todo:release something when remove
	//i4vcpUninit();
	
	misc_deregister(&(vcpdev->cdev));
	
	kfree(g_vcpdev);
	g_vcpdev = NULL;
	
	return 0;
}

#ifdef CONFIG_PM
static int atc_vcp_suspend(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);
	
	vcpdev = platform_get_drvdata(pdev);
	
	if (vcpdev == NULL)
		return -ENODEV;

	mutex_lock(&vcp_mutex);
    
    printk(KERN_INFO "[VCP][INFO]atc_vcp_suspend:backup vcp 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\r\n",0x1F080,RW_PCLRP_HUE_SCECTRL,RW_PCLRP_GAIN_Y,RW_PCLRP_GAIN_UV,RW_PCLRP_BRIGHT_CONT,RW_PCLRP_SATURATION);
    memcpy(&u4VcpEnableReg, (UINT32 *)(IO_BASE_ADDRESS + 0x1F080), 0x4);
    memcpy(&u4VcpHueReg, (UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_HUE_SCECTRL), 0x4);
    memcpy(&u4VcpYGainReg, (UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_GAIN_Y), 0x4);
    memcpy(&u4VcpUVGainReg, (UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_GAIN_UV), 0x4);
    memcpy(&u4VcpBrightReg, (UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_BRIGHT_CONT), 0x4);
    memcpy(&u4VcpSaturationReg, (UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_SATURATION), 0x4);

	mutex_unlock(&vcp_mutex);
	printk(KERN_INFO "[VCP][INFO]: %s\r\n", __func__);
	
    return 0;  
}

static int atc_vcp_resume(struct device *dev)
{
	struct vcp_device *vcpdev;
	struct platform_device *pdev = to_platform_device(dev);
	
	vcpdev = platform_get_drvdata(pdev);
	if (vcpdev == NULL)
		return -ENODEV;

	mutex_lock(&vcp_mutex);
    
    printk(KERN_INFO "[VCP][INFO]atc_vcp_resume:backup vcp 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\r\n",0x1F080,RW_PCLRP_HUE_SCECTRL,RW_PCLRP_GAIN_Y,RW_PCLRP_GAIN_UV,RW_PCLRP_BRIGHT_CONT,RW_PCLRP_SATURATION);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + 0x1F080), &u4VcpEnableReg, 0x4);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_HUE_SCECTRL), &u4VcpHueReg, 0x4);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_GAIN_Y), &u4VcpYGainReg, 0x4);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_GAIN_UV), &u4VcpUVGainReg, 0x4);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_BRIGHT_CONT), &u4VcpBrightReg, 0x4);
    memcpy((UINT32 *)(IO_BASE_ADDRESS + RW_PCLRP_SATURATION), &u4VcpSaturationReg, 0x4);		
	
	mutex_unlock(&vcp_mutex);
	
    printk(KERN_INFO "[VCP][INFO]: %s\r\n", __func__);
	
    return 0;
}


#ifdef CONFIG_PM_RUNTIME
static int atc_vcp_runtime_suspend(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);
     return 0;
}
static int atc_vcp_runtime_resume(struct device *dev)
{
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}

static int atc_vcp_runtime_idle(struct device *dev)
{ 
  
     struct platform_device *pdev = to_platform_device(dev);

     return 0;
}
#endif
#endif

#ifdef CONFIG_PM_SLEEP
static int atc_vcp_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
    printk(KERN_INFO "[VCP][INFO]: %s\n", __func__);
    return 0;
}
static int atc_vcp_legacy_resume(struct platform_device *dev)
{
    printk(KERN_INFO "[VCP][INFO]: %s\n", __func__);
    return 0;
}
#endif


#ifdef CONFIG_PM
static const struct dev_pm_ops atc_vcp_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(atc_vcp_suspend, atc_vcp_resume)
#ifdef CONFIG_PM_RUNTIME		
	SET_RUNTIME_PM_OPS(atc_vcp_runtime_suspend, atc_vcp_runtime_resume, atc_vcp_runtime_idle)
#endif	
};
#endif



static struct platform_driver vcp_plt_drv = {
	.driver = {
		.name = "atc3363-vcp",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM	
		.pm = &atc_vcp_dev_pm_ops,
#endif		
			},
	.probe  = vcp_probe,
	.remove = vcp_remove,
#ifdef CONFIG_PM_SLEEP
     .suspend = atc_vcp_legacy_suspend,
	 .resume = atc_vcp_legacy_resume,
#endif
};

static void vcp_plt_dev_release(struct device *dev)
{
	printk("[VCP][INFO]vcp_plt_dev_release called!\n");
}	

static struct platform_device vcp_plt_dev = {
    .name = "atc3363-vcp",
	.id = 0,
	.dev.release = vcp_plt_dev_release,
};

static int __init vcp_init(void)
{
	int ret;
	ret = os_device_register(&vcp_plt_dev);
	if (ret) {
		printk(KERN_ERR "[VCP][ERR]: %s: register  device failed\n", __func__); 
		goto fail0;
	}
	
	ret = os_driver_register(&vcp_plt_drv);
	if (ret) {
		printk(KERN_ERR "[VCP][ERR]: %s: register  driver failed\n", __func__);
		goto fail1;
	}	
    
	VcpInit();
    
	return ret;
	
fail1:
	os_device_unregister(&vcp_plt_dev);
fail0:
	return ret;
}
module_init(vcp_init);

static void __exit vcp_exit(void)
{
	os_driver_unregister(&vcp_plt_drv);
	os_device_unregister(&vcp_plt_dev);
}
module_exit(vcp_exit);


MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Ziran Xu <ziran.xu@autochips.com>");


#endif

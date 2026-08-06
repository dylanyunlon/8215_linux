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
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <asm/page.h>
#include "x_ver.h"
#include <linux/of_reserved_mem.h>
#include "drv_imgresz.h"
#include "drv_imgresz_errcode.h"
#include "imgresz_hal_if.h"
#include "imgresz_drv/imgresz_log.h"
#include <io.h>
#include <linux/list.h>
#include <linux/delay.h>

#define ATC_KERNEL_LINUX_LICENSE     "GPL"

#define MMISC_MODE_NAME         "IMGR"
#define MMISC_VER_MAJOR         01
#define MMISC_VER_MINOR         00
#define MMISC_VER_REV           00

struct reserved_mem *TempLine_Reserved = NULL; // added by mtk68119
/*#define IMGZDEC_IOCT 10*/
/*#define IMGRESZ_YUV2YC 11*/
IMGRESZ_DRV_TICKET_T global_ticket;
int scale_done = 0;

static long imgresz_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void *private_data;
	IMGRESZ_MW_PARAM pParam;
	bool bRet = true;
	long ret;
	IMGRESZ_DRV_SCALE_MODE scale_mode;
	bool lock_mode;
	IMGRESZ_DRV_DO_SCALE_T do_scale_t;
	int sleep_count = 0;
	IMGRESZ_DRV_RM_INFO_T rm_info;
	IMGRESZ_DRV_SRC_BUF_INFO_T src_buf;
	IMGRESZ_DRV_DST_BUF_INFO_T dst_buf;
	IMGRESZ_DRV_PARTIAL_INFO_T partical_buf;
	IMGR_LOG(IMGR_LOG_LVL_DBG, "enter ioctl,cmd is %d\n",cmd);

	private_data = filp->private_data;

	switch (cmd) {
		case SET_PARAM:
			if (copy_from_user((void *)&pParam, (void *)arg, sizeof(IMGRESZ_MW_PARAM))) {
				IMGR_LOG(IMGR_LOG_LVL_ERR, "fail to copy_from_user\n");
				return -1;
			}
			bRet=ImgreszSetParam(filp,&pParam);
			break;
		case SCALE_FIRE:
			bRet=ImgreszScaleFire(filp);
			break;
		case SCALE_STOP:
			bRet=ImgreszStopScale(filp);
			break;
			
		case IMGRESZ_GET_TICKET :
		ret=i4ImgResz_Drv_GetTicket(&global_ticket);
		if(ret == -1) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get imgresz ticket,return\n");
			return -1;
			}
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get ticket %d\n",global_ticket.u4Ticket);
	    break;
		
		case IMGRESZ_RELEASE_TICKET:
			i4ImgResz_Drv_ReleaseTicket(&global_ticket);
			IMGR_LOG(IMGR_LOG_LVL_ERR, "release ticket %d\n",global_ticket.u4Ticket);
			break;
		
		case IMGRESZ_SET_PRORITY:
			i4ImgResz_Drv_SetPriority(&global_ticket,IMGRESZ_DRV_PRIORITY_HIGH);
			break;

		case IMGRESZ_SET_SCALEMODE:
			if (copy_from_user((void *)&scale_mode, (void *)arg, sizeof(IMGRESZ_DRV_SCALE_MODE))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get scale_mode,return\n");
			return -1;
			}
			IMGR_LOG(IMGR_LOG_LVL_INFO, "scale_mode is %d\n",scale_mode);
			i4ImgResz_Drv_SetScaleMode(&global_ticket,scale_mode);
			break;
			
		case IMGRESZ_SET_LOCK:
			if (copy_from_user((void *)&lock_mode, (void *)arg, sizeof(bool))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get lock_mode,return\n");
			return -1;
			}
			IMGR_LOG(IMGR_LOG_LVL_INFO, "lock_mode is %d\n",lock_mode);
			i4ImgResz_Drv_SetLock(&global_ticket,lock_mode);
			break;

		case IMGRESZ_DO_SCALE:
			if (copy_from_user((void *)&do_scale_t, (void *)arg, sizeof(IMGRESZ_DRV_DO_SCALE_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get do_scale_t,return\n");
			return -1;
			}
			i4ImgResz_Drv_DoScale(&global_ticket,&do_scale_t);
			msleep(5);

			while(scale_done == 0) {

			sleep_count++;
			msleep(4);

			if(sleep_count > 250) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "do scale timeout,return\n");
			return -1;
			}
		}
			scale_done =0;
			IMGR_LOG(IMGR_LOG_LVL_ERR, "do scale done,return\n");
			break;

		case IMGRESZ_STOP_SCALE:
			i4ImgResz_Drv_StopScale(&global_ticket);
			break;

		case IMGRESZ_SET_RMINFO:
			if (copy_from_user((void *)&rm_info, (void *)arg, sizeof(IMGRESZ_DRV_RM_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get rm_info,return\n");
			return -1;
			}

			i4ImgResz_Drv_SetRmInfo(&global_ticket,&rm_info);
			break;

		case IMGRESZ_SET_SRCBUF:
			if (copy_from_user((void *)&src_buf, (void *)arg, sizeof(IMGRESZ_DRV_SRC_BUF_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get src_buf,return\n");
			return -1;
			}
			
			IMGR_LOG(IMGR_LOG_LVL_ERR, "src buf info %d %d,%d,%d,%d,%lx,%lx,%lx,%lx,%d\n",src_buf.eSrcColorMode,src_buf.u4BufWidth,src_buf.u4BufHeight,src_buf.u4PicWidth,src_buf.u4PicHeight,
			src_buf.u4YBufAddr,src_buf.u4CbBufAddr,src_buf.u4CrBufAddr,src_buf.u4ColorPalletSa,src_buf.fgWTEnable);
		
			i4ImgResz_Drv_SetSrcBufInfo(&global_ticket,&src_buf);
			break;

		case IMGRESZ_SET_DSTBUF:
			if (copy_from_user((void *)&dst_buf, (void *)arg, sizeof(IMGRESZ_DRV_DST_BUF_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get dst_buf,return\n");
			return -1;
			}
			
			IMGR_LOG(IMGR_LOG_LVL_ERR, "dst buf info %d,%d,%d,%d,%d,%lx,%lx,%d\n",dst_buf.eDstColorMode,dst_buf.u4BufWidth,dst_buf.u4BufHeight,dst_buf.u4PicWidth,dst_buf.u4PicHeight,
			dst_buf.u4YBufAddr,dst_buf.u4CBufAddr,dst_buf.fgWTEnable);

			i4ImgResz_Drv_SetDstBufInfo(&global_ticket,&dst_buf);
			break;

		case IMGRESZ_SET_PARTICALBUF:
			if (copy_from_user((void *)&partical_buf, (void *)arg, sizeof(IMGRESZ_DRV_PARTIAL_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get dst_buf,return\n");
			return -1;
			}
			
			i4ImgResz_Drv_SetPartialBufInfo(&global_ticket,&partical_buf);
			break;
	default:
		IMGR_LOG(IMGR_LOG_LVL_ERR, "unknown command\n");
		bRet=false;
		break;
	}
	if (!bRet) {
		return -1;
	}

	return 0;
}

struct file_operations const imgresz_fops = {
	.unlocked_ioctl = imgresz_ioctl,
};


static struct miscdevice imgresz_dev = {
	MISC_DYNAMIC_MINOR,
	"imgresz",
	&imgresz_fops
};

void __iomem *imgr0_sysreg_base = NULL;
void __iomem *imgr1_sysreg_base = NULL;
unsigned int imgr0irq = 0;
unsigned int imgr1irq = 0;
struct clk *clk_ac8317_imgr0 = NULL;
struct clk *clk_ac8317_imgr1 = NULL;
struct clk *clk_ac8317_imgr = NULL;


static int imgresz_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *nd = pdev->dev.of_node;

	imgr0_sysreg_base = of_iomap(nd, 0);

	if (!imgr0_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 reg base address failed = %p \r\n",
			 imgr0_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 reg base address = %p \r\n", imgr0_sysreg_base);

	imgr1_sysreg_base = of_iomap(nd, 1);

	if (!imgr1_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 reg base address failed  = %p \r\n",
			 imgr1_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 reg base address = %p \r\n", imgr1_sysreg_base);

	imgr0irq = irq_of_parse_and_map(nd, 0);

	if (imgr0irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 irq failed = %d \r\n", imgr0irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 irq = %d \r\n", imgr0irq);

	imgr1irq = irq_of_parse_and_map(nd, 1);

	if (imgr1irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 irq failed = %d \r\n", imgr1irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 irq = %d \r\n", imgr1irq);

	ImgrGetHwRegAddress();

	clk_ac8317_imgr0 = devm_clk_get(&pdev->dev, "imgr0-device");

	if (!clk_ac8317_imgr0) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 clk failed %p\r\n", clk_ac8317_imgr0);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 clk success %p\r\n", clk_ac8317_imgr0);

	clk_ac8317_imgr1 = devm_clk_get(&pdev->dev, "imgr1-device");

	if (!clk_ac8317_imgr1) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 clk failed %p\r\n", clk_ac8317_imgr1);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 clk success %p\r\n", clk_ac8317_imgr1);


	clk_ac8317_imgr = devm_clk_get(&pdev->dev, "imgr-clkselect");

	if (!clk_ac8317_imgr) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz select clk failed %p\r\n", clk_ac8317_imgr);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz select clk success %p\r\n", clk_ac8317_imgr);
	//added by mtk68119
	of_reserved_mem_device_init(&(pdev->dev));
	TempLine_Reserved = (struct reserved_mem *)(pdev->dev.cma_area);

	if (!TempLine_Reserved) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"TempLine reserved memory get error! dev name: %s\r\n", pdev->name);
		return -1;
	}

	else 
	{
		IMGR_LOG(IMGR_LOG_LVL_INFO,"success to get templinebuf reserved memory,base is %x,size is %x \r\n", 
			TempLine_Reserved->base, TempLine_Reserved->size);
	}

	//added by mtk68119 end
	ret = misc_register(&imgresz_dev);
	

	if (ret) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "imgr_probe: misc_register error %d\r\n", ret);
	}

	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

	i4ImgResz_Drv_Init();

	return ret;
}

static int imgresz_remove(struct platform_device *pdev)
{
	of_reserved_mem_device_release(&(pdev->dev)); //added by mtk68119
	i4ImgResz_Drv_Uninit();

	misc_deregister(&(imgresz_dev));

	return 0;
}

static const struct of_device_id imgr_of_ids[] = {
	{.compatible = "Autochips,ac83xx-imgresz",},
	{}
};

static struct platform_driver imgr_plt_drv = {
	.driver = {
		.name = "ac83xx-imgresz",
		.owner = THIS_MODULE,
		.of_match_table = imgr_of_ids,
	},
	.probe = imgresz_probe,
	.remove = imgresz_remove,
};


static int __init imgresz_init(void)
{
	int ret;

	IMGR_LOG(IMGR_LOG_LVL_INFO,"imgresz_init--->\n");
	ret = platform_driver_register(&imgr_plt_drv);

	if (ret) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"[imgresz]: %s: register  driver failed\n", __func__);
	}

	return ret;

}

static void __exit imgresz_exit(void)
{
	IMGR_LOG(IMGR_LOG_LVL_ERR,"imgresz_exit--->\n");
	platform_driver_unregister(&imgr_plt_drv);
}
module_init(imgresz_init);
module_exit(imgresz_exit);


MODULE_AUTHOR("mtk68024");
MODULE_DESCRIPTION("Imgresz driver");
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE); /*TODO:*/






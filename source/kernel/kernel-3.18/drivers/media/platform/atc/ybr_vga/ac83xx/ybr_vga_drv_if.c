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


#include <generated/atc_project.h>
#ifdef __ARM2__
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "83xx_irqs_vector.h"
#else
#include "irqs_vector.h"
#endif

#else
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#endif
#include "ybr_vga_drv_if.h"
#include "winutil.h"
#include "ybr_vga_errcode.h"
#include "vga_hal_api.h"
#include "ybr_vga_util.h"
#include "drv_hdtv.h"
#include "drv_vga.h"
#include "drv_vdoclk.h"
#include "vga_auto.h"
#include "ybr_vga_oal.h"
#include "ybr_vga_common.h"



#define LOG_TAG "drv_if"

/*
extern u32 g_u4SrcType;
extern u8  g_u1Timing;
*/
YBR_VGA_CFG ybr_vga_cfg_para = {0};
YBR_VGA_VDO_INFO g_rVdoInfo = {0};

/*static struct mutex _Lock;*/
/*#define Lock() mutex_lock(&_Lock)*/
/*#define Unlock() mutex_unlock(&_Lock)*/

struct atc_ybr_isr_data isr_data = {NULL, NULL};
bool register_isr = false;
struct task_struct *ybr_thd = NULL;
/*static spinlock_t ybr_irqLock;*/


int atc_ybr_register_isr(atc_ybr_isr_t isr, void *arg)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");
	if (NULL == isr) {
		YBRVGA_ERROR(LOG_TAG, "ybr_vga isr is null!\n");
		return -EINVAL;
	}

	if ((isr_data.isr != NULL) && (isr_data.isr != isr)
	&& (isr_data.arg != arg)) {
		YBRVGA_ERROR(LOG_TAG, "ybr_vga isr is busy!\n");
		return -EBUSY;
	}

	isr_data.isr = isr;
	isr_data.arg = arg;
	register_isr = true;

	return 0;
}
EXPORT_SYMBOL(atc_ybr_register_isr);

int atc_ybr_unregister_isr(atc_ybr_isr_t isr, void *arg)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");
	if (isr == NULL) {
		YBRVGA_ERROR(LOG_TAG, "ybr_vga isr is null!\n");
		return -EINVAL;
	}

	if ((isr_data.isr == isr) && (isr_data.arg == arg)) {
		isr_data.isr = isr;
		isr_data.arg = arg;
		register_isr = false;
		return 0;
	}

	YBRVGA_ERROR(LOG_TAG, "ybr_vga isr is busy!\n");

	return -EBUSY;
}
EXPORT_SYMBOL(atc_ybr_unregister_isr);

u32 YBR_Init(LPCTSTR pszContext)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return true;
}


bool YBR_Deinit(u32 dwContext)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return true;
}

u32 YBR_Open(u32 dwContext, u32 dwAccessMode, u32 dwShareMode)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return 0;
}
EXPORT_SYMBOL(YBR_Open);


bool YBR_Close(u32 dwContext)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");

	return true;

}
EXPORT_SYMBOL(YBR_Close);

bool YBR_IOControl(u32 context, u32 code, const u8 *pInBuffer, u32 inSize,
	u8 *pOutBuffer, u32 outSize, u32 *pOutSize)
{
	int ret = 0;
	YBRVGA_DEBUG(LOG_TAG, "enter with cmd(%d)\n", code);
	/*Lock();*/
	switch (code) {
	case IOCTL_YBR_VGA_INIT:
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_INIT\n");
		vDrvVideoInit();
		break;

	case IOCTL_YBR_VGA_CONFIG:
		if ((NULL == pInBuffer) || (inSize < sizeof(YBR_VGA_CFG))) {
			YBRVGA_ERROR(LOG_TAG, "please config the right input buf size(%d)\n", inSize);
			return false;
		}
		memcpy((void *)&ybr_vga_cfg_para, (void *) pInBuffer, sizeof(YBR_VGA_CFG));
		g_u4SrcType = ybr_vga_cfg_para.source_type;
		if ((g_u4SrcType == SRC_NULL) || (g_u4SrcType > SRC_VGA)) {
			YBRVGA_ERROR(LOG_TAG, "please config the right source type(%d)\n",
				(unsigned int)g_u4SrcType);
			return false;
		}
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_CONFIG with source type:%s\n",
			(g_u4SrcType == SRC_YBR) ? "YPbPr" : "VGA");
		initYPbPrVGA();
		vDrvVideoConnect(true);
		break;

	case IOCTL_YBR_VGA_START:
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_START\n");
		g_bStop = false;
		ybr_thd = kthread_create(u4DrvVideoMainLoop, NULL, "VGATask_thread" );
		if(IS_ERR(ybr_thd) ){
			YBRVGA_ERROR(LOG_TAG, "create u4DrvVideoMainLoop failed!\n");
			return false;
		}
		wake_up_process(ybr_thd);
		//spin_lock_irqsave(&ybr_irqLock, flags);
		ret = request_irq(ybr_irq, vDrvVideoIrqHandler, 0, "IRQ_YBR", NULL);
		if (ret) {
			YBRVGA_ERROR(LOG_TAG, "register ybr irq(%d) fail(%d)\n", ybr_irq);
			return false;
		}
		//spin_unlock_irqrestore(&ybr_irqLock, flags);
		YBRVGA_INFO(LOG_TAG, "register ybr irq(%d) sucess\n", ybr_irq);
		break;

	case IOCTL_YBR_VGA_STOP:
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_STOP");
		vDrvVideoConnect(SV_OFF);
		vDrvAllHDADCPow(SV_OFF);
		vDrvSOY1EN(0);
		vDrvYbrVgaClkDisable();
		g_bStop = TRUE;
		free_irq(ybr_irq, NULL);
		break;

	case IOCTL_YBR_VGA_GET_VIDEO_INFO:
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_GET_VIDEO_INFO");
		g_rVdoInfo.u2Width = (unsigned int) wDrvVideoInputWidth();
		g_rVdoInfo.u2Height = (unsigned int) wDrvVideoInputHeight();
		g_rVdoInfo.u1Interlace = (unsigned char)((bDrvVideoIsSrcInterlace() == 0) ? 0 : 1);

		YBRVGA_DEBUG(LOG_TAG, "u2Width(%d) u2Height(%d) u1Interlace(%d)\n",
			g_rVdoInfo.u2Width, g_rVdoInfo.u2Height, g_rVdoInfo.u1Interlace);
		memcpy((void *) pOutBuffer, (void *)&g_rVdoInfo, sizeof(YBR_VGA_VDO_INFO));
		break;

	case IOCTL_YBR_VGA_AUTO:
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_AUTO");
		vDrvVideoAuto();
		break;

	case IOCTL_YBR_VGA_GET_SINGAL_STATUS: {
		u32 ybrvga_status = 0;
		YBRVGA_INFO(LOG_TAG, "IOCTL_YBR_VGA_GET_SINGAL_STATUS");
		if ((NULL == pOutBuffer) || (outSize < sizeof(u32))) {
			YBRVGA_ERROR(LOG_TAG, "please config the right out buf size(%d)\n", inSize);
			return false;
		}
		ybrvga_status = bDrvVideoSignalStatus();
		memcpy(pOutBuffer, &ybrvga_status, sizeof(u32));
	}
	break;

	default:
		YBRVGA_ERROR(LOG_TAG, "unknown cmd(%d)\n", code);
		break;
	}
	/*Unlock();*/

	return true;
}
EXPORT_SYMBOL(YBR_IOControl);

void ybr_vga_autocolor(void)
{
	vDrvVideoAutoColor();
}
EXPORT_SYMBOL(ybr_vga_autocolor);

void ybr_vga_auto(void)
{
	vDrvVideoAuto();
}
EXPORT_SYMBOL(ybr_vga_auto);


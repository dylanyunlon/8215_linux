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
#include <linux/types.h>
#include <linux/delay.h>
#include "winutil.h"
#include "vga_hal_api.h"
#include "x_typedef.h"
#include "ybr_vga_oal.h"
#include "ybr_vga_drv_if.h"
#include "ybr_vga_hw_reg.h"
#include "ybr_vga_common.h"
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "ac83xx_irqs_vector.h"
#include "x_bim.h"
#include "wch_if.h"
#else
#ifdef __ARM2__
#include "irqs_vector.h"
#else
#include <linux/slab.h>
#include <linux/interrupt.h>
#endif
#endif


#define LOG_TAG "oal"

#ifdef CONFIG_ATC_PLATFORM_ac823x
extern void mt33xx_mask_ack_bim_irq(uint32_t irq);
#endif


void vUtDelay1ms(u32 n)
{
	mdelay((unsigned long)n);
}
void vUtDelay2us(u32 n)
{
	ndelay((unsigned long)n);
}

irqreturn_t vDrvVideoIrqHandler(int u2Vector, void *dev_id)
{
	g_u4IrqStatus = u4DrvVideoGetIrqStatus();

	if (g_u4IrqStatus != 0) {
		vDrvVideoClearIrqStatus(g_u4IrqStatus);
		if (g_u4IrqStatus & (u32)INT_MODE_CHANGE) {
			/*UTIL_Printf( "Received INT_MODE_CHANGE Interrupt.\r\n");*/
			if (!g_fgWchStopped) {
				/*if (g_u4SrcType == SRC_YBR) {
					if (0 != WchStopByInputSrc(SRC_APP_YPBPR)) {
						pr_err("Stop YPbPr Wch Failed\r\n");
					} else {
						g_fgWchStopped = TRUE;
						pr_info("Stop YPbPr Wch Success\r\n");
					}
				} else  if (g_u4SrcType == SRC_VGA) {
					if (0 != WchStopByInputSrc(SRC_APP_VGA)) {
						pr_err("Stop VGA Wch Failed\r\n");
					} else {
						g_fgWchStopped = TRUE;
						pr_info("Stop VGA Wch Success\r\n");
					}
				}*/
			}
		}

		if (g_u4IrqStatus & (u32)INT_MUTE) {
			/*YBRVGA_DEBUG(LOG_TAG, "Received INT_MUTE Interrupt.\n");*/
		}

		if (g_u4IrqStatus & (u32)INT_VSYNC) {
			/*YBRVGA_DEBUG(LOG_TAG, "Received INT_VSYNC Interrupt.\n");*/
		}


		if (g_u4IrqStatus & (u32)INT_DDS_LOCK) {
			/*YBRVGA_DEBUG(LOG_TAG, "Received INT_DDS_LOCK Interrupt.\n");*/
		}

		switch (g_u4SrcType) {
		case SRC_YBR:
			vHdtvISR();
			break;

		case SRC_VGA:
			vVgaISR();
			break;
		default :
			break;
		}
	}

#ifdef CONFIG_ATC_PLATFORM_ac83xx
	ac83xx_mask_ack_bim_irq((u32)u2Vector);
#else
	mt33xx_mask_ack_bim_irq((uint32_t)u2Vector);
#endif

	return IRQ_HANDLED;
}


/**
    video Main loop
*/
int u4DrvVideoMainLoop(void *arg)
{
	YBRVGA_INFO(LOG_TAG, "u4DrvVideoMainLoop enter with signal_type(%d)\n",
		(unsigned int) g_u4SrcType);

	while ((g_u4SrcType != SRC_NULL) && (!g_bStop)) {
		/* VGA State Machine*/
		if (!_IsVgaDetectDone) {
			vVgaModeDetect();
		} else {
			vVgaChkModeChange();
		}

		/* HDTV State Machine*/
		if (!_IsHdtvDetectDone) {
			vHdtvModeDetect();
		} else {
			vHdtvChkModeChange();
		}

		/*Irq Status Process*/
		/*
		    g_u4IrqStatus = u4DrvVideoGetIrqStatus();
		    if (g_u4IrqStatus != 0)
		    {
			Vga_IRQ(VECTOR_YPBPRINT);
			switch (g_u4SrcType) {
			case SRC_YBR:
			vHdtvISR();
			break;

			case SRC_VGA:
			vVgaISR();
			break;
			}
		    }*/
		/*Signal Status Process*/
		g_u4SigStatus = bDrvVideoSignalStatus();

		if (g_u4SigStatus != g_u4SigPreStatus) {
			/*if (NULL != g_hSigStateEvt) {*/
				switch (g_u4SigStatus) {
				case SV_VDO_NOSIGNAL:
					YBRVGA_INFO(LOG_TAG, "SV_VDO_NOSIGNAL\n");
					g_fgWchStopped = (bool)FALSE;
					/*SetEventData(g_hSigStateEvt, EVT_YBR_VGA_SIG_OFF);*/
					/*SetEvent(g_hSigStateEvt);*/
					YBRVGA_DEBUG(LOG_TAG, "Signal is not found\n");
					if (register_isr) {
						isr_data.isr(&g_u4SigStatus);
						YBRVGA_INFO(LOG_TAG, "Ybr signal Lost!\n");
					}
					break;

				case SV_VDO_NOSUPPORT:
					YBRVGA_INFO(LOG_TAG, "SV_VDO_NOSUPPORT\n");
					/*SetEventData(g_hSigStateEvt,
					EVT_YBR_VGA_SIG_NO_SUPPORT);*/
					/*SetEvent(g_hSigStateEvt);*/
					YBRVGA_DEBUG(LOG_TAG, "Signal is not support\n");
					break;

				case SV_VDO_STABLE:
					YBRVGA_INFO(LOG_TAG, "SV_VDO_STABLE\n");
					/*SetEventData(g_hSigStateEvt, EVT_YBR_VGA_SIG_ON);*/
					/*SetEvent(g_hSigStateEvt);*/
					if (register_isr) {
						isr_data.isr(&g_u4SigStatus);
						YBRVGA_INFO(LOG_TAG, "Ybr signal Get!\n");
					}
					g_u1Timing = bDrvVideoGetTiming();
					YBRVGA_INFO(LOG_TAG, "Signal is Stable, Timing is %d\n", g_u1Timing);
					/*enable blank Level Adjust*/
					vDrvEnableBlankLevelAdjust();
					/*doing auto*/
					/*vDrvVideoAuto();*/
					break;

				default:
					YBRVGA_DEBUG(LOG_TAG, "Signal status(%d) is unknown!\n", g_u4SigStatus);
					break;
				}
			/*}*/
			
			YBRVGA_INFO(LOG_TAG, "Signal Status(Pre,Cur):(%d, %d)\n",
			g_u4SigPreStatus, g_u4SigStatus);
			g_u4SigPreStatus = g_u4SigStatus;
		}

		/* VGA auto state machine*/
		vVdoSP0AutoState();
		vDrvAdjustBlankLevel();
		vDrvOnChipAutoColorIteration(); /* do auto color*/
		vDrvPGALinearityVerify();
		msleep((int)10);
	}
	complete_and_exit(NULL, 0);
	return 0;
}






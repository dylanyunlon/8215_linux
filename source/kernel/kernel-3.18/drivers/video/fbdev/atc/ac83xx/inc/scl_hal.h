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
       
#ifndef _SCL_HAL_H_
#define _SCL_HAL_H_

#ifndef __ARM2__
#include <linux/interrupt.h>
#else
#include "irqreturn.h"
#endif
#include "x_lint.h"
#include "drv_av_d.h"
 
#ifndef PANEL_800_600
#define PANEL_800_600
#endif

#define INPUT_MODE          4
#define SCL_IN_480I      0
#define SCL_IN_576I      1
#define SCL_IN_480P      2
#define SCL_IN_576P      3
#define SCL_IN_480P_800  4
#define SCL_IN_600P_800  5
#define SCL_IN_600P_1024 6
#define SCL_IN_720P_1280 7
#define SCL_IN_800P_1280 8
#define SCL_IN_768P_1024 9

#define PANEL_NO            10
#define SCL_OUT_480_234 0
#ifdef PANEL_640_480
#define SCL_OUT_640_480 1
#else
#define SCL_OUT_400_234 1
#endif
#define SCL_OUT_320_234 2
#define SCL_OUT_640_234 3
#ifndef PANEL_800_600
#define SCL_OUT_480_272 4
#else
#define SCL_OUT_800_600 4
#endif
#define SCL_OUT_800_480 5
#define SCL_OUT_1024_600 6
#define SCL_OUT_1280_720 7
#define SCL_OUT_1280_800 8
#define SCL_OUT_1024_768 9


#define PHASE_NO            8
#define H_Y_TAP_NO          10
#define H_C_TAP_NO          8
#define V_Y_TAP_NO          3
#define V_C_TAP_NO          3

#define H_FILTER_NO       8
#define H_720_480         0
#define H_720_400         1
#define H_720_320         2
#define H_720_640         3
#define H_720_960         4
#define H_720_800         5
#define H_720_1024        6
#define H_720_1280        7
#define V_FILTER_NO       6
#define V_480_234         0
#define V_480_480         1
#define V_576_234         2
#define V_576_480         3
#define V_480_600         4
#define V_576_600         5

#define SCL_VFIR_FLITER       (0)
#define SCL_VFIRLNR_FLITER    (1)
#define SCL_VLINEAR_FLITER    (2)
#define SCL_VMAX_FLITER       (3)
#define SCL_HFIR_FLITER       (0)
#define SCL_HLINEAR_FLITER    (1)
#define SCL_HNONLNR_FLITER    (2)
#define SCL_HMAX_FLITER       (3)

#define SCL_HSCALE_STEP       (0x100)
#define SCL_HSCALE_SHIFT      (8)
#define SCL_VSCALE_STEP       (0x100000)
#define SCL_VSCALE_SHIFT      (20)

/* Freq = 13.5*(MM+1)/(DD+1)*/
#define DCLK_MM_800_480 0x13
#define DCLK_DD_800_480 0x08
#define DCLK_MM_800_600 0x08
#define DCLK_DD_800_600 0x02


/******************************************************************************
* Panel Scaler Hal API
******************************************************************************/
extern bool _fgWaitVBIEvent;
extern void vSclHalInit(bool fgHwReset);
extern void vSclHalIsrInit(void);
extern void vSclHalIsrStop(__u8 ucPmxId);
extern void vSclHalReset(void);
extern void vSclHalRstTiming(void);
extern void vSclHalSetWinActive(__u32 u4Top, __u32 u4Bottom, __u32 u4Left, __u32 u4Right);
extern void vSclHalSetVScale(__u32 u4VScale);
extern void vSclHalSetHScale(__u32 u4HScale);
extern void vSclHalSetLongBuf(bool fgLBEnable);
extern void vSclHalSetVFliter(__u32 u4VFliter);
extern void vSclHalSetHFliter(__u32 u4HFliter);
extern void vSclHalSetBG(__u32 u4BGColor);
extern void vSclHalSetBI(bool fgEnable, __u32 u4BIColor);
extern void vSclHalSetHVTotal(bool fgAdjustOn, __u32 u4HTotal, __u32 u4VTotal);
extern void vSclHalSetMode(void);
extern void vSclHalSetMasterMode(bool fgEnable);
extern void vSclHalSetMasterSrc(__u32 u4VdpMode);
extern void vSclHalSetHCoef(__u32 u4HRatio);
extern void vSclHalSetHCoefSharpness(__u8 u4HRatio);
extern void vSclHalSetHCoefPhaseType(__u8 u4HRatio);
extern void vSclHalSetVCoef(__u32 u4VRatio);
extern void vSclHalSetVCoefSharpness(__u8 u4VRatio, bool fgNtsc);
extern void vSclHalSetVCoefPhaseType(__u8 u4VRatio, bool fgNtsc);
extern void vSclDumpReg(void);
extern irqreturn_t vSclHalIsr(int u2Vector, void *dev_id);
extern __u32 vSclInVertBlank(void);
extern void vSclHalResume(void);

#endif



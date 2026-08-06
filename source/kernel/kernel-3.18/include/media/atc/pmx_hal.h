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

#ifndef _PMX_HAL_H_
#define _PMX_HAL_H_

#ifndef __ARM2__
#include <linux/interrupt.h>
#else
#include "irqreturn.h"
#endif
#include "x_lint.h"
#include "x_typedef.h"
#include "drv_config.h"
#include "chip_ver.h"

#include <generated/atc_project.h>

#define MAINSURFACE_OSD2
#ifdef MAINSURFACE_OSD2
#define PRIMARY_SURF_PLANE	PMX_HW_PLANE_4
#else
#define PRIMARY_SURF_PLANE	PMX_HW_PLANE_3
#endif

#define PMX_PLANE_VIDEO    0
#define PMX_PLANE_OSD2     1
#define PMX_PLANE_OSD3     2
#define PMX_PLANE_OSD4     3
#define PMX_PLANE_OSD1     4
#define PMX_PLANE_MAX      5
#define PMX_PLANE_MASK     0x7
#define PMX_PLANE_DST_MASK 0xF

#ifdef CONFIG_ATC_OS_linux
#define PMX_PLANE_ORDER  0x32410
#else
#define PMX_PLANE_ORDER  0x32104
#endif

#define PMX_PLANE_ORDER_LINUX  0x32014 /* move up video layer*/

/* Plane mixer TV type*/
#define PMX_TV_TYPE_NTSC		0
#define PMX_TV_TYPE_PAL_M		1
#define PMX_TV_TYPE_PAL_N		2
#define PMX_TV_TYPE_PAL			3
#define PMX_TV_TYPE_PAL_1080P_24			4
#define PMX_TV_TYPE_PAL_1080P_25			5
#define PMX_TV_TYPE_PAL_1080P_30		       6
#define PMX_TV_TYPE_NTSC_1080P_23_9		7 /*1080P23.976hz*/
#define PMX_TV_TYPE_NTSC_1080P_29_9	       8/*1080P29.97hz*/
#define PMX_TV_TYPE_480P_800               9
#define PMX_TV_TYPE_480P_800_50            10
#define PMX_TV_TYPE_600P_800               11
#define PMX_TV_TYPE_600P_800_50            12
#define PMX_TV_TYPE_600P_1024              13
#define PMX_TV_TYPE_600P_1024_50           14
#define PMX_TV_TYPE_480P_640               15 /*add for ypbpr*/

extern void ac83xx_mask_ack_bim_irq(__u32 irq);

extern __u32 dwPmxHalSetPinctrl (void);
extern void vPmxHalInit(void);
extern void vPmxHalIsrInit(void);
extern void vPmxHalIsrStop(__u8 ucPmxId);
extern void vPmxHalIsrEnable(__u8 ucPmxId);
extern void vPmxHalIsrDisable(__u8 ucPmxId);
extern void vPmxHalEnableCb(__u8 ucPmxId, __u8 ucCbType);
extern void vPmxHalDisableCb(__u8 ucPmxId);
extern void vPmxHalEnableMute(__u8 ucPmxId);
extern void vPmxHalDisableMute(__u8 ucPmxId);
extern void vPmxHalSetDelay(__u8 ucVdoId, __u8 ucADJ_F, __u16 u2HDelay, __u16 u2VDelay);
extern void vPmxHalRstInVSync(__u8 ucPmxId);
extern void vPmxHalReset(__u8 ucPmxId);
extern void vPmxHalSetMasterMode(bool fgEnable);
extern void vPmxHalSetMode(__u8 ucPmxId, __u8 ucFmt);
extern void vPmxHalSetTvType(__u8 ucPmxId, __u8 ucTvType);
extern void vPmxHalSetDigitalOut(__u8 ucPmxId, __u8 ucEnable, __u8 ucProprietaryMode, __u8 ucDigitalBit);
extern void vPmxHalSetDigitalOutPhase(__u8 ucPmxId, __u32 u4Phase);
extern void vPmxHalSetDigitalOutDrv(__u32 u4Driving);
extern void vPmxHalSetDigitalOutDrv(__u32 u4Driving);
extern void vPmxHalSetDigitalOutRgb(__u8 ucPmxId, __u8 ucRbg);
extern void vPmxHalSetBg(__u8 ucPmxId, __u32 u4Bg);
extern void vPmxHalSetBi(__u8 ucPmxId, __u32 u4Bi);
extern void vPmxHalSetVHTotal(__u8 ucPmxId, bool fgAdjustOn, __u32 u4HTotal, __u32 u4VTotal);
extern void vPmxHalSetBrightness(__u8 ucPmxId, __u8 ucBrightness);
extern void vPmxHalSetContrast(__u8 ucPmxId, __u8 ucContrast);
extern void vPmxHalSetGamma(__u8 ucPmxId, __u8 ucEnable, const __u8 *pucCurve);
extern void vPmxHalSetAlpha(__u8 ucPmxId, __u32 ucInAlpha, __u32 ucOutAlpha, bool fgEnable);
extern void vPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern void vPmxHalNotMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern void vPmxHalNotMixPlaneDelay(__u8 ucPmxId, __u32 u4Plane);
extern bool fgPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern void vPmxHalSetAbleToFlipOSD(__u8 bIsAbleToFlip);
extern void vPmxHalSetFullRange(__u8 ucPmxId, bool fgEnable, bool fg2352255);
extern void vPmxHalSet709To601(__u8 ucPmxId, bool fgEnable, bool fg7092601);
extern void vPmxHalEnableFmt(__u8 ucPmxId);
extern void vPmxHalDisableFmt(__u8 ucPmxId);
extern bool vPmxHalGetFmtEn(__u8 ucPmxId);
extern void vPmxHalSetPlaneOrder(__u8 ucPmxId, __u32 u4PlaneOrder);
extern __u32 vPmxHalGetPlaneOrder(__u8 ucPmxId);
extern void vPmxHalSetPlaneDstColorKey(__u8  ucPmxId, bool fgEnable);
extern void vPmxHalEnableWaitVSync(__u8  ucPmxId);
extern void vPmxHalMixIsr(void);
extern __u32 dwPmxHalWaitVSync(__u8 ucPmxId);
extern void vPmxHalResume(__u8 ucPmxId);
extern irqreturn_t vPmxHalMainIsr(int u2Vector, void *dev_id);
extern void vPmxHalLayerBgEn(__u32 u4Bg);
extern void vPmxHalLayerBgDis(void);
extern void vPmxHalDispFmtHFilter(__u8 ucVdoId, __u8 ucYC, __u8 ucCoef);
#endif  /* _PMX_HAL_H_*/




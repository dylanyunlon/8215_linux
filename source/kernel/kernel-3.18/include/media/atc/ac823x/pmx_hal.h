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
#include "drv_if_pmx.h"
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

#define PMX_HW_VIDEO_LAYER 0  
#define PMX_HW_OSD1_LAYER 4  
#define PMX_HW_OSD2_LAYER 1  
#define PMX_HW_OSD3_LAYER 2  
#define PMX_HW_OSD4_LAYER 3  

#define PMX_HW_PLANE_1    0
#define PMX_HW_PLANE_2    1
#define PMX_HW_PLANE_3    2
#define PMX_HW_PLANE_4    3
#define PMX_HW_PLANE_5    4
#define PMX_HW_PLANE_6    5
#define PMX_HW_PLANE_7    6
#define PMX_HW_PLANE_8    7

#define PMX_HW_VIDEO_MIX  PMX_HW_PLANE_1
#define PMX_HW_OSD1_MIX   PMX_HW_PLANE_3
#define PMX_HW_OSD2_MIX   PMX_HW_PLANE_4
#define PMX_HW_OSD3_MIX   PMX_HW_PLANE_5
#define PMX_HW_OSD4_MIX   PMX_HW_PLANE_7

typedef enum {
    PMX_PAHSE_TYPE_8 = 0,
    PMX_PAHSE_TYPE_16,
    PMX_PAHSE_TYPE_32
}PMX_PHASE_TYPE_E;

typedef enum {
    PMX_SD_MODE_TYPE_OLD_8 = 0,
    PMX_SD_MODE_TYPE_OLD_16,
    PMX_SD_MODE_TYPE_NEW_8,
    PMX_SD_MODE_TYPE_NEW_16
}PMX_SD_MODE_TYPE_E;

extern void PMX_HalSetupSoftwareRegister(void);
extern void vPmxHalInit(void);
extern void vPmxHalIsrInit(void);
extern void vPmxHalIsrStop(__u8 ucPmxId);
extern void vPmxHalRstInVSync(__u8 ucPmxId);
extern void vPmxHalReset(__u8 ucPmxId);
extern void vPmxHalSetMasterMode(bool fgEnable);
extern void PMX_HalSetMode(UCHAR ucPmxId, UCHAR ucFmt);
extern void PMX_HalSetTvType(UCHAR ucPmxId, UCHAR ucTvType);
extern void vPmxHalSetMode(__u8 ucPmxId, __u8 ucFmt);
extern void vPmxHalSetTvType(__u8 ucPmxId, __u8 ucTvType);
extern void vPmxHalSetAlpha(__u8 ucPmxId, __u32 ucInAlpha, __u32 ucOutAlpha, bool fgEnable);
extern void vPmxMixPlane(unsigned char ucPmxId, unsigned int u4Plane);
extern void vPmxNotMixPlane(unsigned char ucPmxId, unsigned int u4Plane);
extern void vPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern void vPmxHalNotMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern bool fgPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane);
extern void vPmxHalSetFullRange(__u8 ucPmxId, bool fgEnable, bool fg2352255);
extern void vPmxHalSet709To601(__u8 ucPmxId, bool fgEnable, bool fg7092601);
extern void vPmxHalSetFmtPhase(UCHAR ucPmxId, PMX_PHASE_TYPE_E eYPhase, PMX_PHASE_TYPE_E eCPhase);
extern void vPmxHalSetSDMode(UCHAR ucPmxId, PMX_SD_MODE_TYPE_E eSDModeType);
extern void vPmxHalEnableFmt(__u8 ucPmxId);
extern void vPmxHalDisableFmt(__u8 ucPmxId);
extern bool vPmxHalGetFmtEn(__u8 ucPmxId);
extern void vPmxHalSetPlaneOrder(__u8 ucPmxId, __u32 u4PlaneOrder);
extern __u32 vPmxHalGetPlaneOrder(__u8 ucPmxId);
extern void vPmxHalSetPlaneDstColorKey(__u8  ucPmxId, bool fgEnable);
extern void vPmxHalEnableWaitVSync(__u8  ucPmxId);
extern void vPmxHalMixIsr(void);
extern void vPmxHalResume(__u8 ucPmxId);
extern irqreturn_t vPmxHalMainIsr(int u2Vector, void *dev_id);

#endif  /* _PMX_HAL_H_*/




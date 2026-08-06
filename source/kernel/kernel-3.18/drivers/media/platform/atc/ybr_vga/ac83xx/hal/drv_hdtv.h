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

#ifndef DRV_HDTV_H_
#define DRV_HDTV_H_

#include "ybr_vga_util.h"
#include "drv_async.h"
#include "drv_vga.h"
#include "vga_auto.h"
#include "drv_vdoclk.h"
#include "drv_autocolor.h"
#include "drv_adcset.h"
#include "drv_auto.h"
#include <linux/types.h>

/***Macro Define***/
#define vHdtvCenSel(fgswitch) vIO32WriteFldAlign(HDTV_00,fgswitch,HDTV_CEN_SEL)
#define vHdtvRGBFormatOn(fgswitch) vIO32WriteFldAlign(HDTV_00,fgswitch,HDTV_RGB)
#define bHdtvCheckCenEnable()	IO32ReadFldAlign(HDTV_00,HDTV_CEN_SEL)

/***Struct Define***/
typedef struct RHDTVNSTDStatus
{
    u8 fgIsNSTD;
    u8 bRefreshRate;
    u16 wVTotal;
    u16 wHTotal;
    s16 wVTotalDiff;
} RHDTVNSTDStatus;

typedef struct
{
    u8 bTimingIdx ;
    u8 bClampStart;
    u8 bClampEnd;
    u8 bBlankStart;
    u8 bVmaskStart;
    u8 bVmaskEnd;
    u8  bPhase;
} HDTVTimingPrmSet;

/***Function Declaration***/    
void vHdtvConnect(bool fgIsOn);
u16 wHdtvInputWidth(void);
u8 bHdtvInputWidthOverSample(u8 bMode);
u16 wHdtvInputHeight(void);
u8 bHdtvRefreshRate(void);
u8 bHdtvInterlace(void);
u8 bHdtvSigStatus(void);
void vHdtvISR(void);
void vHdtvHwInit(void);
void vHdtvModeDetect(void);
void vHdtvChkModeChange(void);
void vHdtvSetInput(u16 wstart, u16 wwidth);
void vHdtvClampWin(u16 wclamp_start, u16 wclamp_end);
void vHdtvBlankStart(u16 wblank_start);
void vHdtvSetInputCapature(u8 bmode, u8 bIsHdtv);
u16 wHdtvDEInputWidth(void);
void vHdtvADC_Clear(u16 adid, u8 bEn);
void vHdtvADC_Mid(u16 adid, u8 bEn);
void vHDTVGetNSTDStatus(RHDTVNSTDStatus* pHDTVNSTDStatus);
void vHdtvRGB2YCbCrSet(void);
void vHdtvRGB2YCbCrAdjustSet(u16 u2Gain[3], u8 u1PreAddr[3], u8 u1PostAddr[3]);
void vHdtvSwReset(void);
void vHdtvStatus(void);
void vHdtvSetOversampleForSD(u8 bmode);
u8 vHdtvGetOversampleForSD(void);
u16 wHdtvGetPorch(u8 bPath,u8 bPorchType);
void vHdtvSetPorch(u8 bPath,u8 bPorchType, u16 wValue);
void vHDTVChkNSTD(u16 VTotal_Measure);

/***Extern Variable Declaration***/
extern u8 bHdtvOpt01_MDmute0; /*for MD , waiting lock mute*/
extern u8 bHdtvOpt01_MDmute1; /*for MD , after lock stable*/
extern u8 bHdtvOpt05_AdaptiveSlicer; /*Enable adaptive slicer*/
extern u8 _bHdtvTiming;
extern u8 _IsHdtvDetectDone;
extern u8 _bUnLockCnt;
extern bool g_fgWchStopped;
extern void vHdtvSetModeCHG(void);
extern void vHdtvSetModeDone(void);
extern void vHdtvPhaseIsr(void);

#endif


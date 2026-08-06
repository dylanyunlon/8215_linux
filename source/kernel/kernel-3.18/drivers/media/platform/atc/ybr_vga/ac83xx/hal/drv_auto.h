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

#ifndef DRV_AUTO_H__
#define DRV_AUTO_H__

#include "ybr_vga_util.h"
#include "drv_vga.h"
#include "drv_vdoclk.h"
#include "vga_auto.h"
#include <linux/types.h>

/***Marcro Define***/
#define TOP_THRE 0x50 /* orig. 0x20 ,gbsh 060901*/ 
#define PSNE_THRE1  0x70 /*orig. 0x20,gbsh 060901*/
#define PSNE_THRE2  0x20 /*orig. 5*/
#define SUPPORT_MIX_PHASE_STA   1

#define wDrvVGAGetHStart()  IO32ReadFldAlign(HDTV_01, HDTV_AV_START)
#define vDrvVGASetHStart(wHStart) vIO32WriteFldAlign(HDTV_01, (wHStart), HDTV_AV_START)
#define vDrvVGASetPhsMix(bMethod)   vIO32WriteFldAlign(ASYNC_17, (bMethod), AS_C_PSNE_SRC_SEL)    
#define bDrvHwAutoClkRdy()  IO32ReadFldAlign(STA_SYNC0_0E, AS_AUTO_CLK_RDY)
#define dDrvAutoGetPhsMaxMinDiff()  IO32ReadFldAlign(STA_SYNC0_0E, AS_PHS_MAXMIN_DIFF_S)
#define bDrvHwAutoPhsRdy()  IO32ReadFldAlign(STA_SYNC0_0C, AS_AUTO_PHASE_RDY)
#define bDrvAutoGetPhsGood()    IO32ReadFldAlign(STA_SYNC0_0C, AS_PHASE_GOOD)

/***Enum Define***/
enum
{
    BD_RED,
    BD_GREEN,       
    BD_BLUE,
    BD_MAX
};

enum
{
    RED_BD_SEL = 0x1,
    GREEN_BD_SEL = 0x2,     
    BLUE_BD_SEL = 0x4,
    ALL_BD_SEL = 0x7
};

enum
{
    Amode,
    Bmode
};

/***Function Declaration***/
extern u16   wSP0Vtotal;
u32 dVGAGetAllDiffValue_peak(void);

u8 bDrvVGAGetPhase(void); 
void vDrvVGASetPhase(u8 bVal)  ;
void vDrvVGASetBDDataTh(u8 bTh) ;
void vDrvVGASetBDCha(u8 bCha);
u16 wDrvVGAGetLeftBound(void);
u16 wDrvVGAGetTopBound(void);
u16 wDrvVGAGetRightBound(void);
u16 wDrvVGAGetBottomBound(void);
u32 dVGAGetAllDiffValue(void);
u32 dHDTVGetAllDiffValue(void); /*YPbPr Auto Phase 2006/11/07*/
void vVGAPhaseModeSet(u8 bMode);
void vDrvVGASetTopThr(u8 bVal);
void vDrvVGASetPsneThr(u8 bThr1, u8 bThr2);
void vVgaHwAutoPhaseEnable(u8 bEnable);
void vVgaHwAutoPhaseReset(void);
void vVgaHwAutoClkEnable(u8 bEnable);
void vDrvVGASetPhase_Simple(u8 bVal);
u8 u1DrvVGAGetPhase(void);
u16 wDrvVGAGetClock(void); /*127 offset*/
u16 wDrvVGAHPositionMax(void);
u16 wDrvVGAHPositionMin(void);
u16 wDrvVGAVPositionMax(void);
u16 wDrvVGAVPositionMin(void);
u16 wDrvVGAGetHPosition(void);
u16 wDrvVGAGetVPosition(void);
u8 wDrvVgaGetClockMax(void);
 
#endif


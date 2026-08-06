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

#ifndef DRV_ADCSET_H__
#define DRV_ADCSET_H__

#include "ybr_vga_util.h"
#include "drv_autocolor.h"
#include <linux/types.h>

/***Function Declaration***/
void vDrvSetHDTVMux(void);
void vDrvADCDefaultSetting(void) ;
void vDrvADCOffsetCal(void) ;
void vDrvHDTVADCDefaultSetting(void) ;
u32 u4DrvGetEfuseGain(void);
void vDrvVGAPD(void);
void vDrvVGAPWON(void);
void vDrvSetHDTVADC(void);
void vDrvAllHDADCPow(bool bPow) ;
void  vSliceQuality536x(void);
void vSetDefaultSlicer(void) ;
void vNextSlicer(u16 wTimeout) ;
void vResetSliceTimer(void); 
void vSetMONSlicer(u8 pair);
void vResetVLenSP0(void);
void vResetVLenSP2(void);
void vSetMONSlicer_Matrix(void);
u8 bReadMONSlicer(void);
u8 bReadNewSlicer(void);
void vDrvSOY1EN(u8 bEn);

/***Extern Variables Declaration***/
extern u8 _bEFUSE_AUTOCOLOR_READY;
extern u8 check_quaity_state;
extern u8 _bCurSlicerIdx;
extern u8 _bCurSlicerIdx_best;

#endif


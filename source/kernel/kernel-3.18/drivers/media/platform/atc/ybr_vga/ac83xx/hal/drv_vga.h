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

#ifndef DRV_VGA_H_
#define DRV_VGA_H_

#include "ybr_vga_util.h"
#include "drv_hdtv.h"
#include "vga_table.h"
#include "drv_autocolor.h"
#include <linux/types.h>

/***Macro Define***/
#define YPBPR_480IP_27MHZ 1
#define SUPPORT_HDTV_HARDWARE_MUTE 1
#define ASYNC_FULL_SCREEN_WA 1 
#define NO_SIGNAL 0
#define HSYNC_UPPER_BD           8 /*5	//11*/
#define HSYNC_LOWER_BD          8 /*6	//11*/
#define VSYNC_UPPER_BD            3
#define VSYNC_LOWER_BD           3
#define NEGATIVE   0
#define POSITIVE    1
#define vVgaSetInput(wstart, wwidth)	vHdtvSetInput(wstart, wwidth)
#define vVgaClampWin( wclamp_start, wclamp_end) 	vHdtvClampWin( wclamp_start, wclamp_end) 

/***Struct and Enum Define***/ 
enum{
	VGAMD_GET_ACTIVESYNC,
	VGAMD_GET_MODE_N_VTOTAL,
	VGAMD_GET_HTOTAL_N_STDTIME,
	VGAMD_SET_INPUT_CAPTURE,
	VGAMD_DETECTDONE,
	VGAMD_AMBCHK,
	VGAMD_DELAY0,
	VGAMD_DELAY1,
	VGAMD_DELAY2,
	VGAMD_DELAY3,  	
};/*VGA Mode Detect State Machine*/

/***Function Declaration***/
void vVgaConnect(bool fgIsOn) ;
u16 wVgaInputWidth(void) ;
u16 wVgaInputHeight(void) ;
u8 bVgaRefreshRate(void) ;
u8 bVgaInterlace(void) ;
u8 bVgaSigStatus(void) ;
void vVgaISR(void) ;
void vVgaModeDetect(void) ;
void vVgaChkModeChange(void) ;
void vVgaSetInputCapature(u8 bmode);
void vVgaSetPorch(u8 bPath,u8 bPorchType, u16 wValue);
void vVgaRGB2YCbCrSet(void);
u16 wVgaGetPorch(u8 bPath,u8 bPorchType);
void vVgaSetPorch(u8 bPath,u8 bPorchType, u16 wValue);
void vVgaSwReset(void);
void vVgaStatus(void);
u8 bVgaCurPhase(void);
void vVgaAmbiguousHInit(void);
#if SUPPORT_VGA_USERMODE
void vVgaUsrStable(void);
void vVgaUsrBroken(void);
void vVgaEraseUserMode(u8 index);
#endif

/***Extern Variable Declaration***/
extern u8 bVgaOpt01_MDmute0; /*for MD , waiting lock mute*/
extern u8 bVgaOpt01_MDmute1; /*for MD , after lock stable*/
extern u8 bVgaOpt02_MCmute0; /*for MC , waiting stable mute*/
extern u8 bVgaOpt02_MCmute1; /*for MC , waiting stable mute*/
extern u8 bVgaOpt02_MCmute2; /*for MC , waiting stable mute*/
extern u8 bVgaOpt02_MCmute3; /*for MC , waiting stable mute*/
extern u8 bVgaOpt03_SoGen; /*for MC , waiting stable mute*/
extern u8 bVgaOpt04_SearchHDTV; /*for MD, std timing search*/
extern u8 bVgaOpt05_SearchNewMode; /*for MD, new mode & user timing search*/
extern u8 bVgaOpt06_SogMaxVsyncWidth ;
extern u8 bVgaOpt07_AutoKeepOldVal;
extern u8 bVgaOpt08_AutoSP0SwitchMux;
extern u8 _IsVgaDetectDone;
extern u8 _IsHdtvDetectDone;
extern u8 _bVgaTiming;
extern u16 wSP0Hclk;
extern u8 _bVgaUserMode;
extern bool _RETIMENeedReset;
extern u8 _bPLLlockCnt;
extern void vVgaSetModeCHG(void);
extern void vVgaSetModeDone(void);
extern u8 _bLockCnt;
extern u8 bSP0HVCheck(void);

#if SUPPORT_VGA_USERMODE
extern u8 bVgaUsrTimingSearch(u8 eepinit);
#endif

#endif


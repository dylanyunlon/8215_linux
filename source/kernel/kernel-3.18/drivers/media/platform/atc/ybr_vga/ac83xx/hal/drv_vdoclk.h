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

#ifndef DRV_VDOCLK_H__
#define DRV_VDOCLK_H__

#include <generated/atc_project.h>
#include "ybr_vga_util.h"
#include "drv_vga.h"
#include "vga_auto.h"
#include "drv_async.h"
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>


/***Marcro Define***/
#define u8Div6432(a, b, c)  ((a) / (b))
#define CHANGE_SDDS_KPI 1
#define VGA_HW_AUTO   0 // please always fix to 0
#define Phase3Channel 1
#if Phase3Channel
    #define CHANNEL_NUM  3
#else
    #define CHANNEL_NUM  1
#endif
#define MAX_PHASE_ARRAY  33

#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define vDrvCLKINSyncSel(bSrc)  vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, bSrc, FLD_RG_VGAPLL_SDDS_HSEL)
#define vDrvCLKINHsyncPol(bPol) vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, bPol, FLD_RG_VGAPLL_SDDS_HPOR)
#else
#define vDrvCLKINSyncSel(bSrc)  vIO32WriteFldAlign(REG_PLL_GROUP_CFG31, bSrc, FLD_RG_VGAPLL_SDDS_HSEL)
#define vDrvCLKINHsyncPol(bPol) vIO32WriteFldAlign(REG_PLL_GROUP_CFG31, bPol, FLD_RG_VGAPLL_SDDS_HPOR)
#endif

//VFE_14
#define vDrvCLKINFreeRun(bEna) vIO32WriteFldAlign(VFE_17, bEna, DCLK_INIT)
#define vDrvCLKINDclkRST(bEna) vIO32WriteFldAlign(VFE_17, bEna, DCLK_RESETB)
#define vDrvCLKINMaxPeriod(bVal) vIO32WriteFldAlign(VFE_14, bVal, MAX_PERIOD)
#define vDrvCLKINFMPeriod(bVal) vIO32WriteFldAlign(VFE_14, bVal, FM_PERIOD)
#define vDrvCLKINFreeHsyncPol(bPol) vIO32WriteFldAlign(VFE_17, bPol, HSYNC_POL_VFE)
#define vDrvCLKINSetLockThr(bVal) vIO32WriteFldAlign(VFE_14, bVal, LOCK_THR)
#define vDrvCLKINSetKI0(bVal) vIO32WriteFldAlign(VFE_14, bVal, DCLK1_KI_0)
#define vDrvCLKINSetKI1(bVal) vIO32WriteFldAlign(VFE_14, bVal, DCLK1_KI_1)
#define vDrvCLKINSetLockCnt(bVal) vIO32WriteFldAlign(VFE_14, bVal, LOCK_CNT)
#define vDrvCLKINSetKP0(bVal) vIO32WriteFldAlign(VFE_14, bVal, DCLK1_KP_0)
#define vDrvCLKINSetKP1(bVal) vIO32WriteFldAlign(VFE_14, bVal, DCLK1_KP_1)
#define vDrvCLKINSetErrLim(bVal) vIO32WriteFldAlign(VFE_14, bVal, ERR_LIM)

//VFE_15
#define vDrvCLKINDCLKFast(bEna) vIO32WriteFldAlign(VFE_15, bEna, DCLK_FAST_LF)
#define vDrvCLKINSetCW(bVal) vIO32WriteFldAlign(VFE_15, bVal, DCLK_FREQ_CW)
#define vDrvCLKINGetCW() IO32ReadFldAlign(VFE_15,  DCLK_FREQ_CW)
#define vDrvCLKINGetCwStatus()   (HAL_READ32(VFE_STA_01) & 0x0fffffff)
#define wDrvCLKINGetHtotal()    (IO32ReadFldAlign(VFE_16, DCLK_HLINE_LENGTH)+1U)

//VFE_16
#define vDrvCLKINDCLKPFDSEL(bEna) vIO32WriteFldAlign(VFE_14, bEna, DCLK_PFD_SEL)

//VFE_17
#define vDrvCLKINSetSecKI0(bVal) vIO32WriteFldAlign(VFE_17, bVal, DCLK2_KI_0)
#define vDrvCLKINSetSecKI1(bVal) vIO32WriteFldAlign(VFE_17, bVal, DCLK2_KI_1)
#define vDrvCLKINSetSecKP0(bVal) vIO32WriteFldAlign(VFE_17, bVal, DCLK2_KP_0)
#define vDrvCLKINSetSecKP1(bVal) vIO32WriteFldAlign(VFE_17, bVal, DCLK2_KP_1)

/***Enum Define***/
enum
{
	IntClk,
	ExtClk
};

enum
{
	DCLK_IN_SOG,
	DCLK_IN_HSYNC
};

enum {
	Bypass_LPF,
	YPbPr_480i_LPF,
	YPbPr_480p_LPF,
	YPbPr_720p_LPF,
	YPbPr_1080p_LPF,
	VGA_40MHZ_Less_LPF,
	VGA_40_60MHZ_LPF,
	VGA_60_90MHZ_LPF,
	VGA_90_130MHZ_LPF,
	VGA_120MHZ_More_LPF,
	Max_Input_timing
};

/***Function Declaration***/
void vDrvYbrVgaClkEnable(void);
void vDrvYbrVgaClkDisable(void);
void vDrvClkInit(void);
void vDrvVGASetClock(u16 wHtotal);
void vPGA_LPF_BW(u8 bLPF);
void vSet_VFE_LPF(u16 wInputPixClk);
void vDrvCLKINSetHtotal(u16 wHtotal);
void vDrvCLKINSetHsyncWid(u8 wWidth);
void vDrvADCPLLSet(u16 wPixClk, u16 wHtotal);

#if CHANGE_SDDS_KPI
void vDrvEnableChang_SDDS_BW(void);
void Set_SDDS_KPI(u8 bValue);
void vDrvClkSetLockBandwidth(u8 bIsLock);
#endif
void vDrvAllADCPLLPow(bool bPow);
void vYPbPrPhaseIsr_New(void);
void vVgaPhaseIsr_New(void);
void vDrvInitPhaseVar(void);

/***Extern Variable Declaration***/
extern u32   _dSumTmp[3];
extern u32 _u4GetLockCw ;
extern u32 _u4UiSetCw ;
extern u8   _bCLKSetFlag;
extern u32 _dwPhase3CH[CHANNEL_NUM];
extern struct clk *g_clk_ybr;

#endif


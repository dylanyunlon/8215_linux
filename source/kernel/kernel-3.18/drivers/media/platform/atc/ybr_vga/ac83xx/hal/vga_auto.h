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

#ifndef VGA_AUTO_H_
#define VGA_AUTO_H_

/******************************************************************************
 * Header Files
 *****************************************************************************/
#include "ybr_vga_util.h"
#include "drv_vdoclk.h"
#include "drv_async.h"
#include "drv_auto.h"
#include "drv_vga.h"
#include <linux/types.h>

/***Macro Define***/
#define VGA_AUTO_DBG_MSG    1
#define VGA_AUTO_UI_DBG 1
#define VGA_AUTO_CLK_DBG    1
#define VGA_AUTO_PHASE_DBG 1
#define VGA_AUTO_POS_DBG    1   
#define VGA_AUTO_SPEEDUP 1
#define VGA_AUTO_CLK_SKIP_TABLE_HTOTAL  0


/*#define  VGA_HV_AUTO_CENTER 1*/

#define fgIsAutoFlgSet(arg) ((_bAutoFlag & (u8)(arg)) > 0)
#define vSetAutoFlg(arg) (_bAutoFlag |= (u8)(arg))
#define vClrAutoFlg(arg) (_bAutoFlag &= (u8)(~(arg)))

#define SP0_AUTO_POSITION (0x1)
#define SP0_AUTO_PHASE (0x1U<<1)
#define SP0_AUTO_CLOCK (0x1U<<2)
#define SP0_AUTO_POSSET (0x1U << 3)
#define SP0_AUTO_ALL (0x0FU)
#define SP1_AUTO_POSITION (0x1U<<4)
#define SP1_AUTO_PHASE (0x1U<<5)
#define SP1_AUTO_CLOCK (0x1U<<6)
#define SP1_AUTO_ALL (0x70U)


#define BDTHRSH 86 /*modified for 5372,gbsh,20060523*/
/*#define VGA_H_OFST 2 // 1 modify by gellmann for 1600x1200*/
#define VGA_H_OFST 1 /*for mt5360 by Tadd*/
#define VGA_V_OFST 2 /* add 1 for omux align , minus 1 for hdtv align*/
#define VGA_CLK_PH_THRE 0x4000

/*SP0*/
#define SP0_H 1
#define SP0_V  (1U<<1)
#define IsSp0SetAuto(flg)   ((_fgIsAuto0PosFlg&(flg))>0)
#define vSetSp0Auto(flg)    (_fgIsAuto0PosFlg |= (flg))
#define vClrSp0Auto(flg)    (_fgIsAuto0PosFlg &= (~(flg)))
#define vSetSP0AutoState(bState)    (_bVdoSP0AutoState = bState)
#define bGetSP0AutoState()  (_bVdoSP0AutoState)

/***Enum Define***/
enum {
    AUTO_FINISHED,
    AUTO_CONTINUED
};

enum {
    MIX_STA,
    PSNE_ONLY   
};

enum {
    PSNE1_ADD,
    PSNE2_ADD,
    PSNE4_ADD,
    PSNE8_ADD   
};

/***Function Declaration***/
void vVdoSP0AutoState(void);
void vDrvVGASetPhase(u8 bVal);
void vDrvSetHPosition(u16 wValue);
void vDrvSetVPosition(u16 wValue);
bool fgDrvAutoEepWriteWord(u16 wAddr, u16 wData);
void vVgaAutoInit(void) ;
void vVgaAutoPosInit(void);

#if VGA_AUTO_SPEEDUP
void vVgaAutoClkIsr(void);
#endif

void vDrvVgaAutoStart(void);
void vDrvVgaAutoStop(void);
void vDrvYPbPrAutoStart(void);

/***Extern Variable Declaration***/
extern u8 _bAutoFlag;
extern u8 _bVdoSP0AutoState;
extern u16 _wVgaClock;
extern u8 _bVgaDelayCnt;
extern u8 _fgIsAuto0PosFlg;
extern u8 _bCurPhase;
extern u8 _bBestPhase;
#if VGA_AUTO_SPEEDUP
extern u8 _bAutoISR;
#endif
#if SUPPORT_VGA_AMBIGUOUS_H_DETECT
extern u32 _dPhaseDiff;
#endif

#endif



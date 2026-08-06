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

#ifndef _DRV_TDC_H_
#define _DRV_TDC_H_
#include "tvd_hw_reg.h"

#define ACTIVE_WIN_NTSC_X_START		(u32)0x08B
#define ACTIVE_WIN_NTSC_X_LENGTH	(u32)0x2F4
#define ACTIVE_WIN_NTSC_Y_START		(u32)0x013
#define ACTIVE_WIN_NTSC_Y_LENGTH	(u32)0x103
#define ACTIVE_WIN_DEMO_NTSC_X_START	(u32)0x1F4 //For right demo

#define ACTIVE_WIN_PAL_X_START		(u32)0x093
#define ACTIVE_WIN_PAL_X_LENGTH		(u32)0x3AA
#define ACTIVE_WIN_PAL_Y_START		(u32)0x017
#define ACTIVE_WIN_PAL_Y_LENGTH		(u32)0x136
#define ACTIVE_WIN_DEMO_PAL_X_START		(u32)0x1FC //For right demo


#define ACTIVE_WIN_PAL_M_X_START	(u32)0x090
#define ACTIVE_WIN_PAL_M_X_LENGTH	(u32)0x2F0
#define ACTIVE_WIN_PAL_M_Y_START	(u32)0x011
#define ACTIVE_WIN_PAL_M_Y_LENGTH	(u32)0x100
#define ACTIVE_WIN_DEMO_PAL_M_X_START	(u32)0x1F4 //For right demo

#define ACTIVE_WIN_PAL_N_X_START	(u32)0x09E
#define ACTIVE_WIN_PAL_N_X_LENGTH	(u32)0x2E8
#define ACTIVE_WIN_PAL_N_Y_START	(u32)0x018
#define ACTIVE_WIN_PAL_N_Y_LENGTH	(u32)0x134
#define ACTIVE_WIN_DEMO_PAL_N_X_START	(u32)0x1F4 //For right demo

#define DRAM_CMP_NTSC_HIGHBW	0x1D
#define DRAM_CMP_NTSC_LOWBW		0x17
#define DRAM_CMP_PAL_HIGHBW		0x24
#define DRAM_CMP_PAL_LOWBW		0x19

#define ADAPTIVE_2D_COMB_SHARP  1
#define ADAPTIVE_APL_DETECT	0
//#define ADAPTIVE_MB_PULL2D		1
//#define ADAPTIVE_LOWAPL_SMALLCHROMAOFF	1
#define ADAPTIVE_POST_NOTCH_FILTER	1
//#define ADAPTIVE_CZP_LEVEL		1
//#define ADAPTIVE_WEAK_SIGNAL	1
#define ADAPTIVE_GOH            0
//#define ADAPTIVE_NTSC3DYGAIN	1
#define ADAPTIVE_CCS_ON_BW      1
//#define ADAPTIVE_SLOW_MOTION    0   // To detect leopard spot pattern.
#define ADAPTIVE_MOVING_SINE 1
#define GOH_FROM_1D 0


typedef enum
{
	E_TDC_HIGH_BW,
	E_TDC_LOW_BW
} E_TDC_DAMODE;


typedef struct{
unsigned int wReg;
unsigned int dwValue;
unsigned int dwMask;
}REGTBL_T ;


#define TDC_IO_READ32(addr)              (*((volatile unsigned int *)(addr)))
#define TDC_IO_WRITE32(addr, value)   	 (*((volatile unsigned int *)(addr)) = value)


#define TDC_READ32(addr)                 (*((volatile unsigned int *)(IO_COMB_BASE+addr)))
#define TDC_WRITE32(addr, value)   	     (*((volatile unsigned int *)(IO_COMB_BASE+addr)) = value)

#define TDC_WRITE32_MASK(addr, value, mask)   (*((volatile unsigned int *)(addr))=((TDC_READ32(addr) & (~mask)) | value))


#define TDC_SET_FIELD(reg,val,field) \
    do {    \
        volatile unsigned int tv = TDC_IO_READ32(reg); \
        tv &= ~(field); \
        tv |= ((val) << (uffs(field) - 1)); \
        TDC_IO_WRITE32(reg,tv); \
    } while(0)

#define TDC_GET_FIELD(reg,field,val) \
    do {    \
        volatile unsigned int tv = TDC_IO_READ32(reg); \
        val = ((tv & (field)) >> (uffs(field) - 1)); \
    } while(0)

#define TDC_FIELD(bit_num, start_bit)  (((0x1 << bit_num) - 1) << start_bit)
#define fgHwTvdSVID_TDC(tvd_base)	   ((TDC_IO_READ32(tvd_base+REG_VSRC_07) & (0x1<<7))>> 7)
#define FBM_POOL_MODE_10BIT (1)
#define SV_OFF (0)
#define SV_ON (1)
#define REG_END	0xFFFF
#define REGTBL_END 0xFFFF
#define TDC_BUF_SIZE 4*1024*1024

/**************************************************************************
 * Function Members
 *************************************************************************/

extern void vDrvLoadRegTbl(REGTBL_T *pRegDwrdTbl);
extern void vDrvTDCOnOff(unsigned char bOnOff);
extern void vDrvTDCSetDramMode(unsigned char b10BitMode);
extern void vTvd3dSetAAF(unsigned char bValue);
extern void vTvd3dSetYCDelay(unsigned char bYCDelay);
extern void vTvd3dSetYCDelaybyAP(unsigned char bYDelay, unsigned char bCDelay);
extern void vDrvTDCSet(unsigned char tvd_mode);
extern void vTdcColorPatch(void);
extern void vTdcUS14Patch(void);
extern void vTdcCrossColorProc(void);
extern void vTdc3dProc(unsigned char tvd_mode);
extern bool vDrvTDCSetDramBase(void);
extern void vDrvTDCInit(void);

#endif


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

/*****************************************************************************
*  Plane Mixer: Interface
*****************************************************************************/

#ifndef _TVE_HAL_C_
#define _TVE_HAL_C_

#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include "tve_hw.h"
#include "tve_hal.h"
#include "pmx_hw.h"
#include "log.h"
#include "drv_env.h"
#include "../osd/hal/osd_hw.h"

extern OFFSET_TABLE_T rLocationOffset[];
static INT32 _Xoffset = 0;
static bool fgTveClockEn = FALSE;
static volatile TVE_BAK0_HAL_UNION_T *_prTveBak0HwReg;
static volatile TVE_BAK1_HAL_UNION_T *_prTveBak1HwReg;
static volatile TVE_BAK2_HAL_UNION_T *_prTveBak2HwReg;

TVE_BAK0_HAL_UNION_T _rTveBak0SwReg;
TVE_BAK1_HAL_UNION_T _rTveBak1SwReg;

void vTveHalClockEn(void)
{
	/*Path clock enable*/
	*(volatile __u32*)0xFD0000D8 &= 0xFFFFFF6;
	/*TVE clock and reset enable*/
	*(volatile __u32*)0xFD0000B4 |= 0x00008000;
	*(volatile __u32*)0xFD0000D0 |= 0x00008000;
}

void vTveHalClockDis(void)
{
	/*TVE clock and reset disable*/
	*(volatile __u32*)0xFD0000B4 &= 0xFFFF7FFF;
	*(volatile __u32*)0xFD0000D0 &= 0xFFFF7FFF;
}

void vTveHalInit(void)
{
	__u32 u4RegIdx;

	_prTveBak0HwReg = (TVE_BAK0_HAL_UNION_T *)tve_reg;
	_prTveBak1HwReg = (TVE_BAK1_HAL_UNION_T *)tvebk1_reg;
	_prTveBak2HwReg = (TVE_BAK2_HAL_UNION_T *)tvebk2_reg;
	FB_PRINT(FB_LOG_LVL_DBG, "TVE", "[vTveHalInit] %x, %x, %x\r\n", (unsigned int)_prTveBak0HwReg
		, (unsigned int)_prTveBak1HwReg, (unsigned int)_prTveBak2HwReg);

	/* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < TVE_BAK0_HAL_REG_NUM; u4RegIdx++) {
		_rTveBak0SwReg.au4Reg[u4RegIdx] = _prTveBak0HwReg->au4Reg[u4RegIdx];
	}

	for (u4RegIdx = 0; u4RegIdx < TVE_BAK1_HAL_REG_NUM; u4RegIdx++) {
		_rTveBak1SwReg.au4Reg[u4RegIdx] = _prTveBak1HwReg->au4Reg[u4RegIdx];
	}

	vTveHalSetMode(RES_480P);
}

bool IsTVE_SRC_AP(void)
{
	bool fgOK = FALSE;

	fgOK = (_prTveBak0HwReg->rField.u4ApSel == 1);

	return fgOK;

}

bool fgTveHalGetEn(void)
{
	bool fgOn = FALSE;

	fgOn = (_prTveBak0HwReg->rField.u4EncOff == 0);

	return fgOn;
}

void vTveHalEnable(__u32 u4TveSource)
{
	FB_PRINT(FB_LOG_LVL_DBG, "TVE", "Tve Enable source type %d!\r\n", (int)u4TveSource);

	switch (u4TveSource) {
	case TVE_DVP:
		_prTveBak0HwReg->rField.u4ApSel = 0;
		vTveHalMixPlane(PMX_MIX_AP2DVD);
		break;

	case TVE_AP:
		_prTveBak0HwReg->rField.u4ApSel = 1;
		_prTveBak0HwReg->rField.u4TvdDirect = 0;
		vTveHalMixPlane(PMX_HW_PLANE_8);
		break;

	case TVE_TVD:
		_prTveBak0HwReg->rField.u4ApSel = 1;
		_prTveBak0HwReg->rField.u4TvdDirect = 1;
		break;

	default:
		if (dwTveHalGetTveSrc() == TVE_DVP) {
			FB_PRINT(FB_LOG_LVL_DBG, "TVE", "Tve Enable mix plane PMX_MIX_AP2DVD!\r\n");
			vTveHalMixPlane(PMX_MIX_AP2DVD);
		} else {
			vTveHalMixPlane(PMX_HW_PLANE_8);
		}

		break;
	}

	_prTveBak0HwReg->rField.u4EncOff = 0;
}

void vTveHalDisable(void)
{
	_prTveBak0HwReg->rField.u4EncOff = 1;
	_prTveBak0HwReg->rField.u4TvdDirect = 0;
}


void tve_suspend(void)
{
	int u4RegIdx;

	for (u4RegIdx = 0; u4RegIdx < TVE_BAK0_HAL_REG_NUM; u4RegIdx++) {
		_rTveBak0SwReg.au4Reg[u4RegIdx] = _prTveBak0HwReg->au4Reg[u4RegIdx];
	}
}


void tve_resume(void)
{

	int u4RegIdx;

	for (u4RegIdx = 0; u4RegIdx < TVE_BAK0_HAL_REG_NUM; u4RegIdx++) {
		_prTveBak0HwReg->au4Reg[u4RegIdx] =   _rTveBak0SwReg.au4Reg[u4RegIdx];
	}

}


void vTveHalReset(void)
{
	_prTveBak0HwReg->rField.u4EncRst = 3;
	_prTveBak0HwReg->rField.u4EncRst = 0;
}

void vTveHalSetMode(__u32 u4Fmt)
{
	if (!fgTveClockEn) {
		vTveHalClockEn();
		fgTveClockEn = TRUE;
	}

	_prTveBak0HwReg->rField.u4YDelay = 2;
	_prTveBak0HwReg->rField.u4SYDelay = 3;
	_prTveBak0HwReg->rField.u4OutMode0 = 1; /*1-CVBS, 2-YPbPr, 3-RGB*/
	_prTveBak0HwReg->rField.u4OutMode1 = 2;
	_prTveBak0HwReg->rField.u4SyncOnG = 1;
	_prTveBak0HwReg->rField.u4SlewOff = 1;
	_prTveBak0HwReg->rField.u4FULW = 1;
	_prTveBak0HwReg->rField.u4Blacker = 0; /* Disable blacker for OSD rear UI garbage issue*/

	_prTveBak0HwReg->rField.u4Osd888 = 1;
	/*Power On DAC*/
	_prTveBak0HwReg->rField.u4DARB = 3;
	_prTveBak0HwReg->rField.u4DAG = 3;
	_prTveBak0HwReg->rField.u4DAX = 3;
	_prTveBak0HwReg->rField.u4DACKPRGB = 1;
	_prTveBak0HwReg->rField.u4DetIdFix = 1;
	_prTveBak0HwReg->rField.u4DACKP = 1;
	_prTveBak0HwReg->rField.u4PDBCD = 0xF;
	_prTveBak0HwReg->rField.u4PDCD = 3;
	_prTveBak0HwReg->rField.u4PDBIAS = 8;
	_prTveBak0HwReg->rField.u4TopThrd = 0xFF;
	_prTveBak0HwReg->rField.u4BotThrd = 0xFF;
	_prTveBak0HwReg->rField.u4Y2hend = 0x3B;
	_prTveBak0HwReg->rField.u4C2hend = 0x0B;
	_prTveBak0HwReg->au4Reg[(0x78 >> 2)] = 0;
}

void vTveHalEnableCB(__u32 u4CBType)
{
	if (u4CBType) {
		_prTveBak0HwReg->rField.u4CBType = 1;
	} else {
		_prTveBak0HwReg->rField.u4CBType = 0;
	}

	_prTveBak0HwReg->rField.u4CBOn = 1;
}

void vTveHalDisableCB(void)
{
	_prTveBak0HwReg->rField.u4CBOn = 0;

	vTveHalClockDis();
	fgTveClockEn = FALSE;
}

void vTveHalMixPlane(__u32 u4Plane)
{
	PMX_HAL_MIX_UNION_T *prPmxMixRegPtr;
	__u8              *prPmxMixRegMode;
	OFFSET_TABLE_T rOffset;
	rOffset = rLocationOffset[RES_480P];

	GET_PMX_MIX_PTR(prPmxMixRegPtr, prPmxMixRegMode);
	/* Need tuning TV full screen to enable*/
	/*prPmxMixRegPtr->rField.fgOSD_R_SYNC_H_P = 0;  // base on osd rear 0xA3308 bit 0 H falling edge*/
	/*prPmxMixRegPtr->rField.fgOSD_R_SYNC_V_P = 1;  // base on osd rear 0xA3308 bit 1 V rising edge*/

	switch (u4Plane) {
	case PMX_HW_PLANE_8:
		_prTveBak0HwReg->rField.u4EncMixSpSel = 1;
		_prTveBak0HwReg->rField.u4EncMixSpSlef = 1;
		prPmxMixRegPtr->rField.fgOSD_R_SRC_SE = 0; /* AP rear timing*/
		_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset);
		_OSD_BASE_Update(OSD_BASE_REAR);
		break;

	case PMX_HW_PLANE_9:
		_prTveBak0HwReg->rField.u4EncMixOsdSel = 0;
		_prTveBak0HwReg->rField.u4EncMixOsdSlef = 1;
		prPmxMixRegPtr->rField.fgOSD_R_SRC_SE = 0; /* AP rear timing*/
		break;

	case PMX_MIX_AP2DVD:
		_prTveBak0HwReg->rField.u4TveMixUi = 1;
		prPmxMixRegPtr->rField.fgOSD_R_SRC_SE = 1; /* DVP timing*/
		_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset + 0x10);
		_OSD_BASE_Update(OSD_BASE_REAR);
		break;
	}

	prPmxMixRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vTveHalNotMixPlane(__u32 u4Plane)
{
	PMX_HAL_MIX_UNION_T *prPmxMixRegPtr;
	__u8              *prPmxMixRegMode;

	switch (u4Plane) {
	case PMX_HW_PLANE_8:
		_prTveBak0HwReg->rField.u4EncMixSpSel = 1;
		_prTveBak0HwReg->rField.u4EncMixSpSlef = 0;
		break;

	case PMX_HW_PLANE_9:
		_prTveBak0HwReg->rField.u4EncMixOsdSel = 1;
		_prTveBak0HwReg->rField.u4EncMixOsdSlef = 0;
		break;

	case PMX_MIX_AP2DVD:
		_prTveBak0HwReg->rField.u4TveMixUi = 0;
		GET_PMX_MIX_PTR(prPmxMixRegPtr, prPmxMixRegMode);
		prPmxMixRegPtr->rField.fgOSD_R_SRC_SE = 0;
		prPmxMixRegPtr->rField.fgOSD_R_SYNC_FLD_P = 0;
		prPmxMixRegPtr->rField.fgOSD_R_SYNC_H_P = 0;
		prPmxMixRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		break;
	}
}

__u32 dwTveHalGetTveSrc(void)
{
	if (0 == _prTveBak0HwReg->rField.u4ApSel) {
		return TVE_DVP;
	}

	if (1 == _prTveBak0HwReg->rField.u4TvdDirect) {
		return TVE_TVD;
	} else {
		return TVE_AP;
	}
}

const __u32 _pdwMvVal[5][6] = {
	{0x25111D36, 0x1B007111, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  /* ntsc*/
	{0x25111D3E, 0x1B007111, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  /* ntsc*/
	{0x2115173E, 0x1B025515, 0x07F8241B, 0x0F0F0000, 0x0450B160, 0x000103FF},  /* ntsc*/
	{0x2A221A36, 0x1C002522, 0x03FE143D, 0x7EFE0154, 0x07408060, 0x00010155},  /* pal*/
	{0x04000024, 0x1E000000, 0x00002C1E, 0x000f0000, 0x00000000, 0x00010000}   /* p-scan*/
};

void vTveHalSetMv(__u32 dwType)
{
	__u8 i;
	__u32 dwMv6;
	__u32 dwMode = _u4RearOutputMode;

	dwMv6 = _prTveBak2HwReg->au4Reg[5] & 0x70000000;

	if (dwType == 0) {
		_prTveBak2HwReg->au4Reg[0] = 0;  /* turn off MacroVision*/
		_prTveBak2HwReg->au4Reg[6] = (_prTveBak2HwReg->au4Reg[6] & ~(_pdwMvVal[4][0])) | 0x00000E00;
	} else {
		for (i = 0; i < 5; i++) {
			_prTveBak2HwReg->au4Reg[i] = _pdwMvVal[dwType - 1][i];
		}

		dwMv6 |= _pdwMvVal[dwType - 1][5];
		_prTveBak2HwReg->au4Reg[5] = dwMv6;

		/*write MV7 register, AGC level and MV p-scan*/
		dwMv6 = (_prTveBak2HwReg->au4Reg[6] & ~MV_AGC_BP_MASK);

		if (dwMode == RES_480P) {
			dwMv6 |= (MV_AGCLVL_NTSC | MV_BPLVL_NTSC);
		} else {
			dwMv6 |= (MV_AGCLVL_PAL | MV_BPLVL_PAL);
		}

		_prTveBak2HwReg->au4Reg[6] = dwMv6 | _pdwMvVal[4][0];
	}
}

void vTveHalSetVbi(__u32 dRegVal)
{
	_prTveBak0HwReg->au4Reg[(0x40 / 4)] = dRegVal;
	_prTveBak0HwReg->au4Reg[(0x44 / 4)] = dRegVal;
}

__u32 dwTveGet525VbiData(void)
{
	return _prTveBak0HwReg->rField.u4WSDataP;
}

__u32 dwTveGet625VbiData(void)
{
	__u32 ret = _prTveBak0HwReg->rField.u4WSDataP & 0x3fff;

	return ret;
}

__u32 dwTveGetVbiCtrl(void)
{
	__u32 ret = _prTveBak0HwReg->au4Reg[(0x40 / 4)] & 0xfff00000;

	return ret;
}

__u8 bBitMirror(__u8 bIn)
{
	__u8 i;
	__u8 bOut;
	__u8 bMask;

	bOut = 0;

	for (i = 0; i < 4; i++) {
		bMask = (0x1 << i);
		bOut |= ((bIn & bMask) << (7 - (i * 2)));
	}

	for (i = 4; i < 8; i++) {
		bMask = (0x1 << i);
		bOut |= ((bIn & bMask) >> ((i * 2) - 7));
	}

	return bOut;
}

void vTveHalSetCc(__u8 bHi, __u8 bLo, __u32 dFld)
{
	_prTveBak0HwReg->rField.u4CCMode = dFld;
	_prTveBak0HwReg->rField.u4CCData = (bBitMirror(bHi) << 8) | bBitMirror(bLo);
}

bool fgTveCheckCCDummy(void)
{
	if ((_prTveBak0HwReg->rField.u4CCData == 0x00000101) || /*check dummy*/
	    (_prTveBak0HwReg->rField.u4CCMode == 0x1)) { /*only TOP field CC*/
		return TRUE;
	}

	return FALSE;
}

bool fgTveGetTvField(void)
{
	return _prTveBak0HwReg->rField.u4DirectSel;  /*0 is top and 1 is bottom*/
}
#endif /*_TVE_HAL_C_*/



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
#ifndef PMX_VERIFY_DRV_H_
#define PMX_VERIFY_DRV_H_

#include "drv_config.h"
#include "pmx_vfy_hal.h"

extern const CHAR* szResStr[];

extern REG_SET_T _ArVdoSetting[];
extern REG_SET_T _ArVdoActRegionSetting[];
extern REG_SET_T _ArDramSetting[];
extern REG_SET_T _ArHdmiSetting[];
extern REG_SET_T _ArCavSetting[];
extern REG_SET_T _ArCvbsSetting[];
extern REG_SET_T _ArCvbsSetting_T[];
extern REG_SET_T _ArCavSetting_T[];
extern REG_SET_T _ArOutputInitSetting[];
extern REG_SET_T _ArEMUfpdFPGADISP[];
extern REG_SET_T _ArSclerTimingSetting[];
extern REG_SET_T _ArSclFIRSetting[];
extern REG_SET_T _ArSclSetting[];
extern REG_SET_T _ArSclDRAMSetting[];
extern REG_SET_T _ArScl422Setting[];

#define PMX_VFY_FIR_LNR	        0
#define PMX_VFY_FIR_4TAP	1
#define PMX_VFY_FIR_8TAP	2
#define PMX_VFY_FIR_SS		3
#define PMX_VFY_FIR_DERING      4
#define PMX_VFY_FIR_H_SHARP     5

#define PMX_VFY_DSD_SUPPORT 0 //fixme: support DSD?
#define PMX_VFY_NEWSD_SUPPORT 1

extern void PmxVerifyDrvClkInit(unsigned char ucVdoId);
extern void PmxVerifyDrvInit(bool fgHwReset);
extern void PmxVerifyCAVSetup(unsigned int u4Mode);
extern void PmxVerifyCVBSSetup(unsigned int u4Mode, unsigned char ucTvType, unsigned char ucInterlace);
extern void PmxVerifyCVBSSetup2(unsigned char ucVdoId, unsigned int ucPmxMode);
extern void PmxVerifySclerSetup(unsigned char ucVdoId, unsigned int ucPmxMode);
extern void PmxVerifySetVDOPtr(unsigned char ucVdoId, unsigned char ucBufIdx, unsigned int u4AddrY, unsigned int u4AddrC);
extern void PmxVerifyVDOSetup(__u8 ucVdoId, __u32 u4Mode);
extern void PmxVerifyVDOSetup_WithSrcType(unsigned char ucVdoId, unsigned int u4Mode,unsigned char ucSrcType);
extern void PmxVerifyVDOActRegionSetup(unsigned char ucVdoId, unsigned int u4Mode, unsigned char ucTvType);
extern void PmxVerifyDispFmtHFilter(unsigned char ucVdoId, unsigned char ucYC, unsigned char ucCoef);
extern void PmxVerifySetMode(unsigned char ucVdoId, unsigned int u4SrcFmt, unsigned int u4PmxFmt, unsigned int u4OutFmt, unsigned int u4CavFmt, 
        unsigned char ucTvType, unsigned char ucFit, unsigned char ucInterlace, unsigned char ucVdoInterlace ,unsigned char ucSrcType,unsigned char uc3d);
extern void PmxVerifySetMode_OSD(unsigned char ucVdoId, unsigned int u4SrcFmt, unsigned int u4PmxFmt, unsigned int u4OutFmt, unsigned int u4CavFmt,
        unsigned char ucTvType, unsigned char ucFit, unsigned char ucInterlace, unsigned char ucVdoInterlace, unsigned char uc148or296, unsigned char uc3d, bool fginit); 
extern void PmxVerifySetMode_720480Vdo(unsigned char ucVdoId, unsigned int u4SrcFmt, unsigned int u4PmxFmt, unsigned int u4OutFmt,
        unsigned int u4CavFmt, unsigned char ucTvType, unsigned char ucFit, unsigned char ucInterlace, unsigned char ucVdoInterlace, unsigned char ucSrcType,
        unsigned char uc3d, unsigned int ucScanline, unsigned int u4AddrY, unsigned int u4AddrC);

#endif /*PMX_VERIFY_DRV_H_*/




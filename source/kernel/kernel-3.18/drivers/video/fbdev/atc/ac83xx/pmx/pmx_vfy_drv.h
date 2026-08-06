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

extern REG_SET_T _ArVdoSetting[];
extern REG_SET_T _ArVdoActRegionSetting[];
extern REG_SET_T _ArCvbsSetting[];
extern REG_SET_T _ArWrChannelSetting[];
extern REG_SET_T _ArOutputInitSetting[];


#define PMX_VFY_FIR_LNR	0
#define PMX_VFY_FIR_4TAP	1
#define PMX_VFY_FIR_8TAP	2
#define PMX_VFY_FIR_SS		3
#define PMX_VFY_FIR_DERING  4

extern void PmxVerifyDrvInit(bool fgHwReset);
extern void PmxVerifySetVDOPtr(__u8 ucVdoId, __u8 ucBufIdx);
extern void PmxVerifyCVBSSetup(__u8 ucVdoId, __u32 ucPmxMode);
extern void PmxVerifyVDOSetup(__u8 ucVdoId, __u32 u4Mode);
extern void PmxVerifyVDOActRegionSetup(__u8 ucVdoId, __u32 ucPmxMode);
extern void PmxVerifySetMode(__u8 ucVdoId, __u32 u4SrcFmt, __u32 u4PmxFmt
	, __u8 ucSrcType, __u8 ucTvType, __u8 ucInterlace);
extern void PmxVerifyVdoXSkip(__u8 ucVdoId, __u32 u4XSkip);
extern void PmxVerifyVdoXSkipRoutine(__u8 ucVdoId);
extern void PmxVerifySetVdoDeIntMode(__u8 ucVdoId, __u8 ucMode);
extern void PmxVerifySetVdoSrcFmt(__u8 ucVdoId, __u8 ucSrcFmt);
extern void PmxVerifySetLumaKey(__u8 ucVdoId, __u8 ucOn, __u32 u4LumaKeyVal);
extern void PmxVerifySetShiftLine(__u8 ucVdoId, __u8 ucHD, __u8 ucLine);
extern void PmxVerifyZoom(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH
	, __u32 u4DstW, __u32 u4DstH, __u32 u4ZoomW, __u32 u4ZoomH);
extern void PmxVerifyZoomRoutine(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH, __u32 u4DstW, __u32 u4DstH);
extern void PmxVerifyDispChkSum(__u8 ucVdoId);
extern void PmxVerifySetOSDMixRatio(__u8 ucOsdId, __u32 u4MixRatio);
extern void PmxVerifySetDataSource(__u8 ucOsdSel, __u8 ucFpdSel);
extern __u32 PmxVerifyProbeRegister(__u32 addr, __u32 one);
extern void PmxVerifyScanRegister(__u32 start_addr, __u32 one);

extern void PmxVerifyLoadData(__u8 ucPatternId, __u32 u4PatternSz);
#if CONFIG_DRV_FPGA_BOARD
extern void PmxVerifySaveData(__u32 u4PicSz);
#endif
extern void PmxVerifyIsrTest(__u8 ucVdoId, __u8 ucOn);
extern void PmxVerifyFrameLock(__u8 uc4TvType, __u8 ucLockOn, __u8 ucTestEn);
extern void PmxVerifyTtlSel(__u8 uc4TtlType);
#endif /*PMX_VERIFY_DRV_H_*/




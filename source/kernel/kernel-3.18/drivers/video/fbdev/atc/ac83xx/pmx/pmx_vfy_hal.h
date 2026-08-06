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
#ifndef PMX_VERIFY_HAL_H_
#define PMX_VERIFY_HAL_H_

typedef struct {
	__u32    *pu4RegSetting;
	SIZE_T  size;
} REG_SET_T;


/*picture mode*/
#define PMX_VFY_VDO_FLD	0
#define PMX_VFY_VDO_FRM	1
#define PMX_VFY_VDO_4MA	2
#define PMX_VFY_VDO_REPEAT  3

/*source format*/
#define PMX_VFY_VDO_SRC_420_MB	0
#define PMX_VFY_VDO_SRC_422_MB	1
#define PMX_VFY_VDO_SRC_420_SL	2
#define PMX_VFY_VDO_SRC_422_SL	3

/*definition of ZOOM type*/
#define PMX_VFY_H_ZOOM_1X		0x1
#define PMX_VFY_H_ZOOM_2X		0x2
#define PMX_VFY_H_ZOOM_3X		0x3
#define PMX_VFY_H_ZOOM_4X		0x4
#define PMX_VFY_H_ZOOM_A2X	0x5
#define PMX_VFY_H_ZOOM_A3X	0x6
#define PMX_VFY_H_ZOOM_A4X	0x7
#define PMX_VFY_H_ZOOM_1D2X	0x8
#define PMX_VFY_H_ZOOM_1D3X	0x9
#define PMX_VFY_H_ZOOM_1D4X	0xA
#define PMX_VFY_H_ZOOM_A1D2X	0xB
#define PMX_VFY_H_ZOOM_A1D3X	0xC
#define PMX_VFY_H_ZOOM_A1D4X	0xD

#define PMX_VFY_V_ZOOM_1X		0x1
#define PMX_VFY_V_ZOOM_2X		0x2
#define PMX_VFY_V_ZOOM_3X		0x3
#define PMX_VFY_V_ZOOM_4X		0x4
#define PMX_VFY_V_ZOOM_A2X	0x5
#define PMX_VFY_V_ZOOM_A3X	0x6
#define PMX_VFY_V_ZOOM_A4X	0x7
#define PMX_VFY_V_ZOOM_1D2X	0x8
#define PMX_VFY_V_ZOOM_1D3X	0x9
#define PMX_VFY_V_ZOOM_1D4X	0xA
#define PMX_VFY_V_ZOOM_A1D2X	0xB
#define PMX_VFY_V_ZOOM_A1D3X	0xC
#define PMX_VFY_V_ZOOM_A1D4X	0xD

extern void vPmxVerifyHalSysInit(void);
extern void vPmxReOpenVOPLL(void);
extern void vPmxVerifyHalReset(__u8 ucVdoId);
extern void vPmxVerifyHalLoadSetting(__u32 pu4Array[], __u32 u4Size);
extern void vPmxVerifyHalVdoPtr(__u8 ucVdoId, __u32 u4YBuf, __u32 u4CBuf);
extern void vPmxVerifyHalWrChVdoPtr(__u8 ucVdoId, __u32 u4YBuf, __u32 u4CBuf);
extern void vPmxVerifyHalWrChOn(__u8 ucVdoId, __u8 ucOn);
/*extern void vPmxHalVerifyFrameLockEn(__u8 uc4TvType, bool fgLockOn, bool fgTestEn);*/
extern void vPmxVerifyHalVdoFit(__u8 ucVdoId, __u32 u4In, __u32 u4Out);
extern void vPmxVerifyHalEnablePMX(__u8 ucVdoId, __u8 ucOn);
extern void vPmxVerifyHalVdoXSkip(__u8 ucVdoId, __u32 u4XSkip);
extern void vPmxVerifyHalZoom(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH
	, __u32 u4DstW, __u32 u4DstH, __u32 u4ZoomW, __u32 u4ZoomH);
extern void vPmxVerifyHalOSDMixRatio(__u8 ucOsdId, __u32 u4MixRatio);
extern void vPmxVerifyHalDeIntMode(__u8 ucVdoId, __u8 ucMode);
extern void vPmxVerifyHalSrcFmt(__u8 ucVdoId, __u8 ucSrcFmt);
extern void vPmxVerifyHalLumaKey(__u8 ucVdoId, __u8 ucOn, __u32 u4LumaKeyVal);
extern void vPmxVerifyHalShiftLine(__u8 ucVdoId, __u8 ucHD, __u8 ucLine);
extern void vPmxVerifyHalDispInitChkSum(__u8 ucVdoId);
extern __u32 u4PmxVerifyHalDispGetChkSum(__u8 ucVdoId);
extern void vPmxVerifyHalRoutineBreak(void);
extern void vPmxVerifyHalSetDataSource(__u8 ucOsdSel, __u8 ucFpdSel);
extern void vPmxVerifyHalInterlaceExtra(__u8 ucVdoId, __u8 ucPmxMode);

#endif /*VDP_VERIFY_HAL_H_*/




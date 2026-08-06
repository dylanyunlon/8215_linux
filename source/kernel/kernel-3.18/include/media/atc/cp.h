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
#ifndef _CP_H
#define _CP_H

#ifndef __ARM2__
#include <linux/types.h>
#else
#include "x_types.h"
#endif
#include "display.h"

#define	WHITEENHANCE		(1)
#define	BLACKENHANCE		(2)
#define	RIGHT					(0)
#define	LEFT					(1)
#define	UP						(0)
#define	DOWN					(1)

#define IS_FPGA_VER  0

typedef enum {
	CP_UNKNOW_TUNE_SEL = 0,

	CP_HUE_RESET,
	CP_HUE_SET,
	CP_HUE_GET,

	CP_YGAIN_RESET,
	CP_YGAIN_SET,
	CP_YGAIN_GET,

	CP_UGAIN_RESET,
	CP_UGAIN_SET,
	CP_UGAIN_GET,

	CP_VGAIN_RESET,
	CP_VGAIN_SET,
	CP_VGAIN_GET,

	CP_SATURATION_RESET,
	CP_SATURATION_SET,
	CP_SATURATION_GET,

	CP_BRIGHTNESS_RESET,
	CP_BRIGHTNESS_SET,
	CP_BRIGHTNESS_GET,

	CP_CONTRAST_RESET,
	CP_CONTRAST_SET,
	CP_CONTRAST_GET,

	CP_TUNE_MAX
} CP_TUNE_ITEM_ENUM;

void vCPReset(bool fgEnable, bool fgFillTable);

void vCPSCEHueConfig(s32 i4GHuePrec, s32 i4PHuePrec, s32 i4UClamp, s32 i4VClamp);
void vCPSetSCE(const u32 * const pu4SCETableWrite);
void vCPGetSCE(u32 * const pu4SCETableRead);

void vCPPatterGenerate(u8 bChanel, u32 u4En, u32 u4Step, u32 u4Prec);
void vCPGppProcess(s32 i4Mode, s32 i4DatY, s32 i4DatU, s32 i4DatV, s32 i4EnY, s32 i4EnU, s32 i4EnV);
void vCPBlackWhiteLevelEx(s32 i4Mode, s32 i4Slope, s32 i4Anchor);

void vCPSCE(s32 i4Mode, s32 i4Luma, s32 i4Hue, s32 i4Sat, s32 i4Dat);

void vCPCTI(s32 i4Mode, s32 i4Value0, s32 i4Value1);
void vCPUV2CbCr(u32 i4En, u32 u4GainU, u32 u4GainV, u32 u4Sign);
void vCPSuppression(u32 u4Mode, u32 u4En, u32 u4Gain,
			 u32 u4Offset, u32 i4SubDiv, u32 i4Spc, u32 i4Spcc);

bool VcpOnOff(u32 u4VcpIdx, bool fgOnOrOff);
void VcpSetHue(u32 u4VcpIdx, u32 u4Hue);
u32 VcpGetHue(u32 u4VcpIdx);
void VcpSetYGain(u32 u4VcpIdx, u32 u4YGain);
u32 VcpGetYGain(u32 u4VcpIdx);
void VcpSetUGain(u32 u4VcpIdx, u32 u4UGain);
u32 VcpGetUGain(u32 u4VcpIdx);
void VcpSetVGain(u32 u4VcpIdx, u32 u4VGain);
u32 VcpGetVGain(u32 u4VcpIdx);
void VcpSetContrast(u32 u4VcpIdx, u32 u4Contrast);
u32 VcpGetContrast(u32 u4VcpIdx);
void VcpSetBrightness(u32 u4VcpIdx, u32 u4Brightness);
u32 VcpGetBrightness(u32 u4VcpIdx);
void VcpSetSaturation(u32 u4VcpIdx, u32 u4Saturation);
u32 VcpGetSaturation(u32 u4VcpIdx);
void VcpEnableClk(u32 u4VcpIdx);
void VcpDisableClk(u32 u4VcpIdx);
void VcpReset(u32 u4VcpIdx);

#ifndef VCP_FOR_ANDROID
void VcpInit(u32 u4VcpIdx);
#else
void vCPBypass(s32 i4Bypass);
void vCPOn(s32 i4On);
void vCPOff(s32 i4Off);
void vCPAppSetGlobalHue(s32 i4GHue,VIDEO_SRC_TYPE src_type);
s32 vCPGetGlobalHue(void);
void vCPAppSetYUVGain(s32 i4YGain,s32 i4UGain,s32 i4VGain,VIDEO_SRC_TYPE src_type);
void vCPGetYUVGain(s32 *i4YGain,s32 *i4UGain,s32 *i4VGain);
void vCPAppSetContrBritSatr(s32 i4Contr,s32 i4Brit,s32 i4Satr,VIDEO_SRC_TYPE src_type);
void vCPGetContrBritSatr(s32 *i4Contr,s32 *i4Brit,s32 *i4Satr);
VIDEO_SRC_TYPE vCPGetSrcType(int i);
void vCPVideoOff(void);
void vCPVideoOn(void);
void vCPVideoSetVCP(VIDEO_SRC_TYPE src_type);
void VcpInit(void);
#endif

u32 VcpTuneOperation(u32 u4VcpIdx, CP_TUNE_ITEM_ENUM Item, u32 u4Value);
#endif /* _TCON_H */


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
*  Plane mixer: Interface
*****************************************************************************/

#ifndef _DRV_PMX_H_
#define _DRV_PMX_H_

#ifndef __ARM2__
#include <media/atc/x_plane_mxr.h>
#else
#include "x_plane_mxr.h"
#endif
#include "drv_config.h"

#define MODEL_DEVELOPE  1

#ifdef MODEL_CLRQAM
#ifndef CC_PMX_MT5371_API
#define CC_PMX_MT5371_API
#endif
#endif

#ifdef MODEL_DCR
#ifndef CC_PMX_MT5371_API
#define CC_PMX_MT5371_API
#endif
#endif

#ifdef MODEL_DEVELOPE
#ifndef CC_PMX_MT5371_API
#define CC_PMX_MT5371_API
#endif
#endif


/* Maximum number of input plane for each mixer*/
#define PMX_MAX_INPORT_NS		8


/* Plane mixer capability*/
#define PMX_CAP_480I			(1 << 0)
#define PMX_CAP_576I			(1 << 1)
#define PMX_CAP_480P			(1 << 2)
#define PMX_CAP_576P			(1 << 3)
#define PMX_CAP_720P60HZ		(1 << 4)
#define PMX_CAP_720P50HZ		(1 << 5)
#define PMX_CAP_1080I60HZ		(1 << 6)
#define PMX_CAP_1080I50HZ		(1 << 7)
#define PMX_CAP_1080P60HZ		(1 << 8)
#define PMX_CAP_1080P50HZ		(1 << 9)
#define PMX_CAP_1080P30HZ		(1 << 10)
#define PMX_CAP_1080P25HZ		(1 << 11)
#define PMX_CAP_1080P24HZ		(1 << 12)
#define PMX_CAP_1080P23_976HZ		(1 << 13)
#define PMX_CAP_1080P29_97HZ		(1 << 14)
#define PMX_CAP_3D_RES			(1 << 15)
#define PMX_CAP_PANEL_RES		(1 << 16)

#define PMX_CAP_REORDER			(1 << 19)
#define PMX_CAP_GAMMA			(1 << 20)
#define PMX_CAP_BRIGHTNESS		(1 << 21)
#define PMX_CAP_CONTRAST		(1 << 22)
#define PMX_CAP_HUE			(1 << 23)
#define PMX_CAP_SATURATION		(1 << 24)
#define PMX_CAP_DIGITAL_OUT		(1 << 25)
#define PMX_CAP_HDMI_DEEP_COLOR		(1 << 26)
#define PMX_CAP_SHARPNESS		(1 << 27)
#define PMX_CAP_HDCP_ON_OFF		(1 << 28)
#define PMX_CAP_CLOSED_CAPTION_ON_OFF	(1 << 29)
#define PMX_CAP_BLACK_LV		(1 << 30)

#define PMX_SWAP_1_2                  1
#define PMX_SWAP_3_4                  2
#define PMX_SWAP_6_7                  3
#define PMX_SWAP_4_7                  4
#define PMX_SWAP_1_3                  5
#define PMX_SWAP_5_8                  6

#define PMX_VDP1_ON             (0x75643021)
#define PMX_VDP1_OFF            (0x75643210)
/******************************************************************************
* PMX API
******************************************************************************/


/************************/
/* PMX native interface */
/************************/
extern void PMX_Init(bool fgHwReset);
extern void PMX_Uninit(void);
extern __s32 i4PMX_Uninit(__u32 u4Case);
extern void PMX_Stop(void);
extern void PMX_Reset(__u8 ucPmxId, bool fgHwReset);
extern void PMX_QueryStatus(void);
extern __u32 PMX_GetCap(__u8 ucPmxId, __u32 *pu4Cap);
extern __u32 PMX_SetEnable(__u8 ucPmxId, __u8 ucEnable);
extern __u32 PMX_SetForceCGMSEnable(__u8 ucPmxId, __u8 ucEnable);
extern __u32 PMX_SetCBEnable(__u8 ucPmxId, __u8 ucEnable);
extern __u32 PMX_SetMuteEnable(__u8 ucPmxId, __u8 ucEnable);
extern __u32 PMX_SetTvType(__u8 ucPmxId, __u8 ucTvType);
extern __u32 PMX_SetBlackLv(__u8 ucPmxId, __u8 ucCAVEn, __u8 ucCVBSEn);
extern __u32 PMX_GetBlackLv(__u8 ucPmxId, __u8 *pucCAVEn, __u8 *pucCVBSEn);
extern __u32 PMX_SetGamma(__u8 ucPmxId, __u8 ucGamma);
extern __u32 PMX_SetBrightness(__u8 ucPmxId, __u8 ucBrightness, __u8 bSetTVE);
extern __u32 PMX_SetContrast(__u8 ucPmxId, __u8 ucContrast, __u8 bSetTVE);
extern __u32 PMX_SetHue(__u8 ucPmxId, __u8 ucHue, __u8 bSetTVE);
extern __u32 PMX_SetSaturation(__u8 ucPmxId, __u8 ucSaturation, __u8 bSetTVE);
extern __u32 PMX_SetSharpness(__u8 ucPmxId, __u8 ucSharpness, __u8 bSetTVE);
extern __u32 PMX_SetBg(__u8 ucPmxId, __u32 u4BgColor);
extern __u32 PMX_SetPlaneOrder(__u8 ucPmxId, __u8 ucVdpId, __u8 ucPlaneOrder);
extern __u32 PMX_SetDigitalOutEnable(__u8 ucPmxId, __u8 ucEnable);
extern __u32 PMX_SetDigitalOutPara(__u8 ucPmxId, __u8 ucDigitalBit, __u8 ucMtkMode);
extern __u32 PMX_SetPlaneOrderArray(__u8 ucPmxId, const __u32 *pu4PlaneOrder);
extern __u32 PMX_SetDelay(__u8 ucVdoId, __u8 ucADJ_F, u16 u2HDelay, u16 u2VDelay);
extern __u32 PMX_SetTVSystem(__u8 ucPmxId, u16 u2VSyncFreq,  bool  fgTvSysAuto);
extern __u32 PMX_GetEnable(__u8 ucPmxId, __u8 *pucEnable);
extern __u32 PMX_GetForceCGMSEnable(__u8 ucPmxId, __u8 *pucEnable);
extern __u32 PMX_GetCBEnable(__u8 ucPmxId, __u8 *pucEnable);
extern __u32 PMX_GetMuteEnable(__u8 ucPmxId, __u8 *pucEnable);
extern __u32 PMX_GetTvType(__u8 ucPmxId, __u8 *pucTvType);
extern __u32 PMX_GetGamma(__u8 ucPmxId, __u8 *pucGamma);
extern __u32 PMX_GetGammaNs(__u8 ucPmxId, __u8 *pucGammaNs);
extern __u32 PMX_GetBrightness(__u8 ucPmxId, __u8 *pucBrightness);
extern __u32 PMX_GetContrast(__u8 ucPmxId, __u8 *pucContrast);
extern __u32 PMX_GetHue(__u8 ucPmxId, __u8 *pucHue);
extern __u32 PMX_GetSaturation(__u8 ucPmxId, __u8 *pucSaturation);
extern __u32 PMX_GetSharpness(__u8 ucPmxId, __u8 *pucSharpness);
extern __u32 PMX_GetPlaneOrder(__u8 ucPmxId, __u8 ucVdpId, __u8 *pucPlaneOrder);
extern __u32 PMX_SetSwapEnable(__u8 ucPmxId, __u8 ucEnable, __u8 ucSwapMode);
extern __u32 PMX_GetSwapEnable(__u8 ucPmxId, __u8 *pucEnable, __u8 ucSwapMode);
extern __u32 PMX_GetDigitalOutEnable(__u8 ucPmxId, __u8 *pucEnable);
extern __u8 PMX_ProgressiveOut(__u8 ucPmxId);
extern __u32 PMX_SetExternalChipSetting(PLA_MXR_EXTERNAL_CHIP_SETTING_E eSetType, __s32 i4Value);
extern __u32 PMX_SetAspectRatio(__u8 ucPmxId, __u32 u4AspectRatio);
extern __u32 PMX_SetAspectRatioEx(__u8 ucPmxId, __u32 u4AspectRatio, bool fgApplyOnNext);
extern __u32 PMX_SetAspectRatioNext(__u8 ucPmxId);
extern __u32 PMX_SetDigitalOutPhase(__u8 ucPmxId, __u32 u4Phase);
extern __u32 PMX_SetDigitalOutDrv(__u32 u4Drving);
extern __u32 PMX_SetDigitalOutRgb(__u8 ucPmxId, __u8 ucRgb);
extern __u32 PMX_GetDigitalOutRgb(__u8 ucPmxId, __u8 *pucRgb);
extern __u32 i4Pmx_Connect(u16 u2ThisCompType, u16 u2ThisCompId
	, u16 u2UpstreamCompType, u16 u2UpstreamCompId);
extern __u32 i4Pmx_Disconnect(u16 u2ThisCompType, u16 u2ThisCompId
	, u16 u2UpstreamCompType, u16 u2UpstreamCompId);
extern __u32 PMX_ChkSupportedRes(__u8 ucPmxId, __u8 ucChkRes, u16 u2VSyncFreq
	,  __u8 *ucSupportedAutoRes, u16 *ucSupportedAutoVsyncFreg);
extern __u32 PMX_SetHdmiDeepColor(__u8 ucPmxId, __u8 ucDeepColor);
extern __u32 PMX_SetHdcpOnOff(__u8 ucPmxId, __u8 ucHdcpOnOff);
extern __u32 PMX_SetClosedCaptionOnOff(__u8 ucPmxId, PLA_MXR_CLOSED_CAPTION_ON_OFF_T eClosedCaptionOnOff);
extern void PMX_RecordUiResAuto(bool fgIsAuto);
extern __u32 PMX_SetDigestAspectRatioEx(__u8 ucPmxId, __u32 u4AspectRatio, bool fgDigestEnable);

extern __u32 u4PmxCompId2LocalId(u16 u2CompType, u16 u2CompId);
extern __u32 PMX_SetMediaType(__u8 ucPmxId, __u8 ucMediaType);
extern __u32 PMX_SetDGOSource(bool fgBeforeMixer);
extern __u32 PMX_SetCVBSSetup(bool fgEnable);
#if CONFIG_DRV_3D_SUPPORT
extern PMX_VSYNC_MODE_T PMX_GetCurrVSyncMode(__u8 ucPmxId);
extern PMX_VSYNC_MODE_T PMX_GetNextVSyncMode(__u8 ucPmxId);
extern void PMX_Set3DMode(PMX_3D_MODE_T ePmx3DMode);
extern PMX_3D_MODE_T PMX_Get3DMode(void);
#endif

extern __u32 PMX_SetArbPlaneOrder(__u8 ucPmxId, __u32 u4PlaneOrder);
extern __u32 PMX_GetArbPlaneOrder(__u8 ucPmxId, __u32 *pu4PlaneOrder);

/***************/
/* MW Temp API */
/***************/
extern __u32 d_PMX_SetOrderVdpComp0(__u8 ucPmxId, __u32 u4OnTop); /* 1: VDP 0 is on top; 0: VDP 0 is on bottom.*/
extern __u32 d_PMX_GetOrderVdpComp0(__u8 ucPmxId, __u32 *pu4OnTop);


/***********/
/* OLD API */
/***********/
#ifdef CC_DIGITAL_VDO_LOOPBACK
extern __u32 PMX_SetTveEnable(__u8 ucTveId);
extern __u32 PMX_SetTveDisable(__u8 ucTveId);
extern __u32 PMX_GetLoopback(__u8 ucPmxId, __u32 *pu4Loopbacking, __u32 *pu4LbWidth, __u32 *pu4LbHeight);
extern __u8 PMX_GetLoopbackPmx(void);
#endif


#endif /* _DRV_PMX_H_ */




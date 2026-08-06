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
*  Video Plane: Interface
*****************************************************************************/

#ifndef _DRV_VDP_H_
#define _DRV_VDP_H_

#include <media/atc/x_vid_plane.h>
#include <media/atc/x_vid_dec.h>
#include "drv_common.h"
#include "drv_config.h"
#include "chip_ver.h"

#define MODEL_DEVELOPE  1

#ifdef MODEL_CLRQAM
#ifndef CC_VDP_MT5371_API
#define CC_VDP_MT5371_API
#endif
#endif

#ifdef MODEL_DCR
#ifndef CC_VDP_MT5371_API
#define CC_VDP_MT5371_API
#endif
#endif

#ifdef MODEL_DEVELOPE
#ifndef CC_VDP_MT5371_API
#define CC_VDP_MT5371_API
#endif
#endif

/* Maximum number of video plane*/
#define VDP_MAX_NS				3
#define VDP_NS					3
#define VDP_HW_NS       2
#define VDP_1					0	/* VDP1*/
#define VDP_2					1	/* VDP2*/
#define VDP_3					2	/* OSD1, Virtual VDP*/
#define VDP_4					3	/* OSD2, Virtual VDP*/
#define VDP_5					4	/* OSD3, Virtual VDP*/
#define VDP_6                   5
#define VDP_7                   6
#define VDP_8                   7

/* Maximum number of input port for each video plane*/

/* Video plane configuration return value*/
#define VDP_SET_ERROR			0
#define VDP_SET_OK				1

/* Video plane status*/
/*#define VDP_STATUS_NOSIGNAL         0
#define VDP_STATUS_NOSUPPORT        1
#define VDP_STATUS_UNKNOWN          2
#define VDP_STATUS_STABLE           3
*/
/* Video plane argument*/
#define VDP_ARG_BLENDING            0
#define VDP_ARG_BRIGHTNESS          1
#define VDP_ARG_CONTRAST            2
#define VDP_ARG_HUE                 3
#define VDP_ARG_SATURATION          4
#define VDP_ARG_BLE                 5
#define VDP_ARG_CTI                 6
#define VDP_ARG_ETI                 7
#define VDP_ARG_SHARPNESS_ON_OFF    8
#define VDP_ARG_SHARPNESS           9
#define VDP_ARG_NR                  10
#define VDP_ARG_COLOR_SUPPRESS      11
#define VDP_ARG_DEINT_FILM          12
#define VDP_ARG_DEINT_DEINT         13
#define VDP_ARG_DEINT_EDGE          14
#define VDP_ARG_ADAPTIVE_LUMA       15
#define VDP_ARG_SCE                 16
#define VDP_ARG_R_GAIN				17
#define VDP_ARG_G_GAIN				18
#define VDP_ARG_B_GAIN				19
#define VDP_ARG_R_OFFSET			20
#define VDP_ARG_G_OFFSET			21
#define VDP_ARG_B_OFFSET			22
#define VDP_ARG_GAMMA				23
#define VDP_ARG_WHITE_PEAK_LMT		24
#define VDP_ARG_BACK_LIGHT_LVL      25
#define VDP_ARG_ADAPTIVE_BACK_LIGHT 26
#define VDP_ARG_3D_NR               27
#define VDP_ARG_NS                  28
#define VDP_ARG_SOFT_BWS            29    /*SoftBWS*/

/*                                     // Video plane mode
#define VDP_MODE_NORMAL			0
#define VDP_MODE_BG				1
#define VDP_MODE_DEINT			3
#define VDP_MODE_UNKNOWN		4
*/
/* Video plane format*/
/*#define VDP_FMT_NORMAL			0
#define VDP_FMT_LETTERBOX		1
#define VDP_FMT_PAN_SCAN		2
#define VDP_FMT_USER			3
#define VDP_FMT_UNKNOWN			4
*/
/* Video plane enhance mode*/
/*#define VDP_ENC_NONE			0
#define VDP_ENC_ON				1
#define VDP_ENC_BLUR			2
#define VDP_ENC_UNKNOWN			3
*/
/* Video plane capability*/
#define VDP_CAP_OUTPUT_AUX		(1 << 0)
#define VDP_CAP_QV				(1 << 1)
#define VDP_CAP_DEINT			(1 << 2)
#define VDP_CAP_ENHANCE			(1 << 3)
#define VDP_CAP_420_CM			(1 << 4)
#define VDP_CAP_32BIT_CM		(1 << 5)

/* Aspect Ratio*/
#define VDP_UNIT_ASPECT_RATIO	0x10000
#define VDP_4_3_ASPECT_RATIO	((VDP_UNIT_ASPECT_RATIO * 4) / 3)
#define VDP_16_9_ASPECT_RATIO	((VDP_UNIT_ASPECT_RATIO * 16) / 9)
#define VDP_221_1_ASPECT_RATIO	((VDP_UNIT_ASPECT_RATIO * 221) / 100)

/*
typedef enum
{
    VDP_REGION_TYPE_UNKNOWN = 0,
    VDP_REGION_TYPE_PIXEL,
    VDP_REGION_TYPE_PERMILLE,
} VDP_REGION_TYPE_T;
*/
/* Video plane region (source/output)*/
typedef struct _VDP_REGION_T {
	VID_PLA_REGION_TYPE_T   eRegionType;
	__u32			            X;
	__u32			            Y;
	__u32			            Width;
	__u32			            Height;
	__u8                   bIsFullRegion;
} VDP_REGION_T;

/* Video Plane Call Back Error*/
#define VDP_CB_ERR				(0x80000000)

typedef enum _EVDP_VIDEO_INPUT_SOURCE_T {
	EVDP_VIS_TUNER_DIGITAL = 0,

	EVDP_VIS_MAX
} EVDP_VIDEO_INPUT_SOURCE_T;

/** Brief of EVDP_UI_VIDEO_QUALITY_ITEM_T
 *  Ui Video Quality Item (UVQI) type
 */
typedef enum _EVDP_UI_VIDEO_QUALITY_ITEM_T {
	EVDP_UVQI_BRIGHTNESS = 0,
	EVDP_UVQI_CONTRAST,
	EVDP_UVQI_HUE,
	EVDP_UVQI_SATURATION,
	EVDP_UVQI_CTI,
	EVDP_UVQI_ETI,
	EVDP_UVQI_SHARPNESS,
	EVDP_UVQI_COLOR_GAIN,
	EVDP_UVQI_COLOR_OFFSET,
	EVDP_UVQI_COLOR_SUPPRESS,
	EVDP_UVQI_NR,
	EVDP_UVQI_BLACK_LVL_EXT,
	EVDP_UVQI_WHITE_PEAK_LMT,
	EVDP_UVQI_FLESH_TONE,       /* SCE: second color enhance*/
	EVDP_UVQI_LUMA,             /* adaptive luma*/
	EVDP_UVQI_BACK_LIGHT_LVL,
	EVDP_UVQI_ADAPTIVE_BACK_LIGHT,
	EVDP_UVQI_3D_NR,

	EVDP_UVQI_MAX
} EVDP_UI_VIDEO_QUALITY_ITEM_T;

typedef void (*PFN_VDP_ERR_NOTIFY)(__u32 u4Arg1, __u32 u4Arg2, __u32 u4Arg3, __u32 u4Arg4);
typedef void (*PTS_CB_FUNC)(__u32 u4Pts, __u32 u4Arg);

/** Brief of VDP_CONF_T
 *  min, max, default values of UI video quality item.
 */
typedef struct _VDP_UI_VQ_MIN_MAX_DFT_T {
	__s32 i4Min;
	__s32 i4Max;
	__s32 i4Dft;
} VDP_UI_VQ_MIN_MAX_DFT_T;
/*

typedef enum
{
    VDP_NFY_COND_ERROR = -1,
    VDP_NFY_COND_CTRL_DONE,
    VDP_NFY_INBAND_CMD_DONE,
    VID_NFY_COND_AUTO_PAUSE_DONE,
    VID_NFY_COND_INJECT_DONE
} VDP_NFY_COND_T;

*/

#if !UNIFORM_DRV_CALLBACK
typedef void (*PFN_VDP_NFY_FCT)(
	void               *pvNfyTag,
	VID_PLA_COND_T      eNfyCond,
	__u32              u4Data1,
	__u32              u4Data2
);


typedef struct _VDP_NFY_INFO_T {
	void *pvTag;
	PFN_VDP_NFY_FCT pfNfyFct;
} VDP_NFY_INFO_T;
#endif

typedef enum {
	CAPTURE_CURRENT_PICTURE,
	CAPTURE_NTH_PICTURE,
	CAPTURE_PICTURE_WITH_PTS,
} VDP_CMD_CAPTURE_MODE_T;
typedef struct {
	VDP_CMD_CAPTURE_MODE_T eCaptureType;
	union {
		u64 Pts;
		__u32 FrameNum;
	} u;
	__u32 AddrY;
	__u32 AddrC;
	__u32 Format;
	__u16 FrameWidth;
	__u16 FrameHeight;
	__u16 Width;
	__u16 Height;
} VDP_CMD_CAPTURE_T;
/* VID_PLA_GET_TYPE_PICTURE_INFO **********************************************/
/*typedef enum
{
    VDP_PIC_INFO_TYPE_UNKNOWN = 0,
    VDP_PIC_INFO_TYPE_CURR_DISPLAY,
    VDP_PIC_INFO_TYPE_LATEST_DISPLAY_I,
} VDP_PIC_INFO_TYPE_T;
*/
typedef struct _VDP_PICTURE_INFO_T {
	VID_PLA_PIC_INFO_TYPE_T   Type;
	u64                    u8Offset;
	u64                    u8Pts;
	PBINF_V                   rPbInf;               /*/< Playback information*/
} VDP_PICTURE_INFO_T;

typedef enum {
	VDP_DISABLED = 0,
	VDP_HIDDEN,
	VDP_ENABLED
} VDP_ENABLE_T;
/* defines.*/
/* Picture Coding Type*/
#define MPEG_PIC_TYPE_I						1
#define MPEG_PIC_TYPE_P						2
#define MPEG_PIC_TYPE_B						3

/* Picture Structure*/
#define MPEG_PIC_TOP_FIELD					1
#define MPEG_PIC_BOTTOM_FIELD				2
#define MPEG_PIC_FRAME						3

/* Frame Rate Code*/
#define MPEG_FRAME_RATE_24_					1	/* minus*/
#define MPEG_FRAME_RATE_24					2
#define MPEG_FRAME_RATE_25					3
#define MPEG_FRAME_RATE_30_					4	/* minus*/
#define MPEG_FRAME_RATE_30					5
#define MPEG_FRAME_RATE_50					6
#define MPEG_FRAME_RATE_60_					7	/* minus*/
#define MPEG_FRAME_RATE_60					8

/* Aspect Ratio*/
#define MPEG_ASPECT_RATIO_1_1				1	/* SAR, 1:1*/
#define MPEG_ASPECT_RATIO_4_3				2	/* DAR, 4:3*/
#define MPEG_ASPECT_RATIO_16_9				3	/* DAR, 16:9*/
#define MPEG_ASPECT_RATIO_221_1				4	/* DAR, 2.21:1*/

/* Colour Primary*/
#define MPEG_COLOR_PRIMARY_709				1

#define VDP_FRAME_RATE_UNKNOWN				0xFF
#define VDP_FRAME_RATE_24					24
#define VDP_FRAME_RATE_25					25
#define VDP_FRAME_RATE_30					30
#define VDP_FRAME_RATE_50					50
#define VDP_FRAME_RATE_60					60


#define VDP_BACKGROUND    2

/* log variables*/
typedef struct VDP_DBG_FRAME_LOG_T {
	__u32  u4Pts;
	__u32  u4Stc;
	__u32  u4SoftPts;
	__u8   ucVsyncNs;
	__u8   VdpId;
} VDP_DBG_FRAME_LOG_T;

#define MAX_FRAME_LOG_COUNT         512

typedef struct {
	VID_PLA_PAUSE_MODE_T  ePauseMode;
} VDP_GLOBAL_SET_T;

extern VDP_DBG_FRAME_LOG_T   FrameLog[MAX_FRAME_LOG_COUNT];

/******************************************************************************
* VDP API
******************************************************************************/
#ifdef __linux__
/*TODO: use the newest vdp driver*/
extern void VDP_Init(void);
extern void VDP_Reset(__u8 ucVdpId);
#else
extern void VDP_Init(bool fgHwReset);
extern void VDP_Reset(__u8 ucVdpId, bool fgHwReset);
#endif
extern __s32 VDP_Uninit(__u32 u4Case);
extern void VDP_CheckFbgReady(void);
extern void VDP_FrcResetPort(__u8 ucVdpId);

extern __u32 VDP_SetEnable(__u8 ucVdpId, VDP_ENABLE_T eEnable);
extern __u32 VDP_SetMode(__u8 ucVdpId, VID_PLA_MODE_T eMode);
extern __u32 VDP_SetFmt(__u8 ucVdpId, VID_PLA_DISP_FMT_T Fmt);
extern __u32 VDP_SetEnhance(__u8 ucVdpId, VID_PLA_ENHANCE_MODE_T Enhance, __u8 ucEnhanceLevel);
extern __u32 VDP_SetAlpha(__u8 ucVdpId, __u8 ucAlphaValue);
extern __u32 VDP_SetInput(__u8 ucVdpId, __u8 ucEsId);
extern __u32 VDP_SetOutput(__u8 ucVdpId, __u8 ucPmxId);
extern __u32 VDP_SetRegion(__u8 ucVdpId, __u8 ucSrcFullRegion, VDP_REGION_T rSrcRegion
	, __u8 ucOutFullRegion, VDP_REGION_T rOutRegion);
extern __u32 VDP_SetSrcRegion(__u8 ucVdpId, __u8 bIsDelayed, VDP_REGION_T rSrcRegion);
extern __u32 VDP_SetOutRegion(__u8 ucVdpId, __u8 bIsDelayed, VDP_REGION_T rOutRegion);
extern __u32 VDP_SetBgRegion(__u8 ucVdpId, VDP_REGION_T rBgRegion, SOURCE_ASPECT_RATIO_T Asp);
extern __u32 VDP_SetBg(__u8 ucVdpId, __u32 u4BgColor);
extern __u32 VDP_SetColorMode(__u8 ucVdpId, DRV_PMX_COLOR_SPACE_T eColorMode);
extern __u32 VDP_SetAddr(__u8 ucVdpId, __u32 u4AddrY, __u32 u4AddrC);
extern __u32 VDP_SetSrcSize(__u8 ucVdpId, __u32 u4SrcWidth, __u32 u4SrcHeight);		/* for CLI test*/
extern __u32 VDP_SetAcsMode(__u8 ucVdpId, __u8 ucAcsFrame, __u8 ucSrcFrame
	, __u8 ucAcsAuto, __u8 ucPhyTop, __u8 ucAcsTop);
extern __u32 VDP_GetEnable(__u8 ucVdpId, VDP_ENABLE_T *pEnable);
extern __u32 VDP_GetMode(__u8 ucVdpId, VID_PLA_MODE_T *peMode);
extern __u32 VDP_GetModeWithProtection(__u8 ucVdpId, __u8 *pucMode);
extern __u32 VDP_GetFmt(__u8 ucVdpId, __u8 *pucFmt);
extern __u32 VDP_GetEnhance(__u8 ucVdpId, VID_PLA_ENHANCE_MODE_T *pEnhance, __u8 *pucEnhanceLevel);
extern __u32 VDP_GetAlpha(__u8 ucVdpId, __u8 *pucAlphaValue);
extern __u32 VDP_GetOutput(__u8 ucVdpId, __u8 *pucPmxId);
extern __u32 VDP_GetSrcRegion(__u8 ucVdpId, VDP_REGION_T *prSrcRegion);
extern __u32 VDP_GetOutRegion(__u8 ucVdpId, VDP_REGION_T *prOutRegion);
extern __u32 VDP_SetFullScreen(__u8 ucVdpId, bool b_full_screen_on);
extern __u32 VDP_GetBgRegion(__u8 ucVdpId, VDP_REGION_T *prRegion);
extern __u32 VDP_GetBg(__u8 ucVdpId, __u32 *pu4BgColor);
extern __u32 VDP_GetColorMode(__u8 ucVdpId, DRV_PMX_COLOR_SPACE_T *eColorMode);
extern __u32 VDP_GetAddr(__u8 ucVdpId, __u32 *pu4AddrY, __u32 *pu4AddrC);
extern __u32 VDP_GetSrcSize(__u8 ucVdpId, __u32 *pu4SrcWidth, __u32 *pu4SrcHeight);
extern __u32 VDP_Get_Sample_Aspect_Ratio(__u8 ucVdpId, __u32 *pu4ARW, __u32 *pu4ARH);
extern __u32 VDP_GetAcsMode(__u8 ucVdpId, __u8 *pucAcsFrame, __u8 *pucSrcFrame
	, __u8 *pucAcsAuto, __u8 *pucPhyTop, __u8 *pucAcsTop);
extern __u32 VDP_GetInput(__u8 ucVdpId, __u8 *pucFbgId);
extern __u32 VDP_IsPopPlane(__u8 ucVdpId, __u32 *pu4PopPlane, __u32 *pu4ColourPrimary);

extern __u32 VDP_SetPtsCb(__u32 u4EsId, PTS_CB_FUNC pfCbPtsFun, __u32 u4CbPts);

extern void VDP_SetZYXW(__u32 u4Value);

#if UNIFORM_DRV_CALLBACK
extern __s32 VDP_SetNotifyFunction(__u8 ucVdpId, DRV_CB_REG_INFO_T *prNfyInfo);
#else
extern __s32 VDP_SetNotifyFunction(__u8 ucVdpId, VDP_NFY_INFO_T *prNfyInfo);
#endif

extern void VDP_GetPicInfo(__u8 ucVdpId, VDP_PICTURE_INFO_T *prPicInfo);
extern void VDP_GetPBInfo(__u8 ucVdpId, VID_PLA_PICTURE_INFO_T *prPicInfo);
extern u64 VDP_GetPts(__u8 ucVdpId);
extern __u32 VDP_CreateDispQueue(__u8 ucVdpId, __u16 u2CompType, __u16 u2CompId);
extern __u32 VDP_DestroyDispQueue(__u8 ucVdpId);

extern __s32 VDP_IsInitialized(void);
extern __s32 i4VDP_Simp_Connect(__u16 u2VdecCompType,
				__u16 u2VdecCompId,
				__u16 u2VdpCompType,
				__u16 u2VdpCompId);

extern __s32 i4VDP_Simp_Disconnect(__u16 u2VdecCompType,
				   __u16 u2VdecCompId,
				   __u16 u2VdpCompType,
				   __u16 u2VdpCompId);

extern __u32 VDP_FlushFrameBuffer(__u8 ucVdpId);
#ifdef ENUM_SRC_ASPECT_RATIO
extern __u32 VDP_InjectPicture(__u8 ucVdpId, __u32 pAddrY, __u32 pAddrC
	, __u16 Width, __u16 Height, SOURCE_ASPECT_RATIO_T eAspectRatio);
#else
extern __u32 VDP_InjectPicture(__u8 ucVdpId, __u32 pAddrY, __u32 pAddrC
	, __u16 Width, __u16 Height, __u8 AspectRatio);
#endif
extern __u32 VDP_GetUiVqItemMinMaxDftCur(__u8 u1VidPath, __u8 ucArgType
	, INT16 * pi2Min, INT16 * pi2Max, INT16 * pi2Dft, __u8 *pucCur);
extern __u32 VDP_SetLumaKey(__s32 bIsOn, VID_PLA_LUMA_T Key);
extern __u32 VDP_SetTvFormat(PMX_RESOLUTION_MODE_T eFmt, __u8 bIsResetHw);
extern __u32 VDP_EnableCC(__s32 bIsOn);
extern __u32 VDP_GetMainVdpId(void);
extern void VDP_GlobalOutputChange(void);
extern void VDP_CheckBufferInvalid(__u8 FBGId, __u8 FBId);
extern __u32 VDP_GetCurrPictureInfo(__u8 ucVdpId, u64 *pPts, __u8 *pbIsOpenB);
extern __u32 VDP_CapturePicture(__u8 ucVdpId, VDP_CMD_CAPTURE_T *pCapInfo);
extern __u32 VDP_CaptureAbort(__u8 ucVdpId);
extern __u8 VDP_GetHwEnable(__u8 ucVdpId);
#if (CONFIG_DRV_NR_SUPPORT)
extern __u32 VDP_SetNRStrength(__u32 u4Strength);
extern __u32 VDP_SetNRDBGLevel(__u32 u4DBGLevel);
extern __u32 VDP_SetNRMode(__u32 u4NRMode);
extern __u32 VDP_SetSharpStrength(__u32 u4Strength);
#endif


__u32 VDP_GetDispQueue(__u32 ucVdpId);
__u32 VDP_GetUnderRunNum(__u32 ucVdpId);
extern __u32 VDP_SetPauseMode(VID_PLA_PAUSE_MODE_T ePauseMode);
extern __u32 VDP_IgnoreCPSIBCmd(bool fgVdpIgnoreCPSIBCmd);
extern __u32 VDP_ClearCPSInfo(bool fgIsScreenSaver);

extern __u32 VDP_SetVideoBlack(__u32 ucVdpId, __u8 VideoY, __u8 VideoCb, __u8 VideoCr);

#endif /* _DRV_VDP_H_ */




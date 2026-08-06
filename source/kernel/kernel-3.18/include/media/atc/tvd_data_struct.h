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


#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H
#ifdef __ARM2__
#include "x_types.h"
#else
#include <linux/types.h>
#endif

#define TVD_EVT_NAME_MAX_LENGTH        20

#define SUPPORT_CALIBRATE_BRIGHTNESS  (1)

#define PAL_FRAME_WIDTH               (720)
#define PAL_FRAME_HEIGHT              (576)

#define SECAM_FRAME_WIDTH             (720)
#define SECAM_FRAME_HEIGHT            (576)

#define NTSC_FRAME_WIDTH              (720)
#define NTSC_FRAME_HEIGHT             (480)

#define TVD_CHA_BYPASS         0U
#define TVD_CHB_BYPASS         1U
#define TVD_CH0_BYPASS         2U


#define TVD_CFG_OUT_NONE    0
#define TVD_CFG_OUT_DRAM    1
#define TVD_CFG_OUT_BYPASS  2

typedef enum {
	TVD_CH_NONE,
	TVD_CHA,
	TVD_CHB,
	TVD_CHA_CHB
} TVD_CHANNEL_T;

typedef enum {
	TVD_IDLE_STATUS,
	TVD_ININIT_STATUS,
	TVD_CONFIG_STATUS,
	TVD_START_STATUS,
	TVD_STOP_STATUS,
} TVD_OPERATION_STATUS;

#define TVD_CTRL_STOP_NONE    0
#define TVD_CTRL_STOP_PAUSE   1
#define TVD_CTRL_STOP_STOP    2

typedef enum {
	TVD_APP_ID_NONE     = 0x01,
	TVD_APP_ID_BACKCAR  = 0x02,
	TVD_APP_ID_AVIN     = 0x04,
	TVD_APP_ID_ATV,
	TVD_APP_ID_IPOD,
	TVD_APP_ID_BACKCAR_ARM2,
	TVD_APP_ID_DGI,
	TVD_APP_ID_DBG_BYPASS,
	TVD_APP_ID_MAX
}  TVD_APP_ID_ENUM;

/* 
	|*0          1          1          1 -------0          1          1          1*| 
	             	  |          |          |                           |          |          | 
	             cu_avin   cu_bc   cu_no                 wi_avin   wi_bc   wi_no
	  the 'cu' is current mean which we are playing
	  the 'wi' is will mean which indicate that we want to play
*/

typedef enum {
	NONE_IDLE         = 0x10,
	NONE_NONE		  = 0x11,
	NONE_BACKCAR      = 0x12,
	NONE_AVIN         = 0x14,
	BACKCAR_IDLE	  = 0x20,
	BACKCAR_NONE      = 0x21,
	BACKCAR_BACKCAR   = 0x22,
	BACKCAR_AVIN      = 0x24,
	AVIN_IDLE         = 0x40,
	AVIN_NONE         = 0x41,
	AVIN_BACKCAR      = 0x42,
	AVIN_AVIN         = 0x44
} TVD_TYPE_STATE_TRANSITION;



typedef enum{
    TVD_SIG_NONE,
    TVD_SIG_READY,
    TVD_SIG_LOST,
    TVD_SIG_CHANGE_START,
    TVD_SIG_CHANGE_DONE,
    TVD_SIG_CLOSE_WCH
} TVD_SIG_STATE_T;

enum {
	CVBSIN_0P = 0,
	CVBSIN_1P,
	CVBSIN_2P,
	CVBSIN_3P,
	CVBSIN_4P,
	CVBSIN_5P,
	CVBSIN_NONE
};


typedef enum  {
	TVD_VDOFMT_UNKNOWN = 0,
	TVD_VDOFMT_YUV420,
	TVD_VDOFMT_YUV422,
	TVD_VDOFMT_YUV444,        /* TVD format */
	TVD_VDOFMT_BT656,
	TVD_VDOFMT_BT601,
} TVD_VDO_FMT_E;


typedef enum {
	TVD_FLIP_NONE,
	TVD_FLIP_H,
	TVD_FLIP_V,
	TVD_FLIP_HnV
} TVD_MIRROR_CFG_E;


/* CVBS Signal System*/
enum  {
	AV_MODE_PAL = 1,
	AV_MODE_NTSC, /* 2*/
	AV_MODE_SECAM, /* 3*/
	AV_MODE_NTSC443,
	AV_MODE_UNSTABLE,  /* Represent video signal is not stable yet! In Hardware, it's reserved // 4*/
	AV_MODE_NONE  /*5*/
};

typedef struct _VDO_BUF_INFO_
{
    u32 dwPhyYAddr;
    u32 dwVirYAddr;
    u32 dwPhyCAddr;
    u32 dwVirCAddr;

    u32 u4ScanMode;
    u32 u4PicWidth;
    u32 u4PicHeight;
    u32 u4AlignWidth;
    u32 u4AlignHeight;

    u32 u4OutType;
    u32 u4Flags;
    u8 szName[10];
    u32 ucCheckSum;
} VDO_BUF_INFO_T,*PVDO_BUF_INFO_T;


typedef struct {
	u32 AppId;
	u32 CtrlCode;
	u32 CtrlParaLen;
	u8 *pCtrlPara;
} TVD_CTRL_T, *PTVD_CTRL_T;

typedef struct {
	u32            u4CHACvbsInxP;
	u32            u4CHBCvbsInxP;
	u32            u4CHAOutDest; /* 1. DRAM ->front or rear vdp 2. bypass to tve*/
	u32            u4CHBOutDest;/* 1. DRAM ->front or rear vdp 2. bypass to tve*/
	TVD_VDO_FMT_E     eVdoInFmt;        /* input   video format */
	TVD_VDO_FMT_E     eVdoOutFmt;       /* output video format */
	s8             szSigStateEvtName[TVD_EVT_NAME_MAX_LENGTH];
	TVD_APP_ID_ENUM source_type;
	u32             u4UVSwap;
} TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T, *PTVD_DRV_CAMERA_PREVIEW_INIT_INFO_T;

typedef struct {
	/*Color Enhancement parameter*/

	u8             u1HueLvl;            /* hue gain level */
	u8             u1SaturationLvl;     /* saturation level */
	u8             u1BrightnessLvl;     /* brightness gain level */
	TVD_MIRROR_CFG_E  eMirrorCfg;     /* Set Mirror direction */

#if SUPPORT_CALIBRATE_BRIGHTNESS
	/**************************************************************************
	* @brief TVD Color Process Convsion
	*        Color process formula: Yout = Yin * YGain + YOffset
	*                             Uout = Uin * UCosGain + Vin * VSinGain + UOffset
	*                             Vout = Vin * VCosGain + Uin * USinGain + VOffset
	*
	***************************************************************************/
	u8            u1Mask; /* YGAIN_VALID_MASK/YOFFSET_VALID_MASK/UCOSGAIN_VALID_MASK/VCOSGAIN_VALID_MASK......*/

	u8            u1YGain;
	u8            u1YOffset;
	u8            u1UCosGain;
	u8            u1VCosGain;

	u8            u1USinGain;
	u8            u1VSinGain;
	u8            u1UOffset;
	u8            u1VOffset;
#endif
	TVD_APP_ID_ENUM source_type;
} TVD_DRV_CAMERA_PREVIEW_CFG_T, *PTVD_DRV_CAMERA_PREVIEW_CFG_T;


typedef struct {
	TVD_CHANNEL_T    stop_channel;/* stop which channel CHA/CHB/CHA&CHB*/
	TVD_APP_ID_ENUM source_type;
} TVD_DRV_STOP_PARA_T, *PTVD_DRV_STOP_PARA_T;


typedef struct {
	TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T rInitInfo;
	TVD_DRV_CAMERA_PREVIEW_CFG_T       rCfgInfo;
	u32          						u4StartCHX;/* Start which channel CHA/CHB/CHA&CHB*/
	TVD_DRV_STOP_PARA_T                rStopInfo;
	u32                             u4CHAOpStat; /* Channel A Operation state*/
	u32                             u4CHBOpStat; /* Channel B  Operation state*/
	u32                             u4UseCHX;
	u32                             u4PwrOnObj; /*Current app power on objs*/
	void                            *hSigStateEvt;
	TVD_APP_ID_ENUM                    eSrcID;
	/*Add for linux event api bug*/

} TVD_DRV_APP_INFO_T, *PTVD_DRV_APP_INFO_T;

typedef struct {
	TVD_SIG_STATE_T signal_state;
	u32			arg;
} TVD_SIG_INFORMATION;

typedef void (*tvd_notify)(void *arg);

struct tvd_notify_s {
	tvd_notify	notify_fun;
	void			*arg;
};


#endif

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

#ifndef _DRV_POST_POST_H_
#define _DRV_POST_POST_H_

#include "drv_av_d.h"
#include "drv_if_pmx.h"

#define POST_OK	           (INT32)(0)
#define POST_FAIL            (INT32)(-1)

#define SV_ON	           (INT32)(1)
#define SV_OFF            (INT32)(0)

typedef enum
{
    POST_VIDEO_CONTRAST = 0,
    POST_VIDEO_BRIGHTNESS,
    POST_VIDEO_HUE,
    POST_VIDEO_SATURATION,
    POST_VIDEO_SHARPNESS,
    POST_VIDEO_CDS,
    POST_VIDEO_CTI,
    POST_VIDEO_ADAPTIVE_LUMA_ONOFF,
    POST_VIDEO_SCE_ONOFF,
    
    POST_VIDEO_COLOR_RED_Y,
    POST_VIDEO_COLOR_RED_S,
    POST_VIDEO_COLOR_RED_H,
    POST_VIDEO_COLOR_GREEN_Y,
    POST_VIDEO_COLOR_GREEN_S,
    POST_VIDEO_COLOR_GREEN_H,
    POST_VIDEO_COLOR_BLUE_Y,
    POST_VIDEO_COLOR_BLUE_S,
    POST_VIDEO_COLOR_BLUE_H,
    POST_VIDEO_COLOR_YELLOW_Y,
    POST_VIDEO_COLOR_YELLOW_S,
    POST_VIDEO_COLOR_YELLOW_H,
    POST_VIDEO_COLOR_CYAN_Y,
    POST_VIDEO_COLOR_CYAN_S,
    POST_VIDEO_COLOR_CYAN_H,
    POST_VIDEO_COLOR_MAGENTA_Y,
    POST_VIDEO_COLOR_MAGENTA_S,
    POST_VIDEO_COLOR_MAGENTA_H,
  
}   POST_UI_ITEM_T;

typedef enum
{
    POST_VIDEO_MODE_STD = 0,
    POST_VIDEO_MODE_VIVID,
    POST_VIDEO_MODE_CINEMA,
    POST_VIDEO_MODE_CUSTOMER,
    POST_VIDEO_MODE_OTHER,
  
}   POST_VIDEO_MODE_T;

typedef struct
{
    UINT8 bLimitAllPos;
    UINT8 bLimitAllNeg;
    BOOL  fgShnEn;
    
} POST_SHN_CTRL_PARA;

typedef enum
{
    SHN_BAND_H1 = 1,
    SHN_BAND_V  = 2,
    SHN_BAND_X1 = 3,
    SHN_BAND_X2 = 4,
    SHN_BAND_H2 = 5,
    SHN_BAND_H3 = 6
  
}   POST_SHN_BAND_T;

typedef struct
{
    POST_SHN_BAND_T eShnBand;
    UINT8 bGain;
    UINT8 bCoring;
    UINT8 bLimitPos;
    UINT8 bLimitNeg;
    UINT8 bClipEn;
    UINT8 bClipThPos;
    UINT8 bClipThNeg;
    
} POST_SHN_BAND_PARA;


// *********************************************************************
// Export API
// *********************************************************************

/*** Post TASK ***/
extern INT32 i4Post_Init(void);
extern INT32 i4Post_UnInit(void);
extern void vPostVsyncTick(void);
extern INT32 i4PostVideoProc(POST_UI_ITEM_T e_UI_Item, 
                             INT16 i2UIMin, INT16 i2UIMax, INT16 i2UIDft, INT16 i2UICur);

#if CONFIG_DRV_ALTHD_SUPPORT
extern void vPostChgRes(PMX_RESOLUTION_MODE_T e_res, BOOL fgALTHD);
#else
extern void vPostChgRes(PMX_RESOLUTION_MODE_T e_res);
#endif
extern void vPostSetMode(UINT8 bOnOff);
extern void vPostChgInputCS(DRV_PMX_COLOR_SPACE_T e_cs);
extern void vPostRequestVDPSrcRegion(UINT32 u4SrcWidth, UINT32 u4OutputWidth);

#endif // #define _DRV_POST_POST_H_

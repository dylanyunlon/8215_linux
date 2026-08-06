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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */
#ifndef _PP_HAL_H_
#define _PP_HAL_H_

#include "pp_drv.h"

typedef enum
{
    //1.TDSHARP
	#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
	    QUALITY_TDSHARP_GAIN1 = 0,
	    QUALITY_TDSHARP_GAIN4,
	    QUALITY_TDSHARP_GAIN5,
	    QUALITY_TDSHARP_GAIN6,
	    QUALITY_TDSHARP_GAIN8,
	    QUALITY_TDSHARP_GAIN9,
	    
	    QUALITY_TDSHARP_CORING1,
	    QUALITY_TDSHARP_CORING4,
	    QUALITY_TDSHARP_CORING5,
	    QUALITY_TDSHARP_CORING6,
	    QUALITY_TDSHARP_CORING8,
	    QUALITY_TDSHARP_CORING9,
	    
	    QUALITY_TDSHARP_LIMIT_POS_ALL,
	    QUALITY_TDSHARP_LIMIT_NEG_ALL,
	    
	    QUALITY_TDSHARP_LIMIT_POS1,
	    QUALITY_TDSHARP_LIMIT_POS4,
	    QUALITY_TDSHARP_LIMIT_POS5,
	    QUALITY_TDSHARP_LIMIT_POS6,
	    QUALITY_TDSHARP_LIMIT_POS8,
	    QUALITY_TDSHARP_LIMIT_POS9,
	    
	    QUALITY_TDSHARP_LIMIT_NEG1,
	    QUALITY_TDSHARP_LIMIT_NEG4,
	    QUALITY_TDSHARP_LIMIT_NEG5,
	    QUALITY_TDSHARP_LIMIT_NEG6,
	    QUALITY_TDSHARP_LIMIT_NEG8,
	    QUALITY_TDSHARP_LIMIT_NEG9,
	    
	    QUALITY_TDSHARP_CLIP_EN1,
	    QUALITY_TDSHARP_CLIP_EN4,
	    QUALITY_TDSHARP_CLIP_EN5,
	    QUALITY_TDSHARP_CLIP_EN6,
	    QUALITY_TDSHARP_CLIP_EN8,
	    QUALITY_TDSHARP_CLIP_EN9,
	    
	    QUALITY_TDSHARP_CLIP_THPOS1,
	    QUALITY_TDSHARP_CLIP_THNEG1,
	    QUALITY_TDSHARP_CLIP_THPOS4,
	    QUALITY_TDSHARP_CLIP_THNEG4,
	    QUALITY_TDSHARP_CLIP_THPOS5,
	    QUALITY_TDSHARP_CLIP_THNEG5,
	    QUALITY_TDSHARP_CLIP_THPOS6,
	    QUALITY_TDSHARP_CLIP_THNEG6,
	    QUALITY_TDSHARP_CLIP_THPOS8,
	    QUALITY_TDSHARP_CLIP_THNEG8,
	    QUALITY_TDSHARP_CLIP_THPOS9,
	    QUALITY_TDSHARP_CLIP_THNEG9,
	#endif
	
    //3.CTI  
	#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
	    QUALITY_ECTI_FLAT_SEL,
	    QUALITY_ECTI_FLAT_OFST,
	    QUALITY_ECTI_T_SEL,
	    QUALITY_ECTI_ADPT_SEL,
	    QUALITY_ECTI_ADPT_OFST,
	    QUALITY_ECTI_LPF_SEL,
	    QUALITY_ECTI_STB_SEL,
	    QUALITY_ECTI_STB_OFST,
	#endif
	
    QUALITY_MAX
} QUALITY_ITEM_T;

#define QUALITY_TDSHARP_BEGIN      QUALITY_TDSHARP_GAIN1
#define QUALITY_TDSHARP_END        QUALITY_TDSHARP_CLIP_THNEG9

#define QUALITY_CTI_BEGIN            QUALITY_ECTI_FLAT_SEL
#define QUALITY_CTI_END              QUALITY_ECTI_STB_OFST

#define FROM_DFT		0xFFFF

typedef struct
{
    unsigned char bDftQtyMin;
    unsigned char bDftQtyMax;
    unsigned char bDftQtyDft;
    unsigned short wDftQtyRefenence;
    unsigned short wDftQtyItem;
} POST_DFT_QTY;

typedef enum
{
    SHN_BAND_H1 = 1,
    SHN_BAND_V  = 2,
    SHN_BAND_X1 = 3,
    SHN_BAND_X2 = 4,
    SHN_BAND_H2 = 5,
    SHN_BAND_H3 = 6,
} POST_SHN_BAND_T;

typedef struct
{
    POST_SHN_BAND_T eShnBand;
    unsigned char bGain;
    unsigned char bCoring;
    unsigned char bLimitPos;
    unsigned char bLimitNeg;
    unsigned char bClipEn;
    unsigned char bClipThPos;
    unsigned char bClipThNeg;

    unsigned char bSoftCoreGain;    
} POST_SHN_BAND_PARA;

typedef struct
{
    unsigned short bLimitAllPos;
    unsigned short bLimitAllNeg;
    unsigned char  fgShnEn;    
} POST_SHN_CTRL_PARA;

typedef struct
{
    unsigned short bECTIStbSel;
    unsigned char  bECTIFlpfSel; 
    unsigned char  fgCtiEn;
} POST_CTI_CTRL_PARA;

void _getPPaddr(unsigned char VideoPath);
void _PostFmt(unsigned char VideoPath, unsigned int PicSize);
int _PostSharpness (unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur);
int _PostCTI (unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur);
void vDrvPostSharpParaSet(POST_SHN_BAND_PARA eBandPara);
void vDrvPostSharpCtrlSet(POST_SHN_CTRL_PARA eCtrlPara);
void eDrvCTIRParamSet(POST_CTI_CTRL_PARA eRetPara);
int i4PostVideoProc(POST_UI_ITEM_T e_UI_Item, int i2UIMin, int i2UIMax, int i2UIDft, int i2UICur);
void _PpInit(unsigned char VideoPath, PP_DISPLAY_MODE_E eDisplayMode);

#endif

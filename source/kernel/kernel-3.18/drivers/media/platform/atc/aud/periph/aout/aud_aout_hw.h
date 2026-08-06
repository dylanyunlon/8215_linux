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

/**
 * @file aud_aout_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_AOUT_HW_H
#define _AUD_AOUT_HW_H

     
#ifdef __cplusplus
    extern "C"
    {
#endif


/**********************************************************************************
*
*   macros
*
**********************************************************************************/




/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{
    void (*InitCfg)(void * pThis, void * prCfg);
    void (*Enable)(void * pThis, bool fgEnable);
    void (*SetNsadr)(void * pThis, u32 u4ChNum, u32 u4Nsadr, u32 u4ChSize);
    void (*SetIntClrBit)(void * pThis, bool fgOn);

    void (*SetDacSrc)(void * pThis, AUD_OUT_PATH_T eAOutPath, AUD_DAC_TYPE_T eDacType);
    void (*SetDataSrc)(void * pThis, AUD_DAC_TYPE_T eDacType, bool fgAdcBypasMode);

    void (*SetArmCtrl)(void * pThis, bool fgEn);
    bool (*IsArmCtrl)(void * pThis);
    
    u32 (*Delete)(void * pThis);

}AOUT_HW_CLS_PUB, *PAOUT_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PAOUT_HW_CLS_PUB AoutHw_New(AUD_AOUT_DEVID eAoutId);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_AOUT_HW_H


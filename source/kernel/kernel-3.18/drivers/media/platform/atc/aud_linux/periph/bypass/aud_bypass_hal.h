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
 * @file aud_bypass_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_BYPASS_HAL_H
#define _AUD_BYPASS_HAL_H

#include "aud_bypass_hal_if.h"
#include "aud_bypass_hw.h"
#include "aud_adc_hal_if.h"


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
    //config from setup caller
    BYPS_EXTPARAMS_T rExtCfg;
    
    AUD_DATA_BUF_T rBuf;

    AUD_ADC_ID eAdcId;
    ADC_EXTPARAMS_T rAdcExtCfg;
}AUD_BYPS_CFG_T, *PAUD_BYPS_CFG_T;


typedef struct 
{
    BYPS_HAL_CLS_PUB rPub; 
    
    AUD_BYPS_CFG_T rCfg;
    
    u32 u4State;
    
    PADC_HAL_CLS_PUB prAdc;
    PBYPS_HW_CLS_PUB prBypsHw;
}BYPS_HAL_CLS, * PBYPS_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_BYPASS_HAL_H


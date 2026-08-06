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
 * @file aud_adc_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_ADC_HAL_H
#define _AUD_ADC_HAL_H

#include "aud_adc_hal_if.h"
#include "aud_adc_hw.h"
     
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
    ADC_HAL_CLS_PUB rPub; 
    
    ADC_EXTPARAMS_T rCfg;

    u32 u4UserNum;
    
    PADC_HW_CLS_PUB prAdcHw;
}ADC_HAL_CLS, * PADC_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_ADC_HAL_H


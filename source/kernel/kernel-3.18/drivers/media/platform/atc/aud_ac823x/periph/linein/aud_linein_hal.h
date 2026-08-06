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
 * @file aud_linein_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_LINEIN_HAL_H
#define _AUD_LINEIN_HAL_H

#include "aud_linein_hal_if.h"
#include "aud_linein_hw.h"
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
    
#define LIN_BUF_ALIGN           256


/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{
    //config from setup caller
    LIN_EXTPARAMS_T rExtCfg;
    
    //config from regkey or default
    LIN_FMT_SETTING_T rFmt; //for internal line in config

    AUD_LSBUF_GAIN eGain;

    AUD_DATA_BUF_T rBuf;

    AUD_ADC_ID eAdcId;
    ADC_EXTPARAMS_T rAdcExtCfg;
}AUD_LIN_CFG_T, *PAUD_LIN_CFG_T;


typedef struct 
{
    LIN_HAL_CLS_PUB rPub; 

    AUD_LIN_DEVID eLinId;
    
    AUD_LIN_CFG_T rCfg;
    
    u32 u4State;

    PADC_HAL_CLS_PUB prAdc;
    PLIN_HW_CLS_PUB prLinHw;
}LIN_HAL_CLS, * PLIN_HAL_CLS;


#ifdef __cplusplus
}
#endif
            
            
#endif // _AUD_LINEIN_HAL_H

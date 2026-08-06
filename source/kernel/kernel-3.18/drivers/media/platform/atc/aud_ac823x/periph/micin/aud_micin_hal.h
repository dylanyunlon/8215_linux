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
 * @file aud_micin_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_MICIN_HAL_H
#define _AUD_MICIN_HAL_H

#include "aud_micin_hal_if.h"
#include "aud_micin_hw.h"
#include "aud_adc_hal_if.h"
#include "aud_io_pinmux_if.h"

#ifdef __cplusplus
     extern "C"
    {
#endif

/**********************************************************************************
*
*   macros
*
**********************************************************************************/
#define MIC_BUF_ALIGN          256


/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{
    //config from setup caller
    MIC_EXTPARAMS_T rExtCfg;
    
    //config from regkey or default
    MCLK_TYPE_T eMclkType;
    AUD_LRCK_CYC_T eCycle;
    AUD_MIC_CLK_SRC eClkSrc; //mic in clock config
    
    AUDFMT_INTF_E eDataFmt;

    ADC_EXTPARAMS_T rAdcExtCfg;
    AUD_ADC_ID eAdcId;

    bool fgInvertBck;
    bool fgInvertLrck;

    AUD_DATA_BUF_T rBuf;
}AUD_MIC_CFG_T, *PAUD_MIC_CFG_T;


typedef struct 
{
    MIC_HAL_CLS_PUB rPub; 
    
    AUD_MIC_CFG_T rCfg;
    
    u32 u4State;

    PADC_HAL_CLS_PUB prAdc;
    PMIC_HW_CLS_PUB prMicHw;
}MIC_HAL_CLS, * PMIC_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_MICIN_HAL_H


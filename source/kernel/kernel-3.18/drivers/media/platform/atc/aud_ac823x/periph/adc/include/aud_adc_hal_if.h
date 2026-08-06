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
 * @file aud_adc_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_ADC_HAL_IF_H
#define _AUD_ADC_HAL_IF_H

#include "aud_if_comm.h"
#include "aud_if_hw.h"
#include "aud_define.h"
     
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
    AUDIO_SAMPLING_T eFs;     //sample rate
    AUD_ADC_INPUT_SRC eInput; //input type
    AUD_AFE_CLK_SEL_E eClkSrc; //clock source
    AUD_LSBUF_GAIN eLinGain; //line in gain config
    u32 u4MicGain; // 0~63: -14db~49db(1db/step)
}ADC_EXTPARAMS_T, *PADC_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    AUD_ADC_ID eAdcId;  //return to up layer, if one adc been allocated

    u32 u4State;

    u32 (*Delete)(void * pThis);
    
}ADC_HAL_CLS_PUB, *PADC_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4AdcLog;

#define ADCLOG_ERR(exp)                 AUDLOG(_u4AdcLog & ALOG_ERR,  (T("<***AUD_ADC_ERR***>")), exp)
#define ADCLOG_WARN(exp)                AUDLOG(_u4AdcLog & ALOG_WARN, (T("<AUD_ADC_WARN>")), exp)
#define ADCLOG_INFO(exp)                AUDLOG(_u4AdcLog & ALOG_INFO, (T("[AUD_ADC]")), exp)
#define ADCLOG_CLI(exp)                 AUDLOG(_u4AdcLog & ALOG_CLI,  (T("[AUD_ADC_CLI]")), exp)
#define ADCLOG_DBG(exp)                 AUDLOG(_u4AdcLog & ALOG_DBG,  (T("[AUD_ADC]")), exp)

#define ADCLOG_TEST(exp)                AUDLOG(_u4AdcLog & ALOG_TEST, (T("[AUD_ADC_TEST]")), exp)

#define ADCLOG_ERR_DBG(err, exp)    \
    if (err){                       \
        ADCLOG_ERR(exp)             \
    } else { \
        ADCLOG_DBG(exp)             \
    }



/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PADC_HAL_CLS_PUB AdcHal_New(PADC_EXTPARAMS_T prAdcExtCfg);
extern void AdcHal_SetInputPinGpioFun(AUD_LIN_PIN_IDX ePinIdx, bool fgGpiFunEn);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_ADC_HAL_IF_H
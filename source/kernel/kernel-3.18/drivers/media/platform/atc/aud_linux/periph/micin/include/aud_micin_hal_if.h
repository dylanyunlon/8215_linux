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
 * @file aud_micin_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_MICIN_HAL_IF_H
#define _AUD_MICIN_HAL_IF_H

#include "aud_if_comm.h"
#include "aud_if_hw.h"
#include "aud_define.h"
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


/**********************************************************************************
*
*   data type
*
**********************************************************************************/

typedef struct
{
    AUD_MIC_SRC eSrc;  //mic in source : internal or external adc
    AUD_PINMUX_I2SMICIN eI2sPin;  //i2s mic in pin select

    AUDIO_SAMPLING_T eFs; //mic in sample rate

    u32 u4MicGain; // 0~63: -14db~49db(1db/step)
    
    u32 u4SrcBitNum; //mic in source bit number
    AUD_LIN_OUT_BITNUM eOutBitNum; //mic in out data bit number

    u32 u4BufPhyAdr; //if mic in buffer allocate by up layer,set this value; allocate by hal layer, set to '0' 
    u32 u4BufSz; 
}MIC_EXTPARAMS_T, *PMIC_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    u32 (*Delete)(void * pThis);
    
}MIC_HAL_CLS_PUB, *PMIC_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4MicLog;

#define MICLOG_ERR(exp, ...)                AUDLOG(_u4MicLog & ALOG_ERR,  T("[AUD][MIC]"), exp, ##__VA_ARGS__)
#define MICLOG_WARN(exp, ...)               AUDLOG(_u4MicLog & ALOG_WARN, T("[AUD][MIC]"), exp, ##__VA_ARGS__)
#define MICLOG_INFO(exp, ...)               AUDLOG(_u4MicLog & ALOG_INFO, T("[AUD][MIC]"), exp, ##__VA_ARGS__)
#define MICLOG_CLI(exp, ...)                AUDLOG(_u4MicLog & ALOG_CLI,  T("[AUD][MIC][CLI]"), exp, ##__VA_ARGS__)
#define MICLOG_DBG(exp, ...)                AUDLOG(_u4MicLog & ALOG_DBG,  T("[AUD][MIC]"), exp, ##__VA_ARGS__)

#define MICLOG_TEST(exp, ...)               AUDLOG(_u4MicLog & ALOG_TEST, T("[AUD][MIC][TEST]"), exp, ##__VA_ARGS__)

#define MICLOG_ERR_DBG(err, exp, ...)    \
    if (err){                       \
        MICLOG_ERR(exp, ##__VA_ARGS__);             \
    } else {                        \
        MICLOG_DBG(exp, ##__VA_ARGS__);             \
    }



/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PMIC_HAL_CLS_PUB MicHal_New(void);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_MICIN_HAL_IF_H

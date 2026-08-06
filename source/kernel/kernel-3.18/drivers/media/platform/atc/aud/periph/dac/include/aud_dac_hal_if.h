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
 * @file aud_dac_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_DAC_HAL_IF_H
#define _AUD_DAC_HAL_IF_H

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
    AUD_OUT_PATH_T eOutPath;  //to front or rear seat
    AUD_DAC_TYPE_T eDacType;  //internal pwm or ext dac
    
    AUD_PINMUX_FS_I2SOUT ePinMuxFsExtDac;  //pin mux config for front I2S out(no need set this if pwm dac select)
    AUD_PINMUX_RS_I2SOUT ePinMuxRsExtDac;  //pin mux config for rear  I2S out(no need set this if pwm dac select)

    AUD_CKGEN_APLL eApll;  //clock domain select for pwm dac
}DAC_EXTPARAMS_T, *PDAC_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    u32 u4State;

    u32 (*Delete)(void * pThis);
    
}DAC_HAL_CLS_PUB, *PDAC_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4DacLog;

#define DACLOG_ERR(exp, ...)                 AUDLOG(_u4DacLog & ALOG_ERR,  T("[AUD][DAC]"), exp, ##__VA_ARGS__)
#define DACLOG_WARN(exp, ...)                AUDLOG(_u4DacLog & ALOG_WARN, T("[AUD][DAC]"), exp, ##__VA_ARGS__)
#define DACLOG_INFO(exp, ...)                AUDLOG(_u4DacLog & ALOG_INFO, T("[AUD][DAC]"), exp, ##__VA_ARGS__)
#define DACLOG_CLI(exp, ...)                 AUDLOG(_u4DacLog & ALOG_CLI,  T("[AUD][DAC][CLI]"), exp, ##__VA_ARGS__)
#define DACLOG_DBG(exp, ...)                 AUDLOG(_u4DacLog & ALOG_DBG,  T("[AUD][DAC]"), exp, ##__VA_ARGS__)

#define DACLOG_TEST(exp, ...)                AUDLOG(_u4DacLog & ALOG_TEST, T("[AUD][DAC][TEST]"), exp, ##__VA_ARGS__)

#define DACLOG_ERR_DBG(err, exp, ...)    \
    if (err){                       \
        DACLOG_ERR(exp, ##__VA_ARGS__);             \
    } else {                        \
        DACLOG_DBG(exp, ##__VA_ARGS__);            \
    }



/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PDAC_HAL_CLS_PUB DacHal_New(PDAC_EXTPARAMS_T prDacExtCfg);
extern void DacHal_SetPwmAnaGpioFun(AUD_PWM_DAC_ID ePwmChId, bool fgEn);
extern void DacHal_SetPwmBasicSetting(void);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_DAC_HAL_IF_H

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
 * @file aud_bypass_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */


#ifndef _AUD_BYPASS_HAL_IF_H
#define _AUD_BYPASS_HAL_IF_H

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
    AUD_BYPS_DST eDst; //bypass to aout1 or aout2
    AUD_BYPS_GAIN_MODE eGainMode; //default : BYPS_VOL_LINER
    AUD_ADC_INPUT_SRC eGroup; //internal line group select
    
    u32 u4Gain; //(max value : 0xffffff(0DB))
    u32 u4Scale; //total 4bits, fade in/out order, default : 1 
}BYPS_EXTPARAMS_T, *PBYPS_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    u32 (*Delete)(void * pThis);
    
}BYPS_HAL_CLS_PUB, *PBYPS_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4BypsLog;

#define BYPSLOG_ERR(exp)                AUDLOG(_u4BypsLog & ALOG_ERR,  (T("<***AUD_BYPS_ERR***>")), exp)
#define BYPSLOG_WARN(exp)               AUDLOG(_u4BypsLog & ALOG_WARN, (T("<AUD_BYPS_WARN>")), exp)
#define BYPSLOG_INFO(exp)               AUDLOG(_u4BypsLog & ALOG_INFO, (T("[AUD_BYPS]")), exp)
#define BYPSLOG_CLI(exp)                AUDLOG(_u4BypsLog & ALOG_CLI,  (T("[AUD_BYPS_CLI]")), exp)
#define BYPSLOG_DBG(exp)                AUDLOG(_u4BypsLog & ALOG_DBG,  (T("[AUD_BYPS]")), exp)

#define BYPSLOG_TEST(exp)               AUDLOG(_u4BypsLog & ALOG_TEST, (T("[AUD_BYPS_TEST]")), exp)

#define BYPSLOG_ERR_DBG(err, exp)    \
    if (err){                        \
        BYPSLOG_ERR(exp)             \
    } else {                         \
        BYPSLOG_DBG(exp)             \
    }


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PBYPS_HAL_CLS_PUB BypsHal_New(void);


#ifdef __cplusplus
    }
#endif
  
            
#endif // _AUD_BYPASS_HAL_IF_H
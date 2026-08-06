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
 * @file aud_aout_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_AOUT_HAL_IF_H
#define _AUD_AOUT_HAL_IF_H

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

//external hw s32 config parameters
typedef struct
{
    u32 u4NSNum;         //next sample number
    u32 u4IntrSize;      //interrupt size
    void (*PFN_ISR_CB)(u32 u4Param); //s32 callback function
}AOUT_INT_CFG_T, *PAOUT_INT_CFG_T;

typedef struct
{
    AUD_OUT_PATH_T eOutPath; //to front or rear seat
    AUD_DAC_TYPE_T eDacType;  //internal pwm or ext dac
    
    AUD_PINMUX_FS_I2SOUT ePinMuxFsExtDac;  //pin mux config for front I2S out(no need set this if pwm dac select)
    AUD_PINMUX_RS_I2SOUT ePinMuxRsExtDac;  //pin mux config for rear  I2S out(no need set this if pwm dac select)

    bool fgAdcBypasMode;     //if aout source from adc(1) / from dram (0)

    AUDIO_SAMPLING_T eFs;    //sample rate
    u32 u4Bps;            //bit per sample
                                                   //BIT CONFIG: 8 ~ 11,    12 ~ 15,    16 ~ 19,    20 ~ 23
    u32 u4ChCfg0;         //channel configure0                 FL          FR          C          CH7
    u32 u4ChCfg1;         //channel configure1                 RL          RR         LFE         CH8
    u32 u4ChCfg2;         //channel configure2                 CH9         CH10       CH11        CH12
    u32 u4ChNum;          //channel number

    u32 u4BufPhyAdr; //if allocate by hal layer, set to '0' 
    u32 u4BufSz; 
    
    AOUT_INT_CFG_T rIntCfg;   //Interrupt config related
}AOUT_EXTPARAMS_T, *PAOUT_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    void (*SetArmCtrl)(void * prThis, bool fgEn);
    bool (*IsArmCtrl)(void * prThis);

    void (*SetAoutPath)(void * prThis, AUD_OUT_PATH_T eAOutPath);
    
    void (*IntConfigInit)(void * prThis);
    
    u32 (*Delete)(void * prThis);
    
}AOUT_HAL_CLS_PUB, *PAOUT_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4AoutLog;

#define AOUTLOG_ERR(exp, ...)                AUDLOG(_u4AoutLog & ALOG_ERR,  T("[AUD][AOUT]"), exp, ##__VA_ARGS__)
#define AOUTLOG_WARN(exp, ...)               AUDLOG(_u4AoutLog & ALOG_WARN, T("[AUD][AOUT]"), exp, ##__VA_ARGS__)
#define AOUTLOG_INFO(exp, ...)               AUDLOG(_u4AoutLog & ALOG_INFO, T("[AUD][AOUT]"), exp, ##__VA_ARGS__)
#define AOUTLOG_CLI(exp, ...)                AUDLOG(_u4AoutLog & ALOG_CLI,  T("[AUD][AOUT][CLI]"), exp, ##__VA_ARGS__)
#define AOUTLOG_DBG(exp, ...)                AUDLOG(_u4AoutLog & ALOG_DBG,  T("[AUD][AOUT]"), exp, ##__VA_ARGS__)

#define AOUTLOG_TEST(exp, ...)               AUDLOG(_u4AoutLog & ALOG_TEST, T("[AUD][AOUT][TEST]"), exp, ##__VA_ARGS__)

#define AOUTLOG_ERR_DBG(err, exp, ...)    \
    if (err){                        \
        AOUTLOG_ERR(exp, ##__VA_ARGS__);             \
    } else {                         \
        AOUTLOG_DBG(exp, ##__VA_ARGS__);             \
    }



/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PAOUT_HAL_CLS_PUB AoutHal_New(AUD_AOUT_DEVID eAoutId);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_AOUT_HAL_IF_H

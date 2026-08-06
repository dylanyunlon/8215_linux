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
 * @file aud_linein_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_LINEIN_HAL_IF_H
#define _AUD_LINEIN_HAL_IF_H

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
    bool fgOn; //on/off interrupt function
    AUD_LIN_INT_PERIOD eIntPeriod;  //s32 generate frequency config
    
    void (*PFN_ISR_CB)(u32 u4Param); //s32 callback function
}LIN_INT_CFG_T, *PLIN_INT_CFG_T;

typedef struct
{
    MCLK_TYPE_T eMclkType;  //mclk type
    AUDIO_SAMPLING_T eFs;  //sample rate
    AUD_LRCK_CYC_T eCycle;  
    AUDFMT_INTF_E eDataFmt;
    
    u32 u4SrcBitNum;
    AUD_LIN_OUT_BITNUM eOutBitNum;

    bool fgInvertBck;
    bool fgInvertLrck;
}LIN_FMT_SETTING_T, *PLIN_FMT_SETTING_T;

typedef struct
{
    AUD_CLK_MODE eClkMode; //IIS IN clock mode : master or slave
    LIN_FMT_SETTING_T rFmt;
    AUD_PINMUX_I2SLIN0 eGrpPin0; //IIS IN0 Group pin sel
    AUD_PINMUX_I2SLIN1 eGrpPin1; //IIS IN1 Group pin sel
}IIS_LIN_CFG_T, *PIIS_LIN_CFG_T;

typedef struct
{
    AUD_LIN_SRC eSrc;  //internal or IIS line in
    AUD_LIN_CLK_SRC eIntClkSrc; //internal clock config
    AUD_ADC_INPUT_SRC eGroup; //internal line group select
    
    u32 u4BufPhyAdr; //if allocate by hal layer, set to '0' 
    u32 u4BufSz; //Buf size for line in hw

    LIN_INT_CFG_T rIntCfg;//interrupt config
    IIS_LIN_CFG_T rI2sLinCfg; //i2s line in config
}LIN_EXTPARAMS_T, *PLIN_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    u32 (*Delete)(void * pThis);
    
}LIN_HAL_CLS_PUB, *PLIN_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4LinLog;

#define LINLOG_ERR(exp, ...)                AUDLOG(_u4LinLog & ALOG_ERR,  T("[AUD][LIN]"), exp, ##__VA_ARGS__)
#define LINLOG_WARN(exp, ...)               AUDLOG(_u4LinLog & ALOG_WARN, T("[AUD][LIN]"), exp, ##__VA_ARGS__)
#define LINLOG_INFO(exp, ...)               AUDLOG(_u4LinLog & ALOG_INFO, T("[AUD][LIN]"), exp, ##__VA_ARGS__)
#define LINLOG_CLI(exp, ...)                AUDLOG(_u4LinLog & ALOG_CLI,  T("[AUD][LIN][CLI]"), exp, ##__VA_ARGS__)
#define LINLOG_DBG(exp, ...)                AUDLOG(_u4LinLog & ALOG_DBG,  T("[AUD][LIN]"), exp, ##__VA_ARGS__)

#define LINLOG_TEST(exp, ...)               AUDLOG(_u4LinLog & ALOG_TEST, T("[AUD][LIN][TEST]"), exp, ##__VA_ARGS__)

#define LINLOG_ERR_DBG(err, exp, ...)    \
    if (err){                       \
        LINLOG_ERR(exp, ##__VA_ARGS__);             \
    } else {                        \
        LINLOG_DBG(exp, ##__VA_ARGS__);             \
    }


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PLIN_HAL_CLS_PUB LinHal_New(AUD_LIN_DEVID eLinId);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_LINEIN_HAL_IF_H
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
 * @file aud_mlin_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */


 
#ifndef _AUD_MLINE_HAL_IF_H
#define _AUD_MLINE_HAL_IF_H

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
   u32 DETAIL    : 05; //BIT[0 : 4] ->Detail type for IEC61937 RAW data
   u32 BSNUM     : 03; //BIT[5 : 7] ->Bit stream number for IEC61937 RAW data
   u32 ROUGH     : 02; //BIT[8 : 9] ->0: PCM. 1: RAW (encoded) data. Detail types will be in bits 4~0.
                                     //->2: The type is 16-bit DTS-CD., 3: The type is 14-bit DTS-CD.
   u32 DEC       : 01; //BIT[10]    -> SPDIF bit stream type decided or not. 
   u32 REV0021   : 21; //BIT[11:31]//reserved
}MLIN_SPDIF_INFO_T, *PMLIN_SPDIF_INFO_T;

typedef struct
{
    u32 u4SrcBitNum; //mlin source data bit number
    AUD_LIN_OUT_BITNUM eOutBitNum; //mlin out data bit number
    AUDFMT_INTF_E eDataFmt;
    AUD_MLIN_CH_NUM_E eMlinChNum;

    AUD_LRCK_CYC_T eCycle; //bck/lrck

    AUD_MLIN_SRC eSrc;

    u32 u4BufPhyAdr; //set '0', if need mlne hal layer to allocate memory
    u32 u4BufSz; 

    AUD_MLIN_INT_PERIOD eIntPeriod;
    void (*PFN_ISR_CB)(u32 u4Param); //s32 callback function
}MLIN_EXTPARAMS_T, *PMLIN_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rHwIf;

    u32 (*SetIRQOnOff)(void * pThis, x_os_isr_fct pfnIsr, bool fgOnOff);
    u32 (*GetSpdifType)(void * pThis, void * pSpdifInfo);
    u32 (*ClrSpdTypeDec)(void * pThis, u8 u1Val); 
    u32 (*SetSrcType)(void * prThis, AUD_MLIN_SRC eSrc);
    u32 (*Delete)(void * pThis);
    
}MLIN_HAL_CLS_PUB, *PMLIN_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4MlinLog;

#define MLINLOG_ERR(exp, ...)                AUDLOG(_u4MlinLog & ALOG_ERR,  T("[AUD][MLIN]"), exp, ##__VA_ARGS__)
#define MLINLOG_WARN(exp, ...)               AUDLOG(_u4MlinLog & ALOG_WARN, T("[AUD][MLIN]"), exp, ##__VA_ARGS__)
#define MLINLOG_INFO(exp, ...)               AUDLOG(_u4MlinLog & ALOG_INFO, T("[AUD][MLIN]"), exp, ##__VA_ARGS__)
#define MLINLOG_CLI(exp, ...)                AUDLOG(_u4MlinLog & ALOG_CLI,  T("[AUD][MLIN][CLI]"), exp, ##__VA_ARGS__)
#define MLINLOG_DBG(exp, ...)                AUDLOG(_u4MlinLog & ALOG_DBG,  T("[AUD][MLIN]"), exp, ##__VA_ARGS__)

#define MLINLOG_TEST(exp, ...)               AUDLOG(_u4MlinLog & ALOG_TEST, T("[AUD][MLIN][TEST]"), exp, ##__VA_ARGS__)

#define MLINLOG_ERR_DBG(err, exp, ...)    \
    if (err){                        \
        MLINLOG_ERR(exp, ##__VA_ARGS__);             \
    } else {                         \
        MLINLOG_DBG(exp, ##__VA_ARGS__);             \
    }


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PMLIN_HAL_CLS_PUB MlinHal_New(void);


#ifdef __cplusplus
    }
#endif
  
            
#endif // _AUD_MLINE_HAL_IF_H
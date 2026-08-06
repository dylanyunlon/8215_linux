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
 * @file aud_pcm_hal_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_PCM_HAL_IF_H
#define _AUD_PCM_HAL_IF_H

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

//external hw s32 config parameters
typedef struct
{
    u32 u4IntSz;           //next sample number
    u32 u4IntBurstTime;    //remain sample number
    void (*PFN_ISR_CB)(u32 u4Param); //s32 callback function
}PCM_INT_CFG_T, *PPCM_INT_CFG_T;

//external hw config parameters
typedef struct
{
    AUD_PCM_HW_MODE eHwMode;  //default : PCM_NORMAL_MODE
    AUD_PCM_SYNC_MODE eSyncMode;  //default : PCM_LONG_MODE
    AUD_PCM_SYNC_CYCLE eSyncCycle; //default : PCM_CLK_CYCLE_32
    u32 u4SampleRate;

    PCM_INT_CFG_T rIntCfg; //pcm tx interrupt config

    uintptr_t u4BufPhyAdr; //set to '0'if allocate by hal layer
    u32 u4RxBufSz;
    u32 u4TxBufSz;
}PCM_EXTPARAMS_T, *PPCM_EXTPARAMS_T;

typedef struct
{
    AUD_HW_IF_T rRxHwIf;  //pcm rx interface
    AUD_HW_IF_T rTxHwIf;  //pcm tx interface

    ////pcm hw interface
    u32 (*PcmCtrl)(void * pThis, bool fgEnable);
    u32 (*PcmStatus)(void * pThis);
    u32 (*PcmCfgUpd)(void * pThis, void * pvParams);
    
    u32 (*Delete)(void * pThis);
    
}PCM_HAL_CLS_PUB, *PPCM_HAL_CLS_PUB;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
extern u32 _u4PcmLog;

#define PCMLOG_ERR(exp)                AUDLOG(_u4PcmLog & ALOG_ERR,  (T("<***AUD_PCM_ERR***>")), exp)
#define PCMLOG_WARN(exp)               AUDLOG(_u4PcmLog & ALOG_WARN, (T("<AUD_PCM_WARN>")), exp)
#define PCMLOG_INFO(exp)               AUDLOG(_u4PcmLog & ALOG_INFO, (T("[AUD_PCM]")), exp)
#define PCMLOG_CLI(exp)                AUDLOG(_u4PcmLog & ALOG_CLI,  (T("[AUD_PCM_CLI]")), exp)
#define PCMLOG_DBG(exp)                AUDLOG(_u4PcmLog & ALOG_DBG,  (T("[AUD_PCM]")), exp)

#define PCMLOG_TEST(exp)               AUDLOG(_u4PcmLog & ALOG_TEST, (T("[AUD_PCM_TEST]")), exp)

#define PCMLOG_ERR_DBG(err, exp)    \
    if (err){                       \
        PCMLOG_ERR(exp)             \
    } else {                        \
        PCMLOG_DBG(exp)             \
    }


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern PPCM_HAL_CLS_PUB PcmHal_New(void);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_PCM_HAL_IF_H
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
 * @file aud_pcm_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_PCM_HAL_H
#define _AUD_PCM_HAL_H

#include "aud_drv.h"
#include "aud_pcm_hal_if.h"
#include "aud_pcm_hw.h"
     
#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************************
*
*   macros
*
**********************************************************************************/

#define AC83XX_BT_PCM_VA    (AC83XX_PCM_VA + 300 * 1024)
#define AC83XX_BT_PCM_PA    (AC83XX_PCM_PA + 300 * 1024)


#define PCM_BUF_ALIGN          16

//pcm control include three parts : PCM(mater switch), RX, TX
//please use below macros for start & stop public interface parameters
#define PCM_CTL_BIT_ON      (1 << 0)  //bit0
#define PCM_CTL_BIT_RX      (1 << 1)  //bit1
#define PCM_CTL_BIT_TX      (1 << 2)  //bit2


/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{
    // 'bank' means : block of data that tx hw burst one time 
    u32 u4BankSz; // data size the tx hw burst one time
    u32 u4BankNum;  // tx buf size / u4BankSz

    u32 u4OBankNum; //bank number that tx hw has output
    u32 u4IBankNum; //bank number that hal layer has input

    u32 u4Rp; //tx buffer read pointer, update by hal layer
    u32 u4Wp; //tx buffer write pointer, update by hal layer caller 
    
    u32 u4UnderRunCnt; //counter for 'buffer empty' case
}AUD_PCM_INT_CTL_T, *PAUD_PCM_INT_CTL_T;

typedef struct
{
    //config from setup caller
    PCM_EXTPARAMS_T rExtCfg;
    
    //config from regkey or default
    AUD_PCM_DATA_ORDER eDataOrder;
    AUD_PCM_BIT_NUM eBitNum;
    AUD_PCM_BIT_MODE eBitMode;
    AUD_PCM_SYNC_LENGTH eSyncLength;
    AUD_PCM_MODE eMode;
    bool fgSignEn;
   
    bool fgInvertClkOut;
    bool fgInvertClkIn;

    AUD_DATA_BUF_T rTxBuf;
    AUD_DATA_BUF_T rRxBuf;
}AUD_PCM_CFG_T, *PAUD_PCM_CFG_T;


typedef struct 
{
    PCM_HAL_CLS_PUB rPub; 
    
    AUD_PCM_CFG_T rCfg;

    AUD_PCM_INT_CTL_T rTxIntCtl;
    
    u32 u4PcmState;
    u32 u4TxState;
    u32 u4RxState;
    
    PPCM_HW_CLS_PUB prPcmHw;
}PCM_HAL_CLS, * PPCM_HAL_CLS;


#ifdef __cplusplus
}
#endif
            
            
#endif // _AUD_PCM_HAL_H


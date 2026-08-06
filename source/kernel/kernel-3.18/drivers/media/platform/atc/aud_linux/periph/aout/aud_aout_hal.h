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
 * @file aud_aout_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_AOUT_HAL_H
#define _AUD_AOUT_HAL_H

#include "aud_aout_hal_if.h"
#include "aud_aout_hw.h"
#include "aud_dac_hal_if.h"


     
#ifdef __cplusplus
     extern "C"
    {
#endif


/**********************************************************************************
*
*   macros
*
**********************************************************************************/
//#define AOUT_ARM_CTL

//#define DAC_POWER_CTL

    
#define AOUT_BUF_ALIGN          4

#define AOUT1_MAX_CHNUM         12
#define AOUT2_MAX_CHNUM         8

#define AOUT1_VECTOR            28 //VECTOR_DSPC
#define AOUT2_VECTOR            120 //VECTOR_AOUT_2ND_RC



/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{   
    // 'bank' means : block of data that aout hw burst one time 
    u32 u4BankSz; // data size the aout hw burst one time
    u32 u4BankNum;  // aout buf size / u4BankSz

    u32 u4OBankNum; //bank number that aout buffer has output
    u32 u4IBankNum; //bank number that aout buffer has input

    u32 u4Rp; //aout buffer read pointer, update by hal layer
    u32 u4Wp; //aout buffer write pointer, update by hal layer caller 
    
    u32 u4UnderRunCnt; //counter for 'buffer empty' case
}AUD_AOUT_INT_CTL_T, *PAUD_AOUT_INT_CTL_T;

typedef struct
{
    //config from setup caller
    AOUT_EXTPARAMS_T rExtCfg;
    DAC_EXTPARAMS_T rDacExtCfg;

    //output to front seat or rear seat
    //set when new aout hal class is creat
    AUD_AOUT_DEVID eAoutId;  
    
    //config from regkey or default
    MCLK_TYPE_T eMclkType;
    AUD_LRCK_CYC_T eCycle;
    AUDFMT_INTF_E eFmt;

    bool fgInvertBck;
    bool fgInvertLrck;

    u32 u4DacBitNum;

    AUD_DATA_BUF_T rBuf;
}AUD_AOUT_CFG_T, *PAUD_AOUT_CFG_T;

typedef struct 
{
    AOUT_HAL_CLS_PUB rPub;
    
    AUD_AOUT_CFG_T rAoutCfg;
   
    AUD_AOUT_DEVID eAoutId;
    AUD_AOUT_INT_CTL_T rIntCtl;
    
    u32 u4State;
    
    PAOUT_HW_CLS_PUB prAoutHw;
    PDAC_HAL_CLS_PUB prDacHw;
}AOUT_HAL_CLS, * PAOUT_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_AOUT_HAL_H


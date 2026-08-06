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
 * @file aud_io_test.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_IO_TEST_H
#define _AUD_IO_TEST_H

#include "aud_micin_hal_if.h"
#include "aud_aout_hal_if.h"


#ifdef __cplusplus
    extern "C"
    {
#endif

/****************** MIC TO AOUT ********************/

#define AOUT_INT_SAMPLE_NUM         0xF0
#define AOUT_INT_BANK_NUM           4

typedef struct
{
    PMIC_HAL_CLS_PUB prMic;
    PAOUT_HAL_CLS_PUB prAout;

    AUD_DATA_BUF_T rMicBuf;
    AUD_DATA_BUF_T rAoutBuf;
    
    u32 u4MicRP;
    u32 u4AoutWP;

    void* hSema;
    void* hThread;
    bool fgThreadEn;
    
}MIC_TO_AOUT_TEST_T, *PMIC_TO_AOUT_TEST_T;


/**************************************************/

#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_TEST_H
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
 * @file aud_mline_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_MLINE_HAL_H
#define _AUD_MLINE_HAL_H

#include "aud_mline_hal_if.h"
#include "aud_mline_hw.h"

#ifdef __cplusplus
     extern "C"
    {
#endif

/**********************************************************************************
*
*   macros
*
**********************************************************************************/

#define MLIN_BUF_ALIGN          256

#define MLIN_VECTOR             183 //119 //VECTOR_AOUT_GPS_RC (lin_multi_rcint)

//#define MLIN_SIMULATE

#ifdef MLIN_SIMULATE
#define MLINREG_BITS_W(szExp1, szExp2, addr, szExp3, start, bitNum, val)                \
    MLINLOG_INFO(T("[%s] MLIN [%s](0x%x)  -----  [%s] BIT(%d:%d): 0x%x \r\n"),            \
       T(szExp1), T(szExp2), addr, T(szExp3), (start + bitNum - 1), start, val);
#else
#define MLINREG_BITS_W(szExp1, szExp2, addr, szExp3, start, bitNum, val)
#endif



/**********************************************************************************
*
*   data type
*
**********************************************************************************/
typedef struct
{
    //config from setup caller
    MLIN_EXTPARAMS_T rExtCfg;
    
    //config from regkey or default
    
    AUD_MLIN_SRC eSrc;  //multi line in source

    bool fgInvertBck;
    bool fgInvertLrck;

    AUD_DATA_BUF_T rBuf;
}AUD_MLIN_CFG_T, *PAUD_MLIN_CFG_T;


typedef struct 
{
    MLIN_HAL_CLS_PUB rPub; 
    
    AUD_MLIN_CFG_T rCfg;
    
    u32 u4State;
    
    PMLIN_HW_CLS_PUB prMlinHw;
}MLIN_HAL_CLS, * PMLIN_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_MLINE_HAL_H


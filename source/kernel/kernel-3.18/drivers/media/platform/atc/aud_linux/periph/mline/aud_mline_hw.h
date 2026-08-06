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
 * @file aud_mline_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_MLINE_HW_H
#define _AUD_MLINE_HW_H

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
    void (*InitCfg)(void * pThis, void * prCfg);
    void (*Enable)(void * pThis, bool fgEnable);
    u32 (*GetWp)(void * pThis);
    void (*GetSpdifInfo)(void * pThis, void * pSpdifInfo);
    void (*ClrSpdTypeDec)(void * pThis, u8 u1Val);
    
    void (*SetSrcBitNum)(void * prThis, u32 u4BitNum);
    void (*SetOutBitNum)(void * prThis, AUD_LIN_OUT_BITNUM eOutBitNum);    
    void (*SetDataFmt)(void * prThis, AUDFMT_INTF_E eFmt);    
    void (*SetChNum)(void * prThis, AUD_MLIN_CH_NUM_E eChNum);    
    void (*SetIntPeriod)(void * prThis, AUD_MLIN_INT_PERIOD eIntPeriod);     
    void (*SetSrc)(void * prThis, AUD_MLIN_SRC eSrc); 
        
    
    u32 (*Delete)(void * pThis);

}MLIN_HW_CLS_PUB, *PMLIN_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PMLIN_HW_CLS_PUB MlinHw_New(void);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_MLINE_HW_H

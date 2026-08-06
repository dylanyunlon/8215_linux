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
 * @file aud_dac_hal.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_DAC_HAL_H
#define _AUD_DAC_HAL_H

#include "aud_dac_hal_if.h"
#include "aud_dac_hw.h"
     
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
    DAC_HAL_CLS_PUB rPub; 
    
    DAC_EXTPARAMS_T rCfg;
    
    PDAC_HW_CLS_PUB prDacHw;
}DAC_HAL_CLS, * PDAC_HAL_CLS;


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_DAC_HAL_H


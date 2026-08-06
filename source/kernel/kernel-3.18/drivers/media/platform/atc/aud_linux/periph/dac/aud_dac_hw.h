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
 * @file aud_dac_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@auotchips.com
 * 
 */
#ifndef _AUD_DAC_HW_H
#define _AUD_DAC_HW_H

     
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
    void (*Start)(void * prThis, void * prCfg);
    void (*Stop)(void * prThis);
    
    u32 (*Delete)(void * prThis);

}DAC_HW_CLS_PUB, *PDAC_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PDAC_HW_CLS_PUB DacHw_New(AUD_OUT_PATH_T eOutPath, AUD_DAC_TYPE_T eDacType);
void DacHw_SetPwmAnaGpioFun(AUD_PWM_DAC_ID ePwmChId, bool fgEn);
void DacHw_SetPwmAnalogPartBasicPowerOn(void);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_DAC_HW_H


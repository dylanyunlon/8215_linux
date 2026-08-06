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
 * @file aud_adc_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@auotchips.com
 * 
 */
#ifndef _AUD_ADC_HW_H
#define _AUD_ADC_HW_H

     
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
    void (*Start)(void * pThis, void * prCfg);
    void (*Stop)(void * pThis);
    
    u32 (*Delete)(void * pThis);

}ADC_HW_CLS_PUB, *PADC_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PADC_HW_CLS_PUB AdcHw_New(AUD_ADC_ID eAdcId);
void AdcHw_SetInputPinGpioFun(AUD_LIN_PIN_IDX ePinIdx, bool fgGpiFunEn);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_ADC_HW_H


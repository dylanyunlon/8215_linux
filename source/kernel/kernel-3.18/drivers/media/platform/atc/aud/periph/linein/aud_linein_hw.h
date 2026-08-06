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
 * @file aud_linein_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_LINEIN_HW_H
#define _AUD_LINEIN_HW_H

     
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
    void (*SelAfe)(void * pThis, AUD_ADC_ID eAdcId);
    u32 (*GetWp)(void * pThis);
    
    u32 (*Delete)(void * pThis);

}LIN_HW_CLS_PUB, *PLIN_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PLIN_HW_CLS_PUB LinHw_New(AUD_LIN_DEVID eLinId);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_LINEIN_HW_H

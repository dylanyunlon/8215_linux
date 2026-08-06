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
 * @file aud_pcm_hw.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_PCM_HW_H
#define _AUD_PCM_HW_H

     
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
    void (*UpdCfg)(void * pThis, void * prCfg);
    void (*Enable)(void * pThis, u32 u4PcmCtrl, bool fgEnable);
    
    u32 (*GetWp)(void * pThis);
    void (*SetNsadr)(void * pThis, u32 u4Nsadr);
    void (*SetHwMode)(void * pThis, AUD_PCM_HW_MODE eHwMode);
    
    u32 (*Delete)(void * pThis);
}PCM_HW_CLS_PUB, *PPCM_HW_CLS_PUB;


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
PPCM_HW_CLS_PUB PcmHw_New(void);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_PCM_HW_H


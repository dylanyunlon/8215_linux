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
 * @file aud_io_clock_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_IO_CLOCK_IF_H
#define _AUD_IO_CLOCK_IF_H

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
typedef enum
{
    CLKPM_MPHONE,
    CLKPM_MLIN,
    CLKPM_MLIN2,
    CLKPM_IEC,
    CLKPM_AUD,
    CLKPM_AUD2,
    CLKPM_APLL_ADJ,
    CLKPM_APLL2_ADJ,
    CLKPM_STC_RISC,
    CLKPM_AXI,
    CLKPM_AP_ASRC,
    CLKPM_GPS_ASRC,
    CLKPM_AFE1_26M,
    CLKPM_AFE2_26M,
    CLKPM_FS_APLL,
    CLKPM_RS_APLL,
    CLKPM_MAX,
}AUD_CLK_POWER_CTL_MODULE_ID;

typedef enum
{
    ASRC_CLI_SIG0,
    ASRC_CLI_SIG1,
    ASRC_CLI_SIG2,
    ASRC_CLI_SIG3,
    ASRC_CLI_SIG4,
    ASRC_CLI_MAX
}AUD_ASRC_CLI_SIG_SEL;

typedef enum
{
    ASRC_CLI_SIG0_LIN1,
    ASRC_CLI_SIG0_MPHONE,
    ASRC_CLI_SIG0_AUDOUT,
}AUD_ASRC_CLI_SIG0_SRC;

typedef enum
{    
    ASRC_CLI_SIG1_LIN2,
    ASRC_CLI_SIG1_LIN1,
    ASRC_CLI_SIG1_MLIN,
}AUD_ASRC_CLI_SIG1_SRC;

typedef enum
{
    ASRC_CLI_SIG2_IEC,
    ASRC_CLI_SIG2_APLL_8K_LRCK,  // useless
    ASRC_CLI_SIG2_AUDOUT2,
}AUD_ASRC_CLI_SIG2_SRC;

typedef enum
{
    ASRC_CLI_SIG3_AUDOUT2,
    ASRC_CLI_SIG3_APLL_8K_LRCK,  // useless
    ASRC_CLI_SIG3_IEC,
}AUD_ASRC_CLI_SIG3_SRC;

typedef enum
{
    ASRC_CLI_SIG4_MLIN,
    ASRC_CLI_SIG4_LIN1,
    ASRC_CLI_SIG4_LIN2,
}AUD_ASRC_CLI_SIG4_SRC;



/**********************************************************************************
*
*   data type
*
**********************************************************************************/



/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/
extern AUD_CKGEN_APLL IoClk_GetApllType(AUDIO_SAMPLING_T eFs);

extern u32 IoClk_GetMclkToFsRatio(MCLK_TYPE_T eMclkType);

extern void IoClk_SetMphMclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);

extern void IoClk_Set26mApll(void);

extern void IoClk_SetPcmMclk(AUD_PCM_SYNC_CYCLE eSyncCycle, u32 u4SampleRate);

extern void IoClk_SetLinMclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);
extern void IoClk_SetLin2Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);

extern void IoClk_SetAout1IecMclk(MCLK_TYPE_T eAoutMclkType, AUDIO_SAMPLING_T eAoutFs, AUDIO_SAMPLING_T eIecFs);

extern void IoClk_SetAout1Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);
extern void IoClk_SetAout2Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);

extern void IoClk_SetAdspPowerOn(void);
extern void IoClk_SetAdspPowerDown(void);

extern void IoClk_SetModulePowerOn(AUD_CLK_POWER_CTL_MODULE_ID eClkId);

extern void IoClk_SetModulePowerDown(AUD_CLK_POWER_CTL_MODULE_ID eClkId);

extern void IoClk_SetDspHwRest(void);

extern void IoClk_SetPwmHwRest(void);

extern void IoClk_SetAsrcCliSig0Src(AUD_ASRC_CLI_SIG0_SRC eSig0Src);
extern void IoClk_SetAsrcCliSig1Src(AUD_ASRC_CLI_SIG1_SRC eSig1Src);
extern void IoClk_SetAsrcCliSig2Src(AUD_ASRC_CLI_SIG2_SRC eSig2Src);
extern void IoClk_SetAsrcCliSig3Src(AUD_ASRC_CLI_SIG3_SRC eSig3Src);
extern void IoClk_SetAsrcCliSig4Src(AUD_ASRC_CLI_SIG4_SRC eSig4Src);

void IoClk_SetIecClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);
void IoClk_SetDvdAoutClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs);



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_CLOCK_IF_H

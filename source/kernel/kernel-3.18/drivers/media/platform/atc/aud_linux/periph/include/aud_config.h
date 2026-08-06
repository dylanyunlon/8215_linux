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



#ifndef _AUD_CONFIG_H_
#define _AUD_CONFIG_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "x_aud_dec.h"

#include <media/atc/aud_output.h>
#include <media/atc/drv_aud.h>
#include "aud_define.h"

typedef enum
{
    AUD_FMT_RJ,     // Right aligned with LRCK
    AUD_FMT_LJ,     // Left aligned with LRCK
    AUD_FMT_I2S     // I2S interface
}AUD_FMT_T;

typedef enum {
    AUD_FRONT_DAC,
    AUD_GPS_DAC,
    AUD_REAR_DAC,
    AUD_UNDEF_DAC
}AUD_DAC_CLASS_T;

typedef enum  {
    AOUT1_MCLK_FROM_XTAL = 0,         // 27M
    AOUT1_MCLK_FROM_K2,               // K2 from APLL
    AOUT1_MCLK_FROM_I2S_OUT0,
    AOUT1_MCLK_FROM_I2S_OUT1
}AOUT1_MCLK_CLK_FROM;

typedef enum  {
    AOUT2_MCLK_FROM_XTAL = 0,         // 27M
    AOUT2_MCLK_FROM_K4,               // K4 from APLL
    AOUT2_MCLK_FROM_I2S_OUT0,
    AOUT2_MCLK_FROM_I2S_OUT1
}AOUT2_MCLK_CLK_FROM;

typedef enum  {
    MCLK_FROM_APLL=0,
    MCLK_FROM_EXT,
    MCLK_FROM_XTAL,
    MCLK_FROM_SPDIF_IN,
    MCLK_FROM_INTERNAL_SPDIF_IN,
    MCLK_FROM_EXT_2,
    MCLK_FROM_INTERNAL_HDMI_RX
}MCLK_CLK_FROM;

#if 0
typedef enum
{
    MCLK_128FS,
    MCLK_192FS,
    MCLK_256FS,
    MCLK_384FS,
    MCLK_512FS,
    MCLK_768FS,
    MCLK_1024FS
}SAMPLE_FREQUENCY_T;

typedef enum
{
    DAC_16_BIT,
    DAC_18_BIT,
    DAC_20_BIT,
    DAC_24_BIT,
}   DAC_DATA_NUMBER_T;

typedef enum
{ 
    LRCK_CYC_16,
    LRCK_CYC_24,
    LRCK_CYC_32
} LRCK_CYC_T;
#endif

typedef enum{
    APLL_CLK270M,
    APLL_CLK294M,
    XTAL_26M
} APLL_DOMAIN;

typedef enum
{
    AUD_CLK_ALL,
    AUD_CLK_AOUT1,
    AUD_CLK_GPS,
    AUD_CLK_AOUT2,
    AUD_CLK_IEC,
    AUD_AUD_LK_IIR,
    AUD_CLK_MP,
    AUD_CLK_LINEIN,
    AUD_CLK_DVD,
    AUD_CLK_MAX
}AUD_CLOCK_T;

#if 0
typedef struct _AOUT_CFG_T
{
    AUD_CFG_ID          eCfgID;         /* ID of this path */
    AUD_OUT_TYPE_T      eSrc;           /* Aout1, Aout2, DVP Aout, GPS Aout */
    AUD_DAC_CLASS_T     eDac;           /* Front DAC, GPS DAC, Rear DAC */
    AUD_DAC_TYPE_T      eDacType;       /* PWM or Ext DAC */
    APLL_DOMAIN         eApllDomain;    /* Domain APLL1(270.1902M) or APLL2(294.912) */
    AUDIO_SAMPLING_T    eFS;            /* Frequency sample rate */
    AUD_FMT_T           eFormat;        /* format of alignment */
    DAC_DATA_NUMBER_T   eBits;          /* number of bits per sample */
    LRCK_CYC_T          eCycle;         /* cycles per sample */
    SAMPLE_FREQUENCY_T  eSampleFreq;    /* DAC sampling frequence */
    bool                fgDataInvert;   /* Invert audio output for OP phase */
    bool                fgLRInvert;     /* Invert L/R audio output */
} AOUT_CFG_T;
#endif

void AudCfg_HWInit(void);
void AudCfg_RestoreAoutRegs(void);
void AudCfg_SwitchAout(u32 dwParam);
void AudCfg_SetAoutSR(u8 u1DecId, u8 u1SmpRate);
void AudCfg_SpdifEnable(AUD_OUT_TYPE_T eSrcID);
void AudCfg_MuteSPDIF(bool fgMute);
void AudCfg_IECSelect(AUD_OUT_TYPE_T eSrcID);
void AudCfg_SetDVPRS(AUDIO_SAMPLING_T eSmpRate);
void AudCfg_UseExtLdo(bool fgUseExtLdo, AUD_DAC_CLASS_T eDacCLs);
bool AudCfg_ChgOutCfg(AUD_SOURCE_CFG_T *prAudSrcParam, AUD_OUTPUT_SETTING_CFG_T* prAudOutParam,
                      AUD_OUTPUT_SETTING_CFG_T* prAudHdmiOutParam);


void AudAout_SampleSet(AUDIO_SAMPLING_T eFS, AUD_CLOCK_T eType);
bool AudAout_PathSet(AUD_OUTPUT_PATH_T *prPath);
bool AudAout_DacTypeSet(AUD_DAC_TYPE_SEL_T *pDacType);
void AudAout_ByPassMode(AUD_AOUT_DEVID eAoutId, bool fgByPass);
void AudAout_ShowStatus(u8 u1ClkId);
void AudAout_APLLA1Sel(void);

void AudPower_Init(void);
void AudPower_Deinit(void);
void AudPower_ErrRecover_Init(void);


#endif // #ifndef _AUD_CONFIG_H_


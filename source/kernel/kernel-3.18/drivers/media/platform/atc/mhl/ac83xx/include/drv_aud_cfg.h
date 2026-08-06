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

#ifndef _DRV_AUD_CFG_H_
#define _DRV_AUD_CFG_H_
#include "x_typedef.h"

/* watermark feature configuration */
#define CONFIG_DRV_AUD_WATERMARK_SUPPORT      0
#define CONFIG_DRV_AWM_AWD_AT_ARM1            0
#define CONFIG_DRV_AUD_AW_DETECTOR_UNIT_TEST  0
#define CONFIG_DRV_AUD_AW_DETECTOR_APP_TEST   0

/* set HDMI & SPDIF output type according to Codec */
#define CONFIG_AUD_SET_CODEC_DIGITAL_OUTPUT   0

#define CONFIG_DRV_CUSTOM1 0

#define CONFIG_DRV_AUD_ANDROID 0

#define CONFIG_DRV_AUD_POWER_REDUCE 0

/* for APLL */
#define APLL_48KBASE 0
#define APLL_44KBASE 1

#include "drv_aud.h"

/*********************************************************************/
/* Type definitions */
/*********************************************************************/

#if 0
typedef enum {
	FORMAT_RJ,
	FORMAT_LJ,
	FORMAT_I2S,
}   DATA_FORMAT_T;
#endif


/* global Type */
typedef struct _SPDIF_IN_FLAG_T {
	/* BYTE 0 */
	UINT32 fgSpdifChk : 1;
	UINT32 fgSpdifLock : 1;
	UINT32 fgChkTimeOut : 1;
	UINT32 fgAUDION : 1;
	UINT32 Freq : 4;
	/* BYTE 1 */
	UINT32 fgPEM : 1;
	UINT32 Reserve : 23;
} SPDIF_IN_FLAG_T;




/*********************************************************************/
/* Export API */
/*********************************************************************/
/* Audio config */
extern void AUD_HwInit(void);
extern void AUD_SetSampleRate(UINT8 u1DecId, UINT8 u1SmpRate);/* in av_d_if.c, temply */
#if CONFIG_DRV_SUPPORT_HDMI_CLK
extern void AUD_SetDacAndApllSampleRate(UINT8 u1DecId, UINT8 u1SmpRate, SAMPLE_FREQUENCY_T u1MCLKFS);
#else
extern void AUD_SetDacAndApllSampleRate(UINT8 u1DecId, UINT8 u1SmpRate);
#endif
extern void AUD_GetAinCfg(UINT8 u1DecId, AIN_CFG_T *prAinCfg);
extern void AUD_AinCfg(UINT8 u1DecId, const AIN_CFG_T *prAinCfg);
extern void AUD_AoutFormat(UINT8 u1DecId, DATA_FORMAT_T eDataFormat);
/* extern void AUD_AoutSampleFreq(UINT8 u1DecId, SAMPLE_FREQUENCY_T eSmpFreq); */
extern void AUD_AoutInvertData(UINT8 u1DecId, BOOL eDataFormat);
extern void AUD_AoutLRCycle(UINT8 u1DecId, LRCK_CYC_T eCycle);
extern void AUD_LineInCfg(void);
extern void AUD_DspClkSel(UINT8 u1ADSPclkSel);
extern void AUD_DspClkEnable(BOOL fgEnable);
extern void AUD_LineInCtrlEnable(UINT8 u1DecId, BOOL fgEnable);
extern void AUD_SpdifOutPadEnable(UINT8 u1DecId, BOOL fgEnable);
extern void AUD_OutputHwInit(void);
#if CONFIG_DRV_SUPPORT_HDMI_CLK
extern void AUD_SetIECFrameRate(UINT8 u1DecId, UINT8 u1IecFrameRate, SAMPLE_FREQUENCY_T u1MCLKFS);
#else
extern void AUD_SetIECFrameRate(UINT8 u1DecId, UINT8 u1IecFrameRate);
#endif
#if CONFIG_DRV_SUPPORT_HDMI_CLK
extern void AUD_SetExtHdmiMclk(BOOL u1IecMclk128FsModeOn , UINT8 u1SmpRate, SAMPLE_FREQUENCY_T u1MCLKFS);
#else
extern void AUD_SetExtHdmiMclk(BOOL u1IecMclk128FsModeOn , UINT8 u1SmpRate);
#endif
extern void vSetApll(UINT8 u1APllMode);
extern UINT8 u1GetApllValue(void);
/* ADAC relative */
extern void ADAC_Init(void);
extern void ADAC_Format(UINT8 u1DataFormat);
extern void ADAC_Power(BOOL fgEnable);
extern void ADAC_SetFs(UINT8 ui1Fs, UINT8 ui1DacMode);
extern UINT8 u1HdmiGetAdacI2SFmt(void);

enum SpdifRxClockAutoLockFSRange {
	NONE_FIXED_MODE, /* HW automatically selects fs_range according to 0x52dc[31:28]. */
	FIXED_MODE_32K,  /* specify 32k */
	FIXED_MODE_44K,  /* specify 44.1k */
	FIXED_MODE_48K,  /* specify 48k */
	FIXED_MODE_64K,  /* specify 64k */
	FIXED_MODE_88K,  /* specify 88.2k */
	FIXED_MODE_96K,  /* specify 96k */
	FIXED_MODE_128K, /* specify 128k */
	FIXED_MODE_176K, /* specify 176.4k */
	FIXED_MODE_192K  /* specify 192k */
};
void vSpdifRxClockAutoLockConfigFSRange(enum SpdifRxClockAutoLockFSRange eFSRange);

/******************************************************************************
 * If HDMI Tx uses the S/PDIF interface as input,
 * its source can be one of the three enumerated in "enum HDMI_SPDIF_SOURCE".
 ******************************************************************************/
enum HDMI_SPDIF_SOURCE {
	HDMI_SPDIF_SRC_IEC1,
	HDMI_SPDIF_SRC_IEC2,
	HDMI_SPDIF_SRC_MUTE,
	HDMI_SPDIF_SRC_SPDIFIN
};

/******************************************************************************
 * If HDMI Tx uses the I2S interface as input,
 * its source can be either aout1 or aout2.
 * Specially, if aout1 is selected, the sdata0 can be chosen from ao1sdata 0 or 1 or 2 or 3 or 4 or 5.
 ******************************************************************************/
enum HDMI_I2S_DATACLK_SRC {
	HDMI_I2S_DATACLK_SRC_AOUT1,
	HDMI_I2S_DATACLK_SRC_AOUT2
};

enum HDMI_I2S_SDATA0_SRC {
	HDMI_I2S_SDATA0_SRC_AO1SDATA0 = 0,
	HDMI_I2S_SDATA0_SRC_AO1SDATA1 = 1,
	HDMI_I2S_SDATA0_SRC_AO1SDATA2 = 2,
	HDMI_I2S_SDATA0_SRC_AO1SDATA3 = 3,
	HDMI_I2S_SDATA0_SRC_AO1SDATA4 = 4,
	HDMI_I2S_SDATA0_SRC_AO1SDATA5 = 5
};

struct HDMI_I2S_SOURCE {
	enum HDMI_I2S_DATACLK_SRC eHdmiI2sDataClkSrc;
	enum HDMI_I2S_SDATA0_SRC eHdmiI2sSdata0Src;
	/* this parameter is availabe only if eHdmiI2sDataClkSrc==HDMI_I2S_DATACLK_SRC_AOUT1 */
};

/******************************************************************************
 * Audio Path to HDMI1 can be configured by function vSelectHDMI1Source()
 * Audio Path to HDMI2 can be configured by function vSelectHDMI2Source()
 ******************************************************************************/
union HDMI_SOURCE {
	struct HDMI_I2S_SOURCE rHdmiI2sSrc;
	enum HDMI_SPDIF_SOURCE eHdmiSpdifSrc;
};

void vSelectHDMI1Source(HDMI_AUDIO_INPUT_TYPE_T ePath, const union HDMI_SOURCE *prSource);
void vSelectHDMI2Source(HDMI_AUDIO_INPUT_TYPE_T ePath, const union HDMI_SOURCE *prSource);

#endif

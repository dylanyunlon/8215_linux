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

#include <linux/init.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/err.h>

#include "x_typedef.h"
#include "windev.h"
#include "aud_pcm_dbg.h"
#include "pcm_audconf.h"
#include "speechdev.h"
#include "micin.h"
#include "outhw.h"
#include "dtmf.h"
#include "input.h"

#include "pcm_debug.h"
#define LOG_TAG "pcm_audconf"

/***************************Set PCM Config Start**************************/

#define SPEECH_DELAY_SAMPLE			1300U
#define SPEECH_16K_DELAY_SAMPLE		2300U
#define SPEECH_MIC_GAIN				34U  /* 40-14=26dB */
#define SPEECH_PLC_ENABLE			1
#define SPEECH_DMNR_ENABLE			0
#define SPEECH_DEBUG_ENABLE			0
#define SPEECH_DL_USE_FILE			0
#define SPEECH_UL_USE_FILE			0

/* External Mic Enable, 0: disable; 1: enable */
#define EXTERNAL_MIC_EN				0
/* External Mic pimmux select
	1: PINMUX_I2SMICIN_GROUP1,	//i2s_in1_d        i2s_in1_bck       i2s_in1_mclk      i2s_in1_lrck
	2: PINMUX_I2SMICIN_GROUP2,	//ain0_r                    ain0_l                ain1_r              ain1_l
	3: PINMUX_I2SMICIN_GROUP3,	//ain2_r                    ain2_l                ain3_r              ain3_l
	4: PINMUX_I2SMICIN_GROUP4,	//demod_rst               ts_d5                 ts_d6               ts_d7
	5: PINMUX_I2SMICIN_GROUP5,	//vb4                            vb5                   vb6                  vb7
	6: PINMUX_I2SMICIN_GROUP6,	//lvds_ao1p          lvds_ao1n           lvds_ao0p         lvds_ao0n
	7: PINMUX_I2SMICIN_GROUP7,	//ts_d1                      ts_d2                  ts_d3               ts_d4
*/
#define EXTERNAL_MIC_I2S_PIN		1
/* External Mic samplerate select */
#define EXTERNAL_MIC_FS				48000
/* External Mic bit number select */
#define EXTERNAL_MIC_SRC_BIT_NUM	24

/* AEC/NDC parameter for signal mic  */
static SPH_ENH_08K_ctrl_struct sph_enh_ctrl1 = {
	{
		192,		/* AEC NLP */
		224,		/* AEC control word */
		2218,		/* AEC Echo suppression */
		28,			/* NDC UL control word */
		53255,		/* NDC NR */
		30,			/* NDC DL control word */
		400,		/* NDC calibration */
		104,		/* Digital Gain */
		336,		/* NDC NR */
		4325,		/* NDC NR aggressive mode */
		4193,		/* NDC RINI */
		0,
		272,		/* AEC AES */
		0,			/* ABF control (0 - ABF off) */
		0,			/* ABF Post filtering (0 - ABF off) */
		0,
		0,
		0,
		0,
		32767,		/* Clipping */
		32769,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	0
};

/* AEC/NDC parameter for dual mic */
static SPH_ENH_08K_ctrl_struct sph_enh_ctrl2 = {
	{
		192,		/* AEC NLP */
		224,		/* AEC control word */
		2218,		/* AEC Echo suppression */
		28,			/* NDC UL control word */
		53255,		/* NDC NR */
		30,			/* NDC DL control word */
		400,		/* NDC calibration */
		104,		/* Digital Gain */
		464,		/* NDC NR */
		4325,		/* NDC NR aggressive mode */
		4193,		/* NDC RINI */
		0,
		2064,		/* AEC AES */
		371,		/* ABF control (0 - ABF off) */
		23,			/* ABF Post filtering (0 - ABF off) */
		0,
		0,
		0,
		0,
		32767,		/* Clipping */
		32769,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	0
};

/* DMNR Param */
static Word16 abf_cal_data[DMNR_PARAM_NUM] = {
	 9392,       0,      0,      0,      0,      0,      0,      0,
		0,       0,   5706,   -462,    675,  -4827,   3236,  -4161,
	 4984,   -2334,  -1140,  -1183,   4484,   -722,     32,  -8171,
	 4010,    4948,   3567,  -4141,   -264,   -582,   6085,  -1284,
	 5499,   -1377,   -826,   2772,   2988,    -36,   1084,  -2452,
	 7351,    2197,     24,   -403,   -613,   -888,  -2409,   1464,
	 -193,    4704,   4393,   3641,   -942,  -2038,   1143,  -1773,
	 4498,     238,   1388,    788,   8976,  -7475,  -9758,  -3220,
	 -425,    -353,   1300,   2073,   4677,    570,   9315,  -6417,
	-8499,   -3277,  -6701,    935,   6436,   1392,   1980,   -425,
		7,   20000,  20000,  20000,  20000,  20000,  20000,  20000,
	20000,       1,  21000,      0,      0,      0,      0,      0
};
static Word16 aec_com_rx[AEC_COM_RX] = {
	32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static Word16 aec_com_tx[AEC_COM_TX] = {
	32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/* AEC/NDC params for 16k sample rate */
static uWord32 sph_enh_ctrl_16k[AEC_NDC_PARAM_NUM] = {
	192,		/* AEC NLP */
	224,		/* AEC control word */
	2218,		/* AEC Echo suppression */
	28,			/* NDC UL control word */
	53255,		/* NDC NR */
	30,			/* NDC DL control word */
	400,		/* NDC calibration */
	104,		/* Digital Gain */
	336,		/* NDC NR */
	4325,		/* NDC NR aggressive mode */
	4193,		/* NDC RINI */
	0,
	272,		/* AEC AES */
	0,			/* ABF control (0 - ABF off) */
	0,			/* ABF Post filtering (0 - ABF off) */
	0,
	0,
	0,
	0,
	32767,		/* Clipping*/
	32769,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

static Word16 dmnr_cal_data_16k[DMNR_PARAM_NUM_16K] = {
		2,        2,        1,       2,       1,        2,       3,       3,        4,       5,
		5,        4,        3,       3,       2,        2,    8440,    8225,    13206,    8705,
	16069,    14304,    11267,   11796,    9873,     8534,    8843,    9280,    14580,   10279,
	13006,     8520,        1,       7,       0,        7,      11,      16,       20,      21,
	  -36,       66,       35,      35,      35,      -56,     -48,      52,      -65,      24,
	 4044,     6200,     3166,    7043,   14082,    10028,    8944,    9438,     9223,    8563,
	 9838,     7093,     6497,   12792,   14281,    24934,   28072,   29389,        2,       0,
	    2,    21930,       68,       0,       0,        0
};

static Word16 compen_filter_16k[COMPEN_FILTER_16K] = {
	32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/***************************Set PCM Config End**************************/

/****************************for volume policy****************************/
static __s32 gBT_SPH_policyStatus = 0;
static __s32 gBT_SPH_LVolume = 0;
static __s32 gBT_SPH_RVolume = 0;

/****************************for volume policy end****************************/

s32 pcm_audconf_set(void)
{
	u32 u4Idx = 0;
	EXT_MIC_CONF rExtMicConf;
	PCM_SPEECH_CONF rSpeechConf;
	PCM_SPEECH_16K_CONF rSpeech16kConf;

	memset(&rExtMicConf, 0, sizeof(rExtMicConf));
	memset(&rSpeechConf, 0, sizeof(rSpeechConf));
	memset(&rSpeech16kConf, 0, sizeof(rSpeech16kConf));

	rExtMicConf.u4ExtMicEn = EXTERNAL_MIC_EN;
	rExtMicConf.u4ExtMicI2sPin = EXTERNAL_MIC_I2S_PIN;
	rExtMicConf.u4ExtMicFs = EXTERNAL_MIC_FS;
	rExtMicConf.u4ExtMicSrcBitNum = EXTERNAL_MIC_SRC_BIT_NUM;
	MicIn_SetExtMicCfg(rExtMicConf);
	MicIn_SetGain((u32)SPEECH_MIC_GAIN);
	MicIn_SetDataFromFile((bool)SPEECH_UL_USE_FILE);

	BtPCMHw_SetDataFromFile((bool)SPEECH_DL_USE_FILE);

	SpeechDev_EnableDump((bool)SPEECH_DEBUG_ENABLE);
	SpeechDev_EnablePLC((bool)SPEECH_PLC_ENABLE);
	SpeechDev_EnableDmnr((bool)SPEECH_DMNR_ENABLE);

	SpeechDev_SetDLDelay((u32)SPEECH_DELAY_SAMPLE);
	SpeechDev_SetDL16KDelay((u32)SPEECH_16K_DELAY_SAMPLE);

	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		rSpeechConf.enhance_pars[u4Idx] = sph_enh_ctrl1.enhance_pars[u4Idx];
		rSpeechConf.enhance_pars2[u4Idx] = sph_enh_ctrl2.enhance_pars[u4Idx];
	}
	rSpeechConf.error_flag = sph_enh_ctrl1.error_flag;
	rSpeechConf.error_flag2 = sph_enh_ctrl2.error_flag;
	SpeechDev_SetEnhanceParam(&rSpeechConf);

	for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM; u4Idx++) {
		rSpeechConf.ABF_cal_data[u4Idx] = abf_cal_data[u4Idx];
	}
	SpeechDev_SetDmnrParam(&rSpeechConf);

	for (u4Idx = 0; u4Idx < AEC_COM_RX; u4Idx++) {
		rSpeechConf.aec_com_rx[u4Idx] = aec_com_rx[u4Idx];
	}
	SpeechDev_SetComRxParam(&rSpeechConf);

	for (u4Idx = 0; u4Idx < AEC_COM_TX; u4Idx++) {
		rSpeechConf.aec_com_tx[u4Idx] = aec_com_tx[u4Idx];
	}
	SpeechDev_SetComTxParam(&rSpeechConf);

	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		rSpeech16kConf.enhance_16k_pars[u4Idx] = sph_enh_ctrl_16k[u4Idx];
	}
	SpeechDev_SetEnhance16KParam(&rSpeech16kConf);

	for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM_16K; u4Idx++) {
		rSpeech16kConf.dmnr_16k_pars[u4Idx] = dmnr_cal_data_16k[u4Idx];
	}
	SpeechDev_SetDmnr16kParam(&rSpeech16kConf);

	SpeechDev_SetFilter16kParam(compen_filter_16k, sizeof(compen_filter_16k));

	return NOERR;
}

s32 pcm_audconf_read(u32 context, void *pBuffer, u32 dwCount)
{
	return 0;
}

s32 pcm_audconf_write(u32 context, void *pBuffer, u32 dwCount)
{
	return 0;
}

s32 pcm_audconf_ioctl(u32 context, u32 code, void *pBuffer, u32 dwCount)
{
	s32 i4Ret = 0;

	PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: cmd = 0x%02x\r\n", (code - SET_SPH_DELAY) >> 2);
	if (NULL == pBuffer) {
		PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: pBuffer is NULL\r\n");
		return -1;
	}

	switch (code) {
	case SET_SPH_DELAY:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_DELAY\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_DELAY count err!\r\n");
			return INVALIDPRAM;
		}
		SpeechDev_SetDLDelay(*((u32 *)pBuffer));
		break;

	case GET_SPH_DELAY:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_SPH_DELAY\r\n");
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: 8K speech has delayed %d samples\r\n",
			(s32)SpeechDev_GetDLDelay());
		*((u32 *)pBuffer) = SpeechDev_GetDLDelay();
		break;

	case SET_SPH_16K_DELAY:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_16K_DELAY\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_16K_DELAY count err!\r\n");
			return INVALIDPRAM;
		}
		SpeechDev_SetDL16KDelay(*((u32 *)pBuffer));
		break;

	case GET_SPH_16K_DELAY:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_SPH_16K_DELAY\r\n");
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: 16K speech has delayed %d samples\r\n",
			(s32)SpeechDev_GetDL16KDelay());
		*((u32 *)pBuffer) = SpeechDev_GetDL16KDelay();
		break;

	case SET_DEVICE_SPH_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_DEVICE_SPH_GAIN\r\n");
		if (dwCount < sizeof(PCM_VOLUME)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_DEVICE_SPH_GAIN count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_SetDevVolume(((PCM_VOLUME *)pBuffer)->u4LVolume, ((PCM_VOLUME *)pBuffer)->u4RVolume);
		break;

	case GET_DEVICE_SPH_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_DEVICE_SPH_GAIN\r\n");
		if (dwCount < sizeof(PCM_VOLUME)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: GET_DEVICE_SPH_GAIN count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_GetDevVolume((u32 *)(&(((PCM_VOLUME *)pBuffer)->u4LVolume)),
			(u32 *)(&(((PCM_VOLUME *)pBuffer)->u4RVolume)));
		break;

	case SET_BT_SPH_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_GAIN\r\n");
		if (dwCount < sizeof(PCM_VOLUME)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_GAIN count err!\r\n");
			return INVALIDPRAM;
		}
        
        if(0 == ((PCM_VOLUME *)pBuffer)->policy ){
            gBT_SPH_LVolume = ((PCM_VOLUME *)pBuffer)->u4LVolume;
            gBT_SPH_RVolume = ((PCM_VOLUME *)pBuffer)->u4RVolume;
            if(0 == gBT_SPH_policyStatus){
    		    i4Ret = SpeechDev_SetSCOVolume(gBT_SPH_LVolume, gBT_SPH_RVolume); 
            }
            
        }else if(1 == ((PCM_VOLUME *)pBuffer)->policy){
            if(0 == gBT_SPH_policyStatus){
                i4Ret = SpeechDev_GetSCOVolume(&gBT_SPH_LVolume, &gBT_SPH_RVolume);
            }
        
            gBT_SPH_policyStatus = 1;
		    i4Ret = SpeechDev_SetSCOVolume(((PCM_VOLUME *)pBuffer)->u4LVolume,
			((PCM_VOLUME *)pBuffer)->u4RVolume);
            
        }else if(2 == ((PCM_VOLUME *)pBuffer)->policy){
            gBT_SPH_policyStatus = 0;
    		i4Ret = SpeechDev_SetSCOVolume(gBT_SPH_LVolume, gBT_SPH_RVolume); 
            
        }   
        
		break;

	case GET_BT_SPH_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_BT_SPH_GAIN\r\n");
		if (dwCount < sizeof(PCM_VOLUME)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: GET_BT_SPH_GAIN count err!\r\n");
			return INVALIDPRAM;
		}
        
        if(1 == gBT_SPH_policyStatus){ 
            (((PCM_VOLUME *)pBuffer)->u4LVolume) = gBT_SPH_LVolume;
            (((PCM_VOLUME *)pBuffer)->u4RVolume) = gBT_SPH_RVolume;
        }else{
    		i4Ret = SpeechDev_GetSCOVolume((u32 *)(&(((PCM_VOLUME *)pBuffer)->u4LVolume)),
    			(u32 *)(&(((PCM_VOLUME *)pBuffer)->u4RVolume)));
        }
       
		break;

	case SET_MIC_MUTE:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_MIC_MUTE\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_MIC_MUTE count err!\r\n");
			return INVALIDPRAM;
		}
		SpeechDev_EnableULMute((bool)(*((u32 *)pBuffer)));
		break;

	case SET_SPH_MIC_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_MIC_GAIN\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_SPH_MIC_GAIN count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = MicIn_SetGain(*((u32 *)pBuffer));
		break;

	case GET_SPH_MIC_GAIN:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_SPH_MIC_GAIN\r\n");
		*((u32 *)pBuffer) = MicIn_GetGain();
		break;

	case SET_DMNR_PARAM:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_DMNR_PARAM\r\n");
		break;

	case SET_AEC_NDC_PARAM:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_AEC_NDC_PARAM\r\n");
		if (dwCount < sizeof(PCM_SPEECH_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_AEC_NDC_PARAM count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_SetEnhanceParam(((PCM_SPEECH_CONF *)pBuffer));
		break;

	case GET_AEC_NDC_PARAM:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_AEC_NDC_PARAM\r\n");
		if (dwCount < sizeof(PCM_SPEECH_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: GET_AEC_NDC_PARAM count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_GetEnhanceParam(((PCM_SPEECH_CONF *)pBuffer));
		break;

	case SET_AEC_NDC_16K_PARAM:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: SET_AEC_NDC_16K_PARAM\r\n");
		if (dwCount < sizeof(PCM_SPEECH_16K_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_AEC_NDC_16K_PARAM count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_SetEnhance16KParam(((PCM_SPEECH_16K_CONF *)pBuffer));
		break;

	case GET_AEC_NDC_16K_PARAM:
		PCM_INFO(LOG_TAG, "pcm_audconf_ioctl: GET_AEC_NDC_16K_PARAM\r\n");
		if (dwCount < sizeof(PCM_SPEECH_16K_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: GET_AEC_NDC_16K_PARAM count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = SpeechDev_GetEnhance16KParam(((PCM_SPEECH_16K_CONF *)pBuffer));
		break;

	case BT_SCO_ENABLE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO_ENABLE\r\n");

		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: BT_SCO_ENABLE count err!\r\n");
			return INVALIDPRAM;
		}
#if 0		
		if (SpeechDev_IsSCOEnable() != true) {
			PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO TO ENABLED\r\n");
			SpeechDev_EnableSCO(true, *((u32 *)pBuffer));
		} else {
			PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO has ENABLED\r\n");
		}
#endif		
        SpeechDev_Enable(TRUE, TRUE, *((u32 *)pBuffer));
		break;

	case BT_SCO_DISABLE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO_DISABLE\r\n");
        SpeechDev_Enable(FALSE, TRUE, 8000);
#if 0		
		if (SpeechDev_IsSCOEnable() != false) {
			PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO TO DISABLE\r\n");
			SpeechDev_EnableSCO(false, 8000);
		} else {
			PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: BT_SCO has DISABLE\r\n");
		}
#endif		
		break;

	case SET_DL_USE_FILE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_DL_USE_FILE\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_DL_USE_FILE count err!\r\n");
			return INVALIDPRAM;
		}
		BtPCMHw_EnableDataFromFile(*((bool *)pBuffer));
		break;

	case SET_UL_USE_FILE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_UL_USE_FILE\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_UL_USE_FILE count err!\r\n");
			return INVALIDPRAM;
		}
		MicIn_EnableDataFromFile(*((bool *)pBuffer));
		break;

	case SET_BT_SPH_PLC:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_PLC\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_PLC count err!\r\n");
			return INVALIDPRAM;
		}
		SpeechDev_EnablePLC(*((bool *)pBuffer));
		break;

	case SET_DSP_MIX_CH:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_DSP_MIX_CH\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_DSP_MIX_CH count err!\r\n");
			return INVALIDPRAM;
		}
		StrmProcDSPMixCh(*((u32 *)pBuffer));
		break;

	case SET_BT_SPH_DUMP:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_DUMP\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_BT_SPH_DUMP count err!\r\n");
			return INVALIDPRAM;
		}
		SpeechDev_EnableDump(*((bool *)pBuffer));
		break;

	case SET_PRIMARY_MIC:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PRIMARY_MIC\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PRIMARY_MIC count err!\r\n");
			return INVALIDPRAM;
		}
		MicIn_SetPrimaryMic(*((u32 *)pBuffer));
		break;

	case SET_PCM_LOG:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_LOG\r\n");
		if (dwCount < sizeof(u32)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_LOG count err!\r\n");
			return INVALIDPRAM;
		}
		break;
	case SET_CAPTURE_NDC_ENABLE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_CAPTURE_NDC_ENABLE\r\n");
		Capture_NdcEnable(*((bool *)pBuffer));
		break;

#if (ENABLE_DTMF_FUNCTION)
	case SET_PCM_DTMF_CTRL:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_CTRL\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_CTRL count err!\r\n");
			return INVALIDPRAM;
		}
		i4Ret = Dtmf_EnableDtmfFunc(((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		break;

	case SET_PCM_DTMF_THRESHOLD:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_THRESHOLD\r\n");
		if (dwCount < sizeof(PCM_DTMF_THRESHOLD)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_THRESHOLD count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetThreshold(((PCM_DTMF_THRESHOLD *)pBuffer)->dlLowThreshold,
			((PCM_DTMF_THRESHOLD *)pBuffer)->dlHighThreshold);
		break;

	case SET_PCM_DTMF_NOISE_RATIO:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_NOISE_RATIO\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_NOISE_RATIO count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetNoiseRatio(((PCM_DTMF_CONF *)pBuffer)->u4Param1);
		break;

	case SET_PCM_DTMF_VALID_TIME:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_VALID_TIME\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_VALID_TIME count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetValidTime(((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		break;

	case SET_PCM_DTMF_INVALID_TIME:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_INVALID_TIME\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: DTMF_INVALID_TIME count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetInValidTime(((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		break;

	case SET_PCM_DTMF_NEW_SAMPLES:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_NEW_SAMPLES\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_NEW_SAMPLES count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetNewSamples(((PCM_DTMF_CONF *)pBuffer)->u4Param1);
		break;

	case SET_PCM_DTMF_MAX_SCALE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_MAX_SCALE\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_MAX_SCALE count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetMaxScale(((PCM_DTMF_CONF *)pBuffer)->u4Param1);
		break;

	case SET_PCM_DTMF_INFO_SENDER_EN:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_INFO_SENDER_EN\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: DTMF_INFO_SENDER_EN count err!\r\n");
			return INVALIDPRAM;
		}
		PCM_DEBUG(LOG_TAG, "[68031]pcm_audconf_ioctl: SENDER_EN (%d, %d)\r\n",
			((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		if (((PCM_DTMF_CONF *)pBuffer)->u4Param1) {
			DtmfInfoSender_Init(((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		} else {
			DtmfInfoSender_UnInit();
		}
		break;

	case SET_PCM_DTMF_INFO_SENDER_WRITE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_INFO_SENDER_WRITE\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: DTMF_INFO_SENDER_WRITE count err!\r\n");
			return INVALIDPRAM;
		}
		if (((PCM_DTMF_CONF *)pBuffer)->u4Param1)
			DtmfInfoSender_GetNewData(((PCM_DTMF_CONF *)pBuffer)->u4Param1,
			((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		else {
			DtmfInfoSender_SimulateProcessData(((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		}
		break;

	case GET_PCM_DTMF_PARAM:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: GET_PCM_DTMF_PARAM\r\n");
		Dtmf_LogParams();
		break;

	case SET_PCM_DTMF_LOG_RANGE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_LOG_RANGE\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_LOG_RANGE count err!\r\n");
			return INVALIDPRAM;
		}
		Dtmf_SetLogRange(((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		break;

	case SET_PCM_DTMF_TEST_USE_FILE:
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: SET_PCM_DTMF_TEST_USE_FILE\r\n");
		if (dwCount < sizeof(PCM_DTMF_CONF)) {
			PCM_ERROR(LOG_TAG, "pcm_audconf_ioctl: DTMF_TEST_USE_FILE count err!\r\n");
			return INVALIDPRAM;
		}
		DtmfTest_AnalyseFiles(((PCM_DTMF_CONF *)pBuffer)->u4Param1, ((PCM_DTMF_CONF *)pBuffer)->u4Param2);
		break;
#endif

	default:
		i4Ret = -1;
		PCM_DEBUG(LOG_TAG, "pcm_audconf_ioctl: UNKNOWN CMD, cmd=%04x, u4Ret=%d\r\n", code, i4Ret);
		break;
	}

	return i4Ret;
}
EXPORT_SYMBOL(pcm_audconf_ioctl);


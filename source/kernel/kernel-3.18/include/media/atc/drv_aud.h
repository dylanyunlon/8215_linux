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

#ifndef _DRV_AUD_H_
#define _DRV_AUD_H_

#include "x_typedef.h"
#include "drv_config.h"
#include "drv_av_d.h"
#include "drv_psr.h"
#include "drv_mem.h"
#include "drv_esm_if.h"

#include "u_pbinf.h"
#include "x_aud_dec.h"


/*
 *  Audio post process type define
 */
typedef enum
{
    AUD_SE_TYPE_NULL = 0,                   // Resolved NULL type
    AUD_SE__XXX_,                           // _XXX_ demo used
    AUD_SE_EQUALIZER,                       // EQ
    AUD_SE_REVERB,                          // Reverb
    AUD_SE_MP3BOOSTER,                      // Mp3 booster
    AUD_SE_DYNAMIC_BASS,                    // Dynamic Bass
    AUD_SE_NEO6,                            // Neo6
    AUD_SE_POSTMIX,                         // PostMix
    AUD_SE_RICH_BASS,                       // RICH Bass Management
    AUD_SE_PROLOGICII,                      // Prologic II (aud_se_prologicii_v2)
    AUD_SE_UPMIX,                           // UpMix (aud_se_UpMix_v2)
    AUD_SE_LOUDNESS,                        // LoudNess (aud_se_LoudNess_v2)
    AUD_SE_CSII,                            // CSII (aud_se_csii)
    AUD_SE_SWAP,                            // Swap (aud_se_swap_v2)
    AUD_SE_DOWNSAMPLE,                      // DownSample (aud_se_downsample_v2)
    AUD_SE_TVS,                             // TVS (aud_se_tvs_v2)
    AUD_SE_DH2DVS2,                         // DH2&DVS2 (aud_se_dh2_dvs2)
    AUD_SE_LPF,                             // Party (aud_se_lpf_v2)
    AUD_SE_3PEQ,                            // QBS (aud_se_3peq)
    AUD_SE_5BEQ,                            // 3peq (aud_se_5beq)
    AUD_SE_QBS,                             // 5beq (aud_se_qbs)
//#if ((defined(CONFIG_AUD_SE_MVS_EN)) && (1==CONFIG_AUD_SE_MVS_EN))
    AUD_SE_MVS,                             // MTK_VS (aud_se_mvs)
    AUD_SE_ATS,                            // MTK_ATS (aud_se_ats)
    AUD_SE_TYPE_ALL = 0x0FE,                // Only UOP could use this type
    AUD_SE_TYPE_MAX = 0x0FF                 // Resolved Max Type ID
} AUD_SE_TYPE_T;

/*
 *  Audio post process pration command struct type
 */
typedef struct _tagAudSeOpCmd{
    __u32 u4OpCode;                        // Operation Command Code
    void   *pvData;                         // Command related information data
    __u32 u4DataSize;                      // related information data size
    AUD_SE_TYPE_T  u1Type;                  // target post process type
} AUD_SE_OPCMD_T;

/*
 *  Audio post process operation code define reference (bit field format)
 */
#define AUD_SE_OPCODE_OFF           (1<<0)  // General Off bit 0
#define AUD_SE_OPCODE_ON            (1<<1)  // General On bit 1
#define AUD_SE_OPCODE_UPG_COEF      (1<<2)  // General Upgrade all coefficient
#define AUD_SE_OPCODE_RESOLVED3     (1<<3)  // General Resolved bit 3
#define AUD_SE_OPCODE_CTRL          (1<<4)  // General Ctrl bit 4
#define AUD_SE_OPCODE_UPG_COEF_HDMI (1<<5)  // General Upgrade coefficient on HDMI
#define AUD_SE_OPCODE_GET_ST        (1<<6)  // Get current status
#define AUD_SE_OPCODE_SET_LEVEL     (1<<7)  // General Set Level bit 7

// *********************************************************************
// Type definitions_1 (Duplicated Middleware Interface Related)
// *********************************************************************
/* Notify Conditions */
typedef enum
{
    AUD_COND_ERROR = -1,
    AUD_COND_CTRL_DONE,
    AUD_COND_AUD_CLIP_DONE,
    AUD_COND_MEM_BUFFER_DONE,
    AUD_COND_FLUSH_DONE,
    AUD_COND_FEED_ME_ON,
    AUD_COND_FEED_ME_OFF,
    AUD_COND_AUD_INFO_CHG,
    AUD_COND_TV_AUD_SYS_DETECTED,
    AUD_COND_AUD_REALPLAY,
    AUD_COND_INBAND_CMD_DONE
}   AUD_DRV_COND_NFY_T;

/* Speaker Types */
typedef enum
{
    AUD_DRV_SPK_L = 0,   AUD_DRV_SPK_C,       AUD_DRV_SPK_R,       AUD_DRV_SPK_LS,
    AUD_DRV_SPK_RS,      AUD_DRV_SPK_LFE1,    AUD_DRV_SPK_RLS,     AUD_DRV_SPK_CS,
    AUD_DRV_SPK_RRS,     AUD_DRV_SPK_LC,      AUD_DRV_SPK_RC,      AUD_DRV_SPK_VHL,
    AUD_DRV_SPK_VHC,     AUD_DRV_SPK_VHR,     AUD_DRV_SPK_TS,      AUD_DRV_SPK_LW,
    AUD_DRV_SPK_RW,      AUD_DRV_SPK_LSD,     AUD_DRV_SPK_RSD,     AUD_DRV_SPK_LFE2,
    AUD_DRV_SPK_LSS,     AUD_DRV_SPK_RSS,     AUD_DRV_SPK_LHS,     AUD_DRV_SPK_RHS,
    AUD_DRV_SPK_LHR,     AUD_DRV_SPK_RHR,     AUD_DRV_SPK_CHR
}   AUD_DRV_SPEAKER_LAYOUT_BITMASK_T;

typedef enum
{
    AUD_DRV_LS_FRONT_LEFT = 0,
    AUD_DRV_LS_FRONT_RIGHT,
    AUD_DRV_LS_REAR_LEFT,
    AUD_DRV_LS_REAR_RIGHT,
    AUD_DRV_LS_CENTER,
    AUD_DRV_LS_SUB_WOOFER,
    AUD_DRV_SPK_AUX1,
    AUD_DRV_SPK_AUX2
}   AUD_DRV_LS_T;

/* Audio types. */
typedef enum
{
    AUD_DRV_TYPE_UNKNOWN = 0,
    AUD_DRV_TYPE_MONO,                 /* 1/0 */
    AUD_DRV_TYPE_MONO_SUB,             /* 1+sub-language */
    AUD_DRV_TYPE_DUAL_MONO,            /* 1+1 */
    AUD_DRV_TYPE_STEREO,               /* 2/0 */
    AUD_DRV_TYPE_STEREO_SUB,           /* 2+sub-language */
    AUD_DRV_TYPE_STEREO_DOLBY_SURROUND,/* 2/0, dolby surround */
    AUD_DRV_TYPE_SURROUND_2CH,         /* 2/1 */
    AUD_DRV_TYPE_SURROUND,             /* 3/1 */
    AUD_DRV_TYPE_3_0,                  /* 3/0 */
    AUD_DRV_TYPE_4_0,                  /* 2/2 */
    AUD_DRV_TYPE_4_1,                  /* 2/2/1 */
    AUD_DRV_TYPE_5_0,                  /* 3/2 */
    AUD_DRV_TYPE_5_1,                  /* 3/2/1 */
    AUD_DRV_TYPE_6_0,                  /* 3/3 */
    AUD_DRV_TYPE_6_1,                  /* 3/3/1 */
    AUD_DRV_TYPE_7_0,                  /* 5/2 */
    AUD_DRV_TYPE_7_1,                  /* 5/2/1 */
    AUD_DRV_TYPE_FM_MONO_NICAM_MONO,
    AUD_DRV_TYPE_FM_MONO_NICAM_STEREO,
    AUD_DRV_TYPE_FM_MONO_NICAM_DUAL,
    AUD_DRV_TYPE_MULTI_CH,
    AUD_DRV_TYPE_1_0_1,                /* 1/0/1 */
    AUD_DRV_TYPE_2_1_0,                /* 2/1/0 */
    AUD_DRV_TYPE_2_1_1,                /* 2/1/1 */
    AUD_DRV_TYPE_2_2_0,                /* 2/2/0 */
    AUD_DRV_TYPE_2_2_1,                /* 2/2/1 */
    AUD_DRV_TYPE_4_2_0,                /* 4/2/0 */
    AUD_DRV_TYPE_4_2_1,                /* 4/2/1 */
    AUD_DRV_TYPE_4_3_0,                /* 4/3/0 */
    AUD_DRV_TYPE_4_3_1,                /* 4/3/1 */
    AUD_DRV_TYPE_5_2_0,                /* 5/2/0 */
    AUD_DRV_TYPE_5_2_1,                /* 5/2/1 */
    AUD_DRV_TYPE_4_0_0,                /* 4/0/0 */
    AUD_DRV_TYPE_4_0_1,                /* 4/0/1 */
    AUD_DRV_TYPE_4_1_0,                /* 4/1/0 */
    AUD_DRV_TYPE_4_1_1,                /* 4/1/1 */
    AUD_DRV_TYPE_4_4_0,                /* 4/4/0 */
    AUD_DRV_TYPE_5_0_0,                /* 5/0/0 */
    AUD_DRV_TYPE_5_0_1,                /* 5/0/1 */
    AUD_DRV_TYPE_5_1_0,                /* 5/1/0 */
    AUD_DRV_TYPE_5_1_1,                /* 5/1/1 */
    AUD_DRV_TYPE_5_3_0,                /* 5/3/0 */
    AUD_DRV_TYPE_OTHERS
}   AUD_DRV_AUD_TYPE_T;

typedef struct
{
    AUD_DRV_AUD_TYPE_T eChannel_Layout;
    __u8 u1Front;
    __u8 u1Middle;
    __u8 u1Rear;
    __u8 u1Sw;
    __u8 u1Others;
} AUD_DRV_CHANNEL_LAYOUT_T;

/* Ouput port types. */
typedef enum
{
    AUD_DRV_OUT_PORT_OFF = 0,
    AUD_DRV_OUT_PORT_2_CH,
    AUD_DRV_OUT_PORT_5_1_CH,
    AUD_DRV_OUT_PORT_SPDIF,
    AUD_DRV_OUT_PORT_2_CH_BY_PASS,
    AUD_DRV_OUT_PORT_SPEAKER
}   AUD_DRV_OUT_PORT_T;

/* SPDIF output types. */
typedef enum
{
    AUD_DRV_SPDIF_OFF = 0,
    AUD_DRV_SPDIF_RAW,
    AUD_DRV_SPDIF_PCM_16,
    AUD_DRV_SPDIF_PCM_24,
    AUD_DRV_SPDIF_NON_PCM
}   AUD_DRV_SPDIF_TYPE_T;

typedef enum
{
    AUD_DRV_HDMI_OFF = 0,
    AUD_DRV_HDMI_RAW,
    AUD_DRV_HDMI_PCM,
    AUD_DRV_HDMI_NON_PCM,
    AUD_DRV_HDMI_PCM_STEREO
}   AUD_DRV_HDMI_TYPE_T;

/* mute types. */
typedef enum
{
    AUD_DRV_MUTE_OFF = 0,
    AUD_DRV_MUTE_L_CH,
    AUD_DRV_MUTE_R_CH,
    AUD_DRV_MUTE_ON
}   AUD_DRV_MUTE_TYPE_T;

/* Audio formats. */
typedef enum
{
    AUD_DRV_FMT_UNKNOWN = 0,                // 00
    AUD_DRV_FMT_MPEG,
    AUD_DRV_FMT_AC3,
    AUD_DRV_FMT_PCM,
    AUD_DRV_FMT_MP3,
    AUD_DRV_FMT_AAC,                        // 05
    AUD_DRV_FMT_DTS,
    AUD_DRV_FMT_WMA,
    AUD_DRV_FMT_RA,
    AUD_DRV_FMT_HDCD,
    AUD_DRV_FMT_MLP,                        // 10
    AUD_DRV_FMT_MTS,   /* remove */
    AUD_DRV_FMT_EU_CANAL_PLUS,
    AUD_DRV_FMT_TV_SYS,
    AUD_DRV_FMT_EAC3,
    AUD_DRV_FMT_EAC3_SEC,                   // 15
    AUD_DRV_FMT_DTSHD_PRI_XLL,
    AUD_DRV_FMT_DTSHD_PRI_NO_XLL,
    AUD_DRV_FMT_DTSHD_SEC,
    AUD_DRV_FMT_DTSCD,
    AUD_DRV_FMT_TRUE_HD,                    // 20
    AUD_DRV_FMT_LOSSLESS_AC3,
    AUD_DRV_FMT_CDDA,
    AUD_DRV_FMT_SACD,//DSD
    AUD_DRV_FMT_VORBIS,
    AUD_DRV_FMT_DST,                        // 25
// AAC_support_DSP
    AUD_DRV_FMT_AAC_PURE,
    // for detailed DTS format @ 12/05/2008
    AUD_DRV_FMT_DTS_ES_6_1_MATRIX,
    AUD_DRV_FMT_DTS_ES_6_1_DISCRETE,
    AUD_DRV_FMT_DTS_ES_8_DISCRETE,
    AUD_DRV_FMT_DTS_96_24,                  // 30
    AUD_DRV_FMT_DTS_96_24_ES_MATRIX,
    AUD_DRV_FMT_DVDA,
    // for detailed DTSHD format @ 6/7/2009
    AUD_DRV_FMT_DTSHD_ES_6_1_MATRIX,
    AUD_DRV_FMT_DTSHD_ES_6_1_DISCRETE,
    AUD_DRV_FMT_DTSHD_ES_8_DISCRETE,        // 35
    AUD_DRV_FMT_DTSHD_96_24,
    AUD_DRV_FMT_DTSHD_96_24_ES_MATRIX,
    //For detailed RealAudio format
    AUD_DRV_FMT_RA_COOK,
    AUD_DRV_FMT_AACPLUS,
    AUD_DRV_FMT_PURE_AACPLUS,               // 40
    AUD_DRV_FMT_HEAAC_V1,
    AUD_DRV_FMT_HEAAC_V2,
    AUD_DRV_FMT_HDMI_IN_PCM,
    // DRA support_DSP
    AUD_DRV_FMT_DRA,
    AUD_DRV_FMT_DRA_EXT,                    // 45
    AUD_DRV_FMT_APE,   //mtk70169 mark add
    AUD_DRV_FMT_FLAC,
    AUD_DRV_FMT_A2DP                        // 48
}   AUD_DRV_FMT_T;

typedef enum
{
   AUD_DRV_PCM_FMT_PCM_DVDV,  // for DVD-Video
   AUD_DRV_PCM_FMT_WAVE,      // for AVI etc.
   AUD_DRV_PCM_FMT_ADPCM, 	  // for ADPCM IMA
   AUD_DRV_PCM_FMT_ADPCM_MS, 	  // for ADPCM MS
   AUD_DRV_PCM_FMT_NORMAL_PCM,     // for ASF, AVI, MPS...etc
   AUD_DRV_PCM_FMT_PCM_BD,     // for Blue-ray
   AUD_DRV_PCM_FMT_PCM_DVDV_2CH,  // for DVD-Video strip 2ch
   AUD_DRV_PCM_FMT_PCM_INV
}AUD_DRV_PCM_FMT_T;

typedef struct
{
    AUD_DRV_PCM_FMT_T ePCM_Format;
    __u16 u2BlockAlign;        // block align, only necessary for ADPCM
    bool fgDeEmphasis;
} AUD_DRV_PCM_INFO_T;

typedef struct
{
    __u16 ui2_sample_per_frame;
    __u32 ui4_frame_size_byte;
    __u16 ui2_region_num;        //Number of consecutive Modulated Lapped Transform coefficients that share the same quantizer value)
    __u32 ui4_cpl_region_start; //Coupling start region:Starting point for the band of quantization bits
    __u16 ui2_Q_bits_num;       //Number of quantization bits for each band in the joint stereo coupling information
}AUD_DRV_RA_COOK_SETTING_T;

typedef enum
{
    AUD_ADC_IN = 0,
    ADC_IIS_IN
}AUDIN_INPUT_T;

typedef struct
{
    AUD_DRV_FMT_T        e_aud_fmt;
    AUD_DRV_AUD_TYPE_T   e_aud_type;
    __u32               ui4_sample_rate;
    __u32               ui4_data_rate;
    __u8                ui1_bit_depth;
    AUD_DRV_PCM_INFO_T   *ptPCM_Info;
    __u16               ui2_pid;               //Only use for BDMW currently
    __u32               u4ChannelLayout;       //for detail channel layout
}   AUD_DRV_AUD_INFO_T;

typedef struct
{
    AUD_DRV_FMT_T        ui1_aud_fmt;
    AUD_DRV_AUD_TYPE_T   ui1_aud_type;
    __u32               ui4_sample_rate;
    __u32               ui4_data_rate;
    __u8                ui1_bit_depth;
    AUD_DRV_PCM_FMT_T    e_PCM_Format;
    __u16               ui2_BlockAlign;        // block align, only necessary for ADPCM
    bool                 fg_DeEmphasis;
    __u16               ui2_pid;               //Only use for BDMW currently
    __u32               ui4_ChannelLayout;       //for detail channel layout
}   AUD_DRV_AUD_INFO_EX_T;

/* HDCD Filter types. */
typedef enum
{
    AUD_DRV_HDCD_FLTR_OFF = 0,
    AUD_DRV_HDCD_FLTR_1X,
    AUD_DRV_HDCD_FLTR_2X,
    AUD_DRV_HDCD_FLTR_4X
}   AUD_DRV_HDCD_FLTR_T;

/* HDCD Config Bitmask */
typedef enum
{
    AUD_DRV_HDCD_ENABLE_BIT = 0,            //bit 0: hdcd enable/disable
    AUD_DRV_HDCD_AUTOLEVEL_BIT,             //bit 1: autolevel flag
    AUD_DRV_HDCD_DITHER_BIT,                //bit 2: dither on/off
    AUD_DRV_HDCD_HDCD_FLTR_ON_NONHDCD_BIT   //bit 3: do hdcd filter on non-hdcd
}   AUD_DRV_HDCD_CFG_BITMASK_T;

/* HDCD Dither Modes */
typedef enum
{
    AUD_DRV_HDCD_DITHER_OFF = 0,
    AUD_DRV_HDCD_DITHER_LVL1,
    AUD_DRV_HDCD_DITHER_LVL2,
    AUD_DRV_HDCD_DITHER_LVL3,
    AUD_DRV_HDCD_DITHER_LVL4,
    AUD_DRV_HDCD_DITHER_LVL5,
    AUD_DRV_HDCD_DITHER_LVL6,
    AUD_DRV_HDCD_DITHER_LVL7
}   AUD_DRV_HDCD_DITHER_MODE_T;

/* Dolby TrueHD DRC Mode Definition */
typedef enum
{
    AUD_DRV_TRUEHD_DRC_OFF = 0,
    AUD_DRV_TRUEHD_DRC_ON,
    AUD_DRV_TRUEHD_DRC_FOLLOW
}   AUD_DRV_TRUEHD_DRC_MODE_T;

/* Dolby TrueHD Configuration */
typedef enum
{
    AUD_DRV_TRUEHD_DRC_MODE = 0,
    AUD_DRV_TRUEHD_LOSSLESS_DEC,
    AUD_DRV_TRUEHD_DIAG_SUPPLY,
    AUD_DRV_TRUEHD_BOOST_VAL,
    AUD_DRV_TRUEHD_CUT_VAL,
    AUD_DRV_TRUEHD_DIAG_NORM_REF
}   AUD_DRV_TRUEHD_OPTION_T;

typedef struct _AUD_DRV_TRUEHD_CFG_T
{
    AUD_DRV_TRUEHD_OPTION_T u1OptionType;
    union U_TRUEHD_OPTION_VAL
    {
        AUD_DRV_TRUEHD_DRC_MODE_T u1TrueHDDrcMode;
        bool fgEnable;
        __u32 u4Val;
    } TrueHDOptionVal;
}   AUD_DRV_TRUEHD_CFG_T;

typedef void (*AudDrvNfyFct) (void*                 pvNfyTag,
                              AUD_DRV_COND_NFY_T    eNfyCond,
                              __u32                u4Data1,
                              __u32                u4Data2);

/* Notify setting info. */
typedef struct _AUD_DRV_NFY_INFO_T
{
    void*          pvTag;
    AudDrvNfyFct   pfAudDecNfy;
}   AUD_DRV_NFY_INFO_T;

/* Decode format setting info. */
typedef struct _AUD_DRV_FMT_INFO_T
{
    AUD_DRV_FMT_T   e_fmt;
    void*           pv_info;
}   AUD_DRV_FMT_INFO_T;

// *********************************************************************
// Type Definitions_2 (Driver Layer Shared Interface)
// *********************************************************************
typedef enum
{
    AUD_STREAM_FROM_TUNER = 1,
    AUD_STREAM_FROM_MEMORY,
    AUD_STREAM_FROM_SPDIF,
    AUD_STREAM_FROM_OTHERS,		// ex. Pink Noise Generator
    AUD_STREAM_FROM_LINE_IN,
    AUD_STREAM_FROM_HDMI
} AUD_DRV_STREAM_FROM_T;

// CAUTION: the index should be idential with UOP_DSP_CONFIG_DELAY_C ... and
// W_CHDELAY_C ... (add this note on 12/15/2008)
typedef enum
{
    AUD_CHANNEL_DELAY_C = 0,
    AUD_CHANNEL_DELAY_L,
    AUD_CHANNEL_DELAY_R,
    AUD_CHANNEL_DELAY_LS,
    AUD_CHANNEL_DELAY_RS,
    AUD_CHANNEL_DELAY_CH7,
    AUD_CHANNEL_DELAY_CH8,
    AUD_CHANNEL_DELAY_SUB,
    AUD_CHANNEL_DELAY_CH9,  // downmix left
    AUD_CHANNEL_DELAY_CH10, // downmix right
    AUD_CHANNEL_DELAY_ALL
}   AUD_CHANNEL_DELAY_T;

typedef struct _AUD_CH_DELAY_SETTING
{
    __u16 u2Ch_Delay_CH0;
    __u16 u2Ch_Delay_CH1;
    __u16 u2Ch_Delay_CH2;
    __u16 u2Ch_Delay_CH3;
    __u16 u2Ch_Delay_CH4;
    __u16 u2Ch_Delay_CH5;
    __u16 u2Ch_Delay_CH6;
    __u16 u2Ch_Delay_CH7;
	__u16 u2Ch_Delay_CH8;
    __u16 u2Ch_Delay_CH9;
}   AUD_CH_DELAY_SETTING_T;
/* IEC types. */
typedef enum
{
    AUD_IEC_CFG_PCM,
    AUD_IEC_CFG_RAW,
    AUD_IEC_CFG_RAW_HD,
    AUD_IEC_CFG_RAW_REENCODE,
    AUD_IEC_CFG_OFF,
    AUD_IEC_CFG_PCM_STEREO
}   AUD_IEC_CFG_T;

/* IEC PCM Channel */
typedef enum
{
    AUD_IEC_PCM_CH_L_R = 0,
    AUD_IEC_PCM_CH_LS_RS,
    AUD_IEC_PCM_CH_C_SW,
    AUD_IEC_PCM_CH_7_8,
    AUD_IEC_PCM_CH_LINE_IN,
    AUD_IEC_PCM_CH_9_10,
    AUD_IEC_PCM_CH_11_12,
    AUD_IEC_MIC
}   AUD_IEC_CH_T;


/* Audio command types. */
typedef enum
{
    AUD_CMD_PLAY    = 0,
    AUD_CMD_STOP    = 1,
    AUD_CMD_RESET   = 2,
    AUD_CMD_PAUSE   = 3,
    AUD_CMD_AVSYNC  = 4,
    AUD_CMD_LOADCODE = 5,
    AUD_CMD_RESUME = 6,
    AUD_CMD_ERRORRECOVER = 7,
}   AUD_DRV_CMD_T;

/*remapping Aud_cmd to HBI_Audio button sound event*/
typedef enum
{
    AUD_2_SOUND_PLAYER_EVN_START_REQ = 0,
    AUD_2_SOUND_PLAYER_EVN_STOP_REQ = 1,
    AUD_2_SOUND_PLAYER_EVN_PAUSE_REQ = 2,
    AUD_2_SOUND_PLAYER_EVN_CONTINUE_REQ = 7
} AUD_2_SOUND_PLAYER_EVENT;

typedef enum
{
    AUD_HDCD_1X= 0,
    AUD_HDCD_2X,
    AUD_HDCD_OFF
} AUD_HDCD_SETTING_T;

typedef enum
{
 	DSP_ASRC_TRACING_SEL_SIG0 = 0,
	DSP_ASRC_TRACING_SEL_SIG1 = 1,
 	DSP_ASRC_TRACING_SEL_SIG2 = 2,
 	DSP_ASRC_TRACING_SEL_SIG3 = 3

} AUD_DSP_ASRC_AUTO_TRACING_SIG_T;


typedef enum
{
    AUD_DIGITAL_PCM= 0,
    AUD_DIGITAL_RAW,
    AUD_DIGITAL_REENCODE,
    AUD_DIGITAL_OFF,
    AUD_DIGITAL_PCM_STEREO
} AUD_DIGITAL_SETTING_T;

typedef enum
{
    AUD_INPUT_1_0= 0,
    AUD_INPUT_1_1,
    AUD_INPUT_2_0,
    AUD_INPUT_2_1,
    AUD_INPUT_3_0, //C,L,R
    AUD_INPUT_3_1, //C,L,R
    AUD_INPUT_4_0, //L,R,RR,RL
    AUD_INPUT_4_1, //L,R,RR,RL
    AUD_INPUT_5_0,
    AUD_INPUT_5_1,
    AUD_INPUT_6_0,
    AUD_INPUT_6_1,
    AUD_INPUT_7_0,
    AUD_INPUT_7_1,
    AUD_INPUT_3_0_LRS, //LRS
    AUD_INPUT_3_1_LRS, //LRS
    AUD_INPUT_4_0_CLRS,//C,L,R,S
    AUD_INPUT_4_1_CLRS,//C,L,R,S
    //new layout added for DTS
    AUD_INPUT_6_1_Cs,
    AUD_INPUT_6_1_Ch,
    AUD_INPUT_6_1_Oh,
    AUD_INPUT_6_1_Chr,
    AUD_INPUT_7_1_Lh_Rh,
    AUD_INPUT_7_1_Lsr_Rsr,
    AUD_INPUT_7_1_Lc_Rc,
    AUD_INPUT_7_1_Lw_Rw,
    AUD_INPUT_7_1_Lsd_Rsd,
    AUD_INPUT_7_1_Lss_Rss,
    AUD_INPUT_7_1_Lhs_Rhs,
    AUD_INPUT_7_1_Cs_Ch,
    AUD_INPUT_7_1_Cs_Oh,
    AUD_INPUT_7_1_Cs_Chr,
    AUD_INPUT_7_1_Ch_Oh,
    AUD_INPUT_7_1_Ch_Chr,
    AUD_INPUT_7_1_Oh_Chr,
    AUD_INPUT_7_1_Lss_Rss_Lsr_Rsr,
    AUD_INPUT_6_0_Cs,
    AUD_INPUT_6_0_Ch,
    AUD_INPUT_6_0_Oh,
    AUD_INPUT_6_0_Chr,
    AUD_INPUT_7_0_Lh_Rh,
    AUD_INPUT_7_0_Lsr_Rsr,
    AUD_INPUT_7_0_Lc_Rc,
    AUD_INPUT_7_0_Lw_Rw,
    AUD_INPUT_7_0_Lsd_Rsd,
    AUD_INPUT_7_0_Lss_Rss,
    AUD_INPUT_7_0_Lhs_Rhs,
    AUD_INPUT_7_0_Cs_Ch,
    AUD_INPUT_7_0_Cs_Oh,
    AUD_INPUT_7_0_Cs_Chr,
    AUD_INPUT_7_0_Ch_Oh,
    AUD_INPUT_7_0_Ch_Chr,
    AUD_INPUT_7_0_Oh_Chr,
    AUD_INPUT_7_0_Lss_Rss_Lsr_Rsr,
    AUD_INPUT_8_0_Lh_Rh_Cs,
    AUD_INPUT_UNKNOWN = 0xFF
} AUD_CH_NUM_T;

typedef enum
{
    AUD_MCLK_128FS,
    AUD_MCLK_192FS,
    AUD_MCLK_256FS,
    AUD_MCLK_384FS,
    AUD_MCLK_512FS,
    AUD_MCLK_768FS,
    AUD_MCLK_1024FS,
    AUD_MCLK_TYPE_MAX,
}MCLK_TYPE_T;

typedef enum
{
    AUD_LRCK_CYC_16,
    AUD_LRCK_CYC_24,
    AUD_LRCK_CYC_32
} AUD_LRCK_CYC_T;

typedef enum {
    AUDFMT_RIGHT_JUSTIFIED,
    AUDFMT_LEFT_JUSTIFIED,
    AUDFMT_RESERVD,
    AUDFMT_IIS,
    AUDFMT_UNDEF_INTF
} AUDFMT_INTF_E;

typedef struct
{
    MCLK_TYPE_T eMclkType;  //mclk type
    AUDIO_SAMPLING_T eFs;  //sample rate
    AUD_LRCK_CYC_T eCycle;
    AUDFMT_INTF_E eDataFmt;
    __u32 u4SrcBitNum;
}AUD_IIS_FMT_SET_T;

// IIS In Params
typedef enum
{
    AUD_MASTER_MODE,
    AUD_SLAVE_MODE
}AUD_CLK_MODE;

typedef enum
{
    PINMUX_I2SIN_DEFAULT,
    PINMUX_I2SIN_GROUP1,
    PINMUX_I2SIN_GROUP2,
    PINMUX_I2SIN_GROUP3,
    PINMUX_I2SIN_GROUP4,
    PINMUX_I2SIN_GROUP5,
    PINMUX_I2SIN_GROUP6,
    PINMUX_I2SIN_GROUP7,
    PINMUX_I2SIN_GROUP_MAX
}AUD_PINMUX_I2SIN_GROUP;

typedef struct _AUD_IIS_CFG_INFO
{
    AUD_CLK_MODE eMode;
    AUD_PINMUX_I2SIN_GROUP ePinGrp;
    AUD_IIS_FMT_SET_T rFmt;
}AUD_IIS_CFG_INFO;

//for msdk struct
typedef enum
{
    LIN_DRAM_FRONT,
	LIN_BYPASS_REAR,
	LIN_BYPASS_FRONT,
	LIN_DRAM_REAR
}LIN_MODE;

typedef enum
{
   LIN_ON,
   LIN_OFF
}LIN_CTRL;

typedef struct _AUD_IIS_CTRL_INFO
{
    LIN_MODE lMode;
	LIN_CTRL fgAudInOnOff;
    AUD_IIS_CFG_INFO rI2sInfo;
}AUD_IIS_CTRL_INFO;

typedef struct
{
    LIN_MODE lMode;
	LIN_CTRL fgAudInOnOff;
	AUD_AADC_LINEIN_GROUP_E eLineINGroupSel;
}AUDIN_SET_ONOFF;

typedef enum
{
    AUD_SACD_OUTPUT_STEREO= 0,
    AUD_SACD_OUTPUT_MULTI
} AUD_SACD_CH_NUM_T;

typedef enum
{
  HDMI_MAX_2CH,
  HDMI_MAX_6CH,
  HDMI_MAX_8CH,
} AUD_HDMI_MAX_OUTPUT_CHANNEL_NUM;

/* UI Setting for AVD. */
typedef struct AUD_UI_AVDCFG_T
{
    AUD_DIGITAL_SETTING_T			eAud_Iec_Ui_Select;
    IEC_FRAME_RATE_T			eAud_Iec_Max_Sampling_Rate;
    AUD_DIGITAL_SETTING_T			eAud_Hdmi_Ui_Select;
    __u32					u4Aud_Speaker_Config;
    AUD_CH_DELAY_SETTING_T			rAud_Ui_Chan_Delay;
    AUD_CH_DELAY_SETTING_T		rAud_Ui_Hdmi_Chan_Delay;
    AUD_IEC_CH_T				eAud_IEC_Chan_Select;
    AUD_HDCD_SETTING_T			eAud_Hdcd_Select;
}   AUD_UI_AVDCFG_T;

/* AVD Decision. */
typedef struct _AUD_OUTPUT_SETTING_CFG_T
{
    AUDIO_SAMPLING_T			u1Sampling_Rate;
    AUD_IEC_CFG_T				eIec_Cfg;
    AUD_CH_NUM_T				u1Aud_Output_Chan_Cnt; //Only for main channel
    AUDIO_BITSTREAM_TYPE_T		u1Aud_Dec_Fmt;
    AUD_CH_DELAY_SETTING_T			rChan_Delay;
    bool                                      fgHBROutI2S;
    __u8                       u1HdmiMaxSupCh;//HDMI Sink Max supported Channel
    __u8                       u1HdmiSpeakAllocat;//HDMI Sink speaker Allocation, see AUD_HDMI_SINK_SPEAK_ALLOCAT each bit
    bool                       fgHDMIConnectStatus;  // True: connect; False: disconnect
}   AUD_OUTPUT_SETTING_CFG_T;

/* AVD Source Info. */
typedef struct AUD_SOURCE_CFG_T
{
    __u8					u1Aud_DecId;
    AUDIO_SAMPLING_T              		u1Aud_Sampling_Rate;
    AUDIO_SAMPLING_T              		u1Aud_Sampling_Rate_2;    //For SACD aout 2 setting
    IEC_FRAME_RATE_T			u1Aud_Iec_Frame_Rate;
    AUDIO_BITSTREAM_TYPE_T  		u1Aud_Codec_Fmt;
    AUDIO_BITSTREAM_TYPE_T  		u1Aud_Codec_Fmt_2; //For SACD aout 2 setting
    AUD_CH_NUM_T				u1Aud_Input_Chan_Cnt;
    AUD_SACD_CH_NUM_T				u1Aud_Sacd_Scom_Input_Chan_Cnt;
    AUDIO_BITSTREAM_TYPE_T		u1Aud_Reencode_Fmt;
    bool                        fgReEncStatus;  // TRUE: ReEnc on; FALSE: ReEnc off
    AUD_UI_AVDCFG_T				rAud_Ui_Setting;
    bool                        fgNeo6Enable;
    AUD_HDMI_MAX_OUTPUT_CHANNEL_NUM	 u1Hdmi_Max_Channel;
    bool                        fgHdmiEnableHdAudioOutput;
    #if CONFIG_AUD_HDMI_CLK_SUPPORT
    I2S_CLK_SAMPLE_FREQUENCY_T eSampleFreq;
    bool                        fgAudin;
    #endif
}   AUD_SOURCE_CFG_T;

typedef enum
{
    HDMI_PLAY = 0,
    HDMI_STOP,
    HDMI_CHANGE_PARA,
    HDMI_CONNECT_CHANGE
} AUD_HDMI_CMD_T;

typedef enum
{
    PRI_DEC = 0,
    SEC_DEC,
    TER_DEC,
    LINEIN1,
    LINEIN2,
    RE_ENC, // rencode
    SWMIX_DEC,
    SWMIX2_DEC,
    DVP_DEC,
    MAX_AUDDRV_NUM
} AUD_DEC_ID_T;


//AFIFO memory mapping for audio driver and Demux

typedef struct _POSINFO_T
{
    uintptr_t u4AfifoRPtr;
    uintptr_t u4AfifoWPtr;
    uintptr_t u4AfifoSA;
    uintptr_t u4AfifoEA;
} POSINFO_T;

typedef struct _AFIFO_POSINFO_T
{
    POSINFO_T posInfo[(TER_DEC - PRI_DEC + 1)];
} AFIFO_POSINFO_T;

typedef enum
{
	LPCM_BD_RESERVED0 = 0,
	LPCM_BD_SAMPLING_RATE_48KHZ,
	LPCM_BD_RESERVED1,
	LPCM_BD_RESERVED2,
	LPCM_BD_SAMPLING_RATE_96KHZ,
	LPCM_BD_SAMPLING_RATE_192KHZ
}AUD_BD_LPCM_SAMPLING_RATE_T;

typedef enum
{
   LPCM_BD_BITS_RESERVED = 0,
   LPCM_BD_BITS_16,
   LPCM_BD_BITS_20,
   LPCM_BD_BITS_24
}AUD_LPCM_BITS_PER_SAMPLE_T;

typedef enum
{
   LPCM_DVD_BITS_16,
   LPCM_DVD_BITS_20,
   LPCM_DVD_BITS_24,
   LPCM_DVD_BITS_RESERVED
}AUD_LPCM_DVD_BITS_PER_SAMPLE_T;


#define AUD_PB_SPEED_NORMAL                   0x00
#define AUD_PB_SPEED_FF_02X                   0x01
#define AUD_PB_SPEED_FF_04X                   0x03
#define AUD_PB_SPEED_FF_08X                   0x07
#define AUD_PB_SPEED_FF_16X                   0x0F
#define AUD_PB_SPEED_FF_32X                   0x1F
#define AUD_PB_SPEED_FF_64X                   0x3F

typedef enum
{
    EFFSND_SAMPLING_FREQ_RESERVED = 0,
    EFFSND_SAMPLING_FREQ_48KHZ,
    EFFSND_SAMPLING_FREQ_96KHZ
}AUD_EFFSND_SAMPLING_FREQ_T;

typedef enum
{
  // sample frequency defined here
  FLSND_SFREQ_RESERVED0 = 0x0,
  FLSND_SFREQ_8K = 0x01,
  FLSND_SFREQ_16K = 0x02,
  FLSND_SFREQ_32K = 0x03,
  FLSND_SFREQ_64K = 0x04,
  FLSND_SFREQ_RESERVED1 = 0x05,
  FLSND_SFREQ_11K = 0x06,
  FLSND_SFREQ_22K = 0x07,
  FLSND_SFREQ_44K = 0x08,
  FLSND_SFREQ_88K = 0x09,
  FLSND_SFREQ_176K = 0x0A,
  FLSND_SFREQ_12K = 0x0B,
  FLSND_SFREQ_24K = 0x0C,
  FLSND_SFREQ_48K = 0x0D,
  FLSND_SFREQ_96K = 0x0E,
  FLSND_SFREQ_192K = 0x0F,
  FLSND_SFREQ_MAX = 0x10
} AUD_FLSND_SFREQ_IDX_T;

typedef enum
{
    EFFSND_BITS_DEPTH_8 = 0,
    EFFSND_BITS_DEPTH_16,
    EFFSND_BITS_DEPTH_20,
    EFFSND_BITS_DEPTH_24,
    EFFSND_BITS_DEPTH_RESERVED
}AUD_EFFSND_BITS_DEPTH_T;


typedef struct _AUD_OUTPUT_VOL
{
	__u32 u4ChVolFrontL;
	__u32 u4ChVolFrontR;
	__u32 u4ChVolFrontLs;
	__u32 u4ChVolFrontRs;
	__u32 u4ChVolFrontC;
	__u32 u4ChVolFrontSub;
	__u32 u4ChVolGpsMix;
	__u32 u4ChVolRearL;
	__u32 u4ChVolRearR;
	__u32 u4ChVolBypassL;	  // Line in by pass volume
	__u32 u4ChVolBypassR;
}AUD_OUTPUT_VOL;



typedef enum
{
    AUD_VOL_NORMAL = 0,
    AUD_VOL_SET_POLICY,
    AUD_VOL_RESET_POLICY
}AUD_VOL_POLICY_T;


typedef struct _AUD_VOLUME_POLICY_INFO_
{
    AUD_VOL_POLICY_T eType;
    AUD_DEC_VOLUME_GAIN_INFO_T rVolGainInfo;
}AUD_VOLUME_POLICY_INFO;


typedef enum
{
    AUD_ADSP_NORMAL = 0,
    AUD_ADSP_RESET_FLAG = 1,
    AUD_ADSP_BUF_FULL = 2,
    AUD_ADSP_RESOUCE_ERR = 3,
    AUD_ADSP_PARA_ERR = 4
}AUD_ADSP_RESET_T;

typedef enum
{
    AOUT_DEFAULT =0,
    AOUT_FRMR
}REAR_AOUT_TYPE;

// *********************************************************************
// Constant definitions
// *********************************************************************

#define AUD_DEC_MAIN PRI_DEC
#define AUD_DEC_AUX  SEC_DEC


#define AUD_CMD_FLAG_PLAY     ((__u32) (1<<(__u32)AUD_CMD_PLAY))
#define AUD_CMD_FLAG_STOP     ((__u32) (1<<(__u32)AUD_CMD_STOP))
#define AUD_CMD_FLAG_RESET    ((__u32) (1<<(__u32)AUD_CMD_RESET))
#define AUD_CMD_FLAG_PAUSE    ((__u32) (1<<(__u32)AUD_CMD_PAUSE))
#define AUD_CMD_FLAG_AVSYNC   ((__u32) (1<<(__u32)AUD_CMD_AVSYNC))
#define AUD_CMD_FLAG_LOADCODE ((__u32) (1<<(__u32)AUD_CMD_LOADCODE))
#define AUD_CMD_FLAG_RESUME    ((__u32) (1<<(__u32)AUD_CMD_RESUME))
#define AUD_CMD_FLAG_ERRORRECOVER    ((__u32) (1<<(__u32)AUD_CMD_ERRORRECOVER))

//#define AUD_Q13_VALUE      8192

#define AUD_EFFAUD_GAIN_COEFF      0x20000
#define AUD_EFFAUD_GAIN_ARRAY_NUM  20

//#define AUD_EFFAUD_SUPPORT_SAMPLE_RATE_48k 48000

#define AUD_SPECTRUM_INFO_NUM 16

// *********************************************************************
// Export API
// *********************************************************************

// extern CLI_EXEC_T* GetAudCmdTbl(void);

extern __s32 AUD_Init(void);
extern __s32 AUD_Uninit(void);
extern __s32 AUD_SetDecType(__u8 ucDecId,  AUD_DRV_STREAM_FROM_T eStreamFrom,
                            const AUD_DRV_FMT_INFO_T * prDecType);

extern __s32 AUD_DSPCmdPlayAsyn(__u8 ucDecId);
extern __s32 AUD_DSPCmdPauseAsyn(__u8 ucDecId);
extern __s32 AUD_DSPCmdResumeAsyn(__u8 u1DecId);
extern __s32 AUD_DSPCmdStopAsyn(__u8 ucDecId);
extern __s32 AUD_DSPCmdResetAsyn(__u8 ucDecId);
// Effect sound commands
//extern INT32 AUD_DSPCmdEffSndPlay(UINT8 u1DecId);
extern __s32 AUD_DSPCmdEffSndStop(__u8 u1DecId);
//extern INT32 AUD_DSPCmdEffSndPause(UINT8 u1DecId);

extern __s32 AUD_GetAudFifo(uintptr_t * pu4Fifo1Start, uintptr_t * pu4Fifo1SEnd,
	                        uintptr_t * pu4Fifo2Start, uintptr_t * pu4Fifo2End);

extern void AUD_GetDspVersionNumber(AUD_DEC_DSP_VERSION_T *prDspVer);
//FIXME // should support other API to query dsp status

extern __s32 i4AudSetPlaySpeed(__u8 u1DecId, AUD_DEC_PB_SPEED_TYPE_T tSpeed);

//extern BOOL fgDspCgmsInfo(UCHAR ucDecId, AUD_DEC_CGMS_INFO_T rCgmsInfo);

//RA_COOK Setting

/* interface for avsync for wince by mtk40292*/
extern void vAudDrvIf_DisableAVSync(__u8 u1DecId);
extern void vAudDrvIf_SetTargetPTS(__u8 u1DecId,  __u64 u8FirstPTS);
extern void vAudDrvIf_GetCurrentPTS(__u8 u1DecId, __u64* u8FirstPTS);
extern void vAudDrvIf_GetLatestPTS(__u8 u1DecId,__u32* u4PTSHi,__u32* u4PTSLo);

void vAudDrvIf_SwitchAout(__u32 dwParam);

// MISC
extern void AudShowCacheInfo(VOID);
extern void AudShowDspStatus(VOID);
extern void AudShowConfig(VOID);
extern void AudDispStates(VOID);
//extern void AudDispPassThrougStates(void);
extern void AudDispUopHistory(VOID);
extern void AudDispIECRegisters(VOID);
extern void vAudDrvIf_DspStopDone(__u8 u1DecId);
//extern BOOL AUD_DRVGetDecodeType(UINT8 u1DecId,  AUD_DRV_STREAM_FROM_T * peStreamFrom,AUD_DRV_FMT_INFO_T * prDecType);
//extern void AUD_SetAvSynMode(UINT8 u1DecId, AV_SYNC_MODE_T eSynMode);
//extern void vAudCodecSet_Bass_Management_Mode(AUD_DEC_BASS_MANAGEMENT_MODE_T t_BM_mode);

extern __s32 i4AudEsm_Notify_Play(__u16 u2ADRV_Comp_Id);
extern __s32 i4AudEsm_Notify_Stop(__u16 u2ADRV_Comp_Id);

extern void AUD_ClipDoneNotify(__u8 u1DecId);
extern void vDspPowerOff (void);

extern void vAudCodecSet_SACD_Input_Info(AUD_DEC_AUD_INFO_T *tInputInfo);
extern void vAudSetSacdInputChannelNum(__u32 u4_sacd_input_channel_num);
#if CONFIG_DRV_AUDIO_IN
extern void vAudCodecSet_CDDA_Info(AUD_DEC_AUD_INFO_T  *ptDecInfo);
extern void vAudInSetEmphasisFlag(bool fgEmphasis);
#endif
extern void vAudCodecSet_DLNA_Info(VOID);
extern void vAudCodecDLNAInit(VOID);
extern void vAudCodecSet_SACD_Output_Info(AUD_DEC_SACD_OUTPUT_T *tInputInfo);

#ifndef DRV_ONLY
extern __s32 i4AUD_CertStart(__u8 u1Mode, __u8 u1OutCh);
extern __s32 i4AUD_CertStop(VOID);
#endif

extern void AUD_GetSpectrumInfo(AUD_DEC_SPECTRUM_INFO_T * ptAudSpectrumInfo);
extern void vAudTestToneSetType(AUD_DEC_TEST_TONE_TYPE_T eType,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudTestToneSetChannel(AUD_DEC_LS_T eChannel,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudTestToneSwitch(AUD_DEC_TESTTONE_ONOFF fgTTONOFF,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudSetDec4Info(AUD_DEC4_INFO_T* ptAudDec4Inf);
extern void vAudLRMixing(AUD_DEC_LRMIX_OUTPUT_T t_lrmix_mode);
extern void vAudSetGlobalBoosterGain(__s32 i4Gain);
extern void AUD_GetPbInfo(__u8 u1DecId, PBINF_A *ptAudPbInfo);

/****************************************************************************
** Export API //from aud_if.h
****************************************************************************/
extern void AUD_WaitCommandDone(__u8 ucDecId, AUD_DRV_CMD_T eAudDecCmd);
extern void AUD_CommandDoneNotify(__u8 u1DecId,  AUD_DRV_CMD_T eAudDecCmd);

#endif /* _DRV_AUD_H_ */


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

#ifndef _X_AUD_DEC_H_
#define _X_AUD_DEC_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_rm.h"
#include "x_drv_cb.h"
#include "x_memtype.h"

#define AUDIO_SETTING_CTS_ENABLED 1

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
typedef struct _APE_SEEKINFO_INFO_T
{
    __u32 ui4_mute_bank_numbers;
    __u32 ui4_invalid_bytes;
}   APE_SEEKINFO_INFO_T;



/* Notify conditions */
typedef enum
{
    AUD_DEC_COND_ERROR = -1,
    AUD_DEC_COND_CTRL_DONE,
    AUD_DEC_COND_AUD_CLIP_DONE,
    AUD_DEC_COND_MEM_BUFFER_DONE,
    AUD_DEC_COND_FLUSH_DONE,
    AUD_DEC_COND_FEED_ME_ON,
    AUD_DEC_COND_FEED_ME_OFF,
    AUD_DEC_COND_AUD_INFO_CHG,
    AUD_DEC_COND_TV_AUD_SYS_DETECTED,
    AUD_DEC_COND_AUD_REALPLAY,
    AUD_DEC_COND_INBAND_CMD_DONE
}   AUD_DEC_COND_T;

typedef enum{
	AUD_DEC_HDMI_REINIT_ON = 0,
	AUD_DEC_HDMI_REINIT_OFF
}AUD_DEC_HDMI_REINIT_T;

/*HTS HDMI CHANNEL MODE */
typedef enum
{
	AUD_DEC_HDMI_CHANNEL_5_1_MODE = 0,
	AUD_DEC_HDMI_CHANNEL_7_1_MODE,
        AUD_DEC_HDMI_CHANNEL_2_MODE
}AUD_DEC_HDMI_CHANNEL_MODE_T;

/* Control types. */
typedef enum
{
    AUD_DEC_CTRL_RESET = 0,
    AUD_DEC_CTRL_STOP,
    AUD_DEC_CTRL_PLAY,
    AUD_DEC_CTRL_PLAY_SYNC,
    AUD_DEC_CTRL_PLAY_AUD_CLIP,
    AUD_DEC_CTRL_FLUSH,
    AUD_DEC_CTRL_PAUSE,
    AUD_DEC_CTRL_RESUME,
    AUD_DEC_CTRL_DETECT_TV_AUD_SYS,
    AUD_DEC_CTRL_SPEED, //. add for speed control
    AUD_DEC_CTRL_START_PAUSE
}AUD_DEC_CTRL_T;


/* Audio types. */
typedef enum
{
    AUD_DEC_TYPE_UNKNOWN = 0,
    AUD_DEC_TYPE_MONO,                 /* 1/0 */
    AUD_DEC_TYPE_MONO_SUB,             /* 1+sub-language */
    AUD_DEC_TYPE_DUAL_MONO,            /* 1+1 */
    AUD_DEC_TYPE_STEREO,               /* 2/0 */
    AUD_DEC_TYPE_STEREO_SUB,           /* 2+sub-language */
    AUD_DEC_TYPE_STEREO_DOLBY_SURROUND,/* 2/0, dolby surround */
    AUD_DEC_TYPE_SURROUND_2CH,         /* 2/1 */
    AUD_DEC_TYPE_SURROUND,             /* 3/1 */
    AUD_DEC_TYPE_3_0,                  /* 3/0 */
    AUD_DEC_TYPE_4_0,                  /* 2/2 */
    AUD_DEC_TYPE_4_1,
    AUD_DEC_TYPE_5_0,
    AUD_DEC_TYPE_5_1,                  /* 3/2 */
    AUD_DEC_TYPE_6_0,
    AUD_DEC_TYPE_6_1,
    AUD_DEC_TYPE_7_0,
    AUD_DEC_TYPE_7_1,                  /* 5/2 */
    AUD_DEC_TYPE_FM_MONO_NICAM_MONO,
    AUD_DEC_TYPE_FM_MONO_NICAM_STEREO,
    AUD_DEC_TYPE_FM_MONO_NICAM_DUAL,
    AUD_DEC_TYPE_MULTI_CH,
    AUD_DEC_TYPE_1_0_1,                /* 1/0/1 */
    AUD_DEC_TYPE_2_1_0,                /* 2/1/0 */
    AUD_DEC_TYPE_2_1_1,                /* 2/1/1 */
    AUD_DEC_TYPE_2_2_0,                /* 2/2/0 */
    AUD_DEC_TYPE_2_2_1,                /* 2/2/1 */
    AUD_DEC_TYPE_4_2_0,                /* 4/2/0 */
    AUD_DEC_TYPE_4_2_1,                /* 4/2/1 */
    AUD_DEC_TYPE_4_3_0,                /* 4/3/0 */
    AUD_DEC_TYPE_4_3_1,                /* 4/3/1 */
    AUD_DEC_TYPE_5_2_0,                /* 5/2/0 */
    AUD_DEC_TYPE_5_2_1,                /* 5/2/1 */
    AUD_DEC_TYPE_4_0_0,                /* 4/0/0 */
    AUD_DEC_TYPE_4_0_1,                /* 4/0/1 */
    AUD_DEC_TYPE_4_1_0,                /* 4/1/0 */
    AUD_DEC_TYPE_4_1_1,                /* 4/1/1 */
    AUD_DEC_TYPE_4_4_0,                /* 4/4/0 */
    AUD_DEC_TYPE_5_0_0,                /* 5/0/0 */
    AUD_DEC_TYPE_5_0_1,                /* 5/0/1 */
    AUD_DEC_TYPE_5_1_0,                /* 5/1/0 */
    AUD_DEC_TYPE_5_1_1,                /* 5/1/1 */
    AUD_DEC_TYPE_5_3_0,                /* 5/3/0 */
    AUD_DEC_TYPE_OTHERS
}   AUD_DEC_AUD_TYPE_T;


/*media types*/
typedef enum
{
    AUD_DEC_MEDIA_BDAV = 0,           ///< BDAV
    AUD_DEC_MEDIA_BDMV,            ///< BDMV
    AUD_DEC_MEDIA_DVDVID,          ///< dvd video
    AUD_DEC_MEDIA_DVDAUD,          ///< dvd audio
    AUD_DEC_MEDIA_DVDMVR,          ///< dvd mvr
    AUD_DEC_MEDIA_DVDPVR,          ///< dvd pvr
    AUD_DEC_MEDIA_VCD20,           ///< vcd 2.0
    AUD_DEC_MEDIA_VCD11,           ///< vcd 1.1
    AUD_DEC_MEDIA_SVCD,            ///< svcd
    AUD_DEC_MEDIA_CVD,             ///< cvd
    AUD_DEC_MEDIA_CDDA,            ///< cdda
    AUD_DEC_MEDIA_HDCD,            ///< hdcd
    AUD_DEC_MEDIA_DTSCD,           ///< dtscd
    AUD_DEC_MEDIA_CDG,             ///< cd_g
    AUD_DEC_MEDIA_SACD,            ///< sacd
    AUD_DEC_MEDIA_STILL,           ///< FPB still picture
    AUD_DEC_MEDIA_AUDIO,           ///< FPB audio
    AUD_DEC_MEDIA_MOVIE,           ///< FBP movie picture
    AUD_DEC_MEDIA_HDDPVR,          ///< hdd pvr
    AUD_DEC_MEDIA_HDDMVR,          ///< hdd mvr
    AUD_DEC_MEDIA_HDDAVI,          ///< hdd avi
    AUD_DEC_MEDIA_INVALID          ///< invalid
}AUD_DEC_MEDIA_T;

typedef enum
{
    AUD_DEC_FS_16K = 0,
	AUD_DEC_FS_22K,
	AUD_DEC_FS_24K,
	AUD_DEC_FS_32K,
	AUD_DEC_FS_44K,
	AUD_DEC_FS_48K,
	AUD_DEC_FS_64K,
	AUD_DEC_FS_88K,
	AUD_DEC_FS_96K,
	AUD_DEC_FS_176K,
	AUD_DEC_FS_192K,
	AUD_DEC_FS512_44K, //for DSD
	AUD_DEC_FS_768K,
	AUD_DEC_FS_UNKNOWN

}AUDIO_DEC_DEFAULT_FS_T;

/* Ouput port types. */
typedef enum
{
    AUD_DEC_OUT_PORT_OFF = 0,
    AUD_DEC_OUT_PORT_2_CH,
    AUD_DEC_OUT_PORT_5_1_CH,
    AUD_DEC_OUT_PORT_SPDIF,
    AUD_DEC_OUT_PORT_2_CH_BY_PASS,
    AUD_DEC_OUT_PORT_SPEAKER
}   AUD_DEC_OUT_PORT_T;


/* SPDIF output types. */
typedef enum
{
    AUD_DEC_SPDIF_OFF = 0,
    AUD_DEC_SPDIF_RAW,
    AUD_DEC_SPDIF_PCM,
    AUD_DEC_SPDIF_NON_PCM  // re-encoder
}   AUD_DEC_SPDIF_TYPE_T;


/* mute types. */
typedef enum
{
    AUD_DEC_MUTE_OFF = 0,
    AUD_DEC_MUTE_L_CH,
    AUD_DEC_MUTE_R_CH,
    AUD_DEC_MUTE_ON
}   AUD_DEC_MUTE_TYPE_T;

/* Dec1 mute  */
typedef enum
{
    AUD_DEC1_MUTE_OFF = 0,
    AUD_DEC1_MUTE_ON
}   AUD_DEC1_MUTE_CTRL_T;


/* HDMI mute types */
typedef enum
{
	AUD_DEC_HDMI_MUTE_OFF = 0,
	AUD_DEC_HDMI_MUTE_ON
}	AUD_DEC_HDMI_MUTE_TYPE_T;

/* SPDIF mute types */
typedef enum
{
	AUD_DEC_SPDIF_MUTE_OFF = 0,
	AUD_DEC_SPDIF_MUTE_ON
}	AUD_DEC_SPDIF_MUTE_TYPE_T;

/* mute source id. */
typedef enum
{
    AUD_DEC_MUTE_UI = 1<<0,
    AUD_DEC_MUTE_AW = 1<<1
}   AUD_DEC_MUTE_SRC_T;

/* metadata types for sec audio stream applied in BD */
typedef enum
{
    AUD_DEC_METADATA_OUTPUT_OFF = 0,
    AUD_DEC_METADATA_OUTPUT_ON
}   AUD_DEC_METADATA_OUTPUT_TYPE_T;

/* Miracast on/off  */
typedef enum
{
    AUD_MIRACAST_OFF = 0,
    AUD_MIRACAST_ON
}   AUD_MIRACAST_CTRL_T;


/* equalizer types. */
#define AUD_DEC_EQ_SET_U4ENABLE  (1<<0)
#define AUD_DEC_EQ_SET_U4BANDNUM (1<<1)
#define AUD_DEC_EQ_SET_AU4GAIN   (1<<2)
#define AUD_DEC_EQ_SET_IIR_COEF  (1<<3)

typedef struct AUD_DEC_EQ_TYPE_T {
    __u32 u4_op;  // bit0: set u4_enable
                   // bit1: set u4_bandnum
                   // bit2: set au4_gain
                   // other bits: reserved

    __u32 u4_enable;
    __u32 u4_bandnum;
    //u32 au4Coefficient[30];  //coefficient will be set to dead by hard code
    __u32 au4_gain[7][11]; // 7 channels, each with 1 dry and 10 band
}  AUD_DEC_EQ_TYPE_T;

/* Audio formats. */
typedef enum
{
    AUD_DEC_FMT_UNKNOWN = 0,
    AUD_DEC_FMT_MPEG,
    AUD_DEC_FMT_AC3,
    AUD_DEC_FMT_PCM,
    AUD_DEC_FMT_MP3,
    AUD_DEC_FMT_AAC,                    // 5
    AUD_DEC_FMT_DTS,
    AUD_DEC_FMT_WMA,
    AUD_DEC_FMT_RA,
    AUD_DEC_FMT_HDCD,
    AUD_DEC_FMT_MLP,                    // 10
    AUD_DEC_FMT_MTS,   /* remove */
    AUD_DEC_FMT_EU_CANAL_PLUS,
    AUD_DEC_FMT_TV_SYS,
    AUD_DEC_FMT_EAC3,
    AUD_DEC_FMT_EAC3_SEC,               // 15
    AUD_DEC_FMT_DTSHD_PRI_XLL,
    AUD_DEC_FMT_DTSHD_PRI_NO_XLL,
    AUD_DEC_FMT_DTSHD_SEC,
    AUD_DEC_FMT_DTSCD,
    AUD_DEC_FMT_TRUE_HD,                // 20
    AUD_DEC_FMT_LOSSLESS_AC3,
    AUD_DEC_FMT_CDDA,
    AUD_DEC_FMT_SACD,//DSD
    AUD_DEC_FMT_VORBIS,
    AUD_DEC_FMT_DST,                    // 25
    AUD_DEC_FMT_AAC_PURE,
    AUD_DEC_FMT_DTS_ES_6_1_MATRIX,
    AUD_DEC_FMT_DTS_ES_6_1_DISCRETE,
    AUD_DEC_FMT_DTS_ES_8_DISCRETE,
    AUD_DEC_FMT_DTS_96_24,              // 30
    AUD_DEC_FMT_DTS_96_24_ES_MATRIX,
    AUD_DEC_FMT_DVDA,
    AUD_DEC_FMT_DTSHD_ES_6_1_MATRIX,
    AUD_DEC_FMT_DTSHD_ES_6_1_DISCRETE,
    AUD_DEC_FMT_DTSHD_ES_8_DISCRETE,    // 35
    AUD_DEC_FMT_DTSHD_96_24,
    AUD_DEC_FMT_DTSHD_96_24_ES_MATRIX,
    AUD_DEC_FMT_RA_COOK,
    AUD_DEC_FMT_AACPLUS,
    AUD_DEC_FMT_PURE_AACPLUS,           // 40
    AUD_DEC_FMT_HEAAC_V1,
    AUD_DEC_FMT_HEAAC_V2,
    AUD_DEC_FMT_HDMI_IN_PCM,
    AUD_DEC_FMT_DRA,
    AUD_DEC_FMT_DRA_EXT,                // 45
    AUD_DEC_FMT_APE,    //mtk70169 mark
    AUD_DEC_FMT_FLAC,
    AUD_DEC_FMT_A2DP

  }   AUD_DEC_FMT_T;



/* output port adjust struct. */
typedef struct
{
    AUD_DEC_OUT_PORT_T  e_out_port;
    bool                b_is_fixed;
}   AUD_DEC_OUTPORT_ADJ_T;



typedef enum
{
   AUD_DEC_PCM_FMT_PCM_DVDV,  /* for DVD-Video*/
   AUD_DEC_PCM_FMT_WAVE,      /* for AVI etc.*/
   AUD_DEC_PCM_FMT_ADPCM,     /* for ADPCM IMA*/
   AUD_DEC_PCM_FMT_ADPCM_MS,  /* for ADPCM MS*/
   AUD_DEC_PCM_FMT_PCM_NORMAL,/* for ASF, AVI, MPS...etc */
   AUD_DEC_PCM_FMT_PCM_BD,     /* for Blue-ray*/
   AUD_DEC_PCM_FMT_PCM_DVDV_2CH /* for DVD-Video strip 2ch*/
}AUD_DEC_PCM_FMT_T;


typedef struct
{
    AUD_DEC_PCM_FMT_T ePCM_Format;
    __u16 u2BlockAlign;        /* block align, only necessary for ADPCM*/
    bool   b_de_emphasis;       /*de-emphasis bit... 0 or 1*/
    bool   b_dlna_exist;		/*DLNA exist or not*/
} AUD_DEC_PCM_INFO_T;

typedef enum
{
    A2DP_BIG_ENDIAN=0,
    A2DP_LITTLE_ENDIAN,
    A2DP_UNDEF_DATA_ENDIAN
} AUD_DEC_A2DP_DATA_ENDIAN;

typedef enum
{
    A2DP_BIT8_PER_SAMPLE=0,
    A2DP_BIT16_PER_SAMPLE,
    A2DP_BIT24_PER_SAMPLE,
    A2DP_UNDEF_PER_SAMPLE
} AUD_DEC_A2DP_BIT_DEPTH;


typedef struct
{
    AUD_DEC_A2DP_BIT_DEPTH eBitDepth;
    AUD_DEC_A2DP_DATA_ENDIAN eDataEndian;
    __u32 u4SmpRate;
    __u32 u4channel_cnt;
} AUD_DEC_A2DP_INFO_T;



typedef struct
{
    AUD_DEC_FMT_T        e_aud_fmt;
    AUD_DEC_A2DP_INFO_T  t_aud_a2dp_info;
}AUD_DEC4_INFO_T;

typedef enum
{
   AUD_DEC_CGMS_FMT_OTHERS,   /* for UNKONW format */
   AUD_DEC_CGMS_FMT_DVD,      /* for DVD */
   AUD_DEC_CGMS_FMT_VCD,      /* for VCD */
   AUD_DEC_CGMS_FMT_BD,       /* for BD  */
   AUD_DEC_CGMS_FMT_DIVX,     /* for Divx */
   AUD_DEC_CGMS_FMT_SACD,     /* for sacd */
   AUD_DEC_CGMS_FMT_CD,       /* for cdda */
   AUD_DEC_CGMS_FMT_DEFAULT,  /* for default */
} AUD_DEC_CGMS_FMT_T;


typedef struct _AUD_DEC_CGMS_INFO_T
{
    AUD_DEC_CGMS_FMT_T e_cgms_format;
    bool               b_is_from_usb;
    bool               b_is_CSS;
} AUD_DEC_CGMS_INFO_T;

/* Audio spectrum info. */
typedef struct _AUD_DEC_SPECTRUM_INFO_T
{
    __u32    u4_aud_spectrum[16];
    __u32    u4_aud_spectrum_bar[16];
} AUD_DEC_SPECTRUM_INFO_T;

typedef struct _AUD_SPECTRUM_BUF_INFO_T
{
    __u8*    u4buf;
    __u32    u4size;
    __u32    u4scalingMode;
} AUD_SPECTRUM_BUF_INFO_T;

typedef struct _AUD_DEC_TRANSCO_INFO_T
{
    AUD_DEC_FMT_T       e_tranc_fmt;
    bool                b_is_enable;
} AUD_DEC_TRANSCO_INFO_T;

#define WMA_SPEC_DATA_MAX_SIZE    20
typedef struct _AUD_DEC_WMA_INFO_T
{
    __u16 ui2_version;
    __u32 ui4_packet_count;
    __u32 ui4_packet_size;
    __u16 ui2_enc_option;
    __u16 ui2_blockalign;
    __u32 ui4_bytes_per_sec;
    __u16 ui2_bit_depth;
    __u16 u2CodecSpecDataSize;
    __u8  au1CodecSpecData[WMA_SPEC_DATA_MAX_SIZE];
} AUD_DEC_WMA_INFO_T;

typedef struct _AUD_DEC_RA_COOK_INFO_T
{
    __u32 u4_sample_rate;
    __u16 ui2_sample_per_frame;
    __u32 ui4_frame_size_byte;
    __u16 ui2_region_num;        //Number of consecutive Modulated Lapped Transform coefficients that share the same quantizer value)
    __u32 ui4_cpl_region_start; //Coupling start region:Starting point for the band of quantization bits
    __u16 ui2_Q_bits_num;       //Number of quantization bits for each band in the joint stereo coupling information
} AUD_DEC_RA_COOK_INFO_T;

typedef struct _AUD_DEC_APE_INFO_T   //mtk70169 mark
{
    __u32 ui4_file_versoin;
    __u32 ui4_compress_level;
    __u32 ui4_block_per_frame;
    __u32 ui4_final_frame_block;
    __u32 ui4_total_frame_num;
    __u32 ui4_bits_per_sample;
    __u32 ui4_channel_num_1;
    __u32 ui4_input_sampling_rate;
	//begin add by mtk40292 for APE info
    __u32 ui4_mute_bank_numbers;//mtk70105 add
    __u32 ui4_invalid_bytes;
	//end add by mtk40292 for APE info
} AUD_DEC_APE_INFO_T;

typedef struct _AUD_DEC_FLAC_INFO_T
{
    __u32 ui4_min_block_size;
    __u32 ui4_max_block_size;
    __u32 ui4_min_frame_size;
    __u32 ui4_max_frame_size;
    __u32 ui4_sampling_rate;
    __u16 ui2_channel_num_1;
    __u16 ui2_bits_per_sample_1;
    __u16 ui2_sample_num_high12;
    __u32 ui4_sample_num_low24;
    __u16 ui2_frame_num_high12;
    __u32 ui4_frame_num_low24;
    __u16 ui2_stream_end;
} AUD_DEC_FLAC_INFO_T;


/* SACD input format */
typedef enum
{
    AUD_DEC_SACD_INPUT_FMT_DST = 0, // 0: DST-coded
    AUD_DEC_SACD_INPUT_FMT_DSD          // 1: PlainDSD
}AUD_DEC_SACD_INPUT_FMT_T;

typedef struct _AUD_DEC_SACD_INPUT_T
{
    //AUD_DEC_AUD_TYPE_T eChNum;
    AUD_DEC_SACD_INPUT_FMT_T eDsdMode;
    bool fgTimeCodeEn;
} AUD_DEC_SACD_INPUT_T;

/* SACD output format */
typedef enum
{
    AUD_DEC_SACD_OUTPUT_FMT_DSD = 0,       // 0: DSD output
    AUD_DEC_SACD_OUTPUT_FMT_PCM,           // 1: PCM output
    AUD_DEC_SACD_OUTPUT_FMT_DST            // 2: DST output
}AUD_DEC_SACD_OUTPUT_FMT_T;

typedef enum
{
    AUD_DEC_SACD_OUTPUT_CH_2CH = 0,       // 0: 2ch
    AUD_DEC_SACD_OUTPUT_CH_MULTI            // 1: Multi-Channels
}AUD_DEC_SACD_OUTPUT_CH_T;

typedef enum
{
    AUD_DEC_SACD_EN = 0,       // 0: enable output
    AUD_DEC_SACD_PWDN            // 1: power down output
}AUD_DEC_SACD_OUTPUT_CTL;

typedef enum
{
    APLL_FS_STD = 0,       // 0: APLL standard setting
    APLL_FS_P1,            // 1: +1%
    APLL_FS_N1,		// 2 : -1%
    APLL_FS_PP1,            // 3: +0.1%
    APLL_FS_NP1		// 4 : -0.1%
}AUD_DEC_APLL_FS_CHG;

typedef enum
{
    APLL_CLK_RST = 0,		// 0: APLL standard setting
    APLL_CLK_ACC,		// 1: Accelerate clock of APLL
    APLL_CLK_DEC			// 2 : decelerate clock of APLL
}AUD_APLL_CTL_MODE;

typedef   struct  _AUD_SET_EXSPDIF_T // For Write Use
{
    __u8 u1Address; // Address
    __u8 u1Value; // value or read part
}  AUD_SET_EXSPDIF_T;

typedef   struct  _AUD_SET_EXSPDIF_INFO_T // For Read Use
{
    __u8 u1CCBAddr;
    __u8 u1ReadOutPart;
    __u8 u1DataReadOut; // Read out data
}  AUD_SET_EXSPDIF_INFO_T;

/* Digital output format related to codec */
typedef enum
{
    AUD_DEC_DIGI_OUTPUT_FMT_UNKNOWN = 0,   // 0: Unknown
    AUD_DEC_DIGI_OUTPUT_FMT_BITSTREAM,     // 1: BITSTREAM output
    AUD_DEC_DIGI_OUTPUT_FMT_PCM            // 2: PCM output
}AUD_DEC_CODEC_OUTPUT_FMT_T;

typedef enum
{
    AUD_DEC_DIGI_OUTPUT_FMT_SET_UNKNOWN = 0,   // 0: Unknown
    AUD_DEC_DIGI_OUTPUT_FMT_Set_UI_OPTION,     // 1: Set UI select
    AUD_DEC_DIGI_OUTPUT_FMT_SET_DISC_INFO      // 2: set Disc Info such as SA/IA existance
}AUD_DEC_CODEC_OUTPUT_FMT_SET_TYPE_T;

typedef struct _AUD_DEC_DISC_INFO_T
{
    bool   fg_SA_IA_Exist;
} AUD_DEC_DISC_INFO_T;

typedef   struct  _AUD_DEC_CODEC_DIGI_OUT_INFO_T
{
    AUD_DEC_CODEC_OUTPUT_FMT_SET_TYPE_T eSetType;
    bool                       fgHDMIisOFF;
    AUD_DEC_CODEC_OUTPUT_FMT_T eDolbyDigiOut;
    AUD_DEC_CODEC_OUTPUT_FMT_T eDTSDigiOut;
    AUD_DEC_DISC_INFO_T        tDiscInfo;
    bool                       fgMixerEnable;
}  AUD_DEC_CODEC_DIGI_OUT_INFO_T;

typedef struct _AUD_DEC_SACD_OUTPUT_T
{
    AUD_DEC_SACD_OUTPUT_FMT_T eOutputModeI2S;
    AUD_DEC_SACD_OUTPUT_FMT_T eOutputModeHDMI;
    __u32 u4SampleRateI2S;
    __u32 u4SampleRateHDMI;
    AUD_DEC_SACD_OUTPUT_CH_T eOutputChI2S;
    AUD_DEC_SACD_OUTPUT_CH_T eOutputChHDMI;
} AUD_DEC_SACD_OUTPUT_T;

/* DVD-Audio input format */
typedef enum
{
    AUD_DEC_DVDA_INPUT_FMT_PCM = 0,     // 0: PCM
    AUD_DEC_DVDA_INPUT_FMT_MLP          // 1: MLP
}AUD_DEC_DVDA_INPUT_FMT_T;

typedef struct _AUD_DEC_DVDA_PCM_INFO_T
{
    __u32               ui4_sample_rate;           /* audio_sampling_frequency */
    __u8                ui1_bit_depth;             /* quantization_word_length  */
} AUD_DEC_DVDA_PCM_INFO_T;

typedef struct _AUD_DEC_DVDA_INFO_T
{
    AUD_DEC_DVDA_INPUT_FMT_T eDsdMode;
    AUD_DEC_DVDA_PCM_INFO_T  t_grp0_info;
    AUD_DEC_DVDA_PCM_INFO_T  t_grp1_info;
    __u8 ui1_ats_dm_coeft[18];         //ats_dm_coeft
    bool b_stereo_prohibit;                //stereo_playback_mode ; 0:permitted;1: prohibited
    bool b_downmix_valid;               //describe down_mix_code is valid or not; 0:valid;1: invalid
    __u8 ui1_channel_assignment;       //describe channel_assignment
} AUD_DEC_DVDA_INFO_T;

typedef struct{
    __u8 u1AudioChannelCount:3;//HDMI_RX_AUDIO_CHANNEL_COUNT_T
    __u8 u1DM_INH:1;// 1:Down mix Prohibites, 0:Permitted down mix
    __u8 u1AudioCodingType:4; //see HDMI_RX_AUDIO_CODING_TYPE_T
    __u8 u1LevelShiftValue:4;//HDMI_RX_AUD_LEVEL_TYPE_T
    __u8 u1SampleSize:2;//HDMI_RX_SAMPLE_SIZE_TYPE_T
    __u8 u1SpeakerPlacement;//HDMI_RX_SPEAKER_ALLOCATE_T
    __u8 u1AudinSampleRate;  // 0~6 : Out of range ; 7~0xF : 32/44.1/48/64/88.2/96/128/176.4/192 KHz
}AUD_DEC_HDMI_IN_PCM_INFO_T;


typedef struct
{
    AUD_DEC_FMT_T        e_aud_fmt;
    AUD_DEC_AUD_TYPE_T   e_aud_type;
    __u32               ui4_sample_rate;
    __u32               ui4_data_rate;
    __u8                ui1_bit_depth;
    union
    {
        AUD_DEC_PCM_INFO_T* __local_space__   pt_pcm_info;
        AUD_DEC_HDMI_IN_PCM_INFO_T*  __local_space__  pt_hdmi_in_pcm_info;
        AUD_DEC_WMA_INFO_T* __local_space__   pt_wma_info;
        AUD_DEC_SACD_INPUT_T* __local_space__ pt_sacd_info;
        AUD_DEC_DVDA_INFO_T* __local_space__  pt_dvda_info;
        AUD_DEC_RA_COOK_INFO_T* __local_space__  pt_ra_cook_info;
		AUD_DEC_APE_INFO_T* __local_space__  pt_ape_info;   //mtk70169 mark
        AUD_DEC_FLAC_INFO_T* __local_space__  pt_flac_info;
    } u_fmt_spec;
    __u16               ui2_pid;               //Only use for BDMW currently
    __u32               u4ChannelLayout;       //for detail channel layout
} AUD_DEC_AUD_INFO_T;

typedef enum
{
    AUD_DEC_SPEED_TYPE_SF_00_03X           = 3,       /* HBI_SpeedSlow5(1/32X) */
    AUD_DEC_SPEED_TYPE_SF_00_06X           = 6,       /* HBI_SpeedSlow4(1/16X) */
    AUD_DEC_SPEED_TYPE_SF_00_13X           = 13,     /* HBI_SpeedSlow3(1/8X) */
    AUD_DEC_SPEED_TYPE_SF_00_25X           = 25,     /* HBI_SpeedSlow2(1/4X) */
    AUD_DEC_SPEED_TYPE_SF_00_50X           = 50,     /* HBI_SpeedSlow1(1/2X) */
    AUD_DEC_SPEED_TYPE_SF_00_60X           = 60,
    AUD_DEC_SPEED_TYPE_SF_00_70X           = 70,
    AUD_DEC_SPEED_TYPE_SF_00_80X           = 80,
    AUD_DEC_SPEED_TYPE_SF_00_90X           = 90,
    AUD_DEC_SPEED_TYPE_NORMAL              = 100,
    AUD_DEC_SPEED_TYPE_FF_01_10X           = 110,
    AUD_DEC_SPEED_TYPE_FF_01_20X           = 120,
    AUD_DEC_SPEED_TYPE_FF_01_30X           = 130,
    AUD_DEC_SPEED_TYPE_FF_01_40X           = 140,
    AUD_DEC_SPEED_TYPE_FF_01_50X           = 150,
    AUD_DEC_SPEED_TYPE_FF_02_00X           = 200,    /* HBI_SpeedFast1(2X) */
    AUD_DEC_SPEED_TYPE_FF_04_00X           = 400,    /* HBI_SpeedFast2(4X) */
    AUD_DEC_SPEED_TYPE_FF_08_00X           = 800,    /* HBI_SpeedFast3(8X) */
    AUD_DEC_SPEED_TYPE_FF_16_00X           = 1600,   /* HBI_SpeedFast4(16X) */
    AUD_DEC_SPEED_TYPE_FF_32_00X           = 3200    /* HBI_SpeedFast5(32X) */
}AUD_DEC_PB_SPEED_TYPE_T;

typedef struct {
    AUD_DEC_FMT_T           e_aud_fmt;
    AUD_DEC_AUD_TYPE_T      e_aud_type;
    __u32                  ui4_sample_rate;
    __u32                  ui4_data_rate;
    __u8                   ui1_bit_depth;
    AUD_DEC_RA_COOK_INFO_T  pt_ra_cook_info;
    AUD_DEC_PCM_INFO_T      pcm_info;
    AUD_DEC_WMA_INFO_T      wma_info;
    AUD_DEC_APE_INFO_T      ape_info;
    AUD_DEC_FLAC_INFO_T     flac_info;
    __u16                   ui2_pid;
} AUD_INFO_T;


typedef struct
{
    AUD_DEC_FMT_T       eAudDecFmt;
    AUD_INFO_T* 	    prInfo;
    AUD_DEC_PB_SPEED_TYPE_T eSpeed;
}AUD_DEC_AUDIO_PB_INFO_T;

typedef enum
{
    AUD_DEC_SEC_AUDIO_ON = 0,
    AUD_DEC_SEC_AUDIO_OFF
}AUD_DEC_SEC_AUDIO_TYPE_T;

/* Loudspeader modes. */
typedef enum
{
    AUD_DEC_LS_MODE_OFF = 0,
    AUD_DEC_LS_MODE_LARGE_FREQ_RANGE,
    AUD_DEC_LS_MODE_SMALL_FREQ_RANGE
}   AUD_DEC_LS_MODE_T;


/* Loudspeader channel types. */
typedef enum
{
    AUD_DEC_LS_FRONT_LEFT = 0,
    AUD_DEC_LS_FRONT_RIGHT,
    AUD_DEC_LS_REAR_LEFT,
    AUD_DEC_LS_REAR_RIGHT,
    AUD_DEC_LS_CENTER,
    AUD_DEC_LS_SUB_WOOFER,
    AUD_DEC_LS_SPK_AUX1,                  //back left
    AUD_DEC_LS_SPK_AUX2,                  //back right
    AUD_DEC_LS_SPK_DOWNMIX_LEFT,
    AUD_DEC_LS_SPK_DOWNMIX_RIGHT,
    AUD_DEC_LS_SPK_ALL
}   AUD_DEC_LS_T;

/* Loudspeader types. */
typedef enum
{
    AUD_DEC_LS_OUT_MODE_DEFAULT = 0,
    AUD_DEC_LS_OUT_MODE_HDMI
}   AUD_DEC_LS_OUT_MODE_T;

//#define AUD_DEV_LS_FLAG_NONE             ((_u32) (0) )
//#define AUD_DEC_LS_FLAG_FRONT_LEFT       MAKE_BIT_MASK_32(0)
//#define AUD_DEC_LS_FLAG_FRONT_RIGHT      MAKE_BIT_MASK_32(1)
//#define AUD_DEC_LS_FLAG_REAR_LEFT        MAKE_BIT_MASK_32(2)
//#define AUD_DEC_LS_FLAG_REAR_RIGHT       MAKE_BIT_MASK_32(3)
//#define AUD_DEC_LS_FLAG_CENTER           MAKE_BIT_MASK_32(4)
//#define AUD_DEC_LS_FLAG_SUB_WOOFER       MAKE_BIT_MASK_32(5)


/* Sound Effects. */
typedef enum
{
    AUD_DEC_SE_TRIM = 0,
    AUD_DEC_SE_DELAY,
    AUD_DEC_SE_SUPERBASS,
    AUD_DEC_SE_EQUALIZER,
    AUD_DEC_SE_REVERB,
    AUD_DEC_SE_BASS,
    AUD_DEC_SE_TREBLE,
    AUD_DEC_SE_BALANCE,
    AUD_DEC_SE_POSTDRC,
    AUD_DEC_SE_VOLUME,
    AUD_DEC_SE_SURROUND,
    AUD_DEC_SE_PROLOG_II,
    AUD_DEC_SE_MP3BOOSTER,      // (aud_se_mp3booster_v2)
    AUD_DEC_SE_DBASS,           // (aud_se_dbass_v2)
    AUD_DEC_SE_SWAP             // (aud_se_swap_v2)
}   AUD_DEC_SOUND_EFFECT_T;

/*Aud Chann Mode.*/
typedef enum
{
	AUD_DEC_CHANN_7_1_MODE = 0,
	AUD_DEC_CHANN_5_1_MODE,
	AUD_DEC_CHANN_2_MODE
}AUD_DEC_CHANN_MODE;

/*Downmix lr type.*/
typedef enum
{
    AUD_DEC_DM_LORO = 0,
    AUD_DEC_DM_LTRT
} AUD_DEC_DOWNMIX_LR_T;




/* Dolby Dynamice Range Control. */
typedef enum
{
    AUD_DEC_DRC_OFF = 0,
    AUD_DEC_DRC_1_8,
    AUD_DEC_DRC_1_4,
    AUD_DEC_DRC_3_8,
    AUD_DEC_DRC_1_2,
    AUD_DEC_DRC_5_8,
    AUD_DEC_DRC_3_4,
    AUD_DEC_DRC_7_8,
    AUD_DEC_DRC_FULL
}   AUD_DEC_DRC_T;

/*media type define*/
typedef enum
{
    AUD_OUT_MEDIA_NONE = 0,
    AUD_OUT_MEDIA_USB,
    AUD_OUT_MEDIA_LINE_IN,
    AUD_OUT_MEDIA_DVD,
    AUD_OUT_MEDIA_LINE_IN2,
    AUD_OUT_MEDIA_UNDEF
}   AUD_OUT_MEDIA_TYPE_T;

typedef enum
{
    AUD_FIFO_RPIMARY = 0,
    AUD_FIFO_HDMI_RX = 1,
    AUD_FIFO_UNDEF
}   AUD_FIFO_TYPE_T;

/* Sample bits. */
typedef enum
{
    AUD_DEC_SAMPLE_BITS_16 = 0,
    AUD_DEC_SAMPLE_BITS_18,
    AUD_DEC_SAMPLE_BITS_20,
    AUD_DEC_SAMPLE_BITS_24
}   AUD_DEC_SAMPLE_BITS_T;


/* Sample frequency. */
typedef enum
{
    AUD_DEC_SAMPLE_FREQ_8 = 0,
    AUD_DEC_SAMPLE_FREQ_11_025,
    AUD_DEC_SAMPLE_FREQ_12,
    AUD_DEC_SAMPLE_FREQ_16,
    AUD_DEC_SAMPLE_FREQ_22_050,
    AUD_DEC_SAMPLE_FREQ_24,
    AUD_DEC_SAMPLE_FREQ_32,
    AUD_DEC_SAMPLE_FREQ_44_100,
    AUD_DEC_SAMPLE_FREQ_48,
    AUD_DEC_SAMPLE_FREQ_96
}   AUD_DEC_SAMPLE_FREQ_T;


/*The tuned volume types */
typedef enum
{
    AUD_DEC_ALL_CH = 0,
    AUD_DEC_INDIVIDUAL_CH
}   AUD_DEC_VOL_TYPE_T;




typedef enum
{
    AUD_DEC_HDMI_OUTPUT_OFF = 0,
    AUD_DEC_HDMI_OUTPUT_PCM,
    AUD_DEC_HDMI_OUTPUT_BITSTREAM,
    AUD_DEC_HDMI_OUTPUT_UNKNOWN
}   AUD_DEC_HDMI_OUTPUT_MODE_T;

typedef struct
{
    AUD_DEC_AUD_TYPE_T eChannel_Layout;
    __u8 u1Front;
    __u8 u1Middle;
    __u8 u1Rear;
    __u8 u1Sw;
    __u8 u1Others;
} AUD_DEC_INPUT_CHANNEL_LAYOUT_T;


/*
0: IEC select L/R as output
1: IEC select Ls/Rs as output
2: IEC select C/LFE as output
3 :IEC select Ch7/Ch8 as output
4: IEC select SPDIF /LINE in as output
5: IEC select Ch9/Ch10 as output
6: IEC select Ch11/Ch12 as output
7: IEC select microphone as output
*/
typedef enum
{
    AUD_DEC_SPDIF_OUTPUT_L_R = 0,
    AUD_DEC_SPDIF_OUTPUT_LS_RS,
    AUD_DEC_SPDIF_OUTPUT_C_LFE,
    AUD_DEC_SPDIF_OUTPUT_CH7_CH8,
    AUD_DEC_SPDIF_OUTPUT_LINE_IN,
    AUD_DEC_SPDIF_OUTPUT_CH9_CH10,
    AUD_DEC_SPDIF_OUTPUT_CH11_CH12,
    AUD_DEC_SPDIF_OUTPUT_MIC
}AUD_DEC_SPDIF_OUTPUT_T;

typedef enum
{
    AUD_DEC_SPDIF_INPUT_OFF = 0,
    AUD_DEC_SPDIF_INPUT_ON
}AUD_DEC_SPDIF_INPUT_T;

/*--------------ATS---------------------*/

/* switch operation value */
typedef enum
{
    AUD_SE_ATS_SWITCH_OFF = 0,
    AUD_SE_ATS_SWITCH_ON
}   AUD_SE_ATS_SWITCH_T;

typedef struct AUD_SE_ATS_COEF_T
{
    __u32 u4CtrlMode;
    __u32 u4InputGain;
    __u32 u4CenterGain;
    __u32 u4LRGain;
    __u32 u4LsRsGain;
    __u32 u4LfeGain;
    __u32 u4CenterInGain;
    __u32 u4LfeInGain;
    __u32 u4LsRsInGain;
    __u32 u4C2LRGain;
    __u32 u4C2LsRsGain;
    __u32 u4Lr2LsRsGain;
    __u32 u4OverallDelay;
    __u32 u4FrontSpkSize;
    __u32 u4SurrSpkSize;

    __u32 u4FrontBassLevel ;
    __u32 u4SurrBassLevel;
    __u32 u4FrontMiddleLevel;
    __u32 u4SurrMiddleLevel;
} AUD_SE_ATS_COEF_T;

typedef enum
{
    AUD_SE_ATS_CTRL_SWITCH = 0,
    AUD_SE_ATS_CTRL_MODE,
    AUD_SE_ATS_INPUT_GAIN,
    AUD_SE_ATS_CENTER_OUTPUT_GAIN ,
    AUD_SE_ATS_LR_OUTPUT_GAIN ,
    AUD_SE_ATS_LSRS_OUTPUT_GAIN ,
    AUD_SE_ATS_LFE_OUTPUT_GAIN ,
    AUD_SE_ATS_CENTER_INPUT_GAIN ,
    AUD_SE_ATS_LFE_INPUT_GAIN ,
    AUD_SE_ATS_LSRS_INPUT_GAIN ,
    AUD_SE_ATS_C_2_LR_GAIN ,
    AUD_SE_ATS_C_2_LSRS_GAIN ,
    AUD_SE_ATS_LR_2_LSRS_GAIN ,
    AUD_SE_ATS_SURROUND_DELAY,
    AUD_SE_ATS_FRONT_SPK_SIZE ,
    AUD_SE_ATS_SURR_SPK_SIZE ,
    AUD_SE_ATS_FRONT_BASS_LEVEL ,
    AUD_SE_ATS_SURR_BASS_LEVEL ,
    AUD_SE_ATS_FRONT_MIDDLE_LEVEL ,
    AUD_SE_ATS_SURR_MIDDLE_LEVEL ,
    AUD_SE_ATS_SETTING_END
}   AUD_SE_ATS_CTRL_T;

typedef struct AUD_SE_ATS_CTRL_INFO_TAG
{
    AUD_SE_ATS_CTRL_T    e_ctrlID; /* IN */
    union {
        AUD_SE_ATS_SWITCH_T e_ats_switch;
        __u32 u4CtrlMode;
        __u32 u4InputGain;
        __u32 u4CenterGain;
        __u32 u4LRGain;
        __u32 u4LsRsGain;
        __u32 u4LfeGain;
        __u32 u4CenterInGain;
        __u32 u4LfeInGain;
        __u32 u4LsRsInGain;
        __u32 u4C2LRGain;
        __u32 u4C2LsRsGain;
        __u32 u4Lr2LsRsGain;
        __u32 u4OverallDelay;
        __u32 u4FrontSpkSize;
        __u32 u4SurrSpkSize;
        __u32 u4FrontBassLevel;
        __u32 u4SurrBassLevel;
        __u32 u4FrontMiddleLevel;
        __u32 u4SurrMiddleLevel;
    } u;
} AUD_SE_ATS_CTRL_INFO_T;

#if 0
/* Capability channels. */
#define AUD_DEC_CAP_TYPE_UNKNOWN     (((u32) 1) << AUD_DEC_TYPE_UNKNOWN)
#define AUD_DEC_CAP_TYPE_MONO        (((u32) 1) << AUD_DEC_TYPE_MONO)
#define AUD_DEC_CAP_TYPE_DUAL_MONO   (((u32) 1) << AUD_DEC_TYPE_DUAL_MONO)
#define AUD_DEC_CAP_TYPE_STEREO      (((u32) 1) << AUD_DEC_TYPE_STEREO)
#define AUD_DEC_CAP_TYPE_STEREO_DOLBY_SURROUND      (((u32) 1) << AUD_DEC_TYPE_STEREO_DOLBY_SURROUND)
#define AUD_DEC_CAP_TYPE_3_0         (((u32) 1) << AUD_DEC_TYPE_3_0)
#define AUD_DEC_CAP_TYPE_4_0         (((u32) 1) << AUD_DEC_TYPE_4_0)
#define AUD_DEC_CAP_TYPE_5_1         (((u32) 1) << AUD_DEC_TYPE_5_1)
#define AUD_DEC_CAP_TYPE_7_1         (((u32) 1) << AUD_DEC_TYPE_7_1)
#define AUD_DEC_CAP_TYPE_OTHERS      (((u32) 1) << AUD_DEC_TYPE_OTHERS)


/* Capability downmix mode. */
#define AUD_DEC_CAP_DM_OFF           (((u32) 1) << AUD_DEC_DM_OFF)
#define AUD_DEC_CAP_DM_LT_RT         (((u32) 1) << AUD_DEC_DM_LT_RT)
#define AUD_DEC_CAP_DM_STEREO        (((u32) 1) << AUD_DEC_DM_STEREO)
#define AUD_DEC_CAP_DM_VIR_SURR      (((u32) 1) << AUD_DEC_DM_VIR_SURR)


/* Capability sound effect. */
#define AUD_DEC_CAP_SE_TRIM          (((u32) 1) << AUD_DEC_SE_TRIM)
#define AUD_DEC_CAP_SE_DELAY         (((u32) 1) << AUD_DEC_SE_DELAY)
#define AUD_DEC_CAP_SE_SUPERBASS     (((u32) 1) << AUD_DEC_SE_SUPERBASS)
#define AUD_DEC_CAP_SE_EQUALIZER     (((u32) 1) << AUD_DEC_SE_EQUALIZER)
#define AUD_DEC_CAP_SE_REVERB        (((u32) 1) << AUD_DEC_SE_REVERB)
#define AUD_DEC_CAP_SE_BASS          (((u32) 1) << AUD_DEC_SE_BASS)
#define AUD_DEC_CAP_SE_TREBLE        (((u32) 1) << AUD_DEC_SE_TREBLE)
#define AUD_DEC_CAP_SE_BALANCE       (((u32) 1) << AUD_DEC_SE_BALANCE)
#define AUD_DEC_CAP_SE_POSTDRC       (((u32) 1) << AUD_DEC_SE_POSTDRC)
#define AUD_DEC_CAP_SE_VOLUME        (((u32) 1) << AUD_DEC_SE_VOLUME)
#define AUD_DEC_CAP_SE_SURROUND      (((u32) 1) << AUD_DEC_SE_SURROUND)


/* Capability format. */
#define AUD_DEC_CAP_FMT_MPEG           (((u32) 1) << AUD_DEC_FMT_MPEG)
#define AUD_DEC_CAP_FMT_AC3            (((u32) 1) << AUD_DEC_FMT_AC3)
#define AUD_DEC_CAP_FMT_PCM            (((u32) 1) << AUD_DEC_FMT_PCM)
#define AUD_DEC_CAP_FMT_MP3            (((u32) 1) << AUD_DEC_FMT_MP3)
#define AUD_DEC_CAP_FMT_AAC            (((u32) 1) << AUD_DEC_FMT_AAC)
#define AUD_DEC_CAP_FMT_DTS            (((u32) 1) << AUD_DEC_FMT_DTS)
#define AUD_DEC_CAP_FMT_WMA            (((u32) 1) << AUD_DEC_FMT_WMA)
#define AUD_DEC_CAP_FMT_RA             (((u32) 1) << AUD_DEC_FMT_RA)
#define AUD_DEC_CAP_FMT_HDCD           (((u32) 1) << AUD_DEC_FMT_HDCD)
#define AUD_DEC_CAP_FMT_MLP            (((u32) 1) << AUD_DEC_FMT_MLP)
#define AUD_DEC_CAP_FMT_MTS            (((u32) 1) << AUD_DEC_FMT_MTS)
#define AUD_DEC_CAP_FMT_EU_CANAL_PLUS  (((u32) 1) << AUD_DEC_FMT_EU_CANAL_PLUS)
#define AUD_DEC_CAP_FMT_TV_SYS         (((u32) 1) << AUD_DEC_FMT_TV_SYS)

/*  gain array*/
/*fixed point format :b0 ~b15 => fraction portion, b16~b31 => integer portion*/
typedef struct _AUD_DEC_GAIN_ARRAY_T
{
    __u32              ui4_L;
    __u32              ui4_R;
    __u32              ui4_C;
    __u32              ui4_LS;
    __u32              ui4_RS;
    __u32              ui4_LFE;
    __u32              ui4_AUX1;
    __u32              ui4_AUX2;
}AUD_DEC_GAIN_ARRAY_T;

/*  gain infomation*/
/* The linear gain converted to a fractional fixed point unsigned integer, as described
 * in [BD-ROM3] part 1 I.4. The value to pass is the linear gain multiplied by 8192, and
 * rounded to the closest integer
 *ui4_gain    b0 ~b15 => fraction portion, b16~b31 => integer portion */
typedef u32 AUD_DEC_MIX_GAIN_INFO_T;
typedef struct AUD_DEC_PANNING_INFO_T
{
    __s32 i4_0;
    __s32 i4_1;
    __s32 i4_2;
    __s32 i4_3;
    __s32 i4_4;
    __s32 i4_5;
    __s32 i4_6;
}AUD_DEC_PANNING_INFO_T;

typedef u32 AUD_DEC_WRITE_PTR_T;
typedef struct
{
    __u32         u4WritePointer;
    __s32           i4WriteLength;
}AUD_DEC_WRITE_INFO_T;



typedef struct
{
    __u32         u4EffsndStrmBufSA;
    __u32         u4EffsndStrmBufSize;
}
AUD_DEC_STRM_BUF_INFO_T;


/* capability info. */
typedef struct _AUD_DEC_CAPABILITY_INFO_T
{
    __u32              ui4_fmt;
    __u32              ui4_channel_type;
    __u32              ui4_sound_effect;
    __u32              ui4_downmix_mode;
    bool                b_is_outport_adj;
}   AUD_DEC_CAPABILITY_INFO_T;

#endif

/* Notify function */
typedef void (*x_aud_dec_nfy_fct) (void*       pv_nfy_tag,
                                   AUD_DEC_COND_T  e_nfy_cond,
                                   __u32          ui4_data_1,
                                   __u32          ui4_data_2 );

/* Notify setting info. */
typedef struct _AUD_DEC_NFY_INFO_T
{
    void*               pv_tag;
    x_aud_dec_nfy_fct   pf_aud_dec_nfy;
}   AUD_DEC_NFY_INFO_T;




/* Decode format setting info. */
typedef struct _AUD_DEC_FMT_INFO_T
{
    AUD_DEC_FMT_T               e_fmt;

    void* __local_space__       pv_info;
}   AUD_DEC_FMT_INFO_T;


/* Channel volume gain info. */
typedef struct _AUD_DEC_CH_VOL_GAIN_T
{
    __u32  u4VolL;
    __u32  u4VolR;
    __u32  u4VolSL;
    __u32  u4VolSR;
    __u32  u4VolC;
    __u32  u4VolSW;
    __u32 u4VolMaster;
}   AUD_DEC_CH_VOL_GAIN_T;

/* Individual channel volume gain setting. */
typedef struct _AUD_DEC_CH_VOLUME_GAIN_T
{
    AUD_DEC_OUT_PORT_T  e_out_port;
    AUD_DEC_LS_T        e_ls;

    __u32              u4FrontChVolGain;
}   AUD_DEC_CH_VOLUME_GAIN_T;


typedef struct _AUD_DEC_VOLUME_GAN_INFO_T
{
    AUD_DEC_VOL_TYPE_T            e_vol_type;

    union
    {
        __u32                   u4FrontMasterVolGain;
        AUD_DEC_CH_VOLUME_GAIN_T     t_ch_gain_vol;
    } u;
}   AUD_DEC_VOLUME_GAIN_INFO_T;




typedef struct _AUD_DEC_REAR_VOLME_GAIN_INFO_T
{
	__u32				 u4RearVolGain;
}AUD_DEC_REAR_VOLUME_GAIN_INFO_T;
//end vol gain set




/* Channel volume info. */
typedef struct _AUD_DEC_CH_VOL_T
{
    __u8  u1VolL;
    __u8  u1VolR;
    __u8  u1VolSL;
    __u8  u1VolSR;
    __u8  u1VolC;
    __u8  u1VolSW;
    __u8  u1VolMaster;
}   AUD_DEC_CH_VOL_T;

/* Individual channel volume gain setting. */
typedef struct _AUD_DEC_CH_VOLUME_T
{
    AUD_DEC_OUT_PORT_T  e_out_port;
    AUD_DEC_LS_T        e_ls;

    __u8               ui1_level;
}   AUD_DEC_CH_VOLUME_T;


/* Volume level setting info. */
typedef struct _AUD_DEC_VOLUME_INFO_T
{
    AUD_DEC_VOL_TYPE_T            e_vol_type;

    union
    {
        __u8                   ui1_level;
        AUD_DEC_CH_VOLUME_T     t_ch_vol;
    } u;
}   AUD_DEC_VOLUME_INFO_T;

//rear volume level setting info
typedef struct _AUD_DEC_REAR_VOLME_INFO_T
{
    __u8                ui1_level;
}AUD_DEC_REAR_VOLUME_INFO_T;

/* Individual channel trim level setting. */
typedef struct _AUD_DEC_CH_TRIM_T
{
    AUD_DEC_LS_OUT_MODE_T e_mode;
    AUD_DEC_LS_T    e_ls;
    __u8           ui1_level;
}   AUD_DEC_CH_TRIM_T;


/* Loudspeader channel types. */
typedef enum
{
    AUD_LS_FRONT_L = 0,
    AUD_LS_FRONT_R,
    AUD_LS_FRONT_LS,
    AUD_LS_FRONT_RS,
    AUD_LS_FRONT_C,
    AUD_LS_FRONT_SUB,
    AUD_LS_FRONT_SPDIF_PCM_L,
    AUD_LS_FRONT_SPDIF_PCM_R,
    AUD_LS_REAR_L,
    AUD_LS_REAR_R
}   AUD_LS_CH_T;

/* Individual channel delay level setting. */
typedef struct _AUD_DEC_CH_DELAY_T
{
    AUD_LS_CH_T    e_ls;
    __u16          ui2_unit_0_1ms;
}   AUD_DEC_CH_DELAY_T;


/* audio presentation types, mono / stereo */
typedef enum
{
    AUD_DEC_CHANNEL_TYPE_MONO = 0,
    AUD_DEC_CHANNEL_TYPE_STEREO,
    AUD_DEC_CHANNEL_TYPE_UNKNOWN
}   AUD_DEC_CHANNEL_TYPE_T;


/* AUD_DEC_GET_TYPE_AV_SYNC/AUD_DEC_SET_TYPE_AV_SYNC *************************/
typedef enum
{
    AUD_DEC_AV_SYNC_FREE_RUN = 0,
    AUD_DEC_AV_SYNC_AUD_SLAVE,
    AUD_DEC_AV_SYNC_AUD_MASTER
} AUD_DEC_AV_SYNC_INFO_T;


/* AUD_DEC_SET_TYPE_DOLBY_KARA_MODE ******************************************/
/* Dolby Digital karaoke mode. */
typedef enum
{
    AUD_DEC_AC3_KARA_DISABLE = 0x0001,
    AUD_DEC_AC3_KARA_AWARE   = 0x0002,
    AUD_DEC_AC3_KARA_NONE    = 0x0004,
    AUD_DEC_AC3_KARA_V1      = 0x0008,
    AUD_DEC_AC3_KARA_V2      = 0x0010,
    AUD_DEC_AC3_KARA_BOTH    = 0x0020,
    AUD_DEC_AC3_NO_MELODY    = 0x0080
}   AUD_DEC_DOLBY_KARA_MODE_T;


/* AUD_DEC_SET_TYPE_DTS_DRC **************************************************/
/* DTS DRC Setting*/
typedef enum
{
    AUD_DEC_DTS_DRC_OFF,
    AUD_DEC_DTS_DRC_MODE_0,
    AUD_DEC_DTS_DRC_MODE_1,
    AUD_DEC_DTS_DRC_MODE_2,
    AUD_DEC_DTS_DRC_MODE_3,
    AUD_DEC_DTS_DRC_MODE_4,
    AUD_DEC_DTS_DRC_MODE_5,
    AUD_DEC_DTS_DRC_MODE_6,
    AUD_DEC_DTS_DRC_MODE_7,
    AUD_DEC_DTS_DRC_MODE_8,
    AUD_DEC_DTS_DRC_MODE_9
}   AUD_DEC_DTS_DRC_MODE_T;


/* AUD_DEC_SET_TYPE_HDCD_CFG *************************************************/
/* HDCD Config Bitmask */
typedef enum
{
    AUD_DEC_HDCD_ENABLE_BIT,                //bit 0: hdcd enable/disable
    AUD_DEC_HDCD_AUTOLEVEL_BIT,             //bit 1: autolevel flag
    AUD_DEC_HDCD_DITHER_BIT,                //bit 2: dither on/off
    AUD_DEC_HDCD_HDCD_FLTR_ON_NONHDCD_BIT   //bit 3: do hdcd filter on non-hdcd
}   AUD_DEC_HDCD_CFG_BITMASK_T;


/* AUD_DEC_SET_TYPE_HDCD_DITHER_MODE *****************************************/
/* HDCD Dither Modes */
typedef enum
{
    AUD_DEC_HDCD_DITHER_OFF = 0,
    AUD_DEC_HDCD_DITHER_LVL1,
    AUD_DEC_HDCD_DITHER_LVL2,
    AUD_DEC_HDCD_DITHER_LVL3,
    AUD_DEC_HDCD_DITHER_LVL4,
    AUD_DEC_HDCD_DITHER_LVL5,
    AUD_DEC_HDCD_DITHER_LVL6,
    AUD_DEC_HDCD_DITHER_LVL7
}   AUD_DEC_HDCD_DITHER_MODE_T;





/* AUD_DEC_SET_TYPE_SPEAKER_LAYOUT *******************************************/
/* Speaker Types */
typedef struct
{
    __u8               ui1_total_spk_num;
   /*
    * ui8_spk_layout format definition:
    * ---------
    * bit0 ~ bit2 (2~5ch)
    * 0: LT/RT                    bit 3: CB (ch6)
    * 1: Mono                     bit 4: ch7 exist or not
    * 2: Stereo                   bit 5: subwoofer exist or not
    * 3: L/R/C
    * 4: L/R/S
    * 5: L/R/C/S
    * 6. L/R/LS/RS
    * 7: L/R/C/LS/RS (over 7 ch, bit 0~2 should be set as 7)
    *
    * bit 12: Center Channel large(1)/small(0)
    * bit 13: Left Channel large(1)/small(0)
    * bit 14: Right Channel large(1)/small(0)
    * bit 15: Left Surround Channel large(1)/small(0)
    * bit 16: Right Surround Channel large(1)/small(0)
    * bit 17: Center Back Channel large(1)/small(0)
    * bit 18: No.7 Channel large(1)/small(0)
    *
    * bit32 ~ bit63
    * represent the channel set
    * 0: no remapping is required
    * Other config is followed the rule below:
    * bit 32: Center exist          bit 40: Overhead (Oh)
    * bit 33: LR                    bit 41: LC/RC
    * bit 34: LS/RS                 bit 42: LW/RW
    * bit 35: LFE                   bit 43: LSS/RSS
    * bit 36: CS (CB)               bit 44: LFE2
    * bit 37: Lh/Rh                 bit 45: LHS/RHS
    * bit 38: LSR/RSR               bit 46: CHR
    * bit 39: Center high (Ch)      bit 47: LHR/RHR
    */
    __u64              ui8_spk_layout;
    __u16              ui2_front_size;
    __u16              ui2_center_size;
    __u16              ui2_rear_size;
    __u16              ui2_sub_size;
    __u32              ui4_sub_force_out;
}   AUD_DEC_SPEAKER_LAYOUT_T;


typedef enum{
    AUD_AADC_LINEIN_INPUT_START,
    AUD_AADC_LINEIN_INPUT_GROUP1 ,
    AUD_AADC_LINEIN_INPUT_GROUP2,
    AUD_AADC_LINEIN_INPUT_GROUP3,
    AUD_AADC_LINEIN_INPUT_GROUP4,
    AUD_AADC_LINEIN_INPUT_GROUP5,
    AUD_AADC_LINEIN_INPUT_UNDEF
} AUD_AADC_LINEIN_GROUP_E;


/* SPDIF LPCM output types. */
typedef enum
{
    AUD_DEC_SPDIF_LPCM_48K = 0,
    AUD_DEC_SPDIF_LPCM_96K,
    AUD_DEC_SPDIF_LPCM_192K,
    AUD_DEC_SPDIF_LPCM_48K_16BIT  // 48kHz, 16bit @ 01/16/2009
}   AUD_DEC_SPDIF_LPCM_TYPE_T;

/* SPDIF AUDIO IN TYPE */
typedef enum
{
	AUD_DEC_SPDIF_INPUT_CD_DISC= 0,
	AUD_DEC_SPDIF_INPUT_LINE_IN
}AUD_DEC_SPDIF_INPUT_MODE_T;


/* DTS Audio Presentation format */
typedef enum
{
    AUD_DEC_DTS_AUD_PRESENTATION_DEFAULT = 0,
    AUD_DEC_DTS_AUD_PRESENTATION_MODE1
}   AUD_DEC_DTS_AUD_PRESENTATION_TYPE_T;


/* Dolby DUAL MONO format */
typedef enum
{
    AUD_DEC_DOLBY_DM_STEREO = 0, // Stereo
    AUD_DEC_DOLBY_DM_DUAL1,  // L-MONO
    AUD_DEC_DOLBY_DM_DUAL2,  // R-MONO
    AUD_DEC_DOLBY_DM_DUAL3   // Mix-MONO
} AUD_DEC_DOLBY_DUAL_MONO_TYPE_T;

/* Bass Management mode. */
typedef enum
{
    AUD_DEC_BASS_MANAGEMENT_LFE = 0,
    AUD_DEC_BASS_MANAGEMENT_LFE_MAIN
}   AUD_DEC_BASS_MANAGEMENT_MODE_T;

/* Karaoke output mode */
typedef enum
{
     AUD_DEC_KARA_OUTPUT_FMT_STEREO = 0,
     AUD_DEC_KARA_OUTPUT_FMT_MIX,
     AUD_DEC_KARA_OUTPUT_FMT_LEFT,
     AUD_DEC_KARA_OUTPUT_FMT_RIGHT,
     AUD_DEC_KARA_OUTPUT_FMT_VOCALMUTE
} AUD_DEC_KARA_OUTPUT_T;

// Add by mtk40292 for LR mix request
typedef enum
{
     AUD_DEC_LRMIX_OUTPUT_FMT_STEREO = 0,
     AUD_DEC_LRMIX_OUTPUT_FMT_LEFT,
     AUD_DEC_LRMIX_OUTPUT_FMT_RIGHT
} AUD_DEC_LRMIX_OUTPUT_T;

/* Neo6 output mode */
typedef enum
{
     AUD_DEC_NEO6_STATUS_UNKNOWN = 0,
     AUD_DEC_NEO6_STATUS_OFF,
     AUD_DEC_NEO6_STATUS_ON_CINEMA,
     AUD_DEC_NEO6_STATUS_ON_MUSIC
} AUD_DEC_NEO6_STATUS_T;

// UI_default
typedef struct _AUD_DEC_CH_DEFAULT_T
{
    AUD_DEC_CH_TRIM_T t_se_trim;
    AUD_DEC_CH_DELAY_T t_se_delay;
}   AUD_DEC_CH_DEFAULT_T;

typedef enum
{
     AUD_DEC_RESET_AOUT = 0,
     AUD_DEC_RESET_OTHERS
} AUD_DEC_RESET_T;

// UI_default
/* AUD_DEC_SET_TYPE_UI_DEFAULT *******************************************/
typedef struct
{
    AUD_DEC_SPEAKER_LAYOUT_T e_spk_layout;
    AUD_DEC_CH_DEFAULT_T t_ch_l;
    AUD_DEC_CH_DEFAULT_T t_ch_r;
    AUD_DEC_CH_DEFAULT_T t_ch_c;
    AUD_DEC_CH_DEFAULT_T t_ch_ls;
    AUD_DEC_CH_DEFAULT_T t_ch_rs;
    AUD_DEC_CH_DEFAULT_T t_ch_ext1;
    AUD_DEC_CH_DEFAULT_T t_ch_ext2;
    AUD_DEC_CH_DEFAULT_T t_ch_sub;
}   AUD_DEC_UI_DEFAULT_T;

// AAC_support_DSP
/* AAC Dual Mono mode. */
typedef enum
{
    AUD_DEC_AAC_DM_STEREO = 0,
    AUD_DEC_AAC_DM_L_MONO,
    AUD_DEC_AAC_DM_R_MONO,
    AUD_DEC_AAC_DM_M_MONO,
}   AUD_DEC_AAC_DM_MODE_T;

/* for test tone */
typedef enum
{
    AUD_DEC_TEST_TONE_PINK_NOISE,
    AUD_DEC_TEST_TONE_TRIANGLE_WAVE,
    AUD_DEC_TEST_TONE_SINE_WAVE,
    AUD_DEC_TEST_TONE_WHITE_NOISE,
    AUD_DEC_TEST_TONE_PINK_NOISE_DOLBY  // pink noise with Dolby required level
} AUD_DEC_TEST_TONE_TYPE_T;

// Test tone
typedef enum
{
    AUD_DEC_TESTTONE_OUTPUT_ALL = 0,
    AUD_DEC_TESTTONE_OUTPUT_ANALOG,
    AUD_DEC_TESTTONE_OUTPUT_SPDIF,
    AUD_DEC_TESTTONE_OUTPUT_HDMI
} AUD_DEC_TESTTONE_OUTPUT_T;

//test tone Front or rear
typedef enum
{
    AUD_DEC_TESTTONE_FRONT,
    AUD_DEC_TESTTONE_REAR
}AUD_DEC_TESTTONE_OUT;
typedef enum
{
    AUD_DEC_TESTTONE_ENABLE = 0,
    AUD_DEC_TESTTONE_DISABLE
}AUD_DEC_TESTTONE_ONOFF;


typedef struct
{
    AUD_DEC_TEST_TONE_TYPE_T         eTTType;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SET_TYPE;

typedef struct
{
    AUD_DEC_TESTTONE_ONOFF         eTTSwitch;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SWITCH_T;

typedef struct
{
    AUD_DEC_LS_T         eTTLs;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SET_CHANNEL;


// for new re-encoder flow @ 01/14/2008
// make sure the definition is idential with SCC interfaces.
// SCC_AUD_REENCODER_FMT_T and SCC_AUD_REENCODER_CONFIG_T
typedef enum
{
    AUD_DEC_REENCODER_FMT_DTS = 0,
    AUD_DEC_REENCODER_FMT_DDCO
} AUD_DEC_REENCODER_FMT_T;

typedef struct _AUD_DEC_REENCODER_CONFIG_T
{
    AUD_DEC_REENCODER_FMT_T e_reencoder_fmt;
    bool                    b_enable;
} AUD_DEC_REENCODER_CONFIG_T;

typedef struct _AUD_DEC_REENCODER_CONFIG_EX_T
{
    AUD_DEC_REENCODER_CONFIG_T r_config;
    bool                       b_force_reinit; // flag for HDMI hot-plug
} AUD_DEC_REENCODER_CONFIG_EX_T;



/*
 *  Audio Encoder Interface define (Ripping / Skype)
 *  -- Water (AUD_RIPPING)
 */

/* Encoder Generic Command List (all command to encoder should contain with this base type) */
typedef enum {
    // Encoder instance create & destroy
    AUD_ENCODER_CMD_CREATE = 1,                 // with AUD_ENCODER_CREATE_T para
    AUD_ENCODER_CMD_DESTROY,                    // with AUD_ENCODER_CREATE_T para

    // Encoder Flow Control Command
    AUD_ENCODER_CMD_START,                      // with AUD_ENCODER_CMD_PROC_T para
    AUD_ENCODER_CMD_FLUSH,                      // with AUD_ENCODER_CMD_PROC_T para
    AUD_ENCODER_CMD_STOP,                       // with AUD_ENCODER_CMD_PROC_T para

    // Encoder config command
    AUD_ENCODER_CMD_SET,                        // with set related para (ex. AUD_ENCODER_SET_RIP_MP3_CFG_T @ Ripping)
    AUD_ENCODER_CMD_GET,                        // with get related para (ex. AUD_ENCODER_GET_RIP_DATA_INFO_T @ Ripping)
} AUD_ENCODER_CMD_T;


/* Encoder generic encode format type list */
typedef enum {
    AUD_ENCODER_FMT_NULL = 0,
    AUD_ENCODER_FMT_RIP_MP3,                    // ripping use mp3 encoder
    AUD_ENCODER_FMT_SKYPE,                      // skype encoder -- future used
} AUD_ENCODER_FMT_T;

/* Encoder inst create and destroy para structure define */
typedef struct {                                // Create & Destroy Cmd need this type
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = Create / Destroy Cmd
    AUD_ENCODER_FMT_T               tEncType;   // with Encoder type (AUD_ENCODER_FMT_RIP_MP3/AUD_ENCODER_FMT_SKYPE...)
    // Other ... TBD
} AUD_ENCODER_CREATE_T;

/* Encoder basic command type (Cmd used normally) */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = Start / Flush / Stop
} AUD_ENCODER_CMD_PROC_T;

/* Encoder set config sub type list */
typedef enum {                                  // Define Encoder Set cmd sub type
    AUD_ENCODER_SET_RIP_MP3_CFG = 1,
    AUD_ENCODER_SET_RIP_READ_PTR,
} AUD_ENCODER_SET_SUB_TYPE_T;

/* Encoder get config sub type list */
typedef enum {                                  // Define Encoder get cmd sub type
    AUD_ENCODER_GET_RIP_DATA_INFO = 1,
} AUD_ENCODER_GET_SUB_TYPE_T;

/* Encoder set config basic item (all set config command should contain thouse basic type items) */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = AUD_ENCODER_CMD_SETCFG @ Ripping mode & use mp3 encoder
    AUD_ENCODER_SET_SUB_TYPE_T      tSetType;   // tSetType = Set sub type
} AUD_ENCODER_SET_BASE_T;

/* Encoder get config basic item (all get config command should contain thouse basic type items) */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = AUD_ENCODER_CMD_GET_RIP_DATAINFO @ Ripping mode
    AUD_ENCODER_GET_SUB_TYPE_T      tGetType;   // tGetType = Get sub type
} AUD_ENCODER_GET_BASE_T;

/*
 *  Ripping use MP3 encoder detailed structure define
 */
/* Ripping mode define 1X or high speed (fast) mode */
typedef enum {
    AUD_ENCODER_RIP_NORMAL = 0,                 // Normal speed (1X)
    AUD_ENCODER_RIP_HIGH_SPEED                  // High speed (Fast) may be >= 4X (aout is mute)
} AUD_ENCODER_RIP_MODE_T;

/* Mp3 encoder bit rate define */
typedef enum {
    AUD_RIP_MP3_BRATE_48K = 0x03,               // 0x03
    AUD_RIP_MP3_BRATE_56K,                      // 0x04
    AUD_RIP_MP3_BRATE_64K,                      // 0x05
    AUD_RIP_MP3_BRATE_80K,                      // 0x06
    AUD_RIP_MP3_BRATE_96K,                      // 0x07
    AUD_RIP_MP3_BRATE_112K,                     // 0x08
    AUD_RIP_MP3_BRATE_128K,                     // 0x09
    AUD_RIP_MP3_BRATE_160K,                     // 0x0A
    AUD_RIP_MP3_BRATE_192K,                     // 0x0B
    AUD_RIP_MP3_BRATE_224K,                     // 0x0C
    AUD_RIP_MP3_BRATE_256K,                     // 0x0D
    AUD_RIP_MP3_BRATE_320K,                     // 0x0E
} AUD_ENCODER_RIP_MP3_BITRATE_T;

/* Ripping use mp3 encoder type config setting */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = AUD_ENCODER_CMD_SETCFG @ Ripping mode & use mp3 encoder
    AUD_ENCODER_SET_SUB_TYPE_T      tSetType;   // tSetType = AUD_ENCODER_SET_RIP_MP3_CFG
    AUD_ENCODER_RIP_MODE_T          tRipMode;   // tRipMode = AUD_ENCODER_RIP_MODE_T
    AUD_ENCODER_RIP_MP3_BITRATE_T   tBitRate;   // tBitRate = AUD_ENCODER_RIP_MP3_BITRATE_T
} AUD_ENCODER_SET_RIP_MP3_CFG_T;

/* Ripping update read pointer (release fifo date used) */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = AUD_ENCODER_CMD_SET_RIP_READ_PTR @ Ripping mode
    AUD_ENCODER_SET_SUB_TYPE_T      tSetType;   // tSetType = AUD_ENCODER_SET_RIP_READ_PTR
    uintptr_t                          u4ReadPtr;  // target read pointer address (user space address)
} AUD_ENCODER_SET_RIP_RPTR_T;

/* Get Ripping bit stream buffer information and stats */
typedef struct {
    AUD_ENCODER_CMD_T               tEncCmd;    // tEncCmd = AUD_ENCODER_CMD_GET_RIP_DATAINFO @ Ripping mode
    AUD_ENCODER_GET_SUB_TYPE_T      tGetType;   // tGetType = AUD_ENCODER_GET_RIP_DATA_INFO
    uintptr_t                          u4BufSa;    // bit stream buffer start address (user space address)
    uintptr_t                          u4BufEa;    // bit stream buffer end address (user space address)
    uintptr_t                          u4CurRPtr;  // bit stream current read pointer address  (user space address)
    uintptr_t                          u4CurWPtr;  // bit stream current write pointer address (user space address)
    bool                            fgEncodeDone;// FALSE: Still encoding, TRUE: No more data to encode
} AUD_ENCODER_GET_RIP_DATA_INFO_T;



// for watermark internal callback function
typedef void (*x_aud_aw_fct) (void* pvParam);

typedef struct
{
    x_aud_aw_fct  pf_aud_aw_nfy;
    void *pvParam;
} AUD_DEC_WATERMARK_FCT_T;

// for watermark event callback function
typedef __s32 (*x_aud_aw_event_fct) (__u8 ui1CmValue, __u16 ui2ArSet, void*pvUser_data);

typedef struct
{
    x_aud_aw_event_fct  pf_aud_aw_event_nfy;
    void *pvParam;
} AUD_DEC_WATERMARK_EVENT_FCT_T;


typedef struct
{
    AUD_DEC_MUTE_TYPE_T eMuteType;
    AUD_DEC_MUTE_SRC_T eMuteSrc;
}   AUD_DEC_GLOBAL_MUTE_T;

typedef struct
{
    __u32 u4DspAVer;
    __u32 u4DspBVer;
    __u32 u4DspCVer;
} AUD_DEC_DSP_VERSION_T;

// for max speaker config behavior control
typedef enum
{
    AUD_DEC_MAXSPEAKERCFG_DEFAULT,        // default mode, care both speaker setting and HDMI sink capability
    AUD_DEC_MAXSPEAKERCFG_SPEAKER_ONLY,   // only care speaker setting
    AUD_DEC_MAXSPEAKERCFG_HDMI_ONLY       // only care HDMI sink capability
} AUD_DEC_MAXSPEAKERCFG_OPTION_T;

typedef enum
{
    AUD_DEC_MUTE_CTRL_APP       = 0,
    AUD_DEC_MUTE_CTRL_PB        = 1,
    AUD_DEC_MUTE_CTRL_AWD       = 2,  // global mute for Audio Watermark
    AUD_DEC_MUTE_CTRL_BDDOT     = 3,  // mute SPDIF(PCM/RAW) and Analog
    AUD_DEC_MUTE_CTRL_WMDRM     = 4,  // mute SPDIF(PCM/RAW) and HDMI(PCM/RAW)
    // add new type here
    AUD_DEC_MUTE_CTRL_FORCE     = 14, // force unmute and clear mute flag (currently not used)
    AUD_DEC_MUTE_CTRL_RESERVED  = 15,
} AUD_DEC_MUTE_CTRL_SOURCE_T;


/* UPMIX type */
typedef enum
{
    AUD_SE_UPMIX_CTRL_SWITCH = 0,
    AUD_SE_UPMIX_CTRL_MODE
}   AUD_SE_UPMIX_CTRL_T;


/* switch operation value */
typedef enum
{
    AUD_SE_UPMIX_SWITCH_OFF = 0,
    AUD_SE_UPMIX_SWITCH_ON,
}   AUD_SE_UPMIX_SWITCH_T;

/*
 *  CSII Coefficient Struct Type define
 */
typedef enum
{
    AUD_SE_CSII_CTRL_SWITCH = 0,
    AUD_SE_CSII_CTRL_MODE,
    AUD_SE_CSII_CTRL_PHANTOM,
    AUD_SE_CSII_CTRL_FB,
    AUD_SE_CSII_CTRL_FOCUS_CENTER,
    AUD_SE_CSII_CTRL_FOCUS_FRONT,
    AUD_SE_CSII_CTRL_FOCUS_REAR,
    AUD_SE_CSII_CTRL_TB,
    AUD_SE_CSII_CTRL_F_SS, // front speaker size
    AUD_SE_CSII_CTRL_S_SS, // sub speaker size
    AUD_SE_CSII_CTRL_R_SS, // rear speaker size
    AUD_SE_CSII_CTRL_F2R,
    AUD_SE_CSII_CTRL_C2R,
    AUD_SE_CSII_CTRL_TBF,
    AUD_SE_CSII_CTRL_TBS,
    AUD_SE_CSII_CTRL_TBR
}   AUD_SE_CSII_CTRL_T;

//for PHI 2010 module logo
typedef enum
{
    AUD_DEC_LOGO_TRUEHD,
    AUD_DEC_LOGO_DD_PLUS,
    AUD_DEC_LOGO_DD,
    AUD_DEC_LOGO_DTS20,
    AUD_DEC_LOGO_DTS_ADO,
    AUD_DEC_LOGO_DTS_HD_MA,
    AUD_DEC_LOGO_DTS_HD_MA_ESS,
    AUD_DEC_LOGO_DTS_DIGI_SURR
}AUD_DEC_MODULE_LOGO_INFO_T;

typedef enum{
    AUD_DEC_DIV_TYPE_LOGO_DOLBY,
    AUD_DEC_DIV_TYPE_LOGO_DTS,
    AUD_DEC_DIV_TYPE_ANALOG_OUTPUT_CHANNEL,
    AUD_DEC_DIV_TYPE_HDMI_OUTPUT_CHANNEL,
    AUD_DEC_DIV_TYPE_POST_PROCESS,
    AUD_DEC_DIV_TYPE_CODEC_SUPPORT
}AUD_DEC_DIV_TYPE_T;

typedef struct _AUD_DEC_DIV_INFO_T
{
    AUD_DEC_DIV_TYPE_T     e_type;
    __u8                  u1_setting;
} AUD_DEC_DIV_INFO_T;


typedef enum{
    I2S_MCLK_128FS,
    I2S_MCLK_192FS,
    I2S_MCLK_256FS,
    I2S_MCLK_384FS,
    I2S_MCLK_512FS,
    I2S_MCLK_768FS,
    I2S_MCLK_UNKNOWN
}I2S_CLK_SAMPLE_FREQUENCY_T;

typedef enum// new add 2007/9/12
{
  HDMI_CLK_FS_16K= 0x00,
  HDMI_CLK_FS_22K,
  HDMI_CLK_FS_24K,
  HDMI_CLK_FS_32K,
  HDMI_CLK_FS_44K,
  HDMI_CLK_FS_48K,
  HDMI_CLK_FS_64K,
  HDMI_CLK_FS_88K,
  HDMI_CLK_FS_96K,
  HDMI_CLK_FS_176K,
  HDMI_CLK_FS_192K,
  HDMI_CLK_FS512_44K,//for DSD
  HDMI_CLK_FS_768K,
  HDMI_CLK_FS128_44k,
  HDMI_CLK_FS_UNKOWN
} HDMI_CLK_AUDIO_SAMPLING_T;

typedef struct _AUD_DEC_HDMI_CLK_INFO_T
{
    HDMI_CLK_AUDIO_SAMPLING_T e_fs;
    I2S_CLK_SAMPLE_FREQUENCY_T e_mclk;
} AUD_DEC_HDMI_CLK_INFO_T;

//for PHI 2010 module aout
typedef enum
{
    AUD_DEC_AOUT_STEREO_CHANNEL,
    AUD_DEC_AOUT_MULTI_CHANNEL
}AUD_DEC_MODULE_AOUT_CHANNEL_INFO_T;

//for PHI 2010 module ddco
typedef enum
{
    AUD_DEC_AOUT_DDCO
}AUD_DEC_MODULE_DDCO_INFO_T;

//for PHI 2010 module bass management
typedef enum
{
    AUD_DEC_BMANAGEMENT_STEREO_CHANNEL,
    AUD_DEC_BMANAGEMENT_MULTI_CHANNEL
}AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T;

//for PHI 2010 feature
typedef enum
{
    AUD_DEC_PROLOGICII,
    AUD_DEC_SACD,
    AUD_DEC_AAC,
    AUD_DEC_AACUSED_ONLY,
    AUD_DEC_NEO6
}AUD_DEC_FEATURE_INFO_T;

//for PHI 2010 module hdmi
typedef enum
{
     AUD_DEC_HDMI_DD_OUTPUT_2,
     AUD_DEC_HDMI_DD_OUTPUT_5_1,
     AUD_DEC_HDMI_DD_OUTPUT_7_1,
     AUD_DEC_HDMI_DTS_OUTPUT_2,
     AUD_DEC_HDMI_DTS_OUTPUT_5_1,
     AUD_DEC_HDMI_DTS_OUTPUT_7_1,
     AUD_DEC_HDMI_OTHER_CODEC_OUTPUT_2,
     AUD_DEC_HDMI_OTHER_CODEC_OUTPUT_5_1,
     AUD_DEC_HDMI_OTHER_CODEC_OUTPUT_7_1
}AUD_DEC_MODULE_HDMI_INFO_T;

//for PHI 2010 DD coefficient clev and slev
typedef struct
{
    __u8 dd_clev;
    __u8 dd_slev;
}AUD_DEC_DD_CLEV_SLEV;

//for PHI 2010 TRUEHD gain
typedef struct
{
    __u8 truehd_gain;
}AUD_DEC_TRUEHD_GAIN;

typedef enum
{
     AUD_DEC_CDDA_OUTPUT_PCM,
     AUD_DEC_CDDA_OUTPUT_RAW,
     AUD_DEC_HDCD_OUTPUT_PCM,
     AUD_DEC_HDCD_OUTPUT_RAW
}AUD_DEC_CD_OUTPUT_MODE_T;

typedef enum{
	AUD_ASDATA_0,
	AUD_ASDATA_1,
	AUD_ASDATA_2,
	AUD_ASDATA_3,
	AUD_ASDATA_4,
	AUD_ASDATA_5
}AUD_ASDATA_SELECT;


typedef enum{
	AUD_CH_1_2,
	AUD_CH_3_4,
	AUD_CH_5_6,
	AUD_CH_7_8,
	AUD_CH_9_10,
	AUD_CH_11_12
}AUD_ASDATA_CH;

typedef struct _AUD_SET_ASDATA_CH_T
{
    AUD_ASDATA_SELECT     aud_asdata_select;
    AUD_ASDATA_CH           aud_asdata_ch;
} AUD_ASDATA_CH_T;

typedef struct _AUD_SET_ASDATA_CH_ALL_T
{
    AUD_ASDATA_SELECT     aud_asdata_select[6];
    AUD_ASDATA_CH           aud_asdata_ch[6];
} AUD_ASDATA_CH_ALL_T;


/* cli type.add by fei  for bd cli to wince*/

typedef enum
{
    AUD_DEC_CLI_TEST = 0,
    AUD_DEC_CLI_HELP,
    AUD_DEC_CLI_CMD_LOG,
    AUD_DEC_CLI_DUMP_AFIFO,
    AUD_DEC_CLI_DSP_CM,
    AUD_DEC_CLI_DSP_SH = 5,
    AUD_DEC_CLI_DSP_SHW,
    AUD_DEC_CLI_UR,
    AUD_DEC_CLI_Q,
    AUD_DEC_CLI_DSP_CFG,
    AUD_DEC_CLI_IECREGS = 10,
    AUD_DEC_CLI_PI,
    AUD_DEC_CLI_PL,
    AUD_DEC_CLI_PU,
    AUD_DEC_CLI_WCM,
    AUD_DEC_CLI_DSP_R = 15,
    AUD_DEC_CLI_DSP_W,
    AUD_DEC_CLI_ESM_ST,
    AUD_DEC_CLI_AUD_PA,
    AUD_DEC_CLI_AUD_ST,
    AUD_DEC_CLI_AUD_V = 20,
    AUD_DEC_CLI_AUD_BYPASS,
    AUD_DEC_CLI_DRV_R,
    AUD_DEC_CLI_DRV_W,
    AUD_DEC_CLI_DIGOUT_EXA,
    AUD_DEC_CLI_DIGOUT_EXI = 25,
    AUD_DEC_CLI_DUMP_AOUT,
    AUD_DEC_CLI_UART_SWITCH,  //(aud_cli_demo by mtk40292)
    AUD_DEC_CLI_RB_ST,
    AUD_DEC_CLI_EQ_ST,
    AUD_DEC_CLI_PL2_ST = 30,
    AUD_DEC_CLI_SPECT_ST,
    AUD_DEC_CLI_BASSM_ST,
    AUD_DEC_CLI_ASRC_ST,
    AUD_DEC_CLI_AUD_LOG,
    AUD_DEC_CLI_ST = 35,//yucai yang
    AUD_DEC_CLI_V,
    AUD_DEC_CLI_TESTTONE_ONOFF,
    AUD_DEC_CLI_TESTTONE_SELFRNREAR,
    AUD_DEC_CLI_TESTTONE_SELTYPE,
    AUD_DEC_CLI_TESTTONE_SELCHANNEL= 40,
    AUD_DEC_CLI_GPSMIX_PLAY_CMD,
    AUD_DEC_CLI_GPSMIX_STOP_CMD,
    AUD_DEC_CLI_GPSMIX_PAUSE_CMD,
    AUD_DEC_CLI_GPSMIX_RESUME_CMD,
    AUD_DEC_CLI_CSII = 45,
    AUD_DEC_CLI_CHECK,
    AUD_DEC_CLI_EXT_LIN_TEST,
    AUD_DEC_CLI_PWMDAC_EXT_LDO,
    AUD_DEC_CLI_SPDIF,
    AUD_DEC_CLI_REAR_VOL_CONTROL = 50,
    AUD_DEC_CLI_UPMIX,
    AUD_DEC_CLI_LOUDNESS,
    AUD_DEC_CLI_DACSEL,
    AUD_DEC_CLI_FRNTVOL,
    AUD_DEC_CLI_REARVOL = 55,
    AUD_DEC_CLI_CHVOL,
    AUD_DEC_CLI_PRINT_VOL_GAIN,
    AUD_DEC_CLI_PRINT_TT_INFO,
    AUD_DEC_CLI_PRINT_UPMIX_GAIN,
    AUD_DEC_CLI_PRINT_LOUDNESS_GAIN = 60,
    AUD_DEC_CLI_LIN_REAR_BYPASS,
    AUD_DEC_CLI_GET_OUTPUT_VOL,
    AUD_DEC_CLI_SET_LRMIX,
    AUD_DEC_CLI_IO_TEST,
    AUD_DEC_CLI_DEBUG,
    AUD_DEC_CLI_V_SRC

}AUD_DEC_CLI_TYPE;

typedef struct _CLICmd{

    __u32 wrptr;
    __u32 rdptr;
    __u32 mw_cmd[128];

}CLICmd;


typedef struct {

	AUD_DEC_CLI_TYPE eAudCliType;

    __u32    u4arg1;//u4InputID;
    __u32    u4arg2;//u4Len;
    __u32    u4arg3;//u4Size;
    __u32    u4arg4;//u4Value;
    void    **ptParam;//filename

}AUD_DEC_CLI_CFG;

typedef struct _AUD_DRV_CONTEXT
{
    __u8 u1DecId;
    __u8 u1Output;
    bool  fgPlaying;
    bool  fgAvinHwOn;
    bool  fgEnPlay;
    AUD_OUT_MEDIA_TYPE_T ePlayType;
}AUD_DRV_CONTEXT;


typedef enum
{
    AUD_DEC_STC1 = 1,
    AUD_DEC_STC2,
    AUD_DEC_UNDEF_STC
}AUD_DEC_STC_T;

typedef struct _AUD_SYNC_CONTROL_INFO
{
    __u64 u8DecReadyPTS;
    __u64 u8TargetPTS;
    __u8 u1DecId;
    AUD_DEC_STC_T eStc;
}AUD_SYNC_CONTROL_INFO;


typedef struct _AUD_PTS_CONTEXT
{
    __u32 u4AudioPTSHi;
    __u32 u4AudioPTSLo;
    __u8 u1DecId;
}AUD_PTS_CONTEXT;

/* microphone ctrl */

enum MicOpCode {
    MIC_SET_INIT,
    MIC_SET_UNINIT,
    MIC_SET_ON_OFF,
    MIC_SET_VOLUME1,
    MIC_SET_VOLIME2,
    MIC_SET_THRESHOLD,
    MIC_SET_ECHO_VOLUME,
    MIC_SET_ECHO_FBGAIN,
    MIC_SET_ECHO_DELAY,
    MIC_SET_LEVEL_ORDER,
    MIC_SET_CH_MIX
};

struct MicThreshold {
    // mute < unmute
    __u32              u4ThresholdMute;
    __u32              u4ThresholdUnmute;
};

#define  AUD_MIC_MIXING_TO_L_R           ((1 << 1) << 8)
#define  AUD_MIC_MIXING_TO_LS_RS         ((1 << 2) << 8)
#define  AUD_MIC_MIXING_TO_C             ((1 << 3) << 8)
#define  AUD_MIC_MIXING_TO_EXT_CH1_CH2   ((1 << 4) << 8)
#define  AUD_MIC_MIXING_TO_DMX_CH1_CH2   ((1 << 5) << 8)
#define  AUD_MIC_MIXING_TO_CH11_CH12     ((1 << 6) << 8)
#define  AUD_MIC_MIXING_TO_SUB           ((1 << 7) << 8)
#define  AUD_MIC_DEFAULT_MIXING   (AUD_MIC_MIXING_TO_L_R)

enum AudMicChannelMixType {
    AUD_MIC_CHANNEL_MIX_AOUT1,
    AUD_MIC_CHANNEL_MIX_AOUT2
};

struct AudMicChannelMix {
    enum AudMicChannelMixType eMixType;
    __u32 u4ChannelMix;
};


/* microphone ctrl end */


enum AudPath {
    AUD_PATH_GENERAL,
    AUD_PATH_SKYPE
};


typedef enum
{
    AUD_DEVICE_ID_PRIMARY = 0,
    AUD_DEVICE_ID_FOUR,
    AUD_DEVICE_ID_GPSMIX,
    AUD_DEVICE_ID_DVD,
    AUD_DEVICE_ID_UNDEF
}DEVICE_ID_PM;

typedef struct _AUD_USER_INFO
{
	char* puser;
    __u32 buf_size;
}AUD_USER_INFO;

#if 0
#define AUD_SE_PARTY_CMPT_BUFFER_SA         0x8000    /* 0x10000 - 0x8000 */
#define AUD_SE_PARTY_CMPT_BUFFER_SZ         0x8000  /* Unit : DOWRD*/

#define AUD_SE_PARTY_MAX_BUFFER_STEP        0x1800  /*4096 Smp: 4096  * 2 channel * 24bit /32 bit = 0x1800 (DWORD) */
#define AUD_SE_PARTY_RISC_GET_BANK64        0x60    /* 64 Smp:  64 * 2(channel)* 24bit/32bit(DWORD) */
#define AUD_SE_PARTY_RISC_PUT_BANK64        0x40    /* 64 Smp:  64 * 2(channel)* 16bit/32bit(DWORD) */
#define AUD_SE_BANK_SAMPLE_COUNT            0x40    /* 64 Smp*/
#define AUD_SE_PARTY_BUF0_SIZE              0x140   //(AUD_SE_PARTY_CMPT_BUFFER_SZ/AUD_SE_PARTY_RISC_GET_BANK64) /*0x155*/
#define AUD_SE_PARTY_BUF1_SIZE              (AUD_SE_PARTY_CMPT_BUFFER_SZ/AUD_SE_PARTY_RISC_PUT_BANK64) /*0x200*/
#define AUD_SE_PARTY_GET_LOOP_SZ            (0x140*AUD_SE_PARTY_RISC_GET_BANK64)
#define AUD_SE_PARTY_CHANNEL_COUNT          2

#define SET_ADSP_C_OFF 1
#endif

typedef enum
{
    AUD_DECONLY_OFF = 0,
    AUD_DECONLY_ON,
    AUD_DECONLY_UNDEF_CTRL
}AUD_DECONLY_CTRL_T;

typedef enum
{
    AUD_DECONLY_BIG_EDNIAN = 0,
    AUD_DECONLY_LITTLE_ENDIAN,
    AUD_DECONLY_UNDEF_ENADIAN
}AUD_DECONLY_DATA_ENDIAN;

typedef enum
{
    AUD_DECONLY_BIT8_DEPTH = 0,
    AUD_DECONLY_BIT16_DEPTH,
    AUD_DECONLY_BIT24_DEPTH,
    AUD_DECONLY_UNDEF_DEPTH
}AUD_DECONLY_BIT_DEPTH;

typedef enum
{
    AUD_DECONLY_NORMAL = 0,
    AUD_DECONLY_DEC_NOT_READY,
    AUD_DECONLY_AFIFO_NOT_ENOUGH,
    AUD_DECONLY_AFIFO_TOO_MUCH,
    AUD_DECONLY_DEC_FINISH,
    AUD_DECONLY_UNDEF_ERROR
}AUD_DECONLY_ERROR_TYPE;

typedef struct _AUD_DECONLY_CH_CFG
{
    __u16 u2ChNum;       // 1~6 channel
    __u8 u1LayoutC;      // 0: current ch off, 1~6: current ch layout position
    __u8 u1LayoutL;
    __u8 u1LayoutR;
    __u8 u1LayoutLs;
    __u8 u1LayoutRs;
    __u8 u1LayoutSub;
}AUD_DECONLY_CH_CFG;

typedef struct _AUD_DECONLY_GET_BUF
{
    AUD_DECONLY_DATA_ENDIAN eDataEndian;
    AUD_DECONLY_BIT_DEPTH eBitDepth;
    AUD_DECONLY_CH_CFG eChCfg;
    __u32 u4SampleRate;
    __u32 u4BufLen;
    uintptr_t u4BufAddr;
    __u64 u8BufPts;
    AUD_DECONLY_ERROR_TYPE eErrType;
}AUD_DECONLY_GET_BUF;


/*media type define*/
typedef enum
{
    AUD_MEDIA_SOURCE_USB,
    AUD_MEDIA_SOURCE_LINEIN,
    AUD_MEDIA_SOURCE_LINEIN2,
    AUD_MEDIA_SOURCE_SWMIX,
    AUD_MEDIA_SOURCE_SWMIX2,
    AUD_MEDIA_SOURCE_DVP,
    AUD_MEDIA_SOURCE_UNDEF
}   AUD_MEDIA_SOURCE_TYPE_T;

typedef enum
{
    AUD_MEDIA_OUT_FRONT,
    AUD_MEDIA_OUT_REAR,
    AUD_MEDIA_OUT_UNDEF
}   AUD_MEDIA_OUT_TYPE_T;

typedef enum
{
    AUD_MEDIA_OFF,
    AUD_MEDIA_ON,
    AUD_MEDIA_ONOFF_UNDEF
}   AUD_MEDIA_ONOFF;

typedef struct _AUD_MEDIA_TYPE
{
    AUD_MEDIA_SOURCE_TYPE_T eMediaSrc;
    AUD_MEDIA_OUT_TYPE_T eMediaOut;
    AUD_MEDIA_ONOFF eMediaCtrl;
}AUD_MEDIA_TYPE;

typedef struct _AUD_SRC_VOL_CTL
{
    AUD_MEDIA_SOURCE_TYPE_T eMediaSrc;
	AUD_MEDIA_OUT_TYPE_T eMediaOut;
	__u32 u4Vol;
}AUD_SRC_VOL_CTL;

typedef struct AUD_SRC_MUTE_CTL
{
    AUD_MEDIA_SOURCE_TYPE_T eMediaSrc;
    AUD_MEDIA_OUT_TYPE_T eMediaOut;
    bool fgMute;
}AUD_SRC_MUTE_CTL;


#if AUDIO_SETTING_CTS_ENABLED

typedef enum
{
    AUD_SETING_ID_SET_PCM_CONFIG = 0,
    AUD_SETING_ID_GET_PCM_CONFIG,
    AUD_SETING_ID_SET_DEV_VOLUME,
    AUD_SETING_ID_SET_MIC_MUTE,
    AUD_SETING_ID_SET_PRIMARY_MIC,
    AUD_SETING_ID_SET_DSP_MIX_CH,
    AUD_SETING_ID_SET_DTMF_CTRL_TYPE,
    AUD_SETING_ID_SET_DTMF_THRESHOLD,
    AUD_SETING_ID_SET_DTMF_NOISE_RATIO,
    AUD_SETING_ID_SET_DTMF_VALID_TIME,
    AUD_SETING_ID_SET_DTMF_INVALID_TIME,
    AUD_SETING_ID_SET_DTMF_NEW_SAMPLES,
    AUD_SETING_ID_SET_DTMF_MAX_SCALE,
    AUD_SETING_ID_SET_DTMFINFO_SENDER,
    AUD_SETING_ID_SET_MUTE,
    AUD_SETING_ID_SET_DAC,
    AUD_SETING_ID_SET_REAR_VOLUME,
    AUD_SETING_ID_SET_EQ_VALUES,
    AUD_SETING_ID_SET_BALANCE,
    AUD_SETING_ID_SET_SPDIF_OUTPUT,
    AUD_SETING_ID_SET_UPMIX,
    AUD_SETING_ID_SET_TEST_TONE,
    AUD_SETING_ID_SET_PL2,
    AUD_SETING_ID_SET_SPK_LAYOUT,
    AUD_SETING_ID_SET_ATS_SWITCH,
    AUD_SETING_ID_SET_ATS_GAIN,
    AUD_SETING_ID_SET_AUD_FEATURE,
    AUD_SETING_ID_SET_REAR_OUT_MODE,
    AUD_SETING_ID_SET_SRS_SWITCH,
    AUD_SETING_ID_SET_SRS_MODE,
    AUD_SETING_ID_SET_SRS_PHATOM,
    AUD_SETING_ID_SET_SRS_FOCUS,
    AUD_SETING_ID_SET_SRS_TRUE_BASS,
    AUD_SETING_ID_SET_SRS_BASS_SIZE,
    AUD_SETING_ID_SET_SRS_FULL_BAND,
    AUD_SETING_ID_SET_FUNC_OPTION,
    AUD_SETING_ID_END
}   AUD_SETTING_ID;

#endif

#endif /* _X_AUD_DEC_H_ */

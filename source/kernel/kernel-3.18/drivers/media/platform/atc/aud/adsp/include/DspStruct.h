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


#ifndef _DSP_Struct_H_
#define _DSP_Struct_H_

#include "DspConst.h"
//#include "x_typedef.h"
#include <linux/types.h>
#include "aud_hal_intf.h"

// *********************************************************************
// Dspctrl structure type definitions
// *********************************************************************
typedef struct
{
    bool fgDspRInt;
    bool fgDspSInt;
} DSPSTATUS_T;

typedef struct
{
    u16 wDspSpkCfg;
    u8 bFrntSpkrNm;
    u8 bSrndSpkrNm;
    bool fgIsSWExst;
    u8 bChNm;
} SPEAKER_SETTING_T;

typedef struct
{
    u16 u2MicMixConf;
    AUD_HAL_IEC_BIT_NUMBER eIecOutBitNum;
    AUD_HAL_IEC_OUTPUT_SEL eIecOutCh;
    bool fgIecMute;
    AUD_HAL_DOWNSAMPLE eIecDownSamp;
    AUD_HAL_DOWNSAMPLE eIec2DownSamp;
} AUDIO_CFG_T;

typedef struct
{
    u32 dwDspFreq;
    u32 dwDspRamCodeType;
    u8 bDspStrTyp;
    u32 dwADacFreq;
    u32 dwStcDiff;
    u32 dwDspReInitPts;
    u32 dwTableFreq;
    u32 dwDacFreq;
    u8 bCode3D;
    u8 bCode3DUpmix;
    u32 dwDspSIntFail;
    u32 dwVolPrevUsr;
    u32 dwVolLastSetting;
    u32 dwVolSettingStep;
    u32 dwDspForceResetCnt;
    u32 dwDspBitrate;
// For mp4
    u32 dwDspMpgTyp;
// For speed control case
    u32 dwDspAckPTS;
    u32 dwDspPlaySpeed;
    u32 dwDspUop;
    bool   fgHDCDTrk;
    SPEAKER_SETTING_T tSrcSpkCfg;
} DSPVARS_T;

typedef struct
{
    u8 bDspReEncTyp;
} DSPREENCVARS_T;

typedef struct
{
    /* common code field*/
    u32 dwInputUnderRun; /* error count for input underrun*/
    u32 dwInputUnderRunDec2; /* error count for input underrun of decoder 2*/
    u32 dwInputUnderRunMixSound; /* error count for input underrun of mixsound*/
    u32 dwOutputUnderRun; /* error count for input output underrun*/
    u32 dwOutputUnderRunDec2; /* error count for input output underrun of decoder2*/
    u32 dwOutputUnderRunMixSound; /* error count for input output underrun of mixsound*/
    u32 dwReserved0[9]; /* reserved field */
    /* first decoder's field*/
    u32 dwFrameRepeatCount; /* repeat count for frames*/
    u32 dwFrameDropCount; /* drop count for frames*/
    u32 dwTotalErrorCount; /* total error count*/
    u32 dwTotalFrameCount; /* total frame count*/
    u32 dwReserved1[3]; /* reserved field*/
    u32 dwDetailLogs[16] ; /* detail log for decoder1*/
    /* second decoder's field*/
    u32 dwFrameRepeatCountDec2; /* repeat count for frames*/
    u32 dwFrameDropCountDec2; /* drop count for frames*/
    u32 dwTotalErrorCountDec2; /* total error count*/
    u32 dwTotalFrameCountDec2; /* total frame count*/
    u32 dwReserved2[3]; /* reserved field*/
    u32 dwDetailLogsDec2[16] ; /* detail log for decoder2*/
} ADSP_ERROR_LOG_T;

typedef struct
{
    u32 u4StartAddr;
    u32 u4EndAddr;
    u32 u4WritePtr;
    u32 u4ReadPtr;
    u32 u4Reserved;
} AFIFO_INFO_T;

typedef struct __TDspIntr
{
    u32  u4DspRIntSD;
    u32  u4DspRIntLD;
    bool    fgDspId;  //DSP A: 0 ; DSP B:1
} TDspIntr;

typedef struct __TDspCmd{
  u8 bWrIdx;
  u8 bRdIdx;
  u8 bCmdNs;
  u8 bStatus;
  u32 pu4Cmd[MAX_DSP_CMD_NS];
  TDspIntr prCmd[MAX_DSP_CMD_NS];
}TDspCmd;

typedef struct _TDspUopInt
{
    u32  u4DspRIntSD;
    u32  u4DspRIntLD;
    u32  u4DspIntAddr;
    bool    fgDspId;  //DSP A: 0 ; DSP B:1
} TDspUopInt;

typedef struct _PBS_BLK{
  u16 wSblk;
  u16 wEblk;
  u32 dwRdPtrAdr;
  u32 dwWrPtrAdr;
}PBS_BLK_SET;  //note that this typdedef should be moved to dspctrl and not seenable to audctrl

typedef struct _DSP_IBC_QUEUE_INFO{
    u32 u4QueueSA;
    u32 u4QueueSize;
    u32 u4ReadPtr;
    u32 u4WritePtr;
    u8  u1AfifoLoop;
}DSP_IBC_QUEUE_INFO_T;


typedef enum
{
    DSP_DISC_LEG = 1,
    DSP_DISC_BD
} DSP_DISC_TYPE;

typedef struct _DSP_PCM_SET_INFO{
   u8 u1ChannelAssignment;
   u8 u1SamplingFrequency;
   u8 u1BitsPerSample;
   DSP_DISC_TYPE u1Disctype;
}DSP_PCM_SET_INFO_T;

typedef struct _DSP_SACD_SETTING
{
  u16 u2SacdFilterLen;
  u16 u2SacdDownSample;
  u16 u2SacdChNum;
  u16 u2SacdDownSampleOrder;
  u16 u2SacdOutputMode;
  u32 u4SacdInputFrame;  // NOTE - MSB Byte must be 0. 8
  u16 u4InputDsdMode;
  u16 u4SacdTimeCodeEnable;
} DSP_SACD_SETTING;

typedef struct _DSP_SACD_DEC_INFO
{
  u32 u4SacdOutputFrame;
  u32 u4TimeCode;
} DSP_SACD_DEC_INFO;

#define PORT_AUD_DOLBY_PL2      (1<<0)
#define PORT_AUD_DTS            (1<<1)
#define PORT_AUD_AC3            (1<<2)
#define PORT_AUD_WMA            (1<<8)
#define PORT_AUD_SRS            (1<<12)
#define PORT_AUD_AAC            (1<<13)
#define PORT_AUD_FLAC_APE       (1<<14)

//Audio driver Support DTS decoder and SRS

typedef struct _DSP_CODEC_BONDING
{
  bool fgDspSacdSupport;
  bool fgDspDtsHDSupport;
  bool fgDspTrueHDSupport;
  bool fgDspEAC3Support;
  bool fgDspMultichannelSupport;
  bool fgDspDDCOSupport;
  bool fgDspAACSupport;
  bool fgDspAACPlusDecUsed;

  // add by mtk40292 efuse HW bonding for audio feature
  bool fgDspFlacAPESupport;
  bool fgDspWMASupport;
  bool fgDspAC3Support;
  bool fgDspDTSSupport;
  bool fgDspSRSSupport;
  bool fgDspPL2Support;
} DSP_CODEC_BONDING;

typedef struct _SPDIF_RAW_SET_INFO{   //CONFIG_DRV_SPDIF_RAW_SUPPORT
   bool fgUsbIsRawOut;
   u8 u1UsbSampRate;
   bool fgDvdIsRawOut;
   u8 u1DvdSampRate;
   bool fgLineinIsRawOut;
   u8 u1LineinSampRate;
}SPDIF_RAW_SET_INFO_T;

// *********************************************************************
// Dspctrl constant enumerations
// *********************************************************************
typedef enum
{
  // sample frequency defined here
  SFREQ_8K = 0x01,
  SFREQ_16K = 0x02,
  SFREQ_32K = 0x03,
  SFREQ_64K = 0x04,
  SFREQ_11K = 0x06,
  SFREQ_22K = 0x07,
  SFREQ_44K = 0x08,
  SFREQ_88K = 0x09,
  SFREQ_176K = 0x0A,
  SFREQ_12K = 0x0B,
  SFREQ_24K = 0x0C,
  SFREQ_48K = 0x0D,
  SFREQ_96K = 0x0E,
  SFREQ_192K = 0x0F,
  SFREQ_SACD = 0x10
} AUD_SFREQ_IDX_T;

typedef enum
{
    DSP_CTRL_INIT,
    DSP_CTRL_SINT,
    DSP_CTRL_UOPSVC
} DSPCTRLSTATE;

typedef enum
{
    DSP_AFIFO,
    DSP_PTS_QUEUE,
    DSP_IN_BAND_CMD_QUEUE
} DSP_BUF_TYPE_T;

typedef enum
{
    DSP_HDCD_ENABLE = 0x1,                // bit 0: hdcd enable/disable
    DSP_HDCD_AUTO_LEVEL_DETECT = 0x2,     // bit 1: autolevel flag (default:on)
    DSP_HDCD_DITHER_ENABLE = 0x4,         // bit 2: dither on/off
    DSP_HDCD_FILTER_ENABLE = 0x8,         // bit 3: do hdcd filter on non-hdcd (OFF, DSP raises itself)
    DSP_HDCD_NO_POST = 0x100
} DSP_HDCD_CONFIG_T;

/* for SPDIF PCM output channel */
#define DSP_SPDIF_OUTPUT_L_R           0
#define DSP_SPDIF_OUTPUT_LS_RS         1
#define DSP_SPDIF_OUTPUT_C_LFE         2
#define DSP_SPDIF_OUTPUT_CH7_CH8       3
#define DSP_SPDIF_OUTPUT_LINE_IN       4
#define DSP_SPDIF_OUTPUT_CH9_CH10      5
#define DSP_SPDIF_OUTPUT_CH11_CH12     6

#define DSP_CH_CFG_DUAL_MONO           0
#define DSP_CH_CFG_MONO                1
#define DSP_CH_CFG_STEREO              2
#define DSP_CH_CFG_LRC                 3
#define DSP_CH_CFG_LRS                 4
#define DSP_CH_CFG_LRCS                5
#define DSP_CH_CFG_LRLsRs              6
#define DSP_CH_CFG_LRCLsRs             7
#define DSP_CH_CFG_CH6                 (1<<3)
#define DSP_CH_CFG_CH7                 (1<<4)
#define DSP_CH_CFG_SUB                 (1<<5)

#define KARAFLAG_LRMIX            ((u8) 1<< 0 )
#define KARAFLAG_VOCALMUTE        ((u8) 1<< 1 )
#define KARAFLAG_VA                ((u8) 1<< 2 )

/* This macro is only for Mix operation both Left channel and Right channel */
#define LRMIX_MAX         17
#define LRMIX_LEFT_ONLY   0
#define LRMIX_RIGHT_ONLY  (LRMIX_MAX - 1)
#define LRMIX_BOTH        ((u8)((LRMIX_MAX - 1)/2))


/* dsp asrc auto tracing mode */
typedef enum
{
 	DSP_ASRC_AUTO_TRACING_NONE = 0,
	DSP_ASRC_AUTO_TRACING1 = 1,
 	DSP_ASRC_AUTO_TRACING2 = 2
} AUD_DSP_ASRC_AUTO_TRACING_T;

typedef enum
{
 	DSP_ASRC_AUTO_TRACING_CLK_SEL_APLL = 0x0,
	DSP_ASRC_AUTO_TRACING_CLK_SEL_DSP = 0x4
} AUD_DSP_ASRC_AUTO_TRACING_CLK_SEL_T;

typedef struct _ASRC_AUTO_TRACING_STATUS
{
    bool fgAsrcAutoSig0;
    bool fgAsrcAutoSig1;
    bool fgAsrcAutoSig2;
    bool fgAsrcAutoSig3;
    bool fgAsrcAutoSig4;
}ASRC_AUTO_TRACING_STATUS;

typedef enum 
{
    AUD_SE_FORCE_2CH_DOWNMIX_MANUAL      = 0,  // manual mode, set by upper layer
    AUD_SE_FORCE_2CH_DOWNMIX_AC3_KARAOKE = 1   // AC3 Karaoke mode, set by audio driver automatically
} AUD_DSP_FORCE_2CH_DOWNMIX_T;



#endif // _DSP_Struct_H_

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

#ifndef _DSP_CONST_H
#define _DSP_CONST_H

// IC Type Distinguishing Config
#include "aud_drv_config.h"
#include "chip_ver.h"

// *********************************************************************
// common define
// *********************************************************************
//DSP event
#define EvDspUop                0x00000001
#define EvDspIsr                0x00000002

#define DSPA_ID                 0
#define DSPB_ID                 1
#define DSPC_ID                 2

#define DSP_TIMEOUT                    (100000)
#define DSP_MPY_SHIFT_15_17             17
#define DSP_METADATA_PGMSCL_INIT        51
#define DSP_METADATA_SCAL_FACT_INIT     0
#define DSP_METADATA_PANMEAN_INIT       0
#define DSP_METADATA_UNIT               0x20000
#define DSP_METADATA_0p707              0x169FB

//Audio Stream Type Definition
//Decoder1 Type (0x00 ~ 0x1f)
#define TYPE_AUD_DEC1            (0x00)
#define AUD_AC3_DEC1             (TYPE_AUD_DEC1)
#define AUD_MPEG_DEC1            (TYPE_AUD_DEC1+0x01)
#define AUD_AAC_DEC1             (TYPE_AUD_DEC1+0x02)
#define AUD_PCM_DEC1             (TYPE_AUD_DEC1+0x03)
#define AUD_MP3_DEC1             (TYPE_AUD_DEC1+0x04)
#define AUD_WMA_DEC1             (TYPE_AUD_DEC1+0x05)
#define AUD_VORBIS_DEC1          (TYPE_AUD_DEC1+0x06)
#define AUD_PINK_DEC1            (TYPE_AUD_DEC1+0x07)
#define AUD_SACD_DEC1            (TYPE_AUD_DEC1+0x08)
#define AUD_SACD_DST_DEC1        (TYPE_AUD_DEC1+0x09)
#define AUD_DTS_DEC1             (TYPE_AUD_DEC1+0x0A)
#define AUD_DTS_CD_DEC1          (TYPE_AUD_DEC1+0x0B)
#define AUD_CDDA_DEC1            (TYPE_AUD_DEC1+0x0C)
#define AUD_CDDA24_DEC1          (TYPE_AUD_DEC1+0x0D)
#define AUD_MLP_DEC1             (TYPE_AUD_DEC1+0x0E)
#define AUD_TRUEHD_DEC1          (TYPE_AUD_DEC1+0x0F)
#define AUD_DTS_MA_DEC1          (TYPE_AUD_DEC1+0x10)
#define AUD_LOSSLESS_AC3_DEC1    (TYPE_AUD_DEC1+0x11)
// AAC_support_DSP
#define AUD_AAC_PURE_DEC1        (TYPE_AUD_DEC1+0x12)
#define AUD_RA_COOK_DEC1         (TYPE_AUD_DEC1+0x13)
#define AUD_DRA_DEC1             (TYPE_AUD_DEC1+0x14)
#define AUD_APE_DEC1             (TYPE_AUD_DEC1+0x15)   // mtk70169 mark add
#define AUD_FLAC_DEC1            (TYPE_AUD_DEC1+0x16)
#define DECODER1_MAX_INDEX       (0x17)

// decoder2 Type (0x40~0x4f)
#define TYPE_AUD_DEC2            (0x40)
#define AUD_AC3_DEC2             (TYPE_AUD_DEC2)
#define AUD_DTS_LBR_DEC2         (TYPE_AUD_DEC2+0x01)

#define AUD_COMM_DEC2            (0x40)
#define DECODER2_MAX_INDEX       (0x3)

// decoder3 Type (0x50~0x5f)
#define TYPE_AUD_DEC3             (0x50)
#define AUD_PCM_DEC3              (TYPE_AUD_DEC3)

// Encoder Type (0x60~0x6f)
#define TYPE_AUD_ENC              0x60
#define AUD_DDCOENC               0x60
#define AUD_DTSENC                0x61
#define AUD_MP3ENC                0x62            // -- Water (AUD_RIPPING)
// TXcoder Type (0x70~0x7f)
#define TYPE_AUD_TXC              0x70
#define AUD_COMMTXC               0x70

// mix sound Type (0x80~0xBf)
#define TYPE_AUD_MIX              0x80
#define AUD_COMM_MIX              0x80

//add fourth dec by fei.zhu 2011.1.5
#define TYPE_AUD_DEC4                   (0xA0)

#define AUD_PCM_DEC4                    (TYPE_AUD_DEC4)
#define AUD_MP3_DEC4                    (TYPE_AUD_DEC4 + 0x01)
#define AUD_A2DP_DEC4                   (TYPE_AUD_DEC4 + 0x02)

#define TYPE_AUD_DEC5                   (0xB0)
#define AUD_PCM_DEC5                    (TYPE_AUD_DEC5)

// Common RAM
#define TYPE_AUD_CMM              0xC0
#define AUD_ROMCOD                0xFC
#define AUD_POSTRAM               0xFD
#define AUD_COMMRAM               0xFE
#define AUD_TRAP                  0xFF  // Trap

// *********************************************************************
// Audio dsp capability
// *********************************************************************
#define LPCM_48k_96k_STEREO        (0x01 << 0)
#define LPCM_48k_96k_SURROUND      (0x01 << 1)
#define LPCM_192k_INCAPABLE        (0x00 << 2)
#define LPCM_192k_STEREO           (0x01 << 2)
#define LPCM_192k_SURROUND         (0x01 << 3)
#define DD_PLUS_INDEP_STEREO       (0x01 << 4)
#define DD_PLUS_INDEP_SURROUND     (0x01 << 5)
#define DD_PLUS_DEP_INCAPABLE      (0x00 << 6)
#define DD_PLUS_DEP_STEREO         (0x01 << 6)
#define DD_PLUS_DEP_SURROUND       (0x01 << 7)
#define DTS_HD_CORE_STEREO         (0x01 << 8)
#define DTS_HD_CORE_SURROUND       (0x01 << 9)
#define DTS_HD_EXT_INCAPABLE       (0x00 << 10)
#define DTS_HD_EXT_STEREO          (0x01 << 10)
#define DTS_HD_EXT_SURROUND        (0x01 << 11)
#define LOSSLESS_DD_STEREO         (0x01 << 12)
#define LOSSLESS_DD_SURROUND       (0x01 << 13)
#define LOSSLESS_MLP_INCAPABLE     (0x00 << 14)
#define LOSSLESS_MLP_STEREO        (0x01 << 14)
#define LOSSLESS_MLP_SURROUND      (0x01 << 15)
#define DRA_CORE_INCAPABLE         (0x00 << 16)
#define DRA_CORE_STEREO            (0x01 << 16)
#define DRA_CORE_SURROUND          (0x01 << 17)
#define DRA_EXT_INCAPABLE          (0x00 << 18)
#define DRA_EXT_STEREO             (0x01 << 18)
#define DRA_EXT_SURROUND           (0x01 << 19)
// *********************************************************************
// Logical Memory Map
// *********************************************************************
#define FLASHC_SA                 0
#define FLASHNC_SA                0
#define DRAMAC_SA                 0
#define DRAMANC_SA                0
#define DRAMBC_SA                 0
#define DRAMBNC_SA                0

#define DSP_EN_CLK               0x1
#define DSP_SEND_INT             0x6
#define DSP_INT_SVC              0x7
#define DSP_UOP_SVC              0x8

// decoder1 & decoder2's bitstream type definition
#define COMMDEC_STREAM          0x00
#define AC3_STREAM              (COMMDEC_STREAM)
#define MPEG12_STREAM           (COMMDEC_STREAM+0x01)
#define DTSDVD_STREAM           (COMMDEC_STREAM+0x02)
#define DTSCD_STREAM            (COMMDEC_STREAM+0x03)
#define AAC_STREAM              (COMMDEC_STREAM+0x04)
#define PCM_STREAM              (COMMDEC_STREAM+0x05)
#define CDDA24_STREAM           PCM_STREAM
#define SACD_STREAM             (COMMDEC_STREAM+0x06)
#define WMA_STREAM              (COMMDEC_STREAM+0x07)
#define VORBIS_STREAM           (COMMDEC_STREAM+0x08)
#define MP3_STREAM              (COMMDEC_STREAM+0x09)
#define CDDA_STREAM             (COMMDEC_STREAM+0x0A)
#define LPCM_STREAM_DVD_VIDEO   (COMMDEC_STREAM+0x0B)
#define LPCM_STREAM_DVD_AUDIO   (COMMDEC_STREAM+0x0C)
#define SOUND_STREAM            (COMMDEC_STREAM+0x0D)
#define MLP_STREAM              (COMMDEC_STREAM+0x0E)
#define PINK_NOISE_STREAM       (COMMDEC_STREAM+0x0F)
// New codec
#define TRUE_HD_STREAM          (COMMDEC_STREAM+0x10)
#define DTSMA_STREAM            (COMMDEC_STREAM+0x11)
#define DTS_LBR_STREAM          (COMMDEC_STREAM+0x12)
#define LOSSLESS_AC3_STREAM     (COMMDEC_STREAM+0x13)
// AAC_support_DSP
#define AAC_PURE_STREAM         (COMMDEC_STREAM+0x14)
//#define LPCM_STREAM_BD          (COMMDEC_STREAM+0x11)
#define RA_COOK_STREAM          (COMMDEC_STREAM+0x15)
#define DRA_STREAM              (COMMDEC_STREAM+0x16)
#define APE_STREAM              (COMMDEC_STREAM+0x17)   //mtk70169 mark add
#define FLAC_STREAM             (COMMDEC_STREAM+0x18)
#define COMMMIX_STREAM          0x40

// Encoder's stream type (0x20 ~ 0x3f)
#define COMMENC_STREAM          0x20
#define DDCO_ENC                (COMMENC_STREAM)
#define DTS_ENC                 (COMMENC_STREAM + 0x01)
#define MP3_ENC                 (COMMENC_STREAM + 0x02)     // -- Water (AUD_RIPPING)

//Table sampling freq index
#define SAMPLE_32K                0
#define SAMPLE_44K                1
#define SAMPLE_48K                2
#define SAMPLE_96K                3
#define SAMPLE_192K               4
#define SAMPLE_88K                5
#define SAMPLE_176K               6

//for IEC max frequency
#define SV_48K                  1
#define SV_96K                  2
#define SV_192K                 3

#define CODE3D_NONE                         0
#define CODE3D_VIRTUAL_SURROUND             1
#define CODE3D_PROLOGIC_II                  2
#define CODE3D_SPATIALIZER                  3
#define CODE3D_NEO6                         4
#define CODE3D_SPATIALIZER_HEADPHONE        5
#define CODE3D_VSURR_SOUND                  6
#define CODE3D_QSURROUND                    7
#define CODE3D_TVS                          8
#define COED3D_DOLBY_HP                     9
#define COD3D_QMSS                          10
#define COED3D_DOLBY_HP2                    11
#define CODE3D_CSII                         12
#define CODE3D_ATS                          13


/* secondary decoder */
#define MAX_DSP_CMD_NS            64

/*--------------------- Data  Interface ----------------------*/
/* DSP control related variables */
#define g_dwDspDec1Freq           (g_rDspDec1Vars.dwDspFreq)
#define g_dwDspDec1RamCodeType    (g_rDspDec1Vars.dwDspRamCodeType)
#define g_u1DspDec1StrTyp         (g_rDspDec1Vars.bDspStrTyp)
#define g_dwADacFreq              (g_rDspDec1Vars.dwADacFreq)
#define g_dwStcDiff               (g_rDspDec1Vars.dwStcDiff)
#define g_dwDspReInitPts          (g_rDspDec1Vars.dwDspReInitPts)
#define g_dwTableFreq             (g_rDspDec1Vars.dwTableFreq       )
#define g_bCode3D                 (g_rDspDec1Vars.bCode3D           )
#define g_bCode3DUpmix            (g_rDspDec1Vars.bCode3DUpmix      )
#define g_dwDspSIntFail           (g_rDspDec1Vars.dwDspSIntFail     )
#define g_dwVolPrevUsr            (g_rDspDec1Vars.dwVolPrevUsr      )
#define g_dwVolLastSetting        (g_rDspDec1Vars.dwVolLastSetting  )
#define g_dwVolSettingStep        (g_rDspDec1Vars.dwVolSettingStep  )
#define g_dwDspForceResetCntDec   (g_rDspDec1Vars.dwDspForceResetCnt)
#define g_dwDspBitrate            (g_rDspDec1Vars.dwDspBitrate      )
#define g_dwDspMpgTyp             (g_rDspDec1Vars.dwDspMpgTyp       )
#define g_dwDspAckPTS             (g_rDspDec1Vars.dwDspAckPTS       )
#define g_dwDspPlaySpeed          (g_rDspDec1Vars.dwDspPlaySpeed    )
#define g_dwDspUop                (g_rDspDec1Vars.dwDspUop          )
#define g_fgHDCDTrk               (g_rDspDec1Vars.fgHDCDTrk         )

/* DSP control related variables */
#define g_dwDspDec2Freq           (g_rDspDec2Vars.dwDspFreq         )
#define g_dwDspDec2RamCodeType    (g_rDspDec2Vars.dwDspRamCodeType  )
#define g_bDspDec2StrTyp          (g_rDspDec2Vars.bDspStrTyp        )
#define g_dwDec2ADacFreq          (g_rDspDec2Vars.dwADacFreq        )
#define g_dwDec2StcDiff           (g_rDspDec2Vars.dwStcDiff         )
#define g_dwDec2DspReInitPts      (g_rDspDec2Vars.dwDspReInitPts    )
#define g_dwDec2TableFreq         (g_rDspDec2Vars.dwTableFreq       )
#define g_dwDec2DacFreq           (g_rDspDec2Vars.dwDacFreq         )
#define g_bDec2Code3D             (g_rDspDec2Vars.bCode3D           )
#define g_bDec2Code3DUpmix        (g_rDspDec2Vars.bCode3DUpmix      )
#define g_dwDspDec2SIntFail       (g_rDspDec2Vars.dwDspSIntFail     )
#define g_dwDec2VolPrevUsr        (g_rDspDec2Vars.dwVolPrevUsr      )
#define g_dwDec2VolLastSetting    (g_rDspDec2Vars.dwVolLastSetting  )
#define g_dwDec2VolSettingStep    (g_rDspDec2Vars.dwVolSettingStep  )
#define g_dwDspDec2ForceResetCnt  (g_rDspDec2Vars.dwDspForceResetCnt)
#define g_dwDspDec2Bitrate        (g_rDspDec2Vars.dwDspBitrate      )
#define g_dwDspDec2MpgTyp         (g_rDspDec2Vars.dwDspMpgTyp       )
#define g_dwDspDec2AckPTS         (g_rDspDec2Vars.dwDspAckPTS       )
#define g_dwDspDec2PlaySpeed      (g_rDspDec2Vars.dwDspPlaySpeed    )


#endif // _DSP_CONST_H

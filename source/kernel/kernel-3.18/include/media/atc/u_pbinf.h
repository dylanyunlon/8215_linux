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

// *********************************************************************
// Memo
// *********************************************************************
/*
*/

#ifndef _U_PBINF_H_
#define _U_PBINF_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "x_typedef.h"

// use PID as component ID
#define LP_IBC_PID_SAME_FILTERID 1

//unit : second
#define AVI_AVSYNC_THRESHOLD 3

// New audio PbInfo
#define NEW_AUD_PBINFO

// RSP after parsing end
#define LP_RSP_AFTER_PRS_END 1

/// Video mpeg playback information
typedef struct _V_MPG
{
    __u64 u8PicPckPos;           ///< picture pack position
    bool fgScPartial;
    __s64  i8StcOffset;           ///< cfa provide it to LPE.
} V_MPG;

/// Video avi playback information
typedef struct _V_AVI
{
    __u32 u4TxedChunk;         ///< Txed Chunk
    __u64 u8PrevPTS;           ///< Prev Video Chung PTS Modify Flag
} V_AVI;

/// Video asf playback information
typedef struct _V_ASF
{
    __u32 u4PacketNum;         ///< packet num
    __u32 u4KfPacketNum;     ///< last key frame packet num
    __u32 u4PicPTS;            ///< pic PTS
    __u64 u8KfPicPTS;        ///< last key frame PTS
} V_ASF;
/// Video mp4 playback information
typedef struct _V_MP4
{
    __u32 u4Chunk;         ///< packet num
    __u32 u41stSamp;    ///Current chunk first sample no
    __u32 u4Samp;         ///Current sample no
    __u32 u4PicPTS;            ///< pic PTS
    __u64 u8Ofst;
} V_MP4;

/// Video mkv playback information
typedef struct _V_MKV
{
    __u64 u8PicTimeCode;         ///< Key frame time code
    __u32 u4VidBlockNo;
    __u64 u8VidClusterStartAdr;     ///< Cluster start offset of Key frame

    __u64 u8AudTimeCode;
    __u32 u4AudBlockNo;
    __u64 u8AudClusterStartAdr;
} V_MKV;

/// Video ogm playback information
typedef struct _V_OGM
{
    __s64  i8VidLastGranule;
    __u64 u8VidStartOfstKF;
    __u8  uVidPacketStartNoKF;

    __s64  i8AudLastGranule;
    __u64 u8AudStartOfst;
    __u8  uAudPacketStartNo;
} V_OGM;

//mtk40301 for swf 20090925
/// Video swf playback information
typedef struct _V_SWF
{
    __u64 u8VidPTS;
    __u64 u8VidStartOfstKF;
    __u64 u8VidStartOfst;

   // __u64 u8AudPTS;
   // __u64 u8AudStartOfst;
} V_SWF;


//mtk40156 20090814 for flv
typedef struct _V_FLV
{
    __u64 u8Pts;
    __u64 u8Offset;
    __u64 u8IFrmOft;
}V_FLV;

//mtk40156 20090908 for rm
typedef struct _V_RM
{
    __u64 u8Pts;
    __u64 u8Offset;
    __u64 u8IFrmOft;
    __u16 u2AStreamNum; //for idx data
    __u16 u2VStreamNum; //for idx data
}V_RM;

//mtk40066 20100329 for mtk private format.
typedef struct _V_MPF
{
    __u64 u8VidPTS;
    __u64 u8VidStartOfstKF;
    __u64 u8VidStartOfst;
}V_MPF;

/// Playback information of a video
typedef union
{
  V_MPG rMpg;       ///< mpeg type playback information
  V_AVI rAvi;         ///< AVI type playback information
  V_ASF rAsf;
  V_MP4 rMp4;
  V_MKV rMkv;
  V_OGM rOgm;
  V_FLV rFlv;
  V_SWF rSwf;
  V_RM rRm;
  V_MPF rMpf;
} PBINF_V;



/// Audio VCD playback information
typedef struct _A_VCD
{
  __u64 u8Pts;                          ///< PTS
  __u64 u8Offset;                     ///< Offset
}A_VCD;

/// Audio CDDA playback information
typedef struct _A_CDDA
{
  __u8 u1TrackNo;                 ///< Track number, 0xFF means invalid
  __u8 u1TrackIdx;                ///< Track index
  __u8 u1RMin;                       ///< R_time minute
  __u8 u1RSec;                       ///< R_time second
  __u8 u1RFrm;                      ///< R_time frame
  __u8 u1AMin;                       ///< A_time minute
  __u8 u1ASec;                       ///< A_time second
  __u8 u1AFrm;                      ///< A_time frame
  __u64 u8Offset;                   ///< Offset
}A_CDDA;

typedef struct _A_MP3
{
  __u32 u4BitRate;            ///< Bit Rate
  __u32 u4CurPlayTime;           ///< Current play time (sec)
  __u64 u8Offset;             ///The current play offset, unit is byte
}A_MP3;

/// Audio wma playback information
typedef struct _A_WMA
{
	__u64 u8PlayTime;         ///Play time information, unit is millisecond(ms)
	__u64 u8Offset;           ///The current play offset, unit is byte
	__u64 u8AudioPts;        ///audio playload pts
} A_WMA;

/// Audio sacd playback information
typedef struct _A_SACD
{
	__u32 u4FrameLen;         ///Frame length, unit is byte
	__u32 u4TimeCode;         ///Time code
	__u32 u4PaddingLen;       ///Padding length, unit is byte
	__u64 u8Offset;           ///The current play offset, unit is byte
} A_SACD;

/// Audio ogm playback information
typedef struct _A_OGM
{
    __s64  i8AudLastGranule;
    __u64 u8AudStartOfst;
    __u8  uAudPacketStartNo;
    __u64 u8PTS;
} A_OGM;

//mtk40301 090929
/// Audio swf playback information
typedef struct _A_SWF
{
    __u64 u8AudStartOfst;
    __u64 u8PTS;
} A_SWF;

typedef struct _A_MPF
{
    __u64 u8AudStartOfst;
    __u64 u8PTS;
} A_MPF;

/// Audio mpeg playback information
typedef struct _A_MPEG
{
    __u64 u8AudPTS;
    __u64 u8AudPckPos;           ///< Audio pack position
} A_MPEG;

/// Audio mkv playback information
typedef struct _A_MKV
{
	__u64 u8AudForMp3; /* Yi Feng modify to fix BDP00112113 @2009/06/12 */
    __u64 u8AudClusterStartAdr;
    __u64 u8AudTimeCode;
    __u32 u4AudBlockNo;	
} A_MKV;

/// Audio mpeg playback information
typedef struct _A_MP4
{
    __u64 u8AudDur;
    __u64 u8AudPts;           ///< Audio pack position
} A_MP4;

/// Audio general type playback information
typedef struct _A_GENERAL
{
  __u64 u8Pts;                            ///< PTS
}A_GENERAL;

#if 1//CONFIG_SUPPORT_AUDIO_IN
typedef struct _A_AUDIN
{
  __u32 u4BitRate;            ///< Bit Rate
  __u32 u4CurPlayTime;           ///< Current play time (sec)
  __u64 u8Offset;             ///The current play offset, unit is byte
}A_AUDIN;
#endif

#define CFA_MPG_DVD_AUDIO_DE_SUPPORT            1
#if CFA_MPG_DVD_AUDIO_DE_SUPPORT
 /// DVD audio playback information
typedef struct _A_DVDAUDIO
{
    __u64 u8AudPTS;
    __u64 u8AudPckPos;           ///< Audio pack position
} A_DVDAUDIO;
#endif

typedef struct _A_APE  //mtk70169 for time show
{
  __u32 u4TimeInfoFrameNumLo;
  __u32 u4TimeInfoFrameNumHi;
  __u32 u4ApeErrType;
  //begin add by mtk40292 for get APE info	
  __u32 u4ApeBanknumInFrame;
  //end add by mtk40292 for get APE info
}A_APE;

typedef struct _A_FLAC
{
  __u32 u4TimeInfoFrameNumL24;
  __u32 u4TimeInfoFrameNumH12;
  __u32 u4FlacErrType;
}A_FLAC;

#ifdef NEW_AUD_PBINFO
typedef union
{
  A_GENERAL  rGeneral;         ///< General audio type playback information
  A_VCD      rVCD;             ///< VCD playback information
  A_CDDA     rCDDA;            ///< CDDA playback information
  A_MP3      rMP3;             ///< MP3 playback information
  A_WMA      rWMA;             ///< WMA playback information
  A_SACD     rSACD;            ///< SACD playback information
  A_OGM      rOGM;
  A_SWF      rSWF;
  A_MPEG     rMPEG;           ///< MPEG playback information
  A_MKV      rMKV;
  #if CFA_MPG_DVD_AUDIO_DE_SUPPORT
  A_DVDAUDIO      rDVDAUDIO;           ///< DVD AUDIO playback information
  #endif
  A_MP4      rMP4; 
#if 1//CONFIG_SUPPORT_AUDIO_IN
  A_AUDIN   rAudIn;
#endif
  //A_MPF     rMpf;
  A_APE     rAPE;    //mtk70169 for time show
  A_FLAC   rFLAC;
}PBINF_AUD;

typedef struct _PBINF_A
{
    __u32 u8DspPlayBackTime;      ///< DSP playback time
    PBINF_AUD rPbInf;             ///<Audio Playback information
} PBINF_A;

#else
/// access unit information of Audio
typedef union
{
  A_GENERAL  rGeneral;         ///< General audio type playback information
  A_VCD      rVCD;             ///< VCD playback information
  A_CDDA     rCDDA;            ///< CDDA playback information
  A_MP3      rMP3;             ///< MP3 playback information
  A_WMA      rWMA;             ///< WMA playback information
  A_SACD     rSACD;            ///< SACD playback information
#if 1//CONFIG_SUPPORT_AUDIO_IN
  A_AUDIN   rAudIn;
#endif
  A_MPF     rMpf;
  A_APE     rAPE;    //mtk70169 for time show
  A_FLAC   rFLAC;  
}PBINF_A;
#endif

/// SubPic playback information
typedef struct _PBINF_SP
{
  // TODO: should change to a knowed name
  __u64 u8CusInf;                         ///< custom information
  __u32 u4HLI_S_PTM;                      ///< High light information, 0xffffffff means invalid
  __u32 u4HLI_E_PTM;                      ///< High light information
  #if  CFA_MPG_DVD_AUDIO_DE_SUPPORT
  __u16 u2SpStreamId;    ///< sub picture stream id
  __u16 u2SpUnitIndexId;///< sub picture unit index
#endif
}PBINF_SP;

/// NV playback information
typedef struct _PBINF_NV
{
  // TODO: should change to a knowed name
  __u64 u8CusInf;                         ///< custom information
  __u64 u8Pts;                            ///< PTS
}PBINF_NV;

/// Graphic playback information
typedef struct _PBINF_G
{
  // TODO: should change to a knowed name
  __u64 u8CusInf;                         ///< custom information
}PBINF_G;

#ifdef __cplusplus
}
#endif

#endif //#ifndef _U_PBINF_H_


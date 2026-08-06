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

#ifndef DMX_CFA_FLV_H
#define DMX_CFA_FLV_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
										macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/


#define CFA_FLV_VIDEO_STREAM_ID				 0	/* 0 in spec */
#define CFA_FLV_AUDIO_STREAM_ID				 1	/* 0 in spec */

/* /> CFA FLV tag header size (1 + 3 + 3 + 1 + 3) */
#define FLV_TAG_HDR_SIZE								(11)

/* /> 8 + 17 + 5 + 8 + 3 (+ 16 + 16) + 2         bits */
#define FLV_EXTRA_DATA_SIZE						(10)

#define FLV_READ_FOR_CODEC_SIZE				(FLV_TAG_HDR_SIZE + FLV_EXTRA_DATA_SIZE)

/* /> CFA FLV tag size(__u32) */
#define FLV_TAG_SIZE_BYTES							(4)

#define CONFIG_CFA_FLV_SUPPORT_ERR_NOTI_LPE 0

#define CFA_MPG_QUERY_INF_MAX_STRM_NS	(32)

#define FLV_FRAME_TYPE_VALUE_I					((__u8)0x0)
#define FLV_FRAME_TYPE_VALUE_P					((__u8)0x20)
#define FLV_FRAME_TYPE_VALUE_B					((__u8)0x40)

#define FLV_FILE_HEADER_LEN						(9)

#define FLV_TAG_TYPE_VALUE_AUD					((__u8)8)
#define FLV_TAG_TYPE_VALUE_VID					((__u8)9)
#define FLV_TAG_TYPE_VALUE_SCRIPT			((__u8)18)
#define FLV_TAG_TYPE_VALUE_UNKNOWN			((__u8)0xFF)
#define FLV_TAG_DATA_MAX_SIZE					((__u32)0x00FFFFFF)

#define IS_FLV_VALID_TAG(tag)		((FLV_TAG_TYPE_VALUE_AUD == (tag)) || \
	(FLV_TAG_TYPE_VALUE_VID == (tag)) || \
	(FLV_TAG_TYPE_VALUE_SCRIPT == (tag)))

#define IS_FLV_VALID_VIDEO_PREVTAGSZ(prevtagsz, codec, datasz)	\
	 (((CFA_FLV_VID_CODEC_VP6 == (codec)) || \
			(CFA_FLV_VID_CODEC_VP6A == (codec))) ? \
		((prevtagsz == datasz + FLV_TAG_HDR_SIZE) || \
		(prevtagsz + 1 == datasz + FLV_TAG_HDR_SIZE)) : \
		(prevtagsz == datasz + FLV_TAG_HDR_SIZE))

#define IS_FLV_VALID_VIDEO_PREVTAGSZ_EX(prevtagsz, codecid, datasz)  \
	 (((4 == (codecid)) || (5 == (codecid))) ?	\
	 ((prevtagsz == datasz + FLV_TAG_HDR_SIZE) ||\
	 (prevtagsz + 1 == datasz + FLV_TAG_HDR_SIZE)) : \
	 (prevtagsz == datasz + FLV_TAG_HDR_SIZE))

#define IS_FLV_VALID_PREVTAGSZ(prevtagsz, datasz)  \
	 ((prevtagsz == datasz + FLV_TAG_HDR_SIZE) ||\
	 (prevtagsz + 1 == datasz + FLV_TAG_HDR_SIZE))

#define IS_FLV_FFMPEG_FLVENC_VIDEO_BUG_PREVTAGSZ(prevtagsz, codec, datasz)	\
	 ((AVCODEC_ID_VP6 == (codec))  ? \
	 (prevtagsz + 1 == datasz + FLV_TAG_HDR_SIZE) : FALSE)


/* /> FLV Audio SampleRate */
#define FLV_AUD_SAMPLERATE_5K5HZ					(5512)
#define FLV_AUD_SAMPLERATE_11KHZ					(11025)
#define FLV_AUD_SAMPLERATE_22KHZ					(22050)
#define FLV_AUD_SAMPLERATE_44KHZ					(44100)

#define FLV_SEQ_HDR_SET_MAX_CNT					(63)
#define FLV_PIC_HDR_SET_MAX_CNT					(255)

/* /> One Audio AU's Max Size in FLV Pure Audio File's FF & RW */
#define FLV_AUD_UNIT_MAX_SZ_IN_FFRW			(100 * 1024)
#define FLV_AUD_RW_MIN_CNT_DELTA_BYTE		(10 * 1024)

#define FLV_GET_TIMESTAMP(u4TimeStamp, u1ExTimeStamp) ( \
	(__u32)((0x00FFFFFF & (u4TimeStamp)) + \
	(__u32)((u1ExTimeStamp & 0x7F) << 24)))

#define FLV_GET_DATA_SIZE(u4DataSize)  \
	((__u32)((u4DataSize) & 0x00FFFFFF))

#define FLV_GET_STM_UID(u4StmID)			 \
	((__u32)((u4StmID) & 0x00FFFFFF))

#define FLV_GET_VID_CODEC_ID(u1VidFstByte)	\
	((__u8)((u1VidFstByte) & 0x0F))

#define FLV_GET_AUD_CODEC_ID(u1AudFstByte)	\
	((__u8)(((__u8)((u1AudFstByte) >> 0x04)) & 0x0F))

/* /> FLV CFA query information type, max 32 information */
enum {
	CFA_FLV_QUERY_INF_TYPE_NONE = 0x00000000,	/* < none */
	CFA_FLV_QUERY_INF_TYPE_VC1_MODE = 0x00000001,	/* < WVC1 parsing mode */
};

typedef enum {
	CFA_FLV_VC1_MD_WITH_SC,
	CFA_FLV_VC1_MD_WITH_OUT_SC,
} CFA_FLV_VC1_MODE_E;

typedef struct CfaFlvQIVc1Mode {
	CFA_FLV_VC1_MODE_E eVc1Mode;
} CfaFlvQIVc1Mode_T;

typedef enum {
	CFA_AUDIO_TYPE_UNKNOWN,
	CFA_AUDIO_TYPE_STEREO,
	CFA_AUDIO_TYPE_MONO
} CfaFlvAudioType_E;

typedef enum {
	CFA_FLV_SKIP_NONE,	/* > Do not skip */
	CFA_FLV_SKIP_BY_PACKET,	/* > Skip by packet number */
	CFA_FLV_SKIP_BY_PTS	/* > Skip by PTS */
} CfaFlvSkipType_E;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	bool fgHasVid;
	bool fgHasAud;

	__u64 u8VidSa;
	__u64 u8VidEa;

	__u32 u4AV1stTime;	/* > ms */

	CfaFlvSkipType_E eSkipMode;
	__u32 u4SkipPacketCount;
	__u64 u8DispPicPTS;

	__u64 u8AudSa;
	__u64 u8AudEa;
	__u64 u8SeekTime;	/* > ms */

	__u64 u8FileSz;

	__u8 u8IframeNum;	/* > get I_Frame count number after setrange */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaFlvRange_T;

typedef struct {
	/* Information of Current Key Frame */
	__u64 u8StartPts;	/* The Start Pts of the Key Frame                                */
	__u64 u8StartOfst;	/* The Start file offset of the Key Frame  */
	__u64 u8CurKeyFrmMaxSz;	/* The max size of current key Frame                     */
	__u32 u4PureAudTxUnitSz;	/* One Audio AU's Max Size in FLV Pure Audio File's FF & RW */
	CfaFlvRange_T rRange;
} CfaFlvKeyFrmRange_T;

typedef struct {
	__u64 u8IFrmCurOfst;
	__u64 u8AudCurOfst;
	__u64 u8VidCurOfst;
	__u64 u8PrsCurOfst;
} CfaFlvCurPosInfo_T;

typedef struct {
	__u16 u2SeqParSetLen;
	__u8 *puSeqParSet;
} SeqParSet_T;

typedef struct {
	__u16 u2PicParSetLen;
	__u8 *puPicParSet;
} PicParSet_T;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u8 u1NumSeqParSet;
	SeqParSet_T rSeqParSet[FLV_SEQ_HDR_SET_MAX_CNT];
	__u8 u1NumPicParSet;
	PicParSet_T rPicParSet[FLV_PIC_HDR_SET_MAX_CNT];
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} DecoderCfgInfo_T;

typedef struct {
	__u8 uObjectProfile;
	__u8 uSampFreqIdx;
	__u8 uChannels;
	__u8 uHeaderLen;
	__u8 *puHeader;
} AacCfgInfo_T;

typedef struct {
	__u64 u8FileSize;
	__u64 u8HeaderSize;
	bool fgCfgRespliter;
} CfaFlvCfgFileInfo_T;

typedef struct {
	__u8 u1StrmNum;	/* CFA_FLV_AUDIO_STREAM_ID */
	AVCODECID_T eCodecID;
	CfaFlvAudioType_E eSoundType;	/* the number of audio channel */
	__u32 u4SoundRate;	/* Samples per second */
	__u16 u2SoundSize;	/* Size of each sample */
	AacCfgInfo_T rAacInfo;
} CfaFlvCfgAudInfo_T;

typedef struct {
	__u8 u1StrmNum;	/* CFA_FLV_VIDEO_STREAM_ID */
	__u8 u1FrameRate;
	__u8 u1PrevTagSzAdd;
	AVCODECID_T eCodecID;
	VCODECVERSION_T rVersion;
	__u8 u1PayloadLenFieldSz;	/* Payload/Slice Length Field Size Per AVC Slice or Advance HEVC Slice */
	__u8 u1SeqHdrLen;
	__u8 *puSeqHdr;
} CfaFlvCfgVidInfo_T;

typedef struct {
	CfaFlvCfgFileInfo_T rCfaFlvCfgFileInfo;
	CfaFlvCfgAudInfo_T rCfaFlvCfgAudInfo;
	CfaFlvCfgVidInfo_T rCfaFlvCfgVidInfo;
} CfaFlvCfgInfo_T;

typedef struct {
	__u8 u1StrmId;
	__u32 u4Pts;
} CfaFlvGetPtsInfo;

#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_FLV_H */

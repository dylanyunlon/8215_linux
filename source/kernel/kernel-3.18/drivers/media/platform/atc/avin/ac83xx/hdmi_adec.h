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

#ifndef HDMI_ADEC_H_
#define HDMI_ADEC_H_
#include <windows.h>
#include "mm_debug.h"
#include "drv_aud.h"
#include "chip_ver.h"

#define INVALID_HANDLE_VALUE  ((HANDLE)-1)

typedef void          *HAVDECINST;
typedef void          *HAVINST;
typedef void          *HAVININST;
typedef void          *HDRMINST;

typedef enum _AVMEDIATYPE {
	AVMEDIA_TYPE_MTK_VIDEO,
	AVMEDIA_TYPE_MTK_AUDIO,
	AVMEDIA_TYPE_EXTSUBPIC,
	AVMEDIA_TYPE_INTSUBPIC,
	AVMEDIA_TYPE_INTSUBTITLE,
	AVMEDIA_TYPE_EXTSUBTITLE,
	AVMEDIA_TYPE_LINEIN_AUD,
	AVMEDIA_TYPE_CC,
} AVMEDIATYPE_T;

typedef enum _AVFORMATTYPE {
	AVFMT_TYPE_VIDEOFMT,
	AVFMT_TYPE_AUDIOFMT,
	AVFMT_TYPE_WAVEFMT
} AVFORMATTYPE_T;

typedef enum _AVCODECID {
	AVCODEC_ID_NONE = 0,
	AVCODEC_ID_UNKNOWN = 1,
	AVCODEC_ID_INVALID_VALUE = 2,
	AVCODEC_ID_MPEG1_2 = 3,/*vodeo codec type*/
	AVCODEC_ID_DIVX311 = 4,
	AVCODEC_ID_MP4V = 5,
	AVCODEC_ID_H263 = 6,
	AVCODEC_ID_H264 = 7,
	AVCODEC_ID_WVC1 = 8,
	AVCODEC_ID_DIVX4 = 9,
	AVCODEC_ID_DIVX6 = 10,
	AVCODEC_ID_MPEG1 = 11,            /*/< MPEG1 mode*/
	AVCODEC_ID_MPEG2 = 12,            /*/< MPEG2 mode*/
	AVCODEC_ID_DIVX3 = 13,            /*/< DIVX3 mode*/
	AVCODEC_ID_DIVX5 = 14,            /*/< DIVX5 mode*/
	AVCODEC_ID_MPEG4 = 15,            /*/< MPEG4 mode*/
	AVCODEC_ID_MPEG4_AVC = 16,        /*/< mpeg4 avc video stream*/
	AVCODEC_ID_SMPTE_VC1 = 17,        /*/< smpte vc-1 video stream*/
	AVCODEC_ID_MVC1 = 18,
	AVCODEC_ID_WMV1 = 19,
	AVCODEC_ID_WMV2 = 20,
	AVCODEC_ID_WMV3 = 21,
	AVCODEC_ID_WMVA = 22,
	AVCODEC_ID_MP43 = 23,
	AVCODEC_ID_VC1 = 24,/*audio codec type*/
	AVCODEC_ID_MP2V = 25,
	AVCODEC_ID_VP6 = 26,
	AVCODEC_ID_VP8 = 27,
	AVCODEC_ID_RV = 28,
	AVCODEC_ID_FLV = 29,
	AVCODEC_ID_AC3 = 30,
	AVCODEC_ID_MPEG = 31,
	AVCODEC_ID_SORENSON = 32,
	AVCODEC_ID_MP3 = 33,
	AVCODEC_ID_DTS = 34,
	AVCODEC_ID_MLP = 35,
	AVCODEC_ID_CDDA = 36,
	AVCODEC_ID_LPCM = 37,
	AVCODEC_ID_WMA = 38,
	AVCODEC_ID_AAC = 39,
	AVCODEC_ID_DTS_CD = 40,
	AVCODEC_ID_VORBIS = 41,
	AVCODEC_ID_HDCD = 42,
	AVCODEC_ID_SACD = 43,
	AVCODEC_ID_DTSHD_PRI_XLL = 44,
	AVCODEC_ID_AAC_PURE = 45,
	AVCODEC_ID_PCM = 46,
	AVCODEC_ID_EAC3 = 47,              /*/< EAC3 mode*/
	AVCODEC_ID_EAC3SEC = 48,              /*/< EAC3SEC mode*/
	AVCODEC_ID_SDDS = 49,             /*/< SDDS mode*/
	AVCODEC_ID_WAV = 50,
	AVCODEC_ID_DTSCD = 51,            /*/< DTS CD mode*/
	AVCODEC_ID_APE = 52,
	AVCODEC_ID_FLAC = 53,

	AVCODEC_ID_HDMV_LPCM = 54,        /*/< HDMV LPCM mode*/
	AVCODEC_ID_DOLBY_LOSSLESS = 55,   /*/< Dolby Lossless mode*/
	AVCODEC_ID_DOLBY_DIGITAL_PLUS = 56,   /*/< Dolby Digital Plus mode*/
	AVCODEC_ID_DTSHD_NO_XLL = 57,          /*/< DTS-HD except XLL mode*/
	AVCODEC_ID_DTSHD_XLL = 58,             /*/< DTS-HD XLL mode*/
	AVCODEC_ID_DOLBY_DIGITAL_PLUS_SECONDARY = 59,
	AVCODEC_ID_DTSHD_SECONDARY = 60,  /*/DTS-HD audio stream for Secondary audio.*/
	AVCODEC_ID_DTSESMATRIX_6_1_CHAN = 61,            /* DTS Extended Surround Matrix 6.1 channel */
	AVCODEC_ID_DTSESDISCRETE_6_1_CHAN = 62,         /* DTS Extended Surround Discrete 6.1 channel */
	AVCODEC_ID_DTSESDISCRETE_8_CHAN = 63,              /* DTS Extended Surround Discrete 8 channel */
	AVCODEC_ID_DTS_96_24 = 64,                                 /* DTS 96/24 */
	AVCODEC_ID_DTS_96_24_ES_MATRIX = 65,                 /* DTS 96/24 Extended Surround Matrix */
	AVCODEC_ID_DtsHd_Es_Matirx_6_1_Chan = 66,
	AVCODEC_ID_DtsHd_Es_Discrete_6_1_Chan = 67,
	AVCODEC_ID_DtsHd_ES_Discrete_8_Chan = 68,
	AVCODEC_ID_DtsHd_96_24 = 69,
	AVCODEC_ID_DtsHd_96_24_Es_Matrix = 70,
	AVCODEC_ID_PPCM = 71,              /*/< Packed PCM mode*/
	AVCODEC_ID_DOLBY_DIGITAL = 72,     /*/< Dolby Digitai*/
	AVCODEC_ID_MPEG2_EX = 73,        /*/< MPEG_2 with extention*/
	AVCODEC_ID_RA_COOK = 74,
	AVCODEC_ID_DVDAPPCM = 75,        /*< dvd audio PPCM*/
	AVCODEC_ID_DVDALPCM = 76,         /*< dvd audio LPCM*/

	AVCODEC_ID_TEXT_SRT = 77,
	AVCODEC_ID_TEXT = 78,
	AVCODEC_ID_SUB = 79,
	AVCODEC_ID_XSUB = 80,
	AVCODEC_ID_XSUB_PLUS = 81,
	AVCODEC_ID_MJPEG = 82,
	AVCODEC_ID_CC = 83,
	AVCODEC_ID_H265 = 84,
	AVCODEC_ID_HDMI_PCM = 85,

	AVCODEC_ID_RA_LPCJ = 86,
	AVCODEC_ID_RA_28_8 = 87,
	AVCODEC_ID_RA_DNET = 88,
	AVCODEC_ID_RA_SIPR = 89,
	AVCODEC_ID_RA_RALF = 90,
	AVCODEC_ID_RA_ATRC = 91,

	AVCODEC_ID_NELLYMOSER = 92,
	AVCODEC_ID_SPEEX = 93,

	AVCODEC_ID_SCRV = 94,

	AVCODEC_ID_AMR = 95,
	AVCODEC_ID_AWB = 96,
} AVCODECID_T;

/*add by fei 2010.12.6*/
typedef enum {
	AVCODEC_AUD_DIV_TYPE_LOGO_DOLBY,
	AVCODEC_AUD_DIV_TYPE_LOGO_DTS,
	AVCODEC_AUD_DIV_TYPE_ANALOG_OUTPUT_CHANNEL,
	AVCODEC_AUD_DIV_TYPE_HDMI_OUTPUT_CHANNEL,
	AVCODEC_AUD_DIV_TYPE_POST_PROCESS,
	AVCODEC_AUD_DIV_TYPE_CODEC_SUPPORT
} AVCODEC_AUD_DIV_TYPE_T;

typedef struct _AVCODEC_AUD_DIV_INFO_T {
	AVCODEC_AUD_DIV_TYPE_T		e_type;
	unsigned char				u1_setting;
} AVCODEC_AUD_DIV_INFO_T;

#define SUPPORT_AUDIO_DRIVER_OUTPUT 1

typedef enum _PARMTYPE {
	APARAM_NONE_T,
	APARAM_PCM_T,
	APARAM_COOK_T,
	APARAM_WMA_T,
	APARAM_APE_T,
	APARAM_FLAC_T,
	APARAM_VOLUME_T,
	APARAM_GET_EOS,
	APARAM_GET_SPECTRUM,
	APARAM_ORIG_SAMPRATE,
	APARAM_SET_SE,
	APARAM_SET_AC3DRC,
	APARAM_SET_DTSDRC,
	APARAM_FEATURE,
	APARAM_BMANAGEMENT,
	APARAM_GET_TARGETPTS,
	APARAM_SET_TARGETPTS,
	APARAM_DISABLE_AVSYNC,
	APARAM_GETCURPLAYTIME,
	APARAM_SET_STC_VALID,
	APARAM_GET_VOLUME_T,
	APARAM_SET_REAR_VOLUME_T,
	APARAM_GET_REAR_VOLUME_T,
	APARAM_AUD_MUTE,
	VPARAM_SET_VDEC_PROG_TYPE,
	VPARAM_GET_VIDEO_ASPECT_RATIO,
	APARAM_MIRACAST_T,
	APARAM_SET_WRITE_T,
	APARAM_SET_FORMAT,
	PARAM_MAX_T,
} ParamType;

static const char * const Param2Str[] = {
	"APARAM_NONE_T",
	"APARAM_PCM_T",
	"APARAM_COOK_T",
	"APARAM_WMA_T",
	"APARAM_APE_T",
	"APARAM_FLAC_T",
	"APARAM_VOLUME_T",
	"APARAM_GET_EOS",
	"APARAM_GET_SPECTRUM",
	"APARAM_ORIG_SAMPRATE",
	"APARAM_SET_SE",
	"APARAM_SET_AC3DRC",
	"APARAM_SET_DTSDRC",
	"APARAM_FEATURE",
	"APARAM_BMANAGEMENT",
	"APARAM_GET_TARGETPTS",
	"APARAM_SET_TARGETPTS",
	"APARAM_DISABLE_AVSYNC",
	"APARAM_GETCURPLAYTIME",
	"APARAM_SET_STC_VALID",
	"APARAM_GET_VOLUME_T",
	"APARAM_SET_REAR_VOLUME_T",
	"APARAM_GET_REAR_VOLUME_T",
	"APARAM_AUD_MUTE",
	"VPARAM_SET_VDEC_PROG_TYPE",
	"VPARAM_GET_VIDEO_ASPECT_RATIO",
	"APARAM_MIRACAST_T",
	"APARAM_SET_WRITE_T",
	"APARAM_SET_FORMAT",
	"PARAM_MAX_T"
};

typedef struct _AVDECODERCFG {
	signed int       i4Width;
	signed int       i4Height;
} AVDECODERCFG_T;

typedef struct _AVSAMPLEINFO {
	AVMEDIATYPE_T eType;
	AVFORMATTYPE_T eFormatType;
	void      **ppvOutputBuf;
	unsigned int     u4SampleSize;
	unsigned int     u4SpecSampleInfoSize;
	void      *pvSpecSampleInfo;
} AVSAMPLEINFO_T;

typedef enum {
	AUD_OUTPUT_NONE,
	AUD_OUTPUT_FRONT,
	AUD_OUTPUT_REAR,
	AUD_OUTPUT_FRONT_REAR,
} AUD_OUTPUT_T;

typedef enum _INS_FLAG {
	INS_FLAG_ISOURDMX       = 0x1 << 0,
	INS_FLAG_IAVINDMX       = 0x1 << 1,
	INS_FLAG_OPEN_DRV_ONLY  = 0x1 << 2,

	INS_FLAG_SINK_FRONT     = 0x1 << 8,
	INS_FLAG_SINK_REAR      = 0x1 << 9,
} INS_FLAG;


typedef struct _AV_BASE_ {
	AVCODECID_T eAVCodeC;
	void (*Release)(HANDLE hInst);

	BOOL(*SetParam)(HANDLE hInst, ParamType eParamType, VOID * prParam, bool flag);

	void (*GetParam)(HANDLE hInst, ParamType eParamType, VOID *prParam);

	BOOL(*SetInputBuf)(HANDLE hInst, VOID *pvBuf, UINT32 u4BufSz, VOID *pvOutBuf, UINT32 u4OutBufSz);
	void (*SetOutputBuf)(HANDLE hInst, void *pvBuf, UINT32 u4BufSz);
	void *(*GetOutputBuf)(HANDLE hInst, UINT32 *pu4BufSz);

	BOOL(*Start)(HANDLE hInst, HANDLE hEvent);
	void (*Stop)(HANDLE hInst);

	BOOL(*Freeze)(HANDLE hInst);
	BOOL(*UNFreeze)(HANDLE hInst);
	BOOL(*Refresh)(HANDLE hInst);
	void (*GetSampleInfo)(HAVDECINST hInst, AVSAMPLEINFO_T *prSampleInfo);

	BOOL(*SetDiversityInfo)(HANDLE hInst, AVCODEC_AUD_DIV_INFO_T  * ptAud_div_info);
	BOOL(*SetSpeed)(HANDLE hInst, UINT32 u4Speed);
	void *(*GetDevHandle)(HANDLE hInst);

	BOOL(*GetAfifoAddr)(HANDLE hInst, void *prParam);
	BOOL(*OpenAOut)(HANDLE hInst, AUD_OUTPUT_T AOutPut);
	BOOL(*CloseAOut)(HANDLE hInst, AUD_OUTPUT_T AOutPut);
	BOOL(*GetWritePoint)(HANDLE hInst, void *prParam);
	BOOL(*GetAfifoSpareLen)(HANDLE hInst, UINT32 * pu4SpareLen);
} AV_BASE;


typedef struct _VIDEO_DECODER_ {
	AV_BASE sAVBase;
	HANDLE  hDev;
} VIDEO_DECODER;

typedef enum {
	AUD_CMD_STATUS_IDLE = 0,
	AUD_CMD_STATUS_PLAY,
	AUD_CMD_STATUS_STOP,
	AUD_CMD_STATUS_PAUSE
} AUD_CMD_STATUS_T;

static const char *const StatusStr[] = {
	"IDLE",
	"PLAY",
	"STOP",
	"PAUSE"
};

typedef struct _AUDIO_DECODER {
	AV_BASE sAVBase;
	struct file  *AudioFilp;
	BOOL    bIsOurDmx;
	BOOL    fgFreeze;
	AVCODECID_T eAUDCodec;
	AUD_CMD_STATUS_T eAudCmdStatus;
	UINT32 u4AudFmt;
	AUD_OUTPUT_T eOutputType;
} AUDIO_DECODER;

typedef struct {
	UINT32  u4WritePointer;
	UINT32  u4Len;
	UINT32  u4FifoSA;
	UINT32  u4FifoEA;
} AUDIO_BUF_INFO_T;

extern BOOL g_ADecIsUsed;
extern BOOL g_fgDumpData;

typedef struct {
	WORD  wFormatTag;
	WORD  nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;
} WAVEFORMATEX;

extern bool ADE_IOControl(u32 context, u32 code, u8 *pInBuffer, u32 inSize,
			  u8 *pOutBuffer, u32 outSize, u32 *pOutSize);
extern UINT32 g_u4AVDecLogLevel;

#endif




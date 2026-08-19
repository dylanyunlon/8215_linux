/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


/*******************************************************************************
*
* Filename:
* ---------
* file avdmx.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #10 $
* $Modtime:$
* $Log:$
*
*******************************************************************************/

#ifndef AVDMX_H_
#define AVDMX_H_

#include "msdkcore_defs.h"
#include "SPDec.h"
#include "mm_errcode.h"
#include "mm_log.h"
#include "linux/types.h"
#include "stdint.h"

#define FILE_HEADER_LEN         (26)

#define MP3_ID3_HEADER_LEN      (10)

#define DEFAULT_PLAY_AUD_ID     (0)

#define DEBUG_LEVEL_CNT         30


typedef struct
{
#ifdef MM_ATE_CHECK
    __u32 u4MMATECHKStart;
#endif
    char  szTitle[MAX_ID3INFO_LEN + 1];
    char  szYear[MAX_ID3INFO_LEN + 1];
    char  szAlbum[MAX_ID3INFO_LEN + 1];
    char  szAuthor[MAX_ID3INFO_LEN + 1];
    char  szGenre[MAX_ID3INFO_LEN + 1];
    __u32 u4Duration;
#ifdef MM_ATE_CHECK
    __u32 u4MMATECHKEnd;
#endif
} AUDIO_ID3INFO_T;

typedef struct
{
    /*ID3 Infomation*/
    char  *pTitle ;
    char  *pYear  ;
    char  *pAlbum ;
    char  *pAuthor;
    char  *pGenre ;
    char  *pAlbArtist;
    char  *pComposer;
    void  *pApicData;

    __u32  u4TitleLen ;
    __u32  u4YearLen  ;
    __u32  u4AlbumLen ;
    __u32  u4AuthorLen;
    __u32  u4GenreLen ;
    __u32  u4AlbArtistLen;
    __u32  u4ComposerLen;
    __u32  u4CDTrackNumLen;
    __u32  u4LocationLen;
    __u32  u4APicDataLen;
    /*audio information*/
    __u32 u4AStmCnt;
    __u32 u4SampleRate[MAX_STM_NUM];
    AVCODECID_T eAStmType[MAX_STM_NUM];
    __u32 u4AudDuration;

    /*video information*/
    __u32 u4BitRate;
    __u32 u4Height;
    __u32 u4Width;
    __u32 u4FrameRate;
    __u32 u4FileDateLen;
    __u32 u4FilePsshLen;
    char  *pLocation;
    char  *pCDTrackNum;
    char  *pFileDate;
    char  *pFilePssh;
    AVCODECID_T eVStmType;
    VIDEO_ROTATION_T rVidRotation;
    __u32 u4VidDuration;

    bool fgHasVideo;
    bool fgHasAudio;
    __u64 u8Duration;
    AV_FILE_TYPE_T eFileType;
} FILE_INFO_T;

typedef struct
{
    __u8  *pAPicBuf;
    __u32 u4APicBufLen;
} AUDIO_ID3INFO_APIC_T;

typedef enum
{
    STREAM_VIDEO    = (__u32)1 << 0,
    STREAM_AUDIO    = (__u32)1 << 1,
    STREAM_SUBTITLE = (__u32)1 << 2
} STREAM_TYPE_T;


typedef struct
{
    AVCODECID_T    eVCodec;
    __u32          u4FrameRate;
    __u32          u4BitRate;
    __u32          u4Width;
    __u32          u4Height;
} MSDKCORE_VIDEO_INFO_T;

typedef struct
{
    AVCODECID_T     eACodec;
    __u32          u4SampleRate;
    __u32          u4BitRate;
} MSDKCORE_AUDIO_INFO_T;

typedef struct
{
    AVCODECID_T     eSubCodec;
    __u32          u4Width;
    __u32          u4Height;
} MSDKCORE_SUBTITLE_INFO_T;

typedef struct
{
#ifdef MM_ATE_CHECK
    __u32 u4MMATECHKStart;
#endif
    AV_FILE_TYPE_T              eFileType;
    __u64                       u8FileSize;
    __u64                       u8Duration;
    __u32                       u4AudStmCnt;
    __u32                       u4SpStmCnt;

    MSDKCORE_VIDEO_INFO_T       rVdoInfo;
    MSDKCORE_AUDIO_INFO_T       rAudInfo;
    MSDKCORE_SUBTITLE_INFO_T    rSubInfo;
#ifdef MM_ATE_CHECK
    __u32 u4MMATECHKEnd;
#endif
} MSDKCORE_MEDIA_INFO_T;


typedef struct _MSDKCORE_PB_QUALITY
{
    __u32  u4Type;
    __u32  u4DropDataLevel;
    __s32  i4Lateness;
    __u64  u8CurrentTime;
} MSDKCORE_PB_QUALITY_T;


typedef struct _MSDKCORE_JUMPINFO
{
    bool    fgIsJumping;
    __s64   i8JumpTime;
} MSDKCORE_JUMPINFO_T;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief for Init
*     @param void
*     @return MRESULT : RET_MSDKC_OK for succsee
*     @note this function is not use
*/
MRESULT     AVDmx_Init(void);

/** @brief for Deinit
*     @param void
*     @return void
*     @note this function is not use
*/
void        AVDmx_Deinit(void);

/** @brief for create a AVDmx instance
*     @param void
*     @return HAVINST : AVDmx handle
*/
HAVINST     AVDmx_CreateInstance(void);

/** @brief for releasing all resources of AVDmx
*     @param hInst : AVDmx handle
*     @return void
*/
void        AVDmx_Release(HAVINST hInst);

/** @brief for starting container track analyze module
*     @param hInst : AVDmx handle
*     @param u4StreamType : Stream type for Starting
*     @param u4StreamID : Stream ID for Starting
*     @return MRESULT : the execution result status of the interface
*     @note this functions trigger container track analyze thread to read AU from file,must called after  AVDmx_PreparePlay,
*/
MRESULT     AVDmx_StartStream(HAVINST hInst, __u32 u4StreamType, __u32 u4StreamID);

/** @brief for stoping container track analyze module
*     @param hInst : AVDmx handle
*     @param u4StreamType : Stream type for Starting
*     @param u4StreamID : Stream ID for Starting
*     @return MRESULT : the execution result status of the interface
*     @note this functions trigger container track analyze release,must called after  AVDmx_StartStream,
*/
MRESULT     AVDmx_StopStream(HAVINST hInst, __u32 u4StreamType, __u32 u4StreamID);
MRESULT     AVDmx_SetSTInst(HAVINST hInst, void *m_hSTInst);
void       *AVDmx_GetSTInst(HAVINST hInst);

/** @brief for preparing container track analyze module
*     @param hInst : AVDmx handle
*     @param u4StreamType : Stream type for Starting
*     @param u4StreamID : Stream ID for Starting
*     @return MRESULT : the execution result status of the interface
*     @note this functions create container track analyze
*/
MRESULT     AVDmx_PreparePlay(HAVINST hInst, __u32 u4StreamType, __u32 u4StreamID);

/** @brief for parsing file information,and keeping some file information
*     @param hInst : AVDmx handle
*     @param pvdataSource : file source for android platform
*     @return MRESULT : the execution result status of the interface
*     @note file information contain ID3/video codec width Height/audio samplerate channel
*/
MRESULT     AVDmx_LoadDataSource(HAVINST hInst, void *pvdataSource);

/** @brief for parsing file information,and keeping some file information
*     @param hInst : AVDmx handle
*     @param szFileName : file name
*     @return MRESULT : the execution result status of the interface
*     @note file information contain ID3/video codec width Height/audio samplerate channel
*/
MRESULT     AVDmx_LoadFile(HAVINST hInst, char *szFileName);

/** @brief for set file name
*     @param hInst : AVDmx handle
*     @param szFileName : file name
*     @return void
*     @note not used
*/
void        AVDmx_SetFileName(HAVINST hInst, char *szFileName);

/** @brief for get video frame width
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video frame width
*     @note u1Idx 0 means first video stream,u1Idx 1 means second video stream
*/
__u32       AVDmx_GetFrameWidth(HAVINST hInst, __u8 u1Idx);

/** @brief for get video frame height
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video frame height
*     @note u1Idx 0 means first video stream,u1Idx 1 means second video stream
*/
__u32       AVDmx_GetFrameHeight(HAVINST hInst, __u8 u1Idx);

/** @brief for get video frame Rate
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video frame Rate
*     @note u1Idx 0 means first video stream,u1Idx 1 means second video stream
*/
__u32       AVDmx_GetFrameRate(HAVINST hInst, __u8 u1Idx);

/** @brief for get Video stream info
*     @param hInst : AVDmx handle
*     @param prVStm : video stream Info
*     @return MRESULT : the execution result status of the interface
*     @note prVStm contain all video stream info parsed by AVDmx_LoadDataSource or AVDmx_LoadFile
*/
MRESULT     AVDmx_GetVStmInfo(HAVINST hInst, AV_VSTM_T *prVStm);

/** @brief for get video timescale
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video timescale
*     @note not used
*/
__u32       AVDmx_GetVidTimescale(HAVINST hInst, __u8 u1Idx);

/** @brief for get video duration
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video duration in microsecond
*     @note u1Idx 0 means first video stream,u1Idx 1 means second video stream
*/
__u64       AVDmx_GetVidDuration(HAVINST hInst, __u8 u1Idx);

/** @brief for get video private data length
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return video private data length
*     @note not used
*/
__u32       AVDmx_GetExtraLen(HAVINST hInst, __u8 u1Idx);

/** @brief for get video private data
*     @param hInst : AVDmx handle
*     @param pvBuf : video private data buffer
*     @param u1Idx : the index of video stream,start from 0
*     @return the execution result status of the interface
*     @note not used
*/
MRESULT     AVDmx_GetExtraData(HAVINST hInst, void *pvBuf, __u8 u1Idx);

/** @brief for check h264 and h265 csd is start with 00 00 01
*     @param hInst : AVDmx handle
*     @return false if csd is start with 00 00 01
*     @note for h264 and h265 csd container have 2 format,one is start with 00 00 01 or 00 00 00 01,the other one is have container info
*/
bool        AVDmx_fgConvertCSD(HAVINST hInst);

/** @brief for get stream CSD
*     @param hInst : AVDmx handle
*     @param eStream : Stream type for Starting
*     @param u1Idx : the index of  stream,start from 0
*     @param pvBuf : CSD buf
*     @param pu4BufLen : CSD buf Len
*     @return the execution result status of the interface
*     @note get audio/video CSD
*/
MRESULT     AVDmx_GetDecInfo(HAVINST hInst,
                             STREAM_TYPE_T eStream,
                             __u8 u1Idx,
                             void *pvBuf,
                             __u32 *pu4BufLen);

/** @brief for get lyric
*     @param hInst : AVDmx handle
*     @param pvBuf : lyric buf
*     @param pu4BufLen : lyric buf Len
*     @return the execution result status of the interface
*/
MRESULT     AVDmx_GetLyricData(HAVINST hInst, void *pvBuf, __u32 *pu4Len);

/** @brief for check current video codec is it supported
*     @param hInst : AVDmx handle
*     @param prVStm : video stream info must be not NULL
*     @return true is support,false is not support
*/
bool        AVDmx_FgVidSupport(HAVINST hInst, AV_VSTM_T *prVStm);

/** @brief for get audio stream info
*     @param hInst : AVDmx handle
*     @param prAStm : audio stream Info
*     @return MRESULT : the execution result status of the interface
*     @note prVStm contain all audio stream info parsed by AVDmx_LoadDataSource or AVDmx_LoadFile
*/
MRESULT     AVDmx_GetAStmInfo(HAVINST hInst, AV_ASTM_T *prAStm);

/** @brief for get audio timescale
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of audio stream,start from 0
*     @return audio timescale
*     @note not used
*/
__u32       AVDmx_GetAudTimescale(HAVINST hInst, __u8 u1Idx);

/** @brief for get audio duration
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of video stream,start from 0
*     @return audio duration in microsecond
*     @note u1Idx 0 means first video stream,u1Idx 1 means second video stream
*/
__u64       AVDmx_GetAudDuration(HAVINST hInst, __u8 u1Idx);

/** @brief for get a position after a block of error data
*     @param hInst : AVDmx handle
*     @param u8Pts : the position after a block of error data use microsecond
*     @param u4PsrType : Stream type
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetTimeAfterErrData(HAVINST hInst, __u64 *u8Pts, __u32 u4PsrType);

/** @brief for get audio stream 0 audio channel count
*     @param hInst : AVDmx handle
*     @return audio channel count
*     @note deprecated
*/
__u16       AVDmx_GetChannels(HAVINST hInst);

/** @brief for get audio stream 0 audio samplerate
*     @param hInst : AVDmx handle
*     @return audio samplerate
*     @note deprecated
*/
__u32       AVDmx_GetSamplesPerSec(HAVINST hInst);

/** @brief for get audio stream 0 audio bits per sample
*     @param hInst : AVDmx handle
*     @return audio bits per sample
*     @note deprecated
*/
__u16       AVDmx_GetBitsPerSample(HAVINST hInst);

/** @brief for get audio stream CSD
*     @param hInst : AVDmx handle
*     @param u1Idx : the index of  stream,start from 0
*     @param ppvData : CSD buf
*     @return the execution result status of the interface
*     @note get audio CSD
*/
MRESULT     AVDmx_GetAudDecInfo(HAVINST hInst, __u8 u1Idx, void *ppvData);

/** @brief for check current video codec is it supported
*     @param hInst : AVDmx handle
*     @param prAStrm : audio stream info must be not NULL
*     @param pu4UnSptInfo : audio stream support info must be not NULL
*     @return true is support,false is not support
*     @note you can known which audio param is not support throw pu4UnSptInfo
*/
bool        AVDmx_FgAudSupport(HAVINST hInst, AV_ASTM_T *prAStrm, __u32 index, __u32 *pu4UnSptInfo);

/** @brief for get internal subtitle stream info
*     @param hInst : AVDmx handle
*     @param prSPStm : internal subtitle stream info
*     @return MRESULT : the execution result status of the interface
*     @note prSPStm contain all internal subtitle stream info parsed by AVDmx_LoadDataSource or AVDmx_LoadFile
*/
MRESULT     AVDmx_GetSPStmInfo(HAVINST hInst, AV_SPSTM_T *prSPStm);

/** @brief for get audio ID3 info
*     @param hInst : AVDmx handle
*     @param pAudioId3Info : ID3 info
*     @return MRESULT : the execution result status of the interface
*     @note title/year/album/author/genre/duration must called after AVDmx_LoadDataSource or AVDmx_LoadFile
*/
MRESULT     AVDmx_GetAudioRawID3Info(HAVINST hInst, AUDIO_ID3INFO_T *pAudioId3Info);

/** @brief for get file info by file name
*     @param pszFileName : file name
*     @param pvFileInfo : file info
*     @return true for success
*     @note get file info include ID3 duration,audio samplerate channel,video width height
*/
bool        AVDmx_GetFileInfo(char *pszFileName, FILE_INFO_T *pvFileInfo);

/** @brief for get file info by datasource
*     @param pvdataSource : data source
*     @param pvFileInfo : file info
*     @param pszFileName : file name not used
*     @return true for success
*     @note get file info include ID3 duration,audio samplerate channel,video width height
*/
bool        AVDmx_GetFileInfoBySrc(void *pvdataSource, void *pFileInfo, char *pszFileName);

/** @brief for get file info by avdmx handle
*     @param hInst : AVDmx handle
*     @param pvFileInfo : file info
*     @return MRESULT : the execution result status of the interface
*     @note get file info include ID3 duration,audio samplerate channel,video width height,must called after AVDmx_LoadDataSource or AVDmx_LoadFile
*/
MRESULT     AVDmx_GetFileInfoBySrcEx(HAVINST hInst, void *pvFileInfo);

/** @brief for get album cover data length
*     @param pszFileName : file name
*     @param pu4PicLen : album cover data length
*     @param pptrFileParserAddr : fileparser object
*     @param fgIOError : true for read file error
*     @return true for success
*     @note pptrFileParserAddr will use in AVDmx_GetID3APicInfoData
*/
//bool        AVDmx_GetID3APicInfoLen(char *pszFileName, __u32 *pu4PicLen, uintptr_t *pptrFileParserAddr,
                                    //bool *fgIOError);

/** @brief for get  album cover data
*     @param pptrFileParserAddr : fileparser object
*     @param pAPicBuf :  album cover data malloced by usr
*     @param u4APicBufLen :  album cover data length
*     @return true for success
*/
bool        AVDmx_GetID3APicInfoData(uintptr_t ptrFileParserAddr, __u8 *pAPicBuf, __u32 u4APicBufLen);

/** @brief for get  album cover data by avdmx handle
*     @param hInst : AVDmx handle
*     @param pcinfo :  album cover data
*     @param pu4infosize :  album cover data length
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetFileApicInfo(HAVINST hInst, void **pcinfo, __u32 *pu4infosize);

/** @brief for creating buf for ID3
*     @param hInst : AVDmx handle
*     @param pFileInfo :  file info
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetFileInfoMem(HAVINST hInst, FILE_INFO_T *pFileInfo);

/** @brief for releasing ID3 buf which create by AVDmx_GetFileInfoMem
*     @param pFileInfo :  file info
*     @return void
*/
void        AVDmx_ReleaseFileInfoMem(void *pvFileInfo);

/** @brief for getting ID3 Title info
*     @param hInst : AVDmx handle
*     @param pcinfo :  Title info
*     @param u4infosize :  Title length
*     @return MRESULT : the execution result status of the interface
*     @note must called after AVDmx_GetFileInfoMem
*/
MRESULT     AVDmx_GetFileTitllnfo(HAVINST hInst, char *pcinfo, __u32 u4infosize);

/** @brief for getting ID3 year info
*     @param hInst : AVDmx handle
*     @param pcinfo :  year info
*     @param u4infosize :  year length
*     @return MRESULT : the execution result status of the interface
*     @note must called after AVDmx_GetFileInfoMem
*/
MRESULT     AVDmx_GetFileYearInfo(HAVINST hInst, char *pcinfo, __u32 u4infosize);

/** @brief for getting ID3 Album info
*     @param hInst : AVDmx handle
*     @param pcinfo :  Album info
*     @param u4infosize :  Album length
*     @return MRESULT : the execution result status of the interface
*     @note must called after AVDmx_GetFileInfoMem
*/
MRESULT     AVDmx_GetFileAlbumInfo(HAVINST hInst, char *pcinfo, __u32 u4infosize);

/** @brief for getting ID3 Author info
*     @param hInst : AVDmx handle
*     @param pcinfo :  Author info
*     @param u4infosize :  Author length
*     @return MRESULT : the execution result status of the interface
*     @note must called after AVDmx_GetFileInfoMem
*/
MRESULT     AVDmx_GetFileAuthorInfo(HAVINST hInst, char *pcinfo, __u32 u4infosize);

/** @brief for getting ID3 Genre info
*     @param hInst : AVDmx handle
*     @param pcinfo :  Genre info
*     @param u4infosize :  Genre length
*     @return MRESULT : the execution result status of the interface
*     @note must called after AVDmx_GetFileInfoMem
*/
MRESULT     AVDmx_GetFileGenreInfo(HAVINST hInst, char *pcinfo, __u32 u4infosize);

/** @brief for getting video stream count
*     @param hInst : AVDmx handle
*     @param pu1Count :  video stream count
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetVStmCnt(HAVINST hInst, __u8 *pu1Count);

/** @brief for getting audio stream count
*     @param hInst : AVDmx handle
*     @param pu1Count :  audio stream count
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetAStmCnt(HAVINST hInst, __u8 *pu1Count);

/** @brief for getting audio stream language
*     @param hInst : AVDmx handle
*     @param u1Index :  audio stream index
*     @param pu4LangID :  audio stream language
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetAStmLang(HAVINST hInst, __u8 u1Index, __u32 *pu4LangID);

/** @brief for getting current audio audio stream index
*     @param hInst : AVDmx handle
*     @param pu1Current :  audio stream index
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetCurrentAudio(HAVINST hInst, __u8 *pu1Current);

/** @brief for termaniting avdmx all function
*     @param hInst : AVDmx handle
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_SetBack(HAVINST hInst);

/** @brief for getting file control info
*     @param hInst : AVDmx handle
*     @param prAVFileCtl : file control info
*     @return MRESULT : the execution result status of the interface
*     @note file control info current support seek FF and RW
*/
MRESULT     AVDmx_GetFileCtlInfo(HAVINST hInst, AV_FILECTL_T *prAVFileCtl);

/** @brief for getting file media info
*     @param hInst : AVDmx handle
*     @param prMediaInfo : file media info
*     @return MRESULT : the execution result status of the interface
*     @note file media info include video audio and subtitle
*/
MRESULT     AVDmx_GetMediaInfo(HAVINST hInst, MSDKCORE_MEDIA_INFO_T *prMediaInfo);

/** @brief for getting file Capabilities
*     @param hInst : AVDmx handle
*     @param pu4Capability :file Capabilities
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetCapabilities(HAVINST hInst, __u32 *pu4Capability);
MRESULT     AVDmx_SupportFFWithoutIdx(HAVINST hInst, bool *pfgSupportFFWithoutIdx);
MRESULT     AVDmx_CanSeekFromHere(HAVINST hInst, __s64 i8SeekingPoint, bool *pfgCanSeekFromHere);

/** @brief for getting key frame position
*     @param hInst : AVDmx handle
*     @param pu8SeekingPoint :position of key frame
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_GetSeekingPoint(HAVINST hInst, __u64 *pu8SeekingPoint);

/** @brief for setting code page
*     @param hInst : AVDmx handle
*     @param eCodePage : code page
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_SetCodePage(HAVINST hInst, E_DMX_CODEPAGE_SET eCodePage);
MRESULT     AVDmx_GetLoadingStatus(HAVINST hInst, __s32 *pi4LoadingStatus, __s64 *pi8RemainTime);
void        AVDmx_Dump(HAVINST hInst, int fd, char args[][DEBUG_INFO_MAXLEN], __u32 u4len);
void        AVDmx_DumpEx(int fd, char args[][DEBUG_INFO_MAXLEN], __u32 u4len);

/** @brief for setting local play and network play
*     @param hInst : AVDmx handle
*     @param u4flags : flag for local play and network play
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_SetFlags(HAVINST hInst, __u32 u4flags);
void        AVDmx_DumpStatus(HAVINST hInst);

/** @brief for getting codec name in string
*     @param ecodec : codec id
*     @param str : sting
*     @param len : string len
*     @return void
*/
void        AVDmx_GetCodecStr(AVCODECID_T ecodec, char *str, __u32 len);

/** @brief for getting a au
*     @param hInst : AVDmx handle
*     @param prSeekParam : seek param
*     @param prSendBufInfo: buf info
*     @return MRESULT : the execution result status of the interface
*/
MRESULT     AVDmx_ReadOutputBuffer(HAVINST hInst, SEEK_INFO *prSeekParam, MEDIA_DATA_INFO *prSendBufInfo);

/** @brief for checking current video codec
*     @param hInst : AVDmx handle
*     @return true for soft codec
*/
bool        AVDmx_FgVideoIsSoftDec(HAVINST hInst);

/** @brief for checking current audio wma codec
*     @param hInst : AVDmx handle
*     @return true for soft codec
*/
bool        AVDmx_FgWmaIsSoftDec(HAVINST hInst);

/** @brief for getting file type
*     @param hInst : AVDmx handle
*     @return true AV_FILE_TYPE_T : file type
*/
AV_FILE_TYPE_T AVDmx_GetFileType(HAVINST hInst);

#ifdef __cplusplus
}
#endif

#endif //_AVDMX_H_


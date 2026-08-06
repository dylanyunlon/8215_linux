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

#ifndef MM_ERRCODE_H
#define MM_ERRCODE_H

#include "linux/types.h"

/*************************************************************************
 *
 *                    The size of error_code is 32 bits
 * Error code structure :
 *
 * | 31(1 bit) |30-24(7 bits) |23-16(8 bits) | 15-0(16 bits) |
 * | Indicator |   Reserved   |  Module ID   |      Code     |
 *
 * Indicator  : 1 - ERROR_CODE; 0 - SUCESS_CODE(RET_MSDKC_OK:0x00000000)
 * Reserved   : Reserved bits
 * Module ID  : module ID, defined below
 * Code       : the module's status code

 *************************************************************************/
/* Composer for Module specific error code */

typedef __u32 MRESULT;

#define MM_IS_STATE_ERROR(errcode)  ((__u8)0x00 == ((__u8)(errcode >> (MRESULT)24) & (MRESULT)0xFF))

#define MAKE_ERR_CODE(mod, code)					\
	(MRESULT)(										\
	(MRESULT)0x80000000 |						\
	(((MRESULT)(mod) & (MRESULT)0x000000FF) << (MRESULT)16) |		\
	((MRESULT)(code) & (MRESULT)0x0000FFFF)			\
	)

#define MAKE_STATE_CODE(mod, code)					\
	(MRESULT)(									\
	(MRESULT)0x00000000 |						\
	(((MRESULT)(mod) & (MRESULT)0x000000FF) << (MRESULT)16) |		\
	((MRESULT)(code) & (MRESULT)0x0000FFFF)			\
	)

////////////////---MODULE_ID----//////////////////////////

// MSDKCORE :  The value range is 0x80010000 ----0x8001FFFFF
#define MOD_ERRCODE_COMMON              0x00000001

// Preparser : The value range is 0x80020000 ----0x8002FFFFF
#define MOD_ERRCODE_PREPARSER           0x00000002

// Demuxer :   The value range is 0x80030000 ----0x8003FFFFF
#define MOD_ERRCODE_DMX                 0x00000003

// RLE :       The value range is 0x80040000 ----0x8004FFFFF
#define MOD_ERRCODE_RLE                 0x00000004

// VDEC :      The value range is 0x80050000 ----0x8005FFFFF
#define MOD_ERRCODE_VDEC                0x00000005

// ADEC :      The value range is 0x80060000 ----0x8006FFFFF
#define MOD_ERRCODE_ADEC                0x00000006

// CC :        The value range is 0x80070000 ----0x8007FFFFF
#define MOD_ERRCODE_CC                  0x00000007

// AVSWI :        The value range is 0x80080000 ----0x8008FFFFF
#define MOD_ERRCODE_AVSWI               0x00000008



/*************************************************************************
*
*                  The specific value of msdkcore error code
*
*          The start value of error_code is 0x80010000(RET_MSDKC_FAIL)
*
*************************************************************************/

// Success, or OK
#define RET_MSDKC_OK                                        (MRESULT)0x00000000

// Fail, or error, but don't know the real reason
#define RET_MSDKC_FAIL                                      MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000000)

// Out of memory, or alloc/realloc fail
#define RET_MSDKC_OUTOFMEMORY                               MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000001)

// Invalid Parameter in function
#define RET_MSDKC_INVALID_PARAMETER                         MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000002)

// file operation fail, such as OpenFile, CreateFile, ReadFile and so on
#define RET_MSDKC_FILE_OPERATION_FAIL                       MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000003)

// Invalid File Data, such as invalid file header
#define RET_MSDKC_INVALID_FILE_DATA                         MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000004)

// The media type of this file is not recognized
#define RET_MSDKC_FILE_TYPE_NOT_SUPPORT                     MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000005)

// Frame-rate not support
#define RET_MSDKC_FRAME_RATE_NOT_SUPPORT                    MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000006)

// Bit-rate not support
#define RET_MSDKC_BITRATE_NOT_SUPPORT                       MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000007)

// Sample-rate not support
#define RET_MSDKC_SAMPLERATE_NOT_SUPPORT                    MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000008)

// Resolution not support
#define RET_MSDKC_RESOLUTION_NOT_SUPPORT                    MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000009)

// Cannot play back the audio: the audio codec is not supported.
#define RET_MSDKC_AUDIO_CODEC_NOT_SUPPORT                   MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000A)

// Cannot play back the video: the video codec is not supported.
#define RET_MSDKC_VIDEO_CODEC_NOT_SUPPORT                   MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000B)

// can't play back the file: the profile or level is not supported.
#define RET_MSDKC_PROFILE_LEVEL_NOT_SUPPORT                 MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000C)

// DRM Not support
#define RET_MSDKC_DRM_NOT_SUPPORT                           MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000D)

// The infomation is not exist when get audio/video/sp infomation
#define RET_MSDKC_NODATA                                    MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000E)

// When call DMX_IO_CTRL encouter some unexpected error
#define RET_MSDKC_DRM_NOT_AUTHORIZED                        MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x0000000F)

// the DRM is not registered
#define RET_MSDKC_DRM_NOT_REGISTERED                        MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000010)

// the DRM file is expired
#define RET_MSDKC_DRM_RENTAL_EXPIRED                        MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000011)

// when call drm api encounter general error
#define RET_MSDKC_DRM_GENERAL_ERROR                         MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000012)

#define RET_MSDKC_DRM_NEVER_REGISTERED                      MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000013)

#define RET_MSDKC_AUDIO_CHANGE_NOT_SUPPORT                  MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0x00000014)



/*************************************************************************
*
*                  The specific value of msdkcore state code
*
*          The start value of state_code is 0x00010000(RET_MSDKC_STATE_TERMINATE_LOAD)
*
*************************************************************************/

#define RET_MSDKC_STATE_TERMINATE_LOAD                      MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 0x00000000)

#define RET_MSDKC_STATE_NO_DRM_HEADER                       MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 0x00000001)

#define RET_MSDKC_STATE_CURRENT_AUD_PLAYING                 MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 0x00000002)

#define RET_MSDKC_STATE_NO_ID3_IMAGE                        MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 0x00000003)

#endif /* MM_ERRCODE_H */



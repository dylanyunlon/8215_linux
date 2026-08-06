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
#ifndef _DRV_VDEC_H_
#define _DRV_VDEC_H_

#include "x_vid_dec.h"
#include "ioctl_vdec.h"

#define FW_WRITE_QUANTIZATION_MATRIX
#define VDSCL_SIZE_LIMIT_SUPPORT
#define VDEC_DDR3_SUPPORT 0

/******************************************************************************
* MPV definition
******************************************************************************/

// VLD ID
#define VLD0   0
#define VLD1   1

// Maximum Elementary Stream number
#define MPV_MAX_ES              2

typedef struct _VDSCL_INFO_T
{
    __u16  u2DesiredWidth;
    __u16  u2DesiredHeight;
    __u16  u2HandledWidth;
    __u16  u2HandledHeight;
    __u16  u2OriginalWidth;
    __u16  u2OriginalHeight;
    __u16  u2OffsetX;
    __u16  u2OffsetY;
    __u16  u2ClipWidth;
    __u16  u2ClipHeight;
    __u16  u2SampleWidth;
    __u16  u2SampleHeight;
    __u32  u4VDSCLFlag;
#ifdef ENUM_SRC_ASPECT_RATIO
    SOURCE_ASPECT_RATIO_T eAspRatio;
#else
    __u16  u2AspectRatio;
#endif
} VDSCL_INFO_T;

/**
 * @par Enumeration
 *   REGISTER_GROUP_T
 * @par Description
 *   This is the item used for register group
 */
typedef enum _REGISTER_GROUP_T
{
    VDEC_VLD,           ///< VDEC_VLD
    VDEC_MISC,          ///< VDEC_MISC
    VDEC_VLD_TOP,       ///< VDEC_VLD_TOP
    VDEC_MC,            ///< VDEC_MC
    VDEC_AVC_VLD,       ///< VDEC_AVC_VLD
    VDEC_AVC_MV,        ///< VDEC_AVC_MV
    VDEC_HEVC_VLD,      ///< VDEC_HEVC_VLD
    VDEC_HEVC_MV,       ///< VDEC_HEVC_MV
    VDEC_PP,            ///< VDEC_PP
    VDEC_DV,            ///< VDEC_DV
    VDEC_VP8_VLD,       ///< VDEC_VP8_VLD
    VDEC_VP6_VLD,       ///< VDEC_VP6_VLD
    VDEC_VP6_VLD2, ///<VDEC_VP6_VLD2
    VDEC_VP6_VLD2_SHIFT, ///<VDEC_VP6_VLD2_SHIFT
    VDEC_VP6_DCAC,      ///<VDEC_VP6_DCAC
    VDEC_VP8_MV,        ///<VDEC_VP8_MV
    VDEC_VP8_VLD2,      ///<VDEC_VP8_VLD2
    VDEC_IMG_RESZ,      ///<VDEC_IMG_RESZ
    VDEC_HEVC_MISC,     ///<VDEC_HEVC_MISC
    VDEC_CRC,           ///< VDEC_CRC
    VDEC_RM_VLD,        /// < VEC_RM_VLD
    VDEC_RM_PP,        /// < VEC_RM_PP
    VDEC_VP6_PP,        /// < VEC_VP6_PP
    VDEC_VP8_PP,        /// < VEC_VP8_PP
    VDEC_WMV_MV,        /// < VDEC_WMV_MV
    VCODEC_MAX          ///< VCODEC_MAX
} REGISTER_GROUP_T;

typedef struct _VDEC_HANDLE_T_
{
    __u32 u4StartMask;       //For judging the memory is trampled or not
    int     fd;            ///< fd_vdec
    VAL_MEMORY_T  rHandleMem;   ///< rHandleMem
    __u8     *mmap[VCODEC_MAX];   ///< mmap[VCODEC_MAX]
    __u8     *pReservedVirMem;
    __u32 u4EndMask;        //For judging the memory is trampled or not
} VDEC_HANDLE_T;

#endif /* _DRV_VDEC_H_ */

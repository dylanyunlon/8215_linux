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
#ifndef _VDEC_INIT_H_
#define _VDEC_INIT_H_

#define VDEC_PM_SUPPORT 0

typedef enum
{
    VDEC_UNKNOWN     = 0xFF,
    VDEC_MPEG        = 0x0,                 ///< MPEG Deocde Request
    VDEC_MPEG1       = 0x1,               ///< MPEG1 Deocde Request
    VDEC_MPEG2       = 0x2,               ///< MPEG2 Deocde Request
    VDEC_DIVX3       = 0x3,              ///< MPEG3 Deocde Request
    VDEC_MPEG4       = 0x4,              ///< MPEG4 Deocde Request
    VDEC_WMV         = 0x10,            /// < WMV Decode Request
    VDEC_WMV1        = 0x11,             ///< WMV7 Deocde Request
    VDEC_WMV2        = 0x12,            ///< WMV8 Deocde Request
    VDEC_WMV3        = 0x13,            ///< WMV9 Deocde Request
    VDEC_VC1         = 0x111,          ///< VC1 Deocde Request
    VDEC_H264        = 0x264,           ///< H264 Deocde Request
    VDEC_H265        = 0x265,          ///< H265 Deocde Request
    VDEC_H263        = 0x263,           ///< H263 Deocde Request
    VDEC_RM          = 0x300,             ///< RM Decode Request
    VDEC_VP6         = 0x600,       ///< VP6 Decode Request
    VDEC_VP8         = 0x700,        ///<VP8 Decode Request
    VDEC_MJPG        = 0x800,        ///< MJPG  Decode Request
    VDEC_WEBP        = 0x900,        ///< WEBP  Decode Request
} VDEC_CODEC_T;

typedef struct _VDEC_CODEC_INFO_T
{
    __u32 u4VDecID;
    __u32 u4ChipFeature;
    __u32 u4CodeType;
    __u32 u4Reserved;
} VDEC_CODEC_INFO_T;

#endif

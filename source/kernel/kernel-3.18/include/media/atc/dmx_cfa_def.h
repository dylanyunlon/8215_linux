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



#ifndef DMX_CFA_DEFINE_H
#define DMX_CFA_DEFINE_H

#include "x_typedef.h"
#include "chip_ver.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {

#endif	/* 
 */

/*! @name CFA Interface Constants, enumerations and macros (6.1) */
/*! @{ */
typedef enum CfaType {
	/*!< For Initial of the variable, CFA is not available if the type is still set with this value */
	CFA_TYPE_UNINITIAL = 0,
	/*!< This is a easy container format analyzer, */
	/*one may treat it as an SPT_DATA_BUF type CFA. */
	CFA_TYPE_TRIVIAL = 1,
	/*!< This is an MPEG container format analyzer, */
	/*use to split the MPEG 1,2 and DVD-VIDEO related system format. */
	CFA_TYPE_MPG = 2,
	/*!< This is an AVI container format analyzer, use to split the AVI movi part data.*/
	/*(In general, it will take care of DivX or Xvid file format). */
	CFA_TYPE_AVI = 3,
	/* Please add any new format here ... */
	/*!< This is for CD type CFA */
	CFA_TYPE_CD = 4,
	/*!< Now this is for mp3 type file CFA */
	CFA_TYPE_AUDIO = 5,
	CFA_TYPE_MP4 = 6,
	/*< This is an asf container format analyzer, use to split ASF, WMV, WMA format. */
	CFA_TYPE_ASF = 7,
	CFA_TYPE_SACD = 8,
	/*< Just for RLE external subtitle parsing sub-picture pack */
	CFA_TYPE_SUB = 9,
	CFA_TYPE_OGM = 10,
	CFA_TYPE_MKV = 11,
	/* This is an AUDIO-IN container format analyzer */
	CFA_TYPE_AUDIN = 12,
	/*This is a FLV(Flash Video) container format analyzer */
	CFA_TYPE_FLV = 13,
	/*This is a SWF(Flash Video) container format analyzer */
	CFA_TYPE_SWF = 14,
	/*This is a RM(RealMedia) container format analyzer */
	CFA_TYPE_RM = 15,
	/*This is a ape container format analyzer */
	CFA_TYPE_APE = 16,
	/*this is a transfor stream analyzer */
	CFA_TYPE_TS = 17,
	/*!< This is an example container format analyzer, use to split the raw data. */
	CFA_TYPE_EXP = 77,
	/*!< For Error of the input setting, */
	/*CFA type is error if the format setting is not found in current support format. */
	CFA_TYPE_ERROR = 99
} CfaType;

typedef enum CfaPrsBitStrmType {
	CFA_PRS_BIT_STRM_TYPE_NONE = ((__u32) 0), /*< none */
	CFA_PRS_BIT_STRM_TYPE_V = ((__u32) (1 << 0)), /*< video */
	CFA_PRS_BIT_STRM_TYPE_A = ((__u32) (1 << 1)), /*< audio */
	CFA_PRS_BIT_STRM_TYPE_SP = ((__u32) (1 << 2)), /*< sp */
	CFA_PRS_BIT_STRM_TYPE_SEC = ((__u32) (1 << 3)), /*< section  */
	CFA_PRS_BIT_STRM_TYPE_DRM = ((__u32) (1 << 4)), /*< DRM data */
	CFA_PRS_BIT_STRM_TYPE_PACKET_HDR = ((__u32) (1 << 5)), /*< Packet Header */
	CFA_PRS_BIT_STRM_TYPE_PAYLOAD_HDR = ((__u32) (1 << 6)) /*< Payload Header */
} CfaPrsBitStrmType;

#define fgIsStmToPlay(u4PrsFlag, BitStmFlag) (0 != ((u4PrsFlag) & (BitStmFlag)))
#define fgIsVidStmToPlay(u4PrsFlag)				 fgIsStmToPlay(u4PrsFlag, CFA_PRS_BIT_STRM_TYPE_V)
#define fgIsAudStmToPlay(u4PrsFlag)				 fgIsStmToPlay(u4PrsFlag, CFA_PRS_BIT_STRM_TYPE_A)
#define fgIsSPStmToPlay(u4PrsFlag)				 fgIsStmToPlay(u4PrsFlag, CFA_PRS_BIT_STRM_TYPE_SP)
#define fgIsSecStmToPlay(u4PrsFlag)				 fgIsStmToPlay(u4PrsFlag, CFA_PRS_BIT_STRM_TYPE_SEC)

/* Old C header file */
#ifdef __cplusplus
}
#endif	/* 
 */

#endif	/* DMX_CFA_DEFINE_H */

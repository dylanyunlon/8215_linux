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



#ifndef DMX_CFA_EXSUB_H
#define DMX_CFA_EXSUB_H

#include "x_typedef.h"
#include "dmx_define.h"
#include "mm_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PB_AVI_SUPPORT_EXTERNAL_RLE				1
#define PB_MP4_SUPPORT_EXTERNAL_SUB				1
#define PB_MPG_SUPPORT_EXTERNAL_SUB				1
#define PB_EXT_SUB_GET_DATA_BY_DEMUXER			0

#define EXT_SUBTITLE_SUPPORT_REVERSE				1

	typedef struct CfaSubReverseRange {
		__u32 u4SlotSa;
		__u32 u4SlotEa;
		__u64 u8CurDisplayPts;
		__u32 *puIdxTable;
		__u32 u4CurSubAtIdxTabID;
		__u32 u4TableNum;
		__u32 u4TableElementNum;
	} CfaSubReverseRange_T;

	typedef struct CfaSubConfig {
		__u32 *puIdxTable;	/* LPE gets idx table created by idx parser. */
		__u32 u4TableNum;
		__u32 u4TableElementNum;
		bool fgTimePlay;	/* for time search */
		__u64 u8DisplayPts;
	} CfaSubConfig_T;

	typedef struct CfaSubRange {
		__u64 u8Sa;
		__u64 u8Ea;
		CfaSubConfig_T rConfig;
		bool fgIsReverse;
		CfaSubReverseRange_T rReverseRange;
	} CfaSubRange_T;

#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_EXSUB_H */

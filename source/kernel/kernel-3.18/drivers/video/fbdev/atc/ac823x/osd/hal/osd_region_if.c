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
/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#ifndef __ARM2__
#include <linux/module.h>
#include <linux/kernel.h>
#include <media/atc/drv_osd_if.h>
#include "x_debug.h"
#else
#include "assert.h"
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"
/*#include "osd_if.h"*/

#include "chip_ver.h"

#define DEFINE_IS_LOG   OSD_IsLog

#include "log.h"


/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/
#define OSD_INVALID_REGION_HANDLE -1
#define OSD_RGN_NODE_ALLOCATED 1
#define OSD_RGN_LIST_ALLOCATED 1
#define OSD_ARB_RGN_MAX        8

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

typedef struct _OSD_REGION_NODE_T {
	__u32 u4NodeStatus;
	__s32 i4Prev, i4Next;
} OSD_REGION_NODE_T;

typedef struct _OSD_REGION_LIST_T {
	__u32 u4ListStatus;
	__s32 i4Head, i4Tail;
	__s32 i4Count;
	bool  b_compressed;
} OSD_REGION_LIST_T;


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

static OSD_REGION_NODE_T _rAllRgnNode[OSD_MAX_NUM_RGN];
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
static OSD_REGION_LIST_T _rAllRgnList[OSD_MAX_NUM_RGN_LIST];
#endif

#ifdef SUPPORT_RGB_YUV_FULL_RANGE_CONVER
bool _fgXvYccEn = FALSE;
#endif


/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
static __s32 _OsdRegionCompare(__u32 u4Rgn1, __u32 u4Rgn2)
{
	__u32 u4DispY1, u4DispH1, u4DispB1;
	__u32 u4DispY2, u4DispH2, u4DispB2;
	__u32 u4DispX1, u4DispW1, u4DispR1;
	__u32 u4DispX2, u4DispW2, u4DispR2;

	__s32 i4Ignore;

	i4Ignore = _OSD_RGN_GetOutputPosY(u4Rgn1, &u4DispY1);
	i4Ignore = _OSD_RGN_GetOutputHeight(u4Rgn1, &u4DispH1);
	u4DispB1 = u4DispY1 + u4DispH1;
	i4Ignore = _OSD_RGN_GetOutputPosY(u4Rgn2, &u4DispY2);
	i4Ignore = _OSD_RGN_GetOutputHeight(u4Rgn2, &u4DispH2);
	u4DispB2 = u4DispY2 + u4DispH2;
	i4Ignore = _OSD_RGN_GetOutputPosX(u4Rgn1, &u4DispX1);
	i4Ignore = _OSD_RGN_GetOutputWidth(u4Rgn1, &u4DispW1);
	u4DispR1 = u4DispX1 + u4DispW1;
	i4Ignore = _OSD_RGN_GetOutputPosX(u4Rgn2, &u4DispX2);
	i4Ignore = _OSD_RGN_GetOutputWidth(u4Rgn2, &u4DispW2);
	u4DispR2 = u4DispX2 + u4DispW2;
	UNUSED(i4Ignore);

	if ((u4DispB1 >= u4DispY2) && (u4DispY1 >= u4DispB2)) {
		return 1; /* region 1 > region 2*/
	} else if ((u4DispB2 >= u4DispY1) && (u4DispY2 >= u4DispB1)) {
		return -1; /* region 2 > region 1*/
	}

	if ((u4DispR1 >= u4DispX2) && (u4DispX1 >= u4DispR2)) {
		return 1; /* region 1 > region 2*/
	} else if ((u4DispR2 >= u4DispX1) && (u4DispX2 >= u4DispR1)) {
		return -1; /* region 2 > region 1*/
	}

	FB_PRINT(FB_LOG_LVL_INFO, "[OSD driver] multi region collision, insert region failed\n");

	return 0; /*COLLISION*/
}
#endif

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
static __s32 _OsdArbRegionCompare(__u32 u4Rgn1, __u32 u4Rgn2)
{
	__u32 u4DispX1, u4DispY1, u4DispW1, u4DispH1, u4DispB1, u4DispC1;
	__u32 u4DispX2, u4DispY2, u4DispW2, u4DispH2, u4DispB2, u4DispC2;
	__s32 i4Ignore;

	i4Ignore = _OSD_RGN_GetOutputPosX(u4Rgn1, &u4DispX1);
	i4Ignore = _OSD_RGN_GetOutputWidth(u4Rgn1, &u4DispW1);
	u4DispB1 = u4DispX1 + u4DispW1;
	i4Ignore = _OSD_RGN_GetOutputPosY(u4Rgn1, &u4DispY1);
	i4Ignore = _OSD_RGN_GetOutputHeight(u4Rgn1, &u4DispH1);
	u4DispC1 = u4DispY1 + u4DispH1;

	i4Ignore = _OSD_RGN_GetOutputPosX(u4Rgn2, &u4DispX2);
	i4Ignore = _OSD_RGN_GetOutputWidth(u4Rgn2, &u4DispW2);
	u4DispB2 = u4DispX2 + u4DispW2;
	i4Ignore = _OSD_RGN_GetOutputPosY(u4Rgn2, &u4DispY2);
	i4Ignore = _OSD_RGN_GetOutputHeight(u4Rgn2, &u4DispH2);
	u4DispC2 = u4DispY2 + u4DispH2;

	UNUSED(i4Ignore);

	if ((((u4DispX2 >= u4DispX1) && (u4DispX2 <  u4DispB1)) ||
	     ((u4DispB2 >  u4DispX1) && (u4DispB2 <= u4DispB1)))  &&
	    (((u4DispY2 >= u4DispY1) && (u4DispY2 <  u4DispC1)) ||
	     ((u4DispC2 >  u4DispY1) && (u4DispC2 <= u4DispC1)))) {
		return 0; /*COLLISION*/
	}

	if ((u4DispC1 < u4DispC2) ||
	    ((u4DispC1 == u4DispC2) && (u4DispX1 <= u4DispX2))) {
		return -1; /* region 1 < region 2*/
	} else {
		return	1; /* region 1 > region 2*/
	}
}
#endif

/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_Init(void)
{
	__s32 i4Index;

	for (i4Index = 0; i4Index < OSD_MAX_NUM_RGN_LIST; i4Index++) {
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
		_rAllRgnList[i4Index].u4ListStatus = 0;
		_rAllRgnList[i4Index].i4Head = OSD_INVALID_REGION_HANDLE;
		_rAllRgnList[i4Index].i4Tail = OSD_INVALID_REGION_HANDLE;
		_rAllRgnList[i4Index].i4Count = 0;
#endif
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Init(void)
{
	__s32 i4Index;

	for (i4Index = 0; i4Index < OSD_MAX_NUM_RGN; i4Index++) {
		_rAllRgnNode[i4Index].u4NodeStatus = 0;
		_rAllRgnNode[i4Index].i4Prev = OSD_INVALID_REGION_HANDLE;
		_rAllRgnNode[i4Index].i4Next = OSD_INVALID_REGION_HANDLE;
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_Get(__u32 u4RgnList, __s32 i4Cmd, __u32 *pu4Value)
{
	OSD_VERIFY_RGNLIST(u4RgnList);

	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/

	switch (i4Cmd) {
	case OSD_RGN_LIST_HEAD:

		*pu4Value = (__u32)_rAllRgnList[u4RgnList].i4Head;
		break;

	case OSD_RGN_LIST_TAIL:
		*pu4Value = (__u32)_rAllRgnList[u4RgnList].i4Tail;
		break;

	case OSD_RGN_LIST_COUNT:
		*pu4Value = (__u32)_rAllRgnList[u4RgnList].i4Count;
		break;

	case OSD_RGN_LIST_COMPRESSED_FLAG:
		*pu4Value = (__u32)_rAllRgnList[u4RgnList].b_compressed;
		break;

	case OSD_LIST_FLAGS:
		*pu4Value = (__u32)_rAllRgnList[u4RgnList].u4ListStatus;
		break;

	default:
		return -(__s32)OSD_RET_INV_ARG;
	}

	return (__s32)OSD_RET_OK;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_Set(__u32 u4RgnList, __s32 i4Cmd, __u32 u4Value)
{
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/

	switch (i4Cmd) {
	case OSD_RGN_LIST_HEAD:
		if (u4Value >= OSD_MAX_NUM_RGN) {
			return -(__s32)OSD_RET_INV_REGION;
		}

		_rAllRgnList[u4RgnList].i4Head = (__s32)u4Value;
		break;

	case OSD_RGN_LIST_TAIL:
		if (u4Value >= OSD_MAX_NUM_RGN) {
			return -(__s32)OSD_RET_INV_REGION;
		}

		_rAllRgnList[u4RgnList].i4Tail = (__s32)u4Value;
		break;

	case OSD_RGN_LIST_COUNT:
		_rAllRgnList[u4RgnList].i4Count = (__s32)u4Value;
		break;

	case OSD_LIST_FLAGS:
		_rAllRgnList[u4RgnList].u4ListStatus = (__u32)u4Value;
		break;

	default:
		return -(__s32)OSD_RET_INV_ARG;
	}

	return (__s32)OSD_RET_OK;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*	fn __s32 OSD_RGN_LIST_Create(__u32 * pu4RgnList)
	brief this function will create a region list handle, just select unused region
	list from the region list allocated before
	param pu4RgnList  the pointer to store the allocated region list
	retval __s32  if 0, the routine successful;else, routine failed.
*/
__s32 OSD_RGN_LIST_Create(__u32 *pu4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4Index;
#endif
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_LIST_Create begin\n");
#endif

	if (pu4RgnList == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	/*[Xiaoxu Du]
	Because OSD_MAX_NUM_RGN_LIST is only 20 or 30,
	a linear search might be acceptable here.
	Creation of a region wont happen so often,
	hence even we have hundreds of region list,
	it should be ok here. */
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "_rAllRgnList[0].u4ListStatus=%d\n", _rAllRgnList[0].u4ListStatus);
	FB_PRINT(FB_LOG_LVL_DBG, "_rAllRgnList[1].u4ListStatus=%d\n", _rAllRgnList[1].u4ListStatus);
#endif

	for (i4Index = 0; i4Index < OSD_MAX_NUM_RGN_LIST; i4Index++) {
		if ((_rAllRgnList[i4Index].u4ListStatus & OSD_RGN_LIST_ALLOCATED) == 0) {
			/* free cell found, allocate this list, init and return it*/
			_rAllRgnList[i4Index].u4ListStatus |= OSD_RGN_LIST_ALLOCATED;
			_rAllRgnList[i4Index].i4Head     = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[i4Index].i4Tail     = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[i4Index].i4Count   = 0;
			_rAllRgnList[i4Index].b_compressed = FALSE;
			*pu4RgnList = (__u32)i4Index;
#ifdef FB_DEBUG
			FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_LIST_Create end\n");
#endif
			return (__s32)OSD_RET_OK;
		}
	}

#endif
	return -(__s32)OSD_RET_OUT_OF_LIST;
}
EXPORT_SYMBOL(OSD_RGN_LIST_Create);

/*the funciton is add for wt */
__s32 OSD_RGN_LIST_Create_Ex(__u32 *pu4RgnList, bool b_compressed)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4Index;

	if (pu4RgnList == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	/*[Xiaoxu Du]
	Because OSD_MAX_NUM_RGN_LIST is only 20 or 30,
	a linear search might be acceptable here.
	Creation of a region wont happen so often,
	hence even we have hundreds of region list,
	it should be ok here. */
	for (i4Index = 0; i4Index < OSD_MAX_NUM_RGN_LIST; i4Index++) {
		if ((_rAllRgnList[i4Index].u4ListStatus & OSD_RGN_LIST_ALLOCATED) == 0) {
			/* free cell found, allocate this list, init and return it*/
			_rAllRgnList[i4Index].u4ListStatus |= OSD_RGN_LIST_ALLOCATED;
			_rAllRgnList[i4Index].i4Head     = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[i4Index].i4Tail     = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[i4Index].i4Count   = 0;
			_rAllRgnList[i4Index].b_compressed = b_compressed;
			*pu4RgnList = (__u32)i4Index;
			return (__s32)OSD_RET_OK;
		}
	}

#endif
	return -(__s32)OSD_RET_OUT_OF_LIST;
}

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_Delete(__u32 u4RgnList)
{
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	_rAllRgnList[u4RgnList].u4ListStatus &= ~OSD_RGN_LIST_ALLOCATED;
	return (__s32)OSD_RET_OK;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_GetHead(__u32 u4RgnList, __s32 *pi4HeadRegion)
{
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/

	if (pi4HeadRegion == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	*pi4HeadRegion = _rAllRgnList[u4RgnList].i4Head;
	return (__s32)OSD_RET_OK;
#endif
	return -(__s32)OSD_RET_INV_ARG;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_GetNext(__u32 u4RgnList, __s32 i4RgnCurr, __s32 *pi4RgnNext)
{
	__s32 i4Ret;
	__s32 i4RgnNext;

	OSD_VERIFY_RGNLIST(u4RgnList);

	if (pi4RgnNext == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	i4Ret = OSD_RGN_Get((__u32)i4RgnCurr, (__s32)OSD_RGN_NEXT, (__u32 *)&i4RgnNext);

	if (i4Ret) {
		return i4Ret;
	}

	*pi4RgnNext = i4RgnNext;
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_DetachAll(__u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4CurrRgn, i4NextRgn;
#endif

	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	i4CurrRgn = _rAllRgnList[u4RgnList].i4Head;

	while (i4CurrRgn != OSD_INVALID_REGION_HANDLE) {
		i4NextRgn = _rAllRgnNode[i4CurrRgn].i4Next;
		_rAllRgnNode[i4CurrRgn].i4Prev = OSD_INVALID_REGION_HANDLE;
		_rAllRgnNode[i4CurrRgn].i4Next = OSD_INVALID_REGION_HANDLE;
		_rAllRgnNode[i4CurrRgn].u4NodeStatus &= ~OSD_RGN_NODE_ALLOCATED;
		/*VERIFY((__s32)OSD_RET_OK == _OSD_RGN_Free((__u32)i4CurrRgn));*/
		i4CurrRgn = i4NextRgn;
	}

	_rAllRgnList[u4RgnList].i4Head = OSD_INVALID_REGION_HANDLE;
	_rAllRgnList[u4RgnList].i4Tail = OSD_INVALID_REGION_HANDLE;
	_rAllRgnList[u4RgnList].i4Count = 0;

	return (__s32)OSD_RET_OK;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}
EXPORT_SYMBOL(OSD_RGN_LIST_DetachAll);


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_LIST_GetCount(__u32 u4RgnList, __s32 *pi4Count)
{
	__s32 i4Count;
	__s32 i4Ret;

	if (pi4Count == NULL) {
		return (__s32)OSD_RET_INV_ARG;
	}

	i4Ret = OSD_RGN_LIST_Get(u4RgnList, (__s32)OSD_RGN_LIST_COUNT,
				 (__u32 *)&i4Count);

	if (i4Ret) {
		return i4Ret;
	}

	*pi4Count = i4Count;
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Get(__u32 u4Region, __s32 i4Cmd, __u32 *pu4Value)
{
	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	OSD_VERIFY_REGION(u4Region);

	switch (i4Cmd) {
	case OSD_RGN_PREV:
		*pu4Value = (__u32)_rAllRgnNode[u4Region].i4Prev;
		break;

	case OSD_RGN_NEXT:
		*pu4Value = (__u32)_rAllRgnNode[u4Region].i4Next;
		break;

	case OSD_RGN_FLAGS:
		*pu4Value = _rAllRgnNode[u4Region].u4NodeStatus;
		break;

	case OSD_RGN_POS_X:
		return _OSD_RGN_GetOutputPosX(u4Region, pu4Value);

	case OSD_RGN_POS_Y:
		return _OSD_RGN_GetOutputPosY(u4Region, pu4Value);

	case OSD_RGN_BMP_W:
		return _OSD_RGN_GetInputWidth(u4Region, pu4Value);

	case OSD_RGN_BMP_H:
		return _OSD_RGN_GetInputHeight(u4Region, pu4Value);

	case OSD_RGN_DISP_W:
	case OSD_RGN_OUT_W:
		return _OSD_RGN_GetOutputWidth(u4Region, pu4Value);

	case OSD_RGN_DISP_H:
	case OSD_RGN_OUT_H:
		return _OSD_RGN_GetOutputHeight(u4Region, pu4Value);

	case OSD_RGN_COLORMODE:
		return _OSD_RGN_GetColorMode(u4Region, pu4Value);

	case OSD_RGN_ALPHA:
		return _OSD_RGN_GetAlpha(u4Region, pu4Value);

	case OSD_RGN_BMP_ADDR:
		return _OSD_RGN_GetDataAddr(u4Region, pu4Value);

	case OSD_RGN_BMP_PITCH: {
		__u32 u4LineSize, u4LineSize8;

		IGNORE_RET(_OSD_RGN_GetLineSize(u4Region, &u4LineSize));
		IGNORE_RET(_OSD_RGN_GetLineSize8(u4Region, &u4LineSize8));
		*pu4Value = (u4LineSize8 << 13) | (u4LineSize << 4);
		break;
	}

	case OSD_RGN_CLIP_V:
		return _OSD_RGN_GetVClip(u4Region, pu4Value);

	case OSD_RGN_CLIP_H:
		return _OSD_RGN_GetHClip(u4Region, pu4Value);

	case OSD_RGN_PAL_LOAD:
		return _OSD_RGN_GetLoadPalette(u4Region, pu4Value);

	case OSD_RGN_PAL_ADDR:
		return _OSD_RGN_GetPaletteAddr(u4Region, pu4Value);

	case OSD_RGN_PAL_PA:
		return _OSD_RGN_GetPalettePA(u4Region, pu4Value);

	case OSD_RGN_PAL_LEN:
		return _OSD_RGN_GetPaletteLen(u4Region, pu4Value);

	case OSD_RGN_STEP_V:
		return _OSD_RGN_GetVStep(u4Region, pu4Value);

	case OSD_RGN_STEP_H:
		return _OSD_RGN_GetHStep(u4Region, pu4Value);

	case OSD_RGN_COLOR_KEY:
		return _OSD_RGN_GetColorKey(u4Region, pu4Value);

	case OSD_RGN_COLOR_KEY_EN:
		return _OSD_RGN_GetColorKeyEnable(u4Region, pu4Value);

	case OSD_RGN_MIX_SEL:
		return _OSD_RGN_GetBlendMode(u4Region, pu4Value);

#ifdef CC_MT5381

	case OSD_RGN_V_FLIP:
		return _OSD_RGN_GetVFlip(u4Region, pu4Value);

	case OSD_RGN_H_MIRROR:
		return _OSD_RGN_GetHMirror(u4Region, pu4Value);
#endif

	case OSD_RGN_ALPHA_SEL:
		return _OSD_RGN_GetASel(u4Region, pu4Value);

	case OSD_RGN_YR_SEL:
		return _OSD_RGN_GetYrSel(u4Region, pu4Value);

	case OSD_RGN_UG_SEL:
		return _OSD_RGN_GetUgSel(u4Region, pu4Value);

	case OSD_RGN_VB_SEL:
		return _OSD_RGN_GetVbSel(u4Region, pu4Value);

	case OSD_RGN_NEXT_EN:
		return _OSD_RGN_GetNextEnable(u4Region, pu4Value);

	case OSD_RGN_NEXT_HDR_ADDR:
		return _OSD_RGN_GetNextRegion(u4Region, pu4Value);

	case OSD_RGN_FIFO_EX:
		return _OSD_RGN_GetFifoEx(u4Region, pu4Value);

	default:
		return -(__s32)OSD_RET_INV_ARG;
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Set(__u32 u4Region, __s32 i4Cmd, unsigned long u4Value)
{
	OSD_VERIFY_REGION(u4Region);

	switch (i4Cmd) {
	case OSD_RGN_PREV:
		_rAllRgnNode[u4Region].i4Prev = (__s32)u4Value;
		break;

	case OSD_RGN_NEXT:
		_rAllRgnNode[u4Region].i4Next = (__s32)u4Value;
		break;

	case OSD_RGN_FLAGS:
		_rAllRgnNode[u4Region].u4NodeStatus = u4Value;
		break;

	case OSD_RGN_POS_X:
		return _OSD_RGN_SetOutputPosX(u4Region, u4Value);

	case OSD_RGN_POS_Y:
		return _OSD_RGN_SetOutputPosY(u4Region, u4Value);

	case OSD_RGN_BMP_W:
		return _OSD_RGN_SetInputWidth(u4Region, u4Value);

	case OSD_RGN_BMP_H:
		return _OSD_RGN_SetInputHeight(u4Region, u4Value);

	case OSD_RGN_DISP_W:
		return OSD_RGN_SetDisplayWidth(u4Region, u4Value);

	case OSD_RGN_DISP_H:
		return OSD_RGN_SetDisplayHeight(u4Region, u4Value);

	case OSD_RGN_OUT_W:
		return _OSD_RGN_SetOutputWidth(u4Region, u4Value);

	case OSD_RGN_OUT_H:
		return _OSD_RGN_SetOutputHeight(u4Region, u4Value);

	case OSD_RGN_COLORMODE:
		return _OSD_RGN_SetColorMode(u4Region, u4Value);

	case OSD_RGN_ALPHA:
		return _OSD_RGN_SetAlpha(u4Region, u4Value);

	case OSD_RGN_BMP_ADDR:
		return _OSD_RGN_SetDataAddr(u4Region, u4Value);

	case OSD_RGN_BMP_PITCH:
		IGNORE_RET(_OSD_RGN_SetLineSize(u4Region, (u4Value >> 4) & 0x1ff));
		IGNORE_RET(_OSD_RGN_SetLineSize8(u4Region, (u4Value >> 13) & 1));
		break;

	case OSD_RGN_CLIP_V: {
#ifdef OSD_REGION_CLIP_TWO_SIDE
		__u32 u4SrcHeight, u4OldVClip;
		/* recover orginal input height*/
		IGNORE_RET(_OSD_RGN_GetVClip(u4Region, &u4OldVClip));
		IGNORE_RET(_OSD_RGN_GetInputHeight(u4Region, &u4SrcHeight));
		u4SrcHeight += u4OldVClip;

		if (u4SrcHeight <= u4Value) {
			return -(__s32)OSD_RET_INV_ARG;
		}

		/* cut input height*/
		IGNORE_RET(_OSD_RGN_SetInputHeight(u4Region, u4SrcHeight - u4Value));
#endif /* OSD_REGION_CLIP_TWO_SIDE*/

		return _OSD_RGN_SetVClip(u4Region, u4Value);
	}

	case OSD_RGN_CLIP_H: {
#ifdef OSD_REGION_CLIP_TWO_SIDE
		__u32 u4SrcWidth, u4OldHClip;
		/* recover orginal input width*/
		IGNORE_RET(_OSD_RGN_GetHClip(u4Region, &u4OldHClip));
		IGNORE_RET(_OSD_RGN_GetInputWidth(u4Region, &u4SrcWidth));
		u4SrcWidth += u4OldHClip;

		if (u4SrcWidth <= u4Value) {
			return -(__s32)OSD_RET_INV_ARG;
		}

		/* cut input width*/
		IGNORE_RET(_OSD_RGN_SetInputWidth(u4Region, u4SrcWidth - u4Value));
#endif

		return _OSD_RGN_SetHClip(u4Region, u4Value);
	}

	case OSD_RGN_PAL_LOAD:
		return _OSD_RGN_SetLoadPalette(u4Region, u4Value);

	case OSD_RGN_PAL_ADDR:
		return _OSD_RGN_SetPaletteAddr(u4Region, u4Value);

	case OSD_RGN_PAL_PA:
		return _OSD_RGN_SetPalettePA(u4Region, u4Value);

	case OSD_RGN_PAL_LEN:
		return _OSD_RGN_SetPaletteLen(u4Region, u4Value);

	case OSD_RGN_STEP_V:
		return _OSD_RGN_SetVStep(u4Region, u4Value);

	case OSD_RGN_STEP_H:
		return _OSD_RGN_SetHStep(u4Region, u4Value);

	case OSD_RGN_COLOR_KEY:
		return _OSD_RGN_SetColorKey(u4Region, u4Value);

	case OSD_RGN_COLOR_KEY_EN:
		return _OSD_RGN_SetColorKeyEnable(u4Region, u4Value);

	case OSD_RGN_MIX_SEL:
		return _OSD_RGN_SetBlendMode(u4Region, u4Value);

	case OSD_RGN_BIG_ENDIAN:
		return OSD_RGN_SetBigEndian(u4Region, u4Value);

	case OSD_RGN_DECOMP_MODE:
		return _OSD_RGN_SetDeCompMode(u4Region, u4Value);

	case OSD_RGN_WT_EN:
		return _OSD_RGN_SetWTEn(u4Region, u4Value);

	case OSD_RGN_SELECT_BYTE_EN:
		return _OSD_RGN_SetSelectByteEn(u4Region, u4Value);

	default:
		return -(__s32)OSD_RET_INV_ARG;
	}

	return (__s32)OSD_RET_OK;
}
EXPORT_SYMBOL(OSD_RGN_Set);



/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Create(__u32 *pu4Region, __u32 u4BmpWidth, __u32 u4BmpHeight,
		     void *pvBitmap, __u32 eColorMode, __u32 u4BmpPitch,
		     __u32 u4DispX, __u32 u4DispY,
		     __u32 u4DispW, __u32 u4DispH)
{
	return OSD_RGN_Create_EX(pu4Region, u4BmpWidth,  u4BmpHeight,
				 pvBitmap,  eColorMode, u4BmpPitch,
				 u4DispX,  u4DispY,
				 u4DispW,  u4DispH, 0);
}
EXPORT_SYMBOL(OSD_RGN_Create);


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Create_EX(__u32 *pu4Region, __u32 u4BmpWidth, __u32 u4BmpHeight,
			void *pvBitmap, __u32 eColorMode, __u32 u4BmpPitch,
			__u32 u4DispX, __u32 u4DispY,
			__u32 u4DispW, __u32 u4DispH, __u32 ui4Plane)
{
	__u32 u4Region;
	__s32 i4Ret;
	__s32 i4MemChn;

	if ((pu4Region == NULL) || (pvBitmap == NULL) ||
	    (((unsigned long)pvBitmap & (unsigned long)0xf) != 0) || (u4BmpWidth == 0) ||
	    (u4BmpHeight == 0) || (u4DispW == 0) || (u4DispH == 0) ||
	    ((u4BmpPitch & 0xf) != 0) || ((u4BmpPitch >> 14) != 0)) {
		return -(__s32)OSD_RET_INV_ARG;
	}

#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Create_EX begin:u4BmpPitch=%d\n", u4BmpPitch);
#endif
	i4MemChn = OSD_DRV_GetMemChannelEx(ui4Plane);
	i4Ret = _OSD_RGN_Alloc(&u4Region,  i4MemChn);
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Create_EX:i4Ret=%d\n", i4Ret);
#endif

	if (i4Ret != (__s32)OSD_RET_OK) {
		return i4Ret;
	}

	i4Ret = _OSD_RGN_SetNextRegion(u4Region, 0);
	i4Ret = _OSD_RGN_SetNextEnable(u4Region, 0);
	i4Ret = _OSD_RGN_SetColorMode(u4Region, eColorMode);
	i4Ret = _OSD_RGN_SetDataAddr(u4Region, ((unsigned long)pvBitmap));
	i4Ret = _OSD_RGN_SetAlpha(u4Region, (__u32)0xFF);
	i4Ret = _OSD_RGN_SetInputWidth(u4Region, u4BmpWidth);
	i4Ret = _OSD_RGN_SetInputHeight(u4Region, u4BmpHeight);
	i4Ret = _OSD_RGN_SetOutputWidth(u4Region, u4DispW);
	i4Ret = _OSD_RGN_SetOutputHeight(u4Region, u4DispH);
	i4Ret = _OSD_RGN_SetOutputPosX(u4Region, u4DispX);
	i4Ret = _OSD_RGN_SetOutputPosY(u4Region, u4DispY);
	i4Ret = _OSD_RGN_SetColorKeyEnable(u4Region, 0);
	i4Ret = _OSD_RGN_SetColorKey(u4Region, 0);
	i4Ret = _OSD_RGN_SetHClip(u4Region, 0);
	i4Ret = _OSD_RGN_SetVClip(u4Region, 0);
	i4Ret = _OSD_RGN_SetAutoMode(u4Region, 1);
	i4Ret = _OSD_RGN_SetMixSel(u4Region, 1);

	if (u4BmpPitch == 0) {
		if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
		    (eColorMode == (__u32)OSD_CM_RGB_CLUT2)) {
			u4BmpPitch = u4BmpWidth >> 2;
		} else if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			   (eColorMode == (__u32)OSD_CM_RGB_CLUT4)) {
			u4BmpPitch = u4BmpWidth >> 1;
		} else if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			   (eColorMode == (__u32)OSD_CM_RGB_CLUT8)) {
			u4BmpPitch = u4BmpWidth;
		} else if ((eColorMode == (__u32)OSD_CM_AYCBCR8888_DIRECT32) ||
			   (eColorMode == (__u32)OSD_CM_ARGB8888_DIRECT32)) {
			u4BmpPitch = u4BmpWidth << 2;
		} else {
			u4BmpPitch = u4BmpWidth << 1;
		}
	}

	i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_BMP_PITCH, u4BmpPitch);
	/* i4Ret = _OSD_RGN_SetLineSize(u4Region, (u4BmpPitch >> 4) & 0x1ff);
	    i4Ret = _OSD_RGN_SetLineSize8(u4Region, u4BmpPitch >> 13);*/
	i4Ret = _OSD_RGN_SetHStep(u4Region, (u4BmpWidth  == u4DispW) ? 0x1000 :
				  ((u4BmpWidth << 12)  / u4DispW));
	i4Ret = _OSD_RGN_SetVStep(u4Region, (u4BmpHeight == u4DispH) ? 0x1000 :
				  ((u4BmpHeight << 12) / u4DispH));
	i4Ret = _OSD_RGN_SetBlendMode(u4Region, (__u32)OSD_BM_PIXEL);

	i4Ret = _OSD_RGN_SetASel(u4Region, 3);
	i4Ret = _OSD_RGN_SetYrSel(u4Region, 2);
	i4Ret = _OSD_RGN_SetUgSel(u4Region, 1);
	i4Ret = _OSD_RGN_SetVbSel(u4Region, 0);

	/* palette mode*/
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Create_EX:eColorMode=%d\n", eColorMode);
#endif

	if (((eColorMode <= (__u32)OSD_CM_YCBCR_CLUT8) ||
	     (eColorMode >= (__u32)OSD_CM_RGB_CLUT2)) &&
	    (eColorMode <= (__u32)OSD_CM_RGB_CLUT8)) {
		if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
		    (eColorMode == (__u32)OSD_CM_RGB_CLUT2)) {
			i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_PAL_LEN, 0);
		} else if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			   (eColorMode == (__u32)OSD_CM_RGB_CLUT4)) {
			i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_PAL_LEN, 1);
		} else if ((eColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			   (eColorMode == (__u32)OSD_CM_RGB_CLUT8)) {
			i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_PAL_LEN, 2);
		}

		/* remove it  */
		/*OSD_CreatePaletteTable((OSD_ARGB_T *) (((__u32)pvBitmap +
			(u4BmpPitch * u4BmpHeight) + 0x2000) & ~0x1fff), eColorMode);*/

#if 0
		i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_PAL_ADDR,
				    (((unsigned long)pvBitmap +
				      (u4BmpPitch * u4BmpHeight) + 0x2000) & ~0x1fff));
#endif
		i4Ret = OSD_RGN_Set(u4Region, (__s32)OSD_RGN_PAL_LOAD, 1);
	}

	UNUSED(i4Ret);
	_rAllRgnNode[u4Region].u4NodeStatus |= OSD_RGN_NODE_ALLOCATED;
	_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
	_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
	*pu4Region = u4Region;
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Create_EX:*pu4Region=%d, _rAllRgnNode[u4Region].u4NodeStatus=%d\n",
		 *pu4Region, _rAllRgnNode[u4Region].u4NodeStatus);
#endif
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Delete(__u32 u4Region)
{
	OSD_VERIFY_REGION(u4Region);

	if (_rAllRgnNode[u4Region].i4Prev != OSD_INVALID_REGION_HANDLE) {
		__s32 i4Prev;

		i4Prev = _rAllRgnNode[u4Region].i4Prev;
		if ((i4Prev >= 0) && (i4Prev < OSD_MAX_NUM_RGN)) {
			/* assign [i4Prev].i4Next = [u4Region].i4Next*/
			_rAllRgnNode[i4Prev].i4Next = _rAllRgnNode[u4Region].i4Next;
		}
	}

	if (_rAllRgnNode[u4Region].i4Next != OSD_INVALID_REGION_HANDLE) {
		__s32 i4Next;

		i4Next = _rAllRgnNode[u4Region].i4Next;
		if ((i4Next >= 0) && (i4Next < OSD_MAX_NUM_RGN)) {
			/* assign [i4Next].i4Prev = [u4Region].i4Prev*/
			_rAllRgnNode[i4Next].i4Prev = _rAllRgnNode[u4Region].i4Prev;
		}
	}

	VERIFY((__s32)OSD_RET_OK == _OSD_RGN_Free(u4Region));
	_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
	_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
	_rAllRgnNode[u4Region].u4NodeStatus &= ~OSD_RGN_NODE_ALLOCATED;

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_AttachTail(__u32 u4Region, __u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4OrigTail;
	__u32 u4RgnAddr;
	__u32 u4ColorMode;
	OSD_REGION_LIST_T *prRgnList;
	__s32 i4Ignore;
#endif

	OSD_VERIFY_REGION(u4Region);
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	prRgnList = &_rAllRgnList[u4RgnList];
	i4OrigTail = prRgnList->i4Tail;

	if (((void *)prRgnList->i4Head == NULL) && (prRgnList->i4Count == 0)) {
		return OSD_RGN_Insert(u4Region, u4RgnList);
	}

	if (i4OrigTail == OSD_INVALID_REGION_HANDLE) {
		return -(__s32)OSD_RET_INV_LIST;
	}

	_rAllRgnList[u4RgnList].i4Count++;
	_rAllRgnList[u4RgnList].i4Tail = (__s32)u4Region;
	_rAllRgnNode[i4OrigTail].i4Next = (__s32)u4Region;
	_rAllRgnNode[u4Region].i4Prev = i4OrigTail;

	i4Ignore = _OSD_RGN_GetAddress(u4Region, (unsigned long*)&u4RgnAddr);
	/*ASSERT(u4RgnAddr & 0xf);*/
	ASSERT((u4RgnAddr & 0xf) == 0);
	i4Ignore = _OSD_RGN_SetNextRegion((__u32)i4OrigTail, u4RgnAddr);
	i4Ignore = _OSD_RGN_SetNextEnable((__u32)i4OrigTail, 1);
	i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0); /* tail means 'no next'*/
	i4Ignore = _OSD_RGN_SetNextRegion(u4Region, 0);

	i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

	if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
	    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
	    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
	    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
	    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
	    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {
		/* check if we should load new palette*/
		__u32 u4Pal1, u4Pal2;
		__u32 u4Len1, u4Len2;

		i4Ignore = _OSD_RGN_GetPaletteAddr(u4Region, &u4Pal1);
		i4Ignore = _OSD_RGN_GetPaletteLen(u4Region, &u4Len1);
		i4Ignore = _OSD_RGN_GetPaletteAddr((__u32)i4OrigTail, &u4Pal2);
		i4Ignore = _OSD_RGN_GetPaletteLen((__u32)i4OrigTail, &u4Len2);
		i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, ((u4Pal1 == u4Pal2) &&
							      (u4Len1 == u4Len2)) ? 0 : 1);
	} else {
		/* not index color mode, should not load palette*/
		i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
	}

	UNUSED(i4Ignore);
	return (__s32)OSD_RET_OK;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Insert(__u32 u4Region, __u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4RgnCurr, i4RgnNext;
	OSD_REGION_LIST_T *prRgnList;
	__u32 u4ColorMode;
	unsigned long u4Address;
	__s32 i4Ignore = 0;
#endif

	OSD_VERIFY_REGION(u4Region);
	OSD_VERIFY_RGNLIST(u4RgnList);
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Insert begin\n");
#endif

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	prRgnList = &_rAllRgnList[u4RgnList];
	i4RgnCurr = prRgnList->i4Head;
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Insert:i4RgnCurr=%d, i4Count=%d\n", i4RgnCurr, prRgnList->i4Count);
#endif

	if (i4RgnCurr == OSD_INVALID_REGION_HANDLE) {
		if (prRgnList->i4Count == 0) {
			_rAllRgnList[u4RgnList].i4Head = (__s32)u4Region;
			_rAllRgnList[u4RgnList].i4Tail = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[u4RgnList].i4Count = 1;
#ifdef FB_DEBUG
			FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Insert:i4Head=%d\n", _rAllRgnList[u4RgnList].i4Head);
#endif
			i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0);
			i4Ignore = _OSD_RGN_SetNextRegion(u4Region, 0);

			i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);
#ifdef FB_DEBUG
			FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Insert:u4ColorMode=%d\n", u4ColorMode);
#endif

			if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {

				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 1);
			} else {
				/* not index color mode, should not load palette*/
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
			}

#ifdef FB_DEBUG
			FB_PRINT(FB_LOG_LVL_DBG, "OSD_RGN_Insert end\n");
#endif
			return (__s32)OSD_RET_OK;
		} else {
			return -(__s32)OSD_RET_INV_LIST;
		}
	}

	OSD_VERIFY_REGION(i4RgnCurr);

	switch (_OsdRegionCompare(u4Region, (__u32)i4RgnCurr)) {
	case -1:
		prRgnList->i4Head = (__s32)u4Region;
		i4Ignore = OSD_PLA_Reflip(u4RgnList);
		_rAllRgnNode[u4Region].i4Next = i4RgnCurr;
		_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
		_rAllRgnNode[i4RgnCurr].i4Prev = (__s32)u4Region;
		prRgnList->i4Count++;

		i4Ignore = _OSD_RGN_GetAddress((__u32)i4RgnCurr, (unsigned long*)&u4Address);
		i4Ignore = _OSD_RGN_SetNextRegion(u4Region, u4Address);
		i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 1);


		i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

		if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
		    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
		    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {

			i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 1);
		} else {
			/* not index color mode, should not load palette*/
			i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
		}

		return (__s32)OSD_RET_OK;

	case 1:
		break;

	default:
		return -(__s32)OSD_RET_REGION_COLLISION;
	}

	while ((i4RgnNext = _rAllRgnNode[i4RgnCurr].i4Next) !=
	       OSD_INVALID_REGION_HANDLE) {
		if ((i4RgnNext < 0) || (i4RgnNext >= OSD_MAX_NUM_RGN)) {
			i4RgnNext = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
			break;
		}

		switch (_OsdRegionCompare(u4Region, (__u32)i4RgnNext)) {
		case -1:
			/* insert*/
			_rAllRgnNode[i4RgnCurr].i4Next = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Next = i4RgnNext;
			_rAllRgnNode[i4RgnNext].i4Prev = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Prev = i4RgnCurr;
			prRgnList->i4Count++;

			/* maintain hw list*/
			i4Ignore = _OSD_RGN_GetAddress(u4Region, (unsigned long*)&u4Address);
			/* current --> region*/
			i4Ignore = _OSD_RGN_SetNextRegion((__u32)i4RgnCurr, u4Address);
			i4Ignore = _OSD_RGN_GetAddress((__u32)i4RgnNext, (unsigned long*)&u4Address);
			/* region --> next*/
			i4Ignore = _OSD_RGN_SetNextRegion(u4Region, u4Address);

			if (i4RgnNext != OSD_INVALID_REGION_HANDLE) {
				/* valid next*/
				i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 1);
			} else {
				/* no next*/
				i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0);
			}

			i4Ignore = _OSD_RGN_SetNextEnable((__u32)i4RgnCurr, 1);


			i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

			if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {
				/* check if we should load new palette*/
				__u32 u4Pal1, u4Pal2;
				__u32 u4Len1, u4Len2;

				i4Ignore = _OSD_RGN_GetPaletteAddr(u4Region, &u4Pal1);
				i4Ignore = _OSD_RGN_GetPaletteLen(u4Region, &u4Len1);
				i4Ignore = _OSD_RGN_GetPaletteAddr((__u32)i4RgnCurr, &u4Pal2);
				i4Ignore = _OSD_RGN_GetPaletteLen((__u32)i4RgnCurr, &u4Len2);
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region,
								   ((u4Pal1 == u4Pal2) &&
								    (u4Len1 == u4Len2)) ? 0 : 1);
			} else {
				/* not index color mode, should not load palette*/
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
			}

			return (__s32)OSD_RET_OK;

		case 1:
			i4RgnCurr = i4RgnNext;
			break;

		default:
			return -(__s32)OSD_RET_REGION_COLLISION;
		}
	}

	UNUSED(i4Ignore);

	_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
	return OSD_RGN_AttachTail(u4Region, u4RgnList);
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}
EXPORT_SYMBOL(OSD_RGN_Insert);


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_ARB_RGN_Insert(__u32 u4Region, __u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4RgnCurr, i4RgnNext;
	OSD_REGION_LIST_T *prRgnList;
	__u32 u4ColorMode;
	unsigned long u4Address;
	__s32 i4Ignore = 0;
#endif

	OSD_VERIFY_REGION(u4Region);
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	prRgnList = &_rAllRgnList[u4RgnList];
	i4RgnCurr = prRgnList->i4Head;

	if (i4RgnCurr == OSD_INVALID_REGION_HANDLE) {
		if (prRgnList->i4Count == 0) {
			_rAllRgnList[u4RgnList].i4Head = (__s32)u4Region;
			_rAllRgnList[u4RgnList].i4Tail = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[u4RgnList].i4Count = 1;

			i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0);
			i4Ignore = _OSD_RGN_SetNextRegion(u4Region, 0);

			i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

			if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {

				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 1);
			} else {
				/* not index color mode, should not load palette*/
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
			}

			return (__s32)OSD_RET_OK;
		} else {
			return -(__s32)OSD_RET_INV_LIST;
		}
	}

	/**/
	if (prRgnList->i4Count == OSD_ARB_RGN_MAX) {
		return OSD_RET_OUT_OF_REGION;
	}

	OSD_VERIFY_REGION(i4RgnCurr);

	switch (_OsdArbRegionCompare(u4Region, (__u32)i4RgnCurr)) {
	case -1:
		prRgnList->i4Head = (__s32)u4Region;
		i4Ignore = OSD_PLA_Reflip(u4RgnList);
		_rAllRgnNode[u4Region].i4Next = i4RgnCurr;
		_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
		_rAllRgnNode[i4RgnCurr].i4Prev = (__s32)u4Region;
		prRgnList->i4Count++;

		i4Ignore = _OSD_RGN_GetAddress((__u32)i4RgnCurr, (unsigned long*)&u4Address);
		i4Ignore = _OSD_RGN_SetNextRegion(u4Region, u4Address);
		i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 1);


		i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

		if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
		    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
		    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
		    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {

			i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 1);
		} else {
			/* not index color mode, should not load palette*/
			i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
		}

		return (__s32)OSD_RET_OK;

	case 1:
		break;

	default:
		return -(__s32)OSD_RET_REGION_COLLISION;
	}

	while ((i4RgnNext = _rAllRgnNode[i4RgnCurr].i4Next) !=
	       OSD_INVALID_REGION_HANDLE) {
		if ((i4RgnNext < 0) || (i4RgnNext >= OSD_MAX_NUM_RGN)) {
			i4RgnNext = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
			break;
		}

		switch (_OsdArbRegionCompare(u4Region, (__u32)i4RgnNext)) {
		case -1:
			/* insert*/
			_rAllRgnNode[i4RgnCurr].i4Next = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Next = i4RgnNext;
			_rAllRgnNode[i4RgnNext].i4Prev = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Prev = i4RgnCurr;
			prRgnList->i4Count++;

			/* maintain hw list*/
			i4Ignore = _OSD_RGN_GetAddress(u4Region, (unsigned long*)&u4Address);
			/* current --> region*/
			i4Ignore = _OSD_RGN_SetNextRegion((__u32)i4RgnCurr, u4Address);
			i4Ignore = _OSD_RGN_GetAddress((__u32)i4RgnNext, (unsigned long*)&u4Address);
			/* region --> next*/
			i4Ignore = _OSD_RGN_SetNextRegion(u4Region, u4Address);

			if (i4RgnNext != OSD_INVALID_REGION_HANDLE) {
				/* valid next*/
				i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 1);
			} else {
				/* no next*/
				i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0);
			}

			i4Ignore = _OSD_RGN_SetNextEnable((__u32)i4RgnCurr, 1);


			i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

			if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {
				/* check if we should load new palette*/
				__u32 u4Pal1, u4Pal2;
				__u32 u4Len1, u4Len2;

				i4Ignore = _OSD_RGN_GetPaletteAddr(u4Region, &u4Pal1);
				i4Ignore = _OSD_RGN_GetPaletteLen(u4Region, &u4Len1);
				i4Ignore = _OSD_RGN_GetPaletteAddr((__u32)i4RgnCurr, &u4Pal2);
				i4Ignore = _OSD_RGN_GetPaletteLen((__u32)i4RgnCurr, &u4Len2);
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region,
								   ((u4Pal1 == u4Pal2) &&
								    (u4Len1 == u4Len2)) ? 0 : 1);
			} else {
				/* not index color mode, should not load palette*/
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
			}

			return (__s32)OSD_RET_OK;

		case 1:
			i4RgnCurr = i4RgnNext;
			break;

		default:
			return -(__s32)OSD_RET_REGION_COLLISION;
		}
	}

	UNUSED(i4Ignore);

	_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
	return OSD_RGN_AttachTail(u4Region, u4RgnList);
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 *  @Note - Just for Verify Use.
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_ARB_RGN_Insert_Ex(__u32 u4Region, __u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4RgnCurr, i4RgnNext;
	OSD_REGION_LIST_T *prRgnList;
	__u32 u4ColorMode;
	__s32 i4Ignore = 0;
#endif

	OSD_VERIFY_REGION(u4Region);
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	prRgnList = &_rAllRgnList[u4RgnList];
	i4RgnCurr = prRgnList->i4Head;

	if (i4RgnCurr == OSD_INVALID_REGION_HANDLE) {
		if (prRgnList->i4Count == 0) {
			_rAllRgnList[u4RgnList].i4Head = (__s32)u4Region;
			_rAllRgnList[u4RgnList].i4Tail = (__s32)u4Region;
			_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
			_rAllRgnList[u4RgnList].i4Count = 1;

			i4Ignore = _OSD_RGN_SetNextEnable(u4Region, 0);
			i4Ignore = _OSD_RGN_SetNextRegion(u4Region, 0);

			i4Ignore = _OSD_RGN_GetColorMode(u4Region, &u4ColorMode);

			if ((u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_YCBCR_CLUT8) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT2) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT4) ||
			    (u4ColorMode == (__u32)OSD_CM_RGB_CLUT8)) {

				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 1);
			} else {
				/* not index color mode, should not load palette*/
				i4Ignore = _OSD_RGN_SetLoadPalette(u4Region, 0);
			}

			return (__s32)OSD_RET_OK;
		} else {
			return -(__s32)OSD_RET_INV_LIST;
		}
	}

	/**/
	if (prRgnList->i4Count == OSD_ARB_RGN_MAX) {
		return OSD_RET_OUT_OF_REGION;
	}

	OSD_VERIFY_REGION(i4RgnCurr);

	i4RgnNext = _rAllRgnList[u4RgnList].i4Head;

	do {
		if ((i4RgnNext < 0) || (i4RgnNext >= OSD_MAX_NUM_RGN)) {
			i4RgnNext = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
			break;
		}

		switch (_OsdArbRegionCompare(u4Region, (__u32)i4RgnNext)) {
		case -1:
		case  1:
			i4RgnCurr = i4RgnNext;
			break;

		default:
			return -(__s32)OSD_RET_REGION_COLLISION;
		}
	} while ((i4RgnNext = _rAllRgnNode[i4RgnCurr].i4Next) !=
		 OSD_INVALID_REGION_HANDLE);

	UNUSED(i4Ignore);

	_rAllRgnNode[i4RgnCurr].i4Next = i4RgnNext;
	return OSD_RGN_AttachTail(u4Region, u4RgnList);
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Detach(__u32 u4Region, __u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4RgnCurr, i4RgnNext, i4RgnPrev;
	OSD_REGION_LIST_T *prRgnList;
	__u32 u4Address;
	__s32 i4Ignore = 0;
#endif

	OSD_VERIFY_REGION(u4Region);
	OSD_VERIFY_RGNLIST(u4RgnList);

#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	prRgnList = &_rAllRgnList[u4RgnList];
	i4RgnCurr = prRgnList->i4Head;

	if (prRgnList->i4Head == OSD_INVALID_REGION_HANDLE) {
		return -(__s32)OSD_RET_INV_REGION;
	}

	while (i4RgnCurr != OSD_INVALID_REGION_HANDLE) {
		OSD_VERIFY_REGION(i4RgnCurr);

		if ((__u32)i4RgnCurr == u4Region) {
			i4RgnNext = _rAllRgnNode[u4Region].i4Next;
			i4RgnPrev = _rAllRgnNode[u4Region].i4Prev;

			if (i4RgnNext != OSD_INVALID_REGION_HANDLE) {
				_rAllRgnNode[i4RgnNext].i4Prev = i4RgnPrev;
			} else { /* region is tail*/
				/* attached to list tail*/
				prRgnList->i4Tail = i4RgnPrev;
			}

			if (i4RgnPrev != OSD_INVALID_REGION_HANDLE) {
				_rAllRgnNode[i4RgnPrev].i4Next = i4RgnNext;
			} else { /* region is head*/
				/* new list head*/
				prRgnList->i4Head = i4RgnNext;
			}

			/* maintain hardware list*/
			if ((i4RgnNext == OSD_INVALID_REGION_HANDLE) &&
			    (i4RgnPrev != OSD_INVALID_REGION_HANDLE)) {
				/* region is tail*/
				i4Ignore = _OSD_RGN_SetNextRegion((__u32)i4RgnPrev, 0);
				i4Ignore = _OSD_RGN_SetNextEnable((__u32)i4RgnPrev, 0);
			} else if ((i4RgnNext != OSD_INVALID_REGION_HANDLE)
				   && (i4RgnPrev == OSD_INVALID_REGION_HANDLE)) {
				/* region is head, re-flip if necessary*/
				i4Ignore = OSD_PLA_Reflip(u4RgnList);
			} else if ((i4RgnNext != OSD_INVALID_REGION_HANDLE)
				   && (i4RgnPrev != OSD_INVALID_REGION_HANDLE)) {
				i4Ignore = _OSD_RGN_GetAddress((__u32)i4RgnNext, (unsigned long*)&u4Address);
				i4Ignore = _OSD_RGN_SetNextRegion((__u32)i4RgnPrev, u4Address);
			} else {
				/* should flip to NONE*/
				i4Ignore = OSD_PLA_Reflip(u4RgnList);
				/*
				(i4RgnNext == OSD_INVALID_REGION_HANDLE) && \
				(i4RgnPrev == OSD_INVALID_REGION_HANDLE)
				*/
			}

			_rAllRgnNode[u4Region].i4Next = OSD_INVALID_REGION_HANDLE;
			_rAllRgnNode[u4Region].i4Prev = OSD_INVALID_REGION_HANDLE;
			prRgnList->i4Count--;

			if (prRgnList->i4Count == 0) {
				prRgnList->i4Head = OSD_INVALID_REGION_HANDLE;
				prRgnList->i4Tail = OSD_INVALID_REGION_HANDLE;
			}

			return (__s32)OSD_RET_OK;
		}else {
			i4RgnCurr = _rAllRgnNode[i4RgnCurr].i4Next;
        }
	}

	UNUSED(i4Ignore);
	return (__s32)OSD_RET_INV_REGION;
#else
	return -(__s32)OSD_RET_INV_ARG;
#endif
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_SetDisplayWidth(__u32 u4Region, __u32 u4Width)
{
	__u32 u4SrcW;
	__u32 u4Step;
	__s32 i4Ignore;

	OSD_VERIFY_REGION(u4Region);

	if (u4Width == 0) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	i4Ignore = _OSD_RGN_GetInputWidth(u4Region, &u4SrcW);
	i4Ignore = _OSD_RGN_SetOutputWidth(u4Region, u4Width);
	u4Step = (u4SrcW == u4Width) ? 0x1000 : ((u4SrcW << 12) / u4Width);
	i4Ignore = _OSD_RGN_SetHStep(u4Region, (u4Step > 0xffff) ? 0xffff : u4Step);
	UNUSED(i4Ignore);

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_SetDisplayHeight(__u32 u4Region, __u32 u4Height)
{
	__u32 u4SrcH;
	__u32 u4Step;
	__s32 i4Ignore;

	OSD_VERIFY_REGION(u4Region);

	if (u4Height == 0) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	i4Ignore = _OSD_RGN_GetInputHeight(u4Region, &u4SrcH);
	i4Ignore = _OSD_RGN_SetOutputHeight(u4Region, u4Height);
	u4Step = (u4SrcH == u4Height) ? 0x1000 : ((u4SrcH << 12) / u4Height);
	i4Ignore = _OSD_RGN_SetVStep(u4Region, (u4Step > 0xffff) ? 0xffff : u4Step);
	UNUSED(i4Ignore);

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_SetBigEndian(__u32 u4Region, bool fgBE)
{
	OSD_VERIFY_REGION(u4Region);

	if (fgBE) {
		IGNORE_RET(_OSD_RGN_SetASel(u4Region, 0));
		IGNORE_RET(_OSD_RGN_SetYrSel(u4Region, 1));
		IGNORE_RET(_OSD_RGN_SetUgSel(u4Region, 2));
		IGNORE_RET(_OSD_RGN_SetVbSel(u4Region, 3));
	} else {
		IGNORE_RET(_OSD_RGN_SetASel(u4Region, 3));
		IGNORE_RET(_OSD_RGN_SetYrSel(u4Region, 2));
		IGNORE_RET(_OSD_RGN_SetUgSel(u4Region, 1));
		IGNORE_RET(_OSD_RGN_SetVbSel(u4Region, 0));
	}

	return (__s32)OSD_RET_OK;
}

#ifdef SUPPORT_RGB_YUV_FULL_RANGE_CONVER
__s32 OSD_RNG_Set_Xvycc_En(bool fgEnXvycc)
{

	if (!fgOsdInit()) {
		return (__s32)OSD_RET_OK;
	}

	_OSD_PLA_SetXVYCCEn(OSD_PLANE_IG, fgEnXvycc);
	_OSD_PLA_SetRgb2YcbrbEn(OSD_PLANE_IG, fgEnXvycc);
	_OSD_PLA_UpdateHwReg(OSD_PLANE_IG);

	_OSD_PLA_SetXVYCCEn(OSD_PLANE_PG, fgEnXvycc);
	_OSD_PLA_SetRgb2YcbrbEn(OSD_PLANE_PG, fgEnXvycc);
	_OSD_PLA_UpdateHwReg(OSD_PLANE_PG);
	return OSD_RET_OK;
}

__s32 OSD_RNG_List_Set_Xvycc_En(__u32 u4RgnList)
{
#if 1/*(!CONFIG_DRV_VERIFY_SUPPORT)*/
	__s32 i4Region;
	__s32 i4Ret;

	OSD_VERIFY_RGNLIST(u4RgnList);

	OSD_RGN_LIST_Get(u4RgnList, (__s32)OSD_RGN_LIST_HEAD, (__u32 *)&i4Region);

	while (i4Region != OSD_INVALID_REGION_HANDLE) {
		__s32 i4Next;

		IGNORE_RET(OSD_RGN_Set(i4Region, (__s32)OSD_RNG_XVYCC_EN, _fgXvYccEn));

		i4Ret = OSD_RGN_Get(i4Region, OSD_RGN_NEXT, (__u32 *)&i4Next);

		if (i4Ret != OSD_RET_OK) {
			return -OSD_RET_INV_LIST;
		}

		i4Region = i4Next;
	}

	return OSD_RET_OK;
#else
	return OSD_RET_INV_LIST;
#endif
}

#endif

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_RGN_Dump(__u32 u4Region)
{
#ifdef CC_CLI
	__u32 u4Prev, u4Next, u4Flag, u4PosX, u4PosY;
	__u32 u4BmpW, u4BmpH, u4DispW, u4DispH, u4ColorMode;
	__u32 u4Alpha, u4BmpAddr, u4BmpPitch, u4ClipV, u4ClipH;
	__u32 u4PalLoad, u4PalAddr, u4PalLen, u4StepV, u4StepH;
	__u32 u4ColorKey, u4ColorKeyEn, u4MixSel;
	__u32 u4VFlip, u4HMirror;


	OSD_VERIFY_REGION(u4Region);

	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_PREV, &u4Prev));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_NEXT, &u4Next));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_FLAGS, &u4Flag));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_POS_X, &u4PosX));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_POS_Y, &u4PosY));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_BMP_W, &u4BmpW));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_BMP_H, &u4BmpH));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_DISP_W, &u4DispW));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_DISP_H, &u4DispH));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_COLORMODE, &u4ColorMode));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_ALPHA, &u4Alpha));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_BMP_ADDR, &u4BmpAddr));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_BMP_PITCH, &u4BmpPitch));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_CLIP_V, &u4ClipV));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_CLIP_H, &u4ClipH));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_V_FLIP, &u4VFlip));  /* ???*/
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_H_MIRROR, &u4HMirror));  /* ???*/
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_PAL_LOAD, &u4PalLoad));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_PAL_ADDR, &u4PalAddr));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_PAL_LEN, &u4PalLen));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_STEP_V, &u4StepV));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_STEP_H, &u4StepH));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_COLOR_KEY, &u4ColorKey));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_COLOR_KEY_EN, &u4ColorKeyEn));
	IGNORE_RET(OSD_RGN_Get(u4Region, (__s32)OSD_RGN_MIX_SEL, &u4MixSel));

	FB_PRINT(FB_LOG_LVL_INFO, "REGION(%d)\n", u4Region);
	FB_PRINT(FB_LOG_LVL_INFO, "\tp[%d] n[%d] fg[%d] pos[%d,%d] src_wh[%d,%d] dst_wh[%d,%d]\n",
	    u4Prev, u4Next, u4Flag, u4PosX, u4PosY, u4BmpW, u4BmpH, u4DispW,
	    u4DispH);
	FB_PRINT(FB_LOG_LVL_INFO, "\tcm[%d] afa[%d] bmp[0x%08x] pitch[0x%x] cliph[%d] clipv[%d]\n",
	    u4ColorMode, u4Alpha, u4BmpAddr, u4BmpPitch, u4ClipH, u4ClipV);
	FB_PRINT(FB_LOG_LVL_INFO, "\tvflip[%d] hmirror[%d]\n", u4VFlip, u4HMirror);
	FB_PRINT(FB_LOG_LVL_INFO, "\tpal_load[%d] pal_addr[0x%08x] pal_len[%d] steph[0x%04x]\n",
	    u4PalLoad, u4PalAddr, u4PalLen, u4StepH);
	FB_PRINT(FB_LOG_LVL_INFO, "\tstepv[0x%04x] key[0x%08x] key_en[%d] blend_mode[%d]\n",
	    u4StepV, u4ColorKey, u4ColorKeyEn, u4MixSel);
#endif
	return (__s32)OSD_RET_OK;
}


/*Decide use DRAM channel 1 or 2 according to the colormode*/
__s32 OSD_DRV_GetMemChannel(OSD_COLOR_MODE_T eClrMode)
{
	switch (eClrMode) {
	/*Pg/UI's Region. We allocate it's surface in the memory pool.*/
	case OSD_CM_YCBCR_CLUT2:
	case OSD_CM_YCBCR_CLUT4:
	case OSD_CM_YCBCR_CLUT8:
	case OSD_CM_RGB_CLUT2:
	case  OSD_CM_RGB_CLUT4:
	case  OSD_CM_RGB_CLUT8:
		return OSDDARMCHANNEL2;

	default:
		/*The IG and PG use the original memory allocation method.*/
		return OSDDARMCHANNEL1;
	}
}

__u32 u4Region[8] = {
	0xe4000000, 0xff000000, 0xe4000000, 0x60000000,
	0xa01e02d0, 0x10001000, 0x000001e0, 0x500002d0
};/**/
/**/
#if 0
void OSD_PLAN_Set_DataAddr(__u32 u4DataPA)
{
	void *pvOSDFastRegionFree = NULL;
	void *pvOSDFastRegionAlign = NULL;
	__u32 pa, paAlign;
	__u32 size;

	size = 32 + 15;
	pvOSDFastRegionFree = AllocPhysMem(size, PAGE_READWRITE, 0xF, 0, &pa);
	pvOSDFastRegionAlign = (void *)((((__u32)pvOSDFastRegionFree) + 15) & (~15));
	paAlign  = (pa + 15) & (~15);

	u4Region[1] = ((u4DataPA & 0xffffff0) >> 4) | 0xff000000;
	u4Region[7] = ((u4DataPA & 0x30000000) >> 5) | u4Region[7];
	memcpy(pvOSDFastRegionAlign, (void *)u4Region, 32);

	/*flip and enable the IG plane*/
	IO_WRITE32(OSD_BASE_REG, 0x104, (paAlign & 0x3fffffff) >> 4);
	IO_WRITE32(OSD_BASE_REG, 0x100, 0xFF3CA081);

	/*trigger shadow register*/
	IO_WRITE32(OSD_BASE_REG, 0x0, 0x3);
}
#endif



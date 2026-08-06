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
#include <linux/mm.h>
#include <asm/io.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/display_inc.h>
#include "x_debug.h"
#else
#include "display_inc.h"
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"
/*#include "osd_if.h"*/
/*#include "x_hal_1176.h"*/
#include "chip_ver.h"

#define DEFINE_IS_LOG   OSD_IsLog

/*#include "x_mmap.h"*/

#include "sys_config.h"
#include "osd_map.h"
#include "log.h"

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

#define OSD_RGN_ALLOCATED 1


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

#define OSD_REGION_FLUSH(X)
/*#define OSD_REGION_FLUSH(X)      HalFlushInvalidateDCache()*/
/*#define OSD_REGION_FLUSH(X)      (BSP_FlushDCacheRange((__u32)&prRgn[u4Region].rField \
	,(__u32)sizeof(OSD_RGN_UNION_T)))*/

/*lint -save -e960 */
#define OSD_REGION_SETGET_TMPL(NAME, FIELD) \
	__s32 _OSD_RGN_Set##NAME(__u32 u4Region, __u32 u4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region); \
		prRgn[u4Region].rField.FIELD = u4Value;          \
		OSD_REGION_FLUSH(u4Region); \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_RGN_Get##NAME(__u32 u4Region, __u32 *pu4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region);   \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = prRgn[u4Region].rField.FIELD; \
		return (__s32)OSD_RET_OK; \
	} \

/* above line is intendedly left blanc */
/*lint -restore */

#ifndef __ARM2__
/*lint -save -e960 */
#define OSD_REGION_SETGET_ADDRESS_TMPL(NAME, FIELD)  \
	__s32 _OSD_RGN_Set##NAME(__u32 u4Region, __u32 u4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region); \
		prRgn[u4Region].rField.FIELD = (u4Value == 0) ? 0 : __pa(u4Value) >> 4; \
		OSD_REGION_FLUSH(u4Region); \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_RGN_Get##NAME(__u32 u4Region, __u32 *pu4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = (__u32)__va(prRgn[u4Region].rField.FIELD << 4); \
		return (__s32)OSD_RET_OK; \
	} \

/* above line is intendedly left blanc */
/*lint -restore */
#else
#define OSD_REGION_SETGET_ADDRESS_TMPL(NAME, FIELD)  \
	__s32 _OSD_RGN_Set##NAME(__u32 u4Region, __u32 u4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region); \
		prRgn[u4Region].rField.FIELD = (u4Value == 0) ? 0 : VA_TO_PA_DAL(u4Value) >> 4; \
		OSD_REGION_FLUSH(u4Region); \
		return (__s32)OSD_RET_OK; \
	} \
	__s32 _OSD_RGN_Get##NAME(__u32 u4Region, __u32 *pu4Value) \
	{ \
		OSD_RGN_UNION_T *prRgn = NULL; \
		OSD_VERIFY_REGION(u4Region); \
		prRgn = OSD_RGN_GetAdd(u4Region); \
		if (pu4Value == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
		*pu4Value = (__u32)PA_TO_VA_DAL(prRgn[u4Region].rField.FIELD << 4); \
		return (__s32)OSD_RET_OK; \
	} \

#endif

#ifdef OSD_MEM_POOL_STATIC
#define OSD_BITMAP_MEM_POOL_SIZE  (42 * 1024 * 1024)
DEFINE_CHANNEL2_MEMORY_AREA(_pbOsdBitmapMemPool, __u8, OSD_BITMAP_MEM_POOL_SIZE, 1024)
bool fgOsdBitmapMemPoolNeedInit = TRUE;
HANDLE_T HOsdMemPool;
#endif
/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

/*static __u8 _au1OsdRegion[(OSD_MAX_NUM_RGN * sizeof(OSD_RGN_UNION_T)) + 15];*/
static __u32 _au4OsdRegionStatus[OSD_MAX_NUM_RGN + 1];
static OSD_RGN_UNION_T *_prOsdRegionReg1;

/*Memory Pool in DRAM Channel 2*/
static bool _fgRgnMemNeedInit = TRUE;
/*static  __u8 *_pu1OsdRgnMem2Raw;*/
static OSD_RGN_UNION_T *_prOsdRegionReg2;



OSD_RGN_UNION_T *OSD_RGN_GetAdd(__u32 u4Region)
{
	if (OSD_RGN_IS_IN_CHANNEL1(u4Region)) {
		return _prOsdRegionReg1;
	} else if (OSD_RGN_IS_IN_CHANNEL2(u4Region)) {
		return _prOsdRegionReg2;
	} else {
		return _prOsdRegionReg1;
	}
}
/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
void _OSD_RGN_UninitApi(void)
{
	/*free for uninit.//added by msz00441*/
	if (_fgRgnMemNeedInit == FALSE) {
		/*wts kfree(_prOsdRegionReg1Free);*/
		/*wts kfree(_pu1OsdRgnMem2Raw);*/
		_prOsdRegionReg2 = NULL;
		_fgRgnMemNeedInit = TRUE;
	}
}

void _OSD_RGN_InitApi(void)
{
	__s32 i4Unused;
	__u32 u4PhyAddr;

#ifdef OSD_MEM_POOL_STATIC
	__s32 i4Ret;
#endif

	/*added by msz00420*/
	/*Init the memory pool if it's the 1st time.*/
	if (_fgRgnMemNeedInit) {
#ifndef __ARM2__
		u4PhyAddr = DAL_OSD_REGION_PHY_BASE;
		_prOsdRegionReg1 = ioremap_nocache(u4PhyAddr, sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN);
#else
		_prOsdRegionReg1 =  x_mem_alloc_ret_phy_addr(sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN, &u4PhyAddr);
#endif
		AddPaVatoMapTableDal(u4PhyAddr, (__u32)_prOsdRegionReg1, sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN);

		/* _prOsdRegionReg1 = (OSD_RGN_UNION_T*)((__u32)(_au1OsdRegion + 0xf) & ~0xf);*/
		i4Unused = (__s32)x_memset(_prOsdRegionReg1, 0, sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN);
		i4Unused = (__s32)x_memset(_au4OsdRegionStatus, 0, sizeof(__u32) * (OSD_MAX_NUM_RGN + 1));
		/* UNUSED(_au1OsdRegion);*/
		UNUSED(i4Unused);
#ifdef OSD_MEM_POOL_STATIC

		if (fgOsdBitmapMemPoolNeedInit) {
			i4Ret = x_mem_part_create(&HOsdMemPool, OSD_BITMAP_MEM_POOL_NAME,
						  (void *)_pbOsdBitmapMemPool, OSD_BITMAP_MEM_POOL_SIZE, 0);

			if (i4Ret != OSR_OK) {
				FB_PRINT(FB_LOG_LVL_ERR, "OSD", "%s %d  x_mem_part_create failed : %d\r\n", __func__
					, __LINE__, i4Ret);

				return;
			}

			fgOsdBitmapMemPoolNeedInit = FALSE;
		}

		_pu1OsdRgnMem2Raw = x_mem_part_alloc(HOsdMemPool,
						     sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN + 16);
#else
		/*wts _pu1OsdRgnMem2Raw = x_mem_ch2_alloc(sizeof(OSD_RGN_UNION_T) *OSD_MAX_NUM_RGN+ 16);*/
#endif

		/*if (_pu1OsdRgnMem2Raw == NULL) {
			FB_PRINT(FB_LOG_LVL_ERR, "OSD", "%s %d  x_mem_part_alloc failed\r\n", __func__, __LINE__);
			return;
		}

		_prOsdRegionReg2 = (OSD_RGN_UNION_T *)(((__u32) _pu1OsdRgnMem2Raw + 15) & (~0xf));
		x_memset(_prOsdRegionReg2, 0, sizeof(OSD_RGN_UNION_T) * OSD_MAX_NUM_RGN); */
		_fgRgnMemNeedInit = FALSE;
	}

}

OSD_REGION_SETGET_ADDRESS_TMPL(NextRegion, u4NextOsdAddr)
OSD_REGION_SETGET_TMPL(FifoEx, fgFifoEx)
OSD_REGION_SETGET_TMPL(NextEnable, fgNextOsdEn)
OSD_REGION_SETGET_TMPL(Alpha, u4MixWeight)
OSD_REGION_SETGET_TMPL(HClip, u4HClip)
OSD_REGION_SETGET_TMPL(VClip, u4VClip)
OSD_REGION_SETGET_TMPL(LineSize8, fgLineSize8)
OSD_REGION_SETGET_TMPL(VbSel, u4VbSel)
OSD_REGION_SETGET_TMPL(UgSel, u4UgSel)
OSD_REGION_SETGET_TMPL(YrSel, u4YrSel)
OSD_REGION_SETGET_TMPL(ASel, u4AlphaSel)
OSD_REGION_SETGET_ADDRESS_TMPL(PaletteAddr, u4PaletteAddr)
OSD_REGION_SETGET_TMPL(PaletteLen, u4PaletteLen)
OSD_REGION_SETGET_TMPL(LoadPalette, fgNewPalette)
OSD_REGION_SETGET_TMPL(InputWidth, u4Ihw)
OSD_REGION_SETGET_TMPL(InputHeight, u4Ivw)
OSD_REGION_SETGET_TMPL(LineSize, u4LineSize)
OSD_REGION_SETGET_TMPL(HStep, u4HStep)
OSD_REGION_SETGET_TMPL(VStep, u4VStep)
OSD_REGION_SETGET_TMPL(OutputHeight, u4Ovw)
OSD_REGION_SETGET_TMPL(OutputPosY, u4Ovs)
OSD_REGION_SETGET_TMPL(OutputWidth, u4Ohw)
OSD_REGION_SETGET_TMPL(OutputPosX, u4Ohs)
OSD_REGION_SETGET_TMPL(DataAddrHI, u4DataAddrHI)
OSD_REGION_SETGET_TMPL(ColorKeyEnable, fgColorKeyEn)
OSD_REGION_SETGET_TMPL(MixSel, u4MixSel)
OSD_REGION_SETGET_TMPL(FrameMode, fgAcsFrame)
OSD_REGION_SETGET_TMPL(AutoMode, fgAcsAuto)
OSD_REGION_SETGET_TMPL(TopField, fgAcsTop)
OSD_REGION_SETGET_TMPL(BlendMode, u4MixSel)

OSD_REGION_SETGET_TMPL(DeCompEn, fgDeCompEn)
OSD_REGION_SETGET_TMPL(DeCompLineBased, fgDeCompLineBased)

OSD_REGION_SETGET_TMPL(SelectByteEn, fgSelectByteEn)


OSD_REGION_SETGET_TMPL(DeCompMode, u4DeCompMode)
OSD_REGION_SETGET_TMPL(WTEn, fgWTEn)


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_SetColorMode(__u32 u4Region, __u32 u4Value)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);

	if ((u4Value == 0) ||
	    (u4Value == 1) ||
	    (u4Value == 2) ||
	    (u4Value == 8) ||
	    (u4Value == 9) ||
	    (u4Value == 10)) {
		IGNORE_RET(_OSD_RGN_SetFifoEx(u4Region, 0));
	} else {
		IGNORE_RET(_OSD_RGN_SetFifoEx(u4Region, 1));
	}

	prRgn = OSD_RGN_GetAdd(u4Region);

	prRgn[u4Region].rField.u4ColorMode = u4Value;
	OSD_REGION_FLUSH(u4Region);
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetColorMode(__u32 u4Region, __u32 *pu4Value)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);

	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	prRgn = OSD_RGN_GetAdd(u4Region);
	*pu4Value = prRgn[u4Region].rField.u4ColorMode;
	/*pu4Value = _prOsdRegionReg[u4Region].rField.u4ColorMode;*/

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_SetColorKey(__u32 u4Region, __u32 u4Value)
{
	OSD_RGN_UNION_T *prRgn;
	__u32 u4ColorMode;

	OSD_VERIFY_REGION(u4Region);

	prRgn = OSD_RGN_GetAdd(u4Region);
	u4ColorMode = prRgn[u4Region].rField.u4ColorMode;

	if (((u4ColorMode >= (__u32)OSD_CM_CBYCRY422_DIRECT16) &&
	     (u4ColorMode <= (__u32)OSD_CM_AYCBCR8888_DIRECT32)) ||
	    ((u4ColorMode >= (__u32)OSD_CM_RGB565_DIRECT16) &&
	     (u4ColorMode <= (__u32)OSD_CM_ARGB8888_DIRECT32))) {
		/*_prOsdRegionReg[u4Region].rField.u4PaletteAddr = u4Value >> 8;*/
		prRgn[u4Region].rField.u4PaletteAddr = u4Value >> 8;
	}

	/*_prOsdRegionReg[u4Region].rField.u4ColorKey = u4Value & 0xff;*/
	prRgn[u4Region].rField.u4ColorKey = u4Value & 0xff;
	OSD_REGION_FLUSH(u4Region);
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetColorKey(__u32 u4Region, __u32 *pu4Value)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);

	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	prRgn = OSD_RGN_GetAdd(u4Region);
	/*pu4Value = (_prOsdRegionReg[u4Region].rField.u4PaletteAddr << 8) |
		_prOsdRegionReg[u4Region].rField.u4ColorKey;*/
	*pu4Value = (prRgn[u4Region].rField.u4PaletteAddr << 8) |
		    prRgn[u4Region].rField.u4ColorKey;
	return (__s32)OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_SetDataAddr(__u32 u4Region, __u32 u4Value)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD", "_OSD_RGN_SetDataAddr begin\n");
#endif
	prRgn = OSD_RGN_GetAdd(u4Region);

	prRgn[u4Region].rField.u4DataAddr   = (u4Value) >> 4;
	prRgn[u4Region].rField.u4DataAddrHI = (u4Value) >> 28;

	OSD_REGION_FLUSH(u4Region);
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD", "_OSD_RGN_SetDataAddr end:u4DataAddr=%x\n",  prRgn[u4Region].rField.u4DataAddr);
#endif
	return (__s32)OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetDataAddr(__u32 u4Region, __u32 *pu4Value)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);

	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	prRgn = OSD_RGN_GetAdd(u4Region);

	*pu4Value  = (prRgn[u4Region].rField.u4DataAddr << 4);
	*pu4Value |= (prRgn[u4Region].rField.u4DataAddrHI << 28);

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetHandle(__u32 u4Addr, __u32 *pu4Region)
{
	OSD_RGN_UNION_T *prRgn1;
	OSD_RGN_UNION_T *prRgn2;

	if (pu4Region == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	prRgn1 = OSD_RGN_GetAdd(OSD_RGN_INDARMCHNANNEL1_START);
	prRgn2 = OSD_RGN_GetAdd(OSD_RGN_INDARMCHNANNEL2_START);

	if ((u4Addr < (__u32)prRgn1) ||
	    (u4Addr >= ((OSD_MAX_NUM_RGN * sizeof(OSD_RGN_UNION_T)) +
			(__u32)prRgn1))) {
		if ((u4Addr < (__u32)prRgn2) ||
		    (u4Addr >= ((OSD_MAX_NUM_RGN * sizeof(OSD_RGN_UNION_T)) + (__u32)prRgn2))) {
			return -(__s32)OSD_RET_UNDEF_ERR;
		}
		*pu4Region = (u4Addr - (__u32)prRgn2) / sizeof(OSD_RGN_UNION_T);
		*pu4Region += OSD_RGN_INDARMCHNANNEL2_START;
	} else {
		*pu4Region = (u4Addr - (__u32)prRgn1) / sizeof(OSD_RGN_UNION_T);
		*pu4Region += OSD_RGN_INDARMCHNANNEL1_START;
	}


	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetAddress(__u32 u4Region , __u32 *pu4Addr)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);

	if (pu4Addr == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	prRgn = OSD_RGN_GetAdd(u4Region);

	/*pu4Addr = (__u32)&prRgn[u4Region];*/
	*pu4Addr = (__u32)&prRgn[u4Region];

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_Alloc(__u32 *pu4Region, __s32 i4MemChannel)
{
	INT16 i2RgnStart, i2RgnEnd;
	__s32 i4Rgn;

	if (pu4Region == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	switch (i4MemChannel) {
	case OSDDARMCHANNEL1:
		i2RgnStart = 0;
		i2RgnEnd = OSD_RGN_INDARMCHNANNEL1_START + OSD_RGN_NUM_PER_MEMCHN;
		break;

	case  OSDDARMCHANNEL2:
		i2RgnStart = OSD_RGN_INDARMCHNANNEL2_START;
		i2RgnEnd = OSD_RGN_INDARMCHNANNEL2_START + OSD_RGN_NUM_PER_MEMCHN;
		break;

	default:
		i2RgnStart = 0;
		i2RgnEnd = OSD_RGN_INDARMCHNANNEL1_START + OSD_RGN_NUM_PER_MEMCHN;
		break;
	}

#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD", "_au4OsdRegionStatus[0]=%d, _au4OsdRegionStatus[1]=%d, _au4OsdRegionStatus[2]=%d\n",
		 _au4OsdRegionStatus[0], _au4OsdRegionStatus[1], _au4OsdRegionStatus[2]);
#endif

	for (i4Rgn = i2RgnStart; i4Rgn < i2RgnEnd; i4Rgn++) {
		if ((_au4OsdRegionStatus[i4Rgn] & OSD_RGN_ALLOCATED) == 0) {
			*pu4Region = (__u32)i4Rgn;
			_au4OsdRegionStatus[i4Rgn] |= OSD_RGN_ALLOCATED;
#ifdef FB_DEBUG
			FB_PRINT(FB_LOG_LVL_DBG, "OSD", "_OSD_RGN_Alloc:*pu4Region=%d\n", *pu4Region);
#endif
			return (__s32)OSD_RET_OK;
		}
	}

	return -(__s32)OSD_RET_OUT_OF_REGION;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_Free(__u32 u4Region)
{
	OSD_RGN_UNION_T *prRgn;

	OSD_VERIFY_REGION(u4Region);
	prRgn = OSD_RGN_GetAdd(u4Region);

	_au4OsdRegionStatus[u4Region] &= ~OSD_RGN_ALLOCATED;
	prRgn[u4Region].rField.fgNextOsdEn = FALSE;
	return (__s32)OSD_RET_OK;
}


#if 0
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_GetAlloc(__u32 u4Region, __u32 *pfgStatus)
{
	if (pfgStatus == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	*pfgStatus = _au4OsdRegionStatus[u4Region];
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_SetAlloc(__u32 u4Region, __u32 fgStatus)
{
	_au4OsdRegionStatus[u4Region] = fgStatus;
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_FreeList(__u32 u4List)
{
	__s32 i4Count;
	__u32 u4NextAddr;
	__u32 fgNextEnable;

	i4Count = 0;

	while (_OSD_RGN_Free(u4List) == (__s32)OSD_RET_OK) {
		IGNORE_RET(_OSD_RGN_GetNextEnable(u4List, &fgNextEnable));

		if (fgNextEnable == 0) {
			/* no next region*/
			/* just return*/
			return (__s32)OSD_RET_OK;
		}

		IGNORE_RET(_OSD_RGN_GetNextRegion(u4List, &u4NextAddr));
		IGNORE_RET(_OSD_RGN_GetHandle(u4NextAddr, &u4List));
		i4Count++;

		if (i4Count >= OSD_MAX_NUM_RGN) {
			return -(__s32)OSD_RET_INV_REGION;
		}
	}

	return -(__s32)OSD_RET_UNDEF_ERR;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 _OSD_RGN_AttachTail(__u32 u4List, __s32 i4Attachment)
{
	__u32 u4AttachAddr;

	OSD_VERIFY_REGION(u4List);
	OSD_VERIFY_REGION(i4Attachment);

	VERIFY((__s32)OSD_RET_OK ==
	       _OSD_RGN_GetAddress((__u32)i4Attachment, &u4AttachAddr));

	return (__s32)OSD_RET_OK;
}
#endif

__s32 _OSD_RGN_SetPalettePA(__u32 u4Region, __u32 u4Value)
{
	OSD_RGN_UNION_T *prRgn = NULL;

	OSD_VERIFY_REGION(u4Region);
	prRgn = OSD_RGN_GetAdd(u4Region);
	prRgn[u4Region].rField.u4PaletteAddr = u4Value >> 4;
	OSD_REGION_FLUSH(u4Region);
	return (__s32)OSD_RET_OK;
}
__s32 _OSD_RGN_GetPalettePA(__u32 u4Region, __u32 *pu4Value)
{
	OSD_RGN_UNION_T *prRgn = NULL;

	OSD_VERIFY_REGION(u4Region);
	prRgn = OSD_RGN_GetAdd(u4Region);

	if (pu4Value == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	*pu4Value = (__u32)prRgn[u4Region].rField.u4PaletteAddr << 4;
	return (__s32)OSD_RET_OK;
}





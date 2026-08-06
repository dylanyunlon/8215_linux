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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock_types.h>
#include <media/atc/drv_osd_if.h>
//#include <media/atc/drv_av_d.h>
#include "windows.h"
#include "x_debug.h"
#include "x_ioopt.h"
#else
#include "assert.h"
#include "drv_osd_if.h"
//#include "drv_av_d.h"
#endif

#include "chip_ver.h"
#include "osd_hw.h"
#include "drv_config.h"
#include "osd_inc.h"
#include "log.h"
/*#include "osd_if.h"*/

#define DEFINE_IS_LOG   OSD_IsLog

#ifndef __ARM2__
static DEFINE_SPINLOCK(ac83xx_osd_plane_lock);
#endif
/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

/* delay to avoid temporal status when switching display mode */
#define MUTE_VSYNC 10

#define RET_ON_FAIL(FUNC)                                              \
	do {                                                           \
		__s32 i4Ret = (__s32)(FUNC);                           \
		if (i4Ret != (__s32)OSD_RET_OK) {                      \
			return i4Ret;                                  \
		}                                                      \
	} while (0)


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

typedef struct _OSD_PLANE_INFO_T {
	__u32 u4FirstRegionAddr;
	__s32 fgPlaneEnable;
	__s32 fgGlobeEnable;
	__s32 i4FlipedList;
} OSD_PLANE_INFO_T;


/*added by msz00420, 0801191506*/
typedef struct _OSD_PLANE_FLIP_FLAG_T {
	__u32 u4List;
#ifndef __ARM2__
	struct semaphore hSemPlane;
	struct semaphore hSemPLAEnable;
#endif
	bool fgNeedFlip;
	bool fgValidReg;
	bool fgNeedEn;
	bool fgEnable;
	__u32 addr; /*add by mtk94020*/
} OSD_PLANE_FLIP_FLAG_T;

/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

__s32 _rOsdCurRgn[OSD_PLANE_MAX_NUM];
__s32 _i4Region_ScrnSvr;
void *_pvScrnSvr_Bitmap;
OSD_SCRN_FLAG _rOsdScrnFlag;

static OSD_PLANE_INFO_T _arOsdPlaneInfo[OSD_PLANE_MAX_NUM];
static __u32 u4MuteCounter;

/*added by msz00420, 0801191506*/
static volatile OSD_PLANE_FLIP_FLAG_T _arOsdPlaneFlipFlag[OSD_PLANE_MAX_NUM];
static volatile OSD_PLANE_FLIP_FLAG_T _arOsdPlaneFlipFlagNowait[OSD_PLANE_MAX_NUM];

#if (OSD_FLIP_SCHEME == 1)
static bool _fgOsdPlaneInit = FALSE;
#endif
static bool _fgOsdYCbCr709 = TRUE;

__s32 _i4AllDramRdTime = 0;
__s32 _i4AllDramWtTime = 0;
__s32 _i4FlipTimes = 0;


static volatile OSD_PLANE_FLIP_FLAG_T _arOsdPlaneFlipFlag[OSD_PLANE_MAX_NUM];
static volatile OSD_PLANE_FLIP_FLAG_T _arOsdPlaneFlipFlagNowait[OSD_PLANE_MAX_NUM];

void OSD_Flip_Flag_Init(void)
{
	INT16 i2Val;

	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		_arOsdPlaneFlipFlag[i2Val].fgEnable = FALSE;
		_arOsdPlaneFlipFlag[i2Val].fgNeedEn = FALSE;
		_arOsdPlaneFlipFlag[i2Val].fgNeedFlip = FALSE;
		_arOsdPlaneFlipFlag[i2Val].fgValidReg = FALSE;
#ifndef __ARM2__
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlag[i2Val].hSemPlane, 1);
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlag[i2Val].hSemPLAEnable, 1);
#endif
		_arOsdPlaneFlipFlag[i2Val].u4List = 0xFFFFFFFF;
		/*add by mtk94020*/
		_arOsdPlaneFlipFlag[i2Val].addr = 0xFFFFFFFF;

		_arOsdPlaneFlipFlagNowait[i2Val].fgEnable = FALSE;
		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedEn = FALSE;
		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedFlip = FALSE;
		_arOsdPlaneFlipFlagNowait[i2Val].fgValidReg = FALSE;
#ifndef __ARM2__
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlagNowait[i2Val].hSemPLAEnable, 1);
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlagNowait[i2Val].hSemPlane, 1);
#endif
		_arOsdPlaneFlipFlagNowait[i2Val].u4List = 0xFFFFFFFF;
	}
}

void OSD_FreeBitmap(void *pvBitmap)
{
#ifndef __ARM2__
	kfree(pvBitmap);
#endif
}

bool OSD_Is_PLA_Enabled(__u32 u4Plane)
{
	__u32 u4CoreRg_00;

	switch (u4Plane) {
	case OSD_PLANE_1:
		u4CoreRg_00 = IO_READ32(osdf_reg, 0x100);
		break;

	case OSD_PLANE_2:
		u4CoreRg_00 = IO_READ32(osdf_reg, 0x200);
		break;

	case OSD_PLANE_3:
		u4CoreRg_00 = IO_READ32(osdf_reg, 0x300);
		break;

	case OSD_PLANE_4:
		u4CoreRg_00 = IO_READ32(osdf_reg, 0xa00);
		break;

	case OSD_PLANE_5:
		u4CoreRg_00 = IO_READ32(osdf_reg, 0xb00);
		break;

	default:
		u4CoreRg_00 = 0;
	}

	if (0 == (u4CoreRg_00 & 1)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

__s32 OSD_SAVE_Cur_Rgn(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

	FB_PRINT(FB_LOG_LVL_INFO, "[OSD_SAVE_Cur_Rgn]: u4FirstRgn is %d, i4FlipedList is %d\n",
		 _rOsdCurRgn[u4Plane], _arOsdPlaneInfo[u4Plane].i4FlipedList);

	if (OSD_Is_PLA_Enabled(u4Plane)) {
		OSD_PLA_GetFirstRegion(u4Plane, (__u32 *) &(_rOsdCurRgn[u4Plane]));

		FB_PRINT(FB_LOG_LVL_INFO, "[OSD_SAVE_Cur_Rgn]: u4FirstRgn is %d\n", _rOsdCurRgn[u4Plane]);
	} else {
		OSD_PLA_Enable(u4Plane, TRUE);
		_rOsdCurRgn[u4Plane] = -1;
	}

	return 0;
}

__s32 OSD_Restore_Rgn(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

	FB_PRINT(FB_LOG_LVL_INFO, "[OSD_Restore_Rgn]: u4FirstRgn is %d, i4FlipedList is %d\n",
		 _rOsdCurRgn[u4Plane], _arOsdPlaneInfo[u4Plane].i4FlipedList);

	OSD_RGN_Detach(_i4Region_ScrnSvr, _arOsdPlaneInfo[u4Plane].i4FlipedList);
	OSD_FreeBitmap(_pvScrnSvr_Bitmap);

	if (-1 == _rOsdCurRgn[u4Plane]) {
		OSD_PLA_Enable(u4Plane, FALSE);
	} else {
		OSD_RGN_Insert((__u32)_rOsdCurRgn[u4Plane], (__u32)_arOsdPlaneInfo[u4Plane].i4FlipedList);
		OSD_PLA_FlipTo(u4Plane, (__u32)_arOsdPlaneInfo[u4Plane].i4FlipedList);
		_rOsdCurRgn[u4Plane] = -1;
	}

	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
static void _Osd_PustReset_Plane(__u32 u4Plane)
{
	__u32 u4Value;

	u4Value = IO_READ32(osdf_reg, 4);

	switch (u4Plane) {
	case OSD_PLANE_1:
		u4Value &= 0xFFFF0030;
		break;

	case OSD_PLANE_2:
		u4Value &= 0xFFFF00c0;
		break;

	case OSD_PLANE_3:
		u4Value &= 0xFFFF0300;
		break;

	case OSD_PLANE_4:
		u4Value &= 0xFFFF0c00;
		break;

	case OSD_PLANE_5:
		u4Value &= 0xFFFF3000;
		break;

	default:
		u4Value |= 0xFFFF3FF0;
		break;
	}

	IO_WRITE32(osdf_reg, 4, u4Value);
}

static void _Osd_ReleaseReset_Plane(void)
{
	__u32 u4Value;

	u4Value = IO_READ32(osdf_reg, 4);
	u4Value &= 0xFFFF0000;
	IO_WRITE32(osdf_reg, 4, u4Value);
}

/*----------------------------------------------------------------------*/
/*function: soft reset osd (1~5)*/
/*param: u4Plane*/
/*return: 0*/
/*----------------------------------------------------------------------*/
__s32 Osd_PustReset_Plane(__u32 u4Plane)
{
	_Osd_PustReset_Plane(u4Plane);
	return OSD_RET_OK;
}

__s32 Osd_ReleaseReset_Plane(void)
{
	_Osd_ReleaseReset_Plane();
	return OSD_RET_OK;
}

static void _UpdatePlaneEnableState(__u32 u4Plane)
{
	__u32 u4Value;

	IGNORE_RET(_OSD_PLA_GetEnable(u4Plane, &u4Value));

	if (u4Value == 0) {
		/* previously disabled*/
		if (_arOsdPlaneInfo[u4Plane].fgGlobeEnable &&
		    _arOsdPlaneInfo[u4Plane].u4FirstRegionAddr &&
		    _arOsdPlaneInfo[u4Plane].fgPlaneEnable) {
			/* all flags on*/
			IGNORE_RET(_OSD_PLA_SetEnable(u4Plane, (__u32)TRUE));
		} else {
		}
	} else {
		/* previously enabled */
		if (!(_arOsdPlaneInfo[u4Plane].fgGlobeEnable &&
		      _arOsdPlaneInfo[u4Plane].u4FirstRegionAddr &&
		      _arOsdPlaneInfo[u4Plane].fgPlaneEnable)) {
			/* not "all flags on"*/
			_Osd_PustReset_Plane(u4Plane);
			IGNORE_RET(_OSD_PLA_SetEnable(u4Plane, (__u32)FALSE));
			_Osd_ReleaseReset_Plane();
		} else {
		}
	}

	/*VERIFY((__s32)OSD_RET_OK == _OSD_PLA_UpdateHwReg(u4Plane));*/
}


/*added by msz00420, 0801191506*/
__s32  _OSD_PLA_Create_Semaphores(void)
{
	INT16 i2Val;

#ifdef SUPPORT_FRAME_ACCURATE_
	{
		_rOsdSetPtsPalEntries.fgNeedSetPtsPalEntries = FALSE;
		_rOsdSetPtsPalEntries.u8Tickets = FALSE;
		_rOsdSetPtsPalEntries.u8Tickets = FALSE;
	}

	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		_arOsdPlanePtsFlipFlag[i2Val].fgNeedFlip = FALSE;

		_arOsdPlanePtsFlipFlag[i2Val].fgNeedFlip = FALSE;
	}

#endif

#ifndef __ARM2__
	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlag[i2Val].hSemPlane, 1);
		_arOsdPlaneFlipFlag[i2Val].fgNeedFlip = FALSE;

		sema_init((struct semaphore *)&_arOsdPlaneFlipFlag[i2Val].hSemPLAEnable, 1);
		_arOsdPlaneFlipFlag[i2Val].fgNeedFlip = FALSE;
	}

	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		sema_init((struct semaphore *)&_arOsdPlaneFlipFlagNowait[i2Val].hSemPlane, 1);
		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedFlip = FALSE;

		sema_init((struct semaphore *)&_arOsdPlaneFlipFlagNowait[i2Val].hSemPLAEnable, 1);
		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedFlip = FALSE;
	}
#else
	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		_arOsdPlaneFlipFlag[i2Val].fgNeedFlip = FALSE;

		_arOsdPlaneFlipFlag[i2Val].fgNeedFlip = FALSE;
	}

	for (i2Val = 0; i2Val < OSD_PLANE_MAX_NUM; i2Val++) {
		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedFlip = FALSE;

		_arOsdPlaneFlipFlagNowait[i2Val].fgNeedFlip = FALSE;
	}
#endif
	return OSD_RET_OK;
}
/*msz00441 delete sema for uninit 08-3-6*/
__s32  _OSD_PLA_Delete_Semaphores(void)
{
	return OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/

void OSD_PLA_Mute(void)
{
	__u32 u4Plane;

	u4MuteCounter = MUTE_VSYNC;
	_OSD_AlwaysUpdateReg(TRUE);

	for (u4Plane = 0; u4Plane < (__s32)OSD_PLANE_MAX_NUM; u4Plane++) {
		IGNORE_RET(OSD_PLA_SetGlobeEnable(u4Plane, FALSE));
	}

	_OSD_AlwaysUpdateReg(FALSE);
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/

void OSD_PLA_Unmute(void)
{
	__u32 u4Plane;

	if (u4MuteCounter > 0) {
		if (u4MuteCounter == 1) {
			for (u4Plane = 0; u4Plane < (__s32)OSD_PLANE_MAX_NUM; u4Plane++) {

				IGNORE_RET(OSD_PLA_SetGlobeEnable(u4Plane, TRUE));
			}
		}

		u4MuteCounter--;
	}
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_SetGlobeEnable(__u32 u4Plane, bool fgGlobeEnable)
{
	OSD_VERIFY_PLANE(u4Plane);
	_arOsdPlaneInfo[u4Plane].fgGlobeEnable = fgGlobeEnable;
	_UpdatePlaneEnableState(u4Plane);

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Init(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);
	IGNORE_RET(OSD_PLA_Reset(u4Plane));

	/*added by msz00420, 0801191506*/
	/*_OSD_PLA_Create_Semaphores();*/
#if (OSD_FLIP_SCHEME == 1)
	_fgOsdPlaneInit = TRUE;
#endif
	return (__s32)OSD_RET_OK;
}
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Uninit(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

#if (OSD_FLIP_SCHEME == 1)
	_fgOsdPlaneInit = TRUE;
#endif
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Reset(__u32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

	/* reset plane's hardware state*/
	IGNORE_RET(_OSD_PLA_SetReg(u4Plane, NULL));
	IGNORE_RET(_OSD_PLA_SetEnable(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetHFilter(u4Plane, 0));/*according Ted's advice 1->0*/
	IGNORE_RET(_OSD_PLA_SetBlending(u4Plane, 0xff));
	IGNORE_RET(_OSD_PLA_SetFading(u4Plane, 0xff));

	IGNORE_RET(_OSD_PLA_SetFakeHdr(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetPrngEn(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetAlphaZeroBlack(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetOutRngColorMode(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetColorExpSel(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetAlphaRatioEn(u4Plane, 0));

	IGNORE_RET(_OSD_PLA_SetContReqLmt(u4Plane, 0x07));
	IGNORE_RET(_OSD_PLA_SetFifoSize(u4Plane, 0x0f));
	IGNORE_RET(_OSD_PLA_SetPauseCnt(u4Plane, 0x0f));
	IGNORE_RET(_OSD_PLA_SetContReqLmt0(u4Plane, 0x07));
	IGNORE_RET(_OSD_PLA_SetBurstDis(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetRgbMode(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetVacancyThr(u4Plane, 0x03));


	/* reset plane's software state*/
	x_memset((void *)&_arOsdPlaneInfo[u4Plane], 0, sizeof(OSD_PLANE_INFO_T));

#if CONFIG_DRV_FAST_LOGO

	if (u4Plane == OSD_PLANE_1) {
		_arOsdPlaneInfo[u4Plane].fgGlobeEnable = TRUE;
	} else
#endif
		IGNORE_RET(OSD_PLA_SetGlobeEnable(u4Plane, (__s32)TRUE));

	/* global enable is set to true by default*/
	/*_UpdatePlaneEnableState(u4Plane);*/

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief: NOTE: it will also disable scaler when disable plane
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Enable(__u32 u4Plane, bool fgEnable)
{
	OSD_VERIFY_PLANE(u4Plane);
	_OSD_PLA_SetEnable(u4Plane, fgEnable);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_Enable(u4Plane));
	return (__s32)OSD_RET_OK;
}

void OSD_PLA_SetDestColorKey(bool fgEnable, __u32 u4ColorKey)
{
	_OSD_PLA_SetDestColorKey(fgEnable, u4ColorKey);
}
/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_FlipTo(__u32 u4Plane, __u32 u4List)
{
	__s32 i4FirstRegion;
	unsigned long u4FirstRegionAddr;
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD_PLA_FlipTo begin\n");
#endif
	OSD_VERIFY_PLANE(u4Plane);
	OSD_VERIFY_RGNLIST(u4List);

	VERIFY((__s32)OSD_RET_OK == OSD_RGN_LIST_GetHead(u4List, &i4FirstRegion));
	/*OSD_VERIFY_REGION(i4FirstRegion);*/
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "i4FirstRegion=%d\n", i4FirstRegion);
#endif

	if ((__u32)i4FirstRegion >= (__u32)OSD_MAX_NUM_RGN) {
		IGNORE_RET(OSD_PLA_FlipToNone(u4Plane));
		return (__s32)OSD_RET_OK;
	}

#if OSD_COLORSPACE_ADJUST

	if (_fgOsdYCbCr709) {
		_OSD_PLA_SetYCbCr709En((__u32)u4Plane, 1);
	} else {
		_OSD_PLA_SetYCbCr709En((__u32)u4Plane, 0);
	}

#endif
	_OSD_PLA_SetXVYCCEn((__u32)u4Plane, 1);
	_OSD_PLA_SetYCbCr709En((__u32)u4Plane, 0);

	VERIFY((__s32)OSD_RET_OK ==
	       _OSD_RGN_GetAddress((__u32)i4FirstRegion, (unsigned long *)&u4FirstRegionAddr));

	ASSERT((u4FirstRegionAddr & 0xf) == 0);
	VERIFY((__s32)OSD_RET_OK ==
	       _OSD_PLA_SetHeaderAddr(u4Plane, u4FirstRegionAddr));

	_arOsdPlaneInfo[u4Plane].u4FirstRegionAddr = u4FirstRegionAddr;
	_UpdatePlaneEnableState(u4Plane);

	_arOsdPlaneInfo[u4Plane].i4FlipedList = (__s32)u4List;
#ifdef FB_dEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "u4FirstRegionAddr=%x, u4List=%d\n", u4FirstRegionAddr, u4List);
#endif
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_FlipToNone(__u32 u4Plane)
{

	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_SetHeaderAddr(u4Plane, 0));
	_arOsdPlaneInfo[u4Plane].u4FirstRegionAddr = 0;

	_UpdatePlaneEnableState(u4Plane);

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Reflip(__u32 u4List)
{
	__s32 i4Index;
	__u32 u4Value;

	for (i4Index = 0; i4Index < (__s32)OSD_PLANE_MAX_NUM; i4Index++) {
		if (u4List == (__u32)_arOsdPlaneInfo[i4Index].i4FlipedList) {
			IGNORE_RET(_OSD_PLA_GetEnable((__u32)i4Index, &u4Value));

			if (u4Value) {
				IGNORE_RET(OSD_PLA_FlipTo((__u32)i4Index, u4List));
			}
		}
	}

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_GetBlendLevel(__u32 u4Plane, __u8 *pu1BlendLevel)
{
	__u32 u4Blending;

	if (pu1BlendLevel == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_GetBlending(u4Plane, &u4Blending));
	*pu1BlendLevel = (__u8) u4Blending;

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_SetBlendLevel(__u32 u4Plane, __u8 u1BlendLevel)
{
	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_SetBlending(u4Plane, u1BlendLevel));
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_UpdateHwReg(u4Plane));
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_GetFading(__u32 u4Plane, __u8 *pu1Fading)
{
	__u32 u4Fading;

	if (pu1Fading == NULL) {
		return -(__s32)OSD_RET_INV_ARG;
	}

	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_GetFading(u4Plane, &u4Fading));
	*pu1Fading = (__u8) u4Fading;

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_SetFading(__u32 u4Plane, __u8 u1Fading)
{
	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_SetFading(u4Plane, u1Fading));
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_UpdateHwReg(u4Plane));
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_SetHFilter(__u32 u4Plane, bool fgEnable)
{
	OSD_VERIFY_PLANE(u4Plane);
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_SetHFilter(u4Plane, fgEnable));
	VERIFY((__s32)OSD_RET_OK == _OSD_PLA_UpdateHwReg(u4Plane));
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_SetFifo(__u32 u4Plane, bool fgFastReq, __u8 u1ExVacThr,
		      __u8 u1VacThr, __u8 u1FullThr)
{

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_Dump(__u32 u4Plane)
{
	/*#ifdef CC_CLI*/
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OSD_PLA_GetFirstRegion(__u32 u4Plane, __u32 *pu4Region)
{
	OSD_VERIFY_PLANE(u4Plane);

	return OSD_RGN_LIST_Get((__u32)_arOsdPlaneInfo[u4Plane].i4FlipedList,
				(__s32)OSD_RGN_LIST_HEAD, pu4Region);
}


/*-----------------------------------------------------------------------------*/
/** Brief  flip plane right now. This function may make the display flash if it's not in VSYNC
period.
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OsdFlipPlaneRightNow(__u32 u4Plane, bool fgValidReg, __u32 u4List)
{
	OSD_VERIFY_PLANE(u4Plane);

	if (eScrnState_Enabled == _rOsdScrnFlag.eOsdScrnState) {
		if (OSD_PLANE_UI != u4Plane && OSD_PLANE_5 != u4Plane) {
			FB_PRINT(FB_LOG_LVL_DBG, "In ScrnSaver, Flip plane %d\n", u4Plane);
			return OSD_RET_OK;
		}
	}

	if (fgValidReg) {
		RET_ON_FAIL(OSD_PLA_FlipTo(u4Plane, u4List));
	} else {
		RET_ON_FAIL(OSD_PLA_FlipToNone(u4Plane));
	}

	/*RET_ON_FAIL(OSD_PLA_Enable(u4Plane, TRUE));*/
	/*changed by msz00441 for reset but not enalbe 20080327*/
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief  flip plane until VSYNC period. This function will block the thread to wait for VSYNC.
 *  @param
 *  @return
 */
/*-----------------------------------------------------------------------------*/
__s32 OsdFlipPlaneInVsyncNowait(__u32 u4Plane, bool fgValidReg, __u32 u4List, bool b_force)
{
	/*CRIT_STATE_T cState;*/
	unsigned long flags;

	OSD_VERIFY_PLANE(u4Plane);

#ifndef __ARM2__
	/*Avoid the same plane being flipped by two thread in the same time.*/
	if (b_force) {
		down((struct semaphore *)&_arOsdPlaneFlipFlagNowait[u4Plane].hSemPlane);
	} else {
		down_trylock((struct semaphore *)&_arOsdPlaneFlipFlagNowait[u4Plane].hSemPlane);
	}
#endif

	/*This block cann't be interrupted!*/
	/*cState = x_crit_start();*/
#ifndef __ARM2__
	spin_lock_irqsave(&ac83xx_osd_plane_lock, flags);
#endif
	_arOsdPlaneFlipFlagNowait[u4Plane].fgNeedFlip = TRUE;
	_arOsdPlaneFlipFlagNowait[u4Plane].u4List = u4List;
	_arOsdPlaneFlipFlagNowait[u4Plane].fgValidReg = fgValidReg;
	/*x_crit_end(cState);*/
#ifndef __ARM2__
	spin_unlock_irqrestore(&ac83xx_osd_plane_lock, flags);
#endif

	return (__s32)OSD_RET_OK;
}

__u32 OSD_PLA_Map(__u32 u4Plane)
{
	switch (u4Plane) {
	case 2:  /* UI 16bpp*/
		return 0;

	case 1:  /* IG 32bpp*/
		return 1;

	case 0:  /* PG 32bpp*/
		/*_OsdSetOsd3Display();*/
		return 2;

	case 4:  /* MSG*/
		return 4;

	default:
		return 0;

	}
}

/* !fn __s32 OSD_DRV_GetMemChannelEx(__u32 u4Plane)
  \brief  Return the related DRAM channel number of the specific plane.
  \note Currently,  OSD2/PG OSD4/UI in channel 2 , OSD1/IG in channel 1
  */
__s32 OSD_DRV_GetMemChannelEx(__u32 u4Plane)
{
	switch (u4Plane) {
	case 0:
		return 1;

	case 1:
		return OSD_2_USE_CHANNEL;

	case 2:
		return 1;

	case 3:
		return OSD_4_USE_CHANNEL;

	default:
		return OSDDARMCHANNEL1;
	}
}

__s32 OSD_PLA_GetPlaneInfo(__u32 u4Plane, OSD_RGN_CMD_T i4Cmd, __u32 *pu4Info)
{
	__s32 i4HeadRegion, ret;
	__u32 u4FstRgn = (__u32)_arOsdPlaneInfo[u4Plane].i4FlipedList;

	OSD_RGN_LIST_GetHead(u4FstRgn, &i4HeadRegion);
	ret = OSD_RGN_Get(i4HeadRegion, i4Cmd, pu4Info);

	return ret;
}

/* !fn __s32 OSDSetColorSpace(__u8 ui1ClrSpa)
  \brief  set wether 601 or 709 is used of all osd planes.
  \note OSD_COLORSPACE_601  /  OSD_COLORSPACE_709
  */
__s32 OSDSetColorSpace(__u8 ui1ClrSpa)
{
	if (OSD_COLORSPACE_601 == ui1ClrSpa) {
		_fgOsdYCbCr709 = FALSE;
	} else {
		_fgOsdYCbCr709 = TRUE;
	}

	return (__s32)OSD_RET_OK;
}


__s32 OSDGetColorSpace(__u8 *ui1ClrSpa)
{
	if (ui1ClrSpa != NULL) {
		if (_fgOsdYCbCr709) {
			*ui1ClrSpa = OSD_COLORSPACE_709;
		} else {
			*ui1ClrSpa = OSD_COLORSPACE_601;
		}
	}

	return (__s32)OSD_RET_OK;
}




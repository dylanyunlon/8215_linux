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
#ifndef __ARM2__
#include <linux/mm.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/pmx_hal.h>
#include "windows.h"
#include "x_debug.h"
#include "x_stl_lib.h"
#else
#include "x_types.h"
#include "drv_av_d.h"
#include "pmx_hal.h"
#endif
#include "drv_config.h"
#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_util.h"
#include "x_printf.h"
#include "x_hal_ic.h"

#include "log.h"
#include "pmx_vfy_drv.h"
/*#include "pmx_vfy_sys.h"*/
#include "pmx_vfy_hal.h"

#include "sys_config.h"

#define  PMX_VRF_IO_SEMI_HOSTING    0
#define  PMX_VRF_IO_RVD_UTIL        1
#define  PMX_VRF_IO_HDD             2
#define  PMX_VRF_IO_USB             3
#define  PMX_VRF_FILE_IO_TYPE       PMX_VRF_IO_SEMI_HOSTING


static bool _fgPmxVerifyDrvInit = FALSE;
void PmxVerifyDrvInit(bool fgHwReset)
{
	if (!_fgPmxVerifyDrvInit) {
		if (fgHwReset) {
			vPmxVerifyHalSysInit();
		}

		vPmxHalInit();

		_fgPmxVerifyDrvInit = TRUE;
	}
}


void PmxVerifyCVBSSetup(__u8 ucVdoId, __u32 ucPmxMode)
{
#if 0 /*front panel*/

	if (ucVdoId == 0) { /*main vdo*/
		switch (ucPmxMode) {
		case RES_480I:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[0].pu4RegSetting, _ArCvbsSetting[0].size);
			break;

		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[1].pu4RegSetting, _ArCvbsSetting[1].size);
			break;

		case RES_576I:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[2].pu4RegSetting, _ArCvbsSetting[2].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[3].pu4RegSetting, _ArCvbsSetting[3].size);
			break;

		default:
			break;
		}
	} else
#endif
	{
		switch (ucPmxMode) {
		case RES_480I:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[4].pu4RegSetting, _ArCvbsSetting[4].size);
			break;

		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[5].pu4RegSetting, _ArCvbsSetting[5].size);
			break;

		case RES_576I:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[6].pu4RegSetting, _ArCvbsSetting[6].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArCvbsSetting[7].pu4RegSetting, _ArCvbsSetting[7].size);
			break;

		default:
			break;
		}
	}
}
void PmxVerifyVDOSetup(__u8 ucVdoId, __u32 u4Mode)
{
	if (ucVdoId == 0) { /*main vdo*/
		switch (u4Mode) {
		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[0].pu4RegSetting, _ArVdoSetting[0].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[1].pu4RegSetting, _ArVdoSetting[1].size);
			break;

		case RES_720P60HZ:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[2].pu4RegSetting, _ArVdoSetting[2].size);
			break;

		case RES_1080P60HZ:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[3].pu4RegSetting, _ArVdoSetting[3].size);
			break;

		case RES_480P_800:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[8].pu4RegSetting, _ArVdoSetting[8].size);
			break;

		case RES_600P_800:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[9].pu4RegSetting, _ArVdoSetting[9].size);
			break;

		case RES_600P_1024:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[12].pu4RegSetting, _ArVdoSetting[12].size);
			break;

		case RES_800P_1280:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[13].pu4RegSetting, _ArVdoSetting[13].size);
			break;

		default:
			break;
		}
	} else { /*sub vdo*/
		switch (u4Mode) {
		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[4].pu4RegSetting, _ArVdoSetting[4].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[5].pu4RegSetting, _ArVdoSetting[5].size);
			break;

		case RES_720P60HZ:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[6].pu4RegSetting, _ArVdoSetting[6].size);
			break;

		case RES_1080P60HZ:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[7].pu4RegSetting, _ArVdoSetting[7].size);
			break;

		case RES_480P_800:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[10].pu4RegSetting, _ArVdoSetting[10].size);
			break;

		case RES_600P_800:
			vPmxVerifyHalLoadSetting(_ArVdoSetting[11].pu4RegSetting, _ArVdoSetting[11].size);
			break;

		default:
			break;
		}
	}
}

void PmxVerifyVDOActRegionSetup(__u8 ucVdoId, __u32 ucPmxMode)
{
	if (ucVdoId == 0) { /*main vdo*/
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "PmxVerifyVDOActRegionSetup: ucPmxMode = %d, RES_480P = %d\r\n"
			, (int)ucPmxMode, RES_480P);

		switch (ucPmxMode) {
		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[0].pu4RegSetting
				, _ArVdoActRegionSetting[0].size);
			break;

		case RES_480I:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[4].pu4RegSetting
				, _ArVdoActRegionSetting[4].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[1].pu4RegSetting
				, _ArVdoActRegionSetting[1].size);
			break;

		case RES_576I:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[5].pu4RegSetting
				, _ArVdoActRegionSetting[5].size);
			break;

		case RES_480P_800:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[2].pu4RegSetting
				, _ArVdoActRegionSetting[2].size);
			break;

		case RES_600P_800:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[3].pu4RegSetting
				, _ArVdoActRegionSetting[3].size);
			break;

		case RES_480P_800_50HZ:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[10].pu4RegSetting
				, _ArVdoActRegionSetting[10].size);
			break;

		case RES_600P_800_50HZ:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[11].pu4RegSetting
				, _ArVdoActRegionSetting[11].size);
			break;

		case RES_600P_1024:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[12].pu4RegSetting
				, _ArVdoActRegionSetting[12].size);
			break;

		case RES_600P_1024_50HZ:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[13].pu4RegSetting
				, _ArVdoActRegionSetting[13].size);
			break;

		case RES_800P_1280:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[14].pu4RegSetting
				, _ArVdoActRegionSetting[14].size);
			break;

		case RES_720P60HZ:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[15].pu4RegSetting
				, _ArVdoActRegionSetting[15].size);
			break;

		default:
			break;
		}
	} else { /*sub vdo*/
		switch (ucPmxMode) {
		case RES_480P:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[6].pu4RegSetting
				, _ArVdoActRegionSetting[6].size);
			break;

		case RES_480I:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[8].pu4RegSetting
				, _ArVdoActRegionSetting[8].size);
			break;

		case RES_576P:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[7].pu4RegSetting
				, _ArVdoActRegionSetting[7].size);
			break;

		case RES_576I:
			vPmxVerifyHalLoadSetting(_ArVdoActRegionSetting[9].pu4RegSetting
				, _ArVdoActRegionSetting[9].size);
			break;

		default:
			break;
		}
	}
}

void PmxVerifySetMode(__u8 ucVdoId, __u32 u4SrcFmt, __u32 u4PmxFmt, __u8 ucSrcType
	, __u8 ucTvType, __u8 ucInterlace)
{
	__u8 u4SrcMode = 0, ucPmxMode = 0;

	switch (u4SrcFmt) {
	case 576:
		u4SrcMode = RES_576P;
		break;

	case 720:
		u4SrcMode = RES_720P60HZ;
		break;

	case 1080:
		u4SrcMode = RES_1080P60HZ;
		break;

	case 600:
		if (ucSrcType == PMX_TV_TYPE_600P_1024) {
			u4SrcMode = RES_600P_1024;
		} else {
			u4SrcMode = RES_600P_800;
		}

		break;

	case 800:
		u4SrcMode = RES_800P_1280;
		break;

	case 480:
	default:
		if (ucSrcType == PMX_TV_TYPE_480P_800) {
			u4SrcMode = RES_480P_800;
		} else {
			u4SrcMode = RES_480P;
		}

		break;
	}

	switch (u4PmxFmt) {
	case 576:
		if (ucInterlace) {
			ucPmxMode = RES_576I;
		} else {
			ucPmxMode = RES_576P;
		}

		break;

	case 600:
		if (ucTvType == PMX_TV_TYPE_600P_800_50) {
			ucPmxMode = RES_600P_800_50HZ;
		} else if (ucTvType == PMX_TV_TYPE_600P_1024) {
			ucPmxMode = RES_600P_1024;
		} else if (ucTvType == PMX_TV_TYPE_600P_1024_50) {
			ucPmxMode = RES_600P_1024_50HZ;
		} else {
			ucPmxMode = RES_600P_800;
		}

		break;

	case 800:
		ucPmxMode = RES_800P_1280;
		break;

	case 720:
		ucPmxMode = RES_720P60HZ;
		break;

	case 480:
	default:
		if (ucInterlace) {
			ucPmxMode = RES_480I;
		} else if (ucTvType == PMX_TV_TYPE_480P_800) {
			ucPmxMode = RES_480P_800;
		} else if (ucTvType == PMX_TV_TYPE_480P_800_50) {
			ucPmxMode = RES_480P_800_50HZ;
		} else {
			ucPmxMode = RES_480P;
		}

		break;
	}

	PmxVerifyDrvInit(FALSE);
	/*PmxVerifySetVDOPtr(ucVdoId, ucVdoId);*/

	PmxVerifyVDOSetup(ucVdoId, u4SrcMode);
	PmxVerifyVDOActRegionSetup(ucVdoId, ucPmxMode);

	if (0 != ucVdoId) {
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[2].pu4RegSetting, _ArOutputInitSetting[2].size);
		PmxVerifyCVBSSetup(ucVdoId, ucPmxMode);
	}

	vPmxHalSetMode(ucVdoId, ucPmxMode);

	/*vPmxVerifyHalVdoFit(ucVdoId, u4SrcFmt, u4PmxFmt);*/
	vPmxVerifyHalVdoFit(ucVdoId, u4SrcMode, ucPmxMode);

	vPmxVerifyHalEnablePMX(ucVdoId, 1);
	/*vPmxHalReset(ucVdoId);*/
	vPmxHalSetTvType(ucVdoId, ucTvType);

	/*vdo interlace 480i or 1080i*/
	if ((ucPmxMode == RES_480I) || (ucPmxMode == RES_576I)) {
		vPmxVerifyHalInterlaceExtra(ucVdoId, ucPmxMode);
	}

	if (ucVdoId == 0) { /*main vdo*/
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_3);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_4);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_6);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_8);
	}

	/*
	else
	{
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_3);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_4);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_6);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_7);
	  vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_8);
	}
	*/
	vPmxHalReset(ucVdoId);

	vPmxVerifyHalReset(ucVdoId);
}

void PmxVerifyVdoXSkip(__u8 ucVdoId, __u32 u4XSkip)
{
	vPmxVerifyHalVdoXSkip(ucVdoId, u4XSkip);
}

void PmxVerifyVdoXSkipRoutine(__u8 ucVdoId)
{
	vPmxVerifyHalVdoXSkip(ucVdoId, 1);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 2);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 3);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 4);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 5);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 6);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 7);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 8);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 9);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 10);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 11);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 12);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 13);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 14);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 15);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalVdoXSkip(ucVdoId, 0);
}

void PmxVerifySetVdoDeIntMode(__u8 ucVdoId, __u8 ucMode)
{
	vPmxVerifyHalDeIntMode(ucVdoId, ucMode);
}

void PmxVerifySetVdoSrcFmt(__u8 ucVdoId, __u8 ucSrcFmt)
{
	vPmxVerifyHalSrcFmt(ucVdoId, ucSrcFmt);
}


void PmxVerifySetLumaKey(__u8 ucVdoId, __u8 ucOn, __u32 u4LumaKeyVal)
{
	vPmxVerifyHalLumaKey(ucVdoId, ucOn, u4LumaKeyVal);
}

void PmxVerifySetShiftLine(__u8 ucVdoId, __u8 ucHD, __u8 ucLine)
{
	vPmxVerifyHalShiftLine(ucVdoId, ucHD, ucLine);
}

void PmxVerifyZoom(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH, __u32 u4DstW, __u32 u4DstH
	, __u32 u4ZoomW, __u32 u4ZoomH)
{
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, u4ZoomW, u4ZoomH);
}

void PmxVerifyZoomRoutine(__u8 ucVdoId, __u32 u4SrcW, __u32 u4SrcH, __u32 u4DstW, __u32 u4DstH)
{
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_2X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_3X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_4X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_1D2X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_1D3X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_1D4X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_2X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_3X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_4X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1D2X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1D3X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1D4X, PMX_VFY_V_ZOOM_1X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A2X, PMX_VFY_V_ZOOM_A2X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A3X, PMX_VFY_V_ZOOM_A3X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A4X, PMX_VFY_V_ZOOM_A4X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A1D2X, PMX_VFY_V_ZOOM_A1D2X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A1D3X, PMX_VFY_V_ZOOM_A1D3X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_A1D4X, PMX_VFY_V_ZOOM_A1D4X);
	vPmxVerifyHalRoutineBreak();
	vPmxVerifyHalZoom(ucVdoId, u4SrcW, u4SrcH, u4DstW, u4DstH, PMX_VFY_H_ZOOM_1X, PMX_VFY_V_ZOOM_1X);
}

volatile __u32 _u4PmxVerifyIsrCnt;
void PmxVerifyDispChkSum(__u8 ucVdoId)
{
	__u32 u4ChkSum = 0;

	vPmxVerifyHalDispInitChkSum(ucVdoId);
	_u4PmxVerifyIsrCnt = 0;

	while (1) {
		if (_u4PmxVerifyIsrCnt > 5) {
			u4ChkSum = u4PmxVerifyHalDispGetChkSum(ucVdoId);
			break;
		}
	}
}

void PmxVerifySetOSDMixRatio(__u8 ucOsdId, __u32 u4MixRatio)
{
	vPmxVerifyHalOSDMixRatio(ucOsdId, u4MixRatio);
}

void PmxVerifySetDataSource(__u8 ucOsdSel, __u8 ucFpdSel)
{
	vPmxVerifyHalSetDataSource(ucOsdSel, ucFpdSel);
}

__u32 PmxVerifyProbeRegister(__u32 addr, __u32 one)
{
	__u32 p, v;
	volatile __u32 *ptr = (volatile __u32 *) addr;

	if (addr & 0x00000003) {
		return 0xffffffff;
	}

	p = 0;

	if (one) {
		/* one test*/
		*ptr = 0xffffffff;
		v = *ptr;
		p |= ~v;
	} else {
		/* zero test*/
		*ptr = 0;
		p |= *ptr;
	}

	return p;
}

void PmxVerifyScanRegister(__u32 start_addr, __u32 one)
{
	__u32 i;
	__u32 pattern;

	for (i = 0; i < 256; i += 4) {
		pattern = PmxVerifyProbeRegister(0x70000000 + start_addr + i, one);

		if (pattern) {
			FB_PRINT(FB_LOG_LVL_ERR, "FMT", "fail: [%04x] ==> %08x\n", (unsigned int)(i + start_addr)
				, (unsigned int)pattern);
		}
	}
}

void PmxVerifyIsrTest(__u8 ucVdoId, __u8 ucOn)
{
	if (ucOn) {
		vPmxHalIsrEnable(ucVdoId);
	} else {
		vPmxHalIsrDisable(ucVdoId);
	}
}

void PmxVerifyFrameLock(__u8 uc4TvType, __u8 ucLockOn, __u8 ucTestEn)
{
	/*bool fgOn = (ucLockOn != 0) ? TRUE : FALSE;*/
	/*bool fgEn = (ucTestEn != 0) ? TRUE : FALSE;*/

	/*vPmxHalVerifyFrameLockEn(uc4TvType, fgOn, fgEn);*/
}

void PmxVerifyTtlSel(__u8 uc4TtlType)
{
	if (0 == uc4TtlType) {
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[0].pu4RegSetting, _ArOutputInitSetting[0].size);
	} else {
		vPmxVerifyHalLoadSetting(_ArOutputInitSetting[1].pu4RegSetting, _ArOutputInitSetting[1].size);
	}
}




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

#include "vga_hal_api.h"
#include "drv_hdtv.h"
#include "drv_vga.h"
#include "drv_vdoclk.h"
#include "vga_auto.h"
#include "ybr_vga_oal.h"
#include <linux/types.h>
/**
    Global Variable Define
*/
u32 g_u4SrcType = SRC_NULL;
u8  g_u1Timing = MODE_NOSIGNAL;
u8 g_u4SigStatus = 0;
u8 g_u4SigPreStatus = 0;
bool g_bStop = FALSE;


HANDLE_T g_hMLoopThread = (HANDLE_T)NULL;

u32 g_u4IrqStatus = 0;

bool fgIsValidSource(void)
{
	return ((g_u4SrcType == SRC_YBR || g_u4SrcType == SRC_VGA) ? TRUE : FALSE);
}
/**
    Interrupt
*/
u32 u4DrvVideoGetIrqStatus(void)
{
	return HAL_READ32(INT_YBR_VGA_STA);
}

void vDrvVideoClearIrqStatus(u32 u4IrqStatus)
{
	HAL_WRITE32(INT_YBR_VGA_STA, u4IrqStatus);
}

void vVga_IRQ(u16 u2Vector)
{
	g_u4IrqStatus = u4DrvVideoGetIrqStatus();
	vDrvVideoClearIrqStatus(g_u4IrqStatus);


	if (g_u4IrqStatus & INT_MODE_CHANGE) {
		/*UTIL_Printf( "Received INT_MODE_CHANGE Interrupt.\r\n");*/
	}

	if (g_u4IrqStatus & INT_MUTE) {
		/*UTIL_Printf( "Received INT_MUTE Interrupt.\r\n");*/
	}

	if (g_u4IrqStatus & INT_VSYNC) {
		/*UTIL_Printf( "Received INT_VSYNC Interrupt.\r\n");*/
	}


	if (g_u4IrqStatus & INT_DDS_LOCK) {
		/* UTIL_Printf( "Received INT_DDS_LOCK Interrupt.\r\n");*/
	}

}

/**
*/
u8 bDrvVideoGetTiming(void)
{
	switch (g_u4SrcType) {
	case SRC_YBR:
		return _bHdtvTiming;

	case SRC_VGA:
		return _bVgaTiming;

	default:
		break;
	}

	return MODE_NOSIGNAL;
}

u8 bVGAMode_Detect(void)
{
	static u8 bPreMode = MODE_NOSIGNAL;
	u8 bMode = MODE_NOSIGNAL;
	u16 u2Width = 0;
	u16 u2Height = 0;
	u8 bInterlace = 0;

	bMode = bDrvVideoGetTiming();
	u2Width = wDrvVideoInputWidth();
	u2Height = wDrvVideoInputHeight();
	bInterlace = (bDrvVideoIsSrcInterlace() ? 1 : 0);

	if ((bMode != MODE_NOSIGNAL) || (bMode != MODE_NOSUPPORT) || (bPreMode != bMode)) {
		pr_debug("VGA Mode Dectect: %d, resolution: %dX%d, interlace:%d\r\n",
			 bMode, u2Width, u2Height, bInterlace);
		bPreMode = bMode;
	}

	return bPreMode;
}

/**
    Init api
*/

/**
    SW Init
*/
void vDrvVideoSwInit(void)
{
	/* vga_auto.c*/
	_bAutoFlag = 0;
	_bVdoSP0AutoState = VDO_AUTO_NOT_BEGIN; /*0;*/
	_bVgaDelayCnt = 0;
	/*vdo_clk.c*/
	_bCLKSetFlag = 0;
	/* drv_autocolor.c*/
	_bAutoColorState0 = VDO_AUTO_COLOR_NOT_BEGIN;
	/* drv_hdtv.c*/
	_bHdtvTiming = NO_SIGNAL;
	_bVgaTiming = NO_SIGNAL;
	/*Global Var Reset*/
	g_u4SigStatus = 0;
	g_u4SigPreStatus = 0;
	/* drv_vga.c*/
#if SUPPORT_VGA_USERMODE
	_bVgaUserMode = 0;
#endif

}
/**
    HW Init
*/

/**
    set RGB2YCbCr Block
*/
void vDrvColorTranSet(void)
{
	if (g_u4SrcType == SRC_VGA) {

		vVgaRGB2YCbCrSet();
	} else {
		vHdtvRGB2YCbCrSet();
	}
}

void vDrvVideoHwInit(void)
{
	/* Clock*/
	vDrvYbrVgaClkEnable();
	/* Video ADC testing*/
	vDrvADCDefaultSetting();/*5371*/
	vDrvADCOffsetCal();
	vDrvAllHDADCPow(FALSE);
}


void vDrvVideoInit(void)
{
	/*Sw Init*/
	vDrvVideoSwInit();
	/*Hw Init*/
	vDrvVideoHwInit();
}


/** config source type api
*/
void initYPbPrVGA(void)
{
	vDrvAllHDADCPow(TRUE);      /*for power saving , power on HD ADC , by huahua 20070427*/
	vDrvSetHDTVADC(); /* mode detect done - TODO */
	vDrvSetHDTVMux();
	vDrvColorTranSet();
	/*UTIL_Printf("initYPbPrVGA success\r\n");*/
}


void vDrvVideoConnect(bool fgOnOff)
{
	switch (g_u4SrcType) {
	case SRC_YBR:
		vHdtvConnect(fgOnOff);
		break;

	case SRC_VGA:
		vVgaConnect(fgOnOff);
		break;
	}

	if (fgOnOff) {
		pr_debug("vDrvVideoConnect %s is connect\r\n", (g_u4SrcType == SRC_YBR) ? "YPbPr" : "VGA");
	} else {
		pr_debug("vDrvVideoConnect %s is disconnect\r\n", (g_u4SrcType == SRC_YBR) ? "YPbPr" : "VGA");
	}
}
/**/

u8 bDrvVideoSignalStatus(void)
{

	u8 bRet = (u8)SV_VDO_NOSIGNAL;

	/*irq_save(flags);*/
	switch (g_u4SrcType) {

	case SRC_YBR:
		bRet = bHdtvSigStatus();
		break;

	case SRC_VGA:
		bRet = bVgaSigStatus();
		break;

	default:
		break;
	}

	/*irq_restore(flags);*/
	return bRet;
}

const CHAR *strDrvVideoGetTimingString(u8 bTiming)
{
	switch (bTiming) {
	case MODE_525I_OVERSAMPLE:
		return "MODE_525I_OVERSAMPLE";

	case MODE_625I_OVERSAMPLE:
		return "MODE_625I_OVERSAMPLE";

	case MODE_480P_OVERSAMPLE:
		return "MODE_480P_OVERSAMPLE";

	case MODE_576P_OVERSAMPLE:
		return "MODE_576P_OVERSAMPLE";

	case MODE_720p_50:
		return "MODE_720p_50";

	case MODE_720p_60:
		return "MODE_720p_60";

	case MODE_1080i_48:
		return "MODE_1080i_48";

	case MODE_1080i_50:
		return "MODE_1080i_50";

	case MODE_1080i:
		return "MODE_1080i";

	case MODE_1080p_24:
		return "MODE_1080p_24";

	case MODE_1080p_25:
		return "MODE_1080p_25";

	case MODE_1080p_30:
		return "MODE_1080p_30";

	case MODE_1080p_50:
		return "MODE_1080p_50";

	case MODE_1080p_60:
		return "MODE_1080p_60";

	case MODE_525I:
		return "MODE_525I";

	case MODE_625I:
		return "MODE_625I";

	case MODE_480P:
		return "MODE_480P";

	case MODE_576P:
		return "MODE_576P";

	case MODE_720p_24:
		return "MODE_720p_24";

	case MODE_720p_25:
		return "MODE_720p_25";

	case MODE_720p_30:
		return "MODE_720p_30";

	case MODE_240P:
		return "MODE_240P";

	case MODE_540P:
		return "MODE_540P";

	case MODE_480P_24:
		return "MODE_480P_24";

	case MODE_480P_30:
		return "MODE_480P_30";

	case MODE_576P_25:
		return "MODE_576P_25";

	case MODE_HDMI_640_480P:
		return "MODE_HDMI_640_480P";

	case MODE_HDMI_720p_24:
		return "MODE_HDMI_720p_24";

	case MODE_3D_720p_50_FP:
		return "MODE_3D_720p_50_FP";

	case MODE_3D_720p_60_FP:
		return "MODE_3D_720p_60_FP";

	case MODE_3D_1080p_24_FP:
		return "MODE_3D_1080p_24_FP";

	case MODE_3D_1080I_60_FP:
		return "MODE_3D_1080I_60_FP";

	case MODE_3D_480p_60_FP:
		return "MODE_3D_480p_60_FP";

	case MODE_3D_576p_50_FP:
		return "MODE_3D_576p_50_FP";

	case MODE_3D_720p_24_FP:
		return "MODE_3D_720p_24_FP";

	case MODE_3D_720p_30_FP:
		return "MODE_3D_720p_30_FP";

	case MODE_3D_1080p_30_FP:
		return "MODE_3D_1080p_30_FP";

	case MODE_3D_480I_60_FP:
		return "MODE_3D_480I_60_FP";

	case MODE_3D_576I_60_FP:
		return "MODE_3D_576I_60_FP";

	case MODE_3D_1080I_50_FP:
		return "MODE_3D_1080I_50_FP";

	case MODE_3D_1080p_50_FP:
		return "MODE_3D_1080p_50_FP";

	case MODE_3D_1080p_60_FP:
		return "MODE_3D_1080p_60_FP";

	case MODE_3D_1650_750_60_FP:
		return "MODE_3D_1650_750_60_FP";

	case MODE_3D_1650_1500_30_FP:
		return "MODE_3D_1650_1500_30_FP";

	case MODE_3D_1080p_24_SBS_FULL:
		return "MODE_3D_1080p_24_SBS_FULL";

	case MODE_3D_1080p_30_SBS_FULL:
		return "MODE_3D_1080p_30_SBS_FULL";

	case MODE_3D_1080I_50_SBS_FULL:
		return "MODE_3D_1080I_50_SBS_FULL";

	case MODE_3D_1080I_60_SBS_FULL:
		return "MODE_3D_1080I_60_SBS_FULL";

	case MODE_3D_720p_24_SBS_FULL:
		return "MODE_3D_720p_24_SBS_FULL";

	case MODE_3D_720p_30_SBS_FULL:
		return "MODE_3D_720p_30_SBS_FULL";

	case MODE_3D_720p_50_SBS_FULL:
		return "MODE_3D_720p_50_SBS_FULL";

	case MODE_3D_720p_60_SBS_FULL:
		return "MODE_3D_720p_60_SBS_FULL";

	case MODE_3D_480p_60_SBS_FULL:
		return "MODE_3D_480p_60_SBS_FULL";

	case MODE_3D_576p_50_SBS_FULL:
		return "MODE_3D_576p_50_SBS_FULL";

	case MODE_3D_480I_60_SBS_FULL:
		return "MODE_3D_480I_60_SBS_FULL";

	case MODE_3D_576I_50_SBS_FULL:
		return "MODE_3D_576I_50_SBS_FULL";


	case MODE_MAX:
	default:
		return "Not Support OR No Signal";
	}
}

u16 wDrvVideoInputWidth(void)
{
	u16 wRet = 0;
	/* irq_save(flags);*/

	switch (g_u4SrcType) {
	case SRC_YBR:
		wRet = wHdtvInputWidth();
		break;

	case SRC_VGA:
		wRet = wVgaInputWidth();
		break;

	default:
		break;
	}

	/* irq_restore(flags);*/
	return wRet;
}

/* **********************************************************************/
/* Function :*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
u16 wDrvVideoInputHeight(void)
{
	u16 wRet = 0;

	switch (g_u4SrcType) {
	case SRC_YBR:
		wRet = wHdtvInputHeight();
		break;

	case SRC_VGA:
		wRet = wVgaInputHeight();
		break;

	default:
		break;
	}

	return wRet;
}
/* **********************************************************************/
/* Function :*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
u8 bDrvVideoIsSrcInterlace(void)
{
	u8 bRet = 0;

	switch (g_u4SrcType) {
	case SRC_YBR:
		bRet = bHdtvInterlace();
		break;

	case SRC_VGA:
		bRet = bVgaInterlace();
		break;


	default:
		break;
	}

	return bRet;
}

/* **********************************************************************/
/* Function :*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
u16 wDrvVideoGetHTotal(void)
{
	u16 wHTotal = 0;

	switch (g_u4SrcType) {
	case SRC_YBR:
	case SRC_VGA: {
		u8 u1Timing;

		u1Timing = (g_u4SrcType == SRC_YBR) ? _bHdtvTiming : _bVgaTiming;

		if (u1Timing < MAX_TIMING_FORMAT) {
			/*wHTotal = vDrvCLKINGetHtotal();*/
			wHTotal =  Get_VGAMODE_IHTOTAL(u1Timing);

			if (bHdtvCheckCenEnable()) {
				wHTotal = wHTotal / (u16)2;
			}
		}

		break;
	}

	default:
		break;
	}

	return wHTotal;
}

/* **********************************************************************/
/* Function :*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
u16 wDrvVideoGetVTotal(void)
{
	u16 wVTotal = 0;

	switch (g_u4SrcType) {
	case SRC_YBR:
		if (_bHdtvTiming < MAX_TIMING_FORMAT) {
			wVTotal =  Get_VGAMODE_IVTOTAL(_bHdtvTiming);
		}

		break;

	case SRC_VGA:
		if (_bVgaTiming < MAX_TIMING_FORMAT) {

			/*wVTotal = Get_VGAMODE_IVTOTAL(_bVgaTiming);*/
			if (fgIsVideoTiming(_bVgaTiming)) {
				wVTotal = Get_VGAMODE_IVTOTAL(_bVgaTiming);
			} else {
				pr_debug("wDrvVideoGetVTotal(): wVTotal is _wSP0StableVtotal:%d\r\n",
				(int)_wSP0StableVtotal);
				wVTotal = _wSP0StableVtotal;
			}
		}

		break;

	default:
		break;
	}

	return wVTotal;
}

/* **********************************************************************/
/* Function :*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
u8 bDrvVideoGetRefreshRate(void)
{
	u8 bRet = 0;

	switch (g_u4SrcType) {
	case SRC_YBR:
		bRet = bHdtvRefreshRate();
		break;

	case SRC_VGA:
		bRet = bVgaRefreshRate();
		break;

	default:
		break;
	}

	return bRet;
}


void vDrvVideoQueryInputTimingInfo(void)
{
	u8 u1VdoStatus = bDrvVideoSignalStatus();

	pr_debug("Source            = %-s\r\n",
		 g_u4SrcType == SRC_YBR ? "YPbPr" :
		 g_u4SrcType == SRC_VGA ? "VGA" : "Unknown");
	pr_debug("Signal Status     = %-s\r\n",
		 u1VdoStatus == SV_VDO_NOSIGNAL ? "No Signal" :
		 u1VdoStatus == SV_VDO_NOSUPPORT ? "Not Support" :
		 u1VdoStatus == SV_VDO_STABLE ? "Stable" : "Unknown");
	pr_debug("Video Timing      = %-s\r\n", strDrvVideoGetTimingString(bDrvVideoGetTiming()));
	pr_debug("Video Resolution  = %dx%d, %dx%d, %dHz, interlace: %d\r\n",
		 wDrvVideoInputWidth(),
		 wDrvVideoInputHeight(),
		 wDrvVideoGetHTotal(),
		 wDrvVideoGetVTotal(),
		 bDrvVideoGetRefreshRate(),
		 bDrvVideoIsSrcInterlace());

}



u8 bDrvPorchTune[2] = {
	SV_PORCHTUNE_DEC,
	SV_PORCHTUNE_DEC
};

u16 wDrvVideoGetPorch(u8 bPath, u8 bPorchType)
{
	u16  wDecPorch = 0;
	u16 wScposPorch = 0;

	/* implemention try to make DI/NR don't eat porch,scops eat porch
	   for overscan or lbox*/
	if (bDrvPorchTune[bPath]&SV_PORCHTUNE_DEC) {
		switch (g_u4SrcType) {
		case SRC_VGA:
			wDecPorch = wVgaGetPorch(bPath, bPorchType);
			break;

		case SRC_YBR:
			wDecPorch = wHdtvGetPorch(bPath, bPorchType);
			break;

		default:
			wDecPorch = 0;  /*rely on scpos's porch*/
			break;
		}
	}

	if (bDrvPorchTune[bPath]&SV_PORCHTUNE_SCPOS) {
		/*wScposPorch=_VDP_ScposGetPorch(bPath,bPorchType);*/
	}

	return wDecPorch + wScposPorch;
}
u16 wDrvVideoSetPorch(u8 bPath, u8 bPorchType, u16 wValue)
{
	u16  wDecPorch = 0, wScposPorch = 0, min, max;
	s32 tmp;
	/*s32 i4VBIadj; VBI LSC adjustion */

	if ((bPorchType != SV_HPORCH_CURRENT) && (bPorchType != SV_VPORCH_CURRENT)) {
		return 0;
	}

	/* only tune porch to decoder, SV_HPORCH_CURRENT|SV_HPORCH_CURRENT */

	if (bDrvPorchTune[bPath] == SV_PORCHTUNE_MIX) {
		/*wScposPorch=_VDP_ScposGetPorch(bPath,  bPorchType);*/
	}

	tmp = (s32)(wValue - wScposPorch);

	if (tmp < 0) {
		tmp = 0;
	}

	if (bDrvPorchTune[bPath]&SV_PORCHTUNE_DEC) {
		switch (g_u4SrcType) {
		case SRC_VGA:
			min = wVgaGetPorch(bPath, bPorchType + 2);
			max = wVgaGetPorch(bPath, bPorchType + 3);

			if (tmp < min) {
				tmp = min;
			}

			if (tmp > max) {
				tmp = max;
			}

			vVgaSetPorch(bPath, bPorchType, tmp);
			wDecPorch = wVgaGetPorch(bPath, bPorchType);
			break;

		case SRC_YBR:
			min = wHdtvGetPorch(bPath, bPorchType + 2);
			max = wHdtvGetPorch(bPath, bPorchType + 3);

			if (tmp < min) {
				tmp = min;
			}

			if (tmp > max) {
				tmp = max;
			}

			vHdtvSetPorch(bPath, bPorchType, tmp);
			wDecPorch = wHdtvGetPorch(bPath, bPorchType);
			break;


		default:
			wDecPorch = 0;
			break;
		}

		if (bDrvPorchTune[bPath] == SV_PORCHTUNE_DEC) {
			return wDecPorch;
		}
	}

	/*  SV_PORCHTUNE_MIX  or SV_PORCHTUNE_SCPOS */
	tmp = wValue - wDecPorch;
	/* scpos's min/max constrain is controlled inside _VDP_ScposSetPorch */
	/*  _VDP_ScposSetPorch(bPath,bPorchType,tmp);*/
	/* wScposPorch=_VDP_ScposGetPorch(bPath,bPorchType);*/
	return wDecPorch + wScposPorch;
}

/**
 * @brief vApiVideoVgaSetPhase(u8 bPath, u8 bValue)
 * Set Vga phase
 * @param  bPath :main/sub path (SV_VP_MAIN / SV_VP_PIP)
 * @param bValue  : Phase value (0~31)
 * @retval void
 * @example vApiVideoVgaSetPhase(SV_VP_MAIN, 5);
 */
u8 fgApiVideoVgaSetPhase(u8 bValue)
{
	if (g_u4SrcType != SRC_NULL) {
		vDrvVGASetPhase(bValue);
	} else {
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief vApiVideoVgaSetClock(u8 bPath, u16 wValue)
 * Set VGA Clock value
 * @param  bPath :main/sub path (SV_VP_MAIN / SV_VP_PIP)
 * @param wValue  : clock  value (not UI value)
 * @retval void
 * @example vApiVideoVgaSetClock(SV_VP_MAIN, 1024);
 */
u8 fgApiVideoVgaSetClock(u16 wValue)
{
	/*for 5371 mw if*/
	if (g_u4SrcType == SRC_VGA) {
		wValue = Get_VGAMODE_IHTOTAL(_bVgaTiming) + wValue - 127;
	} else {
		wValue = Get_VGAMODE_IHTOTAL(_bHdtvTiming) + wValue - 127;
	}

	if (wValue < 300) {
		wValue = 300;
		/*        ASSERT(0);*/
	}


	if (((g_u4SrcType == SRC_VGA) && (!bVgaInterlace()))  ||
	    ((g_u4SrcType == SRC_YBR) && (!bHdtvInterlace()))) {
#if CHANGE_SDDS_KPI
		Set_SDDS_KPI(1);
#endif
		vDrvVGASetClock(wValue);
#if CHANGE_SDDS_KPI
		vDrvEnableChang_SDDS_BW();
#endif
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
 * @brief fgApiVideoGeoHPosition(u8 bPath, u16 wValue)
 * Set VGA H Position
 * @param  bPath :main/sub path (SV_VP_MAIN / SV_VP_PIP)
 * @param wValue  : H Position (not UI value) (H porch)
 * @retval void
 * @example fgApiVideoGeoHPosition(SV_VP_MAIN, 100);
 */
u8 fgApiVideoGeoHPosition(u8 bPath, u16 wValue)
{
#if 0

	/*for 5371 mw if*/
	if (wValue > 511) {
		wValue = 511;
	}

	if (fgIsMainVga() || fgIsPipVga()) {
		wValue = Get_VGAMODE_IPH_BP(_bVgaTiming) + 256 - wValue;
	} else {
		wValue = Get_VGAMODE_IPH_BP(_bHdtvTiming) + 256 - wValue;
	}

	if (wValue < 30) {
		wValue = 30;
		/*        ASSERT(0);*/
	}

#endif

	if (g_u4SrcType != SRC_NULL) {
		vDrvSetHPosition(wValue);
	} else {
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief fgApiVideoGeoVPosition(u8 bPath, u16 wValue)
 * Set VGA V Position
 * @param  bPath :main/sub path (SV_VP_MAIN / SV_VP_PIP)
 * @param wValue  : V Position (not UI value)
 * @retval void
 * @example fgApiVideoGeoVPosition(SV_VP_MAIN, 30);
 */
u8 fgApiVideoGeoVPosition(u8 bPath, u16 wValue)
{

#if 0

	/*for 5371 mw if*/
	if (wValue > 511) {
		wValue = 511;
	}

	if (fgIsMainVga() || fgIsPipVga()) {
		wValue = Get_VGAMODE_IPV_STA(_bVgaTiming) - 1 + 256 - wValue;
	} else {
		wValue = Get_VGAMODE_IPV_STA(_bHdtvTiming) - 1 + 256 - wValue;
	}

	if (wValue < 2) {
		wValue = 2;
		/*        ASSERT(0);*/
	}

#endif


	if (g_u4SrcType != SRC_NULL) {
		vDrvSetVPosition(wValue);
	} else {
		return FALSE;
	}

	return TRUE;

}

/**
 * @brief fgApiVideoVgaAuto(void)
 * Trigger Auto state machine entry,Do it when path is VGA or DVI+ext AD
 * @param void
 * @retval void
 * @example vApiVideoVgaAuto(void);
 */
u8 fgApiVideoVgaAuto(void)
{
	/*By Adam to prevent the Vga auto postition error from tri-sync*/

	/*both vga timing & video timing call auto, and going to notify MW when done*/
	vDrvVgaAutoStart();
	return TRUE;
}

void vApiVgaAutoStop(void)
{
	vDrvVgaAutoStop();
}

u8 bApiVgaSigStatus(void)
{
	return bVgaSigStatus();
}

/**
 * @brief fgApiVideoVgaAuto(void)
 * Trigger Auto state machine entry,Do it when path is VGA or DVI+ext AD
 * @param void
 * @retval void
 * @example vApiVideoVgaAuto(void);
 */
u8 fgApiVideoYPbPrAuto(void)
{
	vDrvYPbPrAutoStart();
	return TRUE;
}

/**
 * @brief vDrvVideoAutoColor(void)
 * Trigger Auto Color state machine (VGA or YPbPr)
 * @param void
 * @retval u8
 * @example vDrvVideoAutoColor(void);
 */
void vDrvVideoAutoColor(void)
{
	if ((((g_u1Timing) >= HDTV_SEARCH_START) && ((g_u1Timing) <= VGA_SEARCH_END)) || fgIsValidTiming(g_u1Timing)) {
		vDrvIntAutoColorStart();
		pr_debug("[YBR_VGA] Doing Auto Color, Please input 100 percent Color Bar with White and Blank\r\n");
	}
}


/**
 * @brief wApiVideoGetVgaStdHtotal(void)
 * Get  Standard Htotal value of current VGA timing
 * @param void
 * @retval :Htotal value
 * @example wApiVideoGetVgaStdHtotal(void);
 */
u16 wApiVideoGetVgaStdHtotal(void)
{
	return Get_VGAMODE_IHTOTAL(_bVgaTiming); /* HTotal*/
}


/**
 * @brief wApiVideoGetVgaStdHPos(void)
 * Get  Standard H position value of current VGA timing
 * @param void
 * @retval :H position value
 * @example wApiVideoGetVgaStdHPos(void);
 */
u16 wApiVideoGetVgaStdHPos(void)
{
	return Get_VGAMODE_IPH_BP(_bVgaTiming); /* V Position*/
}


/**
 * @brief wApiVideoGetVgaStdVPos(void)
 * Get  Standard V position value of current VGA timing
 * @param void
 * @retval :V position value
 * @example wApiVideoGetVgaStdVPos(void);
 */
u16 wApiVideoGetVgaStdVPos(void)
{
	return Get_VGAMODE_IPV_STA(_bVgaTiming); /* H Position*/
}


void vDrvVideoSuspend(void)
{
	/**/
	vDrvVideoConnect(SV_OFF);
	vDrvAllHDADCPow(SV_OFF);
	vDrvSOY1EN(0);
	/*Clock*/
	vDrvYbrVgaClkDisable();
}

/* **********************************************************************/
/* Function :*/
/* Description :for resume*/
/* Parameter :*/
/* Return    :*/
/* **********************************************************************/
void vDrvVideoResume(void)
{
	vDrvVideoInit();
	initYPbPrVGA();
	vDrvVideoConnect(TRUE);
}

void vDrvVideoAuto(void)
{
	if (g_u4SrcType == SRC_YBR) {
		if (bGetSP0AutoState() == VDO_AUTO_NOT_BEGIN) {
			/* vDrvYPbPrAutoStart();*/
			pr_debug("[YPBPR_VGA]:YPbPr Source, Doing Auto Phase\r\n");
		}
	} else if (g_u4SrcType == SRC_VGA) {
		if (bGetSP0AutoState() == VDO_AUTO_NOT_BEGIN) {
			vDrvVgaAutoStart();
			pr_debug("[YPBPR_VGA]:VGA Source, Doing Auto\r\n");
		}
	} else {
		pr_debug("[YPBPR_VGA]:Unknown Source, do nothing\r\n");
	}
}




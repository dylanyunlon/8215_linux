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

#include <linux/jiffies.h>
#include <linux/sched.h>

#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/module.h>

#include "mhl_private.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"
#include "x_bim.h"
#include "drv_thread.h"
#include "x_timer.h"
#include "mhl_drv_if.h"
#include "hdmi_rx_ctrl.h"
#include "hdmi_rx_hal.h"
/* #include "x_gpio.h" */
#include "drv_av_d.h"
#include "hdmi_rx_aud_if.h"
#include "hdmi_rx_aud_task.h"
/* #include "drv_hdmi.h" */
/* #include "drv_hdmi_rx.h" */
#include "x_debug.h"
#include "video_timing.h"
/* #include "drv_vdoin.h" */
#include "mhl_rx_cbus_ctrl.h"
#include "drv_av_d.h"
#include "hdmi_hw_reg.h"
#include "hdmi_debug.h"
#include "winutil.h"
#include "wch_drv.h"
#include "wch_if.h"
#include "hdmi_rx_dvi.h"

#include "mhl_drv.h"
#include "mhl_mod.h"
#include <generated/atc_project.h>
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include "x_ioopt.h"
#endif

/****************************************************************************
** Local define  prototype
****************************************************************************/

#define ENABLE_DE_MODE 1

#define NEW_MODE_CHANGE_FLOW    1


#define HDMI_RTCK_AUTO_Release_FLOW     1
#define HDMI_WAIT_HDCP_AUTH             100


#define Diff(a, b)  (((a) > (b))?((a)-(b)):((b)-(a)))
UINT8  _bEQCH0, _bEQCH1, _bEQCH2;
extern unsigned long  g_IO_VBASE_VA;

enum {
	DVI_SEARCH_STATE = 0,
	DVI_WAIT_AUTO_STATE,
	DVI_MODE_CHG_DONE_STATE
};
enum {
	DVI_NO_SIGNAL,
	DVI_CHK_MODECHG,
	DVI_WAIT_STABLE
};

const CHAR *cDviChkState[3] = {
	"DVI_NO_SIGNAL",
	"DVI_CHK_MODECHG",
	"DVI_WAIT_STABLE",
};

const CHAR *cDviModeDetState[3] = {
	"DVI_SEARCH_STATE",
	"DVI_WAIT_AUTO_STATE",
	"DVI_MODE_CHG_DONE_STATE",
};


typedef enum {
	VSW_COMP_INT_HDMIRX = 0,
	VSW_COMP_EXT_HDMIRX,
	VSW_COMP_EXT_TVD

} VSW_HANDLE_COMP_ID_T;

typedef enum {
	VSW_COMP_NFY_ERROR = -1,
	VSW_COMP_NFY_LOCK = 1,
	VSW_COMP_NFY_UNLOCK,
	VSW_COMP_NFY_RESOLUTION_CHGING,
	VSW_COMP_NFY_RESOLUTION_CHG_DONE,
	VSW_COMP_NFY_ASPECT_CHG,
	VSW_COMP_NFY_COLOR_SPACE_CHG,
	VSW_COMP_NFY_JPEG_CHG,
	VSW_COMP_NFY_CINEMA_CHG
} VSW_NFY_COND_T;

typedef struct _VSW_NFY_INFO_T {
	VSW_HANDLE_COMP_ID_T  eCompId;
	VSW_NFY_COND_T eNfyCond;

} VSW_NFY_INFO_T;




/****************************************************************************
** Local variable prototype
****************************************************************************/

/* timing info, include vclk/hclk/width/height */
UINT8   _bDviVclk;
UINT16   _wDviHclk; /*  timing table: table value; otherwise, the value used by timing search */
UINT16   _wDviWidth;
UINT16   _wDviHeight;

/* detect timing info */
BOOL   _IsDviDetectDone;
UINT8   _bDviTiming = MODE_NOSIGNAL;
UINT32 _u4NotSupportCnt;


UINT32 _bDviMdChgCnt;  /*  hv total change */
UINT8 _bDviDeChgCnt;   /*  active solution change */
UINT8 _bDviPixClkChgCnt;  /*  pixel clock change */
UINT8 _bDviHVClkChgCnt;   /*  hsync, vsync frequency change */
UINT8 _bDviNoSigCnt;


UINT16 _wDviTmpHtotal;
UINT16 _wDviTmpVtotal;
UINT16 _wDviVTotal;
UINT16 _wDviHtotal;

UINT32 _wHDMIPixelClk;
/* UINT32 _u4VsyncPeriod; */
UINT8 _bPWOFFCnt;
static UINT8 _bInfoChgCnt;

UINT8   _bDviModeDetState;
UINT8  _bDviRetry;
UINT8  _bDviModeChged;
UINT8  _bXpcStable;


UINT8 _bDviChkState;
/* HAL_TIME_T _rDviModeChgTime; */
unsigned int _rDviModeChgTime;

#if ENABLE_DE_MODE
UINT8 bEnDVIDE = 0;/* SV_ON; */

#endif


#if HDMI_RTCK_AUTO_Release_FLOW
UINT8 dataXCLK1, dataXCLK2, readXCLKCnt;
UINT8 dataXCLK3;
UINT8 fgXlkStable;
UINT8 fgXlkStableCNT;
UINT32 _bDviMdChgXLKStableCnt;
UINT8 fgRTCKAuto;

#endif



UINT8 _bHResChgIntDetectFlg;  /* jiewen, 20090114 */

UINT8 _u1RxCatureTiming = MODE_NOSIGNAL;



PMX_RESOLUTION_MODE_T _bVDITiming;




/****************************************************************************
** Local function prototype
****************************************************************************/
static UINT16 DviIHSClock(void);
static void vDviPolarityUniform(void);
static void DviSetInputCapture(UINT8 bMode);
static UINT16 DviIHSClock(void);

const CHAR *szRxResStr[] = {
	"MODE_NOSIGNAL",        /* No signal     0 */
	"MODE_525I_OVERSAMPLE",
	"MODE_625I_OVERSAMPLE",
	"MODE_480P_OVERSAMPLE",
	"MODE_576P_OVERSAMPLE",
	"MODE_720p_50",
	"MODE_720p_60",
	"MODE_1080i_48",
	"MODE_1080i_50",
	"MODE_1080i",
	"MODE_1080p_24",
	"MODE_1080p_25",
	"MODE_1080p_30",
	"MODE_1080p_50",
	"MODE_1080p_60",
	"MODE_525I",
	"MODE_625I",
	"MODE_480P",
	"MODE_576P",
	"MODE_720p_24",
	"MODE_720p_25",
	"MODE_720p_30",
	"MODE_240P",
	"MODE_540P",
	"MODE_288P",
	"MODE_480P_24",
	"MODE_480P_30",
	"MODE_576P_25",
	"MODE_3D_720p_50",
	"MODE_3D_720p_60",
	"MODE_3D_1080p_24",
	"MODE_3D_1080I_60_FRAMEPACKING",
	"MODE_3D_1080I_50_FRAMEPACKING",
	"MODE_3D_1080P60HZ",
	"MODE_3D_1080P50HZ",
	"MODE_3D_1080P30HZ",
	"MODE_3D_1080P25HZ",
	"MODE_3D_720P30HZ",
	"MODE_3D_720P25HZ",
	"MODE_3D_720P24HZ",
	"MODE_3D_576P50HZ",
	"MODE_3D_576I50HZ",
	"MODE_3D_480P60HZ",
	"MODE_3D_480I60HZ",
	"MODE_REVERSE1",
	"MODE_REVERSE2",
	"MODE_HDMI_640_480P",
	"MODE_2160P_30HZ",
	"MODE_2160P_25HZ",
	"MODE_2160P_24HZ",
	"MODE_2161P_24HZ",

	/* MODE_MAX,*/
	/* MODE_DE_MODE = 252,*/
	/* MODE_NODISPLAY = 253,*/
	/* MODE_NOSUPPORT = 254,       Signal out of range*/
	/* MODE_WAIT = 255*/
};


const CHAR *szVinResStr[] = {
	"RES_480I=0",
	"RES_576I",
	"RES_480P",
	"RES_576P",
	"RES_480P_1440",
	"RES_576P_1440",
	"RES_480P_2880",
	"RES_576P_2880",
	"RES_720P60HZ",
	"RES_720P50HZ",
	"RES_1080I60HZ",
	"RES_1080I50HZ",
	"RES_1080P60HZ",
	"RES_1080P50HZ",
	"RES_1080P30HZ",
	"RES_1080P25HZ",
	"RES_480I_2880",
	"RES_576I_2880",
	"RES_1080P24HZ",
	"RES_1080P23_976HZ",
	"RES_1080P29_97HZ",
	"RES_3D_1080P23HZ",
	"RES_3D_1080P24HZ",
	"RES_3D_720P60HZ",
	"RES_3D_720P50HZ",
	"RES_3D_720P30HZ",
	"RES_3D_720P25HZ",
	"RES_3D_576P50HZ",
	"RES_3D_480P60HZ",
	"RES_3D_1080I60HZ",
	"RES_3D_1080I50HZ",
	"RES_3D_1080I30HZ",
	"RES_3D_1080I25HZ",
	"RES_3D_576I25HZ",
	"RES_3D_480I30HZ",
	"RES_3D_576I50HZ",
	"RES_3D_480I60HZ",
	"RES_2D_480I60HZ",
	"RES_2D_576I50HZ",
	"RES_2D_640x480P60HZ",
	"RES_PANEL_AUO_B089AW01",
	"RES_3D_720P60HZ_TB",
	"RES_3D_720P50HZ_TB",
	"RES_3D_1080I60HZ_SBS_HALF",
	"RES_3D_1080I50HZ_SBS_HALF",
	"RES_3D_1080P23HZ_TB",
	"RES_3D_1080P24HZ_TB",
	"RES_2160P_23_976HZ",
	"RES_2160P_24HZ",
	"RES_2160P_25HZ",
	"RES_2160P_29_97HZ",
	"RES_2160P_30HZ",
	"RES_2161P_24HZ",

	"RES_720P30HZ",
	"RES_720P25HZ",
	"RES_720P24HZ",
	"RES_720P23HZ",

	"RES_3D_1080P60HZ",
	"RES_3D_1080P50HZ",
	"RES_3D_1080P30HZ",
	"RES_3D_1080P29HZ",
	"RES_3D_1080P25HZ",
	"RES_3D_720P24HZ",
	"RES_3D_720P23HZ",

	"RES_3D_1080P60HZ_TB",
	"RES_3D_1080P50HZ_TB",
	"RES_3D_1080P30HZ_TB",
	"RES_3D_1080P29HZ_TB",
	"RES_3D_1080P25HZ_TB",
	"RES_3D_1080I60HZ_TB",
	"RES_3D_1080I50HZ_TB",
	"RES_3D_1080I30HZ_TB",
	"RES_3D_1080I25HZ_TB",
	"RES_3D_720P30HZ_TB",
	"RES_3D_720P25HZ_TB",
	"RES_3D_720P24HZ_TB",
	"RES_3D_720P23HZ_TB",
	"RES_3D_576P50HZ_TB",
	"RES_3D_576I25HZ_TB",
	"RES_3D_576I50HZ_TB",
	"RES_3D_480P60HZ_TB",
	"RES_3D_480I30HZ_TB",
	"RES_3D_480I60HZ_TB",

	"RES_3D_1080P60HZ_SBS_HALF",
	"RES_3D_1080P50HZ_SBS_HALF",
	"RES_3D_1080P30HZ_SBS_HALF",
	"RES_3D_1080P29HZ_SBS_HALF",
	"RES_3D_1080P25HZ_SBS_HALF",
	"RES_3D_1080P24HZ_SBS_HALF",
	"RES_3D_1080P23HZ_SBS_HALF",
	"RES_3D_1080I30HZ_SBS_HALF",
	"RES_3D_1080I25HZ_SBS_HALF",
	"RES_3D_720P60HZ_SBS_HALF",
	"RES_3D_720P50HZ_SBS_HALF",
	"RES_3D_720P30HZ_SBS_HALF",
	"RES_3D_720P25HZ_SBS_HALF",
	"RES_3D_720P24HZ_SBS_HALF",
	"RES_3D_720P23HZ_SBS_HALF",
	"RES_3D_576P50HZ_SBS_HALF",
	"RES_3D_576I25HZ_SBS_HALF",
	"RES_3D_576I50HZ_SBS_HALF",
	"RES_3D_480P60HZ_SBS_HALF",
	"RES_3D_480I30HZ_SBS_HALF",
	"RES_3D_480I60HZ_SBS_HALF",

	"RES_MODE_NUM",
	"RES_AUTO"
};



#define DVIGetVtotal()     HDMIVTotal()
#define DVIGetHtotal()     HDMIHTotal()
#define DVIGetHAct()       HDMIHsyncAct()
#define DVIGetWidth()      HDMIResoWidth()
#define DVIGetHeight()     HDMIResoHeight()






/****************************************************************************
** Local function
****************************************************************************/



static PMX_RESOLUTION_MODE_T ConvertHdmiRXResToVDORes(UINT8 u2Timing)
{
	PMX_RESOLUTION_MODE_T e_Res = RES_MODE_NUM;

#if 0
	BYTE bVFrontPorch = (BYTE)HDMI_HalGetVFrontPorch();


	switch (u2Timing) {

	case MODE_HDMI_640_480P:
		e_Res = RES_2D_640x480HZ;

		if (bVFrontPorch == 0x12) {
			Set640x480PEnable(2);/* VESA VGA */
		} else {
			Set640x480PEnable(1);/* CEA VGA */
		}

		break;

	case MODE_525I:
		e_Res = RES_480I;
		break;

	case MODE_625I:
		e_Res = RES_576I;
		break;


	case MODE_480P:
		e_Res = RES_480P;
		break;


	case MODE_576P:
		e_Res = RES_576P;
		break;


	case MODE_720p_50:
		e_Res = RES_720P50HZ;
		break;


	case MODE_720p_60:
		e_Res = RES_720P60HZ;
		break;

	case MODE_720p_30:
		e_Res = RES_720P30HZ;
		break;

	case MODE_720p_25:
		e_Res = RES_720P25HZ;
		break;

	case MODE_720p_24:
		e_Res = RES_720P24HZ;
		break;

	case MODE_3D_720p_50:
		e_Res = RES_3D_720P50HZ;
		break;

	case MODE_3D_720p_60:
		e_Res = RES_3D_720P60HZ;
		break;

	case MODE_3D_720P30HZ:
		e_Res = RES_3D_720P30HZ;
		break;

	case MODE_3D_720P25HZ:
		e_Res = RES_3D_720P25HZ;
		break;

	case MODE_3D_720P24HZ:
		e_Res = RES_3D_720P24HZ;
		break;

	case MODE_1080i_50:
		e_Res = RES_1080I50HZ;
		break;

	case MODE_3D_1080I_50_FRAMEPACKING:
		e_Res = RES_3D_1080I50HZ;
		break;

	case MODE_1080i:
		e_Res = RES_1080I60HZ;
		break;

	case MODE_3D_1080I_60_FRAMEPACKING:
		e_Res = RES_3D_1080I60HZ;
		break;

	case MODE_1080p_24:
		e_Res = RES_1080P24HZ;
		break;

	case MODE_3D_1080p_24:
		e_Res = RES_3D_1080P24HZ;
		break;

	case MODE_1080p_50:
		e_Res = RES_1080P50HZ;
		break;

	case MODE_1080p_60:
		e_Res = RES_1080P60HZ;
		break;

	case MODE_1080p_30:
		e_Res = RES_1080P30HZ;
		break;

	case MODE_3D_1080P30HZ:
		e_Res = RES_3D_1080P30HZ;
		break;

	case MODE_1080p_25:
		e_Res = RES_1080P25HZ;
		break;

	case MODE_3D_1080P25HZ:
		e_Res = RES_3D_1080P25HZ;
		break;

	case MODE_2160P_30HZ:
		e_Res = RES_2160P_30HZ;
		break;

	case MODE_2160P_25HZ:
		e_Res = RES_2160P_25HZ;
		break;

	case MODE_2160P_24HZ:
		e_Res = RES_2160P_24HZ;
		break;

	case MODE_2161P_24HZ:
		e_Res = RES_2161P_24HZ;
		break;

	default:
		e_Res = RES_MODE_NUM;
		break;


	}

#endif
	return  e_Res;
}
/**
 * @brief   Calculate hsync clock (27MHz domain) by measured HLEN/VLEN.
 * @param   None
 * @retval  hsync clock (measure in 27MHz domain)
 */
static UINT16 DviIHSClock(void)
{

	return HDMILineFreq();
}

/**
 * @brief   Calculate vsync clock (27MHz domain) by measured HLEN/VLEN.
 * @param   None
 * @retval  vsync clock (measure in 27MHz domain)
 */
UINT8 DviIVSClock(void)
{

	return HDMIRefreshRate();
}

/**
* @brief    vDVISetModeCHG(void);Flag responses mode change
* @param    None
* @retval   None
*/
void vDVISetModeCHG(void)
{
	if (!_bDviModeChged) {
		HalEnableRxPhyRtckAuto();
		Linux_HAL_GetTime((unsigned long *)&_rDviModeChgTime);

		_bDviModeChged = 1;
		_bPWOFFCnt = 0;
		_bXpcStable = 0;
	}
}

void DVISetModeDone(void)
{
	if (_bDviModeChged) {

		HalEnableRxPhyRtckAuto();
		HAL_Delay_us(2);/* vUtDelay2us(1); */
		HalDisableRxPhyRtckAuto();

		HDMI_HalClearModeChgIntState();

		HDMI_HalPhyReset(HDMI_RST_DEEPCOLOR);

		HDMI_HalResetTDFifoAutoRead();  /* jiewen, 20090114 */

		_bHResChgIntDetectFlg = 1;  /* jiewen, 20090114 */
		_bHDMIColorSpace = HDMIInputType(); /*color space */

		_bDviModeChged = 0;

	}
}



UINT32 dwDviPIXClock(void)
{

	return (DviIHSClock() * (HDMIHTotal()));
}

/**
 * @brief   Adjust DVI-related polarity setting according to sync process polarity measure result.
 * @param   None
 * @retval  None
 */
static void vDviPolarityUniform(void)
{

	if (bEnDVIDE == 1) {
#if 0/* kenny mark */
		vRegWrite4BMsk(CCIR_00, 0, Fld2Msk32(CCH_HS_POL));
		/* because h,v of  DVI decoder  is decoded  from DE signal , not inverse */
		vRegWrite4BMsk(CCIR_00, 0, Fld2Msk32(CCH_VS_POL));
		/* because h,v of  DVI decoder  is decoded  from DE signal , not inverse */
#endif
	}
}

/**
 * @brief   DVI Timing search function.
 * @param   bMode: 0 - VGA Timing Search, 1 - Video Timing Search
 * @retval  The found DVI timing or return MODE_NOSUPPORT
 */
UINT32 DviStdTimingSearch(UINT8 bMode, UINT16   wDviHclk, UINT8 bDviVclk, UINT16 wDviHtotal, UINT16 wPclk)
{
	UINT8 bSearch;
	UINT8 bSearchEnd;

	UNUSED(bMode);

	/* Video Mode */
	bSearch = 0;
	bSearchEnd = 100;/*HDTV_SEARCH_END; */

	HDMI_LOG(HDMI_LOG_INFO, "HsyncFreq(%d),VsyncFreq(%d),Htotal(%d),Vbp(%d) \r\n",
		 wDviHclk, bDviVclk, wDviHtotal, wPclk);

	/*setch hdmi timing */
	do {
		if ((bDviVclk >= (Get_HDMIMODE_IVF(bSearch) - 2)) &&
		    (bDviVclk <= (Get_HDMIMODE_IVF(bSearch) + 2))) {
			/*if ((wDviHclk >= (Get_HDMIMODE_IHF(bSearch) - 5)) &&
			(wDviHclk <= (Get_HDMIMODE_IHF(bSearch) + 5))) */
			if ((wDviHclk >= (Get_HDMIMODE_IHF(bSearch) - 30)) &&
			    (wDviHclk <= (Get_HDMIMODE_IHF(bSearch) + 30))) {
				if ((wDviHtotal > (Get_HDMIMODE_IHTOTAL(bSearch) - 40)) &&
				    (wDviHtotal < (Get_HDMIMODE_IHTOTAL(bSearch) + 40))) {
					if ((wPclk > (Get_HDMIMODE_ICLK(bSearch) - 50)) &&
					    (wPclk < (Get_HDMIMODE_ICLK(bSearch) + 50))) {
						/*print log, output the timing be searched. */
						HDMI_LOG(HDMI_LOG_DEBUG, "search timing:%d\r\n", bSearch);
						return bSearch;
					}
				}

			}
		}
	} while (++bSearch <= bSearchEnd);

	return MODE_NOSUPPORT;
}

UINT16 wDviGetTableHactive(void)
{
	UINT16 wHDMIHactive;

	wHDMIHactive = HDMIResoWidth();

	if (fgIsVideoTiming(_bDviTiming)) {
		if ((wHDMIHactive > (Get_HDMIMODE_IPH_WID(_bDviTiming) - 40))
		    && (wHDMIHactive < (Get_HDMIMODE_IPH_WID(_bDviTiming) + 40))) {
			return wHDMIHactive;
		} else {
			return Get_HDMIMODE_IPH_WID(_bDviTiming);
		}
	} else {
		return  wHDMIHactive;
	}
}
/**
 * @brief   DVI set input paramter function.
 * @param   bMode - DVI Timing.
 * @retval  None
 */
static void DviSetInputCapture(UINT8 bMode)
{

	/* In here, you should set CCIR in hardware */
	_u1RxCatureTiming =  bMode; /* kenny add */

}

/**
 * @brief   DVI initial function for each mode change or connect/disconnect.
 * @param   None
 * @retval  None
 */
void DviInitial(void)
{
	/* Software Initialize */
	/* _IsDviDetectDone = FALSE; */
	_bDviTiming = MODE_WAIT;
	_bDviModeDetState = DVI_SEARCH_STATE;

	_bDviMdChgCnt = 0;
	_bDviDeChgCnt = 0;
	_bDviPixClkChgCnt = 0;
	_bDviHVClkChgCnt = 0;
	_bDviNoSigCnt = 0;

	_u4NotSupportCnt = 0;
	/*  Hardware Initialize */

	if (bEnDVIDE == 1) {
		/* vEnterDEMode(SV_ON); */
	} else {
		/* vEnterDEMode(SV_OFF); */
	}

	/*  Set Polarity */
	vDviPolarityUniform();

	vDVISetModeCHG();

}

/* End*/

/**
 * @brief   DVI call-back function used by DI/Display/Scaler driver
 * @param   None
 * @retval  0 - progressive, 1 - interlace
 */
UINT8 bDviInterlace(void)
{
	if ((_IsDviDetectDone) && (_bDviTiming != MODE_NOSIGNAL) && (_bDviTiming < MAX_TIMING_FORMAT)) {
		return Get_HDMIMODE_INTERLACE(_bDviTiming);
	} else {
		return 0;
	}
}





UINT32 u4VsyncDeltaTime = 0;

UINT32 u4GetVsyncPeriod(void)
{
	return 0;/* u4VsyncDeltaTime;*/
}

BOOL HdmiIsWff1(void)
{
	BOOL fgIsEnable = FALSE;
	BOOL fgIsHdmi = FALSE;


	fgIsEnable = BASE_READ32(0x42300) & 0x1;

	if ((BASE_READ32(0x1F030) & (0x3 << 4)) == (0x0 << 4)) {
		fgIsHdmi = TRUE;
	} else {
		fgIsHdmi = FALSE;
	}

	/* RETAILMSG(1,(TEXT("(0x%x)(0x%x)\r\n"), fgIsEnable, fgIsHdmi));*/


	if (fgIsEnable && fgIsHdmi) {
		return TRUE;
	} else {
		return FALSE;
	}


}

BOOL HdmiIsWff2(void)
{
	BOOL fgIsEnable = FALSE;
	BOOL fgIsHdmi = FALSE;


	fgIsEnable = BASE_READ32(0x42500) & 0x1;

	if ((BASE_READ32(0x1F030) & (0x3 << 8)) == (0x0 << 8)) {
		fgIsHdmi = TRUE;
	} else {
		fgIsHdmi = FALSE;
	}


	if (fgIsEnable && fgIsHdmi) {
		return TRUE;
	} else {
		return FALSE;
	}


}




irqreturn_t HdmiRxIrqHandler(UINT32 u2Vector, void *dev_id)
{

	BOOL fgClockLoss = FALSE;

	/* printk("receive irq \r\n"); */
	if (HalIsINTR_VSYNC()) {
		HDMI_HalClearVSYNCIntStatus();

		HalHDMIRxEnableVsyncInt(FALSE);

#if 0

		/*just for source timing to wch.*/
		if ((uStartParam & 0x1) == 0) {
			UINT32 idx = ((uStartParam & 0x000f0000) >> 16);

			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI]HDMI Vysnc start jiffies : %d idx : %d\r\n", jiffies, idx);
			uStartParam = 0x1;
			WchHalStart(idx);
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI]HDMI Vsync end jiffies : %d \r\n", jiffies);
			wake_up_interruptible(&wchqueue);
		}

#endif
		/* printk("HDMI Vysnc \r\n"); */
	}

	/* BIM_ClearIrq(VECTOR_HDMIRXINT); */
	if (HalIsINTR2_CKDT()) {
		/* lost CLOCK */
		HDMI_LOG(HDMI_LOG_DEBUG, "    clr ckdt\r\n");

		if (HDMI_HalGetCKDT() == FALSE) {
			/* vHalEnableINTR2_CKDT(FALSE); */
			HalEnableINTR2_CKDT(FALSE);
			fgClockLoss = TRUE;
		}

		HalClearINTR2_CKDT();
	}

	if (fgClockLoss) {
#if 0

		if (HdmiIsWff1()) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI] Wch 1, close it \r\n");
			WchHalDeinit(0);
		} else if (HdmiIsWff2()) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI] Wch 2, close it \r\n");
			WchHalDeinit(1);
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI] No Wch open \r\n");
		}

#endif

		/*if (0 != WchStopByInputSrc(SRC_APP_HDMI)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "Stop HDMI Wch Fail\r\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "Stop HDMI Wch Success\r\n");
		}*/

		fgClockLoss = FALSE;
	}

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	ac83xx_mask_ack_bim_irq(u2Vector);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	mt33xx_mask_ack_bim_irq(u2Vector);
	#endif
	return IRQ_HANDLED;
}




/**
 * @brief   When DVI signal is not stable, check signal activity and then get hlen/vlen for timing search.
 * @param   None
 * @retval  None
 */
void DviModeDetect(void)
{
	/* if (fgIsExtSrcHDMI() && !((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF))) */
	if (!((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF))) {
		return;
	}

	switch (_bDviModeDetState) {
	/* Timing Search State */
	case DVI_SEARCH_STATE:
		HDMI_LOG(HDMI_LOG_DEBUG, "SEARCH_STATE\r\n");
		HDMI_HalDisableRxAvMute();

		/* check SCDT */
		if (!HDMI_HalGetSCDT()) {
			_bDviTiming = MODE_NOSIGNAL;
		} else {
			/* search timing */
			_wDviHclk = DviIHSClock();
			_bDviVclk = DviIVSClock();

			_bDviTiming =  DviStdTimingSearch(1, _wDviHclk, _bDviVclk, _wDviHtotal, 0);
		}

		if (_bDviTiming == MODE_NOSUPPORT) {
			if (_u4NotSupportCnt++ < 90) {
				break;
			}
		}

		_bDviModeDetState = DVI_MODE_CHG_DONE_STATE;

		/*  Mode found among timing table */
		if ((_bDviTiming > MODE_NOSIGNAL) && (_bDviTiming < MAX_TIMING_FORMAT)) {
			vDviPolarityUniform();/* set H/V Polarity */

			_wDviHclk = Get_HDMIMODE_IHF(_bDviTiming);
			_bDviVclk = Get_HDMIMODE_IVF(_bDviTiming);
			_bDviModeDetState = DVI_WAIT_AUTO_STATE;

		}

		_bDviRetry = 0;
		_wDviWidth =  DVIGetWidth();
		_wDviHeight =  DVIGetHeight();

		break;

	/*  Wait DVI Auto Done state */
	case DVI_WAIT_AUTO_STATE:

		if ((_wDviWidth != DVIGetWidth()) || (_wDviHeight != DVIGetHeight())) {
			if (_bDviRetry++ < 10) {
				_wDviWidth =  DVIGetWidth();
				_wDviHeight =  DVIGetHeight();
				break;
			}
		}

		_bDviModeDetState = DVI_MODE_CHG_DONE_STATE;
		break;

	/*  Mode Chg Done State */
	case DVI_MODE_CHG_DONE_STATE:
		HDMI_LOG(HDMI_LOG_DEBUG, "CHG DOWN \r\n");
		HDMI_HalEnableAvMuteRecv();
		/* vHDMISetColorRalated(); */


		_bDviChkState = DVI_CHK_MODECHG;
		HalHdmiAcrRst();
		SwitchAudioState(ASTATE_RequestAudio);

		/*  set input capture */
		DviSetInputCapture(_bDviTiming);

		/* change to video mode resolution */
		_bVDITiming = ConvertHdmiRXResToVDORes(_bDviTiming);

		HalEnableINTR2_CKDT(TRUE);

		DVISetModeDone();
		HDMIRXColorSpaceConveter();

		_IsDviDetectDone = TRUE;
		_bDviModeDetState = DVI_SEARCH_STATE;
		break;

	default:
		break;
	}

}



/*************************************************************/
/* #define HANDLE unsigned int */


void HdmiPowerEventNotify(void)
{
	if (NULL == g_hEvent_Power) {
		/* g_hEvent_Power = CreateEvent(NULL, FALSE, FALSE, g_rMhlConfig.szPowerEvent); */
	}

	/*HDMI_ResetGlobalVideoInfo();*/

	/*SetEvent(g_hEvent_Power);
	HDMI_LOG(HDMI_LOG_INFO, "Hdmi power [SetEvent]\r\n");*/
}

HDMI_SIG_INFORMATION signal_infor;

void HdmiTimingEventNotify(unsigned int status)
{
	/*if (NULL == g_hEvent_Timing) {
		HDMI_LOG(HDMI_LOG_ERROR, "g_hEvent_Timing is NULL \r\n");
		return;
	}*/

	/* set videoinfo */
	HDMI_ResetGlobalVideoInfo(status);

	HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState) start\r\n");

	if (register_isr) {
		switch (status) {
		case 0:
			signal_infor.signal_state = HDMI_SIG_LOST;
			isr_data.isr(&signal_infor);
			HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState): HDMI_SIG_LOST!\n");
			break;
		case 1:
			signal_infor.signal_state = HDMI_SIG_READY;
			isr_data.isr(&signal_infor);
			HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState): HDMI_SIG_READY!\n");
			break;
		case 3:
			signal_infor.signal_state = HDMI_SIG_CONNECTING;
			isr_data.isr(&signal_infor);
			HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState): HDMI_SIG_CONNECTING!\n");
			break;
		default:
			HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState): other signal, not send!\n");
		}
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState):register_isr = false \r\n");
	}

	/*SetEventData(g_hEvent_Timing, status);
	SetEvent(g_hEvent_Timing);
	HDMI_LOG(HDMI_LOG_INFO, "Hdmi timing [SetEvent]\r\n");*/
}


void HDMIPowerDetextExt(void)
{
	static BOOL fgPreStable = FALSE;
	static BOOL fgCurStable = FALSE;

	fgCurStable = HdmiIsPwr5vStable();

	if ((fgPreStable == FALSE) && (fgCurStable == TRUE)) {
		/* HdmiPowerEventNotify(); */
		HDMI_LOG(HDMI_LOG_INFO, "PowerEventNotify(power on...)\r\n");
	} else if ((fgPreStable == TRUE) && (fgCurStable == TRUE)) {
		/**/
	} else if ((fgPreStable == TRUE) && (fgCurStable == FALSE)) {
		/* HdmiPowerEventNotify(); */
		HDMI_LOG(HDMI_LOG_INFO, "PowerEventNotify(power off...)\r\n");
	} else
		;

	fgPreStable = fgCurStable;
}




static UINT32 g_u4TimingId;



UINT32 HdmiGetTimingID(void)
{
	return g_u4TimingId;
}

void vDviModeDetectExt(void)
{
	static BOOL fgPreStable = FALSE;
	static BOOL fgCurStable = FALSE;
	UINT32 u4TimingId = 0;

	UINT32 u4PixelFreq = 0;
	UINT32 u4Htotal = 0;
	UINT32 u4Vtotal = 0;
	UINT32 u4HsyncFreq = 0;
	UINT32 u4VsyncFreq = 0;
	UINT16 wVbp = 0;
	UINT32 u4DeepColor = 0;
	/*HDMI_SIG_INFORMATION signal_infor;*/

	fgCurStable = HdmiIsTimingStable();/*  && HdmiIsPclkStable(); */
	/*DVD 480/576 output may loss plk sometimes. */

	if ((fgPreStable == FALSE) && (fgCurStable == TRUE)) {
		HDMI_HalDisableRxAvMute();
		u4PixelFreq = HDMI_HalGetPixelClockExt();
		u4Htotal = HDMI_HalGetHtotalExt();
		u4Vtotal = HDMI_HalGetVTotal();
		u4HsyncFreq = (u4Htotal != 0) ? (u4PixelFreq / u4Htotal) : 0;
		u4VsyncFreq = ((u4Vtotal != 0) && (u4Htotal != 0)) ? (u4PixelFreq / u4Vtotal / u4Htotal) : 0;
		wVbp = HDMI_HalGetVBackPorch();
		/* get Timing ID */
		u4TimingId = DviStdTimingSearch(1, u4HsyncFreq / 100, u4VsyncFreq,
						u4Htotal, u4PixelFreq / (1000 * 100));
		g_u4TimingId = u4TimingId;

		HDMI_LOG(HDMI_LOG_INFO,
			 "PixelFreq(%d),HsyncFreq(%d),VsyncFreq(%d),Htotal(%d),Vtotal(%d), Vbp(%d), TimingID(%d) \r\n",
			 (int)u4PixelFreq, (int)u4HsyncFreq, (int)u4VsyncFreq,
			 (int)u4Htotal, (int)u4Vtotal, (int)wVbp, (int)u4TimingId);

		u4DeepColor = HDMI_HalGetDeepColorBpp();

		switch (u4DeepColor) {
		case 0:
			HDMI_LOG(HDMI_LOG_INFO, " Deep color: 24bit \r\n");
			break;

		case 1:
			HDMI_LOG(HDMI_LOG_INFO, " Deep color: 30bit \r\n");
			break;

		case 2:
			HDMI_LOG(HDMI_LOG_INFO, " Deep color: 36bit \r\n");
			break;

		case 3:
			HDMI_LOG(HDMI_LOG_INFO, " Deep color: 48bit \r\n");
			break;

		}

		/* set videoinfo, set event */
#if 0
		if (HdmiIsHdcpStable() || (!sink_fg_phone_support_hdcp())) {
			HDMI_LOG(HDMI_LOG_INFO, " g_u4HdcpStableCnt stable? = %d\r\n", (int)g_u4HdcpStableCnt);
			fgNotifySignal = TRUE;

			if (fgResume) {
				msleep(1000);
			}

			/* Just a patch for resume system.... because the data isn't correcttly. */

			fgResume = FALSE;
			/*msleep(100);*//*Fix green screen for SAMSUNG Note4*/
			HdmiTimingEventNotify(1);
			HDMI_LOG(HDMI_LOG_INFO, "isr_data.isr(&g_u4HdmiState) start\r\n");
			HDMI_LOG(HDMI_LOG_INFO, "TimingEventNotify(signal on...)\r\n");
		}
#endif
	} else if ((fgPreStable == TRUE) && (fgCurStable == TRUE)) {

		if (!fgNotifySignal) {
			HDMI_LOG(HDMI_LOG_DEBUG, "~");
			HDMI_LOG(HDMI_LOG_DEBUG, " g_u4HdcpStableCnt2 = %d\r\n", (int)g_u4HdcpStableCnt);

			/*if (HdmiIsHdcpStable() || (!sink_fg_phone_support_hdcp())) {*/
			if (u4StableCount > 70 || (!sink_fg_phone_support_hdcp())) {
			    if (HdmiIsHdcpStable() || (!sink_fg_phone_support_hdcp())) {
				HDMI_LOG(HDMI_LOG_DEBUG, "HDCP Auth done \r\n");

				if (fgResume) {
					msleep(1000);
				}

				/* Just a patch for resume system....because the data isn't correcttly. */

				fgResume = FALSE;
				HdmiTimingEventNotify(1);
				fgNotifySignal = TRUE;
                            } else {
				u4StableCount++;
                            }
			} else {
				u4StableCount++;
			}

			if (u4StableCount == HDMI_WAIT_HDCP_AUTH) {
				HDMI_LOG(HDMI_LOG_DEBUG, "Wait so long time \r\n");
				HdmiTimingEventNotify(1);
				fgNotifySignal = TRUE;
			}
		}

	} else if ((fgPreStable == TRUE) && (fgCurStable == FALSE)) {
		g_u4TimingId = MODE_NOSUPPORT;

		/*  set videoinfo, set event */
		if (HDMI_DrvGetStart()) {
			HDMI_LOG(HDMI_LOG_INFO, "TimingEventNotify(signal loss...)\r\n");
			HdmiTimingEventNotify(0);
			fgNotifySignal = FALSE;
			u4StableCount = 0;
		} else {
			HDMI_LOG(HDMI_LOG_INFO, "Already stop \r\n");
		}

	} else
		;

	fgPreStable = fgCurStable;
}






/**
 * @brief   Polling function for mode change every output vsync
 * @param   None
 * @retval  None
 */

void DviChkModeChange(void)
{
	UINT16 wvtemp;
	UINT16 whtemp;
	UINT32 u4DviHclkTemp;
	UINT32 u4DviVclkTemp;


	unsigned int rCurTime;

	UINT8 bEQCH0_tmp, bEQCH1_tmp, bEQCH2_tmp;

	/*  check unstable than time out for mode detect done */
	if (0) { /* _bDviModeChged) */
		HDMI_LOG(HDMI_LOG_INFO, "dvi mode chged \r\n");
		Linux_HAL_GetTime((unsigned long *)&rCurTime);

		if ((rCurTime - _rDviModeChgTime) > _wDVI_WAIT_NOSIGNAL_COUNT * 50) {
			if (DVIGetHAct()) { /* True: DE enbale */
				_bDviTiming = MODE_NOSUPPORT;
			} else {
				_bDviTiming = MODE_NOSIGNAL;
			}

			_wDviHtotal = 0;
			_wHDMIPixelClk = 0;
			/* _bDviTiming = MODE_NOSIGNAL; */
			_bDviMdChgCnt = 0;
			_bDviChkState = DVI_NO_SIGNAL;

			DVISetModeDone();
		}
	}

	/* if (fgIsExtSrcHDMI() && !((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF))) */
	if (!((_bHDMIState == HDMI_STATE_AUTH) || (_bHDMIState == HDMI_STATE_PWOFF))) {
		return;
	}

	/*  plug-out  event */
	if (_bUnplugFlag) {
		/* HDMI_LOG(HDMI_LOG_DEBUG,"why here? \r\n"); */
		_bUnplugFlag = 0;

		DviInitial();
		/* vDVISetModeCHG(); */
		_wDviHtotal = 0;
		_wHDMIPixelClk = 0;
		_bDviTiming = MODE_NOSIGNAL;
		_bDviMdChgCnt = 0;
		_bDviChkState = DVI_NO_SIGNAL;

		SwitchAudioState(ASTATE_AudioOff);
		DVISetModeDone();

	}

	/* detect signal_lost */
	if (_bDviChkState != DVI_NO_SIGNAL) {
		if (!DVIGetHAct()) {
			_bDviNoSigCnt++;

			if (_bDviNoSigCnt >= 200) {
				HDMI_LOG(HDMI_LOG_INFO, "to no signal \r\n");
				DviInitial();

				DVISetModeDone();
				_wDviHtotal = 0;
				_wHDMIPixelClk = 0;
				_bDviTiming = MODE_NOSIGNAL;
				_bDviMdChgCnt = 0;
				_bDviChkState = DVI_NO_SIGNAL;
			}
		} else {
			_bDviNoSigCnt =  0;
		}
	}



	whtemp = DVIGetHtotal();
	wvtemp = DVIGetVtotal();
	u4DviHclkTemp = DviIHSClock();
	u4DviVclkTemp = DviIVSClock();

	switch (_bDviChkState) {
	case DVI_NO_SIGNAL:

		HDMIEnable(FALSE);

		if (DVIGetHAct()) {
			HDMI_LOG(HDMI_LOG_WARN, "No signal -> wait \r\n");
			_bDviChkState = DVI_WAIT_STABLE;
			_bDviMdChgCnt = 0;

			return;
		} /*else {

			_bPWOFFCnt++;

			if (_bDviModeChged && (_bPWOFFCnt > 3)) {
				_bPWOFFCnt = 0;
				_bDviTiming = MODE_NOSIGNAL;
				DVISetModeDone();
			}
		}*/

		break;

	case DVI_CHK_MODECHG:
		if (HDMI_HalIsHResChg() && _bHResChgIntDetectFlg) {
			HDMI_HalPhyReset(HDMI_RST_DEEPCOLOR);
			HDMI_HalResetTDFifoAutoRead();
			HDMI_HalClearHresChgIntrState();

			_bHResChgIntDetectFlg = 0;

		}

		if (HDMIInputType() != _bHDMIColorSpace) {
			SwitchAudioState(ASTATE_AudioOff);
			HDMIRXColorSpaceConveter();
			HDMI_LOG(HDMI_LOG_WARN, "Color space change \r\n");
			_bDviChkState = DVI_WAIT_STABLE;
			DviInitial();
		}


		/* check avi info */
		if ((_bHDMIScanInfo != HDMIScanInfo()) ||
		    (_bHDMIAspectRatio != HDMIAspectRatio()) ||
		    (_bHDMIAFD != HDMIAFD()) ||
		    (_bHDMI422Input != HDMI422Input()) ||
		    (_bHDMIITCFlag != HDMIITCFlag()) ||
		    (_bHDMIITCContent != HDMIITCContent()) ||
		    (_bHDMIHDCPStatus != HDMIHDCPStatusGet())
		   ) {
			if (_bInfoChgCnt++ > 3) {
				if ((_bHDMIAspectRatio != HDMIAspectRatio()) || (_bHDMIAFD != HDMIAFD())) {
					_bHDMIScanInfo = HDMIScanInfo();
					_bHDMIAspectRatio = HDMIAspectRatio();
					_bHDMIAFD = HDMIAFD();

				}

				_bInfoChgCnt = 0;

				if (_bHDMI422Input != HDMI422Input()) {
					_bHDMI422Input = HDMI422Input();

					HDMIRXColorSpaceConveter();
				}

				_bHDMIHDCPStatus = HDMIHDCPStatusGet();
				_bHDMIITCFlag = HDMIITCFlag();
				_bHDMIITCContent = HDMIITCContent();
			}
		} else {
			_bInfoChgCnt = 0;
		}

		/*  judge htotal vtotal */
		if (((whtemp >= (_wDviHtotal - 5)) && (whtemp <= (_wDviHtotal + 5))) &&
		    ((wvtemp >= (_wDviVTotal - 2)) && (wvtemp <= (_wDviVTotal + 2)))) {
			_bDviMdChgCnt = 0;
		} else {
			_bDviMdChgCnt++;
		}

		/*  judge hclk, vclk */
#if 0

		if ((((signed int)u4DviHclkTemp >= (_wDviHclk - 10)) &&
		     ((signed int)u4DviHclkTemp <= (_wDviHclk + 10))) &&
		    (((signed int)u4DviVclkTemp >= (_bDviVclk - 2)) &&
		    ((signed int)u4DviVclkTemp <= (_bDviVclk + 2)))) {
			_bDviHVClkChgCnt = 0;
		} else {
			_bDviHVClkChgCnt++;
		}

#endif

#if 1

		/*  judge H active, V active (width x height) */
		if (((DVIGetWidth() >= (_wDviWidth - 3)) && (DVIGetWidth() <= (_wDviWidth + 3))) &&
		    ((DVIGetHeight() >= (_wDviHeight - 3)) && (DVIGetHeight() <= (_wDviHeight + 3)))) {
			_bDviDeChgCnt = 0;
		} else {
			_bDviDeChgCnt++;
		}

#endif

		/* judge pclk */
		if ((dwDviPIXClock() >= (_wHDMIPixelClk - (_wHDMIPixelClk / 10))) &&
		    (dwDviPIXClock() <= (_wHDMIPixelClk + (_wHDMIPixelClk / 10)))) {
			_bDviPixClkChgCnt = 0;
		} else {
			_bDviPixClkChgCnt++;
		}


		if (((_bDviMdChgCnt > 1)  || (_bDviDeChgCnt > 1) ||
		     (_bDviPixClkChgCnt > 5)) || (_bHdmiMD != _bHdmiMode) || (_bDviHVClkChgCnt > 5)) {
			/* SwitchAudioState(ASTATE_AudioOff); */
			/* HDMI_LOG(HDMI_LOG_WARN, "Mode change  %d, %d, %d, %d\r\n",
			_bDviMdChgCnt, _bDviDeChgCnt, _bDviPixClkChgCnt, _bDviHVClkChgCnt); */
			HdmiRxPacketDataInit();   /* zhiqiang add temp */

			HDMI_HalPhyReset(HDMI_RST_ALL);
			HDMI_HalReset();

			/* msleep(100); */

			/*Because Master 1025D's resolution  change is HDMI->DVI->HDMI*/
			if (_bHdmiMD != _bHdmiMode) {
				HDMI_LOG(HDMI_LOG_WARN, "HDMI Mode change \r\n");
				_bHdmiMD = _bHdmiMode;
			}

			_bDviChkState = DVI_WAIT_STABLE;
			HalDisableRxPhyRtckAuto();


#if HDMI_RTCK_AUTO_Release_FLOW
			fgXlkStable = 0;
			fgXlkStableCNT = 0;
			_bDviMdChgXLKStableCnt = 0;
			fgRTCKAuto = 0;
#endif
			_bDviMdChgCnt = 0;
			/* if (fgIsVgaTiming(_bDviTiming ) || fgIsVideoTiming(_bDviTiming)) */
			{

				/* DviInitial(); */
			}

		}

		/* vHDMIRxAudMainTask(); //20ms   no audio at first~ */

		break;

	case DVI_WAIT_STABLE:

		/*  if CLK change */
		if (HDMI_HalCheckIsPclkChanged()) {

			HDMI_HalPhyReset(HDMI_RST_DEEPCOLOR);

			HDMI_HalSetTDFifoAutoReadEnable(TRUE);
			HAL_Delay_us(2);/* vUtDelay2us(1); */

			HDMI_HalSetTDFifoAutoReadEnable(FALSE);
			HAL_Delay_us(100);/* vUtDelay2us(1); */
			HDMI_HalClearPclkChangedIntState(); /*  JOSH */

		} else if (_bXpcStable == 0) {
			/* if (_bIsXpcStable()) */
			if (HdmiIsPclkStable()) {
				_bXpcStable = 1;

				HDMI_HalPhyReset(HDMI_RST_DEEPCOLOR);

				HalEnableRxPhyRtckAuto();
				HDMI_HalSetTDFifoAutoReadEnable(TRUE);
				HAL_Delay_us(2);/* vUtDelay2us(1); */
				HalDisableRxPhyRtckAuto();
				HDMI_HalSetTDFifoAutoReadEnable(FALSE);
			}
		}




		if (((whtemp >= (_wDviTmpHtotal - 2)) && (whtemp <= (_wDviTmpHtotal + 2))) &&
		    ((wvtemp >= (_wDviTmpVtotal - 2)) && (wvtemp <= (_wDviTmpVtotal + 2)))) {

			/* if ((RegReadFldAlign(HDMI_CH0_EQ_STATUS, RG_HDMI_CH0_STATUS)&0x100) == 0x100) */
			if ((HDMI_READ32(REG_ANA_STAT0) & 0xffff & 0x100) == 0x100) {
				/* read eq */
				bEQCH0_tmp = 0;
				bEQCH1_tmp = 0;
				bEQCH2_tmp = 0;
				/* bEQCH0_tmp = RegReadFldAlign(HDMI_CH0_EQ_STATUS, RG_HDMI_CH0_EQERR); */
				/* bEQCH1_tmp = RegReadFldAlign(HDMI_CH1_EQ_STATUS, RG_HDMI_CH1_EQERR); */
				/* bEQCH2_tmp = RegReadFldAlign(HDMI_CH2_EQ_STATUS, RG_HDMI_CH2_EQERR); */
				HDMI_LOG(HDMI_LOG_DEBUG,
					 "==== EQ_temp: 0x%x, 0x%x, 0x%x ====\n", bEQCH0_tmp, bEQCH1_tmp, bEQCH2_tmp);

				if ((Diff(bEQCH0_tmp, _bEQCH0) >= 3) ||
				    (Diff(bEQCH1_tmp, _bEQCH1) >= 3) || (Diff(bEQCH2_tmp, _bEQCH2) >= 3)) {
					_bEQCH0 = bEQCH0_tmp;
					_bEQCH1 = bEQCH1_tmp;
					_bEQCH2 = bEQCH2_tmp;
					HDMI_HalPhyReset(HDMI_RST_EQ);
					HDMI_LOG(HDMI_LOG_DEBUG,
						 "mc: Reset HDMI PHY EQ again #1.....................\n");
				}/* else if ((Diff(bEQCH0_tmp, bEQCH1_tmp) >= 3) ||        can not be true
					   (Diff(bEQCH0_tmp, bEQCH2_tmp) >= 3) || (Diff(bEQCH1_tmp, bEQCH2_tmp) >= 3)) {
					_bEQCH0 = bEQCH0_tmp;
					_bEQCH1 = bEQCH1_tmp;
					_bEQCH2 = bEQCH2_tmp;
					HDMI_HalPhyReset(HDMI_RST_EQ);
					HDMI_LOG(HDMI_LOG_DEBUG,
						 "mc: Reset HDMI PHY EQ again #2.....................\n");
				}*/
			}



			if (_bHdmiMD != _bHdmiMode) {
				_bHdmiMD = _bHdmiMode;
			}

			_wDVI_WAIT_STABLE_COUNT = 3;/* old is 100,zhiqiang modify */

			if (_bDviMdChgCnt++ > _wDVI_WAIT_STABLE_COUNT) {
				HalHDMIRxEnableVsyncInt(TRUE);

				if (_bDviMdChgCnt++ > (_wDVI_WAIT_STABLE_COUNT + 60)) {
					/*  do nothing */
				}



				if (HDMI_HalIsVResStable())
					/* if(RegReadFldAlign(VID_VRES,VID_VRES_STB) || \
					(RegReadFldAlign(VID_MODE,ENDITHER)==1)) */
				{

					if (HDMI_HalIsVResMute()) {
						HDMI_HalSetVResMute();
						HDMI_HalClearVResMute();
					}



					DviInitial();
					_IsDviDetectDone = FALSE;
					_wDviHtotal = DVIGetHtotal();
					_wDviVTotal = DVIGetVtotal();
					_wHDMIPixelClk = dwDviPIXClock();
					HDMI_LOG(HDMI_LOG_WARN, "htotal:%d vtotal:%d \r\n", _wDviHtotal, _wDviVTotal);
					HalDisableRxPhyRtckAuto();
					DVISetModeDone();
					_bDviChkState = DVI_CHK_MODECHG;
					/* SetEvent(g_hEvent_Timing); */
					HdmiTimingEventNotify(1);
				}
			}

		} else {
			HDMI_HalPhyReset(HDMI_RST_RTCK);
			HDMI_HalPhyReset(HDMI_RST_DEEPCOLOR);
			/* HDMI_HalPhyReset(HDMI_RST_ALL); */
			_bDviMdChgCnt = 0;
#if HDMI_RTCK_AUTO_Release_FLOW
			_bDviMdChgXLKStableCnt = 0;
#endif
		}

		_wDviTmpHtotal = whtemp;
		_wDviTmpVtotal = wvtemp;

		break;

	default:
		break;
	}
}



UINT8 GetRxCapturedTiming(void)
{
	return _u1RxCatureTiming;
}


BOOL CheckRxDetectDone(void)
{
	return _IsDviDetectDone;
}





void ShowRxResoInfoStatus(void)
{
}


void HdmiRxDviStatus(void)
{
}



UINT8 HDMI3DPacketVaild(void)
{
	/* not support */
	return 0;

}









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

#include <generated/atc_project.h>
#include "vga_auto.h"


/******************************************************************************
 * Macro, Type Definition, and Variable Declaratoin
 *****************************************************************************/

u8   _bAutoFlag;
u8   _bVdoSP0AutoState;
/*u32    _dSumTmp[3];*/
u16   _wVgaHStart;
u16   _wVgaHEnd;
u16   _wVgaVStart;
u16   _wVgaVEnd;
u8   _fgIsAuto0PosFlg;

u8   _bCurPhase, _bBestPhase;
u8   _bClkCnt;

u16   _wVgaClock;
u8   _bVgaDelayCnt;
u16   _wCurClk, _wEndClk;

u16  _wBestClk;
u32  _dBestSum;
u16  _wOrgClock;
u8  _bOrgPhase;
u8   _bAutoClockFail = 0;


u8   _bBdThrsh = BDTHRSH;

#if VGA_AUTO_SPEEDUP
u8  _bAutoISR;
#endif
#if VGA_HW_AUTO
u8 _bWaitHwRdy;
u8 _bHwAuto = 1;
#endif

/*
#if SUPPORT_VGA_AMBIGUOUS_H_DETECT
extern u32 _dPhaseDiff;
#endif
*/
#define BD_RETRY 80
#define VGA_TIMEDELAY_DBG 0


#if VGA_TIMEDELAY_DBG
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "x_timer.h"
#endif
static HAL_TIME_T cur_time[8];
#endif

u32 _u4VPorchChanged = 0;

/******************************************************************************
 * Function Forward Declaration
 *****************************************************************************/
/**
 * @brief bVGAAutoGetBoundary( void )
 * Wait Input Vsync to Read R/G/B Channel Bounadry of SP0
 * @param  void
 * @retval AUTO_CONTINUED Continue to get Bounadary
 * @retval AUTO_FINISHED Get Bounadary Finished
 * @pre  Need to call vVgaAutoPosInit() to Initialize setting for Auto Boundary
 * @example if (bVGAAutoGetBoundary() == AUTO_FINISHED)
 */
void vVGAAutoGetBoundary(void)
{

#if VGA_AUTO_POS_DBG
	pr_debug("Auto Pos debug <==\r\n");
#endif

	_wVgaHStart = wDrvVGAGetLeftBound();
	_wVgaVStart = wDrvVGAGetTopBound();
	_wVgaHEnd = wDrvVGAGetRightBound();
	_wVgaVEnd = wDrvVGAGetBottomBound();
#if VGA_AUTO_POS_DBG
	pr_debug("LeftBound:%d\r\n", _wVgaHStart);
	pr_debug("TopBound:%d\r\n", _wVgaVStart);
	pr_debug("RightBound:%d\r\n", _wVgaHEnd);
	pr_debug("BottomBound:%d\r\n", _wVgaVEnd);
#endif
}

/**
 * @brief  vVgaAutoPosInit(void)
 * VGA auto position initialize function
 * @param  void
 * @retval void
 * @example vVgaAutoPosInit()
 */
void vVgaAutoPosInit(void)
{
	vSetSp0Auto(SP0_H);
	vSetSp0Auto(SP0_V);
	vSetAsyncMeasureBD(DOMAIN_PIXEL);
}

/**
 * @brief  vVgaAutoInit(void)
 * VGA auto initialize function
 * @param  void
 * @retval void
 * @example vVgaAutoInit()
 */
void vVgaAutoInit(void)
{
	/*Set phase , phase mode, and boundary threshold*/
	vVGAPhaseModeSet(Bmode); /*sc_yang for phase mode init*/
	vDrvVGASetBDDataTh(64);
	vClrAutoFlg(SP0_AUTO_ALL);
	vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);
	/*_bVgaDelayCnt =  0 ;*/
#if VGA_AUTO_SPEEDUP
	_bAutoISR = 0;
#endif
#if VGA_HW_AUTO
	vVgaHwAutoClkEnable(SV_OFF);
	vVgaHwAutoPhaseEnable(SV_OFF);
#endif
	/*boundary detection for defautl all channel*/
	vDrvVGASetBDCha(ALL_BD_SEL);
}

/**
 * @brief vVgaAutoPosSet(void)
 * Check Auto position result and store setting
 * @param  void
 * @retval void
 * @pre  enter  VDO_AUTO_POSITION_1_START state of SP0 for boundary measurement
 * @example vVgaAutoPosSet();
 */
void vVgaAutoPosSet(void)
{
	u16  wHWidth, wHeight;
	u16 wTmp;
	u16 MaxVStart;
	u8 bHcenter, bVcenter;
	u16 offset;
	u16 MaxHStart;

	bHcenter = 1;
	bVcenter = 1;

	/*for black pattern check*/
	if ((_wVgaHStart == 0x1fff) && (_wVgaHEnd == 0)) {
		vClrSp0Auto(SP0_H);
		pr_debug("H fail\r\n");

	}

	if ((_wVgaVStart == 0xfff) && (_wVgaVEnd == 0)) {
		vClrSp0Auto(SP0_V);
		pr_debug("V fail\r\n");
	}

#if 0

	if (_bAutoClockFail && bVgaOpt07_AutoKeepOldVal) {
		vClrSp0Auto(SP0_H);
		vClrSp0Auto(SP0_V);
		UTIL_Printf("Auto Pos skip");
	}

#endif
	wHWidth = (_wVgaHEnd - _wVgaHStart) + 2;
#if VGA_AUTO_DBG_MSG
	pr_debug("wHWidth:%d\r\n", wHWidth);
	pr_debug("tbl width:%d\r\n", Get_VGAMODE_IPH_WID(_bVgaTiming));
	pr_debug("_wVgaHStart:%d\r\n", _wVgaHStart);
	pr_debug("_wVgaHEnd:%d\r\n", _wVgaHEnd);
	pr_debug("tbl bporch:%d\r\n", Get_VGAMODE_IPH_BP(_bVgaTiming));
#endif

	if ((wHWidth > Get_VGAMODE_IPH_WID(_bVgaTiming) + 20) || (wHWidth < Get_VGAMODE_IPH_WID(_bVgaTiming) - 20)) {
		/*check for the gray ramp case*/
		wTmp =  _wVgaHEnd - Get_VGAMODE_IPH_WID(_bVgaTiming) + 2;

		if ((_wVgaHStart < (Get_VGAMODE_IPH_BP(_bVgaTiming) + 20))
		&& (_wVgaHStart > (Get_VGAMODE_IPH_BP(_bVgaTiming) - 20))) {
			bHcenter = 0;
#if VGA_AUTO_DBG_MSG
			/*ramp case left side ok*/
			pr_debug("Left ok\r\n");
#endif
		} else if ((wTmp < (Get_VGAMODE_IPH_BP(_bVgaTiming) + 20))
		&& (wTmp > (Get_VGAMODE_IPH_BP(_bVgaTiming) - 20))) {
			/*ramp case right  side ok*/
			bHcenter = 0;
#if VGA_AUTO_DBG_MSG
			pr_debug("Right ok\r\n");
#endif
			_wVgaHStart = wTmp;
		}
	}

	wHeight = (_wVgaVEnd - _wVgaVStart) + 2;            /* +2 for 8202B version , +1 for 8202A version*/
#if VGA_AUTO_DBG_MSG
	pr_debug("wHeight:%d\r\n", wHeight);
	pr_debug("tbl height:%d\r\n", Get_VGAMODE_IPV_LEN(_bVgaTiming));
#endif

	wTmp = Get_VGAMODE_IPV_LEN(_bVgaTiming);

	if (Get_VGAMODE_INTERLACE(_bVgaTiming) == INTERLACE) {
		wTmp >>= 1;
	}

	if ((wHeight > (wTmp + 20)) || (wHeight < (wTmp - 20))) {
		/*check for the gray ramp case*/
		wTmp =  _wVgaVEnd - wTmp + 2;

		if ((_wVgaVStart < (Get_VGAMODE_IPV_STA(_bVgaTiming) + 20))
		&& (_wVgaVStart > (Get_VGAMODE_IPV_STA(_bVgaTiming) - 20))) {
#if VGA_AUTO_DBG_MSG
			/*ramp case left side ok*/
			pr_debug("top ok\r\n");
#endif
			bVcenter = 0;
		} else if ((wTmp < (Get_VGAMODE_IPV_STA(_bVgaTiming) + 20))
		&& (wTmp > (Get_VGAMODE_IPV_STA(_bVgaTiming) - 20))) {
			/*ramp case right  side ok*/
#if VGA_AUTO_DBG_MSG
			pr_debug("bt ok\r\n");
#endif
			_wVgaVStart = wTmp;
			bVcenter = 0;
		}
	}

	/*_wVgaHStart = _wVgaHStart - VGA_H_OFST;*/
#ifdef MT5360B_WA4

	if ((wVgaInputWidth() != 1920) && (SRM_GetTvMode() == SRM_TV_MODE_TYPE_NORMAL ||
					   SRM_GetTvMode() == SRM_TV_MODE_TYPE_PIP) &&  fgIsMainVga()) {
		_wVgaHStart = _wVgaHStart + VGA_H_OFST - MT5360B_WA4_DLY1;
	} else {
		_wVgaHStart = _wVgaHStart + VGA_H_OFST;
	}

#else
	_wVgaHStart = _wVgaHStart + VGA_H_OFST;
#endif

	/*Modify for Disaplymode delay*/
	if (_wVgaVStart >= (VGA_V_OFST)) {
		_wVgaVStart = _wVgaVStart - (VGA_V_OFST);
	}

	/*------------------------------------------------------------*/
#if VGA_AUTO_UI_DBG
	pr_debug("autoH=>%d\r\n", _wVgaHStart);
	pr_debug("autoV=>%d\r\n", _wVgaVStart);
#endif

	if (IsSp0SetAuto(SP0_H)) {
#if VGA_AUTO_DBG_MSG
		UTIL_Printf("H auto OK\r\n");
#endif

		if (bHcenter) {
			if (wHWidth < (Get_VGAMODE_IPH_WID(_bVgaTiming) - 1)) {
				offset = (((Get_VGAMODE_IPH_WID(_bVgaTiming) - wHWidth) + 1) / 2);

				if (_wVgaHStart > offset) {
					_wVgaHStart -=  offset;
				} else {
					_wVgaHStart = 0;
				}

				/*UTIL_Printf("new h start", _wVgaHStart);*/
			}

#ifndef CC_VGA_SPEC_PC_TIMING_WINDOWS_PATTERN_AUTO
			else if (wHWidth > (Get_VGAMODE_IPH_WID(_bVgaTiming) + 1)) {
				offset = (((wHWidth - Get_VGAMODE_IPH_WID(_bVgaTiming))  + 1) / 2);

				if ((_wVgaHStart + offset) > (Get_VGAMODE_IHTOTAL(_bVgaTiming) - 10)) {
					/*see vDrvSetHPosition*/
					_wVgaHStart = (Get_VGAMODE_IHTOTAL(_bVgaTiming) - 10);
				} else {
					_wVgaHStart += offset;
				}
			}

#endif

		}
	} else {
		_wVgaHStart = Get_VGAMODE_IPH_BP(_bVgaTiming);
	}

	MaxHStart = wDrvVGAHPositionMax();

	if (_wVgaHStart > MaxHStart) {
		_wVgaHStart = MaxHStart;
	}

	/*keepOldVal == 1 , auto fail , keep old value.*/
	if ((IsSp0SetAuto(SP0_H)) || (bVgaOpt07_AutoKeepOldVal == 0)) {
		vVgaSetInput(_wVgaHStart, Get_VGAMODE_IPH_WID(_bVgaTiming));
	}

	if (IsSp0SetAuto(SP0_V)) {
#if VGA_AUTO_DBG_MSG
		pr_debug("V auto OK\r\n");
#endif
		wTmp = Get_VGAMODE_IPV_LEN(_bVgaTiming);

		if (Get_VGAMODE_INTERLACE(_bVgaTiming) == INTERLACE) {
			wTmp >>= 1;
		}

		if (bVcenter) {
			if (wHeight < (wTmp - 1)) {
				u16 offset;

				offset = (((wTmp - wHeight) + 1) / 2);

				if (offset >= _wVgaVStart) {
					_wVgaVStart = 3;
				} else {
					_wVgaVStart -= offset;
				}

				/*UTIL_Printf("v s", _wVgaVStart);*/
			} else if (wHeight > (wTmp + 1)) {
				_wVgaVStart += (((wHeight - wTmp) + 1) / 2);

				if (_wVgaVStart > (wSP0Vtotal >> 1)) { /*see. wDrvVGAVPositionMax*/
					_wVgaVStart = (wSP0Vtotal >> 1);
				}
			}
		}

		_u4VPorchChanged = 1;   /* trigger flag for SCPOS to check she needs to reset Dispmode FreeRun*/

	} else {
		_wVgaVStart = Get_VGAMODE_IPV_STA(_bVgaTiming);
	}

	MaxVStart = wDrvVGAVPositionMax();

	if (_wVgaVStart > MaxVStart) {
		_wVgaVStart = MaxVStart;
	}

	/*keepOldVal == 1 , auto fail , keep old value.*/
	if ((IsSp0SetAuto(SP0_V)) || (bVgaOpt07_AutoKeepOldVal == 0)) {
		vDrvAsyncVsyncStart(_wVgaVStart);
	}


	/*----(End of Restoring)*/
#if VGA_AUTO_UI_DBG
	pr_debug("H Pos eeprom=>%d\r\n", _wVgaHStart);
	pr_debug("V Pos eeprom=>%d\r\n", _wVgaVStart);
#endif
}

#if VGA_AUTO_SPEEDUP
void vVgaAutoClkIsr(void)
{
	u32 dTmp;

	/*this state    to get the max phase diff of 0, 4, 8 .. 28 phase sum*/

	if (_bCurPhase == 0) {                           /* phase counter*/
		_dSumTmp[0] = dVGAGetAllDiffValue();
		_dSumTmp[1] = _dSumTmp[0];
	}

	if (_bCurPhase <= 31) {
		dTmp = dVGAGetAllDiffValue();

		if (_dSumTmp[0] < dTmp) {
			_dSumTmp[0] = dTmp;                   /* max diff value*/
		}

		if (_dSumTmp[1] > dTmp) {
			_dSumTmp[1] = dTmp;                   /* min diff value*/
		}

		_dSumTmp[2] = (_dSumTmp[0] - _dSumTmp[1]);    /* max/min difference*/

		_bCurPhase = _bCurPhase + 4;
		/*vDrvVGASetPhase(_bCurPhase);*/
		vDrvVGASetPhase_Simple(_bCurPhase); /*Do auto replace current setting*/
		_bVgaDelayCnt = 1;
	} else {
		vSetSP0AutoState(VDO_AUTO_CLOCK_1_START);
		_bAutoISR = 0;
	}

}

#endif /*VGA_AUTO_SPEEDUP*/

#if 0
/* vVgaAutoDynThr() maybe still need fine-tune ,20060902,gbsh */
/*void vDrvVGASetPsneThr(u8 bThr1, u8 bThr2);*/
vVgaAutoDynThr(void)
{
	u16 wRGBmaxsum;
	u8 peak_limit, pnse_limit;
	/* update limit */
	/* (peak)20~50, (AD max) 50~e0) */
	wRGBmaxsum = IO32ReadFldAlign(STA_SYNC0_00, RMAX)
	+ IO32ReadFldAlign(STA_SYNC0_00, GMAX) + IO32ReadFldAlign(STA_SYNC0_00, BMAX);
	peak_limit = ((wRGBmaxsum / 3) - 0x50) / 3 + 0x20;

	if (peak_limit < 0x20) {
		peak_limit = 0x20;
	} else if (peak_limit > 0x50) {
		peak_limit = 0x50;
	}

	vDrvVGASetTopThr(peak_limit);

	/* (peak)20~70, (AD max) 50~e0) */
	pnse_limit = ((wRGBmaxsum / 3) - 0x70) / 3 + 0x20;

	if (pnse_limit < 0x20) {
		pnse_limit = 0x20;
	} else if (pnse_limit > 0x70) {
		pnse_limit = 0x70;
	}

	vDrvVGASetPsneThr(pnse_limit, PSNE_THRE2);

	if (peak_limit <= 0x30) {
		vDrvVGASetPhsMix(PSNE2_ADD);
	} else {
		vDrvVGASetPhsMix(PSNE1_ADD);
	}

}
#endif
/**
 * @brief vVdoSP0AutoState( void )
 * Sync Processor 0 Auto state machine
 * @param  void
 * @retval void
 * @example vVdoSP0AutoState()
 */
void vVdoSP0AutoState(void)
{
	if (!fgIsSP0FlgSet(SP0_VGA_AUTO_FLG)) {
		return;
	}

	vClrSP0Flg(SP0_VGA_AUTO_FLG);

	if (_bVgaDelayCnt != 0) {
		_bVgaDelayCnt--;
		return;
	}

	switch (bGetSP0AutoState()) {

	case VDO_AUTO_NOT_BEGIN:
		if (fgIsAutoFlgSet(SP0_AUTO_ALL)) { /*Flag to decide do auto or not*/
			if ((g_u4SrcType == SRC_VGA) && (fgIsCLKLock())) {
				if (fgIsAutoFlgSet(SP0_AUTO_CLOCK)) {
					_wOrgClock = _wVgaClock;
					_bOrgPhase = u1DrvVGAGetPhase();

					/*set clock to default and do the auto boundary for auto clock first*/
					if (_IsVgaDetectDone) {
						vDrvVGASetClock(Get_VGAMODE_IHTOTAL(_bVgaTiming));
					}

					vSetSP0AutoState(VDO_AUTO_CLOCK_START);
					/*for vDrvVGASetClock , remember to delay 1 vsync for ISR clock setting.*/
					_bVgaDelayCnt = 2;
				}
			}
		}

		break;

	case VDO_AUTO_CLOCK_START:
		if ((g_u4SrcType == SRC_VGA) && (fgIsCLKLock())) {
			u16  wTableWidth, wMeasWidth;

#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[1]);
#endif
			vVGAAutoGetBoundary();
			wTableWidth = Get_VGAMODE_IPH_WID(_bVgaTiming);
			wMeasWidth = (_wVgaHEnd - _wVgaHStart) + 1;
#if VGA_AUTO_CLK_DBG
			pr_debug("auto clk tbl width 0:%d\r\n", wTableWidth);
			pr_debug("auto clk width 1:%d\r\n", wMeasWidth);
#endif


#if SUPPORT_VGA_AMBIGUOUS_H_DETECT

			if (_IsVgaDetectDone)
#endif
			{
				u16 wClockDiff;

				wClockDiff = 127;
				/*wClockDiff = wTableWidth/3;*/
				_wCurClk = Get_VGAMODE_IHTOTAL(_bVgaTiming);
				_bAutoClockFail = 0;

				if ((wMeasWidth > (wTableWidth - wClockDiff))
					&& (wMeasWidth < (wTableWidth + wClockDiff))) {
					u16  wTmp;

					wTmp = (u32)(wTableWidth * _wVgaClock) / (u32)wMeasWidth;
#if VGA_AUTO_CLK_DBG
					pr_debug("_wVgaClock:%d\r\n", _wVgaClock);
					pr_debug("tbl clk:%d\r\n", Get_VGAMODE_IHTOTAL(_bVgaTiming));
					pr_debug("Cal clk:%d\r\n", wTmp);
#endif
					_wEndClk = (wTmp & (~0x1)) + 4;
					/* even clock, clk-4, clk-2, clk, clk+2, clk+4*/
					_wBestClk = _wCurClk;
					/* set the best clock to  table default*/

					if ((u16)(wSP0Hclk * wTmp / (u32)1000) > (u16)wVGAADSpec) {

						_wEndClk = (_wCurClk & (~1)) + 4;
#if VGA_AUTO_CLK_DBG
						pr_debug(" pixel clok over, reset clock val\r\n");
#endif
					}
				} else {

					_wEndClk = (_wCurClk & (~1)) + 4;
					_wBestClk = _wCurClk;
#if VGA_AUTO_DBG_MSG
					pr_debug("auto clk fail\r\n");
#endif
					_bAutoClockFail = 1;
					/*if fail , continue to do phase and position*/
				}
			}

			/* set the initial value for auto clock*/
			{
				_bCurPhase = 0;                         /* current phase*/
				_bClkCnt = 0;                           /* clk point cnt*/
				_dBestSum = 0;                          /*Max clock diff*/
				_dSumTmp[2] = 0;                        /*diff accu for clock*/

				/*go to clock2 for default clock phase diff measure*/
				vSetSP0AutoState(VDO_AUTO_CLOCK_2_START);
				_bVgaDelayCnt = 1;
				/*vDrvVGASetPhase(_bCurPhase);*/
				vDrvVGASetPhase_Simple(_bCurPhase); /*Do auto replace current setting*/
#if VGA_HW_AUTO
				_bWaitHwRdy = 0;
#endif
			}
		} else {
			vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);
			vClrAutoFlg(SP0_AUTO_ALL);
		}

		break;

	case VDO_AUTO_CLOCK_1_START:

		/*this state is to find max phase diff of several clock settings.*/
		if (_bClkCnt < 6) {/* even clock, clk-4, clk-2, clk, clk+2, clk+4 and table default*/
#if VGA_AUTO_CLK_DBG
			pr_debug("_dSumTmp[2]:%d\r\n", (int) _dSumTmp[2]);
#endif

			if (_dBestSum < _dSumTmp[2]) {
				_dBestSum = _dSumTmp[2];
				_wBestClk = _wCurClk;
				/* Find the clock that makes the maximum difference.*/
			}

			if (_bClkCnt == 0) {
				_wCurClk = _wEndClk - 8;                /* Start from -4 (Initial is +4)*/
#if VGA_AUTO_CLK_SKIP_TABLE_HTOTAL
				_dBestSum = 0;
				pr_debug("Skipped table value\r\n");
#endif
			} else {
				_wCurClk = _wCurClk + 2;                /* Inc by 2 each time*/
			}

			_bClkCnt++;

			/*UTIL_Printf("_wCurClk", _wCurClk);*/
#if SUPPORT_VGA_AMBIGUOUS_H_DETECT

			if (_IsVgaDetectDone)
#endif
			{
				vSetSP0AutoState(VDO_AUTO_CLOCK_2_START);
				vDrvVGASetClock(_wCurClk);
				_bVgaDelayCnt = 1;
#if VGA_HW_AUTO
				_bWaitHwRdy = 0;
#endif
			}

#if SUPPORT_VGA_AMBIGUOUS_H_DETECT
			else { /*Ambiguous timing Test case*/
				vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);
				_dPhaseDiff = _dSumTmp[2];
				vClrAutoFlg(SP0_AUTO_CLOCK);

			}

#endif

			_bCurPhase = 0;                                 /* phase counter*/
			vDrvVGASetPhase_Simple(_bCurPhase); /*Do auto replace current setting*/
		} else {
			u8 bSkipPhase = 0;

			if ((_dBestSum == 0)
#ifdef CC_VGA_SPEC_PC_TIMING_WINDOWS_PATTERN_AUTO
			    || (_dBestSum < VGA_CLK_PH_THRE)
#endif
			   ) {
				/*if phase diff is zero , reset to default clock*/
#if VGA_AUTO_CLK_DBG
				pr_debug(" reset clock val\r\n");
#endif
				_wBestClk = (bVgaOpt07_AutoKeepOldVal == 1) ?
				_wOrgClock : Get_VGAMODE_IHTOTAL(_bVgaTiming);
				bSkipPhase = 1;

#ifdef CC_VGA_SPEC_PC_TIMING_WINDOWS_PATTERN_AUTO

				if (_dBestSum < VGA_CLK_PH_THRE) {
					bSkipPhase = 0;
				}

#endif


			}

			/*check AD spec is over range or not.*/
			if ((u16)((u32)(wSP0Hclk * _wBestClk) / (u32)1000) > (u16)wVGAADSpec) {

#if VGA_AUTO_CLK_DBG
				pr_debug(" pixel clok over, reset clock val");
#endif
				_wBestClk = (bVgaOpt07_AutoKeepOldVal == 1) ?
				_wOrgClock : Get_VGAMODE_IHTOTAL(_bVgaTiming);
			}

#ifdef __MODEL_slt__
			pr_debug("SLT skip auto clock and set to tbl clock");
			_wBestClk = Get_VGAMODE_IHTOTAL(_bVgaTiming);
#else

			if (_bAutoClockFail && bVgaOpt07_AutoKeepOldVal) {
				_wBestClk = _wOrgClock;
				bSkipPhase = 1;
			}

#endif

			vDrvVGASetClock(_wBestClk);


			if (bSkipPhase) {
#if VGA_AUTO_CLK_DBG
				pr_debug(" skip auto phase\r\n");
#endif
				vDrvVGASetPhase(_bOrgPhase);
				vClrAutoFlg(SP0_AUTO_PHASE);
				vSetSP0AutoState(VDO_AUTO_POSITION_1_START);
				vVgaAutoPosInit();
				_bVgaDelayCnt = 2;
#if CHANGE_SDDS_KPI
				vDrvEnableChang_SDDS_BW();
#endif
			} else {
				vSetSP0AutoState(VDO_AUTO_PHASE_START);
			}

			vClrAutoFlg(SP0_AUTO_CLOCK);
			_wVgaClock = _wBestClk;
#if VGA_AUTO_UI_DBG
			pr_debug("auot clk=>%d\r\n", _wVgaClock);
#endif
		}


		break;

	case VDO_AUTO_CLOCK_2_START:
#if VGA_HW_AUTO
		if (_bHwAuto) {
			if (!_bWaitHwRdy) {
				if (fgIsCLKLock()) {
					/* auto clock trigger*/
					vVgaHwAutoClkEnable(SV_ON);
					_bWaitHwRdy = 1;
				}
			} else {
				if (bDrvHwAutoClkRdy()) {
					/*read the max/min phase difference*/
					_dSumTmp[2] =   dDrvAutoGetPhsMaxMinDiff();
					vVgaHwAutoClkEnable(SV_OFF);
					_bVdoSP0AutoState = VDO_AUTO_CLOCK_1_START;
				}
			}

		} else
#endif /*#if VGA_HW_AUTO*/
		{

			if (_IsVgaDetectDone) {
				_bAutoISR = 1;
			} else {
				/* no speedup*/
				_bAutoISR = 0;

				/*this state  to get the max phase diff of 0, 4, 8 .. 28 phase sum*/
				if (_bCurPhase == 0) {                            /* phase counter*/
					_dSumTmp[0] = dVGAGetAllDiffValue();
					_dSumTmp[1] = _dSumTmp[0];
				}

				if (_bCurPhase <= 31) {
					u32 dTmp;

					dTmp = dVGAGetAllDiffValue();

					if (_dSumTmp[0] < dTmp) {
						_dSumTmp[0] = dTmp;                    /* max diff value*/
					}

					if (_dSumTmp[1] > dTmp) {
						_dSumTmp[1] = dTmp;                    /* min diff value*/
					}

					_dSumTmp[2] = (_dSumTmp[0] - _dSumTmp[1]);  /* max/min difference*/
					_bCurPhase = _bCurPhase + 4;
					/*vDrvVGASetPhase(_bCurPhase);*/
					vDrvVGASetPhase_Simple(_bCurPhase); /*Do auto replace current setting*/
					_bVgaDelayCnt = 1;
				} else {
					_bVdoSP0AutoState = VDO_AUTO_CLOCK_1_START;
				}

			}
		}

		break;

	case VDO_AUTO_PHASE_START:
#if VGA_TIMEDELAY_DBG
		HAL_GetTime(&cur_time[2]);
#endif
#if VGA_HW_AUTO

		/*YPbPr use software auto phase*/
		if (_bHwAuto && fgIsMainVga()) {
			_bWaitHwRdy = 0;
		} else
#endif /*                   #if VGA_HW_AUTO*/
		{
			_bCurPhase = 31;                    /*phase value*/
			_bBestPhase = 31;                   /* best phase*/
			_dBestSum = 0;                      /*Max phase sum*/
			/*vDrvVGASetPhase(_bCurPhase); // Initial set phase to 31*/
			vDrvVGASetPhase_Simple(_bCurPhase); /*Do auto replace current setting*/
			_bVgaDelayCnt = 1;
			vDrvInitPhaseVar();
		}

		vSetSP0AutoState(VDO_AUTO_PHASE_1_START);

		break;

	case VDO_AUTO_PHASE_1_START:
#if VGA_HW_AUTO

		/*YPbPr use software auto phase*/
		if (_bHwAuto && fgIsMainVga()) {
			if (!_bWaitHwRdy) {
				if (fgIsCLKLock()) {
					/* auto clock trigger*/
					vVgaHwAutoPhaseEnable(SV_ON);
					_bWaitHwRdy = 1;
				}
			} else {
				if (bDrvHwAutoPhsRdy()) {
#if NOTIFY_PROGRESS
					_bProgressBar += 32 ; /* 32+48 = 80 ;*/
					vApiNotifyVGAAutoProgress(((fgIsMainVga()) ? 0 : 1), _bProgressBar);
#endif
					/*read the max/min phase difference*/
					_bBestPhase = bDrvAutoGetPhsGood();
					/* reset the clock trigger*/
					vVgaHwAutoPhaseEnable(SV_OFF);
					/*  vDrvADCPLLSetRelatch's setting of auto adjust and UI setting are different.
					It causes the position shift 1 pixel. */
					vDrvVGASetPhase(_bBestPhase);   /*only this not Simple*/
					vDrvRETIMEReset();
					/* Reset for FIFO pointer. For next step auto position*/
					/* Solved VGA right line disappear when source switch*/
					/*vDrvVGASetPhase_Simple(_bBestPhase); //Do auto replace current setting*/
#if VGA_AUTO_UI_DBG
					LogSB("Phase eeprom=>%d\r\n", _bBestPhase);
#endif
					vClrAutoFlg(SP0_AUTO_PHASE);
					vSetSP0AutoState(VDO_AUTO_POSITION_1_START);
					vVgaAutoPosInit();
					_bVgaDelayCnt = 2;
				}
			}
		} else
#endif /*#if VGA_HW_AUTO*/
		{
			_bAutoISR = 1;
		}

		break;


	case VDO_AUTO_POSITION_1_START:
		if ((g_u4SrcType == SRC_VGA)
		    /*&& (_IsVgaDetectDone == TRUE)*/
		    && (fgIsCLKLock())) {
			vVGAAutoGetBoundary();
#if VGA_AUTO_POS_DBG
			pr_debug("_wVgaHStart:%d\r\n", _wVgaHStart);
			pr_debug("_wVgaVStart:%d\r\n", _wVgaVStart);
			pr_debug("_wVgaHEnd:%d\r\n", _wVgaHEnd);
			pr_debug("_wVgaVEnd:%d\r\n", _wVgaVEnd);
#endif

			vClrAutoFlg(SP0_AUTO_POSITION);
#if     VGA_AUTO_SPEEDUP
			/*  bVGAAutoGetBoundary();*/
			vVgaAutoPosSet();
			vClrAutoFlg(SP0_AUTO_POSSET);
#endif /*VGA_AUTO_SPEEDUP*/
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[3]);

			{
				int i;

				for (i = 0; i < 5; i++) {
					pr_debug(3, "[%d] %d:%d \r\n", i, cur_time[i].u4Seconds, cur_time[i].u4Micros);
				}
			}

#endif
			vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);
		}

		break;

	default:
		break;
	}   /*switch case*/
}



/**
 * @brief vSetHPosition(u8 bValue, u8 sp)
 * Set H Active Position
 * @param  wValue : H start value
 * @retval void
 * @example vDrvSetHPosition(20, 0) - set sp0 H start position 20
 */
void vDrvSetHPosition(u16 wValue)
{
	u16 max;

	if (g_u4SrcType == SRC_VGA) {
		max = Get_VGAMODE_IHTOTAL(_bVgaTiming) - 10; /*very unreasonable*/

		if (wValue > max) {
			wValue = max;
		}

		vVgaSetInput(wValue, Get_VGAMODE_IPH_WID(_bVgaTiming));
		/* for the bug: the right side of the VGA picture has the pink garbage*/
	} else {

		max = Get_VGAMODE_IHTOTAL(_bHdtvTiming) - 10; /*very unreasonable*/

		if (wValue > max) {
			wValue = max;
		}

		vVgaSetInput(wValue, Get_VGAMODE_IPH_WID(_bHdtvTiming));
		/* for the bug: the right side of the VGA picture has the pink garbage*/
	}

	/*  UNUSED(sp);*/
}

/**
 * @brief vDrvSetVPosition(u8 bValue, u8 sp)
 * Set V Active Position
 * @param  wValue : V start value
 * @retval void
 * @example vDrvSetVPosition(20, 0) - set sp0 V start position 20
 */
void vDrvSetVPosition(u16 wValue)
{
	u8  bTmpTiming;

	if (g_u4SrcType == SRC_VGA) {
		bTmpTiming = _bVgaTiming;
	} else {
		bTmpTiming = _bHdtvTiming;
	}

	if (Get_VGAMODE_INTERLACE(bTmpTiming) == PROGRESSIVE) {
		vDrvAsyncVsyncStart(wValue);

	}

	/*  UNUSED(sp);*/
}

/* ***********************************************************/
/* Function : void vDrvVgaAutoStart(void)*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* ***********************************************************/
/**
 * @brief vDrvVgaAutoStart(void)
 * Trigger  SP0/SP1 Auto state machine to start
 * @param  void
 * @retval void
 * @example vDrvVgaAutoStart()
 */
void vDrvVgaAutoStart(void)
{
	if (g_u4SrcType == SRC_VGA) {
		if (_IsVgaDetectDone && fgIsValidTiming(_bVgaTiming)) {
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[0]);
#endif
			vSetAsyncMeasureBD(DOMAIN_PIXEL); /* penggang add 20090825 for cr DTV00212128*/
			vVgaAutoInit();
			vSetAutoFlg(SP0_AUTO_ALL);

#if CHANGE_SDDS_KPI
			Set_SDDS_KPI(1);
#endif
		}
	}
}

void vDrvVgaAutoStop(void)
{
	if (g_u4SrcType == SRC_VGA) {
		if ((_IsVgaDetectDone) && (fgIsCLKLock())) {
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[0]);
#endif
			vClrAutoFlg(SP0_AUTO_ALL);
			vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);

#if CHANGE_SDDS_KPI
			vDrvEnableChang_SDDS_BW();
#endif
		}
	}
}

/* ***********************************************************/
/* Function : void vDrvYPbPrAutoStart(void)*/
/* Description :*/
/* Parameter :*/
/* Return    :*/
/* ***********************************************************/
/**
 * @brief vDrvYPbPrAutoStart(void)
 * Trigger  SP0/SP1 Auto state machine to start
 * @param  void
 * @retval void
 * @example vDrvVgaAutoStart()
 */
void vDrvYPbPrAutoStart(void)  /*YPbPr Auto Phase 2006/11/07*/
{

	if (g_u4SrcType == SRC_YBR) {
		if ((_IsHdtvDetectDone) && (fgIsCLKLock())) {
			vSetSP0AutoState(VDO_AUTO_PHASE_START);
#if CHANGE_SDDS_KPI
			Set_SDDS_KPI(1);
#endif
		}
	}
}






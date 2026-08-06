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

#include "drv_vga.h"
#include "ybr_vga_common.h"


#define POLARITY_DEBUG  0

#define VGA_TIMEDELAY_DBG 0
#if VGA_TIMEDELAY_DBG
static HAL_TIME_T cur_time[20];
#endif

u8   _bVgaTiming;

u8   _IsVgaDetectDone;

u8   bModeIndex;
u8   bVGAMDStateMachine;
u8   bVGAMCErrorCnt;
u8   bVGAMCErrorCnt_tmp;
u8   _bVgaHvChkCnt;
u8 _bHsyncWidth;
u8   bVgadbgmask = 0;
static bool _fgVgaAmbiguousUserSel = SV_FALSE;

#define VGA_TIMING_CHECK_POLARITY   1
#define VGA_SEPERATE_ONLY   0
#define ABS(x)                  (((x) >= 0) ? (x) : -(x))


#if SUPPORT_VGA_USERMODE
u8   _bVgaUserMode;          /*just using for copy eep to ram*/
u8   _bVgaUserMode_FIFO;
u8   _bVgaUserMode_Flush;
u16 _dVgaStableCnt  = 0;
#endif

#if SUPPORT_VGA_AMBIGUOUS_H_DETECT
u8   _bBestTiming;
u32   _dPhaseDiff, _dMaxDiff;
#endif

#define AMBIGUOUS_HF_DELTA  10
#define AMBIGUOUS_VF_DELTA  3

u8  _bVgaChkState;
u8 _bVgaModeChged;
u8 _bSP0MuxCnt;
u16 _bSP0MuxSate;
HAL_TIME_T _rVgaModeChgTime;
u8 _bVgaStableDealy = 0;
static u8   NoSignalCnt;
/*
extern u8 _bPLLlockCnt;
extern void vVgaSetModeCHG(void);
extern void vVgaSetModeDone(void);
*/
bool _RETIMENeedReset = TRUE;


/* extern u8 _bUnLockCnt; // for checking tool - QAC Warning*/
/*extern u8 _bLockCnt;*/

u8 _u1Vga422ModeChg = 0;

/*
void vVgaAmbiguousHInit(void);
#if SUPPORT_VGA_USERMODE
void vVgaUsrStable(void);
void vVgaUsrBroken(void);
void vVgaEraseUserMode(u8 index);
#endif
*/
/*//////////////////////////////////////////////////////////////////////////////*/
/**
 * @brief vVgaPolarityUniform( void )
 * According to   fgSP0Hpol & fgSP0Vpol to set suitable setting in each step
 * @param  void
 * @retval void
 * @pre  Need to call the API of vVgaNewPolarity()
 */
void vVgaPolarityUniform(void)
{
	fgSP0Hpol = fgASHPolarityMeasure();
	fgSP0Vpol = fgASVPolarityMeasure();

	if (fgSP0Hpol == (u8)POSITIVE) {
		vDrvCLKINFreeHsyncPol(0); /*ADPART*/
		vDrvCLKINHsyncPol(0); /*ADPART*/
		vDrvHsInv((u8)TRUE); /* Goal:- gbsh (+:stable vlen but buggy field detect)060612*/
#if POLARITY_DEBUG
		UTIL_Printf("H+");
#endif
	} else {
		vDrvCLKINFreeHsyncPol((u32)1); /*ADPART*/
		vDrvCLKINHsyncPol((u32)1); /*ADPART*/
		vDrvHsInv((u8)FALSE); /* Goal:- gbsh (+:stable vlen but buggy field detect)060612*/
#if POLARITY_DEBUG
		UTIL_Printf("H-");
#endif
	}

	if (fgSP0Vpol == (u8)POSITIVE) {
		vDrvVsOutInvPol((u8)FALSE);  /*Goal: +*/
		vDrvVsInv((u8)TRUE); /* Goal:-*/
#if POLARITY_DEBUG
		UTIL_Printf("V +");
#endif
	} else {
		vDrvVsOutInvPol((u8)TRUE);   /*Goal: +*/
		vDrvVsInv((u8)FALSE);    /* Goal:-*/
#if POLARITY_DEBUG
		pr_debug("V -");
#endif
	}

	pr_debug("vVgaPolarityUniform %d %d\r\n", fgSP0Hpol, fgSP0Vpol);
	/*Always invert H-lock*/
	vDrvHsLockInv((u8)TRUE);
}
#ifdef SUPPORT_VGA_UI_AMBIGUOUS_TIMING_SELECT
#ifdef CC_VGA_AMB_SPEC_SETTING_X_768_60
static u8 _au1AmbiguousIndex = (u8)1;
#endif

u8 bAmbiguousHCheck(u8 CandidateTiming)
{
	u8 AmbTimingIndex, EEPromIndex, i, j;
	u16 _wCurHF, _wTempHF;
	u8 _bCurVF, _bTempVF;
#ifdef CC_VGA_AMB_SPEC_SETTING_X_768_60
	u8 AmbNIndex;
#endif


	pr_debug("**********************\r\n");
	pr_debug("CandidateTiming = %d\r\n", CandidateTiming);

	_wCurHF = Get_VGAMODE_IHF(CandidateTiming);
	_bCurVF = Get_VGAMODE_IVF(CandidateTiming);

	/*search to frist timnig  - 1 of ambiguous group*/
	for (AmbTimingIndex = (CandidateTiming - 1); AmbTimingIndex > 0; AmbTimingIndex--) {
		_wTempHF = Get_VGAMODE_IHF(AmbTimingIndex);
		_bTempVF = Get_VGAMODE_IVF(AmbTimingIndex);
		_wTempHF = (_wTempHF > _wCurHF) ? (_wTempHF - _wCurHF):(_wCurHF - _wTempHF);
		_bTempVF = (_bTempVF > _bCurVF) ? (_bTempVF - _bCurVF):(_bCurVF - _bTempVF);

		if ((_wTempHF <= AMBIGUOUS_HF_DELTA) && (_bTempVF <= AMBIGUOUS_VF_DELTA)
		&& (Get_VGAMODE_AmbiguousH(AmbTimingIndex))) {
			continue;
		} else {
			break;
		}
	}

	EEPromIndex = 0;

	for (i = 0; i <= CandidateTiming; i++) {
		if (Get_VGAMODE_AmbiguousH(i)) {
			_wCurHF = Get_VGAMODE_IHF(i);
			_bCurVF = Get_VGAMODE_IVF(i);

			for (j = i; j <= CandidateTiming; j++) {
				_wTempHF = Get_VGAMODE_IHF(j);
				_bTempVF = Get_VGAMODE_IVF(j);
				_wTempHF = (_wTempHF > _wCurHF) ? (_wTempHF - _wCurHF):(_wCurHF
				- _wTempHF);
				_bTempVF = (_bTempVF > _bCurVF) ? (_bTempVF - _bCurVF):(_bCurVF
				- _bTempVF);

				if ((_wTempHF <= AMBIGUOUS_HF_DELTA) && (_bTempVF <= AMBIGUOUS_VF_DELTA)
				&& (Get_VGAMODE_AmbiguousH(j))) {
					continue;
				} else {
					break;
				}
			}

			if (j <= CandidateTiming) {
				EEPromIndex++;
			}

			i = j-1;
		}
	}

#ifdef CC_VGA_AMB_SPEC_SETTING_X_768_60
	pr_debug("Load EEP Loc = %d, ", EEP_VGA_AMBIGUOUS_DEFAULT_START + EEPromIndex);

	AmbNIndex = bApiEepromReadu8(EEP_VGA_AMBIGUOUS_DEFAULT_START + EEPromIndex);

	if (AmbNIndex == 0) {
		/* use shadow for keeping compatible if no NVM available. */
#if 0
		AmbNIndex = _au1AmbiguousIndexShadow[EEPromIndex];
#else
		AmbNIndex = _au1AmbiguousIndex;
#endif

		AmbNIndex = (AmbNIndex == 0)?(1):(AmbNIndex);
	}

	UTIL_Printf("Val = %d\r\n", AmbNIndex);

	if (AmbNIndex != 0xFF) {     /*default value stored.*/
		pr_debug("Return Val = %d\r\n", (AmbNIndex + AmbTimingIndex));
		pr_debug("**********************\r\n");
		return (AmbNIndex + AmbTimingIndex);
	}

#else

	pr_debug("Load EEP Loc = %d, ", EEP_VGA_AMBIGUOUS_DEFAULT_START + EEPromIndex);

	EEPromIndex = bApiEepromReadu8(EEP_VGA_AMBIGUOUS_DEFAULT_START + EEPromIndex);

	pr_debug("Val = %d\r\n", EEPromIndex);

	/*default offset = 1*/
	if (EEPromIndex != 0xFF) {   /*default value stored.*/
		pr_debug("Return Val = %d\r\n", (EEPromIndex+AmbTimingIndex));
		pr_debug("**********************\r\n");
		return (EEPromIndex+AmbTimingIndex);
	}

#endif

	return CandidateTiming;
}
#endif /*SUPPORT_VGA_UI_AMBIGUOUS_TIMING_SELECT*/
#ifndef CC_VGA_VSYNC_WIDTH_AMBIGUOUS_DISABLE
u8 bAmbiguousVsyncWidthCheck(u8 bMode)
{
	u8 bVsyncWidth;
	u8 bRatioH, bRatioV;
	u8 bModeIdx;
	u16 wDiff;

	bVsyncWidth = (u8)wASVSyncWidthMeasure();
	bModeIdx = bMode;

	while (Get_VGAMODE_VSyncWidthChk(bMode)) {
		switch (bVsyncWidth) {
		case (u8)4:/*4:3 , 1024x768 1440x1050*/
			bRatioH = (u8)4;
			bRatioV = (u8)3;
			break;

		case (u8)5:/*16:9 1360x768 ,*/
			bRatioH = (u8)16;
			bRatioV = (u8)9;
			break;

		case (u8)6: /*16:10 1680x1050*/
			bRatioH = (u8)16;
			bRatioV = (u8)10;
			break;

		case (u8)7:/*15:9 1280x768*/
			bRatioH = (u8)15;
			bRatioV = (u8)9;
			break;

		default:/* if not support vsyncWidth return org mode*/
			return bMode;
		}

		/*Printf("bRatioH %d bRatioV %d v %d\r\n", bRatioH, bRatioV, Get_VGAMODE_IPV_LEN(bModeIdx));*/
		/*Printf("%d %d %d\r\n", bModeIdx, ((Get_VGAMODE_IPV_LEN(bModeIdx)*bRatioH)/bRatioV),
		Get_VGAMODE_IPH_WID(bModeIdx));*/
		wDiff = ((u32)(Get_VGAMODE_IPV_LEN(bModeIdx) * bRatioH))/(u32)bRatioV;
		wDiff = (wDiff > Get_VGAMODE_IPH_WID(bModeIdx))
		? (wDiff - Get_VGAMODE_IPH_WID(bModeIdx)):(Get_VGAMODE_IPH_WID(bModeIdx) - wDiff);

		/*Printf("wDiff %d\r\n", wDiff);*/
		if (wDiff < (u16)20) {
			break;
		}

		if (Get_VGAMODE_IVTOTAL(bModeIdx + 1) != Get_VGAMODE_IVTOTAL(bMode)) {
			return bMode;
		}

		bModeIdx++;
	}

	/* UTIL_Printf(" Vsync Width (%d)check mode = %d\r\n", bVsyncWidth, bModeIdx);*/
	return bModeIdx;
}
#endif
/**
* @brief bVgaStdTimingSearch(void)
* VGA Timing search according to bSP0Vclk & wSP0Hclk
* @param  void
* @retval Timing index.
* @pre  Need to measure H/V Frequence and frequence transform.
*/
u8 bVgaStdTimingSearch(void)
{
	u8   CandidateTiming, bsearch, bPolMatchTiming;
	u16  wMinVTotalOffset, wTempVTotalOffset, wTmpVtotal;
	u32 wTempHsyncWOffset, wMinHsyncWOffset;
	u16 wTempHFOffset, wTempVFOffset;
	u32 wHFVFOffsetSum, wMinHFVFOffsetSum = (u32)0xFFFF;
	bool   _IsClosestTable;

	if ((wSP0Hclk < (u16)50) || (bSP0Vclk < (u16)5)) { /*H<5k V<5hz */
		return MODE_NOSUPPORT;
	}

	bsearch = VGA_SEARCH_START;

	if (bVgaOpt04_SearchHDTV) {
		bsearch = (u8)HDTV_SEARCH_START;
	}

	wMinVTotalOffset = (u16)0xffff;
	CandidateTiming = MODE_NOSUPPORT;
	bPolMatchTiming = 0;
	_bHsyncWidth = (u8)wASHSyncWidthMeasure();
	wMinHsyncWOffset = (u16)0xffff;

	do {
		if ((bSP0Vclk > (Get_VGAMODE_IVF(bsearch) - (VSYNC_LOWER_BD))) &&
		(bSP0Vclk < (Get_VGAMODE_IVF(bsearch) + (VSYNC_UPPER_BD)))) {
			if (Get_VGAMODE_VgaDisabled(bsearch)) {
				continue;
			}

			if (!Get_VGAMODE_IHF(bsearch)) {
				continue;
			}

			if (
				(wSP0Hclk > (Get_VGAMODE_IHF(bsearch) - HSYNC_LOWER_BD)) &&
				(wSP0Hclk < (Get_VGAMODE_IHF(bsearch) + HSYNC_UPPER_BD))) {
				/* UTIL_Printf( "in hf vf range, bsearch is %d\r\n", bsearch);*/
#if SUPPORT_VGA_USERMODE

				/* user mode need everything exactly the same, otherwise evaluate a new one */
				if (bsearch >= bUserVgaTimingBegin) {
					u8 usrmode_fifo = bsearch-bUserVgaTimingBegin;

					if (!bVgaOpt05_SearchNewMode) {
						break;
					}

					if (fgSP0Hpol != rVgaUsrEEP[usrmode_fifo].hpol) {
						continue;
					}

					if ((bModeIndex == SEPERATESYNC) &&
					(fgSP0Vpol != rVgaUsrEEP[usrmode_fifo].vpol)) {
						continue;
					}

					if ((wSP0HLength < (rVgaUsrEEP[usrmode_fifo].hlen - 1))
					|| (wSP0HLength > (rVgaUsrEEP[usrmode_fifo].hlen + 1))) {
						continue;
					}

					if ((wSP0Vtotal < (rVgaUsrEEP[usrmode_fifo].vlen - 1))
					|| (wSP0Vtotal > (rVgaUsrEEP[usrmode_fifo].vlen + 1))) {
						continue;
					}

					if ((_bHsyncWidth < (rVgaUsrEEP[usrmode_fifo].hsync_w - 1))
					|| (_bHsyncWidth > (rVgaUsrEEP[usrmode_fifo].hsync_w + 1))) {
						continue;
					}

					return bsearch;
				}

#endif
				_IsClosestTable = 0;
				/* V total */
				wTmpVtotal = Get_VGAMODE_IVTOTAL(bsearch);

				if (Get_VGAMODE_INTERLACE(bsearch)) {
					wTmpVtotal = wTmpVtotal / (u16)2;
				}

				/* interlace mode measure Vtotal= real Vtotal/2 */
				wTempVTotalOffset = (wSP0Vtotal > wTmpVtotal) ? (wSP0Vtotal - wTmpVtotal)
				: (wTmpVtotal - wSP0Vtotal);

				/* find the closest Vtotal*/
				if (wMinVTotalOffset > wTempVTotalOffset) {
					wMinVTotalOffset = wTempVTotalOffset;
					CandidateTiming = bsearch;
					_IsClosestTable = 1;
					/*UTIL_Printf( "the closest Vtotal match %d\r\n", CandidateTiming);*/
					wMinHFVFOffsetSum = ABS(Get_VGAMODE_IHF(bsearch) - wSP0Hclk)
					+ ABS(Get_VGAMODE_IVF(bsearch) - bSP0Vclk);
				} else if (wMinVTotalOffset == wTempVTotalOffset) { /*Check (Diff(HF)+Diff(VF))*/
					wTempHFOffset = ABS(Get_VGAMODE_IHF(bsearch) - wSP0Hclk);
					wTempVFOffset = ABS(Get_VGAMODE_IVF(bsearch) - bSP0Vclk);
					wHFVFOffsetSum = wTempHFOffset + wTempVFOffset;

					if (wHFVFOffsetSum < wMinHFVFOffsetSum) {
						CandidateTiming = bsearch;
						wMinHFVFOffsetSum = wHFVFOffsetSum;
				/*UTIL_Printf( "the closest Vtotal + (Diff(HF)+Diff(VF)) match %d\r\n",
				CandidateTiming);*/
						_IsClosestTable = (bool)1;
					} else if (wHFVFOffsetSum == wMinHFVFOffsetSum) {
						_IsClosestTable = (bool)1;
					}
				}

				if (Get_VGAMODE_PolChk(bsearch)) {
					if (Get_VGAMODE_HPol(bsearch) == fgSP0Hpol) {
						if ((bModeIndex != SEPERATESYNC) ||
						   ((bModeIndex == SEPERATESYNC) &&
						   (Get_VGAMODE_VPol(bsearch) == fgSP0Vpol))) {
							if (wTempVTotalOffset <= wMinVTotalOffset) {
								/*for 576p  and 640x480x50Hz ambiguous*/
								bPolMatchTiming = bsearch;
#if POLARITY_DEBUG
								pr_debug("polarity match", bsearch);
								pr_debug("h", fgSP0Hpol);
								pr_debug("v", fgSP0Vpol);
								/*lint --e{*} Don't lint this function*/
								pr_debug("h1", Get_VGAMODE_HPol(bsearch));
								/*lint --e{*} Don't lint this function*/
								pr_debug("v1", Get_VGAMODE_VPol(bsearch));
#endif
							}
						}
					}
				}

				/* H-sync width */
				if (Get_VGAMODE_HSyncWidthChk(bsearch) && _IsClosestTable == 1) {
					u16 wDiv_temp;

					wDiv_temp = Get_VGAMODE_OverSample(bsearch) + 1;

					/*if ((Get_VGAMODE_ICLK(bsearch))&&(Get_VGAMODE_IPH_STA(bsearch)
					>Get_VGAMODE_IPH_BP(bsearch)))*/
					if (Get_VGAMODE_ICLK(bsearch)) {
						u16 wtable_width;

						/* Hwid/pixclk=STA29/270 ,we want Hwid*/
						wTempHsyncWOffset = (((u32)_bHsyncWidth<<8)
						*(Get_VGAMODE_ICLK(bsearch)/wDiv_temp))/270;
						wTempHsyncWOffset >>= 8;  /* now  Hwid */
						/*wtable_width=(Get_VGAMODE_IPH_STA(bsearch)/wDiv_temp)
						-(Get_VGAMODE_IPH_BP(bsearch)/wDiv_temp);*/
						wtable_width = Get_VGAMODE_IPH_SYNCW(bsearch)/wDiv_temp;
						wTempHsyncWOffset = wTempHsyncWOffset > wtable_width ?
								  wTempHsyncWOffset - wtable_width  :
								  wtable_width - wTempHsyncWOffset;

						/* find the closest hsync-width*/
						if (wMinHsyncWOffset > wTempHsyncWOffset) {
							wMinHsyncWOffset = wTempHsyncWOffset;
							CandidateTiming = bsearch;
							/*UTIL_Printf( "hsync width match %d\r\n", CandidateTiming);*/
						}
					}
				}
			}
		}
	} while (++bsearch <= VGA_SEARCH_END);

	/* if polarity match replace timing.*/
	if (bPolMatchTiming) {
		CandidateTiming = bPolMatchTiming;
	}

	if (Get_VGAMODE_ICLK(CandidateTiming) > PIX_CLK_LIMIT) {
		pr_debug("pix clk is %d, exceed the limit is:%d \r\n",
		Get_VGAMODE_ICLK(CandidateTiming), PIX_CLK_LIMIT);
		return  MODE_NOSUPPORT;
	}

#ifdef SUPPORT_VGA_UI_AMBIGUOUS_TIMING_SELECT

	/*Check if ambiguous VGA has default setting, if yes, replace CandidateTiming with value from EEPROM*/
	if ((_fgVgaAmbiguousUserSel != SV_FALSE) && (Get_VGAMODE_AmbiguousH(CandidateTiming) != 0)) {
		return bAmbiguousHCheck(CandidateTiming);
	} else
#endif /*SUPPORT_VGA_UI_AMBIGUOUS_TIMING_SELECT   */
	{
#ifndef CC_VGA_VSYNC_WIDTH_AMBIGUOUS_DISABLE

		/*move to first ambiguous timing*/
		if (Get_VGAMODE_VSyncWidthChk(CandidateTiming) && (CandidateTiming < bAllTimings)) {
			u8 AmbTimingIndex;

			for (AmbTimingIndex = CandidateTiming; AmbTimingIndex > 0; AmbTimingIndex--) {
				/*check ambiguous flag and Vtotal*/
				if (Get_VGAMODE_VSyncWidthChk(AmbTimingIndex) &&
				   Get_VGAMODE_IVTOTAL(AmbTimingIndex) == Get_VGAMODE_IVTOTAL(CandidateTiming)) {
					continue;
				} else {
					break;
				}
			}

			CandidateTiming = AmbTimingIndex + 1;
		}

		if (Get_VGAMODE_VSyncWidthChk(CandidateTiming)) {
			CandidateTiming = bAmbiguousVsyncWidthCheck(CandidateTiming);
		}

#endif
	}

#if YPBPR_480IP_27MHZ

	if (CandidateTiming == MODE_625I_OVERSAMPLE) {
		CandidateTiming = MODE_625I;
	}

	if (CandidateTiming == MODE_525I_OVERSAMPLE) {
		CandidateTiming = MODE_525I;
	}

#endif
	return CandidateTiming;
}


/**
 * @brief vVgaSetIputIClk(u8 bMode)
 * Set PLL related clock setting according to timing mode.
 * @param  timing mode index
 * @retval void
 * @pre  Must identify timing mode is supported mode first.
 */
void vVgaSetIputIClk(u8 bMode)
{
	u16 wPixelClk;

	if (bMode >= MAX_TIMING_FORMAT) {
		return;
	}

	if (bModeIndex == SYNCONGREEN) {
		vDrvCLKINSyncSel(DCLK_IN_SOG);  /* DCLK_hsync_sel: 0: From SOG   //ADPART*/
	} else {
		vDrvCLKINSyncSel(DCLK_IN_HSYNC);    /* DCLK_hsync_sel: 1: From hsync   //ADPART*/
	}

	if (bModeIndex == SEPERATESYNC) { /*kal 20091223 VG848 GaryBar INV*/
		vIO32WriteFldAlign(PDWNC_VGACFG3, 1 , FLD_RG_SOGY_SINK_PWD);/*FLD_RG_SOGY_SORS_PWD*/
	} else {
		vIO32WriteFldAlign(PDWNC_VGACFG3, 0 , FLD_RG_SOGY_SINK_PWD);/*FLD_RG_SOGY_SORS_PWD*/
	}

	_wVgaClock = Get_VGAMODE_IHTOTAL(bMode);
	wPixelClk = Get_VGAMODE_ICLK(bMode);
	pr_debug("wHtotal %d\r\n ", _wVgaClock);
	pr_debug("wPixelClk %d\r\n", wPixelClk);

	vDrvADCPLLSet(wPixelClk, _wVgaClock);  /*ADPART*/
}

/**
 * @brief vVgaSetInputCapature(u8 bmode)
 * According to timing mode to set suitable Mask, windows etc.
 * @param  Timing Mode index
 * @retval void
 * @pre  Need to identify supported timing mode first.
 */
void vVgaSetInputCapature(u8 bmode)  /*set input start and length*/
{
	u16 wWindow;

	if (bmode >= MAX_TIMING_FORMAT) {
		return;
	}

/*config SDDS*/
	vVgaSetIputIClk(bmode);
	vHdtvSetInputCapature(bmode, 0);

	/*reset clamp win and blank start*/
	if (g_u4SrcType == SRC_VGA) { /*only for VGA*/
		/* [SA7_Michael] 20080415 Overcome VGA-Splitter effect*/
		if (bModeIndex != SYNCONGREEN) {
			vIO32WriteFldAlign(ASYNC_08, (u32)1, AS_VMASK3_OFF);
		} else {
			vIO32WriteFldAlign(ASYNC_08, 0, AS_VMASK3_OFF);
		}

		if (Get_VGAMODE_OverSample(bmode)) { /*if no over sampling*/
			wWindow = (Get_VGAMODE_IPH_BP(bmode))/(u16)3;
			vHdtvClampWin(5, wWindow);
			vHdtvBlankStart(wWindow + (u16)0x10);
		} else {
			wWindow = (Get_VGAMODE_IPH_BP(bmode));

			if (wWindow > (u16)72) {
				if (wWindow > (u16)144) {
					vIO32WriteFldAlign(HDTV_03, 0x02, HDTV_BLANK_AVG);
				} else if (wWindow > (u16)96) {
					vIO32WriteFldAlign(HDTV_03, 0x01, HDTV_BLANK_AVG);
				}

				wWindow = wWindow/(u16)3;
				vHdtvClampWin((u16)5, wWindow + (u16)5);
				vHdtvBlankStart(wWindow + (u16)0x10);
			} else {
				if (wWindow < (u16)50) {
					vHdtvClampWin((u16)0, (u16)18);      /* START=0,  END= 12*/
					vHdtvBlankStart((u16)21);       /*26~42*/
				} else if (wWindow < (u16)60) {
					vHdtvClampWin((u16)0, (u16)24);      /* START=0,  END= 12*/
					vHdtvBlankStart((u16)32);       /*32~48*/
				} else {
					vHdtvClampWin((u16)2, (u16)28);     /* START=0,   END= 12*/
					vHdtvBlankStart((u16)38);       /*38~54*/
				}

				/*
				    wWindow=wWindow>>2;
				    vHdtvClampWin(0, wWindow);      // START=0,     END= 12
				    vHdtvBlankStart(wWindow+8);       //20~36
				  */
			}
		}

		/*vHdtvClampWin(5, 30);*/
		/*vHdtvBlankStart(36);*/
	}

	/* for hdtv timing boundary */
	/*
	if ((bmode>=MODE_720p_50)&&(bmode<=MODE_1080p_60))
	{
	    vDrvAsyncHBDMask(0x40, 0x20);
	}
	*/

	if (bModeIndex == SEPERATESYNC) {
		vDrvAsyncVMask(0 , 0);
	}
}


/**
 * @brief vVgaInitial(void)
 * HW and SW initial setting when mode change.
 * @param  void
 * @retval void
 * @pre  Call it when mode change to reinitial HW and SW Setting.
 */
void vVgaInitial(u8 bReason)
{
	/*UTIL_Printf( "vVgaInitial %d\r\n", bReason);*/

	/* Change H-Lock Sync to SP0 Sync //Modify for Disaplymode delay*/
	vIO32WriteFldAlign(PDWNC_VGACFG3, 0, FLD_RG_SOGY_SORS_PWD);/*power on*/
	vIO32WriteFldAlign(PDWNC_VGACFG3, 0, FLD_RG_SOGY_SINK_PWD);/*power on*/
	vIO32WriteFldAlign(ASYNC_12, 1, AS_DISABLE_VS_OUT);
	/* SW Init*/
	_bVgaHvChkCnt = 0;
	_bVgaTiming = MODE_WAIT;
	bModeIndex = SEPERATESYNC;
	_bPLLlockCnt = 0;
	_bSP0MuxCnt = 0;
	_wSP0StableVtotal = 0;
#if SUPPORT_VGA_USERMODE
	vVgaUsrBroken(); /*roll back if user mode unstable*/
#endif
	bVGAMCErrorCnt = 0;
	bVGAMDStateMachine = VGAMD_GET_ACTIVESYNC; /*ZERO;*/
	_bVgaDelayCnt = (u8)60;

	/*  vVDOINIrqOff(MASK_INT_VSYNC);*/

	/* HW Init*/
	vDrvCLKINFreeRun(ENABLE);
	/*CLKIN free run modify by W.C Shih to prevent the crash of CLKIN 2004/12/30  //ADPART*/
	/*power up pll */
	vASPathReset();  /*CONFIRM*/
	vIO32WriteFldMulti(HDTV_00, P_Fld(0, HDTV_FLD_SEL) | P_Fld((u32)1, HDTV_RGB)
	|P_Fld(0, HDTV_CEN_SEL), HDTV_FLD_SEL | HDTV_RGB | HDTV_CEN_SEL);
	/*Reset all SDTV, PROG,  422 Flag and Enable RGB*/
	SP0Initial();
	vVgaAutoInit();
	/*disable c-deglitch*/
	vIO32WriteFldAlign(ASYNC_00, 0 , AS_CSYNC_DGLITCH_SEL);
#ifdef FULLY_HW_AUTO_CALIBRATION
	vDrvHDTV_HW_AUTO_ONOFF(CALI_DISABLE, (u8)0XFF);
/*   UTIL_Printf("[VGA] -------vDrvHDTV_HW_AUTO_ONOFF----\r\n");*/
#endif
	/*reset the HLEN_ACTIVE*/
	vIO32WriteFldAlign(ASYNC_04, (u32)0x50, AS_HLEN2_S); /*set HLEN2_S*/
	vIO32WriteFldAlign(ASYNC_04, (u32)0x80, AS_HLEN2_E); /*set HLEN2_E*/
}

#if SUPPORT_VGA_USERMODE
void vVgaUserModeTblInit(void)
{
	if (bVgaOpt05_SearchNewMode && (!_bVgaUserMode)) {
		/* loading EEPROM to RAM +setup vga table*/
		u8 i;

		for (i = 0; i < USERMODE_TIMING; i++) {
			/* 1 copy from eeprom */
			vApiEepromDmaRead(EEP_VGA_USR_START + i*sizeof(VGA_USRMODE),
			&rVgaUsrEEP[i], sizeof(VGA_USRMODE));

			/* 2 setup vga table */
			if (rVgaUsrEEP[i].vlen && rVgaUsrEEP[i].hlen && rVgaUsrEEP[i].hsync_w) {
				wSP0Vtotal = rVgaUsrEEP[i].vlen;
				fgSP0Vpol = rVgaUsrEEP[i].vpol;
				wSP0HLength = rVgaUsrEEP[i].hlen;
				fgSP0Hpol = rVgaUsrEEP[i].hpol;
				_bHsyncWidth = rVgaUsrEEP[i].hsync_w;
				/*_bHsyncWidth_var=rVgaUsrEEP[i].hsync_wvar;*/
				/*wSP0HLength_var=rVgaUsrEEP[i].hlen_var;*/
				wSP0Hclk = wSP0IHSClock(wSP0HLength);
				bSP0Vclk = bSP0IVSClock(wSP0HLength, wSP0Vtotal);
				(void)bVgaUsrTimingSearch(1); /*filling*/
			} else {
				_bVgaUserMode_FIFO++;
			}
		}

		_bVgaUserMode_FIFO = bApiEepromReadu8(EEP_VGA_USR_MODE_INDEX);

		/*fix first time eeprom = 0xff problem.*/
		if (_bVgaUserMode_FIFO >= USERMODE_TIMING) {
			_bVgaUserMode_FIFO = 0;
		}

		_bVgaUserMode = 1;
	}
}
#endif

void vVgaAdSpecInit(void)
{

	wVGAADSpec = (u32)743; /*74.3MHz*/

	wHFHeight = (u32)1000; /*100KHz*/

	wHFLow = (u32)130; /*13KHz*/

	wVFHeight = (u32)86; /*86Hz*/

	wVFLow = (u32)49; /*49Hz*/

}


/**
 * @brief   VGA Call-back function for source select connect or disconnect.
 * @param   fgIsOn - SV_ON/SV_OFF
 * @retval  None
 */
void vVgaConnect(bool fgIsOn)
{
/*yunjie mark to fix*/
	/*  vVDOINIrqOff(MASK_INT_VSYNC);*/
	if (fgIsOn) {
		vDrvCLKINFreeRun(ENABLE);
		vVgaAdSpecInit();
		/*vDrvSOGEN(bVgaOpt03_SoGen);*/
		vDrvSOY1EN(bVgaOpt03_SoGen);
#if SUPPORT_VGA_USERMODE
		vVgaUserModeTblInit();
#endif
		vHdtvHwInit();
		vVgaInitial(MDCHG_CON);
		_bVgaModeChged = 0;
		_bVgaChkState = VGA_NO_SIGNAL;
		vVgaSetModeCHG();
		_IsVgaDetectDone = (u8)TRUE;
		/* restore to check the seperate vsync*/
		vIO32WriteFldAlign(ASYNC_00, 0, AS_VSYNC_ACT_SEL);
		HAL_WRITE32(INT_YBR_VGA_MASK, ~(0xFF));
	} else {
		vDrvCLKINFreeRun(ENABLE);
		_IsVgaDetectDone = (u8)TRUE;
		_bVgaTiming = NO_SIGNAL;
		HAL_WRITE32(INT_YBR_VGA_MASK, ~(0xF0));
	}
}


/*u8 bVgaModeDetectDone(void)
{
    return _IsVgaDetectDone;
}*/

/**
 * @brief   VGA call-back function for getting VGA input signal width.
 * @param   None
 * @retval  Height of input signal.
 */
u16 wVgaInputWidth(void)
{

	if ((_IsVgaDetectDone) && (_bVgaTiming != NO_SIGNAL) &&
	(_bVgaTiming != MODE_NOSUPPORT) && (_bVgaTiming < (VGA_SEARCH_END + 1))) {
		return (Get_VGAMODE_IPH_WID(_bVgaTiming)>>(IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL)));
	} else {
		return (u16)2560;
	}
}

/**
 * @brief   VGA call-back function for getting VGA input signal height.
 * @param   None
 * @retval  Height of input signal.
 */
u16 wVgaInputHeight(void)
{
	if ((_IsVgaDetectDone) && (_bVgaTiming != NO_SIGNAL) &&
	(_bVgaTiming != MODE_NOSUPPORT) && (_bVgaTiming < (VGA_SEARCH_END + 1))) {
		return Get_VGAMODE_IPV_LEN(_bVgaTiming);
	} else {
		return (u16)1600;
	}
}

/**
 * @brief   VGA call-back function used by display driver, video mainloop, video ISR general check.
 * @param   None
 * @retval  Refresh rate of input signal.
 */
u8 bVgaRefreshRate(void)
{
	if ((_IsVgaDetectDone) && (_bVgaTiming != NO_SIGNAL) &&
	(_bVgaTiming != MODE_NOSUPPORT) && (_bVgaTiming < (VGA_SEARCH_END + 1))) {
		return Get_VGAMODE_IVF(_bVgaTiming);
	} else if (_bVgaTiming == MODE_NOSUPPORT) {
		return 90;
	} else {
		return 0;
	}
}

/**
 * @brief   VGA call-back function used by DI/Display/Scaler driver
 * @param   None
 * @retval  0 - progressive, 1 - interlace
 */
u8 bVgaInterlace(void)
{
	u8 ret = 0;

	if ((_IsVgaDetectDone) && (_bVgaTiming != NO_SIGNAL) &&
	(_bVgaTiming != MODE_NOSUPPORT) && (_bVgaTiming < (VGA_SEARCH_END + 1))) {
		ret = Get_VGAMODE_INTERLACE(_bVgaTiming);
	}

	return ret;

}

/**
 * @brief   VGA call-back function for vVdoMainState, vVdoPipState polling update video status to UI.
 * @param   None
 * @retval  SV_VDO_UNKNOWN / SV_VDO_NOSIGNAL / SV_VDO_NOSUPPORT / SV_VDO_STABLE
 */
/* mw_if  ignore other state now for 5371*/

u8 bVgaSigStatus(void)
{

	if (!_IsVgaDetectDone) {
		return (u8)SV_VDO_UNKNOWN;
	}
	switch (_bVgaTiming) {
	case MODE_NOSIGNAL:
	case MODE_WAIT:
		return SV_VDO_NOSIGNAL;

	case MODE_NOSUPPORT:
		return SV_VDO_NOSUPPORT;

	default:
		return SV_VDO_STABLE;
	}
}

/**
 * @brief   VGA ISR - Set flags for VGA related mainloop.
 *                                 Set PLL Related Clock Setting/Hold Blank checking and setting
 * @param   None
 * @retval  None
 */

void vVgaISR(void)
{
	/*yunjie mark to fix*/

	if (g_u4SrcType == SRC_VGA) {

		if (fgIsVdoIntSp0Mute() && _IsVgaDetectDone) {
			/*       vVDOINIrqOff(MASK_INT_MUTE);*/

			/* if(fgIsMainVga())*/
			{
				/*vScpipOnVdoModeChange(SV_VP_MAIN);*/
				/* _vDrvVideoSetMute(MUTE_MODULE_MODECHG,SV_VP_MAIN,FOREVER_MUTE,FALSE);*/
				vIO32WriteFldAlign(HDTV_01, 0, HDTV_AV_WIDTH);
			}
		}


		if (fgIsVdoIntSp0Vsyncout()) { /*vsync out*/
#if VGA_AUTO_SPEEDUP

			if (_bAutoISR && _IsVgaDetectDone) {
				if (fgIsAutoFlgSet(SP0_AUTO_CLOCK)) {
					vVgaAutoClkIsr();
				} else {
					vVgaPhaseIsr_New();
				}
			}

#endif /*VGA_AUTO_SPEEDUP*/

			if (fgIsCLKLock()) { /* 5371  //ADPART*/
				_u4GetLockCw = vDrvCLKINGetCwStatus();

				if (_RETIMENeedReset) {
					vDrvRETIMEReset();
				}
			}

			if (_bCLKSetFlag & 0x01) {
				vDrvCLKINSetCW(_u4UiSetCw);
				vDrvCLKINFreeRun(ENABLE);
				vDrvCLKINFreeRun(DISABLE);
				_bCLKSetFlag = 0;
				_RETIMENeedReset = TRUE;
			}

#if CHANGE_SDDS_KPI

			if (fgIsCLKLock()) {
				Set_SDDS_KPI(0);
			}

#endif

			vSetSP0Flg(SP0_VGA_AUTO_FLG | SP0_AUTOCOLOR_FLG);
		}   /*(_IsVgaDetectDone != TRUE)*/
	}       /*fgIsVdoIntSp0Vsyncout*/

}

#if SUPPORT_VGA_AMBIGUOUS_H_DETECT
/**
 * @brief   Set the initial setting for Ambiguous H detectino
 * @param   None
 * @retval  None
 */
void vVgaAmbiguousHInit(void)
{
	_wVgaClock = Get_VGAMODE_IHTOTAL(_bVgaTiming);
	vDrvADCPLLSet(Get_VGAMODE_ICLK(_bVgaTiming), _wVgaClock);   /*ADPART*/
	bSP0VCount = 0;
	_bVgaDelayCnt = 5;/*30*/
	vSetAutoFlg(SP0_AUTO_CLOCK);
}
#endif


/**
 * @brief   When VGA signal is not stable, check signal activity and then get hlen/vlen for timing search.
 * @param   None
 * @retval  None
 */
void vVgaModeDetect(void)
{
	static u16 wTmp;

	if (!(g_u4SrcType == SRC_VGA)) {
		return;
	}

	if (_IsVgaDetectDone) {
		return;
	}
	switch (bVGAMDStateMachine) {
	default:

	/*lint -fallthrough */
	case VGAMD_GET_ACTIVESYNC: /*0:*/
	case VGAMD_GET_HTOTAL_N_STDTIME:
#if VGA_TIMEDELAY_DBG
		HAL_GetTime(&cur_time[2]);
		pr_info("[%d] %d:%d \r\n", 2, cur_time[2].u4Seconds, cur_time[2].u4Micros);
		pr_info("MD2 %d %d\r\n", fgSP0Hpol, fgSP0Vpol);
#endif
		wSP0HLength = wASHLenMeasure();
		bSP0Vclk = bSP0IVSClock(wSP0HLength, wSP0Vtotal);
		wSP0Hclk = wSP0IHSClock(wSP0HLength);
		pr_info("wSP0HLength %d\r\n", wSP0HLength);
		pr_info("wSP0Vtotal %d\r\n", wSP0Vtotal);
		pr_info("bSP0Vclk %d\r\n", bSP0Vclk);
		pr_info("wSP0Hclk %d\r\n", wSP0Hclk);
		vVgaPolarityUniform();
		_bVgaTiming = bVgaStdTimingSearch();
#if SUPPORT_VGA_USERMODE

		if (bVgaOpt05_SearchNewMode && (_bVgaTiming == MODE_NOSUPPORT) &&
		(wSP0HLength > 50) && _bHsyncWidth) {
			_bVgaTiming = bVgaUsrTimingSearch(0);
		}

#endif

		if ((_bVgaTiming != MODE_NOSUPPORT) && (_bVgaTiming != MODE_NOSIGNAL)) {

			vVgaSetInputCapature(_bVgaTiming);
#if SUPPORT_VGA_AMBIGUOUS_H_DETECT

			if (_fgVgaAmbiguousUserSel == SV_FALSE) {
#if 1

				/*move to first ambiguous timing*/
				if ((_bVgaTiming < bAllTimings) && Get_VGAMODE_AmbiguousH(_bVgaTiming)) {
					u8 AmbTimingIndex;

					for (AmbTimingIndex = _bVgaTiming; AmbTimingIndex > 0; AmbTimingIndex--) {
						/*check ambiguous flag and Vtotal*/
						if (Get_VGAMODE_AmbiguousH(AmbTimingIndex) &&
							Get_VGAMODE_IVTOTAL(AmbTimingIndex) ==
							Get_VGAMODE_IVTOTAL(_bVgaTiming)) {
							continue;
						} else {
							break;
						}
					}

#ifndef CC_VGA_VSYNC_WIDTH_AMBIGUOUS_DISABLE

					if (Get_VGAMODE_VSyncWidthChk(_bVgaTiming) == 0)
#endif
						_bVgaTiming = AmbTimingIndex + 1;
				}

#endif

				if (Get_VGAMODE_AmbiguousH(_bVgaTiming + 1) &&
				((_bVgaTiming + 1) < bAllTimings) &&
					(Get_VGAMODE_IVTOTAL(_bVgaTiming) ==
					Get_VGAMODE_IVTOTAL(_bVgaTiming + 1))
#ifndef CC_VGA_VSYNC_WIDTH_AMBIGUOUS_DISABLE
					&& (Get_VGAMODE_VSyncWidthChk(_bVgaTiming) == 0)
#endif
				) {
					vVgaAmbiguousHInit();
					bVGAMDStateMachine = VGAMD_AMBCHK; /*5;*/
					_dMaxDiff = 0;
					/*UTIL_Printf("to 6")   ;*/
					break;
				}
			}

#endif
		}

		vDrvVGASetPhase_Simple(8);
		pr_info("vVgaModeDetect:_bVgaTiming %d\r\n", _bVgaTiming);
		wTmp = bVgaOpt01_MDmute0;
		bVGAMDStateMachine = VGAMD_DETECTDONE;
		break;

	case VGAMD_DETECTDONE:
#if VGA_TIMEDELAY_DBG
		HAL_GetTime(&cur_time[4]);
		pr_debug("[%d] %d:%d \r\n", 4, cur_time[4].u4Seconds, cur_time[4].u4Micros);
		pr_debug("MD4 %d %d\r\n", fgSP0Hpol, fgSP0Vpol);
#endif

		if ((_bVgaTiming) && (_bVgaTiming != MODE_NOSUPPORT) &&
		(_bVgaTiming != MODE_WAIT) && wTmp) {
			wTmp--;
			_bPLLlockCnt++;

			if (!fgIsCLKLock()) {
				break;/*wait for DDS stable*/
			}
			if (bVgaOpt01_MDmute1 && (wTmp > bVgaOpt01_MDmute1)) {
				wTmp = bVgaOpt01_MDmute1;
			}

			break;
		}

		_IsVgaDetectDone = TRUE;
#if SUPPORT_VGA_USERMODE
		_dVgaStableCnt = 0;
#endif

		if (_bVgaTiming == MODE_NOSIGNAL) {
			_bVgaChkState = VGA_NO_SIGNAL;
		} else {
			_bVgaChkState = VGA_CHK_MODECHG;
		}

		if ((_bVgaTiming != MODE_NOSUPPORT)   && (_bVgaTiming != MODE_NOSIGNAL)) {
			/*UTIL_Printf("wASVtotalMeasure() %d\r\n", wASVtotalMeasure());*/
			_wSP0StableVtotal = wASVtotalMeasure();
			pr_debug("Pix Clk Lock Success, Stable VTotal: %d\r\n", _wSP0StableVtotal);
		}

		vVgaSetModeDone();
#if SUPPORT_HDTV_HARDWARE_MUTE
		/* clear SYNC0_MUTE */
		vIO32WriteFldAlign(ASYNC_0D, 1, AS_MUTE_CLR);
		vIO32WriteFldAlign(ASYNC_0D, 0, AS_MUTE_CLR);
#endif

		break;
#if SUPPORT_VGA_AMBIGUOUS_H_DETECT

	case VGAMD_AMBCHK: /*5:*/
		if (_fgVgaAmbiguousUserSel == SV_FALSE) {
			if (fgIsAutoFlgSet(SP0_AUTO_CLOCK)) {
				if (bSP0VCount++ > 254) {
					bVGAMDStateMachine = VGAMD_DETECTDONE;
					vClrAutoFlg(SP0_AUTO_CLOCK);
					/*UTIL_Printf("H fail\r\n");*/
				}
			} else {
				if (_dMaxDiff <= _dPhaseDiff) {
					_bBestTiming =  _bVgaTiming;
					_dMaxDiff = _dPhaseDiff;
				}

#if 0
					UTIL_Printf("_dMaxDiff %x\r\n", _dMaxDiff);
					UTIL_Printf("_dPhaseDiff  %x\r\n", _dPhaseDiff);
					UTIL_Printf("_bBestTiming %x\r\n", _bBestTiming);
#endif

				if (Get_VGAMODE_AmbiguousH(_bVgaTiming + 1)  &&
				((_bVgaTiming + 1) < bAllTimings) &&
				(Get_VGAMODE_IVTOTAL(_bVgaTiming) ==
				Get_VGAMODE_IVTOTAL(_bVgaTiming + 1))
				) {
					_bVgaTiming++;
					vVgaAmbiguousHInit();
					/*UTIL_Printf("next timing %d\r\n", _bVgaTiming);*/
				} else {
					bVGAMDStateMachine = VGAMD_DETECTDONE;
					_bVgaTiming = _bBestTiming;
					vVgaSetInputCapature(_bVgaTiming);
					/*UTIL_Printf("done\r\n");*/
				}
			}
		}

		break;
#endif
	}
}

/*extern u8 bSP0HVCheck(void);*/
/**
* @brief    vVgaSetModeCHG(void);Flag responses mode change
* @param    None
* @retval   None
*/
void vVgaSetModeCHG(void)
{
	if (_bVgaModeChged) {
		return;
	}

	_bVgaModeChged = 1;
	HAL_GetTime(&_rVgaModeChgTime);
	/*
	    if(fgIsMainVga())
	    {
	#ifdef CC_COPLAT_MT5387_TODO
	_vDrvVideoSetMute(MUTE_MODULE_MODECHG, SV_VP_MAIN, FOREVER_MUTE, FALSE);
	#endif
	vClrMainFlg(MAIN_FLG_MODE_DET_DONE);
	vSetMainFlg(MAIN_FLG_MODE_CHG);
	UTIL_Printf( "Main Mode Chg #1 \r\n");
	    }

	    if(fgIsPipVga())
	    {
	#ifdef CC_COPLAT_MT5387_TODO
	_vDrvVideoSetMute(MUTE_MODULE_MODECHG, SV_VP_PIP, FOREVER_MUTE, FALSE);
	#endif
	vClrPipFlg(PIP_FLG_MODE_DET_DONE);
	vSetPipFlg(PIP_FLG_MODE_CHG);
	UTIL_Printf( "Pip Mode Chg #1\r\n");
	    }
	    */
}

void vVgaSetModeDone(void)
{
	if (!_bVgaModeChged) {
		return;
	}

	/*
	    if(fgIsMainVga())
	    {
	vSetMainFlg(MAIN_FLG_MODE_DET_DONE);
	UTIL_Printf( "Main Mode Done\r\n");
	    }

	    if(fgIsPipVga())
	    {
	vSetPipFlg(PIP_FLG_MODE_DET_DONE);
	UTIL_Printf( "Pip Mode Donel\r\n");
	    }
	*/
	_bVgaModeChged = 0;
}

void vSP0SwitchMux(void)
{
	/*UTIL_Printf("_bSP0MuxSate %d \r\n", _bSP0MuxSate);*/
	switch (_bSP0MuxSate) { /*get once at bDrvAsGetActive*/
	case 0:
		_bSP0MuxSate = 1;

		vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(0, FLD_RG_HSYNC_EN) | P_Fld(1,
		FLD_RG_VSYNC_EN) | P_Fld(1, FLD_RG_SOY1_EN),
				   FLD_RG_HSYNC_EN | FLD_RG_VSYNC_EN | FLD_RG_SOY1_EN);
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(8, FLD_RG_SYNC1_VTH) | P_Fld(8, FLD_RG_SYNC1_VTL),
				   FLD_RG_SYNC1_VTH | FLD_RG_SYNC1_VTL);/*SOY*/
		vUtDelay1ms(5);

		if (bASActiveChk() == 0x1) {
			bModeIndex = SYNCONGREEN;
			vASSetSOGSync();
			vIO32WriteFldAlign(ASYNC_00, 1 , AS_CSYNC_DGLITCH_SEL);
			vUtDelay1ms(5);
			pr_debug("set to SOG %d \r\n", bASActiveChk());
			break;
		}

	case 1:
		_bSP0MuxSate = 2;

		vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(1, FLD_RG_HSYNC_EN) | P_Fld(1,
		FLD_RG_VSYNC_EN) | P_Fld(0, FLD_RG_SOY1_EN),
				   FLD_RG_HSYNC_EN | FLD_RG_VSYNC_EN | FLD_RG_SOY1_EN);
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(4, FLD_RG_SYNC1_VTH) | P_Fld(2, FLD_RG_SYNC1_VTL),
				   FLD_RG_SYNC1_VTH | FLD_RG_SYNC1_VTL);/*HSYNC*/
		vUtDelay1ms(5);

		if (bASActiveChk() >= 6) {
			bModeIndex = SEPERATESYNC;
			vASSetSSync();
			pr_debug("set to SEPERATE %d \r\n", bASActiveChk());
			break;
		}

	case 2:
		_bSP0MuxSate = 0;

		vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(1, FLD_RG_HSYNC_EN) |
		P_Fld(1, FLD_RG_VSYNC_EN) | P_Fld(0, FLD_RG_SOY1_EN),
				   FLD_RG_HSYNC_EN | FLD_RG_VSYNC_EN | FLD_RG_SOY1_EN);
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(4, FLD_RG_SYNC1_VTH) | P_Fld(2, FLD_RG_SYNC1_VTL),
				   FLD_RG_SYNC1_VTH | FLD_RG_SYNC1_VTL);/*HSYNC*/
		vUtDelay1ms(5);

		if (bASActiveChk() & 0x4) {
			bModeIndex = COMPOSITESYNC;
			vASSetCSync();
			pr_debug("set to COMPOSITE  %d\r\n", bASActiveChk());
			break;
		}
	}

	vResetVLen();
	_bSP0MuxCnt = 0;
}


#define VGA_FORCE_MODET_DONE_TIME    1 /*sec*/
#define VGA_HLEN_STABLE_THR     2
#define VGA_VLEN_STABLE_THR     2
#define VGA_HLEN_MDCFG_THR      10
#define VGA_VLEN_MDCFG_THR      5
#define WAIT_STABLE_DELAY   1
#if 0
#define VGA_HV_KEEP_STABLE_THR       20
#define VGA_MUX_CHG_CNT 50
#else
u8 VGA_HV_KEEP_STABLE_THR = (u8)5;
u8 VGA_MUX_CHG_CNT = (u8)6;
u8 VGA_STABLE_DEALY = (u8)10;
#endif

void vVgaChkModeChange(void)
{
	u8 bCurrentSignal,  bCurHPol, bCurVPol, bModChg;
	u16 whtemp, wvtemp;

	if (fgIsSP0FlgSet(SP0_MCHG_BYPASS_FLG)
#ifdef CC_SCPOS_PATTERN_GENERATOR
	   || u1GetScposPtGenEnable()
#endif
	  ) {
		return;
	}

	if (g_u4SrcType == SRC_VGA) {
#ifndef CC_UP8032_ATV
#if SUPPORT_VGA_USERMODE

		/*check flush user mode command*/
		if (_bVgaUserMode_Flush) {
			if ((_bVgaUserMode_Flush-bUserVgaTimingBegin) < USERMODE_TIMING) {
				/* for Klocwork warning*/
				x_memset(&rVgaUsrEEP[_bVgaUserMode_Flush-bUserVgaTimingBegin], 0, sizeof(VGA_USRMODE));
			}

			x_memset(&VGATIMING_TABLE[_bVgaUserMode_Flush], 0, sizeof(VGAMODE));
			/*W.C Shih User Mode Modify*/
			_bVgaUserMode_Flush = 0;
		}

#endif
#endif
		bCurrentSignal = bASActiveChk();

		/* check unstable than time out for mode detect done (reduce the transition when mode change)*/
		if (_bVgaModeChged) {
			HAL_TIME_T rCurTime, rDeltaTime;

			HAL_GetTime(&rCurTime);
			HAL_GetDeltaTime(&rDeltaTime, &_rVgaModeChgTime, &rCurTime);

			if ((rDeltaTime.u4Seconds) >= VGA_FORCE_MODET_DONE_TIME) {  /* 2 sec*/
				pr_debug("mc:  Force mode detect done \r\n");
				_bVgaTiming = MODE_NOSIGNAL;
				vVgaSetModeDone();
			}
		}

		if (_bVgaChkState != VGA_NO_SIGNAL) {
			if ((bCurrentSignal == 0)  || (bCurrentSignal == 2)) {
				if (++NoSignalCnt >= 30) { /*6 // for long time to stop audio play*/
					pr_debug("mc: from activity detected to no signal\r\n");
					vVgaInitial(MCHG_NOSIG);

					if (_bVgaTiming != MODE_NOSIGNAL) {
						vVgaSetModeCHG();
						vVgaSetModeDone(); /* for long time to stop audio play*/
					}

					_bVgaTiming = MODE_NOSIGNAL;
					_bVgaChkState = VGA_NO_SIGNAL;
					NoSignalCnt = 0;
				}

				if ((bVgaOpt08_AutoSP0SwitchMux == 1) &&
				(NoSignalCnt%VGA_STABLE_DEALY == 0)) {
					pr_debug("mc:!VGA_NO_SIGNAL to nosignal  SwitchMux\r\n");
					_bSP0MuxSate =  0;
					vSP0SwitchMux();
				}

				return;
			}
			NoSignalCnt = 0;
		}

		wvtemp = wASVtotalMeasure();
		whtemp = wASHLenMeasure();

		switch (_bVgaChkState) {
		case VGA_NO_SIGNAL:
			if ((bCurrentSignal != 0) && (bCurrentSignal != 2)) {
				pr_debug("mc: from no signal to wait stable  %d\r\n", bCurrentSignal);
				_bVgaChkState = VGA_WAIT_STABLE;
				bVGAMCErrorCnt = 0;
				vVgaInitial(MCHG_SIGIN);
#if WAIT_STABLE_DELAY
				_bVgaStableDealy = VGA_STABLE_DEALY;
#else
				_bSP0MuxSate =  0;
				vSP0SwitchMux();

#endif
			} else {
				if ((bVgaOpt08_AutoSP0SwitchMux == 1) && (++NoSignalCnt == VGA_STABLE_DEALY)) {
					/*UTIL_Printf("mc: VGA_NO_SIGNAL to nosignal SwitchMux \r\n");*/
					_bSP0MuxSate =  0;
					vSP0SwitchMux();
					NoSignalCnt  = 0;
				}


			}

			break;

		case VGA_CHK_MODECHG:
			bModChg = MCHG_NO_CHG;

			if (fgIsValidTiming(_bVgaTiming)) {
				if (!fgIsCLKLock()) {
					_bUnLockCnt++;

					if ((_bUnLockCnt > 10) && (!fgIsAutoFlgSet(SP0_AUTO_ALL))) {
						/* vga auto may be unlock*/
						pr_debug("Mchg : Unlock %d\r\n",   _bUnLockCnt);
						bModChg = MCHG_UNLOCK;
					}
				} else {
					_bUnLockCnt = ZERO;
				}


#if SUPPORT_VGA_USERMODE

				if (_dVgaStableCnt <= 10000) {
					_dVgaStableCnt++;
				}

				if (_dVgaStableCnt == 30) {
					vVgaUsrStable();
				}

#endif

				bCurHPol = fgASHPolarityMeasure();
				bCurVPol = fgASVPolarityMeasure();

				if ((bCurHPol != fgSP0Hpol) || (bCurVPol != fgSP0Vpol)) {
					bModChg = MCHG_POL_CHG;
				}
			} /*if(_bVgaTiming !=MODE_NOSUPPORT)*/

			/* Check H/V Lose*/
			if (bModeIndex == SEPERATESYNC) {
				if (bASActiveChk() < 6) { /*Lose H or V*/
					if (++_bVgaHvChkCnt > 10) {
						pr_debug("Mchg : Lose H/V\r\n");
						bModChg = MCHG_HVSYNC_LOSE;
					}
				} else {
					_bVgaHvChkCnt = 0;
				}
			}

			if ((wSP0HLength < (whtemp + VGA_HLEN_MDCFG_THR)) && (wSP0HLength >
			(whtemp - VGA_HLEN_MDCFG_THR)) &&
			   (wSP0Vtotal < (wvtemp + VGA_VLEN_MDCFG_THR)) && (wSP0Vtotal >
			   (wvtemp - VGA_VLEN_MDCFG_THR))) {
				bVGAMCErrorCnt = 0;
			} else {
				bVGAMCErrorCnt++;

				if (bVGAMCErrorCnt > 3) {
					pr_debug("Mchg : H/V Chg\r\n");
					pr_debug("wSP0VCompare0 %d\r\n", wvtemp);
					pr_debug("wSP0Vtotal %d\r\n", wSP0Vtotal);
					pr_debug("wSP0HCompare1 %d\r\n", whtemp);
					pr_debug("wSP0HLength %d\r\n", wSP0HLength);
					bModChg = MCHG_HVLEN_CHG;
				}
			}

#if SUPPORT_HDTV_HARDWARE_MUTE

			if ((IO32ReadFldAlign(STA_SYNC0_01, AS_SYNC0_MUTE) != 0) && (_bVgaTiming
			!= MODE_NOSUPPORT)) {
				bModChg = MCHG_HW_DET;
			}

#endif

			if (_u1Vga422ModeChg) {
				_u1Vga422ModeChg = 0;
				bModChg = MCHG_VGA_422;
			}

			if (bModChg) {
				_bVgaChkState = VGA_WAIT_STABLE;
				bVGAMCErrorCnt = 0;
				/*if (fgIsValidTiming(_bVgaTiming))*/
				{
					vVgaSetModeCHG();
				}
				vVgaInitial(bModChg);
#if WAIT_STABLE_DELAY
				_bVgaStableDealy = VGA_STABLE_DEALY;
#else
				_bSP0MuxSate =  0;
				vSP0SwitchMux();
#endif
			}

			break;

		case VGA_WAIT_STABLE:
#if WAIT_STABLE_DELAY
			if (_bVgaStableDealy != 0) {
				_bVgaStableDealy--;

				/*UTIL_Printf("_bVgaStableDealy %d\r\n", _bVgaStableDealy);*/
				if ((_bVgaStableDealy == 1) && (bVgaOpt08_AutoSP0SwitchMux == 1)) {
					pr_debug("mc: _bVgaStableDealy==1  SwitchMux\r\n");
					_bSP0MuxSate =  0;
					vSP0SwitchMux();
				}

				break;
			}

#endif
#if ASYNC_FULL_SCREEN_WA
			vDrvAsyncSetFieldDet(whtemp);
#endif

			if (((whtemp >= (wSP0HCompare1 - VGA_HLEN_STABLE_THR)) &&
			(whtemp <= (wSP0HCompare1 + VGA_HLEN_STABLE_THR))) &&
			   ((wvtemp >= (wSP0VCompare[0] - VGA_VLEN_STABLE_THR)) &&
			   (wvtemp <= (wSP0VCompare[0] + VGA_VLEN_STABLE_THR)))) {
				if (bVGAMCErrorCnt++ > VGA_HV_KEEP_STABLE_THR) {

					if (bSP0HVCheck()) {
						pr_debug("mc:wait stable to timing search \r\n");
						wSP0HLength = whtemp;
						wSP0Vtotal = wvtemp;
						vVgaSetModeCHG();
						_IsVgaDetectDone = FALSE;
					} else if (bVgaOpt08_AutoSP0SwitchMux == 1) {
						UTIL_Printf("mc:bSP0HVCheck() == 0   SwitchMux \r\n");
						vSP0SwitchMux();
						bVGAMCErrorCnt = 0;
					}
				}
			} else {
				bVGAMCErrorCnt = 0;
			}

/*                UTIL_Printf(  "H/V Clk Chg, V:%d H:%d %d %d\r\n", wvtemp, whtemp, bVGAMCErrorCnt, bCurrentSignal);*/
			/*update  for next check*/
			wSP0HCompare1 = whtemp;
			wSP0VCompare[0] = wvtemp;

			/*if go to timing search , don't change the mux*/
			if (_IsVgaDetectDone) {
				if (_bSP0MuxCnt++ == VGA_MUX_CHG_CNT - 1) {
					bVGAMCErrorCnt_tmp = bVGAMCErrorCnt;
				} else if ((bVgaOpt08_AutoSP0SwitchMux == 1) &&
					  (((_bSP0MuxCnt > VGA_MUX_CHG_CNT)  && (bVGAMCErrorCnt_tmp < 2)
					  && (bVGAMCErrorCnt < 1))
					   || ((_bSP0MuxCnt > VGA_MUX_CHG_CNT*2) && (bVGAMCErrorCnt <
					   VGA_HV_KEEP_STABLE_THR - 1))
					   || (_bSP0MuxCnt > VGA_MUX_CHG_CNT*3)))

				{
					pr_debug("mc:_bSP0MuxCnt > VGA_MUX_CHG_CNT*1/*2/*3   SwitchMux \r\n");
					vSP0SwitchMux();
				}


			}

			break;

		default:
			break;
		}
	}
}
#if SUPPORT_VGA_USERMODE

void vVgaEraseUserMode(u8 index)
{
	/*clean DRAM shadow*/
	u8 i;
	/*DBG_Printf(3,"EEP_VGA_USR_START=%d,sizeof(VGA_USRMODE)=%d\r\n",*/
	/* EEP_VGA_USR_START,sizeof(VGA_USRMODE));*/

	if (index == 0xff) {
		_bVgaUserMode_FIFO = 0;

		for (i = 0; i < bUserVgaTimings; i++) {
#ifndef CC_UP8032_ATV

			if ((bUserVgaTimingBegin+i) != _bVgaTiming) {
				x_memset(&rVgaUsrEEP[i], 0, sizeof(VGA_USRMODE));
				x_memset(&VGATIMING_TABLE[bUserVgaTimingBegin+i], 0, sizeof(VGAMODE));
				/*W.C Shih User Mode Modify*/
			} else {
				_bVgaUserMode_Flush = _bVgaTiming;    /*flush when mode detect*/
			}

#endif
		}

	} else {
		if (index > USERMODE_TIMING) {
			return;
		}

#ifndef CC_UP8032_ATV

		if ((bUserVgaTimingBegin+index) != _bVgaTiming) {
			x_memset(&rVgaUsrEEP[index], 0, sizeof(VGA_USRMODE));
			x_memset(&VGATIMING_TABLE[bUserVgaTimingBegin+index], 0, sizeof(VGAMODE));
			/*W.C Shih User Mode Modify*/
		} else {
			_bVgaUserMode_Flush = _bVgaTiming;    /*flush when mode detect*/
		}

#endif

	}
}

#endif



u16 wVgaGetPorch(u8 bPath, u8 bPorchType)
{
	switch (bPorchType) {
	case    SV_HPORCH_CURRENT:
		return IO32ReadFldAlign(HDTV_01, HDTV_AV_START);

	case    SV_HPORCH_DEFAULT:
		return Get_VGAMODE_IPH_BP(_bVgaTiming);

	case    SV_VPORCH_CURRENT:
		return  IO32ReadFldAlign(ASYNC_11, AS_NEW_VS_OUTP_S1);

	case    SV_VPORCH_DEFAULT:
		return Get_VGAMODE_IPV_STA(_bVgaTiming);

	case    SV_VPORCH_MIN:
#ifdef CC_VGA_UI_POSITION_RANGE_SETTING
		if (Get_VGAMODE_AmbiguousH(_bVgaTiming)) {
			return Get_VGAMODE_IPV_STA(_bVgaTiming)-17;
		} else {
			return wDrvVGAVPositionMin();
		}

#endif
		return wDrvVGAVPositionMin();

	case    SV_VPORCH_MAX:
#ifdef CC_VGA_UI_POSITION_RANGE_SETTING
		if (Get_VGAMODE_AmbiguousH(_bVgaTiming)) {
			return Get_VGAMODE_IPV_STA(_bVgaTiming)+19;
		} else {
			return wDrvVGAVPositionMax();
		}

#endif

		return wDrvVGAVPositionMax();

	case    SV_HPORCH_MIN:
#ifdef CC_VGA_UI_POSITION_RANGE_SETTING
		if (Get_VGAMODE_AmbiguousH(_bVgaTiming)) {
			return Get_VGAMODE_IPH_BP(_bVgaTiming)-100;
		} else {
			return wDrvVGAHPositionMin();
		}

#endif
		return wDrvVGAHPositionMin();

	case    SV_HPORCH_MAX:
#ifdef CC_VGA_UI_POSITION_RANGE_SETTING
		if (Get_VGAMODE_AmbiguousH(_bVgaTiming)) {
			return Get_VGAMODE_IPH_BP(_bVgaTiming)+100;
		} else {
			return 2*Get_VGAMODE_IPH_BP(_bVgaTiming)-wDrvVGAHPositionMin();
		}

#endif

		return wDrvVGAHPositionMax();

	default:
		return 0;
	}
}


void vVgaSetPorch(u8 bPath, u8 bPorchType, u16 wValue)
{
	if (bPorchType == SV_HPORCH_CURRENT) {
		u8 u1Cen = 0;

		u1Cen =  IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL);
#if (!(YPBPR_480IP_27MHZ))

		if ((_bHdtvTiming == MODE_525I_OVERSAMPLE) || (_bHdtvTiming == MODE_625I_OVERSAMPLE)) {
			u1Cen <<= 1;
		}

#endif
		vIO32WriteFldAlign(HDTV_01, (wValue << u1Cen), HDTV_AV_START);
	} else {
		vDrvAsyncVsyncStart(wValue);
	}
}

void vVgaSwReset(void)
{
	vVgaConnect(FALSE);
	vVgaConnect(TRUE);
}

void vVgaStatus(void)
{
	pr_debug("VGA mode[%d]: %d*%d(%c) %dHz %d(clk)\r\n",
		 _bVgaTiming, Get_VGAMODE_IPH_WID(_bVgaTiming),
		 Get_VGAMODE_IPV_LEN(_bVgaTiming),
		 Get_VGAMODE_INTERLACE(_bVgaTiming) ? 'I':'P',
		 Get_VGAMODE_IVF(_bVgaTiming),
		 Get_VGAMODE_ICLK(_bVgaTiming));
	pr_debug("VGA FSM[%d]  DetectDone[%d]\r\n", (int) bVGAMDStateMachine, (int)_IsVgaDetectDone);
	pr_debug("VGA sync[%d] lock[%d]\r\n", (int) bASActiveChk(), (int)fgIsCLKLock());
	pr_debug("VGA hfreq[%d] vfreq[%d] hlen[%d] vlen[%d]\r\n",
		wSP0Hclk, bSP0Vclk, wSP0HLength, wSP0Vtotal);
	pr_debug("VGA CW[%x] CW_STA[%x] Htotal[%d] \r\n",
		(unsigned int)vDrvCLKINGetCW(), (unsigned int)vDrvCLKINGetCwStatus(), (int)wDrvCLKINGetHtotal());
	pr_debug("VGA supports timing[%d]=%d(VGA)+%d(HDTV)  1 entry=[%d]\r\n",
		 (int)bAllTimings, (int)bVgaTimings, (int)bHdtvTimings, sizeof(VGAMODE));
	pr_debug("VGA Phase sum %x\r\n", (unsigned int)dVGAGetAllDiffValue());
#if SUPPORT_VGA_USERMODE
	{
		u8  i;

		pr_debug("VGA user mode:\r\n");

		for (i = bUserVgaTimingBegin; i < bAllTimings; i++) {
			pr_debug("[%d] HF:%d VF:%d CLK:%d [%dx%d]\r\n",
				 i, Get_VGAMODE_IHF(i), Get_VGAMODE_IVF(i),
				 Get_VGAMODE_ICLK(i), Get_VGAMODE_IPH_WID(i),
				 Get_VGAMODE_IPV_LEN(i));
		}
	}
#endif
}


u8 bVgaCurPhase(void)
{
	return bDrvVGAGetPhase();
}

void vVgaRGB2YCbCrSet(void)
{
	vIO32WriteFldMulti(COLORTRANS0, P_Fld(0, EXT_EN) |
			   P_Fld(0x2, IN_TAB_SEL) |
			   P_Fld(0, CT_BYPASS) |
			   P_Fld(1, CT_EN),
			   EXT_EN | IN_TAB_SEL | CT_BYPASS | CT_EN
			  );

}

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
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "x_timer.h"
#endif
#include "drv_hdtv.h"
#include "ybr_vga_common.h"


u8   _bHdtvTiming;
#if ADAPTIVE_SLICER_DEFAULT_SETTING
u8   _bHdtvTiming_old;
#endif

u8   _IsHdtvDetectDone;
u8   _bUnLockCnt;

static u8   NoSignalCnt;

static u8   bHdtvMDStateMachine;
static u8   bHdtvMCErrorCnt;

u8 bActStableCnt;
bool g_fgWchStopped = FALSE;

#define VGA_TIMEDELAY_DBG 0


/*/////////////////////////////////*/
#define CC_HDTV_NON_STANDARD 1
#define SUPPORT_54MHZ_LINECLAMP_EN 0

RHDTVNSTDStatus _rHDTV_NSTDStatus;

/*clamp_start, clamp_end, blank_start, vmask_start, v_mask_end, phase*/
const HDTVTimingPrmSet _arHDTVPrm[] = {
	{0, 5, 30, 36, 0x7, 0xa, 0}, /*default*/
	{MODE_525I_OVERSAMPLE, 0x06, 0xa0, 0xb4, 0x06, 0x0f, 0/*0x1c*/}, /*525i oversample*/
	{MODE_625I_OVERSAMPLE, 0x06, 0xc0, 0xd4, 0x05, 0x12, 0/*0x1c*/}, /*625i oversample*/
	{MODE_480P_OVERSAMPLE, 0x05, 0x38, 0x4c, 0x07, 0x12, 0x0f/*0x10*/}, /*480p oversample*/
	{MODE_576P_OVERSAMPLE, 0x05, 0x40, 0x54, 0x05, 0x12, 0x0f/*0x1c*/}, /*576p oversample  0xf*/
	{MODE_720p_50, 0x28, 0xb0, 0xc4, 0x07, 0x0a, 0x1E/*0x04*/}, /*720p50*/
	{MODE_720p_60, 0x28, 0xb0, 0xc4, 0x07, 0x0a, 0x16/*0x08*/}, /*72060 10*/
	{MODE_1080i_48, 0x28, 0x80, 0x92, 0x07, 0x0a, 0/*0x16*/}, /*1080i48*/
	{MODE_1080i_50, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x15/*0x05*/}, /*1080i50 1F  10*/
	{MODE_1080i, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x1C/*0x1c*/}, /*1080i60  JL 0A*/
	{MODE_1080p_24, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x1A/*0x12*/}, /*1080p24*/
	{MODE_1080p_25, 0x28, 0x80, 0x92, 0x07, 0x0a, 0X15/*0x19*/}, /*1080p25*/
	{MODE_1080p_30, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x1C/*0x12*/}, /*1080p30*/
	{MODE_1080p_50, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x16/*0x1c*/}, /*1080p50*/
	{MODE_1080p_60, 0x28, 0x80, 0x92, 0x07, 0x0a, 0x0C/*0x18*/}, /*1080p60*/
	{MODE_525I, 0x03, 0x50, 0x5A, 0x06, 0x0f, 0x0/*0x1c*/}, /*525i*/
	{MODE_625I, 0x03, 0x60, 0x6a, 0x05, 0x12, 0xc/*0x1c*/}, /*625i*/
	{MODE_480P, 0x01, 0x15, 0x22, 0x07, 0x12, 0/*0x10*/}, /*480p*/
	{MODE_576P, 0x15, 0x22, 0x05, 0x12, 0/*0x10*/}, /*576p*/
	{MODE_720p_24, 0x28, 0xb0, 0xc4, 0x07, 0x0a, 0/*0x04*/}, /*720p24*/
	{MODE_720p_25, 0x28, 0xb0, 0xc4, 0x07, 0x0a, 0x0F/*0x04*/}, /*720p25*/
	{MODE_720p_30, 0x28, 0xb0, 0xc4, 0x07, 0x0a, 0x1E/*0x04*/}, /*720p30*/
	{MODE_240P, 0x06, 0x30, 0x3d, 0x07, 0x0a, 0x00}, /* 240p*/
	{MODE_540P, 0x30, 0x6a, 0x7a, 0x07, 0x0a, 0x00}, /* 540p*/
	{MODE_288P, 0x01, 0x15, 0x22, 0x07, 0x0a, 0x00} /* 288p*/
};

#ifdef CC_HDTV_NON_STANDARD
u8 _bNoSTdCnt;
u16 _wNoStdVtotal;
u32 wHLenDiff;
#endif


u8 _bHdtvMvOn;
u8 _bHdtvMvChgCnt;


u8  _bHdtvChkState;
u8 _bHdtvModeChged;
u8 _bPLLlockCnt;

HAL_TIME_T _rHdtvModeChgTime;
/*
extern void vHdtvSetModeCHG(void);
extern void vHdtvSetModeDone(void);
void vHDTVChkNSTD(u16 VTotal_Measure);
*/
u8 _bHdtvOversampleToDi = (u8)FALSE;
u8 _bHdtvCenEnableForSD = (u8)FALSE;


/*//////////////////////////////////////////////////////////////////////////////*/
/* Local Function*/

/**
 * @brief vvHdtvReset(void)
 * Reset HDTV Decoder.
 * @param  void
 * @VgaInitial used
 */
void vHdtvReset(void)
{
	/*yunjie mark*/
	/*vHdtvResetOn();*/
	vUtDelay2us((u32)20);
	/*vHdtvResetOff();*/
}

/**
 * @brief vVgaSetInput(u16 wstart, u16 wwidth)
 * Input VGA  Horizontal Signal Start Points and width .
 * @param  wstart: Start Points
 * @param wwidth: Scan line width
 * @retval void
 * @pre  Set Start points and san line width from vgatable or from auto action
 */
void vHdtvSetInput(u16 wstart, u16 wwidth)
{
	wwidth >>= (IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL));
	wwidth += DECODER_ADD_WIDTH;
	vIO32WriteFldAlign(HDTV_01, (u32)wstart, HDTV_AV_START);
	vIO32WriteFldAlign(HDTV_01, (u32)wwidth, HDTV_AV_WIDTH);
}

/**
 * @brief vHdtvBlankStart(u16 wclamp_start)
 * Input blanking start positions. The blanking area is (Start position, Start position + 16)
 * @param  wblank_start: blanking start position.
 * @retval void
 * @pre  Need to Set it in a suitable blanking area.
 */
void vHdtvBlankStart(u16 wblank_start)
{
	vIO32WriteFldAlign(HDTV_03, (u32)wblank_start, HDTV_BLANK_START);
}

/**
 * @brief vHdtvClampWin(u16 wclamp_start, u16 wclamp_end)
 * Input Clamping Window Range
 * @param  wclamp_start: Clamping Window Start Position
 * @param wclamp_end: Clamping Window End Position
 * @retval void
 * @pre Need to Set it in a suitable clamping area.
 */
void vHdtvClampWin(u16 wclamp_start, u16 wclamp_end)
{
	vIO32WriteFldAlign(HDTV_04, (u32)wclamp_start, HDTV_CLAMP_START);
	vIO32WriteFldAlign(HDTV_04, (u32)wclamp_end, HDTV_CLAMP_END);
}


void vHdtvADC_Clear(u16 adid, u8 bEn)
{
	if (adid == (u16)1) {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC1_CLR);
	} else if (adid == (u16)2) {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC2_CLR);
	} else {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC3_CLR);
	}

}

void vHdtvADC_Mid(u16 adid, u8 bEn)
{
	if (adid == (u16)1) {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC1_MID);
	} else if (adid == (u16)2) {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC2_MID);
	} else {
		vIO32WriteFldAlign(HDTV_00, bEn, HDTV_ADC3_MID);
	}

}


/**
 * @brief bHdtvTimingSearch(void)
 * HDTV Timing search according to bSP0Vclk & wSP0Hclk
 * @param  void
 * @retval Timing index.
 * @pre  Need to measure H/V Frequence and frequence transform.
 */

#ifdef CC_HDTV_NON_STANDARD
#define HDTV_VSYNC_LOWER_BD  10
#define HDTV_HSYNC_LOWER_BD  10
#else
#define HDTV_VSYNC_LOWER_BD  5
#define HDTV_HSYNC_LOWER_BD  8
#endif
#define HDTV_VTOTAL_BD  50

u8 bHdtvTimingSearch(void)
{
	u8 btime_temp, bsearch;
	u16 wTmpVtotal, wTempVTotalOffset, wMinVTotalOffset;

	u16 wTempHFOffset, wTempVFOffset;
	u32 wHFVFOffsetSum, wMinHFVFOffsetSum = (u32)0xFFFF;

	bsearch = (u8)HDTV_SEARCH_START;    /*HDTV start;*/
	/*UTIL_Printf( "bSP0Vclk %d\r\n", bSP0Vclk);*/
	/*UTIL_Printf( "wSP0Hclk %d\r\n", wSP0Hclk);*/
	btime_temp = MODE_NOSUPPORT;
	wMinVTotalOffset = (u16)0xffff;

	if ((wSP0Hclk < (u16)50) || (bSP0Vclk < (u8)5)) { /*H<5k V<5hz */
		return MODE_NOSUPPORT;
	}

	do {
		if (Get_VGAMODE_YpbprDisabled(bsearch)) {
			continue;
		}

		if ((bSP0Vclk > (u8)(Get_VGAMODE_IVF(bsearch) - (u8)HDTV_VSYNC_LOWER_BD)) &&
		(bSP0Vclk < (u8)(Get_VGAMODE_IVF(bsearch) + (u8)HDTV_VSYNC_LOWER_BD))) {
			if ((wSP0Hclk > (u16)(Get_VGAMODE_IHF(bsearch) - (u8)HDTV_HSYNC_LOWER_BD)) &&
			(wSP0Hclk < (u16)(Get_VGAMODE_IHF(bsearch) + (u8)HDTV_HSYNC_LOWER_BD))) { /*20 too much*/
				/* V total */
				wTmpVtotal = Get_VGAMODE_IVTOTAL(bsearch);

				if (Get_VGAMODE_INTERLACE(bsearch)) {
					wTmpVtotal = wTmpVtotal / (u16)2;
				}

				if (wSP0Vtotal > (u16)1000) { /*for 1080p +-3 vf range*/
					if ((wSP0Vtotal > (wTmpVtotal + (u16)HDTV_VTOTAL_BD + (u16)25)) ||
					(wSP0Vtotal < (wTmpVtotal - (u16)HDTV_VTOTAL_BD - (u16)20))) {
						continue;
					}

				} else  if (wSP0Vtotal > (u16)685) { /*for 720p +-3 vf range*/
					if ((wSP0Vtotal > (wTmpVtotal + (u16)HDTV_VTOTAL_BD + (u16)15)) ||
					(wSP0Vtotal < (wTmpVtotal - (u16)HDTV_VTOTAL_BD - (u16)10))) {
						continue;
					}

				} else

				{
					if ((wSP0Vtotal > (wTmpVtotal + (u16)HDTV_VTOTAL_BD))  ||
					(wSP0Vtotal < (wTmpVtotal - (u16)HDTV_VTOTAL_BD))) {
						continue;
					}
				}

				/* interlace mode measure Vtotal= real Vtotal/2 */
				wTempVTotalOffset = (wSP0Vtotal > wTmpVtotal) ? (wSP0Vtotal - wTmpVtotal)
				: (wTmpVtotal - wSP0Vtotal);

				/* find the closest Vtotal*/
				if (wMinVTotalOffset > wTempVTotalOffset) {
					wMinVTotalOffset = wTempVTotalOffset;
					btime_temp = bsearch;
					/*UTIL_Printf( "the closest Vtotal match %d\r\n", bsearch);*/

					wMinHFVFOffsetSum = ABS(Get_VGAMODE_IHF(bsearch) - wSP0Hclk)
					+ ABS(Get_VGAMODE_IVF(bsearch) - bSP0Vclk);

				}


				else if (wMinVTotalOffset == wTempVTotalOffset) { /*Check (Diff(HF)+Diff(VF))*/
					wTempHFOffset = ABS(Get_VGAMODE_IHF(bsearch) - wSP0Hclk);
					wTempVFOffset = ABS(Get_VGAMODE_IVF(bsearch) - bSP0Vclk);
					wHFVFOffsetSum = (u32)(wTempHFOffset + wTempVFOffset);

					if (wHFVFOffsetSum < wMinHFVFOffsetSum) {
						btime_temp = bsearch;
						wMinHFVFOffsetSum = wHFVFOffsetSum;
			/*UTIL_Printf( "the closest Vtotal + (Diff(HF)+Diff(VF)) match %d\r\n", bsearch);*/
					}
				} else {
				}

			}
		}
	} while ((HDTV_SEARCH_END + 1) != ++bsearch);

	/*for 525i, 625i oversample*/


	if (btime_temp == MODE_525I_OVERSAMPLE) {
		btime_temp = MODE_525I;
	}

	if (btime_temp == MODE_625I_OVERSAMPLE) {
		btime_temp = MODE_625I;
	}

	if (Get_VGAMODE_ICLK(btime_temp) > PIX_CLK_LIMIT) {
		pr_info("pixel clk is large than PIX_CLK_LIMIT\r\n");
		btime_temp = MODE_NOSUPPORT;
	}

	return btime_temp;
}


void vHdtvSetOversampleForSD(u8 bmode)
{
	_bHdtvOversampleToDi = bmode;
}

u8 vHdtvGetOversampleForSD(void)
{
	return _bHdtvOversampleToDi;
}


/**
 * @brief vHdtvSetInputCapature(u8 bmode)
 * According to timing mode to set suitable Mask, windows etc.
 * @param  Timing Mode index
 * @retval void
 * @pre  Need to identify supported timing mode first.
 */
void vHdtvSetInputCapature(u8 bmode, u8 bIsHdtv)  /*set input start and length*/
{
	u16 VS1, VLEN;

	if (bmode >= MAX_TIMING_FORMAT) {
		return;
	}

	vSetAsyncMeasureBD(DOMAIN_27MHz);
	vDrvAsyncSetFieldDet(wSP0HLength);


	if ((bmode == MODE_525I) || (bmode == MODE_625I) || (bmode == MODE_525I_OVERSAMPLE)
	|| (bmode == MODE_625I_OVERSAMPLE)
	    || (bmode == MODE_480P_OVERSAMPLE) || (bmode == MODE_576P_OVERSAMPLE)) {
		if (_bHdtvOversampleToDi) {
			vHdtvCenSel(_bHdtvCenEnableForSD);
		} else {
			vHdtvCenSel((u8)TRUE);
		}


	} else {
		vHdtvCenSel((u8)FALSE);
	}

	vHdtvSetInput(Get_VGAMODE_IPH_BP(bmode), Get_VGAMODE_IPH_WID(bmode));
	vDrvAsyncClampMask((u16)1, (u16)8);

	if (Get_VGAMODE_INTERLACE(bmode) == PROGRESSIVE) {
		/* Disable Field Detection*/
		vIO32WriteFldAlign(ASYNC_12, FIELD_DISABLE, AS_FLD_SELECT); /* 00:ck27,01:pix,10:free run,11:disable*/
	} else {
		/* Set CLK27 Field Detection*/
		vIO32WriteFldAlign(ASYNC_12, FIELD_DET_CLK27, AS_FLD_SELECT); /* Set CLK27 Field Detection*/
		/* 2. ClampMask => Clamping Mask*/
		vDrvAsyncClampMask((u16)1, (u16)8);
	}

	VS1 =  Get_VGAMODE_IPV_STA(bmode);
	VLEN = Get_VGAMODE_IPV_LEN(bmode);

	if (Get_VGAMODE_INTERLACE(bmode)) {
		VLEN >>= 1;
	}



	if (g_u4SrcType == SRC_YBR) {
		u8 bInterlaced;

		bInterlaced = (u8)Get_VGAMODE_INTERLACE(bmode);
		VLEN = VLEN + _wNoStdVtotal - (u16)((u32)Get_VGAMODE_IVTOTAL(bmode) / (u32)(bInterlaced + (u8)1));
	}


	vDrvAsyncVsyncOut(VS1, VLEN);
	vDrvAsyncVMask((u16)7, (u16)10);
	/*-------------------------------*/
	/* Set V-Mask Series*/
	/* 4. VMask2 => 2'th CLIN Mask*/
	vDrvAsyncVMask2((u16)0, (u16)0);
	vIO32WriteFldAlign(ASYNC_0B, (u32)0x04, AS_MASK_SLICE_ST); /*set HLEN2_S*/
	vIO32WriteFldAlign(ASYNC_0B, (u32)0x80, AS_MASK_SLICE_END); /*set HLEN2_E*/
	vIO32WriteFldAlign(ASYNC_08, 0x0, AS_MASK_SLICE_EN);
	/*MV Lock-Window*/
	/*vDrvAsyncVMask2(0x0,0x0);*/
	/* Clamping Window & Blanking*/
	{
		u8 bmodeInx, i;

		bmodeInx = 0;  /*default setting*/

		for (i = 0; i < (sizeof(_arHDTVPrm) / sizeof(HDTVTimingPrmSet)); i++) {
			if (bmode == _arHDTVPrm[i].bTimingIdx) {
				bmodeInx = i;
				break;
			}
		}

		pr_debug("match timing id %d\r\n", bmodeInx);
		vHdtvClampWin((u16)_arHDTVPrm[bmodeInx].bClampStart, (u16)_arHDTVPrm[bmodeInx].bClampEnd);
		vHdtvBlankStart((u16)_arHDTVPrm[bmodeInx].bBlankStart);
		vDrvAsyncVMask((u16)_arHDTVPrm[bmodeInx].bVmaskStart, (u16)_arHDTVPrm[bmodeInx].bVmaskEnd);
		vDrvVGASetPhase(_arHDTVPrm[bmodeInx].bPhase);

		if (((bmode == MODE_525I) || (bmode == MODE_525I_OVERSAMPLE) ||
		(bmode == MODE_625I) || (bmode == MODE_625I_OVERSAMPLE))
		    && (bModeIndex == (u8)SYNCONGREEN)) {
			vIO32WriteFldAlign(ASYNC_08, (u32)0x01, AS_MASK_SLICE_EN);
		}
	}



	if (Get_VGAMODE_OverSample(bmode)) {
		vIO32WriteFldAlign(ASYNC_17, PSNE_ONLY, AS_C_PSNE_STA_SEL);
	}

	switch (bmode) {
	case MODE_1080p_50:
	case MODE_1080p_60:
		break;

	default:
		if (bIsHdtv) {
			vIO32WriteFldAlign(ASYNC_0B, (u32)7, AS_C_DEGLITCH);
			vIO32WriteFldAlign(ASYNC_00, (u32)1 , AS_CSYNC_DGLITCH_SEL);
		}

		break;
	}

	/*Enable vsync_out*/
	vIO32WriteFldAlign(ASYNC_12, 0, AS_DISABLE_VS_OUT);

	vIO32WriteFldAlign(ASYNC_0C, wSP0HLength, AS_H_STABLE_VALUE);
	vIO32WriteFldAlign(ASYNC_0D, wSP0Vtotal, AS_V_STABLE_VALUE);

	/*  set Mute Mask*/
	if (bModeIndex == (u8)SYNCONGREEN) {
		vDrvAsyncSetMuteCriteria(AS_MUTE_CSACT | AS_MUTE_HSPOL | AS_MUTE_VSPOL | AS_MUTE_HLEN | AS_MUTE_VLEN);
	} else {
		vDrvAsyncSetMuteCriteria(AS_MUTE_HSACT | AS_MUTE_HSPOL | AS_MUTE_VSPOL | AS_MUTE_HLEN | AS_MUTE_VLEN);
	}


	_bHdtvMvOn =  bDrvAsyncMvStatus();
	_bHdtvMvChgCnt = 0;

}

void vHdtvHwInit(void)
{
	/*  Sync proc */
	vIO32WriteFldAlign(ASYNC_0E, (u32)1, AS_SP0_PIXEL_EN);
	vIO32WriteFldAlign(ASYNC_02, (u32)1, AS_SP2_EN);
	vASCSSeparatorThre();               /*csync separator threshold*/
	vIO32WriteFldAlign(HDTV_03, 0, HDTV_BLAK_SET);
	vIO32WriteFldAlign(ASYNC_02, (u32)0x159, AS_VACT_MP_TH);   /*for 720P 24Hz*/
	/* Add by Adam for VSYNCP/HSYNCP = 0*/
	vIO32WriteFldAlign(ASYNC_13, 0x0, AS_BDHSYNCP);
	vIO32WriteFldAlign(ASYNC_13, 0x0, AS_BDVSYNCP);
	vIO32WriteFldAlign(ASYNC_12, VS_INPUT_VSYNC_INT, AS_VS_OUT_INT_SEL); /*Vsync interrupt is input signal*/
	/*vsync delay for HLEN/VLEN measurement*/
	vIO32WriteFldAlign(ASYNC_00, 0, AS_VSYNC_DELAY_SEL);
	/* disable auto invert*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1, AS_AUTO_INVP);
	vIO32WriteFldAlign(ASYNC_00, 0, AS_HSYNC_IN_RST_POL);
	/*chang to default value by adam for MV detection and hsync_width*/
	/* set new decomposite algorithm*/
	vASDeCompSel((u8)NEW_DECOMPOSITE2);
	/*for decomp2 method*/
	vIO32WriteFldAlign(ASYNC_18, (u32)16, AS_MAX_RST_CNT_THR);
	vIO32WriteFldAlign(ASYNC_18, (u32)2, AS_TOTAL_LINE_THR);
	vIO32WriteFldMulti(ASYNC_19, P_Fld(0, AS_VMASK4_OFF) | P_Fld(0, AS_VMASK4_INV),
			   AS_VMASK4_OFF | AS_VMASK4_INV);
	/*from 0xB --> 0x10*/
	vIO32WriteFldAlign(ASYNC_01, (u32)0x10, AS_DE_COMP_DIFF_TH); /*set higher value for macrovision.*/
	/* Enable HDTV path clock enable*/
	vIO32WriteFldAlign(HDTV_03, (u32)1, HDTV_EN);
	/* select falling edge reset for - hsync from PLL*/
	vIO32WriteFldAlign(HDTV_03, HDTV_FALLING_EDGE, HDTV_HEDGE_SEL);
	/* set Mv detection range*/
	vDrvAsyncMvDetectV((u16)6, (u16)20); /* from line 6-> 20*/
	vDrvAsyncMvDetectH((u16)0xF, (u16)0xF); /* h ysnc , 15 ->  Htoal -15*/


	if (g_u4SrcType == SRC_YBR) {
		vIO32WriteFldAlign(HDTV_00, 0, HDTV_RGB); /* michael add for mt5360 &&&*/

		/*{1,2,3} = {Y,Pb, Pr}*/
		/*yunjie mark to fix*/
		/*if((P_VGACOMP))*/
		/*{*/
		/*     vIO32WriteFldAlign(HDTV_03, 0x2, HDTV_DATA_SEL);*/
		/*}*/
		/* else*/
		{
			/*Set HDTV DATA SEL : Order=>0x4 :{Y,Pb,Pr} = {1,2,3}*/
			vIO32WriteFldAlign(HDTV_03, (u32)0x4 , HDTV_DATA_SEL);
		}
	} else { /*VGA case*/
		vIO32WriteFldAlign(HDTV_00, (u32)1, HDTV_RGB);
		/*{1,2,3} = {B,G, R}*/
		/*Set HDTV DATA SEL : Order=>0x1 :{Y,Pb,Pr} = {2,1,3}*/
		vIO32WriteFldAlign(HDTV_03, (u32)0x1, HDTV_DATA_SEL);
	}

	/* Get Proper HTotal Value*/
	vIO32WriteFldAlign(ASYNC_04, ENABLE, AS_HLEN_USE_ACTIVE);
	vIO32WriteFldAlign(ASYNC_04, (u32)0x50, AS_HLEN2_S); /*set HLEN2_S*/
	vIO32WriteFldAlign(ASYNC_04, (u32)0x80, AS_HLEN2_E); /*set HLEN2_E*/
	/* Enable Field from sync proc0*/
	vIO32WriteFldAlign(HDTV_00, ENABLE, HDTV_SP0_F);
	/* disable csync de-glitch*/
	vIO32WriteFldAlign(ASYNC_00, 0 , AS_CSYNC_DGLITCH_SEL);
	vIO32WriteFldAlign(ASYNC_0B, (u32)5, AS_C_DEGLITCH);
	/*video mask enable for the picture rolling up after adjusting H-pos*/
	vIO32WriteFldAlign(HDTV_00, (u32)1, HDTV_AVMASK_EN);
	vIO32WriteFldAlign(HDTV_00, (u32)1, HDTV_AVMASK_HEDGE);
	vIO32WriteFldAlign(ASYNC_0E, (u32)1, AS_PIX_VCNT_FLD_RST_EN);
	/*auto skew enable*/
	vIO32WriteFldAlign(ASYNC_03, (u32)1, AS_AUTO_HV_SKEW_CLK27_EN);
	vIO32WriteFldAlign(ASYNC_1A, (u32)1, AS_AUTO_HV_SKEW_PIX_EN);

	vIO32WriteFldAlign(ASYNC_0C, (u32)0x3, AS_H_DIFF_TH);
	vIO32WriteFldAlign(ASYNC_0C, (u32)0x8, AS_MUTE_H_CNT_THR);
	vIO32WriteFldAlign(ASYNC_0D, (u32)0x3, AS_V_DIFF_TH);
	vIO32WriteFldAlign(ASYNC_0D, (u32)0x2, AS_MUTE_V_CNT_THR);
	vIO32WriteFldAlign(ASYNC_0D, (u32)0x3, AS_C_SYNC_H_MUTE);
	vIO32WriteFldAlign(ASYNC_0C, (u32)1, AS_MUTE_EN);
	/*yunjie mark to fix*/
	/*vIO32WriteFldAlign(SYS_05, 1, RETIME_SEL);               //Enable internal HSync & Pixel clock retime*/

	vIO32WriteFldAlign(HDTV_05 , 0, HDTV_YOFFSET);
	/* Field Detection method selection*/
	vIO32WriteFldAlign(ASYNC_12, FIELD_DET_CLK27, AS_FLD_SELECT);
	vIO32WriteFldAlign(ASYNC_03, 0, AS_FIELD_DEF_INV); /*Not Inverse Field*/
	vIO32WriteFldAlign(ASYNC_03, (u32)1, AS_FLD_DET_OLD); /*Use Old Method*/

	/*disable vmask2*/
	/*vIO32WriteFldAlign(ASYNC_0A, 1, AS_VMASK2_OFF);*/
	vIO32WriteFldAlign(ASYNC_08, (u32)1, AS_VMASK2_OFF);
	/*yunjie mark to fix*/
	/*vIO32WriteFldAlign(VSRC_00, 0x01, ADCPLL_VMASK2_SEL);*/
	/*vIO32WriteFldAlign(VSRC_00, 0x01, ADCPLL_VMASK_SEL);*/
	/*Enable HDTV vsync out relatch on output hsync , rising edge*/
	/* Input Vsync 3T delay with input Hsync , av_start min = 3*/
	vIO32WriteFldMulti(HDTV_03, P_Fld(0, HDTV_V_RELATCH_POS) | P_Fld((u32)1, HDTV_V_RELATCH_SEL),
			   HDTV_V_RELATCH_POS | HDTV_V_RELATCH_SEL);
	/*Enable v-deglitch for vga v-sync width measurement*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1 , AS_VSYNC_DGLITCH_SEL);
	vIO32WriteFldAlign(ASYNC_0B, (u32)5, AS_V_DEGLITCH);

	vIO32WriteFldAlign(PDWNC_VGACFG0, 0x0, FLD_RG_VMUX_PWD);
}

#ifdef CC_SOURCE_AUTO_DETECT
void vHdtvHwInit_SrcDetect(void)
{
	/*  Sync proc */
	vIO32WriteFldAlign(ASYNC_0E, (u32)1, AS_SP0_PIXEL_EN);
	vIO32WriteFldAlign(ASYNC_02, (u32)1, AS_SP2_EN);
	vASCSSeparatorThre();               /*csync separator threshold*/
	vIO32WriteFldAlign(HDTV_03, 0, HDTV_BLAK_SET);
	vIO32WriteFldAlign(ASYNC_02, (u32)0x159, AS_VACT_MP_TH);   /*for 720P 24Hz*/
	/* Add by Adam for VSYNCP/HSYNCP = 0*/
	vIO32WriteFldAlign(ASYNC_13, 0x0, AS_BDHSYNCP);
	vIO32WriteFldAlign(ASYNC_13, 0x0, AS_BDVSYNCP);
	vIO32WriteFldAlign(ASYNC_12, VS_INPUT_VSYNC_INT, AS_VS_OUT_INT_SEL); /*Vsync interrupt is input signal*/
	/*vsync delay for HLEN/VLEN measurement*/
	vIO32WriteFldAlign(ASYNC_00, 0, AS_VSYNC_DELAY_SEL);
	/* disable auto invert*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1, AS_AUTO_INVP);
	vIO32WriteFldAlign(ASYNC_00, 0, AS_HSYNC_IN_RST_POL);
	/*chang to default value by adam for MV detection and hsync_width*/
	/* set new decomposite algorithm*/
	vASDeCompSel(NEW_DECOMPOSITE2);
	/*for decomp2 method*/
	vIO32WriteFldAlign(ASYNC_18, (u32)16, AS_MAX_RST_CNT_THR);
	vIO32WriteFldAlign(ASYNC_18, (u32)2, AS_TOTAL_LINE_THR);
	/*from 0xB --> 0x10*/
	vIO32WriteFldAlign(ASYNC_01, (u32)0x10, AS_DE_COMP_DIFF_TH); /*set higher value for macrovision.*/
	/* Enable HDTV path clock enable*/
	vIO32WriteFldAlign(HDTV_03, (u32)1, HDTV_EN);
	/* select falling edge reset for - hsync from PLL*/
	vIO32WriteFldAlign(HDTV_03, HDTV_FALLING_EDGE, HDTV_HEDGE_SEL);
	/*Enable v-deglitch for vga v-sync width measurement*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1 , AS_VSYNC_DGLITCH_SEL);
	vIO32WriteFldAlign(ASYNC_0B, (u32)5, AS_V_DEGLITCH);
}

#endif



u8 _bSetDefaultSlicer_Cnt;
u8 _bSetDefaultSlicer_loop_Cnt;



/**
 * @brief vHdtvInitial(void)
 * HW and SW initial setting when mode change.
 * @param  void
 * @retval void
 * @pre  Call it when mode change to reinitial HW and SW Setting.
 */
void vHdtvInitial(u8 bReason)
{
	/*UTIL_Printf( "hdtv initial %d\r\n", bReason);*/
	vIO32WriteFldAlign(PDWNC_VGACFG3, 0, FLD_RG_SOGY_SORS_PWD);/*power on*/
	vIO32WriteFldAlign(PDWNC_VGACFG3, 0, FLD_RG_SOGY_SINK_PWD);/*power on*/
	/* SW Init*/
	_bPLLlockCnt = 0;
	_bHdtvTiming = MODE_WAIT;
	vIO32WriteFldAlign(ASYNC_12, (u32)1, AS_DISABLE_VS_OUT);
	NoSignalCnt = 0;
	_bAutoISR = 0; /*YPbPr Auto Phase 2006/11/07*/
	vVgaAutoInit(); /*YPbPr Auto Phase 2006/11/07*/

	/* Hw Init*/
	vDrvCLKINFreeRun(ENABLE);
	/*CLKIN free run modify by W.C Shih to prevent the crash of DDS 2004/12/30   //ADPART*/
	vDrvCLKINSyncSel(DCLK_IN_SOG);      /*HS polarity  //ADPART*/
	vIO32WriteFldMulti(HDTV_00, P_Fld(0, HDTV_FLD_SEL) | P_Fld(0, HDTV_RGB) | P_Fld(0, HDTV_CEN_SEL),
			   HDTV_FLD_SEL | HDTV_RGB | HDTV_CEN_SEL); /*Reset all SDTV, PROG, RGB, 422 Flag*/
	/*field select => disable*/
	vIO32WriteFldAlign(ASYNC_12, FIELD_DISABLE, AS_FLD_SELECT); /* 00:ck27,01:pix,10:free run,11:disable*/
	bHdtvMDStateMachine = ZERO;
	bHdtvMCErrorCnt = 0;

	vASSetSOGSync();
	bModeIndex = (u8)SYNCONGREEN; /* for checking the hardware mute setting */

	_bSetDefaultSlicer_Cnt = 0;
	_bSetDefaultSlicer_loop_Cnt = 0;

	SP0Initial();
	/*enable c-deglitch*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1 , AS_CSYNC_DGLITCH_SEL);
	vIO32WriteFldAlign(ASYNC_0B, (u32)5, AS_C_DEGLITCH);
	vDrvHDTV_HW_AUTO_ONOFF(CALI_DISABLE, (u8)0XFF);
	/* UTIL_Printf(" -------vDrvHDTV_HW_AUTO_ONOFF----\r\n");*/

	/*reset the HLEN_ACTIVE*/
	vIO32WriteFldAlign(ASYNC_04, (u32)0x50, AS_HLEN2_S); /*set HLEN2_S*/
	vIO32WriteFldAlign(ASYNC_04, (u32)0x80, AS_HLEN2_E); /*set HLEN2_E*/
}




/**
 * @brief   HDTV Call-back function for source select connect or disconnect.
 * @param   bchannel - SV_VP_MAIN / SV_VP_PIP
 * @param   fgIsOn - SV_ON/SV_OFF
 * @retval  None
 */
void vHdtvConnect(bool fgIsOn)
{
	if (fgIsOn) {
		vDrvCLKINFreeRun(ENABLE);   /*CLKIN free run modify by W.C Shih to prevent the crash of DDS 2004/12/30*/
		_bSP0Flag = 0;
		vHdtvHwInit();
		vHdtvInitial(MDCHG_CON);
		_bHdtvModeChged = 0;
		_bHdtvChkState = HDTV_NO_SIGNAL;
		vHdtvSetModeCHG();
		_IsHdtvDetectDone = (u8)TRUE;
		vIO32WriteFldAlign(INT_YBR_VGA_MASK, 0, INT_ALL);/*Enable All Interrupt*/
	} else {
		vDrvCLKINFreeRun(ENABLE);   /*CLKIN free run modify by W.C Shih to prevent the crash of DDS 2004/12/30*/
		_bHdtvTiming = MODE_NOSIGNAL;
		_IsHdtvDetectDone = (u8)TRUE;
		vIO32WriteFldAlign(INT_YBR_VGA_MASK, (u32)0xF, INT_ALL); /*disable All Interrupt*/
	}

	vSetDefaultSlicer();
	/* also check the decomposited vsync*/
	vIO32WriteFldAlign(ASYNC_00, (u32)1, AS_VSYNC_ACT_SEL);
}


/**
 * @brief   HDTV call-back function for getting HDTV input signal width.
 * @param   None
 * @retval  Height of input signal.
 */
u16 wHdtvInputWidth(void)
{
	if ((_IsHdtvDetectDone) && (_bHdtvTiming != (u8)NO_SIGNAL) && (_bHdtvTiming != (u8)MODE_NOSUPPORT)
	&& (_bHdtvTiming < MAX_TIMING_FORMAT)) {
		return (Get_VGAMODE_IPH_WID(_bHdtvTiming) >> (u16)(IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL)));
	} else {
		return (u16)720;
	}
}


u16 wHdtvDEInputWidth(void)
{
	return (u16)IO32ReadFldAlign(HDTV_01, HDTV_AV_WIDTH);
}

/**
 * @brief   HDTV call-back function for getting HDTV input signal width over sample
 * @param   None
 * @retval  Input width is oversample or not
 */

u8 bHdtvInputWidthOverSample(u8 bMode)
{
	if ((bMode == NO_SIGNAL) || (bMode == MODE_NOSUPPORT)) {
		return (u8)FALSE;
	}

	if (IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL) && (u32)((bMode == (u8)MODE_480P_OVERSAMPLE)
	|| (bMode == MODE_576P_OVERSAMPLE))) {
		return (u8)FALSE;
	} else if (IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL) && (u32)((bMode == (u8)MODE_525I) || (bMode == (u8)MODE_625I))) {
		return (u8)FALSE;
	} else if (_bHdtvOversampleToDi && (u8)((bMode == (u8)MODE_525I_OVERSAMPLE) || (bMode == (u8)MODE_625I_OVERSAMPLE))) {
		/*if drop pixel , return not predown*/
		if (_bHdtvCenEnableForSD) {
			return (u8)FALSE;
		} else {
			return (u8)TRUE;
		}
	} else {
		return Get_VGAMODE_OverSample(bMode);
	}
}


/**
 * @brief   HDTV call-back function for getting HDTV input signal height.
 * @param   None
 * @retval  Height of input signal.
 */
u16 wHdtvInputHeight(void)
{
	if ((_IsHdtvDetectDone) && (_bHdtvTiming != (u8)NO_SIGNAL) &&
	(_bHdtvTiming != MODE_NOSUPPORT) && (_bHdtvTiming < MAX_TIMING_FORMAT)) {
		return Get_VGAMODE_IPV_LEN(_bHdtvTiming);
	} else {
		return (u16)480;
	}
}

/**
 * @brief   HDTV call-back function used by display driver, video mainloop, video ISR general check.
 * @param   None
 * @retval  Refresh rate of input signal.
 */
u8 bHdtvRefreshRate(void)
{
	if ((_IsHdtvDetectDone) && (_bHdtvTiming != (u8)NO_SIGNAL) && (_bHdtvTiming != MODE_NOSUPPORT)
	&& (_bHdtvTiming < MAX_TIMING_FORMAT)) {
		return Get_VGAMODE_IVF(_bHdtvTiming);
	} else {
		return 0;
	}
}

/**
 * @brief   HDTV call-back function used by DI/Display/Scaler driver
 * @param   None
 * @retval  0 - progressive, 1 - interlace
 */
u8 bHdtvInterlace(void)
{
	u8 ret = 0;

	if ((_IsHdtvDetectDone) && (_bHdtvTiming != (u8)NO_SIGNAL) && (_bHdtvTiming < (u8)MAX_TIMING_FORMAT)) {
		ret =  (u8)Get_VGAMODE_INTERLACE(_bHdtvTiming);
	}

	return ret;
}



/**
 * @brief   HDTV call-back function used by Scaler Driver
 * @param   bPath: SV_VP_MAIN/SV_VP_PIP
 * @retval  Vertical Back Porch for SCPOS Setting
 */
u16 wHdtvVPorch(u8 bPath)     /* 2005/01/10 modified by cindy*/
{

	return 0;
}

/**
 * @brief   HDTV call-back function used by Scaler Driver
 * @param   bPath: SV_VP_MAIN/SV_VP_PIP
 * @retval  Horizontal Back Porch for SCPOS Setting
 */
u16 wHdtvHPorch(u8 bPath)
{

	return 0;
}

/**
 * @brief HDTV call-back function for vVdoMainState, vVdoPipState polling update video status to UI.
 * @param   None
 * @retval  SV_VDO_UNKNOWN / SV_VDO_NOSIGNAL / SV_VDO_NOSUPPORT / SV_VDO_STABLE
 */
u8 bHdtvSigStatus(void)
{
	if (!_IsHdtvDetectDone) {
		return (u8)SV_VDO_UNKNOWN;
	}
	switch (_bHdtvTiming) {
	case MODE_NOSIGNAL:
	case MODE_WAIT:
		return SV_VDO_NOSIGNAL;

	case MODE_NOSUPPORT:
		return SV_VDO_NOSUPPORT;

	default:
		return SV_VDO_STABLE;
	}
}

/*extern void vHdtvPhaseIsr(void);*/

/**
 * @brief   Hdtv ISR - Set flags for HDTV related mainloop.
 *                            Hold Blank checking and setting
 * @param   None
 * @retval  None
 */
void vHdtvISR(void)
{

	/**
	    yunjie mark to fix
	*/

	if (g_u4SrcType == SRC_YBR) {
		if (fgIsVdoIntSp0Vsyncout()) { /*vsync out*/
			if (_IsHdtvDetectDone) {
				vSetSP0Flg(SP0_VGA_AUTO_FLG | SP0_AUTOCOLOR_FLG);

				/*YPbPr Auto Phase 2006/11/07*/
				if (_bAutoISR) {

					vYPbPrPhaseIsr_New();

				}

				if (fgIsCLKLock()) {
					Set_SDDS_KPI((u8)0);
				}



				/*read continuous 10 field*/
				if (_bNoSTdCnt == 0) {
					wSP0VCompare[1]  = wASVtotalMeasure();
				} else if (_bNoSTdCnt < (u8)10) {
					if (wASVtotalMeasure() < wSP0VCompare[1]) {
						wSP0VCompare[1]  = wASVtotalMeasure();
					}
				}

				_bNoSTdCnt++;

			}
		}

		if (fgIsCLKLock()) {
			if (_RETIMENeedReset) {
				vDrvRETIMEReset();
			}
		}
	}

}

/* End*/
/*//////////////////////////////////////////////////////////////////////////////*/
/**
 * @brief vHdtvPolarityUniform( void )
 * According to XDATA fgSP0Hpol & fgSP0Vpol to set suitable setting in each step
 * @param  void
 * @retval void
 * @pre  Need to call the API of vHdtvPolarityUniform()
 */
void vHdtvPolarityUniform(void)
{
	vDrvCLKINFreeHsyncPol((u32)1);
	vDrvCLKINHsyncPol((u32)1);
	/*27MHz (Input -/-) goal: --*/
	vDrvCsyncInvPol(0);
	vDrvHsInv((u8)FALSE);
	vDrvVsInv((u8)FALSE);
	/*pixel clk : goal: ++*/
	vDrvHsLockInv((u8)TRUE);
	vDrvVsOutInvPol((u8)TRUE);
}

/**
 * @brief The Non-Standard Signal Status of current input signal of HDTV
 *
 * (Common Function of Video Decoders)
 * return the NSTD Signal Status of current input signal of HDTV.
 *
 * @param pHDTVNSTDStatus  - A pointer to RTvdNSTDStatus to receive HDTV status
 * @return the Signal Status of current video signal, return values include
 * fgIsNSTD, bRefreshRate, wVTotal and wVTotalDiff.
 *
 */
void vHDTVGetNSTDStatus(RHDTVNSTDStatus *pHDTVNSTDStatus)
{
	_rHDTV_NSTDStatus.fgIsNSTD = 0;
	_rHDTV_NSTDStatus.wVTotal = Get_VGAMODE_IVTOTAL(_bHdtvTiming);
	_rHDTV_NSTDStatus.wHTotal = Get_VGAMODE_IHTOTAL(_bHdtvTiming);
	_rHDTV_NSTDStatus.bRefreshRate = Get_VGAMODE_IVF(_bHdtvTiming);
	_rHDTV_NSTDStatus.wVTotalDiff = 0x800;

	pHDTVNSTDStatus->fgIsNSTD = _rHDTV_NSTDStatus.fgIsNSTD;
	pHDTVNSTDStatus->bRefreshRate = _rHDTV_NSTDStatus.bRefreshRate;
	pHDTVNSTDStatus->wVTotal = _rHDTV_NSTDStatus.wVTotal;
	pHDTVNSTDStatus->wHTotal = _rHDTV_NSTDStatus.wHTotal;
	pHDTVNSTDStatus->wVTotalDiff = _rHDTV_NSTDStatus.wVTotalDiff;
}


/**
 * @brief   When VGA signal is not stable, check signal activity and then get hlen/vlen for timing search.
 * @param   None
 * @retval  None
 */
void vHdtvModeDetect(void)
{
	/* Source select will do mode detect and INT will do ,too*/
	static u32 wTmp;

	if (!(g_u4SrcType == SRC_YBR)) {
		return;
	}

	if (!_IsHdtvDetectDone) {
		switch (bHdtvMDStateMachine) {
		case 0:
		case (u8)2:
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[2]);
			pr_info(3, "[%d] %d:%d \r\n", 2, cur_time[2].u4Seconds, cur_time[2].u4Micros);
#endif
			wSP0Hclk = wSP0IHSClock(wSP0HLength);
			bSP0Vclk = bSP0IVSClock(wSP0HLength, wSP0Vtotal);
			vHdtvPolarityUniform(); /*for vlen calculate*/
			_bHdtvTiming = bHdtvTimingSearch();

			pr_info("vHdtvModeDetect:wSP0Vtotal %d, wSP0HLength %d, _bHdtvTiming %d\r\n",
			wSP0Vtotal, wSP0HLength, _bHdtvTiming);

			if (bHdtvOpt05_AdaptiveSlicer && (u8)fgIsVideoTiming(_bHdtvTiming)) {
				pr_info("slicer start\r\n");
				check_quaity_state = 0;
				vSliceQuality536x();/* fine-tune slicer quality here*/
				bHdtvMDStateMachine = (u8)5;
			} else

			{
				bHdtvMDStateMachine = (u8)3;
			}

			break;

		case (u8)3:
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[3]);
			pr_info(3, "[%d] %d:%d \r\n", 3, cur_time[3].u4Seconds, cur_time[3].u4Micros);
#endif
			bHdtvMDStateMachine = (u8)4;

			if ((_bHdtvTiming != MODE_NOSUPPORT) &&
			(_bHdtvTiming != MODE_NOSIGNAL) && (_bHdtvTiming < bAllTimings)) {

				_bNoSTdCnt = 0;
				_wNoStdVtotal =  wSP0Vtotal;
				vDrvVGASetPhase_Simple((u8)8);
				vDrvADCPLLSet(Get_VGAMODE_ICLK(_bHdtvTiming),
				Get_VGAMODE_IHTOTAL(_bHdtvTiming));
				vHdtvSetInputCapature(_bHdtvTiming, (u8)1);
			} else {
				_bHdtvTiming = MODE_NOSUPPORT;
			}

			/* gbsh following move to 4 */
			wTmp = bHdtvOpt01_MDmute0;
			break;

		case (u8)4:
			if ((_bHdtvTiming) && (_bHdtvTiming != MODE_NOSUPPORT) && (_bHdtvTiming != MODE_WAIT) && wTmp) {
				wTmp--;
				_bPLLlockCnt++;

				if (!fgIsCLKLock()) { /*ADPART*/
					break;/*wait for DDS stable*/
				}
				if (bHdtvOpt01_MDmute1 && (wTmp > (u32)bHdtvOpt01_MDmute1)) {
					wTmp = bHdtvOpt01_MDmute1;
				}

				break;  /* 4 more loop for really DDS stable*/
			}

#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[4]);
			pr_info(3, "[%d] %d:%d \r\n", 4, cur_time[4].u4Seconds, cur_time[4].u4Micros);
#endif
			_IsHdtvDetectDone = (u8)TRUE;
			bActStableCnt = 0;

			if (_bHdtvTiming == MODE_NOSIGNAL) {
				_bHdtvChkState = HDTV_NO_SIGNAL;
			} else {
				_bHdtvChkState = HDTV_CHK_MODECHG;
			}

			if ((_bHdtvTiming != MODE_NOSUPPORT)   && (_bHdtvTiming != MODE_NOSIGNAL)) {
				/*Printf("wASVtotalMeasure() %d\r\n", wASVtotalMeasure());*/
				_wSP0StableVtotal = wASVtotalMeasure();
				pr_info("Pix Clk Lock Success, Stable VTotal: %d\r\n", _wSP0StableVtotal);
			}

			vHdtvSetModeDone();

			/*vHDTVChkNSTD(wSP0VCompare[2]);*/
			vHDTVChkNSTD(wSP0Vtotal);

			/* clear SYNC0_MUTE */
			vIO32WriteFldAlign(ASYNC_0D, (u32)1, AS_MUTE_CLR);
			vIO32WriteFldAlign(ASYNC_0D, 0, AS_MUTE_CLR);

			break;

		case (u8)5:
#if VGA_TIMEDELAY_DBG
			HAL_GetTime(&cur_time[5]);
			pr_debug(3, "[%d] %d:%d \r\n", 5, cur_time[5].u4Seconds, cur_time[5].u4Micros);
#endif


			if (check_quaity_state == (u8)10) {
				bHdtvMDStateMachine = (u8)3;
				pr_debug("slicer end\r\n");
			} else {
				vSliceQuality536x();/* fine-tune slicer quality here*/
			}

			break;
		}
	}
}

u8 bSP0HVCheck(void)
{
	u16 wHsync, wVsync;

	wHsync = wASHSyncWidthMeasure();
	wVsync = wASVSyncWidthMeasure();
	/*UTIL_Printf(  "h/v sync width %d %d \r\n", wHsync, wVsync);*/

	if ((wSP0VCompare[0] < 240) || (wSP0VCompare[0] > 1600)) { /*vtotal upper and lower bound*/
		pr_debug("v len over range\r\n");
		return (u8)FALSE;
	}

	if ((wSP0HCompare1 > (u16)2300) || (wSP0HCompare1 < (u16)270)) {
		pr_debug("h len over range\r\n");
		return (u8)FALSE;
	}

	if ((wHsync == 0) || (wVsync == 0)) {
		pr_debug("h/v sync width 0\r\n");
		return (u8)FALSE;
	}



	if ((wVsync > (u16)bVgaOpt06_SogMaxVsyncWidth)  && (u16)(bModeIndex == (u8)SYNCONGREEN)) {
		pr_debug("v sync width fail\r\n");
		return (u8)FALSE;
	}

	if ((bModeIndex == (u8)SYNCONGREEN) || (bModeIndex == (u8)COMPOSITESYNC)) {
		if (IO32ReadFldAlign(STA_SYNC0_02, AS_VSYNC_OUTX_ACT) == 0) {
			pr_debug("vsync outx fail\r\n");
			return (u8)FALSE;
		}
	}

	return (u8)TRUE;
}
/**
* @brief    vHdtvSetModeCHG(void);Flag responses mode change
* @param    None
* @retval   None
*/
void vHdtvSetModeCHG(void)
{
	if (_bHdtvModeChged) {
		return;
	}

	_bHdtvModeChged = (u8)1;
	HAL_GetTime(&_rHdtvModeChgTime);
}

void vHdtvSetModeDone(void)
{
	if (!_bHdtvModeChged) {
		return;
	}

	_bHdtvModeChged = 0;
}

#define HDTV_FORCE_MODET_DONE_TIME    2 /*sec*/
#define HDTV_HLEN_STABLE_THR     2
#define HDTV_VLEN_STABLE_THR     2


#define HDTV_HLEN_MDCFG_THR 3 /* For 5387 HSync & V Line Fix pull in*/
#define HDTV_VLEN_MDCFG_THR 5


/**
 * @brief measure if current input signal is Non-Standard Signal Status or not
 *
 * (Common Function of Video Decoders)
 * return none.
 *
 * @param VTotal_Measure
 * @return none
 *
 */
void vHDTVChkNSTD(u16 VTotal_Measure)
{
	u8 bInterlaced;
	u32 wHDTV_IVF;
	u32 HTotal_Measure_27MHz, HTotal_Std_27MHz;

	bInterlaced = (u8)Get_VGAMODE_INTERLACE(_bHdtvTiming);
	HTotal_Measure_27MHz = wASHLenMeasure();

	if ((HTotal_Measure_27MHz == 0) || (VTotal_Measure == 0)) {
		return;
	}

	if ((_bHdtvTiming != MODE_NOSUPPORT) && (_bHdtvTiming != MODE_NOSIGNAL) && (_bHdtvTiming < bAllTimings)) {
		/*HTotal_Std_27MHz=(CRYSTAL * 1000000 * 2)/Get_VGAMODE_IHF(_bHdtvTiming);*/
		/*HTotal_Std_27MHz=((HTotal_Std_27MHz/100)+1)>>1;*/
		HTotal_Std_27MHz = (u32)(Get_VGAMODE_IHTOTAL(_bHdtvTiming) * (u16)CRYSTAL * (u16)10)
		/ (u32)Get_VGAMODE_ICLK(_bHdtvTiming);

		if (HTotal_Measure_27MHz >= HTotal_Std_27MHz) {
			wHLenDiff = HTotal_Measure_27MHz - HTotal_Std_27MHz;
		} else if (HTotal_Measure_27MHz < HTotal_Std_27MHz) {
			wHLenDiff = HTotal_Std_27MHz - HTotal_Measure_27MHz;
		}
	}

	wHDTV_IVF = (u32)Get_VGAMODE_IVTOTAL(_bHdtvTiming) / (u32)(bInterlaced + (u8)1);

	if (VTotal_Measure >= wHDTV_IVF) {
		_rHDTV_NSTDStatus.wVTotalDiff = 0x800 + ((u32)VTotal_Measure - wHDTV_IVF) * (bInterlaced + (u8)1);
	} else if (VTotal_Measure < wHDTV_IVF) {
		_rHDTV_NSTDStatus.wVTotalDiff = 0x800 - (wHDTV_IVF - (u32)VTotal_Measure) * (bInterlaced + (u8)1);
	}

	if ((VTotal_Measure != wHDTV_IVF) || (wHLenDiff > (u32)3)) {
		_rHDTV_NSTDStatus.fgIsNSTD = 1;

		if (bInterlaced) {
			_rHDTV_NSTDStatus.wVTotal = (VTotal_Measure * (u16)2 + (u16)1);
		} else {
			_rHDTV_NSTDStatus.wVTotal = VTotal_Measure;
		}

		_rHDTV_NSTDStatus.wHTotal = Get_VGAMODE_IHTOTAL(_bHdtvTiming);
		_rHDTV_NSTDStatus.bRefreshRate = (u8)((27000000 * (bInterlaced + (u8)1) +
		((u32)(HTotal_Measure_27MHz) * (u32)(_rHDTV_NSTDStatus.wVTotal)) / (u32)2)
		/ ((u32)(HTotal_Measure_27MHz) * (u32)(_rHDTV_NSTDStatus.wVTotal)));


		pr_debug("==Non_Std==fgIsNSTD=%d\r\n", _rHDTV_NSTDStatus.fgIsNSTD);
		pr_debug("wVTotal=%d\r\n", _rHDTV_NSTDStatus.wVTotal);
		pr_debug("wHTotal=%d\r\n", _rHDTV_NSTDStatus.wHTotal);
		pr_debug("bRefreshRate=%d\r\n", _rHDTV_NSTDStatus.bRefreshRate);
		pr_debug("wVTotalDiff=0x%x\r\n", _rHDTV_NSTDStatus.wVTotalDiff);
		pr_debug("VTotal_Measure=%d\r\n", VTotal_Measure);
		pr_debug("wHLenDiff=%d\r\n", (int) wHLenDiff);
	} else {
		_rHDTV_NSTDStatus.fgIsNSTD = 0;
		_rHDTV_NSTDStatus.wVTotal = Get_VGAMODE_IVTOTAL(_bHdtvTiming);
		_rHDTV_NSTDStatus.wHTotal = Get_VGAMODE_IHTOTAL(_bHdtvTiming);
		_rHDTV_NSTDStatus.bRefreshRate = Get_VGAMODE_IVF(_bHdtvTiming);

		pr_debug("fgIsNSTD=%d\r\n", _rHDTV_NSTDStatus.fgIsNSTD);
		pr_debug("wVTotal=%d\r\n", _rHDTV_NSTDStatus.wVTotal);
		pr_debug("wHTotal=%d\r\n", _rHDTV_NSTDStatus.wHTotal);
		pr_debug("bRefreshRate=%d\r\n", _rHDTV_NSTDStatus.bRefreshRate);
		pr_debug("wVTotalDiff=0x%x\r\n", _rHDTV_NSTDStatus.wVTotalDiff);
		pr_debug("VTotal_Measure=%d\r\n", VTotal_Measure);
		pr_debug("wHLenDiff=%d\r\n", (int)wHLenDiff);
	}
}


u8 HDTV_HV_KEEP_STABLE_THR = (u8)20; /*10;for [DTV00207472]*/
#define _bSetDefaultSlicer_Cnt_th 14

void vHdtvChkModeChange(void)
{
	u8 bCurrentSignal,  bModChg;
	u16 whtemp = 0;
	u16 wvtemp = 0;

	u8 bInterlaced;

	u16 wDDS_MAX_PERR_temp;
	u16 wVwidtemp;

	if (fgIsSP0FlgSet(SP0_MCHG_BYPASS_FLG)) {
		return;
	}

	if (g_u4SrcType != SRC_YBR) {
		return;
	}
	bCurrentSignal = bASHDTVActiveChk();

	/* check unstable than time out for mode detect done*/
	if (_bHdtvModeChged) {
		HAL_TIME_T rCurTime = {0};
		HAL_TIME_T rDeltaTime = {0};

		HAL_GetTime(&rCurTime);
		HAL_GetDeltaTime(&rDeltaTime, &_rHdtvModeChgTime, &rCurTime);

		if ((rDeltaTime.u4Seconds) >= HDTV_FORCE_MODET_DONE_TIME) { /* 2 sec*/
			_bHdtvTiming = MODE_NOSIGNAL;
			pr_info("mc:  Force mode detect done  %d\r\n", _bHdtvTiming);
			vHdtvSetModeDone();
		}
	}

	if (_bHdtvChkState != HDTV_NO_SIGNAL) {
		if ((bCurrentSignal & (u8)0x3) != (u8)3) {
			if (++NoSignalCnt >= 24) { /*6 // for long time to stop audio play*/
				pr_info("mc: from activity detected to no signal\r\n");
				vHdtvInitial(MCHG_NOSIG);

				if (_bHdtvTiming != MODE_NOSIGNAL) {
					vHdtvSetModeCHG();
					vHdtvSetModeDone();
				}

				_bHdtvTiming = MODE_NOSIGNAL;
				_bHdtvChkState = HDTV_NO_SIGNAL;
				vResetSliceTimer();
				vNextSlicer((u16)0);
				NoSignalCnt = 0;
			}
				return;
		}
		NoSignalCnt = 0;
	}

	wvtemp = wASVtotalMeasure();
	whtemp = wASHLenMeasure();
	wVwidtemp = wASVSyncWidthMeasure();

	if (_bHdtvChkState == HDTV_WAIT_STABLE) {
		if (bHdtvMCErrorCnt < (u8)18) {
			vAS2SyncMeasure(SP2_Specific_LVL);
		} else if (bHdtvMCErrorCnt > (u8)20) {
			vAS2SyncMeasure(SP2_Default_LVL);
		}
	} else if (_bHdtvChkState == HDTV_CHK_MODECHG) {
		if (_bSetDefaultSlicer_Cnt < (u8)2) {
			vAS2SyncMeasure(SP2_Specific_LVL);
		} else if (_bSetDefaultSlicer_Cnt >= (u8)4) {
			vAS2SyncMeasure(SP2_Default_LVL);
		}
	}


	switch (_bHdtvChkState) {
	case HDTV_NO_SIGNAL:
		if ((bCurrentSignal & (u8)0x3) == (u8)3) {
			pr_debug("mc: from no signal to wait stable\r\n");
			_bHdtvChkState = HDTV_WAIT_STABLE;
			bHdtvMCErrorCnt = 0;
			vHdtvInitial(MCHG_SIGIN);
			vResetVLen();
			vResetSliceTimer();
		} else {
			vNextSlicer((u16)100);
		}

		break;

	case HDTV_CHK_MODECHG:
		bModChg = MCHG_NO_CHG;

		if (fgIsVideoTiming(_bHdtvTiming)) {
			wDDS_MAX_PERR_temp = (u16)vDDS_MAX_PERR();

			if (!fgIsCLKLock()) {
				_bUnLockCnt++;

				if ((_bUnLockCnt > (u8)20) && (!fgIsAutoFlgSet(SP0_AUTO_ALL))) {
					/* vga auto may be unlock*/
					pr_debug("Mchg : Unlock %d\r\n",   _bUnLockCnt);
					bModChg = MCHG_UNLOCK;
				}
			}

			else if (fgIsCLKLock() && (wDDS_MAX_PERR_temp > (u16)0x400)) { /* >0x700))*/
				_bUnLockCnt++;

				/*UTIL_Printf("  _bUnLockCnt %d vDDS_MAX_PERR %d\r\n",
					bUnLockCnt,wDDS_MAX_PERR_temp);*/
				if ((_bUnLockCnt > (u8)15) && (!fgIsAutoFlgSet(SP0_AUTO_ALL))) {
					pr_debug(" Lock but Error to large : vDDS_MAX_PERR %d\r\n",
					wDDS_MAX_PERR_temp);
					bModChg = MCHG_UNLOCK;
				}
			}

			else {
				_bUnLockCnt = ZERO;
			}

			/* check non-standard signal*/

			if (bHdtvOpt05_AdaptiveSlicer) {
				if (((_bCurSlicerIdx >= (u8)1) || ((_bCurSlicerIdx == (u8)0)
					&& (_bCurSlicerIdx_best != 0)))) {

					if (_bSetDefaultSlicer_Cnt >= (u8)2) {
						/*UTIL_Printf("[1].SP2_V_LEN_S = %d , Cnt =%d, bReadMONSlicer
							=0x%x \r\n",
							SP2VCompare[SP2_Default_LVL].SP2_V_LEN_S,
							bSetDefaultSlicer_Cnt,bReadMONSlicer());*/
					}


					if (((wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S <= (whtemp + (u16)1))
					&& (wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S >= (whtemp - (u16)1)))
					&& ((wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S <= (wvtemp + (u16)1))
					&& (wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S >= (wvtemp - (u16)1)))
					&& ((wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S <= (wVwidtemp + (u16)1))
					&& (wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S) >= (wVwidtemp - (u16)1))
					) {
						if (_bSetDefaultSlicer_Cnt == _bSetDefaultSlicer_Cnt_th) {
							_bSetDefaultSlicer_loop_Cnt++;
							pr_debug("_bSetDefaultSlicer_loop_Cnt = %d \r\n",
							_bSetDefaultSlicer_loop_Cnt);

							if (_bSetDefaultSlicer_loop_Cnt >= (u8)2) {
								pr_debug("[HDTV_CHK_MODECHG] vSetDefaultSlicer\r\n");
								pr_debug("bReadNewSlicer = 0x%x,bReadMONSlicer = 0x%x \r\n",
								bReadNewSlicer(), bReadMONSlicer());
								pr_debug("SP2_H_LEN_S = %d,whtemp = %d \r\n",
								wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S, whtemp);
								pr_debug("SP2_V_LEN_S = %d,wvtemp = %d \r\n",
								wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S, wvtemp);
								pr_debug("SP2_V_WIDTH_S = %d,wVwidtemp = %d \r\n",
								wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S, wVwidtemp);
								pr_debug("SP2_V_LEN_S = %d  \r\n",
								wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S);
								vSetDefaultSlicer();
								_bSetDefaultSlicer_loop_Cnt = 0;
							}
						}
					}

					if (((wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S <= (wvtemp + (u16)1))
					&& (wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S >= (wvtemp - (u16)1)))
						   && (_bSetDefaultSlicer_Cnt >= (u8)8)
					) {
							_bSetDefaultSlicer_Cnt++;
					} else if (_bSetDefaultSlicer_Cnt < (u8)8) {
						_bSetDefaultSlicer_Cnt++;
					} else {
						_bSetDefaultSlicer_Cnt = 0;
						_bSetDefaultSlicer_loop_Cnt = 0;
					}

					if (_bSetDefaultSlicer_Cnt > _bSetDefaultSlicer_Cnt_th) {
						_bSetDefaultSlicer_Cnt = 0;
					}

					if (_bSetDefaultSlicer_Cnt == 0) {
						vSetMONSlicer_Matrix();
					} else if (_bSetDefaultSlicer_Cnt == (u8)3) {
						vSetMONSlicer((u8)0x88);
					} else if (_bSetDefaultSlicer_Cnt == (u8)4) { /* skip transient*/
						vResetVLenSP2();
					} else if (_bSetDefaultSlicer_Cnt == (u8)5) { /* skip transient*/
						vResetVLenSP2();
					}
				}
			} else {
				_bSetDefaultSlicer_Cnt = 0;
				_bSetDefaultSlicer_loop_Cnt = 0;
			}


			bInterlaced = Get_VGAMODE_INTERLACE(_bHdtvTiming);

			if (_bNoSTdCnt > (u8)10) {
				if ((wSP0VCompare[1] - _wNoStdVtotal) > 2 ||
				(wSP0VCompare[1] -  _wNoStdVtotal) < -2) {
					u8 offset;

					vHDTVChkNSTD(wSP0VCompare[1]);
					offset = IO32ReadFldAlign(ASYNC_11, AS_NEW_VS_OUTP_S1);
					/*Get_VGAMODE_IPV_STA(_bHdtvTiming);*/
					_wNoStdVtotal = wSP0VCompare[1];
					pr_debug(" no-std vtal %d\r\n", _wNoStdVtotal);
					wSP0VCompare[1] = ((u32)Get_VGAMODE_IPV_LEN(_bHdtvTiming)
					/ (u32)(bInterlaced + (u8)1)) + _wNoStdVtotal
					- ((u32)Get_VGAMODE_IVTOTAL(_bHdtvTiming)
					/ (u32)(bInterlaced + (u8)1));
					vDrvAsyncVsyncOut(offset, wSP0VCompare[1]);

					/*if wch closed ,need reconnect*/
					if (g_fgWchStopped) {
						/* vHdtvSwReset();*/
						g_fgWchStopped = FALSE;
					}
				}

				_bNoSTdCnt = 0;
			}


		} /*if(_bVgaTiming !=MODE_NOSUPPORT)*/

		if ((wSP0HLength < (whtemp + (u16)HDTV_HLEN_MDCFG_THR))
		&& (wSP0HLength > (whtemp - (u16)HDTV_HLEN_MDCFG_THR))
		&& (wSP0Vtotal < (wvtemp + (u16)HDTV_VLEN_MDCFG_THR))
		&& (wSP0Vtotal > (wvtemp - (u16)HDTV_VLEN_MDCFG_THR))) {
			bHdtvMCErrorCnt = 0;
		} else {
			bHdtvMCErrorCnt++;

			if (bHdtvMCErrorCnt > (u8)3) {
				pr_info("Mchg : H/V Chg\r\n");
				pr_info("wSP0VCompare0 %d\r\n", wvtemp);
				pr_info("wSP0Vtotal %d\r\n", wSP0Vtotal);
				pr_info("wSP0HCompare1 %d\r\n", whtemp);
				pr_info("wSP0HLength %d\r\n", wSP0HLength);
				bModChg = MCHG_HVLEN_CHG;
			}
		}

		if ((IO32ReadFldAlign(STA_SYNC0_01, AS_SYNC0_MUTE) != 0) && (_bHdtvTiming != MODE_NOSUPPORT)) {
			bModChg = MCHG_HW_DET;
		}

		if ((_bHdtvTiming != MODE_NOSUPPORT) && (Get_VGAMODE_IPV_LEN(_bHdtvTiming) <= 720)) {
			/*SDTV Timing , check MV status*/
			if (_bHdtvMvOn != bDrvAsyncMvStatus()) {
				if (_bHdtvMvChgCnt++ > (u8)5) {
					_bHdtvMvOn = bDrvAsyncMvStatus();
					pr_debug("_bHdtvMvOn %d\r\n", _bHdtvMvOn);
					_bHdtvMvChgCnt = 0;
				}
			} else {
				_bHdtvMvChgCnt = 0;
			}
		}


		if (bModChg) {
			_bHdtvChkState = HDTV_WAIT_STABLE;
			bHdtvMCErrorCnt = 0;

			if (fgIsVideoTiming(_bHdtvTiming)) {
				vHdtvSetModeCHG();
			}

			vHdtvInitial(bModChg);
			vResetVLen();
			vResetSliceTimer();
			vNextSlicer(0);
		}

		break;

	case HDTV_WAIT_STABLE:
		vDrvAsyncSetFieldDet(whtemp);

		if (((whtemp >= (wSP0HCompare1 - (u16)HDTV_HLEN_STABLE_THR))
		&& (whtemp <= (wSP0HCompare1 + (u16)HDTV_HLEN_STABLE_THR)))
		&& ((wvtemp >= (wSP0VCompare[0] - (u16)HDTV_VLEN_STABLE_THR))
		&& (wvtemp <= (wSP0VCompare[0] + (u16)HDTV_VLEN_STABLE_THR)))) {
			if (bHdtvMCErrorCnt++ > HDTV_HV_KEEP_STABLE_THR) {
				if (bSP0HVCheck()) {
					pr_debug("mc:wait stable to timing search \r\n");
					/*vHdtvInitial(6);*/
					wSP0HLength = whtemp;
					wSP0Vtotal = wvtemp;
					vHdtvSetModeCHG();
					_IsHdtvDetectDone = (u8)FALSE;

					if (bHdtvOpt05_AdaptiveSlicer) {
						pr_debug("bReadNewSlicer = 0x%x,bReadMONSlicer = 0x%x \r\n",
						bReadNewSlicer(), bReadMONSlicer());
						pr_debug("SP2_H_LEN_S = %d,whtemp = %d \r\n",
						wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S, whtemp);
						pr_debug("SP2_V_LEN_S = %d,wvtemp = %d \r\n",
						wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S, wvtemp);
						pr_debug("SP2_V_LEN_S = %d,wVwidtemp = %d \r\n",
						wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S, wVwidtemp);
						pr_debug("[1].SP2_V_LEN_S = %d \r\n",
						wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S);

						if (((wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S <= (whtemp + (u16)1))
						&& (wSP2VCompare[SP2_Specific_LVL].SP2_H_LEN_S >= (whtemp - (u16)1)))
						&& ((wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S <= (wvtemp + (u16)1))
						&& (wSP2VCompare[SP2_Specific_LVL].SP2_V_LEN_S >= (wvtemp - (u16)1)))
						&& ((wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S <= (wVwidtemp + (u16)1))
						&& (wSP2VCompare[SP2_Specific_LVL].SP2_V_WIDTH_S) >= (wVwidtemp - (u16)1))
						&& ((wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S <= (wvtemp + (u16)1))
						&& (wSP2VCompare[SP2_Default_LVL].SP2_V_LEN_S >= (wvtemp - (u16)1)))
						&& (_bCurSlicerIdx >= (u8)1)) {
							pr_debug("vSetDefaultSlicer\r\n");
							vSetDefaultSlicer();
							vResetSliceTimer();
						}
					}

				} else {
					bHdtvMCErrorCnt = 0;

					wSP0VCompare[2] = (u16)0xFFFF;

				}
			} else {


				if (wASVtotalMeasure() < wSP0VCompare[2]) {
					wSP0VCompare[2] = wASVtotalMeasure();
				}



				if (bHdtvOpt05_AdaptiveSlicer) {
					if (bHdtvMCErrorCnt == (HDTV_HV_KEEP_STABLE_THR / 2)) {
						vSetMONSlicer((u8)0x88);
					} else if (bHdtvMCErrorCnt == ((HDTV_HV_KEEP_STABLE_THR / 2) + 2)) {
						vResetVLenSP2();
					}
				}

			}
		} else {
			bHdtvMCErrorCnt = 0;

			wSP0VCompare[2] = (u16)0xFFFF;

		}

		/*UTIL_Printf(  "H/V Clk Chg, V:%d H:%d %d\r\n", wvtemp, whtemp, bHdtvMCErrorCnt);*/
		vNextSlicer(700);
		/*update  for next check*/
		wSP0HCompare1 = whtemp;
		wSP0VCompare[0] = wvtemp;
		break;

	default:
		break;
	}
}

u16 wHdtvGetPorch(u8 bPath, u8 bPorchType)
{
	u8 u1Cen = 0;

	u1Cen = (u8)IO32ReadFldAlign(HDTV_00, HDTV_CEN_SEL);

	switch (bPorchType) {
	case    SV_HPORCH_CURRENT:

		return (u16)(IO32ReadFldAlign(HDTV_01, HDTV_AV_START) >> u1Cen);

	case    SV_HPORCH_DEFAULT:

		return (Get_VGAMODE_IPH_BP(_bHdtvTiming) >> u1Cen);

	case    SV_VPORCH_CURRENT:
		return  (u16)IO32ReadFldAlign(ASYNC_11, AS_NEW_VS_OUTP_S1);

	case    SV_VPORCH_DEFAULT:
		return (u16)Get_VGAMODE_IPV_STA(_bHdtvTiming);

	case    SV_HPORCH_MAX:
		/*yunjie mark to fix*/
		return 0;/*((wDrvVideoGetHTotal(bPath)-wDrvVideoInputWidth(bPath)));*/

	case    SV_HPORCH_MIN:
		return (u16)10;

	default: /*MinMax*/
		/*yunjie mark to fix*/
		return 0;/*wDrvVideoPorchStd(bPath,bPorchType);*/
	}
}

void vHdtvSetPorch(u8 bPath, u8 bPorchType, u16 wValue)
{
	vVgaSetPorch(bPath, bPorchType, wValue);
}
/******************************5371 diag********************************/
void vHdtvSwReset(void)
{
	vHdtvConnect((bool)FALSE);
	vHdtvConnect((bool)TRUE);
}

void vHdtvStatus(void)
{
	pr_debug("Hdtv mode[%d]: %d*%d(%c) %dHz %d(clk)\r\n",
		 _bHdtvTiming, Get_VGAMODE_IPH_WID(_bHdtvTiming),
		 Get_VGAMODE_IPV_LEN(_bHdtvTiming),
		 Get_VGAMODE_INTERLACE(_bHdtvTiming) ? 'I' : 'P',
		 Get_VGAMODE_IVF(_bHdtvTiming),
		 Get_VGAMODE_ICLK(_bHdtvTiming));
	pr_debug("Hdtv FSM[%d]  DetectDone[%d]\r\n", bHdtvMDStateMachine, _IsHdtvDetectDone);
	pr_debug("Hdtv sync[%d] lock[%d]\r\n", bASHDTVActiveChk(), (int)fgIsCLKLock());
	pr_debug("Hdtv hfreq[%d] vfreq[%d] hlen[%d] vlen[%d]\r\n", wSP0Hclk, bSP0Vclk, wSP0HLength, wSP0Vtotal);
	pr_debug("Hdtv CW[%x] CW_STA[%x] Htotal[%d] \r\n",
	(unsigned int)vDrvCLKINGetCW(), (unsigned int)vDrvCLKINGetCwStatus(), (int)wDrvCLKINGetHtotal());
	pr_debug("VGA supports timing[%d]=%d(VGA)+%d(HDTV)  1 entry=[%d]\r\n",
	bAllTimings, bVgaTimings, bHdtvTimings, sizeof(VGAMODE));
	pr_debug("Hdtv Phase sum %x\r\n", (int) dVGAGetAllDiffValue());
	pr_debug("==Non_Std==fgIsNSTD=%d\r\n", _rHDTV_NSTDStatus.fgIsNSTD);
	pr_debug("wVTotal=%d\r\n", _rHDTV_NSTDStatus.wVTotal);
	pr_debug("wHTotal=%d\r\n", _rHDTV_NSTDStatus.wHTotal);
	pr_debug("bRefreshRate=%d\r\n", _rHDTV_NSTDStatus.bRefreshRate);
	pr_debug("wVTotalDiff=0x%x\r\n", _rHDTV_NSTDStatus.wVTotalDiff);
	pr_debug("wHLenDiff=%d\r\n", (int)wHLenDiff);
}

void vHdtvRGB2YCbCrSet(void)
{
	vIO32WriteFldMulti(COLORTRANS0, P_Fld((u32)1, CT_BYPASS) | P_Fld(0, CT_EN),
			   CT_BYPASS | CT_EN);
}

void vHdtvRGB2YCbCrAdjustSet(u16 u2Gain[3], u8 u1PreAddr[3],
			     u8 u1PostAddr[3])
{
	vIO32WriteFldMulti(COLORTRANS0, P_Fld((u32)1, EXT_EN) |
			   P_Fld(0, CT_BYPASS) |
			   P_Fld((u32)1, CT_EN) |
			   P_Fld(u2Gain[0], CH1_GAIN),
			   EXT_EN | CT_BYPASS | CT_EN | CH1_GAIN);
	vIO32WriteFldMulti(COLORTRANS1, P_Fld(u2Gain[1], CH2_GAIN) |
			   P_Fld(u2Gain[2], CH3_GAIN),
			   CH2_GAIN | CH3_GAIN);
	vIO32WriteFldMulti(COLORTRANS2, P_Fld(u1PreAddr[0], CH1_PRE_ADDR) |
			   P_Fld(u1PreAddr[1], CH2_PRE_ADDR) |
			   P_Fld(u1PreAddr[2], CH3_PRE_ADDR) ,
			   CH1_PRE_ADDR | CH2_PRE_ADDR | CH3_PRE_ADDR);
	vIO32WriteFldMulti(COLORTRANS3, P_Fld(u1PostAddr[0], CH1_POST_ADDR) |
			   P_Fld(u1PostAddr[1], CH2_POST_ADDR) |
			   P_Fld(u1PostAddr[2], CH3_POST_ADDR) ,
			   CH1_POST_ADDR | CH2_POST_ADDR | CH3_POST_ADDR);
}


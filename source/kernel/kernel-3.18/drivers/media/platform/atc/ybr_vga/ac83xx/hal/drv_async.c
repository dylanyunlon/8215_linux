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

#include "drv_async.h"


/* SP0*/
u16   wSP0Hclk;
u8   bSP0Vclk;
u16   wSP0Vtotal;
u16  _wSP0StableVtotal;
u16   wSP0HLength;
u8   fgSP0Hpol;
u8   fgSP0Vpol;
u8   _bSP0Flag;

u16   wSP0VCompare[4];
u16   wSP0HCompare1;
u8   bSP0VCount;
u32 wVGAADSpec;
u32 wHFHeight;
u32 wHFLow;
u32 wVFHeight;
u32 wVFLow;


SP2MonStr wSP2VCompare[wSP2VCompare_Num];

/**
 * @brief void vASCSSeparator(void)
 * Set csync separator threshold
 * @param   void
 * @retval  void
 */
void vASCSSeparatorThre(void)
{
	/*set Schmitt trigger H/L thre and upper bound*/
	vIO32WriteFldAlign(ASYNC_01, CS_SEPARATOR_THU_DEFAULT, AS_CSYNC_CONT_THU);
	vIO32WriteFldAlign(ASYNC_01, CS_SEPARATOR_THL_DEFAULT, AS_CSYNC_CONT_THL);
	vIO32WriteFldAlign(ASYNC_02, CS_SEPARATOR_THH_DEFAULT, AS_CSYNC_CONT_THH);
}

u8 bASGetSyncMode(void)
{
	return (u8)IO32ReadFldAlign(ASYNC_00, AS_SYNC_SEL);
}

u8 bDrvASVsyncOutAct(void)
{
	return (u8)IO32ReadFldAlign(STA_SYNC0_02, AS_VSYNC_OUTX_ACT);
}
/**
 * @brief u8 fgASHPolarityMeasure(void)
 * Get H sync polarity
 * @param   void
 * @retval  Hsync polarity
 */
u8 fgASHPolarityMeasure(void)
{
	return (u8)IO32ReadFldAlign(STA_SYNC0_00, AS_HSYNC_P);
}

/**
 * @brief u8 fgASVPolarityMeasure(void)
 * Get Vsync polarity
 * @param   void
 * @retval  Vsync polarity
 */
u8 fgASVPolarityMeasure(void)
{
	return (u8)IO32ReadFldAlign(STA_SYNC0_00, AS_VSYNC_P);
}

/**
 * @brief u16  wASHLenMeasure(void)
 * Get  counter values in 27MHz between two hsyncs
 * @param   void
 * @retval  Hlen counter
 */
u16 wASHLenMeasure(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_00, AS_H_LEN_S);
}

/**
 * @brief u16  wASHSyncWidthMeasure(void)
 * Get  counter values in 27MHz of Hsync width
 * @param   void
 * @retval  Hlen counter
 */
u16 wASHSyncWidthMeasure(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_01, AS_HSYNC_WIDTH_S);

}

/**
 * @brief u16  wASVSyncWidthMeasure(void)
 * Get  counter values in 27MHz of Hsync width
 * @param   void
 * @retval  Hlen counter
 */
u16 wASVSyncWidthMeasure(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_01, AS_VSYNC_WIDTH_S);
}

/**
 * @brief u16 wASVtotalMeasure(void)
 * Get line numbers between two vsyncs
 * @param   void
 * @retval  vtotal number
 */
u16 wASVtotalMeasure(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_00, AS_V_LEN_S);
}


/**
 * @brief u16 wASHLenPixMeasure(void)
 * Get  counter values in pix clk between two hsyncs
 * @param   void
 * @retval  Hlen counter
 */
u16 wASHLenPixMeasure(void)
{
	u16 wHLenPix = 0;
	u16 wHTotal = 0;

	wHLenPix = (IO32ReadFldAlign(STA_SYNC0_04, AS_H_LEN_PIX_S_7_0)
		    + (IO32ReadFldAlign(STA_SYNC0_06, AS_H_LEN_PIX_S_11_8) << 8)
		    + (IO32ReadFldAlign(STA_SYNC0_0A, AS_H_LEN_PIX_S_12) << 12));
	wHTotal = (u16)wDrvCLKINGetHtotal();

	if (wHLenPix != (u16)wDrvCLKINGetHtotal()) {
		pr_debug("wHLenPix:%d wHTotal:%d\r\n", wHLenPix, wHTotal);
	}

	return wHLenPix;
}

void vAS2SyncMeasure(SP2Mon_LVL type)
{
	if (type < wSP2VCompare_Num) {
		/*longer than u8 status hold.*/
		wSP2VCompare[type].SP2_V_LEN_S = (u16)IO32ReadFldAlign(STA_SYNC2_01, AS2_V_LEN_S);
		wSP2VCompare[type].SP2_H_LEN_S = (u16)IO32ReadFldAlign(STA_SYNC2_01, AS2_H_LEN_S);
		wSP2VCompare[type].SP2_V_WIDTH_S = (u16)IO32ReadFldAlign(STA_SYNC2_00, AS2_VSYNC_WIDTH_S);
	}
}

u16 wASTopBCLine(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_06, AS_NEWTOPBC_S);
}


u16 wASBottomBCLine(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_06, AS_NEWBOTTOMBC_S);
}

/**
 * @brief u8 bASActiveChk(void)
 * get sync processor activity bits.
 * @param   void
 * @retval  H/V/SOG activity
 */
u8 bASActiveChk(void)
{
	return (u8)IO32ReadFldAlign(STA_SYNC0_00, AS_SYNC_ACT);
}

/**
 * @brief u8 bASHDTVActiveChk(void)
 * get sync processor activity bits.
 * @param   void
 * @retval  CSYNC & VSYNC_OUT activity
 */
u8 bASHDTVActiveChk(void)
{
	return (u8)((IO32ReadFldAlign(STA_SYNC0_00, AS_CSYNC_ACT) | (u32)(bDrvASVsyncOutAct() << 1)) & (0x3U));
}


/**
 * @brief void vDrvAsyncBDMask(u16 wstart1, u16 wend1)
 * set the Bouandary Mask
 * @param   wstart1, wend1 : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncBDMask(u16 wstart1, u16 wend1)
{
	/*Start*/
	vIO32WriteFldAlign(ASYNC_13, (u32)wstart1, AS_BDMASK_ST);
	/*End*/
	vIO32WriteFldAlign(ASYNC_13, (u32)wend1, AS_BDMASK_END);
}
/**
 * @brief void vDrvAsyncVMask(u16 wstart1, u16 wend1)
 * set the VMask1 to PLL coast mask
 * @param   wstart1, wend1 : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncVMask(u16 wstart1, u16 wend1)
{
	/* Start*/
	vIO32WriteFldAlign(ASYNC_0A, (u32)wstart1, AS_VMASK1_ST);
	/* End*/
	vIO32WriteFldAlign(ASYNC_0A, (u32)wend1, AS_VMASK1_END);
}

/**
 * @brief void vDrvAsyncVMask2(u16 wstart1, u16 wend1)
 * set the VMask2 for another PLL parameter
 * @param   wstart1, wend1 : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncVMask2(u16 wstart1, u16 wend1)
{
	/*Start*/
	vIO32WriteFldAlign(ASYNC_0A, (u32)wstart1, AS_VMASK2_ST);
	/*End*/
	vIO32WriteFldAlign(ASYNC_0A, (u32)wend1, AS_VMASK2_END);
}

/**
 * @brief void vDrvAsyncClampMask(u16 wstart1, u16 wend1)
 * set the VMask3 for clamping mask
 * @param   wstart1, wend1 : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncClampMask(u16 wstart1, u16 wend1)
{
	/*Start*/
	vIO32WriteFldAlign(ASYNC_18, (u32)wstart1, AS_VMASK3_ST);
	/*End*/
	vIO32WriteFldAlign(ASYNC_18, (u32)wend1, AS_VMASK3_END);
}

/**
 * @brief void vDrvAsyncPreDataActive(u16 wstart)
 * set the post margin for data active
 * @param   wstart: line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncPreDataActive(u16 wstart, u16 wend)
{
	vIO32WriteFldAlign(ASYNC_0E, (u32)(wstart >> 12), AS_PRE_DATA_ACTIVE_12);
	vIO32WriteFldAlign(ASYNC_0E, (u32)(wend >> 12), AS_POST_DATA_ACTIVE_12);
	vIO32WriteFldAlign(ASYNC_0E, (u32)wstart, AS_PRE_DATA_ACTIVE_11_0);
	vIO32WriteFldAlign(ASYNC_0E, (u32)wend, AS_POST_DATA_ACTIVE_11_0);
}

/**
 * @brief void vDrvAsyncHBDMask(u16 Left, u16 Right)
 * set the left/right for data active
 * @param   left from hsync , right count form hsync
 * @retval  void
 */
void vDrvAsyncHBDMask(u16 Left, u16 Right)
{
	/*Boundary active region*/
	/*Start : left from hsync*/
	/*End : Htotal - right*/
	/* Left*/
	vIO32WriteFldAlign(ASYNC_16, (u32)(Left >> 12), AS_H_BD_MASK_L_12);
	vIO32WriteFldAlign(ASYNC_15, (u32)Left, AS_H_BD_MASK_L_11_0);
	/* Right*/
	vIO32WriteFldAlign(ASYNC_16, (u32)(Right >> 12), AS_H_BD_MASK_R_12);
	vIO32WriteFldAlign(ASYNC_15, (u32)Right, AS_H_BD_MASK_R_11_0);
}
/**
 * @brief void vSetAsyncMeasureBD(u8 bmode)
 * set the boundary mask according to the timing table
 * @param   bmode : VGA/HDTV Timing number
 * @retval  void
 */
void vSetAsyncMeasureBD(u8 bmode)
{
	if (bmode == DOMAIN_PIXEL) {
		u16 wHSyncWidthPix;
		u16 wLeftBD, wRightBD;
		u8 bTiming;

		wHSyncWidthPix = IO32ReadFldAlign(STA_SYNC0_05, AS_HSYNC_WIDTH_PIX_S_7_0)
				 + (IO32ReadFldAlign(STA_SYNC0_06, AS_HSYNC_WIDTH_PIX_S_11_8) << 8U)
				 + (IO32ReadFldAlign(STA_SYNC0_0A, AS_HSYNC_WIDTH_PIX_S_12) << 12U);
		UTIL_Printf("[SA7][1]wHSyncWidthPix=%u \r\n", wHSyncWidthPix);
#if 0
		wHLenPix =  IO32ReadFldAlign(STA_SYNC0_04, AS_H_LEN_PIX_S_7_0)
			    + (IO32ReadFldAlign(STA_SYNC0_06, AS_H_LEN_PIX_S_11_8) << 8)
			    + (IO32ReadFldAlign(STA_SYNC0_0A, AS_H_LEN_PIX_S_12) << 12);
		UTIL_Printf("[SA7]wHLenPix=%u \r\n", wHLenPix);

		if ((wHLenPix >> 1) < wHSyncWidthPix) {
			wHSyncWidthPix = wHLenPix - wHSyncWidthPix;
			UTIL_Printf("[SA7][2]wHSyncWidthPix=%u \r\n", wHSyncWidthPix);
		}

#endif
		/*Set Boundary Mask*/
		vDrvAsyncBDMask((u16)7, (u16)1); /* 15 -> 7 for 1680x1050 vstart too small problem.*/
		/*vDrvAsyncBDMask(0x0, 0xa00); // 15 -> 7 for 1680x1050 vstart too small problem.*/
		/* set pre-data and post-data , from h sync count forward and backword (for auto color)*/
		vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);

		if (wHSyncWidthPix > (u16)500) {
			wLeftBD = wRightBD = (u16)10;
		} else {
			wLeftBD = (u16)10;
			wRightBD = (u16)5 + wHSyncWidthPix;
		}

		/*set for SOG tri-level sync 1080P/720P*/

		if (bModeIndex == (u8)SYNCONGREEN) {
			if (g_u4SrcType == SRC_YBR) {
				bTiming = _bHdtvTiming;
			} else {
				bTiming = _bVgaTiming;
			}

			if (((Get_VGAMODE_IPH_WID(bTiming) == (u16)1280) && (Get_VGAMODE_IPV_LEN(bTiming) == (u16)720)) ||
			    ((Get_VGAMODE_IPH_WID(bTiming) == (u16)1920) && (Get_VGAMODE_IPV_LEN(bTiming) == (u16)1080))
			   ) {
				wLeftBD = (u16)0x40;
			}
		}

		vDrvAsyncHBDMask(wLeftBD , wRightBD);
	} else {
		vDrvAsyncBDMask((u16)7, (u16)1); /*for 848x480 60 CVT-RB*/
		/* set pre-data and post-data , from h sync count forward and backword (for auto color)*/
		vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);
		vDrvAsyncHBDMask((u16)10, (u16)10);
	}
}
/**
 * @brief void vDrvAsyncVsyncStart(u16 wStart)
 * Set programmable  vsync start
 * @param   vsync start
 * @retval  void
 */
void vDrvAsyncVsyncStart(u16 wStart)
{
	u16 u2Height;

	u2Height = IO32ReadFldAlign(ASYNC_11, AS_NEW_VS_OUTP_H1);
	vDrvAsyncVsyncOut(wStart, u2Height);
}

/**
 * @brief void vDrvAsyncVsyncOut(u16 wStart1, u16 wH1)
 * Set programmable  vsync out S1,H1, S2/H2
 * @param   wStart1, wH1, : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncVsyncOut(u16 wStart1, u16 wH1)
{
	u8 bMode;

	if (g_u4SrcType == SRC_YBR) {
		bMode = _bHdtvTiming;
	} else {
		bMode = _bVgaTiming;
	}

	/*wH1 = wH1 + 1 ; // for HD full screen request.*/

	vIO32WriteFldAlign(ASYNC_12, 0x01U, AS_VS_UP_GATE);
	vIO32WriteFldAlign(ASYNC_12, 0x00U, AS_VS_UPDATE_EN);
	/* Field 0*/
	/* set Vsync Start for field 0*/
	vIO32WriteFldAlign(ASYNC_11, (u32)(wStart1 - (u16)1), AS_NEW_VS_OUTP_S1);
	/* set Vsync end for field 0*/
	vIO32WriteFldAlign(ASYNC_11, (u32)(wH1 + (u16)1), AS_NEW_VS_OUTP_H1);


	if (Get_VGAMODE_INTERLACE(bMode) && (bModeIndex != (u8)SEPERATESYNC)) {
		wStart1++;
	}


	/* Field 1*/
	/* set Vsync Start for field 1*/
	vIO32WriteFldAlign(ASYNC_12, (u32)(wStart1 - (u16)1), AS_NEW_VS_OUTP_S2);
	/* set Vsync end for field 1*/
	vIO32WriteFldAlign(ASYNC_12, (u32)(wH1 + (u16)1), AS_NEW_VS_OUTP_H2);
	vIO32WriteFldAlign(ASYNC_12, (u32)0x01, AS_VS_UPDATE_EN);
	vIO32WriteFldAlign(ASYNC_12, (u32)0x00, AS_VS_UPDATE_EN);

}

/**
 * @brief void  vDrvAsyncSetFieldDet(void)
 * Set sync processor field detection window
 * @param   void
 * @retval  void
 */
void  vDrvAsyncSetFieldDet(u16 wHLEN)
{
	/*u16 wHLEN;*/
	u16 wTmp = 0;
	/*wHLEN = wASHLenMeasure();*/

	if (bModeIndex != SEPERATESYNC)
		/*Add decomposite delay*/
		wTmp = (u16)(IO32ReadFldAlign(ASYNC_01, AS_DE_COMP_DIFF_TH) * 8U);


	/* FIELD_DETECT_S*/
	vIO32WriteFldAlign(ASYNC_03, (u32)((wHLEN >> 2) + wTmp), AS_FLD_DETECT_S);
	/* FIELD_DETECT_E*/
	vIO32WriteFldAlign(ASYNC_03, (u32)(((wHLEN * (u16)3) >> 2) + wTmp), AS_FLD_DETECT_E);
}

/**
 * @brief u16 wSP0IHSClock(void)
 * Calculate Hsync clock , IC count hsync number with crystal clock,then we should translate to real hsync clock
 * @param   whtotal : H len count measured in 27MHz,
 * @retval  H frequency
 */
u16 wSP0IHSClock(u16 whlen)
{
	u32 lhsync_freq;
	/*multiply 2 for increasing 1bit resolution*/
	/*multiply 100000 for Mega scale*/
	lhsync_freq = whlen;

	if (lhsync_freq == ZERO) {
		return 0;
	}

	lhsync_freq = (u32)(CRYSTAL * 1000000 * 2) / lhsync_freq;
	lhsync_freq = (u32)(lhsync_freq / 100U);    /* resolution be with 100Hz as unit*/
	lhsync_freq = (u32)((lhsync_freq + 1U) >> 1U);
	/*avoid overflow issue*/
	return (u16) lhsync_freq;
}
/**
 * @brief u8 bSP0IVSClock(u16 whtotal, u16 wvtotal)
 * According to Htotal,Vtotal and crystal clock to get Vsync clock
 * @param   whtotal : H len count measured in 27MHz,
 * @param   vtotal : H line count between two vsync
 * @retval  V frequency
 */
/* Calculate Vertical Refresh Rate*/
/* Formula ==>[refresh = Crystal freq / (vHtotalMeasure*vVtotalMeasure )*/
u8 bSP0IVSClock(u16 whtotal, u16 wvtotal)
{
	u32 lvsync_freq;

	lvsync_freq = wvtotal;
	lvsync_freq = lvsync_freq * whtotal;

	if (lvsync_freq == ZERO) {
		return 0;
	}

	lvsync_freq = ((u32)(CRYSTAL * 1000000 * 2)) / lvsync_freq;
	lvsync_freq = ((lvsync_freq + 1U) >> 1U);
	return ((u8) lvsync_freq);
}

/**
 * @brief void SP0Initial(void)
 * Common intial  for SP0 variable
 * @param   void
 * @retval  void
 */
void SP0Initial(void)
{
	wSP0Hclk = ZERO;
	bSP0Vclk = ZERO;
	fgSP0Hpol = ZERO;
	fgSP0Vpol = ZERO;
	wSP0Vtotal = ZERO;
	_bUnLockCnt = 0;
	x_memset(wSP0VCompare, 0, 8U);
	/* Set Serration Window instead of HLength Protection to Get Proper VTotal Value*/
	vIO32WriteFldAlign(ASYNC_09, 20U, AS_SERR_MASK_ST);
	vIO32WriteFldAlign(ASYNC_09, 20U, AS_SERR_MASK_END);
#if 0
	/*reset  polarity*/
	vDrvHsLockInv((u8)FALSE);
	vDrvHsInv((u8)FALSE);
	vDrvVsInv((u8)FALSE);
	vDrvVsOutInvPol((u8)FALSE);
#endif
}

/**
 * @brief void vResetVLen(void)
 * Reset Vlen counter when losing V sync for VGA
 * @param   None
 * @retval  None
 */
void vResetVLen(void)
{
	vIO32WriteFldAlign(ASYNC_02, 0x1U, AS_HLEN_VLEN_RESET);   /* Set HVLEN_RESET*/
	vUtDelay2us((u32)50);
	vIO32WriteFldAlign(ASYNC_02, 0x0U, AS_HLEN_VLEN_RESET);  /* Disable HVLEN_RESET*/
}


/**
 * @brief void vDrvAsyncMvDetectV(WORD wstart1, WORD wend1) large
 * set the MV V detetion Active range
 * @param   wstart1, wend1 : line count  from each frame vsync
 * @retval  void
 */
void vDrvAsyncMvDetectV(u16 wstart1, u16 wend1)
{
	/*Start*/
	vIO32WriteFldAlign(ASYNC_09, (u32)wstart1, AS_MV_VACTIVE_ST);
	/*End*/
	vIO32WriteFldAlign(ASYNC_09, (u32)wend1, AS_MV_VACTIVE_END);
}


/**
 * @brief void vDrvAsyncMvDetectH(WORD wstart1, WORD wend1) large
 * set the MV H detetion Active range
 * @param   wstart1, wend1,: H count  from start 1 to (htotal - end1)
 * @retval  void
 */
void vDrvAsyncMvDetectH(u16 wstart1, u16 wend1)
{
	/* Start*/
	vIO32WriteFldAlign(ASYNC_08, (u32)wstart1, AS_MV_HSTART);
	/* End*/
	vIO32WriteFldAlign(ASYNC_08, (u32)wend1, AS_MV_HEND);
}

/**
 * @brief void bDrvAsyncMvStatus(void) large
 * get MV status
 * @param   void
 * @retval  void
 */
u8 bDrvAsyncMvStatus(void)
{
	u8 f0;
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	/*check only SDTV timing support MV*/
	switch (mode) {
	case MODE_525I:
	case MODE_625I:
	case MODE_525I_OVERSAMPLE:
	case MODE_625I_OVERSAMPLE:
	case MODE_480P:
	case MODE_576P:
	case MODE_480P_OVERSAMPLE:
	case MODE_576P_OVERSAMPLE:
		break;

	default:
		return OFF;
	}

	f0 = (u8)IO32ReadFldAlign(STA_SYNC0_03, AS_F0_PSYNC_NO);

	if (f0 >= (u8)4) {
		return (u8)ON;
	} else {
		return (u8)OFF;
	}
}



/**
 * @brief void vASDeCompSel(u8 bOn) set de composite method
 * @param    DeCompSel : De-composite method #
 * @retval  void
 */
void vASDeCompSel(u8 DeCompSel)
{
	vIO32WriteFldAlign(ASYNC_01, (u32)DeCompSel, AS_DE_COMP_SEL);

	if (DeCompSel == (u8)NEW_DECOMPOSITE1) {
		vIO32WriteFldAlign(ASYNC_07, 0x90U, AS_VSYNC_OUT_HYST_THR);
		/*0x96 -> 0x90 [DTV00123929]*/
	} else {
		vIO32WriteFldAlign(ASYNC_07, 0x0U, AS_VSYNC_OUT_HYST_THR);
	}
}


/**
 * @brief void vDrvAsyncSetMuteCriteria() set hardware mute functionality
 * @param    u4MuteFlag bit0: Enable mute when Hsync active changed
 *                      bit1: Enable mute when Vsync active changed
 *                      bit2: Enable mute when Csync active changed
 *                      bit3: Enable mute when Hsync polarity changed
 *                      bit4: Enable mute when Vsync polarity changed
 *                      bit5: Enable mute when Horizontal length changed
 *                      bit6: Enable mute when Vertical length changed
 * @retval  void
 */
void vDrvAsyncSetMuteCriteria(u32 u4MuteFlag)
{
	vIO32WriteFldAlign(ASYNC_0C, u4MuteFlag, AS_MUTE_MULTI_EN);
}


void vDrvRETIMEReset(void)
{
	vIO32WriteFldAlign(MISC, 1U, C_RETIME_AUTO);          /*reset SDDS internal Hsync & pixel clock*/
	vUtDelay2us((u32)1);
	vIO32WriteFldAlign(MISC, 0U, C_RETIME_AUTO);
	_RETIMENeedReset = FALSE;
}





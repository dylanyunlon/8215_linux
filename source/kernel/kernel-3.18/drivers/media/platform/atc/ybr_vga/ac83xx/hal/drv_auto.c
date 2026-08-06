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

#include "drv_auto.h"
/******************************************************************************
 * Macro, Type Definition, and Variable Declaratoin
 *****************************************************************************/

/*temp*/
/*
extern u16   wSP0Vtotal;
u32 dVGAGetAllDiffValue_peak(void);
*/
/******************************************************************************
 * Function Forward Declaration
 *****************************************************************************/
/**
 * @brief u1DrvVGAGetPhase(void )
 * Get phase value  for VGA
 * @param  NONE
 * @retval Current phase value
 */

u8 u1DrvVGAGetPhase(void)
{
	return (u8)IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
}


u16 wDrvVGAGetClock(void) /*127 offset*/

{
	u8 mode = 0;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if ((mode > 0) && (mode < bAllTimings)) {
		return (u16)((wDrvCLKINGetHtotal() + 127U) - (u32)Get_VGAMODE_IHTOTAL(mode));
	} else {
		return (u16)127;
	}
}

u8 wDrvVgaGetClockMax(void)
{
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if (fgIsValidTiming(mode)) {
		u16 u2MaxHtotal;
		u16 u2Clock;

		u2MaxHtotal = (u16)((u32)(1690 * 1000) / (u32)wSP0Hclk);
		u2Clock = u2MaxHtotal - Get_VGAMODE_IHTOTAL(mode);

		/*UTIL_Printf("htotal %d %d %d\r\n", u2MaxHtotal, Get_VGAMODE_IHTOTAL(mode), u2Clock );*/
		if (u2Clock < (u16)128) {
			return (u8) u2Clock;
		} else {
			return (u8)128;
		}
	} else {

		return (u8)128;
	}
}


u16 wDrvVGAHPositionMin(void)
{
	/*real min = 3*/
	return (u16)10; /*for H/V align , 1 line different*/
}

u16 wDrvVGAHPositionMax(void)
{
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if ((mode > 0) && (mode < bAllTimings)) {
		/*HTOTAL always get it from table, should be safe - width*/
		u16 HMaxValue;

		if (fgIsVideoTiming(mode)) {
#if 0

			if ((mode == MODE_525I) || (mode == MODE_625I) || (mode == MODE_525I_OVERSAMPLE)
			|| (mode == MODE_625I_OVERSAMPLE)) {
				HMaxValue = (Get_VGAMODE_IHTOTAL(mode) - (Get_VGAMODE_IPH_WID(mode) * 2));
		/* Modify larger tolerance H-position adjust space for ambiguous mode limited by UI flow by W.C Shih*/
			} else
#endif
			{
				HMaxValue = (Get_VGAMODE_IHTOTAL(mode) - Get_VGAMODE_IPH_WID(mode));
			}
		} else {
			/*HMaxValue = Get_VGAMODE_IHTOTAL(mode)-Get_VGAMODE_IPH_WID(mode))*4/3;*/

			if (Get_VGAMODE_IPH_WID(mode) <= (u16)800) {
				HMaxValue = (Get_VGAMODE_IHTOTAL(mode) - Get_VGAMODE_IPH_WID(mode));
			/* Modify larger tolerance H-position adjust space for ambiguous mode limited by UI flow by */
			} else {
				HMaxValue = (u16)(((Get_VGAMODE_IHTOTAL(mode) - Get_VGAMODE_IPH_WID(mode)) * (u16)4) / (u16)3);
			/* Modify larger tolerance H-position adjust space for ambiguous mode limited by UI flow by*/
			}

		}

		return HMaxValue; /*Get_VGAMODE_IHTOTAL(mode)-Get_VGAMODE_IPH_WID(mode);*/
	} else {
		return wDrvVGAHPositionMin();
	}
}


u16 wDrvVGAVPositionMin(void)
{
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if (Get_VGAMODE_INTERLACE(mode)) {
		return (u16)6;
	} else {
		return (u16)3;   /*assume Vsout[0-3],5371 vsout must >=3*/
	}
}

u16 wDrvVGAVPositionMax(void)
{
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}


	if (Get_VGAMODE_INTERLACE(mode)) {
		return (u16)(Get_VGAMODE_IPV_STA(mode) * (u8)2 - (u8)8);
	}

	if ((mode > 0U) && (mode < 253U)) {
		u16 ret, min;

		/*assume wSP0Vtotal>Get_VGAMODE_IPV_LEN(mode)*/
		min = wDrvVGAVPositionMin();
		ret = ((Get_VGAMODE_IVTOTAL(mode) - Get_VGAMODE_IPV_LEN(mode)) + Get_VGAMODE_IPV_STA(mode)) - (u8)3;

		if ((ret < min) || (Get_VGAMODE_IVTOTAL(mode) <= Get_VGAMODE_IPV_LEN(mode))) {
			return min;
		}

		if (ret > (Get_VGAMODE_IVTOTAL(mode) >> 1)) {
			return (Get_VGAMODE_IVTOTAL(mode) >> 1);
		}

		return ret;
	} else {
		return wDrvVGAVPositionMin();
	}

}


u16 wDrvVGAGetHPosition(void)
{

	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if ((mode > 0U) && (mode < 253U)) {

		return (u16)IO32ReadFldAlign(HDTV_01, HDTV_AV_START);
	} else {
		return (u16)0;
	}
}

u16 wDrvVGAGetVPosition(void)
{
	u8 mode;

	if (g_u4SrcType == SRC_VGA) {
		mode = _bVgaTiming;
	} else {
		mode = _bHdtvTiming;
	}

	if ((mode > 0U) && (mode < 253U)) {
		return (u16)IO32ReadFldAlign(ASYNC_11, AS_NEW_VS_OUTP_S1);
	} else {
		return (u16)2;
	}

}

void vDrvVGASetPhase_Simple(u8 bVal)
{

	u8 bPhase_diff, bCnt;
	u8 _bPhase[CHANNEL_NUM];

	if (fgIsAutoFlgSet((u8)SP0_AUTO_CLOCK)) {
		_bPhase[0] = bVal;
		_bPhase[1] = bVal;
		_bPhase[2] = bVal;
	} else {
		_bPhase[0] = (u8)IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
		_bPhase[1] = (u8)IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_GX);
		_bPhase[2] = (u8)IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_BX);

		if ((_bPhase[0] == _bPhase[1]) && (_bPhase[0] == _bPhase[2])) {
			_bPhase[0] = bVal;
			_bPhase[1] = bVal;
			_bPhase[2] = bVal;
		} else {
			bPhase_diff = (bVal >= _bPhase[0]) ? (bVal - _bPhase[0]) : (32 - _bPhase[0] + bVal);
			_bPhase[0] = bVal;

			for (bCnt = (u8)1; bCnt < (u8)CHANNEL_NUM; bCnt++) {
				_bPhase[bCnt] = _bPhase[bCnt] + bPhase_diff;

				if (_bPhase[bCnt] > 31) {
					_bPhase[bCnt] = _bPhase[bCnt] - 32;
				}
			}
		}
	}

	vIO32WriteFldAlign(ASYNC_0F, (u32)_bPhase[0] , AS_PHASESEL_RX);
	vIO32WriteFldAlign(ASYNC_0F, (u32)_bPhase[1] , AS_PHASESEL_GX);
	vIO32WriteFldAlign(ASYNC_0F, (u32)_bPhase[2] , AS_PHASESEL_BX);

}

u8 bDrvVGAGetPhase(void)
{
	return (u8)IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
}

/**
 * @brief vDrvVGASetPhase( u8 bVal )
 * Set phase value  for VGA
 * @param  bVale - phase value 0~31
 * @retval void
 * @example vDrvVGASetPhase(0) - Set Phase 0 for VGA
 */
void vDrvVGASetPhase(u8 bVal)
{
	vDrvVGASetPhase_Simple(bVal);
	_RETIMENeedReset = (bool)TRUE;
}

/**
 * @brief vDrvVGASetBDDataTh( void )
 * Set Boundary threshold for SP0
 * @param  bTh - boundary threshold value
 * @retval void
 * @example vDrvVGASetBDDataTh(20) - set boundary data threshold 20
 */
void vDrvVGASetBDDataTh(u8 bTh)
{
	vIO32WriteFldAlign(ASYNC_15, (u32)bTh, AS_BDDATATH);
}

/**
 * @brief vDrvVGASetBDCha( void )
 * Set measurement channel(VGA)
 * @param  bCha - Channel (R/G/B)
 * @retval void
 * @example vDrvVGASetBDCha(BD_RED) - switch boundary measurement channel to Red
 */
void vDrvVGASetBDCha(u8 bCha)
{
	vIO32WriteFldAlign(ASYNC_13, (u32)bCha, AS_BDINSEL);
}

/**
 * @brief wDrvVGAGetLeftBound( void )
 * Get Left boundary value of VGA
 * @param  void
 * @retval Left boundary value
 * @example wDrvVGAGetLeftBound()
 */
u16 wDrvVGAGetLeftBound(void)
{
	return (IO32ReadFldAlign(STA_SYNC0_04, AS_LEFTBC_STA_S_11_0) +
	(IO32ReadFldAlign(STA_SYNC0_0A, AS_LEFTBC_STA_S_12) << 12U));
}

/**
 * @brief wDrvVGAGetTopBound( void )
 * Get Top boundary value of VGA
 * @param  void
 * @retval Left boundary value
 * @example wDrvVGAGetTopBound()
 */
u16 wDrvVGAGetTopBound(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_06, AS_NEWTOPBC_S);
}

/**
 * @brief wDrvVGAGetRightBound( void )
 * Get right boundary value of VGA
 * @param  void
 * @retval Right boundary value
 * @example wDrvVGAGetRightBound()
 */
u16 wDrvVGAGetRightBound(void)
{
	return (IO32ReadFldAlign(STA_SYNC0_04, AS_RIGHTBC_STA_S_11_0) +
	(IO32ReadFldAlign(STA_SYNC0_0A, AS_RIGHTBC_STA_S_12) << 12U));
}


/**
 * @brief wDrvVGAGetBottomBound( void )
 * Get Bottom  boundary value of VGA
 * @param  void
 * @retval Bottom boundary value
 * @example wDrvVGAGetBottomBound()
 */
u16 wDrvVGAGetBottomBound(void)
{
	return (u16)IO32ReadFldAlign(STA_SYNC0_06, AS_NEWBOTTOMBC_S);
}

/**
 * @brief dVGAGetDiffValue( u8 bChnl )
 * Get channel evaluation value of phase detection for VGA
 * @param  bChnl : R/G/B channel
 * @retval Channel phase evaluation Value
 * @example dVGAGetDiffValue(BD_RED) -Get SP0 red channel evaluation value
 */
/*#if SUPPORT_MIX_PHASE_STA*/
u32 dVGAGetDiffValue(u8 bChnl)
{
	switch (bChnl) {

	case BD_RED:
		return (u32)IO32ReadFldAlign(STA_SYNC0_0A, AS_STA_R_S);


	case BD_GREEN:
		return (u32)IO32ReadFldAlign(STA_SYNC0_0B, AS_STA_G_S);


	case BD_BLUE:
		return (u32)IO32ReadFldAlign(STA_SYNC0_0C, AS_STA_B_S);


	default:
		return (u32)0;
	}
}

u32 dVGAGetDiffValue_peak(u8 bChnl)
{
	switch (bChnl) {
	case BD_RED:
		return (u32)IO32ReadFldAlign(STA_SYNC0_07, AS_TOP_SUMRD_S);

	case BD_GREEN:
		return (u32)IO32ReadFldAlign(STA_SYNC0_08, AS_TOP_SUMGD_S);

	case BD_BLUE:
		return (u32)IO32ReadFldAlign(STA_SYNC0_09, AS_TOP_SUMBD_S);

	default:
		return (u32)0;
	}

}

/*#endif //SUPPORT_MIX_PHASE_STA*/
/**
 * @brief dVGAGetAllDiffValue( void )
 * Get R/G/B channel evaluation sum value of phase detection for VGA
 * @param  void
 * @retval VGA phase evaluation value
 * @pre  call vDrvVGASetPhase(bVal) to set phase for phase detection
 * @pre  call vVGAPhaseModeSet() to set phase detection mode
 * @example dVGAGetAllDiffValue() -Get VGA all channel evaluation value
 */
u32 dVGAGetAllDiffValue_peak(void)
{

	u32 s;

	_dwPhase3CH[0] = dVGAGetDiffValue_peak(BD_RED);
	_dwPhase3CH[1] = dVGAGetDiffValue_peak(BD_GREEN);
	_dwPhase3CH[2] = dVGAGetDiffValue_peak(BD_BLUE);
	s = _dwPhase3CH[0] + _dwPhase3CH[1] + _dwPhase3CH[2];
	return s;

}


u8 _bAutoPeakMode = 0;
u32 dVGAGetAllDiffValue(void)
{
	u32 s, p;

	if (_bAutoPeakMode) {
		p = dVGAGetAllDiffValue_peak();
		return p;
	}
	_dwPhase3CH[0] = dVGAGetDiffValue(BD_RED);
	_dwPhase3CH[1] = dVGAGetDiffValue(BD_GREEN);
	_dwPhase3CH[2] = dVGAGetDiffValue(BD_BLUE);
	s = _dwPhase3CH[0] + _dwPhase3CH[1] + _dwPhase3CH[2];
	return s;

	/*  return (dVGAGetDiffValue(BD_RED) + dVGAGetDiffValue(BD_GREEN) + dVGAGetDiffValue(BD_BLUE));*/
}


/*YPbPr Auto Phase 2006/11/07*/
u32 dHDTVGetAllDiffValue(void)
{
#if 1
	u32 r;


	r = dVGAGetDiffValue(BD_RED);
	/*g=dVGAGetDiffValue(BD_GREEN);*/
	/*b=dVGAGetDiffValue(BD_BLUE);*/
	return r;
	/*s=r+g+b;
	p=dVGAGetAllDiffValue_peak();
	if(p<8000)
	    return s; //s>>3;
	return s;*/
#endif

}


/**
 * @brief vDrvVGASetTopThr( u8 bVal )
 * Set Threshold for  Top mode of phase detection for VGA
 * @param  bVal : Threshold value
 * @retval void
 * @example vDrvVGASetTopThr(20) -Set threshold 20 for phase detection in Top mode
 */
void vDrvVGASetTopThr(u8 bVal)
{
	vIO32WriteFldAlign(ASYNC_0F, (u32)bVal, AS_TOP_THR);
}

/**
 * @brief vDrvVGASetPsneThr( u8 bVal )
 * Set Threshold for  psne mode of phase detection for VGA        __
 * @param  bThr1 : Threshold value 1                           __|
 * @param  bThr2 : Threshold value 2                                ____
 * @retval void
 * @example vDrvVGASetPsneThr(20,0) -Set threshold 20 for phase detection in psne mode
 */
/*#if SUPPORT_MIX_PHASE_STA*/
void vDrvVGASetPsneThr(u8 bThr1, u8 bThr2)
{
	/* Threshold 1 R/G/B*/
	vIO32WriteFldAlign(ASYNC_16, (u32)bThr1, AS_PSNE_THB1);
	vIO32WriteFldAlign(ASYNC_16, (u32)bThr1, AS_PSNE_THG1);
	vIO32WriteFldAlign(ASYNC_16, (u32)bThr1, AS_PSNE_THR1);

	/* Threshold 2 R/G/B*/
	vIO32WriteFldAlign(ASYNC_17, (u32)bThr2, AS_PSNE_THB2);
	vIO32WriteFldAlign(ASYNC_17, (u32)bThr2, AS_PSNE_THG2);
	vIO32WriteFldAlign(ASYNC_17, (u32)bThr2, AS_PSNE_THR2);
}

/*#endif //SUPPORT_MIX_PHASE_STA*/
/**
 * @brief vVGAPhaseModeSet( u8 bVal )
 * Set Top/psne mode for VGA  phase detection
 * @param  bMode  (1:Top mode, 0:psne Mode)
 * @retval void
 * @example vVGAPhaseModeSet(0) - Set psne mode for phase detection
 */
void vVGAPhaseModeSet(u8 bMode)
{
	vDrvVGASetTopThr((u8)TOP_THRE);
	/*#if SUPPORT_MIX_PHASE_STA*/
	vDrvVGASetPsneThr((u8)PSNE_THRE1, (u8)PSNE_THRE2);
	/*select phase statistics => mix*/
	vIO32WriteFldAlign(ASYNC_17, MIX_STA, AS_C_PSNE_STA_SEL);
	/*vIO32WriteFldAlign(ASYNC_00, PSNE_ONLY, PSNE_STA_SEL);*/
	/*select phase add method => 1 Top + 2x psne*/
	vDrvVGASetPhsMix((u32)PSNE2_ADD);
	/*#else*/
	/*set default mode to top mode for peak status*/
	vIO32WriteFldAlign(ASYNC_17, (u32)bMode, AS_TOP_PE_SW);
	/*#endif*/
}

/*hardware auto support*/
/**
 * @brief vVgaHwAutoClkEnable( u8 bEnable )
 * Enable and disable hw auto clock
 * @param  bEnable (1:enable , 0:disable)
 * @retval void
 */

void vVgaHwAutoClkEnable(u8 bEnable)
{
	/* reset the clock ready bit*/
	vIO32WriteFldAlign(ASYNC_16, 1U, AS_AUTO_CLK_RDY_CLR);
	vUtDelay2us((u32)10); /* 10us delay*/
	vIO32WriteFldAlign(ASYNC_16, 0U, AS_AUTO_CLK_RDY_CLR);

	/* auto clock trigger enalbe and start/disable*/
	vIO32WriteFldAlign(ASYNC_16, (u32)bEnable, AS_CLK_AUTO);
	vIO32WriteFldAlign(ASYNC_16, (u32)bEnable, AS_CLKDET_INI);
}

/**
 * @brief vVgaHwAutoPhaseEnable( u8 bEnable )
 * Enable and disable hw auto phase
 * @param  bEnable (1:enable , 0:disable)
 * @retval void
 */

void vVgaHwAutoPhaseEnable(u8 bEnable)
{
	/* reset the phase ready bit*/
	vIO32WriteFldAlign(ASYNC_16, 1U, AS_AUTO_PHASE_RDY_CLR);
	vUtDelay2us((u32)10); /* delay*/
	vIO32WriteFldAlign(ASYNC_16, 0U, AS_AUTO_PHASE_RDY_CLR);

	/* reset the phase  trigger*/
	vIO32WriteFldAlign(ASYNC_16, (u32)bEnable, AS_PHASE_AUTO);
	vIO32WriteFldAlign(ASYNC_16, (u32)bEnable, AS_PHSDET_INI);

}

void vVgaHwAutoPhaseReset(void)
{
	vIO32WriteFldAlign(ASYNC_16, 0U, AS_PHSDET_INI);
	vIO32WriteFldAlign(ASYNC_16, 1U, AS_PHSDET_INI);
}


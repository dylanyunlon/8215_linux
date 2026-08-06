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
#include "drv_autocolor.h"

/*define the verification functions*/
#define CHK_BLANK_VARIATION 0
#define CHK_PGA_LINEARITY  1
#define CHK_ADC_LINEARITY 1
#define CHK_GAIN_OFFSET_ITERATION 0 /* 1*/
#define ALWAYS_BLANK 1
#define PGA_4bit_offset 0   /*test for next generation IC*/
/*debug message enable/disable  // 1 --> for debug*/
#define AUTO_COLOR_DEBUG 0
#define AUTO_BLANK_DEBUG  0
#define PGA_LINEARITY_DEBUG 0
#define ON_CHIP_AUTO_COLOR_DEBUG 0
#define AUTO_BLANK_MEASURE  0
#define Debug_Monitor_Blank 0

/*define constant*/
#define AUTO_COLOR_MAX_TIME 0x10
#define ADJUST_BLANK_RETRY_COUNT  3


#define MAX_OFFSET           0x3FF
#define MAX_GAIN               0xFFFF
#define DEFAULT_OFFSET     0x100
#define DEFAULT_GAIN         0xA000

#define INCREASE_OFFSET     1
#define DECREASE_OFFSET    0
#define EQUAL_OFFSET 2

/******************************************************************************
 * Macro, Type Definition, and Variable Declaratoin
*****************************************************************************/
u8 _OnChipAutoColorState;
u8 _bAutoColorState0;
u8 _bOrgType, _bCaliMode, _bOffsetGainDone;
u8 _bAutoColorTimeOutCNT;
u8 _bGainOffset_channel; /*_bAutoColorIsr,,_bOffset_Blank_OK;*/
u8 _bTotalCnt ; /*_bMaxValue,_bTotalCnt;*/
u8 _bType, _bOrder, _bDoubleChkFlag[3];
u8 _bGainIncDir[3], _bChgDirToggle[3], _bIncGainDirPre[3];

u16 _bGain;
u16 _bIndex_mim[3];
u16 _bCaliStep[3];
u16 _bAutoColorGain[4][3];
u16 _bAutoColorOffset[4][3];
u16 _bInitOffset;
u16 _bInitGain;
u16 _bAutoColorGain_for_EFuse[4][3];
u8 _bInitDigSignBit;


u16 _wADCTarget[3], _wData_v1v2[3], _wADC_Diff_value[3], _wDiff_mim[3], _wBlank[3];
u8 _bReadMAXLevel;
u8 _bMode;
u8 _bAutoColorHistory[4];

u32 _dwBlankAvg[3];
u32 _dwMaxLevelAvg[3];
u32 _dwBlankAvg_1[3];
u32 _dwMaxLevelAvg_1[3];
u8 _bAdjBlkState0;
u8 _bBlankIteration;
u8 _bAdjGainAfterBlank;
u8 _bHDTVMax[3];
u8 _bVGAMax[3];
u8 _fgblankfinish = 0;

u8 _bEEPROM_ready;
/* u8 _bWaitSignalStable;*/
u8 _bDoubleCheck;
u8 _bDiffOffset;

u8  _bOffsetCaliDone[4];

u8 _bDigSignBit;
#if CHK_ADC_LINEARITY
u32 _dwAutoColorMax[3];
u32 _dwStPoint[3];
u32 _dwEndPoint[3];
u32 _dwDeltaValue[3];
u32 _dwMaxDiff[128];
u32 _dwMaxDiffTemp;
u32 _dwMaxDiffLocal;
u8 _bLocation;
u8 _EndPoint;
u8 _bADCLinearity;
u8 _bADC_LINEARITY_state = 0;
u8 TOTAL_POINT;
#endif
#if CHK_ADC_LINEARITY
enum {
	ADC_VERIFY_0,
	ADC_VERIFY_1,
	ADC_VERIFY_2,
	ADC_VERIFY_3,
	ADC_VERIFY_4,
	ADC_VERIFY_5,
	ADC_VERIFY_6
};
#endif

#if CHK_PGA_LINEARITY

u8 _bPGA_LINEARITY_state = 0;
enum {
	PGA_VERIFY_NOTHING,
	PGA_VERIFY_INIT,
	PGA_VERIFY_2,
	PGA_VERIFY_3,
	PGA_VERIFY_4,
	PGA_VERIFY_5
};
#endif

u16  wOnChipColorGainTable_Temp[4][3];

u32 wHDTV_Data_Sel[6][3] = {                      /*Modify for AC8317***/
	{STA_HDTV_BLANK_PR, STA_HDTV_BLANK_PB, STA_HDTV_BLANK_Y},       /* 0  confirmed*/
	{STA_HDTV_BLANK_PB, STA_HDTV_BLANK_Y,  STA_HDTV_BLANK_PR},      /* 1  default for VGA*/
	{STA_HDTV_BLANK_PB, STA_HDTV_BLANK_Y,  STA_HDTV_BLANK_PR},      /* 2  confirmed*/
	{STA_HDTV_BLANK_PR, STA_HDTV_BLANK_Y,  STA_HDTV_BLANK_PB},      /* 3  T.B.F*/
	{STA_HDTV_BLANK_Y,  STA_HDTV_BLANK_PB, STA_HDTV_BLANK_PR},      /*4. default for YPbPr*/
	{STA_HDTV_BLANK_PR, STA_HDTV_BLANK_Y,  STA_HDTV_BLANK_PB},      /* 5 onfirmed*/
};
/*
extern u16  wColorBlankValueNew[4][3];
extern u8  bColorMaxType1[4][3];
extern u8  bColorMaxType2[4][3];
extern u8  bColorMaxType3[4][3];
extern u16  wOnChipColorMaxType[4][3];
extern u16  wOnChipColorGainTable[4][3];
extern u16  wOnChipColorGainTable_75[4][3];
extern u16  wYPbPrMappingVgaGainTable[4][3];
*/
/*extern u16  wOnChipColorGainTable_SONY[4][3];*/
/*
extern u8  bOnChipCalibrateTolerance[5][4];
extern u8  bOnChipCheckTolerance[5][2];
extern u8  GAIN_HIGH_LIMIT_100[4][3];
extern u8  GAIN_LOW_LIMIT_100[4][3];
extern u8  GAIN_HIGH_LIMIT_120[4][3];
extern u8  GAIN_LOW_LIMIT_120[4][3];
extern u8  GAIN_HIGH_LIMIT_75_100[4][3];
extern u8  GAIN_LOW_LIMIT_75_100[4][3];
extern u8  bVGADefaultOffsetTABLE_100[4][3];

extern u16  GAIN_HIGH_LIMIT_100_DIGITAL[4][3];
extern u16  GAIN_LOW_LIMIT_100_DIGITAL[4][3];
*/
void vDrvGeneralAutoColorGainSetToNextType(const u16 *ColorGainTablePtr, u8 bOrgType, u8 bNextType)
{
	u8 bOrder;
	u16 wDen, wNum;
	u32 dGain, bGain;
	u16 ColorGainTable[4][3], i, j;

	for (i = (u16)0; i < (u16)4; i++)
		for (j = (u16)0; j < (u16)3; j++) {
			ColorGainTable[i][j] = *(ColorGainTablePtr + i * (u16)3 + j);
		}

	for (bOrder = (u8)0; bOrder < (u8)3; bOrder++) {
		dGain = _bAutoColorGain[bOrgType][bOrder];
		wNum  = ColorGainTable[bOrgType][bOrder];
		wDen  = ColorGainTable[bNextType][bOrder];
		bGain = dGain * wDen / wNum;
		_bAutoColorGain[bNextType][bOrder] = bGain;
	}
}

/******************************************************************************
 * Function Forward Declaration
 *****************************************************************************/


/**
 * @brief bDrvGetHDADCGain_Digital(u8 bOrder )
 * Get digital gain value
 * @param  bOrder : HDADC Channel
 * @retval gain value
 * @example bDrvGetHDADCGain_Digital(bOrder)
 */
u16 bDrvGetHDADCOffset_Digital(u8 bOrder)
{
	switch (bOrder) {
	case 0:
		return (u16)IO32ReadFldAlign(HDFE_00, AD1_OFFSET); /*R channel maximum value in a frame*/

	case (u8)1:
		return (u16)IO32ReadFldAlign(HDFE_00, AD2_OFFSET); /*G channel maximum value in a frame*/

	case (u8)2:
	default:
		return (u16)IO32ReadFldAlign(HDFE_00, AD3_OFFSET); /*B channel maximum value in a frame*/
	}
}

u16 bDrvGetHDADCGain_Digital(u8 bOrder)
{
	switch (bOrder) {
	case (u8)0:
		return (u16)IO32ReadFldAlign(HDFE_01, AD1_GAIN);   /*R channel maximum value in a frame*/

	case (u8)1:
		return (u16)IO32ReadFldAlign(HDFE_01, AD2_GAIN);   /*G channel maximum value in a frame*/

	case (u8)2:
	default:
		return (u16)IO32ReadFldAlign(HDFE_02, AD3_GAIN);   /*B channel maximum value in a frame*/
	}
}
void bDrvSetHDADCGain_Digital(u8 bOrder, u16 value)
{
	switch (bOrder) {
	case (u8)0:
		vIO32WriteFldAlign(HDFE_01, (u32)value, AD1_GAIN);
		break;

	case (u8)1:
		vIO32WriteFldAlign(HDFE_01, (u32)value, AD2_GAIN);
		break;

	case (u8)2:
	default:
		vIO32WriteFldAlign(HDFE_02, (u32)value, AD3_GAIN);
		break;
	}
}



void bDrvSetHDADCOffset(u8 bOrder, u8 value)
{
	u16 Dvalue = (u16)((u32)(value << 1U));

	vDrvSetHDADCDigitalOffset(bOrder, Dvalue);
}

void bDrvSetHDADCGain(u8 bOrder, u8 value)
{
	u16 Dvalue = (u16)((u32)(value << 8U));

	bDrvSetHDADCGain_Digital(bOrder, Dvalue);
}



u8 bDrvGetColorTargetValue(u8 bType, u8 bOrder)
{
	if ((bType < (u8)4) && (bOrder < (u8)3)) {
		return bColorMaxType1[bType][bOrder];
	} else {
		return (u8)0xff;
	}
}


u16 bDrvGetBlankVal(u8 bOrder)    /*MC20080502_1 ****/
{
	u8 bDataOrder;
	u16 wBlankValue;

	bDataOrder = (u8)IO32ReadFldAlign(HDTV_03, HDTV_DATA_SEL);

	if ((bDataOrder < (u8)6) && (bOrder < (u8)3)) {
		wBlankValue = (u16)IO32ReadFldAlign(HDTV_STA_00, wHDTV_Data_Sel[bDataOrder][bOrder]);
		return wBlankValue;
	} else {
		return (u16)0xFFFF;
	}
}                                         /*MC20080502_1 &&&                                       //MC20080502_1 &&&*/

void vDrvPreSetToReadBlank(u8 bOnOff)
{
#if ALWAYS_BLANK
	bOnOff = 0;
	vIO32WriteFldAlign(HDTV_03, (u32)bOnOff, HDTV_BLAK_SET);
#else
	vIO32WriteFldAlign(HDTV_03, (u32)bOnOff, HDTV_BLAK_SET);
#endif
}



/**
 * @brief bDrvSetHDADCDigitalOffset( void )
 *
 * @param  bOrder,value
 *
 * @retval None
  */
void vDrvSetHDADCDigitalOffset(u8 bOrder, u16 value)
{
	switch (bOrder) {
	case (u8)0:
		vIO32WriteFldAlign(HDFE_00, (u32)value, AD1_OFFSET);
		break;

	case (u8)1:
		vIO32WriteFldAlign(HDFE_00, (u32)value, AD2_OFFSET);
		break;

	case (u8)2:
		vIO32WriteFldAlign(HDFE_00, (u32)value, AD3_OFFSET);
		break;
		
	default:
		break;
	}
}

u16 wReadHDTVDigitalOffset(u8 bOrder)
{
	switch (bOrder) {
	case (u8)0:
		return (u16)IO32ReadFldAlign(HDFE_00, AD1_OFFSET);

	case (u8)1:
		return (u16)IO32ReadFldAlign(HDFE_00, AD2_OFFSET);

	case (u8)2:
	default:
		return (u16)IO32ReadFldAlign(HDFE_00, AD3_OFFSET);
	}
}


u16 wReadHDTVDigitalGain(u8 bOrder)
{
	switch (bOrder) {
	case (u8)0:
		return (u16)IO32ReadFldAlign(HDFE_01, AD1_GAIN);

	case (u8)1:
		return (u16)IO32ReadFldAlign(HDFE_01, AD2_GAIN);

	case (u8)2:
	default:
		return (u16)IO32ReadFldAlign(HDFE_02, AD3_GAIN);
	}
}


u16 vCalculateDigOffset(u16 bOffset, u16 bAdjOffset, u8 bCmpResult)
{
	if (bCmpResult == 0) {
		if ((bOffset + bAdjOffset) > (u16)0x1FE) {
			bOffset = (u16)0x1FE;
		} else {
			bOffset = bOffset + bAdjOffset;
		}
	} else {
		if ((bOffset - bAdjOffset) < 0) {
			bOffset = 0;
		} else {
			bOffset = bOffset - bAdjOffset;
		}
	}

	return bOffset;
}


/**
 * @brief vDrvIntAutoColorStart( void )
 * Internal auto color start
 * @param  void
 * @retval void
 * @example vDrvIntAutoColorStart()
 */
void vDrvIntAutoColorStart(void)
{
	_bAutoColorState0 = VDO_AUTO_COLOR_START;
	_bAutoColorTimeOutCNT = 0;
}


/*#if ON_CHIP_AUTO_COLOR*/

u8 bDrvOnChipGetADCMaxValueRGBYPBPR(u8 bOrder)
{
	return (u8)bDrvOnChipGetADCMaxMinValue(bOrder, AS_PHASE_MAX_SEL);
}


u8 bDrvOnChipGetADCMaxMinValue(u8 bOrder, u8 bMode)
{
	vIO32WriteFldAlign(ASYNC_0F, (u32)bMode , AS_PHASE_MAXMIN_SEL);

	switch (bOrder) {
	case 0:
		return (u8)IO32ReadFldAlign(STA_SYNC0_07, AS_RMAXMIND); /*R channel maximum value in a frame*/

	case (u8)1:
		return (u8)IO32ReadFldAlign(STA_SYNC0_08, AS_GMAXMIND); /*G channel maximum value in a frame*/

	case (u8)2:
	default:
		return (u8)IO32ReadFldAlign(STA_SYNC0_09, AS_BMAXMIND); /*B channel maximum value in a frame*/
	}
}



u8 bDrvOnChipGetVFESignalType(u8 bAutoInput)
{
	u8 bColor_Type;

	switch (bAutoInput) {
	case P_VGA:
		bColor_Type = (u8)INT_VGA;
		break;

	case P_YP0:
	case P_YP1:
		bColor_Type = (u8)INT_HDTV;
		break;

	case P_VGACOMP:
		bColor_Type = (u8)INT_VGA_COMPOENT;
		break;

	default:
		bColor_Type = (u8)P_FA;
		break;
	}

	/*UTIL_Printf("---bAutoInput=%x,  bColor_Type=%x   \r\n",bAutoInput,bColor_Type);*/
	return bColor_Type;
}


void vDrvOnChipAutoColorGainSetToNextType(u8 bOrgType, u8 bNextType)
{
	u8 bOrder, bGain;
	u16 wDen, wNum;
	u32 dGain;

	if ((bOrgType > INT_VGA_COMPOENT) || (bNextType > INT_VGA_COMPOENT)) {
		return;
	}
	for (bOrder = (u8)0; bOrder < (u8)3; bOrder++) {
		dGain = _bAutoColorGain[bOrgType][bOrder];
		wNum  = wOnChipColorGainTable_75[bOrgType][bOrder];
		wDen  = wOnChipColorGainTable_75[bNextType][bOrder];
		dGain = ((0xffU + dGain) * wDen) - (0xffU * (u32)wNum);
		bGain = (dGain + (wNum > 1)) / wNum ; /*round off*/
		_bAutoColorGain[bNextType][bOrder] = bGain;
	}
}

/*==========================================================================*/
/*function    : vDrvOnChipGainMapping(u8 bType)*/
/*description : Use the VGA gain calibration to calculate the gain for the other signal type*/
/*input       : bType*/
/*return      : none*/
/*===========================================================================*/
void vDrvOnChipGainMapping(u8 bColorType)
{
	u8 bNextType;

	switch (bColorType) {
	case (u8)INT_HDTV:    /*YPbPr to VGA*/
		bNextType = (u8)INT_VGA;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		bNextType = (u8)INT_SCART;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		bNextType = (u8)INT_VGA_COMPOENT;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		break;

	case (u8)INT_VGA:     /*input source is VGA and map gain to YPbPr*/
		bNextType = (u8)INT_HDTV;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		bNextType = (u8)INT_SCART;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		bNextType = (u8)INT_VGA_COMPOENT;
		vDrvOnChipAutoColorGainSetToNextType(bColorType, bNextType);
		break;
		
	default:
		break;
	}
}

void vDrvOnChipModeOnOff(u8 bOnOff)
{
	u8 bCh;

	vIO32WriteFldAlign(PDWNC_VGACFG0, (u32)bOnOff, FLD_RG_VMUX_PWD); /*to power down VGA MUX*/

	vIO32WriteFldAlign(REG_VGA_Normal_CFG0, (u32)bOnOff, RG_VDC_N_EN);  /* 1:enable "PGA Negative AG Always Connect"*/

	for (bCh = (u8)0; bCh < (u8)3; bCh++) {
		if (bOnOff == (u8)0) {
			;
		} else {
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)(bCh * (u8)4), 0U, RG_VGAADC1_CORE_PWD);/*power on ADC*/
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)(bCh * (u8)4), 0U, RG_VGAADC1_DIV_SEL);
			vIO32WriteFldMulti(REG_VGA_Normal_CFG4, P_Fld(0U, RG_VGAADC1_IO_PWD)
			| P_Fld(0U, RG_VGAADC2_IO_PWD) | P_Fld(0U, RG_VGAADC3_IO_PWD),
			RG_VGAADC1_IO_PWD | RG_VGAADC2_IO_PWD | RG_VGAADC3_IO_PWD);
		}
	}

	vIO32WriteFldAlign(HDTV_03, (u32)bOnOff, HDTV_EN);

#ifdef CONFIG_ATC_PLATFORM_ac83xx
	if (bOnOff == (u8)0) { /*turn off*/
#if Phase3Channel
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld(0x1U, FLD_RG_VGAPLL_CKO_SEL)
		| P_Fld(0x1U, FLD_RG_VGAPLL_B_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_G_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#else
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld(0x2U, FLD_RG_VGAPLL_CKO_SEL)
		| P_Fld(0x0, FLD_RG_VGAPLL_B_EN) | P_Fld(0x0U, FLD_RG_VGAPLL_G_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#endif
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG18, 0x0U, FLD_RG_TL_27M_SEL);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 0U, RG_VDC_P_EN);
	} else {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, 0x0U, FLD_RG_VGAPLL_CKO_SEL);
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG18, 0x1U, FLD_RG_TL_27M_SEL);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 1U, RG_VDC_P_EN);
		/*to power down "ON CHIP auto color buffer and disable 825/450mv,*/
	}
#else
	if (bOnOff == (u8)0) { /*turn off*/
#if Phase3Channel
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld(0x1U, FLD_RG_VGAPLL_CKO_SEL)
		| P_Fld(0x1U, FLD_RG_VGAPLL_B_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_G_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#else
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld(0x2U, FLD_RG_VGAPLL_CKO_SEL)
		| P_Fld(0x0, FLD_RG_VGAPLL_B_EN) | P_Fld(0x0U, FLD_RG_VGAPLL_G_EN) | P_Fld(0x1U, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#endif
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG18, 0x0U, FLD_RG_TL_27M_SEL);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 0U, RG_VDC_P_EN);
	} else {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, 0x0U, FLD_RG_VGAPLL_CKO_SEL);
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG18, 0x1U, FLD_RG_TL_27M_SEL);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 1U, RG_VDC_P_EN);
		/*to power down "ON CHIP auto color buffer and disable 825/450mv,*/
	}
#endif
}

void vDrvSelectAutoColorBufferOutputChannel(u8 bOrder)
{
	vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 1U, RG_VDC_P_EN);  /*5360 select channel buffer of ON CHIP AUTO COLOR*/
}

void vDrvSelV1V2(u8 bLoopCnt_V1V2)
{
	/*  if(bLoopCnt_V1V2 ==0)*/
	/*   {*/
	/*      vIO32WriteFldAlign(REG_VGA_Normal_CFG4, 1, ACHIN); //Select V2=750mv*/
	/*  }*/
	/*  else*/
	/*   {*/
	/*     vIO32WriteFldAlign(REG_VGA_Normal_CFG4, 0, ACHIN); //Select V1=400mv*/
	/*  }*/
}

u16 bDrvGetADCData(u8 bOrder)
{
	/*yunjie mark to fix*/
	/*
		switch (bOrder)
		{
		case 0:
		return (IO32ReadFldAlign(HDTV_STA_07, STA_HDTV_ADC1));  //ADC channel 1 raw data
		case 1:
		return (IO32ReadFldAlign(HDTV_STA_07, STA_HDTV_ADC2));  //ADC channel 2 raw data
		case 2:
		default:
		return (IO32ReadFldAlign(HDTV_STA_07, STA_HDTV_ADC3));  //ADC channel 3 raw data
		}
		*/
	return 0;
}



void vDrvOnChipAutoColorModeSet(u8 bMode, u8 bType)
{
	if ((bType <= (u8)INT_VGA_COMPOENT) && (_bOrder < (u8)3)) { /*for klocwork check*/
		switch (bMode) {
		case ON_CHIP_GAIN_MODE:             /*On chip Gain calibration mode*/
			_bInitOffset  = (u16)DEFAULT_OFFSET;
			_wADCTarget[_bOrder] = wOnChipColorMaxType[bType][_bOrder];
			vDrvOnChipModeOnOff((u8)1);
			bDrvSetHDADCGain_Digital(_bOrder, (u16)0x8000);
			/*UTIL_Printf("Gain order=%2u, target=0x%x\r\n",_bOrder,_wADCTarget[_bOrder]);*/
			break;

		case OFFSET_MODE:
			_wADCTarget[_bOrder] = wColorBlankValueNew[bType][_bOrder];
			_bInitOffset = wReadHDTVDigitalOffset(_bOrder); /*_bAutoColorOffset[bType][_bOrder];*/

			if ((_bInitOffset == (u16)MAX_OFFSET) || (_bInitOffset == (u16)0)) { /* initial value protect*/
				_bInitOffset = (u16)DEFAULT_OFFSET;
			}

			vDrvPreSetToReadBlank((u8)AS_BLANK_ALWAYS);
			/*UTIL_Printf("offset order=%2u, target=0x%x\r\n",_bOrder,_wADCTarget[_bOrder]);*/
			break;

		case GAIN_MODE:
			_bGain = bDrvGetHDADCGain_Digital(_bOrder); /*_bAutoColorGain[bType][_bOrder];*/

			/*_bGain = bApiEepromReadu8(EEP_VIDEO_AUTO_COLOR_START + (bType*3) + _bOrder);*/
			if ((_bGain == (u16)MAX_GAIN) || (_bGain == (u16)0)) { /* initial value protect*/
				_bGain = (u16)DEFAULT_GAIN;
			}

			_bInitOffset = _bAutoColorOffset[bType][_bOrder];

			if ((_bInitOffset == (u16)MAX_OFFSET) || (_bInitOffset == (u16)0)) { /* initial value protect*/
				_bGain = (u16)DEFAULT_GAIN;
			}

			_wADCTarget[_bOrder]  = bDrvGetColorTargetValue(bType, (u8)_bOrder) - (_wBlank[_bOrder] >> 5);
			/*bColorMaxTypeNew[bType][_bOrder];*/

			bDrvSetHDADCGain_Digital(_bOrder, _bGain);
			vDrvPreSetToReadBlank(AS_BLANK_ALWAYS);
			/*UTIL_Printf("Gain order=%2u, _bGain=%x, bInitOffset%x,target=0x%x\r\n",*/
			/*_bOrder,_bGain,_bInitOffset,_wADCTarget[_bOrder]);*/
			break;

		case DIGITAL_OFFSET_MODE:
			_wADCTarget[_bOrder] = wColorBlankValueNew[bType][_bOrder];
			_bInitOffset = wReadHDTVDigitalOffset(_bOrder);
			vDrvPreSetToReadBlank(AS_BLANK_ALWAYS);
			break;

		default:
			break;
		}
	}
}

void vDrvHDTVMeasureSetting(u8 bField_Number)
{
	vIO32WriteFldAlign(HDTV_00, Average_128_line , HDTV_BLK_CALI_LCNT);  /*LCNT * PERIOD +START < HTOTAL*/
	vIO32WriteFldAlign(HDTV_00, Per_1_line, HDTV_BLK_CALI_PERIOD);
	vIO32WriteFldAlign(HDTV_01, Start_line_32, HDTV_BLK_CALI_START);

	if (bField_Number > (u8)3) {
		bField_Number = (u8)3;  /*maximum is 8 field*/
	}

	vIO32WriteFldAlign(HDTV_02, (u32)bField_Number  , HDTV_BLK_CALI_FCNT);
	vIO32WriteFldAlign(HDTV_03, Pixel_16_per_line  , HDTV_BLANK_AVG);
}

void vDrvHDTVClampMethodDefaultSetting(u8 bType)
{
	u8 bTMP1[3];

	if (bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
		if (bType == INT_VGA_COMPOENT) {
			bTMP1[1] = wColorBlankValueNew[bType][0] >> 6;
			bTMP1[0] = wColorBlankValueNew[bType][1] >> 6;
			bTMP1[2] = wColorBlankValueNew[bType][2] >> 6;
		} else {
			bTMP1[0] = wColorBlankValueNew[bType][0] >> 6;
			bTMP1[1] = wColorBlankValueNew[bType][1] >> 6;
			bTMP1[2] = wColorBlankValueNew[bType][2] >> 6;
		}

		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[0], HDTV_BLK_CALI_Y_TAR);
		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[1], HDTV_BLK_CALI_PB_TAR);
		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[2], HDTV_BLK_CALI_PR_TAR);
		vIO32WriteFldAlign(HDTV_00, 0x02U, HDTV_BLK_CALI_MIN);/*mimumu:2  10bit*/
		vIO32WriteFldAlign(HDTV_02, 0x3fU, HDTV_BLK_CALI_THRES); /*maximum:16  8bit*/
		vIO32WriteFldAlign(HDTV_02, CALI_ENABLE  , HDTV_BLK_CALI_EN);
		/*Enable line blank average measurement function*/
		vIO32WriteFldAlign(HDTV_02, CALI_ENABLE, HDTV_BLK_CALI_ADJ_ON);  /*Enable HW blank Adjust Function*/
		vIO32WriteFldAlign(HDTV_00, CALI_DISABLE, HDTV_LCLAMP_EN);          /*Disable line clamp*/

	}

}

void vDrvHDTV_HW_AUTO_ONOFF(u8 bMode, u8 bType)
{
	u8 bTMP1[3];

	if ((bType != P_FA) && (bType <= INT_VGA_COMPOENT)) { /*for klocwork check*/
		if (bType == INT_VGA_COMPOENT) {
			bTMP1[1] = wColorBlankValueNew[bType][0] >> 6;
			bTMP1[0] = wColorBlankValueNew[bType][1] >> 6;
			bTMP1[2] = wColorBlankValueNew[bType][2] >> 6;
		} else {
			bTMP1[0] = wColorBlankValueNew[bType][0] >> 6;
			bTMP1[1] = wColorBlankValueNew[bType][1] >> 6;
			bTMP1[2] = wColorBlankValueNew[bType][2] >> 6;
		}

		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[0] , HDTV_BLK_CALI_Y_TAR);
		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[1] , HDTV_BLK_CALI_PB_TAR);
		vIO32WriteFldAlign(HDTV_02, (u32)bTMP1[2] , HDTV_BLK_CALI_PR_TAR);
	}

	vIO32WriteFldAlign(HDTV_02, (u32)bMode, HDTV_BLK_CALI_ADJ_ON);
	/*Enable or disable  HW blank Adjust Function*/
	vIO32WriteFldAlign(HDTV_02, (u32)bMode, HDTV_BLK_CALI_EN);
	/*Enable line blank average measurement function*/
	vIO32WriteFldAlign(HDTV_00, AFTER_ADJ, HDTV_BLK_STA_SEL);/*blank level after calibration*/
}


void vDrvHDTVSelBlankDataSource(void)
{
	vIO32WriteFldAlign(HDTV_02, CALI_DISABLE, HDTV_BLK_CALI_ADJ_ON);
	/*Disable HW blank Adjust Function*/
	vIO32WriteFldAlign(HDTV_00, AFTER_ADJ, HDTV_BLK_STA_SEL);/*blank level after calibration*/
	vIO32WriteFldAlign(HDTV_02, CALI_ENABLE, HDTV_BLK_CALI_EN);
	/*Enable line blank average measurement function*/
}

u8 bDvMeasureDataSel(void)
{
	if ((IO32ReadFldAlign(HDTV_00, HDTV_BLK_STA_SEL) == BEFORE_ADJ)
	&& (IO32ReadFldAlign(HDTV_02, HDTV_BLK_CALI_EN) == CALI_ENABLE)) {
		return (u8)1;
	} else {
		return (u8)0;
	}
}

void vDrvOnChipAutoColorGetADCData(u8 bMode)
{
	u32 dwSumADCRawData;/*,dwSumADCRawData2;*/
	u16 wSumADC[3][2];
	/*u8  bSampleV1V2,bSampleRGBLevel;*/
	u8  bLoopCnt, bSelV1V2;
	/*,bGainOffsetMode;dwSumADCRawData2 = u4IO32Read4B(REG_VGA_Normal_CFG0+(4*bIndexCnt));*/

	if (_bOrder < (u8)3) { /*for klocwork check*/
		switch (bMode) {
		case ON_CHIP_GAIN_MODE:
			/*UTIL_Printf("-----ON CHIP GAIN MODE----------\r\n");*/
			vDrvSelectAutoColorBufferOutputChannel(_bOrder);
			/*select channel buffer of ON CHIP AUTO COLOR*/

			/*dwSumADCRawData2 = u4IO32Read4B(REG_VGA_Normal_CFG3);*/
			/*UTIL_Printf("Select buffer=%x, register=0x%x, data=0x%x\r\n",*/
			/*_bOrder,REG_VGA_Normal_CFG3,dwSumADCRawData2);*/
			for (bSelV1V2 = (u8)0; bSelV1V2 < (u8)2; bSelV1V2++) {
				vDrvSelV1V2(bSelV1V2);             /*select V1 or V2 for the input of PGA+*/
				vUtDelay1ms((u32)10);
				wSumADC[_bOrder][bSelV1V2] = 0;
				dwSumADCRawData = 0;

				for (bLoopCnt = (u8)0; bLoopCnt < (u8)16; bLoopCnt++) {
					dwSumADCRawData += bDrvGetADCData(_bOrder);
					vUtDelay2us((u32)100);
				}

				dwSumADCRawData = ((dwSumADCRawData + 2U) / 4U);
				/* +8)/16);  //round off // average : gain calibration*/
				wSumADC[_bOrder][bSelV1V2] = dwSumADCRawData;
			}

			_wData_v1v2[_bOrder] = (wSumADC[_bOrder][0] > wSumADC[_bOrder][1]) ?
			(wSumADC[_bOrder][0] - wSumADC[_bOrder][1]) : (wSumADC[_bOrder][1] - wSumADC[_bOrder][0]);
			break;

		case OFFSET_MODE:
			dwSumADCRawData = 0U;

			if (bDvMeasureDataSel() == (u8)1) {
				dwSumADCRawData = bDrvGetBlankVal(_bOrder);
				/*to read the blank level of Y, Pb, Pr respective*/
				_wData_v1v2[_bOrder] = (dwSumADCRawData) << 3;
				/* 10 bits to 13 bits = 8bit sum 32 times*/
			} else {
				for (bLoopCnt = (u8)0; bLoopCnt < (u8)32; bLoopCnt++) {
					dwSumADCRawData += bDrvGetBlankVal(_bOrder);
					/*to read the blank level of Y, Pb, Pr respective*/
					vUtDelay2us((u32)50);
				}

				_wData_v1v2[_bOrder] = (dwSumADCRawData + 2U) >> 2;
				/* 10 bits to 8 bits // +8) >> 4; // 12 bits to 8 bits*/
			}

			break;

		case GAIN_MODE:
			dwSumADCRawData = 0U;

			for (bLoopCnt = (u8)0; bLoopCnt < (u8)32; bLoopCnt++) {
				dwSumADCRawData += bDrvOnChipGetADCMaxMinValue(_bOrder, (u8)AS_PHASE_MAX_SEL);
				/*to read the blank level of Y, Pb, Pr respective*/
				vUtDelay2us((u32)80);
			}

			_wData_v1v2[_bOrder] = dwSumADCRawData / 32U;
			/*bDrvOnChipGetADCMaxMinValue(_bOrder,AS_PHASE_MAX_SEL);*/
			break;

		default:
			break;
		}

		/*UTIL_Printf("-- channel %u sample data=%x\r\n",_bOrder,_wData_v1v2[_bOrder]);*/
	}
}

void bDrvSetSmallStep(u8 bMode)
{
	if (bMode == GAIN_MODE) {
		if (_bCaliStep[_bOrder] > (u16)0x200) {
			_bCaliStep[_bOrder] = (u16)0x200;
		}
	} else {
		if (_bCaliStep[_bOrder] > (u16)0x04) {
			_bCaliStep[_bOrder] = (u16)0x04;
		}
	}
}

void vDrvOnChipAutoColorSearchTarget_V2(u8 bMode)
{
	u8 bGainOffsetMode, bTolenence;
	u16 wAdcBlankLevel;
#define MAX_TOGGLE_TIME         40

	if ((_bOrder < 3) && (bMode <= DIGITAL_OFFSET_MODE)) { /*for Kclockwork check*/
		if (bMode == OFFSET_MODE) {                          /*MC20080417_1 ****/
			bTolenence = (u8)((_bAutoColorTimeOutCNT > (u8)6) ? (0X10) : (0X00));
		} else {
			bTolenence = (u8)((_bAutoColorTimeOutCNT > (u8)6) ? (1) : (0));
		}

		if (bMode == GAIN_MODE) {
			wAdcBlankLevel = bDrvGetBlankVal(_bOrder);

			if ((wAdcBlankLevel <= (_wBlank[_bOrder] >> 3) + (bOnChipCheckTolerance[OFFSET_MODE][1]
			+ bTolenence)) && (wAdcBlankLevel >= (_wBlank[_bOrder] >> 3)
			- (bOnChipCheckTolerance[OFFSET_MODE][1] + bTolenence))) {
				wAdcBlankLevel = (_wBlank[_bOrder] >> 3);
			}

			_wData_v1v2[_bOrder] = (_wData_v1v2[_bOrder]) - (wAdcBlankLevel >> 2); /*gain-blank*/
		}


		_bIncGainDirPre[_bOrder] = _bGainIncDir[_bOrder];

		if ((_bChgDirToggle[_bOrder] > MAX_TOGGLE_TIME) || (_bTotalCnt > 200)) { /* if timeout*/
			_bGain = _bIndex_mim[_bOrder];

			if (bMode == OFFSET_MODE) {
				vDrvSetHDADCDigitalOffset(_bOrder, _bGain);
			} else {
				bDrvSetHDADCGain_Digital(_bOrder, _bGain);
			}

			_bGainOffset_channel |= (0x01U << _bOrder);
		} else if ((_wData_v1v2[_bOrder] <= (_wADCTarget[_bOrder] + bOnChipCalibrateTolerance[bMode][0]
		+ bTolenence)) && (_wData_v1v2[_bOrder] >= (_wADCTarget[_bOrder] -
		(bOnChipCalibrateTolerance[bMode][1] + bTolenence)))) {
			if ((_bDoubleChkFlag[_bOrder] == 1) || (_bChgDirToggle[_bOrder] > 5)) {
				_bGainOffset_channel |= (0x01U << _bOrder);
			} else {
				bDrvSetSmallStep(bMode);
				_bDoubleChkFlag[_bOrder] = 1;           /*no change Gain/Offset*/
			}
		} else {                            /*change Gain/Offset step size*/
			if ((_wData_v1v2[_bOrder] <= (_wADCTarget[_bOrder] +
			bOnChipCalibrateTolerance[bMode][2])) && (_wData_v1v2[_bOrder] >=
			(_wADCTarget[_bOrder] - bOnChipCalibrateTolerance[bMode][3]))) { /*0x0087~0x008B*/
				bDrvSetSmallStep(bMode);
			}

			_bDoubleChkFlag[_bOrder] = 0;

			if (_wData_v1v2[_bOrder] < _wADCTarget[_bOrder]) {
				bGainOffsetMode = 0;  /*to increase offset/gain*/

				if (bMode == OFFSET_MODE) {
					if (_bGain >= (u16)0x1FE) {
						_bChgDirToggle[_bOrder] = MAX_TOGGLE_TIME + 1; /*set quit condition*/
					}
				} else {
					if (_bGain >= (u16)0xfC00) {
						_bChgDirToggle[_bOrder] = MAX_TOGGLE_TIME + 1;  /*set quit condition*/
					}
				}

				_wADC_Diff_value[_bOrder] = (_wADCTarget[_bOrder] - _wData_v1v2[_bOrder]);
			} else {
				bGainOffsetMode = (u8)1;      /*to decrease offset/gain*/

				if (bMode == OFFSET_MODE) {
					if (_bGain <= 0x00) {
						_bChgDirToggle[_bOrder] = MAX_TOGGLE_TIME + 1;  /*set quit condition*/
					}
				} else {
					if (_bGain <= (u16)0x400) {
						_bChgDirToggle[_bOrder] = MAX_TOGGLE_TIME + 1;  /*set quit condition*/
					}
				}

				_wADC_Diff_value[_bOrder] = (_wData_v1v2[_bOrder] - _wADCTarget[_bOrder]);
			}

			if (_wADC_Diff_value[_bOrder] < _wDiff_mim[_bOrder]) {
				_wDiff_mim[_bOrder] = _wADC_Diff_value[_bOrder];
				_bIndex_mim[_bOrder] = _bGain;
			}

			switch (bGainOffsetMode) {
			case 0:     /*to increase offset/gain*/
				if ((_bGainIncDir[_bOrder] == 0) && (bMode == GAIN_MODE)) {
					if (_bCaliStep[_bOrder] > 0x80) {
						_bCaliStep[_bOrder] = _bCaliStep[_bOrder] >> 1;/* calful °£¥H2*/
					}

					/*else
					{
					    if(_wData_v1v2[_bOrder]<5)
					    {
						 _bCaliStep[_bOrder] = (_wADCTarget[_bOrder]-
						 (_wData_v1v2[_bOrder]>>1));
						//transfer to 10 bit base
					    }
					    else
						{
							 _bCaliStep[_bOrder] = (_wADCTarget[_bOrder]-
							 _wData_v1v2[_bOrder])>>3;
							 //transfer to 10 bit base
					    }
					//                          if (_bCaliStep[_bOrder] > 1)
					//                          {
					//                              _bCaliStep[_bOrder] = _bCaliStep[_bOrder] >> 1;
					//              }
					    }*/
				}

				_bChgDirToggle[_bOrder]++;
				_bGainIncDir[_bOrder] = 1;

				if (bMode == GAIN_MODE) {
					_bCaliStep[_bOrder] = (((u32)(_wADCTarget[_bOrder] << 2)) -
						((u32)(_wData_v1v2[_bOrder] << 2))) * 40;

					if (_bGain < (0xffff - _bCaliStep[_bOrder])) {
						_bGain += _bCaliStep[_bOrder];
					} else {
						_bGain += 0x80;
					}
				} else {
					if (((_wADCTarget[_bOrder] - _wData_v1v2[_bOrder]) >> 3) < 2) {
						_bCaliStep[_bOrder] = 1;
					} else {
						/*if(_bOrder>0)*/
						{
							_bCaliStep[_bOrder] = ((_wADCTarget[_bOrder] -
								_wData_v1v2[_bOrder]) >> 3);
							/*transfer to 8 bit base*/
						}
						/*else*/
						/*  {*/
						/*  _bCaliStep[_bOrder] = ((_wADCTarget[_bOrder]-
						_wData_v1v2[_bOrder])>>3)<<1;
						//transfer to 8 bit base*/
						/*  }*/
					}

					/*if(_wData_v1v2[_bOrder]<5)*/
					/*{*/
					/*  _bCaliStep[_bOrder] = (_wADCTarget[_bOrder]-(_wData_v1v2[_bOrder]>>1));
					//transfer to 10 bit base*/
					/*}*/
					/*else*/
					/*{*/
					/*  _bCaliStep[_bOrder] = (_wADCTarget[_bOrder]-_wData_v1v2[_bOrder])>>3;
					//transfer to 10 bit base*/
					/*}*/
					_bGain = vCalculateDigOffset(_bGain, _bCaliStep[_bOrder], bGainOffsetMode);
				}

				break;

			case 1:

				/*
				if(bMode == GAIN_MODE)
				{
				    Printf("CH%x need to decrease gain,  _bGainIncDir=%x\r\n",
				    _bOrder,_bGainIncDir[_bOrder]);
				}
				else
				{
				    Printf("CH%x need to decrease offset, _bGainIncDir=%x\r\n",
				    _bOrder,_bGainIncDir[_bOrder]);
				}
				*/
				if ((_bGainIncDir[_bOrder] == 1) && (bMode == GAIN_MODE)) {
					if (_bCaliStep[_bOrder] > 0x80) {
						_bCaliStep[_bOrder] = _bCaliStep[_bOrder] >> 1;
					}

					/*     else
						{

						if(_wData_v1v2[_bOrder]>(_wADCTarget[_bOrder]<<2))
						{
						_bCaliStep[_bOrder] =_bGain/3*2;
						// (_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])>>1;
						//transfer to 10 bit base

						}
						else if (_wData_v1v2[_bOrder]>(_wADCTarget[_bOrder]<<1))
						{
						 _bCaliStep[_bOrder] =_wData_v1v2[_bOrder]>>5;
						 // (_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])>>1;
						 //transfer to 10 bit base

						}
						else
						{
							_bCaliStep[_bOrder] = (_wData_v1v2[_bOrder]-
							_wADCTarget[_bOrder])>>3;
							//transfer to 10 bit base
						}

					_bGain=vCalculateDigOffset(_bGain,_bCaliStep[_bOrder],bGainOffsetMode);
				}*/
				}

				_bChgDirToggle[_bOrder]++;
				_bGainIncDir[_bOrder] = 0;

				if (bMode == GAIN_MODE) {
					_bCaliStep[_bOrder] = (((u32)(_wData_v1v2[_bOrder] << 2))
					- ((u32)(_wADCTarget[_bOrder] << 2))) * 40;

					if (_bGain > _bCaliStep[_bOrder]) {
						_bGain -= _bCaliStep[_bOrder];
					} else if (_bGain < _bCaliStep[_bOrder]) {
						_bGain = 0;
					} else {
						_bGain -= 0x80;

					}
				} else {
					if (((_wData_v1v2[_bOrder] - _wADCTarget[_bOrder]) >> 3) < 2) {
						_bCaliStep[_bOrder] = 1;
					} else {
						/*if(_bOrder>0)*/
						{
							_bCaliStep[_bOrder] = ((_wData_v1v2[_bOrder] -
								_wADCTarget[_bOrder]) >> 3);
							/*transfer to 8 bit base*/
						}
						/*else*/
						/*{*/
						/*  _bCaliStep[_bOrder] = ((_wData_v1v2[_bOrder]
						_wADCTarget[_bOrder])>>3)<<1;
						//transfer to 8 bit base*/
						/*}*/

					}

					/*if(_wData_v1v2[_bOrder]>(_wADCTarget[_bOrder]<<2))*/
					/*  {*/
					/*          _bCaliStep[_bOrder] =(_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])/10;
					// (_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])>>1; //transfer to 10 bit base*/
					/*          if(_bCaliStep[_bOrder] >=0xFF)*/
					/*              {*/
					/*              _bCaliStep[_bOrder] =0xFF;*/
					/*              }*/

					/*  }*/
					/*else if (_wData_v1v2[_bOrder]>(_wADCTarget[_bOrder]<<1))*/
					/*{*/
					/*      _bCaliStep[_bOrder] =(_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])/5;
					// (_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])>>1; //transfer to 10 bit base*/

					/*}*/
					/*  else*/
					/*  {*/
					/*       _bCaliStep[_bOrder] = (_wData_v1v2[_bOrder]-_wADCTarget[_bOrder])>>3;
					//transfer to 10 bit base*/
					/*  }*/
					_bGain = vCalculateDigOffset(_bGain, _bCaliStep[_bOrder], bGainOffsetMode);
					/* if(_bGain>_bCaliStep[_bOrder])*/
					/* {*/
					/*         _bGain += _bCaliStep[_bOrder];*/
					/* }*/
					/* else*/
					/*{*/
					/*_bGain ++;*/
					/*}*/
				}

				break;
				
			default:
				break;
			}

			/*
			if(bMode == GAIN_MODE)
			{
			 Printf("ch%x new gain=%x\r\n",_bOrder,_bGain);
			}
			else
			{
			 Printf("ch%x new offset=%x\r\n",_bOrder,_bGain);
			}
			*/
		}
	}
}

u8 bDrvOnChipAutoColorCheckSignalReady(void)
{
	u8 bIntputType, bType;

	bType = (u8)P_FA;

	/*1. to check input type*/
	/*MC20080417_1 &&&*/
	if (g_u4SrcType == SRC_YBR) {
		bIntputType = P_YP1;
	} else if (g_u4SrcType == SRC_VGA) {
		bIntputType = P_VGA;
	} else {
		bIntputType = (u8)P_FA;
	}

	if (((_IsVgaDetectDone == (bool)TRUE) || (_IsHdtvDetectDone == (bool)TRUE)) && (fgIsCLKLock())) {
		bType  = bDrvOnChipGetVFESignalType(bIntputType);  /*Signal is stable and ready*/
	} else {
		bType = P_FA;
	}

	return bType;

}


void vDrvOnChipGetADCMaxValue(void)
{
	_wData_v1v2[0] = bDrvOnChipGetADCMaxMinValue((u8)0, (u8)AS_PHASE_MAX_SEL);
	_wData_v1v2[1] = bDrvOnChipGetADCMaxMinValue((u8)1, (u8)AS_PHASE_MAX_SEL);
	_wData_v1v2[2] = bDrvOnChipGetADCMaxMinValue((u8)2, (u8)AS_PHASE_MAX_SEL);
}




void vDrvBlankAdjParaReset(void)
{
	u8 bCnt;

	_bGainOffset_channel = 0x00;
	_bAutoColorTimeOutCNT = 0;

	for (bCnt = (u8)0; bCnt < (u8)3; bCnt++) {
		_bDoubleChkFlag[bCnt] = 0;
		_dwBlankAvg[bCnt] = 0;
		_dwMaxLevelAvg[bCnt] = 0;
	}
}

void vDrvBlankMeasureParaReset(void)
{
	u8 bCnt;

	_bAutoColorTimeOutCNT = 0;

	for (bCnt = (u8)0; bCnt < (u8)3; bCnt++) {
		_dwBlankAvg[bCnt] = 0;
		_dwMaxLevelAvg[bCnt] = 0;
	}
}

void vDrvEnableBlankLevelAdjust(void)
{
	_bBlankIteration = 0;
	_bAdjGainAfterBlank = 0;
	/*    _bWaitSignalStable = 0;*/
	_bDoubleCheck = (u8)ADJUST_BLANK_RETRY_COUNT;
	vDrvBlankAdjParaReset();
	_fgblankfinish = 0;
	_bAdjBlkState0 = BLK_START;
}

void vDrvEnableBlankLevelMeasure(void)
{
	_bAdjBlkState0 = BLK_MEASURE;
	_bDoubleCheck = 0;   /*this flag is used to distinguish the blank level adjust and blank level measurement*/
	vDrvBlankMeasureParaReset();
}

void vDrvMeasureMaxBlankLevel(void)
{
	for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
		vDrvOnChipAutoColorGetADCData(OFFSET_MODE);

		if (bDvMeasureDataSel() == (u8)1) {
			_wData_v1v2[_bOrder] = _wData_v1v2[_bOrder] >> 3; /*10 BIT resoultion*/
		} else {
			_wData_v1v2[_bOrder] = (_wData_v1v2[_bOrder] + 4) / 8; /*10 BIT resoultion*/
		}

		_dwBlankAvg[_bOrder] += _wData_v1v2[_bOrder];
	}

#if AUTO_BLANK_MEASURE
	pr_debug("CH1=%u,  CH2=%u, CH3=%u\r\n", _wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
#endif
	vDrvOnChipGetADCMaxValue();
	_dwMaxLevelAvg[0] += _wData_v1v2[0];    /*8 bit resolution*/
	_dwMaxLevelAvg[1] += _wData_v1v2[1];
	_dwMaxLevelAvg[2] += _wData_v1v2[2];

#if AUTO_BLANK_MEASURE
	pr_debug("MAX 1=%3u, Max2=%3u, Max3=%3u\r\n", _wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
#endif

}

#if PGA_4bit_offset
#define ADJUST_TOLRENCE 0X38   /* 0x38=10bit 7 code.  can not bigger than 0X40,*/
#else
#ifdef FULLY_HW_AUTO_CALIBRATION
#define ADJUST_TOLRENCE 0X10   /*0x10= 10bit 2 code*/
#else
#define ADJUST_TOLRENCE 0X08   /*0x08= 10bit 1 code*/
#endif
#endif

#define ADJUST_MAX_TIME 40
#define GAIN_DOWN   0
#define GAIN_UP         1
/*#define WAIT_STABLE_FIELD_NUMBER   4*/

#if AUTO_BLANK_DEBUG
void vDrvShowDigGainOffset(u8 bOrder)
{
#if DIGITAL_NEW_OFFSET    /*reset the digital offset*/
	u16 wDgain;
	u8 bDOffset;

	wDgain = wReadHDTVDigitalGain(bOrder);
	bDOffset = wReadHDTVDigitalOffset(bOrder);
	_bDigSignBit = (_bOrder) & 0x80;

	if (_bDigSignBit == 0) {
		if (_bAdjBlkState0 == BLK_MEASURE) {
			pr_debug("After BlkAdj: Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, (INT8)(0 - bDOffset));
		} else if (_bAdjBlkState0 == BLK_START) {
			pr_debug("Before BlkAdj: Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, (INT8)(0 - bDOffset));
		} else {
			pr_debug("Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, (INT8)(0 - bDOffset));
		}
	} else {
		if (_bAdjBlkState0 == BLK_MEASURE) {
			pr_debug("After BlkAdj: Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, bDOffset);
		} else if (_bAdjBlkState0 == BLK_START) {
			pr_debug("Before BlkAdj: Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, bDOffset);
		} else {
			pr_debug("Digital gain & offset: CH=%u, DGain=0x%x, Doffset=%d\r\n",
			bOrder, wDgain, bDOffset);
		}
	}

#endif
}

void vDrvShowAnalogGainOffset(u8 bOrder)
{
	u16 bAOffset;
	/*bAgain = bDrvGetHDADCGain_Digital(bOrder);*/
	bAOffset = bDrvGetHDADCOffset_Digital(bOrder); /*V2 OK*/

	if (_bAdjBlkState0 == BLK_MEASURE) {
		pr_debug("After BlkAdj: analog offset : CH=%u, Aoffset=0x%x\r\n", bOrder, bAOffset);
	} else if (_bAdjBlkState0 == BLK_START) {
		pr_debug("Before BlkAdj: analog offset : CH=%u, Aoffset=0x%x\r\n", bOrder, bAOffset);
	} else {
		pr_debug("analog offset : CH=%u, Aoffset=0x%x\r\n", bOrder, bAOffset);
	}

}
#endif

u8 bDrvEFuseOffsetREADY(void)
{
	u8 bResult = 0;

	if ((_bEFUSE_AUTOCOLOR_READY == (u8)1) && (_bType < INT_VGA_COMPOENT)) {
		bResult = (_bOffsetCaliDone[_bType] == 0) ? 0 : 1;
	} else {
		bResult = (u8)1;
	}

	return bResult;
}

#define NO_NEED_TO_DO_ANALOG_CALIBRATION  1
#define NEED_TO_DO_ANALOG_CALIBRATION  0
#define BLANKUNIT_10BIT_1CODE     8   /*DON'T CHANGE*/
#define MAX_STEP 0x40

#define BLANK_ADJ_LOW_TOLERENCE          48  /*6  10 bit*/
#define BLANK_ADJ_UP_TOLERENCE              80 /*10   10 bit   */

u8 bDrvCheckAnalogBlankLevel(void)
{
	if (_bOrder < (u8)3) { /*for Kclockwork check*/
#ifdef FULLY_HW_AUTO_CALIBRATION
		if (bDrvEFuseOffsetREADY() == SV_FALSE) {
			/* This  is used for  E-FUSE  autocolor function.
			It will only execute one time for the VGA/YPbPr source is selected*/
			if ((_wData_v1v2[_bOrder] < (_wADCTarget[_bOrder] - BLANKUNIT_10BIT_1CODE)) ||
				(_wData_v1v2[_bOrder] > (_wADCTarget[_bOrder] + BLANKUNIT_10BIT_1CODE))) {
				/* target +/- 1  (10bit)*/
				return NEED_TO_DO_ANALOG_CALIBRATION;
				/* less than low boundary or bigger than high boundary --> need to do calibration*/
			} else {
				return (u8)NO_NEED_TO_DO_ANALOG_CALIBRATION;
			}
		} else
#endif
		{
			if ((_wData_v1v2[_bOrder] < (_wADCTarget[_bOrder] - BLANK_ADJ_LOW_TOLERENCE)) ||
				(_wData_v1v2[_bOrder] > (_wADCTarget[_bOrder] + BLANK_ADJ_UP_TOLERENCE))) {
				/* less than 10 bit 4  or bigger than (target+ 10 bit 4 )*/
				return NEED_TO_DO_ANALOG_CALIBRATION;
				/* less than low boundary or bigger than high boundary --> need to do calibration*/
			} else {
				return (u8)NO_NEED_TO_DO_ANALOG_CALIBRATION;
			}
		}
	}

	return (u8)NO_NEED_TO_DO_ANALOG_CALIBRATION;
}


void vDrvDigitalOffsetAdjBlk(void)
{

	u8 bCmpResult, bAdjOffset;

	vDrvOnChipAutoColorGetADCData(OFFSET_MODE);
	vDrvOnChipAutoColorModeSet(DIGITAL_OFFSET_MODE, _bType);    /*_bInitOffset is 10 bit base*/

	if (_bOrder < (u8)3) { /*only for Klocwork check*/

		{
			if (_wData_v1v2[_bOrder] > (_wADCTarget[_bOrder] + 0x8)) { /* 0X10=10 bit 2*/
				bAdjOffset = (u8)((_wData_v1v2[_bOrder] - _wADCTarget[_bOrder]) >> 3);
				/*transfer to 10 bit base*/
				_bDoubleChkFlag[_bOrder] = 0;
				bCmpResult = (u8)1;
				_bInitOffset = vCalculateDigOffset(_bInitOffset, (u16)bAdjOffset, bCmpResult);

				if (_bInitOffset == (u16)0x1FE) {
					_bGainOffset_channel |= (0x01U << _bOrder);  /*compensate limit*/
				} else {
					vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);
				}
			} else if (_wData_v1v2[_bOrder] < (_wADCTarget[_bOrder] - 0x8)) {
				bAdjOffset = (u8)((_wADCTarget[_bOrder] - _wData_v1v2[_bOrder]) >> 3);
				/*transfer to 10 bit base*/
				_bDoubleChkFlag[_bOrder] = 0; /*_wADCTarget*/
				bCmpResult = 0;
				bAdjOffset = (u8)((_wADCTarget[_bOrder] - _wData_v1v2[_bOrder]) >> 3);
				/*transfer to 10 bit base*/
				_bInitOffset = vCalculateDigOffset(_bInitOffset, (u16)bAdjOffset, bCmpResult);

				if (_bInitOffset == (u16)0x1fE) {
					_bGainOffset_channel |= (0x01U << _bOrder);  /*compensate limit*/
				} else {
					vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);
				}
			} else {
				if (_bDoubleChkFlag[_bOrder] == 1) {
					if (_wData_v1v2[_bOrder] >= (_wADCTarget[_bOrder] + 6)) {
						bCmpResult = (u8)1;
						bAdjOffset = (u8)1;
						_bInitOffset = vCalculateDigOffset(_bInitOffset,
									(u16)bAdjOffset, bCmpResult);
						vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);
					} else if (_wData_v1v2[_bOrder] <= (_wADCTarget[_bOrder] - 6)) {
						bCmpResult = 0;
						bAdjOffset = (u8)1;
						_bInitOffset = vCalculateDigOffset(_bInitOffset,
									(u16)bAdjOffset, bCmpResult);
						vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);
					} else {
						bAdjOffset = 0;
					}

					_bGainOffset_channel |= (0x01U << _bOrder);
				} else {
					_bDoubleChkFlag[_bOrder] = 1;
				}
			}
		}
	}

}


void vDrvResetAllGainOffset(void)
{
	if (_bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
		for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
			vDrvOnChipAutoColorModeSet(GAIN_MODE, _bType);
			vDrvOnChipAutoColorModeSet(OFFSET_MODE, _bType);
			vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);



		}
	}
}

void vDrvCalculateMaxBLKAvg(void)
{
	for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
		_dwMaxLevelAvg[_bOrder] = (_dwMaxLevelAvg[_bOrder] +
		(_bAutoColorTimeOutCNT / 2)) / _bAutoColorTimeOutCNT;
		_dwBlankAvg[_bOrder] = (_dwBlankAvg[_bOrder] + (_bAutoColorTimeOutCNT / 2)) / _bAutoColorTimeOutCNT;
	}
}

void vDrvAdjustBlankLevel(void)
{
	static u8 _bAdjBlkState0_flow;


	if ((!fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) || (!(_bAutoColorState0 == VDO_AUTO_COLOR_NOT_BEGIN))) {

		return;
	}

	if (_bAdjBlkState0 == BLK_NOTHING) {
		return;
	}

	_bType = bDrvOnChipAutoColorCheckSignalReady();

	if ((_bType > INT_VGA_COMPOENT)  || (_bType == P_FA)) { /*for klocwork check*/
		return;
	}
	if (_bAdjBlkState0 == BLK_START) {
		_bOrgType = _bType;
	} else {
		if (_bType != _bOrgType) { /*type is changed and to restart the flow*/
			pr_debug("---Input type is changed. to restart the adjustment flow ----\r\n");
			vDrvEnableBlankLevelAdjust();
			return;
		}
	}

	if (_bAdjBlkState0 != _bAdjBlkState0_flow) {
		pr_debug("=== state of adjust blank =%x\r\n", _bAdjBlkState0);
		_bAdjBlkState0_flow = _bAdjBlkState0;
	}

	switch (_bAdjBlkState0) {
	case BLK_START:

		/*_bType = bDrvOnChipAutoColorCheckSignalReady();*/
		if ((_bType <= INT_VGA_COMPOENT) && (_bType != P_FA)) { /*for klocwork check*/
			vDrvResetAllGainOffset();
			vDrvHDTVSelBlankDataSource();
			_bAdjGainAfterBlank = 0;
			vDrvPreSetToReadBlank((u8)AS_BLANK_ALWAYS);

			if (_bEFUSE_AUTOCOLOR_READY == (u8)1) {
				_bAdjBlkState0 = BLK_LEVEL_STABLE;  /*always do blank level adjust check*/
			} else {
				if (_bAutoColorHistory[_bType] != SV_TRUE) {
					vDrvHDTVMeasureSetting(UPDATE_PER_1_FIELD);
					vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);
					_fgblankfinish = (u8)1;
					_bAdjBlkState0 = BLK_NOTHING;
					/*no need to jump to _bAdjBlkState0= BLK_CLAMP_DELAY;*/
					vDrvBlankAdjParaReset();
				} else {
					_bAdjBlkState0 = BLK_LEVEL_STABLE;
				}
			}

		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		break;

	case BLK_LEVEL_STABLE:
		vDrvPreSetToReadBlank((u8)AS_BLANK_ALWAYS);

		if (_bAutoColorTimeOutCNT < (u8)4) {
			_bAutoColorTimeOutCNT++;
			vDrvMeasureMaxBlankLevel();
		} else {
			vDrvCalculateMaxBLKAvg();

			if ((_bType == INT_VGA) && ((_dwBlankAvg[0] > 128) ||
			(_dwBlankAvg[1] > 128) || (_dwBlankAvg[2] > 128))) { /*8 bit 32*/
				_fgblankfinish = (u8)1;
				_bAdjBlkState0 = BLK_NOTHING;
			} else {

				if (bDrvEFuseOffsetREADY() == SV_FALSE) { /* if never do offset then do offset adjust*/
					pr_debug("Pre-check, Blank level: CH1=%3u,  CH2=%3u, CH3=%3u\r\n",
					(unsigned int)_dwBlankAvg[0], (unsigned int)_dwBlankAvg[1],
					(unsigned int)_dwBlankAvg[2]);
					_bAdjBlkState0 = BLK_OFFSET_ADJ;
				} else

				{
					_bAdjBlkState0 = BLK_OFFSET_CHECK;
				}

				vDrvBlankAdjParaReset();
			}
		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		break;

	case BLK_OFFSET_ADJ:
		if ((_bType <= INT_VGA_COMPOENT) && (_bType != P_FA)) { /*for klocwork check*/
			if ((_bAutoColorTimeOutCNT < ADJUST_MAX_TIME) && ((_bGainOffset_channel & 0x07) != 0x07)) {
				_bAutoColorTimeOutCNT++;
				_bDiffOffset = (_bAutoColorTimeOutCNT > (ADJUST_MAX_TIME / 2)) ?
				(ADJUST_TOLRENCE + 0x08) : (ADJUST_TOLRENCE) ; /*0x08 is 10 bit 1*/

				for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
					if (!(_bGainOffset_channel & (0x01U << _bOrder))) {

						vDrvDigitalOffsetAdjBlk();

					}
				}

				_bAdjBlkState0 = BLK_OFFSET_DELAY;
				/* [SA7_Michael] 080828 for adjusting blanking level stable*/
			} else {
#ifdef FULLY_HW_AUTO_CALIBRATION

				if (bDrvEFuseOffsetREADY() == (u8)SV_FALSE) {
					/* if never do offset then set the flag and store the offset*/
					_bOffsetCaliDone[_bType] = 1;

					for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
						_bAutoColorOffset[_bType][_bOrder] =
							bDrvGetHDADCOffset_Digital(_bOrder);
					}

					/*vVgaSetInputCapature(_bVgaTiming);*/
					/*vDrvEnableBlankLevelAdjust();*/
					pr_debug(" Offset_calibration spends %u times\r\n",
					(_bAutoColorTimeOutCNT - 1));
					vDrvHDTVMeasureSetting(UPDATE_PER_1_FIELD);
					vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);
					_fgblankfinish = (u8)1;
					_bAdjBlkState0 = BLK_NOTHING;
					/*no need to jump to _bAdjBlkState0= BLK_CLAMP_DELAY;*/
					/*_bAdjBlkState0= BLK_MEASURE ; //for test*/
					vDrvBlankAdjParaReset();

				} else {
					pr_debug(" Offset_calibration spends %u times\r\n",
					(_bAutoColorTimeOutCNT - 1));
					vDrvHDTVMeasureSetting(UPDATE_PER_1_FIELD);
					vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);
					_fgblankfinish = (u8)1;
					_bAdjBlkState0 = BLK_NOTHING;
					/*no need to jump to _bAdjBlkState0= BLK_CLAMP_DELAY;*/
					/*_bAdjBlkState0= BLK_MEASURE ; //for test*/
					vDrvBlankAdjParaReset();
				}

#else
				_bAdjBlkState0 = BLK_OFFSET_CHECK;
				vDrvBlankAdjParaReset();
#endif

				if (_bAdjBlkState0 == BLK_NOTHING) {
					_fgblankfinish = (u8)1;
					_bAdjBlkState0 = BLK_APICMD;
				}
			}
		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		break;

	case BLK_APICMD:
		_dwBlankAvg_1[0] = bDrvGetBlankVal(0); /*kal for sony*/
		_dwBlankAvg_1[1] = bDrvGetBlankVal((u8)1);
		_dwBlankAvg_1[2] = bDrvGetBlankVal((u8)2);
		_dwMaxLevelAvg_1[0] = IO32ReadFldAlign(STA_SYNC0_07, AS_RMAXMIND);
		_dwMaxLevelAvg_1[1] = IO32ReadFldAlign(STA_SYNC0_08, AS_GMAXMIND);
		_dwMaxLevelAvg_1[2] = IO32ReadFldAlign(STA_SYNC0_09, AS_BMAXMIND);
		/*DBG_Printf,"API read blank %4d %4d %4d\r\n", */
		/*_dwBlankAvg_1[0], _dwBlankAvg_1[1], _dwBlankAvg_1[2]); vDrvBlankAdjParaReset();*/
		_fgblankfinish = (u8)1;
		_bAdjBlkState0 = BLK_NOTHING;
		break;

	case BLK_OFFSET_DELAY:  /* [SA7_Michael] 080828 for adjusting blanking level stable*/
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		_bAdjBlkState0 = BLK_OFFSET_DELAY1;
		break;

	case BLK_OFFSET_DELAY1:  /* [SA7_Michael] 080828 for adjusting blanking level stable*/
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		_bAdjBlkState0 = (bDvMeasureDataSel() == 1) ?  BLK_OFFSET_DELAY2 :  BLK_OFFSET_ADJ;
		break;

	case BLK_OFFSET_DELAY2:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		_bAdjBlkState0 = BLK_OFFSET_ADJ;
		break;

	case BLK_OFFSET_CHECK:
		vDrvPreSetToReadBlank(AS_BLANK_ALWAYS);

		if (_bAutoColorTimeOutCNT < (u8)4) {
			_bAutoColorTimeOutCNT++;
			vDrvMeasureMaxBlankLevel();
		} else {
			vDrvCalculateMaxBLKAvg();

			/*UTIL_Printf(" Before check the _bGainOffset_channel=%u\r\n",_bGainOffset_channel);*/
			for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {

				if (!(_bGainOffset_channel & (0x01U << _bOrder))) {

					_wData_v1v2[_bOrder] = _dwBlankAvg[_bOrder] << 3;
					/*change to 13 bit base for bDrvCheckAnalogBlankLevel()*/
					_wADCTarget[_bOrder] = wColorBlankValueNew[_bType][_bOrder];

					if (bDrvCheckAnalogBlankLevel() == NO_NEED_TO_DO_ANALOG_CALIBRATION) {
						_bGainOffset_channel |= (0x01U << _bOrder);
					}
				}
			}

			if (_bBlankIteration == 0) {
				pr_debug("     \r\n");
				pr_debug("before Blank adjust, Blank level: CH1=%3u,  CH2=%3u, CH3=%3u\r\n",
				(unsigned int)_dwBlankAvg[0], (unsigned int)_dwBlankAvg[1],
				(unsigned int)_dwBlankAvg[2]);
				pr_debug("before Blank adjust, Maximum level: CH1=%3u,  CH2=%3u, CH3=%3u\r\n",
				(unsigned int)_dwMaxLevelAvg[0], (unsigned int)_dwMaxLevelAvg[1],
				(unsigned int)_dwMaxLevelAvg[2]);
			} else {

			}

			_bBlankIteration++;

			if (((_bGainOffset_channel & 0x07U) != 0x07) && (_bBlankIteration < (u8)3)) {
				_bAdjBlkState0 = BLK_OFFSET_ADJ;
				pr_debug("--- NEED_TO_DO_ANALOG_CALIBRATION ---\r\n");
			} else {
				pr_debug("--- NO_NEED_TO_DO_ANALOG_CALIBRATION ---\r\n");
				vDrvHDTVMeasureSetting(UPDATE_PER_1_FIELD);
				vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);
				_fgblankfinish = (u8)1;
				_bAdjBlkState0 = BLK_NOTHING;   /*no need to jump to _bAdjBlkState0= BLK_CLAMP_DELAY;*/
				/*_bAdjBlkState0= BLK_MEASURE ; //for test*/
				_bAdjBlkState0 = BLK_APICMD;
				vDrvBlankAdjParaReset();

			}
		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		break;

	case BLK_MEASURE:
		vDrvPreSetToReadBlank(AS_BLANK_ALWAYS);

		if (_bAutoColorTimeOutCNT < (u8)8) {
			_bAutoColorTimeOutCNT++;
			vDrvMeasureMaxBlankLevel();
		} else {
			_bGainOffset_channel = 0;

			for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
				_dwBlankAvg[_bOrder] = (_dwBlankAvg[_bOrder] +
				(_bAutoColorTimeOutCNT / 2)) / _bAutoColorTimeOutCNT;
				_dwMaxLevelAvg[_bOrder] = (_dwMaxLevelAvg[_bOrder] +
				(_bAutoColorTimeOutCNT / 2)) / _bAutoColorTimeOutCNT;

				/*#if AUTO_BLANK_DEBUG*/
				/*UTIL_Printf("CH=%u,Offset=%x\r\n",_bOrder,bDrvGetHDADCOffset(_bOrder));*/
				/*#endif*/
			if (_bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
				if (_bDoubleCheck > 0) {
					if ((_dwBlankAvg[_bOrder] <= ((wColorBlankValueNew[_bType][_bOrder] >> 3) + 1))
						&& (_dwBlankAvg[_bOrder] >=
						((wColorBlankValueNew[_bType][_bOrder] >> 3) - 1))) {
						/* 10 Bit resolution*/
						_bGainOffset_channel |= (0x01U << _bOrder);
					}
				}
			}
			}


			if (((_bGainOffset_channel & 0x07) != 0x07)  && (_bDoubleCheck > 0)) {
				_bAdjBlkState0 = BLK_OFFSET_ADJ;
#if AUTO_BLANK_DEBUG
				pr_debug("go back to OFFSET adjust\r\n");
#endif
				vDrvBlankAdjParaReset();
				_bBlankIteration = (u8)1;
				_bDoubleCheck = _bDoubleCheck - (u8)1;
			} else {
				pr_debug("Blank level adjust %u times\r\n",
				(ADJUST_BLANK_RETRY_COUNT - _bDoubleCheck + 1));
				pr_debug("After vDrvAdjustBlankLevel, Blank level (10 bit) :CH1=%3u, CH2=%3u, CH3=%3u\r\n",
				(unsigned int)_dwBlankAvg[0], (unsigned int)_dwBlankAvg[1],
				(unsigned int)_dwBlankAvg[2]);
				pr_debug("After vDrvAdjustBlankLevel, Maximum level (8b) :CH1=%u, CH2=%u, CH3=%u\r\n",
				(unsigned int)_dwMaxLevelAvg[0], (unsigned int)_dwMaxLevelAvg[1],
				(unsigned int)_dwMaxLevelAvg[2]);

				_fgblankfinish = (u8)1;
				_bAdjBlkState0 = BLK_NOTHING;
			}
		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		break;

	default:
		break;
	}
}


u8 vCust_Current_Cal_Status(void)
{
	return _fgblankfinish;
}

/*u8 bGainBackup[3];*/
/*u8 bOffsetBackup[3];*/
void vDrvSetPGAGainOffsetFromEEPROM(void)
{
	if (_bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
		for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
			vDrvOnChipAutoColorModeSet(GAIN_MODE, _bType);
			vDrvOnChipAutoColorModeSet(OFFSET_MODE, _bType);
			vDrvSetHDADCDigitalOffset(_bOrder, (u16)0);

			/*          bGainBackup[_bOrder]= _bAutoColorGain[_bType][_bOrder];*/
			/*          bOffsetBackup[_bOrder]= _bAutoColorOffset[_bType][_bOrder];*/
		}
	}
}

void vDrvOnChipAutoColor_CheckGainOffset(u8 bMode)
{
	u8 bTolenence;
	u16 wAdcBlankLevel[3];

	if (bMode == GAIN_MODE) {
		for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
			wAdcBlankLevel[_bOrder] = _wData_v1v2[_bOrder];
		}
	}

	if ((_bType <= INT_VGA_COMPOENT) && (bMode <= DIGITAL_OFFSET_MODE)) { /*for klocwork check*/
		if (bMode == OFFSET_MODE) {                     /*MC20080417_1 ****/
			bTolenence = (_bAutoColorTimeOutCNT > (u8)6) ?  0x10 : 0;
		} else {
			bTolenence = (_bAutoColorTimeOutCNT > (u8)6) ?  1 : 0;
		}

		for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
			vDrvOnChipAutoColorModeSet(bMode, _bType); /* target, gain, offset*/

			if (bMode == OFFSET_MODE) {
				vDrvSetHDADCDigitalOffset(_bOrder, _bInitOffset);

			}

			vUtDelay1ms((u32)2);
			vDrvOnChipAutoColorGetADCData(bMode);
		}

#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)
		pr_debug("--- Check Gain/ Offset --\r\n");
#endif

		for (_bOrder = (u8)0;  _bOrder < (u8)3; _bOrder++) {
			/*Printf("ch1=%x, ch2=%x, ch3=%x\r\n",_wData_v1v2[0] ,_wData_v1v2[1], _wData_v1v2[2] );*/
			if (bMode == GAIN_MODE) {
				if ((wAdcBlankLevel[_bOrder] <= _wBlank[_bOrder] + 2)
				&& (wAdcBlankLevel[_bOrder] >= _wBlank[_bOrder] - 2)) {
					wAdcBlankLevel[_bOrder] = _wBlank[_bOrder];
				}

				_wData_v1v2[_bOrder] = (((u16)(_wData_v1v2[_bOrder] << 5)) - (wAdcBlankLevel[_bOrder]));
				/*gain-blank  13bit*/
			}


			if (bMode == GAIN_MODE) {
				if ((_wData_v1v2[_bOrder] >= (((u16)(_wADCTarget[_bOrder] << 5)) -
				(bOnChipCheckTolerance[bMode][1] + bTolenence))) &&
				(_wData_v1v2[_bOrder] <= (((u16)(_wADCTarget[_bOrder] << 5)) +
				(bOnChipCheckTolerance[bMode][0] + bTolenence)))) {
					_bGainOffset_channel |= (0x01U << _bOrder);
					_wData_v1v2[_bOrder] = _wData_v1v2[_bOrder] >> 5;

					if (_bType == INT_HDTV) {
						_bHDTVMax[_bOrder] = _wData_v1v2[_bOrder];
					} else if (_bType == INT_VGA) {
						_bVGAMax[_bOrder] = _wData_v1v2[_bOrder];
					} else {
						pr_debug("_bType not right\n");
					}

#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)

					if (bMode == OFFSET_MODE)  {
						pr_debug("channel %u is OK, Blank=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]) >> 5);
					} else {
						pr_debug("channel %u is OK, MaxLevel=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]));
					}

#endif
				} else {
					_bGainOffset_channel &= (~(0x01U << _bOrder));
#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)

					if (bMode == OFFSET_MODE)   {
						pr_debug("channel %u need cali, Blank=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]) >> 5);
					} else                     {
						pr_debug("channel %u need cali, MaxLevel=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]));
					}

#endif
				}

			} else {
				if ((_wData_v1v2[_bOrder] >= ((_wADCTarget[_bOrder]) -
				(bOnChipCheckTolerance[bMode][1] + bTolenence))) &&
				(_wData_v1v2[_bOrder] <= ((_wADCTarget[_bOrder])
				+ (bOnChipCheckTolerance[bMode][0] + bTolenence)))) {
					_bGainOffset_channel |= (0x01U << _bOrder);
#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)

					if (bMode == OFFSET_MODE)  {
						pr_debug("channel %u is OK, Blank=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]) >> 5);
					} else {
						pr_debug("channel %u is OK, MaxLevel=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]));
					}

#endif
				} else {
					_bGainOffset_channel &= (~(0x01U << _bOrder));
#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)

					if (bMode == OFFSET_MODE)   {
						pr_debug("channel %u need cali, Blank=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]) >> 5);
					} else                     {
						pr_debug("channel %u need cali, MaxLevel=%u\r\n",
						(_bOrder), (_wData_v1v2[_bOrder]));
					}

#endif
				}
			}
		}
	}
}

#if CHK_ADC_LINEARITY

void vDrvSetMeasureWindow(u8 location)
{
	u8 bInput_timing;
	u16 wHor_total;
	u16 wBack_porch;
	u16 wActive_area;
	u16 wMeasure_Start_point;
	u16 wMeasure_End_point;
	/*yunjie mark to fix*/
	/*bInput_timing = bDrvVideoGetTiming(SV_VP_MAIN);*/
	bInput_timing = (u8)5;
	wHor_total = Get_VGAMODE_IHTOTAL(bInput_timing);
	wBack_porch = Get_VGAMODE_IPH_BP(bInput_timing);
	wActive_area = Get_VGAMODE_IPH_WID(bInput_timing) - 0x30U;
#if PGA_LINEARITY_DEBUG

	if (location == 0) {
		pr_debug("H total=%u,  BP=%u,  ACT=%u\r\n", wHor_total, wBack_porch, wActive_area);
	}

#endif
	wMeasure_Start_point = wBack_porch;
	/* wBack_porch + ((wActive_area>>3)*(7-location));   // 7/8, 6/8, 5/8, 4/8*/

	if (location > (TOTAL_POINT - (u8)1)) {
		location = TOTAL_POINT - (u8)1;
	}

	wMeasure_End_point = wHor_total - wBack_porch - (wActive_area * ((u32)((TOTAL_POINT - 1) -
	location)) / (u32)TOTAL_POINT); /*         wHor_total - wMeasure_Start_point -0x40;*/
	vDrvAsyncPreDataActive(wMeasure_Start_point, wMeasure_End_point);
#if PGA_LINEARITY_DEBUG
	pr_debug("pre-margin=%u,  post-margin=%u\r\n", wMeasure_Start_point, wMeasure_End_point);
#endif
}
#endif

void vCheckAutoColorGainRange(void)
{
	u8 bCaliResult;
	u16 wGain_High_limit, wGain_LOW_limit;
	u16 wTmpOffset, wTmpGain;

	if (_bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
		bCaliResult = 0;

		for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
			wTmpGain = bDrvGetHDADCGain_Digital(_bOrder);
			wTmpOffset = bDrvGetHDADCOffset_Digital(_bOrder);

			wGain_High_limit = GAIN_HIGH_LIMIT_100_DIGITAL[_bType][_bOrder];
			wGain_LOW_limit = GAIN_LOW_LIMIT_100_DIGITAL[_bType][_bOrder];

			if ((wTmpGain < wGain_High_limit) && (wTmpGain > wGain_LOW_limit)) {
				bCaliResult = bCaliResult | (0x01U << _bOrder);

				_bAutoColorGain[_bType][_bOrder] = wTmpGain;
				_bAutoColorOffset[_bType][_bOrder] = wTmpOffset;

			}

		}

#if  AUTO_COLOR_DEBUG
		pr_debug("The auto color result=%x\r\n", bCaliResult);
#endif
		pr_debug("==== Iteration count=%u  ===\r\n", _bAutoColorTimeOutCNT);

		if ((bCaliResult == (u8)0x07) && (_bAutoColorTimeOutCNT < (u8)AUTO_COLOR_MAX_TIME)) {
			/*_bInitDigSignBit = vReadSignBit(_bOrder);*/
			_bAutoColorHistory[_bType] = SV_TRUE;
			pr_debug("==== Auto color success  ===\r\n");
		} else {

			_bAutoColorHistory[_bType] = SV_FALSE;
			pr_debug("==== Auto color fail !!!!  ===\r\n");
		}

		/*UTIL_Printf("auto color history: HDTV=%x, VGA=%x, SCART=%x,VGACOMP=%x\r\n",*/
		/*_bAutoColorHistory[0],_bAutoColorHistory[1],_bAutoColorHistory[2],_bAutoColorHistory[3]);*/
	}
}


#if CHK_ADC_LINEARITY

void vDrvGetMaxValue16Times(void)
{
	for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) {
		vDrvOnChipAutoColorGetADCData(GAIN_MODE);
		_dwMaxLevelAvg[_bOrder] += _wData_v1v2[_bOrder] ;    /*8 bit resolution*/
	}

	/*UTIL_Printf("_bAutoColorTimeOutCNT=%u, _wData_v1v2=0x%x, dwMaxLevelAvg=0x%x\r\n",*/
	/*_bAutoColorTimeOutCNT  ,_wData_v1v2[0], _dwMaxLevelAvg[0] );*/
}

#endif
#if (CHK_ADC_LINEARITY || CHK_PGA_LINEARITY)

void vDrvClearMAXvariable(void)
{
	u8 bLoop;

	for (bLoop = (u8)0; bLoop < (u8)3; bLoop++) {
		_dwMaxLevelAvg[bLoop] = 0;
		_wData_v1v2[bLoop] = 0;
	}
}
#endif

void vDrvEnablePGALinearityVerify(void)
{
#if CHK_PGA_LINEARITY

	vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);  /*vDrvSetMeasureWindow(0xff);*/
	vDrvClearMAXvariable();  /* to clear _dwMaxLevelAvg[];*/
	/*_bPGALinearity = 0;*/
	_bPGA_LINEARITY_state = PGA_VERIFY_INIT;
#endif
}

void vDrvPGALinearityVerify(void)
{
#if CHK_PGA_LINEARITY

	if (bDrvOnChipAutoColorCheckSignalReady() == P_FA) {
		return;
	}

	switch (_bPGA_LINEARITY_state) {
	case PGA_VERIFY_INIT:
		vDrvHDTVSelBlankDataSource();              /*to stop HW auto blank adjust function*/
		vDrvResetAllGainOffset();                         /* to reset all of the gain and offset*/
		_bGain = 0;
		_bPGA_LINEARITY_state = PGA_VERIFY_2;
		break;

	case PGA_VERIFY_2:
		for (_bOrder = (u8)0; _bOrder < (u8)3; _bOrder++) { /* to set minimum gain*/
			_dwMaxLevelAvg[_bOrder] = 0;
		}

		_bAutoColorTimeOutCNT = (u8)4;
		_bPGA_LINEARITY_state = PGA_VERIFY_3;
		break;

	case PGA_VERIFY_3:
		_bAutoColorTimeOutCNT = _bAutoColorTimeOutCNT - (u8)1;

		if (_bAutoColorTimeOutCNT == 0) {
			_bPGA_LINEARITY_state = PGA_VERIFY_4;
		}

		break;

	case PGA_VERIFY_4:
		if (_bAutoColorTimeOutCNT < (u8)8) {
			_bAutoColorTimeOutCNT++;
			vDrvMeasureMaxBlankLevel();
		} else {
			vDrvCalculateMaxBLKAvg();
			pr_debug("Maximum level @ Gain= %3u :PGA_Max1= %3u ,PGA_Max2= %3u ,PGA_Max3= %3u\r\n",
			(unsigned int)_bGain, (unsigned int)_dwMaxLevelAvg[0], (unsigned int)_dwMaxLevelAvg[1],
			(unsigned int)_dwMaxLevelAvg[2]);
			pr_debug("Blank level   @ Gain= %3u :CH1_Blank= %3u ,CH2_Blank= %3u ,CH3_Blank= %3u\r\n",
			(unsigned int)_bGain, (unsigned int)_dwBlankAvg[0], (unsigned int)_dwBlankAvg[1],
			(unsigned int)_dwBlankAvg[2]);

			if (_bGain < (u16)0xff) {
				_bGain++;
				_bPGA_LINEARITY_state = PGA_VERIFY_2;
			} else {
				pr_debug("---To restore the gain and offset ---\r\n");
				vDrvSetPGAGainOffsetFromEEPROM();

				for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
					pr_debug("_bOrder=%x, Initial GAIN=0x%x, Initial OFFSET=0x%x\r\n",
					_bOrder, bDrvGetHDADCGain_Digital(_bOrder),
					bDrvGetHDADCOffset_Digital(_bOrder));

				}

				vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);

				_bPGA_LINEARITY_state = PGA_VERIFY_NOTHING;
			}
		}

		break;
		
	default:
		break;
	}

#endif
}

void vDrvOnChipAutoColorIteration(void)
{

	u16 wTmp;
	u16 wTmp2;
	u16 UnitGainADCMaxLevel, UnitGainAdcBlankLevel;

	switch (_bAutoColorState0) {
	case VDO_AUTO_COLOR_NOT_BEGIN:
#if CHK_PGA_LINEARITY
		if (_bPGA_LINEARITY_state != PGA_VERIFY_NOTHING) {
			vDrvPGALinearityVerify();
		}

#endif

#if CHK_ADC_LINEARITY

		if (_bADC_LINEARITY_state != ADC_VERIFY_0) {
			vDrvADCLinearityVerify();
		}

#endif
		break;

	case VDO_AUTO_COLOR_START:
		_bType = bDrvOnChipAutoColorCheckSignalReady();

		if ((_bType <= INT_VGA_COMPOENT) && (_bType != P_FA)) { /*for klocwork check*/
			_bOrgType = _bType;
			vDrvHDTVSelBlankDataSource();
			_bAutoColorState0 = VDO_AUTO_COLOR_1P0_START;  /**/
		} else {
			_bAutoColorTimeOutCNT++;

			if (_bAutoColorTimeOutCNT > (u8)0xf0) {
				/*yunjie mark to fix*/
				/*vApiNotifyAutoColorDone(((fgIsMainVga() || fgIsMainYPbPr())?0:1),FALSE);*/
				_bAutoColorState0 = VDO_AUTO_COLOR_NOT_BEGIN;
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_DO_NOTHING;

				vDrvPreSetToReadBlank((u8)AS_BLANK_RESET);
				/*//yunjie mark to fix*/
				vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);  /*vDrvSetMeasureWindow(0xff);*/
				vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);
			}
		}

		break;

	case VDO_AUTO_COLOR_1P0_START:  /*to set gain and offset for the corresponding signal type*/
		vDrvSetPGAGainOffsetFromEEPROM();
		vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);  /*vDrvSetMeasureWindow(0xff);*/
		_bOffsetGainDone = 0;
		_bAutoColorTimeOutCNT = 0;
		_bAutoColorState0 = VDO_AUTO_COLOR_1_START;

		for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
			bDrvSetHDADCGain_Digital(_bOrder, (u16)0x8000);
			vDrvSetHDADCDigitalOffset(_bOrder, (u16)0xFF);
			wTmp = bDrvGetHDADCOffset_Digital(_bOrder);
			wTmp2 = bDrvGetHDADCGain_Digital(_bOrder);
			pr_debug("== _bOrder=%x, Initial GAIN=0x%x, OFFSET=0x%x ====\r\n", _bOrder, wTmp2, wTmp);
		}



		break;

	case VDO_AUTO_COLOR_1_START:  /*to wait for gain and offset stable*/
		if (fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) {

			for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
				vIO32WriteFldAlign(HDFE_03, 512U, AD1_GAIN_BIAS);   /*5881 fix 512*/
				vIO32WriteFldAlign(HDFE_03, 512U, AD2_GAIN_BIAS);   /*5881 fix 512*/
				vIO32WriteFldAlign(HDFE_03, 512U, AD3_GAIN_BIAS);   /*5881 fix 512*/
				_wADCTarget[_bOrder]  = bDrvGetColorTargetValue(_bType, _bOrder);
				/*bColorMaxTypeNew[bType][_bOrder];*/
				_wBlank[_bOrder] = wColorBlankValueNew[_bType][_bOrder];
				/*bDrvSetHDADCGain_Digital(_bOrder,0x8000);*/
				/*  vDrvSetHDADCDigitalOffset(_bOrder,0);*/
				vUtDelay1ms(50U);
				vDrvOnChipAutoColorGetADCData(GAIN_MODE);
				/*UnitGainADCMaxLevel= bDrvOnChipGetADCMaxMinValue(_bOrder,AS_PHASE_MAX_SEL);*/
				/*_wData_v1v2 [_bOrder];*/
				UnitGainADCMaxLevel = _wData_v1v2[_bOrder];
				vDrvOnChipAutoColorGetADCData(OFFSET_MODE);
				UnitGainAdcBlankLevel = _wData_v1v2[_bOrder];
				/*UnitGainAdcBlankLevel=bDrvGetBlankVal(_bOrder);//_wData_v1v2 [_bOrder];*/
				wTmp = bDrvGetHDADCGain_Digital(_bOrder);
				wTmp2 = bDrvGetHDADCOffset_Digital(_bOrder);
				_bInitGain = (u32)(((((u16)(_wADCTarget[_bOrder] << 2U))
				- (_wBlank[_bOrder] >> 3U)) * 16 * 2048)
				/ (u32)(((u16)(UnitGainADCMaxLevel << 2U)) - ((u16)(UnitGainAdcBlankLevel >> 3U))));
				bDrvSetHDADCGain_Digital(_bOrder, _bInitGain);
				/*pr_debug("initgain=0x%x, UnitGainADCMaxLevel=0x%x,UnitGainAdcBlankLevel=0x%x,
				_wADCTarget[_bOrder]=0x%x,_wBlank[_bOrder]=0x%x,gain=0x%x,offset=0x%x\r\n",
				_bInitGain, UnitGainADCMaxLevel, (UnitGainAdcBlankLevel >> 3),
				(_wADCTarget[_bOrder] << 2), (_wBlank[_bOrder] >> 3), wTmp,
				wTmp2);*/
			}

			_bCaliMode = OFFSET_MODE;

			vClrSP0Flg(SP0_AUTOCOLOR_FLG);
			_bAutoColorState0 = VDO_AUTO_COLOR_2_START;
		}

		break;

	case VDO_AUTO_COLOR_2_START:
		if (((_bOffsetGainDone & 0x03U) == 0x03U) || (_bAutoColorTimeOutCNT > AUTO_COLOR_MAX_TIME)) {
			/*UTIL_Printf("--- _bOffsetGainDone=%x, bAutoColorTimeOutCNT=%x\r\n",*/
			/*_bOffsetGainDone,_bAutoColorTimeOutCNT);*/
			_bAutoColorState0 = VDO_AUTO_COLOR_4_START;
		} else {
			if (fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) {
				vClrSP0Flg(SP0_AUTOCOLOR_FLG);
				_bGainOffset_channel = (u8)0x07;
				/*vIO32WriteFldAlign(HDTV_03, AS_BLANK_ALWAYS, HDTV_BLAK_SET);*/
				vDrvPreSetToReadBlank((u8)AS_BLANK_ALWAYS);
				vDrvOnChipAutoColor_CheckGainOffset(_bCaliMode);

				if ((_bGainOffset_channel & 0x07U) != 0x07U) {
					_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_INITIAL_STATE;
					_bAutoColorTimeOutCNT++;
					_bOffsetGainDone = 0;
				} else {
					_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_END;
					_bOffsetGainDone = (_bCaliMode == OFFSET_MODE) ?
					(_bOffsetGainDone | 0x01U) : (_bOffsetGainDone | 0x02U);
				}

				_bAutoColorState0 = VDO_AUTO_COLOR_3_START;
			}
		}

		break;

	case VDO_AUTO_COLOR_3_START:
		if (_OnChipAutoColorState != ON_CHIP_AUTO_COLOR_END) {
			vDrvOnChipAutoColor_GainOffset(_bCaliMode);

			if (bDvMeasureDataSel() == (u8)1) {
				_bAutoColorState0 = VDO_AUTO_COLOR_3_START_Delay1;
			}
		} else {
			_bCaliMode = (_bCaliMode == GAIN_MODE)  ? OFFSET_MODE : GAIN_MODE;
			_bAutoColorState0 = VDO_AUTO_COLOR_2_START;
		}

		break;

	case VDO_AUTO_COLOR_3_START_Delay1:
		if (fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) {
			vClrSP0Flg(SP0_AUTOCOLOR_FLG);
			_bAutoColorState0 = VDO_AUTO_COLOR_3_START;
		}

		break;

	case VDO_AUTO_COLOR_4_START:
		if (fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) {
			/*--------- to show the PGA offset --------------*/
			for (_bOrder = 0;  _bOrder < (u8)3; _bOrder++) {

				_wData_v1v2[_bOrder] = bDrvGetHDADCOffset_Digital(_bOrder);
			}

			if (_bType == INT_HDTV) {
				pr_debug("HDTV: Offset_1=0x%x, Offset_2=0x%x, Offset_3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_VGA) {
				pr_debug("VGA: Offset_1=0x%x, Offset_2=0x%x, Offset_3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_SCART) {
				pr_debug("SCART: Offset_1=0x%x, Offset_2=0x%x, Offset_3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			}

			else if (_bType == INT_VGA_COMPOENT) {
				pr_debug("VGA_COMPOENT: Offset_1=0x%x, Offset_2=0x%x, Offset_3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else {
				pr_debug("_bType not right\n");
			}

			/*--------- to show the PGA gain --------------*/
			for (_bOrder = 0;  _bOrder < (u8)3; _bOrder++) {
				_wData_v1v2[_bOrder] = bDrvGetHDADCGain_Digital(_bOrder);

			}

			if (_bType == INT_HDTV) {
				pr_debug("HDTV: Gain1=0x%x, Gain2=0x%x, Gain3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_VGA) {
				pr_debug("VGA: Gain1=0x%x, Gain2=0x%x, Gain3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_SCART) {
				pr_debug("SCART: Gain1=0x%x, Gain2=0x%x, Gain3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_VGA_COMPOENT) {
				pr_debug("VGA_COMPOENT: Gain1=0x%x, Gain2=0x%x, Gain3=0x%x\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else {
				pr_debug("_bType not right\n");
			}

			/*--------- to show the blank level --------------*/
			vDrvPreSetToReadBlank(AS_BLANK_ALWAYS);

			for (_bOrder = 0;  _bOrder < (u8)3; _bOrder++) {
				vDrvOnChipAutoColorGetADCData(OFFSET_MODE);
			}

			if (_bType == INT_HDTV) {
				pr_debug("HDTV: 12 bits base: Blank1=0x%3x, Blank2=0x%3x, Blank3=0x%3x\r\n",
				(_wData_v1v2[0] >> 1), (_wData_v1v2[1] >> 1), (_wData_v1v2[2] >> 1));
			} else if (_bType == INT_VGA) {
				pr_debug("VGA: 12 bits base: Blank1=0x%3x, Blank2=0x%3x, Blank3=0x%3x\r\n",
				(_wData_v1v2[0] >> 1), (_wData_v1v2[1] >> 1), (_wData_v1v2[2] >> 1));
			} else if (_bType == INT_SCART) {
				pr_debug("SCART: 12 bits base: Blank1=0x%3x, Blank2=0x%3x, Blank3=0x%3x\r\n",
				(_wData_v1v2[0] >> 1), (_wData_v1v2[1] >> 1), (_wData_v1v2[2] >> 1));
			} else if (_bType == INT_VGA_COMPOENT) {
				pr_debug("VGA_COMPOENT: 12 bits base: Blank1=0x%3x, Blank2=0x%3x, Blank3=0x%3x\r\n",
				(_wData_v1v2[0] >> 1), (_wData_v1v2[1] >> 1), (_wData_v1v2[2] >> 1));
			} else {
				pr_debug("_bType not right\n");
			}

			/*--------- to show the maximum level --------------*/
			vDrvOnChipGetADCMaxValue();

			if (_bType == INT_HDTV) {
				pr_debug("HDTV: Max1=%3u, Max2=%3u, Max3=%3u\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_VGA) {
				pr_debug("VGA: Max1=%3u, Max2=%3u, Max3=%3u\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_SCART) {
				pr_debug("SCART: Max1=%3u, Max2=%3u, Max3=%3u\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else if (_bType == INT_VGA_COMPOENT) {
				pr_debug("VGA_COMPOENT: Max1=%3u, Max2=%3u, Max3=%3u\r\n",
				_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
			} else {
				pr_debug("_bType not right\n");
			}

			vClrSP0Flg(SP0_AUTOCOLOR_FLG);

			if (_bType == _bOrgType) {
				_bAutoColorState0 = VDO_AUTO_COLOR_END;
			} else {
				vCheckAutoColorGainRange();
				_bType = _bOrgType;
				_bAutoColorState0 = VDO_AUTO_COLOR_1P0_START;
			}
		}

		break;

	case VDO_AUTO_COLOR_END:
		/*_bAutoColorBlankMeasure = 0;*/
		_bAutoColorState0 = VDO_AUTO_COLOR_NOT_BEGIN;
		_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_DO_NOTHING;

		vDrvPreSetToReadBlank((u8)AS_BLANK_RESET);
		vCheckAutoColorGainRange();
		/*yunjie mark to fix*/
		vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);  /*vDrvSetMeasureWindow(0xff);*/

		vDrvHDTV_HW_AUTO_ONOFF(CALI_ENABLE, _bType);

		/*yunjie mark to fix*/
		vDrvYPbPrAutoStart();
		break;
		
	default:
		break;
	}
}



/*=============================================================================================================*/
/*Description : This subroutine can support 3 kinds calibration mode by setting the parameter bMode.*/
/**/
/*Parameter   : bMode =0 ; do gain calibration. This mode uses on chip voltage to do gain calibration.*/
/*            : bMode =1 ; do offset calibration. This mode will calibrate the offset base on the input signal.*/
/*            : bMode =2 ; do gain calibration. This mode uses on chip voltage to do gain calibration.*/
/**/
/*return      : none.*/
/*==============================================================================================================*/

void vDrvOnChipAutoColor_GainOffset(u8 bMode)
{
	u8 bIndexCnt, bCaliNG; /*, bTmpCnt, bTmpCNT2;*/

#define CALISTEP                0x400 /*0x08*0x80*/

#define INITGAIN                0x50
#define INITOFFSET              0x80

	if ((bMode == ON_CHIP_GAIN_MODE) || (bMode == ON_CHIP_OFFSET_MODE)) {
		;
	} else {
		if (!fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) {
			return;
		}

		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
	}

	if ((_bType <= INT_VGA_COMPOENT) && (bMode <= DIGITAL_OFFSET_MODE)) { /*for klocwork check*/
		switch (_OnChipAutoColorState) {
		case ON_CHIP_AUTO_COLOR_DO_NOTHING:
			break;

		case ON_CHIP_AUTO_COLOR_INITIAL_STATE:

			/*UTIL_Printf("--------- ON_CHIP_AUTO_COLOR_INITIAL_STATE, mode=%x----\r\n",bMode);*/
			if ((bMode == ON_CHIP_GAIN_MODE) || (bMode == ON_CHIP_OFFSET_MODE)) {
				_bType = INT_VGA;
			} else if ((_bAutoColorState0 == VDO_AUTO_COLOR_NOT_BEGIN) && (bMode == OFFSET_MODE)) {
				_bType = bDrvOnChipAutoColorCheckSignalReady();

				if (_bType == P_FA) {
					break;
				}
			} else {
				pr_debug("_bType not right\n");
			}

			if ((_bGainOffset_channel & 0x07U) == 0x07U) {
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_FINISH;
			}
			if (_bType <= INT_VGA_COMPOENT) { /*for klocwork check*/
				for (bIndexCnt = 0; bIndexCnt < (u8)3; bIndexCnt++) {
					if (!(_bGainOffset_channel & (0x01U << bIndexCnt))) {
						_bOrder = bIndexCnt;
						vDrvOnChipAutoColorModeSet(bMode, _bType);

						vUtDelay1ms(5U);

					if (_bCaliMode == OFFSET_MODE) {
							_bCaliStep[_bOrder]       = 0x08;/*notice*/
					} else {
							_bCaliStep[_bOrder]       = CALISTEP;
					}


						_bDoubleChkFlag[_bOrder] = 0;
						_bGainIncDir[_bOrder]     = 0xff;
						_bChgDirToggle[_bOrder]  = 0;
						_wDiff_mim[_bOrder]      = 0xffff;
					}
				}
			}


				_bTotalCnt       = 0;
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_SEARCH_TARGET;
				/*
				UTIL_Printf("Print the register value");
				u32 dWData;
				for(bIndexCnt=0;bIndexCnt<14;bIndexCnt++)
				{
				  dWData = u4IO32Read4B(REG_VGA_Normal_CFG0+(4*bIndexCnt));
				  UTIL_Printf("Reg address=0x%x value=0x%x\r\n",
				  REG_VGA_Normal_CFG0+(4*bIndexCnt),dWData);
				}
				*/

			break;

		case ON_CHIP_AUTO_COLOR_SEARCH_TARGET:  /*get ADC raw data*/
			/* UTIL_Printf("--------- ON_CHIP_AUTO_COLOR_SEARCH_TARGET ----\r\n");*/
			_bTotalCnt++;

			for (bIndexCnt = 0; bIndexCnt < (u8)3; bIndexCnt++) {
				if (!(_bGainOffset_channel & (0x01U << bIndexCnt))) {
					_bOrder = bIndexCnt;
					vDrvOnChipAutoColorGetADCData(bMode);

					if ((bMode == OFFSET_MODE) || (bMode == ON_CHIP_OFFSET_MODE)) {

						_bGain = bDrvGetHDADCOffset_Digital(_bOrder);

#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)
						pr_debug("order=%2u, offset=%x, step=%x, toggle=%u, Data=0x%x\r\n",
						_bOrder, _bGain, _bCaliStep[_bOrder], _bChgDirToggle[_bOrder],
						_wData_v1v2[_bOrder]);
#endif


						vDrvOnChipAutoColorSearchTarget_V2(bMode);
						vDrvSetHDADCDigitalOffset(_bOrder, _bGain);

					} else {

						_bGain = bDrvGetHDADCGain_Digital(_bOrder);
#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)
						pr_debug("order=%2u, Gain=%x, step=%x, toggle=%u, Data=0x%x\r\n",
						_bOrder, _bGain, _bCaliStep[_bOrder], _bChgDirToggle[_bOrder],
						_wData_v1v2[_bOrder]);
#endif
						vDrvOnChipAutoColorSearchTarget_V2(bMode);
						bDrvSetHDADCGain_Digital(_bOrder, _bGain);

					}
				}
			}

			if ((_bGainOffset_channel & 0x07U) == 0x07U) {
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_CHECK_VALUE;
			} else if ((bMode == GAIN_MODE) || (bMode == OFFSET_MODE)) {
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_WAIT_VALUE;
			} else {
				pr_debug("bMode not right\n");
			}

			break;

		case ON_CHIP_AUTO_COLOR_WAIT_VALUE:
			_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_SEARCH_TARGET;
			break;

		case ON_CHIP_AUTO_COLOR_CHECK_VALUE:
			bCaliNG = 0;

			for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
				if ((_wData_v1v2[_bOrder] >= (_wADCTarget[_bOrder]
				- bOnChipCalibrateTolerance[bMode][3]))
				&& (_wData_v1v2[_bOrder] <= (_wADCTarget[_bOrder]
				+ bOnChipCalibrateTolerance[bMode][2]))) {
					if ((bMode == OFFSET_MODE) || (bMode == ON_CHIP_OFFSET_MODE)) {

						_bGain = bDrvGetHDADCOffset_Digital(_bOrder);

						_bAutoColorOffset[_bType][_bOrder] = _bGain;

#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)
						pr_debug("Channel %u Offset success. offset=%x, ADC=%u, target=%u\r\n",
						_bOrder, _bGain, _wData_v1v2[_bOrder], _wADCTarget[_bOrder]);
#endif
					} else {


						_bGain = bDrvGetHDADCGain_Digital(_bOrder);

					if (bMode == ON_CHIP_GAIN_MODE) {
						_bReadMAXLevel = (u8)1;
					}

						_bAutoColorGain[_bType][_bOrder] = _bGain;

#if (AUTO_COLOR_DEBUG || ON_CHIP_AUTO_COLOR_DEBUG)
						pr_debug("Channel %u Auto Gain success. Gain=%x, ADC=%u, target=%u\r\n",
						_bOrder, _bGain, _wData_v1v2[_bOrder], _wADCTarget[_bOrder]);
#endif
					}
				} else {
					bCaliNG = (u8)1;
#if AUTO_COLOR_DEBUG
					pr_debug("ON_CHIP_AUTO_COLOR fail at mode=%u\r\n", bMode);
#endif
				}
			}

			if (bCaliNG == 0) {
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_FINISH;
			} else {
				if (bMode == ON_CHIP_GAIN_MODE) {
					vDrvOnChipModeOnOff(0); /*turn off "on chip auto color" mode*/
				}

				/*vIO32WriteFldAlign(HDTV_03, AS_BLANK_RESET, HDTV_BLAK_SET);*/
				vDrvPreSetToReadBlank((u8)AS_BLANK_RESET);
				_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_END;
			}

			break;

		case ON_CHIP_AUTO_COLOR_FINISH:
#if ON_CHIP_AUTO_COLOR_DEBUG
			if (bMode == OFFSET_MODE) {
				if (_bReadMAXLevel == 1) {
					_bReadMAXLevel = 0;
					vDrvOnChipGetADCMaxValue();
					pr_debug("Before auto color Max1=%3u, Max2=%3u, Max3=%3u\r\n",
					_wData_v1v2[0], _wData_v1v2[1], _wData_v1v2[2]);
				}
			} else
#endif
			{
				if (bMode == ON_CHIP_GAIN_MODE) {
					vDrvOnChipModeOnOff(0); /*turn off "on chip auto color" mode*/

					vDrvOnChipGainMapping(_bType);
#if ON_CHIP_AUTO_COLOR_DEBUG
					vDrvCheckGain();
#endif
				}
			}

			/*vIO32WriteFldAlign(HDTV_03, AS_BLANK_RESET, HDTV_BLAK_SET);*/
			vDrvPreSetToReadBlank((u8)AS_BLANK_RESET);
			_OnChipAutoColorState = ON_CHIP_AUTO_COLOR_END;
			break;

		case ON_CHIP_AUTO_COLOR_END:
		default:
			break;
		}
	}
}

void vDrvLinClampOnOff(u8 bOnOff)
{
	if (bOnOff == 0) {
		vIO32WriteFldAlign(HDTV_00, CALI_DISABLE, HDTV_LCLAMP_EN);          /*Disable line clamp*/
		vIO32WriteFldAlign(HDTV_05, CALI_DISABLE, HDTV_BLANK_EF_EN);          /*Disable line clamp*/
	} else {
		vIO32WriteFldAlign(HDTV_00, CALI_ENABLE, HDTV_LCLAMP_EN);          /*Disable line clamp*/
		vIO32WriteFldAlign(HDTV_05, CALI_ENABLE, HDTV_BLANK_EF_EN);          /*Disable line clamp*/
	}
}

void vDrvEnableADCLinearityVerify(u8 bType)
{
#if CHK_ADC_LINEARITY

	vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);  /*vDrvSetMeasureWindow(0xff);*/
	vDrvClearMAXvariable();  /* to clear _dwMaxLevelAvg[];*/
	_bLocation = 0;
	_bADCLinearity = 0;
	_bADC_LINEARITY_state = ADC_VERIFY_3;
	TOTAL_POINT = bType;
#endif
}

void vDrvADCLinearityVerify(void)
{
#if CHK_ADC_LINEARITY

	if ((!fgIsSP0FlgSet(SP0_AUTOCOLOR_FLG)) || (!(_bAutoColorState0 == VDO_AUTO_COLOR_NOT_BEGIN))) {
		return;
	}

	_bType = bDrvOnChipAutoColorCheckSignalReady();

if (_bType == INT_VGA) {
	switch (_bADC_LINEARITY_state) {
	case ADC_VERIFY_0:
		break;

	case ADC_VERIFY_1:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);

		if (_bAutoColorTimeOutCNT < (u8)0x10) {
			vDrvGetMaxValue16Times();
			/*UTIL_Printf("TimeOutCNT=%u, _wData1=0x%x, _wData2=0x%x, _wData3=0x%x\r\n",*/
			/*_bAutoColorTimeOutCNT  ,_wData_v1v2[0],_wData_v1v2[1],_wData_v1v2[2]);*/
			_bAutoColorTimeOutCNT++;
		} else {
			_bLocation = 0;
			_dwAutoColorMax[0] = _dwMaxLevelAvg[0];
			_dwAutoColorMax[1] = _dwMaxLevelAvg[1];
			_dwAutoColorMax[2] = _dwMaxLevelAvg[2];
			_bADC_LINEARITY_state = ADC_VERIFY_2;
			_bADCLinearity = (u8)1;
			pr_debug("---- The top level of 3 CH after auto color is: %d %d %d\r\n",
			(int)_dwAutoColorMax[0], (int)_dwAutoColorMax[1], (int)_dwAutoColorMax[2]);
		}

		break;

	case ADC_VERIFY_2:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		vDrvClearMAXvariable();  /* to clear _dwMaxLevelAvg[];*/
		vDrvSetMeasureWindow(_bLocation);  /* to set measure window*/
		_bADC_LINEARITY_state = ADC_VERIFY_3;
		break;

	case ADC_VERIFY_3:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);
		vDrvGetMaxValue16Times();  /* to do dumy read*/
		vDrvClearMAXvariable();  /* to clear _dwMaxLevelAvg[];*/
		_bAutoColorTimeOutCNT = 0;

		if (_bADCLinearity == 0) {
			_bADC_LINEARITY_state = ADC_VERIFY_1;
		} else if (_bADCLinearity == (u8)1) {
			_bADC_LINEARITY_state = ADC_VERIFY_4;
		} else if ((_bADCLinearity == (u8)2)) {
			_bADC_LINEARITY_state = ADC_VERIFY_5;
		} else {
			_bADC_LINEARITY_state = ADC_VERIFY_6;
		}

		break;

	case ADC_VERIFY_4:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);

		if (_bAutoColorTimeOutCNT < (u8)0x10) {
			vDrvGetMaxValue16Times();
			_bAutoColorTimeOutCNT++;
			 break;
		}
		pr_debug("The level of 3 CH at location %2d are CH1: %4d ,CH2: %4d ,CH3: %4d\r\n",
		(int)_bLocation, (int)_dwMaxLevelAvg[0], (int)_dwMaxLevelAvg[1], (int)_dwMaxLevelAvg[2]);
		_dwMaxDiffTemp = 0;

		for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
			_dwMaxLevelAvg[_bOrder] = _dwAutoColorMax[_bOrder] - _dwMaxLevelAvg[_bOrder];
			/* to get the difference between MAX and local max*/

			if (_dwMaxDiffTemp < _dwMaxLevelAvg[_bOrder]) {
				_dwMaxDiffTemp = _dwMaxLevelAvg[_bOrder];
				/* to get the max difference between MAX and local max*/
			}
		}

		_dwMaxDiffLocal = 0;

		for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
			_dwMaxLevelAvg[_bOrder] = _dwMaxDiffTemp - _dwMaxLevelAvg[_bOrder];
			/*to calculate the difference between channel and channel*/

			if (_dwMaxDiffLocal < _dwMaxLevelAvg[_bOrder]) {
				_dwMaxDiffLocal = _dwMaxLevelAvg[_bOrder];
				/* to get the max difference between channel and channel*/
			}
		}

		if (_bLocation < (u8)128) { /*for klocwork check*/
			_dwMaxDiff[_bLocation] = _dwMaxDiffLocal;
			_bLocation = _bLocation + (u8)1;
		}

		if (_bLocation < TOTAL_POINT) {
			_bADC_LINEARITY_state = ADC_VERIFY_2;
		} else {
			/*vDrvAsyncPreDataActive(10,10);*/
			_dwMaxDiffLocal = 0;
			pr_debug("The channel to channel different at position1,2,3,4... are \r\n");

			if (_bLocation < (u8)128) { /*for klocwork check*/
				for (_bOrder = 0; _bOrder < TOTAL_POINT; _bOrder++) {
					if (_dwMaxDiffLocal < _dwMaxDiff[_bOrder]) {
						_dwMaxDiffLocal = _dwMaxDiff[_bOrder];
					}

				pr_debug("Position %2d = %3d\r\n", (int)_bOrder, (int)_dwMaxDiff[_bOrder]);
				}
			}

			pr_debug("The maximum  difference (12 bit) between channel to channel is %d\r\n",
				(int)_dwMaxDiffLocal);
			_bLocation = 0;
			_bADCLinearity = (u8)2;
			_bADC_LINEARITY_state = ADC_VERIFY_2;
		}

		break;

	case ADC_VERIFY_5:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);

		if (_bAutoColorTimeOutCNT < (u8)0x10) {
			vDrvGetMaxValue16Times();
			_bAutoColorTimeOutCNT++;
		} else {
			if (_bLocation == 0) {
				_dwStPoint[0] = _dwMaxLevelAvg[0];
				_dwStPoint[1] = _dwMaxLevelAvg[1];
				_dwStPoint[2] = _dwMaxLevelAvg[2];
#if PGA_LINEARITY_DEBUG
				pr_debug("Start Point= %u , %u , %u\r\n",
				_dwStPoint[0], _dwStPoint[1], _dwStPoint[2]);
#endif

				if (TOTAL_POINT <= (u8)16) {
					_EndPoint = TOTAL_POINT - (u8)2;
				} else if (TOTAL_POINT <= (u8)64) {
					_EndPoint = TOTAL_POINT - (u8)4;
				} else {
					_EndPoint = TOTAL_POINT - (u8)8;
				}

				_bLocation = _EndPoint; /*TOTAL_POINT-2;*/
				_bADC_LINEARITY_state = ADC_VERIFY_2;
			} else if (_bLocation == _EndPoint) { /*(TOTAL_POINT-2))*/
				for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
					_dwEndPoint[_bOrder] = _dwMaxLevelAvg[_bOrder];
					/*_dwDeltaValue[_bOrder] = ((_dwStPoint[_bOrder] - _dwEndPoint[_bOrder])*/
					/*+(TOTAL_POINT/2-1))/(TOTAL_POINT-2);*/
					_dwDeltaValue[_bOrder] = ((_dwStPoint[_bOrder] - _dwEndPoint[_bOrder])
					+ (_EndPoint / 2 - 1)) / (_EndPoint);

				}

#if PGA_LINEARITY_DEBUG
				pr_debug("End Point= %u , %u , %u\r\n", _dwEndPoint[0], _dwEndPoint[1], _dwEndPoint[2]);
#endif
				_bLocation = 0;  /*_bLocation -1;*/
				_bADCLinearity = (u8)3;
				_bADC_LINEARITY_state = ADC_VERIFY_2;
				} else {
					pr_debug("_bLocation not right\n");
				}
				
		}

		break;

	case ADC_VERIFY_6:
		vClrSP0Flg(SP0_AUTOCOLOR_FLG);

		if (_bAutoColorTimeOutCNT < (u8)0x10) {
			vDrvGetMaxValue16Times();
			_bAutoColorTimeOutCNT++;
		} else {
			for (_bOrder = 0; _bOrder < (u8)3; _bOrder++) {
				/*_dwMaxDiffTemp = _dwDeltaValue[_bOrder]*(TOTAL_POINT-2-_bLocation)+
				_dwEndPoint[_bOrder];*/
				_dwMaxDiffTemp = ((_dwStPoint[_bOrder] - _dwEndPoint[_bOrder])
				* (_EndPoint - _bLocation) / (_EndPoint)) + _dwEndPoint[_bOrder];
#if PGA_LINEARITY_DEBUG
				pr_debug("Channel= %u ,point %u  target level= %u ,Current level= %u\r\n",
				_bOrder, _bLocation, _dwMaxDiffTemp, _dwMaxLevelAvg[_bOrder]);
#endif

				if (_dwMaxDiffTemp > _dwMaxLevelAvg[_bOrder]) {
					_dwMaxDiff[_bOrder] = _dwMaxDiffTemp - _dwMaxLevelAvg[_bOrder];
				} else {
					_dwMaxDiff[_bOrder] = _dwMaxLevelAvg[_bOrder] - _dwMaxDiffTemp;
				}
			}

				pr_debug("The channel different at position %2u is %4u , %4u , %4u\r\n",
				(unsigned int)_bLocation, (unsigned int)_dwMaxDiff[0],
				(unsigned int)_dwMaxDiff[1], (unsigned int)_dwMaxDiff[2]);

				_bLocation = _bLocation + (u8)1;

				/*if(_bLocation < (TOTAL_POINT-1))*/
				if (_bLocation < (_EndPoint + (u8)1)) {
					_bADC_LINEARITY_state = ADC_VERIFY_2;
				} else {
					vDrvAsyncPreDataActive((u16)0x20, (u16)0x20);
					_bADC_LINEARITY_state = ADC_VERIFY_0;
				}
			}

			break;
			
		default:
			break;
		}
	} else {
		_bADC_LINEARITY_state = ADC_VERIFY_0;
	}

#endif

}


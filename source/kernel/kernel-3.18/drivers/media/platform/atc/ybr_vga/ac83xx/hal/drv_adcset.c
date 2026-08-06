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


#include "drv_adcset.h"
#include "ybr_vga_common.h"


u32 bINT_BST_DET;
u8 _bPGAGainOffsetHistory[4];
u8 _bEFUSE_AUTOCOLOR_READY = 0;

u8 _bcurrent_use_adc = (u8)0;

void vDrvADCDefaultSetting(void)
{
	vDrvHDTVADCDefaultSetting();
}

void vDrvADCOffsetCal(void)
{
	/*u8 bEEPROMDATA0,bLoopCnt;*/
	/*CVBS ADC Calibration*/

	/*vDrvCvbsAdcCal();*/

	vIO32WriteFldAlign(HDFE_00, 0xFFU, AD1_OFFSET);  /*set digital offset =1 for CH 1*/
	vIO32WriteFldAlign(HDFE_00, 0xFFU, AD2_OFFSET);   /*set digital offset =1 for CH 2*/
	vIO32WriteFldAlign(HDFE_00, 0xFFU, AD3_OFFSET);  /*set digital offset =1 for CH 3*/

}


/**
 * @brief power down VGA
 *
 * @param None
 */
void vDrvVGAPD(void)
{
	vIO32WriteFldAlign(PDWNC_VGACFG0, 0x1U, FLD_RG_VMUX_PWD);
	/*vIO32WriteFldAlign(PDWNC_VGACFG6, 0x1, FLD_RG_SOGY_ADC_PWD);*/
	vIO32WriteFldAlign(PDWNC_VGACFG0, 0x0U, FLD_RG_AUTOBIAS_EN);/*20100415, sc hwu suggestion*/
	/*vIO32WriteFldAlign(PDWNC_PDMISC, 0x1, FLD_VGA_SYNC_DIS);//20100415, chiahsien liu suggestion*/
}


/**
 * @brief power on VGA
 *
 * @param None
 */
void vDrvVGAPWON(void)
{

	vIO32WriteFldAlign(PDWNC_VGACFG0, 0x0U, FLD_RG_VMUX_PWD);
	/*vIO32WriteFldAlign(PDWNC_VGACFG6, 0x0, FLD_RG_SOGY_ADC_PWD);*/
	vIO32WriteFldAlign(PDWNC_VGACFG0, 0x1U, FLD_RG_AUTOBIAS_EN);/*20100415, sc hwu suggestion*/
	/*vIO32WriteFldAlign(PDWNC_PDMISC, 0x0, FLD_VGA_SYNC_DIS);//20100415, chiahsien liu suggestion*/
}

/**
 * @brief get Efuse Gain
 *
 * @param None
 */

u32 u4DrvGetEfuseGain(void)
{
	return IO32ReadFldAlign(YPBPR_VGA_EFUSE, YPBPR_VGA_EFUSE_GAIN);
}

void vDrvHDTVADCDefaultSetting(void)
{
	u8 bCh;
	u8 bTmpvalue[3] = {0};
	u8 bTmpResult;
	{
		bTmpResult = 0;

		for (bCh = (u8)0; bCh < (u8)3; bCh++) {
			if (((IO32ReadFldAlign(YPBPR_VGA_EFUSE, YPBPR_VGA_EFUSE_GAIN)) >> (u32)24) == 0U) {
				bTmpvalue[bCh] = ((IO32ReadFldAlign(YPBPR_VGA_EFUSE, YPBPR_VGA_EFUSE_GAIN))
				>> ((u8)8 * bCh));

			if ((bTmpvalue[bCh] >= (u8)0x80) && (bTmpvalue[bCh] <= (u8)0xEE)) {
				bTmpResult |= (u8)((u8)0x01 << bCh);
			}
		}

		}

		_bEFUSE_AUTOCOLOR_READY = ((bTmpResult & (u8)0x07) != (u8)0x07) ? 0 : 1;

		if (_bEFUSE_AUTOCOLOR_READY == (u8)1) {
			pr_info("=E Fuse Gain Get Success(%d,%d,%d)!\r\n", bTmpvalue[0], bTmpvalue[1], bTmpvalue[2]);
		} else {
			pr_info("==== E Fuse Gain is Empty !!!!  ===\r\n");
		}
	}

	vIO32WriteFldMulti(PDWNC_VGACFG4,
		P_Fld(8U, FLD_RG_SYNC1_VTL) | P_Fld(8U, FLD_RG_SYNC1_VTH),
		FLD_RG_SYNC1_VTL | FLD_RG_SYNC1_VTH);
	vIO32WriteFldMulti(PDWNC_VGACFG0,
		P_Fld(0U, FLD_RG_VMUX_PWD) | P_Fld(1U, FLD_RG_DESPIKE) | P_Fld(1U, FLD_RG_AUTOBIAS_EN),
		FLD_RG_VMUX_PWD | FLD_RG_DESPIKE | FLD_RG_AUTOBIAS_EN); /*Mt5365/95 kal checked*/
	vIO32WriteFldMulti(PDWNC_VGACFG1,
		P_Fld(0U, FLD_RG_SYNC_DESPK_EN) | P_Fld(0U, FLD_RG_DIG_TST_EN),
		FLD_RG_SYNC_DESPK_EN | FLD_RG_DIG_TST_EN); /*5360 Michael checked*/
	vIO32WriteFldMulti(PDWNC_VGACFG3,
		P_Fld(0U, FLD_RG_SOGY_SORS_PWD) | P_Fld(0U, FLD_RG_SOGY_SINK_PWD),
		FLD_RG_SOGY_SORS_PWD | FLD_RG_SOGY_SINK_PWD); /*power on*/
	vIO32WriteFldAlign(ASYNC_0E, 2U, AS_C_MAXMIN_FILTER);
	vIO32WriteFldMulti(REG_VGA_Normal_CFG0,
		P_Fld(0U, RG_VDC_N_EN) | P_Fld(0U, RG_SHORT_FEO) | P_Fld(0U, RG_ADCTEST_EN),
		RG_VDC_N_EN | RG_SHORT_FEO | RG_ADCTEST_EN);
	vIO32WriteFldMulti(REG_VGA_Normal_CFG8,
		P_Fld(0U, RG_CLKOSEL_3) | P_Fld(0U, RG_CLKOSEL_2) | P_Fld(1U, RG_CLKOSEL_1) | P_Fld(0, RG_RGB_REV),
		RG_CLKOSEL_3 | RG_CLKOSEL_2 | RG_CLKOSEL_1 | RG_RGB_REV);
	vIO32WriteFldMulti(REG_VGA_Normal_CFG4,
		P_Fld(2U, RG_VGAADC1_IGBIAS) | P_Fld(2U, RG_VGAADC2_IGBIAS) | P_Fld(2U, RG_VGAADC3_IGBIAS),
		RG_VGAADC1_IGBIAS | RG_VGAADC2_IGBIAS | RG_VGAADC3_IGBIAS);
	/*vUtDelay1ms(2);*/
	vIO32WriteFldAlign(PDWNC_VGACFG2, 0U, FLD_RG_OFFCUR);

	for (bCh = (u8)0; bCh < (u8)3; bCh++) {
		vIO32WriteFldAlign(REG_VGA_Normal_CFG1 + (u32)((u8)4 * bCh), 1U, RG_CLAMP_GATE_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG1 + (u32)((u8)4 * bCh), 0U, RG_FE_1_PWD);
		vIO32WriteFldMulti(REG_VGA_Normal_CFG1 + (u32)((u8)4 * bCh),
			P_Fld(1U, RG_CLAMP_GATE_1) | P_Fld(0U, RG_FE_1_PWD) | P_Fld(3U, RG_COPBIAS_1)
			| P_Fld(0U, RG_COP_1_PWD), RG_CLAMP_GATE_1 | RG_FE_1_PWD | RG_COPBIAS_1 | RG_COP_1_PWD);
		vIO32WriteFldMulti(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), P_Fld(1U, RG_VGAADC1_CKSEL)
				| P_Fld(0U, RG_VGAADC1_DIV_SEL)
				   | P_Fld(0U, RG_VGAADC1_CORE_PWD) | P_Fld(0U, RG_VGAADC1_CK_PWD)
				   | P_Fld(0U, RG_VGAADC1_PHSEL) | P_Fld(1U, RG_VGAADC1_DC_EN)
				   | P_Fld(0U, RG_VGAADC1_VSEL_EN) | P_Fld(0U, RG_VGAADC1_VSEL)
				   | P_Fld(1U, RG_VGAADC1_LSF_EN) | P_Fld(0U, RG_VGAADC1_REV0)
				   | P_Fld(255U, RG_VGAADC1_REV1),
				   RG_VGAADC1_CKSEL | RG_VGAADC1_DIV_SEL | RG_VGAADC1_CORE_PWD | RG_VGAADC1_CK_PWD
				   | RG_VGAADC1_PHSEL | RG_VGAADC1_DC_EN | RG_VGAADC1_VSEL_EN | RG_VGAADC1_VSEL
				   | RG_VGAADC1_LSF_EN | RG_VGAADC1_REV0 | RG_VGAADC1_REV1);

	}

	vIO32WriteFldMulti(REG_VGA_Normal_CFG4,
	P_Fld(0U, RG_VGAADC_MON_SEL) | P_Fld(0U, RG_VGAADC1_IO_PWD) | P_Fld(0U, RG_VGAADC2_IO_PWD)
	| P_Fld(0U, RG_VGAADC3_IO_PWD),
		RG_VGAADC_MON_SEL | RG_VGAADC1_IO_PWD | RG_VGAADC2_IO_PWD | RG_VGAADC3_IO_PWD);
	vIO32WriteFldMulti(REG_VGA_Normal_CFG0, P_Fld(0U, RG_VREFGEN4FE_PWD)
	| P_Fld(0U, RG_VDC_P_EN) | P_Fld(0U, RG_RESSEL),  RG_VREFGEN4FE_PWD | RG_VDC_P_EN | RG_RESSEL);
	vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 0x04U, RG_ADCVREFP);
	vIO32WriteFldMulti(REG_VGA_Normal_CFG8, P_Fld(1U, RG_CLKINV_EN) | P_Fld(1U, RG_RELATCH_EN),
		RG_CLKINV_EN | RG_RELATCH_EN);


#if 1
	/* Slove VGA ADC no data*/
	/*vIO32WriteFldAlign(REG_VGA_Normal_CFG3, 0, CLR);vUtDelay1ms(1);*/
	/*vIO32WriteFldAlign(REG_VGA_Normal_CFG3, 1, CLR);vUtDelay1ms(1);*/
	/*vIO32WriteFldAlign(REG_VGA_Normal_CFG3, 0, CLR);vUtDelay1ms(1);*/
	/* Slove VGA ADC no data*/
#endif


	/*   vIO32WriteFldAlign(HDFE_00, 0x07, CLAMP_MID_EN);*/

	vDrvClkInit(); /*init for adcpll clk*/

	for (bCh = (u8)0; bCh < (u8)4; bCh++) {
		/* to clear the control variables of loading PGA gain and offset initial values.*/
		_bPGAGainOffsetHistory[bCh] = (u8)0;
		_bOffsetCaliDone[bCh] = (u8)0;
	}

	/*UTIL_Printf("vDrvHDTVADCDefaultSetting end\r\n");*/
}
void vDrvEFUSEAutoColorGainSetToNextType(u8 bOrgType, u8 bNextType)
{
	u32 bOrder;
	u16 wDen, wNum;
	u32 dGain, bGain;

	for (bOrder = 0U; bOrder < 3U; bOrder++) {
		dGain = _bAutoColorGain[bOrgType][bOrder];
		wNum  = wOnChipColorGainTable_75[bOrgType][bOrder];
		wDen  = wOnChipColorGainTable_75[bNextType][bOrder];

		bGain = dGain * wDen / wNum;
		_bAutoColorGain[bNextType][bOrder] = bGain;
	}
}

void vSetPGAGainOffsetInitVal(u8 bType)
{
	u32 bCh, i, j;

	if (_bPGAGainOffsetHistory[bType] == 0) { /*This means the gain and offset were never given values.*/
		/*UTIL_Printf("To set the PGA gain and offset \r\n");*/
		_bPGAGainOffsetHistory[bType] = 1;

		if (_bEFUSE_AUTOCOLOR_READY == (u8)1) { /*kalcheng   eEFuseAutocolor*/
			_bAutoColorHistory[bType] = SV_TRUE;

			/*////////////////////////////  1. Read Offset From code table   ////////////////////*/
			for (bCh = 0U; bCh < 3U; bCh++) {
				_bAutoColorGain[bType][bCh] = bVGADefaultGainTABLE_100_DIGITAL[bType][bCh];
				_bAutoColorOffset[bType][bCh] = bVGADefaultOffsetTABLE_100_DIGITAL[bType][bCh];
			}

			/*////////////////////////////  2. Read Gain From E Fuse   ////////////////////////*/

			for (bCh = 0U; bCh < 3U; bCh++) {
				/*compel Read VGA E Fuse address 0xf0008674*/
			if ((IO32ReadFldAlign(YPBPR_VGA_EFUSE, YPBPR_VGA_EFUSE_GAIN) >> 24) == 0U) {
				_bAutoColorGain[1][bCh] = ((IO32ReadFldAlign(YPBPR_VGA_EFUSE,
				YPBPR_VGA_EFUSE_GAIN)) >> (8U * bCh)) << 8;
				/*<<8 change to 16bit //+bEFuseCompensation[bCh];*/
			}
			}

			/*///////////////////////////   3. Transfer to  YPbPr///////////////////////////////////*/
			{
				vDrvEFUSEAutoColorGainSetToNextType(INT_VGA, INT_HDTV);
				/*vDrvEFUSEAutoColorGainSetToNextType(INT_VGA,INT_SCART);*/
				/*vDrvEFUSEAutoColorGainSetToNextType(INT_VGA,INT_VGA_COMPOENT);*/
			}
			_bcurrent_use_adc = (u8)1; /*efuese*/

			/*////////////////////////////  4. add compensation gain /////////////////////////*/
			/*for CLI command n.vga.ebgain 0 or 1 read; kal add*/
			for (i = 0U; i < 4U; i++)
				for (j = 0U; j < 3U; j++) {
					_bAutoColorGain[i][j] = _bAutoColorGain[i][j] + bEFuseCompensation[i][j];
					_bAutoColorGain_for_EFuse[i][j] = _bAutoColorGain[i][j];
				}

			/*///////////////////////////   5. Check Embedded gain  /////////////////////////*/
			if ((IO32ReadFldAlign(YPBPR_VGA_EFUSE, YPBPR_VGA_EFUSE_GAIN)) == 0x00000000U) {
				/*yunjie mark to fix*/
				/*vDrvSetOSTGOutPattern(1);*/
				pr_info("==== E Fuse Auto color is (empty) !!!!  ===\r\n");
			}

		} else {
			for (bCh = 0U; bCh < 3U; bCh++) {

				_bAutoColorGain[bType][bCh] = bVGADefaultGainTABLE_100_DIGITAL[bType][bCh];
				_bAutoColorOffset[bType][bCh] = bVGADefaultOffsetTABLE_100_DIGITAL[bType][bCh];
			}

			_bcurrent_use_adc = (u8)3; /*Default*/
			_bAutoColorHistory[bType] = SV_FALSE;
		}
	} else {
		pr_info("The PGA gain and offset were given\r\n");
	}

	pr_debug("Type   =%d, autocolor ready=%d\r\n", bType, _bAutoColorHistory[bType]);
	pr_debug("Gain1  =0x%x ,Gain2  =0x%x ,Gain3  =0x%x \r\n",
		_bAutoColorGain[bType][0], _bAutoColorGain[bType][1], _bAutoColorGain[bType][2]);
	pr_debug("Offset1=0x%x ,Offset2=0x%x ,Offset3=0x%x \r\n",
		_bAutoColorOffset[bType][0], _bAutoColorOffset[bType][1], _bAutoColorOffset[bType][2]);

}

void vDrvAllHDADCPow(bool bPow)
{
	u8 bCh;

	if (bPow) { /*Power on*/
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 0U, RG_VREFGEN4FE_PWD);
		vUtDelay1ms((u32)2);

		for (bCh = (u8)0; bCh < (u8)3; bCh++) {
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 0U, RG_VGAADC1_CORE_PWD);
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 0U, RG_VGAADC1_DIV_SEL);
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 0U, RG_VGAADC1_CK_PWD);
			vIO32WriteFldMulti(REG_VGA_Normal_CFG1 + (u32)((u8)4 * bCh), P_Fld(0U, RG_FE_1_PWD)
			| P_Fld(0U, RG_COP_1_PWD), RG_FE_1_PWD | RG_COP_1_PWD);
		}

		vIO32WriteFldMulti(REG_VGA_Normal_CFG4,
			P_Fld(0, RG_VGAADC1_IO_PWD) | P_Fld(0, RG_VGAADC2_IO_PWD) | P_Fld(0, RG_VGAADC3_IO_PWD),
			RG_VGAADC1_IO_PWD | RG_VGAADC2_IO_PWD | RG_VGAADC3_IO_PWD);
		vUtDelay1ms((u32)2);

		vIO32WriteFldAlign(HDTV_03, 1U, HDTV_EN);
		vDrvAllADCPLLPow((bool)TRUE);
	} else {
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, 1U, RG_VREFGEN4FE_PWD);

		for (bCh = (u8)0; bCh < (u8)3; bCh++) {
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 1U, RG_VGAADC1_CORE_PWD);
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 1U, RG_VGAADC1_DIV_SEL);
			vIO32WriteFldAlign(REG_VGA_Normal_CFG5 + (u32)((u8)4 * bCh), 1U, RG_VGAADC1_CK_PWD);
		}

		vIO32WriteFldMulti(REG_VGA_Normal_CFG4,
			P_Fld(1U, RG_VGAADC1_IO_PWD) | P_Fld(1U, RG_VGAADC2_IO_PWD) | P_Fld(1U, RG_VGAADC3_IO_PWD),
			RG_VGAADC1_IO_PWD | RG_VGAADC2_IO_PWD | RG_VGAADC3_IO_PWD);
		vIO32WriteFldAlign(HDTV_03, 0U, HDTV_EN);
		vDrvAllADCPLLPow((bool)FALSE);
	}
}

void vDrvSetHDTVADC(void)
{
	u8 bInputType = (u8)0xFF;
	u8 bLoopCnt = 0;

	vIO32WriteFldMulti(PDWNC_VGACFG1,
		P_Fld(0U, FLD_RG_CVBS_EN) | P_Fld(0U, FLD_RG_BYPS_SYNCPROSR) | P_Fld(0U, FLD_RG_BYPS_SOGYPGA) ,
		FLD_RG_CVBS_EN | FLD_RG_BYPS_SYNCPROSR | FLD_RG_BYPS_SOGYPGA); /* Clamp level 0xE8884C58 // 0xE8884D58*/
	vIO32WriteFldMulti(PDWNC_VGACFG3,
		P_Fld(1U, FLD_RG_CLAMPREFSEL) | P_Fld(0x1BU, FLD_RG_SOGY_SINK)
		| P_Fld(0xBU, FLD_RG_SOGY_SOURCE) | P_Fld(0x3U, FLD_RG_SOGY_RGAIN) | P_Fld(0x3U, FLD_RG_SOGY_BW),
		FLD_RG_CLAMPREFSEL | FLD_RG_SOGY_SINK | FLD_RG_SOGY_SOURCE
		| FLD_RG_SOGY_RGAIN | FLD_RG_SOGY_BW);
	vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(0U, FLD_RG_SYNC_TSTSEL)
		| P_Fld(0U, FLD_RG_SYNC_DESPK_EN) | P_Fld(0U, FLD_RG_SYNC_PWD),
		FLD_RG_SYNC_TSTSEL | FLD_RG_SYNC_DESPK_EN | FLD_RG_SYNC_PWD);
		/* [SA7_Nick] 080520 revised, power down despike*/
		/* vIO32WriteFldAlign(PDWNC_VGACFG2, 0x88801011,PDWNC_VGACFG2_All);  //5360 Michael checked*/
	vIO32WriteFldAlign(PDWNC_VGACFG1, 0x01U, FLD_RG_FB_AB_EN);
	vIO32WriteFldAlign(PDWNC_VGACFG2, 1U, FLD_RG_SHIFT_PWD);   /*power down SOGY sink clamp*/
	vIO32WriteFldAlign(PDWNC_VGACFG2, 1U, FLD_RG_OFFCUR_PWD); /*power down SOGY source clamp*/
	vIO32WriteFldAlign(HDFE_03, 0x200U, AD1_GAIN_BIAS);   /*5881 fix 512==>0x200*/
	vIO32WriteFldAlign(HDFE_03, 0x200U, AD2_GAIN_BIAS);   /*5881 fix 512*/
	vIO32WriteFldAlign(HDFE_03, 0x200U, AD3_GAIN_BIAS);  /*5881 fix 512*/
	/*yunjie mark need to fix*/
	/*vDrvVoDDDSRST(1);*/

	/* 1. Enable Mapping of Channel 1*/
	vIO32WriteFldAlign(HDFE_02, 0x0FU, IDX_CHANNEL_EN);  /*[SA7_Michael] enable mapping table for CH1,2,3*/

	switch (g_u4SrcType) {
	case SRC_YBR:   /*HDTV YPbPr0 YPbPr1*/
		vIO32WriteFldAlign(REG_VGA_Normal_CFG1, 0U, RG_CLAMP_MIDDLE_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG2, 1U, RG_CLAMP_MIDDLE_2);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG3, 1U, RG_CLAMP_MIDDLE_3);
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(8U, FLD_RG_SYNC1_VTL) | P_Fld(8U, FLD_RG_SYNC1_VTH),
				   FLD_RG_SYNC1_VTL | FLD_RG_SYNC1_VTH); /*SOY*/
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(0U, FLD_RG_SYNC2_VTH) | P_Fld(0U, FLD_RG_SYNC2_VTL),
				   FLD_RG_SYNC2_VTH | FLD_RG_SYNC2_VTL); /*MON*/
		vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(0U, FLD_RG_VSYNC_EN)
			| P_Fld(0U, FLD_RG_HSYNC_EN) | P_Fld(1U, FLD_RG_BYPS_SYNCPROSR),
				   FLD_RG_VSYNC_EN | FLD_RG_HSYNC_EN | FLD_RG_BYPS_SYNCPROSR);
		bInputType = 0;
		break;

	case SRC_VGA: /*VGA*/
		vIO32WriteFldAlign(REG_VGA_Normal_CFG1, 0, RG_CLAMP_MIDDLE_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG2, 0, RG_CLAMP_MIDDLE_2);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG3, 0, RG_CLAMP_MIDDLE_3);
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(4U, FLD_RG_SYNC1_VTH) | P_Fld(2U, FLD_RG_SYNC1_VTL),
				   FLD_RG_SYNC1_VTH | FLD_RG_SYNC1_VTL); /*HSYNC*/
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld(4U, FLD_RG_SYNC2_VTH) | P_Fld(2U, FLD_RG_SYNC2_VTL),
				   FLD_RG_SYNC2_VTH | FLD_RG_SYNC2_VTL); /*VSYNC*/
		vIO32WriteFldMulti(PDWNC_VGACFG1, P_Fld(1U, FLD_RG_VSYNC_EN) | P_Fld(1U, FLD_RG_HSYNC_EN)
		| P_Fld(0, FLD_RG_BYPS_SYNCPROSR), FLD_RG_VSYNC_EN | FLD_RG_HSYNC_EN | FLD_RG_BYPS_SYNCPROSR);
		bInputType = (u8)1;
		break;

	default:
		bInputType = (u8)0xFF;
		break;
	}

	/*2. Load PGA gain and offset*/
	if (bInputType != (u8)0xFF) {

		vSetPGAGainOffsetInitVal(bInputType);

		for (bLoopCnt = (u8)0; bLoopCnt < (u8)3; bLoopCnt++) {
			vIO32WriteFldAlign(REG_VGA_Normal_CFG1 + (u32)((u8)4 * bLoopCnt), 0x01U, RG_FE_OFFSET_P1);
			/*PPGAOFFSET_2,PGAOFFSET_3*/
			vDrvSetHDADCDigitalOffset(bLoopCnt, _bAutoColorOffset[bInputType][bLoopCnt]);
			bDrvSetHDADCGain_Digital(bLoopCnt, _bAutoColorGain[bInputType][bLoopCnt]);
		}

		vDrvHDTVMeasureSetting(UPDATE_PER_1_FIELD);
		vDrvHDTVClampMethodDefaultSetting(bInputType);
	}
}


void vDrvSetHDTVMux(void)
{
	/* YPbPr/VGA use the same input Mux HDTV1_EN(COM1), SOY1(SOG), Y1P(BP), Pb1P(GP), Pr1P(RP) */
	vIO32WriteFldAlign(PDWNC_VGACFG0, VGAMUX_HDTV1_EN, FLD_RGBHDTV01_EN);   /*Set RGB Mux YPbPb1*/
	vIO32WriteFldAlign(PDWNC_VGACFG1, 2U, FLD_SOY1_SOY0_EN);                 /*Enable SOY1, disable SOY0*/
	vIO32WriteFldAlign(PDWNC_VGACFG1, 0U, FLD_RG_SOG_EN);                    /*disable  SOG*/

}

/*to control the SOY0, SOY1, SOGY ; 1:enable, 0:disable*/
void vDrvSOY1EN(u8 bEn)
{
	if (bEn == (u8)1) { /*Enable SOY1*/
		vIO32WriteFldAlign(PDWNC_VGACFG1, 1U, FLD_RG_SOY1_EN);
	} else { /*Disable SOY1*/
		vIO32WriteFldAlign(PDWNC_VGACFG1, 0U, FLD_RG_SOY1_EN);
	}
}

/** SOG -->SOY1
void vDrvSOGEN(u8 bEn)
{
    if (bEn == 1) //Enable SOG
    {
	IO32WriteFldAlign(PDWNC_VGACFG1, 1, FLD_RG_SOG_EN);
    }
    else //Disable SOG
    {
	IO32WriteFldAlign(PDWNC_VGACFG1, 0, FLD_RG_SOG_EN);
    }
}
*/

/**
 * @brief void vResetVLen(void)
 * Reset Vlen counter when losing V sync for VGA
 * @param   None
 * @retval  None
 */
void vResetVLenSP0(void)
{
	vIO32WriteFldAlign(ASYNC_02, 0x1U, AS_HLEN_VLEN_RESET);   /* Set HVLEN_RESET*/
	vUtDelay2us((u32)10);
	vIO32WriteFldAlign(ASYNC_02, 0x0U, AS_HLEN_VLEN_RESET);  /* Disable HVLEN_RESET*/
}

/**
 * @brief void vResetVLen(void)
 * Reset Vlen counter when losing V sync for VGA
 * @param   None
 * @retval  None
 */
void vResetVLenSP2(void)
{
	vIO32WriteFldAlign(ASYNC_02, 0x1U, AS_HLEN_VLEN_RESET_SP2);   /* Set HVLEN_RESET in SP2*/
	vUtDelay2us((u32)10);
	vIO32WriteFldAlign(ASYNC_02, 0x0U, AS_HLEN_VLEN_RESET_SP2);   /* Disable HVLEN_RESET in SP2*/
}

/**
 * @brief Return Monitor Slicer  V-Length
 * @param void
 */
u16 wASV_MONtotalMeasure(void)
{
	return IO32ReadFldAlign(STA_SYNC2_01, AS2_V_LEN_S);/*MT5360B has the same address with MT5360A*/
}

/**
 * @brief Return composite sync V-Length in Main Slicer
 *
 * @param void
 */
u16 wAS_CV_totalMeasure(void)
{
	return IO32ReadFldAlign(STA_SYNC0_00, AS_V_LEN_S);
}


void vQuerySlicer(void)
{
	pr_debug("slicer Low/High %d/%d\r\n", (int)IO32ReadFldAlign(PDWNC_VGACFG4,  FLD_RG_SYNC1_VTL),
		(int)IO32ReadFldAlign(PDWNC_VGACFG4,  FLD_RG_SYNC1_VTH));
}


u8 check_quaity_state;
#define SLICER_LV_LIST_NUM 5 /* 4*/

u8 MonSlicerSetting[] = {0xAA, 0x43, 0xAA, 0xAA, 0x65};


void vSetSlicer(u8 pair)
{
	if (bHdtvOpt05_AdaptiveSlicer) {
		/*vIO32WriteFldAlign(SyncSlice_Control_01, pair,VTLH_SEL);*/
		/*vIO32WriteFldAlign(PDWNC_SOGYCFG,pair,FLD_PD_SOGY_VTLH);*/
		vIO32WriteFldMulti(PDWNC_VGACFG4, P_Fld((pair & (u8)0xF0) >> 4, FLD_RG_SYNC1_VTL)
		| P_Fld((pair & (u8)0x0F), FLD_RG_SYNC1_VTH), FLD_RG_SYNC1_VTL | FLD_RG_SYNC1_VTH);
		/*UTIL_Printf("set slicer High/Low 0x%x\r\n", pair);*/
		/*UTIL_Printf("set slicer Low/High 0x%x\r\n", pair);*/

		vSetMONSlicer_Matrix();
	}
}


void vSetMONSlicer_Matrix(void)
{
	if (_bCurSlicerIdx >= (u8)SLICER_LV_LIST_NUM) {
		return;
	}

	vSetMONSlicer(MonSlicerSetting[_bCurSlicerIdx]);
}



u8 _bCurSlicerIdx = 0;
u8 _bCurSlicerIdx_best = 0;


void vSetDefaultSlicer(void)
{
	/*UTIL_Printf("set slicer in DefaultSlicer\r\n");*/
	_bCurSlicerIdx = 0;
	_bCurSlicerIdx_best = 0;
	vSetSlicer((u8)0x88);  /* 0x88*/
}

void check_quaity(u16 vlen, const u8 *trylist, u8 trynr, u32 ms_delay, u32 ms_delay2, u8 h_prot)
{
	static  u8 i;

	static  s32 bestdiff;

	static  HAL_TIME_T t_init;
	static  u8 bestone;
	static  s32 ai4DiffScore[SLICER_LV_LIST_NUM];
	static  u8 bestlevel;
	u8 bTmpCnt;
	s32 diff;
	HAL_TIME_T t_end = {0};
	u32 time_esplase;

	if ((!vlen) || (!trynr)) {
		if (trylist) {
			vSetSlicer(trylist[0]);
		}

		check_quaity_state = (u8)10;
		return;
	}

	ms_delay = (u32)ms_delay * 1000U;
	ms_delay2 = (u32)ms_delay2 * 1000U;

	switch (check_quaity_state) {
	case (u8)0:
		i = 0;
		bestone = trylist[0];
		bestdiff = 0xffff;
		check_quaity_state++;
		break;

	case (u8)1:
		/*UTIL_Printf("set slicer in Fine tune\r\n");*/
		vSetSlicer(trylist[i]);
		vResetVLen();
		HAL_GetTime(&t_init);
		check_quaity_state++;
		break;

	case (u8)2:
		HAL_GetTime(&t_end);
		time_esplase = (u32)((t_end.u4Seconds - t_init.u4Seconds) * 1000000) +
			       (t_end.u4Micros - t_init.u4Micros);

		if (time_esplase >= ms_delay) {
			HAL_GetTime(&t_init);
			check_quaity_state++;
		}

		break;

	case (u8)3:
		HAL_GetTime(&t_end);
		diff = (s32)(vlen - wASVtotalMeasure());

		if (diff < 0) {
			diff = -diff;
		}

		ai4DiffScore[i] = diff;

		/*UTIL_Printf( "Slicer[%d] Diff = %x\r\n", i, diff);*/
		if (diff >= 2) {
			if (diff < bestdiff) {
				bestone = trylist[i];
				bestdiff = diff;
				bestlevel = i;
			}

			i++;

			if (i < trynr) {
				check_quaity_state = (u8)1;
			} else {
				check_quaity_state = (u8)4;
			}

			break;
		}

		time_esplase = (u32)((t_end.u4Seconds - t_init.u4Seconds) * 1000000) +
			       (t_end.u4Micros - t_init.u4Micros);

		if (time_esplase >= ms_delay2) {
			bestone = trylist[i];
			bestlevel = i;
			_bCurSlicerIdx_best = bestlevel;
			check_quaity_state = (u8)4;
		}

		break;

	case (u8)4:
		for (bTmpCnt = 0; bTmpCnt < i; bTmpCnt++) {
			if (ai4DiffScore[bestlevel] >= ai4DiffScore[bTmpCnt]) {
				/* bTmpCnt is better or equal to the bestone */
				if (DIFF(bestlevel, trynr / (u8)2) > DIFF(bTmpCnt, trynr / (u8)2)) {
					/* bTmpCnt is near center of the list */
					bestlevel = bTmpCnt;
					bestone = trylist[bTmpCnt];
					/*UTIL_Printf("Center_Slicer = 0x%x\r\n", bestone);*/
				}
			}
		}

		pr_debug("final choose bestone in fine tune = 0x%x\r\n", bestone);
		vSetSlicer(bestone);

		check_quaity_state = (u8)10;
		break;
		
	default:
		break;
	}

}

typedef struct slicerSetting_s {
	u8 bCoarse;
	u8 abVReduce[4];
} slicerSetting_t;


slicerSetting_t rSlicerSetting[] = {
	{0x88, {0x88, 0x99, 0x9A, 0xAA} }, /* 0x58, {0x58,0x69,0x79,0x89}, // 0xa8, {0x98,0xa8,0xb9,0xc9},*/
	{0x7B, {0x7B, 0x8C, 0x9D, 0xAE} }, /* 0x79, {0x69,0x79,0x89,0x9A}, // 0xb9, {0xa8,0xb9,0xc9,0xca},*/
	{0x56, {0x56, 0x67, 0x78, 0x88} },
	{0x35, {0x35, 0x36, 0x47, 0x58} }, /* 0x04, {0x04,0x15,0x26,0x37},  // 480i HV Reduce*/
	{0xCD, {0xCD, 0xDD, 0xED, 0xDE} }, /* 0xCF, {0xCF,0xDF,0xCE,0xDE},  // 720p60 V Reduce*/
};


void  vSliceQuality536x(void)
{
	u8 *list;

	list = rSlicerSetting[(_bCurSlicerIdx) % (sizeof(rSlicerSetting) / sizeof(slicerSetting_t))].abVReduce;

	switch (_bHdtvTiming) {
	case MODE_525I:
	case MODE_525I_OVERSAMPLE:
		check_quaity((u16)263, list, (u8)4, (u32)40, (u32)360, (u8)1);
		break;

	case MODE_625I:
	case MODE_625I_OVERSAMPLE:
		check_quaity((u16)313, list, (u8)4, (u32)50, (u32)440, (u8)1);
		break;

	case MODE_480P:
	case MODE_480P_OVERSAMPLE:
		check_quaity((u16)525, list, (u8)4, (u32)40, (u32)360, (u8)1);
		break;

	case MODE_576P:
	case MODE_576P_OVERSAMPLE:
		check_quaity((u16)625, list, (u8)4, (u32)50, (u32)440, (u8)1);
		break;

	case MODE_720p_50:
		check_quaity((u16)750, list, (u8)4, (u32)50, (u32)440, (u8)0); /*0x2f3*/
		break;

	case MODE_720p_60:
		check_quaity((u16)750, list, (u8)4, (u32)40, (u32)360, (u8)0); /*0x2f3*/
		break;

	case MODE_1080i_50:
		check_quaity((u16)563, list, (u8)4, (u32)50, (u32)440, (u8)0); /*0x243*/
		break;

	case MODE_1080i:
		check_quaity((u16)563, list, (u8)4, (u32)40, (u32)360, (u8)0); /*0x243*/
		break;

	case MODE_1080p_50:
		check_quaity((u16)1125, list, (u8)4, (u32)50, (u32)440, (u8)0); /*0x46a*/
		break;

	case MODE_1080p_60:
		check_quaity((u16)1125, list, (u8)4, (u32)40, (u32)360, (u8)0); /*0x46a*/
		break;

	default:
		/*  MODE_1080i_48,
		MODE_1080p_24,
		MODE_1080p_25,
		MODE_1080p_30,  */
		check_quaity((u16)0, list, (u8)4, (u32)50, (u32)110, (u8)0);
		break;

	case MODE_NOSIGNAL:
	case MODE_NODISPLAY:
	case MODE_NOSUPPORT:
	case MODE_WAIT:
		return;
	}
}


HAL_TIME_T rPreSliceTime, rDeltaSliceTime, _rCurSliceTime;
void vResetSliceTimer(void)
{
	if (bHdtvOpt05_AdaptiveSlicer) {
		HAL_GetTime(&rPreSliceTime);
		/*UTIL_Printf("set slicer in ResetSliceTimer\r\n");*/
		/*vSetDefaultSlicer();*/
		/* default level index */
		/*_bCurSlicerIdx = 0;//reset slicer level to index 0 to match default setting*/
	}
}

void vNextSlicer(u16 wTimeout)
{
	if (bHdtvOpt05_AdaptiveSlicer) {
		u32 u4MilliSec;

		HAL_GetTime(&_rCurSliceTime);
		HAL_GetDeltaTime(&rDeltaSliceTime, &_rCurSliceTime, &rPreSliceTime);
		u4MilliSec = (u32)((rDeltaSliceTime.u4Seconds * 1000) + (rDeltaSliceTime.u4Micros / 1000));

		if (u4MilliSec >= wTimeout) {
			HAL_GetTime(&rPreSliceTime);
			_bCurSlicerIdx++;

			if (_bCurSlicerIdx >= (u8)5) {
				_bCurSlicerIdx = 0;
			}

			/*UTIL_Printf("set slicer in Coarse tune\r\n");*/
			vSetSlicer(rSlicerSetting[(_bCurSlicerIdx)
			% (sizeof(rSlicerSetting) / sizeof(slicerSetting_t))].bCoarse);
			/*#if ADAPTIVE_MONITOR_SLICER_MEASURE*/
			vSetMONSlicer(MonSlicerSetting[_bCurSlicerIdx]);
			/*#endif*/
		}
	}
}


void vSetMONSlicer(u8 pair)
{
	if (bHdtvOpt05_AdaptiveSlicer) {
		vIO32WriteFldAlign(PDWNC_VGACFG4, (u32)pair, FLD_RG_SYNC2_VHLSEL);
		/*UTIL_Printf("[SA7] set MON slicer High/Low = 0x%x \r\n",pair);*/
	}
}

u8 bReadMONSlicer(void)
{
	return IO32ReadFldAlign(PDWNC_VGACFG4, FLD_RG_SYNC2_VHLSEL);
}

u8 bReadNewSlicer(void)
{
	u8 pair;

	pair = (u8)(IO32ReadFldAlign(PDWNC_VGACFG4, FLD_RG_SYNC1_VTL) << 4
	| IO32ReadFldAlign(PDWNC_VGACFG4, FLD_RG_SYNC1_VTH));
	return pair;
}




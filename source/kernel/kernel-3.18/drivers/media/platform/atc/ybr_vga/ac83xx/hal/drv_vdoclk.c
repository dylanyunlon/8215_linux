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
#include "drv_vdoclk.h"
#include "ybr_vga_common.h"


#define LOG_TAG "vdoclk"

u32  _dBestSumALL[CHANNEL_NUM];
u8   _bMaxSlope[CHANNEL_NUM];
u8   _bChBestPhase[CHANNEL_NUM];
u32  _dSumTmpPhase[CHANNEL_NUM][MAX_PHASE_ARRAY];
u32  _dwPhase3CH[CHANNEL_NUM];

u32 _u4GetLockCw;
u32 _u4UiSetCw;
u8   _bCLKSetFlag;


u32  _dSumTmp[3];

#define SYSPLL432 4320
#define SYSPLL324 3240
#define SYSPLL459 4590

enum {
	VO_PLL_CLKOUT,
	VO_PLL_XTAL
};
#define PIX_CLK_INV_Criterion 1390

#define CLKIN_LOCK_LINE_CNT_VAL 0x02
#define CLKIN_LOCK_THR_VAL 0x02
#define CLKIN_HSYNC_WIDTH 32
u32 _u4VPllFrequency = 0;


u8  LPF_Table_1[10][4] = {
	/*ADCREV_1_LPF[21:17]  ADCREV_2_LPF[21:17]   ADCREV_3_LPF[21:17]   REG_VGA_Normal_CFG8[27:24]*/
	{0x00,            0x00,              0x00,            0x00},    /* 0. Bypass_LPF                     (0, 0)*/
	{0x0F,            0x0F,              0x0F,            0x0F},       /* 1. YPbPr_480i_LPF               (F, F)*/
	{0x0D,            0x0D,              0x0D,            0x0A},       /* 2. YPbPr_480p_LPF              (A, A)*/
	{0x08,            0x08,              0x08,            0x05},    /* 3. YPbPr_720p_LPF              (3, 5)*/
	{0x02,            0x02,              0x02,            0x01},    /* 4. YPbPr_1080p_LPF            (2, 1)*/
	{0x0C,            0x0A,              0x0A,            0x0F},    /* 5. VGA_40MHZ_Less_LPF    (7, F)*/
	{0x09,            0x08,              0x08,            0x08},    /* 6. VGA_40_60MHZ_LPF       (6, A)*/
	{0x03,            0x02,              0x02,            0x04},    /* 7. VGA_60_90MHZ_LPF       (3, 5)*/
	{0x03,            0x02,              0x02,            0x01},    /* 8. VGA_90_130MHZ_LPF     (2, 2)*/
	{0x03,            0x02,              0x02,            0x00} /* 9. VGA_120MHZ_More_LPF  (1, 1)*/
};
u8  LPF_Table_2[10][3] = {
	/*PDWNC_VGACFG3[7:6]      PDWNC_VGACFG6[7:6]    REG_VGA_Normal_CFG8[27:24] no use*/
	{0x00,                         0x00,                        0x00},    /* 0. Bypass_LPF              (0, 0)*/
	{0x01,                         0x03,                        0x0F},       /* 1. YPbPr_480i_LPF    (F, F)*/
	{0x01,                         0x02,                        0x0A},       /* 2. YPbPr_480p_LPF  (A, A)*/
	{0x00,                         0x02,                        0x05},    /* 3. YPbPr_720p_LPF      (3, 5)*/
	{0x00,                         0x01,                        0x01},    /* 4. YPbPr_1080p_LPF    (2, 1)*/
	{0x00,                         0x03,                        0x0F},    /* 5. VGA_40MHZ_Less_LPF(7, F)*/
	{0x00,                         0x02,                        0x08},    /* 6. VGA_40_60MHZ_LPF   (6, A)*/
	{0x00,                         0x01,                        0x04},    /* 7. VGA_60_90MHZ_LPF   (3, 5)*/
	{0x00,                         0x01,                        0x01},    /* 8. VGA_90_130MHZ_LPF  (2, 2)*/
	{0x00,                         0x01,                        0x00}     /* 9. VGA_120MHZ_More_LPF(1, 1)*/
};


void vDrvYbrVgaClkEnable(void)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");
#ifdef __ARM2__
	CKGEN_AgtOnClk(e_CLK_YPBPR);
	CKGEN_AgtOnClk(e_CLK_VGA);
#else
	clk_prepare_enable(g_clk_ybr);
#endif
}


void vDrvYbrVgaClkDisable(void)
{
	YBRVGA_DEBUG(LOG_TAG, "enter\n");
#ifdef __ARM2__
	CKGEN_AgtOffClk(e_CLK_YPBPR);
	CKGEN_AgtOffClk(e_CLK_VGA);
#else
	if (__clk_is_enabled(g_clk_ybr)) {
		clk_disable_unprepare(g_clk_ybr);
	}
#endif
}


void vDrvClkInit(void)  /* ToDo - vDrvVideoInit*/
{
	/*UTIL_Printf("vDrvClkInit\r\n");*/
	/*yunjie add*/
	vIO32WriteFldMulti(YBR_VGA_CLK_CFG,
			   P_Fld((u32)0x1, RG_XDDS_CLK_ON) | P_Fld((u32)1, RG_RESYNC_CLK_ON) | P_Fld((u32)0x1, RG_HDTV_CLK_ON),
			   RG_XDDS_CLK_ON | RG_RESYNC_CLK_ON | RG_HDTV_CLK_ON);
	vIO32WriteFldAlign(VFE_18, 0, RG_SDDS_FIFO_START);/*for suspend/resume unlock issue*/

	/*CC_MT5365 according to Walter's suggestion*/
	vIO32WriteFldAlign(VFE_18, 1, RG_SDDS_DATA_SYNC);
	vUtDelay2us((u32)1);
	vIO32WriteFldAlign(VFE_18, 0, RG_SDDS_DATA_SYNC);
	vIO32WriteFldAlign(VFE_18, 0, SDDS_CLK3_INV);
	vIO32WriteFldAlign(VFE_18, (u32)1, SDDS_CLK1_INV);

	/*vDrvCLKINPsncoRST(1);      //MT5387 don't have this bit  //bit29          : C_PSNCO_RST = 1*/
	vDrvCLKINMaxPeriod((u32)3);        /*bit 28~27    : C_MAX_PERIOD =maximum error detection period*/
	vDrvCLKINFMPeriod(0);         /*bit 26~25    : C_FM_PERIOD = Frequency modulation detection period*/
	/*vDrvCLKINFreeHsyncPol(0);  //bit 24         : C_HSYNC_POL = polarity of external sync*/
	vDrvCLKINSetLockThr((u32)2);     /*bit 23~22   : C_LOCK_THR*/
	vDrvCLKINSetKI0((u32)8);              /*bit 21~18  : C_DCLK1_KI0*/
	vDrvCLKINSetKI1((u32)1);              /*bit 17~16  : C_DCLK1_KI1*/
	vDrvCLKINSetLockCnt((u32)2);       /*bit 15~14  : C_LOCK_LINE_CNT*/
	vDrvCLKINSetKP0((u32)8);              /*bit 13~10  : C_DCLK1_KP0 //DTV00130828(7->8)*/
	vDrvCLKINSetKP1((u32)1);              /*bit 9~8     : C_DCLK1_KP1*/
	vDrvCLKINSetErrLim((u32)0xff);     /*bit 7~0     : C_ERR_LIM*/
	vIO32WriteFldAlign(VFE_17, (u32)0x9, SDDS_CKSEL);/*for SDDS 324MHz clock phase selection*/

	/*fix ADCPLL o/p after initialized*/
	vDrvCLKINFreeRun(ENABLE);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld((u32)0x1, FLD_RG_VGAPLL_CKO_SEL) |
		P_Fld((u32)0x1, FLD_RG_VGAPLL_B_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_G_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#else
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld((u32)0x1, FLD_RG_VGAPLL_CKO_SEL) |
		P_Fld((u32)0x1, FLD_RG_VGAPLL_B_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_G_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_CKO_SEL | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#endif
	vIO32WriteFldMulti(REG_VGA_Normal_CFG8, P_Fld((u32)1, RG_CLKINV_EN) | P_Fld((u32)1, RG_RELATCH_EN),
		RG_CLKINV_EN | RG_RELATCH_EN);
		/*be careful the define for this bit is different between MT5365 AND MT8223*/
	vIO32WriteFldAlign(REG_PLL_CTRL, (u32)1, FLD_RG_VGAPLL_PWD);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG29,
		P_Fld((u32)1, FLD_RG_VGAPLL_SDDS_PD_PDB) | P_Fld((u32)1, FLD_RG_VGAPLL_SDDS_PD_EN)
		| P_Fld((u32)7, FLD_RG_XDDS_PI_C) | P_Fld((u32)1, FLD_RG_XDDS_HF) | P_Fld((u32)1, FLD_RG_VGAPLL_RTB_EN),
		FLD_RG_VGAPLL_SDDS_PD_PDB | FLD_RG_VGAPLL_SDDS_PD_EN |
		FLD_RG_XDDS_PI_C | FLD_RG_XDDS_HF | FLD_RG_VGAPLL_RTB_EN);
#else
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG30,
		P_Fld((u32)1, FLD_RG_VGAPLL_SDDS_PD_PDB) | P_Fld((u32)1, FLD_RG_VGAPLL_SDDS_PD_EN)
		| P_Fld((u32)7, FLD_RG_XDDS_PI_C) | P_Fld((u32)1, FLD_RG_XDDS_HF) | P_Fld((u32)1, FLD_RG_VGAPLL_RTB_EN),
		FLD_RG_VGAPLL_SDDS_PD_PDB | FLD_RG_VGAPLL_SDDS_PD_EN |
		FLD_RG_XDDS_PI_C | FLD_RG_XDDS_HF | FLD_RG_VGAPLL_RTB_EN);
#endif
	vIO32WriteFldMulti(REG_PLL_CTRL, P_Fld(0, FLD_RG_VGATL_BIAS_PWD) | P_Fld((u32)1, FLD_RG_XDDS_PWDB),
		FLD_RG_VGATL_BIAS_PWD | FLD_RG_XDDS_PWDB);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG28, P_Fld(0, FLD_RG_VGAPLL_MONCKEN) |
	P_Fld(0, FLD_RG_VGAPLL_MONEN) | P_Fld((u32)1, FLD_RG_VGAPLL_RESERVE),
		FLD_RG_VGAPLL_MONCKEN | FLD_RG_VGAPLL_MONEN | FLD_RG_VGAPLL_RESERVE);
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld((u32)0x1, FLD_RG_VGAPLL_ENTL) | P_Fld(0x0, FLD_RG_VGAPLL_INTH_EN),
		FLD_RG_VGAPLL_ENTL | FLD_RG_VGAPLL_INTH_EN);
#else
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld(0, FLD_RG_VGAPLL_MONCKEN) |
		P_Fld(0, FLD_RG_VGAPLL_MONEN) | P_Fld((u32)1, FLD_RG_VGAPLL_RESERVE),
		FLD_RG_VGAPLL_MONCKEN | FLD_RG_VGAPLL_MONEN | FLD_RG_VGAPLL_RESERVE);
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG31, P_Fld((u32)0x1, FLD_RG_VGAPLL_ENTL) | P_Fld(0x0, FLD_RG_VGAPLL_INTH_EN),
		FLD_RG_VGAPLL_ENTL | FLD_RG_VGAPLL_INTH_EN);
#endif
	vIO32WriteFldAlign(VFE_18, (u32)1, RG_SDDS_FIFO_START);

}

void vDrvAllADCPLLPow(bool bPow)
{

	if (!bPow) { /*Power off*/
		vIO32WriteFldAlign(REG_PLL_CTRL, (u32)1, FLD_RG_VGAPLL_PWD);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld(0x0, FLD_RG_VGAPLL_B_EN) |
		P_Fld(0x0, FLD_RG_VGAPLL_G_EN) | P_Fld(0x0, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#else
		vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld(0x0, FLD_RG_VGAPLL_B_EN) |
		P_Fld(0x0, FLD_RG_VGAPLL_G_EN) | P_Fld(0x0, FLD_RG_VGAPLL_R_EN),
		FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_R_EN);
#endif
	}

}


bool _KPIReset;
u8 _sdds_count;

void vDrvEnableChang_SDDS_BW(void)
{
	_KPIReset = TRUE;
	_sdds_count = 0;
}

/**
 * @brief CLKIN loop filter BW
 *
 * Set CLKIN loop filter
 * @retval void
 */
void Set_SDDS_KPI(u8 bValue)
{
	if (bValue == 0) {
		if (_KPIReset) {
			_sdds_count++;

			if (_sdds_count > (u8)5) {
				vDrvCLKINSetKI0((u32)1);
				vDrvCLKINSetKP0((u32)3);
				pr_debug("Set Kp0=3, Ki0=1\r\n");
				_KPIReset = 0;
				_sdds_count = 0;
			}
		}
	} else {
		vDrvCLKINSetKI0((u32)8);
		vDrvCLKINSetKP0((u32)8);
		pr_debug("Set Kp0=8, Ki0=8\r\n");
		_KPIReset = 0;
		_sdds_count = 0;
	}
}

/*For SCART chang SDDS PI gain*/
void vDrvClkSetLockBandwidth(u8 bIsLock)
{
	if (bIsLock) { /* big*/
		vDrvCLKINSetKI0((u32)1);              /*bit 21~18  : C_DCLK1_KI0*/
		vDrvCLKINSetKP0((u32)2);              /*bit 13~10  : C_DCLK1_KP0*/
	} else {
		vDrvCLKINSetKI0((u32)8);              /*bit 21~18  : C_DCLK1_KI0*/
		vDrvCLKINSetKP0((u32)8);              /*bit 13~10  : C_DCLK1_KP0*/
	}
}

void vPGA_LPF_BW(u8 bLPF)
{
	if (bLPF < Max_Input_timing) {
		vIO32WriteFldAlign(REG_VGA_Normal_CFG1, LPF_Table_1[bLPF][0], RG_MUXCAP_EN1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG2, LPF_Table_1[bLPF][1], RG_MUXCAP_EN2);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG3, LPF_Table_1[bLPF][2], RG_MUXCAP_EN3);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG0, LPF_Table_1[bLPF][3], RG_CAP_EN);
	}
}

void vSet_VFE_LPF(u16 wInputPixClk)
{
	if (g_u4SrcType == SRC_VGA) {
		if (wInputPixClk < (u16)410) {
			vPGA_LPF_BW(VGA_40MHZ_Less_LPF); /*BW is approximate 33MHz*/
		} else if (wInputPixClk < (u16)600) {
			vPGA_LPF_BW(VGA_40_60MHZ_LPF); /*BW is approximate 43MHz*/
		} else if (wInputPixClk < (u16)900) {
			vPGA_LPF_BW(VGA_60_90MHZ_LPF); /*BW is approximate 62MHz*/
		} else if (wInputPixClk < (u16)1200) {
			vPGA_LPF_BW(VGA_90_130MHZ_LPF);  /*BW is approximate 89MHz*/
		} else {
			vPGA_LPF_BW(VGA_120MHZ_More_LPF); /*BW is approximate 119MHz*/
		}
	} else if (g_u4SrcType == SRC_YBR) {
		if (wInputPixClk < (u16)300) {
			vPGA_LPF_BW(YPbPr_480i_LPF); /*13.5MHz oversample to 27.0 MHz*/
		} else if (wInputPixClk < (u16)600) {
			vPGA_LPF_BW(YPbPr_480p_LPF); /* 27MHz oversample to 54.0 MHz*/
		} else if (wInputPixClk < (u16)900) {
			vPGA_LPF_BW(YPbPr_720p_LPF); /* 720p is 74.25MHz*/
		} else {
			vPGA_LPF_BW(YPbPr_1080p_LPF);
		}
	}

}

/**
 * @brief set ADCPLL/CLKIN and Htotal precise frequency
 *
 * Set ADCPLL and CLKIN correct frequency
 * @param wPixClk wPixClk is the target clock rate (MHz)
 * @param wHtotal wHtotal is the quantity of the pixel clock per hsync
 * @retval void
 */
void vDrvADCPLLSet(u16 wPixClk, u16 wHtotal)
{
	u32 dTmp;
	u8 bMcode;
	u8 bADCPLL_NS;

	pr_debug("vDrvADCPLLSet:wPixClk:%d, wHtotal:%d\r\n", wPixClk, wHtotal);

	vDrvCLKINDCLKFast(ENABLE);
	/* Enable for avoid suddenly large errror // Walter suggest disable, Tomson Ena /Dis DCLK Fast lock Loop*/
	vDrvCLKINDCLKPFDSEL(ENABLE); /* Tomson Ena /Dis DCLK PFD High Resoultion detection*/
#if CHANGE_SDDS_KPI
	Set_SDDS_KPI((u8)1);
#endif

	vDrvCLKINSetHtotal(wHtotal);
	vIO32WriteFldAlign(VFE_16, (u32)10, DCLK_HSYNC_WIDTH);  /* vWriteVDOINMsk(VFE_16, 0x80, 0xC0, 1);*/
	/* Avoid the the Flash noise by Data and CLK Delay mismatch*/

	vSet_VFE_LPF(wPixClk);

#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG28, P_Fld((u32)1, FLD_RG_VGAPLL_DIVEN) |
	P_Fld((u32)1, FLD_RG_VGAPLL_V11EN) | P_Fld((u32)1, FLD_RG_VGAPLL_VODEN) |
	P_Fld(0, FLD_RG_VGAPLL_BP) | P_Fld(0, FLD_RG_VGAPLL_BR) | P_Fld(0, FLD_RG_VGAPLL_HF) |
	P_Fld((u32)1, FLD_RG_VGAPLL_LF) | P_Fld(0, FLD_RG_VGAPLL_FBSEL),
	FLD_RG_VGAPLL_DIVEN | FLD_RG_VGAPLL_V11EN | FLD_RG_VGAPLL_VODEN |
	FLD_RG_VGAPLL_BP | FLD_RG_VGAPLL_BR | FLD_RG_VGAPLL_HF | FLD_RG_VGAPLL_LF | FLD_RG_VGAPLL_FBSEL);
#else
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld(1, FLD_RG_VGAPLL_V11EN)|
		P_Fld(1,FLD_RG_VGAPLL_VODEN)|P_Fld(0,FLD_RG_VGAPLL_BP)|P_Fld(0,FLD_RG_VGAPLL_BR)|
		P_Fld(0,FLD_RG_VGAPLL_HF)|P_Fld(1,FLD_RG_VGAPLL_LF)|P_Fld(0, FLD_RG_VGAPLL_FBSEL),
		FLD_RG_VGAPLL_V11EN | FLD_RG_VGAPLL_VODEN | FLD_RG_VGAPLL_BP | FLD_RG_VGAPLL_BR |
		FLD_RG_VGAPLL_HF | FLD_RG_VGAPLL_LF | FLD_RG_VGAPLL_FBSEL);
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld(1, FLD_RG_VGAPLL_DIVEN), FLD_RG_VGAPLL_DIVEN);
#endif

	/*Setting ADCPLL Band*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	if (wPixClk < (u16)450) {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, (u32)0x3, FLD_RG_VGAPLL_BS);
	} else if ((wPixClk < (u16)800) && (wPixClk >= (u16)450)) {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, (u32)0x2, FLD_RG_VGAPLL_BS);
	} else {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, 0x0, FLD_RG_VGAPLL_BS);
	}
#else
	if (wPixClk < (u16)450) {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x3, FLD_RG_VGAPLL_BS);
	} else if ((wPixClk < (u16)800) && (wPixClk >= (u16)450)) {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x2, FLD_RG_VGAPLL_BS);
	} else {
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, 0x0, FLD_RG_VGAPLL_BS);
	}
#endif

	/*Setting before ADCPLL*/
	if (wPixClk < (u16)300) {
		bADCPLL_NS = (u8)1;
		wPixClk = wPixClk << 1; /* Pixel clock*/
		/*vIO32WriteFldAlign(CKGEN_AFEPLLCFG5, 1, FLD_RG_VGAPLL_SDDSO_DIV12_SEL);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, 0x0, FLD_RG_XDDS_CKSEL);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, 0x0, FLD_RG_XDDS_CKSEL);
#endif
	} else if ((wPixClk < (u16)650) && (wPixClk >= (u16)300)) { /*(SDclk CW)/1, ADCPLLx1*/
		bADCPLL_NS = (u8)1;
		/*vIO32WriteFldAlign(CKGEN_AFEPLLCFG5,0, FLD_RG_VGAPLL_SDDSO_DIV12_SEL);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x1, FLD_RG_XDDS_CKSEL);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, (u32)0x1, FLD_RG_XDDS_CKSEL);
#endif
	} else if ((wPixClk < (u16)1200) && (wPixClk >= (u16)650)) { /*(SDclk CW)/2, ADCPLLx2     //wu*/
		bADCPLL_NS = (u8)2;
		wPixClk = wPixClk >> 1; /* Pixel clock divider by 2*/
		/*vIO32WriteFldAlign(CKGEN_AFEPLLCFG5,0, FLD_RG_VGAPLL_SDDSO_DIV12_SEL);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x1, FLD_RG_XDDS_CKSEL);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, (u32)0x1, FLD_RG_XDDS_CKSEL);
#endif
	} else { /* pixel >1000 (SDclk CW)/3, ADCPLLx3*/
		bADCPLL_NS = (u8)3;
		wPixClk = (u16)(wPixClk / 3);/* Pixel clock divider by 3*/
		/*vIO32WriteFldAlign(CKGEN_AFEPLLCFG5,0, FLD_RG_VGAPLL_SDDSO_DIV12_SEL);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x1, FLD_RG_XDDS_CKSEL);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, (u32)0x1, FLD_RG_XDDS_CKSEL);
#endif
	}

	/*Setting before PFD*/
	if (wPixClk < (u16)300) {
		bADCPLL_NS = bADCPLL_NS * 1 - 1;
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, 0, FLD_RG_VGAPLL_PREDIV);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, 0, FLD_RG_VGAPLL_PREDIV);
#endif
	} else if ((wPixClk < (u16)650) && (wPixClk >= (u16)300)) {
		bADCPLL_NS = bADCPLL_NS * 2 - 1;
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, (u32)1, FLD_RG_VGAPLL_PREDIV);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)1, FLD_RG_VGAPLL_PREDIV);
#endif
	} else {
		bADCPLL_NS = bADCPLL_NS * 4 - 1;
#ifdef CONFIG_ATC_PLATFORM_ac83xx
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, (u32)3, FLD_RG_VGAPLL_PREDIV);
#else
		vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)3, FLD_RG_VGAPLL_PREDIV);
#endif
	}

	/* vIO32WriteFldAlign(CKGEN_AFEPLLCFG5, 2, FLD_RG_VGAPLL_SDDS_FBKSEL);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldAlign(REG_PLL_GROUP_CFG29, (u32)0x2, FLD_RG_VGAPLL_SDDS_FBKSEL);
#else
	vIO32WriteFldAlign(REG_PLL_GROUP_CFG31, (u32)0x2, FLD_RG_VGAPLL_SDDS_FBKSEL);
#endif

	/*vIO32WriteFldAlign(CKGEN_AFEPLLCFG0, bADCPLL_NS, FLD_RG_VGAPLL_DIV);*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldAlign(REG_PLL_GROUP_CFG28, bADCPLL_NS, FLD_RG_VGAPLL_FBDIV);
#else
	vIO32WriteFldAlign(REG_PLL_GROUP_CFG30, bADCPLL_NS, FLD_RG_VGAPLL_FBDIV);
#endif

	bMcode = (u8)((u32)SYSPLL324 / (u32) wPixClk);  /*cal the M code*/
	dTmp = SYSPLL324 - (u32) wPixClk * bMcode; /*calculate residue*/

	dTmp = (u32)(dTmp << 16) / (u32) wPixClk;
	/*calculate 24 bit residue, only calculate CW2,1 , CW0 don't care*/
	/*if <<24 will cause (residue ^24 overflow) , so <<16 is enough*/
	dTmp = (bMcode << 24) | (dTmp << 8);

	vDrvCLKINFreeRun(ENABLE); /*SDDS freerun bit[31] set "1"*/

	HAL_WRITE32(VFE_15, dTmp);            /*SDDS CW set*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG29, P_Fld((u32)0x1, FLD_RG_VGAPLL_G_EN) |
	P_Fld((u32)0x1, FLD_RG_VGAPLL_B_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_R_EN),
			   FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_R_EN);
#else
	vIO32WriteFldMulti(REG_PLL_GROUP_CFG30, P_Fld((u32)0x1, FLD_RG_VGAPLL_G_EN) |
	P_Fld((u32)0x1, FLD_RG_VGAPLL_B_EN) | P_Fld((u32)0x1, FLD_RG_VGAPLL_R_EN),
			   FLD_RG_VGAPLL_G_EN | FLD_RG_VGAPLL_B_EN | FLD_RG_VGAPLL_R_EN);
#endif
	vIO32WriteFldAlign(REG_PLL_CTRL, 0, FLD_RG_VGAPLL_PWD);
	vDrvCLKINDCLKFast(ENABLE);
	/* Enable for avoid suddenly large errror //Walter suggest disable, Tomson Ena /Dis DCLK Fast lock Loop*/

	vDrvCLKINFreeRun(DISABLE);/* vWriteVDOINMsk(VFE_14,0x00,0x80,3); SDDS lock 20022490[31] set "0"*/
	vUtDelay2us((u32)200);

	_bCLKSetFlag |= 0x04;

#if CHANGE_SDDS_KPI
	vDrvEnableChang_SDDS_BW();
#endif

	_RETIMENeedReset = TRUE;                                /*Reset RETIME in VGA ISR*/
}


/**
 * @brief Set the Htotal length
 *
 * Set the quantity of pixel clocks per hsync (hsync length)
 * @param wHtotal wHtotal is the quantity of pixel clocks per hsync
 * @retval void
 */
void vDrvCLKINSetHtotal(u16 wHtotal)
{
	if (wHtotal != 0) {
		wHtotal--;
		vIO32WriteFldAlign(VFE_16, wHtotal, DCLK_HLINE_LENGTH);
	}
}


/**
 * @brief Set the internal hsync width
 *
 * Set the internal hsync width by pixels
 * @param wWidth wWidth is the quantity of hsync width
 * @retval void
 */
void vDrvCLKINSetHsyncWid(u8 wWidth)  /*  Only use 8-bits , because the HsyncWifth << 255*/
{

	/*vWriteVDOIN(VFE_16, 2, wWidth>>2);*/
	/*vWriteVDOINMsk(VFE_16, (wWidth&0x03)<<6, 0xc0, 1);*/
	vIO32WriteFldAlign(VFE_16, wWidth, DCLK_HSYNC_WIDTH);

}

/**
 * @brief VGA mode manual clock adjusting
 *
 * Set the Htotal when VGA mode manual adjusting
 * @param wHtotal wHtotal is the quantity of pixel clocks per hsync
 * @retval void
 */
void vDrvVGASetClock(u16 wHtotal)
{
	u32 dRealClk;
	u32 dCalClk;

	/*LogSW("wHtotal", wHtotal);*/
	dRealClk = _u4GetLockCw;

	if ((u83(dRealClk) == (u8)14) && (u82(dRealClk) > (u8)0xc0) && (_wVgaClock > wHtotal)) {
		return;
	}

	dRealClk = dRealClk + (0x01000000L);    /*(0x00010000L);*/

	/* Cindy: divide by zero case, should we do some protection?*/
	if (wHtotal == 0) {
		wHtotal = (u16)1;
	}

	dCalClk = (dRealClk / wHtotal) * _wVgaClock;
	vDrvCLKINSetHtotal(wHtotal);
	_u4UiSetCw = dCalClk & 0xffffff00;

	_wVgaClock = wHtotal;
	_bCLKSetFlag |= 0x01;
}

void vDrvInitPhaseVar(void)
{
	u8 bCnt, bchannel;

	for (bchannel = 0; bchannel < (u8)CHANNEL_NUM; bchannel++) {
		for (bCnt = 0; bCnt < (u8)31; bCnt++) {
			_dSumTmpPhase[bchannel][bCnt] = 0;
		}

		_dBestSumALL[bchannel] = 0;
		_bChBestPhase[bchannel] = (u8)0xff;
	}

}


#if Phase3Channel
u8 bDrvADCOutputClock(void)
{
	u8  bMinimum, bMini_Index, bCnt, i;
	u8  bOrderPhase[6], bTmpPhase[6], bIndex[6];

	/*The following program is to decide the ADC output clock*/
	/*Step 1: sorting the phase ascending*/
	for (bCnt = 0; bCnt < (u8)CHANNEL_NUM; bCnt++) {
		bTmpPhase[bCnt] = _bChBestPhase[bCnt]; /*copy the 3 best phases to tempary variable*/
		/*  UTIL_Printf("Best phase for 3 channel are CH%u = %u\r\n",bCnt+1,_bChBestPhase[bCnt]);*/
	}

	for (bCnt = 0; bCnt < (u8)CHANNEL_NUM; bCnt++) {
		bMinimum = (u8)0xff;
		bMini_Index = 0;

		for (i = 0; i < (u8)CHANNEL_NUM; i++) {
			if (bTmpPhase[i] < bMinimum) {
				bMinimum = bTmpPhase[i];
				bMini_Index = i;
			}
		}

		bTmpPhase[bMini_Index] = 0xff;
		bIndex[bCnt] = bMini_Index;
		bOrderPhase[bCnt] = bMinimum;
		bIndex[bCnt + CHANNEL_NUM] = bMini_Index; /*extend*/
		bOrderPhase[bCnt + CHANNEL_NUM] = bOrderPhase[bCnt] + 32; /*extend*/
		/* UTIL_Printf("After sorting best phase: bOrderPhase= %u ,bIndex= %u\r\n",*/
		/*bOrderPhase[bCnt],bIndex[bCnt]);*/
	}

	/*Step 2: to check the validation of the three phases*/
	bCnt = 0;
	i = 0;

	do {
		if ((bOrderPhase[i]) > 63) { /*because the maximum extended phase=31+32=63*/
			bCnt = 0;
		} else {
			bCnt = (bOrderPhase[i + 1] < (bOrderPhase[i] + 7)) ? (bCnt + 1) : (0);
			/*UTIL_Printf("check bOrderPhase[i+1]=%u  and bOrderPhase[i]+7=%u , cnt=%u\r\n",*/
			/*bOrderPhase[i+1],(bOrderPhase[i]+7),bCnt);*/
		}

		i++;
	} while ((bCnt < (u8)2) && (i < (u8)5));

	if ((bCnt > 1) && (i > 0) && (i < 5)) { /* i only can be one of 1,2,3,4.*/
		/*UTIL_Printf("successful condition : i= %u\r\n",i);*/
		/*UTIL_Printf("Center Phase=%u, ADC output clock= %u\r\n",bOrderPhase[i-1],(bIndex[i-1]+1) );*/
		return bIndex[i - 1]; /*the value of bIndex[x] only can be 0 or 1 or 2*/
	}
	/*UTIL_Printf("Too big difference for 3 channel : Search condition:i= %u\r\n",i);*/
	return (u8)0xFF;
}
#endif

void vDrvSetBestPhase(void)
{
	u8 bCnt, bPhase_Index;
#if Phase3Channel
	u32 dwTmp[MAX_PHASE_ARRAY], dwTmpMAX;
#endif

#if Phase3Channel

	if (g_u4SrcType == SRC_YBR) {                /*wu*/
		bPhase_Index = 0;
		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0], AS_PHASESEL_RX);
		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0], AS_PHASESEL_GX);
		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0], AS_PHASESEL_BX);
	} else {
		bPhase_Index = bDrvADCOutputClock();

		if (bPhase_Index == (u8)0xff) { /*if no channel clock selected*/
		/*if the difference between the 3 best phase is too big then calculate the average of the 3 channel*/
			/*and set the phase which has the maximum PSNE to the 3 clock*/
			for (bCnt = 0; bCnt < (u8)(MAX_PHASE_ARRAY - 2); bCnt++) {
				dwTmp[bCnt] = _dSumTmpPhase[0][bCnt] + _dSumTmpPhase[1][bCnt]
				+ _dSumTmpPhase[2][bCnt];
				/*UTIL_Printf("Sum of Phase=%u,  PSNE=%u\r\n",bCnt, dwTmp[bCnt]);*/
			}

			dwTmpMAX = 0x00;

			for (bCnt = 0; bCnt < (u8)(MAX_PHASE_ARRAY - 2); bCnt++) {
				if (dwTmp[bCnt] > dwTmpMAX) {
					dwTmpMAX = dwTmp[bCnt];
					bPhase_Index = bCnt;
				}
			}

			_bChBestPhase[0] = bPhase_Index;  /*force to the same phase*/
			_bChBestPhase[1] = bPhase_Index;  /*force to the same phase*/
			_bChBestPhase[2] = bPhase_Index;  /*force to the same phase*/
			bPhase_Index = (u8)1;                                 /*Use clock 1 to be the ADC output clock*/
			/*UTIL_Printf("Force ADC output clock=1, Average Phase= %u\r\n",bPhase_Index);*/
		}

#ifdef CC_VGA_SPEC_PC_TIMING_WINDOWS_PATTERN_AUTO

		if ((Get_VGAMODE_IPH_WID(_bVgaTiming) == 1152) && (Get_VGAMODE_IPV_LEN(_bVgaTiming) == 864)
		&& (Get_VGAMODE_IHF(_bVgaTiming) == 541) && (Get_VGAMODE_IVF(_bVgaTiming) == 60)
		    && (RegReadFldAlign(STA_SYNC0_07, AS_TOP_SUMRD_S) < 0x61A8 ||
		    RegReadFldAlign(STA_SYNC0_08, AS_TOP_SUMGD_S) < 0x61A8 ||
		    RegReadFldAlign(STA_SYNC0_09, AS_TOP_SUMBD_S) < 0x61A8))

		{
			DBG_Printf(DBG_AUTO, "set _bChBestPhase phase, 1152*768@60 windows pattern\r\n");

			for (bCnt = 0; bCnt <= (u8)2; bCnt++) {
				if (_bChBestPhase[bCnt] < 6) {
					_bChBestPhase[bCnt] = _bChBestPhase[bCnt] + 32 - 6;
				} else {
					_bChBestPhase[bCnt] = _bChBestPhase[bCnt] - 6;
				}
			}
		}

#endif

		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0] , AS_PHASESEL_RX);
		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[1]  , AS_PHASESEL_GX);
		vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[2]  , AS_PHASESEL_BX);
	}

	switch (bPhase_Index) {
	case 0:
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, (u32)1, RG_CLKOSEL_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_2);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_3);

	case (u8)1:
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, (u32)1, RG_CLKOSEL_2);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_3);

	case (u8)2:
	default:
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, (u32)1, RG_CLKOSEL_3);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_1);
		vIO32WriteFldAlign(REG_VGA_Normal_CFG8, 0, RG_CLKOSEL_2);
		break;
	}

#else
	bPhase_Index = 0;

	for (bCnt = 0; bCnt < CHANNEL_NUM; bCnt++) {
		if (_bChBestPhase[bCnt] <= 0x1f) {
			bPhase_Index |= (0x01 << bCnt);
		}
	}

	if (_bChBestPhase[0] > 0x1f) { /*to avoid the unreasonable phase*/
		_bChBestPhase[0] = 0x10;
	}

	vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0] , AS_PHASESEL_RX);
	vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0]  , AS_PHASESEL_GX);
	vIO32WriteFldAlign(ASYNC_0F, _bChBestPhase[0]  , AS_PHASESEL_BX);
#endif
}

void vPhaseSearchCore_New(void)
{
	u32 dTmp;
	u8 bRealPhase, a , b;
	u8 bChannel_num;

	_bCurPhase = (_bBestPhase == 31) ? 0 : (_bCurPhase + 1);
	dTmp = dVGAGetAllDiffValue();
	_bBestPhase = 0;
	bRealPhase = (_bCurPhase == 0) ? 31 : (_bCurPhase - 1);

	for (bChannel_num = 0; bChannel_num < (u8)CHANNEL_NUM; bChannel_num++) {
		if (bRealPhase < (u8)MAX_PHASE_ARRAY) {
			_dSumTmpPhase[bChannel_num][bRealPhase] = _dwPhase3CH[bChannel_num]; /*dTmp ;*/
		}
	}

#if Phase3Channel
	pr_debug("3ch bRealPhase= %2u ,Sum1= %8u ,Sum2= %8u ,Sum3= %8u\r\n",
	(unsigned int)bRealPhase, (unsigned int)_dwPhase3CH[0], (unsigned int)_dwPhase3CH[1],
	(unsigned int)_dwPhase3CH[2]);
#else
	pr_debug("bRealPhase=%u, Sum=%u\r\n", (unsigned int)bRealPhase, (unsigned int)dTmp);
#endif

	if (bRealPhase == 0) {
		a = (u8)31;
		b = 0;
	} else {
		a = bRealPhase - (u8)1;
		b = bRealPhase;
	}

	for (bChannel_num = 0; bChannel_num < (u8)CHANNEL_NUM; bChannel_num++) {
		if ((a < (u8)MAX_PHASE_ARRAY) && (b < (u8)MAX_PHASE_ARRAY)) {
			/*for klocwork check to protect over dimension*/
			if (_dSumTmpPhase[bChannel_num][a] > _dSumTmpPhase[bChannel_num][b]) {
				/*find max slop*/
				dTmp = _dSumTmpPhase[bChannel_num][a] - _dSumTmpPhase[bChannel_num][b];

				if (dTmp > _dBestSumALL[bChannel_num]) {
					_dBestSumALL[bChannel_num] = dTmp;
					_bMaxSlope[bChannel_num]  = a;
				}
			}
		}
	}

	if (_bCurPhase < (u8)32) { /*      (_bCurPhase <= 32)*/
		/*vDrvVGASetPhase((_bCurPhase == 32) ? 0 : _bCurPhase);*/
		vDrvVGASetPhase_Simple((_bCurPhase == 32) ? 0 : _bCurPhase);
		/*_bVgaDelayCnt = 1;*/
	} else {
		for (bChannel_num = 0; bChannel_num < CHANNEL_NUM; bChannel_num++) {
			_dBestSumALL[bChannel_num]  =  0;
			b = (_bMaxSlope[bChannel_num] >= 3) ? (_bMaxSlope[bChannel_num] - 3) :
			(_bMaxSlope[bChannel_num] + 31 - 2); /*wu*/
			/*b = _bMaxSlope[bChannel_num] ;*/
			a = 8;
			pr_debug("CH %u Sorting Phase from %2u,  %2u\r\n", (bChannel_num + 1),
			_bMaxSlope[bChannel_num] , b);
			do {
				if (b < MAX_PHASE_ARRAY) { /*for klocwork check*/
				pr_debug("bChannel_num =%2u, phase=%2u, data=%8u \r\n",
				(unsigned int)(bChannel_num + 1) , (unsigned int)b,
				(unsigned int)_dSumTmpPhase[bChannel_num][b]);

					if (_dSumTmpPhase[bChannel_num][b] > _dBestSumALL[bChannel_num]) {
						_dBestSumALL[bChannel_num] = _dSumTmpPhase[bChannel_num][b];
						_bChBestPhase[bChannel_num] = b;
					}

					b = (b == 0) ? 31 : (b - 1);
					a--;
				} else { /*for klocwork check*/
					a = 0;
				}
			} while (a > 0);
		}
	}
}

void vYPbPrPhaseIsr_New(void)    /*NEW_PHASE_ALGORITHM*/
{
	if (_bCurPhase < (u8)32) { /* do use <=32*/
		vPhaseSearchCore_New();
		_RETIMENeedReset = TRUE;
	} else {      /* if(_bCurPhase >31 )*/
		vDrvSetBestPhase();
		_RETIMENeedReset = TRUE;
		vClrAutoFlg(SP0_AUTO_PHASE);
		vSetSP0AutoState(VDO_AUTO_NOT_BEGIN);
#if CHANGE_SDDS_KPI
		vDrvEnableChang_SDDS_BW();
#endif
		_bVgaDelayCnt = (u8)2;
		_bAutoISR = 0;
#if Phase3Channel
		_bChBestPhase[0] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
		_bChBestPhase[1] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_GX);
		_bChBestPhase[2] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_BX);
		pr_debug("YPbPr ch1 phase=%2u ,ch2 phase=%2u ,ch3 phase=%2u\r\n",
		_bChBestPhase[0], _bChBestPhase[1], _bChBestPhase[2]);
#else
		_bChBestPhase[0] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
		pr_debug("YPbPr phase=%2u\r\n", _bChBestPhase[0]);
#endif

	}
}

void vVgaPhaseIsr_New(void)
{
	if (_bCurPhase < (u8)32) { /* do use <=32*/
		vPhaseSearchCore_New();
		_RETIMENeedReset = TRUE;
	} else {      /* if(_bCurPhase >31 )*/
		vDrvSetBestPhase();
		_RETIMENeedReset = TRUE;
		vClrAutoFlg(SP0_AUTO_PHASE);

		vSetSP0AutoState(VDO_AUTO_POSITION_1_START);
		vVgaAutoPosInit();
#if CHANGE_SDDS_KPI
		vDrvEnableChang_SDDS_BW();
#endif
		_bVgaDelayCnt = (u8)2;
		_bAutoISR = 0;
#if Phase3Channel
		_bChBestPhase[0] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
		_bChBestPhase[1] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_GX);
		_bChBestPhase[2] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_BX);
		pr_debug("VGA ch1 phase=%2u ,ch2 phase=%2u ,ch3 phase=%2u\r\n",
		_bChBestPhase[0], _bChBestPhase[1], _bChBestPhase[2]);
#else
		_bChBestPhase[0] = IO32ReadFldAlign(ASYNC_0F, AS_PHASESEL_RX);
		pr_debug("VGA phase=%2u\r\n", _bChBestPhase[0]);
#endif

	}
}




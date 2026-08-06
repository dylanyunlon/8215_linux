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

#ifndef __ARM2__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
//#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include "windows.h"
#include "x_debug.h"
#include "x_stl_lib.h"
#else
#include "x_types.h"
//#include "drv_av_d.h"
#include "display_inc.h"
#endif
#include "x_assert.h"
/*#include "x_hal_926.h"*/
#include "x_bim.h"
#include "x_os.h"
#include "x_lint.h"
#include "tcon.h"
#include "log.h"
#include "tcon_reg.h"
#include "tcon_def.h"
#include "x_printf.h"


#define DEFINE_IS_LOG	CLI_IsLog
#include "x_bim.h"

unsigned int gdty_cyc = 0;
__u32	_YCProcTestTbl[25];
static LVDS_OUTPUT_MODE_E _eLVDS_Output_Mode = LVDS_OUTPUT_MODE_8BITS;
extern unsigned long IO_BASE_BRINGUP;
#define vWriteScale(dAddr, dVal)  (*(volatile __u32*)(IO_BASE_BRINGUP + 0x3000 + dAddr) = dVal)

static unsigned char g_pbPanelGamma[64] = {
	0, 1, 3, 5, 7, 9, 11, 13,
	15, 17, 20, 23, 25, 28, 31, 34,
	37, 40, 43, 47, 51, 56, 61, 66,
	71, 76, 82, 87, 92, 97, 102, 107,
	112, 118, 123, 128, 133, 138, 143, 148,
	153, 158, 163, 167, 172, 177, 182, 186,
	191, 195, 200, 204, 208, 212, 216, 220,
	224, 228, 232, 236, 240, 244, 248, 252
};

#ifdef __ARM2__
static void _msleep(unsigned int cnt)
{
#ifdef _3365VERIFY
	int i =0, i4result =0;

	for (i =0; i< (cnt * 0x10000); i++)
	{
	   i4result ++;
	}
#endif
}
#endif

/* **********************************************************************/
/* Function : void vPanelSetGammaGain (__u8 bLevel)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
void vPanelSetGammaGain(__u8 bLevel)
{
	__u8 bData, bData1, bData2;

	if (bLevel == 0xFF) {
		bData = 0x00;
		bData1 = 0x00;
		bData2 = 0x00;
	} else {
		/*base #ifdef GAMMA_USE_BINARY_TABLE*/
		/*base   bData =  bFlashData(_dwTconPara + R_GAIN_OFFSET);*/
		/*base   bData1 =  bFlashData(_dwTconPara + G_GAIN_OFFSET);*/
		/*base   bData2 =  bFlashData(_dwTconPara + B_GAIN_OFFSET);*/
		/*base #else*/
		bData = _pbPanelGain[bLevel];
		bData1 = _pbPanelGain[bLevel];
		bData2 = _pbPanelGain[bLevel];
		/*base #endif*/
	}

	vWritePGMAMsk(RW_PGMA_GAIN, bData, R_GAIN);
	vWritePGMAMsk(RW_PGMA_GAIN, bData1 << 8, G_GAIN);
	vWritePGMAMsk(RW_PGMA_GAIN, bData2 << 16, B_GAIN);
}


/* **********************************************************************/
/* Function :void vPanelSetBackLight(void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
/*-Brightness: 16 steps, //Set PWM output*/
void vPanelSetBackLight(bool fgEnable)
{
	__u32 dwPWM;

	dwPWM = dReadFMT(0x01E4); /*(0x0194);*/

	if (fgEnable) { /*bit 0 , enable*/
		/*dwPWM |= 0x01;*/
		dwPWM = 0x5f2f0001;
	} else {
		dwPWM &= ~0x01;
	}

	vWriteFMT(0x01E4, dwPWM); /*vWriteFMT(0x0194, dwPWM);*/
	vWriteFMTMsk(0x01EC, 0x01, ((1 << 0) | (0 << 1)));
}


/*set clk to 29Mhz*/
void vScalerSetClk(void)
{
	vWriteScale(0x00, 0x00000001);
	vWriteScale(0x00, 0x00506401);
	vWriteScale(0x00, 0x00506407);
	vWriteScale(0x00, 0x00506401);
	vWriteScale(0x00, 0x00522E01);
	vWriteScale(0x00, 0x00522E07);
	vWriteScale(0x00, 0x00522E01);
	vWriteScale(0x00, 0x00522E00);
}



/* **********************************************************************/
/* Function :void vPanelSetBright(void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
/*-Brightness: 16 steps, //Set PWM output*/
void vPanelSetBright(void)
{
	__u8 bBright;
	/*base  __s8 cTmp;*/
	/*base  __u32 dwPWM;*/
	/*base  __u32 dwTmp;*/
	/*base  __u32 dwVcom, dwVcomH, dwVcomL;*/

	/*base  cTmp = bSharedInfo(SI_PANEL_BRIGHTNESS);  //-20~+20*/
	/*base  dwTmp = (__u32)cTmp;*/
	/*base  #ifdef GAMMA_USE_BINARY_TABLE*/
	/*base    bBright = (__u8)(dwTmp * 6 + bFlashData(_dwTconPara + BRIGHT_OFFSET));*/
	/*base  #else*/
	bBright = 0x80; /*(__u8)(dwTmp * 6 + 0x80);*/
	/*base  #endif*/
	/*base  #ifdef PANEL_BRIGHT_USE_COLORP*/
	vWritePCLRPMsk(RW_PCLRP_BRIGHT_CONT, bBright << 16, BRIGHT_GAIN);
	/*base  #elif defined PANEL_BRIGHT_USE_INVERTER*/
	/*dwPWM = (dReadFMT(0x0194)& 0xFF00FFFF);*/
	/*base  dwPWM = 0xff000709| (bBright<<16);    //PWM 2.08K, need fine tune according to inverter board.*/
	/*dwPWM |= 0x01;*/
	/*base  vWriteFMT(0x0194,dwPWM);*/
	/*     vPanelSetBackLight(TRUE);*/
	/*base  #else // PANEL_BRIGHT_USE_VCOM*/

	/*base  dwVcomL = _dwVcom & 0xff;*/
	/*base  dwVcomH = (_dwVcom>>8) & 0xff;*/

	/*base  dwVcomH -= dwTmp;*/
	/*base  dwVcomL += dwTmp;*/

	/*base  dwVcom = _dwVcom & 0xffff0000;*/
	/*base  dwVcom = dwVcom | ((dwVcomH<<8) & 0x0000ff00) |(dwVcomL & 0x000000ff);*/
	/*base  vWritePTCON(0x08,dwVcom);*/
	/*base  #endif*/

}


/*#define I_800_480*/
#define	P_800_480

void vSetScalerParam(void)
{
#if 0
	*(volatile __u32 *)(IO_BASE_ADDRESS + (0x2c48)) = 0x10708341;
	vIrq1On(INT_PSCLR);
#endif

#ifdef	I_800_480
	vWritePSCL(0x14, 0x080000);
	vWritePSCL(0x1c, 0x718000);
	vWritePSCL(0x30, 0x2700);

	vWritePFMT(0x8c, 0x3A0020d);
	vWritePFMT(0x90, 0x0600024);
	vWritePFMT(0xa0, 0x210347);
	vWritePFMT(0xa4, 0xa01ea);
	vWritePFMT(0xa8, 0xa01ea);
	vWritePFMT(0xb0, 0x0e50041);
	vWritePFMT(0xdc, 0x0a000a);
	vWritePFMT(0xe8, 0x35a035a);
	vWritePFMT(0xac, 0x1000000);
#endif

#ifdef	P_800_480
#if 0
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x1094) = 0x14148305;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x10a0) = 0x71034e;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x10a4) = 0x2b020a;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x10a8) = 0x2b020a;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x0c14) = 0x4c;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x2c48) = 0x10708341;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x0c3c) = 0xff;
	*(volatile __u32 *)(IO_BASE_ADDRESS + 0x0c3c) = 0x00;
#endif

	vWritePSCL(0x14, 0x100000);
	vWritePSCL(0x1c, 0x518000);
	vWritePSCL(0x30, 0x2700);

	vWritePFMT(0x8c, 0x3A0020d);
	vWritePFMT(0x90, 0x600022);
	vWritePFMT(0xa0, 0x210347);
	vWritePFMT(0xa4, 0xa01ea);
	vWritePFMT(0xa8, 0xa01ea);
	vWritePFMT(0xb0, 0x0e50047);
	vWritePFMT(0xdc, 0x0a000a);
	vWritePFMT(0xe8, 0x35a035a);
	vWritePFMT(0xac, 0x1000000);
#endif

}


/* **********************************************************************/
/* Function : void vTCONReset (bool fgEnable, bool fgFillTable)*/
/* Description : Fill TCON register, enable TCON or not.*/
/*               current tv system and output mode*/
/* Parameter : bool fgEnable :enable TCON or not, bool fgFillTable:Fill Table or not*/
/* Return    : None*/
/* **********************************************************************/

/*PWM Begin*/
__u32 _aBackLight[] = {
	0x50210 , 0x06503205,
	0x50218 , 0x00000100,
	0x50208 , 0x0cb00005,
	0x50218 , 0x00000010,
};

extern unsigned long IO_BASE_BRINGUP;
#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)) = val)
#define ReadREG(arg)       (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))




__u32 _aCMMSetting[] = {

	/*begin[wts]*/

	0x000000d0 , 0xffffffff ,/*; rst enable*/
	0x0000005c , 0x00400040 ,/*; [6]: 1 -- select dclk output*/

	/*end[wts]*/

	/*------------------------------------------*/
	/* pwm begin*/
	/*------------------------------------------*/
	0x00032200 , 0x06503205,
	0x00032218 , 0x00000003,
	0x00032204 , 0x0CB02205,
	0x00032218 , 0x0000000F,
	/*------------------------------------------*/
	/* pwm begin*/
	/*------------------------------------------*/
	/*FPD TCON*/
	0x000a4700 , 0x03180700,
	0x000a4704 , 0x31ffffff,
	0x000a4724 , 0x2b000000,

	0x000a4804 , 0x0104017e,
	0x000a4820 , 0x28000001,
	0x000a4824 , 0x001f715f,
	0x000a4830 , 0x0006800e,
	0x000a4834 , 0x00000001,
	0x000a48e0 , 0x0000247d,/*; ck sel & fpd En*/
	0x000a48e4 , 0x0005f51a,
	0x000a48f0 , 0x00000002,

	0x00000014 , 0x00600000 ,/* fpd_ap_sel[2:0]   //180MHz/6=30MHz*/

	/*Color Process*/
	/*0x0001f080 , 0x00000001; open Color Process*/
	/*0x00042604 , 0xff400000; test Color Process Fix Y*/

	/*FMT*/
	0x00042094 , 0x14148c20 ,/* SD mode select*/
	0x0004209c , 0x000002d0 ,/* pixel number per line*/
	0x000420a0 , 0x00680337 ,/* H active area*/
	0x000420a4 , 0x002c020b ,/* V Odd active area*/
	0x000420a8 , 0x002c020b ,/* V Even active area*/
	0x000420b0 , 0x01000001 ,/* H scaler 1:1*/
	0x000420b4 , 0x0080f010 ,/* build in color blue*/
	0x000420b8 , 0x00808010 ,/* back ground color*/
	0x000420d0 , 0x806d033c ,/* CCIR H active area*/
	0x000420d4 , 0x03b9020d ,/* H Total & VTotal*/
	0x000420e0 , 0x002c020d ,/* CCIR V Odd*/
	0x000420f0 , 0x002c020d ,/* CCIR V Even*/

	0x000420ac , 0x00000403 ,/* FMT En & Reset*/
	0x000420ac , 0x00000003 ,/**/
	/* 0x0004213c , 0x000000ff ,// VDO Reset*/
	/* 0x0004213c , 0x00000000 ,//*/

	0x00010000 , 0x00432103 ,/**/

	/*SCL 720_480/800_480*/
	0x00010000 , 0x00432102 ,/* source from scl*/
	0x000a48e0 , 0x00002475 ,/* scaler Ck sel*/

	0x000a4514 , 0x00100000 ,/* V Step*/
	0x000a451c , 0x00510000 ,/* Vdn=1 & Hdn=0 & V 16 phase interpolation*/
	0x000a4530 , 0x00002603 ,/* prgs mode*/
	0x000a46b0 , 0x00e60041 ,/* H step & FIR & Scl En*/
	0x000a468c , 0x03b9020d ,/* HTotal & VTotal*/
	0x000a4690 , 0x00100002 ,/* shift active H&V start*/
	0x000a469c , 0x000002d0 ,/* input pixel length per line*/
	0x000a46a0 , 0x0040035f ,/* H active area start & end(320)*/
	0x000a46a4 , 0x002c020b ,/* V Odd active area(1E0)*/
	0x000a46a8 , 0x002c020b ,/* V Even active area(1E0)*/
	/*end*/
};


void vLoadCMMSetting(void)
{
	__s32 i;

	for (i = 0; i < sizeof(_aCMMSetting) / 8; i++) {
		WriteREG(_aCMMSetting[i  * 2], _aCMMSetting[i * 2 + 1]);
	}
}

void vTCONReset(bool fgEnable, bool fgFillTable, bool fgHwReset)
{
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "fgHwReset\r\n");
#endif

	if (fgHwReset) {
#ifdef FB_DEBUG
		FB_PRINT(FB_LOG_LVL_DBG, "fgHwReset1\r\n");
#endif
		__s32 i;

		/* Sleep(1000);*/

		for (i = 0; i < sizeof(_aBackLight) / 8; i++) {
			/*WriteREG(_aBackLight[i  * 2], _aBackLight[i * 2 + 1]);*/
		}

	}

	vWritePGMAMsk(0x80, 0x20, 0xf0);/* output fmt set to 8 bit */
}
EXPORT_SYMBOL(vTCONReset);

void vTconTimingInput(__s32 i4En, __s32 i4HStart, __s32 i4HEnd, __s32 i4VStart, __s32 i4VEnd)
{
#if defined(FEATURE_TCON_INPUTTIMING)
	vWritePCLRPMsk(RW_PCLRP_PTNGEN_L, (i4En % 2) << MLC_TIMINF_IN_EN_SHF, MLC_TIMINF_IN_EN);
	vWritePTCONMsk(RW_PTCON_GLB0, ((i4En % 2) << TCON_EN_SHF) | ((i4En % 2) << TCKA_EN_SHF), TCON_EN | TCKA_EN);
	vWritePTCONMsk(RW_PTCON_GLB1, ((i4En % 2) << TCON_OUT0_EN_SHF), TCON_OUT0_EN);

	if (i4En % 2) {
		vWritePTCONMsk(RW_PTCON_OUT0_H
			       , ((i4HStart & 0X7ff) << TCON_OUT0_HS_SHF) | ((i4HEnd & 0X7ff) << TCON_OUT0_HE_SHF)
			       , TCON_OUT0_HS | TCON_OUT0_HE);
		vWritePTCONMsk(RW_PTCON_OUT0_V
			       , ((i4VStart & 0X7ff) << TCON_OUT0_VS_SHF) | ((i4VEnd & 0X7ff) << TCON_OUT0_VE_SHF)
			       , TCON_OUT0_VS | TCON_OUT0_VE);
		FB_PRINT(FB_LOG_LVL_DBG, "TCON Timing Input Enable&ActiveArea(%d,%d,%d,%d)\r\n", (int)i4HStart
			 , (int)i4HEnd, (int)i4VStart, (int)i4VEnd);
	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "TCON Timing Input Disable\r\n");
	}

#else
	FB_PRINT(FB_LOG_LVL_DBG, "Feature not support!\r\n");
#endif
}
EXPORT_SYMBOL(vTconTimingInput);

/* **********************************************************************/
/* Function : void vPanelSetGamma (__u8 bLevel)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
/*-Gamma: 3 levels //bLevel : 0~2*/

#define	GAMMA_DEBUG

#ifndef	GAMMA_DEBUG
void vPanelSetGamma(void)
{
	__u32 dwData, dwData1, dwAddr;
	__u8 bIndex, bLevel , bData;

	/*dwAddr = dwGetUsrElementAddr(5) - FLASHNC_SA;*/
	/*base    dwAddr = _dwTconPara;*/

#ifndef PANEL_DIGITAL_6BIT
	bLevel = 0; /*bSharedInfo(SI_PANEL_GAMMA);*/
	/* #ifdef PANEL_AUO
	if(bLevel==SV_PQ_GAMMA_HIGH)
	{
	_dwVcom = 0x0020E434;
	}
	else if(bLevel==SV_PQ_GAMMA_MID)
	{
	_dwVcom = 0x0020E434;
	}
	else if(bLevel == SV_PQ_GAMMA_LOW)
	{
	_dwVcom = 0x0020cf49;
	}
	else
	{
	_dwVcom = 0x0020cf49;
	}
	#else
	_dwVcom = _TCON[2];
	#endif
	*/

	if (bLevel != SV_PQ_GAMMA_NONE) {
		vWritePGMAMsk(RW_PGMA_CTRL0, ACCESS_GAMMA, ACCESS_GAMMA);

		for (bIndex = 0; bIndex < 64; bIndex++) {
			dwData1 = _pbPanelGamma[bLevel][bIndex] << 16;
			dwData = dwData1 + bIndex;
			vWritePGMA(RW_PGMA_R_GAMMA, dwData);
			vWritePGMA(RW_PGMA_G_GAMMA, dwData);
			vWritePGMA(RW_PGMA_B_GAMMA, dwData);
		}

		vWritePGMAMsk(RW_PGMA_CTRL0, 0x00 << 1, ACCESS_GAMMA);
		vWritePGMAMsk(RW_PGMA_CTRL0, GAMMA_ON, GAMMA_ON | BYPASS_GAMMA); /*turn on gamma*/
	} else
#else
	{
		vWritePGMAMsk(RW_PGMA_CTRL0, BYPASS_GAMMA, BYPASS_GAMMA); /*turn on gamma*/
	}

#endif
	vWritePTCON(RW_PTCON_GLB0 + 8, 0x0020cf49); /*_dwVcom);*/
	vPanelSetBright();
}
#else
#if 0
void vPanelSetGamma(__u8 bLevel)
{
	__u32 dwData, dwData1;
	__u8 bIndex;

	if (bLevel > SV_PQ_GAMMA_NONE) {
		return;
	}

	if (bLevel != SV_PQ_GAMMA_NONE) {
		vWritePGMAMsk(RW_PGMA_CTRL0, ACCESS_GAMMA, ACCESS_GAMMA);

		for (bIndex = 0; bIndex < 64; bIndex++) {
			dwData1 = _pbPanelGamma[bLevel][bIndex] << 16;
			dwData = dwData1 + bIndex;

			vWritePGMA(RW_PGMA_R_GAMMA, dwData);
			vWritePGMA(RW_PGMA_G_GAMMA, dwData);
			vWritePGMA(RW_PGMA_B_GAMMA, dwData);

		}

		vWritePGMAMsk(RW_PGMA_CTRL0, 0x00 << 1, ACCESS_GAMMA);
		vWritePGMAMsk(RW_PGMA_CTRL0, GAMMA_ON, GAMMA_ON | BYPASS_GAMMA); /*turn on gamma*/
	}

	/*    vWritePTCON(RW_PTCON_GLB0 + 8, 0x0020cf49); //_dwVcom);*/

	/*    vPanelSetBright();*/

}
#else
void vPanelSetGamma(__u8 *pu4GammaData)
{
	__u32 dwData;
	__u8 bIndex;


	if (NULL == pu4GammaData) {
		return;
	}

	vWritePGMAMsk(RW_PGMA_CTRL0, ACCESS_GAMMA, ACCESS_GAMMA);

	for (bIndex = 0; bIndex < 64; bIndex++) {
		FB_PRINT(FB_LOG_LVL_DBG, "[Gamma Read]Index %d : %d \r\n", bIndex, *(pu4GammaData + bIndex));
		dwData = (pu4GammaData[bIndex] << 16) + bIndex;

		vWritePGMA(RW_PGMA_R_GAMMA, dwData);
		vWritePGMA(RW_PGMA_G_GAMMA, dwData);
		vWritePGMA(RW_PGMA_B_GAMMA, dwData);

	}

	vWritePGMAMsk(RW_PGMA_CTRL0, 0x00 << 1, ACCESS_GAMMA);
	vWritePGMAMsk(RW_PGMA_CTRL0, GAMMA_ON, GAMMA_ON | BYPASS_GAMMA); /*turn on gamma*/

}

#endif
#endif

__u8 *g_u1va = 0;

void vPanelResumeGamma(void)
{
	__u32 dwData;
	__u8 bIndex;

	g_u1va = (__u8 *)FB_PHYSICAL_TO_VIRTUAL(ARM2_FBDRV_SHARE_GAMMA_PA);
	vWritePGMAMsk(RW_PGMA_CTRL0, ACCESS_GAMMA, ACCESS_GAMMA);

	for (bIndex = 0; bIndex < 64; bIndex++) {
		FB_PRINT(FB_LOG_LVL_DBG, "[Gamma Read]Index %d : %d \r\n", bIndex, *(g_u1va + bIndex));
		dwData = (g_u1va[bIndex] << 16) + bIndex;

		vWritePGMA(RW_PGMA_R_GAMMA, dwData);
		vWritePGMA(RW_PGMA_G_GAMMA, dwData);
		vWritePGMA(RW_PGMA_B_GAMMA, dwData);

	}

	vWritePGMAMsk(RW_PGMA_CTRL0, 0x00 << 1, ACCESS_GAMMA);
	vWritePGMAMsk(RW_PGMA_CTRL0, GAMMA_ON, GAMMA_ON | BYPASS_GAMMA); /*turn on gamma*/

}

void vPanelSetSCE(void)
{
	__u32 dwData;
	WORD wCnt;
	__u8 bCnt;

	vWritePCLRPMsk(RW_PCLRP_HUE_SCECTRL, 0x02, 0x02); /* SEC sram write mode*/

	for (wCnt = 0; wCnt <= 359; wCnt++) {
		dwData = (_pdTestSCETable[wCnt] << 10) | (wCnt << 1); /**/
		/*dwData = (0x200080<<10) |(wCnt<<1); //*/
		vWritePCLRP(RW_PCLRP_SCE_TABLE, dwData | 0x01); /*write vector and write bit*/

		for (bCnt = 0; bCnt <= 10; bCnt++) { /*delay??*/
			dReadPCLRP(0x0c);
		}

		vWritePCLRP(RW_PCLRP_SCE_TABLE, dwData | 0x00); /*write vector and clear write bit*/

		for (bCnt = 0; bCnt <= 10; bCnt++) { /*delay??*/
			dReadPCLRP(0x0c);
		}
	}

	vWritePCLRPMsk(RW_PCLRP_HUE_SCECTRL, 0x01, 0x01); /* SEC enable,*/
}


/* **********************************************************************/
/* Function : void vPanelSetGammaOffset (__u8 bLevel)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
void vPanelSetGammaOffset(__u8 bLevel)
{
	__u32 dwData, dwData1, dwData2;

	dwData = _pdPanelOffset[bLevel];
	dwData1 = _pdPanelOffset[bLevel];
	dwData2 = _pdPanelOffset[bLevel];

	vWritePGMAMsk(RW_PGMA_OFFSET0, dwData , R_OFFSET);
	vWritePGMAMsk(RW_PGMA_OFFSET0, dwData1 << 16, G_OFFSET);
	vWritePGMAMsk(RW_PGMA_OFFSET1, dwData2, B_OFFSET);
}


/* **********************************************************************/
/* Function :void vPanelSetSaturation(void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
void vPanelSetSaturation(void)
{
	__u8 bSat;

	bSat = (__u8)0x80; /*(dwTmp * 10 + 0x80);*/

	vWritePCLRPMsk(RW_PCLRP_SATURATION, bSat, SAT_GAIN);
	/*0x00 (gain = 0) - 0x80 (gain = 1.0) - 0xFF (gain=2.0). Saturation gain = setting/0x80.*/
}


/*bLevel : 0~2*/
/* **********************************************************************/
/* Function :void vPanelSetUVDelay(void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
void vPanelSetUVDelay(void)
{
	vWritePCLRPMsk(RW_PCLRP_CHROMA_U, 0x07, DELAY_U);
	vWritePCLRPMsk(RW_PCLRP_CHROMA_V, 0x04, DELAY_V);
}


/* **********************************************************************/
/* Function :void vPanelSetContrast(void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
/*-Contrast: 16 steps remember Contrast: -16 ~ + 16 level.*/
void vPanelSetContrast(void)
{
	__u8 bContrast;

	bContrast = (__u8)0x40; /*(dwTmp * 2 + 0x40);*/
	vWritePCLRPMsk(RW_PCLRP_BRIGHT_CONT, bContrast, CONTRAST_GAIN);
}


/* **********************************************************************/
/* Function :void vPanelSetHue (void)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
/*-Hue: remember Hue: - 9 ~ + 9 level. //16 should be enough*/
void vPanelSetHue(void)
{
	__u8 bHue;

	bHue = (__u8)0x20; /*(dwTmp * 3 + 0x20);*/
	vWritePCLRPMsk(RW_PCLRP_HUE_SCECTRL, (bHue << 8), HUE_DEGREE);
}


/*Color P*/
/* **********************************************************************/
/* Function :void vPanelSetCTI (__u8 bLevel)*/
/* Description :*/
/* Parameter : __u8 bLevel*/
/* Return    : None*/
/* **********************************************************************/
void vPanelSetCTI(__u8 bLevel)
{
	vWritePCLRPMsk(RW_PCLRP_CTI, bLevel, CTI_T_SELECT);
}


void vPanelSet(void)
{
#ifndef	GAMMA_DEBUG
	vPanelSetGamma();
#else
	vPanelSetGamma(NULL);
#endif

	vPanelSetGammaOffset(0);
	vPanelSetSaturation();
	vPanelSetUVDelay();
	vPanelSetContrast();
	vPanelSetBright();
	vPanelSetHue();
	vPanelSetCTI(0x03);
}



void vGetDefaultYCPrco(void)
{
	__u8 bIndex;

	for (bIndex = 0; bIndex < 0x19; bIndex++) {
		_YCProcTestTbl[bIndex] = dReadPCLRP(bIndex * 4);
	}
}


void vSetDefaultYCPrco(void)
{
	__u8 bIndex;

	for (bIndex = 0; bIndex < 0x19; bIndex++) {
		vWritePCLRP(bIndex * 4, _YCProcTestTbl[bIndex]);
	}
}


void vPanelInvert(__u8 bLR, __u8 bUD)
{
	vWritePTCONMsk(RW_PTCON_GLB1, (bLR << 16) | (bUD << 17), (TCON_LR) | (TCON_UD));
}

/*not ready,need to check register and value......*/
void vTConPatternGnrtr(__u8 bPattenType)
{
	switch (bPattenType) {
	case 21:
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : Index  0 \r\n");
		vWritePCLRP(0x00, 0x100000);
		vWritePCLRP(0x18, 0x4100000);
		/*vWritePCLRP(0x60, 0x12d20);*/
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : End \r\n");
		break;

	case 22:
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : Index  1 \r\n");
		vWritePCLRP(0x00, 0x3100000);
		vWritePCLRP(0x18, 0x1C700000);
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : End \r\n");
		break;

	case 23:
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : Index  2 \r\n");
		vWritePCLRP(0x00, 0x30100000);
		vWritePCLRP(0x18, 0xC7100000);
		FB_PRINT(FB_LOG_LVL_DBG, "Tcon  Pattern : End \r\n");
		break;
	}

}

/*0x30~0x36 LPGPP*/
/*0x37~0x3D CPGPP*/
/*0x3E			LCPGPP*/
void vTConGeneralPro(__u8 bType)
{

	/*	vSetDefaultYCPrco();*/
	FB_PRINT(FB_LOG_LVL_DBG, "Tcon  vTConGeneralPro : %d \r\n", bType);

	switch (bType) {
	case 0x30:

		vWritePCLRP(0x04, 0x5);
		vWritePCLRP(0x08, 0x801);
		break;

	case 0x31:
		vWritePCLRP(0x04, 0x3);
		vWritePCLRP(0x08, 0x801);
		break;

	case 0x32:
		vWritePCLRP(0x04, 0x0);
		vWritePCLRP(0x08, 0x1001);
		break;

	case 0x33:
		vWritePCLRP(0x04, 0x1f8);
		vWritePCLRP(0x08, 0x801);
		break;

	case 0x34:
		vWritePCLRP(0x04, 0x400000);
		vWritePCLRP(0x08, 0x801);
		break;

	case 0x35:
		vWritePCLRP(0x04, 0x200000);
		vWritePCLRP(0x08, 0x1801);
		break;

	case 0x36:
		vWritePCLRP(0x04, 0x7F000);
		vWritePCLRP(0x08, 0x801);
		break;

	case 0x37:
		vWritePCLRP(0x1C, 0x05);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x05);
		break;

	case 0x38:
		vWritePCLRP(0x1C, 0x03);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x03);
		break;

	case 0x39:
		vWritePCLRP(0x1C, 0x190);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x190);
		break;

	case 0x3A:
		vWritePCLRP(0x1C, 0x00);
		vWritePCLRP(0x24, 0x10011001);
		vWritePCLRP(0x20, 0x00);
		break;

	case 0x3B:
		vWritePCLRP(0x1C, 0x32000);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x32000);
		break;

	case 0x3C:
		vWritePCLRP(0x1C, 0x19400000);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x19400005);
		break;

	case 0x3D:
		vWritePCLRP(0x1C, 0x200000);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x200000);
		break;

	case 0x3E:
		vWritePCLRP(0x04, 0x200000);
		vWritePCLRP(0x08, 0x1801);
		vWritePCLRP(0x1C, 0x200000);
		vWritePCLRP(0x24, 0x8010801);
		vWritePCLRP(0x20, 0x200005);
		break;

	default:
		break;
	}

}


void vTConPanelBWLevel(__u8 bType)
{
	if (WHITEENHANCE == bType) {
		vWritePCLRP(0x00, 0x100000);
		vWritePCLRP(0x1C, 0x400000);
		vWritePCLRP(0x20, 0x400000);
		vWritePCLRP(0x4C, 0x8010803F);
	} else if (BLACKENHANCE == bType) {
		vWritePCLRP(0x00, 0x100000);
		vWritePCLRP(0x1C, 0x400000);
		vWritePCLRP(0x20, 0x400000);
		vWritePCLRP(0x50, 0x80118001);
	}
}


void vTconSetDEMd(void)
{
#if (IS_FPGA_VER == 1)
	vWritePGMAMsk(0x24, 1 << 29, 1 << 29);
#endif

	vWritePTCON(0x90, 0x4c096);
	vWritePTCON(0x94, 0x1f0010);
	/*if scaler is not work ok set bit[19] to 1*/
	vWritePTCON(0xe0, 0x0008041D); /*bit[19]  must be 1*/
	/*else scaler is work ok clr bit[19]*/
	/*vWritePTCON(0xe0,0x0000041D);*/
}

void vTconSetSYNCMd(void)
{
#if (IS_FPGA_VER == 1)
	vWritePGMAMsk(0x24, 0 << 29, 1 << 29);
#endif

	vWritePTCON(0x20, 0x1400000e); /*adj_vsyn*/
	vWritePTCON(0x24, 0x000001fa); /*adj_vsyn line position*/
	vWritePTCON(0x70, 0x001ff000); /*vsyn position*/
	vWritePTCON(0x30, 0x0006803e); /*hsync*/
	vWritePTCON(0x34, 0x00000001); /*hsync*/
	vWritePTCON(0xe0, 0x0033041d); /*[19]-adj_de,[20]-adj_hsyn,[21]-adj_vsyn*/
}


void vTconSetSclDEMd(void)
{
#if (IS_FPGA_VER == 1)
	vWritePGMAMsk(0x24, 1 << 29, 1 << 29);
#endif
	vWritePTCON(0xe0, 0x0000041D);
}

void vTconSetSclSYNCMd(void)
{
#if (IS_FPGA_VER == 1)
	vWritePGMAMsk(0x24, 0 << 29, 1 << 29);
#endif

	vWritePTCON(0xe0, 0x0003041d);
}
#define  TCON_BRIGHTNESS_MASK   0xFF0000
#define  TCON_CONTRAST_MASK        0xFF
#define  BACKLIGHT_MASK                  0xFFF00
#define   TCON_SATURATION_MASK    0xFF
#define   TCON_HUE_MASK                  0x3F00


#define  TCON_BRIGHTNESS_CONTRAST_OFFSET 0x0C
#define  TCON_SATURATION_OFFSET                       0x2C
#define  TCON_HUE_OFFSET                                     0x30


#define MAX_BK_PWM_V        0X35



bool IsLVDS8Bit(void)
{
	bool fgLVDS8Bit = TRUE;

	switch (_eLVDS_Output_Mode) {
	case LVDS_OUTPUT_MODE_AUTO:
		fgLVDS8Bit = (ReadREG(0x180) & 0x80) ?   TRUE : FALSE;
		break;

	case LVDS_OUTPUT_MODE_6BITS:
		fgLVDS8Bit = FALSE;
		break;

	case LVDS_OUTPUT_MODE_8BITS:
		fgLVDS8Bit  = TRUE;
		break;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "IsLVDS8Bit %d\r\n", (int)fgLVDS8Bit);

	return fgLVDS8Bit;
}

void vSetLVDSOutputMode(LVDS_OUTPUT_MODE_E eOutputMode)
{
	_eLVDS_Output_Mode = eOutputMode;
}

//from 3365 verify ---start
enum {
	LVDS_SINGLE_LINK=0,
	LVDS_DUAL_LINK=1,
};

void vPanelLVDSInit(unsigned int LinkMode)
{
	WriteREGMsk(0xB0,1<<0,1<<0);//lvds top en
	WriteREGMsk(0xCC,1<<0,1<<0);//lvds top Reset
#ifndef __ARM2__
	msleep(50);
#else
	_msleep(50);
#endif
//	vWriteRegMsk(0x34,1<<3,0x1<<3);

//Step1: LVDS TX Analog Enable(setting from boo.hu)
	WriteREGMsk(0x664,0x0<<21,0x1<<21);	//RG_VPLL_BG_PWD
	WriteREGMsk(0x634,0x1<<8,0x1<<8);	//RG_LVDS_BIAS_EN_D11
	WriteREGMsk(0x65c,0x1<<15,0x1<<15);	//RG_LVDS_REV
	WriteREGMsk(0x638,(0x4F<<24)|(0x4F<<16),(0x7F<<24)|(0x7F<<16));	//RG_LVDS_DRV_EN_EVEN/ODD_D11
#if 1
	// 4mA
	WriteREGMsk(0x640,0x3333333<<0,0xFFFFFFF<<0);	//RG_LVDS_TVO_EVEN_D11
	WriteREGMsk(0x644,0x3333333<<0,0xFFFFFFF<<0);	//RG_LVDS_TVO_ODD_D11
	// 64c for 1Kohm  set 01
	WriteREGMsk(0x64C,0x15551555<<0,0x3FFFFFFF<<0);	//RG_LVDS_TVO_ODD_D11
#else
	vWriteRegMsk(0x640,0x7FFFFFF<<0,0x7FFFFFF<<0);	//RG_LVDS_TVO_EVEN_D11
	vWriteRegMsk(0x644,0x7FFFFFF<<0,0x7FFFFFF<<0);	//RG_LVDS_TVO_ODD_D11
#endif
	WriteREGMsk(0x634,0x1<<0,0x1<<0);	//RG_LVDS_EN


//Step2: LVDS Decoder Setup
	WriteREG(0xa5000,0x00000000);
#ifndef __ARM2__
	msleep(100);
#else
	_msleep(100);
#endif
	WriteREG(0xa5000,0xFFFFFFFF);
#ifndef __ARM2__
	msleep(100);
#else
	_msleep(100);
#endif

	switch (LinkMode)
	{
	case LVDS_SINGLE_LINK:
		WriteREGMsk(0x0668,(0x0<<17), (0x1<<17)); // RG_VPLL_TXCLK_DIV2[1]   	(param:set 0-> /1) (LVDS_CTS clk must config this bit)

		WriteREG(0xa5014,0x00310000);
		WriteREG(0xa5618,0x02000100);
		WriteREG(0xa5600,0x0402a359);
		WriteREG(0xa5A00,0x00000002);
		WriteREG(0xa5A08,0x00102Ce4);
		WriteREG(0xa5618,0x02000100);
		WriteREG(0xa5904,0x765402a3);
		WriteREG(0xa590c,0xB1B19898);
		break;
	case LVDS_DUAL_LINK:
		WriteREGMsk(0x0668,(0x1<<17), (0x1<<17)); // RG_VPLL_TXCLK_DIV2[1]   	(param:set 1-> /2) (LVDS_CTS clk must config this bit)
		WriteREG(0xa5014,0x00b30000);
		WriteREG(0xa5618,0x02010100);

		WriteREG(0xa5600,0x0402a359);
		WriteREG(0xa5604,0x07800465);
		WriteREG(0xa5608,0x04380465);
		WriteREG(0xa560c,0x01080002);
		WriteREG(0xa5610,0x001E0001);

		WriteREG(0xa5a00,0x00001002);
		WriteREG(0xa5a08,0x00102Ce4);

		WriteREG(0xa5618,0x02100100);

		WriteREG(0xa5904,0x76543210);
		WriteREG(0xa590c,0xBABA9898);
	break;
	}
}

void PanelVopllInit(void)
{
	WriteRegMsk(0xB0,1<<0,1<<0);//LVDS Clock En & Reset
	WriteRegMsk(0xCC,1<<0,1<<0);//LVDS Clock En & Reset

	//VOPLL Clock Divider Setting
	WriteRegMsk(0x0664, (0<<21), (0x1<<21)); // RG_VPLL_BG_PWD
	//delay 1u
#ifndef __ARM2__
	msleep(50);
#else
	_msleep(50);
#endif
	WriteRegMsk(0x0664, (0<<15), (0x1<<15)); // RG_VPLL_PWD
	//delay 1u
#ifndef __ARM2__
	msleep(50);
#else
	_msleep(50);
#endif
	WriteRegMsk(0x0670, (0x1<<16), (0x1<<16)); // RG_VPLL_RESERVE
	//delay 1u
#ifndef __ARM2__
	msleep(50);
#else
	_msleep(50);
#endif
	WriteRegMsk(0x0668, (0x1<<7), (0x1<<7)); // RG_VPLL_AUTOK_EN

	WriteRegMsk(0x0668,(0x1<<26), (0x1<<26)); // RG_VPLL_LVDSCHL_EN (FIX)
	WriteRegMsk(0x0668,(0x1<<20), (0x1<<20)); // RG_VPLL_LVDS_DPIX_DIV2	(param: set 1-> /3.5)

	WriteRegMsk(0x0664,(0x1<<6), (0x3<<6)); // RG_VPLL_PREDIV			(param: set 1-> /2)

	WriteRegMsk(0x0668,(0x0<<18), (0x3<<18)); // RG_VPLL_TXCLK_DIV1		(param:set 0-> /1)
	WriteRegMsk(0x0668,(0x0<<16), (0x1<<16)); // RG_VPLL_TXCLK_DIV2[0] 		(param:set 0-> /1)
	WriteRegMsk(0x0668,(0x3<<14), (0x3<<14)); // RG_VPLL_AUTOK_CTRL (FIX)
	WriteRegMsk(0x0668,(0x8<<0), (0xF<<0)); // RG_VPLL_DLY_DATA (FIX)
	WriteRegMsk(0x066C,(0x8<<28), (0xF<<28)); // RG_VPLL_DLY_CLKA (FIX)
	WriteRegMsk(0x066C,(0x8<<24), (0xF<<24)); // RG_VPLL_DLY_CLKB (FIX)
	WriteRegMsk(0x066C,(0x2<<16), (0x7<<16)); // RG_VPLL_DIVEN (FIX)
	WriteRegMsk(0x066C,(0x9<<8), (0xF<<8)); // RG_VPLL_BP (FIX)
	WriteRegMsk(0x066C,(0x1<<2), (0x1<<2)); // RG_VPLL_FPEN (FIX)
	WriteRegMsk(0x0668,(0x1<<6), (0x1<<6)); // RG_VPLL_AUTOK_LOAD (FIX)
	WriteRegMsk(0x0664,(0x6<<8), (0x7F<<8)); // RG_VPLL_FBDIV			(param: set 0x06-> *7)

}


void vPanelClockGererate(unsigned int freq)
{
#ifndef __ARM2__
	unsigned long tmpV = 648000; // in unit of KHz (648MHz)
#else
	unsigned long long tmpV = 648000;
#endif
	/************************************************************
	*Step0: Config DDDS Clock to 51.2MHz (102.4/2)*
	**************************************************************/
	tmpV = tmpV<<24;
	tmpV = tmpV / freq;
	tmpV >>= 1;
	WriteREG(0x52c0c,0x8000061f);
//	vWriteReg(0x52c00,0x46540000);	// 648MHz / 102.4 MHz * 2^24
	WriteREG(0x52c00,(1<<30)|(tmpV&0x1FFFFFFF));	// 648MHz / 102.4 MHz * 2^24


	WriteREG(0x52c04,0x020d00c3);
	WriteREG(0x52c14,0x313e013e);
	WriteREG(0x52c14,0x713e013e);

	WriteREGMsk(0x005D0,(7<<21)|(0<<24), (0x7<<21)|(1<<24)); // ddds1 clk div2
	WriteREGMsk(0x005D4,(7<<21)|(0<<24), (0x7<<21)|(1<<24)); // ddds2 clk div2


}


void vPanelBklControl(unsigned int duty_en, unsigned int duty_fb)
{
	unsigned int u4PwmRsn = 0xFFF;
	unsigned int u4PwmH = 0x800;

	duty_en = (duty_en > 100) ? 100 : duty_en;
	duty_fb = (duty_fb > 100) ? 100 : duty_fb;

	//Step1: pwm clock source select
	WriteREGMsk(0x34,0<<9,0x7<<9);  //[fb] pwm1 clk sel ->27MHz
	WriteREGMsk(0x34,0<<16,0x7<<16);//[en]pwm3 clk sel ->27MHz
	//Step2: pwm clock En & reset
	WriteREGMsk(0xA8,(1<<25)|(1<<27),(1<<25)|(1<<27));  //[en] pwm1(1<<25) pwm3(1<<27) Clock en
	WriteREGMsk(0xC4,(1<<25)|(1<<27),(1<<25)|(1<<27));  //[rst] pwm1(1<<25) pwm3(1<<27) Clock en
	//Step3: PWM Pad Select
	WriteREGMsk(0x68,1<<25,3<<25);  //pwm3 -> GPIO 125
	WriteREGMsk(0x5C,1,3<<0);       //pwm1 -> GPIO 150
	//Step4: GPIO Pad Config to Output
	WriteREGMsk(0x80,1<<29,1<<29);  //GPIO 125 output en
	WriteREGMsk(0x84,1<<22,1<<22);  //GPIO 150 output en
	//Step5: PWM Output Control
	u4PwmRsn = (269)&0xFFF;
	u4PwmH = (duty_en * u4PwmRsn /100) & 0xFFF;
	WriteREGMsk(0x3220C,(u4PwmRsn<<20)|(u4PwmH<<8)|(4<<2)|(1<<0), \
				(0xFFF<<20)|(0xFFF<<8)|(0x3F<<2)|(1<<0));	// Config EN (pwm3) to 2K Hz : PWMRSN(269) PWMP(49) PWMH(dty_cyc * PWMRSN)

	u4PwmH = (duty_fb * u4PwmRsn /100) & 0xFFF;
	WriteREGMsk(0x32204,(u4PwmRsn<<20)|(u4PwmH<<8)|(4<<2)|(1<<0), \
				(0xFFF<<20)|(0xFFF<<8)|(0x3F<<2)|(1<<0));	// Config FB (pwm1) to 20K Hz: PWMRSN(269) PWMP(4) PWMH(dty_cyc * PWMRSN)
	WriteREGMsk(0x32218, (1<<2)|(1<<6),(1<<2)|(1<<6));     // PWM Output trigger

}


void vPanelTTLInit(void)
{
	/************************************************************
	*	Configure Panel Pad Preplace		 *
	************************************************************/
	// for Panel Display must Config preplace (multi-fun is not needed)
	WriteREGMsk(0x298,0x1<<30,0x1<<30);//2bit LSB	Enable
	WriteREGMsk(0x298,0x1<<31,0x1<<31);//6bit MSB Enable
	WriteREGMsk(0x298,0x1<<29,0x1<<29);//Enable De Pad(preplace)
	WriteREGMsk(0x298,0x1<<28,0x1<<28);//Enable Sync (preplace)

	WriteREGMsk(0x5C,0x1<<7,0x1<<7);//2bit LSB  Enable
	WriteREGMsk(0x94,0x1<<6,0x1<<6);//vb0~vb3  Enable(Pad Mux)
	WriteREGMsk(0x5C,0x1<<6,0x1<<6);//6bit MSB Enable
	WriteREGMsk(0x5C,0x1<<9,0x1<<9);//Enable De Pad(preplace)
	WriteREGMsk(0x5C,0x1<<8,0x1<<8);//Enable Sync (preplace)
}

void vFpdInit (unsigned int dty_cyc, unsigned int pclk)
{
	static bool init_done = false;
	gdty_cyc = dty_cyc;

	if(1) { //!init_done) {
		WriteRegMsk(0xB4,1<<5,1<<5);//FPD Clock En & Reset
		WriteRegMsk(0xD0,1<<5,1<<5);//FPD Clock En & Reset
		WriteRegMsk(0xDC,1<<22,1<<22);//FPD Clock Clock Invert Used for Clock & Data Phase not match

		/************************************************************
		*                 Step2:  Select FPD Colok Source          *
		************************************************************/
		//MT3365 Modify for FPD Clock Path
		//FPD Clock Select
		WriteRegMsk(0x34,0,0x1<<4);      // twds_sel

		WriteRegMsk(0xD8,1<<19,1<<19);       // fpd_on_global
		WriteRegMsk(0xA48E0,1,1<<0);     // fpd_on_aux
		WriteRegMsk(0x1F034,0,1<<31);// test_clk_sel select to 0
		WriteRegMsk(0xA4700,0x3<<18,0x3<<18);     // fpd_8bit Output
		//add 2017-04-15
		WriteRegMsk(0xA4700,0x1<<7,0x1<<7);//full range input
		WriteRegMsk(0xA4700,0x3<<8,0x3<<8);//12bit to 6bit dither output mode
		WriteRegMsk(0xA4700,0x1<<10,0x1<<10);//run_dr_en
		WriteRegMsk(0xA4700,0x1<<25,0x1<<25);//bypass gamma correction
		WriteRegMsk(0xA4704,0x31<<24,0xff<<24);//color width
		WriteRegMsk(0xA4724,0x2b<<24,0x2b<<24);//???
		WriteRegMsk(0xA4780,0x2<<4,0x3<<4);//output fmt = 8bit
		WriteRegMsk(0xA4780,0x1<<10,0x1<<10);//del_sel
		WriteRegMsk(0xA4780,0x1<<31,0x1<<31);//???
		WriteRegMsk(0xA440c,0x84<<16,0x84<<16);//brightness gain
		//end
		PanelVopllInit();

	}
	vPanelClockGererate(pclk);

	//vPanelBklControl(dty_cyc, 100-dty_cyc);

	if(1) {//!init_done) {
		vPanelTTLInit();
		//vPanelLVDSInit();

#if 0 //def CONFIG_MT33XX_VFY_CP_VFY
		vCpInit();
#endif
	}
	init_done = true;

}
//from 3365 verify ---end

void  vTconSetBrightness(__u32 u4Value)
{
	if (u4Value > 100) {
		u4Value = 0xff;
	} else {
		u4Value = (u4Value * 0xff) / 100;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set  tcon  brightness  %d \r\n", (int)u4Value);
	vWritePCLRPMsk(TCON_BRIGHTNESS_CONTRAST_OFFSET, (u4Value << 16), TCON_BRIGHTNESS_MASK);
}
void  vTconSetContrast(__u32 u4Value)
{
	if (u4Value > 100) {
		u4Value = 0xff;
	} else {
		u4Value = (u4Value * 0xff) / 100;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon contrast  %d \r\n", (int)u4Value);


	vWritePCLRPMsk(TCON_BRIGHTNESS_CONTRAST_OFFSET,  u4Value , TCON_CONTRAST_MASK);
}
void vTconSetSaturation(__u32 u4Value)
{
	if (u4Value > 100) {
		u4Value = 0xff;
	} else {
		u4Value = (u4Value * 0xff) / 100;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon saturationi   %d \r\n", (int)u4Value);
	vWritePCLRPMsk(TCON_SATURATION_OFFSET,  u4Value , TCON_SATURATION_MASK);
}


void vTconSetHue(__u32 u4Value)
{

	if (u4Value > 100) {
		u4Value = 0x3f;
	} else {
		u4Value = (u4Value * 0x3f) / 100;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon HUE %d \r\n", (int)u4Value);
	/*WritePCLRPMsk(TCON_HUE_OFFSET,  1 , 0x01);*/
	vWritePCLRPMsk(TCON_HUE_OFFSET,  u4Value << 8 , TCON_HUE_MASK);
}

void vTconSetYGain(__u32 u4Value)
{
	if (u4Value > 0x1ff) {
		u4Value = 0x1ff;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon YGain %d \r\n", (int)u4Value);
	vWritePCLRPMsk(RW_PCLRP_GAIN_Y, (u4Value << 4), MLC_GAIN_Y);
	vWritePCLRPMsk(RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN, MLC_GAIN_Y_EN);
}

void vTconSetUGain(__u32 u4Value)
{
	if (u4Value > 0x1ff) {
		u4Value = 0x1ff;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon UGain %d \r\n", (int)u4Value);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, (u4Value << 4), MLC_GAIN_U);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);
}

void vTconSetVGain(__u32 u4Value)
{
	if (u4Value > 0x1ff) {
		u4Value = 0x1ff;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] set tcon VGain %d \r\n", (int)u4Value);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, (u4Value << 20), MLC_GAIN_V);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);
}

void vTconSetGainYUV(__u32 gain_Y, __u32 gain_U, __u32 gain_V)
{
	/*gain_Y*/
	vWritePCLRPMsk(RW_PCLRP_GAIN_Y, (gain_Y << 4), MLC_GAIN_Y);
	vWritePCLRPMsk(RW_PCLRP_GAIN_Y, MLC_GAIN_Y_EN, MLC_GAIN_Y_EN);

	/*gain_U*/
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, (gain_U << 4), MLC_GAIN_U);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_U_EN, MLC_GAIN_U_EN);

	/*gain_V*/
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, (gain_V << 20), MLC_GAIN_V);
	vWritePCLRPMsk(RW_PCLRP_GAIN_UV, MLC_GAIN_V_EN, MLC_GAIN_V_EN);

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi]set YUV gain (0x%x, 0x%x, 0x%x) \r\n", (unsigned int)gain_Y
		, (unsigned int)gain_U, (unsigned int)gain_V);

}

void vTconSetDefautGainYUV(void)
{
	/*roll back of default gain*/
	vTconSetGainYUV(0X80, 0X5b, 0x80);
}

int TconGetContrast(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(TCON_BRIGHTNESS_CONTRAST_OFFSET) & (TCON_CONTRAST_MASK)) >> (CONTRAST_GAIN_SHF);
	value = (value * 100) / 0xff;

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon contrast %d\r\n", (int)value);
	return (int)value;
}

int TconGetBrightness(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(TCON_BRIGHTNESS_CONTRAST_OFFSET) & (TCON_BRIGHTNESS_MASK)) >> (BRIGHT_GAIN_SHF);
	value = (value * 100) / 0xff;

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon brightness %d\r\n", (int)value);
	return (int)value;
}

int TconGetHue(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(TCON_HUE_OFFSET) & (TCON_HUE_MASK)) >> (HUE_DEGREE_SHF);
	value = (value * 100) / 0x3f;

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon hue %d\r\n", (int)value);
	return (int)value;
}

int TconGetSaturation(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(TCON_SATURATION_OFFSET) & (TCON_SATURATION_MASK)) >> (SAT_GAIN_SHF);
	value = (value * 100) / 0xff;

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon saturation  %d \r\n",  (int)value);
	return (int)value;
}

int TconGetYGain(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(RW_PCLRP_GAIN_Y) & (MLC_GAIN_Y)) >> MLC_GAIN_Y_SHF;

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon YGain  %d \r\n",  (int)value);
	return (int)value;
}

int TconGetUGain(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(RW_PCLRP_GAIN_UV) & (MLC_GAIN_U)) >> (MLC_GAIN_U_SHF);

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon UGain  %d \r\n",  (int)value);
	return (int)value;
}

int TconGetVGain(void)
{
	__u32 value = 0;

	value = (dReadPCLRP(RW_PCLRP_GAIN_UV) & (MLC_GAIN_V)) >> (MLC_GAIN_V_SHF);

	FB_PRINT(FB_LOG_LVL_DBG, "[ddi] get tcon VGain  %d \r\n",  (int)value);
	return (int)value;
}

void delayUs(__u32 us)
{
	volatile unsigned int i, j;

	for (i = 0; i < 800; i++)
		for (j = 0; j < us; j++) {
			i = i;
		}
}


void vTconSetDDDSClk(UINT16 u2LCDType, __u32 fClkFrq)
{
	__u32 fSrcClk     = 648 * 100 / 8;/*divide clk before FPD*/
	UINT64 fNeedDivide = 0;
	__u32 u4DDDSClk  = 0;
	__u32 fFPDClkMax  = 2100;
	__u32 fFPDClkMin  = 1800;
	/*    UINT16 u2CLKint   = fClkFrq /10;*/

	if (1 == u2LCDType) { /*LVDS*/
		u4DDDSClk = (__u32)((UINT64)0xacccccc * 300 / fClkFrq) + 1;
		*(volatile unsigned int *)0xFD052C00 &= 0xF0000000;
		*(volatile unsigned int *)0xFD052C00 |= u4DDDSClk;
		FB_PRINT(FB_LOG_LVL_DBG, "[wts] set clk %x \r\n", (__u32)u4DDDSClk);
	} else if (0 == u2LCDType) { /*TTL*/
		if (fClkFrq >= (fFPDClkMin / 6) && fClkFrq <= (fFPDClkMax / 6)) { /*FPD divide clk 6 times*/
			fNeedDivide = (UINT64)fSrcClk * (1 << 24);
			u4DDDSClk  = (__u32)(fNeedDivide / (fClkFrq * 6));
			*(volatile unsigned int *)0xFD052C00 &= 0xF0000000;/*set DDDS clk 0x52c00*/
			*(volatile unsigned int *)0xFD052C00 |= u4DDDSClk;
			*(volatile unsigned int *)0xFD000014 &= 0xFF1FFFFF;
			delayUs(10);
			*(volatile unsigned int *)0xFD000014 |= (3 << 21);/*set 0x14[23:21] = 3*/
		} else if (fClkFrq <= (fFPDClkMax / 4)) { /*FPD divide clk 4 times*/
			fNeedDivide = (UINT64)fSrcClk * (1 << 24);
			u4DDDSClk = (__u32)(fNeedDivide / (fClkFrq * 4));
			*(volatile unsigned int *)0xFD052C00 &= 0xF0000000;/*set DDDS clk 0x52c00*/
			*(volatile unsigned int *)0xFD052C00 |= u4DDDSClk;
			*(volatile unsigned int *)0xFD000014 &= 0xFF1FFFFF;
			delayUs(10);
			*(volatile unsigned int *)0xFD000014 |= (2 << 21); /*set 0x14[23:21] = 2*/
		}

		FB_PRINT(FB_LOG_LVL_DBG, "[wts] set clk %x \r\n", (__u32)u4DDDSClk);
	}

}

void vTconSetTiming(UINT16 u2LCDType, LCD_TCON_ARGS_T timing)
{
	if ((timing.u4Clock >= 300) && (timing.u4Clock <= 800)) {
		/*Set DDDS & FPD Clk*/
		vTconSetDDDSClk(u2LCDType, timing.u4Clock);

		/*set HVtotal*/
		*(volatile unsigned int *)0xFD0A468c &= ~0x1FFF0FFF;
		*(volatile unsigned int *)0xFD0A468c |= 0x10000000;               /*adjust*/
		*(volatile unsigned int *)0xFD0A468c |= timing.u4HTotal << 16;     /*htotal*/
		*(volatile unsigned int *)0xFD0A468c |= timing.u4VTotal << 0;     /*vtotal*/

		if (0 == u2LCDType) {
			vWritePTCONMsk(RW_PTCON_GLB0, (timing.bEnableTcon) << 0, TCON_EN); /*enable tcon*/

			/*set DE*/
			vWritePTCONMsk(RW_PTCON_FPD_CFG, (timing.rDE.bAdjEnable) << 19, TCON_ADJ_DE_SEL);
			vWritePTCONMsk(RW_PTCON_TIM8_H, (timing.rDE.u4Hstart)   << 0,  TCON_TnHS);
			vWritePTCONMsk(RW_PTCON_TIM8_H, (timing.rDE.u4Hend)     << 12, TCON_TnHE);
			vWritePTCONMsk(RW_PTCON_TIM8_V, (timing.rDE.u4Vstart)   << 0,  TCON_TnVS);
			vWritePTCONMsk(RW_PTCON_TIM8_V, (timing.rDE.u4Vend)     << 12, TCON_TnVE);

			/*set HSYNC*/
			vWritePTCONMsk(RW_PTCON_FPD_CFG, (timing.rHsync.bAdjEnable) << 20, TCON_ADJ_HSYNC_SEL);
			vWritePTCONMsk(RW_PTCON_FPD_CFG, (timing.rHsync.bPolInv)    << 16, TCON_HSYNC_POL_INV);
			vWritePTCONMsk(RW_PTCON_TIM2_H, (timing.rHsync.u4Hstart)   << 0,  TCON_TnHS);
			vWritePTCONMsk(RW_PTCON_TIM2_H, (timing.rHsync.u4Hend)     << 12, TCON_TnHE);
			vWritePTCONMsk(RW_PTCON_TIM2_V, (timing.rHsync.u4Vstart)   << 0,  TCON_TnVS);
			vWritePTCONMsk(RW_PTCON_TIM2_V, (timing.rHsync.u4Vend)     << 12, TCON_TnVE);

			/*set VSYNC*/
			vWritePTCONMsk(RW_PTCON_FPD_CFG, (timing.rVsync.bAdjEnable) << 21, TCON_ADJ_VSYNC_SEL);
			vWritePTCONMsk(RW_PTCON_FPD_CFG, (timing.rVsync.bPolInv)    << 17, TCON_VSYNC_POL_INV);
			vWritePTCONMsk(RW_PTCON_TIM1_H, (timing.rVsync.u4Hstart)   << 0,  TCON_TnHS);
			vWritePTCONMsk(RW_PTCON_TIM1_H, (timing.rVsync.u4Hend)     << 12, TCON_TnHE);
			vWritePTCONMsk(RW_PTCON_TIM1_V, (timing.rVsync.u4Vstart)   << 0,  TCON_TnVS);
			vWritePTCONMsk(RW_PTCON_TIM1_V, (timing.rVsync.u4Vend)     << 12, TCON_TnVE);
			FB_PRINT(FB_LOG_LVL_DBG, "[wts] set tcon timing \r\n");
		}

	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "Set CLK fail! please set clk in range of [30,52.5]! \r\n");
	}


}

#define  TCON_GAMMA_INDEX_NUM      64

/****************************************************************
Function Description:
    Get the Gamma value table of  TCON
Arguments:
    pbReadTable :the array table used for storaging Gamma value
Return Value:
    NULL
****************************************************************/
void TconGetGamma(__u8 *const pbReadTable)
{
#ifdef HWGETGAMMA
	__u32 u4Idx  = 0;
	__u32 u4Data = 0;
	__u32 u4DelayNum = 10;
	__u32 u4Config = dReadPGMA(RW_PGMA_CTRL0);

	vWritePGMAMsk(RW_PGMA_CTRL0, BYPASS_GAMMA, ACCESS_GAMMA | GAMMA_ON | BYPASS_GAMMA);

	for (u4Idx = 0; u4Idx < TCON_GAMMA_INDEX_NUM; u4Idx++) {
		u4Data = u4Idx | GAMMA_READ_EN;
		vWritePGMA(RW_PGMA_GMA_READ, u4Data);
		/*x_thread_delay(u4DelayNum);*/
		mdelay(u4DelayNum);
		vWritePGMAMsk(RW_PGMA_GMA_READ, 0, GAMMA_READ_EN);
		/*x_thread_delay(u4DelayNum);*/
		mdelay(u4DelayNum);

		u4Data = dReadPGMA(RW_PGMA_GMA_READ);

		if (NULL != pbReadTable) {
			/*now r,g,b use the same gamma table*/
			*(pbReadTable + u4Idx) = (__u8)((u4Data & GAMMA_READ_DAT_R) >> GAMMA_READ_DAT_R_SHF);
		} else {
			FB_PRINT(FB_LOG_LVL_DBG, "TconGetGamma :pbReadTable is NULL\r\n");

		}
	}

	vWritePGMA(RW_PGMA_GMA_READ, u4Config);
#else

	g_u1va = (__u8 *)FB_PHYSICAL_TO_VIRTUAL(ARM2_FBDRV_SHARE_GAMMA_PA);
	memcpy((void *)pbReadTable, (void *)g_u1va, 64);

#endif
}

__u32 u4TconReg1[0x100 / 4] = {0};
__u32 u4TconReg2[0x100 / 4] = {0};
__u32 u4TconReg3[0x100 / 4] = {0};
__u32 u4DClk = 0;
__u32 u4HVTotal = 0;

#ifdef HWGETGAMMA
static bool gb_first = TRUE;
static bool gb_defaultgamma = FALSE;
#endif

void vTconSupend(void)
{
	FB_PRINT(FB_LOG_LVL_DBG, "vTconSupend: backup tcon 0x%x, 0x%x, 0x%x\r\n", (unsigned int)tlcp_reg
		, (unsigned int)togc_reg, (unsigned int)tcon_reg);
	memcpy(u4TconReg1, (__u32 *)(tlcp_reg), 0x100);
	memcpy(u4TconReg2, (__u32 *)(togc_reg), 0x100);
	memcpy(u4TconReg3, (__u32 *)(tcon_reg), 0x100);
	memcpy(&u4DClk, (__u32 *)(IO_BASE_BRINGUP + 0x52c00), 0x4);
	memcpy(&u4HVTotal, (__u32 *)(IO_BASE_BRINGUP + 0xa468c), 0x4);
}

void vTconResume(void)
{
	FB_PRINT(FB_LOG_LVL_DBG, "vTconResume: backup tcon 0x%x, 0x%x, 0x%x\r\n", (unsigned int)tlcp_reg
		, (unsigned int)togc_reg, (unsigned int)tcon_reg);
	memcpy((__u32 *)(tlcp_reg), u4TconReg1, 0x100);
	memcpy((__u32 *)(togc_reg), u4TconReg2, 0x100);
	memcpy((__u32 *)(tcon_reg), u4TconReg3, 0x100);
	memcpy((__u32 *)(IO_BASE_BRINGUP + 0x52c00), &u4DClk, 0x4);
	memcpy((__u32 *)(IO_BASE_BRINGUP + 0xa468c), &u4HVTotal, 0x4);
#if 0
	if (dReadPGMA(RW_PGMA_CTRL0) & GAMMA_ON) {
		vPanelResumeGamma();
	}
#endif
	vPanelSetGamma(g_pbPanelGamma);
}

/*Dither*/
/* **********************************************************************/
/* Function : void vPanelSetDither (bool enable, __u32 input, __u32 output)*/
/* Description :*/
/* Parameter :*/
/*                  [intput]:  0-8bit,  1-10bit, 2-12bit,  3-14bit*/
/*                  [output]: 0-4bit,  1-6bit,   2-8bit,    3-10bit*/
/* Return    : None*/
/* **********************************************************************/
void TconSetDither(bool enable, __u32 u4Input, __u32 u4Output)
{
	RANGE(enable, 0, MAX_1BIT);
	RANGE(u4Input,  0, MAX_2BIT);
	RANGE(u4Output, 0, MAX_2BIT);

	if (FALSE == enable) {
		vWritePGMAMsk(RW_PGMA_DITHER, (MAX_1BIT << DITHER_NEW_EN_SHF) , DITHER_NEW_EN);
		vWritePGMAMsk(RW_PGMA_DITHER, (MAX_1BIT << DITHER_BYPASS_SHF) , DITHER_BYPASS);

		return;
	}
	vWritePGMAMsk(RW_PGMA_DITHER, (MAX_1BIT << DITHER_NEW_EN_SHF) , DITHER_NEW_EN); /* enable new dither*/
	vWritePGMAMsk(RW_PGMA_DITHER, (0x0 << DITHER_BYPASS_SHF) , DITHER_BYPASS);  /* not bypass*/
	vWritePGMAMsk(RW_PGMA_DITHER, (u4Input << 0) | (u4Output << OUT_FMT_SHF)
		      , DITHER_IN_FORMAT | DITHER_OUT_FORMAT); /*set in_fmt and out_fmt*/
	FB_PRINT(FB_LOG_LVL_DBG, "dither input= %d,output=%d \r\n", (int)u4Input, (int)u4Output);
}


void DisplaySetDrvAbility(__u32 curt)
{
	/*set display drv ability;[2,4,6,8mA]  ,by wangwj*/
	/*(set all pin drv ability,include de&hsync&vsync&cli& RGB Data)*/
	if (curt == 2) { /* 2mA*/
		*(volatile unsigned int *)0xFD000500 = 0x0;
		*(volatile unsigned int *)0xFD000514 = 0x0;
	} else if (curt == 4) { /* 4mA*/
		*(volatile unsigned int *)0xFD000500 = 0x0;
		*(volatile unsigned int *)0xFD000514 = 0xFFFFFFFF;
	} else if (curt == 6) { /* 6mA*/
		*(volatile unsigned int *)0xFD000500 = 0xFFFFFFFF;
		*(volatile unsigned int *)0xFD000514 = 0x0;
	} else if (curt == 8) { /* 8mA*/
		*(volatile unsigned int *)0xFD000500 = 0xFFFFFFFF;
		*(volatile unsigned int *)0xFD000514 = 0xFFFFFFFF;
	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "[xzr]current=%d \r\n", (int)curt);
	}     /*none*/
}






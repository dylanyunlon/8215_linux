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

/* #if (DRV_SUPPORT_HDMI_RX) */
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/jiffies.h>

#include <linux/gpio.h>
#include "mhl_private.h"
#include "x_lint.h"
#include "x_typedef.h"
#include "x_printf.h"

#include "x_bim.h"
#include "x_printf.h"
#include "x_debug.h"
#include "x_assert.h"
#include "x_timer.h"
#include "chip_ver.h"
#include "drv_hdmi_rx.h"

#include "hdmi_rx_aud_if.h"
#include "drv_hdmi_rx.h"
#include "hal_io.h"
#include "rx_io.h"
#include "hdmi_rx_hal.h"
#include "x_pdwnc.h"
#include "typedef.h"
#include "hdmi_rx_aud_task.h"
#include "x_ckgen.h"
#include "drv_hdmi_rx.h"
#include "edid_data.h"
/* #include "hdmi_rx_hw.h" */
#include "video_timing.h"
#include "mhl_drv.h"


#include "hdmi_hw_reg.h"
#include "hdmi_debug.h"
#include <generated/atc_project.h>
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include "x_ioopt.h"
#endif

UINT32 _wHalHDMI_EQ_ZERO_VALUE;
UINT32 _wHalHDMI_EQ_BOOST_VALUE;
UINT32 _wHalHDMI_EQ_SEL_VALUE;
UINT32 _wHalHDMI_EQ_GAIN_VALUE;
UINT32 _wHalHDMI_LBW_VALUE;
UINT32 _wHalHDMI_HDCP_MASk1;
UINT32 _wHalHDMI_HDCP_MASk2;
UINT8 _CrcResult[3][3];
extern unsigned long  g_IO_VBASE_VA;



UINT16  _bResDigPhy;

/* #endif */

#define M1_V1_BOARD


void vHalSetEqZeroValueVar(UINT32 u4Data)
{
	_wHalHDMI_EQ_ZERO_VALUE = u4Data;
}

void vHalSetEqBoostValueVar(UINT32 u4Data)
{
	_wHalHDMI_EQ_BOOST_VALUE = u4Data;
}

void vHalSetEqSelValueVar(UINT32 u4Data)
{
	_wHalHDMI_EQ_SEL_VALUE = u4Data;
}

void vHalSetEqGainValueVar(UINT32 u4Data)
{
	_wHalHDMI_EQ_GAIN_VALUE = u4Data;
}

void vHalSetLBWValueVar(UINT32 u4Data)
{
	_wHalHDMI_LBW_VALUE = u4Data;
}


void vHalSetRxHdcpMask1Var(UINT32 u4Data)
{
	_wHalHDMI_HDCP_MASk1 = u4Data;
}

void vHalSetRxHdcpMask2Var(UINT32 u4Data)
{
	_wHalHDMI_HDCP_MASk2 = u4Data;
}

void HDMI_HalTmdsOn(BOOL fgOn)
{
	/*  PD_TERM: Enable TMDS-PHY termination 50-ohm resistance */
	if (fgOn) {
		/*turn on TMDS*/
		HDMI_WRITE32_MASK(REG_PD_SYS, 0x00ff0000, 0x00ff0000);
		HDMI_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x1 << 16, RG_HDMI_TERM_EN);

	} else {
		/* turn off TMDS*/
		SwitchAudioState(ASTATE_AudioOff);

		HDMI_WRITE32_MASK(REG_PD_SYS, 0x00cf0000, 0x00ff0000);
		HDMI_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x0 << 16, RG_HDMI_TERM_EN);
	}
}


#if 1
void vHalRxMHLTMDSCTRL(UINT8 bOnOff)
{
	/*  PD_TERM: Enable TMDS-PHY termination 50-ohm resistance */
	if (bOnOff) {
		/*turn on TMDS*/
		HDMI_WRITE32_MASK(REG_PD_SYS, 0x00ff0000, 0x00ff0000);
		/* HDMI_A_WRITE32(REG_HDMI_RX_CFG2, 0x1<<16); */

	} else {
		/* turn off TMDS*/
		/* SwitchAudioState(ASTATE_AudioOff); */

		HDMI_WRITE32_MASK(REG_PD_SYS, 0xcf, 0xff);
		/* HDMI_A_WRITE32(REG_HDMI_RX_CFG2, 0x0<<16); */
	}
}
#endif




void HDMI_HalSetHpd(BOOL fgHigh)
{
	/* set HPD */
	bool ret;
	/*struct pinctrl *pinctrl_hdmi;*/
	struct pinctrl_state *hdmi_hpd_set;
	struct pinctrl_state *hdmi_hpd_clr;

	/*pinctrl_hdmi = devm_pinctrl_get(hdmi_dev);*/
	if(IS_ERR(pinctrl_hdmi))
		HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: devm_pinctrl_get fail \r\n");

	if (is_sink_attached) { /*  mhl */
		/* BIT_SET(0x70, 5); */
		/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, HDMI_HDP_SEL);*/
		hdmi_hpd_set = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_sel_gpio40_in");
		if(IS_ERR(hdmi_hpd_set))
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: if (is_sink_attached) pinctrl_lookup_state fail \r\n");
		ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_set);
		if(ret)
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: if (is_sink_attached) pinctrl_select_state fail \r\n");
		return;
	}

	if (fgHigh) {
		/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, PINMUX_LEVEL_GPIO_END_FLAG);*/
		hdmi_hpd_clr = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_clr_gpio40_in");
		if(IS_ERR(hdmi_hpd_clr))
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: if (fgHigh) pinctrl_lookup_state fail \r\n");
		ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_clr);
		if(ret)
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: if (fgHigh) pinctrl_select_state fail \r\n");

		/* gpio_inout_sel(PIN_40_HDMI_HPD_RX, INPUT); */
		/* BASE_WRITE32(0x70, BASE_READ32(0x70) | (0x1<<13));*/
		/* BIT_CLR(0x70, 5); */
		/* BIT_CLR(0x78, 8); */
		/* BIT_SET(0xe4, 8); */
		/* GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, PINMUX_LEVEL_GPIO_END_FLAG); */
		/* gpio_request(PIN_40_HDMI_HPD_RX, "HDMI_HPD"); */
		gpiod_direction_input(hdmi_hpd_desc);
		/* HDMI_LOG(HDMI_LOG_DEBUG, "Pull HPD High \r\n"); */
	} else {
		/* 0x368 set bit 13, eg 0x2014 */

		/* gpio_configure(PIN_40_HDMI_HPD_RX, OUTPUT, 0); */
		/* BIT_CLR(0x70, 5); */
		/* BIT_SET(0x78, 8); */
		/* BIT_CLR(0xe4, 8); */
		/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, PINMUX_LEVEL_GPIO_END_FLAG);*/
		hdmi_hpd_clr = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_clr_gpio40_in");
		if(IS_ERR(hdmi_hpd_clr))
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: } else { pinctrl_lookup_state fail \r\n");
		ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_clr);
		if(ret)
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalSetHpd: } else { pinctrl_select_state fail \r\n");	

		/* gpio_request(PIN_40_HDMI_HPD_RX, "HDMI_HPD"); */
		gpiod_direction_output(hdmi_hpd_desc, 0);
		/* udelay(50); */
		BASE_WRITE32(0x368, (BASE_READ32(0x368) | (0x1 << 13)));

		/* HDMI_LOG(HDMI_LOG_DEBUG, "Pull HPD Low \r\n"); */
	}
}

void HDMI_HwInit(void)
{
	BASE_WRITE32(0x70,(BASE_READ32(0x70)|0x839));
	
	BASE_WRITE32(0x358, 0xffffffff);/*HDMI CLK CFG*/
	BASE_WRITE32(0x229fc, 0xffffffff);
	
	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	BIT_CLR(0x710, 3);
	BIT_SET(0x710, 24);
	BIT_SET(0x710, 25);
	BIT_SET(0x710, 2);
	BASE_WRITE32(0x6e0, 0x33);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	BIT_CLR(0x76c, 3);
	BIT_SET(0x76c, 24);
	BIT_SET(0x76c, 25);
	BIT_SET(0x76c, 2);
	BASE_WRITE32(0x744, 0x33);
	#endif
	
	udelay(500);

	HDMI_WRITE32(0x2EC, 0x00132000);
	HDMI_A_WRITE32(0x0, 0x80000000);
	HDMI_A_WRITE32(0x4, 0x55a00e4a);
	HDMI_A_WRITE32(0x8, 0x33217622);
	HDMI_A_WRITE32(0xc, 0x0031ef10);
	HDMI_A_WRITE32(0x10, 0x0020f000);
	HDMI_A_WRITE32(0x14, 0xfc000000);
	HDMI_A_WRITE32(0x18, 0x00000000);
	udelay(500);

	HDMI_A_WRITE32(0x10, 0x00207000);
	HDMI_A_WRITE32(0x0, 0x00000000);
	udelay(500);

	HDMI_A_WRITE32(0x4, 0x55a0094a);
	udelay(500);

	HDMI_A_WRITE32(0x8, 0x33217623);
	HDMI_A_WRITE32(0x4, 0x5520094a);
	
	HDMI_WRITE32(0x38,0x00002280);
	HDMI_WRITE32(0x128,0x00009fe4);
	HDMI_WRITE32(0x124,0xf1400000);
	HDMI_WRITE32(0x13c,0xcff60001);
	HDMI_WRITE32(0x8,0x00009407);
	HDMI_WRITE32(0x134,0x00020600);
	HDMI_WRITE32(0x14,0x865e01ea);
	
	BASE_WRITE32(0x368,0x00002014);/*MHL CLK CFG*/
	BASE_WRITE32(0x22950,0x04b16118);
	BIT_CLR(0x22c70, 1);
	BIT_CLR(0x22c70, 2);
	HDMI_HalEqCalibrate();
}

#if 0
void HDMI_HalHwInit(void)
{
	/*  pinmux HDMI I2C. */
	/* GPIO_MultiFun_Set(PIN_41_HDMI_SCL_RX, HDMI_I2C_SEL); */
	/* GPIO_MultiFun_Set(PIN_79_MHL_SENSE, MHL_SENSE_SEL); */
	BIT_SET(0x70, 0);
	/* get setting from designer */
	*(volatile unsigned int *)0xfd0000b4 |= 0x00004000;
	*(volatile unsigned int *)0xfd0000d0 |= 0x00004000;

	*(volatile unsigned int *)0xfd000358 = 0xffffffff;
	*(volatile unsigned int *)0xfd0229fc = 0xffffffff;

	*(volatile unsigned int *)0xfd000710 = 0x03000004;
	*(volatile unsigned int *)0xfd0006e0 = 0x00000033;
	/* msleep(1); */
	udelay(500);

	*(volatile unsigned int *)0xfd022EEC = 0x00132000;
	*(volatile unsigned int *)0xfd000740 = 0x80000000;
	*(volatile unsigned int *)0xfd000744 = 0x55a00e4a;
	*(volatile unsigned int *)0xfd000748 = 0x33217622;
	*(volatile unsigned int *)0xfd00074c = 0x0031ef10;
	*(volatile unsigned int *)0xfd000750 = 0x0020f000;
	*(volatile unsigned int *)0xfd000754 = 0xfc000000;
	*(volatile unsigned int *)0xfd000758 = 0x00000000;
	udelay(500);

	*(volatile unsigned int *)0xfd000750 = 0x00207000;
	*(volatile unsigned int *)0xfd000740 = 0x00000000;
	udelay(500);

	*(volatile unsigned int *)0xfd000744 = 0x55a0094a;
	udelay(500);

	*(volatile unsigned int *)0xfd000748 = 0x33217623;
	*(volatile unsigned int *)0xfd000744 = 0x5520094a;

	*(volatile unsigned int *)0xfd022c38 = 0x00002280;
	*(volatile unsigned int *)0xfd022d28 = 0x00009fe4;
	*(volatile unsigned int *)0xfd022d24 = 0xf1400000;
	*(volatile unsigned int *)0xfd022d3c = 0xcff60001;
	*(volatile unsigned int *)0xfd022c08 = 0x00009407;
	*(volatile unsigned int *)0xfd022d34 = 0x00020600;
	*(volatile unsigned int *)0xfd022c14 = 0x865e01ea;

	*(volatile unsigned int *)0xfd000368 = 0x00002014;
	*(volatile unsigned int *)0xfd022950 = 0x04b16118;
	BIT_CLR(0x22c70, 1);
	BIT_CLR(0x22c70, 2);
	HDMI_HalEqCalibrate();
#if 0

	/* SYS_CTRL1*/
	HDMI_WRITE32_MASK(REG_SYS_CTRL, 0 , 0xFF);
	HDMI_WRITE32_MASK(REG_SYS_CTRL, PD_ALL | EDGE | BSEL , 0xFF);
	HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x1 << 8, RX_EN);

	/* power on all block */
	HDMI_WRITE32_MASK(REG_PD_SYS, 0x1 << 0, PD_TOTAL);
	HDMI_WRITE32_MASK(REG_PD_SYS, PD_PCLK | PD_MCLK | PD_QO | PD_QE | PD_VHDE | PD_ODCK , 0x00FF0000);

	HDMI_WRITE32_MASK(REG_PD_SYS,
		PD_AO | PD_VO | PD_APLL | PD_12CHAN | PD_FULL | PD_OSC | PD_XTAL_APLL,
		0xFF000000);
	HDMI_WRITE32_MASK(REG_PD_SYS, 0, PD_12CHAN | PD_FULL);
	HDMI_WRITE32_MASK(REG_PD_SYS, PD_12CHAN | PD_FULL, PD_12CHAN | PD_FULL);

	/*TMDS setting*/
	HDMI_WRITE32(REG_TMDS_CTRL0, 0xa222025e);
	HDMI_WRITE32(REG_TMDS_CTRL1, 0x80200000);

	/*set EQ*/
	HDMI_WRITE32_MASK(REG_TMDS_CTRL1, 0xd << 28, EQSEL);
	HDMI_WRITE32_MASK(REG_TMDS_CTRL1, 0x2 << 20, EQ_GAIN);
	HDMI_WRITE32_MASK(REG_TMDS_CTRL0, 0x2 << 16, HDMI_LBW);

	/*analog interface 1*/
	HDMI_HalTmdsOn(FALSE);
	/* vHalHDMISubPortFuncSwtich(FALSE);   //write 3c08,3c0c */
	HDMI_WRITE32_MASK(REG_ACR_CTRL3, 0x5C, 0x000000FF);
	HDMI_WRITE32_MASK(REG_LK_THRS_SVAL, (0xFF << 8) | (0xFF << 0), LK_THRS_SVAL_15_8 | LK_THRS_SVAL_7_0);
	HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 19, WS);
	HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 16, FIRST_BIT);

	HDMI_WRITE32_MASK(REG_AUDP_STAT, 0x6 << 8, PREAMBLE_CRI);
	HDMI_WRITE32_MASK(REG_TEST_CTRL, 0xc << 0, HDCP_CRI);

	HDMI_WRITE32(REG_APLL0, 0x21001680);
	HDMI_WRITE32_MASK(REG_EPST, 0x0 << 24, 0xFF000000);
	HDMI_WRITE32_MASK(REG_KS_MASK, 0x0, 0x000000FF);

	/* enable spdif mode */
	/* HDMI_WRITE32_MASK(REG_AUDRX_CTRL, 0x1D<<8, 0xFF<<8); */
	HDMI_WRITE32_MASK(REG_AUDRX_CTRL, PASS_SPDIF_ERR | PASS_AUD_ERR, PASS_SPDIF_ERR | PASS_AUD_ERR);
	HDMI_WRITE32_MASK(REG_AUDRX_CTRL, 0x1 << 9, SPDIF_MODE);
	HDMI_WRITE32_MASK(REG_AUDRX_CTRL, 0x1 << 8, SPDIF_EN);

	/*  KSCL_H */
	HDMI_WRITE32_MASK(REG_VID_SET, 0x1 << 7, KSCL_H);

	/*  HDCP Keymask */
	HDMI_WRITE32_MASK(REG_EPST, 0xFF << 24, 0xFF000000);
	HDMI_WRITE32_MASK(REG_KS_MASK, 0xC3, 0x000000FF);

	/*  setting to avoid TMDS reset -> vHDMITMDSReset */
	HDMI_WRITE32_MASK(REG_AUDP_STAT, BYP_SYNC | BYP_DVIFILT, BYP_SYNC | BYP_DVIFILT);

	/* clear audio mute */
	HDMI_WRITE32_MASK(REG_AUDP_STAT, 0x0 << 25, AUDIO_MUTE);

	/* set Xclk count */
	HDMI_WRITE32_MASK(REG_VID_CRC_OUT, 0x1 << 23, XCLK_IN_PCLK_SEL);

	/* decode di_DE and vi_DE by both preamble and guard-band */
	HDMI_WRITE32_MASK(REG_MUTE_SET, 0x0, BYPASS_DI_GB);

	/* deep color mode must refer output video clock, not TMDS clock */
	HDMI_WRITE32_MASK(REG_VID_CRC_OUT, 0x1 << 22, XCLK_IN_DPCLK);
	HDMI_WRITE32(REG_CKDT, 0x102b1b38);

	/* enable  HW GAMUT packet decoder */
	HDMI_WRITE32_MASK(REG_N_HDMI_CTRL, 0x1 << 10, TT0_GAMUT_EN);

	/* Enable_TDFIFO_RESET */
	HDMI_WRITE32_MASK(REG_MUTE_SET, 0x1 << 20, TDFIFO_SYNC_EN);

	/* Enable_HW_Mute */
	HDMI_WRITE32_MASK(REG_VID_VRES, 0x1 << 23, VRES_MUTE_AUTO_CLR);
	HDMI_WRITE32_MASK(REG_VID_HRES, 0x3 << 12, HCHG_CNT_THR);
	HDMI_WRITE32_MASK(REG_VID_HRES, 0xF << 8, HSTB_CNT_THR);
	HDMI_WRITE32_MASK(REG_VID_VRES, 0x3 << 24, VSTB_CNT_THR);

	/* set enable Mute when ckdt is off */
	HDMI_WRITE32_MASK(REG_VID_SET, 0x1 << 0, MUTE_CKDT);

	HDMI_WRITE32_MASK(REG_N_HDMI_CTRL1, 0x1 << 4, TT88_0_NEW_GAMUT_ONLY);

	/* enable intr mask */
	HDMI_WRITE32_MASK(REG_INTR_MASK0,
			  NEW_ACP_ONLY | NEW_UNREC_ONLY | NEW_MPEG_ONLY | NEW_AUD_ONLY | NEW_SPD_ONLY | NEW_AVI_ONLY,
			  NEW_ACP_ONLY | NEW_UNREC_ONLY | NEW_MPEG_ONLY | NEW_AUD_ONLY | NEW_SPD_ONLY | NEW_AVI_ONLY);

	/*  enable MCLK90SEL for better sampling phase */
	HDMI_WRITE32_MASK(REG_TMDS_CTRL1, 0x1 << 26, MCK90SEL);

	/* analog part */
	HDMI_WRITE32(REG_MHL_CFG, 0x00132000);
	HDMI_WRITE32(REG_ANA_A0, 0x80000000);
	HDMI_WRITE32(REG_ANA_A4, 0x55A00E4A);
	HDMI_WRITE32(REG_ANA_A8, 0x33217622);
	HDMI_WRITE32(REG_ANA_AC, 0x0031EF10);
	HDMI_WRITE32(REG_ANA_B0, 0x0020F000);
	HDMI_WRITE32(REG_ANA_2F0, 0xFC000000);
	HDMI_WRITE32(REG_TMDS_CTRL1, 0x00000000);



	/* unknown setting */
	/* HDMI_WRITE32(REG_VID_HRES, 0x00002280); */
	HDMI_WRITE32_MASK(REG_VID_HRES, 0x2 << 12, HCHG_CNT_THR); /*  chnage num count */
	HDMI_WRITE32_MASK(REG_VID_HRES, 0x2 << 8, HSTB_CNT_THR); /*  stable num count */

	HDMI_WRITE32(REG_AUDRX_CTRL, 0x00001FE4);
	HDMI_WRITE32(REG_I2S_CTRL, 0xF1400000);
	HDMI_WRITE32(REG_PD_SYS, 0xC0060001);
	HDMI_WRITE32(REG_AUDP_STAT, 0x00020600);
	HDMI_WRITE32(REG_SYS_CTRL, 0x00001407);
	HDMI_WRITE32(REG_MUTE_SET, 0x865E01EA);
	/* HDMI_WRITE32(REG_HDCP_DEV, 0x000003A0); */
	/* HDMI_WRITE32(REG_HDCP_ADDR, 0x00000000); */

	HDMI_WRITE32_MASK(REG_VID_CRC_OUT, XCLK_IN_PCLK_SEL | XCLK_IN_DPCLK , XCLK_IN_PCLK_SEL | XCLK_IN_DPCLK);


	/*     vWriteHdmiRXMsk(AUD_INTF_1,0,1<<15);  //PLL PWD Release */
	/*    vRegWriteFldAlign(ANA_INTF_0,0,RG_CDR_RST);  //CDR Reset Release */
	/*     vRegWriteFldAlign(ANA_INTF_1,0,RG_DATA_RST);  //Data Reset Release */
	/*     vRegWriteFldAlign(ANA_INTF_1,0,RG_EQ_RST);  //EQ Reset Release */
	/*    vRegWriteFldAlign(ANA_INTF_2,1,RG_DEEPCLRCLK_RSTN);  //Deep Color Reset Release */


	HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x1 << 12, DDC_EN);
	HDMI_WRITE32_MASK(REG_MUTE_SET, 0x1 << 31, DDC_DEGLITCH_EN);

	HDMI_WRITE32(REG_KS_MASK, 0x00000000);
	HDMI_WRITE32(REG_EPST, 0x0000ff00);
	HDMI_WRITE32(REG_VID_MODE, 0x000a0000);
	HDMI_WRITE32_MASK(REG_APLL0, (1 << 12) | (1 << 13), (1 << 12) | (1 << 13) | (1 << 14));
	HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, (1 << 10) | (1 << 11) | (1 << 12),
		(1 << 10) | (1 << 11) | (1 << 12) | (1 << 13));



	HalEnableINTR2_CKDT(FALSE);
#endif
}
#endif

void HDMI_HalReset(void)
{
	/* get setting from designer*/

	/*
	vRegWriteFldAlign(ANA_INTF_1,1,RG_EQ_RST);
	vRegWriteFldAlign(ANA_INTF_0,1,RG_CDR_RST);
	HAL_Delay_us(1);
	vRegWriteFldAlign(ANA_INTF_1,0,RG_EQ_RST);
	vRegWriteFldAlign(ANA_INTF_0,0,RG_CDR_RST);



	HAL_Delay_us(10);
	vRegWriteFldAlign(CKDT_RST, 0, HDMI_MACRO_RESET);
	HAL_Delay_us(1);
	vRegWriteFldAlign(CKDT_RST, 1, HDMI_MACRO_RESET);
	HAL_Delay_us(1);
	vRegWriteFldAlign(SRST, 1,  SW_RST);//Reset
	HAL_Delay_us(1);
	vRegWriteFldAlign(SRST, 0,  SW_RST);
	*/
}

void HDMI_HalEnableIntr(void)
{
	HDMI_WRITE32_MASK(REG_INTR_STATE0, 0x0 << 1, INTR_POLARITY);
	HDMI_WRITE32_MASK(REG_INTR_STATE0, 0x0 << 2, INTR_OD);
}

void HDMI_HalPhyReset(UINT8 u1ResetSel)
{

	if (u1ResetSel == HDMI_RST_ALL) {
#if 0
		/* reset ALL */
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x1 << 31, RG_HDMI_CDR_RST);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_CDR_STOP);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_CDR_RST);

		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x1 << 10, RG_HDMI_EQ_RST);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x1 << 11, RG_HDMI_EQ_SWRSTSEL);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_EQ_RST);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_EQ_SWRSTSEL);

		HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x1 << 15, C_DATA_SYNC_AUTO);
		HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x0, C_DATA_SYNC_AUTO);

		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x0, RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x0, RG_HDMI_0_DEEPCLRCLK_RSTN);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x1 << 1, RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x1 << 0, RG_HDMI_0_DEEPCLRCLK_RSTN);
#endif
	}

	if (u1ResetSel == HDMI_RST_EQ) {
		/* reset EQ */
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x1 << 10, RG_HDMI_EQ_RST);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x1 << 11, RG_HDMI_EQ_SWRSTSEL);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_EQ_RST);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG0, 0x0, RG_HDMI_EQ_SWRSTSEL);

	}

	if (u1ResetSel == HDMI_RST_DEEPCOLOR) {
		/* reset DEEPCOLOR */
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x0, RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x0, RG_HDMI_0_DEEPCLRCLK_RSTN);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x1 << 1, RG_HDMI_0_DEEPCLRCLK_PDB);
		HDMI_A_WRITE32_MASK(REG_HDMI_RX_CFG2, 0x1 << 0, RG_HDMI_0_DEEPCLRCLK_RSTN);


	}

	if (u1ResetSel == HDMI_RST_FIXEQ) {
		/* reset FIXEQ */
	}

	if (u1ResetSel == HDMI_RST_RTCK) {
		/* reset RTCK */
		HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x1 << 15, C_DATA_SYNC_AUTO);
		HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x0, C_DATA_SYNC_AUTO);

	}
}


/* Reset TMDS-PHY sphtrack block */
void HDMI_HalResetPhySp(void)
{
	/* reset rx phy */
}

void HDMI_HalSwReset(void)
{
	HDMI_WRITE32_MASK(REG_SRST, 0x1 << 8, SW_RST);
	HDMI_WRITE32_MASK(REG_SRST, 0x0, SW_RST);
}

void HDMI_HalDigtailPhyReset(void)
{
	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x0, HDMI_MACRO_SW_RST);
	Sleep(5);
	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x1 << 9, HDMI_MACRO_SW_RST);
}

void HDMI_HalMuteAudio(void)
{
	/* Mute audio channel, ch0~3 */
	HDMI_WRITE32_MASK(REG_CHST1, 0x1 << 16, CH0_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x1 << 17, CH1_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x1 << 18, CH2_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x1 << 19, CH3_MUTE);
}

void HDMI_HalUnMuteAudio(void)
{
	/* unMute audio channel, ch0~3 */
	HDMI_WRITE32_MASK(REG_CHST1, 0x0, CH0_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x0, CH1_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x0, CH2_MUTE);
	HDMI_WRITE32_MASK(REG_CHST1, 0x0, CH3_MUTE);
}

void HDMI_HalOpenApll(void)
{
	HDMI_WRITE32_MASK(REG_PD_SYS, 0x1 << 29, PD_APLL);
	HDMI_WRITE32_MASK(REG_PD_SYS, 0x1 << 31, PD_AO);
}

void HDMI_HalI2sLRInv(BOOL fgInv)
{
	if (fgInv) {
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 19, WS);
	} else {
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, WS);
	}
}

void HDMI_HalSetAudI2sFormat(UINT8 u1fmt, UINT8 u1Cycle)
{
	if (u1fmt == FORMAT_RJ) {
		/*  Right-Justified */
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 28, JUSTIFY);
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 16, FIRST_BIT);
	} else if (u1fmt == FORMAT_LJ) {
		/*  Left Justified */
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, JUSTIFY);
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 16, FIRST_BIT);
	} else if (u1fmt == FORMAT_I2S) {
		/*  I2S */
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, JUSTIFY);
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, FIRST_BIT);
	}

	if (u1Cycle == LRCK_CYC_16) {
		/* 16 cycle */
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 21, SIZE);
	} else if (u1Cycle == LRCK_CYC_32) {
		/* 32 cycle */
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, SIZE);
	}
}

void HDMI_HalSetLRClkEdge(UINT8 u1EdgeFmt)
{
	if (u1EdgeFmt) {
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x1 << 22, CLK_EDGE);
	} else {
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, CLK_EDGE);
	}
}

void HDMI_HalSetI2sMclk(UINT8 u1MclkType)
{
	if (u1MclkType == MCLK_128FS) {
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x0 << 20, FM_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x0 << 22, FM_IN_VAL_SW);
	} else if (u1MclkType == MCLK_256FS) {
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 20, FM_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 22, FM_IN_VAL_SW);
	} else if (u1MclkType == MCLK_384FS) {
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x2 << 20, FM_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x2 << 22, FM_IN_VAL_SW);
	} else if (u1MclkType == MCLK_512FS) {
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x3 << 20, FM_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x3 << 22, FM_IN_VAL_SW);
	} else {

	}
}

void HDMI_HalSetAudioFS(enum HDMI_RX_AUDIO_FS eFS)
{
	switch (eFS) {
	case SW_44p1K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x0 << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_88p2K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x8 << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_176p4K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0xc << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_48K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x2 << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_96K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0xa << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_192K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0xe << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case SW_32K:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x3 << 16, FS_VAL_SW);
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x1 << 1, FS_HW_SW_SEL);
		break;

	case HW_FS:
		HDMI_WRITE32_MASK(REG_ACR_CTRL1, 0x0, FS_HW_SW_SEL);
		break;

	default:
		break;
	}
}

UINT8 HDMI_HalGetI2sMclk(void)
{
	UINT32 u4Value = 0;
	UINT8  u1MclkType = MCLK_128FS;

	u4Value = (HDMI_READ32(REG_ACR_CTRL1) & FM_VAL_SW) >> 20;

	switch (u4Value) {
	case 0x0:
		u1MclkType = MCLK_128FS;
		break;

	case 0x1:
		u1MclkType = MCLK_256FS;
		break;

	case 0x2:
		u1MclkType = MCLK_384FS;
		break;

	case 0x3:
		u1MclkType = MCLK_512FS;
		break;

	default:
		break;
	}

	return u1MclkType;
}

void HDMI_HalEnableAudClk(void)
{
	HDMI_WRITE32_MASK(REG_AUDRX_CTRL, 0x1 << 10, I2S_MODE);
	HDMI_WRITE32_MASK(REG_I2S_CTRL,
			  (SD2_EN | SD1_EN | SD0_EN | MCLK_EN),
			  (SD2_EN | SD1_EN | SD0_EN | MCLK_EN));
	HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, PCM);
}

void HDMI_HalSetAudMuteCondition(void)
{
	HDMI_WRITE32_MASK(REG_AEC_CTRL, 0x16 << 24, EXP_EN_15_8);
}

void HDMI_HalEnableAacToSd0123(void)
{
	HDMI_WRITE32_MASK(REG_AEC_CTRL, 0x1 << 13, AAC_OUT_OFF_EN);
}

UINT8 HDMI_HalGetSCDT(void)/* check Sync */
{
	if (HDMI_READ32(REG_SRST) & SCDT) {
		return 1;
	} else {
		return 0;
	}
}

UINT8 HDMI_HalGetCKDT(void)/* check clock */
{
	if (HDMI_READ32(REG_SRST) & CKDT) {
		return 1;
	} else {
		return 0;
	}
}

#if 0

UINT8 HDMI_HalGetPwr5V(void)/* check +5V */
{
	if (HDMI_READ32(REG_SRST) & PWR5V_RX0) {
		return 1;
	} else {
		return 0;
	}
}

#else

UINT8 HDMI_HalGetPwr5V(void)/* check +5V */
{
	static UINT32 u4TempCount;
	bool ret;
	/*struct pinctrl *pinctrl_hdmi;*/
	struct pinctrl_state *hdmi_hpd_set;
	struct pinctrl_state *hdmi_hpd_clr;

	/*pinctrl_hdmi = devm_pinctrl_get(hdmi_dev);*/
	if(IS_ERR(pinctrl_hdmi))
		HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: devm_pinctrl_get fail \r\n");

	/* if(SinkGetCDSense()) */
	if (is_sink_attached) { /*  mhl */
		/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, HDMI_HDP_SEL);*/
		hdmi_hpd_set = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_sel_gpio40_in");
		if(IS_ERR(hdmi_hpd_set))
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: if (is_sink_attached) pinctrl_lookup_state fail \r\n");
		ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_set);
		if(ret)
			HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: if (is_sink_attached) pinctrl_select_state fail \r\n");

		/* BIT_SET(0x70, 5); */
		return TRUE;
	}

	/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, 0);*/
	hdmi_hpd_clr = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_clr_gpio40_in");
	if(IS_ERR(hdmi_hpd_clr))
		HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: pinctrl_lookup_state fail \r\n");
	ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_clr);
	if(ret)
		HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: pinctrl_select_state fail \r\n");
	
	gpiod_direction_input(hdmi_hpd_desc);


	/* BIT_CLR(0x70, 5); */
	/* BIT_CLR(0x78, 8); */

	if (gpiod_get_value(hdmi_hpd_desc) == 0) { /* GPIO_Input(PIN_40_HDMI_HPD_RX)) */
		u4TempCount++;

		if (u4TempCount >= 200) {
			u4TempCount = 0;
		}

		return 0;
	} else {
		return 1;
	}
}


#endif






void HDMI_HalSelAnaBandExt(HDMI_ANA_BAND eBand)
{
	switch (eBand) {
	case HDMI_ANA_BAND_NULL:
		HDMI_HalSelHdmiAnaBand(HDMI_ANA_BAND_40_160M);
		break;

	case HDMI_ANA_BAND_10_27M:
	case HDMI_ANA_BAND_27_40M:
	case HDMI_ANA_BAND_40_160M:
	case HDMI_ANA_BAND_160_250M:
	case HDMI_ANA_BAND_250_MAX:
		HDMI_HalSelHdmiAnaBand(eBand);
		break;

	case MHL_ANA_BAND_PP_0_30M:
	case MHL_ANA_BAND_PP_30_MAX:
	case MHL_ANA_BAND_0_50M:
	case MHL_ANA_BAND_50_MAX:
		HDMI_HalSelMhlAnaBand(eBand);
		break;

	default:
		break;
	}
}

extern unsigned long  g_IO_VBASE_VA;
void HDMI_HalSelHdmiAnaBand(HDMI_ANA_BAND eBand)
{
	/* UINT32 u4CKPRD; */
	/* u4CKPRD = (HDMI_READ32(REG_N_HDMI_CTRL4) & TMDS_CK_PERIOD)>>12; */

	HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132000, 0x1FFFFFFF);
	HDMI_WRITE32_MASK(REG_HDMI_DIG_MACRO, 0x1 << 8, 0x1 << 8);

	/* TMDS clock = 64*27Mhz / TMDS_CK_PERIOD, */

	switch (eBand) {
	case HDMI_ANA_BAND_NULL:
		break;

	case HDMI_ANA_BAND_10_27M:

		HDMI_LOG(HDMI_LOG_DEBUG, "Select 0~27M\r\n");
		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0x55A00E4A);
		HDMI_A_WRITE32(0x8, 0x33217622);
		HDMI_A_WRITE32(0xc, 0x0031ef10);
		HDMI_A_WRITE32(0x10, 0x0020F000);
		HDMI_A_WRITE32(0x14, 0xFC000000);
		HDMI_A_WRITE32(0x18, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x00207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A0094A);
		msleep(1);

		HDMI_A_WRITE32(0x8, 0x33217623);
		HDMI_A_WRITE32(0x4, 0x5520094A);

		/*
		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0x55A00D4A);
		HDMI_WRITE32_MASK(0x000A8,0x33217622,0xFFFFFFFF);
		HDMI_WRITE32(0x000AC,0x0031ef10);
		HDMI_WRITE32(0x000B0,0x0020F000);
		HDMI_WRITE32(0x002F0,0xFC000000);
		HDMI_WRITE32(0x00094,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x00207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0x55A0094A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x33217623);
		HDMI_WRITE32(0x000A4,0x5520094A);
		HAL_Delay_us(5);
		*/



		break;

	case HDMI_ANA_BAND_27_40M:

		HDMI_LOG(HDMI_LOG_DEBUG, "Select 27~40M\r\n");

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0x55A00E4A);
		HDMI_A_WRITE32(0x8, 0x33217622);
		HDMI_A_WRITE32(0xc, 0x0031ef30);
		HDMI_A_WRITE32(0x10, 0x0020F000);
		HDMI_A_WRITE32(0x14, 0x14000000);
		HDMI_A_WRITE32(0x18, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x00207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A00A4A);
		msleep(1);

		HDMI_A_WRITE32(0x8, 0x33217623);
		HDMI_A_WRITE32(0x4, 0x55200A4A);
		/*
		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0x55A00E4A);
		HDMI_WRITE32_MASK(0x000A8,0x33217622,0xFFFFFFFF);
		HDMI_WRITE32(0x000AC,0x0031ef30);
		HDMI_WRITE32(0x000B0,0x0020F000);
		HDMI_WRITE32(0x002F0,0x14000000);
		HDMI_WRITE32(0x00094,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x00207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0x55A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x33217623);
		HDMI_WRITE32(0x000A4,0x55200A4A);
		HAL_Delay_us(5);
		*/
		break;

	case HDMI_ANA_BAND_40_160M:

		HDMI_LOG(HDMI_LOG_DEBUG, "Select 40~160M\r\n");

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0x55A00E4A);
		HDMI_A_WRITE32(0x8, 0x33217422);
		HDMI_A_WRITE32(0xc, 0x00212310);
		HDMI_A_WRITE32(0x10, 0x0120F000);
		HDMI_A_WRITE32(0x14, 0x00000000);
		HDMI_A_WRITE32(0x18, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x01207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A00A4A);
		msleep(1);

		HDMI_A_WRITE32(0x8, 0x33217423);
		HDMI_A_WRITE32(0x4, 0x55200A4A);

		/*
		HDMI_WRITE32(0x000A0,0x80000000);

		HDMI_WRITE32(0x000A4,0x55A00E4A);
		HDMI_WRITE32_MASK(0x000A8,0x33217422,0xFFFFFFFF);
		HDMI_WRITE32(0x000AC,0x00212310);
		HDMI_WRITE32(0x000B0,0x0120F000);
		HDMI_WRITE32(0x002F0,0x00000000);
		HDMI_WRITE32(0x00094,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x01207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0x55A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x33217423);
		HDMI_WRITE32(0x000A4,0x55200A4A);
		HAL_Delay_us(5);
		*/

		break;

	case HDMI_ANA_BAND_160_250M:

		HDMI_LOG(HDMI_LOG_DEBUG, "Select 160~250M\r\n");

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0xD5A00E4A);
		HDMI_A_WRITE32(0x8, 0x33215022);
		HDMI_A_WRITE32(0xc, 0x08212310);
		HDMI_A_WRITE32(0x10, 0x0120F000);
		HDMI_A_WRITE32(0x14, 0x00000000);
		HDMI_A_WRITE32(0x18, 0x08000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x01207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A00A4A);
		msleep(1);

		HDMI_A_WRITE32(0x8, 0x33215023);
		HDMI_A_WRITE32(0x4, 0xD5200A4A);

		/*
		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0xD5A00E4A);
		HDMI_WRITE32_MASK(0x000A8,0x33215022,0xFFFFFFFF);
		HDMI_WRITE32(0x000AC,0x08212310);
		HDMI_WRITE32(0x000B0,0x0120F000);
		HDMI_WRITE32(0x002F0,0x00000000);
		HDMI_WRITE32(0x00094,0x08000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x01207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0xD5A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x33215023);
		HDMI_WRITE32(0x000A4,0xD5200A4A);
		HAL_Delay_us(5);
		*/
		break;

	case HDMI_ANA_BAND_250_MAX:

		HDMI_LOG(HDMI_LOG_DEBUG, "Select >250\r\n");

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0xD5A00E4A);
		HDMI_A_WRITE32(0x8, 0x31214032);
		HDMI_A_WRITE32(0xc, 0x08214310);
		HDMI_A_WRITE32(0x10, 0x0120F000);
		HDMI_A_WRITE32(0x14, 0x00000000);
		HDMI_A_WRITE32(0x18, 0x08000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x00207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0xD5A00A4A);
		msleep(1);

		HDMI_A_WRITE32(0x8, 0x31214033);
		HDMI_A_WRITE32(0x4, 0xD5200A4A);

		/*
		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0xD5A00E4A);
		HDMI_WRITE32_MASK(0x000A8,0x31214032,0xFFFFFFFF);
		HDMI_WRITE32(0x000AC,0x08214310);
		HDMI_WRITE32(0x000B0,0x0120F000);
		HDMI_WRITE32(0x002F0,0x00000000);
		HDMI_WRITE32(0x00094,0x08000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x00207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0xD5A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x31214033);
		HDMI_WRITE32(0x000A4,0xD5200A4A);
		HAL_Delay_us(5);
		*/

		break;

	default:
		break;

	}
	BASE_WRITE32(0x35c, 0x0);
	msleep(5);
	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x0, HDMI_MACRO_SW_RST);
	msleep(5);
	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x1 << 9, HDMI_MACRO_SW_RST);
	HDMI_HalEqCalibrate();
}


void HDMI_HalSelMhlAnaBand(HDMI_ANA_BAND eBand)
{
	/* UINT32 u4CKPRD; */
	/* u4CKPRD = (HDMI_READ32(REG_N_HDMI_CTRL4) & TMDS_CK_PERIOD)>>12; */

	switch (eBand) {
	case HDMI_ANA_BAND_NULL:
		break;

	case MHL_ANA_BAND_PP_0_30M:
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132002, 0x1FFFFFFF);

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0x55A00E4A);
		/*HDMI_A_WRITE32(0x8, 0x93213422);*/
		HDMI_A_WRITE32_MASK(0x8, 0x93213422, 0xFFFEFFFF);
		HDMI_A_WRITE32(0xc, 0x0031ef10);
		HDMI_A_WRITE32(0x10, 0x8020F000);
		HDMI_A_WRITE32(0x14, 0xc8000000);
		HDMI_A_WRITE32(0x18, 0x70000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x80207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A00A4A);
		msleep(1);

		/*HDMI_A_WRITE32(0x8, 0xd3213423);*/
		HDMI_A_WRITE32_MASK(0x8, 0xd3213423, 0xFFFEFFFF);
		HDMI_A_WRITE32(0x4, 0x55200A4A);
		msleep(1);


		/*
		HDMI_WRITE32_MASK(0x002EC,0x00132006,0x1FFFFFFF);

		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0x55A00E4A);
		HDMI_WRITE32(0x000A8,0x93213422);
		HDMI_WRITE32(0x000AC,0x0031ef10);
		HDMI_WRITE32(0x000B0,0x8020F000);
		HDMI_WRITE32(0x002F0,0xC8000000);
		HDMI_WRITE32(0x00094,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x80207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0x55A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0xD3213423);
		HDMI_WRITE32(0x000A4,0x55200A4A);
		HAL_Delay_us(5);
		*/
		break;

	case MHL_ANA_BAND_PP_30_MAX:
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x20132006, 0xFFFFFFFF);

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0xD5A00E4A);
		/*HDMI_A_WRITE32(0x8, 0x93211022);*/
		HDMI_A_WRITE32_MASK(0x8, 0x93211022, 0xFFFEFFFF);
		HDMI_A_WRITE32(0xc, 0x0831ef10);
		HDMI_A_WRITE32(0x10, 0x8020F000);
		HDMI_A_WRITE32(0x14, 0xc8000000);
		HDMI_A_WRITE32(0x18, 0x78000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0x80207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0xD5A00A4A);
		msleep(1);

		/*HDMI_A_WRITE32(0x8, 0x93211023);*/
		HDMI_A_WRITE32_MASK(0x8, 0x93211023, 0xFFFEFFFF);
		HDMI_A_WRITE32(0x4, 0xD5200A4A);
		msleep(1);

		/*
		HDMI_WRITE32_MASK(0x002EC,0x00132006,0x1FFFFFFF);

		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0xD5A00E4A);
		HDMI_WRITE32(0x000A8,0x93211022);
		HDMI_WRITE32(0x000AC,0x0831ef10);
		HDMI_WRITE32(0x000B0,0x8020F000);
		HDMI_WRITE32(0x002F0,0xC8000000);
		HDMI_WRITE32(0x00094,0x78000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0x80207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0xD5A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0x93211023);
		HDMI_WRITE32(0x000A4,0xD5200A4A);
		HAL_Delay_us(5);
		*/
		break;

	case MHL_ANA_BAND_0_50M:
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132002, 0x1FFFFFFF);

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0x55A00E4A);
		/*HDMI_A_WRITE32(0x8, 0xD3213422);*/
		HDMI_A_WRITE32_MASK(0x8, 0xD3213422, 0xFFFEFFFF);
		HDMI_A_WRITE32(0xc, 0x0031ff30);
		HDMI_A_WRITE32(0x10, 0xc020f000);
		HDMI_A_WRITE32(0x14, 0xc8000000);
		HDMI_A_WRITE32(0x18, 0x70000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0xc0207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0x55A00A4A);
		msleep(1);

		/*HDMI_A_WRITE32(0x8, 0xD3213423);*/
		HDMI_A_WRITE32_MASK(0x8, 0xD3213423, 0xFFFEFFFF);
		HDMI_A_WRITE32(0x4, 0x55200A4A);

		/* Sleep(1); */



		/*
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132002, 0x1FFFFFFF);

		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0x55A00E4A);
		HDMI_WRITE32(0x000A8,0xD3213422);
		HDMI_WRITE32(0x000AC,0x0031ff30);
		HDMI_WRITE32(0x000B0,0xC020F000);
		HDMI_WRITE32(0x002F0,0xC8000000);
		HDMI_WRITE32(0x00094,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0xC0207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0x55A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0xD3213423);
		HDMI_WRITE32(0x000A4,0x55200A4A);
		HAL_Delay_us(5);
		*/
		break;

	case MHL_ANA_BAND_50_MAX:

		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132002, 0x1FFFFFFF);

		HDMI_A_WRITE32(0x0, 0x80000000);
		HDMI_A_WRITE32(0x4, 0xD5A00E4A);
		/*HDMI_A_WRITE32(0x8, 0xD3211022);*/
		HDMI_A_WRITE32_MASK(0x8, 0xD3211022, 0xFFFEFFFF);
		HDMI_A_WRITE32(0xc, 0x0831ff30);
		HDMI_A_WRITE32(0x10, 0xc020f000);
		HDMI_A_WRITE32(0x14, 0xc8000000);
		HDMI_A_WRITE32(0x18, 0x78000000);
		msleep(1);

		HDMI_A_WRITE32(0x10, 0xc0207000);
		HDMI_A_WRITE32(0x0, 0x00000000);
		msleep(1);

		HDMI_A_WRITE32(0x4, 0xD5A00A4A);
		msleep(1);

		/*HDMI_A_WRITE32(0x8, 0xD3211023);*/
		HDMI_A_WRITE32_MASK(0x8, 0xD3211023, 0xFFFEFFFF);
		HDMI_A_WRITE32(0x4, 0xD5200A4A);

		/*
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x00132002, 0x1FFFFFFF);

		HDMI_WRITE32(0x000A0,0x80000000);
		HDMI_WRITE32(0x000A4,0xD5A00E4A);
		HDMI_WRITE32(0x000A8,0xD3211022);
		HDMI_WRITE32(0x000AC,0x0831ff30);
		HDMI_WRITE32(0x000B0,0xC020F000);
		HDMI_WRITE32(0x002F0,0xC8000000);
		HDMI_WRITE32(0x00094,0x08000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000B0,0xC0207000);
		HDMI_WRITE32(0x000A0,0x00000000);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A4,0xD5A00A4A);
		HAL_Delay_us(5);
		HDMI_WRITE32(0x000A8,0xD3211023);
		HDMI_WRITE32(0x000A4,0xD5200A4A);
		HAL_Delay_us(5);
		*/

		break;

	default:
		break;
	}



#if 1

	switch (eBand) {
	case HDMI_ANA_BAND_NULL:
		break;

	case MHL_ANA_BAND_PP_0_30M:
	case MHL_ANA_BAND_PP_30_MAX:
		HDMI_WRITE32(REG_PD_SYS, 0xCff60001);
		HDMI_WRITE32(REG_SYS_CTRL, 0x00009407);
		HDMI_WRITE32(REG_MHL_CFG, 0x30132006);
		BASE_WRITE32(0x35C, 0x00000080);


		break;

	case MHL_ANA_BAND_0_50M:
	case MHL_ANA_BAND_50_MAX:

		HDMI_WRITE32(REG_PD_SYS, 0xCff60001);
		HDMI_WRITE32(REG_SYS_CTRL, 0x00009407);
		HDMI_WRITE32(REG_MHL_CFG, 0x30132002);
		BASE_WRITE32(0x35C, 0x00000080);
		break;

	default:
		break;
	}

#endif

	/*HDMI_HalEqCalibrate();*/

	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x0, HDMI_MACRO_SW_RST);
	/* HAL_Delay_us(50); */
	msleep(1);
	HDMI_WRITE32_MASK(REG_CKDT_RST, 0x1 << 9, HDMI_MACRO_SW_RST);
}



/* Equalizer offset calibration 7--0--8--15 */
void HDMI_HalEqCalibrate(void)
{
	UINT8 i;
	UINT8 u1ch_lock_value[3] = {0, 0, 0};
	UINT8 u1ch_locked[3] = {0, 0, 0};
	UINT8 u1ch_value[3];

	/* PRINT_REG(0x744); */
	/* vIO32WriteFldAlign((ANA_INTF_1) ,1,RG_CKDT_SET); */
	ANA_BIT_SET(0x4, 27);
	/* PRINT_REG(0x744); */
	/* PRINT_REG(0x754); */
	/* vIO32WriteFldAlign((ANA_2F0) ,1,RG_HDMI_0_EQ_CALEN_T); */
	ANA_BIT_SET(0x14, 12);
	/* PRINT_REG(0x754); */

	/* 7--1 */
	for (i = 7; i > 0; i--) {
		/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH0_EQMC_T); */
		vANAWriteRegMsk(0x14, i, 0xf);
		/* PRINT_REG(0x754); */
		/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH1_EQMC_T); */
		vANAWriteRegMsk(0x14, i << 4, (0xf << 4));
		/* PRINT_REG(0x754); */
		/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH2_EQMC_T); */
		vANAWriteRegMsk(0x14, i << 8, (0xf << 8));
		/* PRINT_REG(0x754); */

		/* PRINT_REG(0x768); */
		if (((vANAReadReg(0x28) >> 31) == 1) && (u1ch_locked[0] == 0)) {
			u1ch_lock_value[0] = i;
			u1ch_locked[0] = 1;
			HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH0_EQMM %x\n", u1ch_lock_value[0]);
		}

		if ((((vANAReadReg(0x28) >> 30) & 1) == 1) && (u1ch_locked[1] == 0)) {
			u1ch_lock_value[1] = i;
			u1ch_locked[1] = 1;
			HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH1_EQMM %x\n", u1ch_lock_value[1]);
		}

		if ((((vANAReadReg(0x28) >> 29) & 1) == 1) && (u1ch_locked[2] == 0)) {
			u1ch_lock_value[2] = i;
			u1ch_locked[2] = 1;
			HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH2_EQMM %x\n", u1ch_lock_value[2]);
		}

		if ((u1ch_locked[0] == 1) && (u1ch_locked[1] == 1) && (u1ch_locked[2] == 1)) {
			break;
		}
	}

	/* 0;8--15 */
	if ((u1ch_locked[0] == 0) || (u1ch_locked[1] == 0) || (u1ch_locked[2] == 0)) {
		for (i = 0; i <= 15; i++) {
			/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH0_EQMC_T); */
			vANAWriteRegMsk(0x14, (i), 0xf);
			/* PRINT_REG(0x754); */
			/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH1_EQMC_T); */
			vANAWriteRegMsk(0x14, (i << 4), (0xf << 4));
			/* PRINT_REG(0x754); */
			/* vIO32WriteFldAlign((ANA_2F0) ,i, RG_HDMI_0_CH2_EQMC_T); */
			vANAWriteRegMsk(0x14, (i << 8), (0xf << 8));
			/* PRINT_REG(0x754); */


			/* PRINT_REG(0x768); */
			if ((((vANAReadReg(0x28) >> 31) & 1) == 1) && (u1ch_locked[0] == 0)) {
				u1ch_lock_value[0] = i;
				u1ch_locked[0] = 1;
				HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH0_EQMM %x\n", u1ch_lock_value[0]);
			}

			if ((((vANAReadReg(0x28) >> 30) & 1) == 1) && (u1ch_locked[1] == 0)) {
				u1ch_lock_value[1] = i;
				u1ch_locked[1] = 1;
				HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH1_EQMM %x\n", u1ch_lock_value[1]);
			}

			if ((((vANAReadReg(0x28) >> 29) & 1) == 1) && (u1ch_locked[2] == 0)) {
				u1ch_lock_value[2] = i;
				u1ch_locked[2] = 1;
				HDMI_LOG(HDMI_LOG_DEBUG, "RGS_HDMI_0_CH2_EQMM %x\n", u1ch_lock_value[2]);
			}

			if ((u1ch_locked[0] == 1) && (u1ch_locked[1] == 1) && (u1ch_locked[2] == 1)) {
				break;
			}

			if (i == 0) {
				i = 7;
			}
		}
	}

	for (i = 0; i < 3; i++) {
		if (u1ch_lock_value[i] == 7) {
			u1ch_value[i] = 7;
		} else if (u1ch_lock_value[i] == 0) {
			u1ch_value[i] = 1;
		} else if (u1ch_lock_value[i] == 8) {
			u1ch_value[i] = 0;
		} else if (u1ch_lock_value[i] == 15) {
			u1ch_value[i] = 15;
		} else if ((u1ch_lock_value[i] < 7) && (u1ch_lock_value[i] > 0)) {
			u1ch_value[i] = u1ch_lock_value[i] + 1;
		} else {
			u1ch_value[i] = u1ch_lock_value[i] - 1;
		}
	}

	/* vIO32WriteFldAlign((ANA_2F0) ,u1ch_value[0],RG_HDMI_0_CH0_EQMC_T); */
	vANAWriteRegMsk(0x14, u1ch_value[0], 0xf);
	/* vIO32WriteFldAlign((ANA_2F0) ,u1ch_value[1],RG_HDMI_0_CH1_EQMC_T); */
	vANAWriteRegMsk(0x14, (u1ch_value[1] << 4), (0xf << 4));
	/* vIO32WriteFldAlign((ANA_2F0) ,u1ch_value[2],RG_HDMI_0_CH2_EQMC_T); */
	vANAWriteRegMsk(0x14, (u1ch_value[2] << 8), (0xf << 8));
	/* PRINT_REG(0x754); */


	/* vIO32WriteFldAlign((ANA_INTF_1) ,0,RG_CKDT_SET); */
	ANA_BIT_CLR(0x4, 27);
	/* PRINT_REG(0x744); */
	/* vIO32WriteFldAlign((ANA_2F0) ,0,RG_HDMI_0_EQ_CALEN); */
	ANA_BIT_CLR(0x14, 12);
	/* PRINT_REG(0x754); */

}




UINT32 HDMI_HalGetXclkCnt(void)
{
	UINT32 u4XclkInPclk = 0;

	UINT32 u4Value_h = 0;
	UINT32 u4Value_l = 0;

	u4Value_h = (HDMI_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_10_8) >> 16;
	u4Value_l = (HDMI_READ32(REG_VID_CRC_OUT) & AAC_XCLK_IN_PCLK_7_0) >> 24;

	u4XclkInPclk = (u4Value_h << 8) | u4Value_l;
	u4XclkInPclk = u4XclkInPclk + 1;

	return u4XclkInPclk;
}

UINT32 HDMI_HalGetTmdsPeriod(void)
{
	UINT32 u4TmdsPeriod = 0;

	u4TmdsPeriod = (HDMI_READ32(REG_N_HDMI_CTRL4) & TMDS_CK_PERIOD) >> 12;

	return u4TmdsPeriod;
}


UINT32 HDMI_HalGetTmdsClockExt(void)
{
	UINT32 u4TmdsPeriod = 0;
	UINT32 u4TmdsClock = 0;

	u4TmdsPeriod = HDMI_HalGetTmdsPeriod();

	if (u4TmdsPeriod != 0) {
		u4TmdsClock = (27 * 1000 * 1000) * 64 / u4TmdsPeriod;
	} else {
		u4TmdsClock = 0;
	}

	return u4TmdsClock;
}

UINT32 HDMI_HalGetPixelClockExt(void)
{
	UINT32 u4PclkFreq = 0;
	UINT32 u4PclkNum = 0;
	UINT32 u4XclkFreq = 27 * 1000 * 1000; /* 27M crystal */
	UINT32 u4XclkNum = 0;
	UINT32 u4DeepColor = 0; 

	if (HDMI_READ32(REG_VID_CRC_OUT) & XCLK_IN_PCLK_SEL) {
		u4PclkNum = 1024;
	} else {
		u4PclkNum = 128;
	}

	u4XclkNum = HDMI_HalGetXclkCnt();

	/*  caculate pixel clock,  u4XclkNum * (1/u4XclkFreq) = u4PclkNum * (1/u4PclkFreq) */
	if ((u4XclkNum != 0) && (u4XclkNum != 1)) {
		u4PclkFreq = u4PclkNum * (u4XclkFreq / 1000) / u4XclkNum * 1000; /*  for range of 0xffffffff */
		u4DeepColor = HDMI_HalGetDeepColorBpp();
		switch(u4DeepColor)
		{
		case 0:
				u4PclkFreq = u4PclkFreq;
				break;
		case 1:
				u4PclkFreq = u4PclkFreq / 30 * 24;/*30bpp*/
				break;
		case 2:
				u4PclkFreq = u4PclkFreq / 36 * 24;/*36bpp*/
				break;
		case 3:
				u4PclkFreq = u4PclkFreq / 48 * 24;/*48bpp*/
				break;
		}
	} else {
		u4PclkFreq = 0;
	}

	return u4PclkFreq;
}

UINT32 HDMI_HalGetHtotalExt(void)
{
	UINT32 u4Htotal = 0;

	u4Htotal = HDMI_HalGetHTotal();

	if (HDMI_HalIsPclk2XRepeat()) {
		u4Htotal = u4Htotal * 2;
	}

	return u4Htotal;
}

UINT32 HDMI_HalGetVsyncFreq(void)
{
	UINT32 u4PixelFreq = 0;
	UINT32 u4Htotal = 0;
	UINT32 u4Vtotal = 0;
	UINT32 rate = 0;
	UINT32 deepColor = 0;

	u4PixelFreq = HDMI_HalGetPixelClockExt();
	u4Htotal = HDMI_HalGetHtotalExt();
	u4Vtotal = HDMI_HalGetVTotal();

	HDMI_LOG(HDMI_LOG_INFO, "pf:%u h:%u v:%u \r\n",
		(unsigned int)u4PixelFreq, (unsigned int)u4Htotal, (unsigned int)u4Vtotal);
	deepColor = HDMI_HalGetDeepColorBpp();

	if (deepColor == 1) {
		HDMI_LOG(HDMI_LOG_INFO, "30 BIT Deep Color \r\n");
		u4PixelFreq = (u4PixelFreq / 5) * 4;
	} else if (deepColor == 2) {
		HDMI_LOG(HDMI_LOG_INFO, "36 BIT Deep Color \r\n");
		u4PixelFreq = (u4PixelFreq / 3) * 2;
	} else if (deepColor == 3) {
		HDMI_LOG(HDMI_LOG_INFO, "48 BIT Deep Color \r\n");
		u4PixelFreq = (u4PixelFreq / 2);
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "24 BIT Deep Color\r\n");
	}

	rate = ((u4Vtotal != 0) && (u4Htotal != 0)) ? (u4PixelFreq / u4Vtotal / u4Htotal) : 0;
	HDMI_LOG(HDMI_LOG_INFO, "Cacl rate : %u \r\n", (unsigned int)rate);

	if ((rate <= 51) && (rate >= 49)) {
		rate = 50;
	} else if ((rate <= 57) && (rate >= 55)) {
		rate = 56;
	} else if ((rate <= 61) && (rate >= 59)) {
		rate = 60;
	} else if ((rate <= 68) && (rate >= 65)) {
		rate = 67;
	} else if ((rate <= 71) && (rate >= 69)) {
		rate = 70;
	} else if ((rate <= 73) && (rate >= 71)) {
		rate = 72;
	} else if ((rate <= 76) && (rate >= 74)) {
		rate = 75;
	} else if ((rate <= 86) && (rate >= 84)) {
		rate = 85;
	}

	return rate;

}

/* Set TMDS FIFO Read/Write Control pionter to be different */
void HDMI_HalSetTmdsFifoRWPointerDiff(void)
{
	HDMI_WRITE32_MASK(REG_TMDS_CTRL0, 0x1 << 31, 0x1 << 31); /* register no this addr */
}

/* Set TMDS FIFO Read/Write Control pionter free run */
void HDMI_HalSetTmdsFifoRWPointerFreeRun(void)
{
	HDMI_WRITE32_MASK(REG_TMDS_CTRL0, 0x0, 0x1 << 31);
}

void HDMI_HalClearRxPclkChgStatus(void)/* Clear Pixel clock change interrupt status bit */
{
	HDMI_WRITE32_MASK(REG_INTR_STATE0, 0x1 << 16, INTR2_CLK_CHG);
}

UINT8 HDMI_HalGetRxHdcpStatus(void)
{
	UINT32 u4Status = 0;

	u4Status = (HDMI_READ32(REG_HDCP_STAT) & (HDCP_DECRYPT | HDCP_AUTH)) >> 20;

	return (UINT8)u4Status;
}

BOOL HDMI_HalIsHdmiRXAuthDone(UINT8 u1Data)
{
	if ((u1Data & 0x30) == 0x30) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalChkAviInforFrameExist(void)
{
	UINT32 u4Header = 0;

	u4Header = HDMI_READ32(REG_AVIRX0) & CEA_AVI_HEADER;

	if ((u4Header & 0xff) == 0x82) {
		return TRUE;
	}

	return FALSE;

}

UINT8 HDMI_HalReadAviType(void)
{
	UINT32 u4Type = 0;

	u4Type = HDMI_READ32(REG_AVIRX0) & CEA_AVI_HEADER;
	u4Type &= 0x00FF;

	return (UINT8)u4Type;
}

UINT8 HDMI_HalReadAviVersion(void)
{
	UINT32 u4Version = 0;

	u4Version = HDMI_READ32(REG_AVIRX0) & CEA_AVI_HEADER;
	u4Version = (u4Version & 0xFF00) >> 8;

	return (UINT8)u4Version;
}

UINT8 HDMI_HalReadAviLength(void)
{
	UINT32 u4Length = 0;

	u4Length = (HDMI_READ32(REG_AVIRX0) & CEA_AVI_LENGTH) >> 16;

	return (UINT8)u4Length;
}

UINT8 HDMI_HalReadAviCheckSum(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX0) & CEA_AVI_CHECKSUM) >> 24;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX1) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX1) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte3(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX1) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte4(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX1) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte5(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX2) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte6(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX2) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte7(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX2) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte8(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX2) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte9(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX3) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte10(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX3) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte11(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX3) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte12(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX3) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte13(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX4) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAviByte14(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX4) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAviByte15(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AVIRX4) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfType(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_HEADER_7_0) >> 0;
	u4Value &= 0x00FF;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfVersion(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_HEADER_15_8) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfLength(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_LENGTH) >> 16;

	return (UINT8)u4Value;

}

UINT8 HDMI_HalReadAudioInfCheckSum(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_CHECKSUM) >> 24;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfByte1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX1) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX1) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfByte3(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX1) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte4(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX1) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte5(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX2) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte6(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX2) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAudioInfByte7(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX2) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte8(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX2) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte9(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX3) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAudioInfByte10(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_AUDRX3) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpHb0Header(void)/* ACP Packet Type =0x04 */
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX0) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadHb1HeaderAcpType(void)
	/* HB1 ACP Type: 0:Geberic Audio 1:IEC 60958 Idetified 2:DVD audio, 3:DSD audio */
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX0) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpHb2Header(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX0) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}



UINT8 HDMI_HalReadAcpPB0(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX0) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}



UINT8 HDMI_HalReadAcpPB1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX1) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}



UINT8 HDMI_HalReadAcpPB2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX1) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAcpPB3(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX1) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB4(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX1) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB5(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX2) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadAcpPB6(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX2) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB7(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX2) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB8(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX2) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB9(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX3) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB10(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX3) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB11(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX3) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB12(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX3) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB13(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX4) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB14(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX4) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadAcpPB15(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_ACPRX4) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}



UINT8 HDMI_HalReadSPDType(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX0) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadSPDVersion(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX0) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDLength(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX0) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX0) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadSPDByte2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX1) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte3(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX1) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte4(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX1) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte5(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX1) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte6(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX2) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte7(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX2) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte8(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX2) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}



UINT8 HDMI_HalReadSPDByte9(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX2) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte10(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX3) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte11(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX3) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte12(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX3) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte13(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX3) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadSPDByte14(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX4) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte15(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX4) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte16(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX4) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte17(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX4) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte18(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX5) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte19(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX5) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}

UINT8 HDMI_HalReadSPDByte20(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX5) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte21(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX5) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte22(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX6) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte23(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX6) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte24(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX6) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte25(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX6) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadSPDByte26(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_SPDRX7) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutHb0(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX0) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutHb1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX0) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutHb2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX0) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB0(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX0) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB1(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX1) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB2(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX1) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB3(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX1) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB4(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX1) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB5(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX2) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB6(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX2) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB7(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX2) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB8(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX2) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB9(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX3) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB10(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX3) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB11(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX3) & 0x00FF0000) >> 16;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB12(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX3) & 0xFF000000) >> 24;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB13(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX4) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}


UINT8 HDMI_HalReadGamutPB14(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_GAMUTRX4) & 0x0000FF00) >> 8;

	return (UINT8)u4Value;
}



/* Clear Interrupt, set 1 to clear */
void HDMI_HalClearNewAviIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR3_NEW_AVI);
}

void HDMI_HalClearNewAudIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR3_NEW_AUD);
}

void HDMI_HalClearNewSpdIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR3_NEW_SPD);
}

void HDMI_HalClearNewMpegIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR3_NEW_MPEG);
}

void HDMI_HalClearNewUnRecIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR3_NEW_UNREC);
}

void HDMI_HalClearNewAcpIntStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE1, INTR6_NEW_ACP);
}

void HDMI_HalClearNewVSIntStatus(void)
{
	UINT32 u4Mask = 0;

	/*  set 1 to clear interrupt, but MASK value should not be changed. */
	u4Mask = HDMI_READ32(REG_INTR_VS_ISRC1) |
		 INTR_VSYNC_MASK | INTR_NO_VS_PKT_MASK | INTR_NEW_VS_PKT_MASK | INTR_NEW_ISRC1_PKT_MASK;

	HDMI_WRITE32(REG_INTR_VS_ISRC1, INTR_NEW_VS_PKT | u4Mask);
}

void HDMI_HalClearNewNOVSIntStatus(void)
{
	UINT32 u4Mask = 0;

	/*  set 1 to clear interrupt, but MASK value should not be changed. */
	u4Mask = HDMI_READ32(REG_INTR_VS_ISRC1) |
		 INTR_VSYNC_MASK | INTR_NO_VS_PKT_MASK | INTR_NEW_VS_PKT_MASK | INTR_NEW_ISRC1_PKT_MASK;

	HDMI_WRITE32(REG_INTR_VS_ISRC1, INTR_NO_VS_PKT | u4Mask);
}

void HDMI_HalClearNewISRC1IntStatus(void)
{
	UINT32 u4Mask = 0;

	/*  set 1 to clear interrupt, but MASK value should not be changed. */
	u4Mask = HDMI_READ32(REG_INTR_VS_ISRC1) |
		 INTR_VSYNC_MASK | INTR_NO_VS_PKT_MASK | INTR_NEW_VS_PKT_MASK | INTR_NEW_ISRC1_PKT_MASK;

	HDMI_WRITE32(REG_INTR_VS_ISRC1, INTR_NEW_ISRC1_PKT | u4Mask);
}


void HDMI_HalClearVSYNCIntStatus(void)
{
	UINT32 u4Mask = 0;

	/*  set 1 to clear interrupt, but MASK value should not be changed. */
	u4Mask = HDMI_READ32(REG_INTR_VS_ISRC1) |
		 INTR_VSYNC_MASK | INTR_NO_VS_PKT_MASK | INTR_NEW_VS_PKT_MASK | INTR_NEW_ISRC1_PKT_MASK;

	HDMI_WRITE32(REG_INTR_VS_ISRC1, INTR_VSYNC | u4Mask);
}

/* this function need to be checked, mtk68528 */
void HDMI_HalSetVSNewOnly(BOOL fgEnable)
{
	if (fgEnable) {
		HDMI_WRITE32_MASK(REG_ISRC1RX0, 0x1 << 1, REG_NEW_VS_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_ISRC1RX0, 0x0, REG_NEW_VS_ONLY);
	}
}

/* this function need to be checked, mtk68528 */
void HDMI_HalSetISRC1NewOnly(BOOL fgEnable)
{
	if (fgEnable) {
		HDMI_WRITE32_MASK(REG_ISRC1RX0, 0x1 << 0, REG_NEW_ISRC1_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_ISRC1RX0, 0x0, REG_NEW_ISRC1_ONLY);
	}
}

BOOL HDMI_HalCheckIsPclkChanged(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_CLK_CHG) {
		return TRUE;
	}

	return FALSE;
}

void HDMI_HalClearPclkChangedIntState(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR2_CLK_CHG);
}

BOOL HDMI_HalIsHdmiMode(void)
{
	if (HDMI_READ32(REG_AUDP_STAT) & HDMI_MODE_DET) {
		return TRUE;        /* hdmi mode */
	}

	return FALSE; /* dvi mode */
}

BOOL HDMI_HalIsGcpMuteEnable(void)
{
	if (HDMI_READ32(REG_AUDP_STAT) & HDMI_MUTE) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsNewAcp(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_NEW_ACP) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsAcpInforFrameExist(void)
{
	if (HDMI_HalReadAcpHb0Header() == 0x4) {
		return TRUE;
	}

	return FALSE;
}

UINT8 HDMI_HalGetAcpType(void)
{
	return HDMI_HalReadHb1HeaderAcpType();
}


UINT8 HDMI_HalGetAcptHeader(void)
{
	return HDMI_HalReadAcpHb0Header();
}

void HDMI_HalGetAcpPacket(UINT8 *pu1AcpPacketData)
{
	*(pu1AcpPacketData + 0) = HDMI_HalReadAcpHb0Header();
	*(pu1AcpPacketData + 1) = HDMI_HalReadHb1HeaderAcpType();
	*(pu1AcpPacketData + 2) = HDMI_HalReadAcpHb2Header();
	*(pu1AcpPacketData + 3) = HDMI_HalReadAcpPB0();
	*(pu1AcpPacketData + 4) = HDMI_HalReadAcpPB1();
	*(pu1AcpPacketData + 5) = HDMI_HalReadAcpPB2();
	*(pu1AcpPacketData + 6) = HDMI_HalReadAcpPB3();
	*(pu1AcpPacketData + 7) = HDMI_HalReadAcpPB4();
	*(pu1AcpPacketData + 8) = HDMI_HalReadAcpPB5();
	*(pu1AcpPacketData + 9) = HDMI_HalReadAcpPB6();
	*(pu1AcpPacketData + 10) = HDMI_HalReadAcpPB7();
	*(pu1AcpPacketData + 11) = HDMI_HalReadAcpPB8();
	*(pu1AcpPacketData + 12) = HDMI_HalReadAcpPB9();
	*(pu1AcpPacketData + 13) = HDMI_HalReadAcpPB10();
	*(pu1AcpPacketData + 14) = HDMI_HalReadAcpPB11();
	*(pu1AcpPacketData + 15) = HDMI_HalReadAcpPB12();
	*(pu1AcpPacketData + 16) = HDMI_HalReadAcpPB13();
	*(pu1AcpPacketData + 17) = HDMI_HalReadAcpPB14();
	*(pu1AcpPacketData + 18) = HDMI_HalReadAcpPB15();
	*(pu1AcpPacketData + 19) = 0;
	*(pu1AcpPacketData + 20) = 0;
	*(pu1AcpPacketData + 21) = 0;
	*(pu1AcpPacketData + 22) = 0;
	*(pu1AcpPacketData + 23) = 0;
	/* ... max is 31 bytes */
}

/* select decode type */
/*void HDMI_HalSelectAcppacket(UINT8 bHeader)
{
	HDMI_WRITE32_MASK(REG_ACPRX7, bHeader, ACP_DEC);
}*/


void HDMI_HalGetAviInfoframe(UINT8 *bAviinfoframe)
{
	*(bAviinfoframe + 0) = HDMI_HalReadAviType();
	*(bAviinfoframe + 1) = HDMI_HalReadAviVersion();
	*(bAviinfoframe + 2) = HDMI_HalReadAviLength();
	*(bAviinfoframe + 3) = HDMI_HalReadAviCheckSum();
	*(bAviinfoframe + 4) = HDMI_HalReadAviByte1();
	*(bAviinfoframe + 5) = HDMI_HalReadAviByte2();
	*(bAviinfoframe + 6) = HDMI_HalReadAviByte3();
	*(bAviinfoframe + 7) = HDMI_HalReadAviByte4();
	*(bAviinfoframe + 8) = HDMI_HalReadAviByte5();
	*(bAviinfoframe + 9) = HDMI_HalReadAviByte6();
	*(bAviinfoframe + 10) = HDMI_HalReadAviByte7();
	*(bAviinfoframe + 11) = HDMI_HalReadAviByte8();
	*(bAviinfoframe + 12) = HDMI_HalReadAviByte9();
	*(bAviinfoframe + 13) = HDMI_HalReadAviByte10();
	*(bAviinfoframe + 14) = HDMI_HalReadAviByte11();
	*(bAviinfoframe + 15) = HDMI_HalReadAviByte12();
	*(bAviinfoframe + 16) = HDMI_HalReadAviByte13();
	*(bAviinfoframe + 17) = HDMI_HalReadAviByte14();
	*(bAviinfoframe + 18) = HDMI_HalReadAviByte15();

	/*,,, 19 bytes */
}

void HDMI_HalGetAudioInfoframe(BYTE *bAudioInfoframe)
{
	*(bAudioInfoframe + 0) = HDMI_HalReadAudioInfType();
	*(bAudioInfoframe + 1) = HDMI_HalReadAudioInfVersion();
	*(bAudioInfoframe + 2) = HDMI_HalReadAudioInfLength();
	*(bAudioInfoframe + 3) = HDMI_HalReadAudioInfCheckSum();
	*(bAudioInfoframe + 4) = HDMI_HalReadAudioInfByte1();
	*(bAudioInfoframe + 5) = HDMI_HalReadAudioInfByte2();
	*(bAudioInfoframe + 6) = HDMI_HalReadAudioInfByte3();
	*(bAudioInfoframe + 7) = HDMI_HalReadAudioInfByte4();
	*(bAudioInfoframe + 8) = HDMI_HalReadAudioInfByte5();
	*(bAudioInfoframe + 9) = HDMI_HalReadAudioInfByte6();
	*(bAudioInfoframe + 10) = HDMI_HalReadAudioInfByte7();
	*(bAudioInfoframe + 11) = HDMI_HalReadAudioInfByte8();
	*(bAudioInfoframe + 12) = HDMI_HalReadAudioInfByte9();
	*(bAudioInfoframe + 13) = HDMI_HalReadAudioInfByte10();
	/*,,, 14 bytes */
}

void HDMI_HalGetVSInfoframe(BYTE *bVSinfoframe)
{
	*(UINT32 *)(bVSinfoframe + 0) = HDMI_READ32(REG_VSRX0);
	*(UINT32 *)(bVSinfoframe + 4) = HDMI_READ32(REG_VSRX1);
	*(UINT32 *)(bVSinfoframe + 8) = HDMI_READ32(REG_VSRX2);
	*(UINT32 *)(bVSinfoframe + 12) = HDMI_READ32(REG_VSRX3);
	*(UINT32 *)(bVSinfoframe + 16) = HDMI_READ32(REG_VSRX4);
	*(UINT32 *)(bVSinfoframe + 20) = HDMI_READ32(REG_VSRX5);
	*(UINT32 *)(bVSinfoframe + 24) = HDMI_READ32(REG_VSRX6);
	*(bVSinfoframe + 28) = (HDMI_READ32(REG_VSRX7) & 0x0000FF) >> 0;
	*(bVSinfoframe + 29) = (HDMI_READ32(REG_VSRX7) & 0x00FF00) >> 8;
	*(bVSinfoframe + 30) = (HDMI_READ32(REG_VSRX7) & 0xFF0000) >> 16;
	/* ... 31 bytes */
}

void HDMI_HalGetISRC1Infoframe(BYTE *bISRC1infoframe)
{
	*(UINT16 *)(bISRC1infoframe + 0) = (HDMI_READ32(REG_ISRC1RX0) & 0xFFFF0000) >> 16;
	*(UINT32 *)(bISRC1infoframe + 2) = HDMI_READ32(REG_ISRC1RX1);
	*(UINT32 *)(bISRC1infoframe + 6) = HDMI_READ32(REG_ISRC1RX2);
	*(UINT32 *)(bISRC1infoframe + 10) = HDMI_READ32(REG_ISRC1RX3);
	*(UINT32 *)(bISRC1infoframe + 14) = HDMI_READ32(REG_ISRC1RX4);
}


void HDMI_HalGetMpegInfoframe(BYTE *bMpegInfoframeData)
{
	*(UINT32 *)(bMpegInfoframeData + 0) = HDMI_READ32(REG_MPEGRX0);
	*(UINT32 *)(bMpegInfoframeData + 4) = HDMI_READ32(REG_MPEGRX1);
	*(UINT32 *)(bMpegInfoframeData + 8) = HDMI_READ32(REG_MPEGRX2);
	*(UINT32 *)(bMpegInfoframeData + 12) = HDMI_READ32(REG_MPEGRX3);
	*(UINT32 *)(bMpegInfoframeData + 16) = HDMI_READ32(REG_MPEGRX4);
	*(UINT32 *)(bMpegInfoframeData + 20) = HDMI_READ32(REG_MPEGRX5);
	*(UINT32 *)(bMpegInfoframeData + 24) = HDMI_READ32(REG_MPEGRX6);
	*(UINT32 *)(bMpegInfoframeData + 28) = HDMI_READ32(REG_MPEGRX7);
}

void HDMI_HalGetSpdInfoframe(BYTE *bSpdInfoframeData)
{
	*(UINT32 *)(bSpdInfoframeData + 0) = HDMI_READ32(REG_SPDRX0);
	*(UINT32 *)(bSpdInfoframeData + 4) = HDMI_READ32(REG_SPDRX1);
	*(UINT32 *)(bSpdInfoframeData + 8) = HDMI_READ32(REG_SPDRX2);
	*(UINT32 *)(bSpdInfoframeData + 12) = HDMI_READ32(REG_SPDRX3);
	*(UINT32 *)(bSpdInfoframeData + 16) = HDMI_READ32(REG_SPDRX4);
	*(UINT32 *)(bSpdInfoframeData + 20) = HDMI_READ32(REG_SPDRX5);
	*(UINT32 *)(bSpdInfoframeData + 24) = HDMI_READ32(REG_SPDRX6);
	*(UINT32 *)(bSpdInfoframeData + 28) = HDMI_READ32(REG_SPDRX7);
}

void HDMI_HalGetGamutPacket(BYTE *bGamutData)
{
	*(UINT32 *)(bGamutData + 0) = HDMI_READ32(REG_GAMUTRX0);
	*(UINT32 *)(bGamutData + 4) = HDMI_READ32(REG_GAMUTRX1);
	*(UINT32 *)(bGamutData + 8) = HDMI_READ32(REG_GAMUTRX2);
	*(UINT32 *)(bGamutData + 12) = HDMI_READ32(REG_GAMUTRX3);
	*(UINT32 *)(bGamutData + 16) = HDMI_READ32(REG_GAMUTRX4);
	*(UINT32 *)(bGamutData + 20) = HDMI_READ32(REG_GAMUTRX5);
	*(UINT32 *)(bGamutData + 24) = HDMI_READ32(REG_GAMUTRX6);
	*(UINT32 *)(bGamutData + 28) = HDMI_READ32(REG_GAMUTRX7);
}

void HDMI_HalGetUnRecPacket(BYTE *bUnRecPacketData)
{
	*(UINT32 *)(bUnRecPacketData + 0) = HDMI_READ32(REG_UNRECRX0);
	*(UINT32 *)(bUnRecPacketData + 4) = HDMI_READ32(REG_UNRECRX1);
	*(UINT32 *)(bUnRecPacketData + 8) = HDMI_READ32(REG_UNRECRX2);
	*(UINT32 *)(bUnRecPacketData + 12) = HDMI_READ32(REG_UNRECRX3);
	*(UINT32 *)(bUnRecPacketData + 16) = HDMI_READ32(REG_UNRECRX4);
	*(UINT32 *)(bUnRecPacketData + 20) = HDMI_READ32(REG_UNRECRX5);
	*(UINT32 *)(bUnRecPacketData + 24) = HDMI_READ32(REG_UNRECRX6);
	*(UINT32 *)(bUnRecPacketData + 28) = HDMI_READ32(REG_UNRECRX7);
}

UINT32 HDMI_HalGetHfreqExt(void)
{
	UINT32 u4PixelFreq = 0;
	UINT32 u4Htotal = 0;
	UINT32 u4HsyncFreq = 0;
	UINT32 u4Vtotal = 0;
	UINT32 u4VsyncFreq = 0;

	u4PixelFreq = HDMI_HalGetPixelClockExt();
	u4Htotal = HDMI_HalGetHtotalExt();
	u4Vtotal = HDMI_HalGetVTotal();
	u4HsyncFreq = (u4Htotal != 0)? (u4PixelFreq / u4Htotal) : 0;
	u4VsyncFreq = ((u4Vtotal != 0) && (u4Htotal != 0))? (u4PixelFreq / u4Vtotal /u4Htotal) : 0;

	return u4HsyncFreq;
}

UINT32 HDMI_HalGetVfreqExt(void)
{
	UINT32 u4PixelFreq = 0;
	UINT32 u4Htotal = 0;
	UINT32 u4HsyncFreq = 0;
	UINT32 u4Vtotal = 0;
	UINT32 u4VsyncFreq = 0;

	u4PixelFreq = HDMI_HalGetPixelClockExt();
	u4Htotal = HDMI_HalGetHtotalExt();
	u4Vtotal = HDMI_HalGetVTotal();
	u4HsyncFreq = (u4Htotal != 0)? (u4PixelFreq / u4Htotal) : 0;
	u4VsyncFreq = ((u4Vtotal != 0) && (u4Htotal != 0))? (u4PixelFreq / u4Vtotal /u4Htotal) : 0;

    return u4VsyncFreq;
}
BOOL  HDMI_HalGetAudioInfoFrameExt(Audio_InfoFrame *pAudioInfoFrame)
{
	BYTE checksum;
	int i;

	if (pAudioInfoFrame == NULL) {
		return ER_FAIL;
	}

	pAudioInfoFrame->pktbyte.AUD_HB[0] = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_HEADER_7_0) >> 0;
	/* above,  AUDIO InfoFrame Type Code. Required 0x84 : HDMIRX_AUD_INFOFRAME_TYPE */
	pAudioInfoFrame->pktbyte.AUD_HB[1] = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_HEADER_15_8) >> 8;
	/*  above,  AUDIO InfoFrame Version Code. Required 0x01 : HDMIRX_AUD_INFOFRAME_VER */
	pAudioInfoFrame->pktbyte.AUD_HB[2] = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_LENGTH) >> 16;
	/*  above,  AUDIO InfoFrame Length. Required 0x0A : HDMIRX_AUD_INFOFRAME_LEN */
	checksum = (HDMI_READ32(REG_AUDRX0) & CEA_AUD_CHECKSUM) >> 24;
	/* above,  AUDIO InfoFrame Checksum. : HDMIRX_AUD_INFOFRAME_CHKSUM */

	/* HDMIRX_AUD_INFOFRAME_DB1~BD10 */
	pAudioInfoFrame->pktbyte.AUD_DB[0] = (HDMI_READ32(REG_AUDRX1) & CEA_AUD_DBYTE1) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[1] = (HDMI_READ32(REG_AUDRX1) & CEA_AUD_DBYTE2) >> 8;
	pAudioInfoFrame->pktbyte.AUD_DB[2] = (HDMI_READ32(REG_AUDRX1) & CEA_AUD_DBYTE3) >> 16;
	pAudioInfoFrame->pktbyte.AUD_DB[3] = (HDMI_READ32(REG_AUDRX1) & CEA_AUD_DBYTE4) >> 24;
	pAudioInfoFrame->pktbyte.AUD_DB[4] = (HDMI_READ32(REG_AUDRX2) & CEA_AUD_DBYTE5) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[5] = (HDMI_READ32(REG_AUDRX2) & CEA_AUD_DBYTE6) >> 8;
	pAudioInfoFrame->pktbyte.AUD_DB[6] = (HDMI_READ32(REG_AUDRX2) & CEA_AUD_DBYTE7) >> 16;
	pAudioInfoFrame->pktbyte.AUD_DB[7] = (HDMI_READ32(REG_AUDRX2) & CEA_AUD_DBYTE8) >> 24;
	pAudioInfoFrame->pktbyte.AUD_DB[8] = (HDMI_READ32(REG_AUDRX3) & CEA_AUD_DBYTE9) >> 0;
	pAudioInfoFrame->pktbyte.AUD_DB[9] = (HDMI_READ32(REG_AUDRX3) & CEA_AUD_DBYTE10) >> 8;

	/* config info value */
	pAudioInfoFrame->info.Type = pAudioInfoFrame->pktbyte.AUD_HB[0];
	pAudioInfoFrame->info.Ver = pAudioInfoFrame->pktbyte.AUD_HB[1];
	pAudioInfoFrame->info.Len = pAudioInfoFrame->pktbyte.AUD_HB[2];
	pAudioInfoFrame->info.AudioChannelCount = pAudioInfoFrame->pktbyte.AUD_DB[0] & 0x07;

	pAudioInfoFrame->info.RSVD1 = 0;
	pAudioInfoFrame->info.AudioCodingType = (pAudioInfoFrame->pktbyte.AUD_DB[0] >> 4) & 0x0f;
	pAudioInfoFrame->info.SampleSize = pAudioInfoFrame->pktbyte.AUD_DB[1] & 0x03;
	pAudioInfoFrame->info.SampleFreq = (pAudioInfoFrame->pktbyte.AUD_DB[1] >> 2) & 0x07;
	pAudioInfoFrame->info.Rsvd2 = 0;
	pAudioInfoFrame->info.FmtCoding = (pAudioInfoFrame->pktbyte.AUD_DB[0] >> 4) & 0x0f;
	/*above, actually spec does not have this */
	pAudioInfoFrame->info.SpeakerPlacement = pAudioInfoFrame->pktbyte.AUD_DB[3];
	pAudioInfoFrame->info.Rsvd3 = 0;
	pAudioInfoFrame->info.LevelShiftValue = (pAudioInfoFrame->pktbyte.AUD_DB[4] >> 3) & 0x0f;
	pAudioInfoFrame->info.DM_INH = (pAudioInfoFrame->pktbyte.AUD_DB[4] >> 7) & 0x01;

	for (i = 0; i < 3; i++) {
		checksum += pAudioInfoFrame->pktbyte.AUD_HB[i];
	}

	for (i = 0; i < 5; i++) { /* only 5 bytes is valued */
		checksum += pAudioInfoFrame->pktbyte.AUD_DB[i];
	}

	if (checksum == 0) {
		return 0;
	} else {
		return 1;
	}

}





UINT8 HDMI_HalGetMpegAddrHeader(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_MPEGRX0) & 0x000000FF) >> 0;

	return (UINT8)u4Value;
}

void HDMI_HalMpegAddrSetSelectPacket(BYTE bHeader)
{
	UINT32 u4Value = bHeader & MPEG_DEC;
	HDMI_WRITE32(REG_MPEGRX7, (u4Value| (HDMI_READ32(REG_MPEGRX7) & (~MPEG_DEC))));
	/*HDMI_WRITE32_MASK(REG_MPEGRX7, bHeader, MPEG_DEC);*/
}

UINT8 HDMI_HalMpegAddrGetSelectPacket(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_MPEGRX7) & MPEG_DEC) >> 24;

	return (UINT8)u4Value;
}

UINT32 HDMI_HalGetRxHwCTSValue(void)
{
	UINT32 u4Value  = 0;

	u4Value = (HDMI_READ32(REG_CTS_HVAL) & CTS_VAL_HW_19_0) >> 0;

	return u4Value;
}

UINT32 HDMI_HalGetRxHwNValue(void)
{
	UINT32 u4Value  = 0;

	u4Value = /*(((HDMI_READ32(REG_N_HVAL) & N_VAL_HW_19_16) >> 0) >> 16)  ||*/ /*can not be true*/
		  ((HDMI_READ32(REG_N_SVAL) & N_VAL_HW_15_0) >> 16);

	return u4Value;
}

BOOL HDMI_HalIsNotDeepColorMode(void)
{
	UINT32 u4Mode = 0;

	u4Mode = (HDMI_READ32(REG_SRST) & DEEP_STA) >> 28;

	switch (u4Mode) {
	case 0: /*  24bit mode */
		return TRUE;

	case 1: /* 30bit mode */
		return FALSE;

	case 2: /* 36bit mode */
		return FALSE;

	case 3: /* 48bit mode */
		return FALSE;

	default:
		break;
	}

	return FALSE;
}

UINT32 HDMI_HalGetDeepColorBpp(void)
{
	UINT32 u4Mode = 0;

	u4Mode = (HDMI_READ32(REG_SRST) & DEEP_STA) >> 28;

	return u4Mode;
}

/* this function need to be edit, mtk68528 */
void HDMI_HalReInitAudioClock(void)
{
#if 0
	vRegWriteFldAlign(PD_SYS, 0x0, PD_APLL);
	vRegWriteFldAlign(AUD_INTF_1, 0x0, RG_HAPLL_VCOCAL_EN);

	vRegWriteFldAlign(PD_SYS, 0x1, PD_APLL);
	vRegWriteFldAlign(AUD_INTF_1, 0x1, RG_HAPLL_VCOCAL_EN);
	vRegWrite1B(SRST_1, 0x26); /* mark by ciwu */
	vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_UNDERRUN));/* vRegWrite4B(INTR_MASK0, Fld2Msk32(INTR4_UNDERRUN)); */
	vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_OVERRUN));
	vRegWrite1B(SRST_1, 0x00); /* mark by ciwu */
#endif

}

BOOL HDMI_HalIsNoAvi(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_NO_AVI) {
		return TRUE;
	}

	return FALSE;
}


void HDMI_HalClearVideoModeByte0(void)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0xFF << 0, 0x000000FF);
}

void HDMI_HalClearVideoModeByte1(void)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0xFF << 8, 0x0000FF00);
}

void HDMI_HalClearVideoModeByte2(void)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0xFF << 16, 0x00FF0000);
}

void HDMI_HalClearVideoModeByte3(void)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0xFF << 24, 0xFF000000);
}

void HDMI_HalClearIntrState1Bit0_Bit7(void)
{
	UINT32 u4Value = 0;

	u4Value = HDMI_READ32(REG_INTR_STATE1) & 0x000000FF;
	HDMI_WRITE32(REG_INTR_STATE1, u4Value);
}

void HDMI_HalDisableEncodeSync(void)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0x0, ENSYNCCODES);
}

void HDMI_HalRxDisable422UpSample(void) /* 422 to 444 Up sample */
{
	HDMI_WRITE32_MASK(REG_VID_MODE, 0x0, ENUPSAMPLE);
}

void HDMI_HalSetRxRGBBlankValue(UINT8 u1Blue , UINT8 u1Green, UINT8 u1Red)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, u1Blue << 24, BLANKDATA1);
	HDMI_WRITE32_MASK(REG_VID_BLANK, u1Green << 0, BLANKDATA2);
	HDMI_WRITE32_MASK(REG_VID_BLANK, u1Red << 8, BLANKDATA3);
}

void HDMI_HalSetRxYCbCrBlankValue(UINT8 u1Cb , UINT8 u1Y, UINT8 u1Cr)
{
	HDMI_WRITE32_MASK(REG_VID_MODE, u1Cb << 24, BLANKDATA1);
	HDMI_WRITE32_MASK(REG_VID_BLANK, u1Y << 0, BLANKDATA2);
	HDMI_WRITE32_MASK(REG_VID_BLANK, u1Cr << 8, BLANKDATA3);
}

void HDMI_HalSetRxPclk2XRepeat(BOOL fgEn)
{
	/* 0- 1xpclk, 1- 2xpclk, 2-rsvd, 3- 4xpclk */
	/* HDMI_WRITE32_MASK(REG_SYS_CTRL, u1Data<<4, IDCK); */
	if (fgEn) {
		HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x1 << 4, IDCK);
	} else {
		HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x0 << 4, IDCK);
	}
}

BOOL HDMI_HalIsPclk2XRepeat(void)
{
	if ((HDMI_READ32(REG_SYS_CTRL) & IDCK) >> 4 == 0x1) {
		return TRUE;
	}

	return FALSE;
}

void HDMI_HalSetVideoChannelMap(UINT8 u1Data)
{
	HDMI_WRITE32_MASK(REG_VID_CH_MAP, u1Data << 16, CHANNEL_MAP);
}

BOOL HDMI_HalIsHResChg(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_HRESCHG) {
		return TRUE;
	}

	return FALSE;
}

void HDMI_HalClearHresChgIntrState(void)
{
	HDMI_WRITE32(REG_INTR_STATE1, INTR5_HRESCHG);
}

void HDMI_HalResetTDFifoAutoRead(void)
{
	HDMI_WRITE32_MASK(REG_MUTE_SET, 0x1 << 20, TDFIFO_SYNC_EN);
	HDMI_WRITE32_MASK(REG_MUTE_SET, 0x0, TDFIFO_SYNC_EN);
}

void HDMI_HalSetTDFifoAutoReadEnable(BOOL fgEn)
{
	if (fgEn) {
		HDMI_WRITE32_MASK(REG_MUTE_SET, 0x1 << 20, TDFIFO_SYNC_EN);
	} else {
		HDMI_WRITE32_MASK(REG_MUTE_SET, 0x0, TDFIFO_SYNC_EN);
	}
}

UINT32 HDMI_HalGetActiveWidth(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_BLANK) & VID_PIXELS) >> 16;

	return u4Value;
}

UINT32 HDMI_HalGetActiveHeight(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_STAT) & VID_DELINES) >> 0;

	return u4Value;
}

UINT32 HDMI_HalGetVFrontPorch(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_STAT) & VFRONTPORCH) >> 24;

	return u4Value;
}

UINT32 HDMI_HalGetVBackPorch(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_STAT) & V2ACTIVELINES) >> 16;

	return u4Value;
}

UINT32 HDMI_HalGetHFrontPorch(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_HFP) & VID_HFRONTPORCH) >> 8;

	return u4Value;
}

UINT32 HDMI_HalGetHSyncWidth(void)
{
	UINT32 u4ValueH = 0;
	UINT32 u4ValueL = 0;

	u4ValueH = (HDMI_READ32(REG_VID_AOF) & VID_HSACTIVEWIDTH_9_8) >> 0;
	u4ValueL = (HDMI_READ32(REG_VID_HFP) & VID_HSACTIVEWIDTH_7_0) >> 24;

	return ((u4ValueH << 8) | u4ValueL);
}




UINT32 HDMI_HalGetHTotal(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_HRES) & VID_HRES_12_0) >> 16;

	return u4Value;
}

UINT32 HDMI_HalGetVTotal(void)
{
	UINT32 u4Value = 0;

	u4Value = (HDMI_READ32(REG_VID_VRES) & VID_VRES_11_0) >> 0;

	return u4Value;
}

BOOL HDMI_HalIsHresStable(void)
{
	if (HDMI_READ32(REG_VID_HRES) & VID_HRES_STB) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsVresStable(void)
{
	if (HDMI_READ32(REG_VID_VRES) & VID_VRES_STB) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsHdcpDecrptOn(void)
{
	if (HDMI_READ32(REG_HDCP_STAT) & HDCP_DECRYPT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsSCDTEnable(void)
{
	if (HDMI_READ32(REG_SRST) & SCDT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalGetHsyncPolarity(void)
{
	if (HDMI_READ32(REG_VID_CH_MAP) & HSYNCPOL) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalGetVsyncPolarity(void)
{
	if (HDMI_READ32(REG_VID_CH_MAP) & VSYNCPOL) {
		return TRUE;
	}

	return FALSE;
}


void HalEnableRxPhyRtckAuto(void)
{
	/* set reg 0x90 */
}

void HalDisableRxPhyRtckAuto(void)
{
	/* set reg 0x90 */
}

void HDMI_HalClearModeChgIntState(void)
{
	HDMI_WRITE32(REG_INTR_STATE0,
		     INTR2_HDMI_MODE | INTR2_CKDT | INTR2_SCDT | INTR2_CLK_CHG);
	HDMI_WRITE32(REG_INTR_STATE1,
		     INTR5_VRESCHG | INTR5_HRESCHG | INTR5_POLCHG | INTR5_INTERLACEOUT);
}

BOOL HDMI_HalIsVResStable(void)
{
	if (HDMI_READ32(REG_VID_VRES) & VID_VRES_STB) {
		return TRUE;
	}

	return FALSE;
}

BOOL HDMI_HalIsVResMute(void)
{
	if (HDMI_READ32(REG_VID_VRES) & VID_VRES_MUTE) {
		return TRUE;
	}

	return FALSE;
}

void HDMI_HalSetVResMute(void)
{
	HDMI_WRITE32_MASK(REG_VID_VRES, 0x1 << 22, VRES_MUTE_CLR);
}

void HDMI_HalClearVResMute(void)
{
	HDMI_WRITE32_MASK(REG_VID_VRES, 0x0, VRES_MUTE_CLR);
}

void HDMI_HalDisableRxAvMute(void)
{
	HDMI_WRITE32_MASK(REG_VID_SET, 0x1 << 30, AV_MUTE_CLR);
}

void HDMI_HalEnableAvMuteRecv(void)
{
	HDMI_WRITE32_MASK(REG_VID_SET, 0x0, AV_MUTE_CLR);
}

BOOL HDMI_HalIsInterlace(void)
{
	if (HDMI_READ32(REG_VID_CH_MAP) & INTERLACEDOUT) {
		return TRUE;
	}

	return FALSE;
}


void HalSetHDMIRXPowerOff(void)
{
	/* set power off, 0x390*/
}



/* Int Status */
BOOL HalIsINTR3_CEA_NEW_CP(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_CEA_NEW_CP) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR3_CP_SET_MUTE(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_CP_SET_MUTE) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR3_P_ERR(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_P_ERR) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR3_NEW_UNREC(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_NEW_UNREC) {
		return TRUE;
	}

	return FALSE;
}



BOOL HalIsINTR3_NEW_MPEG(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_NEW_MPEG) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR3_NEW_AUD(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_NEW_AUD) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR3_NEW_SPD(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_NEW_SPD) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR3_NEW_AVI(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR3_NEW_AVI) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_NEW_VS(void)
{
	if (HDMI_READ32(REG_INTR_VS_ISRC1) & INTR_NEW_VS_PKT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_VSYNC(void)
{
	if (HDMI_READ32(REG_INTR_VS_ISRC1) & INTR_VSYNC) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_NO_VS(void)
{
	if (HDMI_READ32(REG_INTR_VS_ISRC1) & INTR_NO_VS_PKT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_NEW_ISRC1(void)
{
	if (HDMI_READ32(REG_INTR_VS_ISRC1) & INTR_NEW_ISRC1_PKT) {
		return TRUE;
	}

	return FALSE;
}

void HalHDMIRxEnableVsyncInt(BOOL bEnable)
{
	if (bEnable == TRUE) {
		HDMI_WRITE32_MASK(REG_INTR_VS_ISRC1, 0x1 << 3, INTR_VSYNC_MASK);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_VS_ISRC1, 0x0, INTR_VSYNC_MASK);
	}
}


BOOL HalIsINTR2_HDMI_MODE(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_HDMI_MODE) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR2_VSYNC(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_VSYNC) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR2_SOFT_INTR_EN(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_SOFT_INTR_EN) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR2_CKDT(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_CKDT) {
		return TRUE;
	}

	return FALSE;
}
void  HalClearINTR2_CKDT(void)
{
	HDMI_WRITE32_MASK(REG_INTR_STATE0, 0x1 << 20, INTR2_CKDT);
}


void  HalEnableINTR2_CKDT(BOOL bEnable)
{
	if (bEnable == TRUE) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 20, INTR2_CKDT);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, INTR2_CKDT);
	}
}


BOOL HalIsINTR2_SCDT(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_SCDT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR2_GOT_CTS(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_GOT_CTS) {
		return TRUE;
	}

	return FALSE;
}


/* wangwj */
BOOL HalIsINTR2_NEW_AUD_PKT(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_NEW_AUD_PKT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR2_CLK_CHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_CLK_CHG) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR1_HW_CTS_CHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_HW_CTS_CHG) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR1_HW_N_CHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_HW_N_CHG) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR1_PKT_ERR(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_PKT_ERR) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR1_PLL_UNLOCKED(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_PLL_UNLOCKED) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR1_FIFO_ERR(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_FIFO_ERR) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR1_BCH_PKT_ERR_ALERT(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_BCH_PKT_ERR_ALERT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsSOFT_INTR_EN(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & SOFT_INTR_EN) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_OD(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR_OD) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR_POLARITY(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR_POLARITY) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR_STATE(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR_STATE) {
		return TRUE;
	}

	return FALSE;
}

UINT32 HalReadINTR_STATE0(void)
{
	return HDMI_READ32(REG_INTR_STATE0);
}


UINT32 HalReadINTR_STATE1(void)
{
	return HDMI_READ32(REG_INTR_STATE1);
}

BOOL HalIsINTR7_RATIO_ERROR(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_RATIO_ERROR) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR7_AUD_CH_STAT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_AUD_CH_STAT) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR7_GCP_CD_CHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_GCP_CD_CHG) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR7_GAMUT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_GAMUT) {
		return TRUE;
	}

	return FALSE;
}

void HalClearGamutIntStatus(void)
{
	HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x0, INTR7_GAMUT);
}

BOOL HalIsINTR7_HBR(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_HBR) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR7_SACD(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_SACD) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR6_PRE_UNDERUN(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_PRE_UNDERUN) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR6_PRE_OVERUN(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_PRE_OVERUN) {
		return TRUE;
	}

	return FALSE;
}



BOOL HalIsINTR6_NEW_ACP(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_NEW_ACP) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR6_P_ERR2(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_P_ERR2) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR6_PWR5V_RX0(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR6_PWR5V_RX0) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_FN_CHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_FN_CHG) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_AUDIO_MUTE(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_AUDIO_MUTE) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_BCH_AUDIO_ALERT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_BCH_AUDIO_ALERT) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_VRESCHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_VRESCHG) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_HRESCHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_HRESCHG) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR5_POLCHG(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_POLCHG) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_INTERLACEOUT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_INTERLACEOUT) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR5_AUD_SAMPLE_F(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_AUD_SAMPLE_F) {
		return TRUE;
	}

	return FALSE;

}

BOOL HalIsINTR4_PKT_RECEIVED_ALERT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_PKT_RECEIVED_ALERT) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR4_HDCP_PKT_ERR_ALERT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_HDCP_PKT_ERR_ALERT) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR4_T4_PKT_ERR_ALERT(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_T4_PKT_ERR_ALERT) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR4_NO_AVI(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_NO_AVI) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR4_CTS_DROPPED_ERR(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_CTS_DROPPED_ERR) {
		return TRUE;
	}

	return FALSE;
}

BOOL fgHalIsINTR4_CTS_REUSED_ERR(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_CTS_REUSED_ERR) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalIsINTR4_OVERRUN(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_OVERRUN) {
		return TRUE;
	}

	return FALSE;
}


BOOL HalIsINTR4_UNDERRUN(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR4_UNDERRUN) {
		return TRUE;
	}

	return FALSE;
}

void HalSetIntOnNewAviOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 0, NEW_AVI_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_AVI_ONLY);
	}

}


void HalSetIntOnNewAcpOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 5, NEW_ACP_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_ACP_ONLY);
	}

}

void HalSetIntOnNewSpdOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 1, NEW_SPD_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_SPD_ONLY);
	}
}

void HalSetIntOnNewAudioInfOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 2, NEW_AUD_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_AUD_ONLY);
	}
}

void HalSetIntOnNewMpegInfOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 3, NEW_MPEG_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_MPEG_ONLY);
	}
}

void HalSetIntOnNewUnrecInfOnlyEnable(BOOL bEnable)
{
	if (bEnable) {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x1 << 4, NEW_UNREC_ONLY);
	} else {
		HDMI_WRITE32_MASK(REG_INTR_MASK0, 0x0, NEW_UNREC_ONLY);
	}
}





BOOL HalHdmiRxCrc(INT16 ntry)
{
	/* to overwrite this function */
	UINT8 idx;
	UINT8 result[3][3];
	UINT8 tmp[3];

	idx = 0;
	result[0][0] = 0;
	result[0][1] = 0;
	result[0][2] = 0;
	result[1][0] = 0;
	result[1][1] = 0;
	result[1][2] = 0;
	result[2][0] = 0;
	result[2][1] = 0;
	result[2][2] = 0;

	_CrcResult[0][0] = 0;
	_CrcResult[0][1] = 0;
	_CrcResult[0][2] = 0;
	_CrcResult[1][0] = 0;
	_CrcResult[1][1] = 0;
	_CrcResult[1][2] = 0;
	_CrcResult[2][0] = 0;
	_CrcResult[2][1] = 0;
	_CrcResult[2][2] = 0;
	tmp[0] = 0;
	tmp[1] = 0;
	tmp[2] = 0;
	HDMI_LOG(HDMI_LOG_DEBUG, "fgHDMICRC: %d \r\n", ntry);

	/* Printf("fgHDMICRC: %d\n", ntry); */

	HDMI_CLR_BIT(REG_VID_CRC_CHK, 23);

	while (ntry > 0) {
		ntry--;
		/* vUtDelay10ms(1); // NOTE: IT IS NECESSARY */
		msleep(10);

		if (idx > 2) {
			/* Printf("CRC fail\n"); */
			_CrcResult[0][0] = result[0][0];
			_CrcResult[0][1] = result[0][1];
			_CrcResult[0][2] = result[0][2];
			_CrcResult[1][0] = result[1][0];
			_CrcResult[1][1] = result[1][1];
			_CrcResult[1][2] = result[1][2];
			_CrcResult[2][0] = tmp[0];
			_CrcResult[2][1] = tmp[1];
			_CrcResult[2][2] = tmp[2];
			return 0;
		}

		/* vRegWrite1B(VID_CRC_CHK_2, 0x8c);// clr */
		HDMI_WRITE32_MASK(REG_VID_CRC_CHK, (0x8c << 16), (0xff << 16));
		/* PRINT_REG(0x22c68); */

		while ((HDMI_READ32(REG_VID_CRC_CHK) & (1 << 23)) != 0x0) {
			msleep(1);
		}

		while ((HDMI_READ32(REG_VID_CRC_CHK) & (0xff << 24)) != 0x00) {
			msleep(1);
		}

		while ((HDMI_READ32(REG_VID_CRC_CHK) & (0xff))  != 0x00) {
			msleep(1);
		}

		while ((HDMI_READ32(REG_VID_CRC_CHK) & (0xff << 8)) != 0x00) {
			msleep(1);
		}

		if (((HDMI_READ32(REG_VID_CRC_CHK)) & (0x81 << 16)) == 0x0) {

			/* vRegWrite1B(VID_CRC_CHK_2, 0x0d);// start trigger */
			HDMI_WRITE32_MASK(REG_VID_CRC_CHK, (0x0d << 16), (0xff << 16));

			/* while (u1RegRead1B(VID_CRC_CHK_2)  != 0x8d) */
			while (!(HDMI_READ32(REG_VID_CRC_CHK) & 0x00800000)) {
				msleep(1);
			}

			/* vRegWrite1B(VID_CRC_CHK_2, 0x0c); */
			HDMI_WRITE32_MASK(REG_VID_CRC_CHK, (0x0c << 16), (0xff << 16));

			/* PRINT_REG(0x22c68); */
			if (((HDMI_READ32(REG_VID_CRC_CHK)) & (0x00800000))) {
				/* HDMI_LOG(HDMI_LOG_DEBUG, "CRC ready\r\n"); */
				tmp[0] = ((HDMI_READ32(REG_VID_CRC_CHK)) >> 24) & (0xff);
				tmp[1] = HDMI_READ32(REG_VID_CRC_CHK) & (0xff);
				tmp[2] = ((HDMI_READ32(REG_VID_CRC_CHK)) >> 8) & (0xff);

				/*  vUtDelay10ms(2); */
				/*  compare and update result if necessary */
				if ((tmp[0] == result[0][0]) && (tmp[1] == result[0][1]) && (tmp[2] == result[0][2])) {
					continue;
				}

				if ((tmp[0] == result[1][0]) && (tmp[1] == result[1][1]) && (tmp[2] == result[1][2])) {
					continue;
				}

				/* ASSERT((idx<3)); */
				if (idx >= 3) {
					HDMI_LOG(HDMI_LOG_ERROR,
						"############    FAIL CRC ########################\r\n");
					return 0;
				}

				result[idx][0] = tmp[0];
				result[idx][1] = tmp[1];
				result[idx][2] = tmp[2];

				idx++;
				continue;
			} else {
				/* PRINT_REG(0x22c68); */
				HDMI_LOG(HDMI_LOG_INFO, "CRC is not ready\n");
				return 0;
			}
		} else {
			HDMI_LOG(HDMI_LOG_INFO, "reset CRC fail");
			return 0;
		}
	}

	/* if (u1RegRead1B(VID_CH_MAP_1) & 0x04) */
	if (HDMI_READ32(REG_VID_CH_MAP) & (1 << 10)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "interlace signal\r\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "progressive signal~\r\n");
	}

	if (idx == 1) {
		/* HDMI_LOG(HDMI_LOG_DEBUG, "jiffies : %d----", jiffies); */
		HDMI_LOG(HDMI_LOG_DEBUG, "assume progressive signal\r\n");
		HDMI_LOG(HDMI_LOG_DEBUG, "CRC result:\r\n");
		HDMI_LOG(HDMI_LOG_DEBUG, "%x %x %x\r\n", result[0][0], result[0][1], result[0][2]);
	} else if (idx == 2) {
		HDMI_LOG(HDMI_LOG_DEBUG, "assume interlaced signal\n");
		HDMI_LOG(HDMI_LOG_DEBUG, "CRC result:\n");
		HDMI_LOG(HDMI_LOG_DEBUG, "%x %x %x\n", result[0][0], result[0][1], result[0][2]);
		HDMI_LOG(HDMI_LOG_DEBUG, "%x %x %x\n", result[1][0], result[1][1], result[1][2]);
	}

	_CrcResult[0][0] = result[0][0];
	_CrcResult[0][1] = result[0][1];
	_CrcResult[0][2] = result[0][2];
	_CrcResult[1][0] = result[1][0];
	_CrcResult[1][1] = result[1][1];
	_CrcResult[1][2] = result[1][2];

	return 1;

}



BOOL HalIsINTR8_AUDFMTCHG(void)
{
	if (HDMI_READ32(REG_INTR_MASK) & INTR_AUD_FMT_CHG_MASK) {
		/* write "1" to clear */
		HDMI_WRITE32(REG_INTR_MASK, INTR_AUD_FMT_CHG_MASK);
		return TRUE;
	}

	return FALSE;

}
BOOL HalIsINTR8_AUDCHSTATUSCHG(void)
{
	if (HDMI_READ32(REG_INTR_MASK) & INTR_DE_EMPH_CHG_MASK) {
		/* write "1" to clear */
		HDMI_WRITE32(REG_INTR_MASK, INTR_DE_EMPH_CHG_MASK);
		return TRUE;
	}

	return FALSE;

}
/*************************************************************************************
void vHDMIRxIntMask(BOOL fgOn)
Describe: This function to set which interrupt will be serviced in  hdmi rx interrupt.

Parameters:

Return:
0 for non-HBR audio , 1 for HBR audio

*************************************************************************************/
void HDMIRxIntMask(BOOL fgOn)
{
	if (fgOn) {
		HDMI_WRITE32_MASK(REG_INTR_MASK, 0x1 << 6, INTR_AUD_FMT_CHG_MASK);
		HDMI_WRITE32_MASK(REG_INTR_MASK, 0x1 << 7, INTR_DE_EMPH_CHG_MASK);
		HDMI_WRITE32_MASK(REG_INTR_MASK1, 0x1 << 0, INTR4_UNDERRUN);
		HDMI_WRITE32_MASK(REG_INTR_MASK1, 0x1 << 1, INTR4_OVERRUN);
		HDMI_WRITE32_MASK(REG_INTR_MASK1, 0x1 << 28, INTR7_AUD_CH_STAT);
		HDMI_WRITE32_MASK(REG_INTR_MASK1, 0x1 << 14, INTR5_AUDIO_MUTE);
	}
}

/*************************************************************************************
BOOL   fgHalHDMIRxHDAudio(void)
Describe: This function indicates the HDMI Audio Packet is HBR(High BitRate) or not.

Parameters: Non

Return:
0 for non-HBR audio , 1 for HBR audio

*************************************************************************************/
BOOL   HalHDMIRxHDAudio(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_HBR) {
		HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 25, INTR7_HBR);
		return TRUE;
	}

	return FALSE;

}

/*************************************************************************************
BOOL   fgHalHDMIRxDSDAudio(void)
Describe: This function indicates the One bit DSD Audio Packet is HBR(High BitRate) or not.

Parameters: Non

Return:
0 for non-DSD audio , 1 for DSD audio
*************************************************************************************/
BOOL   HalHDMIRxDSDAudio(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR7_SACD) {
		HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 24, INTR7_SACD);
		return TRUE;
	}

	return FALSE;

}

/*************************************************************************************
BOOL   fgHalHDMIRxAudioPkt(void)
Describe: This function indicates the Audio Packet or not.

Parameters: Non

Return:
0 for non-Audio Packet , 1 for audio packet

*************************************************************************************/
BOOL   HalHDMIRxAudioPkt(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR2_NEW_AUD_PKT) {
		HDMI_WRITE32_MASK(REG_INTR_STATE0, 0x1 << 17, INTR2_NEW_AUD_PKT);
		return TRUE;
	}

	return FALSE;
}

/*************************************************************************************
BYTE   u1HDMIRxAudioCHSTAT0(void)
Describe: This function indicates audio Channel Status 0 [7:0]

Parameters: Non

Return:
channel status 0 [7:0]
0 : consumer application , 1 : professional application
Audio/Digital bit : 0 : audio sample word represents linear PCM samples ,1 : audio sample word used for other purposes
Copyright information
Pre-empahsis information
00 Mode 0 for digital audio equipment for consumer use
*************************************************************************************/
UINT8   HalHDMIRxAudioCHSTAT0(void)
{
	UINT8 u1AudChstat0;

	u1AudChstat0 = (HDMI_READ32(REG_AUDRX_CTRL) & 0x00FF0000) >> 16;
	return  u1AudChstat0;
}

/*************************************************************************************
BYTE   u1HDMIRxAudioCHSTAT1(void)
Describe: This function indicates audio Channel Status 1 [15:8]

Parameters: Non

Return:
channel status 1 [15:8]
Category Code (corresponds to channel status bits 15:8)
*************************************************************************************/
UINT8   HalHDMIRxAudioCHSTAT1(void)
{
	UINT8 u1AudChstat1;

	u1AudChstat1 = (HDMI_READ32(REG_AUDRX_CTRL) & 0xFF000000) >> 24;
	return  u1AudChstat1;

}

/*************************************************************************************
BYTE   u1HDMIRxAudioCHSTAT2(void)
Describe: This function indicates audio Channel Status 2 [23:16]

Parameters: Non

Return:
channel status 2 [23:16]
Source Number (corresponds to channel status bits 19:16)
Channel Number (corresponds to channel status bits 23:20)
*************************************************************************************/
UINT8   HalHDMIRxAudioCHSTAT2(void)
{
	UINT8 u1AudChstat2;

	u1AudChstat2 = (HDMI_READ32(REG_CHST0) & 0x000000FF) >> 0;
	return  u1AudChstat2;

}

/*************************************************************************************
BYTE   u1HDMIRxAudioCHSTAT3(void)
Describe: This function indicates audio Channel Status 3 [31:24]

Parameters: Non

Return:
channel status 3 [31:24]
Sampling Frequency (channel status bits 27:24)
Clock Accuracy (corresponds to channel status bits 31:28)
*************************************************************************************/
UINT8   HalHDMIRxAudioCHSTAT3(void)
{
	UINT8 u1AudChstat3;

	u1AudChstat3 = (HDMI_READ32(REG_CHST1) & 0x00000FF0) >> 4;
	return  u1AudChstat3;

}

/*************************************************************************************
BYTE   u1HDMIRxAudioCHSTAT4(void)
Describe: This function indicates audio Channel Status 4 [39:32]

Parameters: Non

Return:
channel status 4 [39:32]
Audio Length Max (channel status bit 32) 0/1 :  = maximum sample word length is 20/24 bits
Audio Length (channel status bits 35:33)
original sampling frequency (channel status bits 39:36 )
*************************************************************************************/
UINT8   HalHDMIRxAudioCHSTAT4(void)
{
	UINT8 u1AudChstat4;

	u1AudChstat4 = (HDMI_READ32(REG_CHST1) & 0x0000FF00) >> 8;
	return  u1AudChstat4;
}


/*************************************************************************************
bool HDMIRxMultiPCM(void)
Describe: This function indicates the HDMI Audio Packet is 2channel or multi-channel
		layout.

Parameters: Non

Return: HDMI Audio Packet layout indicator:
		0 : Layout 0 (2-channel) (default)
		1 : Layout 1 (Up to 8-channel).
*************************************************************************************/
BOOL HalHDMIRxMultiPCM(void)
{
	if (HDMI_READ32(REG_AUDP_STAT) & HDMI_LAYOUT) {
		return TRUE;
	}

	return FALSE;
}

/*************************************************************************************
UINT8    u1HalHDMIRxAudioChannelNum(void)
Describe: This function to get audio channel number

Parameters: Non

Return: Non
*************************************************************************************/
UINT8 HalHDMIRxAudioChannelNum(void)
{
	UINT8 u1RxChNum;

	u1RxChNum = (HDMI_READ32(REG_CHST0) & CH_NUM1) >> 4;
	return  u1RxChNum;
}

/*************************************************************************************
UINT8    u1HalHDMIRxAudFsGet(void)
Describe: This function to get audio sampling frequency (channel status bits 27:24)

Parameters: Non

Return:   Sampling Frequency (channel status bits 27:24)

*************************************************************************************/
UINT8 HalHDMIRxAudFsGet(void)
{
	UINT8 u1AudFs;

	u1AudFs = (HDMI_READ32(REG_LK_THRS_SVAL) & RHDMI_AUD_SAMPLE_F) >> 24;
	return  u1AudFs;
}


/*************************************************************************************
UINT8  u1HalHDMIRxAudValidCHGet(void)
Describe: This function is for HDMI RX task get audio valid channels

Parameters: Non

Return: Non

*************************************************************************************/
UINT8 HalHDMIRxAudValidCHGet(void)
{
	UINT8 u1AudValidCh;

	u1AudValidCh = (HDMI_READ32(REG_AUDIO_INFO) & 0x000F00) >> 8;
	return  u1AudValidCh;
}

/*************************************************************************************
void  vHalHDMIRxSetAudValidCH(void)
Describe: This function is for HDMI RX task set audio valid channels

Parameters: Non

Return: Non

*************************************************************************************/
void HalHDMIRxSetAudValidCH(UINT8 u1ValidCh)
{
	HDMI_WRITE32_MASK(REG_I2S_CTRL, u1ValidCh << 28, 0xF << 28);
}
/*************************************************************************************
void  vHalHDMIRxSetAudMuteCH(void)
Describe: This function is for HDMI RX task set audio mute channels

Parameters: Non

Return: Non

*************************************************************************************/
void HalHDMIRxSetAudMuteCH(UINT8 u1MuteCh)
{
	HDMI_WRITE32_MASK(REG_CHST1, u1MuteCh << 16, 0xF << 16);
}


/************************************************************************************
UINT8  vHalHDMIRxAudErrorGet(void)
Describe: This function is to get audio error information

Parameters: Non

Return: Non

*************************************************************************************/
UINT32 HalHDMIRxAudErrorGet(void)
{
	UINT32 u4AudErrtst = 0;

	u4AudErrtst = HDMI_READ32(REG_INTR_STATE1);

	HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 0, INTR4_UNDERRUN);
	HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 1, INTR4_OVERRUN);
	HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 8, INTR5_AUD_SAMPLE_F);

	return (u4AudErrtst & HDMIRX_INT_STATUS_CHK);

}

void HalHDMIRxAudResetMCLK(void)
{
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x1 << 27, REG_MCLK_RST);
	HAL_Delay_us(2);
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x0, REG_MCLK_RST);
}

void HalHdmiAcrRst(void)
{
	UINT32 u4read;

	/* d18[9]=0 */
	u4read = HDMI_AUD_READ32(0x22d18);
	HDMI_AUD_WRITE32(0x22d18, u4read & (~(1 << 0x9)));

	/* d54 */
	HDMI_AUD_WRITE32(0x22d54, 0x14d817c3);

	/* d18[8]=1 */
	u4read = HDMI_AUD_READ32(0x22d18);
	HDMI_AUD_WRITE32(0x22d18, u4read | (1 << 0x8));
	udelay(40);

	/* d18[9]=1 */
	u4read = HDMI_AUD_READ32(0x22d18);
	HDMI_AUD_WRITE32(0x22d18, u4read | (1 << 0x9));
	udelay(80);

	/* 5 d18[9]=0 */
	u4read = HDMI_AUD_READ32(0x22d18);
	HDMI_AUD_WRITE32(0x22d18, u4read & (~(1 << 0x9)));
	udelay(80);

	/* 6 c04[9,10]=1 */
	u4read = HDMI_AUD_READ32(0x22C04);
	HDMI_AUD_WRITE32(0x22C04, u4read | (3 << 0x9));
	udelay(50);

	/* 7 c04[10]=0 */
	u4read = HDMI_AUD_READ32(0x22C04);
	HDMI_AUD_WRITE32(0x22C04, u4read & (~(1 << 0xA)));
	/* Printf("6HDMI Rx Audio reset test count is %d\r\n", u4count++); */
	udelay(1000);
	udelay(1000);
	udelay(1000);
	udelay(1000);

	/* 8 d18[8]=0 */
	u4read = HDMI_AUD_READ32(0x22d18);
	HDMI_AUD_WRITE32(0x22d18, u4read & (~(1 << 0x8)));
	udelay(1000);

	/* 9 c04[9]=0 */
	u4read = HDMI_AUD_READ32(0x22C04);
	HDMI_AUD_WRITE32(0x22C04, u4read & (~(1 << 0x9)));
}

#ifdef CONFIG_ATC_PLATFORM_ac823x
void resetAnaHADDS2(void)
{
	BIT_CLR(0x005e8, 3);
	BIT_CLR(0x005ec, 21);
	BIT_CLR(0x005ec, 25);
	BIT_CLR(0x005e8, 5);
	BIT_CLR(0x005e8, 2);
	BIT_CLR(0x005e8, 6);
	BIT_CLR(0x005e8, 19);
}
#endif

void HalHdmiRxSetApll(void)
{
	UINT32 val = 0;
#if 0
	val = HDMI_AUD_READ32(0x22C04);
	HDMI_AUD_WRITE32(0x22C04, val | (1 << 0xA));
	udelay(20);
	HDMI_AUD_WRITE32(0x22C04, val & (~(1 << 0xA)));
	/* printk("APLL Setting start"); */
#endif
	/* important must not DDS model in sw model, 5e0[4]=0 */
	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	val = BASE_READ32(0x5e0);
	val = (val & (~(1 << 0x4)));
	BASE_WRITE32(0x5e0, val);

	BASE_WRITE32(0x5E4, 0xAF8000);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	//5e0[4]=0 ->5e8[19]
	val = BASE_READ32(0x005e8);
	val = (val &(~ (1<<0x13)));
	BASE_WRITE32(0x005e8, val);

	resetAnaHADDS2();
	#endif

	/* (1 sigma-delta reference input 0x22c88[13]=1 */
	val = BASE_READ32(0x22C88);
	BASE_WRITE32(0x22C88, val | (1 << 0xD));

	/* (2 send sw pcw value */
	BASE_WRITE32(0x22D54, 0x14D8173c);

	/* (3 select fbdiv sw value */
	val = BASE_READ32(0x22d18);
	val = (val | 0x400);
	BASE_WRITE32(0x22d18, val);

	/* (4 fbdiv sw value [23~16] */
	val = BASE_READ32(0x22d18);
	val = (val & (~(0xFF << 0x10)));
	val = (val | 0x150000);
	BASE_WRITE32(0x22d18, val);

	/* (5 sw select */
	val = BASE_READ32(0x22d18);
	BASE_WRITE32(0x22d18, val | (1 << 0x8));

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	/* (6 RG_HADDS2_MONEN 0X5E4[20]=1 */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x14));
	BASE_WRITE32(0x005e4, val);

	/* (7 RG_HADDS2_TSTCKENB 0x5E8[9]=1 */
	val = BASE_READ32(0x005e8);
	val = (val | (1 << 0x9));
	BASE_WRITE32(0x005e8, val);

	/* (8 RG_HADDS2_TSTEN=1 0x5e4[8] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x8));
	BASE_WRITE32(0x005e4, val);

	/* (9 RG_HADDS2_MONCKEN=1 0x5e0[16] */
	val = BASE_READ32(0x005e0);
	val = (val | (1 << 0x10));
	BASE_WRITE32(0x005e0, val);

	/* (10 RG_HADDS2_FMEN=1 0x5e4[12] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0xc));
	BASE_WRITE32(0x005e4, val);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	//6.0X5E4[20]=1 -->5e8[3]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x3));
	BASE_WRITE32(0x005e8, val);

	//(7 RG_HADDS2_TSTCKENB 0x5E8[9]=1 -->5f0[25]
	val = BASE_READ32(0x005f0);
	val = (val | (1<<0x19));
	BASE_WRITE32(0x005f0, val);

	//(8 RG_HADDS2_TSTEN=1 0x5e4[8] -->5ec[21]
	val = BASE_READ32(0x005ec);
	val = (val | (1<<0x15));
	BASE_WRITE32(0x005ec, val);

	//(9 RG_HADDS2_MONCKEN=1 0x5e0[16] -->5e8[31]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x1f));
	BASE_WRITE32(0x005e8, val);
	
	//(10 RG_HADDS2_FMEN=1 0x5e4[12] -->5ec[25]
	val = BASE_READ32(0x005ec);
	val = (val | (1<<0x19));
	BASE_WRITE32(0x005ec, val);
	#endif

	/* (11 RG_HADDS2_PWD=1 0x24180[14] */
	val = BASE_READ32(0x24180);
	val = (val | (1 << 0xe));
	BASE_WRITE32(0x24180, val);

	/* (12 RG_HADDS2_PWD=0 0x24180[14] */
	val = BASE_READ32(0x24180);
	val = (val & (~(1 << 0xe)));
	BASE_WRITE32(0x24180, val);
	udelay(40);

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	/* (13 RG_PLL_MONSEL   //0x5C4[15 ~ 8]=0X0F */
	val = BASE_READ32(0x005c4);
	val = (UINT32)(val & (~(0xFF << 0x8)));
	val = (val | 0xF00);
	BASE_WRITE32(0x005c4, val);

	/* (14 ADDS2_DDS_RSTB=1 0x5e4[22] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x16));
	BASE_WRITE32(0x005e4, val);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	
	//(13 RG_PLL_MONSEL   //0x5C4[15 ~ 8]=0X0F -->5c8[31~24]
	val = BASE_READ32(0x005c8);
	val = (UINT32)(val & (~(0xFF<<0x18)));
	val = (val | 0xF000000);
	BASE_WRITE32(0x005c8, val);
	
	//(14 ADDS2_DDS_RSTB=1 0x5e4[22] -->5e8[5]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x5));
	BASE_WRITE32(0x005e8, val);
	#endif

	/* (15 ADDS2_DDS_PWDB=1 0x180[15] */
	val = BASE_READ32(0x24180);
	val = (val | (1 << 0xf));
	BASE_WRITE32(0x24180, val);
	udelay(40);

	/* (16 ADDS2_PCW_NCPP_CHG=1 */
	val = BASE_READ32(0x22d18);
	val = (val | (1 << 0x9));
	BASE_WRITE32(0x22d18, val);

	/* ( 17 ADDS2_PCW_NCPP_CHG=0 */
	val = BASE_READ32(0x22d18);
	val = (val & (~(1 << 0x9)));
	BASE_WRITE32(0x22d18, val);

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	/* (18 ADDS2_CLK_PH_INV=1 0x5e4[19] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x13));
	BASE_WRITE32(0x005e4, val);
	udelay(40);

	/* (19 RG_HADDS2_FIFO_STAT_MAN=1 0x5e4[23] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x17));
	BASE_WRITE32(0x005e4, val);

	/* (20 RG_HADDS2_NCPO_EN=1 0x5e4[21] */
	val = BASE_READ32(0x005e4);
	val = (val | (1 << 0x15));
	BASE_WRITE32(0x005e4, val);
	udelay(40);

	/* (21 RG_HADDS2_DDSEN=1 0x5e0[4] */
	val = BASE_READ32(0x005e0);
	val = (val | (1 << 0x4));
	BASE_WRITE32(0x005e0, val);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	//(18 ADDS2_CLK_PH_INV=1 0x5e4[19] -->5e8[2]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x2));
	BASE_WRITE32(0x005e8, val);
	udelay(40);
	
	//(19 RG_HADDS2_FIFO_STAT_MAN=1 0x5e4[23] -->5e8[6]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x6));
	BASE_WRITE32(0x005e8, val);
	
	//(20 RG_HADDS2_NCPO_EN=1 0x5e4[21] -->5e8[4]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x4));
	BASE_WRITE32(0x005e8, val);
	udelay(40);
	
	//(21 RG_HADDS2_DDSEN=1 0x5e0[4] -->5e8[19]
	val = BASE_READ32(0x005e8);
	val = (val | (1<<0x13));
	BASE_WRITE32(0x005e8, val);
	#endif

	/* (22 acr_rst=1 fifo_rst=1 //c04[10 9] */
	val = BASE_READ32(0x22c04);
	val = (val | (3 << 0x09));
	BASE_WRITE32(0x22c04, val);

	/* (23 acr_rst=0 */
	val = BASE_READ32(0x22c04);
	val = (val & (~(1 << 0xa)));
	BASE_WRITE32(0x22c04, val);
	mdelay(5);

	/* (24 RG_PCM_NCPO_CHG_SW   D18[8]=0 */
	val = BASE_READ32(0x22d18);
	val = (val & (~(1 << 0x8)));
	BASE_WRITE32(0x22d18, val);

	/* (25 fifo_rsr=0 */
	val = BASE_READ32(0x22c04);
	val = (val & (~(1 << 0x9)));
	BASE_WRITE32(0x22c04, val);

	/* printk("APLL Setting over!"); */


}

void HalHdmiRxAudInitSetting(void)
{
	HalHdmiRxSetApll();

	HDMI_AUD_WRITE32(0x022C38, 0x00002280);
	HDMI_AUD_WRITE32(0x022D28, 0x00009FE4);
	HDMI_AUD_WRITE32(0x022D24, 0xF0400000);
	/* above, important [24] must be 0, if not, multi-line in  dump raw data is 0. */
	HDMI_AUD_WRITE32(0x022D3C, 0xCFF60001);
	HDMI_AUD_WRITE32(0x022C08, 0x00009407);
	HDMI_AUD_WRITE32(0x022D34, 0x00020600);
	HDMI_AUD_WRITE32(0x022C14, 0x865E01EA);
	/*HDMI_AUD_WRITE32(0x000070, 0x00000839);*/
	HDMI_AUD_WRITE32(0x000070, (HDMI_AUD_READ32(0x000070) | (0x1<<11)));
	/*GPIO_MultiFun_Set(PIN_129_SPDIF, HDMI_SPDIF_SEL);*/

	RETAILMSG(1, (TEXT("Verify Golden Setting for audio register...... \r\n")));
}

void HalHdmiAudRegReset(void)
{
#if 0
	BASE_WRITE32(0x5E0, 0xC40101);

	BASE_WRITE32(0x22C88, 0x1CE1688);

	BASE_WRITE32(0x22D54, 0x0);

	BASE_WRITE32(0x22D18, 0x5C);

	BASE_WRITE32(0x5E4, 0xAF8000);

	BASE_WRITE32(0x5E8, 0x200);

	BASE_WRITE32(0x24180, 0x40DA2);

	BASE_WRITE32(0x5C4, 0x200C0);
#endif
}

BOOL HalHdmiRxAudResetAudio(void)
{
	UINT32 u4read;

	u4read = HDMI_AUD_READ32(0x22c78);

	if (u4read & 0x3) {
		HDMI_AUD_WRITE32(0x22c78, u4read);
		msleep(2);

		u4read = HDMI_AUD_READ32(0x22c78);

		if (u4read & 0x3) {
			if (HDMI_AUD_READ32(0x22d90) & 0xFFFFFF) { /* D90 is not 0 */
				HDMI_AUD_WRITE32(0x22c70, HDMI_AUD_READ32(0x22c70));
				msleep(2);

				if ((HDMI_AUD_READ32(0x22c70) & 0x60000) != 0x60000) {
					/* HDMI_LOG(HDMI_LOG_DEBUG, "hdmi/mhl not play audio*****1111\r\n"); */
					return FALSE;
				}

				HalHdmiAcrRst();
				msleep(20);

				HDMI_AUD_WRITE32(0x22c78, HDMI_AUD_READ32(0x22c78));
				msleep(2);
				u4read = HDMI_AUD_READ32(0x22c78);

				if (u4read & 0x3) {
					/* HDMI_LOG(HDMI_LOG_DEBUG,
					"***Reg[0x22C78]=0x%x. reset fail... \r\n", u4read); */
					return FALSE;
				}
				/* HDMI_LOG(HDMI_LOG_DEBUG,
					"***Reg[0x22C78]=0x%x. reset OK... \r\n", u4read); */
			} else {
				HDMI_AUD_WRITE32(0x22c70, HDMI_AUD_READ32(0x22c70));
				msleep(2);

				if ((HDMI_AUD_READ32(0x22c70) & 0x60000) == 0x60000) {
					/*receive CTS, reset apll */
					RETAILMSG(1, (TEXT("[HDMI AUD RESET]reset audio apll... \r\n")));
					HalHdmiRxSetApll();
					HDMI_AUD_WRITE32(0x22c70, 0x40000);
					return FALSE;
				}
				/* HDMI_LOG(HDMI_LOG_DEBUG, "hdmi/mhl not play audio\r\n"); */
				return FALSE;
			}
		} else {
			/* HDMI_LOG(HDMI_LOG_DEBUG, "***reset OK, Reg[0x22C78]=0x%x***\r\n", u4read); */
		}
	}

	return TRUE;
}

void HalHdmiRxAudResetAfifo(void)
{
	/*  Reset Audio Fifo */
	HDMI_WRITE32_MASK(REG_SRST, 0x1 << 9, FIFO_RST);
	msleep(1);
	/*  Set Audio Fifo normal operation */
	HDMI_WRITE32_MASK(REG_SRST, 0x0, FIFO_RST);
	msleep(1);
	/*  Reset  Interrupt status : under-run / over-run / terc4  / hdcp */
	HDMI_WRITE32(REG_INTR_STATE1, HDMIRX_INT_STATUS_CHK);
}

BOOL HalGetMhlAudPlayStatus(void)
{
	if ((HDMI_AUD_READ32(0x22c70) & 0x60000)) { /*  b17: audio packet; b18: ncts packet */
		HDMI_AUD_WRITE32(0x22c70, (HDMI_AUD_READ32(0x22c70) & (~0x60000)));
		return TRUE;  /*  mhl in play status */
	}

	return FALSE; /*  mhl in pause status */
}

BOOL HalChkAudPktReady(void)
{
	HDMI_AUD_WRITE32(0x22c70, HDMI_AUD_READ32(0x22c70));
	msleep(2);

	if ((HDMI_AUD_READ32(0x22c70) & 0x40000)) {
		RETAILMSG(1, (TEXT("[HDMI AUD]Audio packet ready \r\n")));
		return TRUE;
	}

	/* RETAILMSG(1,(TEXT("[HDMI AUD]Audio packet empty \r\n"))); */
	return FALSE;
}

/*************************************************************************************
BOOL  bHalGetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
Describe: This function is for HDMI RX task get adio inforframe

Parameters: Non

Return: True if get  infoframe ok

*************************************************************************************/


BOOL HalCheckIsAAC(void)
{
	if (HDMI_READ32(REG_INTR_STATE1) & INTR5_AUDIO_MUTE) {
		HDMI_WRITE32_MASK(REG_INTR_STATE1, 0x1 << 14, INTR5_AUDIO_MUTE);
		return TRUE;
	}

	return FALSE;

}
/*************************************************************************************
void vHalSetHDMIRxHBR(void)
Describe: This function is to set hdmi rx HBR mode

Parameters: Non

Return:

*************************************************************************************/
void HalSetHDMIRxHBR(BOOL fgHBR)
{
	if (fgHBR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HdmiRxHBR\n");

		HDMI_HalSetAudioFS(SW_192K);

		HDMI_WRITE32_MASK(REG_AUDRX_CTRL, 0x1 << 10, I2S_MODE);
		HDMI_WRITE32_MASK(REG_AUDIO1, 0x0, 0x07 << 17);
		HDMI_WRITE32_MASK(REG_N_HDMI_CTRL,
				  (TT1_8CH | TT0_HBR_8CH | TT0_HBR_EN),
				  (TT1_SPDIF_REQ_8CH | TT1_8CH | TT0_HBR_8CH | TT0_HBR_EN));
		HDMI_WRITE32_MASK(REG_I2S_CTRL,
				  SIZE,
				  (SIZE | DATA_DIR | FIRST_BIT));
		HDMI_WRITE32_MASK(REG_CKDT_RST, 0x0, 0x1 << 16); /* unknown bit */
		HDMI_WRITE32_MASK(REG_AUDIO, 0xB2 << 24, 0xff << 24); /* unknown bit */
	} else {
		HDMI_WRITE32_MASK(REG_N_HDMI_CTRL, 0x0, (TT1_8CH | TT0_HBR_8CH));
		HDMI_WRITE32_MASK(REG_I2S_CTRL, 0x0, SIZE);
	}
}


void HalHDMIRxEnableAudPktReceive(void)
{
	HDMI_WRITE32_MASK(REG_N_HDMI_CTRL, 0x1 << 9, TT0_HBR_EN);
}


/*************************************************************************************
void vHalHdmiRxAudBypass(BOOL fgBypass,BOOL fgBypassSPDIF2Tx)
Describe: This function is to bypass hdmi rx I2S(PCM)/SPDIF mode

Parameters: fgBypass  : TRUE for bypass , FALSE for no-bypass
		fgBypassSPDIF2Tx : TRUE for bypass SPDIF Path , FALSE for bypass I2S Path
Return:

*************************************************************************************/
void HalHdmiRxAudBypass(BOOL fgBypass, BOOL fgBypassSPDIF2Tx)
{
	if (fgBypass) {
		HDMI_LOG(HDMI_LOG_INFO, "fg Bypass is true, but not use\r\n");
		/*HDMI_AV_INFO_T _stAvdAVInfo;


		Set rx's format the same as tx's. 
		switch (_stAvdAVInfo.e_I2sFmt) {
		case HDMI_RJT_24BIT:
		case HDMI_RJT_16BIT:
			HDMI_HalI2sLRInv(TRUE);
			HDMI_HalSetAudI2sFormat(FORMAT_RJ, LRCK_CYC_32);
			break;

		case HDMI_LJT_24BIT:
		case HDMI_LJT_16BIT:
			HDMI_HalI2sLRInv(TRUE);
			HDMI_HalSetAudI2sFormat(FORMAT_LJ, LRCK_CYC_32);
			break;

		case HDMI_I2S_24BIT:
		case HDMI_I2S_16BIT:
			HDMI_HalI2sLRInv(FALSE);
			HDMI_HalSetAudI2sFormat(FORMAT_I2S, LRCK_CYC_32);
			break;

		default:
			break;
		
		}*/
	} else {

		/* Set rx's format as default i2s. */
		HDMI_HalI2sLRInv(FALSE);
		HDMI_HalSetAudI2sFormat(FORMAT_I2S, LRCK_CYC_32);
	}
}



/*************************************************************************************
void vHalSetHDMIRxI2S(void)
Describe: This function is to set hdmi rx I2S(PCM) mode

Parameters: Non

Return:

*************************************************************************************/
void HalSetHDMIRxI2S(void)
{

	/* LOG(9,"[HDMI RX AUD] HdmiRxHBR\n"); */
	/*      vRxWriteReg(0xd0,0x1F);
			vRxWriteReg(0xdc,0x00);
			vRxWriteReg(0xcc,0x10);
			Clock  mode
			vRxWriteRegMsk(0x1F26c,0x02,0x0f);
	*/
}
BOOL HalHDMIRxAPLLStatus(void)
{
	if (HDMI_READ32(REG_APLL1) & ACR_DPLL_LOCK) {
		return TRUE;
	}

	RETAILMSG(1, (TEXT("[HDMI AUD]APLL is unlock\r\n")));
	return FALSE;
}
/*************************************************************************************
void vHalSetHDMIRxDSD(void)
Describe: This function is to set hdmi rx One Bit Audio DSD mode

 Parameters: Non

 Return:

*************************************************************************************/
void HalSetHDMIRxDSD(BOOL fgDSD)
{
	/* not support DSD */
}


BYTE HalGetUnRecPacketHeader(void)
{
	UINT32 u4Value = 0;

	u4Value = HDMI_READ32(REG_UNRECRX0) & 0x000000FF;

	return (UINT8)u4Value;
}



void SetSelectUnRecpacket(BOOL fgEnable, BYTE bHeader)
{
	if (fgEnable) {
		HDMI_WRITE32_MASK(REG_N_HDMI_CTRL, 0x1 << 31, TT2_EXT_PKT_EN);
		HDMI_WRITE32_MASK(REG_N_HDMI_CTRL3, bHeader, EXT_PKT_DEC);
	} else {
		HDMI_WRITE32_MASK(REG_N_HDMI_CTRL, 0x0, TT2_EXT_PKT_EN);
	}
}

/* for HDCP */
void HalRxHdcpReset(void)
{
	HDMI_WRITE32_MASK(REG_SRST, 0x1 << 11, HDCP_RST);
	HDMI_WRITE32_MASK(REG_SRST, 0x0, HDCP_RST);
}

void HDMI_HalResetHdcp(void)
{
	HDMI_WRITE32_MASK(REG_SRST, 0x1 << 11, HDCP_RST);
	HDMI_WRITE32_MASK(REG_SRST, 0x0, HDCP_RST);
}


void HalDisableHDCPDDCPort(void)
{
	HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x0, DDC_EN);
}

void HalEnableHDCPDDCPort(void)
{
	HDMI_WRITE32_MASK(REG_SYS_CTRL, 0x1 << 12, DDC_EN);
}


void HalSetKsvReadyBit(void)
{
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x0, FIFO_RDY_WP);/* pull low */
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x1 << 21, REG_FIFO_RDY);
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x1 << 19, FIFO_RDY_WP); /* pull high */
}

void  HalClearKsvReadyBit(void)
{
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x0, FIFO_RDY_WP);/* pull low */
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x0, REG_FIFO_RDY);
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, 0x1 << 19, FIFO_RDY_WP); /* pull high */
}

#define HDCP_ADDR_MT3363    (0x29310)
#define HDCP_DATA_MT3363    (0x29314)


void HDMI_HalLoadHdcp2Sram(UINT8 *prHdcpKey)
{
	UINT32 i;
	UINT32 u4Data;

	/* IO_WRITE32((0xA0022CFC), ((IO_READ32(0xA0022CFC))|0xC3)); ///KS_MASK */
	/* IO_WRITE32((0xA0022CF8), ((IO_READ32(0xA0022CF8))|(0xFF000000))); //EPST */

	/*  set hw password: 0x00 0x00 */
	HDMI_WRITE32_MASK(REG_KS_MASK, 0x00, 0xFF);
	HDMI_WRITE32_MASK(REG_EPST, 0x0 << 24, 0xFF << 24);

	/*  load HDCP key from EEP to SRAM */
	HDMI_WRITE32(REG_HDCP_RW_STATUS, 0x0);

	for (i = 0; i < 292; i = i + 4) {
		u4Data = *(UINT32 *)(&prHdcpKey[i]);
		HDMI_WRITE32(REG_HDCP_DATA, u4Data);
	}

	u4Data = 0x100 | 0x0A0;
	HDMI_WRITE32(REG_HDCP_CTRL, u4Data);   /* So 3363 can not find those register. */

	/* HDCP Reset */
	HDMI_WRITE32_MASK(REG_SRST, 0x1 << 11, HDCP_RST); /* SRST HDCP RESET */
	HDMI_WRITE32_MASK(REG_SRST, 0x0, HDCP_RST);
}


/* load Edid table to Sram */
/*
void vHalLoadEdid2Sram(UINT8 *pEdid, UINT32 u4Size)
{
    UINT32 val, val0, val1, val2, val3 = 0;
    int i = 0;

    if(u4Size != 256)
    {
		return;
    }

    IO_WRITE32(IO_BASE_VA, 0x22a1c, 0x04800000);
    for(i=0; i<64; i++)
    {
		val0 = pEdid[4*i];
		val1 = pEdid[4*i+1];
		val2 = pEdid[4*i+2];
		val3 = pEdid[4*i+3];
		val = (val3<<24)|(val2<<16)|(val1<<8)|(val0);

		IO_WRITE32(IO_BASE_VA, 0x22a50, val);
    }

    IO_WRITE32(IO_BASE_VA, 0x22a04, (pEdid[255]<<16));
    IO_WRITE32(IO_BASE_VA, 0x22a1c, 0x04000000);

    return ;
}
*/

/* load Edid table to Sram */
void HDMI_HalLoadEdid2Sram(UINT8 *pEdid, UINT32 u4Size)
{
	UINT32 val, val0, val1, val2, val3 = 0;
	int i = 0;

	if (u4Size != 256) {
		return;
	}

	/* enable download mode */
	DDCCI_WRITE32(REG_DDC_07,  DLADR_AUTO_INC | EDID_DL_MODE);

	for (i = 0; i < 64; i++) {
		val0 = pEdid[4 * i];
		val1 = pEdid[4 * i + 1];
		val2 = pEdid[4 * i + 2];
		val3 = pEdid[4 * i + 3];
		val = (val3 << 24) | (val2 << 16) | (val1 << 8) | (val0);

		/* IO_WRITE32(IO_BASE_VA, 0x22a50, val); */
		DDCCI_WRITE32(REG_DDC_14, val & EDID_DL_PORT);
	}

	DDCCI_WRITE32(REG_DDC_01, pEdid[255] << 16); /*  write checksum */

	/*  disable download mode */
	DDCCI_WRITE32(REG_DDC_07, DLADR_AUTO_INC);
}


void  HalSetHdmiCapable(BOOL fgHdmiCapable)
{
	HDMI_WRITE32_MASK(REG_SHD_BSTATUS, fgHdmiCapable << 23, HDMI_CAPABLE);
}

void  HalWriteKsvList(BYTE *prKsvList, BYTE bCount)
{
	/* get setting from designer */
	/*
	  UINT8 bLength;
	  UINT8 i;
	  bLength = bCount*5;
	  for(i=0; i<bLength; i++)
	  {
	   HAL_Delay_us(1);
	   vRegWrite4B(KSV_DATA, *(prKsvList+i));
	  }
	  HAL_Delay_us(1);
	  */
}

UINT32  HalGetKsvFifoAddr(void)
{
	/* get setting from designer */

	/*
	  UINT32 u4Addr;
	  u4Addr = u4RegRead4B(KSV_DATA);
	  u4Addr = (u4Addr>>16);
	  u4Addr &= 0x3FF;
	  return(u4Addr);
	  */
	return 1;
}


void  HalTriggerSHA(void)
{
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x1 << 20, SHA_GO);
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x0, SHA_GO);
}

void  HalRptStartAddrClr(void)
{
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x1 << 22, RPT_START_ADDR_CLR);
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, 0x0, RPT_START_ADDR_CLR);
}

void  HalSetSHALength(UINT32 u4Length)
{
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, u4Length << 8, SHA_LENGTH);
}

void  HalSetSHAAddr(UINT32 u4Addr)
{
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, u4Addr & 0xFF, RPT_START_ADDR_7_0);
	HDMI_WRITE32_MASK(REG_SHA_LENGTH, ((u4Addr >> 8) & 0xFF) << 18, RPT_START_ADDR_9_8);
}


/* AKSV is wrote */
BOOL HalHdcpAuthenticationStart(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_AUTH_START) {
		return TRUE;
	}

	return FALSE;
}

/* R0 is OK */
BOOL HalHdcpAuthenticationDone(void)
{
	if (HDMI_READ32(REG_INTR_STATE0) & INTR1_AUTH_DONE) {
		return TRUE;
	}

	return FALSE;
}

BOOL HalHdcpHdmiMode(void)
{
	if (HDMI_READ32(REG_HDCP_STAT) & RHDMI_MODE_EN) {
		return TRUE;
	}

	return FALSE;
}

void HalClearHdcpAuthenticationStartStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR1_AUTH_START);
}

void HalClearHdcpAuthenticationDoneStatus(void)
{
	HDMI_WRITE32(REG_INTR_STATE0, INTR1_AUTH_DONE);
}

BOOL HalIsVReady(void)
{
	if (HDMI_READ32(REG_INTR) & V_RDY_INTR) {
		return TRUE;
	}

	return FALSE;
}

void  HalSetBstatus(UINT16 u2Bstatus)
{
	/* for repeater, not support */
}



void  HalSetKsvStop(BOOL fgRiscAccressEnable)
{
	/* unknown function, may be for repeater */
}

void HDMI_HalGetAksv(BYTE *prRxAKSV)
{
	*(prRxAKSV + 0) = HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV1;
	*(prRxAKSV + 1) = HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV2;
	*(prRxAKSV + 2) = HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV3;
	*(prRxAKSV + 3) = HDMI_READ32(REG_SHD_AN0) & HDCP_AKSV4;
	*(prRxAKSV + 4) = HDMI_READ32(REG_SHD_AN0) & HDCP_AKSV5;
}

UINT16 HDMI_HalGetRi(void)
{
	UINT8 Ri_0, Ri_1;
	UINT16 Ri;

	Ri_0 = (HDMI_READ32(REG_SHD_AKSV) & HDCP_RI0_7_0) >> 0;
	Ri_1 = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_RI0_15_8) >> 24;
	Ri = ((Ri_0) | (Ri_1 << 8));

	return Ri;
}

void HDMI_HalGetAn(BYTE *prRxAn)
{
	*(prRxAn + 0) = (HDMI_READ32(REG_SHD_AN0) & HDCP_AN1) >> 16;
	*(prRxAn + 1) = (HDMI_READ32(REG_SHD_AN0) & HDCP_AN2) >> 24;
	*(prRxAn + 2) = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN3) >> 0;
	*(prRxAn + 3) = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN4) >> 8;
	*(prRxAn + 4) = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN5) >> 16;
	*(prRxAn + 5) = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN6) >> 24;
	*(prRxAn + 6) = (HDMI_READ32(REG_SHD_BSTATUS) & HDCP_AN7) >> 0;
	*(prRxAn + 7) = (HDMI_READ32(REG_SHD_BSTATUS) & HDCP_AN8) >> 8;
}

void HDMI_HalGetBksv(BYTE *prRxBKSV)
{
	*(prRxBKSV + 0) = (HDMI_READ32(REG_SHD_BKSV0) & HDCP_BKSV1) >> 16;
	*(prRxBKSV + 1) = (HDMI_READ32(REG_SHD_BKSV0) & HDCP_BKSV2) >> 24;
	*(prRxBKSV + 2) = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV3) >> 0;
	*(prRxBKSV + 3) = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV4) >> 8;
	*(prRxBKSV + 4) = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV5) >> 16;
}

UINT32  HDMI_HalGetVblank(void)
{
	UINT32 u4Value = 0;

	u4Value = HDMI_HalGetVFrontPorch() + HDMI_HalGetVBackPorch();

	return u4Value;
}


UINT32 MHLStable(void)
{
	UINT32 u4HTotal = 0;
	UINT32 u4VTotal = 0;
	BOOL bHResStable = 0;
	BOOL bVResStable = 0;
	UINT32 u4Ret = 0;

	bHResStable = HDMI_HalIsHresStable();
	bVResStable = HDMI_HalIsVresStable();
	u4HTotal = HDMI_HalGetHTotal();
	u4VTotal = HDMI_HalGetVTotal();

	if ((bHResStable == 0) || (bVResStable == 0)) {
		u4Ret = 0;
	} else if ((u4HTotal < 200) || (u4VTotal < 200)) {
		u4Ret = 0;
	} else {
		u4Ret = 1;
	}

	return u4Ret;
}



void MHLSetChannelOrder(UINT8 u1Value)
{
	if (u1Value == 0x1) {
		/*  MHL Channel 0_1_2 */
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x1 << 29, REG_CHANNEL_SEL0);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL1);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL2);
	} else if (u1Value == 0x2) {
		/*  MHL Channel 1_2_0 */
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x1 << 30, REG_CHANNEL_SEL1);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL0);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL2);
	} else if (u1Value == 0x3) {
		/*  MHL Channel 2_0_1 */
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x1 << 31, REG_CHANNEL_SEL2);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL0);
		HDMI_WRITE32_MASK(REG_MHL_CFG, 0x0, REG_CHANNEL_SEL1);
	} else {
	}
}

UINT8 MHLGetChannelOrder(void)
{
	UINT8 u1Value = 0;

	if (HDMI_READ32(REG_MHL_CFG) & REG_CHANNEL_SEL0) {
		u1Value = 0x1;
	} else if (HDMI_READ32(REG_MHL_CFG) & REG_CHANNEL_SEL1) {
		u1Value = 0x2;
	} else if (HDMI_READ32(REG_MHL_CFG) & REG_CHANNEL_SEL2) {
		u1Value = 0x3;
	} else {
		u1Value = 0x0;
	}

	return u1Value;
}


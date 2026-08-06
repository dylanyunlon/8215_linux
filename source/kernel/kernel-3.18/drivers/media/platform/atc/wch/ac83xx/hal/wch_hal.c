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

/**************************************************************************
* Header Files
**************************************************************************/
#ifdef __ARM2__
#include "x_types.h"
#endif
#include "atc/wch_drv.h"
#include "wch_log.h"
#include "wch_hal.h"
#include "wch_hw.h"
#include "atc/wch_if.h"
#include "x_bim.h"
#include "x_ckgen.h"
#include "winutil.h"
#include "mach/irqs_vector.h"
#ifdef __ARM2__
#include "ac8317-pinmux.h"
#endif
#ifndef __ARM2__
#include "x_debug.h"
#include "windows.h"
#include <mach/pinmux.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/pinctrl/consumer.h>
#endif
static volatile WCH_HAL_UNION_T *_prWch1HwReg = (WCH_HAL_UNION_T *)WCH1_HAL_REG;/* initialized for arm2 */
static volatile WCH_HAL_UNION_T *_prWch2HwReg = (WCH_HAL_UNION_T *)WCH2_HAL_REG;/* initialized for arm2 */
volatile WCH_HAL_UNION_T _rWch1SwReg;
volatile WCH_HAL_UNION_T _rWch2SwReg;

/* #define MT3360_VERSION */
void GET_WCH_SW_PTR(u8 id, WCH_HAL_UNION_T **reg)
{
	if (id == (u8)WCH_1)
		*reg = (WCH_HAL_UNION_T *) &_rWch1SwReg;
	else if (id == (u8)WCH_2)
		*reg = (WCH_HAL_UNION_T *) &_rWch2SwReg;
	else
		WCH_LOG(WCH_LOG_LVL_ERR, "get wrong id = %d", id);
}
void GET_WCH_HW_PTR(u8 id, WCH_HAL_UNION_T **reg)
{
	if (id == (u8)WCH_1)
		*reg = (WCH_HAL_UNION_T *) _prWch1HwReg;
	else if (id == (u8)WCH_2)
		*reg = (WCH_HAL_UNION_T *) _prWch2HwReg;
	else
		WCH_LOG(WCH_LOG_LVL_ERR, "get wrong id = %d", id);
}
#ifndef __ARM2__/* kernel build branch */

struct pinctrl_state *pins_wch_set_vsync = NULL;
struct pinctrl_state *pins_wch_set_gpio178 = NULL;
struct pinctrl_state *pins_wch_set_gpio179 = NULL;
struct pinctrl_state *pins_wch_set_gpio180 = NULL;
struct pinctrl_state *pins_wch_set_gpio181 = NULL;
struct pinctrl_state *pins_wch_set_gpio182 = NULL;
struct pinctrl_state *pins_wch_set_gpio183 = NULL;
struct pinctrl_state *pins_wch_set_gpio184 = NULL;
struct pinctrl_state *pins_wch_set_gpio185 = NULL;
struct pinctrl_state *pins_wch_set_dclk = NULL;
struct pinctrl_state *pins_wch_set_hsync = NULL;
struct pinctrl_state *pins_wch_set_de = NULL;

void WchEnabelClk(u8 u1WchId)
{
	if (u1WchId == WCH_1)
		clk_prepare_enable(clk_ac8317_wch0);
	else
		clk_prepare_enable(clk_ac8317_wch1);
}

void WchDisabelClk(u8 u1WchId)
{
	if (u1WchId == WCH_1)
		clk_disable_unprepare(clk_ac8317_wch0);
	else
		clk_disable_unprepare(clk_ac8317_wch1);
}

void WchSetPinmux(void)
{
	int ret = 0;

	/* WCH_REG_WRITE_MSK(WCH_PINMUX_REG, WCH_DGI_PINMUX, WCH_DGI_MASK); */
	pins_wch_set_vsync = pinctrl_lookup_state(pinctrl_wch, "wch_set_vsync_in");
	if (IS_ERR(pins_wch_set_vsync))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state vsync err %p\r\n",
			pins_wch_set_vsync);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_vsync);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select vsync err ret=%d\r\n",
			ret);

	pins_wch_set_gpio178 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio178_in");
	if (IS_ERR(pins_wch_set_gpio178))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio178 err %p\r\n",
			pins_wch_set_gpio178);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio178);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio178 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio179 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio179_in");
	if (IS_ERR(pins_wch_set_gpio179))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio179 err %p\r\n",
			pins_wch_set_gpio179);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio179);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio179 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio180 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio180_in");
	if (IS_ERR(pins_wch_set_gpio180))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio180 err %p\r\n",
			pins_wch_set_gpio180);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio180);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio180 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio181 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio181_in");
	if (IS_ERR(pins_wch_set_gpio181))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio181 err %p\r\n",
			pins_wch_set_gpio181);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio181);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio181 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio182 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio182_in");
	if (IS_ERR(pins_wch_set_gpio182))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio182 err %p\r\n",
			pins_wch_set_gpio182);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio182);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio182 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio183 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio183_in");
	if (IS_ERR(pins_wch_set_gpio183))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio183 err %p\r\n",
			pins_wch_set_gpio183);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio183);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio183 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio184 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio184_in");
	if (IS_ERR(pins_wch_set_gpio184))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio184 err %p\r\n",
			pins_wch_set_gpio184);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio184);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio184 err ret=%d\r\n",
			ret);

	pins_wch_set_gpio185 = pinctrl_lookup_state(pinctrl_wch, "wch_set_gpio185_in");
	if (IS_ERR(pins_wch_set_gpio185))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state gpio185 err %p\r\n",
			pins_wch_set_gpio185);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_gpio185);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select gpio185 err ret=%d\r\n",
			ret);

	pins_wch_set_dclk = pinctrl_lookup_state(pinctrl_wch, "wch_set_dclk_in");
	if (IS_ERR(pins_wch_set_dclk))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state dclk err %p\r\n",
			pins_wch_set_dclk);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_dclk);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select dclk err ret=%d\r\n",
			ret);

	pins_wch_set_hsync = pinctrl_lookup_state(pinctrl_wch, "wch_set_hsync_in");
	if (IS_ERR(pins_wch_set_hsync))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state hsync err %p\r\n",
			pins_wch_set_hsync);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_hsync);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select hsync err ret=%d\r\n",
			ret);

	pins_wch_set_de = pinctrl_lookup_state(pinctrl_wch, "wch_set_de_in");
	if (IS_ERR(pins_wch_set_de))
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin state de err %p\r\n",
			pins_wch_set_de);
	ret = pinctrl_select_state(pinctrl_wch, pins_wch_set_de);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetPinmux pin select de err ret=%d\r\n",
			ret);
}

void wchGetHwRegAddress(void)
{
	_prWch1HwReg = (WCH_HAL_UNION_T *) wch0_sysreg_base;
	_prWch2HwReg = (WCH_HAL_UNION_T *) wch1_sysreg_base;
}

#else/* arm2 build branch */

void WchEnabelClk(u8 u1WchId)
{
    if (u1WchId == WCH_1)
        CKGEN_AgtOnClk(e_CLK_WRITE_CHANEL);
    else
        CKGEN_AgtOnClk(e_CLK_WRITE_CHANEL_2);
}

void WchDisabelClk(u8 u1WchId)
{
    if (u1WchId == WCH_1)
        CKGEN_AgtOffClk(e_CLK_WRITE_CHANEL);
    else
        CKGEN_AgtOffClk(e_CLK_WRITE_CHANEL_2);
}

void WchSetPinmux(void)
{
    GPIO_MultiFun_Set(PIN_46_VSYNC_IN, CCIR601_TIMING_VSIN_SEL);
    GPIO_MultiFun_Set(PIN_178_VIN0, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_179_VIN1, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_180_VIN2, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_181_VIN3, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_182_VIN4, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_183_VIN5, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_184_VIN6, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_185_VIN7, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_29_DCLK_IN, CCIR656_601_DATAIN_SEL);
    GPIO_MultiFun_Set(PIN_47_HSYNC_IN, CCIR601_TIMING_HSIN_SEL);
    GPIO_MultiFun_Set(PIN_33_DE_IN, CCIR601_TIMING_DEIN_SEL);
}

#endif
/**************************************************************************

* Local/Static Functions

**************************************************************************/
void vWchHalDumpReg(u8 u1WchId)
{
	u32 u4Addr;
	WCH_HAL_UNION_T *prWchSwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	WCH_LOG((u32)WCH_LOG_LVL_REGRW, "vWchHalDumpReg id = %d\r\n", (int)u1WchId);
	for (u4Addr = 0; u4Addr < WCH_HAL_REG_NUM; u4Addr += 4U) {
		WCH_LOG((u32)WCH_LOG_LVL_REGRW, "[0x%08x] = 0x%08x ",
			 (unsigned int)(prWchSwReg + u4Addr),
			 (unsigned int)prWchSwReg->au4Reg[u4Addr]);
		if (((u4Addr + 4U) % 16U) == 0)
#ifndef __ARM2__
			pr_debug("\r\n");
#else
			printk("\r\n");
#endif
	}
}
void WchHalSetPinmux(WCH_DATA_SRC_E eSrcType)
{
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetPinmux SrcType = %d, 0x%x\r\n", (int)eSrcType,
		 (unsigned int)WCH_REG_READ(WCH_PINMUX_REG));
	if (eSrcType == DATA_SRC_DGI)
		WchSetPinmux();
}
void WchHalSetSrcType(u8 u1WchId, WCH_DATA_SRC_E eSrcType)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetSrcType id = %d, SrcType = %d\r\n", (int)u1WchId,
		   (int)eSrcType);

#ifndef MT3360_VERSION
	if (u1WchId == WCH_1) {
		switch (eSrcType) {
		case DATA_SRC_HDMI:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH1_SEL_HDMI | WCH1_SEL_VDOIN),
					   WCH1_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_HDMI | WCH1_CLK_SEL_VDO),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_YPBPR:
		case DATA_SRC_VGA:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH1_SEL_YPBPR | WCH1_SEL_VDOIN),
					   WCH1_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_YPBPR | WCH1_CLK_SEL_VDO),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_TVD:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH1_SEL_TVD | WCH1_SEL_VDOIN),
					   WCH1_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_TVD | WCH1_CLK_SEL_VDO),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_DGI:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH1_SEL_DGI | WCH1_SEL_VDOIN),
					   WCH1_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_DGI | WCH1_CLK_SEL_VDO),
					   WCH1_CLK_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH_DGI_ENABLE | WCH_DGI_CLK_INVERT),
					   WCH_DGI_CLK_MASK);
			break;
		case DATA_SRC_DVD:
			WCH_REG_WRITE_MSK(WCH_MIX_DVP_REG, DVP_OUT_ENABLE, DVP_OUT_ENABLE);
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_DVD | WCH1_SEL_WRITE_BACK),
					   (WCH1_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_WRTIEBACK),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_FMTR:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_FMTR | WCH1_SEL_WRITE_BACK),
					   (WCH1_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_WRTIEBACK),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_FMTF:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_FMTF | WCH1_SEL_WRITE_BACK),
					   (WCH1_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_WRTIEBACK),
					   WCH1_CLK_SEL_MASK);
			break;
		case DATA_SRC_MIX:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_MIX | WCH1_SEL_WRITE_BACK),
					   (WCH1_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH1_CLK_SEL_WRTIEBACK),
					   WCH1_CLK_SEL_MASK);
			break;
		default:
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchHalSetSrcType: Not support SrcType = %d, id = %d\r\n",
				 (int)eSrcType, (int)u1WchId);
			break;
		}
	} else if (u1WchId == WCH_2) {
		switch (eSrcType) {
		case DATA_SRC_HDMI:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH2_SEL_HDMI | WCH2_SEL_VDOIN),
					   WCH2_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH2_CLK_SEL_HDMI | WCH2_CLK_SEL_VDO),
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_YPBPR:
		case DATA_SRC_VGA:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH2_SEL_YPBPR | WCH2_SEL_VDOIN),
					   WCH2_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH2_CLK_SEL_YPBPR | WCH2_CLK_SEL_VDO),
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_TVD:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH2_SEL_TVD | WCH2_SEL_VDOIN),
					   WCH2_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH2_CLK_SEL_TVD | WCH2_CLK_SEL_VDO),
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_DGI:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WCH2_SEL_DGI | WCH2_SEL_VDOIN),
					   WCH2_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH2_CLK_SEL_DGI | WCH2_CLK_SEL_VDO),
					   WCH2_CLK_SEL_MASK);
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, (WCH_DGI_ENABLE | WCH_DGI_CLK_INVERT),
					   WCH_DGI_CLK_MASK);
			break;
		case DATA_SRC_DVD:
			WCH_REG_WRITE_MSK(WCH_MIX_DVP_REG, DVP_OUT_ENABLE, DVP_OUT_ENABLE);
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_DVD | WCH2_SEL_WRITE_BACK),
					   (WCH2_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, WCH2_CLK_SEL_WRTIEBACK,
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_FMTR:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_FMTR | WCH2_SEL_WRITE_BACK),
					   (WCH2_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, WCH2_CLK_SEL_WRTIEBACK,
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_FMTF:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_FMTF | WCH2_SEL_WRITE_BACK),
					   (WCH2_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, WCH2_CLK_SEL_WRTIEBACK,
					   WCH2_CLK_SEL_MASK);
			break;
		case DATA_SRC_MIX:
			WCH_REG_WRITE_MSK(WCH_MIX_REG, (WRITE_BACK_MIX | WCH2_SEL_WRITE_BACK),
					   (WCH2_SEL_MASK | WRITE_BACK_MASK));
			WCH_REG_WRITE_MSK(WCH_CLK_SEL_REG, WCH2_CLK_SEL_WRTIEBACK,
					   WCH2_CLK_SEL_MASK);
			break;
		default:
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchHalSetSrcType: Not support SrcType = %d, id = %d\r\n",
				 (int)eSrcType, (int)u1WchId);
			break;
		}
	} else{
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetSrcType: Not support write channel id = %d\r\n",
			 (int)u1WchId);
	}
#endif
	if (eSrcType == DATA_SRC_HDMI) {

#if WCH_HDMI_TIMING_EN
		prWchSwReg->rField.fgCKeepIn = 0;
		prWchSwReg->rField.fgAdjDE = 1;

#else
		prWchSwReg->rField.fgCKeepIn = 1;
		prWchSwReg->rField.fgAdjDE = 0;

#endif
		prWchSwReg->rField.fgHdmiCtl = 1;
	} else{
		prWchSwReg->rField.fgCKeepIn = 0;
		prWchSwReg->rField.fgAdjDE = 0;
		prWchSwReg->rField.fgHdmiCtl = 0;
	}
	prWchHwReg->au4Reg[(0x0 / 4)] = prWchSwReg->au4Reg[(0x0 / 4)];
	prWchHwReg->au4Reg[(0x54 / 4)] = prWchSwReg->au4Reg[(0x54 / 4)];
	if (eSrcType >= DATA_SRC_DVD) {
		prWchSwReg->rField.fgHStartSel = 0;
		prWchSwReg->rField.fgHEndSel = 0;
	}

#if WCH_HDMI_TIMING_EN
	else if (eSrcType == DATA_SRC_HDMI) {
		prWchSwReg->rField.fgHStartSel = 0;
		prWchSwReg->rField.fgHEndSel = 0;
	}

#endif
	else if (eSrcType != DATA_SRC_DGI) {	/* DGI config in WchHalSetInput function */
		prWchSwReg->rField.fgHStartSel = 1;
		prWchSwReg->rField.fgHEndSel = 1;
	}
	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
}
void WchHalSetInput(u8 u1WchId, WCH_DATA_FMT_E eSrcFmt, u32 u4SrcWidth, u32 u4SrcHeight,
		       u32 u4StartX, u32 u4StartYTop, u32 u4StartYBot,
		       bool fgProgressive)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		   "WchHalSetInput id = %d, SrcFmt = %d, width = %d, height = %d, progressive = %d\r\n",
		   (int)u1WchId, (int)eSrcFmt, (int)u4SrcWidth, (int)u4SrcHeight,
		   (int)fgProgressive);
	WCH_LOG(WCH_LOG_LVL_DBG,
		 "WchHalSetInput id = %d, startX = %d, YTop = %d, YBottom = %d\r\n", (int)u1WchId,
		 (int)u4StartX, (int)u4StartYTop, (int)u4StartYBot);

	    /* 1. Input Format Configuration. */
	    /* Input source is YUV422/YUV444/656/601 format, which includes
		16-bit/24-bit/8-bit source data and timing information */
	    /* such as field, vertical sync and horizontal sync. */
	switch (eSrcFmt) {
	case DATA_FMT_YUV422:
		prWchSwReg->rField.fgInVdoFmt = 0;
		prWchSwReg->rField.fgInputFmt = 1;
		break;
	case DATA_FMT_YUV444:
		prWchSwReg->rField.fgInVdoFmt = 1;
		prWchSwReg->rField.fgInputFmt = 1;
		break;
	case DATA_FMT_BT656:
		prWchSwReg->rField.fgInVdoFmt = 0;
		prWchSwReg->rField.fgInputFmt = 0;
		prWchSwReg->rField.fgHStartSel = 1;
		prWchSwReg->rField.fgHEndSel = 1;
		prWchSwReg->rField.fgAdjDE = 1;	/* when fgHStartSel and
						 fgHEndSel is 1, send data_active as sync */
		break;
	case DATA_FMT_BT601:
		prWchSwReg->rField.fgInVdoFmt = 1;
		prWchSwReg->rField.fgInputFmt = 0;
		prWchSwReg->rField.fgHStartSel = 0;
		prWchSwReg->rField.fgHEndSel = 0;
		prWchSwReg->rField.fgAdjDE = 1;	/* when fgHStartSel and fgHEndSel
						 is 0, send hsync to wch, and need start pixel */
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchHalSetInput Not support FMT and set fmt YUV444: id = %d, SrcFmt = %d, width = %d, height = %d\r\n",
			 (int)u1WchId, (int)eSrcFmt, (int)u4SrcWidth, (int)u4SrcHeight);
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchHalSetInput Not support FMT id = %d, startX = %d, YTop = %d, YBottom = %d, progressive = %d\r\n",
			 (int)u1WchId, (int)u4StartX, (int)u4StartYTop, (int)u4StartYBot,
			 (int)fgProgressive);
		prWchSwReg->rField.fgInVdoFmt = 1;
		prWchSwReg->rField.fgInputFmt = 1;
		break;
	}
	prWchSwReg->rField.u4FieldSel = 2;

	/* 3. Display Zone Configuration: */
	/* BGnPixel and BGActHCNT are used to decide the display zone of one line, and BGnPixel is */
	/* the first pixel of video, BGActHCNT is the width of video. */
	/* BGTopLine, BGBottomLine and Bgactiveline are used to decide the display zone of one */
	/* field, BGTopLine and BGBottomLine decide the first line of one field, and Bgactiveline */
	/* decides the height of one field. */
	if (eSrcFmt >= DATA_FMT_BT656) {
		prWchSwReg->rField.u4HActCnt = u4SrcWidth << 1;	/* DGI source
						 is 8-bit bus, and need 2 cycle to transfer 1 Y/C data */
	} else{
		prWchSwReg->rField.u4HActCnt = u4SrcWidth;
	}
	if (fgProgressive) {
		prWchSwReg->rField.u4ActLine = u4SrcHeight - 1;
		prWchSwReg->rField.fgProgSel = 1;
		prWchSwReg->rField.fgIntrMode = 0;
	} else{

#if WCH_HDMI_TIMING_EN
		prWchSwReg->rField.u4ActLine = u4SrcHeight - 1;/* retiming
							vertical data /2 in table */
		prWchSwReg->rField.fgProgSel = 0;
		prWchSwReg->rField.fgIntrMode = 1;

#else
		prWchSwReg->rField.u4ActLine = (u4SrcHeight >> 1) - 1;	/* Interlace
							 only half vertical data for one field */
		prWchSwReg->rField.fgProgSel = 0;
		prWchSwReg->rField.fgIntrMode = 1;

#endif
	}
	prWchSwReg->rField.fgInFldSet = 0;
	if (prWchSwReg->rField.fgCKeepIn == 0) {
		prWchSwReg->rField.u4HStartPxl = u4StartX;
		prWchSwReg->rField.u4TopLine = u4StartYTop;
		prWchSwReg->rField.u4BotLine = u4StartYBot;
	}
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
	prWchHwReg->au4Reg[(0xC / 4)] = prWchSwReg->au4Reg[(0xC / 4)];
	prWchHwReg->au4Reg[(0x18 / 4)] = prWchSwReg->au4Reg[(0x18 / 4)];
	prWchHwReg->au4Reg[(0x24 / 4)] = prWchSwReg->au4Reg[(0x24 / 4)];
	prWchHwReg->au4Reg[(0x34 / 4)] = prWchSwReg->au4Reg[(0x34 / 4)];
	WCH_LOG(WCH_LOG_LVL_REGRW, "WchHalSetInput register %x = %x 04 = %x\r\n",
		 (unsigned int)prWchHwReg, (unsigned int)prWchHwReg->au4Reg[0],
		 (unsigned int)prWchHwReg->au4Reg[0x4 / 4]);
	WCH_LOG(WCH_LOG_LVL_REGRW, "WchHalSetInput register 18 = %x 24 = %x\r\n",
		 (unsigned int)prWchHwReg->au4Reg[0x18 / 4],
		 (unsigned int)prWchHwReg->au4Reg[0x24 / 4]);
}
void WchHalSetOutput(u8 u1WchId, WCH_DATA_FMT_E eDstFmt, bool fgProgressive,
			u32 u4DstWidth, u32 u4DstHeight)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		   "WchHalSetOutput id = %d, SrcFmt = %d, width = %d, height = %d, progressive = %d\r\n",
		   (int)u1WchId, (int)eDstFmt, (int)u4DstWidth, (int)u4DstHeight,
		   (int)fgProgressive);

	    /* 4. Output Format Configuration: */
	    /* bg422mode is used to decide the output format(420 or 422). */
	    /* BGProgSel is set to 1'b1 when the input source is progressive. */
	    /* Bgyfldnum and Bgcfldnum are used to generate the maximum line number for output control. */
	    /* bghsize_dw decides the data number of one line for output control. */
	    /* BGCHcntEnd_DW and BGYHcntEnd are used to decide the last data in one line for output control. */
	switch (eDstFmt) {
	case DATA_FMT_YUV420:
		prWchSwReg->rField.fgOutputFmt = 0;
		break;
	case DATA_FMT_YUV422:
		prWchSwReg->rField.fgOutputFmt = 1;
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalSetOutput: Not support this vdo format %d\r\n",
			 (int)eDstFmt);
		break;
	}
	if (fgProgressive) {
		prWchSwReg->rField.u4YFldNum = u4DstHeight - 1;
		if (eDstFmt == DATA_FMT_YUV422)
			prWchSwReg->rField.u4CFldNum = u4DstHeight - 1;
		else
			prWchSwReg->rField.u4CFldNum = (u4DstHeight >> 1) - 1;
#ifdef MT3360_VERSION
		prWchSwReg->rField.u4BGYHEndDW = (u4DstWidth >> 2) - 1;
		prWchSwReg->rField.u4BGCHEndDW = (u4DstWidth >> 2) - 1;

#else
		prWchSwReg->rField.u4BGYHEndDW = (u4DstWidth >> 3) - 1;
		prWchSwReg->rField.u4BGCHEndDW = (u4DstWidth >> 3) - 1;

#endif
		prWchSwReg->rField.u4HSizeDW = u4DstWidth >> 3;
	} else{
		prWchSwReg->rField.u4YFldNum = (u4DstHeight >> 1) - 1;
		if (eDstFmt == DATA_FMT_YUV422)
			prWchSwReg->rField.u4CFldNum = (u4DstHeight >> 1) - 1;
		else
			prWchSwReg->rField.u4CFldNum = (u4DstHeight >> 2) - 1;
#ifdef MT3360_VERSION
		prWchSwReg->rField.u4BGYHEndDW = ((u4DstWidth >> 4) - 1) << 1;
		prWchSwReg->rField.u4BGCHEndDW = ((u4DstWidth >> 4) - 1) << 1;

#else
		prWchSwReg->rField.u4BGYHEndDW = (u4DstWidth >> 2) - 1;
		prWchSwReg->rField.u4BGCHEndDW = (u4DstWidth >> 2) - 1;

#endif
		prWchSwReg->rField.u4HSizeDW = u4DstWidth >> 2;
	}
	prWchSwReg->rField.fgOutFldSet = 0;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	prWchHwReg->au4Reg[(0x28 / 4)] = prWchSwReg->au4Reg[(0x28 / 4)];
	prWchHwReg->au4Reg[(0x38 / 4)] = prWchSwReg->au4Reg[(0x38 / 4)];
	WCH_LOG(WCH_LOG_LVL_REGRW, "WchHalSetOutput register 0 = %x 28 = %x 38 = %x\r\n",
		 (unsigned int)prWchHwReg->au4Reg[0], (unsigned int)prWchHwReg->au4Reg[0x28 / 4],
		 (unsigned int)prWchHwReg->au4Reg[0x38 / 4]);
}
void WchHalSetPolarity(u8 u1WchId, bool VSyncPolarity, bool HSyncPolarity,
			  bool BotFieldFirst)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		   "WchHalSetPolarity id = %d, V = %d, H = %d, BotFieldFirst = %d\r\n",
		   (int)u1WchId, (int)VSyncPolarity, (int)HSyncPolarity, (int)BotFieldFirst);

	/* 2. Input Timing Configuration: */
	/* These registers are used to convert the input timing information to uniform timing */
	/* information, in which, high level represents sync period for HSYNC or VSYNC, and field */
	/* flag equals to 1'b0 means top field. */

	/* set Vsync polarity */
	prWchSwReg->rField.fgVsynInv = VSyncPolarity;

	/* set Hsync polarity */
	prWchSwReg->rField.fgHsynInv = HSyncPolarity;

	/* set filed polarity */
	prWchSwReg->rField.fgFldInv = BotFieldFirst;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	WCH_LOG(WCH_LOG_LVL_REGRW, "WchHalSetPolarity register 0 = %x\r\n",
		 (unsigned int)prWchHwReg->au4Reg[0]);
}
void WchHalSetCtrlSignal(u8 u1WchId, bool fgProgressive)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetCtrlSignal id = %d, Progressive = %d\r\n",
		   (int)u1WchId, (int)fgProgressive);

	/* Control address active in next vsync or next interrupt */
	/* prWchSwReg->rField.fgAddrCtl = 1; */
	/* prWchHwReg->au4Reg[(0x04/4)] = prWchSwReg->au4Reg[(0x04/4)]; */

	/* Control Signal Configuration: */
	/* BGAvCode[2:0] is used to select the field reset signal. */
	prWchSwReg->rField.u4FldEndSel = 1;

	/* BGAvCode[3] and BGAvCode[8] is used to control the state machine, 1 is suggested. */
	prWchSwReg->rField.fgVsyncRst = 1;
	prWchSwReg->rField.fgVsyncEn = 1;

	/* BGAvCode[7:5] is used to select the interrupt generation scheme. */
#if 1
	prWchSwReg->rField.u4IntrEn = 2;	/* 1-DRAM receive request; 2-active falling edge; 4-DRAM write finish */
#else
	if (fgProgressive)
		prWchSwReg->rField.u4IntrEn = 2;
	else
		prWchSwReg->rField.u4IntrEn = 4;/* if tvd source
			set 4 for 3360, need to check in 8317 DE suggest set 2 */
#endif
	/* BGAvCode[10] is used to enable the reset the initial address for writing memory when line-begin. */
	prWchSwReg->rField.fgLStartEn = 1;

	/* BGAvCode[12] is sram enable. */
	prWchSwReg->rField.fgSramCsEn = 1;

	/* BGAvCode[13] is test-bar enable. */
	prWchSwReg->rField.fgTstBar = 0;

#ifdef MT3360_VERSION
	/* Tmp code and romove in 8317 */
	prWchSwReg->rField.u4Option = 1;

#else
	prWchSwReg->rField.u4Option = 0;

#endif
	prWchHwReg->au4Reg[(0x3C / 4)] = prWchSwReg->au4Reg[(0x3C / 4)];
	WCH_LOG(WCH_LOG_LVL_REGRW, "WchHalSetCtrlSignal register 3c = %x\r\n",
		 (unsigned int)prWchHwReg->au4Reg[0x3C / 4]);
}
void WchHalSetYCAddr(u8 u1WchId, u32 u4YAddr, u32 u4CAddr)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_DBG, "WchHalSetYCAddr id = %d, Y = %x, C = %x\r\n", (int)u1WchId,
		   (unsigned int)u4YAddr, (unsigned int)u4CAddr);
	prWchSwReg->rField.u4Y0Addr = u4YAddr >> 3;
	prWchSwReg->rField.u4C0Addr = u4CAddr >> 3;
	prWchHwReg->au4Reg[(0x8 / 4)] = prWchSwReg->au4Reg[(0x8 / 4)];
	prWchHwReg->au4Reg[(0x10 / 4)] = prWchSwReg->au4Reg[(0x10 / 4)];
}
void WchHalSetUvYcSwap(u8 u1WchId, u8 u1Mask, u8 u1UVSwap, u8 u1YCSwap)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetUvYcSwap id = %d, Mask = %d, UV = %d, YC = %d\r\n",
		   (int)u1WchId, (int)u1Mask, (int)u1UVSwap, (int)u1YCSwap);
	if (u1Mask & WCH_UV_SWAP_MASK)
		prWchSwReg->rField.fgUVSwap = u1UVSwap;
	if (u1Mask & WCH_YC_SWAP_MASK)
		prWchSwReg->rField.fgYCSwap = u1YCSwap;
	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
}
void WchHalSetYUVDelay(u8 u1WchId, u8 u1Mask, u8 u1YSel, u8 u1USel, u8 u1VSel)	/* for CVBS */
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL,
		   "WchHalSetYUVDelay id = %d, Mask = %d, Y = %d, U = %d, V = %d\r\n",
		   (int)u1WchId, (int)u1Mask, (int)u1YSel, (int)u1USel, (int)u1VSel);
	if (u1Mask & WCH_Y_SEL_MASK) {
		prWchSwReg->rField.u4DelayY = u1YSel;
		prWchSwReg->rField.u4DeSel = u1YSel;
	}
	if (u1Mask & WCH_U_SEL_MASK)
		prWchSwReg->rField.u4DelayU = u1USel;

	if (u1Mask & WCH_V_SEL_MASK)
		prWchSwReg->rField.u4DelayV = u1VSel;

	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
}
void WchHalSetDEDelay(u8 u1WchId, u8 u4DeSel)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetDEDelay id = %d, DeSel = %d\r\n", (int)u1WchId,
		   (int)u4DeSel);
	prWchSwReg->rField.u4DeSel = u4DeSel;
	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
}
void WchHalSetMirror(u8 u1WchId, u32 u4Mirror)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalSetMirror id = %d, Mirror = %d\r\n", (int)u1WchId,
		   (int)u4Mirror);
	switch (u4Mirror) {
	case WCH_CFG_MIRROR_H:	/* H Flip */
		prWchSwReg->rField.fgHFlip = 1;
		prWchSwReg->rField.fgVFlip = 0;
		break;
	case WCH_CFG_MIRROR_V:	/* V Flip */
		prWchSwReg->rField.fgHFlip = 0;
		prWchSwReg->rField.fgVFlip = 1;
		break;
	case WCH_CFG_MIRROR_HV:	/* H Flip and V Flip */
		prWchSwReg->rField.fgHFlip = 1;
		prWchSwReg->rField.fgVFlip = 1;
		break;
	case WCH_CFG_UNMIRROR:
	default:
		prWchSwReg->rField.fgHFlip = 0;
		prWchSwReg->rField.fgVFlip = 0;
		break;
	}
	prWchHwReg->au4Reg[(0x4 / 4)] = prWchSwReg->au4Reg[(0x4 / 4)];
}
void WchHalLineAverageEn(u8 u1WchId, bool fgOn)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalLineAverageEn id = %d, on = %d\r\n", (int)u1WchId,
		   (int)fgOn);
	if (fgOn)
		prWchSwReg->rField.u4Option = 2;	/* enable line average */
	else
		prWchSwReg->rField.u4Option = 1;	/* disable line average */

	prWchHwReg->au4Reg[(0x3C / 4)] = prWchSwReg->au4Reg[(0x3C / 4)];
}
void WchHalSetRegTouch(u8 u1WchId)
{

#if WCH_SHADOW_ENABLE
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_DBG, "WchHalRegTouch id = %d\r\n", (int)u1WchId);
	prWchSwReg->rField.fgRegTouch = 0;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	prWchSwReg->rField.fgRegTouch = 1;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];

#endif
}
bool WchHalIsOn(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalIsOn id = %d\r\n", (int)u1WchId);
	return (prWchSwReg->rField.fgWchOn && prWchSwReg->rField.fgLineAddrEn);
}
void WchHalStart(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "Before WchHalStart id = %d 0x344:%x\r\n", (int)u1WchId,
		 (unsigned int)WCH_REG_READ(0x42344));
	WchHalResetFsm(u1WchId);
	prWchSwReg->rField.fgWchOn = 1;
	prWchSwReg->rField.fgLineAddrEn = 1;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	WCH_LOG(WCH_LOG_LVL_HAL, "After WchHalStart id = %d 0x344:%x\r\n", (int)u1WchId,
		 (unsigned int)WCH_REG_READ(0x42344));
}
EXPORT_SYMBOL(WchHalStart);
void WchHalStop(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "Before WchHalstop id = %d 0x344:%x\r\n", (int)u1WchId,
		   (unsigned int)WCH_REG_READ(0x42344));
	WchHalDisableFsm(u1WchId);
	prWchSwReg->rField.fgWchOn = 0;
	prWchSwReg->rField.fgLineAddrEn = 0;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	WCH_LOG(WCH_LOG_LVL_HAL, "After WchHalstop id = %d 0x344:%x\r\n", (int)u1WchId,
		 (unsigned int)WCH_REG_READ(0x42344));
}
EXPORT_SYMBOL(WchHalStop);
void WchHalEnableFsm(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalEnableFsm id = %d\r\n", (int)u1WchId);
	prWchSwReg->rField.u4FsmRst = 0;
	prWchHwReg->au4Reg[(0x30 / 4)] = prWchSwReg->au4Reg[(0x30 / 4)];
}
void WchHalDisableFsm(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalDisableFsm id = %d\r\n", (int)u1WchId);
	prWchSwReg->rField.u4FsmRst = 3;
	prWchHwReg->au4Reg[(0x30 / 4)] = prWchSwReg->au4Reg[(0x30 / 4)];
}
void WchHalResetFsm(u8 u1WchId)
{
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);
	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalResetFsm id = %d\r\n", (int)u1WchId);
	prWchSwReg->rField.u4FsmRst = 3;
	prWchHwReg->au4Reg[(0x30 / 4)] = prWchSwReg->au4Reg[(0x30 / 4)];
	prWchSwReg->rField.u4FsmRst = 0;
	prWchHwReg->au4Reg[(0x30 / 4)] = prWchSwReg->au4Reg[(0x30 / 4)];
}
void WchHalInit(u8 u1WchId)
{
	u32 u4Idx = 0;
	WCH_HAL_UNION_T *prWchSwReg = NULL;
	WCH_HAL_UNION_T *prWchHwReg = NULL;

	GET_WCH_SW_PTR(u1WchId, &prWchSwReg);
	GET_WCH_HW_PTR(u1WchId, &prWchHwReg);

	WchEnabelClk(u1WchId);

	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalInit exit id = %d clock = %x\r\n", (int)u1WchId,
		 (unsigned int)WCH_REG_READ(WCH_CK_REG));
	for (u4Idx = 0; u4Idx < WCH_HAL_REG_NUM; u4Idx++) {
		prWchSwReg->au4Reg[u4Idx] = prWchHwReg->au4Reg[u4Idx];
	}
	prWchSwReg->rField.u4Changereq = 1;
	prWchSwReg->rField.u4DisReqTimeOut = 1;

#if WCH_SHADOW_ENABLE
	prWchSwReg->rField.fgCFreeOn = 0;

#else
	prWchSwReg->rField.fgCFreeOn = 1;

#endif
	prWchSwReg->rField.fgSramCtl = 1;/* hw control line buffer,
					read & write maybe have risk and need set to 1 */
	prWchSwReg->rField.fgSramPp = 1;
	prWchHwReg->au4Reg[0] = prWchSwReg->au4Reg[0];
	prWchHwReg->au4Reg[0x4 / 4] = prWchSwReg->au4Reg[0x4 / 4];
	prWchHwReg->au4Reg[0x1c / 4] = prWchSwReg->au4Reg[0x1c / 4];
}
void WchHalDeinit(u8 u1WchId)
{
	WchDisabelClk(u1WchId);

	WCH_LOG(WCH_LOG_LVL_HAL, "WchHalDeinit id = %d clock = %x\r\n", (int)u1WchId,
		 (unsigned int)WCH_REG_READ(WCH_CK_REG));
}
EXPORT_SYMBOL(WchHalDeinit);
WCH_DATA_SRC_E WchHalGetSrcType(u8 u1WchId)
{
	u32 u4RegVal = 0;
	WCH_DATA_SRC_E eSrcType = DATA_SRC_UNKNOWN;

	u4RegVal = WCH_REG_READ(WCH_MIX_REG);
	if (u1WchId == WCH_1) {
		if (u4RegVal & WCH1_SEL_VDOIN) {
			switch (u4RegVal & WCH1_VDOIN_MASK) {
			case WCH1_SEL_HDMI:
				eSrcType = DATA_SRC_HDMI;
				break;
			case WCH1_SEL_YPBPR:
				eSrcType = DATA_SRC_YPBPR;
				break;
			case WCH1_SEL_TVD:
				eSrcType = DATA_SRC_TVD;
				break;
			case WCH1_SEL_DGI:
				eSrcType = DATA_SRC_DGI;
				break;
			}
		} else{
			/* Write back path */
			switch (u4RegVal & WRITE_BACK_MASK) {
			case WRITE_BACK_DVD:
				eSrcType = DATA_SRC_DVD;
				break;
			case WRITE_BACK_FMTR:
				eSrcType = DATA_SRC_FMTR;
				break;
			case WRITE_BACK_FMTF:
				eSrcType = DATA_SRC_FMTF;
				break;
			case WRITE_BACK_MIX:
				eSrcType = DATA_SRC_MIX;
				break;
			}
		}
	} else if (u1WchId == WCH_2) {
		if (u4RegVal & WCH2_SEL_VDOIN) {
			switch (u4RegVal & WCH2_VDOIN_MASK) {
			case WCH2_SEL_HDMI:
				eSrcType = DATA_SRC_HDMI;
				break;
			case WCH2_SEL_YPBPR:
				eSrcType = DATA_SRC_YPBPR;
				break;
			case WCH2_SEL_TVD:
				eSrcType = DATA_SRC_TVD;
				break;
			case WCH2_SEL_DGI:
				eSrcType = DATA_SRC_DGI;
				break;
			}
		} else{

			/* Write back path */
			switch (u4RegVal & WRITE_BACK_MASK) {
			case WRITE_BACK_DVD:
				eSrcType = DATA_SRC_DVD;
				break;
			case WRITE_BACK_FMTR:
				eSrcType = DATA_SRC_FMTR;
				break;
			case WRITE_BACK_FMTF:
				eSrcType = DATA_SRC_FMTF;
				break;
			case WRITE_BACK_MIX:
				eSrcType = DATA_SRC_MIX;
				break;
			}
		}
	} else {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchHalGetSrcType: Not support write channel id = %d\r\n",
			 (int)u1WchId);
	}
	return eSrcType;
}

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
/*****************************************************************************
*  Plane Mixer: Interface
*****************************************************************************/

#ifndef _PMX_HAL_C_
#define _PMX_HAL_C_

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/interrupt.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/display_inc.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include "windows.h"
#include "winutil.h"
#include "drv_dual.h"

#else
#include "x_types.h"
#include "assert.h"
#include "pmx_hal.h"
#include "drv_osd_if.h"
#include "display_inc.h"
#endif
#include "chip_ver.h"
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "vdp.h"
#include "pmx_hw.h"
#include "log.h"
#include "pmx_vfy_hal.h"
#include "x_hal_ic.h"
#include "drv_config.h"
#include "irqs_vector.h"
#include "drv_env.h"
#include "x_bim.h"
#include "x_os.h"

static volatile PMX_HAL_DISP_MAIN_UNION_T *_prPmxDispMainHwReg;
static volatile PMX_HAL_DISP_AUX_UNION_T *_prPmxDispAuxHwReg;
static volatile PMX_HAL_MIX_UNION_T *_prPmxMixHwReg;

PMX_HAL_DISP_MAIN_UNION_T _rPmxHalMainSwReg;
PMX_HAL_DISP_AUX_UNION_T _rPmxHalAuxSwReg;
PMX_HAL_MIX_UNION_T   _rPmxHalMixSwReg;

__u8 _rPmxDispMainRegMode[PMX_HAL_DISP_MAIN_REG_NUM];
__u8 _rPmxDispAuxRegMode[PMX_HAL_DISP_AUX_REG_NUM];
__u8 _rPmxMixRegMode[PMX_HAL_MIX_REG_NUM];

static bool _fgPmxMainIsrInited = FALSE;
static bool _fgPmxAuxIsrInited = FALSE;

/*static bool _fgPmxMainEnable = FALSE;*/
/*static bool _fgPmxAuxEnable = FALSE;*/

bool _fgPmxMainResetInVSync = FALSE;
bool _fgPmxAuxResetInVSync = FALSE;

HANDLE _hVdp1VSyncEvent = NULL;
HANDLE _hVdp2VSyncEvent = NULL;
bool fgWaitVdp1VSync = FALSE;
bool fgWaitVdp2VSync = FALSE;
#define VSync_TimeOut  32 /*16.7 = 1/60*/

#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_VA + (arg)) = val)
#define ReadREG(arg)       (*(volatile __u32*)(IO_BASE_VA + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

static __u32 g_u4HTotal;
static __u32 g_u4VTotal;

#ifndef __ARM2__
wait_queue_head_t wq_workthread;
atomic_t workthread_wakeup;

struct pinctrl_state *pins_fb_32_de = NULL;
struct pinctrl_state *pins_fb_194_hsync = NULL;
struct pinctrl_state *pins_fb_195_vsync = NULL;
struct pinctrl_state *pins_fb_159_vb0 = NULL;
struct pinctrl_state *pins_fb_160_vb1 = NULL;
struct pinctrl_state *pins_fb_168_vg0 = NULL;
struct pinctrl_state *pins_fb_169_vg1 = NULL;
struct pinctrl_state *pins_fb_186_vr0 = NULL;
struct pinctrl_state *pins_fb_187_vr1 = NULL;
struct pinctrl_state *pins_fb_161_vb2 = NULL;
struct pinctrl_state *pins_fb_163_vb3 = NULL;
struct pinctrl_state *pins_fb_164_vb4 = NULL;
struct pinctrl_state *pins_fb_165_vb5 = NULL;
struct pinctrl_state *pins_fb_166_vb6 = NULL;
struct pinctrl_state *pins_fb_167_vb7 = NULL;
struct pinctrl_state *pins_fb_170_vg2 = NULL;
struct pinctrl_state *pins_fb_171_vg3 = NULL;
struct pinctrl_state *pins_fb_172_vg4 = NULL;
struct pinctrl_state *pins_fb_173_vg5 = NULL;
struct pinctrl_state *pins_fb_174_vg6 = NULL;
struct pinctrl_state *pins_fb_175_vg7 = NULL;
struct pinctrl_state *pins_fb_188_vr2 = NULL;
struct pinctrl_state *pins_fb_189_vr3 = NULL;
struct pinctrl_state *pins_fb_190_vr4 = NULL;
struct pinctrl_state *pins_fb_191_vr5 = NULL;
struct pinctrl_state *pins_fb_192_vr6 = NULL;
struct pinctrl_state *pins_fb_193_vr7 = NULL;

__u32 dwPmxHalSetPinctrl(void)
{
	int ret = -EINVAL;

	pins_fb_32_de = pinctrl_lookup_state(pinctrl_fb, "fb_gpio32_de");
	if (IS_ERR(pins_fb_32_de)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_32_de);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_32_de);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_194_hsync = pinctrl_lookup_state(pinctrl_fb, "fb_gpio194_hsync");
	if (IS_ERR(pins_fb_194_hsync)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_194_hsync);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_194_hsync);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_195_vsync = pinctrl_lookup_state(pinctrl_fb, "fb_gpio195_vsync");
	if (IS_ERR(pins_fb_195_vsync)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_195_vsync);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_195_vsync);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_159_vb0 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio159_vb0");
	if (IS_ERR(pins_fb_159_vb0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_159_vb0);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_159_vb0);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_160_vb1 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio160_vb1");
	if (IS_ERR(pins_fb_160_vb1)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_160_vb1);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_160_vb1);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_168_vg0 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio168_vg0");
	if (IS_ERR(pins_fb_168_vg0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_168_vg0);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_168_vg0);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_169_vg1 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio169_vg1");
	if (IS_ERR(pins_fb_169_vg1)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_169_vg1);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_169_vg1);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_186_vr0 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio186_vr0");
	if (IS_ERR(pins_fb_186_vr0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_186_vr0);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_186_vr0);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_187_vr1 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio187_vr1");
	if (IS_ERR(pins_fb_187_vr1)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_187_vr1);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_187_vr1);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_161_vb2 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio161_vb2");
	if (IS_ERR(pins_fb_161_vb2)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_161_vb2);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_161_vb2);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_163_vb3 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio163_vb3");
	if (IS_ERR(pins_fb_163_vb3)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_163_vb3);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_163_vb3);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_164_vb4 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio164_vb4");
	if (IS_ERR(pins_fb_164_vb4)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_164_vb4);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_164_vb4);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_165_vb5 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio165_vb5");
	if (IS_ERR(pins_fb_165_vb5)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_165_vb5);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_165_vb5);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_166_vb6 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio166_vb6");
	if (IS_ERR(pins_fb_166_vb6)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_166_vb6);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_166_vb6);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_167_vb7 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio167_vb7");
	if (IS_ERR(pins_fb_167_vb7)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_167_vb7);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_167_vb7);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_170_vg2 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio170_vg2");
	if (IS_ERR(pins_fb_170_vg2)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_170_vg2);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_170_vg2);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_171_vg3 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio171_vg3");
	if (IS_ERR(pins_fb_171_vg3)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_171_vg3);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_171_vg3);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_172_vg4 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio172_vg4");
	if (IS_ERR(pins_fb_172_vg4)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_172_vg4);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_172_vg4);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_173_vg5 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio173_vg5");
	if (IS_ERR(pins_fb_173_vg5)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_173_vg5);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_173_vg5);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_174_vg6 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio174_vg6");
	if (IS_ERR(pins_fb_174_vg6)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_174_vg6);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_174_vg6);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_175_vg7 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio175_vg7");
	if (IS_ERR(pins_fb_175_vg7)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_175_vg7);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_175_vg7);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_188_vr2 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio188_vr2");
	if (IS_ERR(pins_fb_188_vr2)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_188_vr2);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_188_vr2);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_189_vr3 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio189_vr3");
	if (IS_ERR(pins_fb_189_vr3)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_189_vr3);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_189_vr3);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_190_vr4 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio190_vr4");
	if (IS_ERR(pins_fb_190_vr4)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_190_vr4);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_190_vr4);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_191_vr5 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio191_vr5");
	if (IS_ERR(pins_fb_191_vr5)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_191_vr5);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_191_vr5);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_192_vr6 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio192_vr6");
	if (IS_ERR(pins_fb_192_vr6)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_192_vr6);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_192_vr6);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}
	pins_fb_193_vr7 = pinctrl_lookup_state(pinctrl_fb, "fb_gpio193_vr7");
	if (IS_ERR(pins_fb_193_vr7)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin state err %p\r\n", pins_fb_193_vr7);
		goto err;
	}
	ret = pinctrl_select_state(pinctrl_fb, pins_fb_193_vr7);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "FPD", "vPmxHalSetPinctrl pin select err ret=%d\r\n", ret);
		goto err;
	}

	ret = 0;
err:
	return (__u32)ret;
}
#endif

void vPmxHalInit(void)
{
	__u32 u4RegIdx;

	_prPmxDispMainHwReg = (PMX_HAL_DISP_MAIN_UNION_T *)fmtf_reg;
	_prPmxDispAuxHwReg = (PMX_HAL_DISP_AUX_UNION_T *)fmtr_reg;
	_prPmxMixHwReg = (PMX_HAL_MIX_UNION_T *)mix_reg;

	if (_prPmxDispMainHwReg && _prPmxDispAuxHwReg && _prPmxMixHwReg) {
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "[vPmxHalInit] get reg base sucess 0x%x, 0x%x, 0x%x\r\n"
			, (unsigned int)_prPmxDispMainHwReg, (unsigned int)_prPmxDispAuxHwReg
			, (unsigned int)_prPmxMixHwReg);
	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "[vPmxHalInit] get reg base error 0x%x, 0x%x, 0x%x\r\n"
			, (unsigned int)_prPmxDispMainHwReg, (unsigned int)_prPmxDispAuxHwReg
			, (unsigned int)_prPmxMixHwReg);
		return;
	}

	/* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_MAIN_REG_NUM; u4RegIdx++) {
		_rPmxHalMainSwReg.au4Reg[u4RegIdx] = _prPmxDispMainHwReg->au4Reg[u4RegIdx];
	}

	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_AUX_REG_NUM; u4RegIdx++) {
		_rPmxHalAuxSwReg.au4Reg[u4RegIdx] = _prPmxDispAuxHwReg->au4Reg[u4RegIdx];
	}

	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
		_rPmxHalMixSwReg.au4Reg[u4RegIdx] = _prPmxMixHwReg->au4Reg[u4RegIdx];
	}

	vPmxHalIsrInit();

	/* Create video vsync event*/
	_hVdp1VSyncEvent = x_event_create(NULL, FALSE, FALSE, "Vdp1VSync");

	if (_hVdp1VSyncEvent == NULL) {
		FB_PRINT(FB_LOG_LVL_ERR, "FMT", "ERROR: Vdp1VSync failed create vertical blank event\r\n");
	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "Vdp1VSync: create vertical blank event ok\r\n");
	}

	/* Create video vsync event*/
	_hVdp2VSyncEvent = x_event_create(NULL, FALSE, FALSE, "Vdp2VSync");

	if (_hVdp2VSyncEvent == NULL) {
		FB_PRINT(FB_LOG_LVL_ERR, "FMT", "ERROR: Vdp2VSync failed create vertical blank event\r\n");
	} else {
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "Vdp2VSync: create vertical blank event ok\r\n");
	}
}

void vPmxHalIsrEnable(__u8 ucPmxId)
{
#if 0
	__u32 u4RegIdx;

	if (ucPmxId == PMX_1) {
		if (!_fgPmxMainEnable) {
			if (!BIM_EnableIrq(VECTOR_VSYNC)) {
				ASSERT(0);
			}

			for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_MAIN_REG_NUM; u4RegIdx++) {
				_rPmxDispMainRegMode[u4RegIdx] = PMX_HAL_REG_MODE_NULL;
			}

			for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
				_rPmxMixRegMode[u4RegIdx] = PMX_HAL_REG_MODE_NULL;
			}

			_fgPmxMainEnable = TRUE;
		}
	} else {
		if (!_fgPmxAuxEnable) {
			if (!BIM_EnableIrq(VECTOR_VDOUTREAR)) {
				ASSERT(0);
			}

			for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_AUX_REG_NUM; u4RegIdx++) {
				_rPmxDispAuxRegMode[u4RegIdx] = PMX_HAL_REG_MODE_NULL;
			}

			_fgPmxAuxEnable = TRUE;
		}
	}

	/*if not enabled in FW, set PMX HW to power down mode?*/
#endif
}

void vPmxHalIsrDisable(__u8 ucPmxId)
{
#if 0

	if (ucPmxId == PMX_1) {
		if (_fgPmxMainEnable == TRUE) {
			if (!BIM_DisableIrq(VECTOR_VSYNC)) {
				ASSERT(0);
			}

			_fgPmxMainEnable = FALSE;
		}
	} else {
		if (_fgPmxAuxEnable == TRUE) {
			if (!BIM_DisableIrq(VECTOR_VDOUTREAR)) {
				ASSERT(0);
			}

			_fgPmxAuxEnable = FALSE;
		}
	}

#endif
}

void vPmxHalEnableCb(__u8 ucPmxId, __u8 ucCbType)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.fgCB_ON = 1;
		_rPmxHalMainSwReg.rField.fgCB_TP = ucCbType;
		_rPmxDispMainRegMode[(0xf4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.fgCB_ON = 1;
		_rPmxHalAuxSwReg.rField.fgCB_TP = ucCbType;
		_rPmxDispAuxRegMode[(0xf4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalDisableCb(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.fgCB_ON = 0;
		_rPmxHalMainSwReg.rField.fgCB_TP = 0;
		_rPmxDispMainRegMode[(0xf4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.fgCB_ON = 0;
		_rPmxHalAuxSwReg.rField.fgCB_TP = 0;
		_rPmxDispAuxRegMode[(0xf4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalEnableMute(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 0;
		_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 0;
		_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 0;
		_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 0;
		_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 0;

		_rPmxHalMainSwReg.rField.fgBLACK = 1;
		_rPmxHalMainSwReg.rField.u4BIY = 0x1;
		_rPmxHalMainSwReg.rField.u4BICB = 0x8;
		_rPmxHalMainSwReg.rField.u4BICR = 0x8;

		_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispMainRegMode[(0xb4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.fgBLACK = 1;
		_rPmxHalAuxSwReg.rField.u4BIY = 0x1;
		_rPmxHalAuxSwReg.rField.u4BICB = 0x8;
		_rPmxHalAuxSwReg.rField.u4BICR = 0x8;

		_rPmxDispAuxRegMode[(0xb4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalDisableMute(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 1;
		_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 1;
		_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 1;
		_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 1;
		_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 1;

		_rPmxHalMainSwReg.rField.fgBLACK = 0;

		_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.fgBLACK = 0;

		_rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalSetDelay(__u8 ucPmxId, __u8 ucADJ_F, UINT16 u2HDelay, UINT16 u2VDelay)
{
	if (ucPmxId == PMX_1) { /*VDP_1*/
		_rPmxHalMainSwReg.rField.u4HSYN_DELAY = u2HDelay;
		_rPmxHalMainSwReg.rField.u4VSYN_DELAY = u2VDelay;
		_rPmxHalMainSwReg.rField.fgADJS_F = ucADJ_F;

		_rPmxDispMainRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispMainRegMode[(0xe8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else { /*VDP_2*/
		_rPmxHalAuxSwReg.rField.u4HSYN_DELAY = u2HDelay;
		_rPmxHalAuxSwReg.rField.u4VSYN_DELAY = u2VDelay;
		_rPmxHalAuxSwReg.rField.fgADJS_F = ucADJ_F;

		_rPmxDispAuxRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispAuxRegMode[(0xe8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalRstInVSync(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		_fgPmxMainResetInVSync = TRUE;
	} else {
		_fgPmxAuxResetInVSync = TRUE;
	}
}

void vPmxHalReset(__u8 ucPmxId)
{
	vPmxHalRstInVSync(ucPmxId);
}
EXPORT_SYMBOL(vPmxHalReset);

void vPmxHalSetMasterMode(bool fgEnable)
{
#if MASTER_MODE_ENABLE
	FB_PRINT(FB_LOG_LVL_INFO, "FMT", "vPmxHalSetMasterMode: master mode enable %d\r\n", (int)fgEnable);

	if (fgEnable) { /* Enable master mode for 8317 new feature*/
		_rPmxHalMainSwReg.rField.fgMasterSel = 1;
		_rPmxHalMainSwReg.rField.fgSalveEn = 1;
		_rPmxDispMainRegMode[(0x74 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalMainSwReg.rField.fgMasterSel = 0;
		_rPmxHalMainSwReg.rField.fgSalveEn = 0;
		_rPmxDispMainRegMode[(0x74 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}

#else
	FB_PRINT(FB_LOG_LVL_INFO, "FMT", "vPmxHalSetMasterMode: do not support\r\n");
#endif
}
EXPORT_SYMBOL(vPmxHalSetMasterMode);

void vPmxHalSetMode(__u8 ucPmxId, __u8 ucFmt)
{
	/* special case, don't wait VSYNC, since ISR may NOT happen*/
	FB_PRINT(FB_LOG_LVL_OFF, "FMT", "[TWJ] @@vPmxHalSetMode _u4DispMode,ucFmt,=%d,%d,_u4LCDWidth,_u4LCDHeight =%d,%d,\r\n",
		ucPmxId,ucFmt,_u4LCDWidth,_u4LCDHeight);
	if (ucPmxId == PMX_1) {

		if(ucFmt <= RES_576P){
			_rPmxHalMainSwReg.rField.fgHD_ON = 0;
			_rPmxHalMainSwReg.rField.fgHD_TP = 0;
			_rPmxHalMainSwReg.rField.fgADJ_T = 0;
			_rPmxHalMainSwReg.rField.u4FIRST_PXL_LEAD = 0;
			_rPmxHalMainSwReg.rField.fgPFOFF = 0; //1;
			_rPmxHalMainSwReg.rField.u4PXLLEN = 720;
			_rPmxHalMixSwReg.rField.fgVIDEO_SRC_SEL = 0;
			_rPmxHalMixSwReg.rField.fgOSD_SYNC_FLD_P =1;
			_rPmxHalMixSwReg.rField.fgOSD_SYNC_H_P =1;
			_rPmxHalMixSwReg.rField.fgOSD_SYNC_V_P =1;

		} else {
			_rPmxHalMainSwReg.rField.fgHD_ON = 1;
			_rPmxHalMainSwReg.rField.fgHD_TP = 1;
			_rPmxHalMainSwReg.rField.fgADJ_T = 1;
			_rPmxHalMainSwReg.rField.u4H_TOTAL = g_u4HTotal;
			_rPmxHalMainSwReg.rField.u4V_TOTAL = g_u4VTotal;
			_rPmxHalMainSwReg.rField.u4FIRST_PXL_LEAD = 0;
			_rPmxHalMainSwReg.rField.fgPFOFF = 0;
			_rPmxHalMainSwReg.rField.u4PXLLEN = _u4LCDWidth;//2017
			_rPmxHalMixSwReg.rField.fgVIDEO_SRC_SEL = 1;

		}

		_rPmxDispMainRegMode[(0xc4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispMainRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;

#if MASTER_MODE_ENABLE
		_rPmxHalMixSwReg.rField.fgOSD_SRC_SE = 0;
#else

		if (ucFmt > RES_576P) {
			_rPmxHalMixSwReg.rField.fgOSD_SRC_SE = 1;
		} else {
			_rPmxHalMixSwReg.rField.fgOSD_SRC_SE = 0;
		}

#endif
		_rPmxMixRegMode[(0x0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxMixRegMode[(0x8 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalMainSwReg.au4Reg[(0xe4 / 4)] = 0;
		_rPmxDispMainRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalMainSwReg.rField.u4HSYN_DELAY = 6;
		_rPmxHalMainSwReg.rField.u4VSYN_DELAY = 1;
		_rPmxHalMainSwReg.rField.u4ADJ_H_L = 5;
		_rPmxHalMainSwReg.rField.u4ADJ_V_L = 3;
		_rPmxDispMainRegMode[(0xe8 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		if ((RES_480I == ucFmt) || (RES_576I == ucFmt)) {
			_rPmxHalMainSwReg.rField.fgPRGS = 0;
			_rPmxHalMainSwReg.rField.u4VSYNWIDTH = 8;
		} else {
			_rPmxHalMainSwReg.rField.fgPRGS = 1;
			_rPmxHalMainSwReg.rField.u4VSYNWIDTH = 12; /* u4VSYNWIDTH can not > v active start*/
		}

		_rPmxHalMainSwReg.rField.u4HSYNWIDTH = 32;
		_rPmxDispMainRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalMainSwReg.rField.fgFMTM = 1;
		_rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		switch (ucFmt) {
		case RES_480I:
		case RES_480P:
		case RES_576I:
		case RES_576P:
			_rPmxHalAuxSwReg.rField.fgHD_ON = 0;
			_rPmxHalAuxSwReg.rField.fgHD_TP = 0;
			_rPmxHalAuxSwReg.rField.fgADJ_T = 0;
			_rPmxHalAuxSwReg.rField.fgPFOFF = 1;
			_rPmxHalAuxSwReg.rField.u4FIRST_PXL_LEAD = 0;
			/*_rPmxHalMixSwReg.rField.fgVIDEO_SRC_SEL = 0;*/
			/*_rPmxHalMixSwReg.rField.fgOSD_SRC_SE = 0;*/
			break;

		default:
			FB_PRINT(FB_LOG_LVL_ERR, "FMT", "[vPmxHalSetMode]Aux unknown fmt %d\r\n", ucFmt);
			VERIFY(0);
		}

		_rPmxDispAuxRegMode[(0xc4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxDispAuxRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalAuxSwReg.au4Reg[(0xe4 / 4)] = 0;
		_rPmxDispAuxRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalAuxSwReg.rField.u4HSYN_DELAY = 6;
		_rPmxHalAuxSwReg.rField.u4VSYN_DELAY = 1;
		_rPmxHalAuxSwReg.rField.u4ADJ_H_L = 5;
		_rPmxHalAuxSwReg.rField.u4ADJ_V_L = 3;
		_rPmxDispAuxRegMode[(0xe8 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		if ((RES_480I == ucFmt) || (RES_576I == ucFmt)) {
			_rPmxHalAuxSwReg.rField.fgPRGS = 0;
			_rPmxHalAuxSwReg.rField.u4VSYNWIDTH = 8;
		} else {
			_rPmxHalAuxSwReg.rField.fgPRGS = 1;
			_rPmxHalAuxSwReg.rField.u4VSYNWIDTH = 12; /* u4VSYNWIDTH can not > v active start*/
		}

		_rPmxHalAuxSwReg.rField.u4HSYNWIDTH = 32;
		_rPmxDispAuxRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		_rPmxHalAuxSwReg.rField.fgFMTM = 1;
		_rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalSetTvType(__u8 ucPmxId, __u8 ucTvType)
{
	__u32 u4Tvmode = 0;

	if (ucTvType == PMX_TV_TYPE_NTSC) {
		u4Tvmode = 0;
	} else if (ucTvType == PMX_TV_TYPE_PAL_N) {
		u4Tvmode = 1;
	} else if (ucTvType == PMX_TV_TYPE_PAL_M) {
		u4Tvmode = 2;
	} else if (ucTvType == PMX_TV_TYPE_PAL) {
		u4Tvmode = 3;
	}

	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.u4TVMODE = u4Tvmode;
		_rPmxDispMainRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.u4TVMODE = u4Tvmode;
		_rPmxDispAuxRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalSetDigitalOut(__u8 ucPmxId, __u8 ucEnable, __u8 ucProprietaryMode, __u8 ucDigitalBit)
{
}

void vPmxHalSetDigitalOutPhase(__u8 ucPmxId, __u32 u4Phase)
{
}

void vPmxHalSetDigitalOutDrv(__u32 u4Driving)
{
}

void vPmxHalSetDigitalOutRgb(__u8 ucPmxId, __u8 ucRbg)
{
}

void vPmxHalSetBg(__u8 ucPmxId, __u32 u4Bg)
{
	/*HW support only 1 set of build-in color(background color)*/
	if (ucPmxId == PMX_1) {

		_prPmxDispMainHwReg->rField.u4BGY  = (u4Bg & 0x0000F0) >> 4;
		_prPmxDispMainHwReg->rField.u4BGCB = (u4Bg & 0x00F000) >> 12;
		_prPmxDispMainHwReg->rField.u4BGCR = (u4Bg & 0xF00000) >> 20;
		_rPmxHalMainSwReg.au4Reg[(0xb8 / 4)] = _prPmxDispMainHwReg->au4Reg[(0xb8 / 4)];
	} else {
		_prPmxDispAuxHwReg->rField.u4BGY  = (u4Bg & 0x0000F0) >> 4;
		_prPmxDispAuxHwReg->rField.u4BGCB = (u4Bg & 0x00F000) >> 12;
		_prPmxDispAuxHwReg->rField.u4BGCR = (u4Bg & 0xF00000) >> 20;
		_rPmxHalAuxSwReg.au4Reg[(0xb8 / 4)] = _prPmxDispAuxHwReg->au4Reg[(0xb8 / 4)];
	}
}

void vPmxHalSetBi(__u8 ucPmxId, __u32 u4Bi)
{

	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.u4BICR = (u4Bi & 0x0000F0) >> 4;
		_rPmxHalMainSwReg.rField.u4BICB = (u4Bi & 0x00F000) >> 12;
		_rPmxHalMainSwReg.rField.u4BIY  = (u4Bi & 0xF00000) >> 20;
		_rPmxDispMainRegMode[(0xb4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.u4BICR = (u4Bi & 0x0000F0) >> 4;
		_rPmxHalAuxSwReg.rField.u4BICB = (u4Bi & 0x00F000) >> 12;
		_rPmxHalAuxSwReg.rField.u4BIY  = (u4Bi & 0xF00000) >> 20;
		_rPmxDispAuxRegMode[(0xb4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalSetVHTotal(__u8 ucPmxId, bool fgAdjustOn, __u32 u4HTotal, __u32 u4VTotal)
{
	/*MT8520 HW support only 1 set of build-in color(background color)*/
	if (ucPmxId == PMX_1) {
		if ((u4HTotal != 0) && (u4VTotal != 0)) {
			g_u4HTotal = u4HTotal;
			g_u4VTotal = u4VTotal;
			_rPmxHalMainSwReg.rField.fgADJ_T = fgAdjustOn;
			_rPmxHalMainSwReg.rField.u4H_TOTAL = u4HTotal;
			_rPmxHalMainSwReg.rField.u4V_TOTAL = u4VTotal;
			_rPmxDispMainRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		} else { /*Get scl HV total for HD source*/
			g_u4HTotal = (*(volatile unsigned int *)0xFD0A468c & 0x0FFF0000) >> 16;
			g_u4VTotal = (*(volatile unsigned int *)0xFD0A468c & 0x00000FFF);
		}

		FB_PRINT(FB_LOG_LVL_INFO, "FMT", "g_u4HTotal:0x%x, g_u4VTotal:0x%x \r\n", (unsigned int)g_u4HTotal
			, (unsigned int)g_u4VTotal);
	} else {
		_rPmxHalAuxSwReg.rField.fgADJ_T = fgAdjustOn;
		_rPmxHalAuxSwReg.rField.u4H_TOTAL = u4HTotal;
		_rPmxHalAuxSwReg.rField.u4V_TOTAL = u4VTotal;
		_rPmxDispAuxRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalSetBrightness(__u8 ucPmxId, __u8 ucBrightness)
{
}

void vPmxHalSetContrast(__u8 ucPmxId, __u8 ucContrast)
{
}

void vPmxHalSetGamma(__u8 ucPmxId, __u8 ucEnable, const __u8 *pucCurve)
{
}

void vPmxHalSetAlpha(__u8 ucPmxId, __u32 ucInAlpha, __u32 ucOutAlpha, bool fgEnable)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMixSwReg.rField.fgVIDEO_A_ADJ = fgEnable;
		_rPmxHalMixSwReg.rField.u4VIDEO_A_IN_RANGE = ucInAlpha;
		_rPmxHalMixSwReg.rField.u4VIDEO_A_OUT_RANGE = ucOutAlpha;
		_rPmxMixRegMode[0x0] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxMixRegMode[(0x04 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
	FB_PRINT(FB_LOG_LVL_INFO, "PMX", "PmxHalMixPlaneis %d\r\n",(int)u4Plane);

/*#ifdef CONFIG_ATC_OS_linux
	if(u4Plane != PMX_HW_PLANE_1) {
		_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN =_prPmxMixHwReg->rField.fgVIDEO_MIX_EN;
	}
#endif*/

	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
			if (!_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN) {
				_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 1;
				_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
				_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			}
			break;

		case PMX_HW_PLANE_3:
			_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 1;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;

		case PMX_HW_PLANE_4:
			_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 1;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;

		case PMX_HW_PLANE_5:
			_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 1;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;

		case PMX_HW_PLANE_6:
			_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 1;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;

		default:
			break;
		}
	}
}
EXPORT_SYMBOL(vPmxHalMixPlane);

void vPmxHalNotMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
	FB_PRINT(FB_LOG_LVL_INFO, "PMX", "PmxHalNotMixPlane is %d \r\n",u4Plane);

/*#ifdef CONFIG_ATC_OS_linux
	if(u4Plane != PMX_HW_PLANE_1) {
		_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN =_prPmxMixHwReg->rField.fgVIDEO_MIX_EN;
	}
#endif*/
	/*for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
		 _rPmxHalMixSwReg.au4Reg[u4RegIdx]=_prPmxMixHwReg->au4Reg[u4RegIdx];
	}*/

	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
			if (_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN) {
				_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 0;
				_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			}
		break;
	
		case PMX_HW_PLANE_3:
			_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 0;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
	
		case PMX_HW_PLANE_4:
			_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 0;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
	
		case PMX_HW_PLANE_5:
			_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 0;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
	
		case PMX_HW_PLANE_6:
			_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 0;
			_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;

		default:
			break;
		}
	}

/*	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
			_prPmxMixHwReg->rField.fgVIDEO_MIX_EN = 0;
			_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = _prPmxMixHwReg->rField.fgVIDEO_MIX_EN;
			break;

		case PMX_HW_PLANE_3:
			_prPmxMixHwReg->rField.fgOSD1_MIX_EN = 0;
			_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = _prPmxMixHwReg->rField.fgOSD1_MIX_EN;
			break;

		case PMX_HW_PLANE_4:
			_prPmxMixHwReg->rField.fgOSD2_MIX_EN = 0;
			_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = _prPmxMixHwReg->rField.fgOSD2_MIX_EN;
			break;

		case PMX_HW_PLANE_5:
			_prPmxMixHwReg->rField.fgOSD3_MIX_EN = 0;
			_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = _prPmxMixHwReg->rField.fgOSD3_MIX_EN;
			break;

		case PMX_HW_PLANE_6:
			_prPmxMixHwReg->rField.fgOSD4_MIX_EN = 0;
			_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = _prPmxMixHwReg->rField.fgOSD4_MIX_EN;
			break;

		default:
			break;
		}
	}*/
}
EXPORT_SYMBOL(vPmxHalNotMixPlane);

void vPmxHalNotMixPlaneDelay(__u8 ucPmxId, __u32 u4Plane)
{
	FB_PRINT(FB_LOG_LVL_INFO, "PMX", "vPmxHalNotMixPlaneDelay is %d \r\n",u4Plane);

	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
			_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 0;
			_rPmxMixRegMode[1] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxMixRegMode[1] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			break;

		case PMX_HW_PLANE_3:
			_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 0;
			_rPmxMixRegMode[2] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxMixRegMode[2] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			break;

		case PMX_HW_PLANE_4:
			_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 0;
			_rPmxMixRegMode[3] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxMixRegMode[3] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			break;

		case PMX_HW_PLANE_5:
			_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 0;
			_rPmxMixRegMode[4] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxMixRegMode[4] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			break;

		case PMX_HW_PLANE_6:
			_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 0;
			_rPmxMixRegMode[5] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxMixRegMode[5] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			break;

		default:
			break;
		}
	}
}
EXPORT_SYMBOL(vPmxHalNotMixPlaneDelay);


bool fgPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
	bool fgMixPlane = FALSE;

	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
#ifdef CONFIG_ATC_OS_linux
			if(_rPmxMixRegMode[(0/4)] > 0) {
				/*Pmx write mode*/
				if(_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN == FALSE) {
					fgMixPlane = FALSE;
					break;
				}
			}
			fgMixPlane = (_prPmxMixHwReg->rField.fgVIDEO_MIX_EN == 1) ? TRUE : FALSE;
#else
			fgMixPlane = (_prPmxMixHwReg->rField.fgVIDEO_MIX_EN == 1) ? TRUE : FALSE;
#endif 
			break;

		case PMX_HW_PLANE_3:
			fgMixPlane = (_prPmxMixHwReg->rField.fgOSD1_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_4:
			fgMixPlane = (_prPmxMixHwReg->rField.fgOSD2_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_5:
			fgMixPlane = (_prPmxMixHwReg->rField.fgOSD3_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_6:
			fgMixPlane = (_prPmxMixHwReg->rField.fgOSD4_MIX_EN == 1) ? TRUE : FALSE;
			break;

		default:
			break;
		}
	}

	return fgMixPlane;
}

void vPmxHalSetAbleToFlipOSD(__u8 bIsAbleToFlip)
{
}

void vPmxHalSetCc(__u8 ucPmxId, __u8 ucEnable)
{
}

void vPmxHalSetFullRange(__u8 ucPmxId, bool fgEnable, bool fg2352255)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.fgEn235_255 = fgEnable;
		_rPmxHalMainSwReg.rField.fgC_235_2_255 = fg2352255;
		_rPmxDispMainRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		if (fg2352255) { /* limit < 16 or > 235 value*/
			_rPmxHalMainSwReg.rField.u4Y_LMT_BOT = 16;
			_rPmxHalMainSwReg.rField.u4Y_LMT_TOP = 235;
			_rPmxDispMainRegMode[(0xC4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxHalMainSwReg.rField.fgYLMT_B = 1;
			_rPmxHalMainSwReg.rField.fgYLMT_T = 1;
			_rPmxDispMainRegMode[(0xAC / 4)] |= PMX_HAL_REG_MODE_WRITE;
		}
	} else {
		_rPmxHalAuxSwReg.rField.fgEn235_255 = fgEnable;
		_rPmxHalAuxSwReg.rField.fgC_235_2_255 = fg2352255;
		_rPmxDispAuxRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;

		if (fg2352255) {
			_rPmxHalAuxSwReg.rField.u4Y_LMT_BOT = 16;
			_rPmxHalAuxSwReg.rField.u4Y_LMT_TOP = 235;
			_rPmxDispAuxRegMode[(0xC4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			_rPmxHalAuxSwReg.rField.fgYLMT_B = 1;
			_rPmxHalAuxSwReg.rField.fgYLMT_T = 1;
			_rPmxDispAuxRegMode[(0xAC / 4)] |= PMX_HAL_REG_MODE_WRITE;
		}
	}
}

void vPmxHalSet709To601(__u8 ucPmxId, bool fgEnable, bool fg7092601)
{
	if (ucPmxId == PMX_1) {
		_rPmxHalMainSwReg.rField.fgEn_601_709 = fgEnable;
		_rPmxHalMainSwReg.rField.fgC_709_2_601 = fg7092601;
		_rPmxDispMainRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		_rPmxHalAuxSwReg.rField.fgEn_601_709 = fgEnable;
		_rPmxHalAuxSwReg.rField.fgC_709_2_601 = fg7092601;
		_rPmxDispAuxRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

/******************************************************************************
* Local Function
******************************************************************************/
__u32 _u4ByPassInterrupt = FALSE;
#define MSG_COMBINE(module, id) ((module<<24)|(id))

#ifndef __ARM2__
void vPmxHalWakeUpThread(void)
{
	atomic_inc(&workthread_wakeup);
    wake_up_interruptible(&wq_workthread);
}
#endif

irqreturn_t vPmxHalMainIsr(int u2Vector, void *dev_id)
{
	__u32 u4RegIdx;

	if (_u4ByPassInterrupt) {
		return IRQ_HANDLED;
	}
#ifndef __ARM2__
	vPmxHalWakeUpThread();
#endif

	FB_PRINT(FB_LOG_LVL_IRQ, "FMT", "PMX main irq isr %d\r\n", u2Vector);

	/* call video plane isr*/
	vVdpIsr(VDP_1);

	if (_fgPmxMainResetInVSync == TRUE) {
		_prPmxDispMainHwReg->rField.fgFTRST = 0;
		_prPmxDispMainHwReg->rField.fgFTRST = 1;
		_prPmxDispMainHwReg->rField.fgFTRST = 0;

		_fgPmxMainResetInVSync = FALSE;
	}

	/* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_MAIN_REG_NUM; u4RegIdx++) {
		if (_rPmxDispMainRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
			_prPmxDispMainHwReg->au4Reg[u4RegIdx] = _rPmxHalMainSwReg.au4Reg[u4RegIdx];
			_rPmxDispMainRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		if (_rPmxDispMainRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			_rPmxHalMainSwReg.au4Reg[u4RegIdx] = _prPmxDispMainHwReg->au4Reg[u4RegIdx];
			_rPmxDispMainRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_READ;
		}

#if !(PMX_HAL_REG_MODE_DELAY_WRITE == PMX_HAL_REG_MODE_WRITE)

		if (_rPmxDispMainRegMode[u4RegIdx] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			_rPmxDispMainRegMode[u4RegIdx] = _rPmxDispMainRegMode[u4RegIdx] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "FMT", "FMTF delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxDispMainRegMode[u4RegIdx], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalMainSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxDispMainHwReg->au4Reg[u4RegIdx]);
		}

#endif
	}

	if (fgWaitVdp1VSync) {
		x_event_set(_hVdp1VSyncEvent);
		fgWaitVdp1VSync = FALSE;
	}

	ac83xx_mask_ack_bim_irq(u2Vector);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(vPmxHalMainIsr);


irqreturn_t vPmxHalAuxIsr(int u2Vector, void *dev_id)
{
	__u32 u4RegIdx;

	FB_PRINT(FB_LOG_LVL_IRQ, "FMT", "PMX aux irq isr %d\r\n", u2Vector);

	/* call video plane isr*/
	vVdpIsr(VDP_2);

	if (_fgPmxAuxResetInVSync == TRUE) {
		_prPmxDispAuxHwReg->rField.fgFTRST = 0;
		_prPmxDispAuxHwReg->rField.fgFTRST = 1;
		_prPmxDispAuxHwReg->rField.fgFTRST = 0;

		_fgPmxAuxResetInVSync = FALSE;
	}

	/* update plane mixer register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_AUX_REG_NUM; u4RegIdx++) {
		if (_rPmxDispAuxRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
			_prPmxDispAuxHwReg->au4Reg[u4RegIdx] = _rPmxHalAuxSwReg.au4Reg[u4RegIdx];
			_rPmxDispAuxRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		if (_rPmxDispAuxRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			_rPmxHalAuxSwReg.au4Reg[u4RegIdx] = _prPmxDispAuxHwReg->au4Reg[u4RegIdx];
			_rPmxDispAuxRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_READ;
		}

#if !(PMX_HAL_REG_MODE_DELAY_WRITE == PMX_HAL_REG_MODE_WRITE)

		if (_rPmxDispAuxRegMode[u4RegIdx] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			_rPmxDispAuxRegMode[u4RegIdx] = _rPmxDispAuxRegMode[u4RegIdx] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "FMT", "FMTR delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxDispAuxRegMode[u4RegIdx], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalAuxSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxDispAuxHwReg->au4Reg[u4RegIdx]);
		}

#endif
	}

	if (fgWaitVdp2VSync) {
		x_event_set(_hVdp2VSyncEvent);
		fgWaitVdp2VSync = FALSE;
	}

	/* update osd register at vsync*/
	ac83xx_mask_ack_bim_irq(u2Vector);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(vPmxHalAuxIsr);

#ifndef __ARM2__
static int vPmxHalIsrSend2Arm2(void *arg)
{
	while (1) {
        wait_event_interruptible(wq_workthread, atomic_read(&workthread_wakeup));
        atomic_dec(&workthread_wakeup);
        HWSendMessage(MSG_COMBINE(3 , 5), 0, 0, 0);
    }
	return 0;
}
#endif


void vPmxHalIsrInit(void)
{
	if (!_fgPmxMainIsrInited) {
#ifndef __ARM2__
		static struct task_struct *ts1;
		init_waitqueue_head(&wq_workthread);
		atomic_set(&workthread_wakeup, 0);
		FB_PRINT(FB_LOG_LVL_INFO, "FMT", "wq_workthread: 0x%px\n", &wq_workthread);

		ts1 = kthread_create(vPmxHalIsrSend2Arm2, NULL, "vPmxHalIsrSend2Arm2");
	    if (IS_ERR(ts1)) {
	        FB_PRINT(FB_LOG_LVL_ERR, "FMT", "vPmxHalIsrSend2Arm2 kthread_create error\r\n");
	        return;
	    }
		wake_up_process(ts1);

		if (request_irq(fmtf_irq, vPmxHalMainIsr, 0, "FMTF_VSYNC", (void *)NULL) != OSR_OK) {
			FB_PRINT(FB_LOG_LVL_ERR, "FMT", "vPmxHalIsrInit fmtf irq reigster error\r\n");
			return;
		}

#endif
		/* init local variable*/
		_fgPmxMainIsrInited = TRUE;
	}

	if (!_fgPmxAuxIsrInited) {
#ifndef __ARM2__
		if (request_irq(fmtr_irq, vPmxHalAuxIsr, 0, "FMTR_VSYNC", (void *)NULL) != OSR_OK) {
			FB_PRINT(FB_LOG_LVL_ERR, "FMT", "vPmxHalIsrInit fmtr irq reigster error\r\n");
			return;
		}
		FB_PRINT(FB_LOG_LVL_DBG, "FMT", "PMX irq reigster %d %d\r\n", fmtf_irq, fmtr_irq);
#endif
		
		/* init local variable*/
		_fgPmxAuxIsrInited = TRUE;
	}
}

void vPmxHalIsrStop(__u8 ucPmxId)
{
	if (_fgPmxMainIsrInited == TRUE) {
#ifndef __ARM2__
		free_irq(fmtf_irq, NULL);
#endif
		_fgPmxMainIsrInited = FALSE;
	}

	if (_fgPmxAuxIsrInited == TRUE) {
#ifndef __ARM2__
		free_irq(fmtr_irq, NULL);
#endif
		_fgPmxAuxIsrInited = FALSE;
	}
}
EXPORT_SYMBOL(vPmxHalIsrStop);

void vPmxHalEnableFmt(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		/*reset vdout fmt first*/
		_rPmxHalMainSwReg.rField.fgVDO_EN = 1;
		_rPmxHalMainSwReg.rField.fgFTRST = 1;
		/* FTRST is sw reset and write only register, so need read back after setting*/
		_rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_READ | PMX_HAL_REG_MODE_WRITE;
	} else {
		/*reset vdout fmt first*/
		_rPmxHalAuxSwReg.rField.fgVDO_EN = 1;
		_rPmxHalAuxSwReg.rField.fgFTRST = 1;
		/* FTRST is sw reset and write only register, so need read back after setting*/
		_rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_READ | PMX_HAL_REG_MODE_WRITE;
	}
}
EXPORT_SYMBOL(vPmxHalEnableFmt);


void vPmxHalDisableFmt(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		//_prPmxDispMainHwReg->rField.fgVDO_EN = 0;
		//_rPmxHalMainSwReg.rField.fgVDO_EN = _prPmxDispMainHwReg->rField.fgVDO_EN;
		_rPmxHalMainSwReg.rField.fgVDO_EN = 0;
		_rPmxDispMainRegMode[(0xac / 4)] = PMX_HAL_REG_MODE_WRITE;
	} else {
		_prPmxDispAuxHwReg->rField.fgVDO_EN = 0;
		_rPmxHalAuxSwReg.rField.fgVDO_EN = _prPmxDispAuxHwReg->rField.fgVDO_EN;
	}
}
EXPORT_SYMBOL(vPmxHalDisableFmt);

bool vPmxHalGetFmtEn(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		return _prPmxDispMainHwReg->rField.fgVDO_EN;
	} else {
		return _prPmxDispAuxHwReg->rField.fgVDO_EN;
	}
}

void vPmxHalSetPlaneOrder(__u8 ucPmxId, __u32 u4PlaneOrder)
{
	FB_PRINT(FB_LOG_LVL_INFO, "PMX", "vPmxHalSetPlaneOrder: id = %d, order = %x\r\n", ucPmxId, (unsigned int)u4PlaneOrder);

	if (ucPmxId == PMX_1) {
		_rPmxHalMixSwReg.rField.u4MIX_LAYER0_SEL = u4PlaneOrder & 0x7;
		_rPmxHalMixSwReg.rField.u4MIX_LAYER1_SEL = (u4PlaneOrder & 0x70) >> 4;
		_rPmxHalMixSwReg.rField.u4MIX_LAYER2_SEL = (u4PlaneOrder & 0x700) >> 8;
		_rPmxHalMixSwReg.rField.u4MIX_LAYER3_SEL = (u4PlaneOrder & 0x7000) >> 12;
		_rPmxHalMixSwReg.rField.u4MIX_LAYER4_SEL = (u4PlaneOrder & 0x70000) >> 16;

		_rPmxHalMixSwReg.rField.fgDST_SEL_1 = (u4PlaneOrder & 0x80) >> 7;
		_rPmxHalMixSwReg.rField.fgDST_SEL_2 = (u4PlaneOrder & 0x800) >> 11;
		_rPmxHalMixSwReg.rField.fgDST_SEL_3 = (u4PlaneOrder & 0x8000) >> 15;
		_rPmxHalMixSwReg.rField.fgDST_SEL_4 = (u4PlaneOrder & 0x80000) >> 19;
		_rPmxMixRegMode[(0x50 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}
EXPORT_SYMBOL(vPmxHalSetPlaneOrder);

__u32 vPmxHalGetPlaneOrder(__u8 ucPmxId)
{
	__u32 u4PlaneOrder = 0;

	if (ucPmxId == PMX_1) {
		u4PlaneOrder |= _prPmxMixHwReg->rField.u4MIX_LAYER0_SEL;
		u4PlaneOrder |= _prPmxMixHwReg->rField.u4MIX_LAYER1_SEL << 4;
		u4PlaneOrder |= _prPmxMixHwReg->rField.u4MIX_LAYER2_SEL << 8;
		u4PlaneOrder |= _prPmxMixHwReg->rField.u4MIX_LAYER3_SEL << 12;
		u4PlaneOrder |= _prPmxMixHwReg->rField.u4MIX_LAYER4_SEL << 16;

		u4PlaneOrder |= _prPmxMixHwReg->rField.fgDST_SEL_1 << 7;
		u4PlaneOrder |= _prPmxMixHwReg->rField.fgDST_SEL_2 << 11;
		u4PlaneOrder |= _prPmxMixHwReg->rField.fgDST_SEL_3 << 15;
		u4PlaneOrder |= _prPmxMixHwReg->rField.fgDST_SEL_4 << 19;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "PMX", "vPmxHalGetPlaneOrder: id = %d, order = %x\r\n", ucPmxId, (unsigned int)u4PlaneOrder);

	return u4PlaneOrder;
}
EXPORT_SYMBOL(vPmxHalGetPlaneOrder);

void vPmxHalSetPlaneDstColorKey(__u8  ucPmxId, bool fgEnable)
{
	if (_rPmxHalMixSwReg.rField.u4MIX_LAYER1_SEL == ucPmxId) {
		_rPmxHalMixSwReg.rField.fgDST_SEL_1 = fgEnable ? 1 : 0;
	} else if (_rPmxHalMixSwReg.rField.u4MIX_LAYER2_SEL == ucPmxId) {
		_rPmxHalMixSwReg.rField.fgDST_SEL_2 = fgEnable ? 1 : 0;
	} else if (_rPmxHalMixSwReg.rField.u4MIX_LAYER3_SEL == ucPmxId) {
		_rPmxHalMixSwReg.rField.fgDST_SEL_3 = fgEnable ? 1 : 0;
	} else if (_rPmxHalMixSwReg.rField.u4MIX_LAYER4_SEL == ucPmxId) {
		_rPmxHalMixSwReg.rField.fgDST_SEL_4 = fgEnable ? 1 : 0;
	}

	_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vPmxHalEnableWaitVSync(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		fgWaitVdp1VSync = TRUE;
	} else {
		fgWaitVdp2VSync = TRUE;
	}
}

__u32 dwPmxHalWaitVSync(__u8 ucPmxId)
{
	__u32 ret = 0;
	HANDLE hld = NULL;

	if (ucPmxId == PMX_1) {
		hld = _hVdp1VSyncEvent;
	} else {
		hld = _hVdp2VSyncEvent;
	}
	ret = x_event_wait_for_objects(1, &hld, FALSE, 0xFFFFFFFFU);

	return ret;
}
void vPmxHalResume(__u8 ucPmxId)
{
	__u32 u4RegIdx = 0;

	if (ucPmxId == PMX_1) {
		vPmxVerifyHalSysInit();

		for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_MAIN_REG_NUM; u4RegIdx++) {
			_prPmxDispMainHwReg->au4Reg[u4RegIdx] = _rPmxHalMainSwReg.au4Reg[u4RegIdx];
			_rPmxDispMainRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
			_prPmxMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx];
			_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		_prPmxDispMainHwReg->rField.fgFTRST = 0;
		_prPmxDispMainHwReg->rField.fgFTRST = 1;
		_prPmxDispMainHwReg->rField.fgFTRST = 0;
	} else {
		for (u4RegIdx = 0; u4RegIdx < PMX_HAL_DISP_AUX_REG_NUM; u4RegIdx++) {
			_prPmxDispAuxHwReg->au4Reg[u4RegIdx] = _rPmxHalAuxSwReg.au4Reg[u4RegIdx];
			_rPmxDispAuxRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		_prPmxDispAuxHwReg->rField.fgFTRST = 0;
		_prPmxDispAuxHwReg->rField.fgFTRST = 1;
		_prPmxDispAuxHwReg->rField.fgFTRST = 0;
	}

	vVdpHalResume(ucPmxId);
}

void vPmxHalMixIsr(void)
{
	__u32 u4RegIdx;
	__u32 tmp = 0;

	/* update plane mixer vdout register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
		if ((_rPmxMixRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE)) {
			if ((u4RegIdx == 0) && (_rPmxMixRegMode[u4RegIdx] >> 2)) {
				/* Don't enable video layer and primary surface if delay write set*/
				/*tmp = (1 << PRIMARY_SURF_PLANE) | 0x3;*/
				tmp = 0x3;
				_prPmxMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx] & ~tmp;
				_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
			} else if ((u4RegIdx == 2) && (_rPmxMixRegMode[2] >> 2)) {
				/* Don't disable osd1 layer if delay write set*/
				tmp = 0x4;
				_prPmxMixHwReg->au4Reg[0] = _rPmxHalMixSwReg.au4Reg[0] | tmp;
				_rPmxMixRegMode[2] &= ~PMX_HAL_REG_MODE_WRITE;
			} else if ((u4RegIdx == 1) && (_rPmxMixRegMode[1] >> 2)) {
				/* Don't disable video layer if delay write set*/
				tmp = 0x2;
				_prPmxMixHwReg->au4Reg[0] = _rPmxHalMixSwReg.au4Reg[0] | tmp;
				_rPmxMixRegMode[1] &= ~PMX_HAL_REG_MODE_WRITE;
			} else {
				_prPmxMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx];
				_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
			}
		}

		if (_rPmxMixRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			_rPmxHalMixSwReg.au4Reg[u4RegIdx] = _prPmxMixHwReg->au4Reg[u4RegIdx];
			_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_READ;
		}

#if !(PMX_HAL_REG_MODE_DELAY_WRITE == PMX_HAL_REG_MODE_WRITE)

		if (_rPmxMixRegMode[u4RegIdx] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			_rPmxMixRegMode[u4RegIdx] = _rPmxMixRegMode[u4RegIdx] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "PMX", "MIX delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxMixRegMode[u4RegIdx], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalMixSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxMixHwReg->au4Reg[u4RegIdx]);
		} else if (_rPmxMixRegMode[2] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			//_rPmxMixRegMode[1] = _rPmxMixRegMode[1] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "PMX", "MIX osd1 delay disable write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxMixRegMode[2], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalMixSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxMixHwReg->au4Reg[u4RegIdx]);
		} else if (_rPmxMixRegMode[1] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			//_rPmxMixRegMode[1] = _rPmxMixRegMode[1] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "PMX", "MIX video disable delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxMixRegMode[1], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalMixSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxMixHwReg->au4Reg[u4RegIdx]);
		}

#endif
	}
}

void vPmxHalLayerBgEn(__u32 u4Bg)
{
	_rPmxHalMixSwReg.rField.u4LAYER0_OFF_CR  = u4Bg & 0x0000FF;
	_rPmxHalMixSwReg.rField.u4LAYER0_OFF_CB = (u4Bg & 0x00FF00) >> 8;
	_rPmxHalMixSwReg.rField.u4LAYER0_OFF_Y = (u4Bg & 0xFF0000) >> 16;
	_rPmxHalMixSwReg.rField.fgMIX_LAYER0_En = 0;

	_rPmxMixRegMode[(0x48 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	_rPmxMixRegMode[(0x50 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vPmxHalLayerBgDis(void)
{
	_rPmxHalMixSwReg.rField.fgMIX_LAYER0_En = 1;

	_rPmxMixRegMode[(0x50 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}
#endif /* _PMX_VSYNC_C_ */



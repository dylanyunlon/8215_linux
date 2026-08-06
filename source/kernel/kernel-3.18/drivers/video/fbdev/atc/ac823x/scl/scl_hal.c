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
*  Panel Scaler: HAL
*****************************************************************************/
#ifndef _SCL_HAL_C_
#define _SCL_HAL_C_

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/interrupt.h>
#include <media/atc/ac823x/pmx_hal.h>
#include <media/atc/display_inc.h>
#include <media/atc/display.h>
#include "x_debug.h"
#include "winutil.h"
#include "windows.h"
#include "x_ioopt.h"
#else
#include "x_types.h"
#include "pmx_hal.h"
#include "display_inc.h"
#endif
#include "scl_table.h"
#include "scl_hal.h"
#include "scl_hw.h"
#include "scl_if.h"
#include "pmx_hw.h"
#include "log.h"
#include "x_bim.h"
#include "irqs_vector.h"
#include "x_os.h"

static volatile SCL_HAL_SCL_UNION_T *_prSclHwReg;
static volatile SCL_HAL_FMT_UNION_T *_prSclFmtHwReg;

volatile SCL_HAL_SCL_UNION_T _rSclHalSclReg;
volatile SCL_HAL_FMT_UNION_T _rSclHalFmtReg;

__u8 _rSclRegMode[SCL_HAL_SCL_REG_NUM];
__u8 _rSclFmtRegMode[SCL_HAL_FMT_REG_NUM];

static bool _fgSclIsrInited = FALSE;
static bool _fgSclSwitchSrc = FALSE;
/*static bool _fgIsFromDVD = FALSE;*/
static __u32 _u4InputRes;
static __u32 _u4OutputRes;
static __u32 _u4SrcWidth;
static __u32 _u4SrcHeight;
static __u32 _u4DstWidth;
static __u32 _u4DstHeight;

static HANDLE _hVBEvent;     /* Use for wait for VBI*/
bool _fgWaitVBIEvent = FALSE;

void vSclHalInit(bool fgHwReset)
{
#ifndef __ARM2__
	__u32 u4RegIdx;

	_prSclHwReg = (SCL_HAL_SCL_UNION_T *)scl_reg;
	_prSclFmtHwReg = (SCL_HAL_FMT_UNION_T *)sclf_reg;

	if (_prSclHwReg && _prSclFmtHwReg) {
		VDO_LOG(VDO_LOG_LVL_DBG, "[vSclHalInit] get reg base sucess 0x%lx, 0x%lx\r\n",
			(unsigned long)_prSclHwReg, (unsigned long)_prSclFmtHwReg);
	} else {
		VDO_LOG(VDO_LOG_LVL_ERR, "[vSclHalInit] get reg base error 0x%lx, 0x%lx\r\n",
			(unsigned long)_prSclHwReg, (unsigned long)_prSclFmtHwReg);
		return;
	}

	if (fgHwReset) {
		_prSclHwReg->rField.u4SCLRST = 0xFF;
		_prSclHwReg->rField.u4SCLRST = 0;
	}

	/* update panel scaler register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < SCL_HAL_SCL_REG_NUM; u4RegIdx++) {
		_rSclHalSclReg.au4Reg[u4RegIdx] = _prSclHwReg->au4Reg[u4RegIdx];
	}

	for (u4RegIdx = 0; u4RegIdx < SCL_HAL_FMT_REG_NUM; u4RegIdx++) {
		_rSclHalFmtReg.au4Reg[u4RegIdx] = _prSclFmtHwReg->au4Reg[u4RegIdx];
	}

	vSclHalIsrInit();

	/* Create vertical blank event*/
	//_hVBEvent = x_event_create(NULL, FALSE, FALSE, "VerticalBlank");//for 8237

	if (_hVBEvent == NULL) {
		VDO_LOG(VDO_LOG_LVL_ERR, "ERROR: vSclHalInit failed create vertical blank event\r\n");
	} else {
		VDO_LOG(VDO_LOG_LVL_DBG, "vSclHalInit: create vertical blank event ok\r\n");
	}

	_fgSclSwitchSrc = FALSE;
	_fgWaitVBIEvent = FALSE;
#endif
}


void vSclHalIsrInit(void)
{
	if (_fgSclIsrInited == FALSE) {
#ifndef __ARM2__
		/*register VECTOR_PANEL_SCALER interrupt.*/
		if (request_irq(scl_irq, vSclHalIsr, 0, "SCL_VSYNC", (void *)NULL) != OSR_OK) {
			VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] scaler irq reigster error\r\n");
			return;
		}
		VDO_LOG(VDO_LOG_LVL_INFO, "[SCL] scaler irq reigster %d\r\n", scl_irq);
#endif
		_fgSclIsrInited = TRUE;
	}
}

void vSclHalIsrStop(__u8 ucPmxId)
{
	if (_fgSclIsrInited == TRUE) {
#ifndef __ARM2__
		free_irq(scl_irq, NULL);
#endif
		_fgSclIsrInited = FALSE;
	}
}

/* **********************************************************************/
/* Function : void vSclReset(void)*/
/* Description : Reset Panel Scaler*/
/* Parameter : None*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalReset(void)
{
	_prSclHwReg->rField.u4SCLRST = 0xFF;
	_prSclHwReg->rField.u4SCLRST = 0;
}

void vSclHalRstTiming(void)
{
	/*Panel FMT timing reset*/
	_prSclFmtHwReg->rField.fgRST_MD = 1;
	_prSclFmtHwReg->rField.fgRSTTRIG = 1;
}

void vSclHalSetWinActive(__u32 u4Top, __u32 u4Bottom, __u32 u4Left, __u32 u4Right)
{
	/*Set window mode active start and active end*/ /* Fix video the left pixel issue*/
	_rSclHalFmtReg.rField.u4XWINSTART = _rSclHalFmtReg.rField.u4XACTSTART + u4Left - 1;
	_rSclHalFmtReg.rField.u4XWINEND = _rSclHalFmtReg.rField.u4XACTSTART + u4Right - 1;

	_rSclHalFmtReg.rField.u4YOWINSTART = _rSclHalFmtReg.rField.u4YOACTSTART + u4Top;
	_rSclHalFmtReg.rField.u4YOWINEND = _rSclHalFmtReg.rField.u4YOACTSTART + u4Bottom - 1;
	_rSclHalFmtReg.rField.u4YEWINSTART = _rSclHalFmtReg.rField.u4YEACTSTART + u4Top;
	_rSclHalFmtReg.rField.u4YEWINEND = _rSclHalFmtReg.rField.u4YEACTSTART + u4Bottom - 1;

	_rSclHalFmtReg.rField.fgWINON = 1;

	_rSclFmtRegMode[0xD0 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xD4 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xD8 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0x84 / 4] |= SCL_HAL_REG_MODE_WRITE;
}
EXPORT_SYMBOL(vSclHalSetWinActive);

void vSclHalSetVScale(__u32 u4VScale)
{
	/*Set vertical scale factor (20bit accuracy)*/
	_rSclHalSclReg.rField.u4VSCALE = u4VScale;

	_rSclRegMode[0x14 / 4] |= SCL_HAL_REG_MODE_WRITE;
}
EXPORT_SYMBOL(vSclHalSetVScale);

void vSclHalSetHScale(__u32 u4HScale)
{
	/*Set horizontal scale factor*/
	_rSclHalFmtReg.rField.u4HSCALE  = u4HScale;

	_rSclFmtRegMode[0xB0 / 4] |= SCL_HAL_REG_MODE_WRITE;
}
EXPORT_SYMBOL(vSclHalSetHScale);


void vSclHalSetLongBuf(bool fgLBEnable)
{
	if (fgLBEnable) {
		/*Long buffer only work in FIR_Lnr mode*/
		_rSclHalSclReg.rField.fgLONGBUF = 1;
		_rSclHalSclReg.rField.fgFIR_LNR = 1;
	} else {
		_rSclHalSclReg.rField.fgLONGBUF = 0;
	}

	_rSclRegMode[0x1C / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetVFliter(__u32 u4VFliter)
{
	switch (u4VFliter) {
	case SCL_VFIR_FLITER:
		/* FIR fliter only work in vertical down case*/
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgFIR_LNR = 0;
		_rSclRegMode[0x1C / 4] |= SCL_HAL_REG_MODE_WRITE;
		break;

	case SCL_VFIRLNR_FLITER:
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgFIR_LNR = 1;
		_rSclRegMode[0x1C / 4] |= SCL_HAL_REG_MODE_WRITE;
		break;

	case SCL_VLINEAR_FLITER:
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclRegMode[0x1C / 4] |= SCL_HAL_REG_MODE_WRITE;

		/*In vertical up case, must set 0xA458C bit[31]*/
		if (_u4SrcWidth < _u4DstWidth) {
			_rSclHalSclReg.rField.fgVUPLNR = 1;
			_rSclRegMode[0x8C / 4] |= SCL_HAL_REG_MODE_WRITE;
		}

		break;

	default:
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] Not support %d vertical fliter\r\n", (int)u4VFliter);
		break;
	}
}

void vSclHalSetHFliter(__u32 u4HFliter)
{
	switch (u4HFliter) {
	case SCL_HFIR_FLITER:
		_rSclHalFmtReg.rField.fgHYLNR = 0;
		_rSclHalFmtReg.rField.fgHCLNR = 0;
		break;

	case SCL_HLINEAR_FLITER:
		_rSclHalFmtReg.rField.fgHYLNR = 1;
		_rSclHalFmtReg.rField.fgHCLNR = 1;
		break;

	case SCL_HNONLNR_FLITER:
		_rSclHalFmtReg.rField.fgMULTIR = 1;
		_rSclHalFmtReg.rField.u4INITSCALE = PANEL_SCL_MULTI[_u4OutputRes][_u4InputRes][0];
		_rSclHalFmtReg.rField.u4INCSTART = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][0] + 99;
		_rSclHalFmtReg.rField.u4INCEND = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][0] + 199;
		_rSclHalFmtReg.rField.u4DECSTART = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][1] - 199;
		_rSclHalFmtReg.rField.u4DECEND = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][1] - 99;
		_rSclFmtRegMode[0xC8 / 4] |= SCL_HAL_REG_MODE_WRITE;
		_rSclFmtRegMode[0xCC / 4] |= SCL_HAL_REG_MODE_WRITE;
		break;

	default:
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] Not support %d horizontal fliter\r\n", (int)u4HFliter);
		break;
	}

	_rSclHalFmtReg.rField.fgHSCLON = 1;
	_rSclFmtRegMode[0xB0 / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetBG(__u32 u4BGColor)
{
	_rSclHalFmtReg.rField.u4BGY  = (u4BGColor & 0xF00000) >> 20;
	_rSclHalFmtReg.rField.u4BGCB = (u4BGColor & 0x00F000) >> 12;
	_rSclHalFmtReg.rField.u4BGCR = (u4BGColor & 0x0000F0) >> 4;
	_rSclFmtRegMode[0xB8 / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetBI(bool fgEnable, __u32 u4BIColor)
{
	if (fgEnable) {
		_rSclFmtRegMode[0xAC / 4] |= SCL_HAL_REG_MODE_WRITE;
	} else {
		_rSclHalFmtReg.rField.fgALL_BLK = 0;
		_rSclFmtRegMode[0xAC / 4] |= SCL_HAL_REG_MODE_WRITE;
	}

	_rSclHalFmtReg.rField.u4BIY  = (u4BIColor & 0xF00000) >> 20;
	_rSclHalFmtReg.rField.u4BICB = (u4BIColor & 0x00F000) >> 12;
	_rSclHalFmtReg.rField.u4BICR = (u4BIColor & 0x0000F0) >> 4;
	_rSclHalFmtReg.rField.fgALL_BLK = 1;
	_rSclFmtRegMode[0xB4 / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetHVTotal(bool fgAdjustOn, __u32 u4HTotal, __u32 u4VTotal)
{
	_rSclHalFmtReg.rField.fgADJTOTAL = fgAdjustOn;
	_rSclHalFmtReg.rField.u4HTOTAL = u4HTotal;
	_rSclHalFmtReg.rField.u4VTOTAL = u4VTotal;
	_rSclFmtRegMode[0x8C / 4] |= SCL_HAL_REG_MODE_WRITE;
}


/* **********************************************************************/
/* Function : void vSclHalSetMode(__u32 u4Input, __u32 u4Output)*/
/* Description : Set Panel Scaler Mode*/
/* Parameter : _u4OutputRes : Output Video Resolution*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetMode(__u32 u4Input, __u32 u4Output)
{
	__u32 u4Value;

	VDO_LOG(VDO_LOG_LVL_DBG, "[SCL] vSclHalSetMode input = %d, output = %d\r\n", (int)u4Input, (int)u4Output);

	if ((u4Input >= SCL_IN_480P_800) || (u4Output >= PANEL_NO)) {
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] Set input/output resolution err\r\n");
		return;
	}

	_u4InputRes = u4Input;
	_u4OutputRes = u4Output;

	switch (u4Input) {
	case SCL_IN_480I:
		_u4SrcWidth = 720;
		_u4SrcHeight = 240;
		break;

	case SCL_IN_576I:
		_u4SrcWidth = 720;
		_u4SrcHeight = 288;
		break;

	case SCL_IN_480P:
		_u4SrcWidth = 720;
		_u4SrcHeight = 480;
		break;

	case SCL_IN_576P:
		_u4SrcWidth = 720;
		_u4SrcHeight = 576;
		break;
	}

	switch (u4Output) {
	case SCL_OUT_480_234:
		_u4DstWidth = 480;
		_u4DstHeight = 234;
		break;
#ifdef PANEL_640_480

	case SCL_OUT_640_480:
		_u4DstWidth = 640;
		_u4DstHeight = 240;
		break;
#else

	case SCL_OUT_400_234:
		_u4DstWidth = 400;
		_u4DstHeight = 234;
		break;
#endif

	case SCL_OUT_320_234:
		_u4DstWidth = 320;
		_u4DstHeight = 234;
		break;

	case SCL_OUT_640_234:
		_u4DstWidth = 640;
		_u4DstHeight = 234;
		break;
#ifndef PANEL_800_600

	case SCL_OUT_480_272:
		_u4DstWidth = 480;
		_u4DstHeight = 272;
		break;
#else

	case SCL_OUT_800_600:
		_u4DstWidth = 800;
		_u4DstHeight = 600;
		break;
#endif

	case SCL_OUT_800_480:
		_u4DstWidth = 800;
		_u4DstHeight = 480;
		break;

	case SCL_OUT_1024_600:
		_u4DstWidth = 1024;
		_u4DstHeight = 600;
		break;

	case SCL_OUT_1280_720:
		_u4DstWidth = 1280;
		_u4DstHeight = 720;
		break;

	case SCL_OUT_1280_800:
		_u4DstWidth = 1280;
		_u4DstHeight = 800;
		break;

	case SCL_OUT_1024_768:
		_u4DstWidth = 1024;
		_u4DstHeight = 768;
		break;
	}

#if 0 /* move setting to vSclHalSetMasterMode*/
	/*Set horizontal total pixels and vertical total lines*/
	_rSclHalFmtReg.rField.fgADJTOTAL = 1;
	_rSclHalFmtReg.rField.u4HTOTAL = PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 16;
	_rSclHalFmtReg.rField.u4VTOTAL = (PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 1) & 0xFFFF;

	/*Set last h total*/
	_rSclHalFmtReg.rField.u4HTOTAL_IN = PANEL_INPUT_WIDTH[_u4InputRes];
	_rSclHalFmtReg.rField.u4LAST_HTOTAL = _rSclHalFmtReg.rField.u4HTOTAL;

	/*Set H&V sync shift*/
	_rSclHalFmtReg.rField.u4HSHIFT = PANEL_SYNC_SHIFT[_u4OutputRes][_u4InputRes][1] >> 16;
	_rSclHalFmtReg.rField.u4VSHIFT = PANEL_SYNC_SHIFT[_u4OutputRes][_u4InputRes][1] & 0xFFFF;
#endif

	if (1 == _u4Tm070ddhg) {
		/*Set horizonta, vertical odd, vertical evenl active start and active end*/
		_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE_TM[0];
		_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE_TM[1];
		_rSclHalFmtReg.rField.u4YOACTSTART = PANEL_SCL_ACTIVE_TM[2];
		_rSclHalFmtReg.rField.u4YOACTEND = PANEL_SCL_ACTIVE_TM[3];
		_rSclHalFmtReg.rField.u4YEACTSTART = PANEL_SCL_ACTIVE_TM[4];
		_rSclHalFmtReg.rField.u4YEACTEND = PANEL_SCL_ACTIVE_TM[5];
	} else {
		/*Set horizonta, vertical odd, vertical evenl active start and active end*/
		_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][0];
		_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][1];
		_rSclHalFmtReg.rField.u4YOACTSTART = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][2];
		_rSclHalFmtReg.rField.u4YOACTEND = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][3];
		_rSclHalFmtReg.rField.u4YEACTSTART = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][4];
		_rSclHalFmtReg.rField.u4YEACTEND = PANEL_SCL_ACTIVE[_u4OutputRes][_u4InputRes][5];
	}

	/*Set odd field start line (high 2 bytes) and even field start line (low 2 bytes)*/
	_rSclHalFmtReg.rField.u4SLINE_ACT_O = PANEL_SCL_START_LINE[_u4OutputRes][_u4InputRes][1] >> 16;
	_rSclHalFmtReg.rField.u4SLINE_ACT_E = PANEL_SCL_START_LINE[_u4OutputRes][_u4InputRes][1] & 0xFFFF;

	/*Set vertical scale factor (20bit accuracy)*/
	_rSclHalSclReg.rField.u4VSCALE = PANEL_SCL_RATIO[_u4OutputRes][_u4InputRes][1];

	/*Set horizontal scale factor (high 2 bytes)*/
	_rSclHalFmtReg.rField.u4HSCALE = PANEL_SCL_RATIO[_u4OutputRes][_u4InputRes][0];

	/*Set V/H Fliter, 16 phase, scaler up/down  and fpd V/H clock src*/
	if (_u4SrcHeight >= _u4DstHeight) {
		_rSclHalSclReg.rField.fgVUPSCALE = 0;
		_rSclHalSclReg.rField.fgFIR_LNR = 1;
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgVCLNR = 0;/*CORRECTED*/

		if (_u4SrcWidth >= _u4DstWidth) {
			_rSclHalSclReg.rField.fgHUPSCALE = 0;
			u4Value = HAL_READ32(FPD_HAL_CONFIG_REG) & (~SCL_CLK_SEL_MSK);
			HAL_WRITE32(FPD_HAL_CONFIG_REG, u4Value);
		} else {
			_rSclHalSclReg.rField.fgHUPSCALE = 1;
			u4Value = HAL_READ32(FPD_HAL_CONFIG_REG) & (~SCL_CLK_SEL_MSK);
			u4Value |= HSCL_CLK_SEL;
			HAL_WRITE32(FPD_HAL_CONFIG_REG, u4Value);
		}
	} else {
		_rSclHalSclReg.rField.fgVUPSCALE = 1;
		_rSclHalSclReg.rField.fgFIR_LNR = 1;
		_rSclHalSclReg.rField.fgVYLNR = 0;
		_rSclHalSclReg.rField.fgVYLNR = 0;

		u4Value = HAL_READ32(FPD_HAL_CONFIG_REG) & (~SCL_CLK_SEL_MSK);
		u4Value |= HSCL_CLK_SEL | VSCL_CLK_SEL;
		HAL_WRITE32(FPD_HAL_CONFIG_REG, u4Value);

		if (_u4SrcWidth >= _u4DstWidth) {
			_rSclHalSclReg.rField.fgHUPSCALE = 0;
		} else {
			_rSclHalSclReg.rField.fgHUPSCALE = 1;
		}
	}

	_rSclHalSclReg.rField.fgDROP_C = 1;
	_rSclHalSclReg.rField.fgKEEP_4N = 0;
	_rSclHalSclReg.rField.fgVPHASE16 = 1;
	_rSclHalFmtReg.rField.fgHPHASE16 = 1;

	_rSclHalFmtReg.rField.fgHYLNR = 0;
	_rSclHalFmtReg.rField.fgHCLNR = 0;

	_rSclHalFmtReg.rField.fgHSCLON = 1;

	/*Enable horizontal scaler (bit[10]), enable vertical scaler (bit[9])
		, turn on linear filtering for first 2 lines (bit[1:0]), select prgs out*/
	_rSclHalSclReg.rField.fgY4TAP_ALL = 1;
	_rSclHalSclReg.rField.fgC4TAP_ALL = 1;
	_rSclHalSclReg.rField.fgSELF_EN = 0;
	_rSclHalSclReg.rField.fgSHSCLON = 1;
	_rSclHalSclReg.rField.fgSVSCLON = 1;
	_rSclHalSclReg.rField.fgSPRGSOUT = 1;

	/*Fix vertical last pixel is garbage*/
	_rSclHalSclReg.rField.fgMAHW_OPT = 1;

	/*Set Vsync signal use level or 1T, 1 - level, 0 - 1T*/
	_rSclHalFmtReg.rField.fgVDOIN_VSEL = 1;

	/*Set sync mode to sync all input vertical sync (bit[24] only in progressive output) and
		reset formatter after scaler setting done*/
	_rSclHalFmtReg.rField.fgSYNC_ALL = 1;
	_prSclFmtHwReg->rField.fgRSTTRIG = 1;

	_rSclRegMode[0x14 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x1C / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x30 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x8C / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0x84 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xA0 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xA4 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xA8 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xAC / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xB0 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xDC / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0xE8 / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetMasterMode(bool fgEnable, __u32 u4Input, __u32 u4Output)
{
#if MASTER_MODE_ENABLE

	if (fgEnable) {
		_rSclHalFmtReg.rField.fgMASTER_EN = 1;
		_rSclHalFmtReg.rField.fgPROG_OUT = 1;
		_rSclHalFmtReg.rField.fgSEL_SHIFT = 1;
		_rSclHalFmtReg.rField.fgDELAY_SEL = 1;

		/*Set H sync shift*/
		_rSclHalFmtReg.rField.u4HSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][0] >> 16;

		if (u4Input < SCL_IN_480P_800) {
			_rSclHalFmtReg.rField.fgHD_MODE = 0;
			/*Set horizontal total pixels and vertical total lines*/
			_rSclHalFmtReg.rField.fgADJTOTAL = 1;
			/*_rSclHalFmtReg.rField.u4HTOTAL = PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 16;*/
			/*_rSclHalFmtReg.rField.u4VTOTAL = (PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 1) & 0xFFFF;*/

			/*Set V sync shift*/
			_rSclHalFmtReg.rField.u4VSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][0] & 0xFFFF;

			if (HAL_READ32(CP_ENABLE_HW_REG) & CP_ENABLE_MSK) {
				_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE[u4Output][u4Input][0] - 0x35;
				_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE[u4Output][u4Input][1] - 0x35;
			} else {
				_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE[u4Output][u4Input][0];
				_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE[u4Output][u4Input][1];
			}

			_rSclFmtRegMode[0xA0 / 4] |= SCL_HAL_REG_MODE_WRITE;
		} else {
			_rSclHalFmtReg.rField.fgHD_MODE = 1;
			/*Set horizontal total pixels and vertical total lines*/
			_rSclHalFmtReg.rField.fgADJTOTAL = 1;

			switch (u4Input) {
			case SCL_IN_480P_800:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x3B9;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x20D;*/
				_rSclHalFmtReg.rField.u4HSHIFT = 0x11;
				_rSclHalFmtReg.rField.u4VSHIFT = 0x14;
				break;

			case SCL_IN_600P_800:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x405;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x290;*/
				break;

			case SCL_IN_600P_1024:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x516;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x290;*/
				_rSclHalFmtReg.rField.u4VSHIFT = 0x8;
				break;

			case SCL_IN_720P_1280:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x56B;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x313;*/
				break;

			case SCL_IN_800P_1280:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x564;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x36B;*/
				break;

			case SCL_IN_768P_1024:
				/*_rSclHalFmtReg.rField.u4HTOTAL = 0x7C2;*/
				/*_rSclHalFmtReg.rField.u4VTOTAL = 0x348;*/
				_rSclHalFmtReg.rField.u4VSHIFT = 0x9;
				break;
			}

			if (HAL_READ32(CP_ENABLE_HW_REG) & CP_ENABLE_MSK) {
				_rSclHalFmtReg.rField.u4HSHIFT = _rSclHalFmtReg.rField.u4HSHIFT + 0x35;
			}
		}

		_rSclFmtRegMode[0x7C / 4] |= SCL_HAL_REG_MODE_WRITE;
		_rSclFmtRegMode[0x8C / 4] |= SCL_HAL_REG_MODE_WRITE;
		_rSclFmtRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
	} else
#endif
	{
		if ((u4Input >= SCL_IN_480P_800) || (u4Output >= PANEL_NO)) {
			VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] Set input/output resolution err\r\n");
			return;
		}

		_rSclHalFmtReg.rField.fgMASTER_EN = 0;
		/*Set horizontal total pixels and vertical total lines*/
		_rSclHalFmtReg.rField.fgADJTOTAL = 1;
		/*_rSclHalFmtReg.rField.u4HTOTAL = PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 16;*/
		/*_rSclHalFmtReg.rField.u4VTOTAL = (PANEL_HV_TOTAL[_u4OutputRes][_u4InputRes] >> 1) & 0xFFFF;*/

		/*Set H&V sync shift*/
		_rSclHalFmtReg.rField.u4HSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][1] >> 16;
		_rSclHalFmtReg.rField.u4VSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][1] & 0xFFFF;

		_rSclFmtRegMode[0x7C / 4] |= SCL_HAL_REG_MODE_WRITE;
		_rSclFmtRegMode[0x8C / 4] |= SCL_HAL_REG_MODE_WRITE;
		_rSclFmtRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
	}

	/*Set last h total*/
	_rSclHalFmtReg.rField.u4LAST_HTOTAL = _rSclHalFmtReg.rField.u4HTOTAL;
	_rSclHalFmtReg.rField.u4HTOTAL_IN = _rSclHalFmtReg.rField.u4HTOTAL;
	_rSclFmtRegMode[0xE8 / 4] |= SCL_HAL_REG_MODE_WRITE;
}

void vSclHalSetMasterSrc(__u32 u4Input, __u32 u4Output)
{
#if MASTER_MODE_ENABLE
	_rSclHalFmtReg.rField.u4HSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][0] >> 16;

	if (u4Input < SCL_IN_480P_800) {
		_rSclHalFmtReg.rField.fgHD_MODE = 0;
		/*Set H&V sync shift*/
		_rSclHalFmtReg.rField.u4VSHIFT = PANEL_SYNC_SHIFT[u4Output][u4Input][0] & 0xFFFF;

		if (HAL_READ32(CP_ENABLE_HW_REG) & CP_ENABLE_MSK) {
			_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE[u4Output][u4Input][0] - 0x35;
			_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE[u4Output][u4Input][1] - 0x35;
		} else {
			_rSclHalFmtReg.rField.u4XACTSTART = PANEL_SCL_ACTIVE[u4Output][u4Input][0];
			_rSclHalFmtReg.rField.u4XACTEND = PANEL_SCL_ACTIVE[u4Output][u4Input][1];
		}

		_rSclFmtRegMode[0xA0 / 4] |= SCL_HAL_REG_MODE_WRITE;
	} else {
		_rSclHalFmtReg.rField.fgHD_MODE = 1;

		switch (u4Input) {
		case SCL_IN_480P_800:
			_rSclHalFmtReg.rField.u4HSHIFT = 0x11;
			_rSclHalFmtReg.rField.u4VSHIFT = 0x15;
			break;

		case SCL_IN_600P_800:
			_rSclHalFmtReg.rField.u4VSHIFT = 0xA;
			break;

		case SCL_IN_600P_1024:
			_rSclHalFmtReg.rField.u4VSHIFT = 0xA;
			break;

		case SCL_IN_720P_1280:
			_rSclHalFmtReg.rField.u4VSHIFT = 0xA;
			break;

		case SCL_IN_800P_1280:
			_rSclHalFmtReg.rField.u4HSHIFT = 0x22;
			_rSclHalFmtReg.rField.u4VSHIFT = 0x4;
			break;

		case SCL_IN_768P_1024:
			_rSclHalFmtReg.rField.u4VSHIFT = 0x9;
			break;
		}

		if (HAL_READ32(CP_ENABLE_HW_REG) & CP_ENABLE_MSK) {
			_rSclHalFmtReg.rField.u4HSHIFT = _rSclHalFmtReg.rField.u4HSHIFT + 0x35;
		}
	}

	_rSclFmtRegMode[0x7C / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclFmtRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
#endif
}

/* **********************************************************************/
/* Function : void vSclCoef_H (__u8 u4HRatio)*/
/* Description : Set Panel Scaler FIR Filtet Horizontal Coefficients*/
/* Parameter : u4HRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetHCoef(__u32 u4HRatio)
{
	__u32 u4Idx = 0;

	if (u4HRatio >= H_FILTER_NO) {
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] vSclHalSetHCoef param err %d\r\n", (int)u4HRatio);
		return;
	}

	/*WRITE SCALER Y HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF0_0 = PANEL_SCL_Y_H_COEF[u4HRatio][0][1];
	_rSclHalSclReg.rField.u4YH_COEF0_1 = PANEL_SCL_Y_H_COEF[u4HRatio][0][2];
	_rSclHalSclReg.rField.u4YH_COEF0_2 = PANEL_SCL_Y_H_COEF[u4HRatio][0][3];
	_rSclHalSclReg.rField.u4YH_COEF0_3 = PANEL_SCL_Y_H_COEF[u4HRatio][0][4];
	_rSclHalSclReg.rField.u4YH_COEF0_4 = PANEL_SCL_Y_H_COEF[u4HRatio][0][5];
	_rSclHalSclReg.rField.u4YH_COEF0_5 = PANEL_SCL_Y_H_COEF[u4HRatio][0][6];
	_rSclHalSclReg.rField.u4YH_COEF0_6 = PANEL_SCL_Y_H_COEF[u4HRatio][0][7];
	_rSclHalSclReg.rField.u4YH_COEF0_7 = PANEL_SCL_Y_H_COEF[u4HRatio][0][8];

	/*Phase 1, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF1_0 = PANEL_SCL_Y_H_COEF[u4HRatio][1][1];
	_rSclHalSclReg.rField.u4YH_COEF1_1 = PANEL_SCL_Y_H_COEF[u4HRatio][1][2];
	_rSclHalSclReg.rField.u4YH_COEF1_2 = PANEL_SCL_Y_H_COEF[u4HRatio][1][3];
	_rSclHalSclReg.rField.u4YH_COEF1_3 = PANEL_SCL_Y_H_COEF[u4HRatio][1][4];
	_rSclHalSclReg.rField.u4YH_COEF1_4 = PANEL_SCL_Y_H_COEF[u4HRatio][1][5];
	_rSclHalSclReg.rField.u4YH_COEF1_5 = PANEL_SCL_Y_H_COEF[u4HRatio][1][6];
	_rSclHalSclReg.rField.u4YH_COEF1_6 = PANEL_SCL_Y_H_COEF[u4HRatio][1][7];
	_rSclHalSclReg.rField.u4YH_COEF1_7 = PANEL_SCL_Y_H_COEF[u4HRatio][1][8];

	/*Phase 2, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF2_0 = PANEL_SCL_Y_H_COEF[u4HRatio][2][1];
	_rSclHalSclReg.rField.u4YH_COEF2_1 = PANEL_SCL_Y_H_COEF[u4HRatio][2][2];
	_rSclHalSclReg.rField.u4YH_COEF2_2 = PANEL_SCL_Y_H_COEF[u4HRatio][2][3];
	_rSclHalSclReg.rField.u4YH_COEF2_3 = PANEL_SCL_Y_H_COEF[u4HRatio][2][4];
	_rSclHalSclReg.rField.u4YH_COEF2_4 = PANEL_SCL_Y_H_COEF[u4HRatio][2][5];
	_rSclHalSclReg.rField.u4YH_COEF2_5 = PANEL_SCL_Y_H_COEF[u4HRatio][2][6];
	_rSclHalSclReg.rField.u4YH_COEF2_6 = PANEL_SCL_Y_H_COEF[u4HRatio][2][7];
	_rSclHalSclReg.rField.u4YH_COEF2_7 = PANEL_SCL_Y_H_COEF[u4HRatio][2][8];

	/*Phase 3, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF3_0 = PANEL_SCL_Y_H_COEF[u4HRatio][3][1];
	_rSclHalSclReg.rField.u4YH_COEF3_1 = PANEL_SCL_Y_H_COEF[u4HRatio][3][2];
	_rSclHalSclReg.rField.u4YH_COEF3_2 = PANEL_SCL_Y_H_COEF[u4HRatio][3][3];
	_rSclHalSclReg.rField.u4YH_COEF3_3 = PANEL_SCL_Y_H_COEF[u4HRatio][3][4];
	_rSclHalSclReg.rField.u4YH_COEF3_4 = PANEL_SCL_Y_H_COEF[u4HRatio][3][5];
	_rSclHalSclReg.rField.u4YH_COEF3_5 = PANEL_SCL_Y_H_COEF[u4HRatio][3][6];
	_rSclHalSclReg.rField.u4YH_COEF3_6 = PANEL_SCL_Y_H_COEF[u4HRatio][3][7];
	_rSclHalSclReg.rField.u4YH_COEF3_7 = PANEL_SCL_Y_H_COEF[u4HRatio][3][8];

	/*Phase 4, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF4_0 = PANEL_SCL_Y_H_COEF[u4HRatio][4][1];
	_rSclHalSclReg.rField.u4YH_COEF4_1 = PANEL_SCL_Y_H_COEF[u4HRatio][4][2];
	_rSclHalSclReg.rField.u4YH_COEF4_2 = PANEL_SCL_Y_H_COEF[u4HRatio][4][3];
	_rSclHalSclReg.rField.u4YH_COEF4_3 = PANEL_SCL_Y_H_COEF[u4HRatio][4][4];
	_rSclHalSclReg.rField.u4YH_COEF4_4 = PANEL_SCL_Y_H_COEF[u4HRatio][4][5];
	_rSclHalSclReg.rField.u4YH_COEF4_5 = PANEL_SCL_Y_H_COEF[u4HRatio][4][6];
	_rSclHalSclReg.rField.u4YH_COEF4_6 = PANEL_SCL_Y_H_COEF[u4HRatio][4][7];
	_rSclHalSclReg.rField.u4YH_COEF4_7 = PANEL_SCL_Y_H_COEF[u4HRatio][4][8];

	/*Phase 5, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF5_0 = PANEL_SCL_Y_H_COEF[u4HRatio][5][1];
	_rSclHalSclReg.rField.u4YH_COEF5_1 = PANEL_SCL_Y_H_COEF[u4HRatio][5][2];
	_rSclHalSclReg.rField.u4YH_COEF5_2 = PANEL_SCL_Y_H_COEF[u4HRatio][5][3];
	_rSclHalSclReg.rField.u4YH_COEF5_3 = PANEL_SCL_Y_H_COEF[u4HRatio][5][4];
	_rSclHalSclReg.rField.u4YH_COEF5_4 = PANEL_SCL_Y_H_COEF[u4HRatio][5][5];
	_rSclHalSclReg.rField.u4YH_COEF5_5 = PANEL_SCL_Y_H_COEF[u4HRatio][5][6];
	_rSclHalSclReg.rField.u4YH_COEF5_6 = PANEL_SCL_Y_H_COEF[u4HRatio][5][7];
	_rSclHalSclReg.rField.u4YH_COEF5_7 = PANEL_SCL_Y_H_COEF[u4HRatio][5][8];

	/*Phase 6, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF6_0 = PANEL_SCL_Y_H_COEF[u4HRatio][6][1];
	_rSclHalSclReg.rField.u4YH_COEF6_1 = PANEL_SCL_Y_H_COEF[u4HRatio][6][2];
	_rSclHalSclReg.rField.u4YH_COEF6_2 = PANEL_SCL_Y_H_COEF[u4HRatio][6][3];
	_rSclHalSclReg.rField.u4YH_COEF6_3 = PANEL_SCL_Y_H_COEF[u4HRatio][6][4];
	_rSclHalSclReg.rField.u4YH_COEF6_4 = PANEL_SCL_Y_H_COEF[u4HRatio][6][5];
	_rSclHalSclReg.rField.u4YH_COEF6_5 = PANEL_SCL_Y_H_COEF[u4HRatio][6][6];
	_rSclHalSclReg.rField.u4YH_COEF6_6 = PANEL_SCL_Y_H_COEF[u4HRatio][6][7];
	_rSclHalSclReg.rField.u4YH_COEF6_7 = PANEL_SCL_Y_H_COEF[u4HRatio][6][8];

	/*Phase 7, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF7_0 = PANEL_SCL_Y_H_COEF[u4HRatio][7][1];
	_rSclHalSclReg.rField.u4YH_COEF7_1 = PANEL_SCL_Y_H_COEF[u4HRatio][7][2];
	_rSclHalSclReg.rField.u4YH_COEF7_2 = PANEL_SCL_Y_H_COEF[u4HRatio][7][3];
	_rSclHalSclReg.rField.u4YH_COEF7_3 = PANEL_SCL_Y_H_COEF[u4HRatio][7][4];
	_rSclHalSclReg.rField.u4YH_COEF7_4 = PANEL_SCL_Y_H_COEF[u4HRatio][7][5];
	_rSclHalSclReg.rField.u4YH_COEF7_5 = PANEL_SCL_Y_H_COEF[u4HRatio][7][6];
	_rSclHalSclReg.rField.u4YH_COEF7_6 = PANEL_SCL_Y_H_COEF[u4HRatio][7][7];
	_rSclHalSclReg.rField.u4YH_COEF7_7 = PANEL_SCL_Y_H_COEF[u4HRatio][7][8];

	/*Phase 0, Idx 0, 9 & Phase 1, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF0_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][0][0];
	_rSclHalSclReg.rField.u4YH_COEF0_8 = PANEL_SCL_Y_H_COEF[u4HRatio][0][9];
	_rSclHalSclReg.rField.u4YH_COEF1_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][1][0];
	_rSclHalSclReg.rField.u4YH_COEF1_8 = PANEL_SCL_Y_H_COEF[u4HRatio][1][9];

	/*Phase 2, Idx 0, 9 & Phase 3, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF2_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][2][0];
	_rSclHalSclReg.rField.u4YH_COEF2_8 = PANEL_SCL_Y_H_COEF[u4HRatio][2][9];
	_rSclHalSclReg.rField.u4YH_COEF3_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][3][0];
	_rSclHalSclReg.rField.u4YH_COEF3_8 = PANEL_SCL_Y_H_COEF[u4HRatio][3][9];

	/*Phase 4, Idx 0, 9 & Phase 5, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF4_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][4][0];
	_rSclHalSclReg.rField.u4YH_COEF4_8 = PANEL_SCL_Y_H_COEF[u4HRatio][4][9];
	_rSclHalSclReg.rField.u4YH_COEF5_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][5][0];
	_rSclHalSclReg.rField.u4YH_COEF5_8 = PANEL_SCL_Y_H_COEF[u4HRatio][5][9];

	/*Phase 6, Idx 0, 9 & Phase 7, Idx 0, 9*/
	_rSclHalFmtReg.rField.u4CH_COEF6_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_8 = PANEL_SCL_Y_H_COEF[u4HRatio][6][9];
	_rSclHalFmtReg.rField.u4CH_COEF7_M1 = PANEL_SCL_Y_H_COEF[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_8 = PANEL_SCL_Y_H_COEF[u4HRatio][7][9];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0xFC / 4] =
		PANEL_SCL_Y_H_MSB[u4HRatio][0] + (PANEL_SCL_Y_H_MSB[u4HRatio][1] << 8)
		+ (PANEL_SCL_Y_H_MSB[u4HRatio][2] << 16) + (PANEL_SCL_Y_H_MSB[u4HRatio][3] << 24);

	/*WRITE SCALER C HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF0_0 = PANEL_SCL_C_H_COEF[u4HRatio][0][0];
	_rSclHalFmtReg.rField.u4CH_COEF0_1 = PANEL_SCL_C_H_COEF[u4HRatio][0][1];
	_rSclHalFmtReg.rField.u4CH_COEF0_2 = PANEL_SCL_C_H_COEF[u4HRatio][0][2];
	_rSclHalFmtReg.rField.u4CH_COEF0_3 = PANEL_SCL_C_H_COEF[u4HRatio][0][3];
	_rSclHalFmtReg.rField.u4CH_COEF0_4 = PANEL_SCL_C_H_COEF[u4HRatio][0][4];
	_rSclHalFmtReg.rField.u4CH_COEF0_5 = PANEL_SCL_C_H_COEF[u4HRatio][0][5];
	_rSclHalFmtReg.rField.u4CH_COEF0_6 = PANEL_SCL_C_H_COEF[u4HRatio][0][6];
	_rSclHalFmtReg.rField.u4CH_COEF0_7 = PANEL_SCL_C_H_COEF[u4HRatio][0][7];

	/*Phase 1, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF1_0 = PANEL_SCL_C_H_COEF[u4HRatio][1][0];
	_rSclHalFmtReg.rField.u4CH_COEF1_1 = PANEL_SCL_C_H_COEF[u4HRatio][1][1];
	_rSclHalFmtReg.rField.u4CH_COEF1_2 = PANEL_SCL_C_H_COEF[u4HRatio][1][2];
	_rSclHalFmtReg.rField.u4CH_COEF1_3 = PANEL_SCL_C_H_COEF[u4HRatio][1][3];
	_rSclHalFmtReg.rField.u4CH_COEF1_4 = PANEL_SCL_C_H_COEF[u4HRatio][1][4];
	_rSclHalFmtReg.rField.u4CH_COEF1_5 = PANEL_SCL_C_H_COEF[u4HRatio][1][5];
	_rSclHalFmtReg.rField.u4CH_COEF1_6 = PANEL_SCL_C_H_COEF[u4HRatio][1][6];
	_rSclHalFmtReg.rField.u4CH_COEF1_7 = PANEL_SCL_C_H_COEF[u4HRatio][1][7];

	/*Phase 2, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF2_0 = PANEL_SCL_C_H_COEF[u4HRatio][2][0];
	_rSclHalFmtReg.rField.u4CH_COEF2_1 = PANEL_SCL_C_H_COEF[u4HRatio][2][1];
	_rSclHalFmtReg.rField.u4CH_COEF2_2 = PANEL_SCL_C_H_COEF[u4HRatio][2][2];
	_rSclHalFmtReg.rField.u4CH_COEF2_3 = PANEL_SCL_C_H_COEF[u4HRatio][2][3];
	_rSclHalFmtReg.rField.u4CH_COEF2_4 = PANEL_SCL_C_H_COEF[u4HRatio][2][4];
	_rSclHalFmtReg.rField.u4CH_COEF2_5 = PANEL_SCL_C_H_COEF[u4HRatio][2][5];
	_rSclHalFmtReg.rField.u4CH_COEF2_6 = PANEL_SCL_C_H_COEF[u4HRatio][2][6];
	_rSclHalFmtReg.rField.u4CH_COEF2_7 = PANEL_SCL_C_H_COEF[u4HRatio][2][7];

	/*Phase 3, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF3_0 = PANEL_SCL_C_H_COEF[u4HRatio][3][0];
	_rSclHalFmtReg.rField.u4CH_COEF3_1 = PANEL_SCL_C_H_COEF[u4HRatio][3][1];
	_rSclHalFmtReg.rField.u4CH_COEF3_2 = PANEL_SCL_C_H_COEF[u4HRatio][3][2];
	_rSclHalFmtReg.rField.u4CH_COEF3_3 = PANEL_SCL_C_H_COEF[u4HRatio][3][3];
	_rSclHalFmtReg.rField.u4CH_COEF3_4 = PANEL_SCL_C_H_COEF[u4HRatio][3][4];
	_rSclHalFmtReg.rField.u4CH_COEF3_5 = PANEL_SCL_C_H_COEF[u4HRatio][3][5];
	_rSclHalFmtReg.rField.u4CH_COEF3_6 = PANEL_SCL_C_H_COEF[u4HRatio][3][6];
	_rSclHalFmtReg.rField.u4CH_COEF3_7 = PANEL_SCL_C_H_COEF[u4HRatio][3][7];

	/*Phase 4, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF4_0 = PANEL_SCL_C_H_COEF[u4HRatio][4][0];
	_rSclHalFmtReg.rField.u4CH_COEF4_1 = PANEL_SCL_C_H_COEF[u4HRatio][4][1];
	_rSclHalFmtReg.rField.u4CH_COEF4_2 = PANEL_SCL_C_H_COEF[u4HRatio][4][2];
	_rSclHalFmtReg.rField.u4CH_COEF4_3 = PANEL_SCL_C_H_COEF[u4HRatio][4][3];
	_rSclHalFmtReg.rField.u4CH_COEF4_4 = PANEL_SCL_C_H_COEF[u4HRatio][4][4];
	_rSclHalFmtReg.rField.u4CH_COEF4_5 = PANEL_SCL_C_H_COEF[u4HRatio][4][5];
	_rSclHalFmtReg.rField.u4CH_COEF4_6 = PANEL_SCL_C_H_COEF[u4HRatio][4][6];
	_rSclHalFmtReg.rField.u4CH_COEF4_7 = PANEL_SCL_C_H_COEF[u4HRatio][4][7];

	/*Phase 5, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF5_0 = PANEL_SCL_C_H_COEF[u4HRatio][5][0];
	_rSclHalFmtReg.rField.u4CH_COEF5_1 = PANEL_SCL_C_H_COEF[u4HRatio][5][1];
	_rSclHalFmtReg.rField.u4CH_COEF5_2 = PANEL_SCL_C_H_COEF[u4HRatio][5][2];
	_rSclHalFmtReg.rField.u4CH_COEF5_3 = PANEL_SCL_C_H_COEF[u4HRatio][5][3];
	_rSclHalFmtReg.rField.u4CH_COEF5_4 = PANEL_SCL_C_H_COEF[u4HRatio][5][4];
	_rSclHalFmtReg.rField.u4CH_COEF5_5 = PANEL_SCL_C_H_COEF[u4HRatio][5][5];
	_rSclHalFmtReg.rField.u4CH_COEF5_6 = PANEL_SCL_C_H_COEF[u4HRatio][5][6];
	_rSclHalFmtReg.rField.u4CH_COEF5_7 = PANEL_SCL_C_H_COEF[u4HRatio][5][7];

	/*Phase 6, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF6_0 = PANEL_SCL_C_H_COEF[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_1 = PANEL_SCL_C_H_COEF[u4HRatio][6][1];
	_rSclHalFmtReg.rField.u4CH_COEF6_2 = PANEL_SCL_C_H_COEF[u4HRatio][6][2];
	_rSclHalFmtReg.rField.u4CH_COEF6_3 = PANEL_SCL_C_H_COEF[u4HRatio][6][3];
	_rSclHalFmtReg.rField.u4CH_COEF6_4 = PANEL_SCL_C_H_COEF[u4HRatio][6][4];
	_rSclHalFmtReg.rField.u4CH_COEF6_5 = PANEL_SCL_C_H_COEF[u4HRatio][6][5];
	_rSclHalFmtReg.rField.u4CH_COEF6_6 = PANEL_SCL_C_H_COEF[u4HRatio][6][6];
	_rSclHalFmtReg.rField.u4CH_COEF6_7 = PANEL_SCL_C_H_COEF[u4HRatio][6][7];

	/*Phase 7, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF7_0 = PANEL_SCL_C_H_COEF[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_1 = PANEL_SCL_C_H_COEF[u4HRatio][7][1];
	_rSclHalFmtReg.rField.u4CH_COEF7_2 = PANEL_SCL_C_H_COEF[u4HRatio][7][2];
	_rSclHalFmtReg.rField.u4CH_COEF7_3 = PANEL_SCL_C_H_COEF[u4HRatio][7][3];
	_rSclHalFmtReg.rField.u4CH_COEF7_4 = PANEL_SCL_C_H_COEF[u4HRatio][7][4];
	_rSclHalFmtReg.rField.u4CH_COEF7_5 = PANEL_SCL_C_H_COEF[u4HRatio][7][5];
	_rSclHalFmtReg.rField.u4CH_COEF7_6 = PANEL_SCL_C_H_COEF[u4HRatio][7][6];
	_rSclHalFmtReg.rField.u4CH_COEF7_7 = PANEL_SCL_C_H_COEF[u4HRatio][7][7];

	/*Write MSB Registers*/
	_rSclHalFmtReg.au4Reg[0x4C / 4] =
		PANEL_SCL_C_H_MSB[u4HRatio][0] + (PANEL_SCL_C_H_MSB[u4HRatio][1] << 8)
		+ (PANEL_SCL_C_H_MSB[u4HRatio][2] << 16) + (PANEL_SCL_C_H_MSB[u4HRatio][3] << 24);

	_rSclRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x94 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x98 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x9C / 4] |= SCL_HAL_REG_MODE_WRITE;

	for (u4Idx = 0xC0; u4Idx <= 0xFC; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	for (u4Idx = 0x00; u4Idx <= 0x40; u4Idx += 4) {
		_rSclFmtRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	_rSclFmtRegMode[(0x4C >> 2)] |= SCL_HAL_REG_MODE_WRITE;
}

/* **********************************************************************/
/* Function : void vSclCoef_H_Sharpness (__u8 u4HRatio)*/
/* Description : Set Panel Scaler FIR Filtet Horizontal Coefficients (Sharpness Adjustment)*/
/* Parameter : u4HRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetHCoefSharpness(__u8 u4HRatio)
{
	__u32 u4Idx = 0;

	if (u4HRatio >= H_FILTER_NO) {
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] vSclHalSetHCoef param err %d\r\n", (int)u4HRatio);
		return;
	}

	/*WRITE SCALER Y HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF0_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][1];
	_rSclHalSclReg.rField.u4YH_COEF0_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][2];
	_rSclHalSclReg.rField.u4YH_COEF0_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][3];
	_rSclHalSclReg.rField.u4YH_COEF0_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][4];
	_rSclHalSclReg.rField.u4YH_COEF0_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][5];
	_rSclHalSclReg.rField.u4YH_COEF0_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][6];
	_rSclHalSclReg.rField.u4YH_COEF0_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][7];
	_rSclHalSclReg.rField.u4YH_COEF0_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][8];

	/*Phase 1, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF1_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][1];
	_rSclHalSclReg.rField.u4YH_COEF1_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][2];
	_rSclHalSclReg.rField.u4YH_COEF1_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][3];
	_rSclHalSclReg.rField.u4YH_COEF1_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][4];
	_rSclHalSclReg.rField.u4YH_COEF1_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][5];
	_rSclHalSclReg.rField.u4YH_COEF1_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][6];
	_rSclHalSclReg.rField.u4YH_COEF1_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][7];
	_rSclHalSclReg.rField.u4YH_COEF1_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][8];

	/*Phase 2, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF2_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][1];
	_rSclHalSclReg.rField.u4YH_COEF2_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][2];
	_rSclHalSclReg.rField.u4YH_COEF2_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][3];
	_rSclHalSclReg.rField.u4YH_COEF2_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][4];
	_rSclHalSclReg.rField.u4YH_COEF2_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][5];
	_rSclHalSclReg.rField.u4YH_COEF2_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][6];
	_rSclHalSclReg.rField.u4YH_COEF2_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][7];
	_rSclHalSclReg.rField.u4YH_COEF2_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][8];

	/*Phase 3, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF3_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][1];
	_rSclHalSclReg.rField.u4YH_COEF3_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][2];
	_rSclHalSclReg.rField.u4YH_COEF3_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][3];
	_rSclHalSclReg.rField.u4YH_COEF3_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][4];
	_rSclHalSclReg.rField.u4YH_COEF3_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][5];
	_rSclHalSclReg.rField.u4YH_COEF3_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][6];
	_rSclHalSclReg.rField.u4YH_COEF3_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][7];
	_rSclHalSclReg.rField.u4YH_COEF3_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][8];

	/*Phase 4, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF4_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][1];
	_rSclHalSclReg.rField.u4YH_COEF4_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][2];
	_rSclHalSclReg.rField.u4YH_COEF4_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][3];
	_rSclHalSclReg.rField.u4YH_COEF4_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][4];
	_rSclHalSclReg.rField.u4YH_COEF4_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][5];
	_rSclHalSclReg.rField.u4YH_COEF4_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][6];
	_rSclHalSclReg.rField.u4YH_COEF4_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][7];
	_rSclHalSclReg.rField.u4YH_COEF4_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][8];

	/*Phase 5, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF5_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][1];
	_rSclHalSclReg.rField.u4YH_COEF5_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][2];
	_rSclHalSclReg.rField.u4YH_COEF5_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][3];
	_rSclHalSclReg.rField.u4YH_COEF5_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][4];
	_rSclHalSclReg.rField.u4YH_COEF5_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][5];
	_rSclHalSclReg.rField.u4YH_COEF5_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][6];
	_rSclHalSclReg.rField.u4YH_COEF5_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][7];
	_rSclHalSclReg.rField.u4YH_COEF5_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][8];

	/*Phase 6, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF6_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][1];
	_rSclHalSclReg.rField.u4YH_COEF6_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][2];
	_rSclHalSclReg.rField.u4YH_COEF6_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][3];
	_rSclHalSclReg.rField.u4YH_COEF6_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][4];
	_rSclHalSclReg.rField.u4YH_COEF6_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][5];
	_rSclHalSclReg.rField.u4YH_COEF6_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][6];
	_rSclHalSclReg.rField.u4YH_COEF6_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][7];
	_rSclHalSclReg.rField.u4YH_COEF6_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][8];

	/*Phase 7, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF7_0 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][1];
	_rSclHalSclReg.rField.u4YH_COEF7_1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][2];
	_rSclHalSclReg.rField.u4YH_COEF7_2 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][3];
	_rSclHalSclReg.rField.u4YH_COEF7_3 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][4];
	_rSclHalSclReg.rField.u4YH_COEF7_4 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][5];
	_rSclHalSclReg.rField.u4YH_COEF7_5 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][6];
	_rSclHalSclReg.rField.u4YH_COEF7_6 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][7];
	_rSclHalSclReg.rField.u4YH_COEF7_7 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][8];

	/*Phase 0, Idx 0, 9 & Phase 1, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF0_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][0];
	_rSclHalSclReg.rField.u4YH_COEF0_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][0][9];
	_rSclHalSclReg.rField.u4YH_COEF1_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][0];
	_rSclHalSclReg.rField.u4YH_COEF1_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][1][9];

	/*Phase 2, Idx 0, 9 & Phase 3, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF2_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][0];
	_rSclHalSclReg.rField.u4YH_COEF2_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][2][9];
	_rSclHalSclReg.rField.u4YH_COEF3_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][0];
	_rSclHalSclReg.rField.u4YH_COEF3_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][3][9];

	/*Phase 4, Idx 0, 9 & Phase 5, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF4_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][0];
	_rSclHalSclReg.rField.u4YH_COEF4_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][4][9];
	_rSclHalSclReg.rField.u4YH_COEF5_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][0];
	_rSclHalSclReg.rField.u4YH_COEF5_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][5][9];

	/*Phase 6, Idx 0, 9 & Phase 7, Idx 0, 9*/
	_rSclHalFmtReg.rField.u4CH_COEF6_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][6][9];
	_rSclHalFmtReg.rField.u4CH_COEF7_M1 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_8 = PANEL_SCL_Y_H_COEF_SHARP[u4HRatio][7][9];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0xFC / 4] =
		PANEL_SCL_Y_H_MSB_SHARP[u4HRatio][0] + (PANEL_SCL_Y_H_MSB_SHARP[u4HRatio][1] << 8)
		+ (PANEL_SCL_Y_H_MSB_SHARP[u4HRatio][2] << 16) + (PANEL_SCL_Y_H_MSB_SHARP[u4HRatio][3] << 24);

	/*WRITE SCALER C HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF0_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][0];
	_rSclHalFmtReg.rField.u4CH_COEF0_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][1];
	_rSclHalFmtReg.rField.u4CH_COEF0_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][2];
	_rSclHalFmtReg.rField.u4CH_COEF0_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][3];
	_rSclHalFmtReg.rField.u4CH_COEF0_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][4];
	_rSclHalFmtReg.rField.u4CH_COEF0_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][5];
	_rSclHalFmtReg.rField.u4CH_COEF0_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][6];
	_rSclHalFmtReg.rField.u4CH_COEF0_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][0][7];

	/*Phase 1, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF1_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][0];
	_rSclHalFmtReg.rField.u4CH_COEF1_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][1];
	_rSclHalFmtReg.rField.u4CH_COEF1_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][2];
	_rSclHalFmtReg.rField.u4CH_COEF1_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][3];
	_rSclHalFmtReg.rField.u4CH_COEF1_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][4];
	_rSclHalFmtReg.rField.u4CH_COEF1_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][5];
	_rSclHalFmtReg.rField.u4CH_COEF1_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][6];
	_rSclHalFmtReg.rField.u4CH_COEF1_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][1][7];

	/*Phase 2, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF2_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][0];
	_rSclHalFmtReg.rField.u4CH_COEF2_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][1];
	_rSclHalFmtReg.rField.u4CH_COEF2_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][2];
	_rSclHalFmtReg.rField.u4CH_COEF2_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][3];
	_rSclHalFmtReg.rField.u4CH_COEF2_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][4];
	_rSclHalFmtReg.rField.u4CH_COEF2_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][5];
	_rSclHalFmtReg.rField.u4CH_COEF2_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][6];
	_rSclHalFmtReg.rField.u4CH_COEF2_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][2][7];

	/*Phase 3, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF3_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][0];
	_rSclHalFmtReg.rField.u4CH_COEF3_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][1];
	_rSclHalFmtReg.rField.u4CH_COEF3_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][2];
	_rSclHalFmtReg.rField.u4CH_COEF3_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][3];
	_rSclHalFmtReg.rField.u4CH_COEF3_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][4];
	_rSclHalFmtReg.rField.u4CH_COEF3_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][5];
	_rSclHalFmtReg.rField.u4CH_COEF3_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][6];
	_rSclHalFmtReg.rField.u4CH_COEF3_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][3][7];

	/*Phase 4, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF4_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][0];
	_rSclHalFmtReg.rField.u4CH_COEF4_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][1];
	_rSclHalFmtReg.rField.u4CH_COEF4_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][2];
	_rSclHalFmtReg.rField.u4CH_COEF4_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][3];
	_rSclHalFmtReg.rField.u4CH_COEF4_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][4];
	_rSclHalFmtReg.rField.u4CH_COEF4_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][5];
	_rSclHalFmtReg.rField.u4CH_COEF4_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][6];
	_rSclHalFmtReg.rField.u4CH_COEF4_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][4][7];

	/*Phase 5, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF5_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][0];
	_rSclHalFmtReg.rField.u4CH_COEF5_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][1];
	_rSclHalFmtReg.rField.u4CH_COEF5_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][2];
	_rSclHalFmtReg.rField.u4CH_COEF5_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][3];
	_rSclHalFmtReg.rField.u4CH_COEF5_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][4];
	_rSclHalFmtReg.rField.u4CH_COEF5_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][5];
	_rSclHalFmtReg.rField.u4CH_COEF5_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][6];
	_rSclHalFmtReg.rField.u4CH_COEF5_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][5][7];

	/*Phase 6, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF6_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][1];
	_rSclHalFmtReg.rField.u4CH_COEF6_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][2];
	_rSclHalFmtReg.rField.u4CH_COEF6_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][3];
	_rSclHalFmtReg.rField.u4CH_COEF6_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][4];
	_rSclHalFmtReg.rField.u4CH_COEF6_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][5];
	_rSclHalFmtReg.rField.u4CH_COEF6_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][6];
	_rSclHalFmtReg.rField.u4CH_COEF6_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][6][7];

	/*Phase 7, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF7_0 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_1 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][1];
	_rSclHalFmtReg.rField.u4CH_COEF7_2 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][2];
	_rSclHalFmtReg.rField.u4CH_COEF7_3 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][3];
	_rSclHalFmtReg.rField.u4CH_COEF7_4 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][4];
	_rSclHalFmtReg.rField.u4CH_COEF7_5 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][5];
	_rSclHalFmtReg.rField.u4CH_COEF7_6 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][6];
	_rSclHalFmtReg.rField.u4CH_COEF7_7 = PANEL_SCL_C_H_COEF_SHARP[u4HRatio][7][7];

	/*Write MSB Registers*/
	_rSclHalFmtReg.au4Reg[0x4C / 4] =
		PANEL_SCL_C_H_MSB_SHARP[u4HRatio][0] + (PANEL_SCL_C_H_MSB_SHARP[u4HRatio][1] << 8)
		+ (PANEL_SCL_C_H_MSB_SHARP[u4HRatio][2] << 16) + (PANEL_SCL_C_H_MSB_SHARP[u4HRatio][3] << 24);

	_rSclRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x94 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x98 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x9C / 4] |= SCL_HAL_REG_MODE_WRITE;

	for (u4Idx = 0xC0; u4Idx <= 0xFC; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	for (u4Idx = 0x00; u4Idx <= 0x40; u4Idx += 4) {
		_rSclFmtRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	_rSclFmtRegMode[(0x4C >> 2)] |= SCL_HAL_REG_MODE_WRITE;
}

/* **********************************************************************/
/* Function : void vSclHalSetHCoefPhaseType(__u8 u4HRatio)*/
/* Description : Set Panel Scaler FIR Filtet Horizontal Coefficients (16/8 Odd/Even Test)*/
/* Parameter : u4HRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetHCoefPhaseType(__u8 u4HRatio)
{
	__u32 u4Idx = 0;

	/*WRITE SCALER Y HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF0_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][1];
	_rSclHalSclReg.rField.u4YH_COEF0_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][2];
	_rSclHalSclReg.rField.u4YH_COEF0_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][3];
	_rSclHalSclReg.rField.u4YH_COEF0_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][4];
	_rSclHalSclReg.rField.u4YH_COEF0_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][5];
	_rSclHalSclReg.rField.u4YH_COEF0_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][6];
	_rSclHalSclReg.rField.u4YH_COEF0_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][7];
	_rSclHalSclReg.rField.u4YH_COEF0_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][8];

	/*Phase 1, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF1_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][1];
	_rSclHalSclReg.rField.u4YH_COEF1_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][2];
	_rSclHalSclReg.rField.u4YH_COEF1_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][3];
	_rSclHalSclReg.rField.u4YH_COEF1_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][4];
	_rSclHalSclReg.rField.u4YH_COEF1_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][5];
	_rSclHalSclReg.rField.u4YH_COEF1_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][6];
	_rSclHalSclReg.rField.u4YH_COEF1_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][7];
	_rSclHalSclReg.rField.u4YH_COEF1_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][8];

	/*Phase 2, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF2_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][1];
	_rSclHalSclReg.rField.u4YH_COEF2_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][2];
	_rSclHalSclReg.rField.u4YH_COEF2_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][3];
	_rSclHalSclReg.rField.u4YH_COEF2_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][4];
	_rSclHalSclReg.rField.u4YH_COEF2_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][5];
	_rSclHalSclReg.rField.u4YH_COEF2_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][6];
	_rSclHalSclReg.rField.u4YH_COEF2_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][7];
	_rSclHalSclReg.rField.u4YH_COEF2_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][8];

	/*Phase 3, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF3_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][1];
	_rSclHalSclReg.rField.u4YH_COEF3_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][2];
	_rSclHalSclReg.rField.u4YH_COEF3_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][3];
	_rSclHalSclReg.rField.u4YH_COEF3_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][4];
	_rSclHalSclReg.rField.u4YH_COEF3_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][5];
	_rSclHalSclReg.rField.u4YH_COEF3_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][6];
	_rSclHalSclReg.rField.u4YH_COEF3_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][7];
	_rSclHalSclReg.rField.u4YH_COEF3_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][8];

	/*Phase 4, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF4_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][1];
	_rSclHalSclReg.rField.u4YH_COEF4_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][2];
	_rSclHalSclReg.rField.u4YH_COEF4_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][3];
	_rSclHalSclReg.rField.u4YH_COEF4_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][4];
	_rSclHalSclReg.rField.u4YH_COEF4_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][5];
	_rSclHalSclReg.rField.u4YH_COEF4_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][6];
	_rSclHalSclReg.rField.u4YH_COEF4_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][7];
	_rSclHalSclReg.rField.u4YH_COEF4_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][8];

	/*Phase 5, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF5_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][1];
	_rSclHalSclReg.rField.u4YH_COEF5_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][2];
	_rSclHalSclReg.rField.u4YH_COEF5_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][3];
	_rSclHalSclReg.rField.u4YH_COEF5_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][4];
	_rSclHalSclReg.rField.u4YH_COEF5_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][5];
	_rSclHalSclReg.rField.u4YH_COEF5_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][6];
	_rSclHalSclReg.rField.u4YH_COEF5_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][7];
	_rSclHalSclReg.rField.u4YH_COEF5_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][8];

	/*Phase 6, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF6_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][1];
	_rSclHalSclReg.rField.u4YH_COEF6_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][2];
	_rSclHalSclReg.rField.u4YH_COEF6_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][3];
	_rSclHalSclReg.rField.u4YH_COEF6_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][4];
	_rSclHalSclReg.rField.u4YH_COEF6_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][5];
	_rSclHalSclReg.rField.u4YH_COEF6_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][6];
	_rSclHalSclReg.rField.u4YH_COEF6_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][7];
	_rSclHalSclReg.rField.u4YH_COEF6_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][8];

	/*Phase 7, Idx 1~8*/
	_rSclHalSclReg.rField.u4YH_COEF7_0 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][1];
	_rSclHalSclReg.rField.u4YH_COEF7_1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][2];
	_rSclHalSclReg.rField.u4YH_COEF7_2 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][3];
	_rSclHalSclReg.rField.u4YH_COEF7_3 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][4];
	_rSclHalSclReg.rField.u4YH_COEF7_4 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][5];
	_rSclHalSclReg.rField.u4YH_COEF7_5 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][6];
	_rSclHalSclReg.rField.u4YH_COEF7_6 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][7];
	_rSclHalSclReg.rField.u4YH_COEF7_7 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][8];

	/*Phase 0, Idx 0, 9 & Phase 1, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF0_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][0];
	_rSclHalSclReg.rField.u4YH_COEF0_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][0][9];
	_rSclHalSclReg.rField.u4YH_COEF1_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][0];
	_rSclHalSclReg.rField.u4YH_COEF1_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][1][9];

	/*Phase 2, Idx 0, 9 & Phase 3, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF2_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][0];
	_rSclHalSclReg.rField.u4YH_COEF2_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][2][9];
	_rSclHalSclReg.rField.u4YH_COEF3_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][0];
	_rSclHalSclReg.rField.u4YH_COEF3_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][3][9];

	/*Phase 4, Idx 0, 9 & Phase 5, Idx 0, 9*/
	_rSclHalSclReg.rField.u4YH_COEF4_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][0];
	_rSclHalSclReg.rField.u4YH_COEF4_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][4][9];
	_rSclHalSclReg.rField.u4YH_COEF5_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][0];
	_rSclHalSclReg.rField.u4YH_COEF5_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][5][9];

	/*Phase 6, Idx 0, 9 & Phase 7, Idx 0, 9*/
	_rSclHalFmtReg.rField.u4CH_COEF6_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][6][9];
	_rSclHalFmtReg.rField.u4CH_COEF7_M1 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_8 = PANEL_SCL_Y_H_COEF_PHASETYPE[u4HRatio][7][9];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0xFC / 4] =
		PANEL_SCL_Y_H_MSB_PHASETYPE[u4HRatio][0] + (PANEL_SCL_Y_H_MSB_PHASETYPE[u4HRatio][1] << 8)
		+ (PANEL_SCL_Y_H_MSB_PHASETYPE[u4HRatio][2] << 16) + (PANEL_SCL_Y_H_MSB_PHASETYPE[u4HRatio][3] << 24);

	/*WRITE SCALER C HORIZONTAL COEFFICIENTS*/
	/*Phase 0, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF0_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][0];
	_rSclHalFmtReg.rField.u4CH_COEF0_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][1];
	_rSclHalFmtReg.rField.u4CH_COEF0_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][2];
	_rSclHalFmtReg.rField.u4CH_COEF0_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][3];
	_rSclHalFmtReg.rField.u4CH_COEF0_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][4];
	_rSclHalFmtReg.rField.u4CH_COEF0_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][5];
	_rSclHalFmtReg.rField.u4CH_COEF0_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][6];
	_rSclHalFmtReg.rField.u4CH_COEF0_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][0][7];

	/*Phase 1, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF1_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][0];
	_rSclHalFmtReg.rField.u4CH_COEF1_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][1];
	_rSclHalFmtReg.rField.u4CH_COEF1_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][2];
	_rSclHalFmtReg.rField.u4CH_COEF1_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][3];
	_rSclHalFmtReg.rField.u4CH_COEF1_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][4];
	_rSclHalFmtReg.rField.u4CH_COEF1_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][5];
	_rSclHalFmtReg.rField.u4CH_COEF1_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][6];
	_rSclHalFmtReg.rField.u4CH_COEF1_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][1][7];

	/*Phase 2, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF2_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][0];
	_rSclHalFmtReg.rField.u4CH_COEF2_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][1];
	_rSclHalFmtReg.rField.u4CH_COEF2_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][2];
	_rSclHalFmtReg.rField.u4CH_COEF2_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][3];
	_rSclHalFmtReg.rField.u4CH_COEF2_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][4];
	_rSclHalFmtReg.rField.u4CH_COEF2_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][5];
	_rSclHalFmtReg.rField.u4CH_COEF2_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][6];
	_rSclHalFmtReg.rField.u4CH_COEF2_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][2][7];

	/*Phase 3, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF3_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][0];
	_rSclHalFmtReg.rField.u4CH_COEF3_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][1];
	_rSclHalFmtReg.rField.u4CH_COEF3_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][2];
	_rSclHalFmtReg.rField.u4CH_COEF3_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][3];
	_rSclHalFmtReg.rField.u4CH_COEF3_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][4];
	_rSclHalFmtReg.rField.u4CH_COEF3_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][5];
	_rSclHalFmtReg.rField.u4CH_COEF3_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][6];
	_rSclHalFmtReg.rField.u4CH_COEF3_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][3][7];

	/*Phase 4, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF4_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][0];
	_rSclHalFmtReg.rField.u4CH_COEF4_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][1];
	_rSclHalFmtReg.rField.u4CH_COEF4_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][2];
	_rSclHalFmtReg.rField.u4CH_COEF4_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][3];
	_rSclHalFmtReg.rField.u4CH_COEF4_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][4];
	_rSclHalFmtReg.rField.u4CH_COEF4_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][5];
	_rSclHalFmtReg.rField.u4CH_COEF4_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][6];
	_rSclHalFmtReg.rField.u4CH_COEF4_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][4][7];

	/*Phase 5, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF5_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][0];
	_rSclHalFmtReg.rField.u4CH_COEF5_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][1];
	_rSclHalFmtReg.rField.u4CH_COEF5_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][2];
	_rSclHalFmtReg.rField.u4CH_COEF5_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][3];
	_rSclHalFmtReg.rField.u4CH_COEF5_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][4];
	_rSclHalFmtReg.rField.u4CH_COEF5_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][5];
	_rSclHalFmtReg.rField.u4CH_COEF5_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][6];
	_rSclHalFmtReg.rField.u4CH_COEF5_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][5][7];

	/*Phase 6, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF6_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][0];
	_rSclHalFmtReg.rField.u4CH_COEF6_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][1];
	_rSclHalFmtReg.rField.u4CH_COEF6_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][2];
	_rSclHalFmtReg.rField.u4CH_COEF6_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][3];
	_rSclHalFmtReg.rField.u4CH_COEF6_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][4];
	_rSclHalFmtReg.rField.u4CH_COEF6_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][5];
	_rSclHalFmtReg.rField.u4CH_COEF6_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][6];
	_rSclHalFmtReg.rField.u4CH_COEF6_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][6][7];

	/*Phase 7, Idx 0~7*/
	_rSclHalFmtReg.rField.u4CH_COEF7_0 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][0];
	_rSclHalFmtReg.rField.u4CH_COEF7_1 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][1];
	_rSclHalFmtReg.rField.u4CH_COEF7_2 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][2];
	_rSclHalFmtReg.rField.u4CH_COEF7_3 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][3];
	_rSclHalFmtReg.rField.u4CH_COEF7_4 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][4];
	_rSclHalFmtReg.rField.u4CH_COEF7_5 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][5];
	_rSclHalFmtReg.rField.u4CH_COEF7_6 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][6];
	_rSclHalFmtReg.rField.u4CH_COEF7_7 = PANEL_SCL_C_H_COEF_PHASETYPE[u4HRatio][7][7];

	/*Write MSB Registers*/
	_rSclHalFmtReg.au4Reg[0x4C / 4] =
		PANEL_SCL_C_H_MSB_PHASETYPE[u4HRatio][0] + (PANEL_SCL_C_H_MSB_PHASETYPE[u4HRatio][1] << 8)
		+ (PANEL_SCL_C_H_MSB_PHASETYPE[u4HRatio][2] << 16) + (PANEL_SCL_C_H_MSB_PHASETYPE[u4HRatio][3] << 24);

	_rSclRegMode[0x90 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x94 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x98 / 4] |= SCL_HAL_REG_MODE_WRITE;
	_rSclRegMode[0x9C / 4] |= SCL_HAL_REG_MODE_WRITE;

	for (u4Idx = 0xC0; u4Idx <= 0xFC; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	for (u4Idx = 0x00; u4Idx <= 0x40; u4Idx += 4) {
		_rSclFmtRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}

	_rSclFmtRegMode[(0x4C >> 2)] |= SCL_HAL_REG_MODE_WRITE;
}

/* **********************************************************************/
/* Function : void vSclCoef_V (__u8 u4VRatio)*/
/* Description : Set Panel Scaler FIR Filtet Vertical Coefficients*/
/* Parameter : u4VRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetVCoef(__u32 u4VRatio)
{
	__u32 u4Idx = 0;

	if (u4VRatio >= V_FILTER_NO) {
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] vSclHalSetVCoef param err %d\r\n", (int)u4VRatio);
		return;
	}

	/*WRITE SCALER Y VERTICAL COEFFICIENTS*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF0M1 = PANEL_SCL_Y_V_COEF[u4VRatio][0][0];
	_rSclHalSclReg.rField.u4YV_COEF0_0 = PANEL_SCL_Y_V_COEF[u4VRatio][0][1];
	_rSclHalSclReg.rField.u4YV_COEF0_1 = PANEL_SCL_Y_V_COEF[u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF1M1 = PANEL_SCL_Y_V_COEF[u4VRatio][1][0];
	_rSclHalSclReg.rField.u4YV_COEF1_0 = PANEL_SCL_Y_V_COEF[u4VRatio][1][1];
	_rSclHalSclReg.rField.u4YV_COEF1_1 = PANEL_SCL_Y_V_COEF[u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF2M1 = PANEL_SCL_Y_V_COEF[u4VRatio][2][0];
	_rSclHalSclReg.rField.u4YV_COEF2_0 = PANEL_SCL_Y_V_COEF[u4VRatio][2][1];
	_rSclHalSclReg.rField.u4YV_COEF2_1 = PANEL_SCL_Y_V_COEF[u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF3M1 = PANEL_SCL_Y_V_COEF[u4VRatio][3][0];
	_rSclHalSclReg.rField.u4YV_COEF3_0 = PANEL_SCL_Y_V_COEF[u4VRatio][3][1];
	_rSclHalSclReg.rField.u4YV_COEF3_1 = PANEL_SCL_Y_V_COEF[u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF4M1 = PANEL_SCL_Y_V_COEF[u4VRatio][4][0];
	_rSclHalSclReg.rField.u4YV_COEF4_0 = PANEL_SCL_Y_V_COEF[u4VRatio][4][1];
	_rSclHalSclReg.rField.u4YV_COEF4_1 = PANEL_SCL_Y_V_COEF[u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF5M1 = PANEL_SCL_Y_V_COEF[u4VRatio][5][0];
	_rSclHalSclReg.rField.u4YV_COEF5_0 = PANEL_SCL_Y_V_COEF[u4VRatio][5][1];
	_rSclHalSclReg.rField.u4YV_COEF5_1 = PANEL_SCL_Y_V_COEF[u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF6M1 = PANEL_SCL_Y_V_COEF[u4VRatio][6][0];
	_rSclHalSclReg.rField.u4YV_COEF6_0 = PANEL_SCL_Y_V_COEF[u4VRatio][6][1];
	_rSclHalSclReg.rField.u4YV_COEF6_1 = PANEL_SCL_Y_V_COEF[u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF7M1 = PANEL_SCL_Y_V_COEF[u4VRatio][7][0];
	_rSclHalSclReg.rField.u4YV_COEF7_0 = PANEL_SCL_Y_V_COEF[u4VRatio][7][1];
	_rSclHalSclReg.rField.u4YV_COEF7_1 = PANEL_SCL_Y_V_COEF[u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x80 / 4] =
		PANEL_SCL_Y_V_MSB[u4VRatio][0] + (PANEL_SCL_Y_V_MSB[u4VRatio][1] << 8)
		+ (PANEL_SCL_Y_V_MSB[u4VRatio][2] << 16) + (PANEL_SCL_Y_V_MSB[u4VRatio][3] << 24);

	/*WRITE SCALER C VERTICAL COEFFICIENTS*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF0M1 = PANEL_SCL_C_V_COEF[u4VRatio][0][0];
	_rSclHalSclReg.rField.u4CV_COEF0_0 = PANEL_SCL_C_V_COEF[u4VRatio][0][1];
	_rSclHalSclReg.rField.u4CV_COEF0_1 = PANEL_SCL_C_V_COEF[u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF1M1 = PANEL_SCL_C_V_COEF[u4VRatio][1][0];
	_rSclHalSclReg.rField.u4CV_COEF1_0 = PANEL_SCL_C_V_COEF[u4VRatio][1][1];
	_rSclHalSclReg.rField.u4CV_COEF1_1 = PANEL_SCL_C_V_COEF[u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF2M1 = PANEL_SCL_C_V_COEF[u4VRatio][2][0];
	_rSclHalSclReg.rField.u4CV_COEF2_0 = PANEL_SCL_C_V_COEF[u4VRatio][2][1];
	_rSclHalSclReg.rField.u4CV_COEF2_1 = PANEL_SCL_C_V_COEF[u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF3M1 = PANEL_SCL_C_V_COEF[u4VRatio][3][0];
	_rSclHalSclReg.rField.u4CV_COEF3_0 = PANEL_SCL_C_V_COEF[u4VRatio][3][1];
	_rSclHalSclReg.rField.u4CV_COEF3_1 = PANEL_SCL_C_V_COEF[u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF4M1 = PANEL_SCL_C_V_COEF[u4VRatio][4][0];
	_rSclHalSclReg.rField.u4CV_COEF4_0 = PANEL_SCL_C_V_COEF[u4VRatio][4][1];
	_rSclHalSclReg.rField.u4CV_COEF4_1 = PANEL_SCL_C_V_COEF[u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF5M1 = PANEL_SCL_C_V_COEF[u4VRatio][5][0];
	_rSclHalSclReg.rField.u4CV_COEF5_0 = PANEL_SCL_C_V_COEF[u4VRatio][5][1];
	_rSclHalSclReg.rField.u4CV_COEF5_1 = PANEL_SCL_C_V_COEF[u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF6M1 = PANEL_SCL_C_V_COEF[u4VRatio][6][0];
	_rSclHalSclReg.rField.u4CV_COEF6_0 = PANEL_SCL_C_V_COEF[u4VRatio][6][1];
	_rSclHalSclReg.rField.u4CV_COEF6_1 = PANEL_SCL_C_V_COEF[u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF7M1 = PANEL_SCL_C_V_COEF[u4VRatio][7][0];
	_rSclHalSclReg.rField.u4CV_COEF7_0 = PANEL_SCL_C_V_COEF[u4VRatio][7][1];
	_rSclHalSclReg.rField.u4CV_COEF7_1 = PANEL_SCL_C_V_COEF[u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x84 / 4] =
		PANEL_SCL_C_V_MSB[u4VRatio][0] + (PANEL_SCL_C_V_MSB[u4VRatio][1] << 8)
		+ (PANEL_SCL_C_V_MSB[u4VRatio][2] << 16) + (PANEL_SCL_C_V_MSB[u4VRatio][3] << 24);

	for (u4Idx = 0x40; u4Idx <= 0x84; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}
}

/* **********************************************************************/
/* Function : void vSclCoef_V_Sharpness (__u8 u4VRatio)*/
/* Description : Set Panel Scaler FIR Filtet Vertical Coefficients (Sharpness Adjustment)*/
/* Parameter : u4VRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetVCoefSharpness(__u8 u4VRatio, bool fgNtsc)
{
	__u32 u4Idx = 0;

	if (u4VRatio >= V_FILTER_NO) {
		VDO_LOG(VDO_LOG_LVL_ERR, "[SCL] vSclHalSetVCoef param err %d\r\n", (int)u4VRatio);
		return;
	}

	/*WRITE SCALER Y VERTICAL COEFFICIENTS, 480P/480I - fgNtsc = 0, 576P/576I - fgNtsc = 1*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF0M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][0][0];
	_rSclHalSclReg.rField.u4YV_COEF0_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][0][1];
	_rSclHalSclReg.rField.u4YV_COEF0_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF1M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][1][0];
	_rSclHalSclReg.rField.u4YV_COEF1_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][1][1];
	_rSclHalSclReg.rField.u4YV_COEF1_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF2M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][2][0];
	_rSclHalSclReg.rField.u4YV_COEF2_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][2][1];
	_rSclHalSclReg.rField.u4YV_COEF2_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF3M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][3][0];
	_rSclHalSclReg.rField.u4YV_COEF3_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][3][1];
	_rSclHalSclReg.rField.u4YV_COEF3_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF4M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][4][0];
	_rSclHalSclReg.rField.u4YV_COEF4_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][4][1];
	_rSclHalSclReg.rField.u4YV_COEF4_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF5M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][5][0];
	_rSclHalSclReg.rField.u4YV_COEF5_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][5][1];
	_rSclHalSclReg.rField.u4YV_COEF5_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF6M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][6][0];
	_rSclHalSclReg.rField.u4YV_COEF6_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][6][1];
	_rSclHalSclReg.rField.u4YV_COEF6_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF7M1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][7][0];
	_rSclHalSclReg.rField.u4YV_COEF7_0 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][7][1];
	_rSclHalSclReg.rField.u4YV_COEF7_1 = PANEL_SCL_Y_V_COEF_SHARP[fgNtsc][u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x80 / 4] = PANEL_SCL_Y_V_MSB_SHARP[fgNtsc][u4VRatio][0]
					  + (PANEL_SCL_Y_V_MSB_SHARP[fgNtsc][u4VRatio][1] << 8)
					  + (PANEL_SCL_Y_V_MSB_SHARP[fgNtsc][u4VRatio][2] << 16)
					  + (PANEL_SCL_Y_V_MSB_SHARP[fgNtsc][u4VRatio][3] << 24);

	/*WRITE SCALER C VERTICAL COEFFICIENTS*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF0M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][0][0];
	_rSclHalSclReg.rField.u4CV_COEF0_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][0][1];
	_rSclHalSclReg.rField.u4CV_COEF0_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF1M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][1][0];
	_rSclHalSclReg.rField.u4CV_COEF1_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][1][1];
	_rSclHalSclReg.rField.u4CV_COEF1_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF2M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][2][0];
	_rSclHalSclReg.rField.u4CV_COEF2_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][2][1];
	_rSclHalSclReg.rField.u4CV_COEF2_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF3M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][3][0];
	_rSclHalSclReg.rField.u4CV_COEF3_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][3][1];
	_rSclHalSclReg.rField.u4CV_COEF3_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF4M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][4][0];
	_rSclHalSclReg.rField.u4CV_COEF4_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][4][1];
	_rSclHalSclReg.rField.u4CV_COEF4_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF5M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][5][0];
	_rSclHalSclReg.rField.u4CV_COEF5_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][5][1];
	_rSclHalSclReg.rField.u4CV_COEF5_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF6M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][6][0];
	_rSclHalSclReg.rField.u4CV_COEF6_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][6][1];
	_rSclHalSclReg.rField.u4CV_COEF6_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF7M1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][7][0];
	_rSclHalSclReg.rField.u4CV_COEF7_0 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][7][1];
	_rSclHalSclReg.rField.u4CV_COEF7_1 = PANEL_SCL_C_V_COEF_SHARP[fgNtsc][u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x84 / 4] = PANEL_SCL_C_V_MSB_SHARP[fgNtsc][u4VRatio][0]
					  + (PANEL_SCL_C_V_MSB_SHARP[fgNtsc][u4VRatio][1] << 8)
					  + (PANEL_SCL_C_V_MSB_SHARP[fgNtsc][u4VRatio][2] << 16)
					  + (PANEL_SCL_C_V_MSB_SHARP[fgNtsc][u4VRatio][3] << 24);

	for (u4Idx = 0x40; u4Idx <= 0x84; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}
}

/* **********************************************************************/
/* Function : void vSclCoef_V_PhaseType (__u8 u4VRatio)*/
/* Description : Set Panel Scaler FIR Filtet Vertical Coefficients (16/8 Odd/Even Test)*/
/* Parameter : u4VRatio : Filter Types*/
/* Return    : None*/
/* **********************************************************************/
void vSclHalSetVCoefPhaseType(__u8 u4VRatio, bool fgNtsc)
{
	__u32 u4Idx = 0;

	/*WRITE SCALER Y VERTICAL COEFFICIENTS, 480P/480I - fgNtsc = 0, 576P/576I - fgNtsc = 1*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF0M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][0];
	_rSclHalSclReg.rField.u4YV_COEF0_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][1];
	_rSclHalSclReg.rField.u4YV_COEF0_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF1M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][0];
	_rSclHalSclReg.rField.u4YV_COEF1_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][1];
	_rSclHalSclReg.rField.u4YV_COEF1_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF2M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][0];
	_rSclHalSclReg.rField.u4YV_COEF2_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][1];
	_rSclHalSclReg.rField.u4YV_COEF2_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF3M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][0];
	_rSclHalSclReg.rField.u4YV_COEF3_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][1];
	_rSclHalSclReg.rField.u4YV_COEF3_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF4M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][0];
	_rSclHalSclReg.rField.u4YV_COEF4_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][1];
	_rSclHalSclReg.rField.u4YV_COEF4_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF5M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][0];
	_rSclHalSclReg.rField.u4YV_COEF5_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][1];
	_rSclHalSclReg.rField.u4YV_COEF5_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF6M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][0];
	_rSclHalSclReg.rField.u4YV_COEF6_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][1];
	_rSclHalSclReg.rField.u4YV_COEF6_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4YV_COEF7M1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][0];
	_rSclHalSclReg.rField.u4YV_COEF7_0 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][1];
	_rSclHalSclReg.rField.u4YV_COEF7_1 = PANEL_SCL_Y_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x80 / 4] = PANEL_SCL_Y_V_MSB_PHASETYPE[fgNtsc][u4VRatio][0]
					  + (PANEL_SCL_Y_V_MSB_PHASETYPE[fgNtsc][u4VRatio][1] << 8)
					  + (PANEL_SCL_Y_V_MSB_PHASETYPE[fgNtsc][u4VRatio][2] << 16)
					  + (PANEL_SCL_Y_V_MSB_PHASETYPE[fgNtsc][u4VRatio][3] << 24);

	/*WRITE SCALER C VERTICAL COEFFICIENTS*/
	/*Phase 0, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF0M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][0];
	_rSclHalSclReg.rField.u4CV_COEF0_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][1];
	_rSclHalSclReg.rField.u4CV_COEF0_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][0][2];

	/*Phase 1, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF1M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][0];
	_rSclHalSclReg.rField.u4CV_COEF1_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][1];
	_rSclHalSclReg.rField.u4CV_COEF1_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][1][2];

	/*Phase 2, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF2M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][0];
	_rSclHalSclReg.rField.u4CV_COEF2_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][1];
	_rSclHalSclReg.rField.u4CV_COEF2_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][2][2];

	/*Phase 3, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF3M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][0];
	_rSclHalSclReg.rField.u4CV_COEF3_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][1];
	_rSclHalSclReg.rField.u4CV_COEF3_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][3][2];

	/*Phase 4, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF4M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][0];
	_rSclHalSclReg.rField.u4CV_COEF4_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][1];
	_rSclHalSclReg.rField.u4CV_COEF4_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][4][2];

	/*Phase 5, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF5M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][0];
	_rSclHalSclReg.rField.u4CV_COEF5_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][1];
	_rSclHalSclReg.rField.u4CV_COEF5_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][5][2];

	/*Phase 6, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF6M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][0];
	_rSclHalSclReg.rField.u4CV_COEF6_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][1];
	_rSclHalSclReg.rField.u4CV_COEF6_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][6][2];

	/*Phase 7, Idx 0~2*/
	_rSclHalSclReg.rField.u4CV_COEF7M1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][0];
	_rSclHalSclReg.rField.u4CV_COEF7_0 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][1];
	_rSclHalSclReg.rField.u4CV_COEF7_1 = PANEL_SCL_C_V_COEF_PHASETYPE[fgNtsc][u4VRatio][7][2];

	/*Write MSB Registers*/
	_rSclHalSclReg.au4Reg[0x84 / 4] = PANEL_SCL_C_V_MSB_PHASETYPE[fgNtsc][u4VRatio][0]
					  + (PANEL_SCL_C_V_MSB_PHASETYPE[fgNtsc][u4VRatio][1] << 8)
					  + (PANEL_SCL_C_V_MSB_PHASETYPE[fgNtsc][u4VRatio][2] << 16)
					  + (PANEL_SCL_C_V_MSB_PHASETYPE[fgNtsc][u4VRatio][3] << 24);

	for (u4Idx = 0x40; u4Idx <= 0x84; u4Idx += 4) {
		_rSclRegMode[(u4Idx >> 2)] |= SCL_HAL_REG_MODE_WRITE;
	}
}

void vSclDumpReg(void)
{
#ifndef __ARM2__
	__u32 u4Addr;

	VDO_LOG(VDO_LOG_LVL_DBG, "[SCL] SCL Dump SCL and FMT Registry\n");

	for (u4Addr = 0x10; u4Addr < 0x100; u4Addr += 4) {
		VDO_LOG(VDO_LOG_LVL_DBG, "[0x%08x] = 0x%08x ", (unsigned int)(scl_reg + u4Addr)
			, (unsigned int)_prSclHwReg->au4Reg[u4Addr]);

		if (((u4Addr + 4) % 16) == 0) {
			VDO_LOG(VDO_LOG_LVL_DBG, "\r\n");
		}
	}

	for (u4Addr = 0x0; u4Addr < 0xEC; u4Addr += 4) {
		VDO_LOG(VDO_LOG_LVL_DBG, "[0x%08x] = 0x%08x ", (unsigned int)(sclf_reg + u4Addr)
			, (unsigned int)_prSclFmtHwReg->au4Reg[u4Addr]);

		if (((u4Addr + 4) % 16) == 0) {
			VDO_LOG(VDO_LOG_LVL_DBG, "\r\n");
		}
	}
#endif
}

irqreturn_t vSclHalIsr(int u2Vector, void *dev_id)
{
	__u32 u4RegIdx;

	if (_fgWaitVBIEvent) {
//		x_event_set(_hVBEvent);//for 8237
		_fgWaitVBIEvent = FALSE;
	}

	VDO_LOG(VDO_LOG_LVL_IRQ, "[SCL] scaler irq isr %d\r\n", u2Vector);
	
	vPmxHalMixIsr();

	/* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < SCL_HAL_SCL_REG_NUM; u4RegIdx++) {
		if (_rSclRegMode[u4RegIdx] & SCL_HAL_REG_MODE_WRITE) {
			_prSclHwReg->au4Reg[u4RegIdx] = _rSclHalSclReg.au4Reg[u4RegIdx];
			_rSclRegMode[u4RegIdx] &= ~SCL_HAL_REG_MODE_WRITE;
		}

		if (_rSclRegMode[u4RegIdx] & SCL_HAL_REG_MODE_READ) {
			_rSclHalSclReg.au4Reg[u4RegIdx] = _prSclHwReg->au4Reg[u4RegIdx];
		}
	}

	for (u4RegIdx = 0; u4RegIdx < SCL_HAL_FMT_REG_NUM; u4RegIdx++) {
		if (_rSclFmtRegMode[u4RegIdx] & SCL_HAL_REG_MODE_WRITE) {
			_prSclFmtHwReg->au4Reg[u4RegIdx] = _rSclHalFmtReg.au4Reg[u4RegIdx];
			_rSclFmtRegMode[u4RegIdx] &= ~SCL_HAL_REG_MODE_WRITE;
		}

		if (_rSclFmtRegMode[u4RegIdx] & SCL_HAL_REG_MODE_READ) {
			_rSclHalFmtReg.au4Reg[u4RegIdx] = _prSclFmtHwReg->au4Reg[u4RegIdx];
		}
	}

//	ac83xx_mask_ack_bim_irq(u2Vector);//for 8237

	return IRQ_HANDLED;
}
__u32 vSclInVertBlank(void)
{
	__u32 ret = 0;

	_fgWaitVBIEvent = TRUE;

//	ret = x_event_wait_for_objects(1, &_hVBEvent, FALSE, 0xFFFFFFFFU);//for 8237

	return ret;
}

extern void PmxVerifyDrvClkInit(unsigned char ucVdoId);
extern void vPmxVerifyPanelSizeSel(unsigned int u4VdoId,unsigned int u4PmxFmt,UCHAR ucTvType);
extern void vPmxVerifyHalSysInit(void);
extern void PmxVerifySclerSetup(unsigned char ucVdoId, unsigned int ucPmxMode);

void vSclHalResume(void)
{
	PmxVerifyDrvClkInit(0);
	vPmxVerifyPanelSizeSel(0, 600, 13);
	vPmxVerifyHalSysInit();
	PmxVerifySclerSetup(0, RES_600P_1024);
}
#endif /*_SCL_HAL_C_*/



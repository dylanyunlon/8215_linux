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
#include <linux/module.h>
#include <media/atc/display_inc.h>
#else
#include "display_inc.h"
#endif
#include "scl_hal.h"
#include "scl_if.h"
#include "log.h"

void SCL_Init(bool fgHwReset)
{
	vSclHalInit(fgHwReset);
}
EXPORT_SYMBOL(SCL_Init);

void  SCL_Config(__u32 u4OutputWidth, __u32  u4OutputHeight)
{
	if (u4OutputWidth == 800 && u4OutputHeight == 480) {
		vSclHalSetMode(SCL_IN_480P, SCL_OUT_800_480);
#if MASTER_MODE_ENABLE
		vSclHalSetMasterMode(TRUE, SCL_IN_480P, SCL_OUT_800_480);
#else
		vSclHalSetMasterMode(FALSE, SCL_IN_480P, SCL_OUT_800_480);
#endif
		vSclHalSetHCoef(H_720_800);
		vSclHalSetVCoef(V_480_480);
	} else if (u4OutputWidth == 800 && u4OutputHeight == 600) {
		vSclHalSetMode(SCL_IN_480P, SCL_OUT_800_600);
#if MASTER_MODE_ENABLE
		vSclHalSetMasterMode(TRUE, SCL_IN_480P, SCL_OUT_800_600);
#else
		vSclHalSetMasterMode(FALSE, SCL_IN_480P, SCL_OUT_800_600);
#endif
		vSclHalSetHCoef(H_720_800);
	} else if (u4OutputWidth == 1024 && u4OutputHeight == 600) {
		vSclHalSetMode(SCL_IN_480P, SCL_OUT_1024_600);
#if MASTER_MODE_ENABLE
		vSclHalSetMasterMode(TRUE, SCL_IN_480P, SCL_OUT_1024_600);
#else
		vSclHalSetMasterMode(FALSE, SCL_IN_480P, SCL_OUT_1024_600);
#endif
		vSclHalSetHCoef(H_720_1024);        
	} else if (u4OutputWidth == 1024 && u4OutputHeight == 768) {
		vSclHalSetMode(SCL_IN_480P, SCL_OUT_1024_768);
#if MASTER_MODE_ENABLE
		vSclHalSetMasterMode(TRUE, SCL_IN_480P, SCL_OUT_1024_768);
#else
		vSclHalSetMasterMode(FALSE, SCL_IN_480P, SCL_OUT_1024_768);
#endif
		vSclHalSetHCoef(H_720_1024);        
	} else if (u4OutputWidth == 1280 && u4OutputHeight == 800) {
		vSclHalSetMode(SCL_IN_480P, SCL_OUT_1280_800);
#if MASTER_MODE_ENABLE
		vSclHalSetMasterMode(TRUE, SCL_IN_480P, SCL_OUT_1280_800);
#else
		vSclHalSetMasterMode(FALSE, SCL_IN_480P, SCL_OUT_1280_800);
#endif
		vSclHalSetHCoef(H_720_1280);        
	} else {
		VDO_LOG(VDO_LOG_LVL_ERR, "SCL_Config: unknown type width %d, height %d\r\n"
			, (int)u4OutputWidth, (int)u4OutputHeight);
	}
}
EXPORT_SYMBOL(SCL_Config);















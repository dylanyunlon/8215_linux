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
#include <linux/mm.h>
#include <linux/module.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include <media/atc/pmx_hal.h>
#else
#include "x_types.h"
#include "drv_av_d.h"
#include "display_inc.h"
#include "pmx_hal.h"
#endif
#include "log.h"
#include "pmx_vfy_drv.h"
#include "osd_inc.h"

void PMX_Init(bool fgHwReset)
{
	__u32 u4DispMode[2] = {
		RES_480P,
		_u4RearOutputMode
	};
	__u32 u4TvType = PMX_TV_TYPE_NTSC;
	__u32 u4Idx = 0;

	PmxVerifyDrvInit(fgHwReset);

	if (fgHwReset) {
		for (u4Idx = PMX_1; u4Idx <= PMX_2; u4Idx++) {
			FB_PRINT(FB_LOG_LVL_DBG, "FMT", "PMX_Init: HW id = %d, u4DispMode = %d\r\n", (int)u4Idx
				, (int)u4DispMode[u4Idx]);
			vPmxHalSetMode(u4Idx, u4DispMode[u4Idx]);
			u4TvType = (u4DispMode[u4Idx] == RES_480P) ? PMX_TV_TYPE_NTSC : PMX_TV_TYPE_PAL;
			vPmxHalSetTvType(u4Idx, u4TvType);
			vPmxHalSet709To601(u4Idx, FALSE, FALSE);
			vPmxHalSetFullRange(u4Idx, TRUE, TRUE); /* video normal range and convert to full range*/
			vPmxHalReset(u4Idx);
		}

		vPmxHalSetPlaneOrder(PMX_1, PMX_PLANE_ORDER);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
#ifdef CONFIG_ATC_OS_linux
		//vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3);
#else
#ifndef MAINSURFACE_OSD2
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3);
#endif
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);
#endif
		FB_PRINT(FB_LOG_LVL_INFO, "PMX", "PMX_Init OSD1 is %d\r\n", (int)fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3));
		
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_5);
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_6);
	}

	/*vPmxHalSetPlaneOrder(PMX_1, PMX_PLANE_ORDER_LINUX);*/
}
EXPORT_SYMBOL(PMX_Init);





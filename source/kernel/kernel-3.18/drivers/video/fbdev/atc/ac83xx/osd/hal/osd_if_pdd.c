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
#include <linux/kernel.h>
#include <media/atc/drv_osd_if.h>
#include <ceddk.h>
#include "x_debug.h"
#include "windef.h"
#else
#include "drv_osd_if.h"
#endif

#include "x_os.h"
#include "osd_inc.h"
#include "osd_hw.h"
#include "osd_if_pdd.h"
#include "log.h"

/*add by mtk94020*/
#define DEFINE_IS_LOG   OSD_IsLog

enum EGPEFormat {
	gpe1Bpp,
	gpe2Bpp,
	gpe4Bpp,
	gpe8Bpp,
	gpe16Bpp,
	gpe24Bpp,
	gpe32Bpp,
	gpe16YCrCb,
	gpeDeviceCompatible,
	gpeUndefined
};

__s32  i4OsdVfyCreateSemaphores(void)
{
	return OSD_RET_OK;
}
EXPORT_SYMBOL(i4OsdVfyCreateSemaphores);


__s32  i4OsdVfyDeleteSemaphores(void)
{

	return OSD_RET_OK;
}
EXPORT_SYMBOL(i4OsdVfyDeleteSemaphores);


__s32 i4OsdSetDisplayMode(__u32 u4Plane, __u32 u4DispMode)
{

	__u32 i4Ret = OSD_RET_OK;

	OSD_VERIFY_PLANE(u4Plane);

	if (u4Plane <= OSD_PLANE_5) {
		i4Ret |= OSD_BASE_SetDisplayMode(u4DispMode);
		_OSD_BASE_Update(OSD_BASE_MAIN);
	} else {
		i4Ret |= OSD_R_BASE_SetDisplayMode(u4DispMode);
		_OSD_BASE_Update(OSD_BASE_REAR);
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4OsdSetDisplayMode);

__s32   i4OSDRestoreHwReg(void)
{
	__s32 i;

	_OSD_BASE_Update(OSD_BASE_MAIN);
	_OSD_BASE_Update(OSD_BASE_REAR);


	for (i = OSD_PLANE_1; i <= OSD_PLANE_8; i++) {
		_OSD_PLA_UpdateHwReg(i);
		_OSD_SC_UpdateHwReg(i);
	}

	*((__u32 *)(IO_BASE_VA + 0x10)) |= 0x1 << 28;
	*((__u32 *)(IO_BASE_VA + 0xd4)) |= 0xffff << 16;
	return TRUE;
}



__s32 i4OSDSaveHwReg(void)
{
	/*   memcpy( _au4Mem00To100, (void *)IO_BASE_VA, 0x100);*/

	return TRUE;
}

__s32 i4OsdPlaneEnble(__u32 u4Plane, __u32 fgEnble)
{
	__u32 i4Ret = OSD_RET_OK;

	OSD_PLA_Enable(u4Plane, fgEnble);

	return i4Ret;
}
EXPORT_SYMBOL(i4OsdPlaneEnble);

__s32 i4OsdPlaneUpdate(__u32 u4Plane)
{
	return TRUE;
}



__s32 i4OsdPlaneFlipTo(__u32 u4Plane, __u32 u4RgnList)
{

	__u32 i4Ret = OSD_RET_OK;
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD", "i4OsdPlaneFlipTo begin\n");
#endif
	OSD_PLA_FlipTo(u4Plane, u4RgnList);
	_OSD_PLA_UpdateHwReg(u4Plane);
#ifdef FB_DEBUG
	FB_PRINT(FB_LOG_LVL_DBG, "OSD", "plane flip to %d \r\n", u4Plane);
#endif

	return i4Ret;
}
EXPORT_SYMBOL(i4OsdPlaneFlipTo);



__u32 u4PixelFormatToOSDColorMode(__u32 u4PixelFormat)
{
	OSD_COLOR_MODE_T rOSDColorMode = 0;

	switch (u4PixelFormat) {
	case gpe16Bpp:
		rOSDColorMode = OSD_CM_RGB565_DIRECT16;
		break;

	case gpe32Bpp:
		rOSDColorMode = OSD_CM_ARGB8888_DIRECT32;
		break;

	default:
		FB_PRINT(FB_LOG_LVL_DBG, "OSD", "OSD does not support the pre-defined pixel format \r\n");
		break;

	}

	return rOSDColorMode;
}

bool IsOSDSupportHWScaler(__u32 u4Plane)
{
	bool fgHwScaler = FALSE;

	switch (u4Plane) {
	case OSD_PLANE_2:
		/*    case OSD_PLANE_3:*/
		fgHwScaler = FALSE;
		break;

	default:
		fgHwScaler = FALSE;
		break;


	}

	return fgHwScaler;
}






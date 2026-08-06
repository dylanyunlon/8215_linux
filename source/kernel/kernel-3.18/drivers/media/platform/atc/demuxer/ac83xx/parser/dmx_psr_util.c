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

/*!
 * @file dmx_psr_util.c
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#endif /* __linux__*/

#include "dmx_pvr.h"
#include "dmx_psr_util.h"

bool PSR_IsNonHdrVideoType(VCodeC eVCodeC)
{
	switch (eVCodeC) {
	case VC_DIVX3:
	case VC_RV30:
	case VC_RV40:
	case VC_H263_SORENSON:
	case VC_MJPEG:
	case VC_VP6:
	case VC_VP6A:
	case VC_VP8:
	case VC_WMV1:
	case VC_WMV2:
	case VC_WMV3:
		return TRUE;
	default:
		break;
	}

	return FALSE;
}


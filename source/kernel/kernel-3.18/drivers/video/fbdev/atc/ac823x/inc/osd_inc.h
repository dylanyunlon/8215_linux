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
#ifndef _OSD_INC_H
#define _OSD_INC_H

#ifndef __ARM2__
#include <media/atc/ac823x/pmx_hal.h>
#else
#include "pmx_hal.h"
#endif

#define INVALID_RGN     (-1)

#ifdef MAINSURFACE_OSD2
enum {
	PRIMARY_SURF_ID = OSD_PLANE_2,
	CURSOR_SURF_ID  = OSD_PLANE_5,
};
#else
enum {
	PRIMARY_SURF_ID = OSD_PLANE_1,
	CURSOR_SURF_ID  = OSD_PLANE_5,
};
#endif


typedef struct {
	__u32 u4DataPa;
} MTK2_INFO_T;


#ifdef __cplusplus
extern "C" {
#endif

void SetPlaneRgn(__u32 u4Plane, __u32 u4Rgn);
__u32 GetPlaneRgn(__u32 u4Plane);



#ifdef __cplusplus
}
#endif




#endif



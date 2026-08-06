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
#ifndef _SURFACE_H

#define _SURFACE_H

#include <generated/atc_project.h>


typedef struct _MTKSurface MTKSurface;

typedef struct _MTKSurface *PMTKSurface;


PMTKSurface AllocSurface(__u32 u4Width, __u32 u4Height);

PMTKSurface AllocRoateSurface(UINT32 u4Width,UINT32 u4Height);

typedef struct _MTKSurface {
	__u32 u4Width;
	__u32 u4Height;
	__u32 u4Va;
	__u32 u4Pa;
	void (*SetPicSize)(MTKSurface *pSurf, __u32 u4Width, __u32 u4Height);
	__u32 (*VirtualAddress)(MTKSurface *pSurf);
	__u32 (*PhysicalAddress)(MTKSurface *pSurf);
	__u32 (*GetCbCrlAddress)(MTKSurface *pSurf);
	__u32 (*Width)(MTKSurface *pSurf);
	__u32 (*Height)(MTKSurface *pSurf);
} MTKSurface_T;



#endif



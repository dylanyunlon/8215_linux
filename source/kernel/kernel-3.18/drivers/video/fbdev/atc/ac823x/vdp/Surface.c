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
#include "windows.h"
#include "winutil.h"
#include <media/atc/display.h>
#else
#include "x_types.h"
#include "display.h"
#endif
#include "Surface.h"
#include "log.h"

#define ROUND_UP_COUNT(Count, Pow2) (((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)))

#define MAX_SURFACE     6

static __u64 m_u4PaOfInterPool;
static __u64 m_u4VaOfInterPool;
static __u32 m_u4InterPoolSize;
static __u32 m_u4FreeOffsetOfInterPool;
static __u32 m_u4SurfaceCnt;

static MTKSurface _rSurfaceData[MAX_SURFACE];

#ifdef CONFIG_ATC_OS_android
static MTKSurface _rRoateSurf;
static UINT32  m_u4PaOfRotateBuf;//IMAGE_RESIZE_MEM_PA + IMAGE_RESIZE_MEM_SIZE - 0x400000;
static UINT32  m_u4VaOfRotateBuf;// = MEMRSV_PHY_TO_VIRT(IMAGE_RESIZE_MEM_PA + IMAGE_RESIZE_MEM_SIZE - 0x400000);
#endif

static void _SetPicSize(MTKSurface *pSurf, __u32 u4Width, __u32 u4Height)
{
	if (pSurf) {
		pSurf->u4Height = u4Height;
		pSurf->u4Width   = u4Width;
	}
}

__u64 _PhysicalAddress(MTKSurface *pSurf)
{
	__u64 ret = pSurf ? pSurf->u4Pa : 0;

	return ret;
}

__u64 _GetSurfaceCbCrPa(MTKSurface *pSurf)
{
	__u64 u4Pa;

	u4Pa = pSurf->PhysicalAddress(pSurf) + (ROUND_UP_COUNT(pSurf->Width(pSurf), 16)
		* ROUND_UP_COUNT(pSurf->Height(pSurf), 32));

	return u4Pa;
}


__u64 _VirtualAddress(MTKSurface *pSurf)
{
	__u64 ret = pSurf ? pSurf->u4Va : 0;

	return ret;
}


__u32 _Width(MTKSurface *pSurf)
{
	__u32 ret = pSurf ? pSurf->u4Width : 0;

	return ret;
}

__u32 _Height(MTKSurface *pSurf)
{
	__u32 ret = pSurf ? pSurf->u4Height : 0;

	return ret;
}

#ifdef CONFIG_ATC_OS_android
PMTKSurface AllocRoateSurface(UINT32 u4Width, UINT32 u4Height){    
	UINT32 u4Size;    
	UINT32 u4Pa, u4Va;    
	PMTKSurface  pSurf = NULL;    
	u4Size = ROUND_UP_COUNT(u4Width, 16) * ROUND_UP_COUNT(u4Height, 32) * 3/2 ;    
	u4Size = ROUND_UP_COUNT(u4Size, 0x400);    
	if(u4Size > 0x400000)       
		VDO_LOG(VDO_LOG_LVL_ERR,"AllocRoateSurface:size error,%d > 0x10000\r\n", u4Size);       
	u4Pa = m_u4PaOfRotateBuf;   
	u4Va = m_u4VaOfRotateBuf;       
	VDO_LOG(VDO_LOG_LVL_DBG, "AllocRoateSurface :pa %x va %x w,h %d %d size %d\r\n", (unsigned int)u4Pa, (unsigned int)u4Va, (int)u4Width, (int)u4Height, (int)u4Size);     
	pSurf = &_rRoateSurf;    
	pSurf->u4Width = u4Width;   
	pSurf->u4Height = u4Height;    
	pSurf->u4Pa = u4Pa;    
	pSurf->u4Va = u4Va;    
	pSurf->PhysicalAddress = _PhysicalAddress;    
	pSurf->GetCbCrlAddress = _GetSurfaceCbCrPa;    
	pSurf->Height = _Height;    
	pSurf->Width = _Width;    
	pSurf->VirtualAddress = _VirtualAddress;    
	pSurf->SetPicSize = _SetPicSize;
	clearUp:    
		return (pSurf);
}

#endif

PMTKSurface AllocSurface(__u32 u4Width, __u32 u4Height)
{
	__u32 u4Size;
	__u64 u4Pa, u4Va;
	PMTKSurface  pSurf = NULL;

	u4Size = ROUND_UP_COUNT(u4Width, 16) * ROUND_UP_COUNT(u4Height, 32) * 3 / 2;
	u4Size = ROUND_UP_COUNT(u4Size, 0x400);

	if (m_u4FreeOffsetOfInterPool + u4Size > m_u4InterPoolSize) {
		VDO_LOG(VDO_LOG_LVL_ERR, "AllocSurfaceInternal couldn't allocate physical memory for surface %x\r\n"
			, m_u4InterPoolSize);
		goto clearUp;
	}

	u4Va = m_u4VaOfInterPool + m_u4FreeOffsetOfInterPool;
	u4Pa = m_u4PaOfInterPool + m_u4FreeOffsetOfInterPool;

	VDO_LOG(VDO_LOG_LVL_DBG, "AllocSurface :pa %x va %x w,h %d %d size %d\r\n", (unsigned int)u4Pa
		, (unsigned int)u4Va, (int)u4Width, (int)u4Height, (int)u4Size);

	m_u4FreeOffsetOfInterPool += u4Size;

	if (m_u4SurfaceCnt >= MAX_SURFACE) {
		VDO_LOG(VDO_LOG_LVL_ERR, "AllocSurface : Can not get surface pointer %d\r\n", (int)m_u4SurfaceCnt);
		goto clearUp;
	}

	pSurf = &_rSurfaceData[m_u4SurfaceCnt];
	m_u4SurfaceCnt++;

	pSurf->u4Width = u4Width;
	pSurf->u4Height = u4Height;
	pSurf->u4Pa = u4Pa;
	pSurf->u4Va = u4Va;

	pSurf->PhysicalAddress = _PhysicalAddress;
	pSurf->GetCbCrlAddress = _GetSurfaceCbCrPa;
	pSurf->Height = _Height;
	pSurf->Width = _Width;
	pSurf->VirtualAddress = _VirtualAddress;
	pSurf->SetPicSize = _SetPicSize;

clearUp:
	return pSurf;
}

void set_pool_param(__u64 base, __u64 va, __u32 size)
{
	m_u4PaOfInterPool = base;
	m_u4VaOfInterPool = va;
	m_u4InterPoolSize = size;
	#ifdef CONFIG_ATC_OS_android
	m_u4PaOfRotateBuf = base + size - 0x400000;
	m_u4VaOfRotateBuf = va + size -0x400000;
	#endif
}
EXPORT_SYMBOL(set_pool_param);

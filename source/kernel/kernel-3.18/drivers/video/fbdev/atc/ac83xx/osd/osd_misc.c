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
#include <linux/slab.h>
#include <linux/string.h>
#include <media/atc/drv_osd_if.h>
#include "x_os.h"
#else
#include "assert.h"
#include "drv_osd_if.h"
#endif
/*#include "x_ckgen.h"*/

#include "hal/osd_map.h"

__u32  _u4DispMode;
__u32  _u4LCDWidth;
__u32  _u4LCDHeight;
__u32  _u4LCDType;
__u32  _u4RearOutputMode;

void OSD_IsLog(__s32 level, const __s8 *ps_format, ...)
{
}

#define INVALID_RGN  (-1)

static __u32 _au4PlaneRgn[200] = {
	INVALID_RGN,/*OSD1*/
	INVALID_RGN,/*OSD2*/
	INVALID_RGN,
	INVALID_RGN,
	INVALID_RGN,
	INVALID_RGN,
	INVALID_RGN,
};


void SetPlaneRgn(__u32 u4Plane, __u32 u4Rgn)
{
	_au4PlaneRgn[u4Plane] = u4Rgn;
}
EXPORT_SYMBOL(SetPlaneRgn);


__u32 GetPlaneRgn(__u32 u4Plane)
{
	return _au4PlaneRgn[u4Plane];
}
EXPORT_SYMBOL(GetPlaneRgn);

void HalFlushInvalidateDCache(void)
{
	/*TODO: falls*/
	/*CacheSync(CACHE_SYNC_DISCARD);*/
}

#if 0
void BSP_FlushDCacheRange(__u32 u4Start, __u32 u4Len)
{
	/*TODO: falls*/
	/*CacheRangeFlush((void*)u4Start, u4Len, CACHE_SYNC_WRITEBACK);*/
}

void *x_alloc_aligned_dma_mem(__u32 u4Size, __u32 u4Align)
{

	void *pVa;

	/*TODO: falls*/
	/*unsigned int pa;*/
	/*pVa = AllocPhysMem(u4Size, PAGE_READWRITE, 0xF, 0, &pa);*/
	/*AddPaVatoMapTable(pa, (__u32)pVa, u4Size);*/
	pVa = kmalloc(u4Size, GFP_KERNEL);

	/*ASSERT(0);*/
	return pVa;
}
#endif

void x_free_aligned_dma_mem(void *pUser)
{
	/*TODO: falls*/
	/*FreePhysMem(pUser);*/
	kfree(pUser);
	/*ASSERT(0);*/
}

void *x_alloc_aligned_ch2_mem(__u32 u4Size, __u32 u4Align)
{

	void *pVa;

	/*TODO: falls*/
	/*unsigned int pa;*/
	/*pVa = AllocPhysMem(u4Size, PAGE_READWRITE, 0xF, 0, &pa);*/
	/*AddPaVatoMapTable(pa, (__u32)pVa, u4Size);*/

	ASSERT(0);
	return pVa;

}

void  x_free_aligned_nc_mem(void *pUser)
{
	/*TODO: falls*/
	/*FreePhysMem(pUser);*/
	ASSERT(0);
}

void *x_alloc_aligned_nc_mem(__u32 u4Size, __u32 u4Align)
{
	unsigned int pa;
	void *u4Va;

	ASSERT(0);
	/*TODO: falls*/
	/*u4Va = AllocPhysMem(u4Size, PAGE_READWRITE, 0xF, 0, &pa);*/
	memset(u4Va, 0xFF, u4Size);

	if (u4Align == 1024) {
		return (void *)pa;
	} else {
		return (void *)u4Va;
	}
}




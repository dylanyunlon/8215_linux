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

#ifndef DRV_GRAPHICS_H
#define DRV_GRAPHICS_H

#include <linux/types.h>
#include <linux/ioctl.h>

typedef struct
{
	uint32_t u4Addr;
	uint32_t u4Width;
	uint32_t u4Height;
	uint32_t u4Pitch;
	uint32_t u4CM;
	uint32_t u4X;
	uint32_t u4Y;
}gfx_buffer_paras;

typedef struct
{
	uint32_t u4DisKey;
	uint32_t u4KeyIn;
	uint32_t u4KeyMin;
	uint32_t u4KeyMax;
}gfx_color_key;

typedef struct
{
	uint32_t u4Bltopt;
	uint32_t u4Color;
	uint32_t u4Alpha;
	int pid;
	gfx_buffer_paras srcBuffer;
	gfx_buffer_paras dstBuffer;
	gfx_color_key srcColorkey;
	gfx_color_key dstColorkey;
}gfx_ioc_paras;

#define GFX_IOC_BASE 0x2D

typedef enum
{
	GFX_BITBLT = 0,
	GFX_STRETCHBLT,
	GFX_FILLRECT,
	GFX_ALPHABLEND
}GFX_IOCTL_FUNCTIONS;

#define GFX_IOC_BITBLT					_IOR (GFX_IOC_BASE, GFX_BITBLT, gfx_ioc_paras *) 
#define GFX_IOC_STRETCHBLT			_IOR (GFX_IOC_BASE, GFX_STRETCHBLT, gfx_ioc_paras *) 
#define GFX_IOC_FILLRECT			_IOR (GFX_IOC_BASE, GFX_FILLRECT, gfx_ioc_paras *) 
#define GFX_IOC_ALPHABLEND			_IOR (GFX_IOC_BASE, GFX_ALPHABLEND, gfx_ioc_paras *)

#endif
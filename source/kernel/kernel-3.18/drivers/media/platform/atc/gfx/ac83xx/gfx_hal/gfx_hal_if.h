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
#ifndef GFX_HAL_IF_H
#define GFX_HAL_IF_H

#include "drv_gfx.h"
//#include "gfx.h"

int gfx_bitblt(uint32_t u4TTB,gfx_ioc_paras *gfx_paras);

int gfx_fillrect(uint32_t u4TTB,gfx_ioc_paras *gfx_paras);

int gfx_stretchblt(uint32_t u4TTB,gfx_ioc_paras *gfx_paras);

int gfx_alphablend(uint32_t u4TTB,gfx_ioc_paras *gfx_paras);


#endif


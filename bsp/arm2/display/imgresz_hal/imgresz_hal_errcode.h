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

#ifndef _HAL_IMGRESZ_ERRCODE_H_
#define _HAL_IMGRESZ_ERRCODE_H_


#define IMGRESZ_HAL_UOKCODE(group, okcode)     \
	((s32)				  \
	((u32)(0x00000000) |                 \
	(u32)((group & 0xff) << 8) |  \
	(u32)(okcode & 0xff))	 \
	)


#define IMGRESZ_HAL_UERRCODE(group, errcode)   \
	((s32)				  \
	((u32)(0x80000000) |                 \
	(u32)((group & 0xff) << 8) |  \
	(u32)(errcode & 0xff))	 \
	)


#define IMGRESZ_HAL_GROUP_GENERAL             0


#define S_IMGRESZ_HAL_OK                  IMGRESZ_HAL_UOKCODE(IMGRESZ_HAL_GROUP_GENERAL, 0x00)

#define E_IMGRESZ_HAL_FAIL                IMGRESZ_HAL_UERRCODE(IMGRESZ_HAL_GROUP_GENERAL, 0x00)



#endif



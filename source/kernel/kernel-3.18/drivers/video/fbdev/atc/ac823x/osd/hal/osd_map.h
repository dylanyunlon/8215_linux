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
#ifndef _OSD_MAP_H

#define _OSD_MAP_H

void AddPaVatoMapTable(__u32 u4Pa, __u32 u4Va, __u32 u4Size);
__u32 VA_TO_PA(__u32 u4Va);
__u32 PA_TO_VA(__u32 u4Pa);

#endif

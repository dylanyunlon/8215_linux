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
#ifndef _OSD_IF_PDD_H

#define _OSD_IF_PDD_H

extern __s32  i4OsdVfyCreateSemaphores(void);
extern __s32  i4OsdVfyDeleteSemaphores(void);
extern __s32 i4OsdSetDisplayMode(__u32 u4Plane, __u32 u4DispMode);
extern __s32 i4OsdPlaneEnble(__u32 u4Plane, __u32 fgEnble);
extern __s32 i4OsdPlaneUpdate(__u32 u4Plane);
extern __s32 i4OsdPlaneFlipTo(__u32 u4Plane, __u32 u4RgnList);
extern void i4OsdVsync(void);
extern bool IsOSDSupportHWScaler(__u32 u4Plane);
extern  __u32 u4PixelFormatToOSDColorMode(__u32 u4PixelFormat);
extern __s32 i4OSDRestoreHwReg(void);

#endif

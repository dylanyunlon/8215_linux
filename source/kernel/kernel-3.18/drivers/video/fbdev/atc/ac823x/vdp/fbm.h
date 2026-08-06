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
#ifndef FBM_H

#define FBM_H


__u32 FBM_Init(__u32 u4GroupID, void* *pTable, __s32 i4Size);
void* FBM_Lock(__u32 u4GroupID);
__u32 FBM_Unlock(__u32 u4GroupID);
void* FBM_Flip(__u32 u4GroupID);
void* FBM_GetOnScreen(__u32 u4GroupID);
void FBM_Uninit(__u32 u4GroupID);
bool FBM_IsNotEmpty(__u32 u4GroupID);




#endif




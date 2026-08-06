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

#ifndef X_STL_LIB_H
#define X_STL_LIB_H

#include "x_typedef.h"
#include "x_os.h"

//extern UINT32 StrToInt(const CHAR* pszStr);
extern UINT32 StrToHex(const CHAR* pszStr, UINT32 u4Len);
extern void * UTIL_AlignAlloc(size_t zAllocBytes, size_t zZeroBits);
extern void * UTIL_AlignAlloc16(size_t zAllocBytes);
extern INT32 UTIL_AlignFree(void *pvAlignBlock);

#endif /* X_STL_LIB_H */


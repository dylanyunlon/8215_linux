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

#ifndef _EDID_HAL_H_
#define _EDID_HAL_H_


#include "x_typedef.h"
#include "typedef.h"


VOID MZ_LoadHdcpData(UINT8 *pOutBuf, UINT32 u4Size);
VOID MZ_LoadEdidData(UINT8 *pOutBuf, UINT32 u4Size);
/* inter function */
extern VOID MZ_LoadBinaryData(UINT32 u4BinaryStart, UINT8 *pOutBuf, UINT32 u4Size);


#endif

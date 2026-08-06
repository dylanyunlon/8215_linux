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


#include <linux/module.h>
#include  "_cli.h"

#include <linux/types.h>

u32 StrToHex(const s8 *pszStr, u32 u4Len);
u32 StrToDec(const s8 *pszStr, u32 u4Len);


u32 StrToHex(const s8 *pszStr, u32 u4Len)
{
	u32 u4Idx;
	u32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0)) {
		return 0;
	}

	u4Len = (u4Len > 8) ? 8 : u4Len;

	for (u4Idx = 0; u4Idx < u4Len; u4Idx++) {
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9')) {
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - '0');
		} else if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F')) {
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - 'A') + 10;
		} else if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f')) {
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - 'a') + 10;
		} else {
			return 0;
		}
	}

	return u4ReturnValue;
}
EXPORT_SYMBOL(StrToHex);

u32 StrToDec(const s8 *pszStr, u32 u4Len)
{
	u32 u4Idx;
	u32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0)) {
		return 0;
	}

	/* 0xFFFFFFFF = 4294967295 */
	u4Len = (u4Len > 10) ? 10 : u4Len;

	for (u4Idx = 0; u4Idx < u4Len; u4Idx++) {
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9')) {
			u4ReturnValue *= 10;
			u4ReturnValue += (u32)(u8)(pszStr[u4Idx] - '0');
		} else {
			return 0;
		}
	}

	return u4ReturnValue;
}
EXPORT_SYMBOL(StrToDec);

s32 StrToInt(const char *pszStr)
{
	u32 u4Len;

	if (pszStr == NULL) {
		return 0;
	}

	u4Len = x_strlen(pszStr);

	if (u4Len > 2) {
		if ((pszStr[0] == '0') && (pszStr[1] == 'x')) {
			return StrToHex(&pszStr[2], u4Len - 2);
		}
	}

	return StrToDec(pszStr, u4Len);
}
EXPORT_SYMBOL(StrToInt);


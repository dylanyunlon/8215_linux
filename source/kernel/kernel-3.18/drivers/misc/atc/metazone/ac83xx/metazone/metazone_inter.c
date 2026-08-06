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
#include <linux/vermagic.h>
#include <linux/compiler.h>

#include "metazone_inter.h"
#include "metazone.h"
#include "metazone_ioctl.h"

extern bool _fgWritableZoneInited;
extern void *_pMetaZone;
extern TMetaZone *_pMetaHeader;
extern char *_pbReserve;
extern u32 *_pu4Value;
extern u32 _u4BinaryStart;


u32 _MetaZone_Read(u32 u4Idx, u32 *pu4Data)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			*pu4Data = _pu4Value[u4Idx];
			return MZ_SUCCESS;
		}
		u4Idx += MZ_WR_IDX_START;
	}
	return MZ_FAILURE;
}

u32 _MetaZone_ReadBinary(u32 u4Idx, char *pbData, u32 u4Size)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			u32 u4Tmp =
			    *(u32 *) (_u4BinaryStart +
				      (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx);
			if (u4Size > u4Tmp)
				u4Size = u4Tmp;
			memcpy(pbData,
			       (char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), u4Size);
			return u4Size;
		}
	}
	return MZ_FAILURE;
}


u32 _MetaZone_ReadReserved(char *pbData, u32 u4Size)
{
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;
	memcpy(pbData, _pbReserve, u4Size);

	return u4Size;
}

u32 _MetaZone_Write(u32 u4Idx, u32 u4Data)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			_pu4Value[u4Idx] = u4Data;
			return MZ_SUCCESS;
		}
	}

	return MZ_FAILURE;
}

u32 _MetaZone_WriteBinary(u32 u4Idx, char *pbData, u32 u4Size)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			if (u4Size > _pMetaHeader->dwBinaryItemSize)
				u4Size = _pMetaHeader->dwBinaryItemSize;
			*(u32 *) (_u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx) =
			    u4Size;
			memcpy((char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), pbData,
			       u4Size);
			return MZ_SUCCESS;
		}
	}
	return MZ_FAILURE;
}


u32 _MetaZone_WriteReserved(char *pbData, u32 u4Size)
{
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;
	memcpy(_pbReserve, pbData, u4Size);

	return MZ_SUCCESS;
}

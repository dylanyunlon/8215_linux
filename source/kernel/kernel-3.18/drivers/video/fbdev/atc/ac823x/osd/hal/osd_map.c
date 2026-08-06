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
#ifndef __ARM2__
#include <linux/mm.h>
#else
#include "x_types.h"
#endif

#include "x_os.h"
#include "osd_map.h"

typedef struct {
	__u32    u4Pa;
	__u32    u4Va;
	__u32    u4Size;
} PA_VA_MAP_T;

#define MAP_TABLE_SIZE 2000

PA_VA_MAP_T _aMapTable[MAP_TABLE_SIZE];

static __u32 _u4MapCnt;


void AddPaVatoMapTable(__u32 u4Pa, __u32 u4Va, __u32 u4Size)
{
	if (_u4MapCnt < MAP_TABLE_SIZE) {
		_aMapTable[_u4MapCnt].u4Pa = u4Pa;
		_aMapTable[_u4MapCnt].u4Va  = u4Va;
		_aMapTable[_u4MapCnt].u4Size = u4Size;
		_u4MapCnt++;
	}
}



__u32 PA_TO_VA(__u32 u4Pa)
{
	__u32 j, ret = (__u32)(-1);

	for (j = 0; j < _u4MapCnt; j++) {
		if ((u4Pa >= _aMapTable[j].u4Pa)  &&
		    (u4Pa < _aMapTable[j].u4Pa + _aMapTable[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return ret;
	}

	ret = _aMapTable[j].u4Va + u4Pa - _aMapTable[j].u4Pa;

	return ret;
}

__u32 VA_TO_PA(__u32 u4Va)
{
	__u32 j, ret = (__u32)(-1);

	for (j = 0;  j < _u4MapCnt; j++) {
		if ((u4Va >= _aMapTable[j].u4Va) &&
		    (u4Va < _aMapTable[j].u4Va + _aMapTable[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return ret;
	}
	ret = _aMapTable[j].u4Pa +  u4Va - _aMapTable[j].u4Va;
	return ret;
}



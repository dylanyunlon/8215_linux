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
#ifndef _DRV_ENV_H_
#define _DRV_ENV_H_

#include "x_typedef.h"
#ifndef __ARM2__
#include <media/atc/drv_av_d.h>
#else
#include "drv_av_d.h"
#endif

typedef struct _POSITION_OFFSET_T
{
    INT32 i4XOffset; // Horizontal offset
    INT32 i4YOffset; // Vertical odd offset
    INT32 i4EvenYOffset; // Vertical even offset
} POSITION_OFFSET_T;

typedef struct _OFFSET_TABLE_T
{
    POSITION_OFFSET_T rOSD5;
    POSITION_OFFSET_T rOSD4;
    POSITION_OFFSET_T rOSD3;
    POSITION_OFFSET_T rOSD2;
    POSITION_OFFSET_T rOSD1;
    POSITION_OFFSET_T rVDP2;
    POSITION_OFFSET_T rVDP1;
} OFFSET_TABLE_T;

extern OFFSET_TABLE_T rLocationOffset[];

#endif //_DRV_ENV_H_

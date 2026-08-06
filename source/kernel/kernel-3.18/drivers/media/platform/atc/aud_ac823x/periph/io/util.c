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

#include "util.h"


void AudUtil_Delayus(u32 u4Value)
{     
    volatile u32 wCnt;

    for (wCnt = 0; wCnt < u4Value; wCnt++)
    {
        volatile s32 i;
        for (i = 0; i < 192; i++);
        {/* Base on 800Mhz RISC clock */
        }//for
    }
}


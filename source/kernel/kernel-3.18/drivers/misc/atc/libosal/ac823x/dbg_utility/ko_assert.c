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
#include "x_assert.h"
#include "x_printf.h"


#include <linux/types.h>

extern __s32 SYS_Printf(const char *ps_format, ...);

void Assert(const char* szExpress, const char* szFile, __s32 i4Line)
{
    SYS_Printf(KERN_ERR "\nAssertion fails at:\nFile: %s, line %d\n\n", szFile, (__s32)(i4Line));
    SYS_Printf(KERN_ERR "\t%s\n\n", szExpress);
    dump_stack();
    SYS_Printf(KERN_ALERT "Program terminated.\n");
#if 0
    {
        __u32 dscr;
        asm("mrc p14, 0, %0, c0, c1, 0" : "=r" (dscr) : : "cc");
        if ((dscr & (1 << 14U)) != 0U)
        {
            // here is a software breakpoint.
            asm("bkpt 0");

            // if you want to leave this break function, set PC to here.
        }
    }
#endif
    panic("%s", szExpress);
}

EXPORT_SYMBOL(Assert);


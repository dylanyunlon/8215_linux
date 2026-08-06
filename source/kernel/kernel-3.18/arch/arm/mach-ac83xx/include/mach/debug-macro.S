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
//#include <mach/hardware.h>
#define RS232_PHYS   0xc000
#define REG_RW_UART0_STATUS  0x4
#define VAL_UART0_WRITE_OFFSET  0x2

        .macro  addruart,rx
        mrc p15, 0, \rx, c1, c0
        tst \rx, #1                 @ MMU enabled?
        ;moveq   \rx, #AC83XX_IO_PA       @ physical base address
        ;movne   \rx, #AC83XX_IO_VA       @ virtual base
	moveq   \rx, #0xf000c000       @ physical base address
        movne   \rx, #0xfd00c000       @ virtual base
        add     \rx, \rx, #RS232_PHYS
        .endm

        .macro  senduart,rd,rx
        strb    \rd, [\rx, #0x0]
        .endm

        .macro  waituart,rd,rx
        mov \rd, \rx
        add \rd, \rd, #REG_RW_UART0_STATUS
WTUART1:tst \rd, #VAL_UART0_WRITE_OFFSET
        bne WTUART1
        
        .endm

        .macro  busyuart,rd,rx
		mov	\rd, #0x5000             @ wait for about 5000 cycles
wait_001:
		subs	\rd, \rd, #1
		nop
		cmp	\rd, #0
		bne	wait_001           
        .endm



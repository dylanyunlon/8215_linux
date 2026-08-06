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

 #ifndef _REG_TIMER_H
 #define _REG_TIMER_H

#include "base_regs.h"

/*************************************************************************/
/*   TIMER                                                             */
/*************************************************************************/
#define REG_TIMER_LIMIT		(*((volatile UINT32*)(BIM_BASE + 0x60)))
#define REG_TIMER_CONTROL	(*((volatile UINT32*)(BIM_BASE + 0x78)))

#define REG_RW_TIMER0_LIM_OFFSET    0x60
#define REG_RW_TIMER0_VALUE         0x64
#define REG_RW_TIMER_CTRL_OFFSET    0x78
#define REG_RW_64B_TIMER0_OFFSET  0x728  

#define VAL_T0_AUTOLOAD     0x2
#define VAL_T0_ENABLE       0x1
#define VAL_T1_AUTOLOAD     0x200
#define VAL_T1_ENABLE       0x100
#define VAL_T2_AUTOLOAD     0x20000
#define VAL_T2_ENABLE       0x10000

#define CONFIG_SYS_HZ		   1000		// ticks every 1ms
//#define CFG_HZ_CLOCK		101250000	/* Timer 1 is clocked at 101.24Mhz */

#ifdef CHIP_VER_AC83XX
#define CFG_HZ_CLOCK		324000000  //27000000 //324000000	/* Timer 1 is clocked at 27Mhz  AC83XX FPGA*/
#endif

#define CFG_CLOCK_PER_TICKS	(CFG_HZ_CLOCK / CONFIG_SYS_HZ)
#define CFG_HZ_PER_USEC	    (CFG_HZ_CLOCK / 1000000)

#endif


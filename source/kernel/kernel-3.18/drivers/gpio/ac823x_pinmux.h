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

#ifndef __ASM_ARCH_PINMUX_H
#define __ASM_ARCH_PINMUX_H

#if defined (__ARM2__)

/*
   arm2 build environment
*/
#include <pinmux_reg.h>
	
#define bsp_pinset __bsp_pinset
#define bsp_pinget __bsp_pinget


#elif defined (__UBOOT__)
//#error "uboot build test"


/*
   uboot build environment
*/
#include <pinmux_reg.h>

#define bsp_pinset __bsp_pinset
#define bsp_pinget __bsp_pinget

#else
//#error "kernel build test"

//#include <mach/pinmux_reg.h>
#include "ac823x_pinmux_reg.h"


int bsp_pinset(unsigned pinmux_sel, unsigned function);
int bsp_pinget(unsigned pinmux_sel);

int BSP_PinSet(int funcsel, int i4Func);
int BSP_PinGet(int funcsel);




#endif

#endif



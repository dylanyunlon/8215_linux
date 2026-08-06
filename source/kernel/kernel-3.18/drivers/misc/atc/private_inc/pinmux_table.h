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
#ifndef __PINMUX_TABLE_
#define __PINMUX_TABLE_
  
//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include <nkintr.h>
//#include "gpio_defs.h"
#include "x_gpio.h"
#include "x_pinmux.h"
//#include "pinmux_defs.h"

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

#ifdef CHIP_VER_AC83XX
static const UINT8 _au1PinmuxFunctionMasks[MAX_PINMUX_SEL] =
{
  3,  0,  0,  3,  0,  0,  0,  0,  3,  0, 
  0,  0,  3,  0,  0,  3,  0,  0,  2,  0,
  2,  0,  1,  0,  1,  1,  1,  1,  3,  0,
  0,  0,  

  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  3,  0,  0,  3,  0,
  0,  3,  0,  0,  1,  3,  0,  0,  3,  0,
  0,  0,

  3,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  2,  0,  2,  0,  0,  0,  1,  1,  2,  0,
  2,  0,  1,  1,  1,  1,  3,  0,  0,  1,
  1,  1,

  1,  1,  0,  0,  0,  0,  0,  0,  2,  0,
  1,  0,  2,  0,  0,  0,  2,  0,  2,  0,
  0,  0,  2,  0,  2,  0,  2,  0,  2,  0,
  2,  0,

  2,  0,  2,  0,  2,  0,  2,  0,  2,  0,
  1,  1,  1,  0,  0,  0,  1,  0,  0,  0,
  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,
  0,  0,

  2,  0,  2,  0,  2,  0,  2,  0,  3,  0,
  0,  0,  3,  0,  0,  0,  3,  0,  0,  3,
  0,  0,  3,  0,  0,  3,  0,  0,  3,  0,
  0,  0,

  3,  0,  0,  3,  0,  0,  3,  0,  0,  0,
  0,  2,  0,  2,  0,  0,  2,  0,  2,  0,
  2,  0,  0,  2,  0,  0,  3
};

#endif

#endif//__PINMUX_TABLE_

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

#ifndef X_BIM_H
#define X_BIM_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "drv_config.h"
#include "chip_ver.h"
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX) 
#include "x_bim_83xx.h"
#endif

//============================================================================
// Constant definitions
//============================================================================
#if CONFIG_DRV_LINUX
#define MAX_IRQ_VECTOR              127
#else
#define MAX_IRQ_VECTOR              95
#endif
#endif  // X_BIM_H


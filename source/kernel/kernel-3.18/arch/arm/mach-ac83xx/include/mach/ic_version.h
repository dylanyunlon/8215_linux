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

#ifndef _IC_VERSIO_H_
#define _IC_VERSIO_H_
#include <mach/chip_ver.h>

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)

typedef enum
{
  IC_83XX_VER_FPGA    = 0x0000,           // FPGA
  IC_83XX             = 0x3360,           //
  IC_83XX_VER_UNKNOWN = 0x7fffffff,       // Unknown version
} IC_VERSION_T;
#endif

extern IC_VERSION_T BSP_GetIcVersion(void);

#endif // _IC_VERSIO_H_

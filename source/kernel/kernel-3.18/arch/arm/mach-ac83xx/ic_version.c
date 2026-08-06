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

//==================================================
// header files
//==================================================
#include <mach/chip_ver.h>
#include <linux/module.h>
#include <mach/hardware.h>
#include <mach/ic_version.h>
//#include <x_typedef.h>


//==================================================
// Define
//==================================================


//==================================================
// Private functions
//==================================================

//==================================================
// public functions
//==================================================
IC_VERSION_T BSP_GetIcVersion(void)
{
  uint32_t u4Tmp;
  IC_VERSION_T eVer = {0};

  u4Tmp = IO_READ32(IO_VIRT, 0x0);
  return eVer;

}
EXPORT_SYMBOL(BSP_GetIcVersion);


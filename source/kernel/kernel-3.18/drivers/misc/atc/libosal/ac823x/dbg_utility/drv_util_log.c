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

#include "x_module.h"
#include "section.h"
#include "x_printf.h"

#include <linux/types.h>


extern __s32 Printf(const char *ps_format, ...);

__s32 UTIL_Printf(const char *ps_format, ...)
{
  return Printf(ps_format);
}


EXPORT_SYMBOL(UTIL_Printf);


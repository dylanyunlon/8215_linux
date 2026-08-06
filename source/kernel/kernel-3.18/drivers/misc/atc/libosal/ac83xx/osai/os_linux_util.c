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



#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include "x_os.h"

const char *basename(const char *full_name)
{
	size_t z_len;
	
	z_len = strlen(full_name);
	while (z_len --)
	{
		if ('/' == *(full_name + z_len))
		{
			return full_name + z_len + 1;
		}
	}
	return full_name;
}

EXPORT_SYMBOL(basename);


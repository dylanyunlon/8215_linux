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

#ifndef DIGITAL_HAL_H_
#define DIGITAL_HAL_H_


#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>


typedef enum {
	DIG_IN_TYPE_601_P_480,
	DIG_IN_TYPE_601_P_576,
	DIG_IN_TYPE_601_I_480,
	DIG_IN_TYPE_601_I_576,
	DIG_IN_TYPE_656_P_480,
	DIG_IN_TYPE_656_P_576,
	DIG_IN_TYPE_656_I_480,
	DIG_IN_TYPE_656_I_576,
	DIG_IN_TYPE_656_p_800_480,
	DIG_IN_TYPE_MAX,
} E_DIG_IN_TYPE_T;


int digital_start_video(int mDigInFmt);
int digital_stop_video(void);
int digital_select_video(int index);

#endif

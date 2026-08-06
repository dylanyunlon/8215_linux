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

#include "windows.h"
#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "x_typedef.h"
#include "winutil.h"
#include "linux/kernel.h"
#include "vga_hal_api.h"

#include <linux/types.h>


/*******************************************************************************
 *_CLI_AutoColor
 * Function: Doing AutoColor
*******************************************************************************/
static s32 _CLI_AutoColor(s32 i4Argc, const s8 **szArgv)
{
	ybr_vga_autocolor();
	return 0;
}

static s32 _CLI_Auto(s32 i4Argc, const s8 **szArgv)
{
	ybr_vga_auto();
	return 0;
}


CLI_EXEC_T _arVgaDrvCmdTbl[] = {
	/* auto color */
	{
		TEXT("AutoColor"),
		TEXT("ac"),
		_CLI_AutoColor,
		NULL,
		TEXT("Doing Auto Color"),
		CLI_GUEST
	},
	{
		TEXT("Auto"),
		TEXT("a"),
		_CLI_Auto,
		NULL,
		TEXT("Doing Auto"),
		CLI_GUEST
	},
	/* last cli command record, NULL */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		CLI_SUPERVISOR
	}
};



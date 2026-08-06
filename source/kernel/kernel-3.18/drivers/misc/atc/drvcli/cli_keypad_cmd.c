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

#include <linux/types.h>

#include "winutil.h"

#include "ac83xx_keyadc.h"

#define _ttoi(a)   simple_strtol(a, NULL, 10)

static s32 _CLI_EnableKeypad(s32 i4Argc, const s8 **szArgv)
{
	bool enable = FALSE;

	if (i4Argc < 2) {
		pr_err(TEXT("[Command]Parameter Error.\r\n"));
	}

	enable = (bool)_ttoi(szArgv[1]);

	Keypad_Enable(enable);

	return 0;
}


/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arKeypadCmdTbl[] = {
	{
		TEXT("Enable"), TEXT("en"), _CLI_EnableKeypad,
		NULL, TEXT("Enable Keypad"), CLI_GUEST},
	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};



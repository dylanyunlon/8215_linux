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

#include "cli.h"
#include "_cli.h"
#include "drv_config.h"
#include "x_debug.h"
#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"

extern CLI_EXEC_T _arUartCmdTbl[];
//extern CLI_EXEC_T _arGpioCmdTbl[];
//extern CLI_EXEC_T _arI2CCmdTbl[];
//extern CLI_EXEC_T _arDmxDrvCmdTbl[];
extern CLI_EXEC_T _arMemoryCmdTbl[];
extern CLI_EXEC_T _arAudCmdTbl[];
extern CLI_EXEC_T _arSeCmdTbl[];
//extern CLI_EXEC_T _arWavCmdTbl[];
//extern CLI_EXEC_T _arKeypadCmdTbl[];
//extern CLI_EXEC_T _arVdecCmdTbl[];
//extern CLI_EXEC_T _arMetazoneCmdTbl[];
//extern CLI_EXEC_T _arVgaDrvCmdTbl[];
//extern CLI_EXEC_T _arBluetoothCmdTbl[];
/******************************************************************************
* Variable  : cli default table
******************************************************************************/
static CLI_EXEC_T _arDrvCmdTbl[] = {
	{
		TEXT("uart"),                  //pszCmdStr
		TEXT("u"),
		NULL,              //execution function
		_arUartCmdTbl,
		TEXT("Uart command"),
		CLI_GUEST
	},
	{
		TEXT("memory"),                    //pszCmdStr
		TEXT("m"),
		NULL,                       //execution function
		_arMemoryCmdTbl,
		TEXT("Memory & Register R/W"),
		CLI_GUEST
	},
	/*{
		TEXT("gpio"),                  //pszCmdStr
		TEXT("g"),
		NULL,              //execution function
		_arGpioCmdTbl,
		TEXT("GPIO & PinMux command"),
		CLI_GUEST
	},
	{
		TEXT("I2C"),                  //pszCmdStr
		TEXT("i2c"),
		NULL,              //execution function
		_arI2CCmdTbl,
		TEXT("I2C command"),
		CLI_GUEST
	},*/
	
	{
		TEXT("audio"),                  //pszCmdStr
		TEXT("aud"),
		NULL,              //execution function
		_arAudCmdTbl,
		TEXT("audio log"),
		CLI_GUEST
	},
	/*{
		TEXT("postprocess"),                  //pszCmdStr
		TEXT("se"),
		NULL,              //execution function
		_arSeCmdTbl,
		TEXT("pp audio log"),
		CLI_GUEST
	},
	*/
	/* Zeng Zhang */
	/*{
		TEXT("waveform"),                  //pszCmdStr
		TEXT("wav"),
		NULL,              //execution function
		_arWavCmdTbl,
		TEXT("Waveform"),
		CLI_GUEST
	},*/
	/*{
		TEXT("Keypad"),
		TEXT("kp"),
		NULL,
		_arKeypadCmdTbl,
		TEXT("Keypad"),
		CLI_GUEST
	}, */
	/*
	{
		TEXT("bluetooth"),
		TEXT("bt"),
		NULL,
		_arBluetoothCmdTbl,
		TEXT("bluetooth"),
		CLI_GUEST
	},
	{
		TEXT("demuxer"),
		TEXT("dmx"),
		NULL,
		_arDmxDrvCmdTbl,
		TEXT("demuxer"),
		CLI_GUEST
	},
	{
		TEXT("videodecoder"),
		TEXT("vdec"),
		NULL,
		_arVdecCmdTbl,
		TEXT("videodecoder"),
		CLI_GUEST
	}, */
	/*
	{
		TEXT("metazone"),
		TEXT("mtz"),
		NULL,
		_arMetazoneCmdTbl,
		TEXT("metazone"),
		CLI_GUEST
	},
	{
		TEXT("ypbpr vga"),                  //pszCmdStr
		TEXT("vga"),
		NULL,              //execution function
		_arVgaDrvCmdTbl,
		TEXT("ypbpr vga command"),
		CLI_GUEST
	},
	*/

	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};

/******************************************************************************
* Function      : CLI_GetDriverCmdTbl
* Description   : retrun default command table
******************************************************************************/
CLI_EXEC_T *CLI_GetDriverCmdTbl(void)
{
	return _arDrvCmdTbl;
}

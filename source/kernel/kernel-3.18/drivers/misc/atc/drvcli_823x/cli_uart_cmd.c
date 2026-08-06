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
#include "ac83xx_cli.h"

#include <mach/pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <mach/ac83xx_gpio_pinmux.h>

#define _ttoi(a)   simple_strtol(a, NULL, 10)

extern u32 _u4UartMode;
extern bool _fgKernelLogEnable;
extern bool fgSet_Uart_SourceCLK(u32 _u4UART_SourceClk);


void SwitchUartMode(u32 u4UartMode)
{
	pr_info(TEXT("\r\n UART enter SwitchUartMode.\r\n"));

	if (u4UartMode == UART_NORMAL_MODE) {
		pr_info(TEXT("\r\n UART enter SwitchUartMode. UART_NORMAL_MODE\r\n"));
		fgSet_Uart_SourceCLK(27000000);
		__raw_writel(0x2, (volatile void *)(0xFD00C000 + 0x4));
	} else {
		__raw_writel(0xE2, (volatile void *)(0xFD00C000 + 0x4));
	}
}

static s32 _CLI_UartEnableKernelLog(s32 i4Argc, const s8 **szArgv)
{
	if (!_fgKernelLogEnable) {
		_fgKernelLogEnable = TRUE;
	}
	pr_info(TEXT("\r\n Kernel log has been enabled.\r\n"));
	return 0;
}
void cli_disable_log(bool fgEnable)
{
	if (fgEnable) {
		pr_info(TEXT("\r\n log will be disabled.\r\n"));
		_fgKernelLogEnable = FALSE;
	}
}

#if 0
static s32 _CLI_UartDisableKernelLog(s32 i4Argc, const s8 **szArgv)
{
	if (_fgKernelLogEnable) {
		pr_info(TEXT("\r\n log will be disabled.\r\n"));
		_fgKernelLogEnable = FALSE;
	}
	return 0;
}
#endif

static s32 _CLI_UartEnterDebug(s32 i4Argc, const s8 **szArgv)
{
	pr_info(TEXT("\r\n UART will enter normal.\r\n"));
	mdelay(50);
	SwitchUartMode(UART_NORMAL_MODE);
	_u4UartMode = UART_NORMAL_MODE;

	return 0;
}
static s32 _CLI_ChangeSourceClock(s32 i4Argc, const s8 **szArgv)
{
	u32 u4SourceCLK  = 32400000;

	if (i4Argc < 2) {
		pr_info(TEXT("Current RS232 Source Clock = %d  \r\n"), u4SourceCLK);
		return 0;
	}

	u4SourceCLK = _ttoi(szArgv[1]);
	fgSet_Uart_SourceCLK(u4SourceCLK);

	return 0;
}

static s32 _CLI_OpenDvpDebugUart(s32 i4Argc, const s8 **szArgv)
{
	bool fgUart4DvpDebug = TRUE;

	pr_info(TEXT("[dvp][cli] _CLI_OpenDvpDebugUart entry, arg cnt:%d\r\n"),
			i4Argc);

	if (i4Argc >= 2) {
		const s8 *arg = szArgv[1];

		if (arg != NULL) {
			fgUart4DvpDebug = arg[0] - '0';
		}
	}

	if (fgUart4DvpDebug) {
		GPIO_MultiFun_Set(PIN_143_URXD2, DVD_RS232_SEL);
		GPIO_MultiFun_Set(PIN_155_UTXD2, DVD_RS232_SEL);
	} else {
		GPIO_MultiFun_Set(PIN_143_URXD2, UART2_SEL);
		GPIO_MultiFun_Set(PIN_155_UTXD2, UART2_SEL);
	}

	return 0;
}


/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arUartCmdTbl[] = {

	{
		TEXT("EnterDebug"),
		TEXT("i"),
		_CLI_UartEnterDebug,
		NULL,
		TEXT("Enter debug mode."),
		CLI_GUEST
	},
	{
		TEXT("LogEnable"),                  /* pszCmdStr */
		TEXT("e"),
		_CLI_UartEnableKernelLog,              /* execution function */
		NULL,
		TEXT("Enable kernel log output"),
		CLI_GUEST
	},
	{
		TEXT("ChangeSC"),                   /* pszCmdStr */
		TEXT("csc"),
		_CLI_ChangeSourceClock,            /* execution function */
		NULL,
		TEXT("Change RS232 Source Clock"),
		CLI_GUEST
	},
	{
		TEXT("DvpDebUart"),                 /* pszCmdStr */
		TEXT("dvpd"),
		_CLI_OpenDvpDebugUart,             /* execution function */
		NULL,
		TEXT("Open Dvp Debug Uart"),
		CLI_GUEST
	},
	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};



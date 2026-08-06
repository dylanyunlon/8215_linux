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

/*-----------------------------------------------------------------------------

 * $RCSfile: cli_input.c,v $
 * $Revision: #5 $
 * $Date: 2015/11/17 $
 * $Author: daoyu.chen $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/15 $
 * $SWAuthor: Clear Case Administrator $
 * $MD5HEX: 96b0a89975193784c9eddfef11dec3ef $
 *
 * Description:
 *         This program will handle input from UART to CLI console.
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include <linux/mm.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"

#include <linux/types.h>

//#include "reg_serial.h"
#include "ac83xx_cli.h"

extern CLI_ACCESS_RIGHT_T       e_access_right;



/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define MAX_STMT_ROW                            ((u32)   256)
#define MAX_STMT_LEN                            ((u32)   512)
#define MAX_CLI_ENABLE_PASSWD_TRY               ((u32)     3)
#define SET_UART_TO_NORMAL_MODE_CMD_1           "basic.stop"
#define SET_UART_TO_NORMAL_MODE_ABBR_CMD_1      "b.stop"
#define SET_UART_TO_NORMAL_MODE_CMD_2           "0.st"
bool fgSet_Uart_BaudRate(u32 baudRate);

void CLI_Enable_Uart6(void);
void CLI_Disable_Uart6(void);


/* CLI debug output control */
typedef enum {
	CLI_WITH_DBG_STMT = 0,
	CLI_DISCARD_DBG_STMT,
	CLI_BUFFER_DBG_STMT
} CLI_DBG_CTRL;

/* CLI operation mode */
typedef enum {
	CLI_ENABLED = 0,
	CLI_ENABLING,
	CLI_DISABLED
} CLI_OP_MODE;

/*-----------------------------------------------------------------------------
					data declarations
 ----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT

bool             b_cli_init = FALSE;
static u32           ui4_cli_cmd_buf_row_idx;
static u32           ui4_cli_cmd_buf_ref_row_idx;
static u32           ui4_cli_cmd_buf_idx;
static u32           ui4_ctrl_stmt_buf_idx;
static s8             s_cli_prompt_str[CLI_CMD_BUF_SIZE];

static s8 _szCliCmd[CLI_CMD_BUF_SIZE];
s8 _szCliCmdBuffer[CLI_CMD_BUF_SIZE];
u32 _u4CmdReadPtr = 0;


HANDLE _hDmnrSema = NULL;
static HANDLE _hThread;

bool _fgKernelLogEnable = TRUE;
bool g_fgDmnrConnected = FALSE;

u32 _u4UartMode = UART_TP_MODE;
static int  CliThreadProc(void *lpParameter);

#endif

/*-----------------------------------------------------------------------------
					function declarations
 ----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT

void cli_enable_kernel_log(bool fgEnable)
{
/* KernelIoControl(IOCTL_HAL_ENABLE_UART_LOG,
	&fgEnable,
	sizeof(bool),
	NULL,
	0,
	NULL );*/
}

/*-----------------------------------------------------------------------------
 * Name: cli_get_char
 *
 * Description: This API waits for the first available character.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: ASCII_NULL      The UART queue is empty.
 *          Others          The first available character from UART queue.
 ----------------------------------------------------------------------------*/
s8 cli_get_char(void)
{
#ifdef CLI_SUPPORT
	s8 c_char = ASCII_NULL;

	if (UART_READ32(0x4) & 0x1) {
		c_char = UART_READ32(0);
	}

	return c_char;
#else
	return ASCII_NULL;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: cli_get_char_timeout
 *
 * Description: This API waits for the first available character within
 *              specific time period.
 *
 * Inputs:  ui4_time    The waiting time period in ms.
 *
 * Outputs: -
 *
 * Returns: ASCII_NULL      The UART queue is empty.
 *          Others          The first available character from UART queue.
 ----------------------------------------------------------------------------*/
s8 cli_get_char_timeout(u32 ui4_time)
{
#ifdef CLI_SUPPORT
	s8 c_char = ASCII_NULL;

	return c_char;
#else
	return ASCII_NULL;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: cli_get_string
 *
 * Description: This API waits for the first available string.
 *
 * Inputs:  b_show_srt      Indicate if star signs are shown when character
 *                          input.
 *
 * Outputs: ps_str_buf      The first available string from UART queue.
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
void cli_get_string(s8 *ps_str_buf)
{
#ifdef CLI_SUPPORT
	s8        c_char;
	u32      ui4_idx = 0;
	u32      idx = 0;

	wait_for_completion(&clistrsync.cli_complete);
#if 0
	do {
		/* c_char = cli_get_char(); */
		c_char = clistrsync.clistr[idx++];

		if (!b_cli_init) {
			break;
		}
		if ((c_char != ASCII_KEY_ENTER) && (c_char != ASCII_NULL)) {
			if (c_char == ASCII_KEY_ESC) {
				/* c_char = cli_get_char(); */
				c_char = clistrsync.clistr[idx++];
				if (c_char == ASCII_KEY_LEFT) {
					if (ui4_idx > 0) {
						ui4_idx--;
					}
				}
			} else if (c_char == ASCII_KEY_BS) {
				if (ui4_idx > 0) {
					ui4_idx--;
				}
			} else {
				if ((c_char >= ASCII_KEY_PRINTABLE_MIN) &&
						(c_char <= ASCII_KEY_PRINTABLE_MAX)) {
					if (ui4_idx == CLI_CMD_BUF_SIZE) {
						continue;
					}

					ps_str_buf[ui4_idx++] = c_char;
				}
			}
		}
	} while (c_char != ASCII_KEY_ENTER);
#else
	do {
		c_char = clistrsync.clistr[idx++];
		if (c_char == 0x00)
			break;
		ps_str_buf[ui4_idx++] = c_char;
	} while (1);
#endif
	ps_str_buf[ui4_idx] = ASCII_NULL;

	memset(clistrsync.clistr, 0x00, 256);

#else
	ps_str_buf[0] = ASCII_NULL;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: cli_get_prompt_str_buf
 *
 * Description: This API returns the address of CLI prompt string buffer.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: The address of CLI prompt string buffer.
 ----------------------------------------------------------------------------*/
s8 *cli_get_prompt_str_buf(void)
{
#ifdef CLI_SUPPORT
	return s_cli_prompt_str;
#else
	return NULL;
#endif
}



/*-----------------------------------------------------------------------------
 * Name: cli_is_inited
 *
 * Description: This API returns the initialization status of CLI component.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: TRUE        The CLI has already been initialized.
 *          FALSE       The CLI has not been initialized.
 ----------------------------------------------------------------------------*/
bool cli_is_inited(void)
{
#ifdef CLI_SUPPORT
	return b_cli_init;
#else
	return FALSE;
#endif
}

extern CLI_EXEC_T *CLI_GetDriverCmdTbl(void);


/*-----------------------------------------------------------------------------
 * Name: cli_init
 *
 * Description: CLI initialization function.
 *
 * Inputs:  pt_thread_descr      References a thread descriptor structure.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                     Routine successful.
 *          CLIR_ALREADY_INIT           The CLI has already been initialized.
 ----------------------------------------------------------------------------*/
s32 cli_init()
{
	/* Use interrupt to print log info.  cjzhang@2008/11/19 */
	/* SerOutputLogUseInterrupt(TRUE); */
	int ret = 0;

#ifdef CLI_SUPPORT

	if (b_cli_init) {
		return CLIR_ALREADY_INIT;
	}

	/* Initialize internal parameters */
	ui4_cli_cmd_buf_row_idx = 0;
	ui4_cli_cmd_buf_ref_row_idx = 0;
	ui4_cli_cmd_buf_idx = 0;
	ui4_ctrl_stmt_buf_idx = 0;


	/* Initialize default command tables and set up prompt string */
	if (cli_parser_clear_cmd_tbl() != CLIR_OK) {
		/* Mandatory table will be loaded here */
		/* CLI_ABORT(1); */
	}

#if (ALIAS_SUPPORT)
	/* Initialize alias table */
	if (cli_alias_init() != CLIR_OK) {
		/* CLI_ABORT(1); */
	}
#endif
	e_access_right = CLI_MODE_SUPERVISOR;
	b_cli_init = TRUE;

	x_cli_attach_cmd_tbl(CLI_GetDriverCmdTbl(), CLI_CAT_DRV, CLI_GRP_NONE);

	/* init_completion(&cli_complete); */
	_hThread = kthread_run(CliThreadProc, NULL, "CliThreadProc");
	if (IS_ERR(_hThread)) {
		ret = PTR_ERR(_hThread);
		_hThread = NULL;
		pr_err(TEXT("CliThreadProc errorerrorerror\r\n"));
	}

	return CLIR_OK;
#endif
}

s32 cli_uninit()
{
	if (!b_cli_init) {
		return CLIR_NOT_INIT;
	}

	complete(&clistrsync.cli_complete);
	b_cli_init = FALSE;

	kthread_stop(_hThread);

	mdelay(50);

	return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: x_cli_attach_cmd_tbl
 *
 * Description: This API attaches a command table to CLI.
 *
 * Inputs:  pt_tbl          The Command table to be attached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 *          CLIR_GROUP_TBL_FULL     Module-to-group table is full.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 x_cli_attach_cmd_tbl(CLI_EXEC_T *pt_tbl,
						CLI_CAT_T   e_category,
						u64         ui8_group_mask)
{
#ifdef CLI_SUPPORT
	if (!b_cli_init) {
		return CLIR_NOT_INIT;
	}

	/* Check arguments */
	if (pt_tbl == NULL) {
		return CLIR_INV_ARG;
	}
	return cli_parser_attach_cmd_tbl(pt_tbl, e_category, ui8_group_mask);
#else
	return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: x_cli_detach_cmd_tbl
 *
 * Description: This API detaches a command table from CLI.
 *
 * Inputs:  pt_tbl          The Command table to be detached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_NULL       Command table is null.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 x_cli_detach_cmd_tbl(CLI_EXEC_T *pt_tbl,
						CLI_CAT_T   e_category,
						u64      ui8_group_mask)
{
#ifdef CLI_SUPPORT
	if (!b_cli_init) {
		return CLIR_NOT_INIT;
	}

	/* Check arguments */
	if (pt_tbl == NULL) {
		return CLIR_INV_ARG;
	}

	return cli_parser_detach_cmd_tbl(pt_tbl, e_category, ui8_group_mask);
#else
	return CLIR_NOT_ENABLED;
#endif
}

#if 0

/*-----------------------------------------------------------------------------
 * Name: x_cli_attach_alias
 *
 * Description: This API adds, deletes, or raplaces an alias element in alias
 *              table.
 *
 * Inputs:  ps_alias    Contains the alias to attach.
 *          ps_cmd      Contains the command string corresponding to the alias.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_ALIAS_TOO_LONG     Alias is too long.
 *          CLIR_CMD_TOO_LONG       CLI command is too long.
 *          CLIR_ALIAS_TBL_FULL     CLI alias table is full.
 *          CLIR_CMD_EXEC_ERROR     CLI command execution failed.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 x_cli_attach_alias(const s8 *ps_alias, const s8 *ps_cmd)
{
#ifdef CLI_SUPPORT
	if (!b_cli_init) {
		return CLIR_NOT_INIT;
	}

	/* Check arguments */
	if ((ps_alias == NULL) || (ps_cmd == NULL)) {
		return CLIR_INV_ARG;
	}

	return cli_alias_attach(ps_alias, ps_cmd);
#else
	return CLIR_NOT_ENABLED;
#endif
}

#endif
/*-----------------------------------------------------------------------------
 * Name: x_cli_parser
 *
 * Description: This API parses CLI command and performs corresponding
 *              operation.
 *
 *
 * Inputs:  ps_cmd      Contain the command to parse.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_UNKNOWN_CMD        Unknown CLI command.
 *          CLIR_CMD_NOT_FOUND      CLI command not found.
 *          CLIR_DIR_NOT_FOUND      CLI directory not found.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 x_cli_parser(const s8 *ps_cmd)
{

#ifdef CLI_SUPPORT
	s32  ret = CLIR_OK;

	if (!b_cli_init) {
		return CLIR_NOT_INIT;
	}

	ret = cli_parser(ps_cmd);
	if (ret == CLIR_OK) {
		pr_info(TEXT("\n\r%s"), s_cli_prompt_str);
	}


	return ret;
#else
	return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: x_cli_get_current_root
 *
 * Description: This API return current cli root table pointer
 *
 *
 * Inputs:
 *
 * Outputs: -
 *
 * Returns: root table pointer
 ----------------------------------------------------------------------------*/
extern CLI_EXEC_T *pt_cur_cmd_tbl;

void *x_cli_get_current_root(void)
{
	return (void *)pt_cur_cmd_tbl;

}

/*-----------------------------------------------------------------------------
 * Name: x_cli_get_char_timeout
 *
 * Description: This API waits for the first available character within
 *              specific time period.
 *
 * Inputs:  ui4_time    The waiting time period in ms.
 *
 * Outputs: -
 *
 * Returns: ASCII_NULL      The UART queue is empty.
 *          Others          The first available character from UART queue.
 ----------------------------------------------------------------------------*/
s8 x_cli_get_char_timeout(u32 ui4_time)
{
#if defined(CLI_SUPPORT)
	return cli_get_char_timeout(ui4_time);
#else
	return CLIR_NOT_ENABLED;
#endif
}


static int  CliThreadProc(void *lpParameter)
{
	while (b_cli_init) {
		cli_get_string(_szCliCmd);
		if (_u4UartMode == UART_NORMAL_MODE) {
			_u4UartMode = UART_TP_MODE;
		}
		x_cli_parser(_szCliCmd);
	}

	return 0;
}



/*static s32 _CLI_UartEnterDebug(s32 i4Argc, const s8 **szArgv)
{
	CLI_Printf((TEXT("\r\n UART will enter normal.\r\n")));
	//cli_enable_kernel_log(FALSE);
	SwitchUartMode(UART_NORMAL_MODE);
	_u4UartMode = UART_NORMAL_MODE;

	return 0;
}*/

/*static s32 _CLI_UartDisableKernelLog(s32 i4Argc, const s8 **szArgv)
{
	if (_fgKernelLogEnable) {
		CLI_Printf((TEXT("\r\n Kernel log will be disabled.\r\n")));
		_fgKernelLogEnable = FALSE;
		//cli_enable_kernel_log(_fgKernelLogEnable);
	}
	return 0;
}*/

/*static s32 _CLI_UartEnableKernelLog(s32 i4Argc, const s8 **szArgv)
{
	if (!_fgKernelLogEnable) {
		_fgKernelLogEnable = TRUE;
	}
	//cli_enable_kernel_log(_fgKernelLogEnable);
	CLI_Printf((TEXT("\r\n Kernel log has been enabled.\r\n")));
	return 0;
}*/
/*static s32 _CLI_ChangeSourceClock(s32 i4Argc, const s8 **szArgv)
{
	u32 u4SourceCLK  = 32400000;

	if (i4Argc < 2) {
		CLI_Printf((TEXT("Current RS232 Source Clock = %d\r\n"),
				u4SourceCLK));
		return 0;
	}

	u4SourceCLK = StrToInt(szArgv[1]);
	CLI_Printf((TEXT("Change RS232 Source Clock to  %d\r\n"), u4SourceCLK));
	fgSet_Uart_SourceCLK(u4SourceCLK);
	return 0;
}*/


/*static s32 _CLI_ChangeSampleRate(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Sample  = 115200;

	if (i4Argc < 2 ) {
		CLI_Printf((TEXT("Current RS232 Sample Rate = %d\r\n"), u4Sample));
		return 0;
	}

	u4Sample = StrToInt(szArgv[1]);
	CLI_Printf((TEXT("Change RS232 Sample Rate to  %d\r\n"), u4Sample));
	fgSet_Uart_BaudRate(u4Sample);
	return 0;
}
static s32 _CLI_Uart6_Start(s32 i4Argc, const s8 **szArgv)
{
	CLI_Printf((TEXT("\r\n Uart6 Run Smoothly.\r\n")));

	CLI_Enable_Uart6();

	return 0;
}
static s32 _CLI_Uart6_Stop(s32 i4Argc, const s8 **szArgv)
{
	CLI_Printf((TEXT("\r\n Uart6 End.\r\n")));

	CLI_Disable_Uart6();

	return 0;
}*/

/******************************************************************************
* CLI Commond for UART
******************************************************************************/
/*CLI_EXEC_T _arUartCmdTbl[] = {
	{
		TEXT("EnterDebug"),                    //pszCmdStr
		TEXT("i"),
		_CLI_UartEnterDebug,                       //execution function
		NULL,
		TEXT("Enter debug mode."),
		CLI_GUEST
	},
	{
		TEXT("LogDisable"),                  //pszCmdStr
		TEXT("d"),
		_CLI_UartDisableKernelLog,              //execution function
		NULL,
		TEXT("Disable kernel log output"),
		CLI_GUEST
	},
	{
		TEXT("LogEnable"),                  //pszCmdStr
		TEXT("e"),
		_CLI_UartEnableKernelLog,              //execution function
		NULL,
		TEXT("Enable kernel log output"),
		CLI_GUEST
	},
	{
		TEXT("ChangeSC"),					//pszCmdStr
		TEXT("csc"),
		_CLI_ChangeSourceClock,			   //execution function
		NULL,
		TEXT("Change RS232 Source Clock"),
		CLI_GUEST
	},
	{
		TEXT("ChangeSR"),					//pszCmdStr
		TEXT("csr"),
		_CLI_ChangeSampleRate,			   //execution function
		NULL,
		TEXT("Change RS232 Sample Rate"),
		CLI_GUEST
	},
	{
		TEXT("Uart6Start"),					//pszCmdStr
		TEXT("6y"),
		_CLI_Uart6_Start,			   //execution function
		NULL,
		TEXT("Uart6 Run&RS232 End"),
		CLI_GUEST
	},
	{
		TEXT("Uart6Stop"),					//pszCmdStr
		TEXT("6n"),
		_CLI_Uart6_Stop,			   //execution function
		NULL,
		TEXT("Uart6 End&RS232 Run"),
		CLI_GUEST
	},

	// last cli command record, NULL
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};*/


#endif

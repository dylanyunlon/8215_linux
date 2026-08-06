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
 * $RCSfile: _cli.h,v $ -cli_h,v $
 * $Revision: #6 $
 * $Date: 2015/11/25 $
 * $Author: daoyu.chen $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/7 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: ca6d601bce72fe5d2621e6dd83df1947 $
 *
 * Description:
 *         This is CLI internal include file
 *---------------------------------------------------------------------------*/

#ifndef __CLI_H_
#define __CLI_H_

/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/

#include <windows.h>
#include "u_cli.h"
#include <linux/string.h>
#include <asm/io.h>

#include <linux/types.h>
#include <linux/printk.h>
/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef IO_VIRT
#define IO_VIRT             (0xFD000000)
#endif

/* #define RS232_VIRT                  (IO_VIRT + 0x0000C000) */
#define CKGEN_VIRT_CLI                  (IO_VIRT + 0x0000D000)

#define UART_READ32(REG)            \
	__raw_readl((const volatile void *)(RS232_VIRT + REG))
#define UART_WRITE32(VAL, REG)      __raw_writel(VAL, (volatile void *)(RS232_VIRT + REG))

#define  DRAM_READ32(addr)    HAL_READ32(phys_to_virt((unsigned long)(addr)))
#define  DRAM_WRITE32(addr, val)    \
	HAL_WRITE32(phys_to_virt((unsigned long)(addr)), (val))

#define HAL_WRITE32(_reg_, _val_)    (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)            (*((volatile uint32_t*)(_reg_)))

#define IO_READ32(base, offset)         HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value) HAL_WRITE32((base) + (offset), (value))

/*
#define CKGEN_READ32(offset)            IO_READ32(CKGEN_VIRT_CLI, (offset))
#define CKGEN_WRITE32(offset, value)    \
	IO_WRITE32(CKGEN_VIRT_CLI, (offset), (value))
*/

extern bool _fgKernelLogEnable;

void UART_Printf(const char *sz, ...);


#undef x_strncat
#undef x_memset
#undef x_strlen
#undef x_strncmp
#undef x_strcmp
#undef x_strchr
#undef x_sprintf
#undef x_strncpy
#undef x_strcpy


#define x_strncat   strncat   /*  _tcsncat  */
#define x_memset    memset
#define x_strlen    strlen    /*  _tcslen   */
#define x_strncmp   strncmp   /*  _tcsncmp  */
#define x_strcmp    strcmp    /*  _tcscmp   */
#define x_strchr    strchr    /*  _tcschr   */
#define x_sprintf   sprintf   /*  _stprintf */
#define x_strncpy   strncpy   /*  _tcsncpy  */
#define x_strcpy    strcpy    /*  _tcscpy   */

/* ASCII key definiton */
#define ASCII_NULL                      ((s8) 0x00)
#define ASCII_KEY_CTRL_B                ((s8) 0x02)
#define ASCII_KEY_CTRL_C                ((s8) 0x03)
#define ASCII_KEY_CTRL_D                ((s8) 0x04)
#define ASCII_KEY_CTRL_L                ((s8) 0x0c)
#define ASCII_KEY_BS                    ((s8) 0x08)
#define ASCII_KEY_NL                    ((s8) 0x0a)
#define ASCII_KEY_ENTER                 ((s8) 0x0d)
#define ASCII_KEY_CTRL_W                ((s8) 0x17)
#define ASCII_KEY_ESC                   ((s8) 0x1b)
#define ASCII_KEY_SPACE                 ((s8) 0x20)
#define ASCII_KEY_DBL_QUOTE             ((s8) 0x22)
#define ASCII_KEY_PUNCH                 ((s8) 0x23)
#define ASCII_KEY_SGL_QUOTE             ((s8) 0x27)
#define ASCII_KEY_DOT                   ((s8) 0x2e)
#define ASCII_KEY_DOLLAR                ((s8) 0x24)
#define ASCII_KEY_UP                    ((s8) 0x41)
#define ASCII_KEY_DOWN                  ((s8) 0x42)
#define ASCII_KEY_RIGHT                 ((s8) 0x43)
#define ASCII_KEY_LEFT                  ((s8) 0x44)
#define ASCII_KEY_ARROW                 ((s8) 0x5b)
#define ASCII_KEY_ROOT                  ((s8) 0x2f)
#define ASCII_KEY_PRINTABLE_MIN         ((s8) 0x20)
#define ASCII_KEY_PRINTABLE_MAX         ((s8) 0x7e)

/* CLI input related */
#define CLI_PROMPT_STR          TEXT("Command") /* CLI prompt string */


#define CLI_PROMPT_DIAG_STR     TEXT("Command")

#define CLI_CMD_BUF_SIZE        1024 /* Maximum size of command buffer */
#define CLI_CMD_BUF_ROW_NUM     16   /* Number of command buffers supported */

/* CLI parser related */
#define CLI_ALL_STR                  TEXT("all")
#define CLI_NONE_STR                 TEXT("none")

#define CLI_GRP_PIPE_STR             TEXT("pipe")
#define CLI_GRP_GUI_STR              TEXT("gui")
#define CLI_GRP_EPG_STR              TEXT("epg")
#define CLI_GRP_DRV_STR              TEXT("drv")

#define CLI_CAT_ROOT_STR             TEXT("root")
#define CLI_CAT_BASIC_STR            TEXT("basic")
#define CLI_CAT_BASIC_ABBR_STR       TEXT("b")
#define CLI_CAT_APP_STR              TEXT("app")
#define CLI_CAT_MW_STR               TEXT("mw")
#define CLI_CAT_MMW_STR              TEXT("mmw")
#define CLI_CAT_DRV_STR              TEXT("drv")
#define CLI_CAT_TEST_STR             TEXT("test")
#define CLI_CAT_TEST_ABBR_STR        TEXT("t")
#define CLI_CAT_MTK_TOOL_STR         TEXT("mtktool")
#define CLI_CAT_MTK_TOOL_ABBR_STR    TEXT("0")
#define CLI_CAT_FACTORY_STR          TEXT("factory")


#define CLI_DEBUG_LEVEL_STR          TEXT("debug_level")
#define CLI_D_L_STR                  TEXT("d_l")

#define CLI_AR_SUPERVISOR_STR        TEXT("supervisor")
#define CLI_AR_ADMIN_STR             TEXT("admin")
#define CLI_AR_GUEST_STR             TEXT("guest")

/* Maximum number of command arguments supported */
#define CLI_MAX_ARG_NUM             64

#define CLI_MAX_ARG_LEN             128 /* Maximum size of an argument */

/* Maximum number of attached command tables */
#define CLI_MAX_CMD_TBL_NUM         4

#define CLI_MAX_CMD_TBL_LEVEL       8   /* Maximum level of command table */
#define CLI_MANDA_CMD_TBL_IDX       0   /* Mandatory command table index */

#ifndef CLI_ACCESS_RIGHT
#define CLI_ACCESS_RIGHT                CLI_MODE_SUPERVISOR
#endif

/* CLI alias related */
#define CLI_ALIAS_CMD_STR         TEXT("alias") /* Alias command string */
#define CLI_ALIAS_CMD_ABBR_STR    TEXT("a") /* Alias command abbreviation */
#define CLI_MAX_ALIAS_NUM         64   /* Maximum number of alias supported */
#define CLI_MAX_ALIAS_LEN         16   /* Maximum size of an alias sting */

/* Macro definition */
/*#define CLI_ABORT(_expr)    { if (! (_expr)) DBG_ABORT(DBG_MOD_CLI); } */

/*for factory mode only*/
#define KEY_BLOCK_SIZE                        0x8004     /* 32k + 4 bytes */

/*-----------------------------------------------------------------------------
							functions declarations
 ----------------------------------------------------------------------------*/
/* CLI input related */
extern void cli_get_string(s8 *ps_str_buf);

extern s8 *cli_get_prompt_str_buf(VOID);
extern void SetCliPrompt(s32 accessRight);


/* CLI parser related */
extern s32 cli_parser_list_mandatory_tbl_cmd(void);

extern s32 cli_parser_attach_cmd_tbl(CLI_EXEC_T *pt_tbl,
				CLI_CAT_T e_category, u64 ui8_group_mask);

extern s32 cli_parser_detach_cmd_tbl(CLI_EXEC_T *pt_tbl,
				CLI_CAT_T e_category, u64 ui8_group_mask);


extern s32 cli_parser_clear_cmd_tbl(void);

extern s32 cli_parser(const s8 *ps_cmd);
extern void cli_parser_init(void);

/* CLI alias related */
extern s32 cli_alias_init(void);

extern s32 cli_alias_attach(const s8 *ps_alias, const s8 *ps_cmd);

extern const s8 *cli_alias_search(const s8 *ps_alias);

extern s32 cli_alias(s32 i4_argc, const s8 **pps_argv);
#ifdef __cplusplus
}
#endif


#endif /* __CLI_H_ */


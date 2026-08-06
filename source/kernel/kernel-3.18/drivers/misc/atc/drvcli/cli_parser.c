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
 * $RCSfile: cli_parser.c,v $
 * $Revision: #5 $
 * $Date: 2015/11/17 $
 * $Author: daoyu.chen $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/18 $
 * $SWAuthor: Clear Case Administrator $
 * $MD5HEX: b794305807e8df685f62004bee4502b0 $
 *
 * Description:
 *         This program will handle string parsing of user's input to CLI
 *         console.
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include <linux/mm.h>

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "_cli.h"
#include "cli.h"

#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>

/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define HELP_STR_POS            ((u32)    24)
#define MAX_STR_LEN             ((u32)   128)

#define IS_PRINTABLE(c)    ((((c) > ASCII_NULL) && ((c) < ASCII_KEY_SPACE)) \
		? 0 : 1)
#define IS_SPACE(c)        (((c) == ' ' || (c) == '\n' || (c) == '\t' || \
		(c) == '\r' || (c) == '\a') ? 1 : 0)
#define IS_DOT(c)          (((c) == ASCII_KEY_DOT) ? 1 : 0)
#define IS_ROOT(c)         (((c) == ASCII_KEY_ROOT) ? 1 : 0)

/* Definition of group info structure */
typedef struct _CLI_GRP_T {
	CLI_EXEC_T       *pt_cmd_tbl;
	u64              aui8_grp_msk;
} CLI_GRP_T;

/* Definition of category-to-category string structure */
typedef struct _CLI_CAT_N_STR_T {
	u8       aui8_cat;
	s8       *ps_cat_str;
} CLI_CAT_N_STR_T;

/* Definition of group mask-to-group string structure */
typedef struct _CLI_GRP_N_STR_T {
	u64      aui8_grp_msk;
	s8       *ps_grp_str;
} CLI_GRP_N_STR_T;

/*-----------------------------------------------------------------------------
					data declarations
 ----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT
static CLI_EXEC_T    *apt_cli_array[CLI_CAT_MAX][CLI_MAX_CMD_TBL_NUM];
u32                  aui4_cli_tbl_cnt[CLI_CAT_MAX];
static CLI_GRP_T     at_cli_mod_grp_info[CLI_CAT_MAX][CLI_MAX_CMD_TBL_NUM];
static s8            as_argv[CLI_MAX_ARG_NUM][CLI_MAX_ARG_LEN];
static CLI_EXEC_T    *apt_dir_link[CLI_MAX_CMD_TBL_LEVEL];
static u32           ui4_dir_link_idx;
CLI_EXEC_T           *pt_cur_cmd_tbl;
static bool          b_is_cmd_tbl;
static s8            *ps_cli_prompt_str;

CLI_ACCESS_RIGHT_T       e_access_right = CLI_ACCESS_RIGHT;

/* Access right */
static s8 *ps_access_right[] = {
	CLI_AR_SUPERVISOR_STR,
	CLI_AR_ADMIN_STR,
	CLI_AR_GUEST_STR,
};


static s32 _cli_cmd_change_dir(s32 i4_argc, const s8 **pps_argv);
static s32 _cli_cmd_list_cmd(s32 i4_argc, const s8 **pps_argv);
static s32 _cli_cmd_repeat(s32 i4_argc, const s8 **pps_argv);
static s32 _cli_cmd_get_access_right(s32 i4_argc, const s8 **pps_argv);

static s32 _cli_cmd_set_supervisor_access_right(s32 i4_argc,
		const s8 **pps_argv);
static s32 _cli_cmd_set_admin_access_right(s32 i4_argc, const s8 **pps_argv);



/* Mandatory command table */
static CLI_EXEC_T at_mandatory_cmd_tbl[] = {
	{TEXT("cd"), NULL, _cli_cmd_change_dir,
			NULL, TEXT("Change directory"), CLI_HIDDEN},
	{TEXT("ls"), NULL, _cli_cmd_list_cmd,
			NULL, TEXT("List commands"), CLI_HIDDEN},
	{TEXT("dir"), NULL, _cli_cmd_list_cmd,
			NULL, TEXT("List commands"), CLI_HIDDEN},
	{TEXT("do"), NULL, _cli_cmd_repeat,
			NULL, TEXT("Repeat command"), CLI_HIDDEN},
	{TEXT("whoami"), NULL, _cli_cmd_get_access_right,
			NULL, TEXT("Get access right"), CLI_HIDDEN},
	{TEXT("suadmin"), NULL, _cli_cmd_set_admin_access_right,
			NULL, TEXT("Change access right"), CLI_HIDDEN},
	{TEXT("sumtk"), NULL, _cli_cmd_set_supervisor_access_right,
			NULL, TEXT("Change access right"), CLI_HIDDEN},
	END_OF_CLI_CMD_TBL
};

extern CLI_EXEC_T _arBasicCmdTbl[];

/* Basic command table */
static CLI_EXEC_T at_basic_cmd_tbl[] = {
	{CLI_CAT_BASIC_STR, CLI_CAT_BASIC_ABBR_STR, NULL,
			NULL, TEXT("Basic"), CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* Application command table */
static CLI_EXEC_T at_app_cmd_tbl[] = {
	{CLI_CAT_APP_STR, NULL, NULL, NULL, TEXT("Application"), CLI_ADMIN},
	END_OF_CLI_CMD_TBL
};

/* Middleware command table */
static CLI_EXEC_T at_mw_cmd_tbl[] = {
	{CLI_CAT_MW_STR, NULL, NULL, NULL, TEXT("Middleware"), CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* Multimedia Middleware command table */
static CLI_EXEC_T at_mmw_cmd_tbl[] = {
	{CLI_CAT_MMW_STR, NULL, NULL, NULL, TEXT("Multimedia Middleware"),
			CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* Driver command table */
static CLI_EXEC_T at_drv_cmd_tbl[] = {
	{CLI_CAT_DRV_STR, NULL, NULL, NULL, TEXT("Driver"), CLI_ADMIN},
	END_OF_CLI_CMD_TBL
};

/* Test command table */
static CLI_EXEC_T at_test_cmd_tbl[] = {
	{CLI_CAT_TEST_STR, CLI_CAT_TEST_ABBR_STR, NULL,
			NULL, TEXT("Test"), CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* MTK tool command table */
static CLI_EXEC_T at_mtk_tool_cmd_tbl[] = {
	{CLI_CAT_MTK_TOOL_STR, CLI_CAT_MTK_TOOL_ABBR_STR, NULL,
			NULL, TEXT("MTK tool"), CLI_GUEST},
	END_OF_CLI_CMD_TBL
};


#endif

/*-----------------------------------------------------------------------------
					function declarations
 ----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT
static bool _is_cmd_tbl_array(CLI_EXEC_T *pt_tbl);

static void _get_cmd_tbl_array(CLI_EXEC_T    *pt_tbl,
								bool          b_skip_manda_tbl,
								u32           *pui4_tbl_cnt,
								CLI_EXEC_T ***pppt_cmd_tbl_array);

static bool _format_help_str(s8      *ps_dst,
							u32      ui4_dst_len,
							const s8 *ps_cmd_str,
							const s8 *ps_cmd_abbr_str,
							const s8 *ps_cmd_help_str);

static void _show_help(CLI_EXEC_T *pt_tbl);

static s32 _find_cmd_argv(const s8     *ps_cmd,
							u32        ui4_argv_num,
							u32        ui4_argv_len,
							s8 * const *pps_argv);

static CLI_EXEC_T *_search_tbl(const s8 *ps_cmd, CLI_EXEC_T *pt_tbl);

static s32 _parse_cmd(s32      i4_argc,
					const s8 **pps_argv,
					CLI_EXEC_T *pt_tbl);

static void _generate_prompt(void);

/*-----------------------------------------------------------------------------
 * Name: _get_cmd_tbl_array
 *
 * Description: The API checks if input command table links to a command table
 *              array.
 *
 * Inputs:  pt_tbl      The command table to check.
 *
 * Outputs: -
 *
 * Returns: TRUE        This table links to a command table array.
 *          FALSE       This table doea not link to a command table array.
 ----------------------------------------------------------------------------*/
static bool _is_cmd_tbl_array(CLI_EXEC_T *pt_tbl)
{
	if ((pt_tbl == NULL) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_BASIC_STR) == 0 ) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_BASIC_ABBR_STR) == 0 ) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_APP_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MW_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MMW_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_DRV_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_TEST_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MTK_TOOL_STR) == 0 ) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MTK_TOOL_ABBR_STR) == 0 )) {

		return TRUE;
	} else {
		return FALSE;
	}
}


/*-----------------------------------------------------------------------------
 * Name: _get_cmd_tbl_array
 *
 * Description: The API returns current available command table array and its
 *              element count.
 *
 * Inputs:  pt_tbl                  The command table to reference.
 *          b_skip_manda_tbl        Indicates if skipping mandatory command
 *                                  table is required.
 *
 * Outputs: pui4_tbl_cnt            The command table element count.
 *          pppt_cmd_tbl_array      The command table array.
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void _get_cmd_tbl_array(CLI_EXEC_T   *pt_tbl,
								bool        b_skip_manda_tbl,
								u32         *pui4_tbl_cnt,
								CLI_EXEC_T  ***pppt_cmd_tbl_array)

{
	if (pt_tbl == NULL) {/* Root */
		if (b_skip_manda_tbl) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_ROOT] - 1;
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][CLI_MANDA_CMD_TBL_IDX + 1]);
		} else {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_ROOT];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
		}
	} else {
		if ((x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_BASIC_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_BASIC_ABBR_STR) == 0)) {

			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_BASIC];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_BASIC][0]);
		} else if (x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_APP_STR) == 0) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_APP];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
		} else if (x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MW_STR) == 0) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_MW];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
		} else if (x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MMW_STR) == 0) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_MMW];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MMW][0]);
		} else if (x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_DRV_STR) == 0) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_DRV];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_DRV][0]);
		} else if (x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_TEST_STR) == 0) {
			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_TEST];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_TEST][0]);
		} else if ((x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MTK_TOOL_STR) == 0) ||
			(x_strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MTK_TOOL_ABBR_STR) == 0)) {

			*pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_MTK_TOOL];
			*pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MTK_TOOL][0]);
		} else {
			*pui4_tbl_cnt = 0;
			*pppt_cmd_tbl_array = NULL;
		}
	}
}


/*-----------------------------------------------------------------------------
 * Name: _format_help_str
 *
 * Description: The API formats help string for display.
 *
 * Inputs:  ui4_dst_len         Destination buffer size.
 *          ps_cmd_str          Command string.
 *          ps_cmd_abbr_str     Command abbreviation string.
 *          ps_cmd_help_str     Command description.
 *
 * Outputs: ps_dst              Destination buffer.
 *
 * Returns: TRUE                Routine successful.
 *          FALSE               Routine failed.
 ----------------------------------------------------------------------------*/
static bool _format_help_str(s8      *ps_dst,
							u32      ui4_dst_len,
							const s8 *ps_cmd_str,
							const s8 *ps_cmd_abbr_str,
							const s8 *ps_cmd_help_str)
{
	u32      ui4_cmd_str_len;
	u32      ui4_cmd_abbr_str_len;
	u32      ui4_help_str_len;
	u32      ui4_idx;

	/* Check arguments */
	if (ps_cmd_str == NULL) {
		return FALSE;
	}

	/* Format => xxx(xxx):        xxxxxxxx */
	ui4_cmd_str_len = x_strlen(ps_cmd_str);
	if (ps_cmd_abbr_str != NULL) {
		ui4_cmd_abbr_str_len = x_strlen(ps_cmd_abbr_str) + 2; /* (xxx) */
	} else {
		ui4_cmd_abbr_str_len = 2;
	}

	if (ps_cmd_help_str != NULL) {
		ui4_help_str_len = x_strlen(ps_cmd_help_str);
	} else {
		ui4_help_str_len = 0;
	}

	/* Check if destination buffer size is big enough */
	if (ui4_cmd_str_len + ui4_cmd_abbr_str_len + ui4_help_str_len + 1 >
			ui4_dst_len) {
		return FALSE;
	}

	/* Format command and command abbreviation strings */
	switch (e_access_right) {
	default:
		if (ps_cmd_abbr_str != NULL) {
			x_sprintf(ps_dst, TEXT("\n\r %s(%s):\n\r"), ps_cmd_str, ps_cmd_abbr_str);
		} else {
			x_sprintf(ps_dst, TEXT("\n\r %s:\n\r"), ps_cmd_str);
		}
		break;
	}


	/* Copy command help string */
	if (ps_cmd_help_str != NULL) {
		for (ui4_idx = x_strlen(ps_dst); ui4_idx < HELP_STR_POS;
				ui4_idx++) {
			ps_dst[ui4_idx] = ' ';
		}
		x_strcpy(ps_dst + HELP_STR_POS, ps_cmd_help_str);
	}

	return TRUE;
}


/*-----------------------------------------------------------------------------
 * Name: _show_help
 *
 * Description: This API shows CLI command table help strings.
 *
 * Inputs:  pt_tbl      The command table.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void _show_help(CLI_EXEC_T *pt_tbl)
{
	u32          ui4_cmd_idx = 0;
	u32          ui4_tbl_cnt;
	s8           s_help[MAX_STR_LEN];
	CLI_EXEC_T   *pt_cmd_tbl;
	CLI_EXEC_T   **ppt_cmd_tbl_array;
	u32          ui4_idx;

	if (b_is_cmd_tbl) { /* Commad table */
		if (pt_tbl == NULL) {
			return;
		}
		switch (e_access_right) {
		default:
			pr_info(TEXT("[Help]\n\r"));
			break;
		}
		while (pt_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL) {
			if (_format_help_str(s_help,
					MAX_STR_LEN,
					pt_tbl[ui4_cmd_idx].ps_cmd_str,
					pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
					pt_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE) {
				switch (e_access_right) {
				default:
					if ((pt_tbl[ui4_cmd_idx].e_access_right >= e_access_right)
						&& (pt_tbl[ui4_cmd_idx].e_access_right != CLI_HIDDEN)) {

						pr_err(TEXT("%s\n\r"), s_help);
					}
				break;
				}
			}
			ui4_cmd_idx++;
		}
	} else { /* Command table array */
		_get_cmd_tbl_array(pt_tbl, FALSE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);

		if (ui4_tbl_cnt > 0) {
			switch (e_access_right) {
			default:
				pr_info(TEXT("[Help]\n\r"));
				break;
			}
		}
		for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++) {
			ui4_cmd_idx = 0;
			pt_cmd_tbl = ppt_cmd_tbl_array[ui4_idx];
			while ((pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str != NULL) ||
				(pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str != NULL) ||
				(pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL)) {

				if (_format_help_str(s_help,
						MAX_STR_LEN,
						pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str,
						pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
						pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE) {
					switch (e_access_right) {
					default:
						if ((pt_cmd_tbl[ui4_cmd_idx].e_access_right >=
								e_access_right) &&
								(pt_cmd_tbl[ui4_cmd_idx].e_access_right !=
								CLI_HIDDEN)) {

							pr_info(TEXT("%s\n\r"), s_help);
						}
						break;
					}
				}

				ui4_cmd_idx++;
			}
		}
	}
}


/*-----------------------------------------------------------------------------
 * Name: _find_cmd_argv
 *
 * Description: The API finds all the arguments of a command string and stores
 *              them into argument buffer.
 *
 * Inputs:  ps_cmd              Contains the command to be parse.
 *          ui4_argv_num        The maximum argument number that argument
 *                              buffer supports.
 *          ui4_argv_len        The maximum length per argument.
 *
 * Outputs: pps_argv            Points to argument buffer.
 *
 * Returns: Number of arguments found.
 ----------------------------------------------------------------------------*/
static s32 _find_cmd_argv(const s8     *ps_cmd,
							u32        ui4_argv_num,
							u32        ui4_argv_len,
							s8 * const *pps_argv)
{
	s32      i4_argc = 0;
	s8       *ps_argv;
	s8       c_char;
	u32      ui4_cmd_idx;
	u8       ui1_quote_state; /* 0: no quote found
								1: first single quote (') found
								2: first double quota (") found  */

	/* Check arguments */
	if ((ps_cmd == NULL) ||
		(ui4_argv_num == 0) ||
		(ui4_argv_len == 0) ||
		(pps_argv == NULL)) {

		return 0;
	}

	/* Start finding arguments of a command */
	c_char = *ps_cmd;
	while ((c_char != ASCII_NULL) && (c_char != ASCII_KEY_PUNCH)) {

		ps_argv = pps_argv[i4_argc];

		/* Search the first non-space and printable character */
		while (!IS_PRINTABLE(c_char) || IS_SPACE(c_char)) {
			c_char = *(++ps_cmd);
		}

		/* Grab an argument */
		ui4_cmd_idx = 0;
		ui1_quote_state = 0;
		while (IS_PRINTABLE(c_char) &&
			((ui1_quote_state == 0 && !IS_SPACE(c_char)) ||
			(ui1_quote_state == 1 && c_char != ASCII_KEY_SGL_QUOTE) ||
			(ui1_quote_state == 2 && c_char != ASCII_KEY_DBL_QUOTE))) {

			if (c_char == ASCII_NULL ||             /* End of string */
			ui4_cmd_idx >= (ui4_argv_len - 1)) { /*Exceed maximum arg len*/

				*ps_argv = ASCII_NULL;
				break;
			}

			/* Take care of quote issue */
			if ((ui1_quote_state != 2) &&
					(c_char == ASCII_KEY_SGL_QUOTE)) {
				if (ui1_quote_state == 0) {
					ui1_quote_state = 1;
				} else if (ui1_quote_state == 1) {
					ui1_quote_state = 0;
				}
				c_char = *(++ps_cmd);
				ui4_cmd_idx++;
				continue;
			} else if ((ui1_quote_state != 1) &&
					(c_char == ASCII_KEY_DBL_QUOTE)) {
				if (ui1_quote_state == 0) {
					ui1_quote_state = 2;
				} else if (ui1_quote_state == 2) {
					ui1_quote_state = 0;
				}
				c_char = *(++ps_cmd);
				ui4_cmd_idx++;
				continue;
			}

			/* Copy the character into argument buffer */
			*ps_argv = c_char;
			ps_argv++;
			c_char = *(++ps_cmd);
			ui4_cmd_idx++;
		}

		if ((c_char == ASCII_KEY_SGL_QUOTE) ||
				(c_char == ASCII_KEY_DBL_QUOTE)) {
			c_char = *(++ps_cmd);
			ui4_cmd_idx++;
		}

		if (ui4_cmd_idx > 0) {
			*ps_argv = ASCII_NULL;
			i4_argc++;
		}

		if ((u32)i4_argc >= ui4_argv_num) {
			break;
		}
	}

	return i4_argc;
}


/*-----------------------------------------------------------------------------
 * Name: _search_tbl
 *
 * Description: This API finds out a command table corresponding to the
 *              command.
 *
 * Inputs:  ps_cmd      The command to search.
 *          pt_tbl      The command table for search.
 *
 * Outputs: -
 *
 * Returns: NULL        No matched command table found.
 *          Other       The address of matched command table.
 ----------------------------------------------------------------------------*/
static CLI_EXEC_T *_search_tbl(const s8 *ps_cmd, CLI_EXEC_T *pt_tbl)
{
	const s8     *ps_dir_str;
	u32          ui4_dir_str_len;
	u32          ui4_cmd_len;
	u32          ui4_cmd_abbr_len;
	u32          ui4_cmd_idx;
	bool         b_found = FALSE;

	/* Check arguments */
	if ((ps_cmd == NULL) ||
		    (pt_tbl == NULL)) {
		return NULL;
	}

	ui4_dir_str_len = 0;
	ps_dir_str = ps_cmd;
	while ((!IS_DOT(*ps_dir_str)) &&
		   (*ps_dir_str != ASCII_NULL)) {
		ui4_dir_str_len++;
		ps_dir_str++;
	}

	/* Search commmad from current command table */
	ui4_cmd_idx = 0;
	while (pt_tbl[ui4_cmd_idx].ps_cmd_str != NULL) {
		/* Compare command string */
		ui4_cmd_len = x_strlen(pt_tbl[ui4_cmd_idx].ps_cmd_str);
		if ((ui4_dir_str_len == ui4_cmd_len) &&
			(x_strncmp(pt_tbl[ui4_cmd_idx].ps_cmd_str, ps_cmd,
				ui4_dir_str_len) == 0)) {
			b_found = TRUE;
			break;
		}

		/* Compare command abbreviation string */
		if (pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str != NULL) {
			ui4_cmd_abbr_len = x_strlen(pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str);

			if ((ui4_dir_str_len == ui4_cmd_abbr_len) &&
				(x_strncmp(pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str, ps_cmd,
					ui4_dir_str_len) == 0)) {
				b_found = TRUE;
				break;
			}
		}

		ui4_cmd_idx++;
	}

	/* Return the matched command structure */
	if (b_found) {
		return &pt_tbl[ui4_cmd_idx];
	}

	return NULL;
}


/*-----------------------------------------------------------------------------
 * Name: _parse_cmd
 *
 * Description: This API parses and executes a command.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *          pt_tbl          The command table to start parsing.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_INV_CMD_TBL        Invalid CLI command table format.
 *          CLIR_CMD_NOT_FOUND      CLI command not found.
 *          CLIR_DIR_NOT_FOUND      CLI directory not found.
 ----------------------------------------------------------------------------*/
static s32 _parse_cmd(s32 i4_argc, const s8 **pps_argv, CLI_EXEC_T *pt_tbl)
{
	u32          ui4_idx;
	const s8     *ps_dir_str;
	CLI_EXEC_T   *pt_cmd_tbl = NULL;
	u32          ui4_tbl_cnt;
	CLI_EXEC_T   **ppt_cmd_tbl_array;
	bool         b_cmd_tbl_orig;

	/* Check arguments */
	if (pps_argv == NULL) {
		return CLIR_INV_ARG;
	}

	ps_dir_str = pps_argv[0];
	while ((!IS_DOT(*ps_dir_str)) && (*ps_dir_str != ASCII_NULL)) {
		ps_dir_str++;
	}

	if (_is_cmd_tbl_array(pt_tbl) == TRUE) {
		_get_cmd_tbl_array(pt_tbl, FALSE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);
		for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++) {
			pt_cmd_tbl = _search_tbl(pps_argv[0], ppt_cmd_tbl_array[ui4_idx]);
			if (pt_cmd_tbl != NULL) {
				break;
			}
		}
	} else {
		pt_cmd_tbl = _search_tbl(pps_argv[0], pt_tbl);
	}

	/* Execute command */
	if (pt_cmd_tbl != NULL) {
		switch (e_access_right) {
		default:
			if (pt_cmd_tbl->e_access_right >= e_access_right)	{
				if ((pt_cmd_tbl->pt_next_level == NULL) &&
					(pt_cmd_tbl->pf_exec_fct == NULL)) {

					if (_is_cmd_tbl_array(pt_cmd_tbl) != TRUE) {
						return CLIR_INV_CMD_TBL;
					}
				}

				if (pt_cmd_tbl->pf_exec_fct != NULL) {
					if ((pt_cmd_tbl->pt_next_level == NULL) ||
						(!IS_DOT(*ps_dir_str) && (i4_argc > 1))) {
						/* Entry can be a directory or a command */
						return pt_cmd_tbl->pf_exec_fct(i4_argc, pps_argv);
					}
				}

				if (pt_cmd_tbl->pt_next_level != NULL) {
					if (*ps_dir_str == ASCII_NULL) {
						/* Show CLI help if command contains only directory or
							module name */
						if (i4_argc == 1) {
							if (_is_cmd_tbl_array(pt_cmd_tbl) == TRUE) {
								_show_help(pt_cmd_tbl);
							} else {
								b_cmd_tbl_orig = b_is_cmd_tbl;
								b_is_cmd_tbl = TRUE;
								_show_help(pt_cmd_tbl->pt_next_level);
								b_is_cmd_tbl = b_cmd_tbl_orig;
							}

							return CLIR_INV_CMD_USAGE;
						} else {
							return CLIR_CMD_NOT_FOUND;
						}
					}

					pps_argv[0] = ++ps_dir_str; /* Skip '.' character */

					/* Go to next level */
					if (_is_cmd_tbl_array(pt_cmd_tbl) == TRUE) {
						return _parse_cmd(i4_argc, pps_argv, pt_cmd_tbl);
					} else {
						return _parse_cmd(i4_argc, pps_argv,
							pt_cmd_tbl->pt_next_level);
					}
				}
			} else {
				return CLIR_CMD_NOT_FOUND;
			}
			break;
		}
	}

	return CLIR_CMD_NOT_FOUND;
}


/*-----------------------------------------------------------------------------
 * Name: _generate_prompt
 *
 * Description: This API generates CLI prompt string according to current
 *              directory level.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static void _generate_prompt(void)
{
	u32      ui4_total_len = 0;
	u32      ui4_dir_str_len;
	s8       *ps_dir_str;
	u32      ui4_idx;

	/*set cli prompt
	//Get Root cli prompt. */
	ps_cli_prompt_str = cli_get_prompt_str_buf();

	ps_cli_prompt_str[0] = ASCII_NULL;
	switch (e_access_right) {
	default:
		x_strncat(ps_cli_prompt_str, CLI_PROMPT_STR, sizeof(CLI_PROMPT_STR));
		break;
	}

	/* Get sub path dir prompt. */
	for (ui4_idx = 1; ui4_idx <= ui4_dir_link_idx; ui4_idx++) {
		if (apt_dir_link[ui4_idx]->ps_cmd_abbr_str != NULL) {
			ps_dir_str = apt_dir_link[ui4_idx]->ps_cmd_abbr_str;
		} else {
			ps_dir_str = apt_dir_link[ui4_idx]->ps_cmd_str;
		}

		ui4_dir_str_len = x_strlen(ps_dir_str);

		if (ui4_total_len + ui4_dir_str_len > CLI_CMD_BUF_SIZE) {
			x_strncat(ps_cli_prompt_str, TEXT(">"), 1);
			return;
		}

		x_strncat(ps_cli_prompt_str, TEXT("."), 1);
		x_strncat(ps_cli_prompt_str, ps_dir_str, (ui4_dir_str_len + 1));
		ui4_total_len += (ui4_dir_str_len + 1); /* .xxx */
	}

	x_strncat(ps_cli_prompt_str, TEXT(">"), 1);
}


/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_change_dir
 *
 * Description: This API changes current command level.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_change_dir(s32 i4_argc, const s8 **pps_argv)
{
	const s8      *ps_dir_str;
	const s8      *ps_cur_dir;
	CLI_EXEC_T    *pt_tbl;
	u32           ui4_tbl_cnt;
	CLI_EXEC_T    **ppt_cmd_tbl_array;
	u32           ui4_idx;

	/* cd [directory path] */

	/* Check arguments */
	if ((i4_argc != 2) || (pps_argv == NULL) || (pps_argv[1] == NULL)) {
		_show_help(pt_cur_cmd_tbl);
		return CLIR_INV_CMD_USAGE;
	}

	ps_dir_str = pps_argv[1];
	while (*ps_dir_str != ASCII_NULL) {
		if (IS_DOT(*ps_dir_str)) {
			ps_dir_str++;
			if (IS_DOT(*ps_dir_str)) { /* cd .. */
				ps_dir_str++;
				if (ui4_dir_link_idx > 0) {
					ui4_dir_link_idx--;

					if (ui4_dir_link_idx == 0) {
						pt_cur_cmd_tbl = NULL;
						b_is_cmd_tbl = FALSE;
					} else {
						/* Check if currect directory points to a command
							table or command table array */

						pt_tbl = apt_dir_link[ui4_dir_link_idx];
						if (_is_cmd_tbl_array(pt_tbl) == TRUE) {
							pt_cur_cmd_tbl = pt_tbl;
							b_is_cmd_tbl = FALSE;
						} else {
							pt_cur_cmd_tbl = pt_tbl->pt_next_level;
							b_is_cmd_tbl = TRUE;
						}
					}
					_generate_prompt();
				}
			} else {
				return CLIR_DIR_NOT_FOUND;
			}
		} else {
			if (IS_ROOT(*ps_dir_str)) { /* cd / */
				ps_dir_str++;
				ui4_dir_link_idx = 0;
				pt_cur_cmd_tbl = NULL;
				b_is_cmd_tbl = FALSE;
				_generate_prompt();
			} else { /* cd xxx.xxx.xxx... */
				ps_cur_dir = ps_dir_str;
				if (b_is_cmd_tbl) { /* Command table */
					pt_tbl = _search_tbl(ps_cur_dir, pt_cur_cmd_tbl);
				} else { /* Command table array */
					pt_tbl = NULL;
					_get_cmd_tbl_array(pt_cur_cmd_tbl, TRUE, &ui4_tbl_cnt,
							&ppt_cmd_tbl_array);

					for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++) {
						pt_tbl = _search_tbl(ps_cur_dir,
								ppt_cmd_tbl_array[ui4_idx]);

						if (pt_tbl != NULL) {
							break;
						}
					}
				}
				if ((pt_tbl != NULL) &&	(pt_tbl->pt_next_level != NULL)) {
					switch (e_access_right) {
					default:
						if (pt_tbl->e_access_right >= e_access_right) {
							if (ui4_dir_link_idx <
									(CLI_MAX_CMD_TBL_LEVEL - 1)) {
								/* Print out current directory path */
								ui4_dir_link_idx++;
								apt_dir_link[ui4_dir_link_idx] = pt_tbl;
								_generate_prompt();

								/* Check if currect directory points to a
								command table or command table array */
								if (_is_cmd_tbl_array(pt_tbl) == TRUE) {
									pt_cur_cmd_tbl = pt_tbl;
									b_is_cmd_tbl = FALSE;
								} else {
									pt_cur_cmd_tbl = pt_tbl->pt_next_level;
									b_is_cmd_tbl = TRUE;
								}
							} else {
								/* There is something wrong => change
									directory back to root */
								ui4_dir_link_idx = 0;
								pt_cur_cmd_tbl = NULL;
								b_is_cmd_tbl = FALSE;
								_generate_prompt();
							}
						} else {
							return CLIR_DIR_NOT_FOUND;
						}
						break;
					}
				} else {
					return CLIR_DIR_NOT_FOUND;
				}

				/* Proceed next level directory path */
				while (!IS_DOT(*ps_dir_str) && (*ps_dir_str != ASCII_NULL)) {
					ps_dir_str++;
				}

				/* Skip '.' character */
				if (IS_DOT(*ps_dir_str)) {
					ps_dir_str++;
				}
			}
		}
	}
	_show_help(pt_cur_cmd_tbl);

	return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_list_cmd
 *
 * Description: This API list commands at current command level.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_list_cmd(s32 i4_argc, const s8 **pps_argv)
{
	/* ls */

	_show_help(pt_cur_cmd_tbl);

	return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_repeat
 *
 * Description: This API executes a CLI command for a specific loop.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_repeat(s32 i4_argc, const s8 **pps_argv)
{
	s32      i4_return;
	s8       ui1_data;
	u32      ui4_idx;
	u32      ui4_cmd_str_len;
	u32      ui4_loop = 0;
	u32      ui4_delay = 0;
	bool     b_loop_found = FALSE;
	bool     b_delay_found = FALSE;
	s8       s_cmd_buf[CLI_CMD_BUF_SIZE - 8];

	/* do [-d] [loop] [cmd] */

	/* Check arguments */
	if ((i4_argc < 3) || (pps_argv == NULL)) {
		pr_info(TEXT("do [-d] [loop] [cmd]\teg: do -d 1000 10 read 0x200 0x10\n\r"));
		pr_info(TEXT("  [-d]: Delay between two executions (ms)\n\r"));
		return CLIR_INV_CMD_USAGE;
	}

	/* Get the CLI message queue handle */

	/* Get the real command for loop run */
	ui4_cmd_str_len = 0;
	s_cmd_buf[0] = ASCII_NULL;
	for (ui4_idx = 1; ui4_idx < (u32) i4_argc; ui4_idx++) {
		/* Check if "-d" is specified */
		if (!b_delay_found &&
				(x_strlen(pps_argv[ui4_idx]) == 2) &&
				(x_strncmp(pps_argv[ui4_idx], TEXT("-d"), 2) == 0)) {
			ui4_delay = (u32)(simple_strtoull/*_tcstoul*/(pps_argv[ui4_idx + 1],
					NULL, 10));
			ui4_idx = ui4_idx + 1;
			b_delay_found = TRUE;
			continue;
		} else if (!b_loop_found) { /* Check loop count */
			ui4_loop = (u32)(simple_strtoull/*_tcstoul*/(pps_argv[ui4_idx], NULL, 10));
			b_loop_found = TRUE;
			continue;
		}

		/* Otherwise, it is treated as partial command string */
		ui4_cmd_str_len += x_strlen(pps_argv[ui4_idx]) + 1;

		if (ui4_cmd_str_len >= CLI_CMD_BUF_SIZE) {
			return CLIR_CMD_TOO_LONG;
		}

		x_strncat(s_cmd_buf, pps_argv[ui4_idx],
				(x_strlen(pps_argv[ui4_idx]) + 1));
		x_strncat(s_cmd_buf, TEXT(" "), 2);
	}

	/* Execute the command */
	for (ui4_idx = 0; ui4_idx < ui4_loop; ui4_idx++) {
		if (ui4_idx > 0) {
			pr_info(TEXT("\n\r"));
		}

		pr_info(TEXT("[Loop %d of %d]\n\r"), ui4_idx + 1, ui4_loop);

		i4_return = cli_parser(s_cmd_buf);
		if (i4_return != CLIR_OK) {
			return i4_return;
		}
		ui1_data = cli_get_char_timeout(1);

		if (ui1_data == ASCII_KEY_CTRL_B) {
			pr_info(TEXT("Interrupt!!!\n\r"));
			return CLIR_OK;
		}

		if (b_delay_found && (ui4_idx < ui4_loop - 1)) {
			mdelay(ui4_delay);
		}
	}
	return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_set_supervisor_access_right
 *
 * Description: This API changes system access right.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_set_supervisor_access_right(s32 i4_argc,
			const s8 **pps_argv)
{
	/* Check new input access right */
	if (e_access_right == CLI_SUPERVISOR) {
		pr_info(TEXT("Same access right!\n\r"));
	} else {
		e_access_right = CLI_SUPERVISOR;
		pr_info(TEXT("\n\rAccess right change successful!\n\r"));
	}

	return CLIR_OK;
}
/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_set_admin_access_right
 *
 * Description: This API changes system access right.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_set_admin_access_right(s32 i4_argc, const s8 **pps_argv)
{
	/* Check new input access right */
	if (e_access_right == CLI_ADMIN) {
		pr_info(TEXT("Same access right!\n\r"));
	} else {
		e_access_right = CLI_ADMIN;
		pr_info(TEXT("\n\rAccess right change successful!\n\r"));
	}

	return CLIR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_get_access_right
 *
 * Description: This API gets current system access right.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 ----------------------------------------------------------------------------*/
static s32 _cli_cmd_get_access_right(s32 i4_argc, const s8 **pps_argv)
{
	/* whoami */

	/* Check arguments */
	if ((i4_argc != 1) || (pps_argv == NULL)) {
		_show_help(pt_cur_cmd_tbl);
		return CLIR_OK;
	}

	pr_info(TEXT("%s\n\r"), ps_access_right[e_access_right]);
	return CLIR_OK;
}


#endif

/*-----------------------------------------------------------------------------
 * Name: cli_parser_list_mandatory_tbl_cmd
 *
 * Description: This API lists all commands from mandatory command table.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 cli_parser_list_mandatory_tbl_cmd(void)
{
#ifdef CLI_SUPPORT
	u32          ui4_cmd_idx = 0;
	s8            s_help[MAX_STR_LEN];

	pr_info(TEXT("\n\r[Help]\n\r"));

	while (at_mandatory_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL) {
		if (_format_help_str(s_help,
				MAX_STR_LEN,
				at_mandatory_cmd_tbl[ui4_cmd_idx].ps_cmd_str,
				at_mandatory_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
				at_mandatory_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE) {
			pr_info(TEXT("%s\n\r"), s_help);
		}

		ui4_cmd_idx++;
	}

	_generate_prompt();

	return CLIR_OK;
#else
	return CLIR_NOT_ENABLED;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: cli_parser_attach_cmd_tbl
 *
 * Description: This API attaches a command table to another command table.
 *
 * Inputs:  pt_tbl          The command table to be attached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 *          CLIR_GROUP_TBL_FULL     Module-to-group table is full.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 cli_parser_attach_cmd_tbl(CLI_EXEC_T *pt_tbl,
							CLI_CAT_T   e_category,
							u64      ui8_group_mask)
{
#ifdef CLI_SUPPORT
	u32          ui4_idx;
	u32          *pui4_tbl_cnt;
	CLI_EXEC_T   *pt_cmd_tbl = NULL;
	CLI_EXEC_T   **ppt_cmd_tbl_array;
	CLI_GRP_T    *pt_mod_grp_info;
	CLI_EXEC_T   *pt_tbl_tmp_a = NULL;
	CLI_EXEC_T   *pt_tbl_tmp_b = NULL;
	u64          ui8_group_mask_tmp_a = 0;
	u64          ui8_group_mask_tmp_b = 0;
	bool         b_replace_cmd_tbl = FALSE;
	bool         b_sort_cmd_tbl = FALSE;
	bool         b_swap = FALSE;

	/* Check arguments */
	if ((pt_tbl == NULL) ||
			(pt_tbl->ps_cmd_str == NULL) ||
			(e_category >= CLI_CAT_MAX)) {
		return CLIR_INV_ARG;
	}
	/* Attach the command table to requested category */
	switch (e_category) {
	case CLI_CAT_ROOT:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_ROOT];
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_ROOT][0]);
		break;

	case CLI_CAT_BASIC:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_BASIC];
		pt_cmd_tbl = at_basic_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_BASIC][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_BASIC][0]);
		break;

	case CLI_CAT_APP:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_APP];
		pt_cmd_tbl = at_app_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_APP][0]);
		break;

	case CLI_CAT_MW:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MW];
		pt_cmd_tbl = at_mw_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MW][0]);
		break;

	case CLI_CAT_MMW:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MMW];
		pt_cmd_tbl = at_mmw_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MMW][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MMW][0]);
		break;

	case CLI_CAT_DRV:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_DRV];
		pt_cmd_tbl = at_drv_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_DRV][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_DRV][0]);
		break;

	case CLI_CAT_TEST:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_TEST];
		pt_cmd_tbl = at_test_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_TEST][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_TEST][0]);
		break;

	case CLI_CAT_MTK_TOOL:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MTK_TOOL];
		pt_cmd_tbl = at_mtk_tool_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MTK_TOOL][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MTK_TOOL][0]);
		break;

	default:
		return CLIR_INV_ARG;
	}

	/* Check if buffer size is ok */
	if (*pui4_tbl_cnt >= CLI_MAX_CMD_TBL_NUM) {
		return CLIR_CMD_TBL_FULL;
	}

	/* Check if command table sorting is required */
	if (*pui4_tbl_cnt > 0 && e_category > CLI_CAT_ROOT) {
		b_sort_cmd_tbl = TRUE;
	}

	/* If the command table is duplicated, replace existed commad
		table with new one */
	for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++) {
		if ((x_strcmp(pt_tbl->ps_cmd_str,
				ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == 0) &&
				(x_strcmp(pt_tbl->ps_cmd_abbr_str,
				ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str) == 0)) {
			b_replace_cmd_tbl = TRUE;
			break;
		}
	}

	if (b_replace_cmd_tbl) {
		ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
		pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
		pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

		if (*pui4_tbl_cnt == 0 && e_category > CLI_CAT_ROOT) {
			pt_cmd_tbl[0].pt_next_level = pt_tbl;
		}
	} else {
		ui4_idx = *pui4_tbl_cnt;
		(*pui4_tbl_cnt)++;

		if (b_sort_cmd_tbl) {
			/* Start sortig */
			for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++) {
				if (!b_swap && ui4_idx < *pui4_tbl_cnt - 1) {
					if (x_strcmp(pt_tbl->ps_cmd_str,
							ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == -1) {
						b_swap = TRUE;
						pt_tbl_tmp_b = ppt_cmd_tbl_array[ui4_idx];
						ui8_group_mask_tmp_b = pt_mod_grp_info[ui4_idx].aui8_grp_msk;

						ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
						pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
						pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

						if (ui4_idx == 0 && e_category > CLI_CAT_ROOT) {
							pt_cmd_tbl[0].pt_next_level = pt_tbl;
						}
					}
				} else {
					if (b_swap) {
						/* Do command table swap */
						pt_tbl_tmp_a = ppt_cmd_tbl_array[ui4_idx];
						ui8_group_mask_tmp_a = pt_mod_grp_info[ui4_idx].aui8_grp_msk;
						ppt_cmd_tbl_array[ui4_idx] = pt_tbl_tmp_b;
						pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl_tmp_b;
						pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask_tmp_b;
						pt_tbl_tmp_b = pt_tbl_tmp_a;
						ui8_group_mask_tmp_b = ui8_group_mask_tmp_a;
					} else {
						ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
						pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
						pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;
					}
				}
			}
		} else {
			ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
			pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
			pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

			if (ui4_idx == 0 && e_category > CLI_CAT_ROOT) {
				pt_cmd_tbl[0].pt_next_level = pt_tbl;
			}
		}
	}

	return CLIR_OK;
#else
	return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: cli_parser_detach_cmd_tbl
 *
 * Description: This API detaches a command table.
 *
 * Inputs:  pt_tbl          The command table to be detached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_NULL       Command table is null.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 cli_parser_detach_cmd_tbl(CLI_EXEC_T *pt_tbl,
							CLI_CAT_T	e_category,
							u64		ui8_group_mask)
{
#ifdef CLI_SUPPORT
	u32         ui4_idx;
	u32         *pui4_tbl_cnt;
	CLI_EXEC_T  *pt_cmd_tbl = NULL;
	CLI_EXEC_T  **ppt_cmd_tbl_array;
	CLI_GRP_T   *pt_mod_grp_info;
	bool        b_replace_cmd_tbl = FALSE;

	/* Check arguments */
	if ((pt_tbl == NULL) ||
			(pt_tbl->ps_cmd_str == NULL) ||
			(e_category >= CLI_CAT_MAX)) {
		return CLIR_INV_ARG;
	}

	/* Attach the command table to requested category */
	switch (e_category) {
	case CLI_CAT_ROOT:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_ROOT];
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_ROOT][0]);
		break;

	case CLI_CAT_BASIC:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_BASIC];
		pt_cmd_tbl = at_basic_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_BASIC][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_BASIC][0]);
		break;

	case CLI_CAT_APP:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_APP];
		pt_cmd_tbl = at_app_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_APP][0]);
		break;

	case CLI_CAT_MW:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MW];
		pt_cmd_tbl = at_mw_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MW][0]);
		break;

	case CLI_CAT_MMW:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MMW];
		pt_cmd_tbl = at_mmw_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MMW][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MMW][0]);
		break;

	case CLI_CAT_DRV:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_DRV];
		pt_cmd_tbl = at_drv_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_DRV][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_DRV][0]);
		break;

	case CLI_CAT_TEST:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_TEST];
		pt_cmd_tbl = at_test_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_TEST][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_TEST][0]);
		break;

	case CLI_CAT_MTK_TOOL:
		pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MTK_TOOL];
		pt_cmd_tbl = at_mtk_tool_cmd_tbl;
		ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MTK_TOOL][0]);
		pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MTK_TOOL][0]);
		break;

	default:
		return CLIR_INV_ARG;
	}

	/* Check if buffer size is ok */
	if (0 == *pui4_tbl_cnt) {
		return CLIR_CMD_TBL_NULL;
	}

	/* If the command table is duplicated, replace existed
		commad table with new one */
	for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++) {
		bool fgCond1 = FALSE, fgCond2 = FALSE;

		if (NULL == pt_tbl->ps_cmd_str &&
				NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str)
			fgCond1 = TRUE;
		else if (NULL == pt_tbl->ps_cmd_str ||
				NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str)
			fgCond1 = FALSE;
		else if (x_strcmp(pt_tbl->ps_cmd_str,
				ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == 0)
			fgCond1 = TRUE;

		if (NULL == pt_tbl->ps_cmd_abbr_str &&
				NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str)
			fgCond2 = TRUE;
		else if (NULL == pt_tbl->ps_cmd_abbr_str ||
				NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str)
			fgCond2 = FALSE;
		else if (x_strcmp(pt_tbl->ps_cmd_abbr_str,
				ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str) == 0)
			fgCond2 = TRUE;

		if (TRUE == fgCond1 && TRUE == fgCond2) {
			if (pt_mod_grp_info[ui4_idx].pt_cmd_tbl == pt_tbl &&
					pt_mod_grp_info[ui4_idx].aui8_grp_msk == ui8_group_mask) {
				b_replace_cmd_tbl = TRUE;
				break;
			}
		}
	}

	if (b_replace_cmd_tbl) {
		for (; ui4_idx < *pui4_tbl_cnt; ui4_idx++) {
			if (CLI_MAX_CMD_TBL_NUM - 1 > ui4_idx) {
				ppt_cmd_tbl_array[ui4_idx] = ppt_cmd_tbl_array[ui4_idx + 1];
				pt_mod_grp_info[ui4_idx] = pt_mod_grp_info[ui4_idx + 1];
			} else if (CLI_MAX_CMD_TBL_NUM - 1 == ui4_idx) {
				ppt_cmd_tbl_array[ui4_idx] = NULL;
				pt_mod_grp_info[ui4_idx].pt_cmd_tbl = NULL;
				pt_mod_grp_info[ui4_idx].aui8_grp_msk = CLI_GRP_NONE;
			}
		}
		(*pui4_tbl_cnt)--;
	} else {
		return CLIR_CMD_NOT_FOUND;
	}

	return CLIR_OK;
#else
	return CLIR_NOT_ENABLED;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: cli_parser_clear_cmd_tbl
 *
 * Description: This API removes all command tables from CLI root then
 *              attaches default ones.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 *          CLIR_GROUP_TBL_FULL     Module-to-group table is full.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 cli_parser_clear_cmd_tbl(void)
{
#ifdef CLI_SUPPORT
	s32       i4_return;
	u32      ui4_i;
	u32      ui4_j;

	/* Remove all command tables attached to CLI root */
	for (ui4_i = CLI_CAT_ROOT; ui4_i < CLI_CAT_MAX; ui4_i++) {
		for (ui4_j = 0; ui4_j < CLI_MAX_CMD_TBL_NUM; ui4_j++) {
			apt_cli_array[ui4_i][ui4_j] = NULL;
			aui4_cli_tbl_cnt[ui4_i] = 0;
			at_cli_mod_grp_info[ui4_i][ui4_j].pt_cmd_tbl = NULL;
			at_cli_mod_grp_info[ui4_i][ui4_j].aui8_grp_msk = CLI_GRP_NONE;
		}
	}

	/* Attach default command tables to CLI root */
	at_basic_cmd_tbl[0].pt_next_level = NULL;
	at_app_cmd_tbl[0].pt_next_level = NULL;
	at_mw_cmd_tbl[0].pt_next_level = NULL;
	at_mmw_cmd_tbl[0].pt_next_level = NULL;
	at_drv_cmd_tbl[0].pt_next_level = NULL;
	at_test_cmd_tbl[0].pt_next_level = NULL;
	at_mtk_tool_cmd_tbl[0].pt_next_level = NULL;

	/* Mandatory command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mandatory_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}

	/* Basic command table */
	i4_return = cli_parser_attach_cmd_tbl(at_basic_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}
	/* Driver command table */
	i4_return = cli_parser_attach_cmd_tbl(at_drv_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}
	#if 1
	/* Application command table */
	i4_return = cli_parser_attach_cmd_tbl(at_app_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}

	/* Middleware command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mw_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}

	/* Multimedia Middleware command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mmw_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}




	/* MTK tool command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mtk_tool_cmd_tbl,
			CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK) {
		return i4_return;
	}
#endif
	/* Init internal variables */
	cli_parser_init();

	/* Set up prompt string */
	SetCliPrompt(e_access_right);

	return CLIR_OK;
#else
	return CLIR_NOT_ENABLED;
#endif
}
/*-----------------------------------------------------------------------------
 * Name: cli_parser_init
 *
 * Description: This API is used to init cli parser module
 *
 * Inputs:  NULL
 *
 * Outputs: NULL
 *
 * Returns:  NULL
 ----------------------------------------------------------------------------*/
void cli_parser_init(void)
{
	/* Init internal variables */
	ui4_dir_link_idx = 0;
	apt_dir_link[0] = NULL;
	pt_cur_cmd_tbl = NULL;
	b_is_cmd_tbl = FALSE;
}

/*-----------------------------------------------------------------------------
 * Name: cli_parser
 *
 * Description: This API parses a CLI command and performs corresponding
 *              operation.
 *
 * Inputs:  ps_cmd      Contain the command to parse.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_UNKNOWN_CMD        Unknown CLI command.
 *          CLIR_CMD_NOT_FOUND      CLI command not found.
 *          CLIR_DIR_NOT_FOUND      CLI directory not found.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
s32 cli_parser(const s8 *ps_cmd)
{
#ifdef CLI_SUPPORT
	s32           i4_return = CLIR_OK;
	s32           i4_argc;
#if (ALIAS_SUPPORT)
	u32          ui4_inp_cmd_len;
	u32          ui4_cmd_len;
	u32          ui4_cmd_abbr_len;
	u32          ui4_alias_idx;
	u32          ui4_alias_argc;
	s8           *ps_alias_argv[CLI_MAX_ARG_NUM];
	const s8     *ps_alias_str;
#endif
	u32          ui4_argc;
	u32          ui4_idx;
	s8           *ps_argv[CLI_MAX_ARG_NUM];



	/* Check arguments */
	if (ps_cmd == NULL) {
		return CLIR_INV_ARG;
	}

	/* Init argument buffer */
	for (ui4_idx = 0; ui4_idx < CLI_MAX_ARG_NUM; ui4_idx++) {
		ps_argv[ui4_idx] = as_argv[ui4_idx];
		as_argv[ui4_idx][0] = 0;
	}

	/* Get all the arguments of the command */
	i4_argc = _find_cmd_argv(ps_cmd,
	CLI_MAX_ARG_NUM,
	CLI_MAX_ARG_LEN,
	ps_argv);

	ui4_argc = (u32)i4_argc;
	if (ui4_argc > 0) {
		/* Alias handling: replace alias with corresponding string */
#if (ALIAS_SUPPORT)
		ui4_inp_cmd_len = x_strlen(ps_argv[0]);
		ui4_cmd_len = x_strlen(CLI_ALIAS_CMD_STR);
		ui4_cmd_abbr_len = x_strlen(CLI_ALIAS_CMD_ABBR_STR);

		if (!(((ui4_inp_cmd_len == ui4_cmd_len) &&
			(x_strncmp(ps_argv[0], CLI_ALIAS_CMD_STR, ui4_inp_cmd_len) == 0))
			|| ((ui4_inp_cmd_len == ui4_cmd_abbr_len) &&
			(x_strncmp(ps_argv[0], CLI_ALIAS_CMD_ABBR_STR,
			ui4_inp_cmd_len) == 0)))) {

			ui4_idx = 0;
			while (ui4_idx < ui4_argc) {
				/* Retrieve string corresponding to the alias */
				ps_alias_str = NULL;

				if (ps_alias_str) {
					/* Fill current alias arguments in to
						alias argument buffer */
					for (ui4_alias_idx = 0; ui4_alias_idx < CLI_MAX_ARG_NUM;
							ui4_alias_idx++) {
						ps_alias_argv[ui4_alias_idx] = as_alias_argv[ui4_alias_idx];
					}

					ui4_alias_argc = (u32)_find_cmd_argv(ps_alias_str,
						CLI_MAX_ARG_NUM,
						CLI_MAX_ARG_LEN,
						ps_alias_argv);

					/* Too many arguments after alias parsing */
					if (ui4_alias_argc > (CLI_MAX_ARG_NUM - (ui4_argc + 1))) {
						ui4_alias_argc = CLI_MAX_ARG_NUM - (ui4_argc + 1);
					}

					/* Copy string arguments after alias backward */
					for (ui4_alias_idx = (ui4_argc - 1);
							ui4_alias_idx > ui4_idx;
							ui4_alias_idx--) {
						x_strncpy(ps_argv[ui4_alias_idx + (ui4_alias_argc-1)],
								ps_argv[ui4_alias_idx],
								CLI_MAX_ARG_LEN);
					}

					/* Copy alias string arguemnts */
					for (ui4_alias_idx = 0; ui4_alias_idx < ui4_alias_argc;
							ui4_alias_idx++) {
						x_strncpy(ps_argv[ui4_idx + ui4_alias_idx],
						ps_alias_argv[ui4_alias_idx],
						CLI_MAX_ARG_LEN);
					}

					ui4_argc += (ui4_alias_argc - 1);
					ui4_idx--;
				}

				ui4_idx++;
			}
		}
#endif
		i4_argc = (s32)ui4_argc;
		/* Command parsing: mandatory table */
		i4_return = _parse_cmd(i4_argc,
				(const s8 **)ps_argv,
				apt_cli_array[CLI_CAT_ROOT][CLI_MANDA_CMD_TBL_IDX]);

		if (i4_return != CLIR_CMD_NOT_FOUND) {
			return i4_return;
		}
		/* Command parsing: other command tables */
		if (i4_argc > 0) {
			i4_return = _parse_cmd(i4_argc,
					(const s8 **)ps_argv,
					pt_cur_cmd_tbl);
			if (i4_return != CLIR_CMD_NOT_FOUND) {
				return i4_return;
			}
		}
	}

	if ((i4_argc > 0) && (i4_return == CLIR_CMD_NOT_FOUND)) {
		return i4_return;
	}
	/* Display CLI help */
	_show_help(pt_cur_cmd_tbl);
	return CLIR_OK;

#else
	return CLIR_NOT_ENABLED;
#endif
}

void SetCliPrompt(s32 accessRight)
{
	/* Set up prompt string */
	ps_cli_prompt_str = cli_get_prompt_str_buf();
	ps_cli_prompt_str[0] = ASCII_NULL;
	switch (accessRight) {
	default:
		x_strncat(ps_cli_prompt_str, CLI_PROMPT_STR, sizeof(CLI_PROMPT_STR));
		break;
	}
	x_strncat(ps_cli_prompt_str, TEXT(">"), 1);
}

void SetAccessRight(s32 accessRight)
{
	e_access_right = (CLI_ACCESS_RIGHT_T)accessRight;
}

void x_cli_set_mode(s32 accessRight)
{
	SetAccessRight(accessRight);
}



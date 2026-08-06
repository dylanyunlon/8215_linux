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

 * $RCSfile: u_cli.h,v $ u_cli_h,v $
 * $Revision: #4 $
 * $Date: 2015/10/24 $
 * $Author: wangjing.wang $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/11 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: d3f4bd3088d0839e70c155f2e911dd7a $
 *
 * Description:
 *         This header file contains CLI related definitions, which are
 *         known to applications and middleware.
 *---------------------------------------------------------------------------*/

#ifndef _U_CLI_H_
#define _U_CLI_H_

/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include "u_common.h"

#include <linux/types.h>

/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* CLI API return values */
#define CLIR_INV_CMD_USAGE          ((s32)    1)
#define CLIR_OK                     ((s32)    0)
#define CLIR_NOT_INIT               ((s32)   -1)
#define CLIR_ALREADY_INIT           ((s32)   -2)
#define CLIR_NOT_ENABLED            ((s32)   -3)
#define CLIR_INV_ARG                ((s32)   -4)
#define CLIR_INV_CMD_TBL            ((s32)   -5)
#define CLIR_CMD_TOO_LONG           ((s32)   -6)
#define CLIR_ALIAS_TOO_LONG         ((s32)   -7)
#define CLIR_CMD_TBL_FULL           ((s32)   -8)
#define CLIR_ALIAS_TBL_FULL         ((s32)   -9)
#define CLIR_CMD_NOT_FOUND          ((s32)  -10)
#define CLIR_DIR_NOT_FOUND          ((s32)  -11)
#define CLIR_CMD_EXEC_ERROR         ((s32)  -12)
#define CLIR_UNKNOWN_CMD            ((s32)  -13)
#define CLIR_CMD_TBL_NULL           ((s32)  -14)

/* ASCII key definiton */
#define CLI_ASCII_KEY_CTRL_B        ((s8) 0x02)

/* Definition of get & set debug level command and help strings */
#define CLI_GET_DBG_LVL_STR         "gdl"
#define CLI_SET_DBG_LVL_STR         "sdl"
#define CLI_GET_DBG_LVL_HELP_STR    "Get debug level (e=error, a=api, i=info, n=none)"
#define CLI_SET_DBG_LVL_HELP_STR    "Set debug level (e=error, a=api, i=info, n=none)"

/* Definition of group debug level control */
#define CLI_GRP_NONE                ((u64)   0x00000000)
#define CLI_GRP_PIPE                ((u64)   0x00000001)
#define CLI_GRP_GUI                 ((u64)   0x00000002)
#define CLI_GRP_EPG                 ((u64)   0x00000004)
#define CLI_GRP_DRV                 ((u64)   0x00000008)
#define CLI_GRP_MAX                 ((u64)   0x00000010)

/* Definition of get & set time measurement level command and help strings */
#define CLI_GET_TMS_LVL_STR         "gtl"
#define CLI_SET_TMS_LVL_STR         "stl"
#define CLI_GET_TMS_LVL_HELP_STR    "Get TMS level (r=real-time, o=off-line, n=none)"
#define CLI_SET_TMS_LVL_HELP_STR    "Set TMS level (r=real-time, o=off-line, n=none)"

/* Definition of command table terminator */
#define END_OF_CLI_CMD_TBL          {NULL, NULL, NULL, NULL, NULL, CLI_HIDDEN}

#define CLI_SUPPORT
#define ALIAS_SUPPORT 0


/* CLI command access right */
/* New entries must be added to the end of the enumeration before CLI_HIDDEN */
typedef enum {
	CLI_SUPERVISOR = 0, /* Commands used by MTK only */
	CLI_ADMIN,          /* Commands used by customer only */
	CLI_GUEST,          /* Commands used by end user only */
	CLI_HIDDEN          /* Hide commands from help list */
}   CLI_ACCESS_RIGHT_T;

#define CLI_MODE_SUPERVISOR         CLI_SUPERVISOR
#define CLI_MODE_ADMIN              CLI_ADMIN
#define CLI_MODE_GUEST              CLI_GUEST





/* CLI category definition */
typedef enum {
	CLI_CAT_ROOT = 0,
	CLI_CAT_BASIC,
	CLI_CAT_APP,
	CLI_CAT_MW,
	CLI_CAT_MMW,    /* For BD multimedia MW use only */
	CLI_CAT_DRV,    /* For driver module use only */
	CLI_CAT_TEST,   /* For test use only */
	CLI_CAT_MTK_TOOL,
	CLI_CAT_MAX
} CLI_CAT_T;

/* CLI password calculation definition */
typedef enum {
	CLI_PASSWD_ODD = 0,
	CLI_PASSWD_EVEN,
	CLI_PASSWD_TWO_DIGITS_SUM_DEC,
	CLI_PASSWD_TWO_DIGITS_SUM_HEX,
	CLI_PASSWD_4EVEN_3ODD,
	CLI_PASSWD_4ODD_3EVEN,
	CLI_PASSWD_REVERSE_7EVEN,
	CLI_PASSWD_MAX_TYPE_NUM
} CLI_PASSWD_T;


/* CLI execution function */
typedef s32 (*x_cli_exec_fct)(s32 i4_argc, const s8 **pps_argv);

/* CLI command table structure */
typedef struct _CLI_EXEC_T {
	s8                  *ps_cmd_str;			/* Command string */
	s8                  *ps_cmd_abbr_str;	/* Command abbreviation string */
	x_cli_exec_fct      pf_exec_fct;        /* Execution function */
	struct _CLI_EXEC_T  *pt_next_level;      /* Next level command table */
	s8                  *ps_cmd_help_str;	/* Command help string */
	CLI_ACCESS_RIGHT_T  e_access_right;		/* Command access right */
} CLI_EXEC_T;


#endif /* _U_CLI_H_ */


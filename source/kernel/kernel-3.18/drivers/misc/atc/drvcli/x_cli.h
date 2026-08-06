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
 * $RCSfile: x_cli.h,v $ x_cli_h,v $
 * $Revision: #4 $
 * $Date: 2015/10/24 $
 * $Author: wangjing.wang $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/4 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: f561d156232286bf00fd87f76b0f5314 $
 *
 * Description:
 *         This header file contains CLI related definitions, which are
 *         exported.
 *---------------------------------------------------------------------------*/

#ifndef _X_CLI_H_
#define _X_CLI_H_

/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include <windows.h>
#include <types.h>

#include <linux/types.h>


#include "u_cli.h"
#include "drv_cli.h"

/*-----------------------------------------------------------------------------
					functions declarations
 ----------------------------------------------------------------------------*/
extern s32 cli_init(void);
extern s32 x_cli_attach_cmd_tbl(CLI_EXEC_T *pt_tbl,
								CLI_CAT_T  e_category,
								u64        ui8_group_mask);

extern s32 x_cli_detach_cmd_tbl(CLI_EXEC_T  *pt_tbl,
								CLI_CAT_T   e_category,
								u64         ui8_group_mask);


extern s32 x_cli_attach_alias(const s8 *ps_alias, const s8 *ps_cmd);

extern s32 x_cli_parser(const s8 *ps_cmd);

extern s8 x_cli_get_char_timeout(u32 ui4_time);

extern s32 x_cli_show_dbg_level(u16 ui2_dbg_level);

extern s32 x_cli_parse_tms_level(s32 i4_argc, const s8 **pps_argv,
		u16 *pui2_tms_level);

extern void x_cli_set_mode(s32 accessRight);
extern void *x_cli_get_current_root(void);

#endif /* _X_CLI_H_ */


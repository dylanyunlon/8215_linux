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
 * $RCSfile: cli.h,v $ cli_h,v $
 * $Revision: #4 $
 * $Date: 2015/10/24 $
 * $Author: wangjing.wang $
 * $CCRevision: /main/DTV_X_HQ_int/2 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: 169e2c3278568041785782cdcc283ebe $
 *
 * Description:
 *         This header file contains CLI related definitions, which are
 *         known to the whole Middleware.
 *---------------------------------------------------------------------------*/

#ifndef _CLI_H_
#define _CLI_H_

/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include <windows.h>
#include "x_common.h"
#include "x_cli.h"

#include <linux/types.h>

/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
					functions declarations
 ----------------------------------------------------------------------------*/
extern s32 cli_init(void);

extern bool cli_is_inited(void);
extern s32 cli_uninit(void);

extern s8 cli_get_char(void);

extern s8 cli_get_char_timeout(u32 ui4_time);

/* extern INT32 cli_receive_char(CHAR cCmd);  */

#define UART_TP_MODE    0
#define UART_NORMAL_MODE 1
void SwitchUartMode(u32 u4UartMode);


#endif /* _CLI_H_ */


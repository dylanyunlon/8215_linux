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
 * $RCSfile: cli_common.h,v $ cli_common,v $
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

#ifndef _X_CLI_COMMON_H_
#define _X_CLI_COMMON_H_

/*-----------------------------------------------------------------------------
					include files
 ----------------------------------------------------------------------------*/
#include <windows.h>
#include <types.h>

#include <linux/types.h>


#include "u_cli.h"
#include "drv_cli.h"

#ifdef __cplusplus
extern "C"
{
#endif

void cli_enable_kernel_log(bool fgEnable);
u32 UartRead32(u32 *pu4Value, u32 u4TimeOut);
u32 UartWriteBytes(u8 *pbValue,  u32 u4Size);
u32 UartReadBytes(u8 *pbValue,  u32 u4Size, u32 u4TimeOut);
void   UARTWriteData32(u32 u4Value);
extern HANDLE _hDmnrSema;
extern CRITICAL_SECTION _rCliCs;
extern s8 _szCliCmdBuffer[CLI_CMD_BUF_SIZE];
extern u32 _u4CmdReadPtr;
extern bool   b_cli_init;

extern u32 DMNR_Record(u32 u4Size);
extern u32 DMNR_Stop_Record();

extern bool g_fgDmnrPlay;
extern bool g_fgDmnrRec;
extern u32 DMNR_Init_PlayFile(u32 u4SampleRate, u32 u4Channels, u32 u4Size);
extern u32 DMNR_Receive_PlayData();
extern u32 DMNR_StartPlay();
extern u32 DMNR_StopPlay();

#ifdef __cplusplus
}
#endif

#endif /* _X_CLI_COMMON_H_ */


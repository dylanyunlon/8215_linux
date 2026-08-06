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
#ifndef _AUD_CMD_H_
#define _AUD_CMD_H_

// This file for export function for audio command call by cli driver.
extern void AudSetCliCmd(AUD_DEC_CLI_TYPE eAudCli,u32 arg1,u32 arg2,
                         u32 arg3,u32 arg4,const s8 **pfilename);


#endif

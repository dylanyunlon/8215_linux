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




#ifndef _ASV_TRIGGER_H_
#define _ASV_TRIGGER_H_

#include  "x_typedef.h"
#include <linux/types.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define AUD_VERIFY(x)		do { \
    if (!(x))	\
	    LOG(LOG_CTRLF, TEXT("[AUD]DBGCHK Failed: %s, %d!\r\n"), TEXT(__FILE__), (s32)__LINE__); \
    } while (0)


extern u8 u1AsvDspAAoutOn(void);
extern u8 u1AsvDspAConnect(u8 u1DecId);
extern void vAsvDspAAoutResume(void);

s32 i4AsvSendPlayCmd(s32 eDecId);
s32 i4AsvSendStopCmd(s32 eDecId);
s32 i4AsvSendDisconnectCmd(s32 eDecId);

s32 i4AsvDspNotifyPlayCmdGot(s32 eDecId);
s32 i4AsvDspNotifyDecReady(s32 eDecId);
s32 i4AsvDspNotifyDecStopped(s32 eDecId);

bool fgDspAWakeup(void);
bool fgDspBWakeup(void);
void vPowerDownDsp(void);


#ifdef __cplusplus
}
#endif


#endif

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




#ifndef _ASV_NOTIFY_H_
#define _ASV_NOTIFY_H_

#include  "x_typedef.h"
#include <linux/types.h>

#ifdef __cplusplus
extern "C"
{
#endif
extern bool fgAsvQueryAVD(void * rAudSrcCfg,void* rAudOutputCfg,
    void * rAudHdmiOutputCfg);
extern void vAsvSetAout1Periph(void);
extern void vAsvSetAout2Periph(void);
extern void vAsvNotifyDecReady(u8 u1DecId);
extern void vAsvNotifyPlayCmdGot(u8 u1DecId);
//extern void vAsvNotifyResumeCmdGot(void);
extern void vAsvNotifyStopDone(u8 u1DecId);
extern void vAsvNotifyPauseDone(u8 u1DecId);
extern void vAsvNotifyResumeDone(u8 u1DecId);
extern void vAsvNotifyReencStartDone(u8 u1DecId);
extern void vAsvNotifyReencStopDone(u8 u1DecId);
//extern void vAsvNotifyEncoderStartDone(u8 u1DecId);        // -- Water (AUD_RIPPING)
//extern void vAsvNotifyEncoderStopDone(u8 u1DecId);
extern bool fgAsvNotifyAVDChStatus(u8 * prLChStatus,
    u8 * prRChStatus);

#ifdef __cplusplus
}
#endif


#endif
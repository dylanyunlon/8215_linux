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




#ifndef _AUDMHL_TASK_H_
#define _AUDMHL_TASK_H_

#if CONFIG_DRV_HDMI_RX
#include <media/atc/dmx_splitter.h>

/******************************************************************************
* AUDIN CMD Queue 
******************************************************************************/
#define AUDINTASK_SPDIFIN_CMD_Q_NAME    _T("AUDInSPDIFCmd")


// *********************************************************************
// Audmhl task API
// *********************************************************************
void AudmhlNotifySPDDataType(AUD_DRV_FMT_T uDataType);
void AudmhlSendAudMsg(u32 u4Cmd, u8 bPri);
typedef __u32 (*pu4SendSlot)(void *pvDmxTag, SEND_BUFFER *prSendBuffer);

extern void AudmhlMsgToMW(u32 u4Msg, u8  u1MsgArg);
extern bool AudmhlIsMHLIn(void);
extern bool AudmhlMLinTypeDecided(void);
extern struct atc_hdmiaudio_isr_data isr_data;
extern bool audio_register_isr;


#endif

#endif //_AUDIN_TASK_H_

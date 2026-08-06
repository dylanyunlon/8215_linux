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


#ifndef AUDIO_SPEECH_DUAL_H
#define AUDIO_SPEECH_DUAL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "drv_dual.h"

u32 SHARE_MEM(u32 u4VA, u32 u4PA);
bool TAKE_BT_HW_SEMAPHORE(void);
bool RELEASE_BT_HW_SEMAPHORE(void);
bool SpeechSendMessage(u32 u4Msg, u32 u4P1, u32 u4P2, u32 u4P3);
bool AECSendMessage(u32 u4Msg, u32 u4P1, u32 u4P2, u32 u4P3);

typedef u32 (*PFN_Callback)(u32 u4MsgID, u32 u4Param1, u32 u4Param2, u32 u4Param3);

bool SpeechRegCallback(PFN_Callback pfnCallback);

#if defined(__cplusplus)
}
#endif

#endif

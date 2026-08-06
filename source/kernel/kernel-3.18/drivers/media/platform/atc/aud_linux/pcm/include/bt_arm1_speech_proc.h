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



#ifndef BT_ARM1_SPEECH_PROC_H
#define BT_ARM1_SPEECH_PROC_H

u32 SpeechInit(void);
u32 SpeechStateMachine(void);
u32 SpeechCB(u32 u4MsgID, u32 u4P1, u32 u4P2, u32 u4P3);

#endif

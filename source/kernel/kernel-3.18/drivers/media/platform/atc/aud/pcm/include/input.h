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


#ifndef INPUT_H
#define INPUT_H


extern s32 CopyMicDataThread_wq_flag;
extern wait_queue_head_t  CopyMicDataThread_wq;

s32 CopyMicDataThread(void *data);
s32 CopyRfDataThread(void *data);
void Capture_NdcEnable(bool fgEnable);


#endif



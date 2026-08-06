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

#ifndef HDMI_DMX_H_
#define HDMI_DMX_H_
#include "drv_esm_if.h"

enum {
	ST_DMX_UNKNOWN,
	ST_DMX_INIT,
	ST_DMX_START,
	ST_DMX_STOP,
	ST_DMX_UNINIT,
};
int DMXInit(void);
int DMXUninit(void);
int DMXStart(void);
int DMXStop(void);
void *GetAudioOutputBuf(int *pu4BufSz, int u4TimeWait);
BOOL ReleaseAudioOutputBuf(void);

#endif /*HDMI_DMX_H_*/

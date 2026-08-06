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



#ifndef _AUDIO_HAL_H_
#define _AUDIO_HAL_H_


#include "windev.h"


typedef enum {
	DEST_TYPE_INVALID,
	DEST_TYPE_FRONT,
	DEST_TYPE_REAR,
	DEST_TYPE_FRONT_REAR,
} E_DEST_TYPE_T;


extern bool ADE_IOControl(uintptr_t context, u32 code, u8 *pInBuffer, u32 inSize, u8 *pOutBuffer, u32 outSize, u32 *pOutSize);
extern void lineinInit(void);
extern bool lineinAudStart(int portNum);
extern bool lineinAudStop(int portNum);

extern bool start_digitalAud(E_DEST_TYPE_T destination);
extern bool stop_digitalAud(E_DEST_TYPE_T destination);


#endif


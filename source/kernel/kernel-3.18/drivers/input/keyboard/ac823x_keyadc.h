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

#ifndef _AC823X_KEYADC_H
#define _AC823X_KEYADC_H

#include <linux/types.h>
#include <ac823x_auxadc.h>

#define CONFIG_AC823X_KEYADC
#define CONFIG_AC823X_POWER_KEY
//#define CONFIG_IR_AC823X
//#define CONFIG_KNOB_AC823X
//#define CONFIG_VIRTUAL_KEY_AC823X

extern ulong IO_UCV_BASE_FOR_KP;//cgx

#define ADCKEY_NUM     16

#define  MAX_KEYS                   16

u32 AuxGetKeyPadDat(char channel);
bool AuxADCInitKeypad(void);
bool AuxADCDeInitKeypad(void);
u32 GET_KEY(unsigned int sample_value);

u32 ac823x_knob_init(void);
void Keypad_Enable(bool fgEnable);

#endif


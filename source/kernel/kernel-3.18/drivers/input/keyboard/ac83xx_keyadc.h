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

#ifndef _AC83XX_KEYADC_H
#define _AC83XX_KEYADC_H

#include <linux/types.h>
#include <ac83xx_auxadc.h>

#define CONFIG_AC83XX_KEYADC
#define CONFIG_PWRK_ATOMIC_PROTECT
#define CONFIG_AC83XX_POWER_KEY
//#define CONFIG_IR_AC83XX
//#define CONFIG_KNOB_AC83XX
//#define CONFIG_VIRTUAL_KEY_AC83XX

#define ADCKEY_NUM     16

#define  MAX_KEYS                   16

u32 AuxGetKeyPadDat(char channel);
bool AuxADCInitKeypad(void);
bool AuxADCDeInitKeypad(void);
u32 GET_KEY(unsigned int sample_value);

u32 ac83xx_knob_init(void);
void Keypad_Enable(bool fgEnable);

#endif


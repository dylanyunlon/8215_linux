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

#ifndef HARDWARE_H
#define HARDWARE_H

s32 HardWare_Init(ac_83xx *chip);
s32 HardWare_UnInit(ac_83xx *chip);

u32 ReleaseStreamProcess(struct snd_pcm_substream *substream);
s32 GetStreamProcess(struct snd_pcm_substream *substream);

#endif


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

 /******************************************************************************
*[File]                DspAsvInc.h
*[Author]
*[Description]
* This files contains the syntax to exporte interface from dspctrl to Asv
******************************************************************************/


#ifndef _DSP_ASV_INC_H_
#define _DSP_ASV_INC_H_

#include "DspUop.h"
#include "aud_drv_config.h"

extern void vDspCmd (u32 u4Cmd);
extern void vDspUpdateMixVolumn(bool flag);

extern u8 u1AsvDspAAoutOff(void);
extern u8 u1AsvDspAAout2Off(void);
#endif //DSP_ASV_INC_H


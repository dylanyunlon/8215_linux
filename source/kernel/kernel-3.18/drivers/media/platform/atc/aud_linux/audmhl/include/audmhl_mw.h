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

#ifndef  _AUDMHL_MW_H_
#define  _AUDMHL_MW_H_

#include "audmhl_var.h"

#if CONFIG_DRV_HDMI_RX


//extern s32 MW_Audin_Init(void);
//extern s32 MW_Audin_Unint(void);
//extern Drv_Comp_T* MW_AudinBUF_GetCompInfo(void);/*-----------------------------------------------------------------------------
                    Macros, defines, typedefs, enums
----------------------------------------------------------------------------*//* Get operations */void AudmhlSwitchFunc(AUDIO_IN_TYPE_T u1Input, AUDIN_DIGITAL_DETECT u1Detect);

#endif




#endif
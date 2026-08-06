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




#ifndef _AUD_PWMDAC_H_
#define _AUD_PWMDAC_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "drv_def.h"

#include "aud_config.h"
#include "aud_clock.h"

/****************************************************************************
** Type definitions
****************************************************************************/

/****************************************************************************
** Constant definitions
****************************************************************************/

/****************************************************************************
** Export API
****************************************************************************/

#define AUDVFY_PWM_YRAM_IDX                 0

#define AU4_YRAM_SIZE                       75


s32 Aud_PWMDAC_Init(AUD_DAC_CLASS_T eDacCls);
void Aud_Pwm_Apll_Select(AUD_DAC_CLASS_T eDacCls, APLL_DOMAIN eApllDomain);
s32 Aud_PWMDAC_LRChannelSelect(AUD_DAC_CLASS_T eDacCls);
s32 Aud_PWMDAC_SDATASelect(AUD_DAC_CLASS_T eDacCls, AUD_OUT_TYPE_T eSource);
//void Aud_PWMDAC_Setting(AUD_DAC_CLASS_T ePwmType);
extern s32 Aud_PwmDacSourceSwitch(AUD_DAC_CLASS_T ePwmDacType,
                                    AUD_OUT_TYPE_T eSource);

extern s32 Aud_PWMDAC_MultiFuncSel(AUD_DAC_CLASS_T eDacType, bool fgSel);
extern void Aud_PWMDAC_Setting(AUD_DAC_CLASS_T ePwmType);
extern void Aud_Pwm_Output_Zero(AUD_DAC_CLASS_T ePwmDac, bool fgEnable);
extern void Aud_Pwm_Apll_Select(AUD_DAC_CLASS_T eDacCls, APLL_DOMAIN eApllDomain);

#endif // #ifndef _AUD_PWMDAC_H_

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



/************************************************************************************************/
                                 /*Headfile include*/
/************************************************************************************************/

#ifndef _AUD_POWER_H_
#define _AUD_POWER_H_

#include  "aud_3360_reg_rw.h"
#include <media/atc/drv_aud.h>
#include <mach/base_regs.h>
#include  "aud_drv.h"  
#include  "x_os.h"
#include "x_aud_dec.h"
#include "aud_drv_config.h"


#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
/************************************************************************************************/
                                 /*variable & function declartion*/
/************************************************************************************************/
extern bool _fgDspAWakeUpFlag;
extern bool _fgDspBWakeUpFlag;

extern u8 _u1DspAoutState;
extern u8 _u1DspAout2State;

extern void GpsMix_SetAoutCfg(void);
extern void AudDrvThreadInit(void);


#define DEVICE_POWERON                1
#define DEVICE_POWERDOWN              0

typedef struct
{
  u32 PRIMARY_DEVICEID       :1;      //bit0
  u32 FOUR_DEVICEID          :1;      //bit1
  u32 GPSMIX_DEVICEID        :1;      //bit2
  u32 DVD_DEVICEID           :1;      //bit3
  u32 DSP_STATE              :1;      //bit4
  u32 RESERVED0              :27;
} AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL;


typedef union _AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL_UNION_T
{
  AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL DeviceIdPowerControlBit;
  
  u32 dPowerControlWord;
  
} AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL_UNION_T;

#ifdef __cplusplus
extern "C"
{
#endif

void Aud_DrvInitPower(void);
void AudDev_PowerDown(DEVICE_ID_PM eDId);
void AudDev_PowerOn(DEVICE_ID_PM eDId);


#ifdef __cplusplus
}
#endif

#endif // #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT

#endif

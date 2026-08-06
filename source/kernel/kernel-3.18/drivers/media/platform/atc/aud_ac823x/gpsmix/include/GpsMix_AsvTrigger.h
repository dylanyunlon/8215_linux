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

//---------------------------------------------------------------------------
//GPS_MIX state machine
//---------------------------------------------------------------------------
#ifndef _GPS_MIX_ASV_TRIGGER_H_
#define _GPS_MIX_ASV_TRIGGER_H_


//#include "x_typedef.h"
#include <linux/types.h>
#include "DspFunc.h"
#include "DspUop.h"

    //DSP state
#define ST_GPS_MIX_DSP_READY                0x00
#define ST_GPS_MIX_DSP_STARTED              0x01
#define ST_GPS_MIX_DSP_STOPPED              0x02
#define ST_GPS_MIX_DSP_PAUSED               0x03

//AUD-DSPA state trigger
#define TR_GPS_MIX_DSP_R_START              0x00
#define TR_GPS_MIX_DSP_R_STOP               0x01
#define TR_GPS_MIX_DSP_R_PAUSE              0x02
#define TR_GPS_MIX_DSP_R_RESUME             0x03

    //AUD-DSP state machine result
#define RTN_GPS_MIX_DSP_FAIL                0x00
#define RTN_GPS_MIX_DSP_SUCCESS             0x01



void vAudGpsMixStateInit(void);
u8 u1AudGpsMixDspState(u8 u1Trigger);
u8 u1AudGpsMixDspGetState(void);
void u1AsvGpsMixDspAStartCmd(void);
void u1AsvGpsMixDspAStopCmd(void);
void u1AsvGpsMixDspAPauseCmd(void);
void u1AsvGpsMixDspAResumeCmd(void);
u8 u1AsvGpsMixStartCmd(void);
u8 u1AsvGpsMixStopCmd(void);
u8 u1AsvGpsMixPauseCmd(void);
u8 u1AsvGpsMixResumeCmd(void);
void i4AsvGpsMixDspNotifyPlayCmdDone(void);
void i4AsvGpsMixDspNotifyStopCmdDone(void);
void i4AsvGpsMixDspNotifyPauseCmdDone(void);
void i4AsvGpsMixDspNotifyResumeCmdDone(void);
u32 i4AsvGpsMixDspNotifyConsumedData(void);

#endif






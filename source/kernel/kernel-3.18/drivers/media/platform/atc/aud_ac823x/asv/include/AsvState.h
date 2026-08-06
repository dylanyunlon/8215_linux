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

#ifndef _ASV_STATE_H_
#define _ASV_STATE_H_

#include  "x_typedef.h"
#include <linux/types.h>
//#include  "aud_drv_config.h"
//#include "aud_debug.h"
#ifdef __cplusplus
extern "C"
{
#endif
//---------------------------------------------------------------------------
// state machine
//---------------------------------------------------------------------------
extern u8 u1AudDspState(u8 u1Trigger);
extern u8 u1DspAState(u8 u1Trigger);
extern u8 u1DspAoutState(u8 u1Trigger);
extern u8 u1DspAout2State(u8 u1Trigger);
extern u8 u1DspBDec1State(u8 u1Trigger);
extern u8 u1DspBDec2State(u8 u1Trigger);
extern u8 u1DspBDec3State(u8 u1Trigger);
extern u8 u1DspReencState(u8 u1Trigger);
extern u8 u1DspEncoderState(u8 u1Trigger);        // -- Water (AUD_RIPPING)

//---------------------------------------------------------------------------
// API in ASV
//---------------------------------------------------------------------------
extern u8 u1AsvDspStopped(u8 u1DecId);
extern u8 u1AsvDspPlayed(u8 ucDecId);
extern u8 u1AsvDspPaused(u8 ucDecId);
extern u8 u1AsvDspResumed(u8 u1DecId);
extern u8 u1AsvDspBPlayCmd(u8 u1DecId);
extern u8 u1AsvDspBStopCmd(u8 u1DecId);

extern u8 u1AsvDspAAoutOff(void);
extern u8 u1AsvDspAAout2Off(void);
extern u8 u1AsvDspAConnect(u8 u1DecId);
extern u8 u1AsvDspADisconnect(u8 u1DecId);

//---------------------------------------------------------------------------
// get state machine status
//---------------------------------------------------------------------------
extern u8 u1AudDspGetState(void);
extern u8 u1DspAGetState(void);
extern u8 u1DspAoutGetState(void);
extern u8 u1DspAout2GetState(void);
extern u8 u1DspBDec1GetState(void);

#ifdef __cplusplus
}
#endif


#endif

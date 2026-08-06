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



// *********************************************************************
// Include
// *********************************************************************
#include "aud_oal.h"
//#include "x_typedef.h"
#include <linux/types.h>
#include "AsvDef.h"
#include <media/atc/drv_aud.h>
#include "aud_drv_config.h"

// *********************************************************************
// Groble Variables
// *********************************************************************
u8 _u1AudDspState;
u8 _u1DspAState;
u8 _u1DspAoutState;
u8 _u1DspAout2State;

u8 _u1DspBDec1State;
u8 _u1DspBDec2State;
u8 _u1DspBDec3State;

u8 _u1DspReencState;

static DEFINE_SPINLOCK(dsp_state_lock);
static DEFINE_SPINLOCK(dsp_aout1_lock);
static DEFINE_SPINLOCK(dsp_aout2_lock);
static DEFINE_SPINLOCK(dsp_reenc_lock);
static DEFINE_SPINLOCK(dspa_lock);
static DEFINE_SPINLOCK(dspb_dec1_lock);
static DEFINE_SPINLOCK(dspb_dec2_lock);
static DEFINE_SPINLOCK(dspb_dec3_lock);


#define DSPB_STATE_DBG_LEN      64

u16 _u2DspBDbgCnt = 0;
u16 _u2DspBDbgState[DSPB_STATE_DBG_LEN];

/***************************************************************************
Function : u1AudDspState
Description : AUD-DSP state machine
Parameter : u1Trigger: trigger for state change
Return    : as defined
***************************************************************************/
u8 u1AudDspState(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dsp_state_lock, flags);
    //Check original status
    switch (_u1AudDspState)
    {
    //DSP power off
    case ST_DSP_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP power on
        case TR_DSP_POWER_ON:
            //Change to init state
            _u1AudDspState = ST_DSP_INIT;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_POWER_OFF changed to ST_DSP_INIT.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP initialization
    case ST_DSP_INIT:
        switch (u1Trigger)
        {
        //DSP power off
        case TR_DSP_POWER_OFF:
            //Change to DSP off
            _u1AudDspState = ST_DSP_POWER_OFF;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_INIT changed to ST_DSP_POWER_OFF.\n"));
            break;

        //DSP A and B ready
        case TR_DSP_INIT_READY:
            //Change to DSP ready
            _u1AudDspState = ST_DSP_READY;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_INIT changed to ST_DSP_READY.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP ready
    case ST_DSP_READY:
        switch (u1Trigger)
        {
        //DSP power off
        case TR_DSP_POWER_OFF:
            //Change to DSP off state
            _u1AudDspState = ST_DSP_POWER_OFF;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_READY changed to ST_DSP_POWER_OFF.\n"));
            break;

        //Send DSP play command
        case TR_DSP_R_PLAY:
            //Change to DSP preparing playback
            _u1AudDspState = ST_DSP_PLAYING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_READY changed to ST_DSP_PLAYING.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP is preparing playback
    case ST_DSP_PLAYING:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_DSP_R_STOP:
            //Change to DSP preparing stop
            _u1AudDspState = ST_DSP_STOPPING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PLAYING changed to ST_DSP_STOPPING.\n"));
            break;

        //DSP send play
        case TR_DSP_S_PLAY_OK:
            //Change to DSP playaback
            _u1AudDspState = ST_DSP_PLAYOK;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PLAYING changed to ST_DSP_PLAYOK.\n"));
            break;

        //Send DSP pause
        case TR_DSP_R_PAUSE:
            //Change to DSP preparing pause
            _u1AudDspState = ST_DSP_PAUSING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PLAYING changed to ST_DSP_PAUSING.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP is playing
    case ST_DSP_PLAYOK:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_DSP_R_STOP:
            //Change to DSP preparing stop
            _u1AudDspState = ST_DSP_STOPPING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PLAYOK changed to ST_DSP_STOPPING.\n"));
            break;

        //Send DSP pause
        case TR_DSP_R_PAUSE:
            //Change to DSP preparing pause
            _u1AudDspState = ST_DSP_PAUSING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PLAYOK changed to ST_DSP_PAUSING.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP is preparing pause
    case ST_DSP_PAUSING:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_DSP_R_STOP:
            //Change to DSP preparing stop
            _u1AudDspState = ST_DSP_STOPPING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PAUSING changed to ST_DSP_STOPPING.\n"));
            break;

        //DSP send pause ok
        case TR_DSP_S_PAUSE_OK:
            //Change to DSP paused
            _u1AudDspState = ST_DSP_PAUSED;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PAUSING changed to ST_DSP_PAUSED.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP is paused
    case ST_DSP_PAUSED:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_DSP_R_STOP:
            //Change to DSP preparing stop
            _u1AudDspState = ST_DSP_STOPPING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PAUSED changed to ST_DSP_STOPPING.\n"));
            break;

        //Send DSP resume
        case TR_DSP_R_RESUME:
            //Change to DSP preparing resume
            _u1AudDspState = ST_DSP_RESUMING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_PAUSED changed to ST_DSP_RESUMING.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //DSP is prepare resume
    case ST_DSP_RESUMING:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_DSP_R_STOP:
            //Change to DSP preparing stop
            _u1AudDspState = ST_DSP_STOPPING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_RESUMING changed to ST_DSP_STOPPING.\n"));
            break;

        //DSP send resume ok
        case TR_DSP_S_RESUME_OK:
            //Change to DSP playback
            _u1AudDspState = ST_DSP_PLAYING;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_RESUMING changed to ST_DSP_PLAYING.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

        //DSP is prepare stop
    case ST_DSP_STOPPING:
        switch (u1Trigger)
        {
        //DSP send stop ok
        case TR_DSP_S_STOP_OK:
            //Change to DSP playback
            _u1AudDspState = ST_DSP_READY;
            LOG(LOG_FEATURE, TEXT("_u1AudDspState from ST_DSP_STOPPING changed to ST_DSP_READY.\n"));
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_FAIL;
            break;
        }
        break;

    //Not acceptable for other state
    default:
        u1Ret = RTN_DSP_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dsp_state_lock, flags);

    return u1Ret;
}


/***************************************************************************
Function : u1DspAState
Description : DSP A (PRI) state machine (decoder to audio output)
Parameter : u1Trigger: trigger for state change
Return    : as defined
***************************************************************************/
u8 u1DspAState(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_A_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dspa_lock, flags);
    //Check original status
    switch (_u1DspAState)
    {
    //DSP A power off
    case ST_DSP_A_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP A power on
        case TR_DSP_A_POWER_ON:
            //Change to init state
            _u1DspAState = ST_DSP_A_INIT;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A initialization
    case ST_DSP_A_INIT:
        switch (u1Trigger)
        {
        //DSP A power off
        case TR_DSP_A_POWER_OFF:
            //Change to DSP A off
            _u1DspAState = ST_DSP_A_POWER_OFF;
            break;

        //DSP A ready
        case TR_DSP_A_INIT_READY:
            //Change to DSP A audio output off
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A is disconnected (and audio output is on)
    case ST_DSP_A_DISCONNECTED:
        switch (u1Trigger)
        {
        //Send DSP A connect
        case TR_DSP_A_R_CONNECT:
            //Change to DSP A connecting
            _u1DspAState = ST_DSP_A_CONNECTING;
            break;

        //Send DSP A step command
        case TR_DSP_A_R_STEP:
            //Change to DSP A connecting
            _u1DspAState = ST_DSP_A_STEPPING;
            break;

#ifdef NEW_STEP_FLOW
        case TR_DSP_A_R_STEP_TO_END:
            _u1DspAState = ST_DSP_A_STEPPING_TO_END;
            break;

        //DSP A send step done
        case TR_DSP_A_S_STEP_CANCEL_DONE:
            //Change to DSP A audio output disconnected
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;
#endif

        //Don't change state if receive 2 step_done from DSP A
        case TR_DSP_A_S_STEP_DONE:
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A is connecting
    case ST_DSP_A_CONNECTING:
        switch (u1Trigger)
        {
        //DSP A send connected
        case TR_DSP_A_S_CONNECTED:
            //Change to DSP A audio output connected
            _u1DspAState = ST_DSP_A_CONNECTED;
            break;

        //Send DSP A disconnect
        case TR_DSP_A_R_DISCONNECT:
            //Change to DSP A disconnecting
            _u1DspAState = ST_DSP_A_DISCONNECTING;
            break;

#ifdef NEW_STEP_FLOW
        // Accept these triggers since it is possible
        case TR_DSP_A_R_STEP:
            // State not change
            break;

        case TR_DSP_A_S_STEP_DONE:
            // State not change
            break;

        case TR_DSP_A_S_STEP_CANCEL_DONE:
            // State not change
            break;
#endif

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A is disconnected
    case ST_DSP_A_CONNECTED:
        switch (u1Trigger)
        {
        //Send DSP A disconnect
        case TR_DSP_A_R_DISCONNECT:
            //Change to DSP A disconnecting
            _u1DspAState = ST_DSP_A_DISCONNECTING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A is disconnecting
    case ST_DSP_A_DISCONNECTING:
        switch (u1Trigger)
        {
        //DSP A send disconnected
        case TR_DSP_A_S_DISCONNECTED:
            //Change to DSP A audio output disconnected
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;

        //DSP A send connected
        case TR_DSP_A_S_CONNECTED:
            //Ignore DSP A connected event
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A is skepping
    case ST_DSP_A_STEPPING:
        switch (u1Trigger)
        {
        //DSP A send step done
        case TR_DSP_A_S_STEP_DONE:
            //Change to DSP A audio output disconnected
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;

#ifdef NEW_STEP_FLOW
        //Send DSP A step command for step to the end
        case TR_DSP_A_R_STEP_TO_END:
            //Change to DSP A connecting
            _u1DspAState = ST_DSP_A_STEPPING_TO_END;
            break;

        //DSP A send step done
        case TR_DSP_A_S_STEP_CANCEL_DONE:
            //Change to DSP A audio output disconnected
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;
#else
        //Send DSP A step command  for step to the end
        case TR_DSP_A_R_STEP:
            //Change to DSP A connecting
            _u1DspAState = ST_DSP_A_STEPPING;
            break;
#endif
        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

#ifdef NEW_STEP_FLOW
    case ST_DSP_A_STEPPING_TO_END:
        switch (u1Trigger)
        {
        //DSP A send step done
        case TR_DSP_A_S_STEP_CANCEL_DONE:
            //Change to DSP A audio output disconnected
            _u1DspAState = ST_DSP_A_DISCONNECTED;
            break;

        //DSP A send step done: Accept the step done condition but not change state for the racing condition
        case TR_DSP_A_S_STEP_DONE:
            break;

        // Accept step command for racing condition:
        // not accept the step cmd from syncctrl, but it is possible for dsp to give cmd ok notify
        case TR_DSP_A_R_STEP:
            break;
        
        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;
#endif

    //Not acceptable for other state
    default:
        u1Ret = RTN_DSP_A_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dspa_lock, flags);

    return (u1Ret);
}

/***************************************************************************
Function : u1DspAoutState
Description : DSP Aout state machine (decoder to audio output)
Parameter : u1Trigger: trigger for state change
Return    : as defined
***************************************************************************/
u8 u1DspAoutState(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_A_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dsp_aout1_lock, flags);
    //Check original status
    switch (_u1DspAoutState)
    {
    //DSP Aout power off
    case ST_DSP_A_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP A power on
        case TR_DSP_A_POWER_ON:
            //Change to init state
            _u1DspAoutState = ST_DSP_A_INIT;
            break;
            
        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_POWER_OFF\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout initialization
    case ST_DSP_A_INIT:
        switch (u1Trigger)
        {
        //DSP Aout power off
        case TR_DSP_A_POWER_OFF:
            //Change to DSP A off
            _u1DspAoutState = ST_DSP_A_POWER_OFF;
            break;

        //DSP A ready
        case TR_DSP_A_INIT_READY:
            //Change to DSP A audio output off
            _u1DspAoutState = ST_DSP_A_AOUT_OFF;
            break;
        
        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_INIT\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A audio output is off
    case ST_DSP_A_AOUT_OFF:
        switch (u1Trigger)
        {
        //DSP Aout power off
        case TR_DSP_A_POWER_OFF:
            //Change to DSP A off
            _u1DspAoutState = ST_DSP_A_POWER_OFF;
            break;

        //Send DSP A audio output on
        case TR_DSP_A_R_AOUT_ON:
            //Change to DSP A audio output starting
            _u1DspAoutState = ST_DSP_A_AOUT_STARTING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_OFF\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A audio output is ON
    case ST_DSP_A_AOUT_ON:
        switch (u1Trigger)
        {
        //Send DSP A audio output on
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output starting
            _u1DspAoutState = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_ON\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout audio output is starting
    case ST_DSP_A_AOUT_STARTING:
        switch (u1Trigger)
        {
        //DSP Aout send audio output started
        case TR_DSP_A_S_AOUT_STARTED:
            //Change to DSP A audio output disconnected (but audio output is on)
            _u1DspAoutState = ST_DSP_A_AOUT_ON;
            break;

        //DSP A send audio output stopped
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output off
            _u1DspAoutState = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_STARTING\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout audio output is stopping
    case ST_DSP_A_AOUT_STOPPING:
        switch (u1Trigger)
        {
        //DSP A send audio output stopped
        case TR_DSP_A_S_AOUT_STOPPED:
            //Change to DSP A audio output off
            _u1DspAoutState = ST_DSP_A_AOUT_OFF;
            break;

        // patch for BDP00017407 @ 11/25/2008
        case TR_DSP_A_S_AOUT_STARTED:
        //DSP A send audio output stopped
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output off
            _u1DspAoutState = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_STOPPING\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //Not acceptable for other state
    default:
        LOG(LOG_CTRLF, _T("[AUD][AOUT2]Not acceptable state\n"));
        u1Ret = RTN_DSP_A_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dsp_aout1_lock, flags);

    return (u1Ret);

}


/***************************************************************************
Function : u1DspAout2State
Description : DSP HDMI Aout state machine
Parameter : u1Trigger: trigger for state change
Return    : as defined
***************************************************************************/
u8 u1DspAout2State(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dsp_aout2_lock, flags);
    //Check original status
    switch (_u1DspAout2State)
    {
    //DSP Aout power off
    case ST_DSP_A_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP A power on
        case TR_DSP_A_POWER_ON:
            //Change to init state
            _u1DspAout2State = ST_DSP_A_INIT;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_POWER_OFF\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout initialization
    case ST_DSP_A_INIT:
        switch (u1Trigger)
        {
        //DSP Aout power off
        case TR_DSP_A_POWER_OFF:
            //Change to DSP A off
            _u1DspAout2State = ST_DSP_A_POWER_OFF;
            break;

        //DSP A ready
        case TR_DSP_A_INIT_READY:
            //Change to DSP A audio output off
            _u1DspAout2State = ST_DSP_A_AOUT_OFF;
            break;
            
        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_INIT\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A audio output is off
    case ST_DSP_A_AOUT_OFF:
        switch (u1Trigger)
        {
        //DSP Aout power off
        case TR_DSP_A_POWER_OFF:
            //Change to DSP A off
            _u1DspAout2State = ST_DSP_A_POWER_OFF;
            break;

        //Send DSP A audio output on
        case TR_DSP_A_R_AOUT_ON:
            //Change to DSP A audio output starting
            _u1DspAout2State = ST_DSP_A_AOUT_STARTING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_OFF\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP A audio output is ON
    case ST_DSP_A_AOUT_ON:
        switch (u1Trigger)
        {
        //Send DSP A audio output on
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output starting
            _u1DspAout2State = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_ON\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout audio output is starting
    case ST_DSP_A_AOUT_STARTING:
        switch (u1Trigger)
        {
        //DSP Aout send audio output started
        case TR_DSP_A_S_AOUT_STARTED:
            //Change to DSP A audio output disconnected (but audio output is on)
            _u1DspAout2State = ST_DSP_A_AOUT_ON;
            break;

        //DSP A send audio output stopped
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output off
            _u1DspAout2State = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_STARTING\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //DSP Aout audio output is stopping
    case ST_DSP_A_AOUT_STOPPING:
        switch (u1Trigger)
        {
        //DSP A send audio output stopped
        case TR_DSP_A_S_AOUT_STOPPED:
            //Change to DSP A audio output off
            _u1DspAout2State = ST_DSP_A_AOUT_OFF;
            break;

        //DSP A send audio output stopped
        case TR_DSP_A_S_AOUT_STARTED:
        case TR_DSP_A_R_AOUT_OFF:
            //Change to DSP A audio output off
            _u1DspAout2State = ST_DSP_A_AOUT_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            LOG(LOG_CTRLF, _T("[AUD][AOUT2]Previous state is ST_DSP_A_AOUT_STOPPING\n"));
            u1Ret = RTN_DSP_A_FAIL;
            break;
        }
        break;

    //Not acceptable for other state
    default:
        LOG(LOG_CTRLF, _T("[AUD][AOUT2]Not acceptable state\n"));
        u1Ret = RTN_DSP_A_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dsp_aout2_lock, flags);

    return (u1Ret);

}

u8 u1DspReencState(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_REENC_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dsp_reenc_lock, flags);
    //Check original status
    switch (_u1DspReencState)
    {
    //DSP encoder power off
    case ST_DSP_REENC_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP Encoder power on
        case TR_DSP_REENC_POWER_ON:
            //Change to init state
            _u1DspReencState = ST_DSP_REENC_INIT;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    //DSP encoder INIT. state
    case ST_DSP_REENC_INIT:
        switch (u1Trigger)
        {
        //DSP Encoder power on
        case TR_DSP_REENC_INIT_READY:
            //Change to init state
            _u1DspReencState = ST_DSP_REENC_STOP;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    // DSP Encoder STOP state.
    case ST_DSP_REENC_STOP:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_R_START:
            _u1DspReencState = ST_DSP_REENC_STARTING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

        // DSP Encoder STARTING state.
    case ST_DSP_REENC_STARTING:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_S_STARTED:
            _u1DspReencState = ST_DSP_REENC_START;
            break;

        case TR_DSP_REENC_R_STOP:
            _u1DspReencState = ST_DSP_REENC_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    // DSP Encoder START state.
    case ST_DSP_REENC_START:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_R_STOP:
            _u1DspReencState = ST_DSP_REENC_STOPPING;
            break;

#ifdef DSP_REENC_ON_DSPC
        case TR_DSP_REENC_R_PAUSE:
            _u1DspReencState = ST_DSP_REENC_PAUSING;
            break;
#endif

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    // DSP Encoder STOPPING state.
    case ST_DSP_REENC_STOPPING:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_S_STOPPED:
            _u1DspReencState = ST_DSP_REENC_STOP;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

#ifdef DSP_REENC_ON_DSPC
    // DSP Encoder PAUSE state.
    case ST_DSP_REENC_PAUSE:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_R_RESUME:
            _u1DspReencState = ST_DSP_REENC_RESUMING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    // DSP Encoder PAUSING state.
    case ST_DSP_REENC_PAUSING:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_S_PAUSED:
        case TR_DSP_REENC_S_STOPPED:
            _u1DspReencState = ST_DSP_REENC_PAUSE;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;

    // DSP Encoder RESUMING state.
    case ST_DSP_REENC_RESUMING:
        switch (u1Trigger)
        {
        case TR_DSP_REENC_S_STARTED:
            _u1DspReencState = ST_DSP_REENC_START;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_REENC_FAIL;
            break;
        }
        break;
#endif

    //Not acceptable for other state
    default:
        u1Ret = RTN_DSP_REENC_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dsp_reenc_lock, flags);

    return (u1Ret);
}


static void vDspBStateDbg(u16 u2State)
{
    _u2DspBDbgState[_u2DspBDbgCnt%DSPB_STATE_DBG_LEN] = u2State;
    _u2DspBDbgCnt++;
}

u8 u1DspBDec2State(u8 u1Trigger)
{
    u32 flags = 0;

    ENTERCRITICALSECTION(&dspb_dec2_lock, flags);
    switch (_u1DspBDec2State)
    {
    case ST_DSP_B_POWER_OFF:
        if (u1Trigger == TR_DSP_B_POWER_ON)
        {
            _u1DspBDec2State = ST_DSP_B_INIT;
        }
        break;
    case ST_DSP_B_INIT:
        if (u1Trigger == TR_DSP_B_INIT_READY)
        {
            _u1DspBDec2State = ST_DSP_B_READY;
        }
        break;

    case ST_DSP_B_READY:
        if (TR_DSP_B_S_SEND_CFG == u1Trigger) //call @setsample rate
        {
            _u1DspBDec2State = ST_DSP_B_DECODING;
        }
        break;

    case ST_DSP_B_DECODING:
        if (TR_DSP_B_S_STOP_OK == u1Trigger) //call @setsample rate
        {
            _u1DspBDec2State = ST_DSP_B_READY;
        }
        break;

    default:
        break;
    }
    LEAVECRITICALSECTION(&dspb_dec2_lock, flags);

    return RTN_DSP_B_SUCCESS;
}



u8 u1DspBDec3State(u8 u1Trigger)
{
    u32 flags = 0;

    ENTERCRITICALSECTION(&dspb_dec3_lock, flags);
    switch (_u1DspBDec3State)
    {
    case ST_DSP_B_POWER_OFF:
        if (u1Trigger == TR_DSP_B_POWER_ON)
        {
            _u1DspBDec3State = ST_DSP_B_INIT;
        }
        break;
    case ST_DSP_B_INIT:
        if (u1Trigger == TR_DSP_B_INIT_READY)
        {
            _u1DspBDec3State = ST_DSP_B_READY;
        }
        break;

    case ST_DSP_B_READY:
        if (TR_DSP_B_S_SEND_CFG == u1Trigger) //call @setsample rate
        {
            _u1DspBDec3State = ST_DSP_B_DECODING;
        }
        break;

    case ST_DSP_B_DECODING:
        if (TR_DSP_B_S_STOP_OK == u1Trigger) //call @setsample rate
        {
            _u1DspBDec3State = ST_DSP_B_READY;
        }
        break;

    default:
        break;
    }
    LEAVECRITICALSECTION(&dspb_dec3_lock, flags);

    return RTN_DSP_B_SUCCESS;
}


/***************************************************************************
Function : u1DspBDec1State
Description : DSP B Decoder 1 state machine
Parameter : u1Trigger: trigger for state change
Return    : as defined
***************************************************************************/
u8 u1DspBDec1State(u8 u1Trigger)
{
    u8 u1Ret = RTN_DSP_B_SUCCESS;
    u32 flags = 0;

    ENTERCRITICALSECTION(&dspb_dec1_lock, flags);
    //Check original status
    switch (_u1DspBDec1State)
    {
    //DSP B power off
    case ST_DSP_B_POWER_OFF:
        switch (u1Trigger)
        {
        //DSP B power on
        case TR_DSP_B_POWER_ON:
            //Change to init state
            _u1DspBDec1State = ST_DSP_B_INIT;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B initialization
    case ST_DSP_B_INIT:
        switch (u1Trigger)
        {
        //DSP B power off
        case TR_DSP_B_POWER_OFF:
            //Change to DSP B off
            _u1DspBDec1State = ST_DSP_B_POWER_OFF;
            break;

        //DSP B ready
        case TR_DSP_B_INIT_READY:
            //Change to DSP B decoder 1 ready
            _u1DspBDec1State = ST_DSP_B_READY;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is ready for playback
    case ST_DSP_B_READY:
        switch (u1Trigger)
        {
        //DSP B power off
        case TR_DSP_B_POWER_OFF:
            //Change to DSP B off
            _u1DspBDec1State = ST_DSP_B_POWER_OFF;
            break;
        
        //Send DSP B decoder 1 play command
        case TR_DSP_B_R_PLAY:
            //Change to DSP B decoder 1 parsing
            _u1DspBDec1State = ST_DSP_B_PARSING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is parsing (bitstream or header)
    case ST_DSP_B_PARSING:
        switch (u1Trigger)
        {
        //Send DSP B decoder 1 stop
        case TR_DSP_B_R_STOP:
            //Change to DSP B decoder 1 stopping
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //DSP B decoder 1 send config
        case TR_DSP_B_S_SEND_CFG:
            //Change to DSP B decoder 1 waiting config ack
            _u1DspBDec1State = ST_DSP_B_WAIT_CFG_ACK;
            break;

        //DSP B decoder 1 send end of stream
        case TR_DSP_B_S_EOS:
            //Change to DSP B decoder 1 end of stream
            _u1DspBDec1State = ST_DSP_B_EOS;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is waiting config ack from RISC
    case ST_DSP_B_WAIT_CFG_ACK:
        switch (u1Trigger)
        {
        //Send DSP B decoder 1 stop
        case TR_DSP_B_R_STOP:
            //Change to DSP B decoder 1 stopping
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //Send DSP B decoder 1 config ack
        case TR_DSP_B_R_RECEIVE_CFG:
            //Change to DSP B decoder 1 initialization (fill DSP B->A buffer)
            _u1DspBDec1State = ST_DSP_B_DECODER_INIT;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is decoding initializing (fill DSP B->A buffer)
    case ST_DSP_B_DECODER_INIT:
        switch (u1Trigger)
        {
        //Send DSP B decoder 1 stop
        case TR_DSP_B_R_STOP:
            //Change to DSP B decoder 1 stopping
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //DSP B decoder 1 send decoding ok
        case TR_DSP_B_S_DECODING_OK:
            //Change to DSP B decoder 1 decoding
            _u1DspBDec1State = ST_DSP_B_DECODING;
            break;

        //DSP B decoder 1 send end of stream
        case TR_DSP_B_S_EOS:
            //Change to DSP B decoder 1 end of stream
            _u1DspBDec1State = ST_DSP_B_EOS;
            break;

        //DSP B decoder 1 send config (due to bitstream changed)
        case TR_DSP_B_S_SEND_CFG:
            //Change to DSP B decoder 1 waiting config ack
            _u1DspBDec1State = ST_DSP_B_WAIT_CFG_ACK;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is decoding
    case ST_DSP_B_DECODING:
        switch (u1Trigger)
        {
        //Send DSP B decoder 1 stop
        case TR_DSP_B_R_STOP:
            //Change to DSP B decoder 1 stopping
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //DSP B decoder 1 send end of stream
        case TR_DSP_B_S_EOS:
            //Change to DSP B decoder 1 end of stream
            _u1DspBDec1State = ST_DSP_B_EOS;
            break;

        //DSP B decoder 1 send config (due to bitstream changed)
        case TR_DSP_B_S_SEND_CFG:
            //Change to DSP B decoder 1 waiting config ack
            _u1DspBDec1State = ST_DSP_B_WAIT_CFG_ACK;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is end of stream
    case ST_DSP_B_EOS:
        switch (u1Trigger)
        {
        //Send DSP B decoder 1 stop
        case TR_DSP_B_R_STOP:
            //Change to DSP B decoder 1 stopping
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //Send DSP B decoder 1 replay
        case TR_DSP_B_R_REPLAY:
            //Change to DSP B decoder 1 parsing (again)
            _u1DspBDec1State = ST_DSP_B_PARSING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //DSP B decoder 1 is stopping
    case ST_DSP_B_STOPPING:
        switch (u1Trigger)
        {
        //DSP B decoder 1 send stop ok
        case TR_DSP_B_S_STOP_OK:
            //Change to DSP B decoder 1 ready for playback
            _u1DspBDec1State = ST_DSP_B_READY;
            break;

        case TR_DSP_B_S_DECODING_OK:
            vDspBStateDbg(TR_DSP_B_S_DECODING_OK);
            //Change to DSP B decoder 1 decoding
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //DSP B decoder 1 send config
        case TR_DSP_B_S_SEND_CFG:
            vDspBStateDbg(TR_DSP_B_S_SEND_CFG);
            //Change to DSP B decoder 1 waiting config ack
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //Send DSP B decoder 1 config ack
        case TR_DSP_B_R_RECEIVE_CFG:
            vDspBStateDbg(TR_DSP_B_R_RECEIVE_CFG);
            //Change to DSP B decoder 1 initialization (fill DSP B->A buffer)
            _u1DspBDec1State = ST_DSP_B_STOPPING;
            break;

        //Not acceptable for other trigger
        default:
            u1Ret = RTN_DSP_B_FAIL;
            break;
        }
        break;

    //Not acceptable for other state
    default:
        u1Ret = RTN_DSP_B_FAIL;
        break;
    }
    LEAVECRITICALSECTION(&dspb_dec1_lock, flags);

    return (u1Ret);
}

/***************************************************************************
Function : vAudStateInit
Description : Aud state machine initial
Parameter : none
Return    : none
***************************************************************************/
void vAudStateInit(void)
{
    //AUD-DSP state machine initialize
    _u1AudDspState = ST_DSP_POWER_OFF;

    //DSP A state machine initialize
    _u1DspAState = ST_DSP_A_POWER_OFF;
    _u1DspAoutState= ST_DSP_A_POWER_OFF;
    _u1DspAout2State= ST_DSP_A_POWER_OFF;
    //DSP B decoder state machine initialize
    _u1DspBDec1State = ST_DSP_B_POWER_OFF;
    _u1DspBDec2State = ST_DSP_B_POWER_OFF;    
    _u1DspBDec3State = ST_DSP_B_POWER_OFF;
    // DSP encoder
    _u1DspReencState = ST_DSP_REENC_POWER_OFF;
}


/***************************************************************************
Function : vAudStateReset
Description : Aud state machine initial
Parameter : none
Return    : none
***************************************************************************/
void vAudStateReset(void)
{
    //AUD-DSP state machine initialize
    _u1AudDspState = ST_DSP_READY;

    //DSP A state machine initialize
    _u1DspAState = ST_DSP_A_DISCONNECTED;
    _u1DspAoutState= ST_DSP_A_AOUT_OFF;
    _u1DspAout2State= ST_DSP_A_AOUT_OFF;
    //DSP B decoder state machine initialize
    _u1DspBDec1State = ST_DSP_B_READY;
    _u1DspBDec2State = ST_DSP_B_READY;    
    _u1DspBDec3State = ST_DSP_B_READY;
    // DSP encoder
    _u1DspReencState = ST_DSP_B_READY;
}

/***************************************************************************
Function : u1AudDspGetState
Description : Get AUD-DSP state
Parameter : none
Return    : as defined
***************************************************************************/
u8 u1AudDspGetState(void)
{
    //return status
    return _u1AudDspState;
}

/***************************************************************************
Function : u1DspAGetState
Description : Get DSP A state
Parameter : none
Return    : as defined
***************************************************************************/
u8 u1DspAGetState(void)
{
    //return status
    return _u1DspAState;
}

/***************************************************************************
Function : u1DspAoutGetState
Description : Get DSP Aout state
Parameter : none
Return    : as defined
***************************************************************************/
u8 u1DspAoutGetState(void)
{
    //return status
    return _u1DspAoutState;
}

/***************************************************************************
Function : u1DspBDec1GetState
Description : Get DSP B Decoder 1 state
Parameter : none
Return    : as defined
***************************************************************************/
u8 u1DspBDec1GetState(void)
{
    //return status
    return _u1DspBDec1State;
}

u8 u1DspBDec2GetState(void)
{
    //return status
    return _u1DspBDec2State;
}


u8 u1DspBDec3GetState(void)
{
    //return status
    return _u1DspBDec3State;
}

/***************************************************************************
Function : u1DspAout2GetState
Description : Get DSP Aout2 state
Parameter : none
Return    : as defined
***************************************************************************/
u8 u1DspAout2GetState(void)
{
    //return status
    return _u1DspAout2State;
}

u8 u1DspReencGetState(void)
{
    //return status
    return _u1DspReencState;
}

/***************************************************************************
Function : fgASVCheckInit
Description :
Parameter :
Return    : TRUE: All state machine is initialized
***************************************************************************/
bool fgASVCheckInit(void)
{
    if((u1AudDspGetState() != ST_DSP_INIT) &&
        (u1DspAGetState() != ST_DSP_A_INIT) &&
        (u1DspBDec1GetState()!=ST_DSP_B_INIT))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


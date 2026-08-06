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
                                                      headfile include
******************************************************************************/
#include "aud_oal.h"
#include "GpsMix_AsvTrigger.h"
#include "GpsMix_mw.h"
#include "GpsMix_drvthread.h"
#include "DspFunc.h"
#include "GpsMix_if.h"
#include "aud_debug.h"

u8 _u1AudGpsMixDspState;

/******************************************************************************
                                                   Function 
******************************************************************************/
void vAudGpsMixStateInit(void)
{
    _u1AudGpsMixDspState = ST_GPS_MIX_DSP_READY;
}

u8 u1AudGpsMixDspState(u8 u1Trigger)
{
    switch (_u1AudGpsMixDspState)
    {
    case ST_GPS_MIX_DSP_READY:
        switch (u1Trigger)
        {
        case TR_GPS_MIX_DSP_R_START:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STARTED;
            return RTN_GPS_MIX_DSP_SUCCESS;
                
        default:
            return RTN_GPS_MIX_DSP_FAIL;
        }
        break;

    case ST_GPS_MIX_DSP_STARTED:
        switch (u1Trigger)
        {
        case TR_GPS_MIX_DSP_R_STOP:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STOPPED;
            return RTN_GPS_MIX_DSP_SUCCESS;
                
        case TR_GPS_MIX_DSP_R_PAUSE:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_PAUSED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        case TR_GPS_MIX_DSP_R_START:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STARTED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        default:
            return RTN_GPS_MIX_DSP_FAIL;
        }
        break;

    case ST_GPS_MIX_DSP_STOPPED:
        switch(u1Trigger)
        {
        case TR_GPS_MIX_DSP_R_START:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STARTED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        case TR_GPS_MIX_DSP_R_STOP:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STOPPED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        default:
            return RTN_GPS_MIX_DSP_FAIL;
        }
        break;

    case ST_GPS_MIX_DSP_PAUSED:
        switch (u1Trigger)
        {
        //Send DSP stop command
        case TR_GPS_MIX_DSP_R_STOP:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STOPPED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        case TR_GPS_MIX_DSP_R_RESUME:
            _u1AudGpsMixDspState = ST_GPS_MIX_DSP_STARTED;
            return RTN_GPS_MIX_DSP_SUCCESS;

        default:
            return RTN_GPS_MIX_DSP_FAIL;
        }
        break;

    default:
        return RTN_GPS_MIX_DSP_FAIL;
    }
}

u8 u1AudGpsMixDspGetState(void)
{
     return _u1AudGpsMixDspState;
}

void i4AsvGpsMixDspNotifyPlayCmdDone(void)
{
   AudGpsMix_AsvCommandDone(AUD_GPS_MIX_CMD_START);
}

void i4AsvGpsMixDspNotifyStopCmdDone(void)
{
   AudGpsMix_AsvCommandDone(AUD_GPS_MIX_CMD_STOP);
}

void i4AsvGpsMixDspNotifyPauseCmdDone(void)
{
   AudGpsMix_AsvCommandDone(AUD_GPS_MIX_CMD_PAUSE);
}

void i4AsvGpsMixDspNotifyResumeCmdDone(void)
{
   AudGpsMix_AsvCommandDone(AUD_GPS_MIX_CMD_RESUME);
}

u32 i4AsvGpsMixDspNotifyConsumedData(void)
{
    u32 u4CusDataSize = 0;
    DspGetGpsMixConsumedData(&u4CusDataSize);
    x_event_set(m_hGpsMixConsumeDataEvent);
    return (u4CusDataSize);
}

void u1AsvGpsMixDspAStartCmd(void)
{
    vDspCmd(UOP_DSP_GPS_MIX_START);
}

void u1AsvGpsMixDspAStopCmd(void)
{
    vDspCmd(UOP_DSP_GPS_MIX_STOP);
}
void u1AsvGpsMixDspAPauseCmd(void)
{
    vDspCmd(UOP_DSP_GPS_MIX_PAUSE);
}

void u1AsvGpsMixDspAResumeCmd(void)
{    
    vDspCmd(UOP_DSP_GPS_MIX_RESUME);
}

u8 u1AsvGpsMixStartCmd(void)
{
    u8 u1trigger, u1result;
    u1trigger = TR_GPS_MIX_DSP_R_START;

    u1result = u1AudGpsMixDspState(u1trigger);

    if(u1result == RTN_GPS_MIX_DSP_FAIL)
    {
        u8 u1_AudState;
        u1_AudState = u1AudGpsMixDspGetState();
        LOG(LOG_FEATURE, TEXT("*****[AUDGPSMIX] DSPA:TR_GPS_MIX_DSP_R_START fail; AUDGPSMIX State = 0x%x\r\n"),u1_AudState);
        VERIFY(u1result);
    }
    
    u1AsvGpsMixDspAStartCmd();
    return RTN_GPS_MIX_DSP_SUCCESS;
}

u8 u1AsvGpsMixStopCmd(void)
{
    u8 u1trigger, u1result;
    
    u1trigger = TR_GPS_MIX_DSP_R_STOP;
    u1result = u1AudGpsMixDspState(u1trigger);
    if(u1result == RTN_GPS_MIX_DSP_FAIL)
    {
        u8 u1_AudState;
        u1_AudState = u1AudGpsMixDspGetState();
       LOG(LOG_FEATURE, TEXT("*****[AUDGPSMIX] DSPA:TR_GPS_MIX_DSP_R_STOP fail; AUDGPSMIX State = 0x%x\r\n"),u1_AudState);
        VERIFY(u1result);
    }
    
    u1AsvGpsMixDspAStopCmd();
    return RTN_GPS_MIX_DSP_SUCCESS;
}

u8 u1AsvGpsMixPauseCmd(void)
{
    u8 u1trigger, u1result;
    
    u1trigger = TR_GPS_MIX_DSP_R_PAUSE;
    u1result = u1AudGpsMixDspState(u1trigger);
    if(u1result == RTN_GPS_MIX_DSP_FAIL)
    {
        u8 u1_AudState;
        u1_AudState = u1AudGpsMixDspGetState();
        LOG(LOG_FEATURE, TEXT("*****[AUDGPSMIX] DSPA:TR_GPS_MIX_DSP_R_PAUSE fail; AUDGPSMIX State = 0x%x\r\n"),u1_AudState);
        VERIFY(u1result);
    }

     u1AsvGpsMixDspAPauseCmd();
     return RTN_GPS_MIX_DSP_SUCCESS;
}

u8 u1AsvGpsMixResumeCmd(void)
{
    u8 u1trigger, u1result;
    
    u1trigger = TR_GPS_MIX_DSP_R_RESUME;
    u1result = u1AudGpsMixDspState(u1trigger);
    if(u1result == RTN_GPS_MIX_DSP_FAIL)
    {
        u8 u1_AudState;
        u1_AudState = u1AudGpsMixDspGetState();
        LOG(LOG_FEATURE, TEXT("*****[AUDGPSMIX] DSPA:TR_GPS_MIX_DSP_R_RESUME fail; AUDGPSMIX State = 0x%x\r\n"),u1_AudState);
        VERIFY(u1result);
    }
    
     u1AsvGpsMixDspAResumeCmd();
     return RTN_GPS_MIX_DSP_SUCCESS;
}



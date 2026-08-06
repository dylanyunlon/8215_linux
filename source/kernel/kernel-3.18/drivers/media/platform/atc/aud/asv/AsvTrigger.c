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
//#include "x_typedef.h"
#include <linux/types.h>
#include "x_assert.h"
#include "aud_debug.h"
#include "AsvNotify.h"
#include "AsvState.h"
#include "AsvDef.h"
#include "DspAsvInc.h"
#include "AsvAudDrv.h"
#include "AsvDspCtrl.h"
#include "aud_drv.h"
#include "aud_power.h"
#include "aud_oal.h"
#include "aud_drv_config.h"
#include <media/atc/aud_output.h>
#include "DspShm.h"
#include "DspFunc.h"
#include "aud_if.h"

extern void* g_hAudFlushEvent;
extern void* g_hAudDecReady;

extern u8 _u1DspAoutState;
extern u8 _u1DspAout2State;

extern u32 g_u4AudPriICBId;
extern bool   g_fgFirstAoutArrive;
extern bool   g_fgAVSyncSkipToend;
extern bool   g_fgAudEosState;

extern void DspSetBassManageTable(u32 u4FreqIdx);
extern u8 uReadShmUINT8(u16 u2Addr);
extern u32 g_u4DspSetSampleRateAck[];

// DSP A State
u8 u1AsvDspAInit(void)
{
    u8 u1result = u1DspAState(TR_DSP_A_POWER_ON);

    if (u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspAoutState(TR_DSP_A_POWER_ON);
    if (u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspAout2State(TR_DSP_A_POWER_ON);
    if (u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    return(u1result);
}

u8 u1AsvDspAReady(void)
{
    u8 u1result = u1DspAState(TR_DSP_A_INIT_READY);

    if (u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspAoutState(TR_DSP_A_INIT_READY);
    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspAout2State(TR_DSP_A_INIT_READY);
    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    return(u1result);
}

u8 u1AsvReencStartCmd(u8 ucDecId)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_R_START);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vDspCmd(UOP_DSP_REENC_START);

    return(u1result);
}

u8 u1AsvReencStarted(u8 ucDecId)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_S_STARTED);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vAsvNotifyReencStartDone(ucDecId);

    return(u1result);
}

u8 u1AsvReencStopCmd(u8 ucDecId)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_R_STOP);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vDspCmd(UOP_DSP_REENC_STOP);

    return(u1result);
}

u8 u1AsvReencStopped(u8 ucDecId)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_S_STOPPED);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    vAsvNotifyReencStopDone(ucDecId);

    return(u1result);
}

u8 u1AsvReencPauseCmd(void)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_R_PAUSE);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vDspCmd(UOP_DSP_REENC_STOP);

    return(u1result);
}


u8 u1AsvReencResumeCmd(void)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_R_RESUME);

    if (u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vDspCmd(UOP_DSP_REENC_START);

    return(u1result);
}


u8 u1AsvDspReencReady(void)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_INIT_READY);

    if(u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    return(u1result);
}

u8 u1AsvDspAAoutOn(void)
{
    u8 u1result = u1DspAoutState(TR_DSP_A_R_AOUT_ON);
    u16 u2Type = uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE);

    DspSetAOutMediaType(AUD_AOUT1, u2Type);
    DspSetLineDmxRemapping(AUD_AOUT1, u2Type);

    if(u1result == RTN_DSP_A_SUCCESS)
    {
        vDspCmd(UOP_DSP_AOUT_ON);
    }
    if(u1result == RTN_DSP_A_FAIL)
    {
        u8 u1_DspAoutState;
        u1_DspAoutState = u1DspAoutGetState();
        LOG(LOG_FAIL, TEXT("[AUD]DSPA:TR_DSP_A_R_AOUT_ON fail; DSPAout State = 0x%x.\n"),u1_DspAoutState);
        AUD_VERIFY(u1result);
    }
    return(u1result);
}

//only for system suspend-->resume
void vAsvDspAAoutResume(void)
{
    u8 u1result;

    _u1DspAoutState = ST_DSP_A_AOUT_OFF;
    u1result = u1DspAoutState(TR_DSP_A_R_AOUT_ON);
    if(u1result == RTN_DSP_A_SUCCESS)
    {
        LOG(LOG_CTRLF, TEXT("[AsvTrigger]Send AOUT ON UOP when DSPA Aout Resume\n"));
        vDspCmd(UOP_DSP_AOUT_ON);
    }
    if(u1result == RTN_DSP_A_FAIL)
    {
        u8 u1_DspAoutState = u1DspAoutGetState();
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_R_AOUT_ON fail; DSPAout State = 0x%x.\n"),
            u1_DspAoutState);
        AUD_VERIFY(u1result);
    }
}
u8 u1AsvDspAAoutOff(void)
{
    u8 u1result = RTN_DSP_A_SUCCESS;
    u8 u1_DspAoutState = u1DspAoutGetState();

    if (u1_DspAoutState == ST_DSP_A_AOUT_STARTING)
    {
        LOG(LOG_CTRLF, TEXT("[AUD] to send AOUT_OFF on AOUT_STARTING state.\n"));
        mdelay(200);
        // now, DSP can accept AOUT_OFF (but u1_DspAoutState may not be updated)
    }

    u1_DspAoutState = u1DspAoutGetState();

    LOG(LOG_CTRLF, TEXT("[AUD]AsvDspAAoutOff:AoutState (%d).\n"), u1_DspAoutState);

    if ((u1_DspAoutState == ST_DSP_A_AOUT_ON) || (u1_DspAoutState == ST_DSP_A_AOUT_STARTING))
    {
        u1result = u1DspAoutState(TR_DSP_A_R_AOUT_OFF);
        if(u1result == RTN_DSP_A_SUCCESS)
        {
            vDspCmd(UOP_DSP_AOUT_OFF);
        }
        if(u1result == RTN_DSP_A_FAIL)
        {
            LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_R_AOU_OFF fail; DSPAout State = 0x%x.\n"),
                u1_DspAoutState);
            AUD_VERIFY(u1result);
        }
    }
    return(u1result);
}

void vAsvDspDecReadyTrigger(u8 u1DecId)
{
    //Send a command to DSP
    if( u1DecId == PRI_DEC)
    {
        vDspCmd(UOP_DSP_MIXER_PTS_READY);
    }
}

u8 u1AsvDspAAoutStopped(void)
{
    u8 u1result = u1DspAoutState(TR_DSP_A_S_AOUT_STOPPED);

    if (u1result == RTN_DSP_A_FAIL)
    {
        u8 u1_DspAoutState = u1DspAoutGetState();
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_S_AOUT_STOPPED fail; DSPAout State = 0x%x.\n"),
            u1_DspAoutState);
        AUD_VERIFY(u1result);
    }

#if 0 /*disable IEC off:1.triggle amp pop;2.affect mixhw pop */
    //Mute IEC if required
    if (fgDspChkFreqSetting() || fgDspChkSampleRateChange())
    {
        vAudHalSetIecMute();
        //Delay 100ms
        mdelay(10);
    }
#endif

    //refresh speaker size FREQ
    DspSetBassManageTable(SAMPLE_48K);

    //Change HW setting, including clock and HDMI info
    vAsvSetAout1Periph();

    //Set IEC according to the new setting
    vDspSetFreqDone ();

    //Peripheral Setting done, turn on Aout
    //vAsvPeriphDone();
    u1AsvDspAAoutOn();

    return(u1result);
}

u8 u1AsvDspAAoutStarted(void)
{
    u8 u1result = u1DspAoutState(TR_DSP_A_S_AOUT_STARTED);
    u16 u2FrnType = 0;
    u16 u2RearType = 0;

    if (u1result == RTN_DSP_A_FAIL)
    {
        u8 u1_DspAoutState = u1DspAoutGetState();
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_S_AOUT_STARTED fail; DSPAout State = 0x%x \n"),
            u1_DspAoutState);
        AUD_VERIFY(u1result);
    }

    DspGetAOutMediaType(AUD_AOUT1, &u2FrnType);
    DspGetAOutMediaType(AUD_AOUT2, &u2RearType);

    if (g_u4DspSetSampleRateAck[PRI_DEC] == 1)
    {
        if (u2RearType != AUD_OUT_MEDIA_USB)
        {
            if((u1DspBDec1GetState() == ST_DSP_B_DECODER_INIT) ||(u1DspBDec1GetState() == ST_DSP_B_STOPPING))
            {
                LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC1\n"));
                vDspCmd(UOP_DSP_FS_ACK);
                g_u4DspSetSampleRateAck[PRI_DEC] = 0;
            }
            else
            {
                LOG(LOG_DUALCTRL, TEXT("DSPA:u1DspBDec1GetState()=0x%x \n"),u1DspBDec1GetState());
            }
        }
        else
        {
            LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC1 When AOUT2 Started\n"));
        }
    }
    if (g_u4DspSetSampleRateAck[SEC_DEC] == 1)
    {
        if (u2RearType != AUD_OUT_MEDIA_LINE_IN)
        {
            if ((u1DspBDec2GetState() == ST_DSP_B_DECODING))
            {
                LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC2\n"));
                vDspCmd(UOP_DSP_FS_ACK_DEC4);
                g_u4DspSetSampleRateAck[SEC_DEC] = 0;
            }
        }
        else
        {
            LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC2 When AOUT2 Started\n"));
        }
    }
    if (g_u4DspSetSampleRateAck[TER_DEC] == 1)
    {
        if (u2RearType != AUD_OUT_MEDIA_LINE_IN2)
        {
            if ((u1DspBDec3GetState() == ST_DSP_B_DECODING))
            {
                LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC3\n"));
                vDspCmd(UOP_DSP_FS_ACK_DEC5);
                g_u4DspSetSampleRateAck[TER_DEC] = 0;
            }
        }
        else
        {
            LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC2 When AOUT2 Started\n"));
        }
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("[AOUT1] Media type is 0x%x\n"),u2FrnType);
    }

    if((u1DspBDec1GetState() == ST_DSP_B_DECODING)&&
       (u1DspAGetState()==ST_DSP_A_CONNECTING))
    {
        // if decoder is ready
        vAsvNotifyDecReady(PRI_DEC);
    }

    return(u1result);
}

u8 u1AsvDspAAout2On(void)
{
    u8 u1result;
    u16 u2Type =u2ReadDspShmWORD(B_REAR_AOUT_MEDIA_TYPE);
    LOG(LOG_DUALCTRL, TEXT("Enter u1AsvDspAAout2On\n"));

    u1result = u1DspAout2State(TR_DSP_A_R_AOUT_ON);
    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    LOG(LOG_CTRLF, TEXT("u1AsvDspAAout2On rear media type = 0x%x\n"), u2Type);

    DspSetAOutMediaType(AUD_AOUT2, u2Type);
    DspSetLineDmxRemapping(AUD_AOUT2, u2Type);

    vDspCmd(UOP_DSP_AOUT2_ON);
    return TRUE;
}

u8 u1AsvDspAAout2Off(void)
{
    u8 u1result = u1DspAout2State(TR_DSP_A_R_AOUT_OFF);

    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    vDspCmd(UOP_DSP_AOUT2_OFF);
    return TRUE;
}


u8 u1AsvDspAAout2Stopped(void)
{
    u8 u1result;
    LOG(LOG_DUALCTRL, TEXT("Enter u1AsvDspAAout2Stopped\n"));

    u1result = u1DspAout2State(TR_DSP_A_S_AOUT_STOPPED);
    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    LOG(LOG_DUALCTRL, TEXT("rear media type = 0x%x\n"),uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE));
    LOG(LOG_DUALCTRL, TEXT("front media type = 0x%x\n"),uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE));

     /* if Line in Rear ,Select Bypass Mode*/
    if(uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE) != AUD_OUT_MEDIA_UNDEF)
    {
       // Change HW setting, including clock and HDMI info
        LOG(LOG_DUALCTRL, TEXT("[u1AsvDspAAout2Stopped]Set NOT  Bypass!\r\n"));
        vAsvSetAout2Periph();
    }
    //Set IEC according to the new setting
    vDspSetFreqDone();

    //Peripheral Setting done, turn on Aout
    //vAsvPeriphDone();
    u1AsvDspAAout2On();

    return RTN_DSP_A_SUCCESS;
}

u8 u1AsvDspAAout2Started(void)
{
    u8 u1result;
    u16 u2Type = 0;
    LOG(LOG_DUALCTRL, TEXT("Enter u1AsvDspAAout2Started\n"));

    u1result = u1DspAout2State(TR_DSP_A_S_AOUT_STARTED);

    if(u1result == RTN_DSP_A_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    DspGetAOutMediaType(AUD_AOUT2, &u2Type);
    LOG(LOG_DUALCTRL, TEXT("u1AsvDspAAout2Started rear media type = 0x%x\n"),u2Type);

    if (g_u4DspSetSampleRateAck[PRI_DEC]==1)
    {
        u8 u1DspBDecState = u1DspBDec1GetState();
        if((u1DspBDecState == ST_DSP_B_DECODER_INIT) ||( u1DspBDecState== ST_DSP_B_STOPPING))
        {
            LOG(LOG_DUALCTRL, TEXT("[AUD]FS ACK TO DSPB PRIMARY DEC in AOUT2 Started\n"));
            vDspCmd(UOP_DSP_FS_ACK);
            g_u4DspSetSampleRateAck[PRI_DEC] =0;
        }
    }
    if (g_u4DspSetSampleRateAck[SEC_DEC]==1)
    {
        LOG(LOG_DUALCTRL, TEXT("[AUD]FS ACK TO DSPB DEC2 in AOUT2 Started[BYPASS]\n"));

        if ((u1DspBDec2GetState() == ST_DSP_B_DECODING))
        {
            LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC2\n"));
            vDspCmd(UOP_DSP_FS_ACK_DEC4);
            g_u4DspSetSampleRateAck[SEC_DEC] =0;
        }
    }
    if (g_u4DspSetSampleRateAck[TER_DEC]==1)
    {
        LOG(LOG_DUALCTRL, TEXT("[AUD]FS ACK TO DSPB DEC3 in AOUT2 Started.\n"));
        LOG(LOG_DUALCTRL, TEXT("u1DspBDec3GetState 0x%x.\n"), u1DspBDec3GetState());

        if ((u1DspBDec3GetState() == ST_DSP_B_DECODING))
        {
            LOG(LOG_DUALCTRL, TEXT("[AOUT1]ACK to DEC3\n"));
            vDspCmd(UOP_DSP_FS_ACK_DEC5);
            g_u4DspSetSampleRateAck[TER_DEC] =0;
        }
    }

    return RTN_DSP_A_SUCCESS;
}

u8 u1AsvDspAConnect(u8 u1DecId)
{
    u8 u1result;
    u8 u1DspState = u1DspAGetState();

    if (u1DspState == ST_DSP_A_CONNECTING)
    {
        u1result = RTN_DSP_A_SUCCESS;
        return(u1result);
    }

    u1result = u1DspAState(TR_DSP_A_R_CONNECT);
    if(u1result == RTN_DSP_A_SUCCESS)
    {
        if (u1DecId == PRI_DEC)
        {
            vDspCmd(UOP_DSP_MIXER_CONNECT); //Connect DSP A and DSP B primary decoder
        }
        else
        {
            vDspCmd(UOP_DSP_SECONDARY_MIXER_CONNECT); //Connect DSP A and DSP B primary decoder
        }
    }
    else // (u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_R_CONNECT fail; DecId= 0x%X ;DSPA State = 0x%x\n"),
            u1DecId, u1DspAGetState());
        AUD_VERIFY(u1result);
    }
    return(u1result);
}

u8 u1AsvDspAConnected(u8 u1DecId)
{
    u8 u1DspBState, u1DspState;
    u8 u1result = u1DspAState(TR_DSP_A_S_CONNECTED);

    if (u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_S_CONNECTED fail; DecId= 0x%X ;DSPA State = 0x%x\n"),
            u1DecId, u1DspAGetState());
        AUD_VERIFY(u1result);
    }
    u1DspBState = u1DspBDec1GetState();
    u1DspState = u1AudDspGetState();

    if(((u1DspBState== ST_DSP_B_DECODING) ||
        (u1DspBState == ST_DSP_B_EOS) ||
        (u1DspBState== ST_DSP_B_DECODER_INIT) ||
        (u1DspBState== ST_DSP_B_WAIT_CFG_ACK))&&
        ( u1DspState== ST_DSP_PLAYING))
    {
       u1AsvDspPlayed(u1DecId);
    }

    if((u1DspState == ST_DSP_STOPPING)||
       (u1DspState == ST_DSP_PAUSING)||
       (u1DspBState == ST_DSP_B_WAIT_CFG_ACK))
    {
       u1AsvDspADisconnect(u1DecId);
    }

    return(u1result);

}

u8 u1AsvDspADisconnect(u8 u1DecId)
{
    u8 u1result;

    u8 u1DspState = u1DspAGetState();
    if (u1DspState == ST_DSP_A_DISCONNECTING)
    {
        u1result = RTN_DSP_A_SUCCESS;
        LOG(LOG_FAIL, TEXT("[u1AsvDspADisconnect]: DSP A Disconnecting..."));
        return(u1result);
    }

    u1result = u1DspAState(TR_DSP_A_R_DISCONNECT);

    if (u1result == RTN_DSP_A_SUCCESS)
    {
        if (u1DecId == PRI_DEC)
        {
            vDspCmd(UOP_DSP_MIXER_DISCONNECT);
        }
        else
        {
            vDspCmd(UOP_DSP_SECONDARY_MIXER_DISCONNECT);
        }
    }
    else
    {
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_S_DISCONNECTED fail,DecId= 0x%X DSPA State = 0x%x \n"),
            u1DecId, u1DspAGetState());
        AUD_VERIFY(u1result);
    }
    return (u1result);
}

u8 u1AsvDspADisconnected(u8 u1DecId)
{
    u8 u1DspState;
    u8 u1result = u1DspAState(TR_DSP_A_S_DISCONNECTED);

    if (u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_S_DISCONNECTED fail,DecId= 0x%X DSPA State = 0x%x \n"),
            u1DecId, u1DspAGetState());
        AUD_VERIFY(u1result);
    }

    u1DspState = u1AudDspGetState();
    if (u1DspState == ST_DSP_STOPPING)
    {
        u1result = u1AsvDspBStopCmd(u1DecId);
    }
    else if (u1DspState == ST_DSP_PAUSING)
    {
        u1result = u1AsvDspPaused(u1DecId);
    }
    else
    {
        LOG(LOG_FAIL, TEXT("[Disconnected]DSP State is %d\n"), u1DspState);
    }
    return (u1result);
}


void vAsvDspAStepCmd (u8 u1DecId)
{
    u8 u1result = 0;

    if (u1DecId == PRI_DEC)
    {
        u1result = u1DspAState(TR_DSP_A_R_STEP);
    }
    else
    {
        AUD_ASSERT(0);
    }

    if(u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA:TR_DSP_A_R_STEP fail; DecId= 0x%X ;DSPA State = 0x%X.\n"),
            u1DecId,u1DspAGetState());
        AUD_ASSERT(u1result);
    }
}


u8 u1AsvDspAStepCancel (u8 u1DecId)
{
    if(u1DecId == PRI_DEC)
    {
        vDspCmd(UOP_DSP_MIXER_CANCEL_STEP);
    }
    else
    {
        AUD_VERIFY(0);
    }

    return(RTN_DSP_A_SUCCESS);
}

void vAsvDspAStepDoneProc(u8 u1DecId)
{
    u8 u1result;

    u8 u1DspAudState = u1AudDspGetState();
    u8 u1DspASteate = u1DspAGetState();
    u8 u1DspBState = u1DspBDec1GetState();

    if ((u1DspAudState == ST_DSP_STOPPING) && (u1DspBState != ST_DSP_B_STOPPING ))
    {
        u1result = u1AsvDspBStopCmd(u1DecId);

        if(u1result == RTN_DSP_A_FAIL)
        {
            LOG(LOG_FAIL, TEXT("DSPB STOP CMD fail,DecId= 0x%x, DSPB State = 0x%x \n"),
                u1DecId, u1DspBState);
            AUD_VERIFY(u1result);
        }
    }
    else if((u1DspAudState == ST_DSP_RESUMING) &&
            (( u1DspASteate== ST_DSP_A_DISCONNECTED)&&
             (u1DspBState != ST_DSP_B_POWER_OFF)&&
             (u1DspBState != ST_DSP_B_INIT)&&
             (u1DspBState != ST_DSP_B_READY)&&
             (u1DspBState != ST_DSP_B_STOPPING)))
    {
        u1result = u1AsvDspAConnect(u1DecId);
        if ((u1DspBState == ST_DSP_B_DECODING)&&( u1DspASteate== ST_DSP_A_DISCONNECTED))
        {
            vAsvDspDecReadyTrigger(u1DecId);
        }
    }
}

u8 u1AsvDspAStepDone(u8 u1DecId)
{
    u8 u1result = u1DspAState(TR_DSP_A_S_STEP_DONE);

    if (u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA u1AsvDspAStepDone fail; DecId= 0x%X ;DSPA State = 0x%x \n"),
            u1DecId, u1DspAGetState());
        AUD_VERIFY(u1result);
    }
    vAsvDspAStepDoneProc(u1DecId);

    if (!g_fgAVSyncSkipToend)
    {
        vAsvNotifyStepOK(u1DecId);
    }

    return(u1result);
}

void vAsvDspAStepCancelDone(u8 u1DecId)
{
    u8 u1result = u1DspAState(TR_DSP_A_S_STEP_CANCEL_DONE);

    if (u1result == RTN_DSP_A_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPA vAsvDspAStepCancelDone fail; DecId= 0x%x; DSPA State = 0x%x.\n"),
            u1DecId,u1DspAGetState());
        AUD_ASSERT(u1result);
    }
    vAsvDspAStepDoneProc(u1DecId);

    if (!g_fgAVSyncSkipToend)
    {
        vAsvNotifyStepOK(u1DecId);
    }

}
// DSP B state
u8 u1AsvDspBInit(void)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_POWER_ON);
    if(u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    u1result = u1DspBDec2State(TR_DSP_B_POWER_ON);
    if(u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspBDec3State(TR_DSP_B_POWER_ON);
    if(u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    return(u1result);
}

u8 u1AsvDspBReady(void)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_INIT_READY);

    if (u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspBDec2State(TR_DSP_B_INIT_READY);
    if(u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    u1result = u1DspBDec3State(TR_DSP_B_INIT_READY);
    if(u1result == RTN_DSP_B_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    return (u1result);
}

u8 u1AsvDspBPlayCmd(u8 u1DecId)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_R_PLAY);

    if (u1result == RTN_DSP_B_FAIL)
    {
        LOG(LOG_FAIL, TEXT("DSPB u1AsvDspBPlayCmd DecId= 0x%x ;AUD State = 0x%x .\n"),
            u1DecId,u1DspBDec1GetState());
        AUD_VERIFY(u1result);
    }
    if (u1result == RTN_DSP_SUCCESS)
    {
        vDspCmd(DSP_PLAY + (u1DecId << 16));
    }

    return(u1result);
}

u8 u1AsvDspBStopCmd(u8 u1DecId)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_R_STOP);

    if (u1result == RTN_DSP_B_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspBStopCmd fail: DecId= 0x%x ;DSPB State = 0x%x \n"),
            u1DecId, u1DspBDec1GetState());
        AUD_VERIFY(u1result);
    }

    if (u1result == RTN_DSP_B_SUCCESS)
    {
        vDspCmd(DSP_STOP + (u1DecId << 16));    // stop primary decoder
        LOG(LOG_ADSP_INFO, TEXT("[u1AsvDspBStopCmd]Send DSP Command to Queue:_tDspCmd.\n"));

        if (u1DecId == PRI_DEC)
        {
            g_fgFirstAoutArrive = FALSE;
        }
    }
    return(u1result);
}

u8 u1AsvDspBSendCfg(u8 u1DecId)
{
    u8 u1result = 0;

    if (PRI_DEC == u1DecId)
    {
        u1result = u1DspBDec1State(TR_DSP_B_S_SEND_CFG);
    }
    else if (SEC_DEC == u1DecId)
    {
        u1result = u1DspBDec2State(TR_DSP_B_S_SEND_CFG);
    }
    else if (TER_DEC == u1DecId)
    {
        u1result = u1DspBDec3State(TR_DSP_B_S_SEND_CFG);
    }
    else
    {
        AUD_VERIFY(0);
    }

    if(u1result == RTN_DSP_B_FAIL)
    {
        u8 u1DspBState = 0;
        if(PRI_DEC == u1DecId)
        {
            u1DspBState = u1DspBDec1GetState();
        }
        else if(SEC_DEC == u1DecId)
        {
            u1DspBState = u1DspBDec2GetState();
        }
        else if(TER_DEC == u1DecId)
        {
            u1DspBState = u1DspBDec3GetState();
        }
        LOG(LOG_FAIL, TEXT("u1AsvDspBSendCfg fail, Dec ID =0x%x ,DSPB State = 0x%x \n"),
            u1DecId,u1DspBState);
    }

    return (u1result);
}

u8 u1AsvDspBReceiveCfg(bool fgCfgchanged)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_R_RECEIVE_CFG);

    LOG(LOG_DUALCTRL, TEXT("Enter u1AsvDspBReceiveCfg\n"));

    if(fgCfgchanged)
    {
        u16 u2FrnType  =0;
        u16 u2RearType =0;
        u16 u2MediaType = 0;
        DspGetAOutMediaType(AUD_AOUT1, &u2FrnType);
        DspGetAOutMediaType(AUD_AOUT2, &u2RearType);
        u2MediaType = u2ReadDspShmWORD(W_MEDIA_TYPE);
        //check aout media type
        if ((u2FrnType == AUD_OUT_MEDIA_USB) || (u2MediaType && 0x1))
        {
            //Aout1 Reinit
            LOG(LOG_DUALCTRL, TEXT("AOUT1 Reset\n"));
            u1AsvDspAAoutOff();
        }
        if ((u2RearType == AUD_OUT_MEDIA_USB) || (u2MediaType && 0x2))
        {
            //Aout2 Reinit
            LOG(LOG_DUALCTRL, TEXT("AOUT2 Reset\n"));
            u1AsvDspAAout2Off();
        }
    }
    else
    {  // If config doesn't  change, set Sfreq to DSPB
        LOG(LOG_DUALCTRL,"[AUD][u1AsvDspBReceiveCfg]set Sfreq to DSPB\n");
        vDspCmd(UOP_DSP_FS_ACK);
    }

    return(u1result);
}

u8 u1AsvDspBDecReady(u8 u1DecId)
{
    u8 u1DspAState;
    u8 u1Aout1State, u1Aout2State;
    u8 u1result = u1DspBDec1State(TR_DSP_B_S_DECODING_OK);

    if (u1result == RTN_DSP_B_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspBDecReady fail DecId= 0x%x ;DSPB State = 0x%x \n"),
            u1DecId, u1DspBDec1GetState());
        AUD_VERIFY(u1result);
    }

    vAsvDspDecReadyTrigger(u1DecId);

    u1DspAState = u1DspAGetState();
    if(u1DecId == PRI_DEC)
    {
        vAsvNotifyDecReady(u1DecId);
        x_event_set(g_hAudDecReady);
        LOG(LOG_CTRLF,TEXT("Set Decoder Ready Event \n"));

        u1Aout1State = u1DspAoutGetState();
        u1Aout2State = u1DspAout2GetState();

        if((u1DspAState != ST_DSP_A_CONNECTING) ||((ST_DSP_A_AOUT_ON != u1Aout1State) && (ST_DSP_A_AOUT_ON != u1Aout2State)))
        {
            LOG(LOG_FAIL, TEXT("u1DspAState = 0x%x. ASt1 = 0x%x. ASt2 = 0x%x. \r\n"),u1DspAState, u1Aout1State, u1Aout2State);
            LOG(LOG_FAIL, TEXT("Uplayer need connect DspA before play.\r\n"));
        }
    }

    return(u1result);
}

u8 u1AsvDspBDecStopped(u8 u1DecId)
{
    u8 u1result = u1DspBDec1State(TR_DSP_B_S_STOP_OK);

    if (u1result == RTN_DSP_B_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspBDecStopped fail, DecId= 0x%x ;AUD State  = 0x%x \n"),
            u1DecId, u1DspBDec1GetState());

        AUD_VERIFY(u1result);
    }
    u1result = u1AsvDspStopped(u1DecId);
    return(u1result);
}

u8 u1AsvDspIbcNotify(u8 ucDecId, s32 i4IbcId)
{
    AUD_InbandCmdNotify(ucDecId, i4IbcId);

    return(RTN_DSP_SUCCESS);
}

u8 u1AsvDspBFlushCmd(u8 u1DecId)
{
    if (u1DecId == PRI_DEC)
    {
        vDspCmd(DSP_FLUSH + (PRI_DEC<<16));
    }
    else
    {
        return(RTN_DSP_B_FAIL);
    }

    return(RTN_DSP_SUCCESS);
}

u8 u1AsvDspBFlushDone(u8 u1DecId)
{
    u8 u1result;
    bool fgDspBEosState = g_fgAudEosState;
    u32 u4IcbId = g_u4AudPriICBId;

    AUD_ASSERT(u1DecId == PRI_DEC);

    x_event_set(g_hAudFlushEvent);
    LOG(LOG_CTRLF, TEXT("Set Flush Done Event \n"));

    if (fgDspBEosState == TRUE)
    {
        u1result = u1AsvDspIbcNotify(u1DecId, u4IcbId);

        vAudDrvIf_DspEosNotify(u1DecId);

        if(u1result != RTN_DSP_SUCCESS)
        {
            AUD_VERIFY(u1result);
        }

        g_fgAudEosState = FALSE;
    }

    return(RTN_DSP_SUCCESS);
}

// Dsp Encoder state init.
u8 u1AsvDspReencInit(void)
{
    u8 u1result = u1DspReencState(TR_DSP_REENC_POWER_ON);

    if(u1result == RTN_DSP_REENC_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    return(u1result);
}

// Aud-DSP state
u8 u1AsvDspInit(void)
{
    // Primary state machine
    u8 u1result = u1AudDspState(TR_DSP_POWER_ON);
    if(u1result == RTN_DSP_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    return(u1result);
}

u8 u1AsvDspReady(void)
{
    u8 u1result = u1AudDspState(TR_DSP_INIT_READY);

    if (u1result == RTN_DSP_FAIL)
    {
        AUD_VERIFY(u1result);
    }
    return(u1result);
}
u8 u1AsvPlayCmd(u8 u1DecId)
{
    u8 u1result = u1AudDspState(TR_DSP_R_PLAY);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvPlayCmd, DecId= 0x%x ;AUD State = 0x%x .\n"),
            u1DecId,u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    u1result = u1AsvDspBPlayCmd(u1DecId);
    vDspUpdateMixVolumn(TRUE);

    return (u1result);
}

u8 u1AsvStopCmd(u8 u1DecId)
{
    u8 u1DspState;
    u8 u1result = u1AudDspState(TR_DSP_R_STOP);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvStopCmd fail, DecId= 0x%x ;AUD State = 0x%x .\n"),
            u1DecId,u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    // check DSP A is in step state or not
    u1DspState = u1DspAGetState();
    u1result = u1AsvDspAStepCancel(u1DecId);

    // check DSP B is in EOS state or not
    if (g_fgAudEosState == TRUE)
    {
        u1result = u1AsvDspBFlushDone(u1DecId);
    }

    if ((u1DspState == ST_DSP_A_CONNECTED) || (u1DspState == ST_DSP_A_CONNECTING))
    {
        u1result = u1AsvDspADisconnect(u1DecId);
    }
    else if (u1DspState == ST_DSP_A_DISCONNECTED)
    {
        u1result = u1AsvDspBStopCmd(u1DecId);
    }

    if (u1DecId == PRI_DEC)
    {
        //Set sample rate ready for 2nd audio on ARM2
        DspSetArm2FsReady(0);
    }

    vDspUpdateMixVolumn(FALSE);

    return(u1result);
}

u8 u1AsvPauseCmd(u8 u1DecId)
{
    u8 u1DspState;
    u8 u1result = u1AudDspState(TR_DSP_R_PAUSE);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvPauseCmd fail, DecId= 0x%x ;AUD State  = 0x%x \n"),
            u1DecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    u1DspState = u1DspAGetState();
    if (u1DspState == ST_DSP_A_CONNECTED)
    {
        u1result = u1AsvDspADisconnect(u1DecId);
    }
    else if (u1DspState == ST_DSP_A_CONNECTING)
    {
        u1result = u1AsvDspADisconnect(u1DecId);
    }
    else if (u1DspState == ST_DSP_A_DISCONNECTED)
    {
        u1result = u1AsvDspPaused(u1DecId);
    }

    return(u1result);
}


u8 u1AsvResumeCmd(u8 u1DecId)
{
    u8 u1DspState, u1DspBState;
    u8 u1result = u1AudDspState(TR_DSP_R_RESUME);
    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvResumeCmd fail; DecId= 0x%x ;AUD State  = 0x%x \n"),
            u1DecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    // check DSP A is in step state or not
    u1DspState = u1DspAGetState();

    u1result = u1AsvDspAStepCancel(u1DecId);

    u1DspState = u1DspAGetState();
    u1DspBState = u1DspBDec1GetState();

    // there is a racing condition when u1DspState== ST_DSP_A_DISCONNECTED now
    // but DSP send D2RC_STEP_CMD_OK  to RISC later, so ASV should accept ST_DSP_A_DISCONNECTED when connecting
    if((u1DspState == ST_DSP_A_DISCONNECTED)&&
       (u1DspBState != ST_DSP_B_POWER_OFF)&&
       (u1DspBState != ST_DSP_B_INIT)&&
       (u1DspBState != ST_DSP_B_READY)&&
       (u1DspBState != ST_DSP_B_STOPPING))
    {
        u1result = u1AsvDspAConnect(u1DecId);
        if ((u1DspBState == ST_DSP_B_DECODING)&&( u1DspState== ST_DSP_A_DISCONNECTED))
        {
            vAsvDspDecReadyTrigger(u1DecId);
        }
    }

    return(u1result);
}

u8 u1AsvFlushCmd(u8 u1DecId)
{
    u8 u1result = RTN_DSP_SUCCESS;
    u8 u1_AudState = u1AudDspGetState();

    if ((u1_AudState == ST_DSP_STOPPING) || (u1_AudState == ST_DSP_READY))
    {
        return RTN_DSP_FAIL;
    }

    u1result = u1AsvDspBFlushCmd(u1DecId);
    if(u1result == RTN_DSP_FAIL)
    {
        AUD_VERIFY(u1result);
    }

    return(u1result);
}

u8 u1AsvDspStopped(u8 u1DecId)
{
    u8 u1result = u1AudDspState(TR_DSP_S_STOP_OK);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspStopped fail; DecId= 0x%x ;AUD State  = 0x%x \n"),
            u1DecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }
    vAsvNotifyStopDone(u1DecId);

    return (u1result);
}

u8 u1AsvDspPlayed(u8 ucDecId)
{
    u8 u1result = u1AudDspState(TR_DSP_S_PLAY_OK);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspPlayed fail; DecId= 0x%x ;AUD State  = 0x%x \n"),
            ucDecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }
    AUD_RealPlayNotify(ucDecId, AUD_CMD_PLAY);
    return (u1result);
}

u8 u1AsvDspPaused(u8 u1DecId)
{
    u8 u1result = u1AudDspState(TR_DSP_S_PAUSE_OK);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspPaused fail; DecId= 0x%x; AUD State = 0x%x \n"),
            u1DecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    vAsvNotifyPauseDone(u1DecId);
    return(u1result);
}

u8 u1AsvDspResumed(u8 u1DecId)
{
    u8 u1result = u1AudDspState(TR_DSP_S_RESUME_OK);

    if (u1result == RTN_DSP_FAIL)
    {
        LOG(LOG_FAIL, TEXT("u1AsvDspResumed fail; DecId= 0x%x; AUD State  = 0x%x \n"),
            u1DecId, u1AudDspGetState());
        AUD_VERIFY(u1result);
    }

    vAsvNotifyResumeDone(u1DecId);
    return(u1result);
}

u8 u1AsvDsp_Hdcd_Trk_Stm_Chg(u8 u1DecId,bool isHdcdTrk)
{
    //u8 u1result = RTN_DSP_SUCCESS;

    AUD_HdcdTrkStmChg_Notify(u1DecId, isHdcdTrk);

    return (RTN_DSP_SUCCESS);
}

u8 u1AsvDsp_InputChCfg_Notify(u8 u1DecId, AUD_DRV_AUD_TYPE_T eAudChCfg)
{
    //u8 u1result = RTN_DSP_SUCCESS;

    AUD_Ch_Cfg_Notify(u1DecId, eAudChCfg);

    return (RTN_DSP_SUCCESS);
}

s32 i4AsvSendPlayCmd(s32 eDecId)
{
    if (SEC_DEC == eDecId)
    {
        vDspCmd(DSP_PLAY + (eDecId << 16));
    }
    else if (TER_DEC == eDecId)
    {
        vDspCmd(DSP_PLAY + (eDecId << 16));
    }

    return 0;
}

s32 i4AsvSendStopCmd(s32 eDecId)
{
    if (SEC_DEC == eDecId)
    {
        // 4th decoder stopped, disconnect DSP A/B now
        vDspCmd(DSP_STOP + (eDecId << 16));
    }
    else if (TER_DEC == eDecId)
    {
        // 4th decoder stopped, disconnect DSP A/B now
        vDspCmd(DSP_STOP + (eDecId << 16));
    }

    return 0;
}

s32 i4AsvSendDisconnectCmd(s32 u1DecId)
{
    if (SEC_DEC == u1DecId)
    {
        // 4th decoder stopped, disconnect DSP A/B now
        vDspCmd(UOP_DSP_FOURTH_MIXER_DISCONNECT);
    }
    else if(TER_DEC == u1DecId)
    {
        vDspCmd(UOP_DSP_FIFTH_MIXER_DISCONNECT);
    }
    return 0;
}
s32 i4AsvDspNotifyPlayCmdGot(s32 eDecId)
{
    if (SEC_DEC == eDecId)
    {
        vDspCmd(UOP_DSP_FOURTH_MIXER_CONNECT);
        AUD_AsvCommandDone(eDecId, AUD_CMD_PLAY);

    }
    else if (TER_DEC == eDecId)
    {
        vDspCmd(UOP_DSP_FIFTH_MIXER_CONNECT);
        AUD_AsvCommandDone(eDecId, AUD_CMD_PLAY);
    }

    return 0;
}

s32 i4AsvDspNotifyDecReady(s32 eDecId)
{
    if (SEC_DEC == eDecId)
    {
        LOG(LOG_CTRLF, TEXT("The 2nd decode ready\n"));
    }
    else if (TER_DEC == eDecId)
    {
        LOG(LOG_CTRLF, TEXT("The 3rd decode ready\n"));
    }

    return 0;
}


s32 i4AsvDspNotifyDecStopped(s32 eDecId)
{
    if (SEC_DEC == eDecId)
    {
        u8 u1Result;
        // stop command is done
        AUD_AsvCommandDone(eDecId, AUD_CMD_STOP);
        u1Result = u1DspBDec2State(TR_DSP_B_S_STOP_OK);
        if (u1Result == RTN_DSP_B_SUCCESS)
        {
            LOG(LOG_CTRLF, TEXT("[i4AsvDspNotifyDecStopped] Dec2 \r\n"));
        }
    }
    else if (TER_DEC == eDecId)
    {
        u8 u1Result;
        // stop command is done
        AUD_AsvCommandDone(eDecId, AUD_CMD_STOP);
        u1Result = u1DspBDec3State(TR_DSP_B_S_STOP_OK);
        if (u1Result == RTN_DSP_B_SUCCESS)
        {
            LOG(LOG_CTRLF, TEXT("[i4AsvDspNotifyDecStopped] Dec3 \r\n"));
        }
    }
    return 0;
}


#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
bool fgDspAWakeup(void)
{
    return _fgDspAWakeUpFlag;
}

bool fgDspBWakeup(void)
{
    return _fgDspBWakeUpFlag;
}

void vPowerDownDsp(void)
{
    _fgDspAWakeUpFlag = FALSE;
    _fgDspBWakeUpFlag = FALSE;
}
#endif

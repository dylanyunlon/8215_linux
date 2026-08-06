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
#include <media/atc/drv_aud.h>
#include "aud_debug.h"
#include "AsvTrigger.h"
#include "AsvDef.h"
#include "AsvState.h"
#include "AudDrvSubInc.h"
#include "AsvDspCtrl.h"
#include "DspFunc.h"
#include "aud_if.h"

void  vAsvPeriphDone(void)
{
    u1AsvDspAAoutOn();
}

bool fgAsvQueryAVD(void * rAudSrcCfg, void * rAudOutputCfg, void * rAudHdmiOutputCfg)
{
    bool fgSfChanged = fgAudDrvIf_RequestOutputCfg(rAudSrcCfg, rAudOutputCfg, rAudHdmiOutputCfg);
    return fgSfChanged;
}

bool fgAsvNotifyAVDChStatus(u8 * prLChStatus,u8 * prRChStatus)
{
    return fgAudDspIf_SetAVDChStatusPcmMode(prLChStatus,prRChStatus);
}


void vAsvSetAout1Periph(void)
{
    vAudDrvIf_SetAout1Periph();
}

void vAsvSetAout2Periph(void)
{
    vAudDrvIf_SetAout2Periph();
}

void vAsvNotifyDecReady(u8 u1DecId)
{
    vAudDrvIf_DecOutputReady(u1DecId);
}

void vAsvNotifyPlayCmdGot(u8 u1DecId)
{
    u8 u1DspState = 0;
    u8 u1result = RTN_DSP_A_SUCCESS;

    AUD_AsvCommandDone(u1DecId,AUD_CMD_PLAY);
    //Notify AV sync control that dsp gets  play cmd
    vAudDrvIf_DspGetPlayCmd(u1DecId);

    #if CONFIG_AUD_DECONLY_EN
    if((AUD_DECONLY_ON == DspGetDeconlyCtrl()) && (PRI_DEC == u1DecId))
    {
        return;
    }
    #endif

    // Connect DSP A, after notifying "got PLAY command" event
    u1DspState = u1AudDspGetState();
    if ((u1DspState== ST_DSP_PLAYING) || (u1DspState == ST_DSP_RESUMING))
    {
        u1result = u1AsvDspAConnect(u1DecId);
        if(u1result == RTN_DSP_A_FAIL)
        {
            AUD_VERIFY(u1result);
        }
    }

}

void vAsvNotifyStepOK(u8 u1DecId)
{
    vAudDrvIf_DspStepDone(u1DecId);
}

void vAsvNotifyBeginPtsDone(u8 u1DecId)
{
    vAudDrvIf_DspBeginPTSDone(u1DecId);
}

void vAsvNotifyPauseBeginPts(u8 u1DecId)
{
    vAudDrvIf_DspPauseBeginPTSDone(u1DecId);
}
void vAsvNotifyEndPtsDone(u8 u1DecId, bool fgNotifySyncctrl)
{
    vAudDrvIf_DspEndPTSDone(u1DecId, fgNotifySyncctrl);
}
void vAsvNotifyStopDone(u8 u1DecId)
{
    AUD_AsvCommandDone(u1DecId, AUD_CMD_STOP);
}

void vAsvNotifyPauseDone(u8 u1DecId)
{
    AUD_AsvCommandDone(u1DecId, AUD_CMD_PAUSE);
    vAudDrvIf_DspPauseDone(u1DecId);
}

void vAsvNotifyResumeDone(u8 u1DecId)
{
    AUD_AsvCommandDone(u1DecId, AUD_CMD_RESUME);
    vAudDrvIf_Resumed(u1DecId);
}

void vAsvNotifyReencStartDone(u8 u1DecId)
{
    AUD_AsvCommandDone(u1DecId,    AUD_CMD_PLAY);
}

void vAsvNotifyReencStopDone(u8 u1DecId)
{
    AUD_AsvCommandDone(u1DecId,    AUD_CMD_STOP);
}



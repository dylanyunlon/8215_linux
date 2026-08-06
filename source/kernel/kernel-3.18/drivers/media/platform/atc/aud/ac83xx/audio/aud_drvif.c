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
/*-----------------------------------------------------------------------------
Include header files
-----------------------------------------------------------------------------*/
#include "x_lint.h"
#include <linux/types.h>
#include "x_assert.h"

#include "aud_oal.h"
#include "aud_drv_config.h"
#include "AsvAudDrv.h"
#include "DspDrvInc.h"
#include "DspFunc.h"

#include "drv_if_syncctrl.h"
#include "aud_drv.h"
#include "aud_config.h"
#include "aud_if.h"

#include "aud_esm.h"
#include "aud_io_clock_if.h"


#include <media/atc/mm_common.h>

/*-----------------------------------------------------------------------------
Data declarations
-----------------------------------------------------------------------------*/
typedef struct _AUD_SYNC_CONTEXT_T
{
    u16 u2Type;
    u16 u2PktFltr_Comp_Id;

    u16 u2SyncCtrlId;
    void *pvSyncCtrlTag;
    ISyncCtrl *piSyncCtrl;
    u64 u8FirstAudPts;
    u64 u8LastAudPts;
    u64 u8PausedPTS;
    u64 u8BeginPTS;
    u64 u8EndPTS;
    bool fgDspReadyForPlayCmd;
    bool fgDspBeginPtsCmdFlag;
    bool fgDspEndPtsCmdFlag;
    bool fgDspEosFlag;
    bool fgDspAConnectStatus; // DSP A after connecting: True ; stop reset: False
    u64 u8DspEndPTSDone;
}   AUD_SYNC_CONTEXT_T;

static AUD_SYNC_CONTEXT_T _rAudSyncContext[PRI_DEC+1];

bool g_fgFirstAoutArrive = FALSE;
AUDIO_SAMPLING_T g_eDvdSamplRate = FS_UNKNOWN;

#define AUD_GET_SYNC_PIPELINE(u) ((u) == 0 ? SYNCCTRL_PIPELINE_MAIN : SYNCCTRL_PIPELINE_SUB)

extern bool g_fgEnableSkipData;
extern SPDIF_RAW_SET_INFO_T g_rSpdifRawInfo;

/****************************************************************************
** Function prototypes
****************************************************************************/
void vAudDrvIf_StcValid(u64 u8FirstPTS, u8 u1DecId);

/****************************************************************************
*    Function : vAudSyncCtrlInfoInit
* Description :
*   Parameter :
*   Return    :
*   Note     :
****************************************************************************/
void vAudSyncCtrlInfoInit(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = NULL;
    // current only Primary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext = &_rAudSyncContext[u1DecId];
    // Use this flag to know the Dsp Ready corresponds to play command
    // (Since "Aout on" and "DspB Ready" condition may not caued by play command, caused by bitstream sampling rate change)
    pContext->fgDspReadyForPlayCmd=TRUE;
    pContext->u8LastAudPts = (0xFFFFFFFFFFFFFFFFLL);
    pContext->fgDspEosFlag=FALSE;
    pContext->fgDspAConnectStatus=FALSE;

    LOG(5, TEXT("vAudSyncCtrlInfoInit, u1DecId = 0x%X\n"), u1DecId);
}

void vAudUpdateLastPts(u64 u8Pts,u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext;

    // current only Primary and Secondary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext = &_rAudSyncContext[u1DecId];
    pContext->u8LastAudPts = u8Pts;
}

/****************************************************************************
*    Function : vAudNotifyAVSyncDspReady
* Description : Notify AVSync module that DSP get the play command
*   Parameter : Decoder ID/The PTS value to Sync Ctrl
*   Return    :
*   Note     :
****************************************************************************/
void vAudNotifyAVSyncDspReady(u8 u1DecId, u64 u8PtsToSyncCtrl )
{
    AUD_SYNC_CONTEXT_T *pContext;

    // current only Primary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext = &_rAudSyncContext[u1DecId];
    if(pContext->piSyncCtrl )
    {
        if ((pContext->piSyncCtrl->pi4AudioReadyTo))
        {
            s32 i4Ret = AUD_OK;

            i4Ret = (pContext->piSyncCtrl->pi4AudioReadyTo)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId), SYNCCTRL_CMD_PLAY,u8PtsToSyncCtrl);
            LOG(5, TEXT("Notify AVSyncCtrl audio is ready,ptsH = 0x%X , ptsL = 0x%X i4Ret = 0x%X\n"),
				                   (u32)((u8PtsToSyncCtrl&0xFFFFFFFF00000000LL)>>32),(u32)(u8PtsToSyncCtrl&0xFFFFFFFF),i4Ret);
            if( i4Ret < 0 ) VERIFY(0);
        }
    }
}


/****************************************************************************
*    Function : vAudNotifyAVSyncStopDone
* Description :
*   Parameter :
*   Return    :
*   Note     :
****************************************************************************/
void vAudNotifyAVSyncStopDone(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext;

    //current only Primary and Secondary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext = &_rAudSyncContext[u1DecId];

    if (pContext->piSyncCtrl)
    {
        if ((pContext->piSyncCtrl->pi4AudioStopped))
        {
            (pContext->piSyncCtrl->pi4AudioStopped)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId));
            LOG(5, TEXT("vAudNotifyAVSyncStopDone/pi4AudioStopped, u1DecId = 0x%X\n"), u1DecId);
        }
    }
}


/****************************************************************************
*    Function : vAudNotifyAVSyncPlayCmd
* Description : Notify AVSync module that DSP get the play command
*   Parameter :
*   Return    :
*   Note     :
****************************************************************************/
void vAudNotifyAVSyncPauseDone(u8 u1DecId)
{
    u64 u8Pts = 0xFFFFFFFFFFFFFFFFLL;
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

    // current only Primary and Secondary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext->u8PausedPTS = i8DspGetDspPts(u1DecId);

    if(pContext->piSyncCtrl)
    {
        if(g_fgFirstAoutArrive && u1DecId == PRI_DEC)
        {
            u8Pts = pContext->u8PausedPTS;
        }

        if ((pContext->piSyncCtrl->pi4AudioPaused))
        {
            (pContext->piSyncCtrl->pi4AudioPaused)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId), u8Pts);
            LOG(5, TEXT("vAudNotifyAVSyncPauseDone/pi4AudioPaused, u1DecId = 0x%X\n"), u1DecId);
        }
    }
}

/****************************************************************************
*    Function : vAudSaveFirstAudPts
* Description :
*   Parameter :
*   Return    :
*   Note     :
****************************************************************************/
void vAudSaveFirstAudPts(u32 u4FirstPts, u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    if (u1DecId == PRI_DEC)
    {
        pContext->u8FirstAudPts=(u64)u4FirstPts<<1;
    }
}


void vAudResetFirstAudPts(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext->u8FirstAudPts=(u64)0;
}

/****************************************************************************
*    Function : vAudDrvIf_DecOutputReady
* Description : Notify Sync Ctrl that dsp output ready
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DecOutputReady(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    // current only Primary and Secondary
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    // Notify Sync control that dsp is ready to play
    if(pContext->fgDspReadyForPlayCmd)
    {
#if (SYNC_FRAMEACCURATE)
        if(pContext->fgDspBeginPtsCmdFlag == FALSE)
        {
#endif
            vAudNotifyAVSyncDspReady(u1DecId, pContext->u8FirstAudPts);
#if (SYNC_FRAMEACCURATE)
        }
#endif
        pContext->fgDspReadyForPlayCmd=FALSE;
    }
}

/****************************************************************************
*    Function : vAudDrvIf_Resumed
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_Resumed(u8 u1DecId)
{
    u8 u1DspBState;
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    // (current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext->u8PausedPTS = i8DspGetDspPts(u1DecId);
    // Notify Sync control that dsp is ready to play
    // hjwei
    u1DspBState = u1DspBDec1GetState();
    if (u1DspBState == ST_DSP_B_DECODING)
    {
        vAudNotifyAVSyncDspReady(u1DecId, pContext->u8PausedPTS);
    }
}


/****************************************************************************
*    Function : vAudDrvIf_DspStepDone
* Description : Notify AVSync that decoder finish step command
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspStepDone(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext->u8PausedPTS = i8DspGetDspPts(u1DecId);

    if(pContext->piSyncCtrl )
    {
        if ((pContext->piSyncCtrl->pi4AudioSkipDoneNotify))
        {
            (pContext->piSyncCtrl->pi4AudioSkipDoneNotify)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId), pContext->u8PausedPTS);
            LOG(6, TEXT("vAudDrvIf_DspStepDone/pi4AudioSkipDoneNotify, u1DecId = 0x%X\n"), u1DecId);
        }
    }
}


void vAudDrvIf_DspEosNotify(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    // For step EOS case
    // set audio last PTS to unreachable PTS.
    if(g_fgEnableSkipData==TRUE )
    {
        u64 u8PrePST = 0;

        u8PrePST = _rAudSyncContext[u1DecId].u8LastAudPts;
        //_rAudSyncContext[u1DecId].u8LastAudPts = u8PrePST + 9000;

        LOG(5, TEXT("[AUD][Step] Adjust audio last PTS from %lld to %lld\n"), u8PrePST, _rAudSyncContext[u1DecId].u8LastAudPts);
    }

    if(pContext->piSyncCtrl )
    {
        if ((pContext->piSyncCtrl->pi4AudioEOSNotify))
        {
            (pContext->piSyncCtrl->pi4AudioEOSNotify)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId));
            LOG(5, TEXT("vAudDrvIf_DspEosNotify/pi4AudioEOSNotify, u1DecId = 0x%X\n"), u1DecId);
        }
    }

    LOG(5, TEXT("[AUD][Step] Notify AVSyncCtrl EOS during stepping\n"));
}

/****************************************************************************
*    Function : vAudDrvIf_DspPauseBeginPTSDone
* Description : Notify AVSync that decoder finish Begin PTS command
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspPauseBeginPTSDone(u8 u1DecId)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    pContext->fgDspBeginPtsCmdFlag = FALSE;
}

/****************************************************************************
*    Function : vAudDrvIf_DspBeginPTSDone
* Description : Notify AVSync that decoder finish Begin PTS command
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspBeginPTSDone(u8 u1DecId)
{
    /*
    2009-6-10 KuiWang remove  "|| SYNC_DSP_SKIPBEGINPTS_NOTIFY"
    cause: for linux clean build warning
    description: SYNC_DSP_SKIPBEGINPTS_NOTIFY is only for AVSync early development, and its definition can't be found.
    */
#if (SYNC_FRAMEACCURATE /*|| SYNC_DSP_SKIPBEGINPTS_NOTIFY*/)
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }
#endif

#if (SYNC_FRAMEACCURATE)
    //if(pContext->fgDspBeginPtsCmdFlag == FALSE)
    //{
    //    // If no Begin Pts Cmd but driver get Begin Pts Cmd done notify
    //    VERIFY(0);
    // }
    pContext->fgDspBeginPtsCmdFlag = FALSE;
    //Notify sync ctrl DSP ready
    vAudDrvIf_StcValid(0, 0);

#endif

}


/****************************************************************************
*    Function : vAudDrvIf_DspEndPTSDone
* Description : Notify AVSync that decoder finish Begin PTS command
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspEndPTSDone(u8 u1DecId, bool fgNotifySyncctrl)
{
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];
    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    if(pContext->fgDspEndPtsCmdFlag == FALSE)
    {
        // If no End Pts Cmd but driver get Begin Pts Cmd done notify
        LOG(5, TEXT("NO End Pts Cmd(maybe canceled) but aud driver get Begin Pts Cmd done notify from dsp\n"));
        //VERIFY(0);
    }
    pContext->fgDspEndPtsCmdFlag = FALSE;

    if (fgNotifySyncctrl)
    {
        pContext->u8DspEndPTSDone = i8DspGetDspEndPts(u1DecId);

#if (SYNC_FRAMEACCURATE)
        if(pContext->piSyncCtrl )
        {
            if ((pContext->piSyncCtrl->pi4AudioEndPtsDoneNotify))
            {
                (pContext->piSyncCtrl->pi4AudioEndPtsDoneNotify)(pContext->pvSyncCtrlTag, AUD_GET_SYNC_PIPELINE(u1DecId), pContext->u8DspEndPTSDone);
                LOG(5, TEXT("vAudDrvIf_DspEndPTSDone/pi4AudioEndPtsDoneNotify, u1DecId = 0x%X\n"), u1DecId);
            }
        }
#endif
    }
}


//==============================================================================

// Provided API for sync ctrl


/****************************************************************************
*    Function : vAudDrvIf_SetBeginPts
* Description : Sync ctrl set begin PTS to dsp
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_SetBeginPts(u64 u8BeginPTS, u8 u1DecId)
{
    u16 u2UopIndex[2];
#if (SYNC_FRAMEACCURATE)
    //DECODER_STATE_T eDecState;
    u16 u2CancelUopIndex[2];
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

    if (u1DecId >PRI_DEC)
    {
        VERIFY(0);
    }

    LOG(5, TEXT("Sync Ctrl (vAudDrvIf_)SetBeginPts to audio driver,Begin PtsH = 0x%X , PtsL = 0x%X , u1DecId = 0x%X\n"),
		(u32)((u8BeginPTS&0xFFFFFFFF00000000LL)>>32),(u32)(u8BeginPTS&0xFFFFFFFF), u1DecId);

    u2CancelUopIndex[PRI_DEC] = UOP_DSP_MIXER_CANCEL_BEGIN_PTS;

    if(u8BeginPTS == (u64)INVALID_TIMESTAMP)
    {
        // make sure that dsp have got Begin pts command
        if(pContext->fgDspBeginPtsCmdFlag == FALSE)
        {
            //VERIFY(0);
            return;   // According to Victor's suggest
        }
        pContext->fgDspBeginPtsCmdFlag=FALSE;
        //Begin PTS = invalid : cancel begin pts command
        vDspCmd(u2CancelUopIndex[u1DecId]);
        LOG(5, TEXT("vAudDrvIf_SetBeginPts send CANCEL_BEGIN_PTS UOP to DSP, u1DecId = 0x%X\n"), u1DecId);

    }
    else
    {

        pContext->u8BeginPTS=u8BeginPTS;
#endif

        u2UopIndex[PRI_DEC] = UOP_DSP_MIXER_BEGIN_PTS;

        vDspSyncCtrlSetFirstTargetPts((u32)(u8BeginPTS>>1),u1DecId);

        pContext->fgDspBeginPtsCmdFlag=TRUE;
        vDspCmd(u2UopIndex[u1DecId]);
        LOG(5, TEXT("vAudDrvIf_SetBeginPts send BEGIN_PTS UOP to DSP, u1DecId = 0x%X\n"), u1DecId);

#if (SYNC_FRAMEACCURATE)
    }
#endif

}

/****************************************************************************
*    Function : vAudDrvIf_SetEndPts
* Description : Sync ctrl set end PTS to dsp
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_SetEndPts(u64 u8EndPTS, u8 u1DecId)
{
    u16 u2CancelUopIndex[PRI_DEC+1];
    u16 u2UopIndex[PRI_DEC+1];
    AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }

    u2CancelUopIndex[PRI_DEC] = UOP_DSP_MIXER_END_PTS_OFF;

    LOG(5, TEXT("Sync Ctrl (vAudDrvIf_)SetEndPts,End PtsH = 0x%X , PtsL = 0x%X ,u1DecId = 0x%X\n"),
		(u32)((u8EndPTS&0xFFFFFFFF00000000LL)>>32),(u32)(u8EndPTS&0xFFFFFFFF), u1DecId);

    if(u8EndPTS == (u64)INVALID_TIMESTAMP)
    {
        // make sure that dsp have got END pts command
        if(pContext->fgDspEndPtsCmdFlag == FALSE)
        {
            //VERIFY(0);
            return; // According to Victor's suggest
        }
        pContext->fgDspEndPtsCmdFlag=FALSE;
        //Begin PTS = invalid : cancel begin pts command
        vDspCmd(u2CancelUopIndex[u1DecId]);
        LOG(5, TEXT("vAudDrvIf_SetEndPts send MIXER_END_PTS_OFF to DSP, u1DecId = 0x%X\n"), u1DecId);

    }
    else
    {

        if(pContext->fgDspEndPtsCmdFlag == TRUE)
        {
            LOG(5, TEXT("vAudDrvIf receive 2 END_PTS_CMD, u1DecId = 0x%X\n"), u1DecId);
            VERIFY(0);
        }


        pContext->u8EndPTS=u8EndPTS;

        u2UopIndex[PRI_DEC] = UOP_DSP_MIXER_END_PTS;

        vDspSyncCtrlSetEndPts((u32)(u8EndPTS>>1),u1DecId);
        vDspSetEosPts((u32)(u8EndPTS>>1),u1DecId);

        pContext->fgDspEndPtsCmdFlag=TRUE;
        vDspCmd(u2UopIndex[u1DecId]);
        LOG(5, TEXT("vAudDrvIf_SetEndPts send MIXER_END_PTS to DSP, u1DecId = 0x%X\n"), u1DecId);


    }
}

/****************************************************************************
*    Function : vAudDrvIf_StcValid
* Description : AVSync informs Dsp that STC has been reset for AV Re-Sync
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_StcValid(u64 u8FirstPTS, u8 u1DecId)
{
    // VERIFY if decoder ID(u4DecId) > syncctrl interface(current only Primary and Secondary)
    if (u1DecId > PRI_DEC)
    {
        VERIFY(0);
    }
    LOG(5, TEXT("Sync Ctrl set (vAudDrvIf_)StcValid ,Target PtsH = 0x%X , PtsL = 0x%X, u1DecId = 0x%X\n"),
	                    (u32)((u8FirstPTS&0xFFFFFFFF00000000LL)>>32),(u32)(u8FirstPTS&0xFFFFFFFF),u1DecId);
    vDspSyncCtrlSetFirstTargetPts((u32)(u8FirstPTS>>1),u1DecId);

    vSendADSPCmd(UOP_DSP_MIXER_AVSYNC_START);
    LOG(5, TEXT("vAudDrvIf_StcValid send UOP_DSP_MIXER_AVSYNC_START\n"));
}


/****************************************************************************
*    Function : vAudDrvIf_DspStopDone
* Description : Notify AVSync that decoder has stopped
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspStopDone(u8 u1DecId)
{
    vAudNotifyAVSyncStopDone(u1DecId);
}

/****************************************************************************
*    Function : vAudDrvIf_DspPauseDone
* Description : Notify AVSync that decoder has Paused
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspPauseDone(u8 u1DecId)
{
    vAudNotifyAVSyncPauseDone(u1DecId);
}

/****************************************************************************
*    Function : vAudDrvIf_DspGetPlayCmd
* Description : Notify AVSync that decoder has stopped
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_DspGetPlayCmd(u8 u1DecId)
{
    //vAudNotifyAVSyncPlayCmd(u1DecId);
}

/****************************************************************************
*    Function : vAudDrvIf_SetNoSync
* Description : AVSync informs Dsp that STC has been reset for AV Re-Sync
*   Parameter :
*   Return    :
****************************************************************************/
void vAudDrvIf_SetNoSync(void)
{
    vSendADSPCmd(UOP_DSP_MIXER_AVSYNC_BYPASS);
}

/* Interface for avsync for wince by mtk40292*/
void vAudDrvIf_DisableAVSync(u8 u1DecId)
{
    if (PRI_DEC == u1DecId)
    {
        vSendADSPCmd(UOP_DSP_MIXER_AVSYNC_BYPASS);
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("This Dec  = 0x%x do not need avsync \n"),u1DecId);
    }
}

void vAudDrvIf_SetTargetPTS(u8 u1DecId,u64 u8FirstPTS)
{
    LOG(LOG_CTRLF, TEXT("Sync Ctrl set Target PtsH = 0x%X, PtsL = 0x%X \n"),
		(u32)((u8FirstPTS&0xFFFFFFFF00000000LL)>>32),(u32)(u8FirstPTS&0xFFFFFFFF));

    vDspSyncCtrlSetFirstTargetPts((u32)(u8FirstPTS>>1),u1DecId);
    vSendADSPCmd(UOP_DSP_MIXER_AVSYNC_START);
    LOG(LOG_CTRLF, TEXT("vAudDrvIf_StcValid send UOP_DSP_MIXER_AVSYNC_START\n"));
}

void vAudDrvIf_GetCurrentPTS(u8 u1DecId,u64* u8FirstPTS)
{
    if (PRI_DEC == u1DecId)
    {
        AUD_SYNC_CONTEXT_T *pContext = &_rAudSyncContext[u1DecId];

        (*u8FirstPTS) = pContext->u8FirstAudPts;

        LOG(LOG_CTRLF, TEXT("Sync Ctrl Get First PtsH = 0x%x, PtsL = 0x%x\n"),
		(u32)(((pContext->u8FirstAudPts)&0xFFFFFFFF00000000LL)>>32),(u32)((pContext->u8FirstAudPts)&0xFFFFFFFF));
    }
}

void vAudDrvIf_GetLatestPTS(u8 u1DecId,u32* u4PTSHi,u32* u4PTSLo)
{
    if (PRI_DEC == u1DecId)
    {
        DspGetUpdatePtsValue(u1DecId, u4PTSHi, u4PTSLo);
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("This Dec id is error = 0x%x \n"),u1DecId);
    }   
}

// bit0 -- 0: Aout1 -> Aout1
//         1: Aout1 -> Aout2
// bit1 -- 0: Aout2 -> Aout1
//         1: Aout2 -> Aout2
void vAudDrvIf_SwitchAout(u32 dwParam)
{
    //AudCfg_SwitchAout(dwParam);
}

//==============================================================================


// Provided API for AVD
/****************************************************************************
*    Function : fgAudDrvIf_RequestOutputCfg
* Description : Call AVD interface to get output decision
*   Parameter :
*   Return    :
*   Note     : DTS-HD may pass more bitstream info to AVD in the future to get correct decoding decision.
****************************************************************************/
bool fgAudDrvIf_RequestOutputCfg(AUD_SOURCE_CFG_T   *prSrcParam, AUD_OUTPUT_SETTING_CFG_T *prOutParam,
                                 AUD_OUTPUT_SETTING_CFG_T   *prHdmiOutParam)
{
    bool fgSfChanged = AudCfg_ChgOutCfg(prSrcParam, prOutParam, prHdmiOutParam);
    return fgSfChanged;
}


/****************************************************************************
*    Function : fgAudDspIf_SetAVDChStatusPcmMode
* Description : Set channel status to avd when output pcm mode
*   Parameter :
*   Return    :
****************************************************************************/
bool fgAudDspIf_SetAVDChStatusPcmMode(u8 * prLChStatus,u8 * prRChStatus)
{
    // Removed by MTK40043
    //return fgAudDrvSetHdmiChStatus(prLChStatus,prRChStatus);
    return (TRUE);
}

#if CONFIG_DRV_SPDIF_RAW_SUPPORT

void vAudDrvIf_SetDvdClk(void)
{    
    AUDIO_SAMPLING_T u1DvdSamplRateCur = FS_UNKNOWN;
    if((g_rSpdifRawInfo.u1DvdSampRate == FS_44K) && g_rSpdifRawInfo.fgDvdIsRawOut)
    {
        u1DvdSamplRateCur = FS_44K;
    }
    else
    {
        u1DvdSamplRateCur = FS_48K;
    }

    if(u1DvdSamplRateCur != g_eDvdSamplRate)
    {
        IoClk_SetDvdAoutClk(AUD_MCLK_256FS, u1DvdSamplRateCur);
        g_eDvdSamplRate = u1DvdSamplRateCur;
    }
}

void vAudDrvIf_SetIecClk(AUDIO_SAMPLING_T u1IecSamplRateCur)
{
    static AUDIO_SAMPLING_T u1IecSamplRatePrev = FS_UNKNOWN;
    if(u1IecSamplRateCur != u1IecSamplRatePrev)    
    {        
        //IoClk_SetIecClk(AUD_MCLK_128FS, u1IecSamplRateCur);  ////remove, set with aout1 together 
        u1IecSamplRatePrev = u1IecSamplRateCur;
    }
    else
    {        
        LOG(LOG_DATAF, TEXT("[IEC]SamplRate is same as prev.\n"));
    }
}

void vAudDrvIf_SetAout1Clk(void)
{
    AUDIO_SAMPLING_T u1Aout1SamplRateCur = FS_UNKNOWN;
    if((AUD_OUT_MEDIA_DVD == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE)) &&
       (g_rSpdifRawInfo.u1DvdSampRate == FS_44K) &&
        g_rSpdifRawInfo.fgDvdIsRawOut)
    {
        u1Aout1SamplRateCur = FS_44K;
    }
    else if((AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE)) &&
            (g_rSpdifRawInfo.u1UsbSampRate == FS_44K) &&
             g_rSpdifRawInfo.fgUsbIsRawOut)
    {
        u1Aout1SamplRateCur = FS_44K;
    }
    else
    {
        u1Aout1SamplRateCur = FS_48K;
    }     
    AudAout_SampleSet(u1Aout1SamplRateCur, AUD_CLK_AOUT1);    
    //vAudDrvIf_SetIecClk(u1Aout1SamplRateCur);  //remove, set with aout1 together 
}

void vAudDrvIf_SetAout2Clk(void)
{
    AUDIO_SAMPLING_T u1Aout2SamplRateCur = FS_UNKNOWN;
    if((AUD_OUT_MEDIA_DVD == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE)) &&
       (g_rSpdifRawInfo.u1DvdSampRate == FS_44K) &&
        g_rSpdifRawInfo.fgDvdIsRawOut)
    {
        u1Aout2SamplRateCur = FS_44K;
    }
    else if((AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE)) &&
            (g_rSpdifRawInfo.u1UsbSampRate == FS_44K) &&
             g_rSpdifRawInfo.fgUsbIsRawOut)
    {
        u1Aout2SamplRateCur = FS_44K;
    }
    else
    {
        u1Aout2SamplRateCur = FS_48K;
    }
    AudAout_SampleSet(u1Aout2SamplRateCur, AUD_CLK_AOUT2);
}
#endif

void vAudDrvIf_SetAout1Periph(void)
{    
#if CONFIG_DRV_SPDIF_RAW_SUPPORT
    LOG(LOG_DATAF, TEXT("[aud_drvif.c] vAudDrvIf_SetAout1Periph \n"));
    if(AUD_OUT_MEDIA_DVD == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE))
    {
        vAudDrvIf_SetDvdClk();
    }        
    vAudDrvIf_SetAout1Clk();
    
#else
    LOG(LOG_DATAF, TEXT("[aud_drvif.c] vAudDrvIf_SetAout1Periph \n"));
    AudAout_SampleSet(FS_48K, AUD_CLK_AOUT1);
#endif    
}

void vAudDrvIf_SetAout2Periph(void)
{
#if CONFIG_DRV_SPDIF_RAW_SUPPORT
    LOG(LOG_DATAF, TEXT("[aud_drvif.c] vAudDrvIf_SetAout2Periph \n"));
    if(AUD_OUT_MEDIA_DVD == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE))
    {
        vAudDrvIf_SetDvdClk();
    }        
    vAudDrvIf_SetAout2Clk();
#else
    LOG(LOG_DATAF, TEXT("[aud_drvif.c] vAudDrvIf_SetAout2Periph \n"));
    AudAout_SampleSet(FS_48K, AUD_CLK_AOUT2);
#endif
}


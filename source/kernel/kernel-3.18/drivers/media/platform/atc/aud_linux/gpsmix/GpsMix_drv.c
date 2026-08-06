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



#include "aud_oal.h"
#include "GpsMix_drvthread.h"
#include "GpsMix_if.h"
#include "GpsMix_AsvTrigger.h"
#include "aud_debug.h"
#include "aud_config.h"

/******************************************************************************
                                                         variable define
******************************************************************************/
AUD_GPS_MIX_DEC_INFO _GpsMixDecInfo;

static struct task_struct *g_hAudGpsMixDrvThread;
static u32 _AudGpsMixCmdQueue;
static struct semaphore _GpsMixSemaUopComDone;

static bool _fgAudGpsMixDrvInitiated = FALSE;
volatile static bool _fgAudDrvGpsMixThreadInit = FALSE;
static  AUD_GPS_MIX_DRV_STATE_T _AudGpsMixDrvState = AUD_GPS_MIX_DRV_UNINITIALIZED;

static s8 *_pGpsMixAudCmd[4] =
{
   TEXT("AUD_GPS_MIX_CMD_START"),
   TEXT("AUD_GPS_MIX_CMD_CMD_STOP"),
   TEXT("AUD_GPS_MIX_CMD_PAUSE"),
   TEXT("AUD_GPS_MIX_CMD_RESUME")
};
/******************************************************************************
                                                      function declare
******************************************************************************/
static void _GpsMixDrvThreadInit(void);
static s32 _AudGpsMixDrvThread(void* pvArg);

/******************************************************************************
                                                      function 
******************************************************************************/
void AudGpsMix_DrvInit(void)
{
    if(!_fgAudGpsMixDrvInitiated)
    {
        _GpsMixDrvThreadInit();  
    }
    _fgAudGpsMixDrvInitiated =TRUE;
}


static void AudGpsMixDrvThread_Uninit(void)
{
   vAudGpsMix_SemaphoreDelete();
   vAudGpsMixMsgqDelete();
}

void vAudGpsMix_SemaphoreDelete()
{
    if(_fgAudGpsMixDrvInitiated == FALSE)
    {
        //VERIFY(x_sema_delete(_GpsMixSemaUopComDone) == OSR_OK);
    }
}

void vAudGpsMixMsgqDelete(void)
{
     if(_fgAudGpsMixDrvInitiated == FALSE)
     {
          VERIFY(x_msg_q_delete(_AudGpsMixCmdQueue) == OSR_OK);
     }
}

static void _GpsMixDrvThreadInit(void)
{
    s8 s_wname[20];
    s8 s_name[20];

    if (!_fgAudDrvGpsMixThreadInit)
    {
        _fgAudDrvGpsMixThreadInit = TRUE;
       
        x_snwprintf(s_wname, sizeof(s_wname), TEXT("%s"), ADRV_GPSMIX_CMD_Q_NAME);
        VERIFY((x_msg_q_create(&_AudGpsMixCmdQueue, s_wname, sizeof(u32), AUD_GPSMIX_CMD_QUEUE_SIZE)) == OSR_OK);
        Aud_snprintf(s_name, sizeof(s_name), "%s", AUD_DRV_GPS_MIX_THREAD_NAME);
        
        g_hAudGpsMixDrvThread = kthread_create(_AudGpsMixDrvThread, (void *)NULL, "s_name");
    	if (IS_ERR(g_hAudGpsMixDrvThread)) {
    		LOG(LOG_CTRLF, TEXT("[_GpsMixDrvThreadInit]_AudGpsMixDrvThread create fail \r\n"));
    		g_hAudGpsMixDrvThread = NULL;
    		return;
    	}
        else
        {
            struct sched_param param;
            s32 ret;

            param.sched_priority = to_sched_priority(AUD_DRV_GPS_MIX_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAudGpsMixDrvThread, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
    	wake_up_process(g_hAudGpsMixDrvThread);
        
        sema_init(&_GpsMixSemaUopComDone, 0);

        m_hGpsMixStartEvent       =  x_event_create(NULL, FALSE, FALSE, TEXT("GPSMIXSTART"));
        m_hGpsMixStopEvent        =  x_event_create(NULL, FALSE, FALSE, TEXT("GPSMIXSTOP"));
        m_hGpsMixPauseEvent       =  x_event_create(NULL, FALSE, FALSE, TEXT("GPSMIXPAUSE"));
        m_hGpsMixResumeEvent      =  x_event_create(NULL, FALSE, FALSE, TEXT("GPSMIXRESUME"));
        m_hGpsMixConsumeDataEvent =  x_event_create(NULL, FALSE, FALSE, TEXT("GPSMIXCONSUMEDATA"));
    }
}


void AudGpsMix_AsvCommandDone(u32 u4Command)
{
    LOG(LOG_FEATURE, TEXT("*****Gps Mix AsvCommand %s done\r\n"), _pGpsMixAudCmd[u4Command]);
    up(&_GpsMixSemaUopComDone);
}

void AudGpsMix_WaitAsvCommandDone(u32 u4Command)
{
    LOG(LOG_FEATURE, TEXT("*****Gps Wait Mix AsvCommand %s done\r\n"), _pGpsMixAudCmd[u4Command]);
    down(&_GpsMixSemaUopComDone);
}

bool AudGpsMix_DrvCmd(AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd)
{
    u32 u4Msg = (u32)eGpsMixCmd;
    VERIFY((x_msg_q_send(_AudGpsMixCmdQueue, &u4Msg, sizeof(u32), 1)) == OSR_OK);
    return TRUE;
}

static void _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_STATE_T eNewGpsMixState)
{
    if (_AudGpsMixDrvState == eNewGpsMixState)
    {
        return ;
    }
    else
    {
        _AudGpsMixDrvState = eNewGpsMixState;
        LOG(LOG_FEATURE, TEXT("change gps mix audio state to %d\r\n"), eNewGpsMixState);
    }
}

void AudGpsMix_CommandDoneNotify(AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd)
{
    if (eGpsMixCmd == AUD_GPS_MIX_CMD_START) 
    {
        LOG(LOG_FEATURE, TEXT("[GPSMIX]Enter AUD_GPS_MIX_CMD_START\r\n"));
        x_event_set(m_hGpsMixStartEvent);
    }
    else if(eGpsMixCmd == AUD_GPS_MIX_CMD_STOP)
    {
        LOG(LOG_FEATURE, TEXT("[GPSMIX]Enter StopEvent\r\n"));
        x_event_set(m_hGpsMixStopEvent);
    }

    else if(eGpsMixCmd == AUD_GPS_MIX_CMD_PAUSE)
    {
        LOG(LOG_FEATURE, TEXT("[GPSMIX]Enter PauseEvent\r\n"));
        x_event_set(m_hGpsMixPauseEvent);
    }
    else if(eGpsMixCmd == AUD_GPS_MIX_CMD_RESUME)
    {
        LOG(LOG_FEATURE, TEXT("[GPSMIX]Enter ResumeEvent\r\n"));
        x_event_set(m_hGpsMixResumeEvent);
    }
    else
    {
        LOG(LOG_FEATURE, TEXT("[GPSMIX]Enter NoEvent\r\n"));
    }

    LOG(LOG_FEATURE, TEXT("[GPSMIX]AUD_GPS_MIX_CMD = %s is done.\n"),_pGpsMixAudCmd[eGpsMixCmd]);
}

static void _AudGpsMixSetEvent(u32 u4Event)
{
    _GpsMixDecInfo.u4GpsMixEventFlag = u4Event;
}

static void _AudGpsMixWaitEvent(u32 * pu4Event)
{
    u32 u4Event;
    u16 u2MsgIdx;
    u32 zMsgSize = sizeof(u32);
    AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd;
    s32 i4CmdOk;

    do
    {
        VERIFY(x_msg_q_receive(&u2MsgIdx, &u4Event, &zMsgSize,
                               &_AudGpsMixCmdQueue, 1, X_MSGQ_OPTION_WAIT) == OSR_OK);

        eGpsMixCmd = (AUD_DRV_GPS_MIX_CMD_T)u4Event;
        
        if(((eGpsMixCmd == AUD_GPS_MIX_CMD_START) && (_GpsMixDecInfo.eGpsMixDecState == AUD_GPS_MIX_DEC_STARTING)) ||
           ((eGpsMixCmd == AUD_GPS_MIX_CMD_STOP) && (_GpsMixDecInfo.eGpsMixDecState == AUD_GPS_MIX_DEC_STOP)))
        {
            if (eGpsMixCmd == AUD_GPS_MIX_CMD_START)
            {
                AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_START);
            }
            else if (eGpsMixCmd == AUD_GPS_MIX_CMD_STOP)
            {
                AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_STOP);
            }
            i4CmdOk = FALSE;
        }
        else
        {
            /* Check command and decoder event flag */
            if ((1 << (u8)eGpsMixCmd) & _GpsMixDecInfo.u4GpsMixEventFlag)
            {
                i4CmdOk = TRUE;
            }
            else
            {
                i4CmdOk = FALSE;
            }
        }
    } while (!i4CmdOk);

    if (pu4Event != NULL)
    {
        *pu4Event = u4Event;
    }
}

static s32 _AudGpsMixDrvThread(void* pvArg)
{
    u32 u4Event;
    AUD_GPS_MIX_DRV_STATE_T tGpsMixState = AUD_GPS_MIX_DRV_STOPPING;

    _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_OPENING);
    while (_fgAudDrvGpsMixThreadInit)
    {
        switch (_AudGpsMixDrvState)
        {
        case AUD_GPS_MIX_DRV_OPENING:
            _GpsMixDecInfo.eGpsMixDecState= AUD_GPS_MIX_DEC_INIT;
            _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_STOPPED);
            break;

        case AUD_GPS_MIX_DRV_STOPPED:
            _GpsMixDecInfo.eGpsMixDecState = AUD_GPS_MIX_DEC_STOP;
            _AudGpsMixSetEvent(AUD_GPS_MIX_CMD_FLAG_START |
                               AUD_GPS_MIX_CMD_FLAG_STOP);
            _AudGpsMixWaitEvent(&u4Event);
            switch (u4Event)
            {
            case AUD_GPS_MIX_CMD_START:
                tGpsMixState = AUD_GPS_MIX_DRV_STARTING;
                break;

            case AUD_GPS_MIX_CMD_STOP:
                tGpsMixState = AUD_GPS_MIX_DRV_STOPPED;
                break;

            default:
                LOG(LOG_FEATURE, TEXT("NO This Gps Mix Cmd\r\n"));
                VERIFY(0);
            }
            _ChangeGpsMixAudioState(tGpsMixState);
            break;

        case AUD_GPS_MIX_DRV_STARTED:
            _AudGpsMixSetEvent(AUD_GPS_MIX_CMD_FLAG_START |
                               AUD_GPS_MIX_CMD_FLAG_STOP  |
                               AUD_GPS_MIX_CMD_FLAG_PAUSE);
            _AudGpsMixWaitEvent(&u4Event);

            switch (u4Event)
            {
            case AUD_GPS_MIX_CMD_START:
                tGpsMixState = AUD_GPS_MIX_DRV_STARTED;
                break;

            case AUD_GPS_MIX_CMD_STOP:
                tGpsMixState = AUD_GPS_MIX_DRV_STOPPING;
                break;
                        
            case AUD_GPS_MIX_CMD_PAUSE:
                tGpsMixState = AUD_GPS_MIX_DRV_PAUSING;
                break;
                            
            default:
                LOG(LOG_FEATURE, TEXT("NO This Gps Mix Cmd\n"));
                VERIFY(0);
            }
            _ChangeGpsMixAudioState(tGpsMixState);
            break;
                    
        case AUD_GPS_MIX_DRV_PAUSED:
            _GpsMixDecInfo.eGpsMixDecState = AUD_GPS_MIX_DEC_PAUSED;

            _AudGpsMixSetEvent(AUD_GPS_MIX_CMD_FLAG_RESUME|
                               AUD_GPS_MIX_CMD_FLAG_STOP);

            _AudGpsMixWaitEvent(&u4Event);
            switch (u4Event)
            {
            case AUD_GPS_MIX_CMD_RESUME:
                tGpsMixState = AUD_GPS_MIX_DRV_RESUMING;
                break;

            case AUD_GPS_MIX_CMD_STOP:
                tGpsMixState = AUD_GPS_MIX_DRV_STOPPING;
                break;

            default:
                LOG(LOG_FEATURE, TEXT("NO This Gps Mix Cmd\n"));
                VERIFY(0);
            }
            _ChangeGpsMixAudioState(tGpsMixState);
            break;

        case AUD_GPS_MIX_DRV_PAUSING:
            _GpsMixDecInfo.eGpsMixDecState = AUD_GPS_MIX_DEC_PAUSING;
            VERIFY(u1AsvGpsMixPauseCmd());
            AudGpsMix_WaitAsvCommandDone(AUD_GPS_MIX_CMD_PAUSE);
            LOG(LOG_FEATURE, TEXT("*****Gps Mix Pause Command Done\n"));
            _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_PAUSED);
            AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_PAUSE);
            break;

        case AUD_GPS_MIX_DRV_STARTING:
            _GpsMixDecInfo.eGpsMixDecState  = AUD_GPS_MIX_DEC_STARTING;
            AudCfg_RestoreAoutRegs();
            VERIFY(u1AsvGpsMixStartCmd());
            AudGpsMix_WaitAsvCommandDone(AUD_GPS_MIX_CMD_START);
            LOG(LOG_FEATURE, TEXT("*****Gps Mix Start Command Done\n"));
            _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_STARTED);
            AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_START);
            break;
                
        case AUD_GPS_MIX_DRV_STOPPING:
            _GpsMixDecInfo.eGpsMixDecState  = AUD_GPS_MIX_DEC_STOP;
            VERIFY(u1AsvGpsMixStopCmd());
            AudGpsMix_WaitAsvCommandDone(AUD_GPS_MIX_CMD_STOP);
            LOG(LOG_FEATURE, TEXT("*****Gps Mix Stop Command Done\n"));
            _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_STOPPED);
            AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_STOP);
            break;
                
        case AUD_GPS_MIX_DRV_RESUMING:
            _GpsMixDecInfo.eGpsMixDecState  = AUD_GPS_MIX_DRV_RESUMING;
            VERIFY(u1AsvGpsMixResumeCmd());
            AudGpsMix_WaitAsvCommandDone(AUD_GPS_MIX_CMD_RESUME);
            LOG(LOG_FEATURE, TEXT("*****Gps Mix Stop Command Done\n"));
            _ChangeGpsMixAudioState(AUD_GPS_MIX_DRV_STARTED);       
            AudGpsMix_CommandDoneNotify(AUD_GPS_MIX_CMD_RESUME);
            break;
        default:
            break;
        }
    }
    AudGpsMixDrvThread_Uninit();
    complete_and_exit(NULL, 0);

	return 0;
}





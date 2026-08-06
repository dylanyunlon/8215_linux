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

#include "x_lint.h"
#include <linux/types.h>
#include "aud_oal.h"
#include "aud_debug.h"
#include "drv_thread.h"
#include "drv_dsp_cfg.h"
#include "DspDrvInc.h"
#include "aud_drv.h"
#include "aud_if.h"
#include "AsvAudDrv.h"
#include "DspTask.h"
#include "DspFunc.h"
#include "AsvDspCtrl.h"

#include "aud_drv_config.h"
#include <media/atc/aud_output.h>

#include "aud_power.h"
#include "audin_if.h"
#include "aud_esm.h"
#include "aud_raw.h"

#if CONFIG_AUD_ADSP_ERR_RECOVER_EN
#include "DspErrProc.h"
#endif

#include "aud_smix.h"
/****************************************************************************
** Local definitions
****************************************************************************/
#define ADRV_CMD_Q_NAME   TEXT("AUD CMDQ")
#define AUD_CMD_QUEUE_SIZE 32
#define AUD_THREAD_NAME  ("AudDrv")

/****************************************************************************
** Function prototypes
****************************************************************************/
#if 0
static void AudDrvThreadUninit(void);
#endif
static void AudDrvChgState(u8 u1DecId, AUD_DRV_STATE_T eNewState);
static s32 AudDrvDec1Thd(void* pvArg);
static s32 AudDrvDec2Thd(void* pvArg);
static s32 AudDrvDec3Thd(void* pvArg);
static void AudDrvSetEvent(u8 u1DecId, u32 u4Event);
static void AudDrvWaitEvent(u8 u1DecId, u32 * pu4Event);

/****************************************************************************
** Local structures and variable
****************************************************************************/
volatile static bool g_fgAudDrvThreadInit = FALSE;
static bool g_fgAudDrvInited = FALSE;

static struct semaphore g_hSemaAoutReset;
static struct semaphore g_hSemaAout2Reset;
static struct semaphore g_hSemaLoadCode;
static struct semaphore g_hSemaUopComDone[MAX_AUDDRV_NUM];

static bool g_fgAoutResetLocked = FALSE;
static bool g_fgAout2ResetLocked = FALSE;

static struct task_struct *g_hAudDrvThread;
static struct task_struct *g_hAud2DrvThread;
static struct task_struct *g_hAud3DrvThread;

static uintptr_t g_hAudCmdQueue[MAX_AUDDRV_NUM] = {0};

AUD_DECODER_T g_rAudDecoder[MAX_AUDDRV_NUM];

AUD_DRV_CONTEXT g_rAudResouceManger[MAX_AUDDRV_NUM];

#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT    
AUDIO_POWER_MANAGEMNT_DEVICEID_CONTROL_UNION_T  g_rDevPowerCtrl; 
#endif


static AUD_ENCODER_T g_rAudEncoder = {0};
AUD_DRV_STATE_T g_rAudDrvState[MAX_AUDDRV_NUM] = {0};

volatile bool g_fgStopping = FALSE;

static s8 *g_ptcAudCmd[7] =
{
    TEXT("AUD_CMD_PLAY"),
    TEXT("AUD_CMD_STOP"),
    TEXT("AUD_CMD_RESET"),
    TEXT("AUD_CMD_PAUSE"),
    TEXT("AUD_CMD_AVSYNC"),
    TEXT("AUD_CMD_LOADCODE"),
    TEXT("AUD_CMD_RESUME")
};

static s8 *g_ptcAudState[AUD_DRV_STATE_CNT] =
{
    _T("UNINITIALIZED"),
    _T("TRIGGER_ADSP"),
    _T("OPENING"),
    _T("STOPPING"),
    _T("STOPPED"),
    _T("PLAYING"),
    _T("PLAYED"),
    _T("PAUSING"),
    _T("PAUSED"),
    _T("RESUMING"),
    _T("PRESTARTING"),
    _T("RISC_INIT"),    //used only on EMULATION_FPGA
    _T("RISC_POWERDOWN"),
};


/****************************************************************************
** Local functions
****************************************************************************/
static void AudDrvSetEvent(u8 u1DecId, u32 u4Event)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    g_rAudDecoder[u1DecId].u4EventFlag = u4Event;
}

/******************************************************************************
* Function      : AudDrvWaitEvent
* Description  : Polling for audio service command queue until a specified cmd received in a specified state.
* Parameter   :
* Return        : ADSP read pointer of the decoder
******************************************************************************/
static void AudDrvWaitEvent(u8 u1DecId, u32 * pu4Event)
{
    u32 u4Event;
    u16 u2MsgIdx;
    u32 zMsgSize = sizeof(u32);
    AUD_DRV_CMD_T eCmd;
    s32 i4CmdOk;

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));

    do
    {
        VERIFY(x_msg_q_receive(&u2MsgIdx, &u4Event, &zMsgSize, &g_hAudCmdQueue[u1DecId], 1, X_MSGQ_OPTION_WAIT) == OSR_OK);

        eCmd = (AUD_DRV_CMD_T)u4Event;
        /* Check if decoder state is the same as command */
        if (((eCmd == AUD_CMD_PLAY) && (g_rAudDecoder[u1DecId].eDecState == AUD_DEC_PLAYING)) ||
                ((eCmd == AUD_CMD_STOP) && (g_rAudDecoder[u1DecId].eDecState == AUD_DEC_STOP)))
        {
            if (eCmd == AUD_CMD_PLAY)
            {
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);
            }
            else if (eCmd == AUD_CMD_STOP)
            {
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
            }
            i4CmdOk = FALSE;
        }
        else
        {
            /* Check command and decoder event flag */
            if ((1 << (u8)eCmd) & g_rAudDecoder[u1DecId].u4EventFlag)
            {
                i4CmdOk = TRUE;
            }
            else
            {
                i4CmdOk = FALSE;
            }
        }
    }
    while (!i4CmdOk);

    if (pu4Event != NULL)
    {
        *pu4Event = u4Event;
    }
}

#if 0 //mark for warning cancel
void AudDrvSemaDelete(void)
{
    u8 u1DecId;

    if (FALSE == g_fgAudDrvThreadInit)
    {
        //VERIFY(x_sema_delete(g_hSemaAoutReset) == OSR_OK);
        //VERIFY(x_sema_delete(g_hSemaAout2Reset) == OSR_OK);
        //VERIFY(x_sema_delete(g_hSemaLoadCode) == OSR_OK);
        for (u1DecId=PRI_DEC; u1DecId<MAX_AUDDRV_NUM; u1DecId++)
        {
            //VERIFY(x_sema_delete(g_hSemaUopComDone[u1DecId]) == OSR_OK);
        }
    }
}

static void AudDrvMsgqDelete(void)
{
    if (FALSE == g_fgAudDrvThreadInit)
    {
        VERIFY(x_msg_q_delete(g_hAudCmdQueue[PRI_DEC]) == OSR_OK);
        VERIFY(x_msg_q_delete(g_hAudCmdQueue[SEC_DEC]) == OSR_OK);
        VERIFY(x_msg_q_delete(g_hAudCmdQueue[TER_DEC]) == OSR_OK);
    }
}
#endif

void AudDrvThreadExit(void)
{
    u8 u1DecId = PRI_DEC;
    g_fgAudDrvThreadInit = FALSE;

    for (; u1DecId <= RE_ENC; u1DecId++)
    {
        switch (g_rAudDrvState[u1DecId])
        {
        case AUD_DRV_PLAYING:
            AUD_AsvCommandDone(u1DecId, AUD_CMD_PLAY);
            break;
        case AUD_DRV_STOPPING:
            AUD_AsvCommandDone(u1DecId, AUD_CMD_STOP);
            break;
        case AUD_DRV_PAUSING:
            AUD_AsvCommandDone(u1DecId, AUD_CMD_PAUSE);
            break;
        case AUD_DRV_RESUMING:
            AUD_AsvCommandDone(u1DecId, AUD_CMD_RESUME);
            break;
        case AUD_DRV_PAUSED:
        case AUD_DRV_PLAYED:
        case AUD_DRV_STOPPED:
            AUD_VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_RESET));
            break;
        default:
            break;

        }
    }
}

void AudDrvThreadInit(void)
{
    s8 s_wname[20];
    s8 s_name[20];
    if (!g_fgAudDrvThreadInit)
    {
        u8 u1DecId;
        g_fgAudDrvThreadInit = TRUE;

        sema_init(&g_hSemaAoutReset, 1);
        sema_init(&g_hSemaAout2Reset, 1);
        sema_init(&g_hSemaLoadCode, 1);
        //Create a Semaphore, a Message Queue, and a Thread for each audio playback instance
        for (u1DecId=PRI_DEC; u1DecId<MAX_AUDDRV_NUM; u1DecId++)
        {
            //Init control flow acknowledge semaphores with LOCKED
            sema_init(&g_hSemaUopComDone[u1DecId], 0);
        }

        for (u1DecId = PRI_DEC; u1DecId <= TER_DEC; u1DecId++)
        {
            x_snwprintf(s_wname, sizeof(s_wname), TEXT("%s%d"), ADRV_CMD_Q_NAME, u1DecId);
            VERIFY((x_msg_q_create(&g_hAudCmdQueue[u1DecId], s_wname, sizeof(u32), AUD_CMD_QUEUE_SIZE)) == OSR_OK);
        }

        // create primary audio thread
        Aud_snprintf(s_name,sizeof(s_name), "%s%d", AUD_THREAD_NAME, PRI_DEC);
        g_hAudDrvThread = kthread_create(AudDrvDec1Thd, (void *)NULL, s_name);
	    if (IS_ERR(g_hAudDrvThread)) {
    		LOG(LOG_FAIL, TEXT("[AudDrvThreadInit]AudDrvDec1Thd create fail \r\n"));
    		g_hAudDrvThread = NULL;
    		return;
	    }
        else
        {
            struct sched_param param;
            s32 ret;

            param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAudDrvThread, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
	    wake_up_process(g_hAudDrvThread);

        Aud_snprintf(s_name, sizeof(s_name), "%s%d", AUD_THREAD_NAME, SEC_DEC);
        g_hAud2DrvThread = kthread_create(AudDrvDec2Thd, (void *)NULL, s_name);
	    if (IS_ERR(g_hAud2DrvThread)) {
    		LOG(LOG_FAIL, TEXT("[AudDrvThreadInit]AudDrvDec2Thd create fail \r\n"));
    		g_hAud2DrvThread = NULL;
    		return;
	    }
        else
        {
            struct sched_param param;
            s32 ret;

            param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAud2DrvThread, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
	    wake_up_process(g_hAud2DrvThread);
        
        //create five audio thread
        Aud_snprintf(s_name, sizeof(s_name), "%s%d", AUD_THREAD_NAME, TER_DEC);
        g_hAud3DrvThread = kthread_create(AudDrvDec3Thd, (void *)NULL, s_name);
	    if (IS_ERR(g_hAud3DrvThread)) {
    		LOG(LOG_FAIL, TEXT("[AudDrvThreadInit]AudDrvDec3Thd create fail \r\n"));
    		g_hAud3DrvThread = NULL;
    		return;
	    }
        else
        {
            struct sched_param param;
            s32 ret;

            param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
            ret = sched_setscheduler_nocheck(g_hAud3DrvThread, SCHED_RR, &param);
            ASSERT(ret == 0);
        }
	    wake_up_process(g_hAud3DrvThread);
        
        VERIFY(i4AudEsm_TaskCreate()==AUD_OK);

    }
}

#if 0 //mark for warning cancel
static void AudDrvThreadUninit(void)
{
    AudDrvSemaDelete();
    AudDrvMsgqDelete();
}
#endif

static void AudDrvChgState(u8 u1DecId, AUD_DRV_STATE_T eNewState)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    if (g_rAudDrvState[u1DecId] != eNewState)
    {
        g_rAudDrvState[u1DecId] = eNewState;
        LOG(LOG_FEATURE, TEXT("Dec[%d] Change AUD Drv State to %s.\n"), 
            u1DecId, g_ptcAudState[eNewState]);
    }
    
}

AUD_DRV_STATE_T AudDrvGetState(u8 u1DecId)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    return g_rAudDrvState[u1DecId];
}

static void AudDrvWaitDelayOnStop(void)
{
    u32 u4MaxDelayMs = DspGetDelayValue();

    if (u4MaxDelayMs > 0)
    {
        mdelay(u4MaxDelayMs);
        LOG(LOG_FEATURE, TEXT("[AUD] wait %dms for audio delay before notify stopped\n"), 
            u4MaxDelayMs);
    }
}

// *********************************************************************
// Function : void AudDrvDec1Thd(void)
// Description : Primary audio state machine
// Parameter : none
// Return    : None
// *********************************************************************
static s32 AudDrvDec1Thd(void* pvArg)
{
    u8 u1DecId = PRI_DEC;
    AUD_DRV_STATE_T eNewState = AUD_DRV_STOPPING;
    u32 u4Event;

    if (pvArg != NULL)
    {
        u1DecId = *(u8 *)pvArg;
    }

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    AudDrvChgState(u1DecId, AUD_DRV_OPENING);

    while (g_fgAudDrvThreadInit == TRUE)
    {
        switch (g_rAudDrvState[u1DecId])
        {
        case AUD_DRV_PLAYING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_PLAYING;
            if (g_rAudDecoder[u1DecId].eDecFormat == AUD_DRV_FMT_TRUE_HD)
            {
                vDspSetPRSAfifoAddr();
            }

            // Send play command
            AUD_VERIFY(u1AsvPlayCmd(u1DecId));

            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_PLAY);
            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Play Command Done\n"),u1DecId);
            AudDrvChgState(u1DecId, AUD_DRV_PLAYED);
            i4AudEsm_Notify_Play(u1DecId);

            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(u1DecId))
            {
                LOG(LOG_CTRLF, TEXT("errrecover:PLAYING->STOPPED.\n"));
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            #endif

            /* Release Play command API waiting semaphore */
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);
            break;

        case AUD_DRV_STOPPING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;

            while (g_fgStopping)
            {
                LOG(LOG_CTRLF, TEXT("DEC[%d]Wait for Other Stop Done.\n"), u1DecId);
                Sleep(2000);
            }
            g_fgStopping = TRUE;
            AUD_VERIFY(u1AsvStopCmd(u1DecId));
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_STOP);
            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Stop Command Done\n"),u1DecId);
            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            i4AudEsm_Notify_Stop(u1DecId);

            // wait for Fade Out is done before notify audio stopped
            // when audio delay > 100ms
            AudDrvWaitDelayOnStop();

            //add by fei for xiaolu test

            AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
            // Reset Sync Ctrl info
            vAudSyncCtrlInfoInit(u1DecId);
            //Notify AV Sync control that dsp stops
            vAudDrvIf_DspStopDone(u1DecId);
            g_fgStopping = FALSE;
            break;

        case AUD_DRV_PAUSING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_PAUSING;
            AUD_VERIFY(u1AsvPauseCmd(u1DecId));
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_PAUSE);
            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Pause Command Done\n"),u1DecId);
            AudDrvChgState(u1DecId, AUD_DRV_PAUSED);

            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(u1DecId))
            {
                LOG(LOG_CTRLF, TEXT("errrecover:PAUSING->STOPPED.\n"));
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            #endif
            
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_PAUSE);
            break;

        case AUD_DRV_RESUMING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_RESUMING;
            AUD_VERIFY(u1AsvResumeCmd(u1DecId));
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_RESUME);
            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Resume Command Done\n"),u1DecId);
            AudDrvChgState(u1DecId, AUD_DRV_PLAYED);

            #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
            if (u4AdspErrProcNotifyFlagGet(u1DecId))
            {
                LOG(LOG_CTRLF, TEXT("errrecover:RESUMING->STOPPED.\n"));
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            #endif
            
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_RESUME);
            break;

        case AUD_DRV_PAUSED:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_PAUSED;
            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_RESUME | AUD_CMD_FLAG_STOP);
            AudDrvWaitEvent(u1DecId, &u4Event);

            switch (u4Event)
            {
            case AUD_CMD_STOP:
                eNewState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_RESUME:
                eNewState = AUD_DRV_RESUMING;
                break;
            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eNewState);
            break;

        case AUD_DRV_PLAYED: 
            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY   |
                         AUD_CMD_FLAG_STOP   |
                         AUD_CMD_FLAG_PAUSE  |
                         AUD_CMD_FLAG_RESET  |
                         AUD_CMD_FLAG_ERRORRECOVER);
            AudDrvWaitEvent(u1DecId, &u4Event);

            switch (u4Event)
            {
            case AUD_CMD_PLAY:
                eNewState = AUD_DRV_PLAYED;
                break;
            case AUD_CMD_STOP:
                eNewState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_PAUSE:
                eNewState = AUD_DRV_PAUSING;
                break;
            case AUD_CMD_RESET:
                eNewState = AUD_DRV_STOPPING;
                break;
             case AUD_CMD_ERRORRECOVER:
                eNewState = AUD_DRV_STOPPED;
                break;
            default:
                AUD_VERIFY(0);
            }
            
            AudDrvChgState(u1DecId, eNewState);
            break;

        case AUD_DRV_STOPPED:   
          #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))
           AudDev_PowerDown(AUD_DEVICE_ID_PRIMARY);
          #endif
            
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;
          
            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY   |
                                    AUD_CMD_FLAG_STOP   |
                                    AUD_CMD_FLAG_RESET  |
                                    AUD_CMD_FLAG_LOADCODE);
            
            AudDrvWaitEvent(u1DecId, &u4Event);
                        
           #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))
            if(u4Event == AUD_CMD_PLAY)
            {
                AudDev_PowerOn(AUD_DEVICE_ID_PRIMARY);
            }
           #endif
           
            switch (u4Event)
            {
            case AUD_CMD_PLAY:
                eNewState = AUD_DRV_PLAYING;
                break;
            case AUD_CMD_STOP:
                eNewState = AUD_DRV_STOPPED;
                break;
            case AUD_CMD_RESET:
                eNewState = AUD_DRV_STOPPED;
                break;
            case AUD_CMD_LOADCODE:
                eNewState = AUD_DRV_PRESTARTING;
                break;
            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eNewState);
            break;

        case AUD_DRV_OPENING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;
            if (fgASVCheckInit())
            {
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            else
            {
                mdelay(1);
            }
            // Reset Sync Ctrl info
            vAudSyncCtrlInfoInit(u1DecId);
            break;

        case AUD_DRV_PRESTARTING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;
            down(&g_hSemaLoadCode);
            DspLoadAdspCode(u1DecId, g_rAudDecoder[u1DecId].eDecFormat);
            LOG(LOG_CTRLF, TEXT("[AUD]Aud decoder[%d] Load [%d] code \n"),u1DecId,g_rAudDecoder[u1DecId].eDecFormat);
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_LOADCODE);
            up(&g_hSemaLoadCode);

            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            break;

        default:
            LOG(LOG_FAIL, TEXT("Dec1 u1DecId:%d, AudDrvState[u1DecId]:%d\n"),
                u1DecId,g_rAudDrvState[u1DecId]);
            AUD_VERIFY(0);
        } // end of switch
    } //end of while (TRUE)

    complete_and_exit(NULL, 0);

	return 0;
}

// *********************************************************************
// Function : void AudDrvDec2Thd(void)
// Description : fourth aud drv thread
// Parameter : none
// Return    : None
// Author    : fei.zhu
// Time      : 2011.1.5
// *********************************************************************
static s32 AudDrvDec2Thd(void * pvArg)
{
    u8 u1DecId = (u8)SEC_DEC;
    AUD_DRV_STATE_T eFourthState = AUD_DRV_STOPPING;
    u32 u4Event;

    if (pvArg != NULL)
    {
        u1DecId = *(u8 *)pvArg;
    }

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));

    AudDrvChgState(u1DecId, AUD_DRV_OPENING);

    while (g_fgAudDrvThreadInit == TRUE)
    {
        switch (g_rAudDrvState[u1DecId])
        {
        case AUD_DRV_PLAYING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_PLAYING;
            // Send play command            
            LOG(LOG_CTRLF, TEXT("DEC2 AUD_DRV_PLAYING\n "));
            i4AsvSendPlayCmd(u1DecId);
            
            //wait asv cmd done
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_PLAY);

            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Play Command Done\n"),u1DecId);
            AudDrvChgState(u1DecId, AUD_DRV_PLAYED);
            i4AudEsm_Notify_Play(u1DecId);

            /* Release Play command API waiting semaphore */
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);            
            LOG(LOG_CTRLF, TEXT("DEC2 AUD_DRV_PLAYED\n "));
            break;

        case AUD_DRV_STOPPING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;
            //send stop cmd
            while (g_fgStopping)
            {
                LOG(LOG_CTRLF, TEXT("DEC[%d]Wait for Other Stop Done.\n"), u1DecId);
                Sleep(2000);
            }
            g_fgStopping = TRUE;

            LOG(LOG_CTRLF, TEXT("DEC2 Disconnecting...\n "));
            i4AsvSendDisconnectCmd(u1DecId);

            //wait asv cmd done
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_STOP);

            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Stop Command Done\n"),u1DecId);

            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);

            i4AudEsm_Notify_Stop(u1DecId);

            // wait for Fade Out is done before notify audio stopped
            // when audio delay > 100ms
            AudDrvWaitDelayOnStop();

            AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
            g_fgStopping = FALSE;
            LOG(LOG_CTRLF, TEXT("DEC2 AUD_DRV_STOPPED\n "));
            break;


        case AUD_DRV_PLAYED:
            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY   |
                         AUD_CMD_FLAG_STOP   |
                         AUD_CMD_FLAG_RESET);
            AudDrvWaitEvent(u1DecId, &u4Event);

            switch (u4Event)
            {
            case AUD_CMD_STOP:
                eFourthState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_RESET:
                eFourthState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_PLAY:
                eFourthState = AUD_DRV_PLAYED;
		AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);
                break;

            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eFourthState);
            break;

        case AUD_DRV_STOPPED:
           #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))           
            AudDev_PowerDown(AUD_DEVICE_ID_FOUR);
           #endif
           
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;

            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY   |
                                    AUD_CMD_FLAG_STOP   |
                                    AUD_CMD_FLAG_RESET  |
                                    AUD_CMD_FLAG_LOADCODE);
            
            AudDrvWaitEvent(u1DecId, &u4Event);
           
          #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))
           if(u4Event == AUD_CMD_PLAY)
           {
                AudDev_PowerOn(AUD_DEVICE_ID_FOUR);
           }
           #endif
        
            switch (u4Event)
            {
            case AUD_CMD_PLAY:
                eFourthState = AUD_DRV_PLAYING;
                break;
            case AUD_CMD_STOP:
                eFourthState = AUD_DRV_STOPPED;
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
                break;
            case AUD_CMD_RESET:
                eFourthState = AUD_DRV_STOPPED;
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
                break;
            case AUD_CMD_LOADCODE:
                eFourthState = AUD_DRV_PRESTARTING;
                break;
            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eFourthState);
            break;

        case AUD_DRV_OPENING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;

            if (fgASVCheckInit())
            {
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            else
            {
                mdelay(1);
            }
            break;

        case AUD_DRV_PRESTARTING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;
            down(&g_hSemaLoadCode);
            DspLoadAdspCode(u1DecId, g_rAudDecoder[u1DecId].eDecFormat);
            LOG(LOG_FEATURE, TEXT("[AUD]Aud decoder[%d] Load [%d] code \n"),
                u1DecId,g_rAudDecoder[u1DecId].eDecFormat);
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_LOADCODE);
            up(&g_hSemaLoadCode);
            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            break;

        default:
            LOG(LOG_FAIL, TEXT("Dec2 State: u1DecId:%d, AudDrvState[u1DecId]:%d\n"),
                u1DecId,g_rAudDrvState[u1DecId]);
            AUD_VERIFY(0);

        }
    }
    complete_and_exit(NULL, 0);
}

static s32 AudDrvDec3Thd(void * pvArg)
{
    u8 u1DecId = (u8)TER_DEC;
    AUD_DRV_STATE_T eState = AUD_DRV_STOPPING;
    u32 u4Event = 0;

    if (pvArg != NULL)
    {
        u1DecId = *(u8 *)pvArg;
    }

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));

    AudDrvChgState(u1DecId, AUD_DRV_OPENING);

    while (g_fgAudDrvThreadInit == TRUE)
    {
        switch (g_rAudDrvState[u1DecId])
        {
        case AUD_DRV_PLAYING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_PLAYING;
            // Send play command            
            LOG(LOG_CTRLF, TEXT("DEC3 AUD_DRV_PLAYING\n "));
            i4AsvSendPlayCmd(u1DecId);
            
            //wait asv cmd done
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_PLAY);

            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Play Command Done\n"),u1DecId);

            AudDrvChgState(u1DecId, AUD_DRV_PLAYED);

            i4AudEsm_Notify_Play(u1DecId);

            /* Release Play command API waiting semaphore */
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);            
            LOG(LOG_CTRLF, TEXT("DEC3 AUD_DRV_PLAYED\n "));
            break;

        case AUD_DRV_STOPPING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;
            //send stop cmd  
            LOG(LOG_CTRLF, TEXT("DEC3 Disconnecting...\n "));
            i4AsvSendDisconnectCmd(u1DecId);

            //wait asv cmd done
            AUD_WaitAsvCommandDone(u1DecId, AUD_CMD_STOP);

            LOG(LOG_FEATURE, TEXT("Aud decoder[%d] Stop Command Done\n"),u1DecId);

            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);

            i4AudEsm_Notify_Stop(u1DecId);

            // wait for Fade Out is done before notify audio stopped
            // when audio delay > 100ms
            AudDrvWaitDelayOnStop();

            AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
            LOG(LOG_CTRLF, TEXT("DEC3 AUD_DRV_STOPPED\n "));
            break;


        case AUD_DRV_PLAYED:
            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY |
                         AUD_CMD_FLAG_STOP   |
                         AUD_CMD_FLAG_RESET);
            AudDrvWaitEvent(u1DecId, &u4Event);

            switch (u4Event)
            {
            case AUD_CMD_STOP:
                eState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_RESET:
                eState = AUD_DRV_STOPPING;
                break;
            case AUD_CMD_PLAY:
                eState = AUD_DRV_PLAYED;
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_PLAY);
                break;

            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eState);
            break;

        case AUD_DRV_STOPPED:
           #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))            
            AudDev_PowerDown(AUD_DEVICE_ID_FOUR);
           #endif
           
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_STOP;

            AudDrvSetEvent(u1DecId, AUD_CMD_FLAG_PLAY   |
                         AUD_CMD_FLAG_STOP   |
                         AUD_CMD_FLAG_RESET  |
                         AUD_CMD_FLAG_LOADCODE);

            AudDrvWaitEvent(u1DecId, &u4Event);
           
          #if ((CONFIG_AUD_POWER_MANAGEMENT_SUPPORT == 1) && (CONFIG_AUD_PM_SIMPLE_VERSION == 0))
           if(u4Event == AUD_CMD_PLAY)
           {
                AudDev_PowerOn(AUD_DEVICE_ID_FOUR);
           }
          #endif
        
            switch (u4Event)
            {
            case AUD_CMD_PLAY:
                eState = AUD_DRV_PLAYING;
                break;
            case AUD_CMD_STOP:
                eState = AUD_DRV_STOPPED;
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
                break;
            case AUD_CMD_RESET:
                eState = AUD_DRV_STOPPED;
                AUD_CommandDoneNotify(u1DecId, AUD_CMD_STOP);
                break;
            case AUD_CMD_LOADCODE:
                eState = AUD_DRV_PRESTARTING;
                break;
            default:
                AUD_VERIFY(0);
            }
            AudDrvChgState(u1DecId, eState);
            break;

        case AUD_DRV_OPENING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;

            if (fgASVCheckInit())
            {
                AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            }
            else
            {
                mdelay(1);
            }
            break;

        case AUD_DRV_PRESTARTING:
            g_rAudDecoder[u1DecId].eDecState = AUD_DEC_INIT;
            down(&g_hSemaLoadCode);
            DspLoadAdspCode(u1DecId, g_rAudDecoder[u1DecId].eDecFormat);
            LOG(LOG_FEATURE, TEXT("[AUD]Aud decoder[%d] Load [%d] code \n"),
                u1DecId, g_rAudDecoder[u1DecId].eDecFormat);
            AUD_CommandDoneNotify(u1DecId, AUD_CMD_LOADCODE);
            up(&g_hSemaLoadCode);
            AudDrvChgState(u1DecId, AUD_DRV_STOPPED);
            break;

        default:
            LOG(LOG_FAIL, TEXT("Dec3 State: u1DecId:%d, AudDrvState[u1DecId]:%d\n"),
                u1DecId,g_rAudDrvState[u1DecId]);
            AUD_VERIFY(0);

        }
    }
    complete_and_exit(NULL, 0);
}

/****************************************************************************
** Global functions
****************************************************************************/
void AUD_AsvCommandDone(u8 u1DecId, u32 u4Command)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    LOG(LOG_CTRLF, TEXT("Dec (%d) AsvCommand %s done\n"), u1DecId, g_ptcAudCmd[u4Command]);
    up(&g_hSemaUopComDone[u1DecId]);
}

void AUD_WaitAsvCommandDone(u8 u1DecId, u32 u4Command)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    LOG(LOG_CTRLF, TEXT("Dec (%d) Wait AsvCommand %s done \n"), u1DecId, g_ptcAudCmd[u4Command]);
    down(&g_hSemaUopComDone[u1DecId]);
}

void AudDrvSetAvSynMode(u8 u1DecId, AV_SYNC_MODE_T eSynMode)
{
    VERIFY(u1DecId <= TER_DEC);
    g_rAudDecoder[u1DecId].eSynMode = eSynMode;
}

void AudDrvGetDecStatus(u8 u1DecId, AUD_DECODER_T *prAudDec)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    x_memcpy(prAudDec, (const void *)&g_rAudDecoder[u1DecId], sizeof(AUD_DECODER_T));
}

bool AudDrvIsDecPlay(u8 u1DecId)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    return (g_rAudDecoder[u1DecId].eDecState == AUD_DEC_PLAYING) ? TRUE : FALSE;
}

void AudDrvInit(void)
{
    AUD_DEC_ID_T eDecId;
    
    if (!g_fgAudDrvInited)
    {
        memset(g_rAudDecoder, 0x0, sizeof(g_rAudDecoder));
        memset(g_rAudResouceManger, 0x0, sizeof(g_rAudResouceManger));

        vADSPTaskInit();

        #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
        u4AdspErrProcInit();
        #endif

        #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
        Aud_DrvInitPower();
        #else
        AudDrvThreadInit();
        Aud_Linein_Init();
        AudGpsMix_DrvInit();
        #endif

        #if 0
        vAVolDetectInit();
        #endif
        AdspMediaSemaInit();
        AudSmixInit();
    }
    g_fgAudDrvInited = TRUE;
}

bool AudDrvSetCmd(u8 u1DecId, AUD_DRV_CMD_T eCmd)
{
    u32 u4Msg = (u32)eCmd;
    bool fgRet = TRUE;

    if ((PRI_DEC == u1DecId) || (SEC_DEC == u1DecId) || (TER_DEC == u1DecId))
    {
        VERIFY((x_msg_q_send(g_hAudCmdQueue[u1DecId], &u4Msg, sizeof(u32), 1)) == OSR_OK);
    }
    else
    {
        fgRet = FALSE;
        LOG(LOG_FAIL, _T("AudDrvSetCmd unsupport Dec ID."));
    }

    return (fgRet);
}

s32 AudDrvSetDecType(u8 u1DecId,  AUD_DRV_STREAM_FROM_T eStreamFrom, const AUD_DRV_FMT_INFO_T * prDecType)
{
    if(g_rAudDecoder[u1DecId].eDecState != AUD_DEC_STOP)
    {
        LOG(LOG_CTRLF, _T("Dec state is %d, Load Code error.\r\n"), g_rAudDecoder[u1DecId].eDecState);		
        return AUD_FAIL;
    }
	
    if (prDecType != NULL)
    {
        g_rAudDecoder[u1DecId].eDecFormat =  prDecType->e_fmt;
    }
    else
    {
        return AUD_FAIL;
    }

    g_rAudDecoder[u1DecId].eStreamFrom = eStreamFrom;

    // Need wait if AOUT RESET is in progress, lock then unlock
   // i4AudLockAoutReset();
    //i4AudUnlockAoutReset();

    //i4AudLockAout2Reset();
    //i4AudUnlockAout2Reset();

    AudDrvSetCmd(u1DecId, AUD_CMD_LOADCODE);
    AUD_WaitCommandDone(u1DecId, AUD_CMD_LOADCODE);

    return AUD_OK;
}



bool AudDrvGetDecType(u8 u1DecId,  AUD_DRV_STREAM_FROM_T * peStreamFrom,
                      AUD_DRV_FMT_INFO_T * prDecType)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    if ((peStreamFrom != NULL) && (prDecType != NULL))
    {
        *peStreamFrom = g_rAudDecoder[u1DecId].eStreamFrom;
        prDecType->e_fmt = g_rAudDecoder[u1DecId].eDecFormat;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

void AudDrvGetNfy(u8 u1DecId, AUD_DRV_NFY_INFO_T * prAudNfyInfo)
{
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    if (prAudNfyInfo != NULL)
    {
        if(u1DecId == RE_ENC)
        {
            prAudNfyInfo->pvTag = g_rAudEncoder.rNfyInfo.pvTag;
			prAudNfyInfo->pfAudDecNfy = g_rAudEncoder.rNfyInfo.pfAudDecNfy;
        }
		else
		{
            prAudNfyInfo->pvTag = g_rAudDecoder[u1DecId].rNfyInfo.pvTag;
			prAudNfyInfo->pfAudDecNfy = g_rAudDecoder[u1DecId].rNfyInfo.pfAudDecNfy;
		}
    }
}

/****************************************************************************
* Function : AudDrvGetDecState
* Description :
* Parameter :
* Return    :
* Note      :
****************************************************************************/
DECODER_STATE_T AudDrvGetDecState(u8 u1DecId)
{
    return g_rAudDecoder[u1DecId].eDecState;
}

// Lock Aout Reset
s32 i4AudLockAoutReset(void)
{
    s32 i4Ret = OSR_OK;

    down(&g_hSemaAoutReset);

    g_fgAoutResetLocked = TRUE; // manually implement a binary semaphore

    return i4Ret;
}

// Unlock Aout Reset
s32 i4AudUnlockAoutReset(void)
{
    s32 i4Ret = OSR_OK;

    if (g_fgAoutResetLocked) // manually implement a binary semaphore
    {
        up(&g_hSemaAoutReset);
        g_fgAoutResetLocked = FALSE;
    }

    return i4Ret;
}

// Lock Aout2 Reset
s32 i4AudLockAout2Reset(void)
{
    s32 i4Ret = OSR_OK;

    down(&g_hSemaAout2Reset);

    g_fgAout2ResetLocked = TRUE; // manually implement a binary semaphore

    return i4Ret;
}

// Unlock Aout2 Reset
s32 i4AudUnlockAout2Reset(void)
{
    s32 i4Ret = OSR_OK;

    if (g_fgAout2ResetLocked) // manually implement a binary semaphore
    {
        up(&g_hSemaAout2Reset);
        g_fgAout2ResetLocked = FALSE;
    }

    return i4Ret;
}


void GpsMix_SetAoutCfg(void)
{
    AUD_OUTPUT_PATH_T rGpsMixSelect = {0};
    rGpsMixSelect.eOut = AUD_FRONT;
    rGpsMixSelect.eSrc = AUD_AOUT1;
    //AudCfg_SetAoutCfg(&rGpsMixSelect);
}



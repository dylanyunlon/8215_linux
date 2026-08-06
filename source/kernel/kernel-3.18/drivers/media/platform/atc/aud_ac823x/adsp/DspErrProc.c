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
*[File]                DspErrProc.c
*[Author] 
*[Description]
******************************************************************************/

/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include <linux/types.h>
#include "DspErrProc.h"
#include "aud_oal.h"
#include "aud_drv.h"
#include "aud_drv_config.h"
#include "aud_debug.h"
#include "DspFunc.h"
#include "aud_ioctrl.h"
#include "aud_config.h"
#include "aud_if.h"
#include "aud_esm.h"
#include "AsvDspCtrl.h"
#include "AsvAudDrv.h"
#include "drv_thread.h"
#include "aud_io_clock_if.h"
#include "aud_config.h"



/*******************************variable define **********************/
DSP_DET_STATE_T rDspDetState;
u32 u4ErrDetLoopCounter = 0;

u32 u4DetLoopCnt = 10;
u32 eDspErrDetState = DSP_PROC_INIT;
bool fgCodecNofityReset[TER_DEC + 1];
bool fgDspErrProcInit = 0;

static struct semaphore  _h_adsp_err_proc_sema;
static struct task_struct *g_hadsp_err_proc_thread = NULL;


WT_DSPB_CODEC_STATE_T rFstCodecSt, rSndCodecSt, rTrdCodecSt;  //NULL means no need to check

extern bool   g_fgAudEosState;
extern void* g_hAudFlushEvent;

/*******************************function declarations *******************/
s32 u4AdspErrProcLoop(void* pvArg);
extern u32 u4DspWorkStateDetect(DSP_DET_STATE_T *prDspDetState);
extern u32 u4DspStateDetectReinit(DSP_DET_STATE_T *prDspDetState, u32 u4DetLoopCnt);
extern void vRstRptr2KeepAfifoNotFull(u16 u2DecId);


/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
static bool fgErrTimerEn = FALSE;
static u32 u4ErrTimerCnt = 0;
void vErrTimerAdd(void)
{
    fgErrTimerEn = TRUE;
    u4ErrTimerCnt = 0;
    LOG(LOG_CTRLF, TEXT("AddETimer\n"));
    return;
}
void vErrTimerDelete(void)
{
    fgErrTimerEn = FALSE;
    u4ErrTimerCnt = 0;
    LOG(LOG_CTRLF, TEXT("DelETimer\n"));
    return;
}

bool fgErrTimerEvent(void)
{
    bool fgRet = FALSE;
    
    if(fgErrTimerEn)
    {
        u4ErrTimerCnt++;
        if(u4ErrTimerCnt > 20)  // over 2s
        {
            LOG(LOG_CTRLF, TEXT("MW_CMD Time Out Over 2s, Triggle AdspErrHandle \n"));
            fgErrTimerEn = FALSE;
            u4ErrTimerCnt = 0;
            fgRet = TRUE;            
        }
    }
    return fgRet;
}

/******************************************************************************
* Function      :u4AdspErrProcStateSet
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
u32 u4AdspErrProcStateSet(DSP_RPOC_STATE eNewProcSt)
{
    u32 u4ProcResult = DSP_PROC_OK;

    down(&_h_adsp_err_proc_sema);
    eDspErrDetState = eNewProcSt;
    up(&_h_adsp_err_proc_sema);

    return u4ProcResult;
}

/******************************************************************************
* Function      :u4AdspErrProcStateGet
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
DSP_RPOC_STATE u4AdspErrProcStateGet(void)
{
    DSP_RPOC_STATE eProcSt;
    
    down(&_h_adsp_err_proc_sema);
    eProcSt = eDspErrDetState;
    up(&_h_adsp_err_proc_sema);

    return eProcSt;
}

/******************************************************************************
* Function      :u4AdspErrProcNotifyFlagSet
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
u32 u4AdspErrProcNotifyFlagSet(u32 u4DecId, bool fgNotify)
{
    u32 u4ProcResult = DSP_PROC_OK;
    
    down(&_h_adsp_err_proc_sema);
    fgCodecNofityReset[u4DecId] = fgNotify;
    up(&_h_adsp_err_proc_sema);

    return u4ProcResult;
}

/******************************************************************************
* Function      :u4AdspErrProcNotifyFlagGet
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
bool u4AdspErrProcNotifyFlagGet(u32 u4DecId)
{
    bool fgNotify;

    down(&_h_adsp_err_proc_sema);
    fgNotify = fgCodecNofityReset[u4DecId];
    up(&_h_adsp_err_proc_sema);

    return fgNotify;
}

/******************************************************************************
* Function      :vDspSetEvent
* Description   :set event to dsp
* Parameter     :u4Evt: event id
* Return        :
******************************************************************************/
 u32 u4AdspDrvStateMachineReset(u8 u1DecId)
 {
    u32 u4ProcResult = DSP_PROC_OK;
     
    AUD_DRV_STATE_T eDrvState;

    eDrvState = AudDrvGetState(u1DecId);

    LOG(LOG_CTRLF, TEXT("u4AdspErrHandle,eDrvState (%d) \n"), eDrvState);
    
    switch (eDrvState)
    {
    case AUD_DRV_PLAYING:
        AUD_AsvCommandDone(u1DecId,AUD_CMD_PLAY);
        break;

    case AUD_DRV_STOPPING:
        AUD_AsvCommandDone(u1DecId,AUD_CMD_STOP);
        break;

    case AUD_DRV_PAUSING:
        AUD_AsvCommandDone(u1DecId,AUD_CMD_PAUSE);
        break;

    case AUD_DRV_RESUMING:
        AUD_AsvCommandDone(u1DecId,AUD_CMD_RESUME);
        break;

    case AUD_DRV_PLAYED: 
        AudDrvSetCmd(u1DecId, AUD_CMD_ERRORRECOVER);
        break;

    default:
        break;

    }

    return u4ProcResult;

 }

/******************************************************************************
* Function      :u4AdspErrProcInit
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
u32 u4AdspErrProcInit(void)
{
    u32 u4ProcResult = DSP_PROC_OK;

    LOG(LOG_CTRLF, TEXT("u4AdspErrProcInit \n"));

    vErrTimerDelete();
    
    u4DetLoopCnt = 10;

    fgDspErrProcInit = 1;
    eDspErrDetState = DSP_PROC_INIT;
    
    fgCodecNofityReset[PRI_DEC] = 0;
    fgCodecNofityReset[SEC_DEC] = 0;
    fgCodecNofityReset[TER_DEC] = 0;

    rDspDetState.prFstCodecSt = &rFstCodecSt;
    rDspDetState.prSndCodecSt = NULL;
    rDspDetState.prTrdCodecSt = NULL;

    u4DspStateDetectReinit(&rDspDetState, u4DetLoopCnt);

    sema_init(&_h_adsp_err_proc_sema, 1);

    g_hadsp_err_proc_thread = kthread_create(u4AdspErrProcLoop, (void *)NULL, "ADSP_ERR_PROC");
	if (IS_ERR(g_hadsp_err_proc_thread)) {
		LOG(LOG_CTRLF, TEXT("[u4AdspErrProcInit]u4AdspErrProcLoop thread create fail \r\n"));
		g_hadsp_err_proc_thread = NULL;
		return DSP_PROC_FAIL;
	}
    else
    {
        struct sched_param param;
        s32 ret;

        param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hadsp_err_proc_thread, SCHED_RR, &param);
        ASSERT(ret == 0);
    }
	wake_up_process(g_hadsp_err_proc_thread);

    LOG(LOG_CTRLF, TEXT("u4AdspErrProcInit  done \n"));

    return u4ProcResult;
}

/******************************************************************************
* Function      :vAdspErrProcUnInit
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
u32 vAdspErrProcUnInit(void)
{
    u32 u4ProcResult = DSP_PROC_OK;
    
    fgDspErrProcInit = 0;

    return u4ProcResult;
}

/******************************************************************************
* Function      :fgAdspaErrHappen
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
bool fgAdspaErrHappen(DSP_DET_STATE_T *prDspDetSt)
{
    bool fgErrHapeen;
    WT_DSP_HW_STATE_T *prDspaHwSt = &(prDspDetSt->rDspaHwSt);

    fgErrHapeen = 0;
    
    fgErrHapeen = (prDspaHwSt->fgDspHangUp && prDspaHwSt->fgDspHangUpValid) ? 1 : 0;

    if (prDspaHwSt->fgDspBusy && prDspaHwSt->fgDspBusyValid)
    {
        fgErrHapeen = 1;
    }

    return fgErrHapeen;
}

/******************************************************************************
* Function      :fgAdspbErrHappen
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
bool fgAdspbErrHappen(DSP_DET_STATE_T *prDspDetSt)
{
    bool fgErrHapeen;
    
    AUD_DRV_STATE_T eDrvState;
    WT_DSP_HW_STATE_T *prDspbHwSt = &(prDspDetSt->rDspbHwSt);

    fgErrHapeen = 0;
    
    fgErrHapeen = (prDspbHwSt->fgDspHangUp && prDspbHwSt->fgDspHangUpValid) ? 1 : 0;

    if (NULL != prDspDetSt->prFstCodecSt)
    {
        if (prDspbHwSt->fgDspEndlessLoop && prDspbHwSt->fgDspEndLessLoopValid && prDspDetSt->prFstCodecSt->fgDecAbNormal)
        {
            eDrvState = AudDrvGetState(PRI_DEC);
            if ((AUD_DRV_PLAYING == eDrvState) || (AUD_DRV_PLAYED == eDrvState) || (AUD_DRV_STOPPING == eDrvState))
            {
                fgErrHapeen = 1;
            }
        }
    }

    if (prDspbHwSt->fgDspBusy && prDspbHwSt->fgDspBusyValid)
    {
        fgErrHapeen = 1;
    }

    return fgErrHapeen;
}

/******************************************************************************
* Function      :u4AdspErrHandle
* Description   :
* Parameter    :
* Return        :
******************************************************************************/
u32 u4AdspErrHandle(void)
{
    u32 u4ProcResult = DSP_PROC_OK;
    u32 u4DecId, i;
    AUD_DRV_STATE_T eDrvState;

    LOG(LOG_CTRLF, TEXT("u4AdspErrHandle start...  \n"));
    
    // 1. reset adsp hw
    AudPower_Deinit();
    IoClk_SetDspHwRest();
    vDspPowerOff();
   
    vRstRptr2KeepAfifoNotFull(PRI_DEC);

    // 3. reset driver state mechine
    vAudStateReset();

    // 4. change error detect state
    u4AdspErrProcStateSet(DSP_PROC_RESET);

    u4DspStateDetectReinit(&rDspDetState, u4DetLoopCnt);

    // dspinit
    vDspErrRecoverOnInit(u4ErrDetLoopCounter);
    AudPower_ErrRecover_Init();

    i = 100;
    while(i > 0)
    {
        if(fgDspAWakeup() && fgDspBWakeup())
        {
            u1AsvDspAAoutOn();
            u1AsvDspAAout2On();

            break;
        }
        msleep(20);
        i--;
    }

    // 2. check codec state
    for (u4DecId = PRI_DEC; u4DecId <= TER_DEC; u4DecId++)
    {
        eDrvState = AudDrvGetState(u4DecId);
        if (((AUD_DRV_STOPPED == eDrvState) || (AUD_DRV_PAUSED == eDrvState)))
        {
            u4AdspErrProcNotifyFlagSet(u4DecId, 0);
        }
        else
        {
            u4AdspErrProcNotifyFlagSet(u4DecId, 1);
            u4AdspDrvStateMachineReset(u4DecId);
        }
    }

    // 5. error notify
    if (TRUE == g_fgAudEosState)
    {
        LOG(LOG_CTRLF, TEXT("u4AdspErrHandle notify EOS event \n"));
        
        x_event_set(g_hAudFlushEvent);
        
        g_fgAudEosState = FALSE;

        u4AdspErrProcNotifyFlagSet(u4DecId, 1);
        //u4AdspErrProcStateSet(DSP_PROC_RUN);
    }

    LOG(LOG_CTRLF, TEXT("u4AdspErrHandle end... \n"));

    return u4ProcResult;
}

/******************************************************************************
* Function      :vAdspErrProcLoop
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
s32 u4AdspErrProcLoop(void* pvArg)
{   
    bool fgDspaErrHappen, fgDspbErrHappen;
    u32 timeout;

    while (fgDspErrProcInit)
    {  
        if (DSP_PROC_RUN == u4AdspErrProcStateGet())
        {            
            fgDspaErrHappen = 0;
            fgDspbErrHappen = 0;
            
            //error detect
            u4DspWorkStateDetect(&rDspDetState);

            //error check
            if (rDspDetState.i4DetCntRemain <= 0)
            {
                fgDspaErrHappen = fgAdspaErrHappen(&rDspDetState);
                fgDspbErrHappen = fgAdspbErrHappen(&rDspDetState);
            }

            //error proc
            if (fgDspaErrHappen || fgDspbErrHappen || fgErrTimerEvent())
            {                
                LOG(LOG_CTRLF, TEXT("u4AdspErrHandle start dspaErr(%d),  dspbErr(%d) \n"),fgDspaErrHappen, fgDspbErrHappen);

                LOG(LOG_CTRLF, TEXT("fgDspaEndlessLoop(%d), fgDspaHangUp(%d), fgDspaBusy(%d), u4CuraPcVal(0x%x), u4FstaPcVal(0x%x) \n"),
                rDspDetState.rDspaHwSt.fgDspEndlessLoop,
                rDspDetState.rDspaHwSt.fgDspHangUp,
                rDspDetState.rDspaHwSt.fgDspBusy,
                rDspDetState.rDspaHwSt.u4CurPcVal,
                rDspDetState.rDspaHwSt.u4FstPcVal);
                
                LOG(LOG_CTRLF, TEXT("fgDspbEndlessLoop(%d), fgDspbHangUp(%d), fgDspbBusy(%d), u4CurbPcVal(0x%x), u4FstbPcVal(0x%x) \n"),
                rDspDetState.rDspbHwSt.fgDspEndlessLoop,
                rDspDetState.rDspbHwSt.fgDspHangUp,
                rDspDetState.rDspbHwSt.fgDspBusy,
                rDspDetState.rDspbHwSt.u4CurPcVal,
                rDspDetState.rDspbHwSt.u4FstPcVal);

                u4ErrDetLoopCounter++;
                LOG(LOG_CTRLF, TEXT("detect coutner (%d) \n"),u4ErrDetLoopCounter);
                
                u4AdspErrHandle();
            }
        }

        msleep(100);
    }
    
    complete_and_exit(NULL, 0);

	return 0;
}

void vAdspErrRecoveryDbgLog(void)
{
    WT_DSPB_CODEC_STATE_T *prFstCodecSt;

    LOG(LOG_CTRLF, TEXT("fgDspErrProcInit (%d) \n"),fgDspErrProcInit);
    LOG(LOG_CTRLF, TEXT("u4DetLoopCnt (%d) \n"),u4DetLoopCnt);
    LOG(LOG_CTRLF, TEXT("u4ErrDetLoopCounter (%d) \n"),u4ErrDetLoopCounter);
    LOG(LOG_CTRLF, TEXT("eDspErrDetState (%d) \n"),eDspErrDetState);
    LOG(LOG_CTRLF, TEXT("eDrvStatePrimary (%d) \n"),AudDrvGetState(PRI_DEC));    
    //LOG(LOG_CTRLF, TEXT("ADDR_D2RC_PLAYBACK_BANK_CNT (0x%x) \n"),dReadDspCommDram(ADDR_D2RC_PLAYBACK_BANK_CNT));
    

    LOG(LOG_CTRLF, TEXT("rFstCodecSt: fgDecAbNormal(%d), u4DecFstBankNum(0x%x), u4DecCurBankNum(0x%x) \n"),
    rFstCodecSt.fgDecAbNormal,
    rFstCodecSt.u4DecFstBankNum,
    rFstCodecSt.u4DecCurBankNum);
    
    LOG(LOG_CTRLF, TEXT("-------------- rDspDetState -------------- \n"));
    if (NULL != rDspDetState.prFstCodecSt)
    {
        prFstCodecSt = rDspDetState.prFstCodecSt;
        LOG(LOG_CTRLF, TEXT("prFstCodecSt: fgDecAbNormal(%d), u4DecFstBankNum(0x%x), u4DecCurBankNum(0x%x) \n"),
        prFstCodecSt->fgDecAbNormal,
        prFstCodecSt->u4DecFstBankNum,
        prFstCodecSt->u4DecCurBankNum);
    }
    
    LOG(LOG_CTRLF, TEXT("dspAHangUp(%d)valid(%d), EndlessLoop(%d)valid(%d), Busy(%d)valid(%d), CuraPcVal(0x%x), FstaPcVal(0x%x) \n"),
    rDspDetState.rDspaHwSt.fgDspHangUp,
    rDspDetState.rDspaHwSt.fgDspHangUpValid,
    rDspDetState.rDspaHwSt.fgDspEndlessLoop,
    rDspDetState.rDspaHwSt.fgDspEndLessLoopValid,
    rDspDetState.rDspaHwSt.fgDspBusy,
    rDspDetState.rDspaHwSt.fgDspBusyValid,
    rDspDetState.rDspaHwSt.u4CurPcVal,
    rDspDetState.rDspaHwSt.u4FstPcVal);
    
    LOG(LOG_CTRLF, TEXT("dspBHangUp(%d)valid(%d), EndlessLoop(%d)valid(%d), Busy(%d)valid(%d), CurbPcVal(0x%x), FstbPcVal(0x%x) \n"),
    rDspDetState.rDspbHwSt.fgDspHangUp,
    rDspDetState.rDspbHwSt.fgDspHangUpValid,
    rDspDetState.rDspbHwSt.fgDspEndlessLoop,
    rDspDetState.rDspbHwSt.fgDspEndLessLoopValid,
    rDspDetState.rDspbHwSt.fgDspBusy,
    rDspDetState.rDspbHwSt.fgDspBusyValid,
    rDspDetState.rDspbHwSt.u4CurPcVal,
    rDspDetState.rDspbHwSt.u4FstPcVal);

    LOG(LOG_CTRLF, TEXT("i4DetCntRemain (%d) \n"),rDspDetState.i4DetCntRemain);

    return;    
}

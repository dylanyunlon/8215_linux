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
#include "x_assert.h"

#include "aud_oal.h"
#include "DspDrvInc.h"
#include "DspFunc.h"
#include "DspShm.h"
#include "AsvAudDrv.h"

#include "drv_thread.h"
#include "aud_drv_config.h"
#include "aud_drv.h"
#include "aud_debug.h"
#include "audin_if.h"
#include "aud_if.h"
#include "aud_ioctrl.h"
#include <media/atc/ose_mem.h>
#include <media/atc/mm_common.h>
#include "aud_esm.h"
#include "u_uerrcode.h"
#include "aud_comm_macros.h"
#include "aud_reg_env.h"
#include <linux/slab.h>
#include <linux/list.h>

/****************************************************************************
** Local structures and enumerations
****************************************************************************/
#define AUD_ESM_TMR_TICKS 25

typedef enum
{
   AUD_ESM_DISCNT = 0,
   AUD_ESM_CNT
}AUD_ESM_CNT_STATE;

AUD_AFIFO_POSINFO_T rAud_AFifo_PosInfo;
AUD_AFIFO_POSINFO_T *m_aud_afifo_info;

typedef struct AU_LIST_UNIT
{
  AU_AUDIO rAu;
  struct list_head au_queue;
}AU_LIST_UNIT;


/****************************************************************************
** Local variable
****************************************************************************/
#ifndef __linux__
AFIFO_POSINFO_T *m_afifo_info = NULL; //for share with esm.c
#endif

//for bitstream decoders
AUD_ESM_CONTEXT_T g_rAudEsmContext[(TER_DEC + 1)];

u32 g_u4AudPriICBId  = 0;
u32 g_u4AudFourICBId = 0;
u32 g_u4AudFiveICBId = 0;
u8  g_u1SkipDataMode = 0xFF;

bool   g_fgEnableSkipData  = FALSE;
bool   g_fgAVSyncSkipToend = FALSE;
bool   g_fgAudEosState     = FALSE;
bool   g_fgAudioEosDone    = FALSE;
bool   g_fgAudIgnoreAU     = FALSE;

bool   g_fgPrintAU = FALSE;

static u32 g_u4SkipDataAddr = 0xFFFFFFFF;

volatile static bool g_fgAudEsmThdInited = FALSE;

//audio thread handle
static struct task_struct *g_hAudEsmThread = NULL;
static struct task_struct *g_hAudLineinThd = NULL;
static struct task_struct *g_hAudLinein2Thd = NULL;

static u32 _hAudEsmEvntGrp;
static struct list_head _hAudEsmAuQueue[TER_DEC + 1];
static struct kmem_cache * g_hAudEsmAuChunk[TER_DEC + 1];
static struct timer_list _h_aud_esm_rptr_update[TER_DEC + 1];

//HANDLE g_hAudSetCmd[TER_DEC + 1] = {NULL};
void * g_hAudFlushEvent;
void * g_hAudDecReady;

AUD_PTS_QUEUE_INFO_T g_rDspPTSQueueInfo[2];
static u32 g_u4PtsQueueErrCnt[2];

#ifndef __linux__
CRITICAL_SECTION esm_send_au_lock;
#else
static DEFINE_SPINLOCK(esm_send_au_lock);
#endif
static DEFINE_SPINLOCK(esm_notify_state_lock);


extern u64 g_u8Pts;
extern u64 g_u8STC;

static void i4AudEsm_Uninit(void);
static void vAudEsm_StopInit(u16 u2ADRV_Comp_Id);
static void AudEsmInitPtsQueue(u8 u1DecId);
static void AudEsmSetPtsToQueue(u8 u1DecId,u32 u4PtsAddr, u32 u4PtsVal);

static u32 g_u4AuCount = 0;
static void vAudEsm_PrintAu(AUD_ESM_CONTEXT_T *pContext, AU_AUDIO *pAu)
{
    LOG(LOG_CTRLF, TEXT("DecID: %d,AuType: 0x%x,ptrSAddr: 0x%lx,ptrEAddr: 0x%lx.\n"),
        pContext->u2AudDrvCompId, pAu->eAuType, pAu->ptrSAddr, pAu->ptrEAddr);
}

void vAudEsm_InitTotalPBTimeCount(void)
{
    AUD_ESM_CONTEXT_T* pContext = &g_rAudEsmContext[SEC_DEC];
    pContext->u4TotalPBBankCount = 0;
    pContext->u4TotalPBFrameCount = 0;

    pContext = &g_rAudEsmContext[TER_DEC];
    pContext->u4TotalPBBankCount = 0;
    pContext->u4TotalPBFrameCount = 0;
}

u32 u4AudEsm_GetTotalPBBankCount(void)
{
    return g_rAudEsmContext[SEC_DEC].u4TotalPBBankCount;
}

void vAudEsm_SetTotalPBBankCount(u32 u4_update_bank_count)
{
    g_rAudEsmContext[SEC_DEC].u4TotalPBBankCount = u4_update_bank_count;
}

/****************************************************************************
*    Function : vAudEsmUpdateReadPtr
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsmUpdateReadPtr(AUD_ESM_CONTEXT_T *pContext)
{
    u32 u4CurReadPtr;

    if (!pContext->fgIsPlay)
    {
        return;
    }

    //update AFIFO Read Pointer to ESM
    u4CurReadPtr = u4AudHalGetBufRPtr((u8)pContext->u2AudDrvCompId, DSP_AFIFO);
    if (pContext->u4AfifoRPtr != u4CurReadPtr)
    {
        if (pContext->u4AfifoRPtr == pContext->u4AfifoSA &&
            pContext->u4AfifoEA - u4CurReadPtr < 0x400)
            return;

        if (u4CurReadPtr > pContext->u4AfifoRPtr)
        {
            pContext->u4ReadCnt += u4CurReadPtr - pContext->u4AfifoRPtr;
        }
        else
        {
            pContext->u4ReadCnt += pContext->u4AfifoEA + u4CurReadPtr - pContext->u4AfifoRPtr - pContext->u4AfifoSA;
        }

        pContext->u4AfifoRPtr = u4CurReadPtr;

        m_aud_afifo_info->posInfo[(u8)pContext->u2AudDrvCompId].ptrAfifoRPtr = u4CurReadPtr - pContext->u4AfifoSA;

        // Update AU read index after DSP has decoded related data.
        if (pContext->fgFirstAUArrive)
        {
            pContext->u4LastUpdatedRptr = u4CurReadPtr;
        }
    }

}


/****************************************************************************
*    Function : vAudEsmUpdateAllReadPtr
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsmUpdateAllReadPtr(u32 u4DecId)
{
	AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[PRI_DEC];
    // RISC skip AFIFO data solution
    if(g_fgEnableSkipData)
    {
        u32 u4WrPtr = 0, u4RdPtr = 0;

        u4RdPtr = u4AudHalGetBufRPtr((u8)pContext->u2AudDrvCompId, DSP_AFIFO);

        if (pContext->fgIsPlay)
        {
            switch (g_u1SkipDataMode)
            {
            case 0:
                if (u4RdPtr < g_u4SkipDataAddr)
                {
                    g_u1SkipDataMode = 1;
                }
                break;

            case 1:
                if (u4RdPtr >= g_u4SkipDataAddr)
                {
                    g_u1SkipDataMode = 2;// change to RISC skip mode
                }
                break;

            case 2:
                {
                    u32 u4AUWrIdx = 0;
                    u4AUWrIdx = (pContext->prDecoderOpIf->pu4AUT_GetWrIdx)(pContext->u4Handle);
                    // Update AU read index to write index
                    VERIFY(SUCCEEDED((pContext->prDecoderOpIf->pi4AUT_SetRdIdx)(pContext->u4Handle, u4AUWrIdx)));

                    // Update read pointer to last write pointer
                    // Get information from AU Table
                    u4WrPtr = (*pContext->prDecoderOpIf->pu4FIFO_GetWp)(pContext->u4Handle);
                    (*pContext->prDecoderOpIf->pi4FIFO_SetRp)(pContext->u4Handle, u4WrPtr);
                }
                // in this mode, no need to update RP from DSP
                return;

            default:
                break;
            }
        }

    }
    if( !g_fgAudIgnoreAU )
    {
        switch (u4DecId)
        {
        case PRI_DEC:
            if((pContext->u4Handle != ESM_INVALID_HANDLE) && (pContext->u4AfifoSA != 0))
            {
                vAudEsmUpdateReadPtr(pContext);
            }
            break;

        case SEC_DEC:
			pContext = &g_rAudEsmContext[SEC_DEC];
            if((pContext->u4Handle != ESM_INVALID_HANDLE) && (pContext->u4AfifoSA != 0))
            {
                vAudEsmUpdateReadPtr(pContext);
            }
            break;

       case TER_DEC:
            pContext = &g_rAudEsmContext[TER_DEC];
            if((pContext->u4Handle != ESM_INVALID_HANDLE) &&(pContext->u4AfifoSA != 0))
            {
                vAudEsmUpdateReadPtr(pContext);
            }
            break;
        }
    }
    //del_timer(&(_h_aud_esm_rptr_update[u4DecId]));
    _h_aud_esm_rptr_update[u4DecId].expires = jiffies + (AUD_ESM_TMR_TICKS * HZ / 1000);
    add_timer(&(_h_aud_esm_rptr_update[u4DecId]));
}


/****************************************************************************
*    Function : uAudEsmGetWritePtr
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
u32 u4AudEsmGetWritePtr(AUD_ESM_CONTEXT_T *pContext)
{
    //update AFIFO Read Pointer to ESM
    u32 u4CurWritePtr = pContext->u4AfifoSA + m_aud_afifo_info->posInfo[(u8)pContext->u2AudDrvCompId].ptrAfifoWPtr;

    return(u4CurWritePtr);
}


/****************************************************************************
*    Function : uAudEsmGetReadPtr
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
u32 u4AudEsmGetReadPtr(AUD_ESM_CONTEXT_T *pContext)
{
    //update AFIFO Read Pointer to ESM
    u32 u4CurReadPtr = pContext->u4AfifoSA + m_aud_afifo_info->posInfo[(u8)pContext->u2AudDrvCompId].ptrAfifoRPtr;

    return(u4CurReadPtr);
}


/****************************************************************************
*    Function : vAudEsm_TimerEnable
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsm_TimerCreate(u8 u1DecId)
{
    //create the timer
    if (u1DecId < sizeof(_h_aud_esm_rptr_update) / sizeof(_h_aud_esm_rptr_update[0]))
    {
        init_timer(&(_h_aud_esm_rptr_update[u1DecId]));
        _h_aud_esm_rptr_update[u1DecId].expires = jiffies + (AUD_ESM_TMR_TICKS * HZ / 1000);
        _h_aud_esm_rptr_update[u1DecId].function = vAudEsmUpdateAllReadPtr;
        _h_aud_esm_rptr_update[u1DecId].data = (u32)u1DecId;
    }
}

/****************************************************************************
*    Function : vAudEsm_TimerDelete
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsm_TimerDelete(u8 u1DecId)
{
    //delete the timer
    if (u1DecId < sizeof(_h_aud_esm_rptr_update) / sizeof(_h_aud_esm_rptr_update[0]))
    {
            del_timer_sync(&(_h_aud_esm_rptr_update[u1DecId]));
    }
}

/****************************************************************************
*    Function : vAudEsm_TimerEnable
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsm_TimerEnable(u8 u1DecId)
{
    //start the timer
    if (u1DecId < sizeof(_h_aud_esm_rptr_update) / sizeof(_h_aud_esm_rptr_update[0]))
    {
        add_timer(&(_h_aud_esm_rptr_update[u1DecId]));
    }

    return;
}

/****************************************************************************
*    Function : vAudEsm_TimerDisable
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsm_TimerDisable(u8 u1DecId)
{
    //stop the timer //delete time has been do it, so need set again.
    //if (u1DecId < sizeof(_h_aud_esm_rptr_update) / sizeof(_h_aud_esm_rptr_update[0]))
    //{
    //    del_timer(&(_h_aud_esm_rptr_update[u1DecId]));
    //}

    return;
}

/****************************************************************************
*    Function : i4AudEsm_SetEnabled
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
static s32 i4AudEsm_SetEnabled(bool fgValue, u8 u1DecId)
{
    (fgValue == TRUE) ? vAudEsm_TimerEnable(u1DecId) : vAudEsm_TimerDisable(u1DecId);
    return (AUD_OK);
}


/******************************************************************************
* Function     :        i4AudEsm_DealDataAU
* Description :        deal Au data
* Parameter  :       u1DecId,pAu,pContext
* Return        :       int32
* Author        :       fei.zhu
* Time           :      2011-01-04
******************************************************************************/
static s32 i4AudEsm_DealDataAU(u8 u1DecId,AUD_ESM_CONTEXT_T *pContext, AU_AUDIO *pAu)
{
    ASSERT(pAu);
    ASSERT(pContext);

    pAu->ptrSAddr = pContext->u4AfifoSA + pAu->ptrSAddr;
    pAu->ptrEAddr = pContext->u4AfifoSA + pAu->ptrEAddr;

    //au addr between AfifoSA and Afifo EA
    if ((pAu->ptrEAddr <= pContext->u4AfifoEA) && (pAu->ptrSAddr >= pContext->u4AfifoSA))
    {
        if(pAu->ptrEAddr == pContext->u4AfifoEA)
        {
            pAu->ptrEAddr = pContext->u4AfifoSA;
        }
        vAudHalSetBufWPtr(u1DecId, DSP_AFIFO, pAu->ptrEAddr);
        pContext->u4AfifoWPtr = pAu->ptrEAddr;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("i4AudEsm_WriteAU failed:\n"));
        LOG(LOG_FAIL, TEXT("i4AudEsm_WriteAU, pAu->ptrSAddr:0x%xl, pAu->ptrEAddr:0x%lx\n"),
            pAu->ptrSAddr, pAu->ptrEAddr);
        LOG(LOG_FAIL, TEXT("i4AudEsm_WriteAU, pContext->u4AfifoSA:0x%x, pContext->u4AfifoEA:0x%x\n"),
            pContext->u4AfifoSA, pContext->u4AfifoEA);
    }

    return (AUD_OK);
}

/******************************************************************************
* Function     :        i4AudEsm_DealDataAU
* Description :        deal Au CMD
* Parameter  :       u1DecId,pAu,pContext
* Return        :       int32
* Author        :       fei.zhu
* Time           :      2011-01-04
******************************************************************************/
static s32 i4AudEsm_DealCmdAU(u8 u1DecId,AUD_ESM_CONTEXT_T *pContext, AU_AUDIO *pAu)
{
    u32 i4Ret = 0;
    if (!pContext->fgIsPlay)
    {
        LOG(LOG_CTRLF, TEXT("[Audio] (%d) Receive EOS inband cmd when STOP!\n"),
            pContext->u2AudDrvCompId);
        return 0;
    }

    if (pContext->u2AudDrvCompId == PRI_DEC)
    {
        g_u4AudPriICBId = 3;// end of stream
        g_fgAudEosState = TRUE;
    }
    else if (SEC_DEC == u1DecId)
    {
        g_u4AudFourICBId = IBC_InbandCmdTypeEndOfStream;
    }
    else if (TER_DEC == u1DecId)
    {
        g_u4AudFiveICBId = IBC_InbandCmdTypeEndOfStream;
    }

    if (!u1AsvFlushCmd((u8)pContext->u2AudDrvCompId))
    {
        x_event_set(g_hAudFlushEvent);
        g_fgAudIgnoreAU = TRUE;
        LOG(LOG_FAIL, TEXT("[Audio][ERROR] DspB Flush command fail!!\n"));
        return i4Ret;
    }

    return i4Ret;
}

/******************************************************************************
* Function     :        i4AudEsm_AUProc
* Description :        AU handle include au_data and au_cmd
* Parameter  :       pvData,pContext
* Return        :       AUD_OK
* Author        :       fei.zhu
* Time           :      2010-12-31
******************************************************************************/
s32 i4AudEsm_AUProc(void *pvData, AUD_ESM_CONTEXT_T *pContext)
{
    s32 i4Ret = AUD_OK;
    AU_LIST_UNIT *pAuUnit = NULL;
    u8 u1DecId = (u8)pContext->u2AudDrvCompId;

    while (1)
    {
        if (list_empty(&(_hAudEsmAuQueue[u1DecId])))
        {
            break;
        }

        pAuUnit = list_entry((_hAudEsmAuQueue[u1DecId]).next, AU_LIST_UNIT, au_queue);
        list_del_init((_hAudEsmAuQueue[u1DecId]).next);

        if (AU_DATA == pAuUnit->rAu.eAuType)
        {
            i4AudEsm_DealDataAU(u1DecId, pContext, &(pAuUnit->rAu));
        }

        else if (AU_CMD == pAuUnit->rAu.eAuType)
        {
            i4AudEsm_DealCmdAU(u1DecId, pContext, &(pAuUnit->rAu));
        }
        else
        {
            LOG(LOG_FAIL, TEXT("No this eAuType\n"));
            ASSERT(0);
        }
        // Free AU
        kmem_cache_free(g_hAudEsmAuChunk[u1DecId], pAuUnit);
    }

    if (!pContext->fgFirstAUArrive)
    {
        pContext->fgFirstAUArrive = TRUE;
    }

    return i4Ret;
}

s32 i4AudEsm_SetAuEvent(u8 u1DecId)
{
    switch (u1DecId)
    {
    case PRI_DEC:
        VERIFY(x_ev_group_set_event(_hAudEsmEvntGrp, AUD_ESM_EVENT_AU_PRI, X_EV_OP_OR) == OSR_OK);
        break;
    case SEC_DEC:
        VERIFY(x_ev_group_set_event(_hAudEsmEvntGrp, AUD_ESM_EVENT_AU_SEC, X_EV_OP_OR) == OSR_OK);
        break;
    case TER_DEC:
        VERIFY(x_ev_group_set_event(_hAudEsmEvntGrp, AUD_ESM_EVENT_AU_TER, X_EV_OP_OR) == OSR_OK);
        break;
    case LINEIN1:
        VERIFY(x_ev_group_set_event(_hAudEsmEvntGrp, AUD_ESM_EVENT_AU_LINEIN1, X_EV_OP_OR) == OSR_OK);
        break;
    case LINEIN2:
        VERIFY(x_ev_group_set_event(_hAudEsmEvntGrp, AUD_ESM_EVENT_AU_LINEIN2, X_EV_OP_OR) == OSR_OK);
        break;
    default:
        break;
    }

    return AUD_OK;
}

static u32 u4AudEsm_WaitAuEvent(void)
{

    EV_GRP_EVENT_T  eWaitEvent = AUD_ESM_EVENT_AU_PRI | AUD_ESM_EVENT_AU_SEC | AUD_ESM_EVENT_AU_TER;

    EV_GRP_EVENT_T eEvent = 0;

    VERIFY(x_ev_group_wait_event(_hAudEsmEvntGrp, eWaitEvent, &eEvent, X_EV_OP_OR_CONSUME) == OSR_OK);

    return ((u32)eEvent);
}

static u32 u4AudEsm_WaitLineinEvent(void)
{
    EV_GRP_EVENT_T  eWaitEvent = AUD_ESM_EVENT_AU_LINEIN1;
    EV_GRP_EVENT_T eEvent = 0;

    VERIFY(x_ev_group_wait_event(_hAudEsmEvntGrp, eWaitEvent, &eEvent, X_EV_OP_OR_CONSUME) == OSR_OK);

    return ((u32)eEvent);
}


static u32 u4AudEsm_WaitLinein2Event(void)
{
    EV_GRP_EVENT_T  eWaitEvent = AUD_ESM_EVENT_AU_LINEIN2;
    EV_GRP_EVENT_T  eEvent = 0;

    VERIFY(x_ev_group_wait_event(_hAudEsmEvntGrp, eWaitEvent, &eEvent, X_EV_OP_OR_CONSUME) == OSR_OK);

    return ((u32)eEvent);
}


void vAudEsmThreadExit(void)
{
    g_fgAudEsmThdInited = FALSE;
    i4AudEsm_SetAuEvent(PRI_DEC);
    i4AudEsm_SetAuEvent(SEC_DEC);
    i4AudEsm_SetAuEvent(TER_DEC);
    i4AudEsm_SetAuEvent(LINEIN1);
    i4AudEsm_SetAuEvent(LINEIN2);
}

//LINE_IN_NEW_FLOW
static AU_AUDIO g_rLin1Au = {AU_DATA, 0, 0, FALSE};
static AU_AUDIO g_rLin2Au = {AU_DATA, 0, 0, FALSE};
static void AudEsm_GetLin1Au(void)
{
    // Get Data Length from Line in HW.
    u32 u4Wptr = Aud_Linein_GetWPtr();
    if (u4Wptr != 0)
    {
        g_rLin1Au.ptrSAddr = g_rLin1Au.ptrEAddr;
        g_rLin1Au.ptrEAddr = u4Wptr;
        i4AudEsm_SendAU(SEC_DEC, &g_rLin1Au);
    }
    else
    {
        LOG(LOG_FEATURE, _T("AudEsm_GetLin1Au u4LineDataLen is 0.\n"));
    }

    Sleep(30);
}

static void AudEsm_GetLin2Au(void)
{
    // Get Data Length from Line in HW.
    u32 u4Wptr = Aud_Linein2_GetWPtr();

    if (0 != u4Wptr)
    {
        g_rLin2Au.ptrSAddr = g_rLin2Au.ptrEAddr;
        g_rLin2Au.ptrEAddr = u4Wptr;
        i4AudEsm_SendAU(TER_DEC, &g_rLin2Au);
    }
    else
    {
        LOG(LOG_FEATURE, _T("AudEsm_GetLin2Au u4Wptr is 0.\n"));
    }
    Sleep(30);
}


static s32 vAudSendAudinAUThread(void* pvArg)
{
    EV_GRP_EVENT_T eEvent = 0;

    while (g_fgAudEsmThdInited)
    {
        //wait line in startup.
        eEvent = u4AudEsm_WaitLineinEvent();
        if (AUD_ESM_EVENT_AU_LINEIN1 == eEvent)
        {
            LOG(LOG_CTRLF, _T("Linein1 path set event.\n"));
            while (AUD_REG_BITS_READ(REGENV_SPLIN_CTRL, BIT_STR_SPLIN_EN, BIT_NUM_SPLIN_EN))
            {
                AudEsm_GetLin1Au();
            }
        }
        else
        {
            VERIFY(0); // Get other event is error.
        }
    }

    complete_and_exit(NULL, 0);

	return 0;
}


static s32 vAudSendAudin2AUThread(void* pvArg)
{
    EV_GRP_EVENT_T eEvent = 0;

    while (g_fgAudEsmThdInited)
    {
        //wait line in startup.
        eEvent = u4AudEsm_WaitLinein2Event();
        if (AUD_ESM_EVENT_AU_LINEIN2 == eEvent)
        {
            LOG(LOG_CTRLF, _T("Linein2 path set event.\n"));
            while (AUD_REG_BITS_READ(REGENV_SPLIN_CTL_LIN2, BIT_STR_LIN2_EN, BIT_NUM_LIN2_EN))
            {
                AudEsm_GetLin2Au();
            }
        }
        else
        {
            VERIFY(0); // Get other event is error.
        }
    }

    complete_and_exit(NULL, 0);

	return 0;
}

/******************************************************************************
* Function    :        vAudEsmThread
* Description :        AU handle (pri,sec,thrid,fourth)
* Parameter   :       none
* Return      :       void
* Author      :       fei.zhu
* Time        :      2010-12-31
******************************************************************************/
static s32 vAudEsmThread(void* pvArg)
{
    while(g_fgAudEsmThdInited == TRUE)
    {
        s32 i4Ret = AUD_OK;
        EV_GRP_EVENT_T eEvent = u4AudEsm_WaitAuEvent();

        // Wait AU in event
        if (FALSE == g_fgAudEsmThdInited)
        {
            break;
        }

        if (eEvent & AUD_ESM_EVENT_AU_PRI)
        {
            i4Ret = i4AudEsm_AUProc(NULL, &g_rAudEsmContext[PRI_DEC]);
            VERIFY( i4Ret == AUD_OK );
        }

        if ((eEvent & AUD_ESM_EVENT_AU_SEC))
        {
            i4Ret = i4AudEsm_AUProc(NULL, &g_rAudEsmContext[SEC_DEC]);
            VERIFY(i4Ret == AUD_OK);
        }

        if (eEvent & AUD_ESM_EVENT_AU_TER)
        {
            i4Ret = i4AudEsm_AUProc(NULL, &g_rAudEsmContext[TER_DEC]);
            VERIFY(i4Ret == AUD_OK);
        }

    }
    i4AudEsm_Uninit();

    complete_and_exit(NULL, 0);

	return 0;
}

/******************************************************************************
* Function     :        i4AudEsm_TaskCreate
* Description :       create esm thread
* Parameter  :       none
* Return        :       AUD_OK
* Author        :       fei.zhu
* Time           :      2010-12-31
******************************************************************************/
s32 i4AudEsm_TaskCreate(void)
{
    s8 cEsmThdName[20] = "AudEsm";
    s8 cEsmEveName[20] = "AudEsmEvtGrp";
	s8 cEsmLin1AuThdName[20] = "Audin1AU";
	s8 cEsmLin2AuThdName[20] = "Audin2AU";
    s8 *pcEsmAuChunkName[TER_DEC - PRI_DEC + 1] = {
        "aud-esm-chunk-dec1",
        "aud-esm-chunk-dec2",
        "aud-esm-chunk-dec3"
    };
	u8 u1Ind = 0;

    g_fgAudEsmThdInited = TRUE;

    // create event group
    VERIFY(x_ev_group_create(&_hAudEsmEvntGrp, cEsmEveName, 0) == OSR_OK);

	for(u1Ind = PRI_DEC; u1Ind <=TER_DEC; u1Ind++)
	{
        INIT_LIST_HEAD(&_hAudEsmAuQueue[u1Ind]);
        g_hAudEsmAuChunk[u1Ind] = kmem_cache_create(pcEsmAuChunkName[u1Ind], sizeof(AU_LIST_UNIT),
				       sizeof(AU_LIST_UNIT), 0, NULL);
	}

    // create audio esm thread
    g_hAudEsmThread = kthread_create(vAudEsmThread, (void *)NULL, cEsmThdName);
	if (IS_ERR(g_hAudEsmThread)) {
		LOG(LOG_CTRLF, TEXT("[i4AudEsm_TaskCreate]vAudEsmThread create fail \n"));
		g_hAudEsmThread = NULL;
		return AUD_FAIL;
	}
    else
    {
        struct sched_param param;
        s32 ret;

        param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hAudEsmThread, SCHED_RR, &param);
        ASSERT(ret == 0);
    }
	wake_up_process(g_hAudEsmThread);

    // create audio line1 thread
    g_hAudLineinThd = kthread_create(vAudSendAudinAUThread, (void *)NULL, cEsmLin1AuThdName);
	if (IS_ERR(g_hAudLineinThd)) {
		LOG(LOG_CTRLF, TEXT("[i4AudEsm_TaskCreate]vAudSendAudinAUThread create fail \n"));
		g_hAudLineinThd = NULL;
		return AUD_FAIL;
	}
    else
    {
        struct sched_param param;
        s32 ret;

        param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hAudLineinThd, SCHED_RR, &param);
        ASSERT(ret == 0);
    }
	wake_up_process(g_hAudLineinThd);

    // create audio line2 thread
    g_hAudLinein2Thd = kthread_create(vAudSendAudin2AUThread, (void *)NULL, cEsmLin2AuThdName);
	if (IS_ERR(g_hAudLinein2Thd)) {
		LOG(LOG_CTRLF, TEXT("[i4AudEsm_TaskCreate]vAudSendAudin2AUThread create fail \n"));
		g_hAudLinein2Thd = NULL;
		return AUD_FAIL;
	}
    else
    {
        struct sched_param param;
        s32 ret;

        param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hAudLinein2Thd, SCHED_RR, &param);
        ASSERT(ret == 0);
    }
	wake_up_process(g_hAudLinein2Thd);

    //Jianhua Feng deal EOS
    g_hAudFlushEvent = x_event_create(NULL, FALSE, FALSE, TEXT("AUDEOS"));
    g_hAudDecReady = x_event_create(NULL, FALSE, FALSE, TEXT("AUDDECREADY"));

    //the cmd event used by all decoder.
    //g_hAudSetCmd[PRI_DEC] = x_event_create(NULL, FALSE, FALSE, TEXT("AUDSETCMD"));
    //g_hAudSetCmd[SEC_DEC] = x_event_create(NULL, FALSE, FALSE, TEXT("AUDDEC2SETCMD"));
    //g_hAudSetCmd[TER_DEC] = x_event_create(NULL, FALSE, FALSE, TEXT("AUDDEC3SETCMD"));
    return AUD_OK;
}


/******************************************************************************
* Function : i4AudEsm_FIFOSetProc
* Description : Reset all conditions
* Parameter :
* Return :
* Note :
******************************************************************************/
s32 i4AudEsm_FifoSetProc(void *pvData, AUD_ESM_CONTEXT_T *pContext)
{
    s32 i4Ret = AUD_OK;
    u32 u4Start = MT3360_AFIFO_VA;
    u32 u4End = MT3360_AFIFO_VA + AUD_AFIFO_TOTAL_SIZE;

    if (PRI_DEC == pContext->u2AudDrvCompId)
    {
        //The Primary decoder
        u4Start = MT3360_AFIFO_VA;
        u4End = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE;
    }
    else if (SEC_DEC == pContext->u2AudDrvCompId)
    {
        // A2DP
        if (pContext->eType == AUD_DEC_FMT_A2DP)
        {
            u4Start = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE;
        }
        else
        {
            // Line in.
            u4Start = MT3360_AFIFO_VA + (AUD_AFIFO_PRIMARY_SIZE+ (AUD_AFIFO_AVIN_A2DP_SIZE*2/3));
        }
        u4End   = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE + AUD_AFIFO_AVIN_A2DP_SIZE;
    }
    else if (TER_DEC == pContext->u2AudDrvCompId)
    {
        // The Fifth decoder
        u4Start = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE + AUD_AFIFO_AVIN_A2DP_SIZE;
        u4End   = u4Start + AUD_AFIFO_AVIN2_SIZE;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("ESM unsupport Dec = 0x%x.\n"), pContext->u2AudDrvCompId);
        return (AUD_FAIL);
    }

    pContext->u4AfifoSA = u4Start;
    pContext->u4AfifoEA = u4End;

    LOG(LOG_DATAF, TEXT("Set AFifo SA = 0x%X, EA = 0x%X, for decoder [%d]\n"),
        pContext->u4AfifoSA, pContext->u4AfifoEA, pContext->u2AudDrvCompId);

    vAudHalSetBufStartAddr((u8)pContext->u2AudDrvCompId, DSP_AFIFO, u4Start);
    vAudHalSetBufEndAddr((u8)pContext->u2AudDrvCompId, DSP_AFIFO, u4End);

    vAudEsm_StopInit(pContext->u2AudDrvCompId);

    if((PRI_DEC == pContext->u2AudDrvCompId) ||
       (SEC_DEC == pContext->u2AudDrvCompId)||
       (TER_DEC == pContext->u2AudDrvCompId))
    {
        //Reset cgms info
        if(uReadDspShmBYTE(B_CGMS_KEEP_FLAG) == 1)
        {
            LOG(LOG_DATAF, TEXT("[AUD]Keep cgms info!\n"));
            vWriteShmUINT8(B_CGMS_KEEP_FLAG,0);
        }
        else
        {
            LOG(LOG_DATAF, TEXT("[AUD]Clear cgms info!\n"));
            DspSetCgmsInfo(0);
        }
    }

    return(i4Ret);
}

/******************************************************************************
* Function : i4AudEsm_FIFOFlushProc
* Description : Reset RP/WP and LPCM flags
* Parameter :
* Return :
* Note :
******************************************************************************/
s32 i4AudEsm_FifoFlushProc(void *pvData, AUD_ESM_CONTEXT_T *pContext)
{
    vAudEsm_StopInit(pContext->u2AudDrvCompId);

    return(AUD_OK);
}

/******************************************************************************
* Function : i4AudEsm_FifoDestoryProc
* Description : ESM destory procedure
* Parameter :
* Return :
* Note :
******************************************************************************/
s32 i4AudEsm_FifoDestoryProc(void *pvData, AUD_ESM_CONTEXT_T *pContext)
{
    return(AUD_OK);
}

/******************************************************************************
* Function : vAudEsm_DataIn_Callback
* Description :
* Parameter :
* Return :
* Note : Need to consider reentrant of multiple packet filter threads
******************************************************************************/
#if 0  //for warning cancel
static void vAudEsm_DataIn_Callback(ES_CBEVENT eEvent, void *pvData, void *pvPrivate)
{
    s32 i4Ret = AUD_OK;
    AUD_ESM_CONTEXT_T *pContext = (AUD_ESM_CONTEXT_T *)pvPrivate;

    switch(eEvent)
    {
        //set au event esmtask get event
    case CBE_AU_IN:
        //i4Ret = i4AudEsm_AUProc(pvData, pContext);
        i4Ret = i4AudEsm_SetAuEvent((u8)pContext->u2AudDrvCompId);
        break;
    case CBE_FIFO_SET:
        i4Ret = i4AudEsm_FifoSetProc(pvData, pContext);
        break;
    case CBE_FIFO_FLUSH:
        i4Ret = i4AudEsm_FifoFlushProc(pvData, pContext);
        break;
    case CBE_FIFO_DESTROY:
        i4Ret = i4AudEsm_FifoDestoryProc(pvData, pContext);
        break;
    default:
        break;
    }

    if( i4Ret != AUD_OK )
    {
        LOG(LOG_ERROR, TEXT("[Audio] Esm call back function error, 0x%X\n"), i4Ret);
    }

}
#endif

/****************************************************************************
*    Function : i4AudEsm_Connect
* Description :
*   Parameter : u2ADRV_Comp_Id is the audio driver component id
*               eType specifies the packet filter type
*               u2PktFltr_Comp_Id is the packet filter component id
*   Return    :
*   Note      : The function will be called N times if N elementary streams are instantiated.
*               It registers corresponding AudEsm instance (g_rAudEsmContext[u2ADRV_Comp_Id]) to ESM
*               so that the corresponding interface will be carried in vAudEsm_DataIn_Callback.
****************************************************************************/
s32 i4AudEsm_Connect(u16 u2DecId)
{
    s32 i4Ret = AUD_OK;
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u2DecId];

    VERIFY(u2DecId <= TER_DEC);
    LOG(LOG_CTRLF, TEXT("[ESM]Connect with id = %d.\n"), u2DecId);

    if(AUD_ESM_DISCNT == pContext->m_u1EsmState)
    {
        //init esm context info
        pContext->u2AudDrvCompId = u2DecId;
        pContext->u4AfifoSA = 0;
        pContext->u4AfifoEA = 0xFFFFFFFF;
        pContext->u4CurWrIdx = 0;
        pContext->u4Handle = 0;
        pContext->m_u1EsmState = AUD_ESM_CNT;

        //Create timer
        vAudEsm_TimerCreate((u8)u2DecId);
        //Enable timer
        i4AudEsm_SetEnabled(TRUE, (u8)u2DecId);
        //Fifo set
        i4AudEsm_FifoSetProc(NULL, pContext);
        //reset the playback time calc param
        vAudEsm_InitTotalPBTimeCount();

        g_u4AuCount = 0;  //for debug

        //primary decoder used next event.
        if(PRI_DEC == u2DecId)
        {
            if (g_hAudDecReady)
            {
                x_event_reset(g_hAudDecReady);
            }

            if (g_hAudFlushEvent)
            {
                x_event_reset(g_hAudFlushEvent);
            }
        }
    }
    else
    {
        LOG(LOG_FAIL, TEXT("Dec(%d)esm already connected.\n"), u2DecId);
        i4Ret = AUD_FAIL;
    }

    return (i4Ret);
}


s32 i4AudEsm_Disconnect(u16 u2DecId)
{
    s32 i4Ret = AUD_OK;
    AUD_ESM_CONTEXT_T *pContext = NULL;

    VERIFY(u2DecId <= TER_DEC);
    LOG(LOG_CTRLF, TEXT("[ESM]Disconnect with id = %d\n"), u2DecId);

    pContext = &g_rAudEsmContext[u2DecId];
    if(AUD_ESM_CNT == pContext->m_u1EsmState)
    {
        pContext->m_u1EsmState = AUD_ESM_DISCNT;
        //Stop timer
        i4AudEsm_SetEnabled(FALSE, (u8)u2DecId);
        //Delete timer
        vAudEsm_TimerDelete((u8)u2DecId);

        pContext->u4Handle = ESM_INVALID_HANDLE;
        pContext->eType = 0;

        vAudHalResetBufWPtr(u2DecId);

        m_aud_afifo_info->posInfo[u2DecId].ptrAfifoRPtr = 0;
        m_aud_afifo_info->posInfo[u2DecId].ptrAfifoWPtr = 0;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("Dec(%d)esm already disconnected.\n"), u2DecId);
        i4Ret = AUD_FAIL;
    }

    return (i4Ret);
}

/****************************************************************************
*    Function : i4AudEsm_Init
* Description :
*   Parameter :
*   Return    :
****************************************************************************/
s32 i4AudEsm_Init(void)
{
    u8 u1DecId = 0;
    AUD_ESM_CONTEXT_T *pContext = NULL;

    memset(g_rAudEsmContext, 0x0, sizeof(g_rAudEsmContext));
    for (u1DecId = PRI_DEC; u1DecId <= TER_DEC; u1DecId++)
    {
        pContext = &g_rAudEsmContext[u1DecId];
        pContext->u4Handle = ESM_INVALID_HANDLE;
        pContext->fgFirstAUArrive = FALSE;
        pContext->m_u1EsmState = AUD_ESM_DISCNT;
    }
	
    memset(&rAud_AFifo_PosInfo, 0, sizeof(rAud_AFifo_PosInfo));
    m_aud_afifo_info = &rAud_AFifo_PosInfo;
    m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoSA = MT3360_AFIFO_PA;
    m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoEA = MT3360_AFIFO_PA + AUD_AFIFO_PRIMARY_SIZE;
    m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoVirSA = MT3360_AFIFO_VA;
    m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoVirEA = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE;

   return 0;
}

/****************************************************************************
*   Function : i4AudEsm_unInit
*   Description :
*   Parameter :
*   Return    :
****************************************************************************/
void i4AudEsm_Uninit(void)
{
    //release resource
    VERIFY(x_ev_group_delete(_hAudEsmEvntGrp) == OSR_OK);

    DeleteCriticalSection(&esm_send_au_lock);
}


/****************************************************************************
*    Function : vAudEsm_StopInit
* Description : This API should be called when dsp is stopped to initial some settings
*   Parameter :
*   Return    :
****************************************************************************/
static void vAudEsm_StopInit(u16 u2DecId)
{
    u32 u4Write = 0;
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u2DecId];

    VERIFY((u2DecId == PRI_DEC) ||(u2DecId == SEC_DEC)||(u2DecId == TER_DEC));

    u4Write = pContext->u4AfifoSA;

    pContext->u4AfifoWPtr = u4Write;
    pContext->u4AfifoRPtr = u4Write;
    vAudHalSetBufWPtr((u8) pContext->u2AudDrvCompId, DSP_AFIFO, u4Write);
    LOG(LOG_FEATURE, TEXT("Reset dec[%d] afifo write pointer 0x%X,.\n"), u2DecId, u4Write);

    // reset context settings
    pContext->fgFirstAUArrive = FALSE;
    pContext->u4LastIteratedAUIdx = 0;
    pContext->u4AfifoWPtrForApp = u4Write;
    pContext->u4ReadCnt = 0;

    m_aud_afifo_info->posInfo[u2DecId].ptrAfifoRPtr = 0;
    m_aud_afifo_info->posInfo[u2DecId].ptrAfifoWPtr = 0;

    //initial PTS Queue write pointer and AFIFO Loop
    AudEsmInitPtsQueue((u8)pContext->u2AudDrvCompId);

    g_u4AudPriICBId = 0;
    g_fgEnableSkipData = FALSE;
    g_fgAVSyncSkipToend = FALSE;
    g_u1SkipDataMode = 0xFF;
    g_u4SkipDataAddr = 0xFFFFFFFF;
    g_fgAudEosState = FALSE;
    g_fgAudioEosDone = FALSE;
    g_fgAudIgnoreAU = FALSE;
}

s32 i4AudEsm_Notify_Play(u16 u2ADRV_Comp_Id)
{
    //CRIT_STATE_T old_state;
    AUD_ESM_CONTEXT_T *pContext;
    u32 flags = 0;

    VERIFY(u2ADRV_Comp_Id < (TER_DEC - PRI_DEC + 1));

    pContext = &g_rAudEsmContext[u2ADRV_Comp_Id];

    //old_state = x_crit_start();
    ENTERCRITICALSECTION(&esm_notify_state_lock, flags);
    pContext->fgIsPlay = TRUE;
    LEAVECRITICALSECTION(&esm_notify_state_lock, flags);
    //x_crit_end(old_state);

    return 0;
}

s32 i4AudEsm_Notify_Stop(u16 u2ADRV_Comp_Id)
{
    //CRIT_STATE_T old_state;
    AUD_ESM_CONTEXT_T *pContext;
    u32 flags = 0;

    VERIFY(u2ADRV_Comp_Id < (TER_DEC - PRI_DEC + 1));
    pContext = &g_rAudEsmContext[u2ADRV_Comp_Id];

    //old_state = x_crit_start();
    ENTERCRITICALSECTION(&esm_notify_state_lock, flags);
    pContext->fgIsPlay = FALSE;
    LEAVECRITICALSECTION(&esm_notify_state_lock, flags);

    //x_crit_end(old_state);
    return 0;
}


s32 i4AudEsm_SendAU(u8 U1DecId, AU_AUDIO *pAu)
{
    AU_LIST_UNIT *pNewAuUnit = NULL;
    u32 flags = 0;
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[U1DecId];

    ASSERT(pAu);

    if(g_fgPrintAU)
    {
        LOG(LOG_DATAF, TEXT("[ESM] ===== Receive New AU ======\n"));
        vAudEsm_PrintAu(pContext, pAu);
    }

    ENTERCRITICALSECTION(&esm_send_au_lock, flags);

    pNewAuUnit = kmem_cache_alloc(g_hAudEsmAuChunk[U1DecId], GFP_KERNEL | __GFP_ZERO);
    pNewAuUnit->rAu = *pAu;

    if (pAu->eAuType == AU_DATA)
    {
        pContext->u4AfifoWPtrForApp = pAu->ptrEAddr + pContext->u4AfifoSA;
    }

    list_add_tail(&(pNewAuUnit->au_queue), &_hAudEsmAuQueue[U1DecId]);

    LEAVECRITICALSECTION(&esm_send_au_lock, flags);

    i4AudEsm_SetAuEvent(U1DecId);

    return 0;
}

bool AudEsm_SendBufferInfo(u8 u1DecId, AUD_SEND_BUF_INFO *pBufInfo)
{
    AUD_ESM_CONTEXT_T *pEsmContext = &g_rAudEsmContext[u1DecId];
    AU_AUDIO au;
    u32 u4Tmp;
    u32 u4WPtr = pEsmContext->u4AfifoWPtrForApp;

    if (0 == pBufInfo->ptrBufAddr || 0 == pBufInfo->u4BufLen
        || (u4AudEsm_GetSpareBufLen(pEsmContext) <= pBufInfo->u4BufLen))
    {
        LOG(LOG_FAIL, TEXT("AudEsm_SendBufferInfo fail, BufAddr 0x%lx, BufLen 0x%x, AFIFOSpareBufLen 0x%x\n"),
            pBufInfo->ptrBufAddr, pBufInfo->u4BufLen, u4AudEsm_GetSpareBufLen(pEsmContext));
        return FALSE;
    }

    au.eAuType = AU_DATA;

    if (pBufInfo->u4BufLen < pEsmContext->u4AfifoEA - u4WPtr)
    {
        memcpy((void *)u4WPtr, (void *)pBufInfo->ptrBufAddr, pBufInfo->u4BufLen);
        au.ptrSAddr = u4WPtr - pEsmContext->u4AfifoSA;
        au.ptrEAddr = au.ptrSAddr + pBufInfo->u4BufLen;
    }
    else
    {
        u4Tmp = pEsmContext->u4AfifoEA - u4WPtr;
        memcpy((void *)u4WPtr, (void *)pBufInfo->ptrBufAddr, u4Tmp);
        pBufInfo->u4BufLen -= u4Tmp;
        if (pBufInfo->u4BufLen)
            memcpy((void *)pEsmContext->u4AfifoSA, (u8 *)pBufInfo->ptrBufAddr + u4Tmp, pBufInfo->u4BufLen);

        au.ptrSAddr = u4WPtr - pEsmContext->u4AfifoSA;
        au.ptrEAddr = pBufInfo->u4BufLen;
    }

    if(((s64)pBufInfo->u8Pts > (s64)g_u8Pts)&&((s64)(pBufInfo->u8Pts)!= INVALID_TIMESTAMP))
    {
        AudEsmSetPtsToQueue(0,(au.ptrSAddr + pEsmContext->u4AfifoSA),
            ((u32)((pBufInfo->u8Pts)>>1)));
        g_u8Pts = pBufInfo->u8Pts;
        g_u8STC = u8Aud_GetSTC();
    }

    if (i4AudEsm_SendAU(u1DecId,&au))
    {
        return FALSE;
    }

    return TRUE;
}

bool AudEsm_SendEsmInfo(u8 u1DecId, ESM_IO_BUF_INFO *pEsmInfo)
{
    u32 u4SA, u4EA, u4AfifoSA, u4AfifoEA;
    AUD_ESM_CONTEXT_T *pEsmContext = NULL;
    u64 u8Pts = 0;

    if (u1DecId > TER_DEC)
    {
        LOG(LOG_FAIL, TEXT("AudEsm_SendEsmInfo u1DecId error\t\n"));
        return (FALSE);
    }
    pEsmContext = &g_rAudEsmContext[u1DecId];

    u4AfifoSA = pEsmContext->u4AfifoSA;
    u4AfifoEA = pEsmContext->u4AfifoEA;

    u4SA = u4AfifoSA + pEsmInfo->rAU.rAudioAU.ptrSAddr;
    u4EA = u4AfifoSA + pEsmInfo->rAU.rAudioAU.ptrEAddr;

    u8Pts = pEsmInfo->rAU.rAudioAU.rAUInfo.rInfo.u8Pts;
    if(((s64)u8Pts > (s64)g_u8Pts) && ((s64)u8Pts != INVALID_TIMESTAMP))
    {
        AudEsmSetPtsToQueue(0,u4SA,((u32)(u8Pts>>1)));
        g_u8Pts = u8Pts;
        g_u8STC = u8Aud_GetSTC();
    }

    if(u4SA < u4EA)
    {
        if (i4AudEsm_SendBuffer(u1DecId,(void *)u4SA, u4EA - u4SA))
            return FALSE;
    }
    else
    {
        if (i4AudEsm_SendBuffer(u1DecId, (void *)u4SA, u4AfifoEA - u4SA + u4EA - u4AfifoSA))
            return FALSE;
    }
    return TRUE;
}

static s32 x_physical_to_virtual(u32 u4Physical, u32 *pu4Virtual, u32 u4Size)
{
#ifndef __linux__
    void * hDst = (void *)GetDirectCallerProcessId();
    void * hSrc = (void *)GetCurrentProcessId();
    u32 u4Ret;

    u4Ret = (u32)VirtualAllocEx(hDst, NULL, u4Size, MEM_RESERVE, PAGE_NOACCESS);
    if (!u4Ret)
        return -1;

    if (!VirtualCopyEx(hDst, (void *)u4Ret, hSrc, (void *)(u4Physical >> 8), u4Size,
        PAGE_NOCACHE | PAGE_READWRITE | PAGE_PHYSICAL))
    {
        VirtualFreeEx(hDst, (void *)u4Ret, 0, MEM_RELEASE);
        return -2;
    }

    *pu4Virtual = u4Ret;
#endif // #ifndef __linux__
    return 0;
}

bool AudEsm_GetAfifoInfoVirtual(u8 u1DecId, AUDIO_BUF_INFO *pInfo)
{
    u32 u4SA, u4Size;
    s32 i4Ret;

    i4AudEsm_GetAudioBuffer(u1DecId, pInfo);
    u4SA = pInfo->ptrFifoSA;
    u4Size = pInfo->ptrFifoEA - pInfo->ptrFifoSA;

    i4Ret = x_physical_to_virtual(u4SA, &u4SA, u4Size);
    if(0 != i4Ret)
    {
        LOG(LOG_CTRLF, TEXT("x_physical_to_virtual ret=%d\n"), i4Ret);
        return FALSE;
    }
    else
    {
        pInfo->u4WritePointer = pInfo->u4WritePointer - pInfo->ptrFifoSA + u4SA;
        pInfo->ptrFifoEA = pInfo->ptrFifoEA - pInfo->ptrFifoSA + u4SA;
        pInfo->ptrFifoSA = u4SA;
    }
    return TRUE;
}



u32 u4AudEsm_GetSpareBufLen(AUD_ESM_CONTEXT_T *pContext)
{
    u32 u4Ret;

    ASSERT(pContext);

    if (pContext->u4AfifoWPtrForApp > pContext->u4AfifoRPtr)
    {
        u4Ret = pContext->u4AfifoEA - pContext->u4AfifoWPtrForApp + pContext->u4AfifoRPtr - pContext->u4AfifoSA;
    }
    else if (pContext->u4AfifoWPtrForApp < pContext->u4AfifoRPtr)
    {
        u4Ret = pContext->u4AfifoRPtr - pContext->u4AfifoWPtrForApp;
    }
    else
    {
        u4Ret = pContext->u4AfifoEA - pContext->u4AfifoSA;
    }

    return u4Ret;
}


s32 i4AudEsm_GetAudioBuffer(u8 u1DecId,AUDIO_BUF_INFO *pInfo)
{
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u1DecId];
    ASSERT(pInfo);

    pInfo->ptrFifoSA = AFIFO_PHYSICAL(pContext->u4AfifoSA);
    pInfo->ptrFifoEA = AFIFO_PHYSICAL(pContext->u4AfifoEA);
    pInfo->u4WritePointer = AFIFO_PHYSICAL(pContext->u4AfifoWPtrForApp);
    pInfo->u4Len = u4AudEsm_GetSpareBufLen(pContext);

    return 0;
}

s32 i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_TYPE_T eFifoType, AUD_POSINFO_T *pAudPos)
{
    if (eFifoType >= AUD_FIFO_UNDEF)
    {
        return (-1);
    }

    if (AUD_FIFO_RPIMARY == eFifoType)
    {
        pAudPos->ptrAfifoSA = MT3360_AFIFO_PA;
        pAudPos->ptrAfifoEA = MT3360_AFIFO_PA + AUD_AFIFO_PRIMARY_SIZE;

        pAudPos->ptrAfifoVirSA = MT3360_AFIFO_VA;
        pAudPos->ptrAfifoVirEA = MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE;

        pAudPos->ptrAfifoRPtr = m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoRPtr;
    }
    else if (AUD_FIFO_HDMI_RX == eFifoType)
    {
        pAudPos->ptrAfifoSA = AUD_HDMI_RX_BUF_PA;
        pAudPos->ptrAfifoEA = AUD_HDMI_RX_BUF_PA + AUD_HDMI_RX_BUF_SIZE;

        pAudPos->ptrAfifoVirSA = AUD_HDMI_RX_BUF_VA;
        pAudPos->ptrAfifoVirEA = AUD_HDMI_RX_BUF_VA + AUD_HDMI_RX_BUF_SIZE;
    } else {
		    return (-1);
    }

    return 0;
}

EXPORT_SYMBOL(i4AudEsm_GetAudioCodecFifoInfo);

u32 u4AudEsm_GetReadCount(u8  u1DecId)
{
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u1DecId];
    return pContext->u4ReadCnt;
}


s32 i4AudEsm_SendBuffer(u8 u1DecId,void *buf, u32 u4Size)
{
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u1DecId];
    AU_AUDIO au;
    u32 u4WPtr = pContext->u4AfifoWPtrForApp;

    if (!buf || 0 == u4Size)
    {
        LOG(LOG_FAIL,     TEXT("[AUD]i4AudEsm_SendBuffer error, buf is 0x%x, u4Size is 0x%x\n"), (u32)buf, u4Size);
        return (-1);
    }

    if (u4AudEsm_GetSpareBufLen(pContext) < u4Size)
    {
        LOG(LOG_FAIL, TEXT("[AUD]i4AudEsm_SendBuffer error, SpareBufLen is less, SpareBufLen %d, u4Size is %d\n"),
            u4AudEsm_GetSpareBufLen(pContext), u4Size);
        return (-2);
    }

    au.eAuType = AU_DATA;

    u4WPtr = (u32)buf;
    if (u4Size <= pContext->u4AfifoEA - u4WPtr)
    {
        au.ptrSAddr = (u32)buf - pContext->u4AfifoSA;
        au.ptrEAddr = u4Size + au.ptrSAddr;
    }
    else
    {
        au.ptrSAddr = (u32)buf - pContext->u4AfifoSA;
        au.ptrEAddr = u4Size + (u32)buf - pContext->u4AfifoEA;
    }

    g_u4AuCount++;
    return i4AudEsm_SendAU(u1DecId,&au);
}

u64 u8Aud_GetSTC(void)
{
    u64  u8Time = 0;

    if(!STC_HalGetTime(0, &u8Time))
    {
        LOG(LOG_FAIL, TEXT("GetHWClockTime, Failed to get STC.\n"));
    }

    return (u8Time);
}


//=================================================================================
//                DSP PTS Queue control with audio driver
//=================================================================================
/***************************************************************************
 Function    : AudEsmInitPtsQueue
 Description : init pts queue
 Parameter   : u1DecId: decoder id
 Return      :
***************************************************************************/
static void AudEsmInitPtsQueue(u8 u1DecId)
{
    AUD_PTS_QUEUE_INFO_T *prPtsInfo = NULL;

    if (PRI_DEC == u1DecId)
    {
        prPtsInfo = &g_rDspPTSQueueInfo[u1DecId];
        prPtsInfo->u1AfifoLoop=0;
        prPtsInfo->u4LastPtsAddr = 0;
        prPtsInfo->fgFirstPTS = TRUE;
        prPtsInfo->u1SkipTempCnt= 0;
        prPtsInfo->u1SkipPTSCnt=1;

        g_u4PtsQueueErrCnt[u1DecId]=0;

        DspSetPtsWptr(0);
    }
}

/***************************************************************************
 Function    : AudEsmSetPtsToQueue
 Description : Put PTS address and value into PTS Queue
 Parameter   : u4PtsAddr, u4PtsVal
 Return      :
***************************************************************************/
static void AudEsmSetPtsToQueue(u8 u1DecId,u32 u4PtsAddr, u32 u4PtsVal)
{
    u32 u4FifoBase = u4AudHalGetAFIFOBaseAddr(u1DecId);//b1000000
    AUD_PTS_QUEUE_INFO_T *prPtsInfo = &g_rDspPTSQueueInfo[u1DecId];
    if (u1DecId != PRI_DEC)
    {
        return;
    }

    //AFIFO memory aligment issue --
    u4PtsAddr = AFIFO_PHYSICAL(u4PtsAddr);
    u4PtsAddr-= (AFIFO_PHYSICAL(u4FifoBase)) & 0x7FF00000;

    // Skip PTS since common code can not support more than one PTS per frame in this stage
    if(prPtsInfo->u1SkipTempCnt == 0)
    {
        u8 u1DecType = 0;
        DspGetDec1StrType(&u1DecType);
        if(u1DecType == TRUE_HD_STREAM)
        {
            // TrueHD    : Set skip Cnt = 7 for (8 pts per frame)
            prPtsInfo->u1SkipTempCnt = 7;
        }
        else if(u1DecType == MP3_STREAM)
        {
            // HJWei, modify for DivX-HD certification
            prPtsInfo->u1SkipTempCnt = 0;
        }
        else
        {
            prPtsInfo->u1SkipTempCnt = prPtsInfo->u1SkipPTSCnt;
        }
    }
    else
    {
        prPtsInfo->u1SkipTempCnt--;
        return;
    }

	DspGetPtsQueueInfo(prPtsInfo);

    //Check if Space is enough to put data into PTS Queue
    if(((prPtsInfo->u4WritePtr+1) & (prPtsInfo->u4QueueSize-1)) == prPtsInfo->u4ReadPtr)
    {
        VERIFY(u1DecId < 2);
        g_u4PtsQueueErrCnt[u1DecId]++;
        return;
    }

    //Check current u4PtsAddr in AFIFO
    if(prPtsInfo->u4LastPtsAddr > u4PtsAddr)
    {
        prPtsInfo->u1AfifoLoop++;
    }
    prPtsInfo->u4LastPtsAddr = u4PtsAddr;
	DspSetPtsNormalMode(prPtsInfo, u4PtsAddr, u4PtsVal);

    if (prPtsInfo->fgFirstPTS)
    {
        DspSetFirstPtsValue(u4PtsVal);
        vAudSaveFirstAudPts(u4PtsVal, u1DecId);
        prPtsInfo->fgFirstPTS = FALSE;
    }
    else
    {
        if (u4PtsVal < prPtsInfo->u4LastPtsValue)
        {
            LOG(5, TEXT("[AUD]PTS ERROR: Primary get a PTS smaller than latest PTS\n"));
            LOG(5, TEXT("[AUD]PTS ERROR: Primary AUD Got PTS high = 0x%X ; PTS low= 0x%X\n"),
				(u4PtsVal& 0xFFFF0000)>>16,(u4PtsVal& 0xFFFF));
            LOG(5, TEXT("[AUD]PTS ERROR: Primary AUD Latest PTS high = 0x%X ; PTS low= 0x%X\n"),
				((prPtsInfo->u4LastPtsValue) & 0xFFFF0000)>>16,((prPtsInfo->u4LastPtsValue) & 0xFFFF));
        }
    }
    prPtsInfo->u4LastPtsValue = u4PtsVal;

    //Update Write pointer
    if((prPtsInfo->u4WritePtr+1) == prPtsInfo->u4QueueSize)
    {
        prPtsInfo->u4WritePtr = 0;
    }
    else
    {
        prPtsInfo->u4WritePtr++;
    }
    DspSetPtsWptr(prPtsInfo->u4WritePtr);
}


void vRstRptr2KeepAfifoNotFull(u16 u2DecId)
{
    AUD_ESM_CONTEXT_T *pContext = &g_rAudEsmContext[u2DecId];
    u32 u4AfifoSize = pContext->u4AfifoEA - pContext->u4AfifoSA;
    u32 u4AfifoWPtr = pContext->u4AfifoWPtrForApp - pContext->u4AfifoSA;

    LOG(0, TEXT("*********befere : R(0x%x)W(0x%x)*********\n"), \
                                        m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoRPtr,  \
                                        u4AfifoWPtr);

    //Stop timer
    i4AudEsm_SetEnabled(FALSE, (u8)u2DecId);
    //Delete timer
    vAudEsm_TimerDelete((u8)u2DecId);

    if(u4AfifoWPtr > 0x400)
        m_aud_afifo_info->posInfo[u2DecId].ptrAfifoRPtr = u4AfifoWPtr - 0x400;
    else
        m_aud_afifo_info->posInfo[u2DecId].ptrAfifoRPtr = u4AfifoSize - 0x400;


    LOG(0, TEXT("*********after  : R(0x%x)W(0x%x)*********\n"), \
                                        m_aud_afifo_info->posInfo[PRI_DEC].ptrAfifoRPtr,  \
                                        u4AfifoWPtr);
    return;
}


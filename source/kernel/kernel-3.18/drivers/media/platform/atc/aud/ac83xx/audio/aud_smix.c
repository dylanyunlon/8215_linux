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

#include "aud_if.h"
#include "aud_oal.h"
#include "aud_smix.h"
#include "aud_debug.h"

#include "DspUop.h"
#include "DspFunc.h"

#include <linux/slab.h>
#include <linux/list.h>
//#include "drv_thread.h"

/*-----------------------------------------------------------------------------
                    Local structures and enumerations
-----------------------------------------------------------------------------*/
#define BYTE_PER_SAMPLE 4     // 16bit 2channel
#define MIN_BUF_16      0x40  // 64 sample
#define BUF_DATA_AVAIL  0x10  // DWORD, avoid write pointer avail read pointer

/*-----------------------------------------------------------------------------
                       Local variable
-----------------------------------------------------------------------------*/
AUD_SMIX_CONTEXT_T g_rAudSmixContext;
u32 g_u4totalWriteLen = 0;

extern uintptr_t g_u4DspDramBuf[];

/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
extern void WriteDspMicDram(u32 addr, u32 val);
void AudSmixSetState(AUD_SMIX_STATE_T eStatus);
//s32 write_file(const s8 *filename, const s8 *wbuf, u32 length);

/******************************************************************************
* Function     :  AudSmixInit
* Description :  audio smix interface: set software init
* Parameter   :  u1ctrl: 0: off, other on
* Return        :
******************************************************************************/
s32 AudSmixInit(void)
{
    AUD_SMIX_CONTEXT_T* pContext = &g_rAudSmixContext;
    u32 u4fifoLen = RISC_SW_MIX_CH_SIZE;
    u32 u4SA      = RISC_SW_MIX_BASE_ADDR;

    //semaphore init
    sema_init(&pContext->Cmdlock, 1);
    sema_init(&pContext->AFifolock, 1);

    //afifo pointer init
    pContext->AFifo.u4AfifoSA = u4SA;
    pContext->AFifo.u4AfifoEA = u4SA + u4fifoLen;

    pContext->AFifo.u4AfifoWPtr = u4SA;
    pContext->AFifo.u4AfifoRPtr = u4SA;

    //dsp dram fifo use offset
    DspSetSwMixWptr(pContext->AFifo.u4AfifoWPtr - u4SA);

    //status init
    AudSmixSetState(AUD_SMIX_INITED);

    return (AUD_OK);
}

/******************************************************************************
* Function     :  AudSmixSetState
* Description :  audio smix interface: set state
* Parameter   :  u1ctrl: 0: off, other on
* Return        :
******************************************************************************/
void AudSmixSetState(AUD_SMIX_STATE_T eStatus)
{
    g_rAudSmixContext.Status = eStatus;
}

/******************************************************************************
* Function     :  AudSmixGetState
* Description :  audio smix interface: get state
* Parameter   :  u1ctrl: 0: off, other on
* Return        :
******************************************************************************/
static inline AUD_SMIX_STATE_T AudSmixGetState(void)
{
    return (g_rAudSmixContext.Status);
}

/******************************************************************************
* Function     :  AudSmixSetMwCtrl
* Description :  audio smix interface: set software mix start/stop
* Parameter   :  ctrl: 0: stop, 1 start
* Return        :
******************************************************************************/
bool AudSmixSetMwCtrl(AUD_MEDIA_TYPE* pType)
{
    bool  fgRet = TRUE;
    AUD_SMIX_STATE_T eState = AUD_SMIX_UNINIT;
    AUD_SMIX_CONTEXT_T* pContext = &g_rAudSmixContext;

    LOG(LOG_CTRLF, _T("%s command type(%d), Source(%d), Output(%d).\r\n"),
        __func__, pType->eMediaCtrl, pType->eMediaSrc, pType->eMediaOut);

    if(pType->eMediaOut != AUD_MEDIA_OUT_FRONT){
        LOG(LOG_INFO, _T("%s smix only support front out.\r\n"),__func__);
    }

    down(&pContext->Cmdlock);

    eState = AudSmixGetState();
    if((AUD_SMIX_STARTED == eState&& AUD_MEDIA_ON == pType->eMediaCtrl)||
        (AUD_SMIX_STOPED == eState&& AUD_MEDIA_OFF == pType->eMediaCtrl)){
        LOG(LOG_CTRLF, _T("%s state is %d, ctrl cmd is %d.\r\n"),__func__, eState, pType->eMediaCtrl);
        up(&pContext->Cmdlock);
        return TRUE;
    }

    switch(pType->eMediaCtrl)
    {
    case AUD_MEDIA_ON:
        // 1. set software mix media type on
        if(FALSE == fgAdspSetMediaType(*pType)){
            LOG(LOG_FAIL, _T("%s play set media type error.\r\n"),__func__);
            up(&pContext->Cmdlock);
            return FALSE;
        }
        // 2. set dspa software mix connect
		vSendADSPCmd(UOP_DSP_SMIX_ON);
        // 3. set software mix path status started
        AudSmixSetState(AUD_SMIX_STARTED);
        break;

    case AUD_MEDIA_OFF:
        // 1. set software mix path status stoped
        AudSmixSetState(AUD_SMIX_STOPED);

        // 2. set dspa software mix disconnect
		vSendADSPCmd(UOP_DSP_SMIX_OFF);

        // 3. set software mix media type off
        if(FALSE == fgAdspSetMediaType(*pType)){
            LOG(LOG_FAIL, _T("%s stop set media type error.\r\n"),__func__);
            up(&pContext->Cmdlock);

            /* Modify for smix reset r/w pointer after err recover */
            //return FALSE;
            fgRet = FALSE;
        }

        // 4. reset afifo read and write pointer
        pContext->AFifo.u4AfifoRPtr = pContext->AFifo.u4AfifoSA;
        pContext->AFifo.u4AfifoWPtr = pContext->AFifo.u4AfifoSA;

        // 5. dsp dram fifo use offset address
        DspSetSwMixWptr(pContext->AFifo.u4AfifoWPtr - pContext->AFifo.u4AfifoSA);
        break;

    default:
        LOG(LOG_FAIL, _T("%s unsupport cmd %d.\r\n"), __func__, pType->eMediaCtrl);
        fgRet = FALSE;
        break;
    }

    up(&pContext->Cmdlock);

    LOG(LOG_INFO, _T("%s done.\r\n"), __func__);

    return (fgRet);
}

/****************************************************************************
* Function     : AudSmixUpFifoWptr
* Description : audio smix interface: update software mix read pointer
* Parameter   :
* Return        :
****************************************************************************/
static inline void AudSmixUpFifoWptr(AUD_SMIX_BUF_INFO_T *pInfo, u32 u4Len)
{
    pInfo->u4AfifoWPtr += u4Len;
}

/****************************************************************************
* Function     : AudSmixUpFifoRptr
* Description : audio smix interface: update software mix read pointer
* Parameter   :
* Return        :
****************************************************************************/
static inline void AudSmixUpFifoRptr(AUD_SMIX_BUF_INFO_T *pInfo, u32 u4Len)
{
    pInfo->u4AfifoRPtr += u4Len;
}

/****************************************************************************
* Function     : AudSmixGetBufLen
* Description : audio smix interface: get software mix afifo free len to write data
* Parameter   :
* Return        :
****************************************************************************/
static u32 AudSmixGetBufLen(AUD_SMIX_BUF_INFO_T *pInfo)
{
    u32 u4Ret;
    u32 u4Rptr = DspGetSwMixRptr();

    if(u4Rptr > RISC_SW_MIX_CH_SIZE){
        LOG(LOG_FAIL, _T("%s read pointer(0x%x) error, AFIFO SA(0x%x), EA(0x%x).\r\n"),
            __func__, u4Rptr, pInfo->u4AfifoSA, pInfo->u4AfifoEA);
        return 0;
    }

    if(NULL == pInfo){
        LOG(LOG_FAIL, _T("%s info null.\r\n"), __func__);
        return 0;
    }

    pInfo->u4AfifoRPtr = u4Rptr + pInfo->u4AfifoSA;

    if (pInfo->u4AfifoWPtr > pInfo->u4AfifoRPtr){
        u4Ret = pInfo->u4AfifoEA - pInfo->u4AfifoWPtr + pInfo->u4AfifoRPtr - pInfo->u4AfifoSA;
    }
    else if (pInfo->u4AfifoWPtr < pInfo->u4AfifoRPtr){
        u4Ret = pInfo->u4AfifoRPtr - pInfo->u4AfifoWPtr;
    }
    else{
        u4Ret = pInfo->u4AfifoEA - pInfo->u4AfifoSA;
    }

    return u4Ret;
}

/****************************************************************************
* Function     : AudSmixGetDataAvail
* Description : audio smix interface: get software mix afifo free len to write data
* Parameter   :
* Return        :
****************************************************************************/
static u32 AudSmixGetDataAvail(AUD_SMIX_BUF_INFO_T *pInfo)
{
    u32 u4Ret;

    if(NULL == pInfo){
        LOG(LOG_FAIL, _T("%s info pointer is null.\r\n"), __func__);
        return 0;
    }

    if (pInfo->u4AfifoWPtr < pInfo->u4AfifoRPtr){
        u4Ret = pInfo->u4AfifoEA + pInfo->u4AfifoWPtr - pInfo->u4AfifoRPtr - pInfo->u4AfifoSA;
    }
    else if (pInfo->u4AfifoWPtr > pInfo->u4AfifoRPtr){
        u4Ret = pInfo->u4AfifoWPtr - pInfo->u4AfifoRPtr;
    }
    else{
        u4Ret = 0;
    }
    return u4Ret;
}

static inline s32 AudSmixDataCpy(u32 pDst, u16* pSrc, u32 u4Len)
{
    u32 u4Data    = 0;
    u32 u4SampIdx = 0;
    u32 u4SrcIdx  = 0;
    u32 pDst_l    = pDst;
    u32 pDst_r    = pDst + RISC_SW_MIX_CH_SIZE;

    if( NULL == pSrc ||0 == u4Len){
        LOG(LOG_FAIL, _T("%s pointer is null.\r\n"), __func__);
        return 0;
    }

    for(u4SampIdx = 0; u4SampIdx < u4Len; u4SampIdx++){
        u4SrcIdx = u4SampIdx << 2;
        //left channel
        u4Data = pSrc[u4SrcIdx + 2];
        u4Data = ((u4Data <<16)&0xFFFF0000) + pSrc[u4SrcIdx];

       *((volatile u32 *)(g_u4DspDramBuf[6]+(pDst_l<<2))) = u4Data;
        pDst_l++;

        //right channel
        u4Data = pSrc[u4SrcIdx + 3];
        u4Data = ((u4Data <<16)&0xFFFF0000) + pSrc[u4SrcIdx+1];
        *((volatile u32 *)(g_u4DspDramBuf[6]+(pDst_r<<2))) = u4Data;
        pDst_r++;
    }

    //write_file("//storage/udisk2/aud_swmix_dst.bin", (s8*)pDstxx, u4Len*4);

    return u4SampIdx;
}

/******************************************************************************
* Function     : AudSmixSendBuffer
* Description : audio smix interface: user space send buffer
* Parameter   : pBufInfo: user space buffer info pointer
* Return        :
******************************************************************************/
s32 AudSmixSendBuffer(AUD_SEND_BUF_INFO *pBufInfo)
{
    u32 u4BufLen  = 0;
    u32 u4SrcLen = 0;
    u32 u4TmpLen  = 0;
    u32 u4Len     = 0;
    u32 u4WPtr    = 0;
    void* pSrcAddr = NULL;

    AUD_SMIX_CONTEXT_T*  pContext = &g_rAudSmixContext;
    AUD_SMIX_BUF_INFO_T* pInfo    = &pContext->AFifo;

    //check input parameter pointer
    if(NULL == pBufInfo || NULL == pBufInfo->ptrBufAddr || 0 == pBufInfo->u4BufLen){
        LOG(LOG_FAIL, _T("%s param error, addr 0x%x, len 0x%x.\n"),__func__,
            pBufInfo->ptrBufAddr, pBufInfo->u4BufLen);
        return AUD_FAIL;
    }

    if(pBufInfo->u4BufLen % BYTE_PER_SAMPLE != 0){
        LOG(LOG_FAIL, _T("%s BufLen 0x%x need 4-byte align.\n"),__func__, pBufInfo->u4BufLen);
        return AUD_FAIL;
    }

    //check space data in kernel
    if (!access_ok(VERIFY_READ, (void __user *)pBufInfo->ptrBufAddr, pBufInfo->u4BufLen)){
        LOG(LOG_FAIL, _T("%s access_ok fail.\r\n"), __func__);
        return AUD_FAIL;
    }

    // dsp common dram dword align, 2 channel: 2 dwords(8 bytes) align
    u4SrcLen = pBufInfo->u4BufLen >> 3;

    down(&pContext->AFifolock);

    do{
        while((u4BufLen = AudSmixGetBufLen(pInfo)) < MIN_BUF_16){
            if(AUD_SMIX_STARTED != AudSmixGetState()){
                goto SENDBUF_EXIT;
            }
            Sleep(10);
        }

        u4BufLen -= BUF_DATA_AVAIL;

        //source buffer start address, byte per unit
        pSrcAddr = (s8*)pBufInfo->ptrBufAddr+ pBufInfo->u4BufLen - (u4SrcLen <<3);

        //send data length compare with afifo length
        u4TmpLen = u4SrcLen < u4BufLen ? u4SrcLen : u4BufLen;
        u4WPtr = pInfo->u4AfifoWPtr;

        if (u4TmpLen < pInfo->u4AfifoEA - u4WPtr){
            AudSmixDataCpy(u4WPtr, (u16* )pSrcAddr, u4TmpLen);
            //update write pointer
            pInfo->u4AfifoWPtr = pInfo->u4AfifoWPtr + u4TmpLen;
            DspSetSwMixWptr(pInfo->u4AfifoWPtr - pInfo->u4AfifoSA);
        }
        else{
            u4Len  = pInfo->u4AfifoEA - u4WPtr;
            AudSmixDataCpy(u4WPtr, (u16* )pSrcAddr, u4Len);
            u4Len = u4TmpLen - u4Len;
            if (u4Len > 0 ){
                AudSmixDataCpy(pInfo->u4AfifoSA, (u16* )pSrcAddr + ((u4TmpLen - u4Len)<<2), u4Len);
            }
            //update write pointer
            pInfo->u4AfifoWPtr = pInfo->u4AfifoSA + u4Len;
            DspSetSwMixWptr(pInfo->u4AfifoWPtr - pInfo->u4AfifoSA);
        }

        u4SrcLen -= u4TmpLen;
        if(AUD_SMIX_STARTED != AudSmixGetState()){
            goto SENDBUF_EXIT;
        }
    }while(u4SrcLen > 0);

    //write_file("//storage/udisk2/aud_swmix_src.bin", pBufInfo->ptrBufAddr, pBufInfo->u4BufLen);

SENDBUF_EXIT:
    up(&pContext->AFifolock);

    return (pBufInfo->u4BufLen);
}


/****************************************************************************
* Function     : AudSmixWriteData
* Description : audio smix interface: get software mix afifo free len to write data
* Parameter   :
* Return        :
****************************************************************************/
u32 AudSmixWriteData(AUD_SMIX_BUF_INFO_T *pInfo)
{
    return 0;
}


#if 0
static struct task_struct *g_hsmixData;
/****************************************************************************
* Function     : AudSmixReFmt16To24
* Description : audio smix interface: convert uplayer data 16bit 2ch to dsp data 24bit 2ch
* Parameter   : srcPtr: src data addr, dstLPtr dsp left channel pointer, dstRPtr dsp right channel pointer,
                      u4Samplen: sample size
* Return        :
****************************************************************************/
u32 AudSmixReFmt16To24(u8* dstPtr, u8* srcPtr, u32 u4Len)
{
    u32 u4SampIdx = 0;
    u32 u4SrcIdx  = 0;
    u32 u4DstIdx  = 0;
    u8* dstLptr   = NULL;
    u8* dstRptr   = NULL;
    u32 u4SmpLen  = u4Len >>2;

    if(srcPtr == NULL || dstPtr == NULL|| u4Len == 0){
        LOG(LOG_FAIL, _T("%s pointer is null.\r\n"), __func__);
        return 0;
    }
    dstLptr = dstPtr;
    dstRptr = dstPtr + u4SmpLen*3;

    for(u4SampIdx = 0; u4SampIdx < u4SmpLen; u4SampIdx++){
        u4SrcIdx = u4SampIdx << 2;               // 4 byte data per one sample two channel
        u4DstIdx = (u4SampIdx << 1) + u4SampIdx; // 3 byte data per one sample one channel

        //left channel
        dstLptr[u4DstIdx]     = 0;
        dstLptr[u4DstIdx + 1] = srcPtr[u4SrcIdx];
        dstLptr[u4DstIdx + 2] = srcPtr[u4SrcIdx + 1];

        //right channel
        dstRptr[u4DstIdx]     = 0;
        dstRptr[u4DstIdx + 1] = srcPtr[u4SrcIdx + 2];
        dstRptr[u4DstIdx + 2] = srcPtr[u4SrcIdx + 3];
    }
    return u4SampIdx;
}

/****************************************************************************
* Function     : AudSmixDataProcThread
* Description : audio smix interface: write afifo data to dsp ab buffer
* Parameter   :
* Return        :
****************************************************************************/
static s32 AudSmixDataProcThread(void* pvArg)
{
    u32 u4Avail   = 0;
    u32 u4Tmp     = 0;
    u32 u4BufLen  = 0;
    u32 u4SendLen = 0;
    AUD_SMIX_CONTEXT_T* pContext = &g_rAudSmixContext;
    AUD_SMIX_BUF_INFO_T *pInfo = &pContext->AFifo;

    do{
         // 1. get afifo avail data size: 24bit 2ch
        while((u4Avail = AudSmixGetDataAvail(pInfo))< MIN_BUF_24){
            Sleep(20);
            if(AudSmixGetState() == AUD_SMIX_STOPED)
                goto DATA_EXIT;
        }

        //make sure 64 sample align
        u4Avail = u4Avail - u4Avail % MIN_BUF_24;

        while(u4Avail != 0){
            // get ab buffer left space
            while((u4BufLen = AudSmixGetBufLen(pInfo)) < MIN_BUF_24){
                Sleep(20);
                if(AudSmixGetState() == AUD_SMIX_STOPED)
                    goto DATA_EXIT;
            }

            u4SendLen = u4Avail > u4BufLen ? u4BufLen : u4Avail;

            //make sure send buffer len 64 bit size
            if((u4Tmp = u4SendLen % MIN_BUF_24) != 0)
                u4SendLen  -= u4Tmp;

            if(AudSmixGetState() == AUD_SMIX_STOPED)
                goto DATA_EXIT;

            //write data to dsp ab buffer.
            if(0 == AudSmixWriteData(pInfo))
            {
            }
            u4Avail -= u4SendLen;
        }

    }while(AudSmixGetState() == AUD_SMIX_STARTED);


DATA_EXIT:
    LOG(LOG_FAIL, TEXT("[AudDrvThreadInit]AudDrvDec1Thd create fail \r\n"));
}

s32 AudSmixCreateDataProcThread(void)
{
    s8 s_name[32] = "audsmixProc";

    // create audio smix data process thread
    g_hsmixData = kthread_create(AudSmixDataProcThread, (void *)NULL, s_name);

    if (IS_ERR(g_hsmixData)) {
        LOG(LOG_FAIL, TEXT("[AudDrvThreadInit]AudDrvDec1Thd create fail \r\n"));
        g_hsmixData = NULL;
        return AUD_FAIL;
    }
    else{
        struct sched_param param;
        s32 ret;
        param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hsmixData, SCHED_RR, &param);
        if(ret!=0)
            return AUD_FAIL;
    }
    wake_up_process(g_hsmixData);

    return AUD_OK;
}
#endif






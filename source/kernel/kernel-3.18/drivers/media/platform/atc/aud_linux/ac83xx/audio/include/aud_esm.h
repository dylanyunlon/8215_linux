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


#ifndef _AUD_ESM_H_
#define _AUD_ESM_H_

#include <media/atc/drv_esm_if.h>
#include <media/atc/x_aud_dec.h>

typedef struct _AUD_ESM_CONTEXT_T
{
    //got from _AudConnect
    u16 u2AudDrvCompId;          //the audio driver component id
    u16 eType;                   //specifies the packet filter type

    //got from i4ESM_CreateESInterface of ESM
    u32 u4Handle;
    Decoder_OpIf *prDecoderOpIf;

    u32 u4LastUpdatedRptr;       // last Rptr updated to ESM
    u32 u4LastIteratedAUIdx;     // last AU index updated to DSP
    u32 u4Read;

    u32 u4AfifoSA,u4AfifoEA;
    u32 u4AfifoRPtr, u4AfifoWPtr, u4AfifoWPtrForApp;  //got from dsp/dspctrl, reset when Stop and Init

    u32 u4PTSQRPtr, u4IBCQRPtr;          //got from dsp/dspctrl, reset when Stop and Init
    u32 u4PTSQWPtr, u4IBCQWPtr;          //got from ESM
    u32 u4PTSQStrAddr, u4IBCQStrAddr;    //from dspctrl after memory partitioned
    u32 u4PTSQEndAddr, u4IBCQEndAddr;    //from dspctrl after memory partitioned

    u32 u4CurWrIdx;
    u32 u4ReadCnt;
    bool fgIsPlay;
    bool fgFirstAUArrive;
    
    u32 u4TotalPBBankCount;      //Used for calculating the total playback time
    u32 u4TotalPBFrameCount;     //Used for calculating the MP3 total playback time
    void* hMap;
    //void * m_hAudCmdEvent;
    u8  m_u1EsmState;    
}   AUD_ESM_CONTEXT_T;

extern u32 u4AudEsmGetWritePtr(AUD_ESM_CONTEXT_T *pContext);
extern u32 u4AudEsmGetReadPtr(AUD_ESM_CONTEXT_T *pContext);

extern s32 i4AudEsm_TaskCreate(void);
//API for audiodecoder.c
extern s32 i4AudEsm_Connect(u16 u2DecId);
extern s32 i4AudEsm_Disconnect(u16 u2DecId);
extern s32 i4AudEsm_SendAU(u8 u1DecId, AU_AUDIO *pAu);
extern bool  AudEsm_SendBufferInfo(u8 u1DecId, AUD_SEND_BUF_INFO *pBufInfo);
extern bool  AudEsm_SendEsmInfo(u8 u1DecId, ESM_IO_BUF_INFO *pBufInfo);
extern bool  AudEsm_GetAfifoInfoVirtual(u8 u1DecId, AUDIO_BUF_INFO *pInfo);
extern s32 i4AudEsm_GetAudioBuffer(u8 u1DecId, AUDIO_BUF_INFO *pInfo);
extern s32 i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_TYPE_T eFifoType, AUD_POSINFO_T *pAudPos);
extern s32 i4AudEsm_SendBuffer(u8 u1DecId,void *buf, u32 u4Size);
extern u32 u4AudEsm_GetSpareBufLen(AUD_ESM_CONTEXT_T *pContext);
extern u32 u4AudEsm_GetReadCount(u8 u1DecId);

extern s32 i4AudEsm_SetAuEvent(u8 u1DecId);
extern void vAudEsm_InitTotalPBTimeCount(void);
extern u32 u4AudEsm_GetTotalPBBankCount(void);
extern void vAudEsm_SetTotalPBBankCount(u32 u4_update_bank_count);




extern u64 u8Aud_GetSTC(void);






#endif  // #ifndef _AUD_ESM_H_



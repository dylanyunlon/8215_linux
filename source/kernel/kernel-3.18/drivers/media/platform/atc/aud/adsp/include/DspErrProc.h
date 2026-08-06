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
*[File]       :  DspErrProc.h
*[Author] 
*
*[Description]
*   
******************************************************************************/
    
#ifndef _H_DSP_ERR_PROC
#define _H_DSP_ERR_PROC


typedef enum
{
    DSP_DET_OK = 0,
    DSP_DET_FAIL = 1
}DSP_DET_RET;

typedef enum
{
    DSP_PROC_OK = 0,
    DSP_PROC_FAIL = 1
}DSP_PROC_RET;


typedef enum
{
    DSP_PROC_INIT = 0,
    DSP_PROC_RUN = 1,
    DSP_PROC_RESET = 2
}DSP_RPOC_STATE;

typedef struct _WT_DSP_HW_STATE_T
{
    bool fgDspHangUp;
    bool fgDspHangUpValid;
    bool fgDspEndlessLoop;
    bool fgDspEndLessLoopValid;
    bool fgDspBusy;
    bool fgDspBusyValid;
    u32  u4FstPcVal;
    u32  u4CurPcVal;
}WT_DSP_HW_STATE_T;

typedef struct _WT_DSPB_CODEC_STATE_T
{
    bool fgDecAbNormal;
    u32  u4DecFstBankNum;
    u32  u4DecCurBankNum;
}WT_DSPB_CODEC_STATE_T;

typedef struct _DSP_DET_STATE_T
{
    WT_DSP_HW_STATE_T rDspbHwSt;
    WT_DSP_HW_STATE_T rDspaHwSt;

    //NULL means no need to check
    WT_DSPB_CODEC_STATE_T *prFstCodecSt;
    WT_DSPB_CODEC_STATE_T *prSndCodecSt;
    WT_DSPB_CODEC_STATE_T *prTrdCodecSt;

    s32  i4DetCntRemain;
}DSP_DET_STATE_T;

extern u32 u4AdspErrProcInit(void);
extern u32 vAdspErrProcUnInit(void);


extern u32 u4AdspErrProcStateSet(DSP_RPOC_STATE eNewProcSt);
extern DSP_RPOC_STATE u4AdspErrProcStateGet(void);

extern u32 u4AdspErrProcNotifyFlagSet(u32 u4DecId, bool fgNotify);
extern bool u4AdspErrProcNotifyFlagGet(u32 u4DecId);
extern void vErrTimerAdd(void);
extern void vErrTimerDelete(void);
#endif

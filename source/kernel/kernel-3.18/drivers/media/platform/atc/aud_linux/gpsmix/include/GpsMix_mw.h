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



//#include "x_typedef.h"
#include <linux/types.h>

#ifndef _GPS_MIX_MW_H_
#define _GPS_MIX_MW_H_


#define GPSMIX_RET_OK                          ((s8)0)
#define GPSMIX_RET_FAIL                        ((s8)-1)
#define MIC_BUF_INDEX                           (6)

/* Control types. */
typedef enum
{
    AUD_DEC_GPS_MIX_CTRL_STOP = 0,
    AUD_DEC_GPS_MIX_CTRL_START,
    AUD_DEC_GPS_MIX_CTRL_PAUSE,
    AUD_DEC_GPS_MIX_CTRL_RESUME
}AUD_DEC_GPS_MIX_T;

typedef struct
{
    u32 u4CommBufSA;
    u32 u4CommBufEA;
    u32 u4CommBufRptr;
    u32 u4ConsumeData;
}AUD_GPS_MIX_COMM_BUF_INFO;

typedef struct
{
    u32 u4MicBufBTSA;
    u32 u4MicBufBTSize;
    u32 u4MicPageSA;
    u32 u4WorkBufferSA;
    u32 u4TotalPhy;
    u32 u4BlockPhy;
    u32 u4BankPhy;
    u32 u4OffsetPhy;
}AUD_MIC_BUF_FOR_BT_INFO;

s32 _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_T  eAudGpsMixCtrl);
s32 _AudGetConsumData(void);
void _AudGetMicBufInfo(AUD_MIC_BUF_FOR_BT_INFO *pMicBufInfo);

extern u32 g_u4DspDramBuf[8];
extern u8 * g_ucAdspWorkingBuffer;
extern u32 dReadDspCommDram(u32 addr);


extern void DspCfgSetGpsMixCh(u32 u4MixCh);
extern void DspCfgGetCommBufInfo(AUD_GPS_MIX_COMM_BUF_INFO *pCommBufInfo);


#endif

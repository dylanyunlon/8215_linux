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
#include "GpsMix_mw.h"
#include "GpsMix_AsvTrigger.h"
#include "GpsMix_if.h"
#include "aud_debug.h"
#include "DspFunc.h"
#include "aud_drv.h"


s32 _AudSetGpsMixCtrl(AUD_DEC_GPS_MIX_T  eAudGpsMixCtrl)
{
     LOG(LOG_FEATURE, TEXT("*****[zf:_AudSetGpsMixCtrl] eAudGpsMixCtrl = %d \r\n"), eAudGpsMixCtrl);
     switch(eAudGpsMixCtrl)
     {
         case AUD_DEC_GPS_MIX_CTRL_STOP:
            AudGpsMix_CmdStop();
            break;

         case AUD_DEC_GPS_MIX_CTRL_START:         
            AudGpsMix_CmdStart();
            break;

        case AUD_DEC_GPS_MIX_CTRL_RESUME:
            AudGpsMix_CmdResume();
            break;

        case AUD_DEC_GPS_MIX_CTRL_PAUSE:
            AudGpsMix_CmdPause();
            break;

        default:
            LOG(LOG_FEATURE, TEXT("*****[zf:_AudSetGpsMixCtrl] NO This GpsMixCtrl Type!!\r\n"));
            return GPSMIX_RET_FAIL;
     }
     return GPSMIX_RET_OK;
}

s32 _AudGetConsumData()
{   
    u32 u4ConsumeData;
    u4ConsumeData = i4AsvGpsMixDspNotifyConsumedData();
    return u4ConsumeData;
}

void _AudGetMicBufInfo(AUD_MIC_BUF_FOR_BT_INFO *pMicBufInfo)
{
    u32 u4Total = 0;
    uintptr_t u4WkbufPhyAddr = ADSP_PHYSICAL((uintptr_t)g_ucAdspWorkingBuffer);
    uintptr_t u4MicPagePhyAddr = ADSP_PHYSICAL(g_u4DspDramBuf[MIC_BUF_INDEX]);

    DspGetMicBufInfo(pMicBufInfo, u4WkbufPhyAddr, u4MicPagePhyAddr);
    DspGetMicBufTotal(&u4Total);
    pMicBufInfo->u4TotalPhy = ADSP_PHYSICAL(u4Total);
    pMicBufInfo->u4OffsetPhy = (((pMicBufInfo ->u4TotalPhy)-u4MicPagePhyAddr) & 0x3fffc); //byte address
}



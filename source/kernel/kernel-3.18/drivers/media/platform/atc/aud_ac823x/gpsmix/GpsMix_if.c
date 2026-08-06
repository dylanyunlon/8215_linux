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
#include "GpsMix_if.h"
#include "GpsMix_mw.h"
#include "aud_debug.h"

void* m_hGpsMixStartEvent = NULL;
void* m_hGpsMixStopEvent = NULL;
void* m_hGpsMixPauseEvent = NULL;
void* m_hGpsMixResumeEvent = NULL;
void* m_hGpsMixConsumeDataEvent =NULL;


/******************************************************************************
                                                      function 
******************************************************************************/
s32 AudGpsMix_CmdPause()
{
    LOG(LOG_FEATURE, TEXT("*****[zf:AudGpsMix_CmdPause] GpsMix Drv receive Pause Command!!\r\n"));
    VERIFY(AudGpsMix_DrvCmd(AUD_GPS_MIX_CMD_PAUSE));
    return GPSMIX_RET_OK;
}

s32 AudGpsMix_CmdStart()
{
    LOG(LOG_FEATURE, TEXT("*****[zf:AudGpsMix_CmdStart] GpsMix Drv receive Start Command!!\r\n"));
    VERIFY(AudGpsMix_DrvCmd(AUD_GPS_MIX_CMD_START));
    return GPSMIX_RET_OK;
}

s32 AudGpsMix_CmdStop()
{
    LOG(LOG_FEATURE, TEXT("*****[zf:AudGpsMix_CmdStop] GpsMix Drv receive stop Command!!\r\n"));
    VERIFY(AudGpsMix_DrvCmd(AUD_GPS_MIX_CMD_STOP));
    return GPSMIX_RET_OK;

}

s32 AudGpsMix_CmdResume()
{
    LOG(LOG_FEATURE, TEXT("*****[zf:AudGpsMix_CmdResume] GpsMix Drv receive Resume Command!!\r\n"));
    VERIFY(AudGpsMix_DrvCmd(AUD_GPS_MIX_CMD_RESUME));
    return GPSMIX_RET_OK;

}




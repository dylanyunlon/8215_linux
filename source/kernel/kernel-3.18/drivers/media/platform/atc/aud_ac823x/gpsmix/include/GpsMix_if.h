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

#ifndef _GPS_MIX_IF_H_
#define _GPS_MIX_IF_H_

#include "x_os.h"
#include "u_os.h"
#include "aud_debug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "GpsMix_drvthread.h"
#include "GpsMix_mw.h"
//#include "x_typedef.h"
#include <linux/types.h>


s32 AudGpsMix_CmdPause(void);
s32 AudGpsMix_CmdStart(void);
s32 AudGpsMix_CmdStop(void);
s32 AudGpsMix_CmdResume(void);
bool AudGpsMix_DrvCmd(AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd);
//void _AudGetCommBufInfo(AUD_GPS_MIX_COMM_BUF_INFO *pCommBufInfo);
//void _AudSetCommBufWptr(u32 u4WptrVal);


extern void * m_hGpsMixStartEvent;
extern void * m_hGpsMixStopEvent;
extern void * m_hGpsMixPauseEvent;
extern void * m_hGpsMixResumeEvent;
extern void * m_hGpsMixConsumeDataEvent;


#ifdef __cplusplus
}
#endif

#endif

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



#ifndef _GPS_MIX_DRVTHREAD_H_
#define _GPS_MIX_DRVTHREAD_H_

#include "u_priority.h"
//#include "x_typedef.h"
#include <linux/types.h>
#include "x_assert.h"


#define ADRV_GPSMIX_CMD_Q_NAME              TEXT("AUD GPXMIXCMDQ")
#define AUD_GPSMIX_CMD_QUEUE_SIZE           32


#define AUD_DRV_GPS_MIX_THREAD_NAME         "AudGpsMixDrvThread"
#define AUD_DRV_GPS_MIX_THREAD_STACK_SIZE   2048
#define AUD_DRV_GPS_MIX_THREAD_PRIORITY     PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_TIME_CRITICAL, 0)


#define AUD_GPS_MIX_CMD_FLAG_START          ((u32) (1<<(u32)AUD_GPS_MIX_CMD_START))
#define AUD_GPS_MIX_CMD_FLAG_STOP           ((u32) (1<<(u32)AUD_GPS_MIX_CMD_STOP))
#define AUD_GPS_MIX_CMD_FLAG_PAUSE          ((u32) (1<<(u32)AUD_GPS_MIX_CMD_PAUSE))
#define AUD_GPS_MIX_CMD_FLAG_RESUME         ((u32) (1<<(u32)AUD_GPS_MIX_CMD_RESUME))


typedef enum
{
    AUD_GPS_MIX_CMD_START   = 0,
    AUD_GPS_MIX_CMD_STOP    = 1,
    AUD_GPS_MIX_CMD_PAUSE   = 2,
    AUD_GPS_MIX_CMD_RESUME  = 3
}   AUD_DRV_GPS_MIX_CMD_T;

typedef enum
{
    AUD_GPS_MIX_DRV_UNINITIALIZED = 0,
    AUD_GPS_MIX_DRV_OPENING,
    AUD_GPS_MIX_DRV_STOPPING,
    AUD_GPS_MIX_DRV_STOPPED,
    AUD_GPS_MIX_DRV_STARTING,
    AUD_GPS_MIX_DRV_STARTED,
    AUD_GPS_MIX_DRV_PAUSING,
    AUD_GPS_MIX_DRV_PAUSED,
    AUD_GPS_MIX_DRV_RESUMING
}   AUD_GPS_MIX_DRV_STATE_T;

typedef enum
{
    AUD_GPS_MIX_DEC_STOP = 0,
    AUD_GPS_MIX_DEC_STARTING,
    AUD_GPS_MIX_DEC_INIT,
    AUD_GPS_MIX_DEC_PAUSING,
    AUD_GPS_MIX_DEC_PAUSED,
    AUD_GPS_MIX_DEC_RESUMING
}AUD_GPS_MIX_DECODER_STATE_T;

typedef struct
{
    AUD_GPS_MIX_DECODER_STATE_T eGpsMixDecState;  
    u32                      u4GpsMixEventFlag;
}AUD_GPS_MIX_DEC_INFO;

void AudGpsMix_DrvInit(void);
void vAudGpsMix_SemaphoreDelete(void);
void vAudGpsMixMsgqDelete(void);
void AudGpsMix_AsvCommandDone(u32 u4Command);
void AudGpsMix_WaitAsvCommandDone(u32 u4Command);
bool AudGpsMix_DrvCmd(AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd);

void AudGpsMix_CommandDoneNotify(AUD_DRV_GPS_MIX_CMD_T eGpsMixCmd);


#endif

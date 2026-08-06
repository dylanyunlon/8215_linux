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

#ifndef _AUD_SMIX_H_
#define _AUD_SMIX_H_

#include <linux/types.h>
#include <media/atc/drv_aud.h>


#define RISC_SW_MIX_BASE_ADDR  0xDB00
#define RISC_SW_MIX_CH_SIZE    0x1000
#define RISC_SW_MIX_DRAM_PAGE  0x600

typedef enum
{
    AUD_SMIX_UNINIT   = 0,
    AUD_SMIX_INITED   = 1,
    AUD_SMIX_STOPED   = 2,
    AUD_SMIX_STARTED  = 3
}AUD_SMIX_STATE_T;

typedef enum
{
    AUD_SMIX_STOP   = 0,
    AUD_SMIX_PLAY   = 1
}AUD_SMIX_CMD_T;

typedef enum
{
    AUD_SMIX_OUT_F   = 0,
    AUD_SMIX_OUT_R   = 1
}AUD_SMIX_OUT_T;

typedef struct _AUD_SMIX_BUF_INFO_T
{
    volatile __u32 u4AfifoRPtr;
    volatile __u32 u4AfifoWPtr;
    volatile __u32 u4AfifoSA;
    volatile __u32 u4AfifoEA;
} AUD_SMIX_BUF_INFO_T;

typedef struct _AUD_SMIX_CONTEXT_T
{
    volatile AUD_SMIX_STATE_T Status;
    AUD_SMIX_BUF_INFO_T       AFifo;

    struct semaphore Cmdlock;
    struct semaphore AFifolock;
}AUD_SMIX_CONTEXT_T;


/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
s32 AudSmixInit(void);
bool AudSmixSetMwCtrl( AUD_MEDIA_TYPE* pType);
s32 AudSmixSendBuffer(AUD_SEND_BUF_INFO *pBufInfo);

#endif  // #ifndef _AUD_SMIX_H_



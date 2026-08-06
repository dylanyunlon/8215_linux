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

#ifndef _DRV_AUD_ERRCODE_H_
#define _DRV_AUD_ERRCODE_H_

#include "x_typedef.h"
#include "u_uerrcode.h"

#define AUD_UERRCODE(grpid, errid)  \
    ( UERRCODE( DRL_MODULE_AUD, ((u32)((grpid & 0xf) << 12) | (u32)(errid & 0x0fff)) ) )


#define S_AUD_OK                           0

#define E_AUD_GRP_DSPCTL                 0x1
#define E_AUD_GRP_ASV                    0x2
#define E_AUD_GRP_DSPDRV                 0x3
#define E_AUD_GRP_ESM                    0x4
#define E_AUD_GRP_2ND_AUD                0x5

// Audio driver related error codes
#define E_AUD_DRV_UNEXPECT               AUD_UERRCODE(E_AUD_GRP_DSPDRV,0))

// Audio ESM related error codes
#define E_AUD_ESM_INVALID_IM_HANDLE      AUD_UERRCODE(E_AUD_GRP_ESM, 0)
#define E_AUD_ESM_INVALID_DISC_TYPE      AUD_UERRCODE(E_AUD_GRP_ESM, 1)
#define E_AUD_ESM_INVALID_AU_IDX         AUD_UERRCODE(E_AUD_GRP_ESM, 2)

#define E_DSP_IBC_QUEUE_FULL             AUD_UERRCODE(E_AUD_GRP_DSPCTL, 0)
#define E_DSP_IBC_INVALID_AFIFO_ADDR     AUD_UERRCODE(E_AUD_GRP_DSPCTL, 1)

// 2nd audio on ARM error definition
#define E_AUD_2ND_AUD_UNKNOWN            AUD_UERRCODE(E_AUD_GRP_2ND_AUD, 0)
#define E_AUD_2ND_AUD_SEND_INT           AUD_UERRCODE(E_AUD_GRP_2ND_AUD, 1)
#define E_AUD_2ND_AUD_WRONG_INT_ID       AUD_UERRCODE(E_AUD_GRP_2ND_AUD, 2)
#define E_AUD_2ND_AFIFO_AU_FULL          AUD_UERRCODE(E_AUD_GRP_2ND_AUD, 3)

// Flash Lite SRC
#define E_AUD_FL_SRC_SUCCESS             0x0
#define E_AUD_FL_SRC_INIT_STATE          0x1
#define E_AUD_FL_SRC_BUFFER_INVALID      0x2
#define E_AUD_FL_BUFFER_INVALID          0x3

#endif // _DRV_AUD_ERRCODE_H_


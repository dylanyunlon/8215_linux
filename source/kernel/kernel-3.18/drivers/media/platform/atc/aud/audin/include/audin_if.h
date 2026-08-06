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

#ifndef _AUDIN_IF_H_
#define _AUDIN_IF_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "x_aud_dec.h"

#include "aud_ioctrl.h"
#include "aud_clock.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define AUD_AFIFO_LINEIN1_OFFSET 0xC000
#define AUD_AFIFO_LINEIN2_OFFSET 0x12000

#define AUD_AFIFO_LINEIN_SIZE 0x6000

#define AUD_AFIFO_LINEIN1_START \
    (MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE + AUD_AFIFO_LINEIN1_OFFSET)

#define AUD_AFIFO_LINEIN2_START \
    (MT3360_AFIFO_VA + AUD_AFIFO_PRIMARY_SIZE + AUD_AFIFO_LINEIN2_OFFSET)

extern bool Aud_Linein_Init(void);
extern bool Aud_Linein_DeInit(void);
extern bool Aud_Linein_SetCtrl(AUDIN_SET_ONOFF* prLinInCmd, u8 u1DecID);
extern void Aud_Linein_GetDecInfo(AUDIN_INFO *pAudInfo);
extern void Aud_Linein_GetDataLen(AUDIN_DEC_DATA_LEN * pAudDecLinDataLen);
extern void Aud_Linein_VolCtrl(AUD_DEC_REAR_VOLUME_INFO_T * pRearChVol);
extern void Aud_Linein_VolGainCtrl(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * pRearChVol);
extern void Aud_Linein_ShowParams(u32 u4AudinId);
extern void Aud_Linein_SetInputType(u8 u1Input);

extern u32 Aud_Linein_GetWPtr(void);
extern u32 Aud_Linein2_GetWPtr(void);

extern bool Aud_Linein_SetIisin(AUD_IIS_CTRL_INFO* prI2sCtrl, u8 u1DecId);
extern bool Aud_Linein_AdcInFlowCtrl(AUD_DRV_CONTEXT *prContext, AUDIN_SET_ONOFF* prParam);
extern bool Aud_Linein_IisInFlowCtrl(AUD_DRV_CONTEXT *prContext, AUD_IIS_CTRL_INFO* prParam);

#ifdef __cplusplus
}
#endif

#endif

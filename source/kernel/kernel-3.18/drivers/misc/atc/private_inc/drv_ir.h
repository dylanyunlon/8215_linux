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

#ifndef __DRV_IR_H__
#define __DRV_IR_H__


#define IR_FAIL             (-1)
#define IR_SUCC             (0)

#include "drv_config.h"
#include "chip_ver.h"



/***************************************************************************/
#define BTN_NONE                ((UINT32) 0xffffffff)
#define BTN_NO_DEF              ((UINT32) 0xfffffffe)
#define BTN_KEY_REPEAT              ((UINT32) 0xfffffffd)

/******************************************************************************
* u4Info:
*   Bit 31~24 is the value of the sampling counter in the 3rd pulse.
*   Bit 23~16 is the value of the sampling counter in the 2nd pulse.
*   Bit 15~08 is the value of the sampling counter in the 1st pulse.
*   Bit 05~00 is the bit count of this IR command.
******************************************************************************/
#define INFO_TO_BITCNT(u4Info)      ((u4Info & IRRX_CH_BITCNT_MASK)    >> IRRX_CH_BITCNT_BITSFT)
#define INFO_TO_1STPULSE(u4Info)    ((u4Info & IRRX_CH_1ST_PULSE_MASK) >> IRRX_CH_1ST_PULSE_BITSFT)
#define INFO_TO_2NDPULSE(u4Info)    ((u4Info & IRRX_CH_2ND_PULSE_MASK) >> IRRX_CH_2ND_PULSE_BITSFT)
#define INFO_TO_3RDPULSE(u4Info)    ((u4Info & IRRX_CH_3RD_PULSE_MASK) >> IRRX_CH_3RD_PULSE_BITSFT)
typedef void (* PFN_IRRXCB_T)(UINT32 u4Info, const UINT8 *pu1Data);

/******************************************************************************/
/* Power down mode functions. */
//extern INT32 IRHW_Down(const INT32 *pi4Data);
//extern INT32 IRHW_PKey(const INT32 *pi4Data);
//extern UINT32 IRHW_PowerBitNum(const UINT32 *pu4Data);
//extern UINT32 IRHW_PowerUpKey1(const UINT32 *pu4Data);
//extern UINT32 IRHW_PowerUpKey2(const UINT32 *pu4Data);

extern INT32 IRRX_InitMtkIr(void);
extern INT32 IRRX_StopMtkIr(void);
extern INT32 IRRX_ResetMtkIr(void);
extern INT32 IRRX_PollMtkIr(UINT32 *pu4Key);

/* HWIR RX external functions declare. */
extern void IRHW_RxRdConf(INT32 *pi4Config, INT32 *pi4SaPeriod, INT32 *pi4Threshold);
extern void IRHW_RxWrConf(INT32 i4Config, INT32 i4SaPeriod, INT32 i4Threshold);
extern INT32 IRHW_RxInit(INT32 i4Config, INT32 i4SaPeriod, INT32 i4Threshold);
extern INT32 IRHW_RxStop(void);
extern INT32 IRHW_RxSetCallback(PFN_IRRXCB_T pfnCallback, PFN_IRRXCB_T *ppfnOld);
extern INT32 i4IrHWUninit(void);
extern INT32 i4IrUninit(void); 
//extern INT32 IR_Status_WD(void);
//extern void IRHW_FastejectKeyNotify(void);
extern void IRHW_SetEnable(BOOL fgEnable);

/******************************************************************************/
/* IRTX IF functions. */

/* HWIR TX external functions declare. */
//extern void IRHW_TxRdConf(INT32 *pi4Config, INT32 *pi4TPeriod, INT32 *pi4Modulation);
//extern void IRHW_TxWrConf(INT32 i4Config, INT32 i4TPeriod, INT32 i4Modulation);
//extern void IRHW_TxSendData(const INT32 *pi4DataArray, INT32 i4BitNum);

//extern void IRHW_PDwn(void);
//extern void IRHW_LoadPDwnCode(void);
//extern INT32 _IRHW_WAKE_UP_ENABLE(UINT8 u1IRIdx, UINT32 u4KeyCodeM, UINT32 u4KeyCodeL);
//extern INT32 _IRHW_POWER_DOWN_ENABLE(UINT8 u1IRIdx, UINT32 u4KeyCodeM, UINT32 u4KeyCodeL);

#endif /* __DRV_IR_H__ */


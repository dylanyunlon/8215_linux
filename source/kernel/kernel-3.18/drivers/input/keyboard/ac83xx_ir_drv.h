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

#define DEBUGMSG(fg, x... )  \
do { \
       if(fg) \
        printk x ;\
} while ( 0 )

#define TEXT(x) x

/***************************************************************************/
#define BTN_NONE                ((unsigned int) 0xffffffff)
#define BTN_NO_DEF              ((unsigned int) 0xfffffffe)
#define BTN_KEY_REPEAT              ((unsigned int) 0xfffffffd)

#define SCAN_CODE_MAX               0x60
#define MAIN_SCAN_CODE_TABLE_SIZE   0x54


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
typedef void (* PFN_IRRXCB_T)(unsigned int u4Info, const unsigned char *pu1Data);

/******************************************************************************/
extern int IRRX_InitMtkIr(void *dev_id);
extern int IRRX_StopMtkIr(void *dev_id);
extern int IRRX_ResetMtkIr(void *dev_id);
extern int IRRX_PollMtkIr(unsigned int *pu4Key);

/* HWIR RX external functions declare. */
extern void IRHW_RxRdConf(int *pi4Config, int *pi4SaPeriod, int *pi4Threshold);
extern void IRHW_RxWrConf(int i4Config, int i4SaPeriod, int i4Threshold);
extern int IRHW_RxInit(int i4Config, int i4SaPeriod, int i4Threshold, void *dev_id);
extern int IRHW_RxStop(void *dev_id);
extern int IRHW_RxSetCallback(PFN_IRRXCB_T pfnCallback, PFN_IRRXCB_T *ppfnOld);
extern int i4IrHWUninit(void *dev_id);
extern int i4IrUninit(void *dev_id); 
extern void IRHW_SetEnable(bool fgEnable);


#endif /* __DRV_IR_H__ */


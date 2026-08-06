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

#ifndef _SPDIF_IF_H_
#define _SPDIF_IF_H_

#include "drv_config.h"

#if (CONFIG_DRV_SPDIF_TYPE == CONFIG_DRV_SPDIF_TYPE_LC89058)
#include "spdif_if_LC89058.h"
#else // Demo Board
#include "spdif_if_CS8415.h"
#endif


// Common defines for different SPDIF-IN DIR ICs
// *********************************************************************
// Constant definitions - Input Selection
// *********************************************************************
// for ADAC_CMD_AIN_SEL
#define AUDIN_SEL_COAXL              0x01
#define AUDIN_SEL_OPT1                0x04
#define AUDIN_SEL_OPT2                0x05
#define AUDIN_SEL_HDMIRX            0x06

#define AUDIN_SEL_LINEIN             0x07
#define AUDIN_SEL_MICIN               0x08
#define AUDIN_SEL_OFF                  0xFF

// *********************************************************************
// Constant definitions - Others
// *********************************************************************
#define SPDIF_CMD_ILLEGAL            0xFF
#define AUDIN_CMD_PRI_HIGH         1
#define AUDIN_CMD_PRI_LOW          2

#define DATA_PCM                             0
#define DATA_NON_PCM                    1

#define DATA_PAUSE_STATUS           1
#define DATA_NON_PAUSE_STATUS  0
#define DATA_UNKNOWN_PAUSE_STATUS   0xFF

#if CONFIG_DRV_AUDIO_IN_SUPPORT
extern void SPDIF_InInit(void);
extern void SPDIF_InUnInit(void);
extern void vSendAUDINCmd(UINT32 u4Cmd, BYTE bPri);
extern BOOL SPDIF_InLockCheck(void);
extern void vAinChSel(UINT8 u1Ch);
extern void vSpdifChInSel(UINT8 u1Ch) ;
extern void SPDIF_InChSel(UINT8 u1Ch);
extern void vAIN_ACK_CFG_Multi(AUDIO_IN_TYPE_T u1Input, AUDIN_DIGITAL_DETECT u1Detect);
extern UINT8 AUD_HDMIRX_SF_Switch(UINT8 u1Msg);
 // no real use
extern void SPDIF_InRegisterDump(void);
extern void AUD_SpdifInPinSelect(BOOL fgSet);
#if CONFIG_DRV_ISPDIF_IN_SUPPORT
extern void ISPDIF_InInit(void);
extern void ISPDIF_UnInit(void);
extern void ISPDIF_InputSW(AUDIO_IN_TYPE_T u1Input);
extern void vAPLLMSModeRoughTune(void);
extern void vAPLLMSModeFineTune(UINT32 u4SampleCnt, UINT16 u2FirstCnt);
extern UINT8 AUD_SF_Switch(UINT8 u1Msg);
extern void vReserveFS(UINT32 u4FS);
extern void vMonitorRWPTR(void);
extern void vCheckFSChange(void);
#endif
 #endif

#endif  //_SPDIF_IF_H_


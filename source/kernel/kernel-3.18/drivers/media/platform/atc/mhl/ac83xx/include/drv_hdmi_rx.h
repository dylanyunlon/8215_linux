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

#ifndef _DRV_HDMI_RX_H_
#define _DRV_HDMI_RX_H_

#include "x_typedef.h"
#include "chip_ver.h"
#include "drv_config.h"
#include "x_audin.h"
#include "drv_vdoin.h"


/* HDMI_SWITCH */
enum eHDMI_RX_IN_SWITCH_NO {
	HDMI_RX_SWITCH_INIT = 0,
	HDMI_RX_SWITCH_1,
	HDMI_RX_SWITCH_2,
	HDMI_RX_SWITCH_3,
	HDMI_RX_SWITCH_4,
	HDMI_RX_SWITCH_5,
	HDMI_RX_SWITCH_6
};

typedef enum {
	HDMI_RX_DEBUG_EDID = (1 << 0),
	HDMI_RX_DEBUG_HOT_PLUG = (1 << 1),
	HDMI_RX_DEBUG_HDCP = (1 << 2),
	HDMI_RX_DEBUG_HDCP_RI = (1 << 3),
	HDMI_RX_DEBUG_HV_TOTAL = (1 << 4),
	HDMI_RX_DEBUG_AUDIO = (1 << 5),
	HDMI_RX_DEBUG_INFOFRAME = (1 << 6),
	HDMI_RX_DEBUG_DEEPCOLOR = (1 << 7),
	HDMI_RX_DEBUG_3D = (1 << 8),
	HDMI_RX_DEBUG_XVYCC = (1 << 9),
	HDMI_RX_DEBUG_SYNC_DET = (1 << 10),
	HDMI_RX_DEBUG_MHL = (1 << 11),
	HDMI_RX_DEBUG_ALL = (1 << 23),/* max 23 */
} HDMI_Rx_DEBUG_MESSAGE_T;

/* #if (!CONFIG_DRV_MT8520) */
#if 1 /*(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)*/

void vUpdateHdmiRxEdid(UINT8 u1UpdateType);
void vSetRxPort1HPDLevel(BOOL fgHighLevel);
void vSetRxPort2HPDLevel(BOOL fgHighLevel);
void vDumpHdmiRxEdid(UINT8 u1Edid);
void vIssueHdmiRxUpdateEdidCmd(UINT8 u1EdidReady);

/* #if (CONFIG_DRV_HDMI_RX) */
void vAudInGetRxInAudInfo(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info);
void vEnableHdmiRxTask(UINT8 u1Enable);
#if CONFIG_DRV_CUSTOM_JSN
void vHdmiRxNotifyPacket(void);
#endif
#if (CONFIG_DRV_HDMI_RX_INTERNAL)
void vEnableHdmiRxAudTask(UINT8 u1Enable);
void  vSetHDMIInAudTxBypassMode(BOOL fgBypassMode);
#endif
/* #endif */
void vTxSetRxReceiverMode(void);
void vTxSetRxRepeaterMode(void);
void vTxSetKsvReady(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch);
BOOL fgIs640x480PEnable(void);
BOOL fgIsHdmiRxBoardExist(void);
void vSetHdmiRxSpdifOn(BOOL fgOn);/*fgOn: TRUE, turn on HDMI RX spdif out, FALSE:turn FFF HDMI RX spdif out */
void vSetHdmiRxI2sOn(UINT8 u1OnBit);/*u1OnBit see x_audin.h HDMI_I2S_CH_SEL_T,
for example:u1OnBit=0x0f means all I2S channels are on,  u1OnBit=0x00 means all I2S channels are off.*/
/* #if (CONFIG_DRV_HDMI_RX) */
int GetHdmiRxAudioInfoFrame(HDMI_RX_Audio_InfoFrame *pAudioInfoFrame);/*HDMI_RX_Audio_InfoFrame.
please see x_audin.h, just info is used */
int GetHdmiRxAudioChannelStatus(HDMI_RX_AUDIO_CHSTS *pHdmiRxChStat);/*HDMI_RX_AUDIO_CHSTS in x_audin.h */
/* #endif */
UINT8 u1GetHDMIRxACPType(void);
#endif
#if CONFIG_DRV_HDMI_RX
void vVSWGetRXInfo(INPUT_DEVICE_INFO_T *pv_get_info);
#endif
BOOL fgUpStreamNeedAuth(void);
BOOL fgRxIsHdmiMode(void);
UINT8 bHDMIDeepColorStatus(void);
void vHdmiRxSetTxPacket(void);
void vHDMISetSwitch(UINT8 bSwitch);
BOOL fgIsHdmiRxBoardExist(void);
/* CLI command for external */
INT32 _HdmiRxDbgChgAudioOutToUpdateEdidMode(INT32 i4Argc, const CHAR **szArgv);
INT32 _EnableHdmiRxAllLog(INT32 i4Argc, const CHAR **szArgv);
INT32 _vShowRxStatus(INT32 i4Argc, const CHAR **szArgv);
INT32 _vShowRxInfoStatus(INT32 i4Argc, const CHAR **szArgv);
INT32 _ReadRxEdidLog(INT32 i4Argc, const CHAR **szArgv);
INT32 _ReadRxEdidPA(INT32 i4Argc, const CHAR **szArgv);
INT32 _ModEdidPA(INT32 i4Argc, const CHAR **szArgv);
INT32 _ModEdidBlock(INT32 i4Argc, const CHAR **szArgv);

INT32 _i4ShowRxHDCPBstatus(INT32 i4Argc, const CHAR **szArgv);
INT32 _i4RxChangeDepth(INT32 i4Argc, const CHAR **szArgv);

void vShowHDMIRxStatus(void);
void vShowHDMIRxInfo(void);
UINT32 u4GetHdmiRxLogLevelFromNorFlash(void);
INT32 i4WriteHdmiRxLogLevelToNorFlash(UINT32 *u4prDbgLevel);




UINT8 bHDMIInputType(void);
UINT8 bHDMI422Input(void);
BOOL fgIsHdmiRxDebug(UINT32 u4MessageType);
BOOL fgHDMIRxIsHdmiMode(void);
BOOL fgHDMIRxDetectTmds(void);
BYTE bRxAcpType(void);
void vShowRepeaterInforFrame(void);
void vHdmiRxForce3D(UINT32 bType);
void vShowAviInforFrame(void);
void vShowGamutInforFrame(void);
void vShowVENDInforFrame(void);
void vShowISRC1InforFrame(void);
void vShowISRC2InforFrame(void);
void vShowSPDInforFrame(void);
void vShowACPInforFrame(void);

void vSetVinHwSetting(BOOL fgEnable, UINT16 u2Width, UINT16 u2Height);
void vSetVinBufAddress(void);
void vHDMIRXVinInit(void);
void vVinCalculateAveargeRGB(UINT16 u2Width, UINT16 u2Height);
void vVinSetFbFlag(BOOL fgEnable);
UINT8 vVinSampleInfo(void);
INT32 _HDMISetCalcFrameAverageRGB(INT32 i4Argc, const CHAR **szArgv);


#endif /* _DRV_HDMI_H_ */

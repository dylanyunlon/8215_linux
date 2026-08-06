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

#ifndef _DRV_HDMI_H_
#define _DRV_HDMI_H_

#include "x_typedef.h"
#include "chip_ver.h"
#include "drv_config.h"


/*********************************************************************
Type definitions
*********************************************************************/

/*********************************************************************
Constant definitions
*********************************************************************/
#define HDMI_OK	     (INT32)(0)
#define HDMI_FAIL     (INT32)(-1)


#define SV_OK	     (INT8)(0)
#define SV_FAIL      (INT8)(-1)



/*********************************************************************
 Export API
*********************************************************************/
extern INT32 HDMI_Init(void);
extern INT32 i4Hdmi_Uninit(UINT32 u4Case);
extern INT32 HDMI_RX_Init(void);
extern INT32 i4HdmiRx_Uninit(UINT32 u4Case);

/* added by zhangyue on 2012-05-26 for debug start */
extern INT32 MHL_Init(void);
extern void vMHLModuleInit(void);
extern INT32 i4Mhl_Uninit(UINT32 u4Case);
extern INT32 vMhlSetDbgLevel(UINT8 level);
extern void IrSetRokuPauseHome(void);

/* added by zhangyue on 2012-05-26 for debug end */

/* extern INT32 MW_HDMI_Init(void); */
/* For IC verify */
extern BOOL fgReadEDID(void);/* for test */
extern void vDBGetEdid(UINT8 u1Type);                      /* WinDebug */
extern void vDBGetSinkAvCap(UINT8 u1Type);                /* WinDebug */
extern void vDBGetInfoFm(UINT8 u1Type, UINT8 u1InfoCode);  /* WinDebug */
extern void vDBGetNCTS(UINT8 u1Type);                      /*WinDebug */
extern void vDBGetAvdToAudPara(UINT8 u1Type);  /* WinDebug */
extern void vDBGetAudToAvdPara(UINT8 u1Type);  /* WinDebug */
extern void vDBGetReg(UINT8 u1Type, UINT32 u4Addr);  /* WinDebug */
extern void vSendAVIInfoFrame(void);
extern void vHdmiUpdataContentType(BYTE contenttype);
extern void vSendVendorSpecificInfoFrame(void);
extern void vHDCPBStatus(void);
extern VOID u2SetModelOptionPlayerType(UINT16 ui2PlayerType);

void vUpdateAFD(UINT8 u1SourceAR, UINT8 u1DisplayAR);
void vUpdateSPD(UINT8 *u1PrSPD, BOOL fgSendSPD);
void vHDMI_I2S_C_Status(void);
void vSetJpegPlayOn(BOOL fgJpegPlayOn);
void vSetHdmiNoEdidCheck(UINT8 u1NoEdidOn);
BOOL fgIsHdmiNoEDIDCheck(void);
void vSetHdmiUserEdidCheck(UINT8 u1UserEdidOn);
void vSetDebugEdidAudioPcm(UINT8 u1ChNum, UINT8 u1Fs, UINT8 u1BitSize);
void vSetDebugEdidSpkAllocation(UINT8 u1SpkAllocation);

void vSetDebugEdidAudDecoder(UINT16 u2AudDec);
void vSetUiXvColorEnable(UINT8 u1Enable);

UINT32 u4HDMIGetVSyncMode(void);
void vHDMISetVSyncMode(BYTE bSyncMode);
void vHDMISet3DSync(BYTE bSyncMode);
void vHDMIPreSet3DSyncDelay(BYTE bSyncMode, BOOL fgPreSetOn);
void vUnSet3DMask(void);
/* #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8555) || (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)
BOOL fgHDMI3DProgressModeAlign(void);
BOOL fgHDMI3DInterlaceModeAlign(void);
#endif */
BOOL vGetTxPortEdid(UINT8 *prBuff, UINT8 u1BlcokNum, UINT8 u1Len);
void vHdmiStatus(void);
void vShowEdidInformation(void);
void vShowEdidRawData(void);
void vShowInforFrame(void);
void vShowOutputVideoResolution(void);
void vShowHpdRsenStatus(void);
void vShowDviOrHdmiMode(void);
void vShowDeepColor(void);
void vShowColorSpace(void);
void vShowXvColor(void);
void vShowHdmiAudioStatus(void);
void vCliSetSRMSignatureChkFlag(UINT8 u1Flag);
BOOL fgHdmiPortUseForDebug(void);
BOOL vIsTmdsOn(void);
void vEnableACRSend(BOOL fgEnable);
void vSendHdmiPacket(BYTE bType, BOOL fgEnable,  BYTE *pr_bData);
BYTE bConvert3DResolutionTo2DResolutionIndex(BYTE bInputResolution);
void vHdmiPacketSendCtl(UINT8 bType, UINT8 bPKEnable, UINT8 bOPPKModeEnable);


#if 1/*CONFIG_DUALTX_DRV_EN*/
extern BOOL d_md_support_dual_hdmi(VOID);
extern BOOL fgReadEDID2(void);/* for test */
extern void vSendAVI2InfoFrame(void);
extern void vSendVendorSpecificInfoFrame2(void);
void vHDMI2_I2S_C_Status(void);
void vUpdateSPD2(UINT8 *u1PrSPD, BOOL fgSendSPD);
void vShowEdid2Information(void);
void vShowEdidRawData2(void);
void vHdmi2Status(void);
void vShowHpdRsen2Status(void);
void vShowOutputVideo2Resolution(void);
void vShowDviOrHdmi2Mode(void);
void vShowDeepColor2(void);
void vShowColorSpace2(void);
void vShowXvColor2(void);
void vShowInforFrame2(void);
void vShowHdmi2AudioStatus(void);
void vSetHdmi2NoEdidCheck(UINT8 u1NoEdidOn);
BOOL fgIsHdmi2NoEDIDCheck(void);
void vHdmi2PacketSendCtl(UINT8 bType, UINT8 bPKEnable, UINT8 bOPPKModeEnable);
extern void vHDCP2BStatus(void);
BOOL fgHdmiMovePatten(void);
void vHdmiMoveCheck(void);
void vDualHDMIAVMute(void);

#if 1 /*CONFIG_SUPPORT_SS*/
extern VOID u2SetPAtoMicom(UINT16 u2Pa);
extern UINT16 u2GetPAtoMicom(VOID);
#endif

#endif

#endif /* _DRV_HDMI_H_ */

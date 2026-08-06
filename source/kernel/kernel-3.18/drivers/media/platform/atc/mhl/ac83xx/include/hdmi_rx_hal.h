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

#ifndef _HDMI_RX_HAL_H_
#define _HDMI_RX_HAL_H_

#include "x_hal_ic.h"
#include "chip_ver.h"
#include "hdmi_rx_hw.h"
#include "windows.h"

#define HDMI_AUD_READ32(offset)             (*(volatile unsigned int *)((g_IO_VBASE_VA + (offset))))
#define HDMI_AUD_WRITE32(offset, value)     ((*(volatile unsigned int *)(g_IO_VBASE_VA + (offset))) = value)

typedef enum {
	RX_NON_DEEP = 0x00,
	RX_30BITS_DEEP_COLOR = 0x01,
	RX_36BITS_DEEP_COLOR = 0x02,
	RX_48BITS_DEEP_COLOR = 0x03
} RX_DEEP_COLOR_MODE;

typedef enum {
	RX_CH_MAP_RGB = 0,
	RX_CH_MAP_RBG = 1,
	RX_CH_MAP_GRB = 2,
	RX_CH_MAP_BRG = 3,
	RX_CH_MAP_GBR = 4,
	RX_CH_MAP_BGR = 5,
	RX_CH_MAP_RGB1 = 6,
	RX_CH_MAP_RGB2 = 7,

} RX_VIDEO_CH_MAP_TYPE;

#if 1 /* HDMI_SWITCH */
enum eHDMI_SWITCH_NO {
	HDMI_SWITCH_INIT = 0,
	HDMI_SWITCH_1,
	HDMI_SWITCH_2,
	HDMI_SWITCH_3,
	HDMI_SWITCH_4,
	HDMI_SWITCH_5,
	HDMI_SWITCH_6
};
#endif

enum eHDMI_Rx_PHY_RESET {
	HDMI_RST_ALL = 1,
	HDMI_RST_EQ,
	HDMI_RST_DEEPCOLOR,
	HDMI_RST_FIXEQ,
	HDMI_RST_RTCK
};

typedef enum {
	HDMI_ANA_BAND_NULL = 0,
	HDMI_ANA_BAND_10_27M, /* 10~27M */
	HDMI_ANA_BAND_27_40M, /*  27~40M */
	HDMI_ANA_BAND_40_160M, /* 40 ~160M */
	HDMI_ANA_BAND_160_250M,
	HDMI_ANA_BAND_250_MAX,

	MHL_ANA_BAND_PP_0_30M, /*  mhl pixel repeat mode */
	MHL_ANA_BAND_PP_30_MAX,

	MHL_ANA_BAND_0_50M,  /* normal mode */
	MHL_ANA_BAND_50_MAX,
} HDMI_ANA_BAND;

enum HDMI_RX_AUDIO_FS {
	SW_44p1K,
	SW_88p2K,
	SW_176p4K,
	SW_48K,
	SW_96K,
	SW_192K,
	SW_32K,
	HW_FS
};

void vHalSetEqZeroValueVar(UINT32 u4Data);
void vHalSetEqBoostValueVar(UINT32 u4Data);
void vHalSetEqSelValueVar(UINT32 u4Data);
void vHalSetEqGainValueVar(UINT32 u4Data);
void vHalSetLBWValueVar(UINT32 u4Data);
void vHalSetRxHdcpMask1Var(UINT32 u4Data);
void vHalSetRxHdcpMask2Var(UINT32 u4Data);


/* hal function, by wangwj */
void HDMI_HalTmdsOn(BOOL fgOn);
/* void HDMI_HalHwInit(void); */
void HDMI_HalReset(void);
void HDMI_HalEnableIntr(void);
void HDMI_HalPhyReset(UINT8 u1ResetSel);
void HDMI_HalResetPhySp(void);
void HDMI_HalSwReset(void);
void HDMI_HalDigtailPhyReset(void);

void HDMI_HalMuteAudio(void);
void HDMI_HalUnMuteAudio(void);
void HDMI_HalOpenApll(void);
void HDMI_HalI2sLRInv(BOOL fgInv);
void HDMI_HalSetAudI2sFormat(UINT8 u1fmt, UINT8 u1Cycle);
void HDMI_HalSetLRClkEdge(UINT8 u1EdgeFmt);
void HDMI_HalSetI2sMclk(UINT8 u1MclkType);
void HDMI_HalSetAudioFS(enum HDMI_RX_AUDIO_FS eFS);
UINT8 HDMI_HalGetI2sMclk(void);
void HDMI_HalEnableAudClk(void);
void HDMI_HalSetAudMuteCondition(void);
void HDMI_HalEnableAacToSd0123(void);

UINT8 HDMI_HalGetSCDT(void);  /*  check hsync */
UINT8 HDMI_HalGetCKDT(void);         /* check clock */
UINT8 HDMI_HalGetPwr5V(void);        /* check +5V */




void HDMI_HalSelAnaBandExt(HDMI_ANA_BAND eBand); /*  hal interface */
void HDMI_HalSelHdmiAnaBand(HDMI_ANA_BAND eBand);
void HDMI_HalSelMhlAnaBand(HDMI_ANA_BAND eBand);

UINT32 HDMI_HalGetXclkCnt(void);
UINT32 HDMI_HalGetTmdsClockExt(void);
UINT32 HDMI_HalGetPixelClockExt(void);
UINT32 HDMI_HalGetHtotalExt(void);
UINT32 HDMI_HalGetVsyncFreq(void);


void HDMI_HalSetTmdsFifoRWPointerDiff(void);
void HDMI_HalSetTmdsFifoRWPointerFreeRun(void);
void HDMI_HalClearRxPclkChgStatus(void);
UINT8 HDMI_HalGetRxHdcpStatus(void);
BOOL HDMI_HalIsHdmiRXAuthDone(UINT8 u1Data);
BOOL HDMI_HalChkAviInforFrameExist(void);







void HalDisableHDCPDDCPort(void);
void HalEnableHDCPDDCPort(void);




/*void vHalSwResetHdmiRxModule(void);
void vHalMuteHdmiRxAudioOut(void);
void vHalUnMuteHdmiRxAudioOut(void);
void vHalOpenAPLL(void);
void vHalSetRxI2sLRInv(UINT8 u1LRInv);
void vHalSetRxI2sAudFormat(UINT8 u1fmt, UINT8 u1Cycle);
void vHalSetLRCKEdge(UINT8 u1EdgeFmt);
void vHalSetRxI2sMclk(UINT8 u1MclkType);

void vHalEnableRxAudClk(void);
void vHalSetRxAudMuteCondition(void);
void vHalEnableRxAACToSd0Sd1Sd2Sd3(void);
UINT8 u1HalChkDataEnableExist(void);
void vHalSelANABand(void);
void vHalSelMHLANABand(void);
UINT32 u4HalGetRxPixelClock(void);
void vHalSetTmdsFifoRWPointerDiff(void);
void vHalSetTmdsFifoRWPointerFreeRun(void);
void vHalClearRxPclkChgStatus(void);
UINT8 vHalGetRxHdcpStatus(void);
BOOL fgHalCheckHdmiRXAuthDone(UINT8 u1Data);
BOOL fgHalCheckAviInforFrameExist(void);
*/
UINT8 HDMI_HalReadAviType(void);
UINT8 HDMI_HalReadAviVersion(void);
UINT8 HDMI_HalReadAviLength(void);
UINT8 HDMI_HalReadAviCheckSum(void);
UINT8 HDMI_HalReadAviByte1(void);
UINT8 HDMI_HalReadAviByte2(void);
UINT8 HDMI_HalReadAviByte3(void);
UINT8 HDMI_HalReadAviByte4(void);
UINT8 HDMI_HalReadAviByte5(void);
UINT8 HDMI_HalReadAviByte6(void);
UINT8 HDMI_HalReadAviByte7(void);
UINT8 HDMI_HalReadAviByte8(void);
UINT8 HDMI_HalReadAviByte9(void);
UINT8 HDMI_HalReadAviByte10(void);
UINT8 HDMI_HalReadAviByte11(void);
UINT8 HDMI_HalReadAviByte12(void);
UINT8 HDMI_HalReadAviByte13(void);
UINT8 HDMI_HalReadAviByte14(void);
UINT8 HDMI_HalReadAviByte15(void);
UINT8 HDMI_HalReadAudioInfType(void);
UINT8 HDMI_HalReadAudioInfVersion(void);
UINT8 HDMI_HalReadAudioInfLength(void);
UINT8 HDMI_HalReadAudioInfCheckSum(void);
UINT8 HDMI_HalReadAudioInfByte1(void);
UINT8 HDMI_HalReadAudioInfByte2(void);
UINT8 HDMI_HalReadAudioInfByte3(void);
UINT8 HDMI_HalReadAudioInfByte4(void);
UINT8 HDMI_HalReadAudioInfByte5(void);
UINT8 HDMI_HalReadAudioInfByte6(void);
UINT8 HDMI_HalReadAudioInfByte7(void);
UINT8 HDMI_HalReadAudioInfByte8(void);
UINT8 HDMI_HalReadAudioInfByte9(void);
UINT8 HDMI_HalReadAudioInfByte10(void);
UINT8 HDMI_HalReadAcpHb0Header(void);
UINT8 HDMI_HalReadHb1HeaderAcpType(void);
UINT8 HDMI_HalReadAcpHb2Header(void);
UINT8 HDMI_HalReadAcpPB0(void);
UINT8 HDMI_HalReadAcpPB1(void);
UINT8 HDMI_HalReadAcpPB2(void);
UINT8 HDMI_HalReadAcpPB3(void);
UINT8 HDMI_HalReadAcpPB4(void);
UINT8 HDMI_HalReadAcpPB5(void);
UINT8 HDMI_HalReadAcpPB6(void);
UINT8 HDMI_HalReadAcpPB7(void);
UINT8 HDMI_HalReadAcpPB8(void);
UINT8 HDMI_HalReadAcpPB9(void);
UINT8 HDMI_HalReadAcpPB10(void);
UINT8 HDMI_HalReadAcpPB11(void);
UINT8 HDMI_HalReadAcpPB12(void);
UINT8 HDMI_HalReadAcpPB13(void);
UINT8 HDMI_HalReadAcpPB14(void);
UINT8 HDMI_HalReadAcpPB15(void);
UINT8 HDMI_HalReadSPDType(void);
UINT8 HDMI_HalReadSPDVersion(void);
UINT8 HDMI_HalReadSPDLength(void);
UINT8 HDMI_HalReadSPDByte1(void);
UINT8 HDMI_HalReadSPDByte2(void);
UINT8 HDMI_HalReadSPDByte3(void);
UINT8 HDMI_HalReadSPDByte4(void);
UINT8 HDMI_HalReadSPDByte5(void);
UINT8 HDMI_HalReadSPDByte6(void);
UINT8 HDMI_HalReadSPDByte7(void);
UINT8 HDMI_HalReadSPDByte8(void);
UINT8 HDMI_HalReadSPDByte9(void);
UINT8 HDMI_HalReadSPDByte10(void);
UINT8 HDMI_HalReadSPDByte11(void);
UINT8 HDMI_HalReadSPDByte12(void);
UINT8 HDMI_HalReadSPDByte13(void);
UINT8 HDMI_HalReadSPDByte14(void);
UINT8 HDMI_HalReadSPDByte15(void);
UINT8 HDMI_HalReadSPDByte16(void);
UINT8 HDMI_HalReadSPDByte17(void);
UINT8 HDMI_HalReadSPDByte18(void);
UINT8 HDMI_HalReadSPDByte19(void);
UINT8 HDMI_HalReadSPDByte20(void);
UINT8 HDMI_HalReadSPDByte21(void);
UINT8 HDMI_HalReadSPDByte22(void);
UINT8 HDMI_HalReadSPDByte23(void);
UINT8 HDMI_HalReadSPDByte24(void);
UINT8 HDMI_HalReadSPDByte25(void);
UINT8 HDMI_HalReadSPDByte26(void);
UINT8 HDMI_HalReadGamutHb0(void);
UINT8 HDMI_HalReadGamutHb1(void);
UINT8 HDMI_HalReadGamutHb2(void);
UINT8 HDMI_HalReadGamutPB0(void);
UINT8 HDMI_HalReadGamutPB1(void);
UINT8 HDMI_HalReadGamutPB2(void);
UINT8 HDMI_HalReadGamutPB3(void);
UINT8 HDMI_HalReadGamutPB4(void);
UINT8 HDMI_HalReadGamutPB5(void);
UINT8 HDMI_HalReadGamutPB6(void);
UINT8 HDMI_HalReadGamutPB7(void);
UINT8 HDMI_HalReadGamutPB8(void);
UINT8 HDMI_HalReadGamutPB9(void);
UINT8 HDMI_HalReadGamutPB10(void);
UINT8 HDMI_HalReadGamutPB11(void);
UINT8 HDMI_HalReadGamutPB12(void);
UINT8 HDMI_HalReadGamutPB13(void);
UINT8 HDMI_HalReadGamutPB14(void);


/* Clear Interrupt, set 1 to clear */
void HDMI_HalClearNewAviIntStatus(void);
void HDMI_HalClearNewAudIntStatus(void);
void HDMI_HalClearNewSpdIntStatus(void);
void HDMI_HalClearNewMpegIntStatus(void);
void HDMI_HalClearNewUnRecIntStatus(void);
void HDMI_HalClearNewAcpIntStatus(void);
void HDMI_HalClearNewVSIntStatus(void);
void HDMI_HalClearNewNOVSIntStatus(void);
void HDMI_HalClearNewISRC1IntStatus(void);
void HDMI_HalClearVSYNCIntStatus(void);
void HDMI_HalSetVSNewOnly(BOOL fgEnable);
void HDMI_HalSetISRC1NewOnly(BOOL fgEnable);


BOOL HDMI_HalCheckIsPclkChanged(void);
void HDMI_HalClearPclkChangedIntState(void);
BOOL HDMI_HalIsHdmiMode(void);
BOOL HDMI_HalIsGcpMuteEnable(void);
BOOL HDMI_HalIsNewAcp(void);
BOOL HDMI_HalIsAcpInforFrameExist(void);
UINT8 HDMI_HalGetAcpType(void);
UINT8 HDMI_HalGetAcpHeader(void);
void HDMI_HalGetAcpPacket(UINT8 *pu1AcpPacketData);
void HDMI_HalSelectAcppacket(UINT8 u1Header);

void HDMI_HalGetAviInfoframe(UINT8 *bAviinfoframe);
void HDMI_HalGetAudioInfoframe(BYTE *bAudioInfoframe);
void HDMI_HalGetVSInfoframe(BYTE *bVSinfoframe);
void HDMI_HalGetISRC1Infoframe(BYTE *bISRC1infoframe);
void HDMI_HalGetMpegInfoframe(BYTE *bMpegInfoframeData);
void HDMI_HalGetSpdInfoframe(BYTE *bSpdInfoframeData);
void HDMI_HalGetGamutPacket(BYTE *bGamutData);
void HDMI_HalGetUnRecPacket(BYTE *bUnRecPacketData);

BOOL HDMI_HalGetAudioInfoFrameExt(Audio_InfoFrame *pAudioInfoFrame);
UINT32	HDMI_HalGetHfreqExt(void);
UINT32	HDMI_HalGetVfreqExt(void);
UINT8 HDMI_HalGetMpegAddrHeader(void);
void HDMI_HalMpegAddrSetSelectPacket(BYTE bHeader);
UINT8 HDMI_HalMpegAddrGetSelectPacket(void);
UINT32 HDMI_HalGetRxHwCTSValue(void);
UINT32 HDMI_HalGetRxHwNValue(void);
BOOL HDMI_HalIsNotDeepColorMode(void);
UINT32 HDMI_HalGetDeepColorBpp(void);
void HDMI_HalReInitAudioClock(void);
BOOL HDMI_HalIsNoAvi(void);
void HDMI_HalClearVideoModeByte0(void);
void HDMI_HalClearVideoModeByte1(void);
void HDMI_HalClearVideoModeByte2(void);
void HDMI_HalClearVideoModeByte3(void);
void HDMI_HalClearIntrState1Bit0_Bit7(void);
void HDMI_HalDisableEncodeSync(void);
void HDMI_HalRxDisable422UpSample(void);
void HDMI_HalSetRxRGBBlankValue(UINT8 u1Blue , UINT8 u1Green, UINT8 u1Red);
void HDMI_HalSetRxYCbCrBlankValue(UINT8 u1Cb , UINT8 u1Y, UINT8 u1Cr);
VOID HDMI_HalSetRxPclk2XRepeat(BOOL fgEn);
BOOL HDMI_HalIsPclk2XRepeat(void);
void HDMI_HalSetVideoChannelMap(UINT8 u1Data);
BOOL HDMI_HalIsHResChg(void);
void HDMI_HalClearHresChgIntrState(void);
void HDMI_HalResetTDFifoAutoRead(void);
void HDMI_HalSetTDFifoAutoReadEnable(BOOL fgEn);
UINT32 HDMI_HalGetActiveWidth(void);
UINT32 HDMI_HalGetVFrontPorch(void);
UINT32 HDMI_HalGetVBackPorch(void);
UINT32 HDMI_HalGetActiveHeight(void);
UINT32 HDMI_HalGetHTotal(void);
UINT32 HDMI_HalGetVTotal(void);
BOOL HDMI_HalIsHresStable(void);
BOOL HDMI_HalIsVresStable(void);
BOOL HDMI_HalIsHdcpDecrptOn(void);
BOOL HDMI_HalIsSCDTEnable(void);
BOOL HDMI_HalGetHsyncPolarity(void);
BOOL HDMI_HalGetVsyncPolarity(void);


void HalEnableRxPhyRtckAuto(void);
void HalDisableRxPhyRtckAuto(void);

void HDMI_HalClearModeChgIntState(void);
BOOL HDMI_HalIsVResStable(void);
BOOL HDMI_HalIsVResMute(void);
void HDMI_HalSetVResMute(void);
void HDMI_HalClearVResMute(void);
void HDMI_HalDisableRxAvMute(void);
void HDMI_HalEnableAvMuteRecv(void);
BOOL HDMI_HalIsInterlace(void);
/* void vHalRxEnableTDFifoAutoRead(void); */
/* void vHalRxDisableTDFifoAutoRead(void); */
/* void vHalSetRxHDMIHPDLow(); */
void HDMI_HalSetHpd(BOOL fgHigh);

void HalSetRxInPortSwitchEnable(UINT8 bSwitch);
void HalSetHDMIRXPowerOff(void);
BOOL HalHdmiRxCrc(INT16 ntry);
BOOL HalIsINTR3_CEA_NEW_CP(void);
BOOL HalIsINTR3_CP_SET_MUTE(void);
BOOL HalIsINTR3_P_ERR(void);
BOOL HalIsINTR3_NEW_UNREC(void);
BOOL HalIsINTR3_NEW_MPEG(void);
BOOL HalIsINTR3_NEW_AUD(void);
BOOL HalIsINTR3_NEW_SPD(void);
BOOL HalIsINTR3_NEW_AVI(void);

BOOL HalIsINTR_NEW_VS(void);
BOOL HalIsINTR_VSYNC(void);
BOOL HalIsINTR_NO_VS(void);
BOOL HalIsINTR_NEW_ISRC1(void);
void HalHDMIRxEnableVsyncInt(BOOL fgEnable);

BOOL HalIsINTR2_HDMI_MODE(void);
BOOL HalIsINTR2_VSYNC(void);
BOOL HalIsINTR2_SOFT_INTR_EN(void);
BOOL HalIsINTR2_CKDT(void);
void  HalClearINTR2_CKDT(void);
void  HalEnableINTR2_CKDT(BOOL fgenable);
BOOL HalIsINTR2_SCDT(void);
BOOL HalIsINTR2_GOT_CTS(void);
BOOL HalIsINTR2_NEW_AUD_PKT(void);
BOOL HalIsINTR2_CLK_CHG(void);
BOOL HalIsINTR1_HW_CTS_CHG(void);
BOOL HalIsINTR1_HW_N_CHG(void);
BOOL HalIsINTR1_FIFO_ERR(void);
BOOL HalIsSOFT_INTR_EN(void);
BOOL HalIsINTR_OD(void);
BOOL HalIsINTR_POLARITY(void);
BOOL HalIsINTR_STATE(void);
UINT32 HalReadINTR_STATE0(void);
UINT32 HalReadINTR_STATE1(void);
BOOL HalIsINTR7_RATIO_ERROR(void);
BOOL HalIsINTR7_AUD_CH_STAT(void);
BOOL HalIsINTR7_GCP_CD_CHG(void);
BOOL HalIsINTR7_GAMUT(void);
BOOL HalIsINTR7_HBR(void);
BOOL HalIsINTR7_SACD(void);
BOOL HalIsINTR6_PRE_UNDERUN(void);
BOOL HalIsINTR6_PRE_OVERUN(void);
BOOL HalIsINTR6_PWR5V_RX2(void);
BOOL HalIsINTR6_PWR5V_RX1(void);
BOOL HalIsINTR6_NEW_ACP(void);
BOOL HalIsINTR6_P_ERR2(void);
BOOL HalIsINTR6_PWR5V_RX0(void);
BOOL HalIsINTR5_FN_CHG(void);
BOOL HalIsINTR5_AUDIO_MUTE(void);
BOOL HalIsINTR5_BCH_AUDIO_ALERT(void);
BOOL HalIsINTR5_VRESCHG(void);
BOOL HalIsINTR5_HRESCHG(void);
BOOL HalIsINTR5_POLCHG(void);
BOOL HalIsINTR5_INTERLACEOUT(void);
BOOL HalIsINTR5_AUD_SAMPLE_F(void);
BOOL HalIsINTR4_PKT_RECEIVED_ALERT(void);
BOOL HalIsINTR4_HDCP_PKT_ERR_ALERT(void);
BOOL HalIsINTR4_T4_PKT_ERR_ALERT(void);
BOOL HalIsINTR4_NO_AVI(void);
BOOL HalIsINTR4_CTS_DROPPED_ERR(void);
BOOL HalIsINTR4_CTS_REUSED_ERR(void);
BOOL HalIsINTR4_OVERRUN(void);
BOOL HalIsINTR4_UNDERRUN(void);
void HalSetIntOnNewAviOnlyEnable(BOOL fgEnable);
void HalSetIntOnNewAcpOnlyEnable(BOOL fgEnable);
void HalSetIntOnNewSpdOnlyEnable(BOOL fgEnable);
void HalSetIntOnNewAudioInfOnlyEnable(BOOL fgEnable);
void HalSetIntOnNewMpegInfOnlyEnable(BOOL fgEnable);
void HalSetIntOnNewUnrecInfOnlyEnable(BOOL fgEnable);

BOOL   HalHDMIRxHDAudio(void);
BOOL   HalHDMIRxDSDAudio(void);
BOOL   HalHDMIRxAudioPkt(void);
BOOL   HalHDMIRxMultiPCM(void);
UINT8   HalHDMIRxAudioCHSTAT0(void);
UINT8   HalHDMIRxAudioCHSTAT1(void);
UINT8   HalHDMIRxAudioCHSTAT2(void);
UINT8   HalHDMIRxAudioCHSTAT3(void);
UINT8   HalHDMIRxAudioCHSTAT4(void);
UINT8  HalHDMIRxAudValidCHGet(void);
UINT8    HalHDMIRxAudFsGet(void);
UINT32  HalHDMIRxAudErrorGet(void);
void  HalHDMIRxSetAudValidCH(UINT8 u1ValidCh);
BOOL HalIsINTR8_AUDFMTCHG(void);
BOOL HalIsINTR8_AUDCHSTATUSCHG(void);
void  HalHDMIRxSetAudMuteCH(UINT8 u1MuteCh);
void HDMIRxIntMask(BOOL fgOn);
void HalHdmiRxAudResetAfifo(void);
BOOL HalGetMhlAudPlayStatus(void);
BOOL HalChkAudPktReady(void);
void HalHDMIRxAudResetMCLK(void);
/* void HalHdmiRxAudioReset(void); */
/* BOOL  bHalGetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame); */
BOOL HalCheckIsAAC(void);
void HalSetHDMIRxHBR(BOOL fgHBR);
void HalSetHDMIRxI2S(void);
BOOL HalHDMIRxAPLLStatus(void);
void HalSetHDMIRxDSD(BOOL fgDSD);
void HalHdmiRxAudBypass(BOOL fgBypass, BOOL fgBypassSPDIF2Tx);

/* UINT8 u1HalGetRxI2sMclk(void); */


void  HalRxHdcpReset(void);
void  HalDisableHDCPDDCPort(void);
void  HalEnableHDCPDDCPort(void);
void  HalSetKsvReadyBit(void);
void  HalClearKsvReadyBit(void);
void  HDMI_HalLoadEdid2Sram(UINT8 *pEdid, UINT32 u4Size);
void  HDMI_HalLoadHdcp2Sram(UINT8 *prHdcpKey);
/* void  vHalSetRepeaterMode(BOOL fgRepeater); */
void  HalSetHdmiCapable(BOOL fgHdmiCapable);
void  HalWriteKsvList(BYTE *prKsvList, BYTE bCount);
UINT32  HalGetKsvFifoAddr(void);
void  HalTriggerSHA(void);
void  HalRptStartAddrClr(void);
BOOL  HalHdcpAuthenticationStart(void);
BOOL  HalHdcpAuthenticationDone(void);
void  HalClearHdcpAuthenticationStartStatus(void);
void  HalClearHdcpAuthenticationDoneStatus(void);
BOOL  HalHdcpHdmiMode(void);
void  HalSetBstatus(UINT16 u2Bstatus);
void  HalSetSHALength(UINT32 u4Length);
void  HalSetSHAAddr(UINT32 u4Addr);
void  HalSetKsvStop(BOOL fgRiscAccressEnable);
/* void  RxUse27M(void); */
BOOL  HalIsVReady(void);
void  HDMI_HalGetAksv(BYTE *prRxAKSV);
UINT16 HDMI_HalGetRi(void);
void  HDMI_HalGetAn(BYTE *prRxAn);
void  HDMI_HalGetBksv(BYTE *prRxBKSV);

UINT32  HDMI_HalGetVblank(void);


BYTE HalGetUnRecPacketHeader(void);
void HalGetUnRecPacket(BYTE *bUnknowPacketData);
void SetSelectUnRecpacket(BOOL fgEnable, BYTE bHeader);



void HalGetAviInfoframe(BYTE *bAviinfoframe);
/* void vHalGetAudioInfoframe(BYTE *bAudioInfoframe); */

void HalGetVSInfoframe(BYTE *bVSinfoframe);
void HalGetISRC1Infoframe(BYTE *bISRC1infoframe);

void HalGetMpegInfoframe(BYTE *bMpegInfoframeData);
void HalGetSpdInfoframe(BYTE *bSpdInfoframeData);
void HalGetGamutPacket(BYTE *bGamutData);

void HDMI_HalEqCalibrate(void);

/* void vHalClearNewAviIntStatus(void); */
/* void vHalClearNewAudIntStatus(void); */
/* void vHalClearNewSpdIntStatus(void); */
/* void vHalClearNewMpegIntStatus(void); */
/* void vHalClearNewUnRecIntStatus(void); */
void HalClearGamutIntStatus(void);
/* BYTE vHalGetMpegAddrHeader(void); */
/* UINT8 u1HalChkPwr5VExist(void); */
/* UINT8 u1HalChkCKDTExist(void);//check Sync */

void vHalRxMHLTMDSCTRL(UINT8 bOnOff);
UINT32 MHLStable(void);
void MHLSetChannelOrder(UINT8 u1Value);
UINT8 MHLGetChannelOrder(void);

void HalHdmiRxSetApll(void);

void HDMI_Set_I2S(void);

void HalHdmiRxAudInitSetting(void);
void HalHdmiAudRegReset(void);
BOOL HalHdmiRxAudResetAudio(void);
void HDMI_HalResetHdcp(void);
void HalHdmiAcrRst(void);
extern UINT32 _u4DebugRxMessageType;
/*extern int gpio_direction_output(unsigned, int);
extern int gpio_direction_input(unsigned);*/
/* #if DRV_SUPPORT_MHL_RX */
#include "mhl_rx_cbus_ctrl.h"
#include "mhl_rx_cbus.h"

extern BOOL _bMHLModeBackup;
extern BOOL _bMHLMode;
extern BOOL _bPPModeBackup;
extern BOOL _bPPMode;
extern UINT8   _bInternalEdid; /*  0 - EDID, 1 - InternalEDID */
extern BOOL is_sink_attached; /* mhl detected flag */
/*extern int gpio_configure(unsigned gpio, int dir, int value);
extern int gpio_get_value(unsigned gpio);*/
extern struct device *hdmi_dev;
extern struct gpio_desc *hdmi_hpd_desc;
extern struct pinctrl *pinctrl_hdmi;

#endif

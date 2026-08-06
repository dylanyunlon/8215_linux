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


#ifndef _HDMI_RX_CTRL_H_
#define _HDMI_RX_CTRL_H_
/* #define DEFINE_IS_LOG           HDMI_IsLog */
#include "x_debug.h"
#include "x_audin.h"
#include "rx_io.h"
#include "typedef.h"
#include "drv_av_d.h"
#include "mhl_private.h"
#include "mhl_drv_if.h"
#include "hdmi_rx_hal.h"

#define _SUPPORT_RX_REPEATER_ /* for HDCP repeater */
#define MAX_PACKET 16
/*#define DEFINE_IS_LOG           HDMI_IsLog */
#define HDMI_8CH 0
#define MUTE_TEMP 1
#define Support_EQ              0
#define Support_HWAdaptive_EQ   0
#define DTV_5365_Reset_Flow 1
#if defined(CC_MT5387) || defined(CC_MT5363)
#define Support_EQ_HWAuto   1   /* 1:Analog 0:Software */
#define PureAnalog_EQ  1    /* 1: PureAnalog_EQ 0:SemiAnalog_EQ */
#else
#define Support_EQ_HWAuto   0   /* 1:Analog 0:Software */
#define PureAnalog_EQ  0    /* 1: PureAnalog_EQ 0:SemiAnalog_EQ */
#endif
#ifdef CC_Support_Sil9285
extern UINT8 _bHDMI9287switchstopflag;
#endif

extern UINT8 _bHPD_Indep_Ctrl;
extern UINT8 _bHDMIState;
extern UINT32 _u4DebugRxMessageType;


extern UINT8 _u1TxEdidReady;
extern UINT8 _u1RxSysState;
extern const CHAR *cAviScanStr[4];
extern const CHAR *cAviBarStr[4];
extern const CHAR *cAviAspectStr[4];
extern const CHAR *cAviRgbYcbcrStr[4];
extern const CHAR *cAviActivePresentStr[2];
extern const CHAR *cAviActiveStr[16];
extern const CHAR *cAviColorimetryStr[4];
extern const CHAR *cAviScanStr[4];
extern const CHAR *cAviRGBRangeStr[4];
extern const CHAR *cAviScaleStr[4];
extern const CHAR *cAviExtColorimetryStr[2];
extern const CHAR *cAviItContentStr[2];
extern const CHAR *cAudChCountStr[8];
extern const CHAR *cAudCodingTypeStr[16];
extern const CHAR *cAudSampleSizeStr[4];
extern const CHAR *cAudFsStr[8];
extern const CHAR *cAudChMapStr[32];
extern const CHAR *cAudDMINHStr[2];
extern const CHAR *cSPDDeviceStr[16];
extern UINT8 u1HDMIINEDID[512];
extern UINT16 _u2EDID0PA;
extern UINT8 _u1EDID0PAOFF;
extern BOOL is_sink_attached;
extern bool isHDMIstop;
extern BOOL _fgHDMIRxBypassMode;


/* extern void HdmiMonitorReset(); */
extern UINT8   _bIntMuteCnt;
extern UINT32   _bDviMdChgCnt;
extern UINT8   _bDviDeChgCnt;
extern UINT8   _bDviPixClkChgCnt;
/* mtk68528 */
extern UINT8 _bDviModeChged;
extern void HDMIRxEDIDNotify(UINT8  *prCbData);
extern void SinkTaskSetHPD(void);
/* extern void SinkTaskClrHPD(void); */
/* extern void SinkTaskSetStandby(BOOL en); */
#if CONFIG_DRV_CUSTOM_JSN
extern BOOL b_s_diag_mode;
#endif
extern void vSetPLLGP_HDMIDDS(BYTE bResIndex);
extern UINT32 HDMI_HalGetHFrontPorch(void);
extern UINT32 HDMI_HalGetHSyncWidth(void);
/* extern UINT32 _u4CKPDRDOLD; */
extern BOOL  readDevcapDone;
extern void sink_read_DeviceCaps(UINT32 u4Index);
extern BOOL fgSinkDiscoveryOk(void);
extern BOOL fgSinkAttachSource(void);
extern void HdmiTimingEventNotify(unsigned int);
#if CONFIG_FASTBOOT_MULTI_PHASE_INIT_DRIVER_EN
extern UINT8 (*_pu1HdmiGetAdacI2SFmt)(void);
#endif
/* extern HDMI_AV_INFO_T _stAvdAVInfo; */
extern BOOL is_sink_hpd_on;

/* HDMI Event Flag */
extern UINT8   _bHdmiFlag;
#define vSetHdmiFlg(arg) (_bHdmiFlag |= (arg))
#define vClrHdmiFlg(arg) (_bHdmiFlag &= (~(arg)))
#define fgIsHdmiFlgSet(arg) ((_bHdmiFlag & (arg)) > 0)
#define HDMI_CHECK (1<<0)
#define HDMI_OCLKDIV2 (1<<1)
#define HDMI_AUDIO_ON (1<<2)
#define HDMI_MODE_CHG (1<<3)
typedef struct {
	UINT8 framesize;
	UINT32 framedata[10];
} HDMI_INFOFRAME_DESCRIPTION;
enum {
	HDMI_InfoFrame_AVI,
	HDMI_InfoFrame_SPD,
	HDMI_InfoFrame_AUDIO,
	HDMI_InfoFrame_MPEG,
	HDMI_InfoFrame_UNREC,
	HDMI_InfoFrame_GAMUT,
	HDMI_InfoFrame_ACP
};
enum {
	HDMI_STATE_NOTREADY,
	HDMI_STATE_INIT,
	HDMI_STATE_PWOFF,
	HDMI_STATE_PWON,
	HDMI_STATE_PWON2,
	HDMI_STATE_SCDT,
	HDMI_STATE_AUTH
};


#define INFORM_MDCHG 1
#ifndef LIMITED_TIMING
#define LIMITED_TIMING 0
#endif
#if 1
#define CHECHCKDT 1
#define PWR5V_INT 0
#else
#define CHECHCKDT 0
#define PWR5V_INT 1
#endif



/*---------------------------------------------------------------------------- */
/* HDMI option flag */
/*---------------------------------------------------------------------------- */

#define HDMI_SUPPORT_EXT_SWITCH (0)
#define HDMI_BYPASS_INITIAL_FLOW (1)
#define HDMI_OFFON_MUTE_COUNT (100) /*ms */
#define DVI_WAIT_STABLE_COUNT (120)
#define DVI_WAIT_NOSIGNAL_COUNT (12)    /*second */
#define HDMI_WAIT_SCDT_STABLE_COUNT (1) /*second */
#define HDMI_TMDS_EQ_ZERO_VALUE (0x1)
#define HDMI_TMDS_EQ_BOOST_VALUE (0xd)
#define HDMI_TMDS_EQ_SEL_VALUE (0xd) /*default */
#define HDMI_TMDS_EQ_GAIN_VALUE (0x2)
#define HDMI_TMDS_HDMI_LBW_VALUE (0x2)
#define HDMI_HDCP_Mask1 (0xff)
#define HDMI_HDCP_Mask2 (0xc3)

/*HDCP Bstatus */
#define DEVICE_COUNT         0x7F
#define MAX_DEVS_EXCEEDED    (0x01<<7)
#define DEVICE_DEPTH         (0x07<<8)
#define MAX_CASCADE_EXCEEDED (0x1<<11)
#define HDMI_MODE            (0x1<<12)

#define TX_MAX_KSV_COUNT 9
#define RX_MAX_KSV_COUNT 10

typedef enum _RxHDCP_State_Type {
	RxHDCP_UnAuthenticated = 0,
	RxHDCP_Computations,
	RxHDCP_WaitforDownstream,
	RxHDCP_AssembleKSVList,
	RxHDCP_WaitVReady,
	RxHDCP_Authenticated
} RxHDCPStateType;

#define HDCP_RECEIVER   0
#define HDCP_REPEATER   1
/*Packet header */
#define ACR_PACKET_HEADER             0x01
#define AUDIO_SAMPLE_PACKET_HEADER    0x02
#define GENERAL_CONTROL_PACKET_HEADER 0x03
#define ACP_PACKET_HEADER             0x04
#define ISRC1_PACKET_HEADER           0x05
#define ISRC2_PACKET_HEADER           0x06
#define DSD_PACKET_HEADER             0x07
#define DST_PACKET_HEADER             0x08
#define HBR_PACKET_HEADER             0x09
#define GAMUT_PACKET_HEADER           0x0A

#define VS_INFOFRAME_HEADER    0x81
#define AVI_INFOFRAME_HEADER   0x82
#define SPD_INFOFRAME_HEADER   0x83
#define AUDIO_INFOFRAME_HEADER 0x84
#define MPEG_INFOFRAME_HEADER  0x85


/*VS  infoframe */
#define VS_INFOFRAME_HB0  0
#define VS_INFOFRAME_HB1  1
#define VS_INFOFRAME_HB2  2
#define VS_INFOFRAME_PB0       3
#define VS_INFOFRAME_PB1       4
#define VS_INFOFRAME_PB2       5
#define VS_INFOFRAME_PB3       6
#define VS_INFOFRAME_PB4       7
#define HDMI_VIDEO_FORMAT_MSK  (0x07<<5)
#define NO_ADDITIONAL_HDMI_VIDEO_FORMAT_PRESENT 0x00
#define EXTENDED_RESOLUTION_FORMAT_PRESENT      0x01/*4kx2k */
#define _3D_FORMAT_PRESENT                       0x02
#define VS_INFOFRAME_PB5          8
#define HDMI_VIC               (0xFF)
#define _3D_STRUCTURE_MSK       (0xF<<4)

#define VS_INFOFRAME_PB6          9
#define _3D_Ext_Data            (0xF<<4)

#define CBUS_DDC_DATA_ADRW 0x0A0
#define CBUS_DDC_DATA_ADRR 0x0A1

/*MHL CBUS HDCP */
#define CBUS_DDC_DATA_HDCP_ADRW 0x074
#define CBUS_DDC_DATA_HDCP_ADRR 0x075
#define CBUS_DDC_DATA_HDCP_BKSV_OFFSET 0x00
#define CBUS_DDC_DATA_HDCP_RI1_OFFSET 0x08
#define CBUS_DDC_DATA_HDCP_AKSV_OFFSET 0x10
#define CBUS_DDC_DATA_HDCP_AINFO_OFFSET 0x15
#define CBUS_DDC_DATA_HDCP_AN_OFFSET 0x18
#define CBUS_DDC_DATA_HDCP_BCAPS_OFFSET 0x40
#define CBUS_DDC_DATA_HDCP_BSTATUS_OFFSET 0x41

#define CBUS_DDC_DATA_HDCP_RSVD1 0X05 /*3 */
#define CBUS_DDC_DATA_HDCP_PJ 0X0A /* 1/ */
#define CBUS_DDC_DATA_HDCP_RSVD2 0X0B /*5 */
#define CBUS_DDC_DATA_HDCP_RSVD3 0X16 /*2 */
#define CBUS_DDC_DATA_HDCP_VH0 0X20/*4 */
#define CBUS_DDC_DATA_HDCP_VH1 0X24/*4 */
#define CBUS_DDC_DATA_HDCP_VH2 0X28/*4 */
#define CBUS_DDC_DATA_HDCP_VH3 0X2C/*4 */
#define CBUS_DDC_DATA_HDCP_VH4 0X30/*4 */
#define CBUS_DDC_DATA_HDCP_RSVD4 0X34/*12 */
#define CBUS_DDC_DATA_HDCP_KSVFIFO 0X43/*1 */
#define CBUS_DDC_DATA_HDCP_RSVD5 0X44/*124 */
#define CBUS_DDC_DATA_HDCP_DBG 0XC0 /*64 */

enum {
	HDMI_3D_Structure_FramePacking = 0,
	HDMI_3D_Structure_FieldAlternative,
	HDMI_3D_Structure_LineAlternative,
	HDMI_3D_Structure_SideBySideFull,
	HDMI_3D_Structure_LDepth,
	HDMI_3D_Structure_LDepthGraph,
	HDMI_3D_Structure_TopBottom,
	HDMI_3D_Structure_SideBySideHalf = 8,
	HDMI_3D_Structure_Unknow
};

typedef struct {
	UINT8 HDMI_3D_Enable;
	UINT8 HDMI_3D_Video_Format;
	UINT8 HDMI_3D_Structure;
	UINT8 HDMI_3D_EXTDATA;
} HDMI_3D_INFOFRAME;

typedef struct HDMI_RX_PACKET_INFO {
	BOOL fgValid;
	BOOL fgChanged;
	BYTE bCount;
	BYTE bLength;
	BYTE PacketHeader;
	unsigned long u4LastReceivedTime;
	unsigned long u4timeout;
	BYTE PacketData[31];
} HDMI_RX_PACKET_INFO;


extern UINT8 _bHdmiMode;
extern UINT8 _bHdmiMD;
extern void HDMIInterRxInit(void);
extern void vHDMIMainLoop(void);
extern void vHDMISetOclkDIV2(UINT8 fgEnable);
extern void vHDMISetOutputDrv(UINT8 bStr);
extern void vHDMIPowerOff(void);
void HDMIVideoOutOn(void);
void HDMIVideoOutOff(void);
extern void HDMIAudioOutOn(void);
extern BOOL fgIsHDMIPlugOn(void);
extern BOOL fgHDMIAudioClkOk(void);
extern BOOL fgHDMISupportAudio(void);
extern UINT8 bHDMIGetAudSampleFreq(void);
extern void vHDMIHandleAudFmtChange(void);
extern UINT8 u1HDMIGetAudioSamplingFreq(void);
extern UINT8 u1HdmiGetAdacI2SFmt(void);

extern void vHDMISpdifOutEnable(BOOL fgEnable);
UINT8 bHDMIReadDevH(UINT8 bReg);
void vHDMIWriteDevH(UINT8 bReg, UINT8 bValue);
UINT8 bHDMIReadDevL(UINT8 bReg);
void vHDMIWriteDevL(UINT8 bReg, UINT8 bValue);
#ifdef SUPPORT_HDMI_SWITCH_INIT
void vHDMISwitchInit(void);
#endif
void vHDMIGPIOInit(void);
/* debug usage */
BOOL HDMICRC(INT16 ntry);
BOOL fgDVICRC(INT16 ntry);
void vHDMIBypassVdo(void);
void vHDMIHDCPKey(void);
void vHDMIReloadHDCPKey(void);
void vHDMIHDCPSelfBist(UINT8 mode);
#if 0
void vHDMIvLoadColorMatrix(BYTE idx);
#endif
void vHDMISetHDMIState(UINT8 tmp);
void vHDMIStopFw(void);
void vHDMIResumeFw(void);
void vHDMIDebugMsg(void);
UINT16 HDMIResoWidth(void);
UINT16 HDMIResoHeight(void);
UINT32 HDMIHTotal(void);
BOOL HDMIinterlaced(void);
BOOL fgHDMIHsyncPolarity(void);
BOOL fgHDMIVsyncPolarity(void);
UINT32 HDMILineFreq(void);
UINT8 HDMIRefreshRate(void);
UINT8 bHDMIAVIPixelCount(void);
BOOL HDMIHsyncAct(void);
void  vHDMIGetInfoFrame(UINT8);
UINT32 bHDMIGetHPDAdjust(void);
void vHDMIHPDAdjust(UINT32);
UINT16 wHDMIInfoFrameMask(void);
UINT8 HDMIDeepColorStatus(void);
UINT8 bHDMIMCMHdcpWrite(void);
/*UINT8 _bIsXpcStable(void); */

UINT8 vHDMIEQ(UINT8);

UINT8 bHDMIClocrimetry(void);
#if INFORM_MDCHG
BOOL fgHDMIQueryModeChange(void);
#endif
void vHDMIHPDIndepCtrl(UINT8);
void vHDMITMDSIndepCTRL(UINT8);

extern void HDMIHPDHigh(BOOL);
extern UINT16 HDMIVTotal(void);
/*HDMI switch related */
void vHDMITMDSFloating(void);
void vHDMITMDSPullHigh(void);
UINT8 HDMIScanInfo(void);
UINT8 HDMIAspectRatio(void);
UINT8 HDMIAFD(void);
UINT8 HDMIRgbRange(void);
HDMI_INFOFRAME_DESCRIPTION *API_HDMI_Get_InfoFrame(UINT8);
extern UINT8 _bHDMICurrSwitch;
void bHDMIEQTwoGear(UINT8);
UINT8 HDMIITCFlag(void);
UINT8 HDMIITCContent(void);
void vHDMISetEQRsel(UINT8);
void vHDMIPowerOnOff(UINT8);
UINT8 bHDMIHDCPKeyCheck(void);
UINT8 HDMIHDCPStatusGet(void);



extern UINT8   _bHDMIAudioInit;
extern UINT8   _bHDMISampleChange;
extern HDMI_RX_PACKET_INFO _RxPacket[MAX_PACKET];

extern HDMI_ANA_BAND eBand_pre;
extern HDMI_ANA_BAND eBand;


/*extern UINT8   _bHDMIAudioForm4; */
extern void HDMIUpdateAudParm(void);
void HDMISetColorRalated(void);
void HDMIHpdLoop(void);
void HDMIAudErrorHandler(void);


/*FOr repeater HDCP */
void RxHDCPSetReceiver(void);
void RxHDCPSetRepeater(void);
void vRxHDCPSetTxKsv(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch);
/*for audio interface */
int ExportAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);
void ExportAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus);
/*void vGetHdmiRxAudioParameter(HDMI_RX_IN_AUDIO_INFO_T *prAudIf); */


void ShowHDMIRxAudioChannelStatus(void);
void ShowRxDeepColorStatus(void);
void ShowAVMuteStatus(void);
void ShowAviInforFrame(void);
void ShowAudioInforFrame(void);
void ShowACPInforFrame(void);
void ShowSPDInforFrame(void);
void ShowGamutInforFrame(void);
void ShowGCPInforFrame(void);
BOOL HDMICRC(INT16 ntry);
void ShowAllIntStatus(void);
void ShowRxHDCPBstatus(void);
void ModifyBstatusDepth(UINT8 u1Depth);
void RecoverBstatusDepth(void);



void HDMIRXColorSpaceConveter(void);

void HDMIRxHdcpService(void);
void HdmiRxLoadEdidTable(void);
void HdmiRxLoadHdcpKey(void);
void  SetRxHdcpStatus(RxHDCPStateType bStatus);
BOOL TxAuthDone(void);
void RxHdcpMode(UINT8 u1Mode);
void HdmiRxStatus(void);
void EnableHdmiRxDebug(UINT32 u4MessageType);
void DisableHdmiRxDebug(UINT32 u4MessageType);
void HdmiRxPacketDataInit(void);
void HdmiRxGetPacketData(void);
void HdmiGet3DInfo(BOOL fgPrintLog);

void BdModeChk(void);
void TriggleEdidForceUpdate(void);
void Set640x480PEnable(BYTE bType);

void CheckPwr5vStatus(void);

void SinkSetAttachMode(BOOL bMHLEn);
void SinkSetPPMode(BOOL bPPMode);
void SinkDDCWriteData(UINT8 uDevID, UINT8 uOffset, UINT8 bData);
UINT8 SinkDDCReadData(UINT8 uDevID, UINT8 uOffset);
void SinkTaskSetPathEn(BOOL bMHLen);



/*#define HDMI_RX_DEBUG_HOT_PLUG   0x1 */

BOOL fgIsHdmiRxDebug(UINT32 u4MessageType);

/*UINT8 u1HalGetRxI2sMclk(void); */


UINT8 HDMIInputType(void);
UINT8 HDMI422Input(void);

UINT32 BitCount(UINT8 *pData, UINT8 Len);


/*mtk68528 add */
void HdmiPwr5vMonitor(void);
BOOL HdmiIsPwr5vStable(void);

void HdmiHVtotalMonitor(void);
BOOL HdmiIsHVStable(void);

void HdmiPclkMonitor(void);
BOOL HdmiIsPclkStable(void);

void HdmiHdcpMonitor(BOOL bEn);
BOOL HdmiIsHdcpStable(void);
void HdmiHVActiveMonitor(void);

BOOL HdmiIsActiveStable(void);
void HdmiAviinfoMonitor(void);
BOOL HdmiIsAviinfoStable(void);
BOOL HdmiIsTimingStable(void);

void HdmiMonitorReset(void);

void HDMI_ResetGlobalVideoInfo(int status);
void HDMI_DrvConfigEvent(MHL_DRV_CONFIG_T rMhlConfig);
void HDMI_DrvGetVideoInfo(MHL_VIDEO_INFO_T *pVideoInfo);
void HDMI_DrvSetStart(BOOL fgStart);
BOOL HDMI_DrvGetStart(void);


void Linux_HAL_GetTime(unsigned long *prTime);
BOOL Linux_HAL_GetDeltaTime(unsigned long *u4OverTime, unsigned long *prStartT, unsigned long *prCurrentT);

void HdmiSelectAnaBand(BOOL);
void HdmiResetAnaBand(void);

void MHLChannelAdjust(void);
BOOL IsMhlMode(void);




typedef enum {
	DAC_16_BIT,
	DAC_18_BIT,
	DAC_20_BIT,
	DAC_24_BIT,
}   DAC_DATA_NUMBER_T;


typedef enum {
	MCLK_128FS,
	MCLK_192FS,
	MCLK_256FS,
	MCLK_384FS,
	MCLK_512FS,
	MCLK_768FS,
	MCLK_1024FS
} SAMPLE_FREQUENCY_T;

typedef enum {
	LRCK_CYC_16,
	LRCK_CYC_24,
	LRCK_CYC_32
} LRCK_CYC_T;

enum {
	AVI_INFOFRAME = 0,
	AUDIO_INFOFRAME,
	ACP_PACKET,
	ISRC1_PACKET,
	ISRC2_PACKET,
	GAMUT_PACKET,
	VENDOR_INFOFRAME,
	SPD_INFOFRAME,
	MPEG_INFOFRAME,
	GEN_INFOFRAME,
	MAX_PACKET_E,
};


#endif

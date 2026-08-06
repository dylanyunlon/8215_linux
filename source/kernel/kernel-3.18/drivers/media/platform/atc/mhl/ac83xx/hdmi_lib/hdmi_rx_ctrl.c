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

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>

#include "hdmi_rx_ctrl.h"
#include "mhl_private.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_rtos.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"

#include "x_bim.h"
#include "drv_thread.h"
#include "x_timer.h"
#include "mhl_drv_if.h"
/*#include "drv_hdmi_rx.h" */
/*#include "drv_hdmi.h" */
#include "hdmi_rx_hal.h"


#include "hdmi_rx_dvi.h"
#include "hdmi_rx_aud_if.h"
#include "x_debug.h"
#include "hdmi_rx_aud_task.h"
#include "hdmi_rx_task.h"
#include "drv_km_errcode.h"
#include "edid_data.h"
#include "x_avd.h"
#include "hdmi_hw_reg.h"
#include "hdmi_debug.h"
/* #include "x_gpio.h" */
#include "mhl_rx_cbus_ctrl.h"
#include "mhl_drv.h"
#include "edid_hal.h"
#include "vga_table.h"
#include "winutil.h"
#include <generated/atc_project.h>
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include "x_ioopt.h"
#endif


/* #include "hdmi_rx_hw.h" */


/****************************************************************************
 ** Local define  prototype
 ****************************************************************************/
#define IC_3x3color_test 1
/* #define DEBUG_HDMI_STATE_CHG 0 */
/* #define DEBUG_HDMI_INT 0 */
#define GetTickCount() (1000 * jiffies / HZ)
#define HDMI_DEBUG 1
#define HDMI_PING 0
#define IC_VERIFY 1
#define MT8202_ECN 1
#define HDMI_SELFTESTING 0
#define ECO_0914 1
#define TMDS_EQ_CUSTOM 1 /*  josh */
#define CCIR_HV_Position 0
#define AUD_MUTE_PERIOD                2   /*  2 second */
#define HDMI_Audio_NewFlow  1
/* #define ANA_INTF_1_31_16 Fld(16, 16, AC_FULLW32) //31:16 */
#define PixelInvCutFreq      160 /*  2 second */
#define Enable_HW_Mute      1
/* #define FLD_HDMI_CLK_Crystral Fld(1, 20, AC_MSKB2) //20 */
#define Protect_XCLKInPCLk      1
#define     XCLKInPCLkCNT       5
#define   XCLKInPCLkCNTMid    2 /*  (XCLKInPCLkCNT-1)/2 */
/*  SLT test */
#ifdef __MODEL_slt__
#define __HDMI_SLT__ 1  /*  for SLT testing */
#endif
#define ReHPD    0
#define IS_AUD_MUTE()   _fgAudMute
#define hdmi_audio 0
/*  Configuration */
#define HDMI_AUD_FMT_CHG_PROTECT
/*  Constant define */
#define HDMI_AUD_OK               0
#define HDMI_AUD_NG               1
#define HDMI_AUD_UNSTABLE_LVL_1   1
#define HDMI_AUD_UNSTABLE_LVL_2   2
#define HDMI_AUD_UNSTABLE_LVL_3   3
#define DDC_0 0x80E0        /*   PADCFG3 */
#define DDC_12 0x80EC   /*  PADCFG6 */
#define DDC_32 Fld(32, 0, AC_MSKDW)
#define MT53XX_PREAMBLE_CTI_MASK  (0xf << 8)
#define MT53XX_BYP_SYNC     (0x1 << 17)
#define MT53XX_BYP_DVIFILT  (0x1 << 18)
#define MT53XX_FIFO_RESET   (0x1 << 9)
#define HPD_5V  1
#define Enable_TDFIFO_RESET 1    /*  WT.Chang command for deep color */
#define INTER_AUDCLK 1
#define INTERNAL_HDCP_KEY
#if MUTE_TEMP
/*  PRIVATE UINT8 bHDMIdelayTDMSReset; */
/* #define HDMI_OFFON_MUTE 60 // off to on */
#define HDMI_HPD_ONDELAY 3000
#define HDMI_MUTE_COUNT 20 /*  mode change */
#endif
#define HDMI_XPC_STABLE_CNT 30 /*  unit, per Vsync, xclk in pclk stable count */
#define SAVE_WEAK_IC 0
#define DATA_CNT_MAX 2
#define EDID_NUM_MAX   (256)
#define HDMI_RX_DEBUG_HOT_PLUG   0x1         /* for compile error */
UINT32 _u4DebugRxMessageType = 0; /* HDMI_RX_DEBUG_EDID | HDMI_RX_DEBUG_HOT_PLUG;//| HDMI_RX_DEBUG_INFOFRAME; */
static BYTE _bHdmiRepeaterMode;/* HDMI_SOURCE_MODE; */
BYTE _bAppHdmiRepeaterMode = 0;/* HDMI_SOURCE_MODE; */
BYTE _bHdmiRepeaterModeDelayCount = 15;
UINT8 _bHdmiAudioOutputMode = 0;/* 0: SPEAKER,SPEAKER+HDMI; 1 :HDMI */
/* TX info */
BYTE    _TxDownStreamCount;
UINT16  _TxBStatus;
BYTE    _TxBKsv[5];
BYTE    _TxKsvList[TX_MAX_KSV_COUNT * 5]; /*  5 bytes * 9 Group = 35 bytes */
BOOL    _fgTxHDCPAuthDone = FALSE;
BOOL    _fgTxVMatch = FALSE;
/* RX info */
/* CHAR RxHdcpKey[292]; */
RxHDCPStateType  _RxHDCPState;
#if CONFIG_DRV_HDMI_SUPPORT_HDCP_BDS_X80
RxHDCPSetType  _RxHDCPSetStatus;
#endif
BYTE    _RxDownStreamCount;
BYTE    _RxKsvList[RX_MAX_KSV_COUNT * 5]; /* 5 byte * 8 group = 40 bytes */
UINT16  _RxBstatus;
BYTE    _bHDCPMode = HDCP_RECEIVER;
UINT16  _wRxWaitTxKsvListCount = 0;
UINT8   _bHDMIRxHDCPStatus = 0;
BYTE _HdmiAvMuteShalow = FALSE;
BYTE _RxAKSVShadow[5];
BYTE _RxBKSVShadow[5];
BYTE _RxAnShadow[8];
UINT16 _u2RxRiShadow;
UINT16 _u2RxRiShadowOld;
/* 3D info */
HDMI_3D_INFOFRAME _3DInfo;
HDMI_3D_INFOFRAME _3DInfoOld;
UINT8 _bSCDTTMonitor = 0;
UINT8 _u1TxEdidReadyOld = HDMI_PLUG_OUT;
BOOL _fgUseModifiedDepth = FALSE;
UINT8 _u1ModifiedDepth = 0;
UINT8 _u1Force3DType = 0;
extern unsigned long  g_IO_VBASE_VA;

const CHAR *cRxHdcpStatus[6] = {
	"RxHDCP_UnAuthenticated",
	"RxHDCP_Computations",
	"RxHDCP_WaitforDownstream",
	"RxHDCP_AssembleKSVList",
	"RxHDCP_WaitVReady",
	"RxHDCP_Authenticated",
};

const CHAR *cHDMIState[7] = {
	"HDMI_STATE_NOTREADY",
	"HDMI_STATE_INIT",
	"HDMI_STATE_PWOFF",
	"HDMI_STATE_PWON",
	"HDMI_STATE_PWON2",
	"HDMI_STATE_SCDT",
	"HDMI_STATE_AUTH",
};

const CHAR *cHDMIPacketName[10] = {
	"AVI",
	"AUDIO",
	"ACP",
	"ISRC1",
	"ISRC2",
	"GAMUT",
	"VENDOR",
	"SPD",
	"MPEG",
	"GEN",
};

const CHAR *c3DStructure[10] = {
	"HDMI_3D_Structure_FramePacking",
	"HDMI_3D_Structure_FieldAlternative",
	"HDMI_3D_Structure_LineAlternative",
	"HDMI_3D_Structure_SideBySideFull",
	"HDMI_3D_Structure_LDepth",
	"HDMI_3D_Structure_LDepthGraph",
	"HDMI_3D_Structure_TopBottom",
	"HDMI_3D_Structure_RSV_For_Future_use",
	"HDMI_3D_Structure_SideBySideHalf",
	"HDMI_3D_Structure_Unknow",
};

UINT8 pdInternalRxHdcpKey[292] = {
	0x00 , 0x14 , 0x7c , 0xE6 , 0x2C , 0x37, 0xFF , 0xFF , /* 0X00,KSV,0Xff,0xff */
	0x00 , 0x18 , 0x8b , 0x78 , 0x94, /* 0x00,HDCP KEY */
	0xc8 , 0xef , 0xb3 , 0x00 , 0xd5 , 0x89 , 0xdb , 0x9f , 0xf7 , 0xb8 , 0x51 , 0x7d , 0x41 , 0x07 , 0x0a , 0x5f,
	0x92 , 0xf1 , 0xc3 , 0x46 , 0x20 , 0x92 , 0x7c , 0x2c , 0x2a , 0x4f , 0xd5 , 0xbd , 0x86 , 0x74 , 0xe5 , 0x29,
	0xca , 0x74 , 0xf0 , 0xc5 , 0x9c , 0x68 , 0x42 , 0x0f , 0x8e , 0x7f , 0x89 , 0x4b , 0xa1 , 0xa5 , 0xb1 , 0xe1,
	0xb6 , 0x03 , 0x1e , 0x34 , 0x83 , 0x1a , 0xe8 , 0x51 , 0x0d , 0xbb , 0xe2 , 0x46 , 0x0e , 0x05 , 0x38 , 0xad,
	0x1d , 0xb1 , 0x99 , 0xee , 0x36 , 0x5a , 0x7d , 0x35 , 0xd7 , 0xa4 , 0xc9 , 0xdd , 0xb0 , 0x83 , 0xec , 0x0d,
	0xe1 , 0x45 , 0x62 , 0xed , 0xa4 , 0x16 , 0xea , 0x08 , 0x5b , 0x5b , 0x02 , 0x32 , 0xe2 , 0x4c , 0xfd , 0x73,
	0x76 , 0x68 , 0x4f , 0x9a , 0xb0 , 0x9d , 0x94 , 0x81 , 0xa8 , 0xb8 , 0x67 , 0x00 , 0x24 , 0x0e , 0x79 , 0xb2,
	0x3b , 0x7d , 0x2d , 0x36 , 0x25 , 0x13 , 0x3d , 0x89 , 0x57 , 0xec , 0xef , 0xf0 , 0x53 , 0xa8 , 0x6b , 0xf6,
	0xc8 , 0x47 , 0xc0 , 0xf6 , 0xc6 , 0xa9 , 0x91 , 0x61 , 0xc6 , 0x6a , 0xd0 , 0x4e , 0x88 , 0x01 , 0xcb , 0x18,
	0x5a , 0x7d , 0x3e , 0xcb , 0x66 , 0x3b , 0xd5 , 0x99 , 0xa8 , 0x95 , 0xe7 , 0x4e , 0x5d , 0x8c , 0xc3 , 0x3a,
	0x9a , 0xc3 , 0x8c , 0x72 , 0x59 , 0xa3 , 0xd1 , 0xf1 , 0x02 , 0xd9 , 0x44 , 0x37 , 0x30 , 0x73 , 0x2b , 0x73,
	0x97 , 0x99 , 0x9e , 0xc6 , 0x13 , 0xbc , 0x6d , 0x3a , 0x53 , 0x3c , 0xd1 , 0x81 , 0x45 , 0xfb , 0x00 , 0x2b,
	0x0f , 0x7e , 0xb6 , 0xf5 , 0xc1 , 0x50 , 0x33 , 0xff , 0x4a , 0x04 , 0x22 , 0x0f , 0x53 , 0xa3 , 0x78 , 0xef,
	0xde , 0x93 , 0x2b , 0x54 , 0xc6 , 0xfc , 0xb4 , 0x4b , 0xf0 , 0x3a , 0x35 , 0x83 , 0x9d , 0x69 , 0x29 , 0xcd,
	0x99 , 0x12 , 0x36 , 0xfa , 0xa6 , 0x44 , 0xd9 , 0x92 , 0xd3 , 0x4f , 0x36 , 0xb5 , 0x52 , 0xfa , 0x27 , 0xac,
	0x3a , 0x2f , 0x1e , 0xb8 , 0xe8 , 0x29 , 0x9a , 0x90 , 0x54 , 0x8d , 0xd3 , 0x9e , 0x9e , 0xb4 , 0xa4 , 0x13,
	0x92 , 0x26 , 0x34 , 0xff , 0x97 , 0x07 , 0xe1 , 0xed , 0x6d , 0x3e , 0x60 , 0x53 , 0x94 , 0xc6 , 0x62 , 0x8d,
	0x66 , 0x5e , 0x13 , 0xaa , 0x00 , 0x00, 0x00 /* HDCP KEY 0x00,0x00,0x00 */
};

/****************************************************************************
 ** Local function prototype
 ****************************************************************************/
/*static void HDMIMuteAudio(void);*/
/*static void HDMIUnMuteAudio(void);*/
static void HDMIAudConfig(void);

static UINT32 wHDMIXPCCNT(void);
/*static void HDMIHandleAudFifoFault(void);*/
static UINT8 HDMIAUDIOSampleRateCal(void);
static UINT32 HDMIPixelFreq(void);
static void HDMIVideoHdmiSetting(void);
/*static void HDMIHDCPRst(void);
static void LogHdmiStateChange(UINT8 u1HdmiState);*/
static void HDMITMDSCTRL(BOOL bOnOff);
/****************************************************************************
 ** Local variable prototype
 ****************************************************************************/
UINT8   _bHDMIAudioInit;
UINT8   _bAudHdmiFlg;
UINT8   _bHDMISampleChange;
/* UINT8   _bSmpFrq = AUD_FS_44K; */
static UINT8 _bHdmiAudFreq;
BOOL _fgVideoOn = FALSE;
BOOL _fgAudOutMute = FALSE;
/* PRIVATE BOOL _fgVideoStableAudioUnstable = FALSE; */
/* Audio format configuration */
RX_AUD_AIN_CFG_T _rAudCfg;
UINT32 u4GetHdmiRxAudioI2SFmt(void)
{
	return _rAudCfg.eFormat;
}
EXPORT_SYMBOL(u4GetHdmiRxAudioI2SFmt);


unsigned int _rHdmiPlugWaitTime;
unsigned int _rHdmiLowPlugWaitTime;
unsigned int _rHdmiUnplugTime;


#if HDMI_Audio_NewFlow
UINT8 _bHDMIAudFIFOflag;
#endif



/* gloable aviablue --------------------------------MTK68528---------- */
UINT8   _bHDMIState;      /*  hdmi state */

UINT32 _u4HpdLowCount;    /* HPD set low count */
UINT32 _u4SyncLostCount;  /* Sync,DE lost count */

/*static UINT32 u4PreXpcCnt;*/
/*static UINT32 u4CurXpcCnt;*/
UINT32 g_u4XpcStableCnt; /*  xclk stable count */



BOOL fgNotifySignal = FALSE;
UINT32 u4StableCount = 0;




static BOOL _fgAudMute = FALSE;

UINT8   _bHdmiFlag;
UINT8   _bHdmiCmd;
UINT8   _bHdmiCnt;
UINT16  _bCKDTcnt;

/*  HDMI Status */
UINT8   _bHdmiAudFs;
UINT8   _bHdmiMode; /*  0 - DVI, 1 - HDMI */
BOOL    _bMHLModeBackup = 0;
BOOL    _bMHLMode = 0;
BOOL    _bPPModeBackup = 0;
BOOL    _bPPMode = 0;
UINT8   _uStableCount = 0;
UINT8   _bHdmiMD;
UINT8   _bHDMICurrSwitch = HDMI_SWITCH_INIT;
UINT8   _bAppHDMICurrSwitch = HDMI_SWITCH_INIT;


UINT8   _bInternalEdid = 0; /*  0 - EDID, 1 - Internal EDID */


#if 0 /* INFORM_MDCHG */
UINT8 bHDMIMCCnt; /*  counter for mode change */
#endif

#if SAVE_WEAK_IC
UINT8 bHDMIBadSync;
#endif

UINT8 _bAVIInfo_tmp;
UINT8   _bHDMIScanInfo;
UINT8 _bHDMIAspectRatio;
UINT8 _bHDMIAFD;
UINT8 _bHDMIHDCPStatus;
UINT8 _bHDMI422Input;
UINT8 _bHDMIITCFlag;
UINT8 _bHDMIITCContent;
UINT8 _bIntr_CK_CHG;

UINT8 _bNEW_AVI_Info;
UINT8 _bACPCount;
/* UINT8 _bHPD_Indep_Ctrl;   //  1 is 5v detect , 0 is CKDT detect */
UINT32 _wHDMI_OFFON_MUTE_COUNT;
UINT32 _wDVI_WAIT_STABLE_COUNT;
UINT32 _wHDMIBypassFlag;
UINT32 _wDVI_WAIT_NOSIGNAL_COUNT;
UINT32 _wHDMI_WAIT_SCDT_STABLE_COUNT;
#if (TMDS_EQ_CUSTOM == 1)
UINT32 _wHDMI_EQ_ZERO_VALUE;
UINT32 _wHDMI_EQ_BOOST_VALUE;
/* #if defined(CC_MT5387) || defined(CC_MT5363) */
UINT32 _wHDMI_EQ_SEL_VALUE;
UINT32 _wHDMI_EQ_GAIN_VALUE;
UINT32 _wHDMI_LBW_VALUE;
/* #endif */
#endif
UINT32 _wHDMI_HDCP_MASk1;
UINT32 _wHDMI_HDCP_MASk2;

UINT32 _u4AuthDoneCount;
UINT32 _u4StableCount;


UINT32 _bHDP_Value;
UINT8 _bHDMIConnectFlag;


/*  plug event */
UINT8 _bUnplugFlag;
UINT8 _bUnplugCount;
UINT8 _bForceHPDLow;
UINT8 _bHDMIColorSpace;

UINT8   _bEQFlag;







static UINT8 bInitHDCP;


/*****************************************************************************
 * HDMI State Machine Declaration
 *****************************************************************************/


/*static char *_aszHdmiState[] = {
	"HDMI_STATE_NOTREADY",
	"HDMI_STATE_INIT",
	"HDMI_STATE_PWOFF",
	"HDMI_STATE_PWON",
	"HDMI_STATE_PWON2",
	"HDMI_STATE_SCDT",
	"HDMI_STATE_AUTH"
};*/





#if (CONFIG_DRV_LINUX)
void Linux_HAL_GetTime(unsigned long *prTime)
{
	*prTime = jiffies;
}


BOOL Linux_HAL_GetDeltaTime(unsigned long *u4OverTime, unsigned long *prStartT, unsigned long *prCurrentT)
{
	unsigned long u4DeltaTime;

	u4DeltaTime = *prStartT + (*u4OverTime) * HZ / 1000;

	if (time_after(*prCurrentT, u4DeltaTime)) {
		return TRUE;
	}

	return FALSE;

}
#endif


static BOOL g_fgHdmiStart = FALSE;

void HDMI_DrvSetStart(BOOL fgStart)
{
	g_fgHdmiStart = fgStart;
}

BOOL HDMI_DrvGetStart(void)
{
	return g_fgHdmiStart;
}


MHL_DRV_CONFIG_T g_rMhlConfig = {{0}, {0} };
HANDLE g_hEvent_Power = 0;
HANDLE g_hEvent_Timing = 0;

void HDMI_DrvConfigEvent(MHL_DRV_CONFIG_T rMhlConfig)
{
	g_rMhlConfig = rMhlConfig;

	/*g_hEvent_Power = CreateEvent(NULL, FALSE, FALSE, MHL_EVT_DEVICE);

	if (g_hEvent_Power == NULL) {
		HDMI_LOG(HDMI_LOG_INFO, "create evt error 1\r\n");
	}

	g_hEvent_Timing = CreateEvent(NULL, FALSE, FALSE, MHL_EVT_TIMING);

	if (g_hEvent_Timing == NULL) {
		HDMI_LOG(HDMI_LOG_INFO, "create evt error 2\r\n");
	}*/
}

MHL_VIDEO_INFO_T g_rVideoInfo;

void HDMI_DrvGetVideoInfo(MHL_VIDEO_INFO_T *pVideoInfo)
{
	if (NULL != pVideoInfo) {
		memcpy(pVideoInfo, &g_rVideoInfo, sizeof(g_rVideoInfo));
		pVideoInfo->bUVSwap = 1;
#if 0

		/*  UV swap */
		if (is_sink_attached) {
			/*  MHL Mode */
			if (_bPPMode) {
				HDMI_LOG(HDMI_LOG_DEBUG, "MHL PP mode\r\n");
				pVideoInfo->bUVSwap = 1;
			} else {
				HDMI_LOG(HDMI_LOG_DEBUG, "MHL 24 bit mode\r\n");
				pVideoInfo->bUVSwap = 1;
			}
		} else {
			/*  HDMI Mode */
			HDMI_LOG(HDMI_LOG_DEBUG, "HDMI mode\r\n");
			pVideoInfo->bUVSwap = 1;
		}

#endif
	}
}



#define ABS_DIFF(_a, _b)      (((_a) >= (_b))?((_a)-(_b)):((_b)-(_a)))
void HDMI_ResetGlobalVideoInfo(int status)
{
	UINT32 u4Hfp, u4Hbp, u4Hpw = 0;
	UINT32 u4Vfp, u4Vbp, u4Vpw = 0;
	/*UINT32 u4PixelFreq = 0;
	UINT32 u4Htotal = 0;
	UINT32 u4Vtotal = 0;
	UINT32 u4HsyncFreq = 0;
	UINT32 u4VsyncFreq = 0;*/

	if(1 == status) {
	u4Hfp = HDMI_HalGetHFrontPorch();
	u4Hpw = HDMI_HalGetHSyncWidth();
	u4Hbp = HDMI_HalGetHtotalExt() - HDMI_HalGetActiveWidth() - u4Hfp;

	u4Vfp = HDMI_HalGetVFrontPorch();
	u4Vbp = HDMI_HalGetVBackPorch();
	u4Vpw = 0;


	if (HDMI_HalIsHdmiMode()) {
		g_rVideoInfo.bDviMode = FALSE;
	} else {
		g_rVideoInfo.bDviMode = TRUE;
	}

	g_rVideoInfo.u4TimingID = HdmiGetTimingID();
	g_rVideoInfo.bInterlaced = HDMI_HalIsInterlace();
	if(g_rVideoInfo.u4TimingID < 114 && g_rVideoInfo.u4TimingID > 0)
	{
	g_rVideoInfo.u4Width = Get_HDMIMODE_IPH_WID(g_rVideoInfo.u4TimingID);
	g_rVideoInfo.u4Height = Get_HDMIMODE_IPV_LEN(g_rVideoInfo.u4TimingID);
	} else {
		g_rVideoInfo.u4Width = HDMI_HalGetActiveWidth();
		g_rVideoInfo.u4Height = HDMI_HalGetActiveHeight();
	}
	HDMI_LOG(HDMI_LOG_INFO, "HDMI_ResetGlobalVideoInfo g_rVideoInfo.u4Width = %d\r\n",(int)g_rVideoInfo.u4Width);
	HDMI_LOG(HDMI_LOG_INFO, "HDMI_ResetGlobalVideoInfo g_rVideoInfo.u4Height = %d\r\n",(int)g_rVideoInfo.u4Height);

	/*u4PixelFreq = HDMI_HalGetPixelClockExt();
	u4Htotal = Get_HDMIMODE_IHTOTAL(g_rVideoInfo.u4TimingID);
	u4Vtotal = Get_HDMIMODE_IVTOTAL(g_rVideoInfo.u4TimingID);

	u4HsyncFreq = Get_HDMIMODE_IHF(g_rVideoInfo.u4TimingID);
	u4VsyncFreq = Get_HDMIMODE_IVF(g_rVideoInfo.u4TimingID);


	g_rVideoInfo.u4VFreq = u4VsyncFreq;*/

	/* HDMI_LOG(HDMI_LOG_DEBUG, "HDMI width %d  Height %d u4VFreq %d\r\n",
	g_rVideoInfo.u4Width, g_rVideoInfo.u4Height, g_rVideoInfo.u4VFreq); */

#if 1

	if (g_rVideoInfo.bInterlaced && g_rVideoInfo.u4TimingID > 113) {
		if(((g_rVideoInfo.u4Width== 720) && (g_rVideoInfo.u4Height == 480)) ||
			((g_rVideoInfo.u4Width== 720) && (g_rVideoInfo.u4Height == 576)))
		{
			g_rVideoInfo.bInterlaced = FALSE;
			HDMI_LOG(HDMI_LOG_DEBUG, "Interlace hw detect wrong, Reset it with sw mode\t\n");
		} else {
		g_rVideoInfo.u4Height *= 2;
		}
	}

	if (HDMI_HalIsPclk2XRepeat()) {
		g_rVideoInfo.u4Width *= 2;
	}

#endif

	g_rVideoInfo.u4Hfp = u4Hfp;
	g_rVideoInfo.u4Hbp = u4Hbp;
	g_rVideoInfo.u4Hpw = u4Hpw;
	g_rVideoInfo.u4Vfp = u4Vfp;
	g_rVideoInfo.u4Vbp = u4Vbp;
	g_rVideoInfo.u4Vpw = u4Vpw;

	if(g_rVideoInfo.u4TimingID < 114 && g_rVideoInfo.u4TimingID > 0) {
		g_rVideoInfo.u4VFreq = Get_HDMIMODE_IVF(g_rVideoInfo.u4TimingID);
	} else {
		g_rVideoInfo.u4VFreq = HDMI_HalGetVfreqExt();
	}

	if((g_rVideoInfo.u4Width == 720) && (g_rVideoInfo.u4Height == 480))
	{
		g_rVideoInfo.u4VFreq =60;
	} else if((g_rVideoInfo.u4Width == 720) && (g_rVideoInfo.u4Height == 576)) {
		g_rVideoInfo.u4VFreq = 50;
	}

	g_rVideoInfo.bHPol = HDMI_HalGetHsyncPolarity();
	g_rVideoInfo.bVPol = HDMI_HalGetVsyncPolarity();
	g_rVideoInfo.eScanInfo = SCANINFO_NODATA;
	g_rVideoInfo.ePicAspectRatio = PAR_16_9;
	g_rVideoInfo.eActiveRation = AFAR_16_9;
	g_rVideoInfo.eExtenedColorimetry = EC_YCC709;
	g_rVideoInfo.eRgbRange = RQR_FULL_RANGE;
	HDMI_LOG(HDMI_LOG_INFO, "reset info(%d, %d, %d), \r\n", (int)g_rVideoInfo.u4Width,
		(int)g_rVideoInfo.u4Height, (int)g_rVideoInfo.u4VFreq);
	} else {
		g_rVideoInfo.u4Width = 0;
		g_rVideoInfo.u4Height =0;
		g_rVideoInfo.bInterlaced =FALSE;
	}
	
}




/****************************************************************************
 ** Extended Function
 ****************************************************************************/

/* void vHDMIMainLoop(void) */


#define HDMI_HPD_LOW_TIME   (200) /*  200ms */
#define HDMI_WAIT_SYNC_TIME  (150)
#define HDMI_SCDT_ENTER_TIME   (300) /*  3000ms */
#define HDMI_SCDT_RESET_PHY_TIME (5000)/*5000ms*/
UINT32 g_u4HdmiState = HDMI_STATE_NOTREADY;



BOOL fgCrcFail = FALSE;
UINT32 syncCount = 0;
extern BOOL  uDisconInt;

void HDMIHpdLoop(void)
{
	/* UINT32 dReadData = 0; */
	/* UINT32 u4Htotal = 0; */
	/* UINT32 u4Vtotal = 0; */

	static UINT32 u4State_pre = HDMI_STATE_NOTREADY;
	static UINT32 u4State_cur = HDMI_STATE_NOTREADY;
	static UINT32 u4State_next = HDMI_STATE_NOTREADY; /*  first state */

	/* static UINT32 u4HpdTime_Old = 0; */
	/* static UINT32 u4HpdTime_New = 0; */

	/* static UINT32 u4TmpCkdt = 0; */
	static UINT32 u4TimeScdt1;
	static UINT32 u4TimePwron1 = 0;
	static UINT32 u4TimePwron2 = 0;
	static UINT32 u4ReadDevCapCount;
	
	/* HDMI_LOG(HDMI_LOG_DEBUG, "vHDMIHpdLoop... \r\n"); */
	if (bInitHDCP == 0) {
		bInitHDCP++;
		return;
	} else if (bInitHDCP == 1) {
		bInitHDCP++;

		HdmiRxLoadEdidTable();   /* load EDID table 1 to eeprom */
		/* HdmiRxLoadHdcpKey();     // load HDCP key form EEP to SRAM */

		/* HDMIHDCPRst(); */

		return;
	}

	HdmiPwr5vMonitor();   /*  pwr5v monitor */
	HdmiHVtotalMonitor(); /* hv total monitor */
	HdmiHVActiveMonitor();
	HdmiPclkMonitor();    /* pixel clock monitor */
	HdmiHdcpMonitor(FALSE);   /* monitor HDCP */
	HdmiAviinfoMonitor();

	/*  if receive STOP cmd, force monitor count to 0. force goto state HDMI_STATE_PWOFF */
	if (!HDMI_DrvGetStart()) {
		HdmiMonitorReset();
	}

	HDMI_HalSetVideoChannelMap(RX_CH_MAP_RGB); /* set data path, RGB */

	/* HDMIAudErrorHandler(); Audio fifo error handler */

	HdmiSelectAnaBand(FALSE);   /* select analog band */
	MHLChannelAdjust();    /* mhl channel adjust */

	/* LogHdmiStateChange(_bHDMIState); */

	g_u4HdmiState = u4State_cur;

	switch (u4State_cur) {
	case HDMI_STATE_NOTREADY:
	case HDMI_STATE_INIT:

		HDMI_HalReset(); /*  reset */
		HDMI_HalEnableIntr();
		HDMIVideoOutOff(); /* mute video */
		/* HDMIMuteAudio();   // mute audio */
		/* HDMIAudConfig();   // audio config */

		u4State_next = HDMI_STATE_PWOFF;
		break;

	case HDMI_STATE_PWOFF:

		/* HDMI_LOG(HDMI_LOG_INFO, "[HDMI_STATE_PWOFF]... \r\n"); */
		/* HDMIHPDHigh(TRUE); // hpd low */

		if (HdmiIsPwr5vStable()) {
			u4State_next = HDMI_STATE_PWON;
			syncCount = 0;
			HDMI_LOG(HDMI_LOG_INFO, "HDMI Power5v is stable \r\n");
		} else {
			u4State_next = HDMI_STATE_PWOFF;

			if (isHDMIstop == true) {
				HDMI_LOG(HDMI_LOG_INFO, "isHDMIstop == true \r\n");
				HDMIHPDHigh(FALSE);  /*  hpd low */
				msleep(20);
				HDMIHPDHigh(TRUE);  /*  hpd high */
				isHDMIstop = false;
			}
		}

		break;

	case HDMI_STATE_PWON:
		/* HDMI_LOG(HDMI_LOG_INFO, "[HDMI_STATE_PWOn]... \r\n"); */
		/* HDMIEnable(TRUE); */
		HDMI_HalSwReset();/* hdcp audio fifo ACR */
		HDMIHPDHigh(FALSE);  /*  hpd low */
		HDMI_HalSetTmdsFifoRWPointerDiff(); /* Set TMDS FIFO Read/Write Control pionter to be different */
		/* HalRxHdcpReset(); */
		HalEnableHDCPDDCPort();
		HDMITMDSCTRL(TRUE);
		/* HDMIMuteAudio(); */
		u4ReadDevCapCount = 0;
		readDevcapDone = FALSE;

		/*  get hpd time */
		if (u4State_pre == HDMI_STATE_PWOFF) {
			/* u4HpdTime_Old = GetTickCount(); */
		}

		/* u4HpdTime_New = GetTickCount(); */

		/* judge hpd time is large than 100ms */
#if 0

		if (u4HpdTime_New - u4HpdTime_Old > HDMI_HPD_LOW_TIME) {
			u4State_next = HDMI_STATE_PWON2;
			HDMI_LOG(HDMI_LOG_INFO, "Hpd low -> high(0x%x -> 0x%x), delta time = %d ms  \r\n",
				 u4HpdTime_New, u4HpdTime_Old, u4HpdTime_New - u4HpdTime_Old);
		} else {
			u4State_next = HDMI_STATE_PWON;
		}

#else
		msleep(200);
		HDMIHPDHigh(TRUE);
		u4State_next = HDMI_STATE_PWON2;

#endif

		break;

	case HDMI_STATE_PWON2:

		/* add */
		if (u4State_pre <= HDMI_STATE_PWON2) {
			HdmiResetAnaBand();
			HdmiSelectAnaBand(FALSE);   /* select analog band */
			Sleep(10);
			HDMI_HalPhyReset(HDMI_RST_ALL);
		}


		/* Sleep(30); */
		HalRxHdcpReset();

		HDMI_HalDigtailPhyReset();/* reset analog */
		HDMI_HalSwReset(); /*  add */
		HDMI_HalReset();
		HDMI_HalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);
		HDMIHPDHigh(TRUE);  /*  hpd high */

		if (HdmiIsPwr5vStable()) {
			u4State_next = HDMI_STATE_SCDT;
		} else {
			u4State_next = HDMI_STATE_PWOFF;
		}

		break;

	case HDMI_STATE_SCDT:

		/* HDMI_LOG(HDMI_LOG_INFO, "[HDMI_STATE_SCDT]... \r\n"); */
		if (HDMI_HalGetSCDT()) {
			HDMI_HalClearRxPclkChgStatus(); /* Clear Pixel clock change interrupt status bit */
			/*  auth done */
			if (HdmiIsHdcpStable() || HdmiIsTimingStable()) {
				HdmiRxPacketDataInit();
				HalEnableINTR2_CKDT(TRUE); /* CKDT loos detect */
			}
		}else{
			if(is_sink_attached && uDisconInt == TRUE){
				HdmiTimingEventNotify(0);
				uDisconInt = FALSE;
			}
		}

		u4ReadDevCapCount = 0;
		readDevcapDone = FALSE;

#if 0

		if (SinkAttachSource() && (!SinkDiscoveryOk())) {
			HDMI_HalResetHdcp();
			/* RETAILMSG(1,(TEXT("attach,low(%d, %d)"), SinkAttachSource(), SinkCbusStuchLow())); */
			/* u4State_next = HDMI_STATE_SCDT; */
		}

#endif

		/*  SCDT state loop */
		if (!HdmiIsPwr5vStable()) {
			HdmiTimingEventNotify(0); /*  vid loss */
			u4State_next = HDMI_STATE_PWOFF;
		} else {
			/* if(HdmiIsHdcpStable() || HdmiIsHVStable() ) */
			if (HdmiIsTimingStable()) {
				u4State_next = HDMI_STATE_AUTH;

				if (HdmiIsTimingStable()) {
					HDMI_LOG(HDMI_LOG_DEBUG, "HDMI hdcp authdone is stable \r\n");
				}

				/* HDMI_LOG(HDMI_LOG_WARN, "HDMI hvtotal is stable \r\n"); */
				if (HdmiIsHdcpStable()) {
					HDMI_LOG(HDMI_LOG_DEBUG, "HDMI hdcp authdone is stable \r\n");
				}

				/*  HDMI_LOG(HDMI_LOG_WARN, "HDMI hdcp authdone is stable \r\n"); */
				/* HalHdmiRxSetApll(); */
				/* HDMI_Set_I2S(); */

				HdmiTimingEventNotify(3);  /* vid on */
				/* HDMI_LOG(HDMI_LOG_INFO, "HDMI mode change connecting!!! \r\n"); */
			} else {
#if 0
				u4State_next = HDMI_STATE_SCDT;
				syncCount++;

				if (syncCount == HDMI_WAIT_SYNC_TIME) {
					HDMI_LOG(HDMI_LOG_DEBUG, "@ SCDT Long time \r\n");
					HdmiSelectAnaBand(TRUE);
					syncCount = 0;
					/* u4State_next = HDMI_STATE_PWOFF; */
				}

#endif

			}
		}

		if (!is_sink_attached) {
			if (HDMI_HalIsHdmiMode()) {
				if (HDMI_HalReadAviType() == 0x0) {
					if(!HdmiIsPwr5vStable()) {
						HdmiTimingEventNotify(0);
						u4State_next = HDMI_STATE_PWOFF;
						HDMI_LOG(HDMI_LOG_INFO, "connecting to off\r\n");
					} else {
					HDMI_HalDigtailPhyReset();
					/* HdmiTimingEventNotify(2);  */
					/* HDMI_LOG(HDMI_LOG_INFO, "HDMI mode change connecting!!! \r\n"); */
					/* HDMI_LOG(HDMI_LOG_INFO, "HDMI mode, no avi info..force reset1!!! \r\n"); */
					}
				}

			}
		}

		/* judge if reset analog band */
		if (u4State_pre != HDMI_STATE_SCDT) { /*  enter time */
			u4TimeScdt1 = 0;
		}

		if (u4State_next == HDMI_STATE_SCDT) { /*  scdt last time */
			u4TimeScdt1++;

			if (u4TimeScdt1 >= HDMI_SCDT_ENTER_TIME) {
				u4TimeScdt1 = 0; /*  restart timer */

#if 0
				HdmiResetAnaBand();
#else
				u4State_next = HDMI_STATE_PWON2;

#endif
				/* u4State_next = HDMI_STATE_PWON2; */
				HdmiTimingEventNotify(0);
				HDMI_LOG(HDMI_LOG_INFO, "HDMI SCDT >= 1.5s, Reset analog band...\r\n");
			}
		}
		//judge if reset digtail phy
		if(u4State_pre == HDMI_STATE_PWON2) {
			u4TimePwron1 = GetTickCount();
		}
		if(!HdmiIsTimingStable()) {
			u4TimePwron2 = GetTickCount();
			if((u4TimePwron2 - u4TimePwron1) >= HDMI_SCDT_RESET_PHY_TIME) {
				HDMI_HalDigtailPhyReset();
				u4TimePwron1 = GetTickCount();
				u4TimePwron2 = 0;
				HDMI_LOG(HDMI_LOG_INFO, "SCDT time large than 5s, then reset digitail phy!!");
			}
		}
		break;


	case HDMI_STATE_AUTH:
		/* HDMI_LOG(HDMI_LOG_INFO, "[HDMI_STATE_auth]... \r\n"); */
		syncCount = 0;
		HDMIVideoOutOn();
		HDMISetColorRalated();
		HdmiRxGetPacketData();

		HDMIRXColorSpaceConveter();

		/*  solve HD2600 VGA Card */
		if (HDMI_HalCheckIsPclkChanged()) {
			/*Pixel clock change */
			HDMI_HalSetTmdsFifoRWPointerDiff();/* TMDS FIFO READ/WRITE POINTER differ a gap */
			HAL_Delay_us(2); /* vUtDelay2us(1); */
			HDMI_HalSetTmdsFifoRWPointerFreeRun();/* TMDS FIFO READ/WRITE POINTER free-run */
		}

		if (IsMhlMode()) {
			if (u4ReadDevCapCount < 16) {
				sink_read_DeviceCaps(u4ReadDevCapCount);
				u4ReadDevCapCount++;
			}

			if (u4ReadDevCapCount == 16) {
				readDevcapDone = TRUE;
				u4ReadDevCapCount++;
			}
		}

		/*  Check DVI Mode */
		if ((!HDMI_HalIsHdmiMode()) && HDMI_HalIsGcpMuteEnable()) { /* DVI mode but receive GCP MUTE */
			HDMI_HalSwReset();/*SW reset */
		}

		if (HDMI_HalIsHdmiMode()) {
			/*  Check audio underrun & overrun */
			/* HDMIHandleAudFifoFault(); */
		}
		if(!is_sink_attached) {
			if(HDMI_HalIsHdmiMode()) {
				/*check audio underrun & overrun
				HDMIHandleAudFifoFault();
				if HDMI mode , no AVI info, then reset all.*/
				if(HDMI_HalReadAviType() == 0x0) {
					HDMI_HalSwReset();
					HDMI_HalDigtailPhyReset();
					HDMI_LOG(HDMI_LOG_INFO, "HDMI mode, no avi info..force reset2!!!\n");
				}
			}
		}
		if (!(HDMI_HalGetSCDT())) {
			HdmiRxPacketDataInit();
			HalHDMIRxEnableVsyncInt(FALSE);
		} else {
			HalHDMIRxEnableVsyncInt(TRUE);
			HalEnableINTR2_CKDT(TRUE);
		}

		if (!HdmiIsPwr5vStable()) {
			u4State_next = HDMI_STATE_PWOFF;
		} else if (!(HdmiIsHVStable())) {
			HDMI_LOG(HDMI_LOG_INFO, "back to SCDT \r\n");
			u4State_next = HDMI_STATE_SCDT;
		} else {
			u4State_next = HDMI_STATE_AUTH; /*  auth done */
		}

		break;

	default:
		u4State_next = HDMI_STATE_INIT;
		break;
	}


	//if (u4State_next != u4State_cur) {
		/* HDMI_LOG(HDMI_LOG_WARN, "HPD LOOP: change state [ %s --> %s ] \r\n", */
		/*      _aszHdmiState[u4State_cur], _aszHdmiState[u4State_next]); */
	//}

	u4State_pre = u4State_cur;
	u4State_cur = u4State_next;
}

/**
 * HDMI CRC check
 *
 * @NOTE units test: fgHDMICRC(1); under interlaced signal
 * @NOTE units test: fgHDMICRC(100); under interlaced signal
 */
BOOL HDMICRC(INT16 ntry)
{
	BOOL fgResult = 0;

	fgResult = HalHdmiRxCrc(ntry);

	return  fgResult;
}


void ShowAllIntStatus(void)
{
	UINT32 u4Data;

	u4Data = HalReadINTR_STATE0();

	/* Printf("[HDMI RX]INTR_STATE0 = 0x%x\n", u4Data); */

	u4Data = HalReadINTR_STATE1();

	/* Printf("[HDMI RX]INTR_STATE1 = 0x%x\n", u4Data); */


}


/**
 * @brief   HDMI Initial function
 * @param   None
 * @retval  None
 */
void HDMIInterRxInit(void)
{

	_bHDMIState = HDMI_STATE_PWOFF;

	/*  2. Software Initial */
	/* _bHPD_Indep_Ctrl=0; */

	_bHdmiFlag = 0;
	_bHdmiCmd = 0;
	_bHdmiCnt = 0;
	_bHdmiAudFs = 0xf;
	_bHdmiMode = 0;
	_bHDMISampleChange = 0;
	_bHDMIAudioInit = 0;
	_bHdmiMD = 0;
	_bHDMIScanInfo = HDMIScanInfo(); /*  get scaninfo */
	_bHDMIAspectRatio = HDMIAspectRatio(); /* Aspect Ratio 16:9 or 4:3 */
	_bHDMIAFD = HDMIAFD();               /*  Active Portion Aspect Ratio, 16:9 or 4:3 or 14:9 */
	_bHDMIHDCPStatus = HDMIHDCPStatusGet();
	_bHDMI422Input = 0;
	_bHDMIITCFlag = 0;
	_bHDMIITCContent = 0;
	_bIntr_CK_CHG = 0;
	_bNEW_AVI_Info = 0;
	_bACPCount = 0;

	_bUnplugFlag = 0;
	_bUnplugCount = 0;
	_bHDMIColorSpace = 0;

	_bHDMIAudFIFOflag = HDMI_AUD_NG;


	/* g_u4XpcStableCnt = 0; */

	_bHdmiAudFreq = AUD_FS_44K;  /*  44.1 k */


	if (_bHDMIState == HDMI_STATE_NOTREADY) {
		return;
	}


	/* kenny add, here you need to init some value, you can init it by custom */
	_wHDMI_EQ_ZERO_VALUE = HDMI_TMDS_EQ_ZERO_VALUE;
	_wHDMI_EQ_BOOST_VALUE = HDMI_TMDS_EQ_BOOST_VALUE;
	_wHDMI_EQ_SEL_VALUE = HDMI_TMDS_EQ_SEL_VALUE;
	_wHDMI_EQ_GAIN_VALUE = HDMI_TMDS_EQ_GAIN_VALUE;
	_wHDMI_LBW_VALUE  = HDMI_TMDS_HDMI_LBW_VALUE;
	_wHDMI_HDCP_MASk1 = HDMI_HDCP_Mask1;
	_wHDMI_HDCP_MASk2 = HDMI_HDCP_Mask2;
	_wHDMI_OFFON_MUTE_COUNT = HDMI_OFFON_MUTE_COUNT;
	_wDVI_WAIT_STABLE_COUNT = DVI_WAIT_STABLE_COUNT;
	_wHDMIBypassFlag = HDMI_BYPASS_INITIAL_FLOW;
	_wDVI_WAIT_NOSIGNAL_COUNT = DVI_WAIT_NOSIGNAL_COUNT;
	_wHDMI_WAIT_SCDT_STABLE_COUNT = HDMI_WAIT_SCDT_STABLE_COUNT;

	/* THe following Initial Variable Function need to set before vHalRxHwInit() */
	vHalSetEqZeroValueVar(HDMI_TMDS_EQ_ZERO_VALUE);
	vHalSetEqBoostValueVar(HDMI_TMDS_EQ_BOOST_VALUE);
	vHalSetEqSelValueVar(HDMI_TMDS_EQ_SEL_VALUE);
	vHalSetEqGainValueVar(HDMI_TMDS_EQ_GAIN_VALUE);
	vHalSetLBWValueVar(HDMI_TMDS_HDMI_LBW_VALUE);
	vHalSetRxHdcpMask1Var(_wHDMI_HDCP_MASk1);
	vHalSetRxHdcpMask2Var(_wHDMI_HDCP_MASk2);

	/* THe following is Hardware init */
	/* HDMI_HalHwInit(); // call in Function, HDMI_Rx_HwInit */
	HdmiRxPacketDataInit();

	/* HdmiResetAnaBand(); */
	eBand_pre = HDMI_ANA_BAND_NULL;
	eBand = HDMI_ANA_BAND_NULL;
}



/************************************************************************
Function    : void vHDMIVideoOutOff(void)

Description : This function will set video out OFF Flag
Parameter   : None
Return      : None
 ********************************************************************/
void HDMIVideoOutOff(void)
{
	if (_bHDMIState == HDMI_STATE_NOTREADY) {
		return;
	}

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Block Video\n");
	}

	_fgVideoOn = FALSE;
}

/************************************************************************
Function    : void vHDMIVideoOutOff(void)

Description : This function will set video out On Flag
Parameter   : None
Return      : None:
 ********************************************************************/
void HDMIVideoOutOn(void)
{
	if (_bHDMIState == HDMI_STATE_NOTREADY) {
		return;
	}

	_fgVideoOn = TRUE;
}

/*****************************************************************/

/************************************************************************
Function    : void vHDMIHPDHigh(UINT8 u1HDMICurrSwitch)

Description : This function will control HPD High for each switch port
Parameter   : u1Switch: eHDMI_SWITCH_NO
Return      : TRUE:
 ********************************************************************/
void HDMIHPDHigh(BOOL fgHigh)
{
	HDMI_HalSetHpd(fgHigh);
}

void HDMIAudioOutOn(void)
{
	/* UINT8 bI2CWriteData; */

	if (_bHDMIState == HDMI_STATE_NOTREADY) {
		return;
	}

	/*  Open APLL */
	HDMI_HalOpenApll();
	vSetHdmiFlg(HDMI_AUDIO_ON);
}

UINT8 HDMIDeepColorStatus(void)
{
	return HDMI_HalGetDeepColorBpp();
}

void HDMISetColorRalated(void)
{
	if (HDMI_HalIsHdmiMode()) {
		/* Video Setting */
		HDMIVideoHdmiSetting();

		/*  2x pixel clock setting; */
		/*  0-norepeat;  1~10,n repeat. 11~15, reserved */
		/*if ((HDMI_HalReadAviByte5() & 0x0F) != 0) {
			HDMI_LOG(HDMI_LOG_DEBUG, "vHDMISetColorRalated: 2xPclk \r\n");
			HDMI_HalSetRxPclk2XRepeat(FALSE);
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "vHDMISetColorRalated: 1xPclk \r\n");
			HDMI_HalSetRxPclk2XRepeat(FALSE);
		}*/
		HDMI_HalSetRxPclk2XRepeat(FALSE);
	} else {
		/* DVI mode */
		HDMI_HalClearVideoModeByte0();
		HDMI_HalSetRxPclk2XRepeat(FALSE);
		HDMI_HalClearVideoModeByte1();
		/*HDMI_HalSetRxRGBBlankValue(0x10, 0x10, 0x10);*/
	}
	HDMI_HalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);
}



void HDMIHandleAudFmtChange(void)
{
	UINT8 u1Fs;
	/*  Audio sampling rate change */

	u1Fs = HDMIAUDIOSampleRateCal();

	/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] vHDMIHandleAudFmtChange : FS= %x\n",u1Fs); */

	if (u1Fs == AUD_FS_192K) {
		/* AUD_AoutDacFs(AUD_DEC_MAIN, FS_192K); // FIXME ! Support dual decoder later */
		_rAudCfg.eSampleFreq =  MCLK_128FS;
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 192K\n"); */
	} else if (u1Fs == AUD_FS_176K) {
		/*   AUD_AoutDacFs(AUD_DEC_MAIN, FS_176K); // FIXME ! Support dual decoder later */
		_rAudCfg.eSampleFreq =  MCLK_128FS;
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 176K\n"); */
	} else if (u1Fs == AUD_FS_48K) {
		/*    AUD_AoutDacFs(AUD_DEC_MAIN, FS_48K); // FIXME ! Support dual decoder later */
		/*     _rAudCfg.eSampleFreq =  MCLK_512FS; */
#if CONFIG_DRV_CUSTOM_0
		_rAudCfg.eSampleFreq =  MCLK_512FS;
#else
		_rAudCfg.eSampleFreq =  MCLK_256FS;
#endif
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 48K\n"); */
	} else if (u1Fs == AUD_FS_96K) {
		/*    AUD_AoutDacFs(AUD_DEC_MAIN, FS_96K); // FIXME ! Support dual decoder later */
		_rAudCfg.eSampleFreq =  MCLK_256FS;
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 96K\n"); */
	} else if (u1Fs == AUD_FS_88K) {
		/*   AUD_AoutDacFs(AUD_DEC_MAIN, FS_88K); // FIXME ! Support dual decoder later */
		_rAudCfg.eSampleFreq =  MCLK_256FS;
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 88K\n"); */
	} else if (u1Fs == AUD_FS_32K) {
		/* AUD_AoutDacFs(AUD_DEC_MAIN, FS_32K); // FIXME ! Support dual decoder later */
		/*                     _rAudCfg.eSampleFreq =  MCLK_512FS; */
#if CONFIG_DRV_CUSTOM_0
		_rAudCfg.eSampleFreq =  MCLK_512FS;
#else
		_rAudCfg.eSampleFreq =  MCLK_256FS;
#endif
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 32K\n"); */
	} else {
		/*  Default set to 44.1kHz */
		/*  AUD_AoutDacFs(AUD_DEC_MAIN, FS_44K); // FIXME ! Support dual decoder later */
		/*                     _rAudCfg.eSampleFreq =  MCLK_512FS; */
#if CONFIG_DRV_CUSTOM_0
		_rAudCfg.eSampleFreq =  MCLK_512FS;
#else
		_rAudCfg.eSampleFreq =  MCLK_256FS;
#endif
		HDMIAudConfig();
		/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD] HDMI audio sampling rate change to 44K\n"); */
	}

}


BOOL HDMIAudFifoFault(void)
{
	if (HalIsINTR4_UNDERRUN() || HalIsINTR4_OVERRUN() || HalIsINTR5_AUD_SAMPLE_F()) {
		return TRUE;
	}

	return FALSE;
}

void HDMIAudErrorHandler(void)
{

#if 0

	if ((_bHdmiMode == 0) || (_bHDMIState != HDMI_STATE_AUTH) || _bDviModeChged) {
		HDMI_WRITE32_MASK(REG_AUDP_STAT, 1 << 25, AUDIO_MUTE);
		HDMI_WRITE32_MASK(REG_AEC_CTRL, 0 << 8, AAC_EN);
		HDMI_WRITE32(REG_INTR_STATE1, INTR5_AUDIO_MUTE);
		_bHDMIAudFIFOflag = HDMI_AUD_NG;
	} else {
		if (HDMIAudFifoFault()) {
			if (HDMI_READ32(REG_INTR_STATE0) & INTR2_NEW_AUD_PKT) {
				HDMI_WRITE32(REG_INTR_STATE0, INTR2_NEW_AUD_PKT);

				_bHDMIAudFIFOflag = HDMI_AUD_NG;
				HDMI_WRITE32_MASK(REG_AUDP_STAT, 1 << 25, AUDIO_MUTE);
				HDMI_WRITE32_MASK(REG_AEC_CTRL, 0 << 8, AAC_EN);
				HDMI_WRITE32_MASK(REG_SRST, 1 << 9, FIFO_RST);
				HAL_Delay_us(6);
				HDMI_WRITE32_MASK(REG_SRST, 0 << 9, FIFO_RST);
				HDMI_WRITE32_MASK(REG_AEC_CTRL, 1 << 8, AAC_EN);
				msleep(1);

				if (HDMI_READ32(REG_INTR_STATE1) & INTR5_AUD_SAMPLE_F) {
					HDMI_WRITE32(REG_INTR_STATE1, INTR5_AUD_SAMPLE_F);
					HDMIHandleAudFmtChange();
				}

				if (!_fgAudOutMute) {
					_fgAudOutMute = TRUE;
				}

				/* Clear interrupt */
#if 0
				vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR4_OVERRUN) | Fld2Msk32(INTR4_UNDERRUN) |
				Fld2Msk32(INTR5_AUD_SAMPLE_F));
				vRegWrite4B(INTR_STATE1, Fld2Msk32(INTR5_AUDIO_MUTE));
#endif
				HAL_Delay_us(20);
			}
		} else {
			if (_fgAudOutMute) {
				_fgAudOutMute = FALSE;
			}

			_bHDMIAudFIFOflag = HDMI_AUD_OK;
			HDMI_WRITE32_MASK(REG_AEC_CTRL, 1 << 8, AAC_EN);
			HDMI_WRITE32_MASK(REG_AUDP_STAT, 0 << 25, AUDIO_MUTE);

			if ((HDMI_READ32(REG_I2S_CTRL) & 0xf8000000) == 0x0) {
				HDMI_WRITE32_MASK(REG_AEC_CTRL, 0 << 8, AAC_EN);
				HDMI_WRITE32_MASK(REG_AEC_CTRL, 1 << 8, AAC_EN);
			}
		}
	}

	if (_bHDMIAudFIFOflag == HDMI_AUD_NG) {
		_bHdmiAudFreq = AUD_FS_44K;
	} else {
		_bHdmiAudFreq = (UINT8)(HDMI_READ32(REG_CHST1) & AUD_SAMPLE_F);
	}

#endif

}

/**
 * @brief   HDMI input is RGB /Ycbcr
 * @param   None
 * @retval  1=RGB ,0 =YCBCR
 */
UINT8 HDMIInputType(void)
{
	/* check color space */

	UINT8 bReadData;

	if (HDMI_HalIsHdmiMode()) {
		if (HDMI_HalChkAviInforFrameExist() == FALSE) {
			return 1;
		}

		_bNEW_AVI_Info = 1;
		bReadData = HDMI_HalReadAviByte1();
		/* vHalClearNewAviIntStatus(); */
		_bAVIInfo_tmp = bReadData;


		if ((bReadData & 0x60) == 0x00) { /*  RGB */
			return 1;
		} else { /* Ycbcr */
			return 0;
		}
	}

	return 1;

}


UINT8 HDMIAVIPixelCount(void)
{

	return (HDMI_HalReadAviByte5()  & 0x0f);
}

UINT16 HDMIResoWidth(void)
{
	UINT32 tmp;

	tmp = HDMI_HalGetActiveWidth();

	if (HDMI_HalIsPclk2XRepeat()) { /*  ICLK x2 */
		tmp <<= 1;
	}

	return tmp;
}

UINT16 HDMIResoHeight(void)
{
	UINT16 tmp;

	tmp = HDMI_HalGetActiveHeight();

	return tmp;
}

UINT32 HDMIHTotal(void)
{
	UINT32 tmp;

	tmp = HDMI_HalGetHTotal();

	if (HDMI_HalIsPclk2XRepeat()) { /*  ICLK */
		tmp <<= 1;
	}

	return tmp;
}

UINT16 HDMIVTotal(void)
{
	UINT16 tmp;

	tmp = HDMI_HalGetVTotal();

	return tmp;
}

BOOL HDMIHsyncAct(void)
{

#if CHECHCKDT

	if (HDMI_HalGetSCDT()) /* modify by ciwu */{
		return TRUE;
	}
#else
	if (((u1RegRead1B(SRST_2) & 0x01) == 0x01) && _fgVideoOn == TRUE) {
		return TRUE;
	}
#endif
	return FALSE;
}

UINT8 HDMIScanInfo(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	return (HDMI_HalReadAviByte1()  & 0x03);
}

UINT8 HDMIAspectRatio(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	return ((HDMI_HalReadAviByte2()  & 0x30) >> 4);
}

UINT8 HDMIAFD(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	return (HDMI_HalReadAviByte2()  & 0xf);
}

UINT8 HDMI422Input(void)
{

	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!HDMI_HalIsHdmiMode())) {
		return 0;
	}

	if ((HDMI_HalReadAviByte1() & 0x60) == 0x20) {
		return 1;
	}

	return 0;
}
UINT8 HDMIITCFlag(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	if ((HDMI_HalReadAviByte3() & 0x80) == 0x80) {
		return 1;
	}

	return 0;
}
UINT8 HDMIITCContent(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	return (HDMI_HalReadAviByte5() & 0x30) >> 4;

}
/*
   full range: 0 ~ 255.
   limited range: 16 ~ 235.
00: Default, depend on video format.
PC timing: full range.
Video timing: limited range.
01: limited range.
10: full range.
11: Reserved.
*/
UINT8 HDMIRgbRange(void)
{
	if ((HDMI_HalChkAviInforFrameExist() == FALSE) || (!_bHdmiMode)) {
		return 0;
	}

	return ((HDMI_HalReadAviByte3() & 0x0c) >> 2);
}

UINT8 HDMIHDCPStatusGet(void)
{
	if (HDMI_HalIsHdcpDecrptOn()) {
		return 1;
	}

	return 0;
}

/**
 * @brief   _bIsXpcStable
 * @param   None
 * @retval  1: stable, 0: unstable.
 */
/*
UINT8 _bIsXpcStable(void)
{
   return ((g_u4XpcStableCnt > HDMI_XPC_STABLE_CNT) ? 1 : 0);
}
*/

BOOL HDMIinterlaced(void)
{
	if (HDMI_HalIsInterlace()) {
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG)) {
			/*  UTIL_Printf( "[HDMI RX]interface signal\n"); */
		}

		return 1;
	}
	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG)) {
		/*  UTIL_Printf( "[HDMI RX]progressive signal\n"); */
	}
	return 0;
}

/**
 * @brief bHDMIRefreshRate
 *
 * return the frame rate.
 * @note range: 49~51 -> 50; 59~61 -> 60
 */
UINT8 HDMIRefreshRate(void)
{
	UINT32 pfreq;
	UINT8 rate;
	UINT32 dwtmp;

	dwtmp = HDMIHTotal() * HDMIVTotal();

	if (dwtmp == 0) { /*  avoid divide by zero */
		return 1;
	}

	pfreq = HDMIPixelFreq();

	/* pfreq = HDMI_HalGetPixelClockExt(); */
	/* rate = (pfreq*1000) / dwtmp; */
	/* RETAILMSG(1,(TEXT("freq(0x%x),dwtmp(0x%x) \r\n"), pfreq, dwtmp)); */

	rate = ((pfreq * 1000) + (dwtmp - 1)) / dwtmp; /* modify by ciwu */

	if ((rate <= 51) && (rate >= 49)) {
		rate = 50;
	} else if ((rate <= 57) && (rate >= 55)) {
		rate = 56;
	} else if ((rate <= 61) && (rate >= 59)) {
		rate = 60;
	} else if ((rate <= 68) && (rate >= 65)) {
		rate = 67;
	} else if ((rate <= 71) && (rate >= 69)) {
		rate = 70;
	} else if ((rate <= 73) && (rate >= 71)) {
		rate = 72;
	} else if ((rate <= 76) && (rate >= 74)) {
		rate = 75;
	} else if ((rate <= 86) && (rate >= 84)) {
		rate = 85;
	}

	return rate;
}


UINT32 HDMILineFreq(void)
{
	UINT32 ret;
	UINT16 wDiv;

	wDiv = HDMIHTotal();

	if (wDiv == 0) {
		return 1;
	}

	/* ret = ((dwHDMIPixelFreq()*10) / (wDiv)); */
	ret = HDMI_HalGetPixelClockExt() / wDiv;

	return ret;/* return freq_line*10/1000 */
}

void ShowAviInforFrame(void)
{
	/*
	if(_RxPacket[AVI_INFOFRAME].fgValid)
	{

		Printf("*****AVI Inforframe START********************\n");
		Printf("    AVI TYPE = 0x%x\n", _RxPacket[AVI_INFOFRAME].PacketData[0]);
		Printf(" AVI Version = 0x%x\n", _RxPacket[AVI_INFOFRAME].PacketData[1]);
		Printf("  AVI Length = 0x%x\n", _RxPacket[AVI_INFOFRAME].PacketData[2]);
		Printf("AVI CheckSum = 0x%x\n", _RxPacket[AVI_INFOFRAME].PacketData[3]);

		****PB0 - PB13****
		Printf("   AVI BYTE0~7 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
			_RxPacket[AVI_INFOFRAME].PacketData[3],_RxPacket[AVI_INFOFRAME].PacketData[4],
			_RxPacket[AVI_INFOFRAME].PacketData[5],
			_RxPacket[AVI_INFOFRAME].PacketData[6],_RxPacket[AVI_INFOFRAME].PacketData[7],
			_RxPacket[AVI_INFOFRAME].PacketData[8],
			_RxPacket[AVI_INFOFRAME].PacketData[9],_RxPacket[AVI_INFOFRAME].PacketData[10]);
		Printf("   AVI BYTE8~13 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
			_RxPacket[AVI_INFOFRAME].PacketData[11],_RxPacket[AVI_INFOFRAME].PacketData[12],
			_RxPacket[AVI_INFOFRAME].PacketData[13],
			_RxPacket[AVI_INFOFRAME].PacketData[14],_RxPacket[AVI_INFOFRAME].PacketData[15],
			_RxPacket[AVI_INFOFRAME].PacketData[16]);

		****_RxPacket[AVI_INFOFRAME].PacketData[4] is the PB1 of the AVI INFO
		UTIL_Printf("S1,S0: 0x%x, %s\n", _RxPacket[AVI_INFOFRAME].PacketData[4]&0x03,
		cAviScanStr[_RxPacket[AVI_INFOFRAME].PacketData[4]&0x03]);
		UTIL_Printf("B1,S0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>2)&0x03,
		cAviBarStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>2)&0x03]);
		UTIL_Printf("A0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>4)&0x01,
		cAviActivePresentStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>4)&0x01]);
		UTIL_Printf("Y1,Y0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[4]>>5)&0x03,
		cAviRgbYcbcrStr[(_RxPacket[AVI_INFOFRAME].PacketData[4]>>5)&0x03]);
		UTIL_Printf("R3~R0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[5])&0x0f,
		cAviActiveStr[(_RxPacket[AVI_INFOFRAME].PacketData[5])&0x0f]);
		UTIL_Printf("M1,M0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[5]>>4)&0x03,
		cAviAspectStr[(_RxPacket[AVI_INFOFRAME].PacketData[5]>>4)&0x03]);
		UTIL_Printf("C1,C0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[5]>>6)&0x03,
		cAviColorimetryStr[(_RxPacket[AVI_INFOFRAME].PacketData[5]>>6)&0x03]);
		UTIL_Printf("SC1,SC0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[6])&0x03,
		cAviScaleStr[(_RxPacket[AVI_INFOFRAME].PacketData[6])&0x03]);
		UTIL_Printf("Q1,Q0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>2)&0x03,
		cAviRGBRangeStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>2)&0x03]);
		if(((_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07)<=1)
			UTIL_Printf("EC2~EC0: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07,
			cAviExtColorimetryStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>4)&0x07]);
		else
			UTIL_Printf("EC2~EC0: resevered\n");

		UTIL_Printf("ITC: 0x%x, %s\n", (_RxPacket[AVI_INFOFRAME].PacketData[6]>>7)&0x01,
		cAviItContentStr[(_RxPacket[AVI_INFOFRAME].PacketData[6]>>7)&0x01]);
	    Printf("*****AVI Inforframe END**********************\n");

	}
	*/
}

void ShowAudioInforFrame(void)
{
	/*
	if(_RxPacket[AUDIO_INFOFRAME].fgValid)
	{
		Printf("*****Audio Inforframe START********************\n");
		Printf("    AUD TYPE = 0x%x\n", _RxPacket[AUDIO_INFOFRAME].PacketData[0]);
		Printf(" AUD Version = 0x%x\n", _RxPacket[AUDIO_INFOFRAME].PacketData[1]);
		Printf("  AUD Length = 0x%x\n", _RxPacket[AUDIO_INFOFRAME].PacketData[2]);
		Printf("AUD CheckSum = 0x%x\n", _RxPacket[AUDIO_INFOFRAME].PacketData[3]);

		*** PB0 - PB10
		Printf("   AUD BYTE0~7 = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[AUDIO_INFOFRAME].PacketData[3],_RxPacket[AUDIO_INFOFRAME].PacketData[4],
				_RxPacket[AUDIO_INFOFRAME].PacketData[5],
				_RxPacket[AUDIO_INFOFRAME].PacketData[6],_RxPacket[AUDIO_INFOFRAME].PacketData[7],
				_RxPacket[AUDIO_INFOFRAME].PacketData[8],
				_RxPacket[AUDIO_INFOFRAME].PacketData[9],_RxPacket[AUDIO_INFOFRAME].PacketData[10]);
		Printf("   AUD BYTE8~10 = 0x%x,0x%x,0x%x\n",
				_RxPacket[AUDIO_INFOFRAME].PacketData[11],_RxPacket[AUDIO_INFOFRAME].PacketData[12],
				_RxPacket[AUDIO_INFOFRAME].PacketData[13]);


		UTIL_Printf("CC2~ CC0: 0x%x, %s\n", (_RxPacket[AUDIO_INFOFRAME].PacketData[4])&0x07,
		cAudChCountStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[4])&0x07]);
		UTIL_Printf("CT3~ CT0: 0x%x, %s\n", (_RxPacket[AUDIO_INFOFRAME].PacketData[4]>>4)&0x0f,
		cAudCodingTypeStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[4]>>4)&0x0f]);
		 UTIL_Printf("SS1, SS0: 0x%x, %s\n", (_RxPacket[AUDIO_INFOFRAME].PacketData[5])&0x03,
		 cAudSampleSizeStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[5])&0x03]);
		UTIL_Printf("SF2~ SF0: 0x%x, %s\n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[5])>>2)&0x07,
		cAudFsStr[((_RxPacket[AUDIO_INFOFRAME].PacketData[5])>>2)&0x07]);
		UTIL_Printf("CA7~ CA0: 0x%x, %s\n", (_RxPacket[AUDIO_INFOFRAME].PacketData[7])&0xff,
		cAudChMapStr[(_RxPacket[AUDIO_INFOFRAME].PacketData[7])&0xff]);
		UTIL_Printf("LSV3~LSV0: %d db\n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>3)&0x0f);
		UTIL_Printf("DM_INH: 0x%x ,\n", ((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>7)&0x01,
		cAudDMINHStr[((_RxPacket[AUDIO_INFOFRAME].PacketData[8])>>7)&0x01]);


		Printf("*****Audio Inforframe END**********************\n");
	}
	*/
}


void ShowACPInforFrame(void)
{

	/*
	if(_RxPacket[ACP_PACKET].fgValid)
	{
		Printf("*****ACP Inforframe START********************\n");
		Printf(" ACP HB0 = 0x%x\n", _RxPacket[ACP_PACKET].PacketData[0]);
		Printf("ACP TYPE = 0x%x\n", _RxPacket[ACP_PACKET].PacketData[1]);
		Printf(" ACP HB2 = 0x%x\n", _RxPacket[ACP_PACKET].PacketData[2]);


	    if(_RxPacket[ACP_PACKET].PacketData[1] ==0)
	    {
			UTIL_Printf("Generic Audio\n");


			Printf("  Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],
					_RxPacket[ACP_PACKET].PacketData[5],
					_RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],
					_RxPacket[ACP_PACKET].PacketData[8],
					_RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
			Printf("  Data Byte (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],
					_RxPacket[ACP_PACKET].PacketData[13],
					_RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],
					_RxPacket[ACP_PACKET].PacketData[17],
					_RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
	    }
	    else if(_RxPacket[ACP_PACKET].PacketData[1] ==1)
	    {
			UTIL_Printf("IEC 60958-Identified Audio\n");
			Printf("  Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],
					_RxPacket[ACP_PACKET].PacketData[5],
					_RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],
					_RxPacket[ACP_PACKET].PacketData[8],
					_RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
			Printf("  Data Byte (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],
					_RxPacket[ACP_PACKET].PacketData[13],
					_RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],
					_RxPacket[ACP_PACKET].PacketData[17],
					_RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
	    }
	    else if(_RxPacket[ACP_PACKET].PacketData[1] ==2)
	    {
			UTIL_Printf("DVD Audio\n");
			Printf("   Data Byte (0~1)= 0x%x,0x%x\n",
				_RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4]);

			UTIL_Printf("DVD-AUdio_TYPE_Dependent Generation = 0x%x\n",
			_RxPacket[ACP_PACKET].PacketData[3]);
			UTIL_Printf("Copy Permission = 0x%x\n", ((_RxPacket[ACP_PACKET].PacketData[4])>>6)&0x03);
			UTIL_Printf("Copy Number = 0x%x\n", ((_RxPacket[ACP_PACKET].PacketData[4])>>3)&0x07);
			UTIL_Printf("Quality = 0x%x\n", ((_RxPacket[ACP_PACKET].PacketData[4])>>1)&0x03);
			UTIL_Printf("Transaction = 0x%x\n", (_RxPacket[ACP_PACKET].PacketData[4])&0x01);

	    }
	    else if(_RxPacket[ACP_PACKET].PacketData[1] ==3)
	    {
			UTIL_Printf("SuperAudio CD\n");

			Printf("  CCI_1 (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[3],_RxPacket[ACP_PACKET].PacketData[4],
					_RxPacket[ACP_PACKET].PacketData[5],
					_RxPacket[ACP_PACKET].PacketData[6],_RxPacket[ACP_PACKET].PacketData[7],
					_RxPacket[ACP_PACKET].PacketData[8],
					_RxPacket[ACP_PACKET].PacketData[9],_RxPacket[ACP_PACKET].PacketData[10]);
			Printf("  CCI_1 (8~15)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
					_RxPacket[ACP_PACKET].PacketData[11],_RxPacket[ACP_PACKET].PacketData[12],
					_RxPacket[ACP_PACKET].PacketData[13],
					_RxPacket[ACP_PACKET].PacketData[14],_RxPacket[ACP_PACKET].PacketData[15],
					_RxPacket[ACP_PACKET].PacketData[17],
					_RxPacket[ACP_PACKET].PacketData[18],_RxPacket[ACP_PACKET].PacketData[19]);
	    }


	    Printf("*****ACP Inforframe END**********************\n");
	}
	*/
}


void ShowSPDInforFrame(void)
{
	/*
	if(_RxPacket[SPD_INFOFRAME].fgValid)
	{
	    Printf("*****SPD Inforframe START********************\n");
	    Printf("    SPD TYPE = 0x%x\n", u1HalReadSPDType());
	    Printf(" SPD Version = 0x%x\n", u1HalReadSPDVersion());
	    Printf("  SPD Length = 0x%x\n", u1HalReadSPDLength());


	    Printf("     Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[SPD_INFOFRAME].PacketData[3],_RxPacket[SPD_INFOFRAME].PacketData[4],
				_RxPacket[SPD_INFOFRAME].PacketData[5],
				_RxPacket[SPD_INFOFRAME].PacketData[6],_RxPacket[SPD_INFOFRAME].PacketData[7],
				_RxPacket[SPD_INFOFRAME].PacketData[8],
				_RxPacket[SPD_INFOFRAME].PacketData[9],_RxPacket[SPD_INFOFRAME].PacketData[10]);
	    Printf("    Data Byte (8~15) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[SPD_INFOFRAME].PacketData[11],_RxPacket[SPD_INFOFRAME].PacketData[12],
				_RxPacket[SPD_INFOFRAME].PacketData[13],
				_RxPacket[SPD_INFOFRAME].PacketData[14],_RxPacket[SPD_INFOFRAME].PacketData[15],
				_RxPacket[SPD_INFOFRAME].PacketData[16],
				_RxPacket[SPD_INFOFRAME].PacketData[17],_RxPacket[SPD_INFOFRAME].PacketData[18]);
	    Printf("    Data Byte (16~23)= 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[SPD_INFOFRAME].PacketData[19],_RxPacket[SPD_INFOFRAME].PacketData[20],
				_RxPacket[SPD_INFOFRAME].PacketData[21],
				_RxPacket[SPD_INFOFRAME].PacketData[22],_RxPacket[SPD_INFOFRAME].PacketData[23],
				_RxPacket[SPD_INFOFRAME].PacketData[24],
				_RxPacket[SPD_INFOFRAME].PacketData[25],_RxPacket[SPD_INFOFRAME].PacketData[26]);
	    Printf("    Data Byte (24~25)= 0x%x,0x%x\n",
				 _RxPacket[SPD_INFOFRAME].PacketData[27],_RxPacket[SPD_INFOFRAME].PacketData[28]);

				UTIL_Printf("Source Device information is %s\n",
				cSPDDeviceStr[_RxPacket[SPD_INFOFRAME].PacketData[28]]);


	    Printf("*****SPD Inforframe END**********************\n");
	}
	*/
}

void ShowGamutInforFrame(void)
{
	/*
	if(_RxPacket[GAMUT_PACKET].fgValid)
	{
	    Printf("*****GAMUT Inforframe START********************\n");
	    Printf("   GAMUT HB0 = 0x%x\n", _RxPacket[GAMUT_PACKET].PacketData[0]);
	    Printf("   GAMUT HB1 = 0x%x\n", _RxPacket[GAMUT_PACKET].PacketData[1]);
	    Printf("   GAMUT HB2 = 0x%x\n", _RxPacket[GAMUT_PACKET].PacketData[2]);

	    Printf("     Data Byte (0~7) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[GAMUT_PACKET].PacketData[3],_RxPacket[GAMUT_PACKET].PacketData[4],
				_RxPacket[GAMUT_PACKET].PacketData[5],
				_RxPacket[GAMUT_PACKET].PacketData[6],_RxPacket[GAMUT_PACKET].PacketData[7],
				_RxPacket[GAMUT_PACKET].PacketData[8],
				_RxPacket[GAMUT_PACKET].PacketData[9],_RxPacket[GAMUT_PACKET].PacketData[10]);
	    Printf("    Data Byte (8~14) = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				_RxPacket[GAMUT_PACKET].PacketData[11],_RxPacket[GAMUT_PACKET].PacketData[12],
				_RxPacket[GAMUT_PACKET].PacketData[13],
				_RxPacket[GAMUT_PACKET].PacketData[14],_RxPacket[GAMUT_PACKET].PacketData[15],
				_RxPacket[GAMUT_PACKET].PacketData[16],
				_RxPacket[GAMUT_PACKET].PacketData[17]);
	    Printf("*****GAMUT Inforframe END**********************\n");
	}
	*/
}


void ShowMPEGInforFrame(void)
{
	/*
	if(_RxPacket[MPEG_INFOFRAME].fgValid)
	{

	    UTIL_Printf("====================MPEG inforFrame Start ====================================\n\r");

	    Printf("    MPEG TYPE = 0x%x\n", _RxPacket[MPEG_INFOFRAME].PacketData[0]);
	    Printf(" MPEG Version = 0x%x\n", _RxPacket[MPEG_INFOFRAME].PacketData[1]);
	    Printf("  MPEG Length = 0x%x\n", _RxPacket[MPEG_INFOFRAME].PacketData[2]);

	    UTIL_Printf("Data Byte (0~7) = 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[MPEG_INFOFRAME].PacketData[3],
					_RxPacket[MPEG_INFOFRAME].PacketData[4],
					_RxPacket[MPEG_INFOFRAME].PacketData[5],
					_RxPacket[MPEG_INFOFRAME].PacketData[6],
					_RxPacket[MPEG_INFOFRAME].PacketData[7],
					_RxPacket[MPEG_INFOFRAME].PacketData[8],
					_RxPacket[MPEG_INFOFRAME].PacketData[8],
					_RxPacket[MPEG_INFOFRAME].PacketData[10]);
	    UTIL_Printf("Data Byte (8~15)= 0x%x 0x%x  0x%x  0x%x
					0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[MPEG_INFOFRAME].PacketData[11],
					_RxPacket[MPEG_INFOFRAME].PacketData[12],
					_RxPacket[MPEG_INFOFRAME].PacketData[13],
					_RxPacket[MPEG_INFOFRAME].PacketData[14],
					_RxPacket[MPEG_INFOFRAME].PacketData[15],
					_RxPacket[MPEG_INFOFRAME].PacketData[16],
					_RxPacket[MPEG_INFOFRAME].PacketData[17],
					_RxPacket[MPEG_INFOFRAME].PacketData[18]);
	    UTIL_Printf("====================MPEG inforFrame End
	    ======================================\n\r");
	}
	*/

}


void ShowISRC1InforFrame(void)
{
	/*
	if(_RxPacket[ISRC1_PACKET].fgValid)
	{

	    UTIL_Printf("====================ISRC1 inforFrame Start ====================================\n\r");


	    Printf(" ISRC1 TYPE = 0x%x\n", _RxPacket[ISRC1_PACKET].PacketData[0]);
	    Printf("  ISRC1 HB1 = 0x%x\n", _RxPacket[ISRC1_PACKET].PacketData[1]);
	    Printf("  ISRC1 HB2 = 0x%x\n", _RxPacket[ISRC1_PACKET].PacketData[2]);

	    UTIL_Printf("Data Byte (0~7) = 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[ISRC1_PACKET].PacketData[3],_RxPacket[ISRC1_PACKET].PacketData[4],
					_RxPacket[ISRC1_PACKET].PacketData[5],
					_RxPacket[ISRC1_PACKET].PacketData[6],_RxPacket[ISRC1_PACKET].PacketData[7],
					_RxPacket[ISRC1_PACKET].PacketData[8],
					_RxPacket[ISRC1_PACKET].PacketData[8],_RxPacket[ISRC1_PACKET].PacketData[10]);
	    UTIL_Printf("Data Byte (8~15)= 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[ISRC1_PACKET].PacketData[11],_RxPacket[ISRC1_PACKET].PacketData[12],
					_RxPacket[ISRC1_PACKET].PacketData[13],
					_RxPacket[ISRC1_PACKET].PacketData[14],_RxPacket[ISRC1_PACKET].PacketData[15],
					_RxPacket[ISRC1_PACKET].PacketData[16],
					_RxPacket[ISRC1_PACKET].PacketData[17],_RxPacket[ISRC1_PACKET].PacketData[18]);

	    UTIL_Printf("====================ISRC1 inforFrame End ======================================\n\r");
	}
	*/

}

void ShowISRC2InforFrame(void)
{
	/*
	if(_RxPacket[ISRC2_PACKET].fgValid)
	{

	    UTIL_Printf("====================ISRC2 inforFrame Start ====================================\n\r");

	    Printf("  ISRC2 HB0 = 0x%x\n", _RxPacket[ISRC2_PACKET].PacketData[0]);
	    Printf("  ISRC2 HB1 = 0x%x\n", _RxPacket[ISRC2_PACKET].PacketData[1]);
	Printf("  ISRC2 HB2 = 0x%x\n", _RxPacket[ISRC2_PACKET].PacketData[2]);

	    UTIL_Printf("Data Byte (0~7)= 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[ISRC2_PACKET].PacketData[3],_RxPacket[ISRC2_PACKET].PacketData[4],
					_RxPacket[ISRC2_PACKET].PacketData[5],
					_RxPacket[ISRC2_PACKET].PacketData[6],_RxPacket[ISRC2_PACKET].PacketData[7],
					_RxPacket[ISRC2_PACKET].PacketData[8],
					_RxPacket[ISRC2_PACKET].PacketData[8],_RxPacket[ISRC2_PACKET].PacketData[10]);
	    UTIL_Printf("Data Byte (8~15)= 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[ISRC2_PACKET].PacketData[11],_RxPacket[ISRC2_PACKET].PacketData[12],
					_RxPacket[ISRC2_PACKET].PacketData[13],
					_RxPacket[ISRC2_PACKET].PacketData[14],_RxPacket[ISRC2_PACKET].PacketData[15],
					_RxPacket[ISRC2_PACKET].PacketData[16],
					_RxPacket[ISRC2_PACKET].PacketData[17],_RxPacket[ISRC2_PACKET].PacketData[18]);

	    UTIL_Printf("====================ISRC2 inforFrame End ======================================\n\r");
	}
	else
	    UTIL_Printf("[HDMI RX]ISRC2 is not Valid\n");

	*/

}

void ShowVENDInforFrame(void)
{

	/*
	if(_RxPacket[VENDOR_INFOFRAME].fgValid)
	{

	    Printf("    VEND TYPE = 0x%x\n", _RxPacket[VENDOR_INFOFRAME].PacketData[0]);
	    Printf(" VEND Version = 0x%x\n", _RxPacket[VENDOR_INFOFRAME].PacketData[1]);
	    Printf("  VEND Length = 0x%x\n", _RxPacket[VENDOR_INFOFRAME].PacketData[2]);

	    UTIL_Printf("====================Vendor Spec inforFrame Start ====================================\n\r");

	    UTIL_Printf("Data Byte (0~6) = 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
					_RxPacket[VENDOR_INFOFRAME].PacketData[3],
					_RxPacket[VENDOR_INFOFRAME].PacketData[4],
					_RxPacket[VENDOR_INFOFRAME].PacketData[5],
					_RxPacket[VENDOR_INFOFRAME].PacketData[6],
					_RxPacket[VENDOR_INFOFRAME].PacketData[7],
					_RxPacket[VENDOR_INFOFRAME].PacketData[8],
					_RxPacket[VENDOR_INFOFRAME].PacketData[8]);

	    UTIL_Printf("====================Vendor Spec inforFrame End ======================================\n\r");
	}
	*/

}

/*  add by mingming fu */

void ShowRepeaterInforFrame(void)
{
	/*
	UTIL_Printf("====================Vendor Specific inforFrame Start ====================================\n\r");

	UTIL_Printf("Data Byte (0~6) = 0x%x 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
	    _RxPacket[VENDOR_INFOFRAME].PacketData[3],_RxPacket[VENDOR_INFOFRAME].PacketData[4],
	    _RxPacket[VENDOR_INFOFRAME].PacketData[5],
	    _RxPacket[VENDOR_INFOFRAME].PacketData[6],_RxPacket[VENDOR_INFOFRAME].PacketData[7],
	    _RxPacket[VENDOR_INFOFRAME].PacketData[8],
	    _RxPacket[VENDOR_INFOFRAME].PacketData[8]);

	UTIL_Printf("====================Vendor Specific inforFrame End ======================================\n\r");
	 */
}


void ShowGCPInforFrame(void)
{
	/* UINT8 u1Tmp = 0; */
	/*
	******if(_RxPacket[AVI_INFOFRAME].fgValid)
	{


	UTIL_Printf("====================General Control Packet Start ====================================\n\r");

	Printf("  General Control Packet HB0 = 0x03\n");
	Printf("  General Control Packet HB1 = 0x00\n");
	Printf("  General Control Packet HB2 = 0x00\n");

	UTIL_Printf("Data Byte (0~6) =\n");
	if(vHalCheckGcpMuteEnable())
	UTIL_Printf("0x01 ");
	else
	UTIL_Printf("0x10 ");

	u1Tmp = bHDMIDeepColorStatus() | (1<<2);
	UTIL_Printf("0x%x  0x00  0x00  0x00  0x00  0x00\n",u1Tmp);

	vShowAVMuteStatus();
	vShowRxDeepColorStatus();

	UTIL_Printf("====================General Control Packet End ======================================\n\r");
	}
	*/

}


/* -------------------------------------------------------------------------

Internal Function
---------------------------------------------------------------------- */






/*static void HDMIMuteAudio(void)
{
	if (!IS_AUD_MUTE()) {
		HDMI_HalMuteAudio();
		_fgAudMute = TRUE;
	}
}*/

/*static void HDMIUnMuteAudio(void)
{
	if (IS_AUD_MUTE()) {
		HDMI_HalUnMuteAudio();
		_fgAudMute = FALSE;
	}
}*/


/**
 * @brief   HDMI Audio Config (audio initial)
 * @param   None
 * @retval  None
 */
HDMI_AV_INFO_T _stAvdAVInfo;





static void HDMIAudConfig(void)
{



	/*  Load audio configuration */
	/* Here, you need to get current Audio Cfg */
	HDMI_AUDIO_I2S_FMT_T e_I2sFmt = _fgHDMIRxBypassMode ? _stAvdAVInfo.e_I2sFmt : HDMI_I2S_24BIT;

	/* if (!AUD_GetAinCfg(AUD_STREAM_FROM_HDMI, &_rAudCfg))//temply mark */
	{
		/*  If audio input not initialed, use the following setting */
		_rAudCfg.eStrSrc =      AUD_STREAM_FROM_HDMI;
		/* I2S
		   _rAudCfg.eFormat =      FORMAT_I2S;
		   _rAudCfg.fgLRInvert =   FALSE;
		   */
		/*  if(fgHDMIAudRxBypassMode()) */
		{
			switch (e_I2sFmt) {
			case HDMI_RJT_24BIT:
				_rAudCfg.eFormat =    FORMAT_RJ;
				_rAudCfg.fgLRInvert =  TRUE;
				_rAudCfg.eBits =        DAC_24_BIT;
				break;

			case HDMI_RJT_16BIT:
				_rAudCfg.eFormat =    FORMAT_RJ;
				_rAudCfg.fgLRInvert =  TRUE;
				_rAudCfg.eBits =        DAC_16_BIT;
				break;

			case HDMI_LJT_24BIT:
				_rAudCfg.eFormat =    FORMAT_LJ;
				_rAudCfg.fgLRInvert =  TRUE;
				_rAudCfg.eBits =        DAC_24_BIT;
				break;

			case HDMI_LJT_16BIT:
				_rAudCfg.eFormat =    FORMAT_LJ;
				_rAudCfg.fgLRInvert =  TRUE;
				_rAudCfg.eBits =        DAC_16_BIT;
				break;

			case HDMI_I2S_16BIT:
				_rAudCfg.eFormat =    FORMAT_I2S;
				_rAudCfg.fgLRInvert =  FALSE;
				_rAudCfg.eBits =        DAC_16_BIT;
				break;

			case HDMI_I2S_24BIT:
			default:
				_rAudCfg.eFormat =    FORMAT_I2S;
				_rAudCfg.fgLRInvert =  FALSE;
				_rAudCfg.eBits =        DAC_24_BIT;
				break;
			}

		}
		/* else
		   {
		   _rAudCfg.eFormat =    FORMAT_LJ;
		   _rAudCfg.fgLRInvert =  TRUE;
		   _rAudCfg.eBits =        DAC_24_BIT;
		   }
		   */
		_rAudCfg.eCycle =       LRCK_CYC_32;
		/*       _rAudCfg.eSampleFreq =  MCLK_256FS; */
		_rAudCfg.fgDataInvert = FALSE;
		/* RJ
		   _rAudCfg.eFormat =      FORMAT_RJ;
		   _rAudCfg.fgLRInvert =   FALSE;
		   */
	}

	/* Here You need to setup I2S output Format, I2s, Rj, LJ, 24bits or 16bits.. */
	HDMI_HalI2sLRInv(_rAudCfg.fgLRInvert);
	HDMI_HalSetAudI2sFormat(_rAudCfg.eFormat, _rAudCfg.eCycle);

	/*  Sample edge is falling */
	/* vHalSetLRCKEdge(LRCK_EDGE_FALL);//temply */

	/*  Setup sampling frequency MCLK */
	HDMI_HalSetI2sMclk(_rAudCfg.eSampleFreq);

	/*  Enable CLK */
	HDMI_HalEnableAudClk();

#ifdef HDMI_AUD_FMT_CHG_PROTECT

	/* Enable auto audio configuration
	=> Auto mute as one of the following condition raised
		a. Audio FIFO Underrun INTR4[0]
		b. Audio FIFO Overrun INTR4[1]
		c. CTS Reused INTR4[2]
		d. Fs Changed INTR5[0]
	Clear condtion by the following operation
	 -> vRegWrite4BMsk(INTR_MASK1, Fld2Msk32(INTR5_AUDIO_MUTE), Fld2Msk32(INTR5_AUDIO_MUTE));
	Setup auto mute condition a+b+d */
	HDMI_HalSetAudMuteCondition();
#if !HDMI_Audio_NewFlow
	/*  Enable auto audio configuration */
	vRegWrite4BMsk(AEC_CTRL, Fld2Msk32(AAC_EN), Fld2Msk32(AAC_EN));
#endif
	/*  Enable AAC to control SD0~3 */
	HDMI_HalEnableAacToSd0123();

#endif

	/*  FIXME */
	HDMIAudioOutOn();
}


/*static void XpcStableCount(void)
{
	if (HDMI_HalGetSCDT() && ((_bHDMIState == HDMI_STATE_SCDT) || (_bHDMIState == HDMI_STATE_AUTH))) {

		u4CurXpcCnt = wHDMIXPCCNT();

		 ***CKDT stable counting ***
		if (RANGE_CHECKING(u4CurXpcCnt, u4PreXpcCnt, 300)) {
			 ***set max count is 255 ***
			if (g_u4XpcStableCnt < 255) {
				g_u4XpcStableCnt++;
			}
		} else {
			g_u4XpcStableCnt = 0;
		}

		u4PreXpcCnt = u4CurXpcCnt;
		 ***HDMI_LOG(HDMI_LOG_DEBUG, "u4CurXpcCnt = %d \r\n", u4CurXpcCnt); ***
	}
}*/




/* HDMI state monitor, clk/ckdt/scdt/hvtotal/modechange */

/*----------------------5V power monitor-----------------------------------------------*/
#define PWR5V_STABLE_COUNT  (3)

UINT32 g_u4Pwr5vStableCnt;

void HdmiPwr5vMonitor(void)
{
	static BOOL fgPwr5v_pre = FALSE;
	static BOOL fgPwr5v_cur = FALSE;

	if (!HDMI_HalGetPwr5V()) {
		fgPwr5v_cur = FALSE;
		g_u4Pwr5vStableCnt = 0;
		return;
	}
	fgPwr5v_cur = TRUE;

	if (g_u4Pwr5vStableCnt < 255) {
		g_u4Pwr5vStableCnt++;
	}
	fgPwr5v_pre = fgPwr5v_cur;

}

BOOL HdmiIsPwr5vStable(void)
{
	if (g_u4Pwr5vStableCnt >= PWR5V_STABLE_COUNT) {
		return TRUE;
	}

	return FALSE;
}



/*----------------------hvtotal monitor-----------------------------------------------*/
#define HTOTAL_VALUE_MAX  (4096)
#define HTOTAL_VALUE_MIN  (400)
#define VTOTAL_VALUE_MAX  (2048)
#define VTOTAL_VALUE_MIN  (200)
#define HTOTAL_MAX_ERROR  (10)
#define VTOTAL_MAX_ERROR  (10)
#define HVTOTAL_STABLE_COUNT  (15)
#define HVACTIVE_STABLE_COUNT (10)

UINT32 g_u4HVtotalStableCnt;
UINT32 g_u4HVActiveStableCnt = 0;

void HdmiHVtotalMonitor(void)
{
	static UINT32 u4PreHtotal;
	static UINT32 u4CurHtotal;

	static UINT32 u4PreVtotal;
	static UINT32 u4CurVtotal;
	/* RETAILMSG(1,(TEXT("HdmiHVtotalMonitor... \r\n"))); */

	if (!HDMI_HalGetSCDT()) {
		u4PreHtotal = u4CurHtotal = 0;
		u4PreVtotal = u4CurVtotal = 0;
		g_u4HVtotalStableCnt = 0;
		return;
	}

	u4CurHtotal = HDMI_HalGetHTotal();
	u4CurVtotal = HDMI_HalGetVTotal();

	if (RANGE_CHECKING(u4CurHtotal, u4PreHtotal, HTOTAL_MAX_ERROR) &&
	    RANGE_CHECKING(u4CurVtotal, u4PreVtotal, VTOTAL_MAX_ERROR) &&
	    (u4CurHtotal <= HTOTAL_VALUE_MAX) &&
	    (u4CurHtotal >= HTOTAL_VALUE_MIN) &&
	    (u4CurVtotal <= VTOTAL_VALUE_MAX) &&
	    (u4CurVtotal >= VTOTAL_VALUE_MIN)
	   ) {
		if (HDMI_HalGetSCDT() == 1) {
			if (g_u4HVtotalStableCnt < 255) {
				g_u4HVtotalStableCnt++;
			}
		}
	} else {
		if (HDMI_DrvGetStart()) {
			HDMI_LOG(HDMI_LOG_DEBUG, "HdmiHVtotalMonitor:(%d, %d), %d times \r\n",
				(int) u4CurHtotal, (int) u4CurVtotal, (int) g_u4HVtotalStableCnt);
		}
		g_u4HVtotalStableCnt = 0;
	}

	u4PreHtotal = u4CurHtotal;
	u4PreVtotal = u4CurVtotal;

	/* printk("HdmiHVtotalMonitor:(%d, %d), %d times \r\n", u4CurHtotal, u4CurVtotal, g_u4HVtotalStableCnt); */
}

BOOL HdmiIsHVStable(void)
{
	if (g_u4HVtotalStableCnt > HVTOTAL_STABLE_COUNT) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------hvActive monitor-----------------------------------------------*/
#define HACTIVE_VALUE_MAX  (1920 + 20)
#define HACTIVE_VALUE_MIN  (400)
#define VACTIVE_VALUE_MAX  (1080 + 20)
#define VACTIVE_VALUE_MIN  (200)
#define HACTIVE_MAX_ERROR  (10)
#define VACTIVE_MAX_ERROR  (10)
#define HVACTIVE_STABLE_COUNT  (10)

UINT32 g_u4HVActiveStableCnt;

void HdmiHVActiveMonitor(void)
{
	static UINT32 u4PreHActive;
	static UINT32 u4CurHActive;

	static UINT32 u4PreVActive;
	static UINT32 u4CurVActive;
	/* RETAILMSG(1,(TEXT("HdmiHVtotalMonitor... \r\n"))); */

	if (!HDMI_HalGetSCDT()) {
		u4PreHActive = u4CurHActive = 0;
		u4PreVActive = u4CurVActive = 0;
		g_u4HVActiveStableCnt = 0;
		return;
	}

	u4CurHActive = HDMI_HalGetActiveWidth();
	u4CurVActive = HDMI_HalGetActiveHeight();

	if (RANGE_CHECKING(u4CurHActive, u4PreHActive, HACTIVE_MAX_ERROR) &&
	    RANGE_CHECKING(u4CurVActive, u4PreVActive, VACTIVE_MAX_ERROR) &&
	    (u4CurHActive <= HACTIVE_VALUE_MAX) &&
	    (u4CurHActive >= HACTIVE_VALUE_MIN) &&
	    (u4CurVActive <= VACTIVE_VALUE_MAX) &&
	    (u4CurVActive >= VACTIVE_VALUE_MIN)
	   ) {
		/*  set max count is 255 */
		if (g_u4HVActiveStableCnt < 255) {
			g_u4HVActiveStableCnt++;
		}
	} else {
		g_u4HVActiveStableCnt = 0;
	}

	u4PreHActive = u4CurHActive;
	u4PreVActive = u4CurVActive;

	/* HDMI_LOG(HDMI_LOG_DEBUG, "HdmiHVActiveMonitor(%d, %d), %d times \r\n",
	u4CurHActive, u4CurVActive, g_u4HVActiveStableCnt); */
}

BOOL HdmiIsActiveStable(void)
{
	if (g_u4HVActiveStableCnt > HVACTIVE_STABLE_COUNT) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*----------------------------AVI infoframe packet monitor---------------------------------------*/

#define AVIINFO_MAX_ERROR     (0)
#define AVIINFO_STABLE_COUNT  (20)
#define XPC_MAX_ERROR     (30 * 100)
#define XPC_STABLE_COUNT  (5)

UINT32 g_u4AviinfoStableCnt;

void HdmiAviinfoMonitor(void)
{
	static UINT32 u4PreAviinfo;
	static UINT32 u4CurAviinfo;

	if (HDMI_HalGetSCDT()) {
		if (HDMI_HalIsHdmiMode()) {
			u4CurAviinfo = HDMI_HalReadAviCheckSum();
		} else {
			u4CurAviinfo = 0;
		}

		/*  check sum */
		if (RANGE_CHECKING(u4CurAviinfo, u4PreAviinfo, XPC_MAX_ERROR)) {
			if (g_u4AviinfoStableCnt < 255) {
				g_u4AviinfoStableCnt++;
			}
		} else {
			g_u4AviinfoStableCnt = 0;
		}

		u4PreAviinfo = u4CurAviinfo;
	} else {
		g_u4AviinfoStableCnt = 0;
	}

	/* HDMI_LOG(HDMI_LOG_DEBUG, "HdmiAviinfoMonitor(%d, %d), %d times \r\n",
	u4CurAviinfo, u4PreAviinfo, g_u4AviinfoStableCnt); */
}

BOOL HdmiIsAviinfoStable(void)
{
	if (g_u4AviinfoStableCnt > XPC_STABLE_COUNT) {
		return TRUE;
	} else {
		return FALSE;
	}
}



BOOL HdmiIsTimingStable(void)
{
	if (HdmiIsHVStable() && HdmiIsActiveStable() && HdmiIsAviinfoStable()) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*----------------------------xclk monitor---------------------------------------*/



UINT32 g_u4XpcStableCnt; /*  xclk stable count */

void HdmiPclkMonitor(void)
{
	static UINT32 u4PreXpc;
	static UINT32 u4CurXpc;

	if (HDMI_HalGetSCDT()) {

		u4CurXpc = wHDMIXPCCNT();

		/*  xpc stable counting */
		if (RANGE_CHECKING(u4CurXpc, u4PreXpc, XPC_MAX_ERROR)) {
			/*  set max count is 255 */
			if (g_u4XpcStableCnt < 255) {
				g_u4XpcStableCnt++;
			}
		} else {
			g_u4XpcStableCnt = 0;
		}

		u4PreXpc = u4CurXpc;
	} else {
		g_u4XpcStableCnt = 0;
	}
}

BOOL HdmiIsPclkStable(void)
{
	if (g_u4XpcStableCnt > XPC_STABLE_COUNT) {
		return TRUE;
	} else {
		return FALSE;
	}
}





/**
 * Return the average of (VID_XPCNT*100).
 * NOTE: CAN NOT CALL wHDMIXPCCNT FREQUENTLY
 */
static UINT32 wHDMIXPCCNT(void)   /*return xclk*100*/
{

#if 0
	UINT32 datacnt[DATA_CNT_MAX];
	UINT32 idx, i, tmp;
#if Protect_XCLKInPCLk
	UINT32 j, k, tmp_clk[5], tmp_clk_buffer;
#endif

	for (i = 0 ; i < DATA_CNT_MAX ; i++) {
		datacnt[i] = 0;
	}

	idx = 0;

	/* TODO */
	for (i = 0; i < 10; i++) {

#if Protect_XCLKInPCLk

		for (j = 0; j < XCLKInPCLkCNT; j++) {
			tmp_clk[j] = u4HalGetRxPixelClock();
			HAL_Delay_us(2);/* vUtDelay2us(1); */
		}

		for (k = XCLKInPCLkCNT; k > 0; k--) {
			for (j = 0; j < k - 1; j++) {
				if (tmp_clk[j] > tmp_clk[j + 1]) {
					tmp_clk_buffer = tmp_clk[j];
					tmp_clk[j] = tmp_clk[j + 1];
					tmp_clk[j + 1] = tmp_clk_buffer;
				}
			}
		}

		tmp = tmp_clk[XCLKInPCLkCNTMid];
#else
		tmp = u4HalGetRxPixelClock();
#endif

		if (datacnt[0] == tmp) {
			continue;
		}

		if (datacnt[1] == tmp) {
			continue;
		}

		if (idx < DATA_CNT_MAX) {
			datacnt[idx] = tmp;
		}

		idx++;

		if (idx == 2) {
			break;
		}
	}

	if (idx == 2) {
		return (datacnt[0] + datacnt[1]) * (100 / 2);
	} else {
		return ((datacnt[0]) * (100));
	}

#else

	return HDMI_HalGetXclkCnt() * 100;  /* return xclk * 100 */

#endif

}




/*----------------------------hdcp auth done monitor---------------------------------------*/
#define HDCP_STABLE_COUNT  (25)

UINT32 g_u4HdcpStableCnt = 0;
static UINT32 g_u4HdcpStatus;

void HdmiHdcpMonitor(BOOL bEn)
{
	UINT32 u4Hdcp_status = 0;
	UINT8 An1, An2, An3, An4, An5, An6, An7, An8 = 0;
	UINT8 Aksv1, Aksv2, Aksv3, Aksv4, Aksv5 = 0;
	UINT8 Bksv1, Bksv2, Bksv3, Bksv4, Bksv5 = 0;
	UINT8 Ri1, Ri2 = 0;

	u4Hdcp_status = (HDMI_READ32(REG_HDCP_STAT) & (HDCP_DECRYPT | HDCP_AUTH)) >> 20;

	/* stable count
	HDMI_HalGetRxHdcpStatus();
	if(HDMI_HalIsHdmiRXAuthDone(UINT8 u1Data)) */

	if (u4Hdcp_status == 0x3) {
		if (g_u4HdcpStableCnt < 255) {
			g_u4HdcpStableCnt++;
		}
	} else {
		g_u4HdcpStableCnt = 0;
	}


	/* printk("hdcp cnt: %d \r\n", g_u4HdcpStableCnt); */
	/* show hdcp state, */
	if (bEn || (u4Hdcp_status != g_u4HdcpStatus)) {
		/* 0-NO hdcp, 01-Authenticating, 11- Auth done */
		if (HDMI_DrvGetStart()) {
			HDMI_LOG(HDMI_LOG_DEBUG, "HDcpStatus: 0x%u -> 0x%u \r\n",
			(unsigned int) g_u4HdcpStatus, (unsigned int)u4Hdcp_status);

			if (u4Hdcp_status == 0) {
				HDMI_LOG(HDMI_LOG_DEBUG, "HDMI hdcp authdone is stable \r\n");
				/* HDMI_LOG(HDMI_LOG_DEBUG,"HDCP_NULL \r\n"); */
			} else if (u4Hdcp_status == 0x01) {
				/* pr_info(KERN_INFO"HDCP_Authenticating \r\n"); */
				HDMI_LOG(HDMI_LOG_DEBUG, "HDCP_Authenticating\r\n");
			} else if (u4Hdcp_status == 0x3) {
				/* pr_info(KERN_INFO"HDCP_AUTH_DONE\r\n"); */
				HDMI_LOG(HDMI_LOG_INFO, "HDCP_AUTH_DONE\r\n");
				/* HdmiTimingEventNotify(1);  //vid on */
			} else {
				;/* HDMI_LOG(HDMI_LOG_INFO,"\r\n"); */
			}
		}
	}


	if (bEn) {
		/* print An */
		An1 = (HDMI_READ32(REG_SHD_AN0) & HDCP_AN1) >> 16;
		An2 = (HDMI_READ32(REG_SHD_AN0) & HDCP_AN2) >> 24;
		An3 = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN3) >> 0;
		An4 = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN4) >> 8;
		An5 = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN5) >> 16;
		An6 = (HDMI_READ32(REG_SHD_AN1) & HDCP_AN6) >> 24;
		An7 = (HDMI_READ32(REG_SHD_BSTATUS) & HDCP_AN7) >> 0;
		An8 = (HDMI_READ32(REG_SHD_BSTATUS) & HDCP_AN8) >> 8;

		/*  HDMI_LOG(HDMI_LOG_INFO, "An: 0x%2x, 0x%2x, 0x%2x, 0x%2x,    0x%2x, 0x%2x, 0x%2x, 0x%2x \r\n",
			An1, An2, An3, An4, An5, An6, An7, An8); */

		/* print AKSV */
		Aksv1 = (HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV1) >> 8;
		Aksv2 = (HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV2) >> 16;
		Aksv3 = (HDMI_READ32(REG_SHD_AKSV) & HDCP_AKSV3) >> 24;
		Aksv4 = (HDMI_READ32(REG_SHD_AN0) & HDCP_AKSV4) >> 0;
		Aksv5 = (HDMI_READ32(REG_SHD_AN0) & HDCP_AKSV5) >> 8;

		/* HDMI_LOG(HDMI_LOG_INFO, "Aksv: 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x \r\n",
			Aksv1, Aksv2, Aksv3, Aksv4, Aksv5); */

		if (BitCount(&Aksv1, 1) + BitCount(&Aksv2, 1) + BitCount(&Aksv3, 1) +
		    BitCount(&Aksv4, 1) + BitCount(&Aksv5, 1) != 20) {
			/* HDMI_LOG(HDMI_LOG_ERROR, "Invalid AKSV !!!!!!\r\n"); */
			HDMI_LOG(HDMI_LOG_ERROR, "Invalid AKSV !!!!!!\r\n");
		} else {
			HDMI_LOG(HDMI_LOG_INFO, "Valid AKSV !\r\n");
			/* HDMI_LOG(HDMI_LOG_ERROR, "Valid AKSV !\r\n"); */
		}


		/* print BKSV */
		Bksv1 = (HDMI_READ32(REG_SHD_BKSV0) & HDCP_BKSV1) >> 16;
		Bksv2 = (HDMI_READ32(REG_SHD_BKSV0) & HDCP_BKSV2) >> 24;
		Bksv3 = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV3) >> 0;
		Bksv4 = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV4) >> 8;
		Bksv5 = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_BKSV5) >> 16;

		/* HDMI_LOG(HDMI_LOG_INFO, "Bksv: 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x \r\n", */
		/*     Bksv1, Bksv2, Bksv3, Bksv4, Bksv5); */

		if (BitCount(&Bksv1, 1) + BitCount(&Bksv2, 1) + BitCount(&Bksv3, 1) +
		    BitCount(&Bksv4, 1) + BitCount(&Bksv5, 1) != 20) {
			/* HDMI_LOG(HDMI_LOG_ERROR, "Invalid BKSV !!!!!!\r\n"); */
		} else {
			/* HDMI_LOG(HDMI_LOG_ERROR, "Valid BKSV !\r\n"); */
		}

		/* print Ri */
		Ri1 = (HDMI_READ32(REG_SHD_AKSV) & HDCP_RI0_7_0) >> 0;
		Ri2 = (HDMI_READ32(REG_SHD_BKSV1) & HDCP_RI0_15_8) >> 24;

		/*  HDMI_LOG(HDMI_LOG_INFO, "Ri: 0x%2x, 0x%2x \r\n", Ri1, Ri2); */

	}

	g_u4HdcpStatus = u4Hdcp_status;
}
/***********************************************************xiaochuan*/

BOOL HdmiIsHdcpStable(void)
{
	if (g_u4HdcpStableCnt > HDCP_STABLE_COUNT) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*  reset monitor count */
void HdmiMonitorReset(void)
{
	g_u4Pwr5vStableCnt = 0;
	g_u4HVtotalStableCnt = 0;
	g_u4HVActiveStableCnt = 0;
	g_u4XpcStableCnt = 0;
	g_u4HdcpStableCnt = 0;
	fgNotifySignal = FALSE;
	u4StableCount = 0;
}





/*static UINT8 u1HdmiStateOld;*/

/*static void LogHdmiStateChange(UINT8 u1HdmiState)
{
	 ***Check if the state is correct ***
	if (u1HdmiState <= HDMI_STATE_AUTH) {
		if (u1HdmiStateOld != u1HdmiState) {
			 ***HDMI_LOG(HDMI_LOG_DEBUG, "change state from %s to %s \r\n",
				_aszHdmiState[u1HdmiStateOld],
				_aszHdmiState[u1HdmiState]
				); ***

			u1HdmiStateOld = u1HdmiState;
		}
	} else {  ***Undefined state ***
		 *** HDMI_LOG(HDMI_LOG_ERROR, "change state error(unknown state) \r\n");
	}
}*/




static UINT32 HDMIPixelFreq(void)
{
	UINT32 pfreq;
	UINT32 div;

	div = wHDMIXPCCNT();  /* div returned is , xclk*100 */

	if (div == 0) {
		return 1;
	}

	pfreq = ((UINT32)(1024 * 27 * 1000) * 100);
	pfreq /= (div);

	return pfreq;  /* return pclk/1000 */
}

static UINT8 HDMIAUDIOSampleRateCal(void)
{
	UINT32 wCTS_HW, wN_HW;
	UINT32 wAudSampleRate;
	UINT32 btmp;
	static UINT8 Ori_audio_FS;

	btmp = 20;
	wCTS_HW = HDMI_HalGetRxHwCTSValue();
	wN_HW = HDMI_HalGetRxHwNValue();
	/* wAudSampleRate= (dwHDMIPixelFreq()*10/128) *(wN_HW/wCTS_HW); */
	wAudSampleRate = (((HDMIPixelFreq() * 1000) / wCTS_HW) * ((wN_HW * 100) / 128)) / 10000;
	/* LOG(6, " [HDMI RX]HW CTS =%d  and N =%d , AudSample rate =%d\n", wCTS_HW,wN_HW,wAudSampleRate); */
#if HDMI_DEBUG
	/* HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HW CTS =%d  and N =%d\n", wCTS_HW,wN_HW); */
#endif

	if (HDMIDeepColorStatus() == RX_30BITS_DEEP_COLOR) {
		wAudSampleRate = (wAudSampleRate * 10) / 8;
	} else if (HDMIDeepColorStatus() == RX_36BITS_DEEP_COLOR) {
		wAudSampleRate = (wAudSampleRate * 12) / 8;
	} else if (HDMIDeepColorStatus() == RX_48BITS_DEEP_COLOR) {
		wAudSampleRate = (wAudSampleRate * 16) / 8;
	}

#if HDMI_DEBUG
	/* HDMI_LOG(HDMI_LOG_DEBUG,"[HDMI RX]AudSample rate =%d\n",wAudSampleRate); */
#endif

	if (wAudSampleRate > (UINT32)(320 - btmp) && wAudSampleRate < (UINT32)(320 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_32K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_32K;
		}

#endif
		return AUD_FS_32K;
	} else if (wAudSampleRate > (441 - btmp) && wAudSampleRate < (441 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_44K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_44K;
		}

#endif
		return AUD_FS_44K;
	} else if (wAudSampleRate > (480 - btmp) && wAudSampleRate < (480 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_48K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_48K;
		}

#endif
		return AUD_FS_48K;
	} else if (wAudSampleRate > (880 - btmp) && wAudSampleRate < (880 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_88K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_88K;
		}

#endif
		return AUD_FS_88K;
	} else if (wAudSampleRate > (960 - btmp) && wAudSampleRate < (960 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_96K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_96K;
		}

#endif
		return AUD_FS_96K;
	} else if (wAudSampleRate > (1760 - btmp) && wAudSampleRate < (1760 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_176K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_176K;
		}

#endif
		return AUD_FS_176K;
	} else if (wAudSampleRate > (1920 - btmp) && wAudSampleRate < (1920 + btmp)) {
#if 1

		if (Ori_audio_FS != AUD_FS_192K) {
			HDMI_HalReInitAudioClock();
			Ori_audio_FS = AUD_FS_192K;

		}

#endif
		return AUD_FS_192K;
	} else {
		return AUD_FS_UNKNOWN;
	}

}


/**
 * @brief   vHDMIHandleAudFifoFault
 * @param   None
 * @retval  None
 */
/*static void HDMIHandleAudFifoFault(void)
{
	UINT32 HW_CTS_Value;
	UINT8 u1Fs;

	HW_CTS_Value = HDMI_HalGetRxHwCTSValue();


	if (_fgVideoOn) {
		u1Fs = HDMIAUDIOSampleRateCal();

		if (u1Fs == AUD_FS_UNKNOWN) {
			HDMIMuteAudio();
		} else {
			if (HDMI_HalIsNotDeepColorMode() && (650000 < HW_CTS_Value || 81000 == HW_CTS_Value)) {
				HDMIMuteAudio();
			} else {
				HDMIUnMuteAudio();
			}
		}
	}
}*/


static void HDMIVideoHdmiSetting(void)
{
	UINT8 bReadData;

	if (HDMI_HalIsNoAvi()) {
		HDMI_HalClearIntrState1Bit0_Bit7();/* Clear Audio releative InT state */
		return;
	}

#if IC_3x3color_test

	HDMI_HalClearVideoModeByte0();
	HDMI_HalClearVideoModeByte1();

	/*  check AVI infoframe packet type code */
	if (HDMI_HalChkAviInforFrameExist() == FALSE) { /* no AVI */

		/* default: RGB
		vRegWrite1B(VID_MODE_0,0x00);
		vRegWrite1B(VID_MODE_1,0x00);
		vRegWrite1B(VID_MODE_2,0x08);// RGB to YCbCr */
		HDMI_HalDisableEncodeSync();
		HDMI_HalRxDisable422UpSample();
		/*HDMI_HalSetRxRGBBlankValue(0x10, 0x10, 0x10);*/
		/* always YCbCr */
		/* vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80	 */
		return;
	}

	/*
	   AVI DBYTE1
	   [6:5] Y1 Y0
	   00 - RGB
	   01 - YCbCr 422
	   10 - YCbCr 444
	   */
	bReadData = HDMI_HalReadAviByte1();

	if ((bReadData & 0x60) == 0x00) { /* RGB */
		/*
		   AVI DBYTE2
		   [7:6] C1 C0
		   00 - No Data
		   01 - ITU601
		   10 - ITU709
		   */
		bReadData = HDMI_HalReadAviByte2();

		/* RGB to YCbCr Space Convert: 1=BT709, 0=BT601)
			vRegWrite1B(VID_MODE_0,((bReadData&0xc0)==0x40) ? 0x00 : 0x01);
			vRegWrite1B(VID_MODE_1,0x00);
			RGB to YCBCr Space Convert
			vRegWrite1B(VID_MODE_2,0x08);  RGB to YCbCr */
		HDMI_HalDisableEncodeSync();
		HDMI_HalRxDisable422UpSample();

		/*HDMI_HalSetRxRGBBlankValue(0x10, 0x10, 0x10);*/
		/*  always YCbCr */
		/* vHalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);//Cb:0x80, Y:0x10, Cr:0x80 */
	} else { /*  YCbCR */
		/* vRegWrite1B(VID_MODE_0,0x00);
		vRegWrite1B(VID_MODE_1,0x00);
		if 4:2:2 then do 4:2:2 to 4:4:4 up sample
		vRegWriteFldAlign(VID_MODE, ((bReadData&0x60)==0x20) ? 0x1: 0x00, ENUPSAMPLE); */
		HDMI_HalRxDisable422UpSample();
		/* vRegWrite1B(VID_MODE_2,((bReadData&0x60)==0x20) ? 0x04 : 0x00); */
		/*HDMI_HalSetRxYCbCrBlankValue(0x80, 0x10, 0x80);*//*Cb:0x80, Y:0x10, Cr:0x80 */

	}

#endif
}

/*static void HDMIHDCPRst(void)
{
	HalRxHdcpReset();
}*/


UINT32 BitCount(UINT8 *pData, UINT8 Len)
{
	UINT32 count = 0;
	UINT8 i = 0;
	UINT8 u1Buf = 0;

	for (i  = 0; i < Len; i++) {
		u1Buf = *(pData + i);

		while (u1Buf) {
			count++;
			u1Buf &= (u1Buf - 1);
		}
	}

	return count;
}




static void HDMITMDSCTRL(BOOL fgOn)
{
	if (!IsMhlMode()) {
		HDMI_HalTmdsOn(fgOn);
	}
}

void HDMIRXColorSpaceConveter(void)
{
	if (HDMIInputType() == 0x0) {
		/* YUV */
		if (!HDMI422Input()) {
			/* YUV444 input */
			HDMI_WRITE32_MASK(REG_SRST, 0x0 << 26, BYPASS_VPROC);
			HDMI_WRITE32_MASK(REG_VID_MODE, 0x0 << 19, ENRGB2YC);
			HDMI_WRITE32_MASK(REG_VID_MODE, 0x1 << 17, ENDOWNSAMPLE);
			HDMI_WRITE32_MASK(REG_AUDIO, 0x0, YUV422_OUT_REPACK);
		} else {
			/* YUV4222 input */
			HDMI_WRITE32_MASK(REG_SRST, 0x0 << 26, BYPASS_VPROC);
			HDMI_WRITE32_MASK(REG_VID_MODE, 0x0 << 19, ENRGB2YC);
			HDMI_WRITE32_MASK(REG_VID_MODE, 0x0, ENDOWNSAMPLE);
			/* HDMI_WRITE32_MASK(REG_VID_MODE, 0x1<<17, ENDOWNSAMPLE); */
			HDMI_WRITE32_MASK(REG_AUDIO, 0x1 << 15, YUV422_OUT_REPACK);
		}

		/* HDMI_WRITE32_MASK(REG_SRST, 0x1<<27, BYPASS_VPROC_ATUO); */
	} else {
		/* RGB */
		HDMI_WRITE32_MASK(REG_SRST, 0x0, BYPASS_VPROC);
		HDMI_WRITE32_MASK(REG_VID_MODE, 0x1 << 19, ENRGB2YC);
		HDMI_WRITE32_MASK(REG_VID_MODE, 0x1 << 17, ENDOWNSAMPLE);
		HDMI_WRITE32_MASK(REG_AUDIO, 0x0, YUV422_OUT_REPACK);

		if (HDMIDeepColorStatus() != RX_NON_DEEP) {
			HDMI_WRITE32_MASK(REG_SRST, 0x0, BYPASS_VPROC_ATUO);
		} else {
			HDMI_WRITE32_MASK(REG_SRST, 0x1 << 27, BYPASS_VPROC_ATUO);
		}
	}

	if (HDMIDeepColorStatus() != RX_NON_DEEP) {
		HDMI_WRITE32_MASK(REG_SRST, 0x0, BYPASS_VPROC_ATUO);
	} else {
		HDMI_WRITE32_MASK(REG_SRST, 0X1 << 27, BYPASS_VPROC_ATUO);
	}
}

/*HDMI RX HDCP FUNCTION */
/************************************************************************
Function    : void RxHDCPSetReceiver(void)

Description : This function will set RX in HDCP Receive Mode only
Parameter   : None
Return      : None
 ********************************************************************/
void RxHDCPSetReceiver(void)
{
	/*vHalSetRepeaterMode(FALSE); */
	_bHDCPMode = HDCP_RECEIVER;


}
/************************************************************************
Function    : void RxHDCPSetRepeater(void)

Description : This function will set RX in HDCP Repeater Mode
Parameter   : None
Return      : None
 ********************************************************************/


void TxSetRxHdcpAuthDone(BOOL fgTxHdcpAuthDone)
{
	_fgTxHDCPAuthDone = fgTxHdcpAuthDone;

}

/************************************************************************
Function    : void vRxHDCPSetTxKsv(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist)

Description : This function will set TX Bstaus, Bksv and down-stream Ksv-List
Parameter   : None
Return      : None
 ********************************************************************/
void RxHDCPSetTxKsv(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch)
{
	BYTE i, j;

	_TxDownStreamCount = bTxDownStream;
	_TxBStatus = u2TxBStatus;
	_fgTxVMatch = fgTxVMatch;

	for (i = 0; i < 5 ; i++) {
		_TxBKsv[i] = *(prbTxBksv + i);
	}

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    UTIL_Printf("[HDMI RX][HDCP] vRxHDCPSetTxKsv\n") ;
	    UTIL_Printf("[HDMI RX][HDCP] _TxDownStreamCount = 0x%x\n",_TxDownStreamCount) ;
	    UTIL_Printf("[HDMI RX][HDCP] _TxBStatus = 0x%x\n",_TxBStatus) ;
	    UTIL_Printf("[HDMI RX][HDCP] _fgTxVMatch = 0x%x\n",_fgTxVMatch) ;
	    UTIL_Printf("[HDMI RX][HDCP] _TxBKsv[1~5] = 0x%x,0x%x,0x%x,0x%x,0x%x\n",
	    _TxBKsv[0],_TxBKsv[1],_TxBKsv[2],_TxBKsv[3],_TxBKsv[4]) ;
	}
	*/

	if (_TxDownStreamCount <= 9) {
		for (j = 0; j < _TxDownStreamCount ; j++)
			for (i = 0; i < 5 ; i++) {
				_TxKsvList[j * 5 + i] =  *(prbTxKsvlist + j * 5 + i);
			}
	} else {
		/*
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
		{
		    UTIL_Printf("[HDMI RX][HDCP] !!!Error!! TX dowstream over 9 = %d\n", _TxDownStreamCount) ;
		}
		*/

	}

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    for(j=0; j < _TxDownStreamCount ; j++)
	if(j < 9)
	{
		**UTIL_Printf("[HDMI RX][HDCP] _TxKsvlist[%x][1~5] = 0x%x,0x%x,0x%x,0x%x,0x%x\n",
		j,_TxKsvList[j*5],_TxKsvList[j*5+1],_TxKsvList[j*5+2],_TxKsvList[j*5+3],_TxKsvList[j*5+4]) ;
	}
	}
	*/

	TxSetRxHdcpAuthDone(TRUE);
}

BOOL TxAuthDone(void)
{
	return _fgTxHDCPAuthDone;
}

BOOL RxIsHdmiMode(void)
{	bool ret;

	ret = HDMI_HalIsHdmiMode();
	return ret;
}

void RxAuthStartInt(void)
{
	SetRxHdcpStatus(RxHDCP_UnAuthenticated);
	HalClearKsvReadyBit();
}


void    RxMergeKSVList(void)
{
	BYTE    i, j;
	BYTE bTxDownStreamCount;

	bTxDownStreamCount = _TxDownStreamCount;

	if (bTxDownStreamCount > 9) {
		bTxDownStreamCount = 9;
	}

	for (i = 0; i < (RX_MAX_KSV_COUNT * 5); i++) {
		_RxKsvList[i] = 0;
	}

	for (j = 0; j < bTxDownStreamCount; j++)
		for (i = 0; i < 5; i++) {
			_RxKsvList[j * 5 + i] = _TxKsvList[j * 5 + i];

		}

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	UTIL_Printf((" [HDMI RX][HDCP]RX Merge TX KSV List() [0]="));
	for(j=0;j<bTxDownStreamCount;j++)
	    for(i=0;i<5;i++)
			UTIL_Printf("0x%x ",_RxKsvList[j*5+i]);

	UTIL_Printf(("\n"));
	}
	*/
}


void RxMergeBKSV(void)
{
	int i;
	BYTE bTxDownStreamCount;

	bTxDownStreamCount = _TxDownStreamCount;

	if (bTxDownStreamCount > 9) {
		bTxDownStreamCount = 9;
	}

	for (i = 0; i < 5; i++) {
		_RxKsvList[bTxDownStreamCount * 5 + i] = _TxBKsv[i];
	}

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    UTIL_Printf("[HDMI RX][HDCP]RX Merge TX BKSV () [0]=");
	    for(i=0;i<5;i++)
			UTIL_Printf("0x%x ",_RxKsvList[bTxDownStreamCount*5+i]);
	    UTIL_Printf(("\n"));
	}
	*/
}

void RxMergeBstatus(void)
{
	BYTE bTxDepth;

	_RxBstatus = 0;

	_RxDownStreamCount = _TxDownStreamCount + 1;
	_RxBstatus |= (_RxDownStreamCount & DEVICE_COUNT);

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    UTIL_Printf(" [HDMI RX][HDCP]_RxDownStreamCount = 0x%x\n",_RxDownStreamCount);
	    UTIL_Printf(" [HDMI RX][HDCP]_TxDownStreamCount = 0x%x\n",_TxDownStreamCount);
	    UTIL_Printf(" [HDMI RX][HDCP]_TxBStatus = 0x%x\n",_TxBStatus);
	}
	*/

	if ((_TxBStatus & MAX_DEVS_EXCEEDED) || (_RxDownStreamCount > RX_MAX_KSV_COUNT)) {
		_RxBstatus |=  MAX_DEVS_EXCEEDED;
	}

	bTxDepth = (_TxBStatus & DEVICE_DEPTH) >> 8;


	if (_fgUseModifiedDepth) {
		_RxBstatus |= ((_u1ModifiedDepth << 8)&DEVICE_DEPTH);
	} else {
		_RxBstatus |= (((bTxDepth + 1) << 8)&DEVICE_DEPTH);
	}


	if (_fgUseModifiedDepth) {
		if (_u1ModifiedDepth >= 7) {
			_RxBstatus |= MAX_CASCADE_EXCEEDED;
		}
	} else {
		if ((_TxBStatus & MAX_CASCADE_EXCEEDED) || (bTxDepth >= 7)) {
			_RxBstatus |= MAX_CASCADE_EXCEEDED;
		}
	}

	if (HDMI_HalIsHdmiMode()) {
		_RxBstatus |= HDMI_MODE;
	}

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    UTIL_Printf(" [HDMI RX][HDCP]_RxBstatus = 0x%x\n",_RxBstatus);
	    UTIL_Printf(" [HDMI RX][HDCP]_fgUseModifiedDepth = 0x%x\n",_fgUseModifiedDepth);
	    UTIL_Printf(" [HDMI RX][HDCP]_u1ModifiedDepth = 0x%x\n",_u1ModifiedDepth);
	}
	*/

}

void HDMIRxGenV(void)
{
	UINT32 u4Addr;
	UINT8 i;

	RxMergeKSVList();
	RxMergeBKSV();
	RxMergeBstatus();
	HalSetSHALength(_RxDownStreamCount * 5);
	HalSetBstatus(_RxBstatus);

	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    UTIL_Printf(" [HDMI RX][HDCP]_RxBstatus = 0x%x\n",_RxBstatus);
	}
	*/
	for (i = 0; i < 5; i++) {
		HalRptStartAddrClr();
		HalClearKsvReadyBit();
		HalSetKsvStop(TRUE);
		HalWriteKsvList(_RxKsvList, _RxDownStreamCount);
		HalSetKsvStop(FALSE);
		u4Addr = HalGetKsvFifoAddr();

		/*
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
		{
		    UTIL_Printf(" [HDMI RX][HDCP]KSV u4Addr = %d, i = %d\n",u4Addr,i);
		}
		*/
		if (u4Addr == _RxDownStreamCount * 5) {
			break;
		}
	}

	HalRptStartAddrClr();
	HalTriggerSHA();
}

#if 0
BYTE pdSNYEdid_ATC[] = {
	0x00, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0x00,   0x36, 0x8B, 0x01, 0x00,  0x01, 0x01, 0x01, 0x01,
	0x01, 0x0F, 0x01, 0x03,  0x80, 0x10, 0x09, 0x78,   0x0A, 0x0D, 0xC9, 0xA0,  0x57, 0x47, 0x98, 0x27,
	0x12, 0x48, 0x4C, 0x20,  0x00, 0x00, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x01, 0x1D,   0x00, 0x72, 0x51, 0xD0,  0x1E, 0x20, 0x6E, 0x28,
	0x55, 0x00, 0xC4, 0x8E,  0x21, 0x00, 0x00, 0x1E,   0x01, 0x1D, 0x80, 0x18,  0x71, 0x1C, 0x16, 0x20,
	0x58, 0x2C, 0x25, 0x00,  0xC4, 0x8E, 0x21, 0x00,   0x00, 0x9E, 0x00, 0x00,  0x00, 0xFC, 0x00, 0x4D,
	0x54, 0x4B, 0x20, 0x4C,  0x43, 0x44, 0x54, 0x56,   0x0A, 0x20, 0x20, 0x20,  0x00, 0x00, 0x00, 0xFD,
	0x00, 0x31, 0x4C, 0x0F,  0x50, 0x0E, 0x00, 0x0A,   0x20, 0x20, 0x20, 0x20,  0x20, 0x20, 0x01, 0x29,
	0x02, 0x03, 0x26, 0x74,  0x4B, 0x04, 0x10, 0x1F,   0x05, 0x13, 0x14, 0x01,  0x82, 0x11, 0x06, 0x15,
	0x26, 0x0D, 0x7F, 0x07,  0x15, 0x7F, 0x02, 0x83,   0x0F, 0x00, 0x00, 0x6A,  0x03, 0x0C, 0x00, 0x10,
	0x00, 0xB8, 0x2D, 0x0F,  0x00, 0x00, 0x01, 0x1D,   0x00, 0xBC, 0x52, 0xD0,  0x1E, 0x20, 0xB8, 0x28,
	0x55, 0x40, 0xC4, 0x8E,  0x21, 0x00, 0x00, 0x1E,   0x01, 0x1D, 0x80, 0xD0,  0x72, 0x1C, 0x16, 0x20,
	0x10, 0x2C, 0x25, 0x80,  0xC4, 0x8E, 0x21, 0x00,   0x00, 0x9E, 0x8C, 0x0A,  0xD0, 0x8A, 0x20, 0xE0,
	0x2D, 0x10, 0x10, 0x3E,  0x96, 0x00, 0x13, 0x8E,   0x21, 0x00, 0x00, 0x18,  0x8C, 0x0A, 0xD0, 0x90,
	0x20, 0x40, 0x31, 0x20,  0x0C, 0x40, 0x55, 0x00,   0x13, 0x8E, 0x21, 0x00,  0x00, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x39
};
#else
BYTE pdSNYEdid_ATC[] = {
	0x00, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0x00,   0x0D, 0x04, 0x30, 0x00,  0x01, 0x00, 0x00, 0x00,
	0x01, 0x16, 0x01, 0x03,  0x80, 0x8B, 0x4E, 0x78,   0x2A, 0x50, 0x1F, 0xA3,  0x59, 0x49, 0x97, 0x24,
	0xBB, 0x4F, 0x53, 0x21,  0x08, 0x00, 0x81, 0x80,   0x81, 0xC0, 0x81, 0x00,  0xD1, 0xC0, 0x61, 0x7C,
	0x81, 0xFC, 0x01, 0x01,  0x01, 0x01, 0x02, 0x3A,   0x80, 0x18, 0x71, 0x38,  0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0x72, 0x10,  0x53, 0x00, 0x00, 0x1E,   0x66, 0x21, 0x50, 0xB0,  0x51, 0x00, 0x1B, 0x30,
	0x40, 0x70, 0x36, 0x00,  0x72, 0x10, 0x53, 0x00,   0x00, 0x1E, 0x0E, 0x1F,  0x00, 0x80, 0x51, 0x00,
	0x1E, 0x30, 0x50, 0x80,  0x37, 0x00, 0x72, 0x10,   0x53, 0x00, 0x00, 0x1C,  0x00, 0x00, 0x00, 0xFC,
	0x00, 0x43, 0x48, 0x46,  0x48, 0x44, 0x0A, 0x20,   0x20, 0x20, 0x20, 0x20,  0x20, 0x20, 0x01, 0xDD,
	0x02, 0x03, 0x29, 0xF2,  0x4E, 0x01, 0x02, 0x03,   0x04, 0x05, 0x07, 0x90,  0x12, 0x13, 0x14, 0x16,
	0x9F, 0x20, 0x22, 0x23,  0x09, 0x17, 0x07, 0x83,   0x01, 0x00, 0x00, 0x6D,  0x03, 0x0C, 0x00, 0x10,
	0x00, 0x98, 0x3C, 0x20,  0xA0, 0x22, 0x03, 0x01,   0x41, 0x16, 0x22, 0x56,  0x9E, 0x51, 0x00, 0x31,
	0x30, 0x3E, 0x90, 0xA5,  0x00, 0x72, 0x10, 0x53,   0x00, 0x00, 0x18, 0x02,  0x3A, 0x80, 0xD0, 0x72,
	0x38, 0x2D, 0x40, 0x10,  0x2C, 0x45, 0x80, 0x72,   0x10, 0x53, 0x00, 0x00,  0x1E, 0x02, 0x3A, 0x80,
	0x18, 0x71, 0x38, 0x2D,  0x40, 0x58, 0x28, 0x45,   0x00, 0x72, 0x10, 0x53,  0x00, 0x00, 0x1E, 0x00,
	0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0xF0
};


BYTE hdmi_edid_ac3_dts[] = {
	0x00, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0x00,   0x36, 0x8B, 0x01, 0x00,  0x01, 0x01, 0x01, 0x01,
	0x0F, 0x18, 0x01, 0x03,  0x80, 0x10, 0x09, 0x78,   0x0A, 0x0D, 0xC9, 0xA0,  0x57, 0x47, 0x98, 0x27,
	0x12, 0x48, 0x4C, 0xBF,  0xEF, 0x00, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x01, 0x1D,   0x00, 0x72, 0x51, 0xD0,  0x1E, 0x20, 0x6E, 0x28,
	0x55, 0x00, 0xC4, 0x8E,  0x21, 0x00, 0x00, 0x1E,   0x01, 0x1D, 0x80, 0x18,  0x71, 0x1C, 0x16, 0x20,
	0x58, 0x2C, 0x25, 0x00,  0xC4, 0x8E, 0x21, 0x00,   0x00, 0x9E, 0x00, 0x00,  0x00, 0xFC, 0x00, 0x4D,
	0x54, 0x4B, 0x20, 0x4C,  0x43, 0x44, 0x54, 0x56,   0x0A, 0x20, 0x20, 0x20,  0x00, 0x00, 0x00, 0xFD,
	0x00, 0x31, 0x4C, 0x0F,  0x50, 0x0E, 0x00, 0x0A,   0x20, 0x20, 0x20, 0x20,  0x20, 0x20, 0x01, 0x84,
	0x02, 0x03, 0x26, 0x74,  0x4B, 0x84, 0x10, 0x1F,   0x05, 0x13, 0x14, 0x01,  0x02, 0x11, 0x06, 0x15,
	0x29, 0x0D, 0x7F, 0x07,  0x15, 0x7F, 0x18, 0x3D,   0x7F, 0x18, 0x83, 0x0F,  0x00, 0x00, 0x67, 0x03,
	0x0C, 0x00, 0x10, 0x00,  0xF8, 0x2D, 0x01, 0x1D,   0x00, 0xBC, 0x52, 0xD0,  0x1E, 0x20, 0xB8, 0x28,
	0x55, 0x40, 0xC4, 0x8E,  0x21, 0x00, 0x00, 0x1E,   0x01, 0x1D, 0x80, 0xD0,  0x72, 0x1C, 0x16, 0x20,
	0x10, 0x2C, 0x25, 0x80,  0xC4, 0x8E, 0x21, 0x00,   0x00, 0x9E, 0x8C, 0x0A,  0xD0, 0x8A, 0x20, 0xE0,
	0x2D, 0x10, 0x10, 0x3E,  0x96, 0x00, 0x13, 0x8E,   0x21, 0x00, 0x00, 0x18,  0x8C, 0x0A, 0xD0, 0x90,
	0x20, 0x40, 0x31, 0x20,  0x0C, 0x40, 0x55, 0x00,   0x13, 0x8E, 0x21, 0x00,  0x00, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x1E
};


#endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
void SelectHDMI_SCL_SDA(void)
{
	bool ret;

	struct pinctrl_state *hdmi_scl;
	hdmi_scl = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_scl_rx");
	if(IS_ERR(hdmi_scl)) {
		HDMI_LOG(HDMI_LOG_ERROR, "hdmi_scl:  pinctrl_lookup_state fail \r\n");
	}
	
	ret = pinctrl_select_state(pinctrl_hdmi, hdmi_scl);
	if(ret) {
		HDMI_LOG(HDMI_LOG_ERROR, "hdmi_scl:  pinctrl_select_state fail \r\n");
	}

	struct pinctrl_state *hdmi_sda;
	hdmi_sda = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_sda_rx");
	if(IS_ERR(hdmi_sda)) {
		HDMI_LOG(HDMI_LOG_ERROR, "hdmi_sda:  pinctrl_lookup_state fail \r\n");
	}
	
	ret = pinctrl_select_state(pinctrl_hdmi, hdmi_sda);
	if(ret) {
		HDMI_LOG(HDMI_LOG_ERROR, "hdmi_sda:  pinctrl_select_state fail \r\n");
	}
}
#endif

void HdmiRxLoadEdidTable(void)
{
#if 0
	int i = 0;
	UINT32 u4Ret = 0;
	UINT8 auEdidTable[EDID_NUM_MAX];

	UINT32 val0 = 0, val1 = 1, val2 = 2, val3 = 3;
	UINT32 val = 0;

	/* load edid from metazone to local */
	MZ_LoadEdidData(auEdidTable, EDID_NUM_MAX);

	/* printf metazone edid */
	/* RETAILMSG(1,(TEXT("Metazone EDID: \r\n"))); */
	for (i = 0; i < EDID_NUM_MAX; i++) {
		/* RETAILMSG(1,(TEXT("%2x "),auEdidTable[i])); */
		if ((i + 1) % 16 == 0) {
			/* RETAILMSG(1,(TEXT(" \r\n"))); */
		}
	}

	/*  load edid from local to sram */
	HDMI_HalLoadEdid2Sram(auEdidTable, EDID_NUM_MAX);
#else
	int i = 0;
	UINT32 val0 = 0, val1 = 1, val2 = 2, val3 = 3;
	UINT32 val = 0;

	#ifdef CONFIG_ATC_PLATFORM_ac823x
	SelectHDMI_SCL_SDA();//selest hdmi scl & sda, if not, then load edia fail and can not get audio packet interrupt 
	#endif

	BASE_WRITE32(0x22a1c, 0x04800000);

	for (i = 0; i < 64; i++) {
		val0 = hdmi_edid_ac3_dts[4 * i];
		val1 = hdmi_edid_ac3_dts[4 * i + 1];
		val2 = hdmi_edid_ac3_dts[4 * i + 2];
		val3 = hdmi_edid_ac3_dts[4 * i + 3];
		/*printk("%d : 0x%x  %d : 0x%x  %d : 0x%x  %d : 0x%x \r\n",
			i, val0, i+1, val1, i+2, val2, i+3, val3); */
		val = (val3 << 24) | (val2 << 16) | (val1 << 8) | (val0);
		/* printk("val to 0xa50 is : 0x%08x \r\n", val); */
		BASE_WRITE32(0x22a50, val);
	}

	BASE_WRITE32(0x22a04, (hdmi_edid_ac3_dts[255] << 16));
	BASE_WRITE32(0x22a1c, 0x04000000);
#endif
}

UINT8 _au1HDCP[320] = {
	0x00 , 0x14 , 0x7c , 0xe6 , 0x2c , 0x37 , 0xf3 , 0xe8 , 0xa8 , 0x4d , 0x27 , 0x66 , 0xa8 , 0xd0 , 0x2f , 0x13,
	0x55 , 0x79 , 0x97 , 0xe7 , 0x87 , 0x37 , 0x18 , 0x04 , 0xd1 , 0x5f , 0x3b , 0x12 , 0x9f , 0x32 , 0xa4 , 0x6f,
	0x58 , 0x1c , 0x8a , 0xbc , 0x8c , 0x7f , 0xe3 , 0xcb , 0x81 , 0x9e , 0xb4 , 0x45 , 0x7c , 0x66 , 0x6a , 0xcc,
	0xdd , 0x5c , 0xc8 , 0x17 , 0xa3 , 0x90 , 0x43 , 0x91 , 0x8b , 0x01 , 0xf0 , 0x1d , 0xff , 0x8a , 0x1b , 0xde,
	0x94 , 0xd6 , 0xb6 , 0xf6 , 0x6d , 0x15 , 0x7b , 0x42 , 0x13 , 0xa2 , 0x1b , 0x04 , 0xb5 , 0xdd , 0x11 , 0xcc,
	0x42 , 0x28 , 0x66 , 0x65 , 0xf5 , 0x77 , 0xf1 , 0x65 , 0xc3 , 0x8c , 0x9b , 0x2c , 0xad , 0xb4 , 0xe9 , 0x7c,
	0xd1 , 0xbc , 0xd6 , 0x4a , 0x5d , 0xf7 , 0x45 , 0x3e , 0x2a , 0x22 , 0xec , 0xa8 , 0xdf , 0x68 , 0x54 , 0x57,
	0x5a , 0x10 , 0xc8 , 0x38 , 0x9f , 0x94 , 0xa0 , 0xa7 , 0xa0 , 0x71 , 0xa2 , 0x67 , 0x8e , 0x23 , 0xbd , 0x8d,
	0x63 , 0x89 , 0x0d , 0x01 , 0x91 , 0x97 , 0x4c , 0xba , 0x5c , 0x4d , 0x94 , 0x73 , 0x36 , 0x68 , 0x12 , 0x6c,
	0xe8 , 0xfa , 0xb1 , 0x51 , 0xc1 , 0x93 , 0xc6 , 0xce , 0x72 , 0x90 , 0xc1 , 0x6b , 0x4d , 0xf6 , 0x63 , 0x02,
	0xd3 , 0xa6 , 0x9b , 0x80 , 0x35 , 0xb6 , 0xa9 , 0xff , 0x8e , 0xfd , 0xd9 , 0x6f , 0x24 , 0xa6 , 0xdb , 0x4c,
	0xd2 , 0x0c , 0x0f , 0xcf , 0xcd , 0x1a , 0x19 , 0xe4 , 0x62 , 0x9c , 0x6d , 0x17 , 0x6b , 0x57 , 0x39 , 0xcb,
	0x6a , 0x0d , 0x80 , 0x75 , 0xfa , 0xf3 , 0x69 , 0x7d , 0x9f , 0x79 , 0xe3 , 0xc0 , 0x8b , 0x5a , 0xd2 , 0xa8,
	0xc9 , 0xd9 , 0x90 , 0x93 , 0xaa , 0xe6 , 0x1a , 0x1e , 0x17 , 0x93 , 0x03 , 0x2d , 0x43 , 0xc0 , 0xaf , 0x33,
	0x94 , 0x66 , 0xa9 , 0x18 , 0x55 , 0xcc , 0x22 , 0xf5 , 0x23 , 0xc8 , 0xc5 , 0x37 , 0xf1 , 0x81 , 0xd2 , 0x96,
	0xaf , 0x0a , 0x5a , 0xe5 , 0x8a , 0x13 , 0xef , 0x63 , 0x19 , 0x4c , 0xc6 , 0x3f , 0x6c , 0x9a , 0x7a , 0xb2,
	0xa6 , 0xd4 , 0x31 , 0x5a , 0x30 , 0x01 , 0x21 , 0xcd , 0xa2 , 0x86 , 0x74 , 0x04 , 0x46 , 0x3e , 0x38 , 0x08,
	0xe7 , 0x57 , 0xa7 , 0xb4 , 0x41 , 0x73 , 0x02 , 0x78 , 0x93 , 0x34 , 0x93 , 0xce , 0x93 , 0x5a , 0x46 , 0xd3,
	0x0a , 0x14 , 0xf7 , 0x61 , 0x03 , 0xb7 , 0x8f , 0x22 , 0xd2 , 0xf1 , 0x3a , 0x98 , 0xba , 0x28 , 0x62 , 0x53,
	0xca , 0x4c , 0x1d , 0x19 , 0xce , 0x49 , 0x87 , 0xf7 , 0xb5 , 0x26 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00
};

void HdmiRxLoadHdcpKey(void)
{

	UINT32 i;
	UINT32 u4Data;

	/* BASE_WRITE32((0x220FC), ((BASE_READ32(0x220FC))|0xFF)); ///KS_MASK */
	BASE_WRITE32((0x220F8), ((BASE_READ32(0x220F8)) | (0xFF000000))); /* EPST */

	/*  load HDCP key from EEP to SRAM */
	BASE_WRITE32(0x22E84, 0);

	/*  write to hdcp sram pointer. */
	for (i = 0; i < 320; i = i + 4) {
		u4Data = *(UINT32 *)(&_au1HDCP[i]);

		/*  write to hdcp sram pointer. */
		BASE_WRITE32(0x22E88, u4Data);
	}

	u4Data = 0x100 | 0x0A0;
	BASE_WRITE32(0x22E80, u4Data);   /* So 3363 can not find those register. */

	/* HDCP Reset */
	vRxWriteRegMsk(0x22c04, (1 << 11), (1 << 11)); /* SRST HDCP RESET */
	vRxWriteRegMsk(0x22c04, 0, (1 << 11));

	vRxWriteReg(0x22cfc, 0xc3);
}

#if 0
void HdmiRxLoadHdcpKey(void)
{
	UINT32 i;
	UINT8 pui1_key[320];

	memset(pui1_key, 0, 320);
	/* MZ_LoadHdcpData(pui1_key, sizeof(pui1_key)); */


	/* printf metazone hdcp */
	/* RETAILMSG(1,(TEXT("Metazone HDCP: \r\n"))); */
	for (i = 0; i < sizeof(pui1_key); i++) {
		/* RETAILMSG(1,(TEXT("%2x "), pui1_key[i])); */
		if ((i + 1) % 16 == 0) {
			/* RETAILMSG(1,(TEXT("\r\n"))); */
		}
	}

	HDMI_HalLoadHdcp2Sram(pui1_key);
}
#endif

void SetRxHdcpStatus(RxHDCPStateType bStatus)
{
	_RxHDCPState = bStatus;
	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
	{
	    HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX][HDCP]Set RX HDCP STATUS = %s\n",cRxHdcpStatus[_RxHDCPState]);
	}
	*/
}




BOOL PmxDrvSetAvd(
	PMX_SET_AVD_COND_T e_nfy_cond,
	void        *pv_nfy_info,
	UINT32        u4InfoLen)
{
	return FALSE;
}



void HDMIRxHdcpService(void)
{
	BYTE ucOnoff = 0;
	BOOL fgAVMUTE;
	UINT8 _bNewHDMIRxHDCPStatus;

	fgAVMUTE = HDMI_HalIsGcpMuteEnable();


	_bNewHDMIRxHDCPStatus = HDMIHDCPStatusGet();

	if (_bHDMIRxHDCPStatus != _bNewHDMIRxHDCPStatus) {
		/*
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
		{
		    HDMI_LOG(HDMI_LOG_INFO,
		    "[HDMI RX][HDCP]_bHDMIRxHDCPStatus=%d,_bNewHDMIRxHDCPStatus=%d,fgAVMUTE=%d\n",
		    _bHDMIRxHDCPStatus,_bNewHDMIRxHDCPStatus,fgAVMUTE );
		}
		*/
		_bHDMIRxHDCPStatus = _bNewHDMIRxHDCPStatus;

		if (!_bNewHDMIRxHDCPStatus) {
			if (!fgAVMUTE) {
				/*
				if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
				{
				    HDMI_LOG(HDMI_LOG_INFO,
				    "[HDMI RX][HDCP] Upstream disable Enc,notify Tx disable HDCP\n" );
				}
				*/
				RxAuthStartInt();
			}
		}
	}

	if (!HDMI_HalGetSCDT()) {
		if (HalHdcpHdmiMode()) {
			/*
			if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
			{
			    HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX][HDCP] No detect Data Enable signal,Reset Rx Hdcp\n" );
			}
			*/
			RxAuthStartInt();
			HDMI_HalSwReset();
		}

		return;
	}

	if (_bHDCPMode == HDCP_RECEIVER) {
		{
			if (HalHdcpAuthenticationStart()) {
				HalClearHdcpAuthenticationStartStatus();
				HDMI_HalGetAksv(_RxAKSVShadow);
				HDMI_HalGetBksv(_RxBKSVShadow);
				HDMI_HalGetAn(_RxAnShadow);
				/*
				if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
				{
				    HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX][HDCP][SINK MODE] Rx Auth Start\n");
				    HDMI_LOG(HDMI_LOG_INFO,
				    "[HDMI RX][HDCP][SINK MODE] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				    _RxAKSVShadow[0],_RxAKSVShadow[1],_RxAKSVShadow[2],
				    _RxAKSVShadow[3],_RxAKSVShadow[4]);
				    HDMI_LOG(HDMI_LOG_INFO,
				    "[HDMI RX][HDCP][SINK MODE] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				    _RxBKSVShadow[0],_RxBKSVShadow[1],_RxBKSVShadow[2],
				    _RxBKSVShadow[3],_RxBKSVShadow[4]);
				    HDMI_LOG(HDMI_LOG_INFO,
				    "[HDMI RX][HDCP][SINK MODE] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
				    _RxAnShadow[0],_RxAnShadow[1],_RxAnShadow[2],_RxAnShadow[3],
				    _RxAnShadow[4],_RxAnShadow[5],_RxAnShadow[6],_RxAnShadow[7]);
				}
				*/
			}

			_u2RxRiShadow = HDMI_HalGetRi();

			if (_u2RxRiShadow != _u2RxRiShadowOld) {
				_u2RxRiShadowOld = _u2RxRiShadow;
				/*
				if(fgIsHdmiRxDebug( HDMI_RX_DEBUG_HDCP_RI))
				{
				    HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX][HDCP][SINK MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
				}
				*/
			}
		}
		return;
	}

	if (HalHdcpAuthenticationStart()) {
		HalClearHdcpAuthenticationStartStatus();
		/*
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
		{
		    vGetRxAKsv(_RxAKSVShadow);
		    vGetRxBKsv(_RxBKSVShadow);
		    vGetRxAn(_RxAnShadow);
		    HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX][HDCP][REPEATER MODE] Rx Auth Start\n");
		    HDMI_LOG(HDMI_LOG_INFO,
		    "[HDMI RX][HDCP][REPEATER MODE] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
		    _RxAKSVShadow[0],_RxAKSVShadow[1],_RxAKSVShadow[2],_RxAKSVShadow[3],_RxAKSVShadow[4]);
		    HDMI_LOG(HDMI_LOG_INFO,
		    "[HDMI RX][HDCP][REPEATER MODE] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
		    _RxBKSVShadow[0],_RxBKSVShadow[1],_RxBKSVShadow[2],_RxBKSVShadow[3],_RxBKSVShadow[4]);
		    HDMI_LOG(HDMI_LOG_INFO,
		    "[HDMI RX][HDCP][REPEATER MODE] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		    _RxAnShadow[0],_RxAnShadow[1],_RxAnShadow[2],_RxAnShadow[3],
		    _RxAnShadow[4],_RxAnShadow[5],_RxAnShadow[6],_RxAnShadow[7]);
		}
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
		{
		    if(fgHalHdcpAuthenticationStart())
		    {
				HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP START STATUS FAIL\n");
		    }
		    else
		    {
				HDMI_LOG(HDMI_LOG_DEBUG,
				"[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP START STATUS SUCCESS\n");
		    }
		}
		*/
		TxSetRxHdcpAuthDone(FALSE);
		SetRxHdcpStatus(RxHDCP_Computations);
		PmxDrvSetAvd(PMX_HDCP_ON_OFF, &ucOnoff, 1);
	}

	switch (_RxHDCPState) {
	case RxHDCP_UnAuthenticated:
		break;

	case RxHDCP_Computations:
		if (HalHdcpAuthenticationDone()) {
			HalClearHdcpAuthenticationDoneStatus();
			_u2RxRiShadow = HDMI_HalGetRi();
			/*
			if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HDCP))
			{
			    _u2RxRiShadowOld = _u2RxRiShadow;
			    UTIL_Printf("[HDMI RX][HDCP][REPEATER MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
			    if(HalHdcpAuthenticationDone())
				{
					HDMI_LOG(HDMI_LOG_DEBUG,
					"[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP DONE STATUE FAIL\n");
			    }
			    else
				{
					HDMI_LOG(HDMI_LOG_DEBUG,
					"[HDMI RX][HDCP][REPEATER MODE] CLEAR HDCP DONE STATUE SUCCESS\n");
			    }
			}
			*/
			SetRxHdcpStatus(RxHDCP_WaitforDownstream);
			_wRxWaitTxKsvListCount = 0;
		}

		break;

	case RxHDCP_WaitforDownstream:
		_wRxWaitTxKsvListCount++;

		if (_wRxWaitTxKsvListCount > 250) { /* timer is about 80ms */
			RxAuthStartInt();
			break;
		}

		if (TxAuthDone()) {
			SetRxHdcpStatus(RxHDCP_AssembleKSVList);
		}

		break;

	case RxHDCP_AssembleKSVList:
		HDMIRxGenV();
		SetRxHdcpStatus(RxHDCP_WaitVReady);
		break;

	case RxHDCP_WaitVReady:
		if (HalIsVReady()) {
			if (_fgTxVMatch) {
				HalSetKsvReadyBit();
			}

			SetRxHdcpStatus(RxHDCP_Authenticated);
		}

		break;

	case RxHDCP_Authenticated:
		/*
		if(fgIsHdmiRxDebug( HDMI_RX_DEBUG_HDCP_RI))
		{
		    _u2RxRiShadow = vGetRxRi();
		    if(_u2RxRiShadow!=_u2RxRiShadowOld)
		    {
				_u2RxRiShadowOld = _u2RxRiShadow;
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP][REPEATER MODE] Rx Ri =0x%x\n",_u2RxRiShadow);
		    }
		}
		*/
		break;

	default:
		break;
	}
}

void RxHdcpMode(UINT8 u1Mode)
{

	RxHDCPSetReceiver();

}

void EnableHdmiRxDebug(UINT32 u4MessageType)
{
	_u4DebugRxMessageType |= u4MessageType;
}

void DisableHdmiRxDebug(UINT32 u4MessageType)
{
	_u4DebugRxMessageType &= ~u4MessageType;

}

BOOL UpStreamNeedAuth(void)
{

#if CONFIG_DRV_HDMI_SUPPORT_HDCP_BDS_X80
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]_RxHDCPSetStatus = %d\n ", _RxHDCPSetStatus);

	if (_RxHDCPSetStatus == RxHDCPSet_UnAuthenticated) {
		return FALSE;
	} else {
		return TRUE;
	}

#else

	if (_RxHDCPState == RxHDCP_UnAuthenticated) {
		return FALSE;
	} else {
		return TRUE;
	}

#endif
}

void ShowRxHpdRsenStatus(void)
{
	/*
	   if(RegReadFldAlign(ANA_INTF_1,RG_HDMI_TERM_EN) == 1)
	   {
	   HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]RSEN On\n");
	   }
	   else
	   {
	   HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]RSEN Off\n");
	   }
	   */
	if (HDMI_HalGetPwr5V() != 0) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]Power 5V On\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]Power 5V Off\n");
	}



}

void ShowRxEDIDStatus(void)
{
	/* vHdmiDumpEdidTable(); */
}


void ShowRxHdcpStatus(void)
{
	if (_fgTxHDCPAuthDone) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDCP Auth\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDCP Not Auth\n");
	}

}

void ShowRxHDCPBstatus(void)
{
	UINT16 u2Temp = 0;

	if (_bHDCPMode == HDCP_REPEATER) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] Bstatus = 0x%x\n", _RxBstatus);

		if (_RxBstatus & (0x1 << 12)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI MODE = 1\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI MODE = 0\n");
		}

		if (_RxBstatus & (0x1 << 11)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] MAX_CASCADE_EXCEEDED = 1\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] MAX_CASCADE_EXCEEDED = 0\n");
		}

		u2Temp = (_RxBstatus >> 8) & (0x7);
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] DEPTH = %d\n", u2Temp);

		if (_RxBstatus & (0x1 << 7)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] MAX_DEVS_EXCEEDED = 1\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] MAX_DEVS_EXCEEDED = 0\n");
		}

		u2Temp = _RxBstatus & 0x7F;
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] DEVICE_COUNT = %d\n", u2Temp);

	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] A connected device is only Sink!!\n");
	}


}

void ShowRxSynDetStatus(void)
{
	if (HDMI_HalGetSCDT() == 1) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] Detceted Sync\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] No Detceted Sync\n");
	}

}

void ShowRxHDMIModeStatus(void)
{
	if (HDMI_HalIsHdmiMode()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]RX RECEIVER HDMI\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]RX RECEIVER DVI\n");
	}

}
void ShowRxColorSpaceStatus(void)
{
	if (HDMIInputType() == 0x0) { /*  YCbCr */
		if (!HDMI422Input()) { /*  4:4:4 */
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = YCbCr444\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = YCbCr422\n");
		}
	} else { /* RGB */
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = RGB\n");
	}
}

void ShowRxDeepColorStatus(void)
{
	if (HDMIDeepColorStatus() == RX_NON_DEEP) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 24 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_30BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 30 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_36BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 36 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_48BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 48 BIT\n");
	}
}

void ShowRx3DStatus(void)
{
}

void ShowRxInputStatus(void)
{

	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]Input Status: HDMI/MHL\n");

}

void ShowRxNCTSStatus(void)
{
	UINT32 wCTS_HW, wN_HW;

	wCTS_HW = HDMI_HalGetRxHwCTSValue();
	wN_HW = HDMI_HalGetRxHwNValue();
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]N = 0x%x\n", (unsigned int)wN_HW);
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]CTS = 0x%x\n" , (unsigned int)wCTS_HW);

}

void ShowAVMuteStatus(void)
{
	if (_HdmiAvMuteShalow == TRUE) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]SET_AVMUTE\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]Clear_AVMUTE\n");
	}

}


void ShowAudioStatus(void)
{
	UINT8 u1ChannelNo, u1BitLen;
	HDMI_RX_IN_AUDIO_INFO_T prAudIf;
	AUDIO_CAPS pAudioCaps;
	Audio_State_Type state;

	state = 0;

	u1ChannelNo = 2;
	u1BitLen = 0;

	pAudioCaps.AudInf.info.DM_INH = 0;
	pAudioCaps.AudioFlag = 0;
	pAudioCaps.AudSrcEnable = 0;
	pAudioCaps.AudChStat.WorldLen = 0;
	pAudioCaps.ChStat[0] = 0;
	pAudioCaps.ChStat[1] = 0;
	pAudioCaps.ChStat[2] = 0;
	pAudioCaps.ChStat[3] = 0;
	pAudioCaps.ChStat[4] = 0;
	pAudioCaps.SampleFreq = 0;


	vGetHdmiRxAudioParameter(&prAudIf);
	getHDMIRxInputAudio(&pAudioCaps);
	state = GetRxAudioState();

	if (state == ASTATE_AudioOn) {

#if 0

		if (1) { /* (_u1HDMIRxAudTmtType != 0) */
			if ((_u1HDMIRxAudTmtType == 3) || (_u1HDMIRxAudTmtType == 22)
			    || (_u1HDMIRxAudTmtType == 43)) {
				if (pAudioCaps.AudInf.info.DM_INH == 1) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio DownMix status: 5.1ch->2ch\n");
				} else {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio DownMix status: No downmix\n");
				}

				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio is PCM\n");

				if ((pAudioCaps.AudioFlag & B_LAYOUT) == B_MULTICH) {
					if (1) {
						u1ChannelNo =
							(_RxPacket[0x22/*AUDIO_INFOFRAME*/].PacketData[4] & 0x07) + 1;
					} else {
						u1ChannelNo = 6;
					}
				} else {
					u1ChannelNo = 2;
				}

				if (u1ChannelNo > 0) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Number = %d\n", u1ChannelNo);
				} else {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Number = refer stream header\n");
				}

				if ((pAudioCaps.AudChStat.WorldLen & 0x1) == 0x1) {
					switch (pAudioCaps.AudChStat.WorldLen >> 1) {
					case 1:
						u1BitLen = 20;
						break;

					case 2:
						u1BitLen = 22;
						break;

					case 4:
						u1BitLen = 23;
						break;

					case 5:
						u1BitLen = 24;
						break;

					case 6:
						u1BitLen = 21;
						break;

					default:
						break;
					}
				} else {
					switch (pAudioCaps.AudChStat.WorldLen >> 1) {
					case 1:
						u1BitLen = 16;
						break;

					case 2:
						u1BitLen = 18;
						break;

					case 4:
						u1BitLen = 19;
						break;

					case 5:
						u1BitLen = 20;
						break;

					case 6:
						u1BitLen = 17;
						break;

					default:
						break;
					}
				}

				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]BIT  = %d\n", u1BitLen);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status0 = %d\n", pAudioCaps.ChStat[0]);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status1 = %d\n", pAudioCaps.ChStat[1]);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status2 = %d\n", pAudioCaps.ChStat[2]);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status3 = %d\n", pAudioCaps.ChStat[3]);
				HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status4 = %d\n", pAudioCaps.ChStat[4]);
			} else {
				if (_u1HDMIRxAudTmtType == 2) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio AC3\n");
				}


				if (_u1HDMIRxAudTmtType == 6) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio DTS\n");
				}

				if (pAudioCaps.AudioFlag & B_CAP_DSD_AUDIO) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio In DSD\n");
				}

				if (pAudioCaps.AudioFlag & B_CAP_HBR_AUDIO) {
					HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio In HBR\n");
				}


			}
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio In Unknow\n");
		}

#endif

		if (pAudioCaps.SampleFreq == 0) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 44.1khz\n");
		}

		if (pAudioCaps.SampleFreq == 2) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 48khz\n");
		}

		if (pAudioCaps.SampleFreq == 3) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 32khz\n");
		}

		if (pAudioCaps.SampleFreq == 8) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 88.2khz\n");
		}

		if (pAudioCaps.SampleFreq == 9) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 768khz\n");
		}

		if (pAudioCaps.SampleFreq == 0x0a) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 96khz\n");
		}

		if (pAudioCaps.SampleFreq == 0x0c) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 176k hz\n");
		}

		if (pAudioCaps.SampleFreq == 0x0E) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX AUD]FS is 192k hz\n");
		}

		HDMI_HalGetI2sMclk();
	} else {
		if (state == ASTATE_AudioOff) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio In OFF\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Audio Unstable\n");
		}
	}

	if (_bHdmiAudioOutputMode) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDMI RX AUDIO OUTPUT MODE:HDMI\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDMI RX AUDIO OUTPUT MODE:SPEAKER or SPEAKER+HDMI\n");
	}

}


void ShowHDMIRxAudioChannelStatus(void)
{

	AUDIO_CAPS pAudioCaps;

	pAudioCaps.ChStat[0] = 0;
	pAudioCaps.ChStat[1] = 0;
	pAudioCaps.ChStat[2] = 0;
	pAudioCaps.ChStat[3] = 0;
	pAudioCaps.ChStat[4] = 0;
	pAudioCaps.AudioFlag = 0;
	getHDMIRxInputAudio(&pAudioCaps);


	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status0 = %d\n", pAudioCaps.ChStat[0]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status1 = %d\n", pAudioCaps.ChStat[1]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status2 = %d\n", pAudioCaps.ChStat[2]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status3 = %d\n", pAudioCaps.ChStat[3]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Channel Status4 = %d\n", pAudioCaps.ChStat[4]);


}



void HdmiRxHdcpStatus(void)
{
	BYTE i;
	BYTE _RxAKSV[5];
	BYTE _RxBKSV[5];
	BYTE _RxAn[8];

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDCP STATUS\n");

	if (_bHDCPMode == HDCP_RECEIVER) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDCP RECEIVER MODE\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDCP REPEATER MODE\n");
	}

	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]RX HDCP STATUS = %s\n", cRxHdcpStatus[_RxHDCPState]);
	HDMI_HalGetAksv(_RxAKSV);
	HDMI_HalGetBksv(_RxBKSV);
	HDMI_HalGetAn(_RxAn);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP] Rx AKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
		 _RxAKSV[0], _RxAKSV[1], _RxAKSV[2], _RxAKSV[3], _RxAKSV[4]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP] Rx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
		 _RxBKSV[0], _RxBKSV[1], _RxBKSV[2], _RxBKSV[3], _RxBKSV[4]);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP] Rx AN[1~8]=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		 _RxAn[0], _RxAn[1], _RxAn[2], _RxAn[3], _RxAn[4], _RxAn[5], _RxAn[6], _RxAn[7]);

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI TX][HDCP] _TxDownStreamCount = 0x%x\n", _TxDownStreamCount);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI TX][HDCP] _TxBStatus = 0x%x\n", _TxBStatus);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI TX][HDCP] Tx BKSV[1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
		 _TxBKsv[0], _TxBKsv[1], _TxBKsv[2], _TxBKsv[3], _TxBKsv[4]);

	for (i = 0; i < _TxDownStreamCount; i++) {
		if (i < 9)
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI TX] Tx KSVLIST[%d][1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				 i, _TxKsvList[5 * i], _TxKsvList[5 * i + 1], _TxKsvList[5 * i + 2],
				 _TxKsvList[5 * i + 3], _TxKsvList[5 * i + 4]);
	}

	if (_fgTxHDCPAuthDone) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI TX]HDCP Auth\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI TX]HDCP Not Auth\n");
	}

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP] _RxDownStreamCount = 0x%x\n", _RxDownStreamCount);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX][HDCP] _RxBStatus = 0x%x\n", _RxBstatus);

	for (i = 0; i < _RxDownStreamCount; i++) {
		if (i < 10)
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] Rx KSVLIST[%d][1~5]=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				 i, _RxKsvList[5 * i], _RxKsvList[5 * i + 1],
				 _RxKsvList[5 * i + 2], _RxKsvList[5 * i + 3],
				 _RxKsvList[5 * i + 4]);
	}

	/*  UTIL_Printf("[HDMI RX][HDCP] RxHdcpKey addr = 0x%x\n",&RxHdcpKey); */

}





void HdmiRxHdmiStatus(void)
{
	UINT32 wCTS_HW, wN_HW;

	if (HDMI_HalIsHdmiMode()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]RX RECEIVER HDMI\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]RX RECEIVER DVI\n");
	}

	if (HDMIDeepColorStatus() == RX_NON_DEEP) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 24 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_30BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 30 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_36BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 36 BIT\n");
	} else if (HDMIDeepColorStatus() == RX_48BITS_DEEP_COLOR) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]DEEP COLOR = 48 BIT\n");
	}

	if (HDMIInputType() == 0x0) { /*  YCbCr */
		if (!HDMI422Input()) { /*  4:4:4 */
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = YCbCr444\n");
		} else {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = YCbCr422\n");
		}
	} else { /* RGB */
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Color Space = RGB\n");
	}

	if (_HdmiAvMuteShalow == TRUE) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]SET_AVMUTE\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Clear_AVMUTE\n");
	}

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDMI RX STATUS\n");
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]_bHDMIState = %s\n", cHDMIState[_bHDMIState]);
	HDMI_LOG(HDMI_LOG_DEBUG, "_bHdmiRepeaterMode = %d, _bAppHdmiRepeaterMode = %d\n",
		 _bHdmiRepeaterMode, _bAppHdmiRepeaterMode);
	HDMI_LOG(HDMI_LOG_DEBUG, "_bHDMICurrSwitch = %d, _bAppHDMICurrSwitch = %d\n",
		 _bHDMICurrSwitch, _bAppHDMICurrSwitch);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]_u1RxSysState = %d\n", _u1RxSysState);


	HDMI_LOG(HDMI_LOG_DEBUG, "_bMHLMode = %d, _bMHLModeBackup = %d\n", (int)_bMHLMode, (int)_bMHLModeBackup);
	HDMI_LOG(HDMI_LOG_DEBUG, "_bPPMode = %d, _bPPModeBackup = %d\n", (int)_bPPMode, (int)_bPPModeBackup);
	HDMI_LOG(HDMI_LOG_DEBUG, "is_sink_hpd_on = %d\n", (int)is_sink_hpd_on);


	if (_bHdmiRepeaterMode == 0x22/*HDMI_REPEATER_VIDEO_BYPASS_MODE*/) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_REPEATER_VIDEO_BYPASS_MODE\n");
	} else if (_bHdmiRepeaterMode == 0x22/*HDMI_REPEATER_VIDEO_DRAM_MODE*/) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_REPEATER_VIDEO_DRAM_MODE\n");
	} else if (_bHdmiRepeaterMode == 0x22/*HDMI_SOURCE_MODE*/) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_SOURCE_MODE\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_MODE error\n");
	}

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]_u1TxEdidReady = %d, _bHdmiAudioOutputMode = %d\n",
		 _u1TxEdidReady, _bHdmiAudioOutputMode);

	if (_bHdmiAudioOutputMode) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDMI RX AUDIO OUTPUT MODE:HDMI\n");
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]HDMI RX AUDIO OUTPUT MODE:SPEAKER or SPEAKER+HDMI\n");
	}


	wCTS_HW = HDMI_HalGetRxHwCTSValue();
	wN_HW = HDMI_HalGetRxHwNValue();
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]N = %x\n", (unsigned int)wN_HW);
	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]CTS = %x\n" , (unsigned int)wCTS_HW);

	if (IS_AUD_MUTE()) {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDMI RX AUDIO MUTE\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]HDMI RX AUDIO UNMUTE\n");
	}


	ShowAviInforFrame();
	ShowAudioInforFrame();
	ShowSPDInforFrame();
	ShowGamutInforFrame();

}
void HdmiRxStatus(void)
{
	HdmiRxHdmiStatus();
	HdmiRxDviStatus();
	HdmiRxHdcpStatus();
}

void ShowHDMIRxStatus(void)
{
	ShowRxHpdRsenStatus();
	ShowRxEDIDStatus();
	ShowRxHdcpStatus();
	ShowRxSynDetStatus();
	ShowRxResoInfoStatus();
	ShowRxHDMIModeStatus();
	ShowRxColorSpaceStatus();
	ShowRxDeepColorStatus();
	ShowRx3DStatus();
	ShowRxInputStatus();
	ShowRxNCTSStatus();
	ShowAVMuteStatus();
	ShowAudioStatus();

}

void ShowHDMIRxInfo(void)
{
	ShowAviInforFrame();
	ShowAudioInforFrame();
	ShowSPDInforFrame();
	ShowMPEGInforFrame();
	ShowACPInforFrame();
	ShowISRC1InforFrame();
	ShowISRC2InforFrame();
	ShowVENDInforFrame();
	ShowGamutInforFrame();
	ShowGCPInforFrame();
}


HDMI_RX_PACKET_INFO _RxPacket[MAX_PACKET];


void HdmiRxPacketDataInit(void)
{
	BYTE i, j;
	unsigned int LastestTime;

	Linux_HAL_GetTime((unsigned long *)(&LastestTime));

	for (i = 0; i < MAX_PACKET; i++) {
		/* if(_RxPacket[i].fgValid)
			{
			vSendHdmiPacket(i,_RxPacket[i].fgValid, &_RxPacket[i].PacketData[0]);
			} */
		_RxPacket[i].fgValid = 0;
		_RxPacket[i].fgChanged = 0;
		_RxPacket[i].u4LastReceivedTime = LastestTime;

		for (j = 0; j < 31; j++) {
			_RxPacket[i].PacketData[j] = 0;
		}
	}

	_RxPacket[AVI_INFOFRAME].PacketHeader = AVI_INFOFRAME_HEADER;
	_RxPacket[AUDIO_INFOFRAME].PacketHeader = AUDIO_INFOFRAME_HEADER;
	_RxPacket[MPEG_INFOFRAME].PacketHeader = MPEG_INFOFRAME_HEADER;
	_RxPacket[SPD_INFOFRAME].PacketHeader = SPD_INFOFRAME_HEADER;
	_RxPacket[VENDOR_INFOFRAME].PacketHeader = VS_INFOFRAME_HEADER;
	_RxPacket[ACP_PACKET].PacketHeader = ACP_PACKET_HEADER;
	_RxPacket[ISRC1_PACKET].PacketHeader = ISRC1_PACKET_HEADER;
	_RxPacket[ISRC2_PACKET].PacketHeader = ISRC2_PACKET_HEADER;
	_RxPacket[GAMUT_PACKET].PacketHeader = GAMUT_PACKET_HEADER;

	_RxPacket[AVI_INFOFRAME].u4timeout = 100; /* 100ms */
	_RxPacket[AUDIO_INFOFRAME].u4timeout = 100;
	_RxPacket[MPEG_INFOFRAME].u4timeout = 100;
	_RxPacket[SPD_INFOFRAME].u4timeout = 2000;
	_RxPacket[VENDOR_INFOFRAME].u4timeout = 100;
	_RxPacket[ACP_PACKET].u4timeout = 600; /* 600ms */
	_RxPacket[ISRC1_PACKET].u4timeout = 300; /* 300ms */
	_RxPacket[ISRC2_PACKET].u4timeout = 300;
	_RxPacket[GAMUT_PACKET].u4timeout = 100;

	_RxPacket[AVI_INFOFRAME].bLength = 14; /* include checksum */
	_RxPacket[AUDIO_INFOFRAME].bLength = 11; /* include checksum */
	_RxPacket[MPEG_INFOFRAME].bLength = 11; /* include checksum */
	_RxPacket[SPD_INFOFRAME].bLength = 26; /* include checksum */
	_RxPacket[VENDOR_INFOFRAME].bLength = 28;
	_RxPacket[ACP_PACKET].bLength = 28;
	_RxPacket[ISRC1_PACKET].bLength = 28;
	_RxPacket[ISRC2_PACKET].bLength = 28;
	_RxPacket[GAMUT_PACKET].bLength = 28;


	SetSelectUnRecpacket(TRUE, ISRC2_PACKET_HEADER);
	HDMI_HalSetVSNewOnly(FALSE);
	HDMI_HalSetISRC1NewOnly(FALSE);

	HDMI_HalMpegAddrSetSelectPacket(MPEG_INFOFRAME_HEADER);
	vNotifyHDMIRxACPTypeChange(0/*ACP_LOST_DISABLE*/);

	_HdmiAvMuteShalow = FALSE;
}

BOOL Isrc1PacketValidIsSet(void)
{
	return (_RxPacket[ISRC1_PACKET].PacketData[1] & 0x40);
}

BOOL Isrc1PacketContIsSet(void)
{
	return (_RxPacket[ISRC1_PACKET].PacketData[1] & 0x80);
}

BOOL ChecksumOk(BYTE bType)
{
	BYTE i;
	BYTE bData = 0;

	for (i = 3; i < 31; i++) {
		bData += _RxPacket[bType].PacketData[i];
	}

	if (bData) {
		/*
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_INFOFRAME))
		{
		    UTIL_Printf( "[HDMI RX]%s CHECKSUM FALSE\n",cHDMIPacketName[bType]);
		}
		*/
		return FALSE;
	}
	/*
	if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_INFOFRAME))
	{
		UTIL_Printf( "[HDMI RX]%s CHECKSUM OK\n",cHDMIPacketName[bType]);
	}
	*/
	return TRUE;
}

void HdmiRxGetPacketData(void)
{
	BYTE i, j;
	BYTE bIndex;
	BYTE bcount;
	BYTE bBuffer1[31];
	BYTE bBuffer2[31];
	BYTE *prData;
	unsigned long CurrTime = 0;

	Linux_HAL_GetTime(&CurrTime);
	/* HAL_GetTime(&CurrTime); */

	bIndex = AVI_INFOFRAME;

	if (HalIsINTR3_NEW_AVI()) {
		HDMI_HalClearNewAviIntStatus();
		HDMI_HalGetAviInfoframe(bBuffer1);
		HDMI_HalGetAviInfoframe(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 19; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 19) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 19; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 19) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}

	bIndex = AUDIO_INFOFRAME;

	if (HalIsINTR3_NEW_AUD()) {
		HDMI_HalClearNewAudIntStatus();
		HDMI_HalGetAudioInfoframe(bBuffer1);
		HDMI_HalGetAudioInfoframe(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 14; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 14) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 14; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 14) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}

	bIndex = SPD_INFOFRAME;

	if (HalIsINTR3_NEW_SPD()) {
		HDMI_HalClearNewSpdIntStatus();
		HDMI_HalGetSpdInfoframe(bBuffer1);
		HDMI_HalGetSpdInfoframe(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}

	bIndex = VENDOR_INFOFRAME;

	if (HalIsINTR_NEW_VS()) {
		HDMI_HalClearNewVSIntStatus();
		HDMI_HalGetVSInfoframe(bBuffer1);
		HDMI_HalGetVSInfoframe(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}



	bIndex = GAMUT_PACKET;

	if (HalIsINTR7_GAMUT()) {
		HalClearGamutIntStatus();
		HDMI_HalGetGamutPacket(bBuffer1);
		HDMI_HalGetGamutPacket(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}


	bIndex = ACP_PACKET;

	if (HDMI_HalIsNewAcp()) {
		HDMI_HalClearNewAcpIntStatus();
		HDMI_HalGetAcpPacket(bBuffer1);
		HDMI_HalGetAcpPacket(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			/* 2010/08/04,ychung, Check if ACP Type is changed , if yes  notify hdmi rx audio task */
			if (bBuffer2[0] == ACP_PACKET_HEADER) {
				if (bBuffer2[1] != _RxPacket[bIndex].PacketData[1]) {
					vNotifyHDMIRxACPTypeChange(bBuffer2[1]);
					_RxPacket[AUDIO_INFOFRAME].fgChanged = TRUE;
			/* above, jitao@20111031 for acp type is DVD-A
			SACD will mute Audio Infoframe. */
				}
			}

			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = TRUE;
		}

	}

	else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
				/* vNotifyHDMIRxACPTypeChange(ACP_LOST_DISABLE); */
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}


	bIndex = MPEG_INFOFRAME;

	if (HalIsINTR3_NEW_MPEG()) {
		HDMI_HalClearNewMpegIntStatus();
		HDMI_HalGetMpegInfoframe(bBuffer1);
		HDMI_HalGetMpegInfoframe(bBuffer2);

		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			_RxPacket[bIndex].fgValid = TRUE;

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}


	bIndex = ISRC1_PACKET;

	if (HalIsINTR_NEW_ISRC1()) {
		HDMI_HalClearNewISRC1IntStatus();
		HDMI_HalGetISRC1Infoframe(bBuffer1);
		HDMI_HalGetISRC1Infoframe(bBuffer2);
		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			if (_RxPacket[bIndex].fgValid == FALSE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = TRUE;

		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}


	bIndex = ISRC2_PACKET;

	if (HalIsINTR3_NEW_UNREC()) {
		HDMI_HalClearNewUnRecIntStatus();
		HDMI_HalGetUnRecPacket(bBuffer1);
		HDMI_HalGetUnRecPacket(bBuffer2);

		_RxPacket[bIndex].u4LastReceivedTime = CurrTime;
		bcount = 0;

		for (i = 0; i < 31; i++) {
			if (bBuffer1[i] == bBuffer2[i]) {
				bcount++;
			}
		}

		if (bcount == 31) {
			prData = &_RxPacket[bIndex].PacketData[0];
			bcount = 0;

			for (i = 0; i < 31; i++) {
				if (*(prData + i) == bBuffer2[i]) {
					bcount++;
				}

				*(prData + i) = bBuffer2[i];
			}

			if (bcount != 31) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			if (_RxPacket[bIndex].fgValid == FALSE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = TRUE;
			/* UTIL_Printf( "[HDMI RX]ISRC2 received,change to ISRC1\n"); */
		}

	} else {
		if (Linux_HAL_GetDeltaTime(&_RxPacket[bIndex].u4timeout,
			&_RxPacket[bIndex].u4LastReceivedTime, &CurrTime)) {
			if (_RxPacket[bIndex].fgValid == TRUE) {
				_RxPacket[bIndex].fgChanged = TRUE;
			}

			_RxPacket[bIndex].fgValid = FALSE;

		}
	}



	for (i = 0; i < MAX_PACKET; i++) {
		if (_RxPacket[i].fgChanged) {
			_RxPacket[i].fgChanged = 0;

			if (!(_RxPacket[i].fgValid)) {
				for (j = 0; j < 31; j++) { /* don't clear packet header */
					_RxPacket[i].PacketData[j] = 0;
				}
			}

		}
	}

	/* notify GCP MUTE */
	i = HDMI_HalIsGcpMuteEnable();

	if (_HdmiAvMuteShalow != i) {
		_HdmiAvMuteShalow = i;
		/* if(fgHdmiRepeaterIsBypassMode()) */
		{
			/* vHdmiRxDrvSetAvd(0, &_HdmiAvMuteShalow); 0= HDMI_RX_SET_TX_GCP*/
		}
	}
}



void HdmiGet3DInfo(BOOL fgPrintLog)
{
	if ((_RxPacket[VENDOR_INFOFRAME].fgValid) &&
		(_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_HB0] == VS_INFOFRAME_HEADER)) {
		if ((_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4] >> 5) == _3D_FORMAT_PRESENT) {
			_3DInfo.HDMI_3D_Enable = 1;
			_3DInfo.HDMI_3D_Video_Format = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB4] >> 5);
			_3DInfo.HDMI_3D_Structure = (_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB5]) >> 4;

			if (_3DInfo.HDMI_3D_Structure >= HDMI_3D_Structure_SideBySideHalf) {
				_3DInfo.HDMI_3D_EXTDATA =
					(_RxPacket[VENDOR_INFOFRAME].PacketData[VS_INFOFRAME_PB6]) >> 4;
			} else {
				_3DInfo.HDMI_3D_EXTDATA = 0;
			}
		} else {
			_3DInfo.HDMI_3D_Enable = 0;
		}
	} else {
		_3DInfo.HDMI_3D_Enable = 0;
	}

	if (_u1Force3DType) {
		_3DInfo.HDMI_3D_Enable = 1;

		if (_u1Force3DType == 1) {
			_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_FramePacking;
		} else if (_u1Force3DType == 2) {
			_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_SideBySideHalf;
		} else if (_u1Force3DType == 3) {
			_3DInfo.HDMI_3D_Structure = HDMI_3D_Structure_TopBottom;
		}

	}

	/*if (_3DInfo.HDMI_3D_Enable) {

	} else {

	}*/
}

void HdmiRxForce3D(UINT32 bType)
{
	_u1Force3DType = bType;
}

UINT32 GetRxHwCTSValue(void)
{
	UINT32 u4Data = 0;

	u4Data = HDMI_HalGetRxHwCTSValue();
	return u4Data;


}

UINT32 GetRxHwNValue(void)
{
	UINT32 u4Data = 0;

	u4Data = HDMI_HalGetRxHwNValue();
	return u4Data;
}

BOOL fgIsHdmiRxDebug(UINT32 u4MessageType)
{
	if (_u4DebugRxMessageType & u4MessageType) {
		return TRUE;
	} else {
		return FALSE;
	}
}




void BdModeChk(void)
{

	if ((_bHdmiRepeaterMode != _bAppHdmiRepeaterMode) || (_bAppHDMICurrSwitch != _bHDMICurrSwitch)) {


		/*  HDMIEnable(FALSE); */


		if (_bHdmiRepeaterMode != _bAppHdmiRepeaterMode) {
			if (_bAppHdmiRepeaterMode == 0/*HDMI_SOURCE_MODE*/) {
				_bHdmiRepeaterMode = _bAppHdmiRepeaterMode;
				HalDisableHDCPDDCPort();

				if (!_bMHLMode) {
					HDMITMDSCTRL(FALSE);
				}

				/* vHalRxSwitchPortSelect(_bAppHDMICurrSwitch); */
				/* vSetHdmiDisableCcirIn(); */
				_bHDMICurrSwitch = HDMI_SWITCH_INIT;
				/* vChageRxSysState(RX_IDLE_STATE); */
				_bHDMIState = HDMI_STATE_PWOFF;

				/* SinkTaskSetStandby(TRUE); */

			} else {
				/* vChageRxSysState(RX_DETECT_STATE); */

				if (!_bMHLMode) {
					HDMITMDSCTRL(FALSE);
				}

				/* vHalRxSwitchPortSelect(_bAppHDMICurrSwitch); */
				HDMIHPDHigh(FALSE);

				/* SinkTaskClrHPD(); */
				/* SinkTaskSetStandby(FALSE); */

				_bHdmiRepeaterMode = _bAppHdmiRepeaterMode;
				_bHDMICurrSwitch = _bAppHDMICurrSwitch;
				_bHDMIState = HDMI_STATE_INIT;

			}
		} else {
			_bHDMICurrSwitch = _bAppHDMICurrSwitch;
			HalDisableHDCPDDCPort();

			if (!_bMHLMode) {
				HDMITMDSCTRL(FALSE);
			}

			HDMIHPDHigh(FALSE);

			/* SinkTaskClrHPD(); */
			_bHDMIState = HDMI_STATE_INIT;
		}

	}
}






UINT32 _u4Pwr5VStatus = 0;

void CheckPwr5vStatus(void)
{

	UINT32 u4Pwr5VStatus = 0;

	u4Pwr5VStatus = HDMI_HalGetPwr5V();

	/* HDMI_LOG(HDMI_LOG_DEBUG, "Hdmi Power5V = %d \r\n", u4Pwr5VStatus); */
	if (u4Pwr5VStatus != _u4Pwr5VStatus) {

		HDMI_LOG(HDMI_LOG_INFO, "Hdmi Power5V change notify: %u -> %u \r\n",
			(unsigned int)_u4Pwr5VStatus, (unsigned int)u4Pwr5VStatus);

		/* notify APP */
		/* vCBRMNfyFuncHDMIRxPWR5VStatus(u4Pwr5VStatus); */
		_u4Pwr5VStatus = u4Pwr5VStatus;
	}
}






void HDMIEnable(BOOL fgEnable)
{
}



void SinkSetAttachMode(BOOL bMHLEn)
{
	if (_bMHLMode != bMHLEn) {
		HDMI_LOG(HDMI_LOG_DEBUG, "vSinkSetAttachMode bMHLEn = %d; _bMHLModeBackup = %d\n",
			(int)bMHLEn, (int)_bMHLModeBackup);
	}

	_bMHLModeBackup = _bMHLMode;
	_bMHLMode = bMHLEn;

	if (bMHLEn) {
		/*  clock divider */
		HDMI_WRITE32_MASK(REG_MHL_HDCP_CTRL1, 0x20 << 8, RISC_HDCP_CLK_DIV);
		/*  TODO, set 1 at MHL mode and set 0 at non-MHL modes */
		HDMI_WRITE32_MASK(REG_MHL_HDCP_CTRL1, 0x1 << 0, RISC_ADDR_PAGE);
	} else {
		/* clock divider
			vIO32WriteFldAlign(MHL_HDCP_CTRL_1, 0x20, RISC_HDCP_CLK_DIV);
			TODO, set 1 at MHL mode and set 0 at non-MHL mode
			vIO32WriteFldAlign(MHL_HDCP_CTRL_1, 0x0, HDCP_RISC_SEL); */
		HDMI_WRITE32_MASK(REG_MHL_HDCP_CTRL1, 0x0, RISC_ADDR_PAGE);

		/* GPIO_Config(PIN_HDMI_HPD_CBUS_RX, 0,1); */
	}
}

void SinkSetPPMode(BOOL bPPMode)
{
	_bPPModeBackup = _bPPMode;
	_bPPMode = bPPMode;
}

void SinkTaskSetPathEn(BOOL fgEn)
{
	/* vHalRxMHLTMDSCTRL(bEn); */
	HDMI_HalTmdsOn(fgEn);
}

void SinkDDCWriteData(UINT8 uDevID, UINT8 uOffset, UINT8 bData)
{

	UINT32 u4Data;

	if (CBUS_DDC_DATA_HDCP_ADRW == uDevID) {
		u4Data = (uOffset << 8) + bData;
		HDMI_WRITE32(REG_MHL_HDCP_CTRL1, 0x1401);
		HDMI_WRITE32(REG_MHL_HDCP_CTRL0, u4Data);
	}
}

static BYTE mhl_edid[256] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x4d, 0xd9, 0x00, 0x84, 0x01, 0x01, 0x01, 0x01,
	0x32, 0x0f, 0x01, 0x03, 0x80, 0x46, 0x28, 0x78, 0x0a, 0xee, 0x91, 0xa3, 0x54, 0x4C, 0x99, 0x26,
	0x0f, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80, 0xd0, 0x72, 0x38, 0x2d, 0x40, 0x10, 0x2c,
	0x45, 0x80, 0xDf, 0xA4, 0x21, 0x00, 0x00, 0x1e, 0x8C, 0x0A, 0xd0, 0x8A, 0x20, 0xe0, 0x2d, 0x10,
	0x10, 0x3e, 0x96, 0x00, 0xdf, 0xa4, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x53,
	0x4f, 0x4e, 0x59, 0x20, 0x54, 0x56, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
	0x00, 0x30, 0x3e, 0x0e, 0x46, 0x0f, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xda,
	0x02, 0x03, 0x22, 0x77, 0x4e, 0x14, 0x03, 0x04, 0x12, 0x13, 0x05, 0x01, 0x07, 0x16, 0x9f, 0x10,
	0x15, 0x11, 0x06, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x66, 0x03, 0x0c, 0x00, 0x10,
	0x00, 0x80, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0xdf, 0xa4,
	0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0c, 0x40, 0x55, 0x00,
	0xdf, 0xa4, 0x21, 0x00, 0x00, 0x18, 0x01, 0x1d, 0x00, 0xbc, 0x52, 0xD0, 0x1E, 0x20, 0xb8, 0x28,
	0x55, 0x40, 0xdf, 0xa4, 0x21, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
	0x58, 0x2c, 0x25, 0x00, 0xdf, 0xa4, 0x21, 0x00, 0x00, 0x9e, 0x01, 0x1d, 0x80, 0xd0, 0x72, 0x1c,
	0x16, 0x20, 0x10, 0x2c, 0x25, 0x80, 0xdf, 0xa4, 0x21, 0x00, 0x00, 0x9e, 0x00, 0x00, 0x00, 0x49
};

BYTE TV_EDID[] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x36, 0x8B, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x0F, 0x01, 0x03, 0x80, 0x3C, 0x22, 0x78, 0x0A, 0x0D, 0xC9, 0xA0, 0x57, 0x47, 0x98, 0x27,
	0x12, 0x48, 0x4C, 0xBF, 0xEF, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28,
	0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
	0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x4D,
	0x54, 0x4B, 0x20, 0x4C, 0x43, 0x44, 0x54, 0x56, 0x0A, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x31, 0x4C, 0x0F, 0x50, 0x0E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x56,
	0x02, 0x03, 0x23, 0x74, 0x4B, 0x84, 0x10, 0x1F, 0x05, 0x13, 0x14, 0x01, 0x02, 0x11, 0x06, 0x15,
	0x26, 0x09, 0x7F, 0x03, 0x11, 0x7F, 0x18, 0x83, 0x01, 0x00, 0x00, 0x67, 0x03, 0x0C, 0x00, 0x10,
	0x00, 0xB8, 0x2D, 0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0xC4,
	0x8E, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10, 0x2C, 0x25,
	0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
	0x3E, 0x96, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31,
	0x20, 0x0C, 0x40, 0x55, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52

};



UINT8 SinkDDCReadData(UINT8 uDevID, UINT8 uOffset)
{
	UINT8 bGetData = 0;
	/* HDCP */
	UINT8 u1Bksv[5];

	/* get BKSV, */
	HDMI_HalGetBksv(u1Bksv);


	if (CBUS_DDC_DATA_HDCP_ADRR == uDevID) {
		if (uOffset < CBUS_DDC_DATA_HDCP_RSVD1) {
			if (CBUS_DDC_DATA_HDCP_BKSV_OFFSET == uOffset) {
				bGetData = u1Bksv[0];
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Bksv1:0x%x\n",bGetData); */
			} else if ((CBUS_DDC_DATA_HDCP_BKSV_OFFSET + 1) == uOffset) {
				bGetData = u1Bksv[1];
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Bksv2:0x%x\n",bGetData); */
			} else if ((CBUS_DDC_DATA_HDCP_BKSV_OFFSET + 2) == uOffset) {
				bGetData = u1Bksv[2];
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Bksv3:0x%x\n",bGetData); */
			} else if ((CBUS_DDC_DATA_HDCP_BKSV_OFFSET + 3) == uOffset) {
				bGetData = u1Bksv[3];
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Bksv4:0x%x\n",bGetData); */
			} else if ((CBUS_DDC_DATA_HDCP_BKSV_OFFSET + 4) == uOffset) {
				bGetData = u1Bksv[4];
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Bksv5:0x%x\n",bGetData); */
			} else {
				/* HDMI_LOG(HDMI_LOG_ERROR,"get hdcp Bksv error, null pointer\n"); */
			}
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RSVD1) && (uOffset < CBUS_DDC_DATA_HDCP_RI1_OFFSET)) {
			/* HDMI_LOG(HDMI_LOG_WARN,"RSVD1!\n"); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RI1_OFFSET) && (uOffset < CBUS_DDC_DATA_HDCP_PJ)) {
			if (CBUS_DDC_DATA_HDCP_RI1_OFFSET == uOffset) {
				bGetData = (HDMI_HalGetRi() >> 8) & 0xFF;
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Ri2:0x%x\n",bGetData); */

			} else if ((CBUS_DDC_DATA_HDCP_RI1_OFFSET + 1) == uOffset) {
				bGetData = (HDMI_HalGetRi() >> 0) & 0xFF;
				/* HDMI_LOG(HDMI_LOG_DEBUG,"Ri1:0x%x\n",bGetData); */
			} else {
				/* HDMI_LOG(HDMI_LOG_DEBUG,"get hdcp Ri error, null pointer\n"); */
			}
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_PJ) && (uOffset < CBUS_DDC_DATA_HDCP_RSVD2)) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"PJ!\n"); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RSVD2) && (uOffset < CBUS_DDC_DATA_HDCP_AKSV_OFFSET)) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"RSVD2!\n"); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RSVD3) && (uOffset < CBUS_DDC_DATA_HDCP_AN_OFFSET)) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"RSVD3!\n"); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_VH0) && (uOffset < CBUS_DDC_DATA_HDCP_VH1)) {
			/* VHx for repeater */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_VH1) && (uOffset < CBUS_DDC_DATA_HDCP_VH2)) {
			/* VHx is for Repeater */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_VH2) && (uOffset < CBUS_DDC_DATA_HDCP_VH3)) {
				/**/
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_VH3) && (uOffset < CBUS_DDC_DATA_HDCP_VH4)) {
				/**/
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_VH4) && (uOffset < CBUS_DDC_DATA_HDCP_RSVD4)) {
				/**/
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RSVD4) && (uOffset < CBUS_DDC_DATA_HDCP_BCAPS_OFFSET)) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"RSVD4!\n"); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_BCAPS_OFFSET) &&
		(uOffset < CBUS_DDC_DATA_HDCP_BSTATUS_OFFSET)) {
			bGetData = (HDMI_READ32(REG_SHD_BSTATUS) & (BCAPS1 | BCAPS0)) >> 16;
			/* HDMI_LOG(HDMI_LOG_DEBUG,"Bcaps:0x%x\n",bGetData); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_BSTATUS_OFFSET) && (uOffset < CBUS_DDC_DATA_HDCP_KSVFIFO)) {
			if (CBUS_DDC_DATA_HDCP_BSTATUS_OFFSET == uOffset) {
				bGetData = (HDMI_READ32(REG_SHD_BSTATUS) & 0xFF000000) >> 24;
				/* HDMI_LOG(HDMI_LOG_DEBUG,"bStatus1:0x%x ",bGetData); */
			} else if ((CBUS_DDC_DATA_HDCP_BSTATUS_OFFSET + 1) == uOffset) {
				bGetData = (HDMI_READ32(REG_HDCP_STAT) & DEPTH) >> 0;
				/* HDMI_LOG(HDMI_LOG_DEBUG,"bStatus2:0x%x ",bGetData); */
			} else {
				/* HDMI_LOG(HDMI_LOG_DEBUG,"get hdcp bStatus error, null pointer\n"); */
			}
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_KSVFIFO) && (uOffset < CBUS_DDC_DATA_HDCP_KSVFIFO + 50)) {
			bGetData = _RxKsvList[uOffset - CBUS_DDC_DATA_HDCP_KSVFIFO];
			/* DMI_LOG(HDMI_LOG_DEBUG,"KsvFifo:0x%x",bGetData); */
		} else if ((uOffset >= CBUS_DDC_DATA_HDCP_RSVD5) && (uOffset < CBUS_DDC_DATA_HDCP_DBG)) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"RSVD5!\n"); */
		} else if (uOffset >= CBUS_DDC_DATA_HDCP_DBG) {
			/* HDMI_LOG(HDMI_LOG_DEBUG,"Debug!\n"); */
		}

		/* printk("%X:%X\n",uOffset,bGetData); */
	}
	/* EDID */
	else if (CBUS_DDC_DATA_ADRR == uDevID) {

		bGetData = mhl_edid[uOffset];
		/*memcpy(&bGetData, mhl_edid, 1);
		bGetData = TV_EDID[uOffset];
		HDMI_LOG(HDMI_LOG_DEBUG,"EDID[%d]:0x%x Internal?%d = ", \
		uOffset, bGetData, _bInternalEdid); */
	} else {
		/* HDMI_LOG(HDMI_LOG_DEBUG,"The Read deviceID is error\n"); */
	}

	return bGetData;
}


BOOL fgHDMIRxDetectTmds(void)
{

	BYTE bData;

	bData = HDMI_HalGetSCDT();
	return bData;

}

void ModifyBstatusDepth(UINT8 u1Depth)
{

	_fgUseModifiedDepth = TRUE;
	_u1ModifiedDepth = u1Depth;
}

void RecoverBstatusDepth(void)
{

	_fgUseModifiedDepth = FALSE;
	_u1ModifiedDepth = 0;

}


BOOL IsMhlMode(void)
{
	return _bMHLMode;
}

BOOL IsMhlPixelRepeat(void)
{
	return _bPPMode;
}


#define FREQUENCY_10M  (10*1000*1000)
#define FREQUENCY_30M  (30*1000*1000)
#define FREQUENCY_40M  (40*1000*1000)
#define FREQUENCY_50M  (50*1000*1000)
#define FREQUENCY_160M  (160*1000*1000)
#define FREQUENCY_250M  (250*1000*1000)


HDMI_ANA_BAND eBand_pre = HDMI_ANA_BAND_NULL;
HDMI_ANA_BAND eBand = HDMI_ANA_BAND_NULL;

void HdmiSelectAnaBand(BOOL fgForce)
{
	BOOL fgIsMhl = FALSE;
	BOOL fgPixelRepeat = FALSE;
	UINT32 u4TmdsClock = 0;

#if 0

	if (!HDMI_HalGetCKDT()) {
		return;
	}

#endif

	/* set  band */
	fgIsMhl = IsMhlMode();
	fgPixelRepeat = IsMhlPixelRepeat();
	u4TmdsClock = HDMI_HalGetTmdsClockExt();

	/*  mhl */
	if (fgIsMhl) {
		if (fgPixelRepeat) { /*  pixel repeat mode */
			if (u4TmdsClock < FREQUENCY_30M) {
				eBand = MHL_ANA_BAND_PP_0_30M;
			} else {
				eBand = MHL_ANA_BAND_PP_30_MAX;
			}
		} else {       /*  normal mode */
			if (u4TmdsClock < FREQUENCY_50M) {
				eBand = MHL_ANA_BAND_0_50M;
			} else {
				eBand = MHL_ANA_BAND_50_MAX;
			}
		}
	}
	/*  hdmi */
	else {
		if (u4TmdsClock < FREQUENCY_10M) {
			eBand = HDMI_ANA_BAND_NULL;
		} else if (u4TmdsClock < FREQUENCY_30M) {
			eBand = HDMI_ANA_BAND_10_27M;
		} else if (u4TmdsClock < FREQUENCY_40M) {
			eBand = HDMI_ANA_BAND_27_40M;
		} else if (u4TmdsClock < FREQUENCY_160M) {
			eBand = HDMI_ANA_BAND_40_160M;
		} else if (u4TmdsClock < FREQUENCY_250M) {
			eBand = HDMI_ANA_BAND_160_250M;
		} else {
			eBand = HDMI_ANA_BAND_250_MAX;
		}
	}

	if ((eBand != eBand_pre) || fgForce) {
		HDMI_HalSelAnaBandExt(eBand);
		HDMI_HalEqCalibrate();
		/* HDMI_LOG(HDMI_LOG_INFO, "Select analog band (%d) -> (%d), fgForce: %d\r\n", \
		eBand, eBand, fgForce); */
	}

	eBand_pre = eBand;
}

void HdmiResetAnaBand(void)
{
	eBand = HDMI_ANA_BAND_NULL;
	eBand_pre = HDMI_ANA_BAND_NULL;
}


#define CHANNEL_UNSTABLE_COUNT  (5)

void MHLChannelAdjust(void)
{
	static UINT32  u4ResDigPhy;
	UINT8 u1ChannelOrder = 0;
	BOOL fgCKDT = FALSE;

	fgCKDT = HDMI_HalGetCKDT();

	if (!IsMhlMode() || (!fgCKDT)) {
		u4ResDigPhy = 0;
		return;
	}

	if (MHLStable()) {
		u4ResDigPhy = 0;
	} else {
		u4ResDigPhy++;
	}

	if (u4ResDigPhy > CHANNEL_UNSTABLE_COUNT) { /*  20*15ms = 300ms */
		if (IsMhlPixelRepeat()) { /*  pixel repeate mode */
			u1ChannelOrder = MHLGetChannelOrder();

			switch (u1ChannelOrder) {
			case 0x0:
			case 0x1:
				MHLSetChannelOrder(0x2);
				/* HDMI_LOG(HDMI_LOG_INFO, "MHLChannelAdjust order = 2 \r\n"); */
				break;

			case 0x2:
			case 0x3:
				MHLSetChannelOrder(0x1);
				/* HDMI_LOG(HDMI_LOG_INFO, "MHLChannelAdjust order = 1 \r\n"); */
				break;
			}
		} else {
			u1ChannelOrder = MHLGetChannelOrder();

			switch (u1ChannelOrder) {
			case 0x0:
			case 0x1:
				MHLSetChannelOrder(0x2);
				/* HDMI_LOG(HDMI_LOG_INFO, "MHLChannelAdjust order = 2 \r\n"); */
				break;

			case 0x2:
				MHLSetChannelOrder(0x3);
				/* HDMI_LOG(HDMI_LOG_INFO, "MHLChannelAdjust order = 3 \r\n"); */
				break;

			case 0x3:
				MHLSetChannelOrder(0x1);
				/* HDMI_LOG(HDMI_LOG_INFO, "MHLChannelAdjust order = 1 \r\n"); */
				break;
			}

		}

		u4ResDigPhy = 0;   /*  reset count */
	}
}







#ifdef MT3363_HDMI_EMU
void HdmiWriteReg_emu(UINT32 offset, UINT32 value)
{
	/*if (gHdmiReg_emu != 0) {gHdmiReg_emu is array, is not 0*/
		HDMI_WRITE32(offset, value);
	/*}*/

	/* HDMI_LOG(HDMI_LOG_DEBUG, "write [0x%x] = 0x%x\r\n", offset, value); */
}

UINT32 HdmiReadReg_emu(UINT32 offset)
{
	UINT32 u4Value = 0;

	/*if (gHdmiReg_emu != 0) { gHdmiReg_emu is array, is not 0*/
		u4Value = HDMI_READ32(offset);
	/*}*/

	/*  HDMI_LOG(HDMI_LOG_DEBUG, "read [0x%x] = 0x%x  \r\n", offset, u4Value); */
	return u4Value;
}


void HdmiDumpReg_emu(void)
{
	int i = 0;
	UINT32 u4Value = 0;

	/* HDMI_LOG(HDMI_LOG_DEBUG, "\r\nHdmiDumpReg_emu: "); */
	/*if (gHdmiReg_emu != 0) {gHdmiReg_emu is array, is not 0*/
		for (i = 0; i < 0x300; i += 4) {
			if (i % 16 == 0)
				/* HDMI_LOG(HDMI_LOG_DEBUG, "\r\n"); */

			{
				u4Value = HDMI_READ32(i);
			}

			/* HDMI_LOG(HDMI_LOG_DEBUG, "0x%x ", u4Value); */

		}
	/*}*/
}

#endif





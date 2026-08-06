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

#ifndef _HDMI_RX_TASK_H_
#define _HDMI_RX_TASK_H_

#include "x_debug.h"
#include "x_os.h"
/*#include "x_hal_ic.h"*/
#include "mhl_drv_if.h"

/******************************************************************************
* Hdmi Task thread configuration
******************************************************************************/

#define HDMI_RX_TASK_CMD_Q_NAME                 "HDMIRxCmd"
#define HDMI_RX_EVENT_NAME                      "HDMIRxEvent"

#define SRV_OK       (INT32)(0)
#define SRV_FAIL     (INT32)(-1)

typedef enum {
	HDMI_RX_SERVICE_CMD = (EV_GRP_EVENT_T)(0x1 << 0),
	HDMI_DISABLE_HDMI_RX_TASK_CMD = (EV_GRP_EVENT_T)(0x1 << 1),
	HDMI_RX_EG_WAKEUP_THREAD = (EV_GRP_EVENT_T)(HDMI_RX_SERVICE_CMD | HDMI_DISABLE_HDMI_RX_TASK_CMD)
} HDMI_TASK_COMMAND_TYPE_T;


typedef enum {
	RX_DETECT_STATE = 0,
	RX_CHANGE_EDID_STATE = 1,
	RX_IDLE_STATE = 20,

} HDMI_RX_STATE_TYPE;



#define HDMI_RX_TIMER_5MS 1  /*timer is 5 ticket*/
#define HDMI_RX_TIMER_10MS 2
#define HDMI_RX_TIMER_15MS 3
#define HDMI_RX_TIMER_20MS 4
#define HDMI_RX_TIMER_100MS 20
#define HDMI_RX_TIMER_200MS 40
#define HDMI_RX_TIMER_400MS 80
#define HDMI_RX_TIMER_1S 200



/*HDMI_RX_DEBUG_EDID | HDMI_RX_DEBUG_HOT_PLUG;//| HDMI_RX_DEBUG_INFOFRAME; */

void vChageRxSysState(UINT8 u1State);
extern BOOL _bMHLModeBackup;
extern BOOL _bMHLMode;
extern BOOL _bPPModeBackup;
extern BOOL _bPPMode;
extern UINT8   _bInternalEdid; /*  0 - EDID, 1 - InternalEDID */
extern UINT8 _u1TxEdidReadyOld;
/* extern bool bApSclerMasterModeEn; */
extern void PmxVerifySetVdoSrcFmt(UCHAR ucVdoId, UCHAR ucSrcFmt);
extern void PmxVerifySetMode(UCHAR ucVdoId, UINT32 u4SrcFmt, UINT32 u4PmxFmt,
	UCHAR ucSrcType, UCHAR ucTvType, UCHAR ucInterlace);
extern void ac83xx_mask_ack_bim_irq(uint32_t irq);
extern void mt33xx_mask_ack_bim_irq(uint32_t irq);
/* extern BOOL RegHDMIAudFunc(HDMI_REG_AUD_F *pRegFun); */
/*extern int gpio_request(unsigned gpio, const char *label);*/
extern struct device *hdmi_dev;
extern BOOL fgCrcFail;
extern BOOL is_sink_attached;
extern BOOL _bPPMode;
/* extern MHL_VIDEO_INFO_T g_rVideoInfo; */
/* extern    UINT8 _au1HDCP[]; */
/* extern UINT8 bInitHDCP; */
extern void vChageRxSysState(UINT8 u1State);
extern void Set640x480PEnable(BYTE bType);
extern BOOL fgIsHdmiRxBoardExist(void);
extern void _HDMI_RX_TmrIsr(void);
extern void PmxVerifyWriteChnEntry_Hdmi(UCHAR ucVdoId, UCHAR ucOn, UINT32 width,
	UINT32 height, UINT32 u4DataType, UINT32 u4Interlace, UINT32 u4MuxSelect,
	UINT32 u4WrchIndex, x_os_isr_fct pf_isr);
INT32 HDMI_RX_Init(void);
UINT32 HDMI_RX_Config(MHL_DRV_CONFIG_T rMhlConfig);
UINT32 HDMI_RX_Start(void);
UINT32 HDMI_RX_Stop(void);
MHL_DEVICE_TYPE_T HDMI_RX_GetDeviceType(void);
UINT32 HDMI_RX_GetSignalStatus(void);
UINT32 HDMI_RX_GetClockStable(BOOL *pStable);
UINT32 HDMI_RX_GetHsyncStable(BOOL *pStable);
VOID HDMI_PrintInfo(void);
UINT32 HDMI_RX_GetVideoInfo(MHL_VIDEO_INFO_T *pVideoInfo);
UINT32 HDMI_RX_SendRcpKey(UINT32 u4Key);
#endif

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

#include "mhl_mod.h"

#ifndef _HDMI_RX_DVI_H_
#define _HDMI_RX_DVI_H_

#define Diff(a, b)  (((a) > (b))?((a)-(b)):((b)-(a)))
#define RANGE_CHECKING(a, b, offset)  ((UINT32)((a)+(offset)-(b)) <= ((offset)*2))
#define CCIR_decode_444  0

UINT8 GetRxCapturedTiming(void);
void DviModeDetect(void);

void HDMIPowerDetextExt(void); /* mtk68528 */
void vDviModeDetectExt(void); /* mtk68528 */
void DviChkModeChange(void);
BOOL CheckRxDetectDone(void);
UINT32 HdmiGetTimingID(void); /* mtk68528 */
UINT32 DviStdTimingSearch(UINT8 bMode, UINT16   wDviHclk, UINT8 bDviVclk, UINT16 wDviHtotal, UINT16 wHfp);
void HdmiRxDviStatus(void);
void ShowRxResoInfoStatus(void);


irqreturn_t HdmiRxIrqHandler(UINT32 u2Vector, void *dev_id);
void DviInitial(void);


extern UINT8 _bHDMIScanInfo;
extern UINT8 _bHDMIAspectRatio;
extern UINT8 _bHDMIAFD;
extern UINT8 _bHDMIHDCPStatus;
extern UINT8 _bHDMI422Input;
extern UINT8 _bHDMIITCFlag;
extern UINT8 _bHDMIITCContent;
extern UINT32 _wDVI_WAIT_STABLE_COUNT;
extern UINT32 _wDVI_WAIT_NOSIGNAL_COUNT;
extern UINT8 _bHDMIColorSpace;
extern UINT8 _bUnplugFlag;
extern HANDLE g_hEvent_Timing;
extern UINT32 (*_u4pfHDMIRX_SetNfy)(void *pt_nfy_info);
extern UINT32 HDMI_HalGetVFrontPorch(void);
/* extern UINT32 uStartParam ; */
/* extern wait_queue_head_t wchqueue; */
/* extern    VOID WchHalStart(UINT8 u1WchId); */
/* extern  VOID WchHalDeinit(UINT8 u1WchId); */
extern void ac83xx_mask_ack_bim_irq(uint32_t irq);
extern void mt33xx_mask_ack_bim_irq(uint32_t irq);
extern HANDLE g_hEvent_Power;
extern HANDLE g_hEvent_Timing;
extern MHL_DRV_CONFIG_T g_rMhlConfig;
extern BOOL fgResume;
extern BOOL fgNotifySignal;
extern UINT32 u4StableCount;
extern UINT32 HDMI_HalGetVBackPorch(void);
/* extern BOOL sink_fg_phone_support_hdcp(); */
extern void HDMIEnable(BOOL fgEnable);
extern void Linux_HAL_GetTime(unsigned long *prTime);
extern  BOOL Linux_HAL_GetDeltaTime(unsigned long *u4OverTime,
	unsigned long *prStartT, unsigned long *prCurrentT);

extern atc_hdmi_isr_data isr_data;
extern bool register_isr;
extern UINT32 g_u4HdmiState;
extern UINT32 g_u4HdcpStableCnt;




#endif

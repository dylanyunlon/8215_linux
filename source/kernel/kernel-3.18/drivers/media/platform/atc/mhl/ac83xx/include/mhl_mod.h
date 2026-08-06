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

#ifndef _MHL_MOD_H_
#define _MHL_MOD_H_

#include "hdmi_rx_task.h"

extern BOOL   HDMICRC(INT16 ntry);
extern UINT32 HDMI_RX_Config(MHL_DRV_CONFIG_T rMhlConfig);
extern UINT32 HDMI_RX_GetVideoInfo(MHL_VIDEO_INFO_T *pVideoInfo);
extern MHL_DEVICE_TYPE_T HDMI_RX_GetDeviceType(void);
extern UINT32 HDMI_RX_GetSignalStatus(void);
extern UINT32 HDMI_RX_Start(void);
extern UINT32 HDMI_RX_Stop(void);
extern VOID   HDMI_PrintInfo(void);
extern BOOL   GetSourceVidInfo(MHL_VIDEO_INFO_T *pmhlvidInfo);
extern VOID   HDMI_RX_LoadHDCPKeyToSRAM(HDCP_KEY_ST *hk);
extern VOID   HDMI_RX_LoadHDCPKeyToSRAM2(HDCP_KEY_ST *hk);
extern BOOL   _bPPMode;
/* extern void HDMI_HalHwInit(void); */
extern void vMHLCbusHwInit(void);
extern void HDMI_HalLoadHdcp2Sram(UINT8 *prHdcpKey);
extern  UINT8 hdcp_key_to_sram[292];
extern void HdmiRxLoadEdidTable(void);
extern void  HDMIInterRxInit(void); 
extern void  HDMI_HwInit(void);

#ifdef HDMI_BURN_IN
extern BOOL HdmiIsHVStable(void);
extern BOOL HdmiIsPclkStable(void);
extern int enable_output;
#endif
extern void HalHdmiRxSetApll(void);
extern void HdmiRxLoadHdcpKey(void);

extern int mhl_open(struct inode *inode, struct file *file);
extern int mhl_release(struct inode *inode, struct file *filp);
/*extern long mhl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);*/

extern int __init mhl_init(void);

typedef struct _atc_hdmi_isr_data {
	atc_hdmi_isr_t	isr;
	void			*arg;
} atc_hdmi_isr_data;

#endif

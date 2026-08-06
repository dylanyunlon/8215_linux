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

#ifndef _MHL_RX_CBUS_CTRL_H_
#define _MHL_RX_CBUS_CTRL_H_
/* #if DRV_SUPPORT_MHL_RX */
#define RCPKEY_MAX 0x7F
typedef enum {
	SINK_CBUS_LINK_UNATTACHED = 0,
	SINK_CBUS_LINK_ATTACHED,
	SINK_CBUS_LINK_STATE_CONNECTED,
	SINK_CBUS_LINK_STATE_CONTENT_ON,
	SINK_CBUS_LINK_POWER_OFF,
	SINK_CBUS_LINK_FLOAT_CBUS,
	SINK_CBUS_LINK_NONE,
} SINK_CBUS_LINK_STATE;

typedef struct {
	BOOL fgHPDOn;
	BOOL fgHPDChanged;
} SINK_HPD_ST;
typedef struct {
	BOOL fgPATHOn;
	BOOL fgPATHChanged;
} SINK_PATH_ST;
typedef struct {
	BOOL fgEDIDReady;
	BOOL fgEDIDChanged;
} SINK_EDID_ST;
typedef struct {
	BOOL fgDCAPReady;
	BOOL fgDCAPChanged;
} SINK_DCAP_ST;
typedef enum {
	SINK_3DINFO_REQ_STATE_IDLE = 0,
	SINK_3DINFO_REQ_STATE_REQ,
	SINK_3DINFO_REQ_STATE_GRT,
} SINK_3DINFO_STATE;
typedef struct {
	SINK_3DINFO_STATE state;
} SINK_3DINFO_REQ_ST;
typedef enum {
	SINK_DSCR_REQ_STATE_IDLE = 0,
	SINK_DSCR_REQ_STATE_REQ,
	SINK_DSCR_REQ_STATE_GRT,
	SINK_DSCR_REQ_STATE_CHG,
} SINK_DSCR_STATE;
typedef struct {
	SINK_DSCR_STATE state;
} SINK_DSCR_REQ_ST;
typedef enum {
	SINK_BURST_TYPE_NONE = 0,
	SINK_BURST_TYPE_DSCR,
	SINK_BURST_TYPE_3DINFO,
} SINK_BURST_TYPE;
typedef struct {
	SINK_BURST_TYPE type;
	SINK_3DINFO_REQ_ST st_3dinfo;
	SINK_DSCR_REQ_ST st_dscr;
} SINK_BURST_REQ_ST;
typedef struct {
	BOOL fgClkModeReady;
	BOOL fgClkModeChanged;
} SINK_CLKMODE_ST;
typedef struct {
	BOOL fgIsValid;
	unsigned short u2Code;
	unsigned char u1Val;
}  SINK_RCP_ST;
typedef struct {
/* device register */
	BOOL fgIsHPD;
	BOOL fgIsHPDBak;
	BOOL fgIsRense;
	BOOL fgIsRAP;
	/* 020 */
	BOOL fgIsDCapChg;
	BOOL fgIsDScrChg;
	BOOL fgIsReqWrt;
	BOOL fgIsGrtWrt;
	BOOL fgIs3DReq;
	/* 0x21 */
	BOOL fgIsEdidChg;
	/* 0x30 */
	BOOL fgIsDCapRdy;
	/* 0x31 */
	BOOL fgIsPPMode;
	BOOL fgIsPathEn;
	BOOL fgIsMuted;
	/* for me */
	BOOL fgMyIsPPModeChg;
	BOOL fgMyIsPathEnChg;
	BOOL fgMyIsDcapChg;
	BOOL fgMyIsMutedChg;
	BOOL fgMyIsPOWChg;

/* event */
	SINK_HPD_ST st_hpd;
	SINK_PATH_ST st_path;
	SINK_EDID_ST st_edid;
	SINK_DCAP_ST st_dcap;
	SINK_BURST_REQ_ST st_burst;
	SINK_CLKMODE_ST st_clkmode;
	SINK_RCP_ST st_rcp;
} SINK_DEVICE_CONTROL_ST;

typedef struct {
	unsigned short test_mode;
	unsigned char counter;
}  SINK_TEST_MODE;

typedef struct {
	char phoneName[32];
	BOOL fgHdcp;
	unsigned int devCap[16];
} MHL_PHONE_CAP;


extern SINK_DEVICE_CONTROL_ST st_dev_ctrl;
extern unsigned int sink_100k_r;
/* extern void u1SinkGetEfuseConfig(void);*/
/* extern void u1TestGetEfuseConfig(unsigned char addr);*/


BOOL fgIsSinkCbusConnected(void);
void vSinkSetAttachStatus(BOOL en);
void vSetSinkCbusStuchLow(void);
void vSetSinkWakeupErr(void);
void vSetSinkDiscoveryOk(void);
void vMhlIntProcess(void);
void vSinkLinkState(void);
void vSinkCommandState(void);
void vSinkCbusTimer(void);
void sink_cbus_cmd(unsigned int cmd, unsigned int arg0, unsigned int arg1, unsigned int arg2);
int sink_get_keycode_from_ir(UINT32 keycode);
void sink_discovery_ok(void);
void sink_cbus_low_err(void);
void vTrigRCPMsg(unsigned int key);
void sink_delay_set_hpd(void);
BOOL fgSinkAttachSource(void);
BOOL fgSinkDiscoveryOk(void);
BOOL sink_fg_phone_support_hdcp(void);



#endif

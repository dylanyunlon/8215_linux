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

#ifndef _MHL_CBUS_H_
#define _MHL_CBUS_H_
/* #if DRV_SUPPORT_MHL_RX */

/************************************************

     device register space define

*************************************************/
#define SINK_CBUS_MHL_VERSION                           0x01
#define SINK_CBUS_MSC_DEV_CAT                           0x02
#define     SINK_CBUS_MSC_DEV_CAT_POW                   0x10
#define SINK_CBUS_MSC_DEV_CAT_DEV_TYPE              0x0F

#define SINK_CBUS_ADOPTER_ID_H              0x03
#define SINK_CBUS_ADOPTER_ID_L              0x04
#define SINK_CBUS_VID_LINK_MODE             0x05
#define SINK_CBUS_SUPP_RGB444           0x01
#define SINK_CBUS_SUPP_YCBCR444     0x02
#define SINK_CBUS_SUPP_YCBCR422     0x04
#define SINK_CBUS_SUPP_PP               0x08
#define SINK_CBUS_SUPP_ISLANDS          0x10
#define SINK_CBUS_SUPP_VGA              0x20
#define SINK_CBUS_AUD_LINK_MODE         0x06
#define SINK_CBUS_SUPP_AUD_2CH          0x01
#define SINK_CBUS_SUPP_AUD_8CH          0x02
#define SINK_CBUS_VIDEO_TYPE                0x07
#define SINK_CBUS_VT_GRAPHICS                   0x01
#define SINK_CBUS_VT_PHOTO                  0x02
#define SINK_CBUS_VT_CINEMA                 0x04
#define SINK_CBUS_VT_GAME                       0x08
#define SINK_CBUS_VT_SUPP                       0x80
#define SINK_CBUS_LOG_DEV_MAP                                   0x08
#define    SINK_CBUS_LOG_DEV_MAP_LD_DISPLAY                    0x01
#define    SINK_CBUS_LOG_DEV_MAP_LD_VIDEO                      0x02
#define    SINK_CBUS_LOG_DEV_MAP_LD_AUDIO                      0x04
#define    SINK_CBUS_LOG_DEV_MAP_LD_MEDIA                      0x08
#define    SINK_CBUS_LOG_DEV_MAP_LD_TUNER                      0x10
#define    SINK_CBUS_LOG_DEV_MAP_LD_RECORD                 0x20
#define    SINK_CBUS_LOG_DEV_MAP_LD_SPEAKER                    0x40
#define    SINK_CBUS_LOG_DEV_MAP_LD_GUI                            0x80
#define SINK_CBUS_BANDWIDTH     0x09
#define SINK_CBUS_FEATURE_FLAG                                      0x0A
#define    SINK_CBUS_FEATURE_FLAG_RCP_SUPPORT                      0x01
#define    SINK_CBUS_FEATURE_FLAG_RAP_SUPPORT                      0x02
#define    SINK_CBUS_FEATURE_FLAG_SP_SUPPORT                       0x04
#define    SINK_CBUS_FEATURE_UCP_SEND_SUPPORT                      0x08
#define    SINK_CBUS_FEATURE_UCP_RECV_SUPPORT                      0x10

#define SINK_CBUS_DEVICE_ID_H                   0x0B
#define SINK_CBUS_DEVICE_ID_L                   0x0C

#define SINK_CBUS_SCRATCHPAD_SIZE               0x0D

#define SINK_CBUS_INT_STAT_SIZE                 0x0E

#define SINK_CBUS_MSC_RCHANGE_INT                                           0x20
#define     SINK_CBUS_MSC_RCHANGE_INT_DCAP_CHG                              0x01
#define     SINK_CBUS_MSC_RCHANGE_INT_DSCR_CHG                              0x02
#define     SINK_CBUS_MSC_RCHANGE_INT_REQ_WRT                               0x04
#define     SINK_CBUS_MSC_RCHANGE_INT_GRT_WRT                               0x08
#define     SINK_CBUS_MSC_RCHANGE_INT_3D_REQ                                0x10

#define SINK_CBUS_MSC_DCHANGE_INT                                           0x21
#define     SINK_CBUS_MSC_DCHANGE_INT_EDID_CHG                              0x02

#define SINK_CBUS_MSC_STATUS_CONNECTED_RDY                              0x30
#define     SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY                 0x01

#define SINK_CBUS_MSC_STATUS_LINK_MODE                                  0x31
#define     SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE                     0x07
#define     SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__Normal             0x03
#define     SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__PacketPixel            0x02
#define     SINK_CBUS_MSC_STATUS_LINK_MODE_PATH_EN                          0x08
#define     SINK_CBUS_MSC_STATUS_IS_MUTED                                   0x10

#define SINK_CBUS_MSC_SCRATCHPAD                0x40

/*********************************************************************/
#define     SINK_CBUS_DEVICE_LENGTH 0x50

#define SINK_CBUS_MSC_ACK 0x533
#define SINK_CBUS_MSC_NACK 0x534
#define SINK_CBUS_MSC_ABORT 0x535
#define SINK_CBUS_MSC_WRITE_STATE 0x560/* type 3, source should send 3 packet,need ack */
#define SINK_CBUS_MSC_SET_INT 0x560/* type 3, source should send 3 packet,need ack */
#define SINK_CBUS_MSC_READ_DEVCAP 0x561/* type 1, source should send 2 packet,need ack, and 1 value */
#define SINK_CBUS_MSC_GET_STATE 0x562/* type 0, no ack , only value */
#define SINK_CBUS_MSC_GET_VENDER_ID 0x563/* type 0, no ack , only value */
#define SINK_CBUS_MSC_SET_HPD 0x564/* type 0, no ack , only value */
#define SINK_CBUS_MSC_CLR_HPD 0x565/* type 0, no ack , only value */
#define SINK_CBUS_MSC_MSC_MSG 0x568/* type 3, source should send 3 packet,need ack, */
/* type 2, source should send 3 packet,need ack,and 3 value */
#define SINK_CBUS_MSC_GET_SC1_EC 0x569/* type 0, no ack , only value */
#define SINK_CBUS_MSC_GET_DDC_EC 0x56A/* type 0, no ack , only value */
#define SINK_CBUS_MSC_GET_MSC_EC 0x56B/* type 0, no ack , only value */
#define SINK_CBUS_MSC_WRITE_BURST 0x56C
#define SINK_CBUS_MSC_GET_SC3_EC 0x56D/* type 0, no ack , only value */
#define SINK_CBUS_MSC_EOF 0x532

enum {
	SINK_MSC_MSG_MSGE             = 0x402,     /*  RCP sub-command */
	SINK_MSC_MSG_RCP             = 0x410,     /*  RCP sub-command */
	SINK_MSC_MSG_RCPK            = 0x411,     /*  RCP Acknowledge sub-command */
	SINK_MSC_MSG_RCPE            = 0x412,     /*  RCP Error sub-command */
	SINK_MSC_MSG_RAP             = 0x420,     /*  RAP sub-command */
	SINK_MSC_MSG_RAPK            = 0x421,     /*  RAP Acknowledge sub-command */
	SINK_MSC_MSG_UCP             = 0x430,
	SINK_MSC_MSG_UCPK            = 0x431,
	SINK_MSC_MSG_UCPE            = 0x432,
};
/************************************************

************************************************/
#define MHL_LINK_TIME (10)
#define SINK_MSCDDC_TMR_OUT_200MS (200/MHL_LINK_TIME) /* 90ms timer out */
#define SINK_MSCDDC_ERR_2S (2100/MHL_LINK_TIME)
#define MHL_TXOK_TMR_OUT_20MS (20/MHL_LINK_TIME) /* 10ms */
#define MHL_TXOK_TMR_OUT_40MS (40/MHL_LINK_TIME) /* 10ms */
#define MHL_TXOK_TMR_OUT_60MS (60/MHL_LINK_TIME) /* 10ms */

/************************************************
    DDC cmd
************************************************/


#define SINK_CBUS_DDC_READ 0x01
#define SINK_CBUS_DDC_SEG_READ 0x02
#define SINK_CBUS_DDC_SHORT_READ 0x03
#define SINK_CBUS_DDC_CURRENT_READ 0x04
#define SINK_CBUS_DDC_WRITR 0x05

#define NONE_PACKET 0xffff
#define NULL_DATA 0xffff

enum { /* msc command reactions */
	SINK_MSC_BAD_OFFSET = 0x10,/* bad offset, send abort */
	SINK_MSC_BAD_OPCODE   = 0x08,/* bad opcode, send abort */
	SINK_MSC_MSG_TIMEOUT = 0x04,/* SINK_MSC_TOO_FEW_PKT  = 0x04,//time out */
	/* SINK_MSC_INCOMPLETE_PKT = 0x04,//time out */
	SINK_MSC_DEV_BUSY = 0x20,/* nack */
	SINK_MSC_PROTOCOL_ERR = 0x02,/* abort */
	SINK_MSC_RETRY_EXCEED = 0x01,/* time out */
};

#define SINK_CBUS_STATE_IDLE            0
#define SINK_CBUS_STATE_RESPONDER   1
#define SINK_CBUS_STATE_REQUESTER       2
#define SINK_CBUS_STATE_DISCONNET       3

#define SINK_CBUS_STATE_S0  0
#define SINK_CBUS_STATE_S1  1
#define SINK_CBUS_STATE_S2  2
#define SINK_CBUS_STATE_S3  3
#define SINK_CBUS_STATE_S4  4
#define SINK_CBUS_STATE_S5  6
#define SINK_CBUS_STATE_S6  6
#define SINK_CBUS_STATE_S7  7
#define SINK_CBUS_STATE_S8  8
#define SINK_CBUS_STATE_ABORT   100

#define     EDID_BLOCK_LEN      128
#define     EDID_ADDR_EXT_BLOCK_FLAG              0x7E

#define SINK_TX_HW_BUF_MAX 24
#define SINK_RX_HW_BUF_MAX 32

/*******************************************************
     DDC
********************************************************/
#define SINK_CBUS_DDC_CTRL_SOF 0x130
#define SINK_CBUS_DDC_CTRL_EOF 0x132
#define SINK_CBUS_DDC_CTRL_ACK 0x133
#define SINK_CBUS_DDC_CTRL_NACK 0x134
#define SINK_CBUS_DDC_CTRL_ABORT 0x135
#define SINK_CBUS_DDC_CTRL_CONT 0x150
#define SINK_CBUS_DDC_CTRL_STOP 0x151

#define SINK_CBUS_DDC_DATA_SEGW 0x060
#define SINK_CBUS_DDC_DATA_ADRW 0x0A0
#define SINK_CBUS_DDC_DATA_ADRR 0x0A1

#define SINK_CBUS_DDC_DATA_HDCP_ADRW 0x074
#define SINK_CBUS_DDC_DATA_HDCP_ADRR 0x075

#define EDID_BLOCK_LEN      128
#define EDID_SIZE 256
/********************************************************/
/* ddc state */
#define SINK_CBUS_STATE_DDC_IDEL    0
#define SINK_CBUS_STATE_DDC_SOF     1
#define SINK_CBUS_STATE_DDC_ADDR    2

#define SINK_CBUS_STATE_DDC_OFFSET      10
#define SINK_CBUS_STATE_DDC_W_WAIT      11

#define SINK_CBUS_STATE_DDC_CONT                20
#define SINK_CBUS_STATE_DDC_R_WAIT          21

#define SINK_CBUS_STATE_DDC_STOP    30
#define SINK_CBUS_STATE_DDC_EOF     31
#define SINK_CBUS_STATE_DDC_ABORT   32
/********************************************************/

#define SINK_CBUS_DDC_TOO_FEW_ERR           0x04
#define SINK_CBUS_DDC_PROTOCOL_ERR      0x02
#define SINK_CBUS_DDC_INCOMPLETE_ERR        0x04
#define SINK_CBUS_DDC_RETRY_ERR         0x01

#define CBUS_3D_VIC_ID_H    0x00
#define CBUS_3D_VIC_ID_L    0x10
#define CBUS_3D_DTD_ID_H    0x00
#define CBUS_3D_DTD_ID_L    0x11


typedef struct {

} stMhlSinkDev_st;
typedef struct {
	unsigned char u1MhlVersion;
	unsigned char u1DevType;
	BOOL        fgIsPower;
	unsigned char u1PowerCap;
	unsigned int    u4AdopterID;
	unsigned char u1VidLinkMode;
	unsigned char u1AudLinkMode;
	unsigned char u1VideoType;
	unsigned char u1LogDevMap;
	unsigned int    u4BandWidth;
	unsigned char u1FeatureFlag;
	unsigned int    u4DeviceID;
	unsigned char u1ScratchpadSize;
	unsigned char u1intStatSize;
} stMhlSinkDcap_st;

typedef struct {
	unsigned short  u2Cmd;  /* cmd name */
	unsigned short  u2State; /* cmd flow state */
	BOOL    fgOk;
	BOOL    fgFinish;
}  stSinkReq_st;

typedef struct {
	unsigned short  u2Cmd;  /* cmd name */
	unsigned short  u2State; /* cmd flow state */
	unsigned short  u2Step;
	BOOL    fgOk;
}  stSinkDdc_st;

typedef struct {
	unsigned short  u2Cmd;  /* cmd name */
	unsigned short  u2State; /* msc flow */
}  stSinkResp_st;

typedef enum {
	CBUS_TX_IDLE = 0,
	CBUS_TX_DDC_BUSY,
	CBUS_TX_MSC_BUSY
} CBUS_TX_STATE;

typedef struct {
	unsigned short  u2CbusMscMode;
	stSinkReq_st        stReq;
	stSinkResp_st   stResp;
	stSinkDdc_st        stDdc;
	unsigned short  u2RXBuf[SINK_RX_HW_BUF_MAX];
	unsigned short  u2RXBufIndex;
}  stSinkCbus_st;
typedef enum {
	SINK_CBUS_REQ_IDLE             = 0,
	SINK_CBUS_REQ_BEGIN,
	SINK_CBUS_REQ_WAIT,
} SINK_CBUS_REQ_STATE;
typedef struct {
	SINK_CBUS_REQ_STATE             req_state;
	unsigned short  u2ReqBuf[SINK_RX_HW_BUF_MAX];
	unsigned short  u4Len;
}  stSinkCbusRequester_st;
typedef enum {
	SINK_CBUS_TX_IDLE             = 0,
	SINK_CBUS_TX_VALID,
	SINK_CBUS_TX_SEND,
} SINK_CBUS_TX_STATE;
typedef struct {
	SINK_CBUS_TX_STATE  tx_sate; /* buf have data */
	unsigned short  u2TxBuf[SINK_TX_HW_BUF_MAX];
	unsigned short  u2Len;
	unsigned short  u2TxOkRetry;
}  stSinkCbusTxBuf_st;

typedef struct {
	BOOL fgIsValid;
	unsigned short u2Code;
	unsigned char u1Val;
}  stSinkCbusMscMsg_st;

typedef struct {
	unsigned char   u1DevAddrW;  /* cmd name */
	unsigned char   u1DevAddrR;  /* cmd name */
	unsigned char   u1Offset; /* offset addr */
}  stSinkDdcEdidHdcpState_st;

typedef struct {
	BOOL is_hdcp;
	stSinkDdcEdidHdcpState_st       edid; /* offset addr */
	stSinkDdcEdidHdcpState_st   hdcp;
}  stSinkDdcState_st;

/***************************************************/
typedef struct {
	unsigned int u4MscDdcTmr;
	BOOL fgTmrOut;
} st_SinkMscDdcTmrOut_st;
/***************************************************/
typedef enum {
	SINK_CBUS_R_FLOAT             = 0,
	SINK_CBUS_R_1K,
	SINK_CBUS_R_100K,
	SINK_CBUS_R_NONE,
} SINK_CBUS_R_TYPE;


/***************************************************/
extern unsigned char     src_dev_reg[SINK_CBUS_DEVICE_LENGTH];
extern unsigned char     my_dev_reg[SINK_CBUS_DEVICE_LENGTH];

extern st_SinkMscDdcTmrOut_st st_SinkMscTmrOut;
extern st_SinkMscDdcTmrOut_st st_SinkDdcTmrOut;
extern unsigned int sink_link_timer;
extern bool sink_link_timer_reset;
extern bool sink_test_mode_timer_reset;

extern unsigned int u1SinkCbusHw1KStatus;
extern unsigned int u1SinkCbusHwWakeupStatus;
extern unsigned int u1SinkCbusMscAbortDelay2S;
extern unsigned int u1SinkCbusDdcAbortDelay2S;
extern unsigned int sink_cbus_timer_live;
extern unsigned int sink_cbus_int_live;
extern unsigned int sink_test_mode_timer;
extern BOOL fgSinkCbusStop;
extern unsigned char u1ForceSinkCbusStop;
extern BOOL fgSinkCbusFlowStop;
extern stSinkCbusMscMsg_st stSinkCbusMscMsg;
extern stSinkCbusMscMsg_st stSinkCbusDbgWrite;
extern stSinkCbusMscMsg_st stSinkCbusDbgRead;
extern unsigned int sink_cbus_req_delay_by_discovery;
extern void HdmiTimingEventNotify(unsigned int);
extern unsigned int sink_delay_hpd;
extern BOOL is_discovery_ok;
extern BOOL is_sink_cbus_stuch_low;
extern BOOL is_sink_attached;
extern unsigned char sink_efuse_config;
extern bool forceRxsence;
EXTERN BOOL BIM_ClearIrq(UINT32 u4Vector);
extern struct device *hdmi_dev;
extern struct pinctrl *pinctrl_hdmi;

void vSinkCbusInit(void);
void vMHLCbusHwInit(void);
void vSinkCbusReset(void);
void sink_reset_attach(void);

BOOL fgSinkCbusReqGetStateCmd(void);
BOOL fgSinkCbusReqGetVendorIDCmd(void);
BOOL fgSinkCbusReqReadDevCapCmd(unsigned char u1Data);
BOOL fgSinkCbusReqWriteStatCmd(unsigned char u1Offset, unsigned char u1Data);
BOOL fgSinkCbusReqSetIntCmd(unsigned char u1Offset, unsigned char u1Data);
BOOL fgSinkCbusReqWriteBrustCmd(unsigned char *ptData, unsigned char u1Len);
BOOL fgSinkCbusReqMscMsgCmd(unsigned short u1Cmd, unsigned char u1Val);
BOOL fgSinkCbusReqMscERRCodeCmd(void);
BOOL fgSinkCbusReqSetHPDCmd(void);
BOOL fgSinkCbusReqClrHPDCmd(void);

void vMhlSinkIntProcess(void);
void vSinkCbusCmdStatus(void);
BOOL fgSinkCbusMscIdle(void);
BOOL fgSinkCbusDdcIdle(void);
BOOL fgSinkCbusHwTxIdle(void);
BOOL fgSinkCbusHwRxIdle(void);
BOOL fgSinkCbusHwTxRxIdle(void);
void vSinkCbusTimer(void);
void vSinkCbusRxDisable(void);

BOOL fgSinkRxBufEmpty(void);
BOOL fgSinkTxBufEmpty(void);
void vSinkCbusSelectR(SINK_CBUS_R_TYPE type);
void vMhlSinkUsePinTrigForDebug(BOOL fgen);
BOOL fgIsSinkMscMsg(unsigned short u2data);

void vSetSinkCbusBurstReqWaitTmr(unsigned int u2Tmr);
void vClrSinkCbusBurstReqWaitTmr(void);
void vResetSinkCbusDDCStateAll(void);
BOOL sink_fg_phone_support_hdcp(void);

#endif

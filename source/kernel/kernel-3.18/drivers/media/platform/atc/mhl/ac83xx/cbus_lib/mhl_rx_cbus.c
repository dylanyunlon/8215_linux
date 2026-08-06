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

#include "mhl_private.h"
#include "types.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"
#include "drv_thread.h"
#include "x_timer.h"
#include "x_bim.h"
#include "mhl_drv_if.h"
#include "mhl_drv.h"
/*#include "x_bsp.h"*/
/*#include "pm.h"*/
#include "mhl_rx_cbus.h"
#include "mhl_rx_cbus_hw.h"
#include "mhl_rx_dbg.h"
#include <hdmi_debug.h>
#include "mhl_rx_cbus_ctrl.h"
#include "hdmi_rx_ctrl.h"
#include "x_pdwnc.h"
/*#include "drv_ir.h"*/
#include "drv_config.h"
#include "mhl_drv.h"
#include <generated/atc_project.h>

#if 1
/********************************************************/
unsigned int mhl_rx_log_on  = 0x08;/*hdmicbuserr*/
BOOL fgSinkCbusStop = FALSE;
unsigned char u1ForceSinkCbusStop = 0;
BOOL fgSinkCbusFlowStop = FALSE;
/********************************************************/
stMhlSinkDcap_st st_mhl_sink_dcap;

unsigned int u1SinkCbusTxHwStatus = 0;
unsigned int u1SinkCbusRxHwStatus = 0;
unsigned int u1SinkCbusHw1KStatus = 0;
unsigned int u1SinkCbusHwWakeupStatus = 0;

st_SinkMscDdcTmrOut_st st_SinkMscTmrOut;
st_SinkMscDdcTmrOut_st st_SinkDdcTmrOut;
st_SinkMscDdcTmrOut_st sink_cbus_tx_timer;
st_SinkMscDdcTmrOut_st sink_cbus_burst_timer;

unsigned char u1SinkCbusDdcErrCode = 0;
unsigned char u1SinkCbusMscErrCode = 0;
unsigned int u1SinkCbusMscErrCodeDelay2S = 0;
unsigned int u1SinkCbusDdcErrCodeDelay2S = 0;
unsigned int u1SinkCbusMscAbortDelay2S = 0;
unsigned int u1SinkCbusDdcAbortDelay2S = 0;
/**********************************************/
/*for Sink device reg*/
unsigned char   src_dev_reg[SINK_CBUS_DEVICE_LENGTH];
/*for me(Source) device reg*/
unsigned char   my_dev_reg[SINK_CBUS_DEVICE_LENGTH];

unsigned char u1MhlVendorID = 0;
unsigned char u1MhlSinkVendorID = 0;

stSinkCbus_st stSinkCbus;
stSinkCbusRequester_st stSinkCbusRequester;
stSinkCbusMscMsg_st stSinkCbusMscMsg;
stSinkCbusMscMsg_st stSinkCbusDbgWrite;
stSinkCbusMscMsg_st stSinkCbusDbgRead;

stSinkCbusTxBuf_st  sink_cbus_msc_tx;
stSinkCbusTxBuf_st  sink_cbus_ddc_tx;
stSinkCbusTxBuf_st  sink_cbus_tx_ctrl;

stSinkDdcState_st st_sink_ddc;

unsigned int sink_link_timer = 0;
bool sink_link_timer_reset = false;

unsigned int u1SinkCbusHw1KStatus;
unsigned int u1SinkCbusHwWakeupStatus;
unsigned int u1SinkCbusMscAbortDelay2S;
unsigned int u1SinkCbusDdcAbortDelay2S;
/***************************************************/
unsigned int u4SinkCbusWaitTimeOut = 0;
BOOL fgSinkReqWaitFinish = FALSE;
BOOL uDisconInt = FALSE;
extern unsigned long  g_IO_VBASE_VA;

static void vSinkCbusReqFinish(void)
{
	fgSinkReqWaitFinish = TRUE;
}
static void vSinkCbusReqWaitFinish(void)
{
	u4SinkCbusWaitTimeOut = 0;

	while ((fgSinkReqWaitFinish == FALSE) && fgIsSinkCbusConnected()) {
		msleep(2); /*delay 5ms*/

		if (u4SinkCbusWaitTimeOut++ > (500 / 2)) {
			/*SINK_CBUS_LOG("req wait time out\n");*/
			break;
		}
	}
}
static void vSinkCbusTriggerCmpInterrupt(void)
{
	unsigned int tmp;

	tmp = u4SinkReadCbus(REG_CBUS_STA_01) & SINK_WAKEUP_FSM_MASK;
	vSinkWriteCbusMsk(REG_CBUS_LINK_0A, tmp + 1, SINK_MON_CMP_SEL_MASK | SINK_MON_CMP_DATA_MASK);
	vSinkWriteCbusMsk(REG_CBUS_LINK_0A, tmp, SINK_MON_CMP_SEL_MASK | SINK_MON_CMP_DATA_MASK);
}
/****************************************************************/
static BOOL fgSinkCbusTxEvent(void)
{
	if ((u1SinkCbusTxHwStatus &
	     (SINK_TX_OK_INT_MASK | SINK_TX_ARB_FAIL_INT_MASK | SINK_TX_RETRY_TO_INT_MASK | SINK_TX_TRIG_FAIL_INT_MASK))
	    == 0)
		return FALSE;

	return TRUE;
}
static BOOL fgSinkCbusTxOk(void)
{
	if ((u1SinkCbusTxHwStatus &
	     (SINK_TX_OK_INT_MASK | SINK_TX_ARB_FAIL_INT_MASK | SINK_TX_RETRY_TO_INT_MASK | SINK_TX_TRIG_FAIL_INT_MASK))
	    == SINK_TX_OK_INT_MASK)
		return TRUE;

	return FALSE;
}
static BOOL fgSinkCbusTxErr(void)
{
	if ((u1SinkCbusTxHwStatus & (SINK_TX_ARB_FAIL_INT_MASK | SINK_TX_RETRY_TO_INT_MASK)) == 0)
		return FALSE;

	return TRUE;
}
static BOOL fgSinkCbusTriFail(void)
{
	if ((u1SinkCbusTxHwStatus &
	     (SINK_TX_OK_INT_MASK | SINK_TX_ARB_FAIL_INT_MASK | SINK_TX_RETRY_TO_INT_MASK | SINK_TX_TRIG_FAIL_INT_MASK))
	    == SINK_TX_TRIG_FAIL_INT_MASK)
		return TRUE;

	return FALSE;
}
/*
static BOOL fgSinkCbusRxEvent(void)
{
	if((u1SinkCbusRxHwStatus
		&(SINK_LINKRX_TIMEOUT_INT_MASK|SINK_RBUF_PULS1_INT_MASK))
		== 0)
		return FALSE;
	return TRUE;
}

static BOOL fgSinkCbusRxOk(void)
{
	if((u1SinkCbusRxHwStatus
		& (SINK_LINKRX_TIMEOUT_INT_MASK|SINK_RBUF_PULS1_INT_MASK))
		== SINK_RBUF_PULS1_INT_MASK)
		return TRUE;
	return FALSE;
}
*/

static BOOL fgSinkCbusRxErr(void)
{
	if ((u1SinkCbusRxHwStatus & SINK_LINKRX_TIMEOUT_INT_MASK) == 0)
		return FALSE;

	return TRUE;
}
static unsigned int u4CbusReadRxLen(void)
{
	unsigned int tmp;

	tmp = u4SinkReadCbus(REG_CBUS_STA_00);
	tmp = (tmp & SINK_RBUF_LVL_LAT_MASK) >> SINK_RBUF_LVL_LAT;
	return tmp;
}

BOOL fgSinkCbusMscIdle(void)
{
	if (stSinkCbus.u2CbusMscMode == SINK_CBUS_STATE_IDLE)
		return TRUE;
	else
		return FALSE;
}
BOOL fgSinkCbusDdcIdle(void)
{
	if (stSinkCbus.stDdc.u2State == SINK_CBUS_STATE_DDC_IDEL)
		return TRUE;
	else
		return FALSE;
}

BOOL fgSinkRxBufEmpty(void)
{
	if (u4CbusReadRxLen() == 0)
		return TRUE;
	else
		return FALSE;
}

BOOL fgSinkTxBufEmpty(void)
{
	if ((sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_IDLE) && (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_IDLE))
		return TRUE;
	else
		return FALSE;
}

BOOL fgSinkCbusHwTxRxIdle(void)
{
	unsigned int tmp;

	tmp = u4SinkReadCbus(REG_CBUS_STA_01);

	if ((tmp & (SINK_LINKRX_FSM_MASK | SINK_LINKTX_FSM_MASK)) == 0)
		return TRUE;

	return FALSE;
}

/************************************************
    MSC control packets
************************************************/
static BOOL fgIsSinkMscData(unsigned short u2data)
{
	if ((u2data & 0x700) == 0x400)
		return TRUE;
	else
		return FALSE;
}
static unsigned char u1GetSinkMscData(unsigned short u2data)
{
	return (unsigned char)(u2data & 0xff);
}
static unsigned short u2SetSinkMscData(unsigned short u2Data)
{
	return ((u2Data & 0xff) | 0x400);
}
static BOOL fgIsSinkMscACK(unsigned short u2data)
{
	if (u2data == SINK_CBUS_MSC_ACK)
		return TRUE;
	else
		return FALSE;
}
static BOOL fgIsSinkMscNACK(unsigned short u2data)
{
	if (u2data == SINK_CBUS_MSC_NACK)
		return TRUE;
	else
		return FALSE;
}
static BOOL fgIsSinkMscAbort(unsigned short u2data)
{
	if (u2data == SINK_CBUS_MSC_ABORT)
		return TRUE;
	else
		return FALSE;
}
static BOOL fgIsSinkMscOpInvalid(unsigned short u2data)
{
	if ((u2data & 0x100) == 0)
		return FALSE;

	if ((u2data == SINK_CBUS_MSC_ACK)
	    || (u2data == SINK_CBUS_MSC_ABORT)
	    || (u2data == SINK_CBUS_MSC_NACK)
	    || (u2data == SINK_CBUS_MSC_WRITE_STATE)
	    || (u2data == SINK_CBUS_MSC_READ_DEVCAP)
	    || (u2data == SINK_CBUS_MSC_GET_STATE)
	    || (u2data == SINK_CBUS_MSC_GET_VENDER_ID)
	    || (u2data == SINK_CBUS_MSC_MSC_MSG)
	    || (u2data == SINK_CBUS_MSC_GET_SC1_EC)
	    || (u2data == SINK_CBUS_MSC_GET_DDC_EC)
	    || (u2data == SINK_CBUS_MSC_GET_MSC_EC)
	    || (u2data == SINK_CBUS_MSC_WRITE_BURST)
	    || (u2data == SINK_CBUS_MSC_GET_SC3_EC)
	    || (u2data == SINK_CBUS_MSC_EOF)
	   )
		return FALSE;
	else
		return TRUE;
}
static BOOL fgIsSinkRespMscCmdValid(unsigned short u2data)
{
	if ((u2data == SINK_CBUS_MSC_WRITE_STATE)
	    || (u2data == SINK_CBUS_MSC_READ_DEVCAP)
	    || (u2data == SINK_CBUS_MSC_GET_STATE)
	    || (u2data == SINK_CBUS_MSC_GET_VENDER_ID)
	    || (u2data == SINK_CBUS_MSC_MSC_MSG)
	    || (u2data == SINK_CBUS_MSC_GET_SC1_EC)
	    || (u2data == SINK_CBUS_MSC_GET_DDC_EC)
	    || (u2data == SINK_CBUS_MSC_GET_MSC_EC)
	    || (u2data == SINK_CBUS_MSC_WRITE_BURST)
	    || (u2data == SINK_CBUS_MSC_GET_SC3_EC)
	   )
		return TRUE;
	else
		return FALSE;
}
/*
static BOOL fgIsSinkRespMscACKNACKABORTCmd(unsigned short u2data)
{
	if((u2data == SINK_CBUS_MSC_ACK)
		||(u2data == SINK_CBUS_MSC_NACK)
		||(u2data == SINK_CBUS_MSC_ABORT)
	)
		return TRUE;
	else
		return FALSE;
}
*/

BOOL fgIsSinkMscMsg(unsigned short u2data)
{
	if ((u2data == SINK_MSC_MSG_MSGE)
	    || (u2data == SINK_MSC_MSG_RCP)
	    || (u2data == SINK_MSC_MSG_RCPK)
	    || (u2data == SINK_MSC_MSG_RCPE)
	    || (u2data == SINK_MSC_MSG_RAP)
	    || (u2data == SINK_MSC_MSG_RAPK)
	    || (u2data == SINK_MSC_MSG_UCP)
	    || (u2data == SINK_MSC_MSG_UCPK)
	    || (u2data == SINK_MSC_MSG_UCPE)
	   )
		return TRUE;
	else
		return FALSE;
}

void vSetSinkCbusMSCWaitTmr(unsigned int u2Tmr)
{
	st_SinkMscTmrOut.u4MscDdcTmr = u2Tmr;
	st_SinkMscTmrOut.fgTmrOut = FALSE;
}
void vClrSinkCbusMSCWaitTmr(void)
{
	st_SinkMscTmrOut.u4MscDdcTmr = 0;
	st_SinkMscTmrOut.fgTmrOut = FALSE;
}

BOOL fgSinkCbusMSCWaitTmrOut(void)
{
	if (st_SinkMscTmrOut.fgTmrOut == TRUE)
		return TRUE;
	else
		return FALSE;
}
void vSetSinkCbusTXWaitTmr(unsigned int u2Tmr)
{
	sink_cbus_tx_timer.u4MscDdcTmr = u2Tmr;
	sink_cbus_tx_timer.fgTmrOut = FALSE;
}
void vClrSinkCbusTXWaitTmr(void)
{
	sink_cbus_tx_timer.u4MscDdcTmr = 0;
	sink_cbus_tx_timer.fgTmrOut = FALSE;
}

BOOL fgSinkCbusTXWaitTmrOut(void)
{
	if (sink_cbus_tx_timer.fgTmrOut == TRUE)
		return TRUE;
	else
		return FALSE;
}
void vSetSinkCbusDDCWaitTmr(unsigned int u2Tmr)
{
	st_SinkDdcTmrOut.u4MscDdcTmr = u2Tmr;
	st_SinkDdcTmrOut.fgTmrOut = FALSE;
}
void vClrSinkCbusDDCWaitTmr(void)
{
	st_SinkDdcTmrOut.u4MscDdcTmr = 0;
	st_SinkDdcTmrOut.fgTmrOut = FALSE;
}

BOOL fgSinkCbusDDCWaitTmrOut(void)
{
	if (st_SinkDdcTmrOut.fgTmrOut == TRUE)
		return TRUE;
	else
		return FALSE;
}
void vSetSinkCbusBurstReqWaitTmr(unsigned int u2Tmr)
{
	sink_cbus_burst_timer.u4MscDdcTmr = u2Tmr;
	sink_cbus_burst_timer.fgTmrOut = FALSE;
}
void vClrSinkCbusBurstReqWaitTmr(void)
{
	sink_cbus_burst_timer.u4MscDdcTmr = 0;
	sink_cbus_burst_timer.fgTmrOut = FALSE;
}

BOOL fgSinkCbusBurstReqWaitTmrOut(void)
{
	if (sink_cbus_burst_timer.fgTmrOut == TRUE)
		return TRUE;
	else
		return FALSE;
}
/*************************************************/

void vResetSinkCbusMSCState(void)
{
	vClrSinkCbusMSCWaitTmr();
	stSinkCbus.u2RXBufIndex = 0;
	stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S0;
	stSinkCbus.stReq.u2State = SINK_CBUS_STATE_S0;
	stSinkCbus.u2CbusMscMode = SINK_CBUS_STATE_IDLE;
}
void vResetSinkCbusDDCState(void)
{
	vClrSinkCbusDDCWaitTmr();
	st_sink_ddc.hdcp.u1DevAddrW = 0;
	st_sink_ddc.hdcp.u1DevAddrR = 0;
	st_sink_ddc.edid.u1DevAddrW = 0;
	st_sink_ddc.edid.u1DevAddrR = 0;
	stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_IDEL;
}

static void vSinkCbusTxTrigger(void)
{
	vSinkWriteCbusMsk(REG_CBUS_LINK_00, SINK_TX_TRIG_MASK, SINK_TX_TRIG_MASK);
	vSinkWriteCbusMsk(REG_CBUS_LINK_00, 0, SINK_TX_TRIG_MASK);
}

static unsigned char u1SinkCbusSendMsg(unsigned short *pMsgData, unsigned char dataSize)
{
	unsigned char i;

	/*fill tx data*/
	for (i = 0; i < dataSize; i = i + 2)
		vSinkWriteCbus(REG_CBUS_WBUF0 + (i << 1), ((*(pMsgData + i + 1)) << 16) | (*(pMsgData + i)));

	/*fill length*/
	vSinkWriteCbusMsk(REG_CBUS_LINK_00, (dataSize << SINK_TX_NUM), SINK_TX_NUM_MASK);
	vSinkCbusTxTrigger();

#if 0

	for (i = 0; i < dataSize; i++) {
		if ((*(pMsgData + i)) & 0x600)
			HDMI_LOG(HDMI_LOG_DEBUG, "<<T:%X\r\n", *(pMsgData + i));
	}

#endif
	return 0;
}

void vCbusCmdTest(UINT32 cmdid, UINT32 subcmd)
{
	unsigned short MsgData[32] = {0};

	switch (cmdid) {
	case 1:
		/* set hpd*/
		MsgData[0] = SINK_CBUS_MSC_SET_HPD;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 2:
		/* clr hpd*/
		MsgData[0] = SINK_CBUS_MSC_CLR_HPD;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 3:
		/* set path en*/
		MsgData[0] = SINK_CBUS_MSC_WRITE_STATE;
		MsgData[1] = SINK_CBUS_MSC_STATUS_LINK_MODE | 0x400;
		MsgData[2] = 0x40B;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 4:
		/* set DCAP_RDY ready*/
		MsgData[0] = SINK_CBUS_MSC_WRITE_STATE;
		MsgData[1] = SINK_CBUS_MSC_STATUS_CONNECTED_RDY | 0x400;
		MsgData[2] = 0x401;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 5:
		/* DCAP_CHG*/
		MsgData[0] = SINK_CBUS_MSC_SET_INT;
		MsgData[1] = SINK_CBUS_MSC_RCHANGE_INT | 0x400;
		MsgData[2] = 0x401;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 6:
		/* set EDID_CHG change*/
		MsgData[0] = SINK_CBUS_MSC_SET_INT;
		MsgData[1] = SINK_CBUS_MSC_DCHANGE_INT | 0x400;
		MsgData[2] = 0x402;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 7:
		/* read DEV_CAP register*/
		/*MscReadDevCap = 1;*/
		/*ReadDevCapOffset = CBUS_MSC_DEVCAP_DEV_STATE;*/
		break;

	case 9:
		/* write REQ_WRT*/
#if 0
		if (_arMhlSrcDevCap.FeatureFlag & 0x4) {
			LOG(1, "send REQ_WRT\n");
			MsgData[0] = CBUS_MSC_CTRL_SET_INT;
			MsgData[1] = CBUS_MSC_REG_REGCHANGE_INT;/*40~7F*/
			MsgData[2] = INT_REQ_WRT | 0x400;
			u1SinkCbusSendMsg(MsgData, 3);
		} else
			LOG(1, "source do not support scratch pad register: 0x%x", _arMhlSrcDevCap.FeatureFlag);

#endif
		break;

	case 10:
		/*write burst command*/
		/*u1SinkCbusSendMsg(0, NULL, 0);*/
		break;

	case 11:
#if 0
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_RAP;
		MsgData[2] = MHL_RAP_CMD_POLL;
		u1SinkCbusSendMsg(MsgData, 3);
		break;
#endif

	case 12:
#if 0
		LOG(1, "RAP content on test");
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_RAP;
		MsgData[2] = MHL_RAP_CMD_CONTENTON;
		u1SinkCbusSendMsg(MsgData, 3);
		break;
#endif

	case 13:
#if 0
		LOG(1, "RAP content off test");
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_RAP;
		MsgData[2] = MHL_RAP_CMD_CONTENTOFF;
		u1SinkCbusSendMsg(MsgData, 3);
		break;
#endif

	case 14:
#if 0
		LOG(1, "RAPK test");
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_RAPK;
		MsgData[2] = 0x400;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 15:
		LOG(0, "1k test 0");
		/*u11Ktest = 0;*/
		break;

#ifdef CC_MHL_3D_SUPPORT

	case 16:
		LOG(0, "3D edid config");
		vMHLParsingEDIDForMHL3D(arEdid);
		/*Mhl_3D_EDID_Configuration_VIC_DTD(&t3D_VIC_Struct,&t3D_DTD_Struct);*/
		break;

	case 17:
		LOG(0, "3D write burst");
		vCbusWriteBurst3D();
		break;
#endif

	case 18:
		LOG(1, "get vendor id\n");
		MsgData[0] = CBUS_MSC_CTRL_GET_VENDER_ID;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 19:
		LOG(1, "get msc error code\n");
		MsgData[0] = CBUS_MSC_CTRL_GET_MSC_EC;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 20:
		LOG(1, "PATH DISABLE");
		MsgData[0] = CBUS_MSC_CTRL_WRITE_STATE;
		MsgData[1] = CBUS_MSC_REG_ACTIVE_LINK_MODE;/*40~7F*/
		MsgData[2] = 0x400;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 21:
#ifdef CC_MHL_3D_SUPPORT
		LOG(0, "3D REQ TEST\n");
		MsgData[0] = CBUS_MSC_CTRL_SET_INT;
		MsgData[1] = CBUS_MSC_REG_REGCHANGE_INT;
		MsgData[2] = 0x400 | INT_REQ_WRT;
		u1SinkCbusSendMsg(MsgData, 3);
#endif
		break;

#ifdef SYS_MHL_SUPPORT

	case 22:
		LOG(0, "rcp receiver test, 0x%x", sub_cmd_id);
		Mhl_NFYRcp(sub_cmd_id);
		break;
#endif

	case 30:
		LOG(1, "get DEV_STATE\n");
		MsgData[0] = CBUS_MSC_CTRL_GET_STATE;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 31:
		LOG(1, "get GET_DDC_ERRORCODE\n");
		MsgData[0] = CBUS_MSC_CTRL_GET_DDC_EC;
		u1SinkCbusSendMsg(MsgData, 1);
		break;

	case 32:
		LOG(1, "RCPE cmd\n");
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_RCPE;
		MsgData[2] = 0x402;
		u1SinkCbusSendMsg(MsgData, 3);
		break;

	case 33:
		LOG(1, "UCPE cmd\n");
		MsgData[0] = CBUS_MSC_CTRL_MSC_MSG;
		MsgData[1] = MHL_MSC_MSG_UCPE;
		MsgData[2] = 0x401;
		u1SinkCbusSendMsg(MsgData, 3);
		break;
#endif

	default:
		break;
	}

}

static void vSinkCbusSendMscMsg(unsigned short *pMsgData, unsigned char dataSize)
{
	unsigned char i;

	for (i = 0; i < dataSize; i++)
		sink_cbus_msc_tx.u2TxBuf[i] = pMsgData[i];

	sink_cbus_msc_tx.u2Len  = dataSize;
	sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_VALID;

	if (sink_cbus_msc_tx.u2Len > SINK_TX_HW_BUF_MAX)
		HDMI_LOG(HDMI_LOG_DEBUG, "[mhl_rx] tx f\n");

	/*SINK_CBUS_ERR("tx f\n");*/
}

static void vSinkCbusSendDdcMsg(unsigned short *pMsgData, unsigned char dataSize)
{
	unsigned char i;

	for (i = 0; i < dataSize; i++)
		sink_cbus_ddc_tx.u2TxBuf[i] = pMsgData[i];

	sink_cbus_ddc_tx.u2Len  = dataSize;
	sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_VALID;

	if (sink_cbus_ddc_tx.u2Len > SINK_TX_HW_BUF_MAX)
		HDMI_LOG(HDMI_LOG_DEBUG, "[mhl_rx] tx f\n");

	/* SINK_CBUS_ERR("tx f\n");*/
}
void vMhlSinkUsePinTrigForDebug(BOOL fgen)
{
	/*gpio25*/
	(*((volatile unsigned int *)(IO_BASE + 0x180))) = (*((volatile unsigned int *)(IO_BASE + 0x180))) | (1 << 25);

	if (fgen)
		(*((volatile unsigned int *)(IO_BASE + 0x1A0))) =
		(*((volatile unsigned int *)(IO_BASE + 0x1A0))) | (1 << 25);
	else
		(*((volatile unsigned int *)(IO_BASE + 0x1A0))) =
		(*((volatile unsigned int *)(IO_BASE + 0x1A0))) & (~(1 << 25));
}

static void vSinkCbusMscErrHandling(unsigned char u1ErrorCode)
{
	unsigned short  arTxMscMsgs[2];

	vClrSinkCbusMSCWaitTmr();
	u1SinkCbusMscErrCode = u1ErrorCode;
	u1SinkCbusMscAbortDelay2S = SINK_MSCDDC_ERR_2S;
	/*SINK_CBUS_ERR("MSC EC = %x\n",u1SinkCbusMscErrCode);*/
	arTxMscMsgs[0] = SINK_CBUS_MSC_ABORT;
	vSinkCbusSendMscMsg(arTxMscMsgs, 1);

	if (u1ForceSinkCbusStop  == 0xA5) {
		vMhlSinkUsePinTrigForDebug(1);
		fgSinkCbusStop = TRUE;
		return;
	}
}
/*static void vSinkCbusDdcErrHandling(unsigned char u1ErrorCode)
{

	vClrSinkCbusDDCWaitTmr();
	u1SinkCbusDdcErrCode = u1ErrorCode;
	u1SinkCbusDdcAbortDelay2S = SINK_MSCDDC_ERR_2S;
	SINK_CBUS_ERR("DDC EC = %x\n", u1SinkCbusDdcErrCode);
	if (u1ForceSinkCbusStop  == 0xA5) {
		vMhlSinkUsePinTrigForDebug();
		fgSinkCbusStop = TRUE;
		while (fgSinkCbusStop)
			msleep(500);
	}
}*/


/*
static void vSinkCbusRespAbort(void)
{
    unsigned short u2msg;

    vResetSinkCbusMSCState();
    u2msg = SINK_CBUS_MSC_ABORT;
    vSinkCbusSendMscMsg(&u2msg,1);
}
*/
static void vSinkCbusRespAck(void)
{
	unsigned short u2msg;

	u2msg = SINK_CBUS_MSC_ACK;

	vSinkCbusSendMscMsg(&u2msg, 1);
}
static void vSinkCbusRespData(unsigned char u1Data)
{
	unsigned short u2msg;

	u2msg = (unsigned short)u1Data | 0x400;

	vSinkCbusSendMscMsg(&u2msg, 1);
}

/*************************************************

    requseter state process

*************************************************/
static void vSinkCbusReqSetHPDState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqSetHPD:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait value*/
		if (fgIsSinkMscACK(u2RxMsg))
			stSinkCbus.stReq.fgOk = TRUE;
		else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqSetHPDCmd(void)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_SET_HPD;
	stSinkCbusRequester.u4Len = 1;
	fgSinkReqWaitFinish = FALSE;
	HDMI_LOG(HDMI_LOG_DEBUG, "shpd\n");

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
static void vSinkCbusReqClrHPDState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqClrHPD:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait value*/
		if (fgIsSinkMscACK(u2RxMsg))
			stSinkCbus.stReq.fgOk = TRUE;
		else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqClrHPDCmd(void)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_CLR_HPD;
	stSinkCbusRequester.u4Len = 1;
	fgSinkReqWaitFinish = FALSE;
	HDMI_LOG(HDMI_LOG_DEBUG, "clrhpd\n");

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
static void vSinkCbusReqGetStateState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqGetState:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait value*/
		if (fgIsSinkMscData(u2RxMsg)) {
			src_dev_reg[0] = u1GetSinkMscData(u2RxMsg);
			stSinkCbus.stReq.fgOk = TRUE;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqGetStateCmd(void)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_GET_STATE;
	stSinkCbusRequester.u4Len = 1;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
static void vSinkCbusReqGetVendorIDState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqGetVendorID:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok*/
		if (fgIsSinkMscData(u2RxMsg)) {
			u1MhlSinkVendorID = u1GetSinkMscData(u2RxMsg);
			stSinkCbus.stReq.fgOk = TRUE;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqGetVendorIDCmd(void)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_GET_VENDER_ID;
	stSinkCbusRequester.u4Len = 1;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
void vSinkCbusReqReadDevCapState(unsigned short u2RxMsg)
{
	unsigned char u1MscData = 0;

	/*SINK_CBUS_LOG("ReqReadDevCap:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait ack*/
		if (fgIsSinkMscACK(u2RxMsg)) {
			vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			stSinkCbus.stReq.u2State = SINK_CBUS_STATE_S1;
			break;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	case SINK_CBUS_STATE_S1: /*wait value*/
		if (fgIsSinkMscData(u2RxMsg)) {
			/*SINK_CBUS_LOG("ID=%X,Data=%X\n",
			u1GetSinkMscData(stSinkCbusRequester.u2ReqBuf[1]),
			u1GetSinkMscData(u2RxMsg));*/
			u1MscData = u1GetSinkMscData(stSinkCbusRequester.u2ReqBuf[1]);

			if (u1MscData < SINK_CBUS_DEVICE_LENGTH)
				src_dev_reg[u1MscData] = u1GetSinkMscData(u2RxMsg);

			stSinkCbus.stReq.fgOk = TRUE;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqReadDevCapCmd(unsigned char u1Data)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_READ_DEVCAP;
	stSinkCbusRequester.u2ReqBuf[1] = u2SetSinkMscData(u1Data);
	stSinkCbusRequester.u4Len = 2;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
static void vSinkCbusReqWriteStatState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqWriteStat:%d\n",(stSinkCbus.stReq.u2State));*/

	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait ack*/
		if (fgIsSinkMscACK(u2RxMsg))
			stSinkCbus.stReq.fgOk = TRUE;
		else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}
BOOL fgSinkCbusReqWriteStatCmd(unsigned char u1Offset, unsigned char u1Data)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_WRITE_STATE;
	stSinkCbusRequester.u2ReqBuf[1] = u2SetSinkMscData(u1Offset);
	stSinkCbusRequester.u2ReqBuf[2] = u2SetSinkMscData(u1Data);
	stSinkCbusRequester.u4Len = 3;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}

void vSinkCbusReqWriteBrustState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqWriteBrust:%d\n",(stSinkCbus.stReq.u2State));*/

	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok*/
		if (fgIsSinkMscACK(u2RxMsg))
			stSinkCbus.stReq.fgOk = TRUE;
		else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}
}
BOOL fgSinkCbusReqWriteBrustCmd(unsigned char *ptData, unsigned char u1Len)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	if (u1Len > 17) {
		HDMI_LOG(HDMI_LOG_DEBUG, "ReqWriteBrust Len Err\n");
		return FALSE;
	}

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_WRITE_BURST;

	for (i = 0; i < u1Len; i++)
		stSinkCbusRequester.u2ReqBuf[i + 1] = u2SetSinkMscData(ptData[i]); /*offset/adopter/data*/

	stSinkCbusRequester.u2ReqBuf[u1Len + 1] = SINK_CBUS_MSC_EOF;
	stSinkCbusRequester.u4Len = u1Len + 2;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
void vSinkCbusReqMscMsgState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ReqMscMsg:%d\n",(stSinkCbus.stReq.u2State));*/

	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok,wait ack*/
		if (fgIsSinkMscACK(u2RxMsg))
			stSinkCbus.stReq.fgOk = TRUE;
		else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			stSinkCbus.stReq.fgOk = FALSE;
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			stSinkCbus.stReq.fgOk = FALSE;
		}

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}
}
BOOL fgSinkCbusReqMscMsgCmd(unsigned short u1Cmd, unsigned char u1Val)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_MSC_MSG;
	stSinkCbusRequester.u2ReqBuf[1] = u2SetSinkMscData(u1Cmd);
	stSinkCbusRequester.u2ReqBuf[2] = u2SetSinkMscData(u1Val);
	stSinkCbusRequester.u4Len = 3;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}
static void vSinkCbusReqMSCERRCodeDState(unsigned short u2RxMsg)
{
	unsigned char tmp;

	/*SINK_CBUS_LOG("ReqMSCERRCode:%d\n",(stSinkCbus.stReq.u2State));*/
	switch (stSinkCbus.stReq.u2State) {
	case SINK_CBUS_STATE_S0: /*wait read_devcap/regoffset ok*/
		if (fgIsSinkMscData(u2RxMsg)) {
			tmp = u1GetSinkMscData(u2RxMsg);
			/*SINK_CBUS_ERR("errc:%X\n",tmp);*/
			stSinkCbus.stReq.fgOk = TRUE;
		} else
			stSinkCbus.stReq.fgOk = FALSE;

		vResetSinkCbusMSCState();
		vSinkCbusReqFinish();
		break;

	default:
		break;
	}

}

BOOL fgSinkCbusReqMscERRCodeCmd(void)
{
	unsigned char i;

	/*SINK_CBUS_FUNC();*/
	if (u1SinkCbusMscAbortDelay2S)
		return FALSE;

	if (fgIsSinkCbusConnected() == FALSE)
		return FALSE;

	i = 15;

	stSinkCbusRequester.req_state = SINK_CBUS_REQ_BEGIN;
	stSinkCbusRequester.u2ReqBuf[0] = SINK_CBUS_MSC_GET_MSC_EC;
	stSinkCbusRequester.u4Len = 1;
	fgSinkReqWaitFinish = FALSE;

	while (stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) {
		vSinkCbusTriggerCmpInterrupt();
		msleep(2);
		i--;

		if (i == 0) {
			/*SINK_CBUS_ERR("cbus busy\n");*/
			return FALSE;
		}
	}

	vSinkCbusReqWaitFinish();

	if (stSinkCbus.stReq.fgOk)
		return TRUE;

	return FALSE;
}

/*******************************************************

     DDC

********************************************************/
BOOL  fgIsSinkDdcCmdValid(unsigned short u2data)
{
	if ((u2data == SINK_CBUS_DDC_CTRL_SOF)
	    || (u2data == SINK_CBUS_DDC_CTRL_EOF)
	    || (u2data == SINK_CBUS_DDC_CTRL_ACK)
	    || (u2data == SINK_CBUS_DDC_CTRL_NACK)
	    || (u2data == SINK_CBUS_DDC_CTRL_ABORT)
	    || (u2data == SINK_CBUS_DDC_CTRL_CONT)
	    || (u2data == SINK_CBUS_DDC_CTRL_STOP)
	   )
		return TRUE;

	return FALSE;
}
BOOL fgIsSinkDdcChannel(unsigned short u2data)
{
	if ((u2data & 0x600) == 0)
		return TRUE;

	return FALSE;
}
BOOL fgIsSinkMscChannel(unsigned short u2data)
{
	if ((u2data & 0x600) == 0x400)
		return TRUE;

	return FALSE;
}

unsigned char u1GetSinkDdcData(unsigned short u2Data)
{
	return (u2Data & 0xff);
}
unsigned short u2SetSinkDdcData(unsigned char u1Data)
{
	return (unsigned short)(u1Data & 0xff);
}

BOOL fgIsSinkDdcData(unsigned short u2Data)
{
	if ((u2Data & 0x0700) == 0)
		return TRUE;

	return FALSE;
}
BOOL fgIsSinkDdcAbort(unsigned short u2data)
{
	if (u2data == SINK_CBUS_DDC_CTRL_ABORT)
		return TRUE;
	else
		return FALSE;
}
BOOL fgIsSinkDdcCont(unsigned short u2data)
{
	if (u2data == SINK_CBUS_DDC_CTRL_CONT)
		return TRUE;
	else
		return FALSE;
}
BOOL fgIsSinkDdcSof(unsigned short u2data)
{
	if (u2data == SINK_CBUS_DDC_CTRL_SOF)
		return TRUE;
	else
		return FALSE;
}
BOOL fgIsSinkDdcEof(unsigned short u2data)
{
	if (u2data == SINK_CBUS_DDC_CTRL_EOF)
		return TRUE;
	else
		return FALSE;
}
BOOL fgIsSinkDdcStop(unsigned short u2data)
{
	if (u2data == SINK_CBUS_DDC_CTRL_STOP)
		return TRUE;
	else
		return FALSE;
}
void vSinkCbusSendAck(void)
{
	unsigned short u2Data[1];

	u2Data[0] = SINK_CBUS_DDC_CTRL_ACK;

	vSinkCbusSendDdcMsg(u2Data, 1);
}
void vSinkCbusSendNack(void)
{
	unsigned short u2Data[1];

	u2Data[0] = SINK_CBUS_DDC_CTRL_NACK;

	vSinkCbusSendDdcMsg(u2Data, 1);
}
void vSinkCbusSendDdcAbort(void)
{
	unsigned short u2Data[1];

	u2Data[0] = SINK_CBUS_DDC_CTRL_ABORT;

	vSinkCbusSendDdcMsg(u2Data, 1);
}
void vSinkCbusSendData(unsigned char data)
{
	unsigned short u2Data[1];

	u2Data[0] = data;

	vSinkCbusSendDdcMsg(u2Data, 1);
}

BOOL fgSinkDDCDevValid(unsigned char dev)
{
	if ((dev == SINK_CBUS_DDC_DATA_ADRW)
	    || (dev == SINK_CBUS_DDC_DATA_ADRR)
	    || (dev == SINK_CBUS_DDC_DATA_HDCP_ADRW)
	    || (dev == SINK_CBUS_DDC_DATA_HDCP_ADRR))
		return TRUE;

	return FALSE;
}
BOOL fgSinkDDCOffsetValid(unsigned char dev, unsigned int offset)
{
	if ((dev == SINK_CBUS_DDC_DATA_ADRW) || (dev == SINK_CBUS_DDC_DATA_ADRR)) {
		if (offset < 256)
			return TRUE;
	} else if (dev == SINK_CBUS_DDC_DATA_HDCP_ADRW) {
		if ((offset >= 0x10) && (offset <= 0x14)) /*AKsv 5*/
			return TRUE;
		else if (offset == 0x15) /*Ainfo 1*/
			return TRUE;
		else if ((offset >= 0x18) && (offset <= 0x1F)) /*An 8*/
			return TRUE;
	} else if (dev == SINK_CBUS_DDC_DATA_HDCP_ADRR) {
		if (offset < 0x10) /*BKsv 5*/
			return TRUE;
		else if ((offset >= 0x16) && (offset < 0x18)) /*Ri 2*/
			return TRUE;
		else if ((offset >= 0x20) && (offset < 0xc0)) /*V 20*/
			return TRUE;
	}

	return FALSE;
}
BOOL fgSinkDDCHDCPOffsetValid(unsigned int offset)
{
	if (offset < 0xc0)
		return TRUE;

	return FALSE;
}

static void vSinkCbusDdcErrHandling(unsigned char u1ErrorCode)
{
	unsigned short  arTxMscMsgs[2];

	vClrSinkCbusDDCWaitTmr();
	u1SinkCbusDdcErrCode = u1ErrorCode;
	u1SinkCbusDdcAbortDelay2S = SINK_MSCDDC_ERR_2S;
	/*SINK_CBUS_ERR("DDC EC = %x\n",u1SinkCbusDdcErrCode);*/
	arTxMscMsgs[0] = SINK_CBUS_DDC_CTRL_ABORT;
	vSinkCbusSendDdcMsg(arTxMscMsgs, 1);

	if (u1ForceSinkCbusStop  == 0xA5) {
		vMhlSinkUsePinTrigForDebug(1);
		fgSinkCbusStop = TRUE;
	}
}
void vResetSinkCbusDDCStateAll(void)
{
	/*SINK_CBUS_FUNC();*/
	vClrSinkCbusDDCWaitTmr();
	st_sink_ddc.hdcp.u1DevAddrW = 0;
	st_sink_ddc.hdcp.u1DevAddrR = 0;
	st_sink_ddc.hdcp.u1Offset = 0;
	st_sink_ddc.edid.u1DevAddrW = 0;
	st_sink_ddc.edid.u1DevAddrR = 0;
	st_sink_ddc.edid.u1Offset = 0;
	stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_IDEL;
}

BOOL mhl_cbus_ddc_short_read = FALSE;

void vSinkCbusDDCState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("ddc:%d,%d,%d,%X\n",
	stSinkCbus.stDdc.u2State,st_sink_ddc.hdcp.u1Offset,
	st_sink_ddc.edid.u1Offset,u2RxMsg);*/

	if (fgIsSinkDdcAbort(u2RxMsg)) {
		/*SINK_CBUS_ERR("ddc abort fail\n");*/
		u1SinkCbusDdcAbortDelay2S = SINK_MSCDDC_ERR_2S;
		vResetSinkCbusDDCState();

		if (u1ForceSinkCbusStop  == 0xA5) {
			vMhlSinkUsePinTrigForDebug(1);
			fgSinkCbusStop = TRUE;
			return;
		}
	}

	if (stSinkCbus.stDdc.u2State == SINK_CBUS_STATE_DDC_IDEL) {
		if (fgIsSinkDdcCmdValid(u2RxMsg) == TRUE)
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_SOF;
		else
			vSinkCbusDdcErrHandling(SINK_CBUS_DDC_PROTOCOL_ERR);
	}

	switch (stSinkCbus.stDdc.u2State) {
	case SINK_CBUS_STATE_DDC_SOF: /*sof*/
		if (u2RxMsg == SINK_CBUS_DDC_CTRL_SOF) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		}

		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_ADDR: /*device address*/
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcData(u2RxMsg)) {
			if ((u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_ADRW) ||
				(u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_HDCP_ADRW)) {
				vSinkCbusSendAck();

				if (u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_HDCP_ADRW) {
					st_sink_ddc.hdcp.u1DevAddrW = u1GetSinkDdcData(u2RxMsg);
					st_sink_ddc.is_hdcp = TRUE;
				} else {
					st_sink_ddc.edid.u1DevAddrW = u1GetSinkDdcData(u2RxMsg);
					st_sink_ddc.is_hdcp = FALSE;
				}

				stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_OFFSET;
				vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			} else if ((u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_ADRR) ||
			(u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_HDCP_ADRR)) {
				vSinkCbusSendAck();

				if (u1GetSinkDdcData(u2RxMsg) == SINK_CBUS_DDC_DATA_HDCP_ADRR) {
					st_sink_ddc.hdcp.u1DevAddrR = u1GetSinkDdcData(u2RxMsg);
					st_sink_ddc.is_hdcp = TRUE;

					if (st_sink_ddc.hdcp.u1DevAddrW == 0)
						st_sink_ddc.hdcp.u1Offset = 0x08;
				} else {
					st_sink_ddc.edid.u1DevAddrR = u1GetSinkDdcData(u2RxMsg);
					st_sink_ddc.is_hdcp = FALSE;
				}

				stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_CONT;
				vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			} else {
				vSinkCbusSendNack();
				stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
				vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			}

			break;
		} else if (fgIsSinkDdcStop(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_OFFSET: /*offset*/
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcStop(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcData(u2RxMsg)) { /*is offset, write operate*/
			if (st_sink_ddc.is_hdcp) {
				st_sink_ddc.hdcp.u1Offset = u1GetSinkDdcData(u2RxMsg);

				if (fgSinkDDCHDCPOffsetValid(st_sink_ddc.hdcp.u1Offset)) {
					vSinkCbusSendAck();
					stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_W_WAIT;
				} else {
					vSinkCbusSendNack();
					stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
				}

				vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			} else {
				st_sink_ddc.edid.u1Offset = u1GetSinkDdcData(u2RxMsg);
				vSinkCbusSendAck();
				stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_W_WAIT;
				vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			}

			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_W_WAIT:
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcStop(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if ((fgSinkDDCOffsetValid(SINK_CBUS_DDC_DATA_HDCP_ADRW,
			st_sink_ddc.hdcp.u1Offset) ==
			FALSE) && (st_sink_ddc.is_hdcp == TRUE)) {
			vSinkCbusSendNack();
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if ((fgSinkDDCOffsetValid(SINK_CBUS_DDC_DATA_ADRW,
			st_sink_ddc.edid.u1Offset) == FALSE) &&
			(st_sink_ddc.is_hdcp == FALSE)) {
			vSinkCbusSendNack();
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcData(u2RxMsg)) {
			if (st_sink_ddc.is_hdcp) {
				SinkDDCWriteData(st_sink_ddc.hdcp.u1DevAddrW,
					st_sink_ddc.hdcp.u1Offset, u1GetSinkDdcData(u2RxMsg));
				st_sink_ddc.hdcp.u1Offset++;
			} else {
				SinkDDCWriteData(st_sink_ddc.edid.u1DevAddrW,
					st_sink_ddc.edid.u1Offset, u1GetSinkDdcData(u2RxMsg));
				st_sink_ddc.edid.u1Offset++;
			}

			vSinkCbusSendAck();
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_CONT:
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcCont(u2RxMsg)) { /*is cont,wait data,read operate*/
			if (st_sink_ddc.is_hdcp) {
				vSinkCbusSendData(SinkDDCReadData(st_sink_ddc.hdcp.u1DevAddrR,
					st_sink_ddc.hdcp.u1Offset));
				st_sink_ddc.hdcp.u1Offset++;
			} else {
				vSinkCbusSendData(SinkDDCReadData(st_sink_ddc.edid.u1DevAddrR,
					st_sink_ddc.edid.u1Offset));
				st_sink_ddc.edid.u1Offset++;
			}

			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_R_WAIT;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcStop(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_R_WAIT:
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcStop(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if ((fgSinkDDCOffsetValid(SINK_CBUS_DDC_DATA_HDCP_ADRR,
		st_sink_ddc.hdcp.u1Offset) == FALSE) &&
		(st_sink_ddc.is_hdcp == TRUE)) {
			vSinkCbusSendNack();
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if ((fgSinkDDCOffsetValid(SINK_CBUS_DDC_DATA_ADRR,
			st_sink_ddc.edid.u1Offset) == FALSE) &&
			(st_sink_ddc.is_hdcp == FALSE)) {
			vSinkCbusSendNack();
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_EOF;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcCont(u2RxMsg)) { /*is cont,wait data,read operate*/
			if (st_sink_ddc.is_hdcp) {
				vSinkCbusSendData(SinkDDCReadData(st_sink_ddc.hdcp.u1DevAddrR,
					st_sink_ddc.hdcp.u1Offset));
				st_sink_ddc.hdcp.u1Offset++;
			} else {
				vSinkCbusSendData(SinkDDCReadData(st_sink_ddc.edid.u1DevAddrR,
					st_sink_ddc.edid.u1Offset));
				st_sink_ddc.edid.u1Offset++;
			}

			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	case SINK_CBUS_STATE_DDC_EOF:
		if (fgIsSinkDdcSof(u2RxMsg)) {
			stSinkCbus.stDdc.u2State = SINK_CBUS_STATE_DDC_ADDR;
			vSetSinkCbusDDCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			break;
		} else if (fgIsSinkDdcEof(u2RxMsg)) {
			vResetSinkCbusDDCState();
			break;
		}

		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		vResetSinkCbusDDCState();
		break;

	default:
		break;
	}
}

/****************************************************
    cbus responder cmd process
****************************************************/
void vSinkCbusRespGetStateState(unsigned short u2RxMsg)
{
	unsigned short u2Data[2];

	/*SINK_CBUS_LOG("RespGetState:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		u2Data[0] = src_dev_reg[0];
		u2Data[0] |= 0x400;
		vSinkCbusSendMscMsg(u2Data, 1);
		vResetSinkCbusMSCState();
		break;

	default:
		break;
	}
}
void vSinkCbusRespGetVendorIDState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("RespGetVendorID:%d\n",stSinkCbus.stResp.u2State);*/

	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSinkCbusRespData(u1MhlVendorID);
		vResetSinkCbusMSCState();
		break;

	default:
		break;
	}
}

void vSinkCbusRespGetDdcECState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("RespGetDdcEC:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSinkCbusRespData(u1SinkCbusDdcErrCode);
		vResetSinkCbusMSCState();
		break;

	default:
		break;
	}
}
void vSinkCbusRespGetMscECState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("RespGetMscEC:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSinkCbusRespData(u1SinkCbusMscErrCode);
		vResetSinkCbusMSCState();
		break;

	default:
		break;
	}
}
void vSinkCbusRespReadDevCapState(unsigned short u2RxMsg)
{
	unsigned short u2Data[2] = {0, 0};
	unsigned char u1Data = 0;

	/*SINK_CBUS_LOG("RespReadDevCap:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
		stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S1;
		break;

	case SINK_CBUS_STATE_S1: /*offset*/
		if (fgIsSinkMscData(u2RxMsg)) {
			if (u1GetSinkMscData(u2RxMsg) > 0x0f)
				vSinkCbusMscErrHandling(SINK_MSC_BAD_OFFSET);
			else {
				/*printk("cap off:%x ack:%x\r\n", (u2RxMsg&0xff),
				my_dev_reg[u1GetSinkMscData(stSinkCbus.u2RXBuf[1])]);*/
				stSinkCbus.u2RXBuf[1] = (u2RxMsg & 0xff);
				u2Data[0] = SINK_CBUS_MSC_ACK;
				u1Data = u1GetSinkMscData(stSinkCbus.u2RXBuf[1]);

				if (u1Data < 0x50)
					u2Data[1] = u2SetSinkMscData(my_dev_reg[u1Data]);

				vSinkCbusSendMscMsg(u2Data, 2);
			}

			vResetSinkCbusMSCState();
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	default:
		break;
	}
}
void vSinkCbusRespWriteStatState(unsigned short u2RxMsg)
{
	unsigned char u1Data, u1Data1;

	/*SINK_CBUS_LOG("RespWriteStat:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
		stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S1;
		break;

	case SINK_CBUS_STATE_S1: /*get offset*/
		if (fgIsSinkMscData(u2RxMsg)) {
			/*set_int : 0x20,write_stat :0x30*/
			bool flg1 = u1GetSinkMscData(u2RxMsg) >= 0x20;
			bool flg2 = u1GetSinkMscData(u2RxMsg) <= (0x20 + (src_dev_reg[SINK_CBUS_INT_STAT_SIZE] & 0x0F));
			bool flg3 = u1GetSinkMscData(u2RxMsg) >= 0x30;
			bool flg4 = u1GetSinkMscData(u2RxMsg) <= (0x30 + (src_dev_reg[SINK_CBUS_INT_STAT_SIZE] >> 4));

			if ((flg1 && flg2) || (flg3 && flg4)) {
				stSinkCbus.u2RXBuf[1] = u1GetSinkMscData(u2RxMsg);
				vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
				stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S2;
			} else {
				vSinkCbusMscErrHandling(SINK_MSC_BAD_OFFSET);
				vResetSinkCbusMSCState();
			}
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	case SINK_CBUS_STATE_S2: /*get data*/
		if (fgIsSinkMscData(u2RxMsg)) {
			u1Data1 = u1GetSinkMscData(stSinkCbus.u2RXBuf[1]);

			/*printk("ws d:%x\r\n", u2RxMsg);*/
			if (u1Data1 < 0x30) { /*set_int*/
				/*set_int*/
				src_dev_reg[u1Data1] |= u1GetSinkMscData(u2RxMsg);
			} else  if (u1Data1 < 0x50) {
				/*write stat*/
				src_dev_reg[u1Data1] = u1GetSinkMscData(u2RxMsg);
			}

			u1Data = u1GetSinkMscData(u2RxMsg);

			if (u1Data1 == SINK_CBUS_MSC_RCHANGE_INT) {
				if (u1Data & SINK_CBUS_MSC_RCHANGE_INT_DCAP_CHG) {
					/*SINK_CBUS_LOG("fgIsDCapChg\n");*/
					st_dev_ctrl.fgIsDCapChg = TRUE;
				}

				if (u1Data & SINK_CBUS_MSC_RCHANGE_INT_DSCR_CHG) {
					st_dev_ctrl.fgIsDScrChg = TRUE;
					/*SINK_CBUS_LOG("fgIsDScrChg\n");*/
				}

				if (u1Data & SINK_CBUS_MSC_RCHANGE_INT_REQ_WRT) {
					st_dev_ctrl.fgIsReqWrt = TRUE;
					/*SINK_CBUS_LOG("fgIsReqWrt\n");*/
				}

				if (u1Data & SINK_CBUS_MSC_RCHANGE_INT_GRT_WRT) {
					st_dev_ctrl.fgIsGrtWrt = TRUE;
					/*SINK_CBUS_LOG("fgIsGrtWrt\n");*/
				}

				if (u1Data & SINK_CBUS_MSC_RCHANGE_INT_3D_REQ) {
					st_dev_ctrl.fgIs3DReq = TRUE;
					/*SINK_CBUS_LOG("fgIs3DReq\n");*/
				}

				src_dev_reg[u1Data1] = 0;
			} else if (u1Data1 == SINK_CBUS_MSC_DCHANGE_INT) {
				if (u1Data & SINK_CBUS_MSC_DCHANGE_INT_EDID_CHG) {
					st_dev_ctrl.fgIsEdidChg = TRUE;
					/*SINK_CBUS_LOG("fgIsEdidChg\n");*/
				}

				src_dev_reg[u1Data1] = 0;
			} else if (u1Data1 == SINK_CBUS_MSC_STATUS_CONNECTED_RDY) {
				if (u1Data & SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY) {
					st_dev_ctrl.fgIsDCapRdy = TRUE;
					st_dev_ctrl.fgIsDCapChg = TRUE;
				} else
					st_dev_ctrl.fgIsDCapRdy = FALSE;

				/*SINK_CBUS_LOG("fgIsDCapRdy:%d\n",st_dev_ctrl.fgIsDCapRdy);*/
			} else if (u1Data1 == SINK_CBUS_MSC_STATUS_LINK_MODE) {
				/*printk("wr lk: %x\r\n", u1Data);*/
				if ((u1Data & SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE) ==
					SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__PacketPixel) {
					HDMI_LOG(HDMI_LOG_DEBUG, "ppture\r\n");
					st_dev_ctrl.fgIsPPMode = TRUE;
					SinkSetPPMode(TRUE);
				} else {
					HDMI_LOG(HDMI_LOG_DEBUG, "ppfalse\r\n");
					st_dev_ctrl.fgIsPPMode = FALSE;
					SinkSetPPMode(FALSE);
				}

				/*SINK_CBUS_LOG("fgIsPPMode:%d\n",st_dev_ctrl.fgIsPPMode);*/
				if (u1Data & SINK_CBUS_MSC_STATUS_LINK_MODE_PATH_EN) {
					if (st_dev_ctrl.fgIsPathEn == FALSE) {
						st_dev_ctrl.fgIsPathEn = TRUE;
						st_dev_ctrl.st_hpd.fgHPDOn = TRUE;
						st_dev_ctrl.st_hpd.fgHPDChanged = TRUE;
						sink_delay_hpd = 0;
					}
				} else
					st_dev_ctrl.fgIsPathEn = FALSE;

				/*SINK_CBUS_LOG("fgIsPathEn:%d\n",st_dev_ctrl.fgIsPathEn);*/
				if (u1Data & SINK_CBUS_MSC_STATUS_IS_MUTED)
					st_dev_ctrl.fgIsMuted = TRUE;
				else
					st_dev_ctrl.fgIsMuted = FALSE;

				/*SINK_CBUS_LOG("fgIsMuted:%d\n",st_dev_ctrl.fgIsMuted);*/
			}

			vSinkCbusRespAck();
			vResetSinkCbusMSCState();
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	default:
		break;
	}
}
void vSinkCbusRespWriteBrustState(unsigned short u2RxMsg)
{
	unsigned char i;
	/*SINK_CBUS_LOG("RespWriteBrust:%d\n",stSinkCbus.stResp.u2State);*/

	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd*/
		vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
		stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S1;
		break;

	case SINK_CBUS_STATE_S1: /*offset*/
		if ((fgIsSinkMscData(u2RxMsg)) && ((u2RxMsg & 0xff) > (my_dev_reg[SINK_CBUS_SCRATCHPAD_SIZE] + 0x40))) {
			vSinkCbusMscErrHandling(SINK_MSC_BAD_OFFSET);
			vResetSinkCbusMSCState();
		} else if (fgIsSinkMscData(u2RxMsg)) {
			stSinkCbus.u2RXBuf[1] = (u2RxMsg & 0xff);
			stSinkCbus.u2RXBufIndex = 0;
			vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S2;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	case SINK_CBUS_STATE_S2: /*data*/

		/*printk("rb:%d,%x\n",stSinkCbus.u2RXBufIndex,u2RxMsg);*/
		if (u2RxMsg == SINK_CBUS_MSC_EOF) { /*end*/
			if ((stSinkCbus.u2RXBufIndex <= 2) ||
				((stSinkCbus.u2RXBufIndex + 0x40)
				> (u1GetSinkMscData(stSinkCbus.u2RXBuf[1])
				+ my_dev_reg[SINK_CBUS_SCRATCHPAD_SIZE]))) /*least 2 data,max 16byte*/
				vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			else if (stSinkCbus.u2RXBufIndex > 2) {
					/*scratchpad*/
				if ((stSinkCbus.u2RXBuf[2] == my_dev_reg[SINK_CBUS_ADOPTER_ID_H])
				    && (stSinkCbus.u2RXBuf[3] == my_dev_reg[SINK_CBUS_ADOPTER_ID_L])) {
					for (i = 0; i < stSinkCbus.u2RXBufIndex; i++)
						src_dev_reg[stSinkCbus.u2RXBuf[1] + i] =
						stSinkCbus.u2RXBuf[i + 4];
				}
				/*3D VIC info*/
				/*if (stSinkCbus.u2RXBuf[2] == CBUS_3D_VIC_ID_H
					 && stSinkCbus.u2RXBuf[3] == CBUS_3D_VIC_ID_L) {
				}*/
				/*3D DTD info*/
				/*if (stSinkCbus.u2RXBuf[2] == CBUS_3D_DTD_ID_H
					 && stSinkCbus.u2RXBuf[3] == CBUS_3D_DTD_ID_L) {
				}*/

				vSinkCbusRespAck();

				} else {
					vSinkCbusRespAck();
				}

			vResetSinkCbusMSCState();
		} else if (fgIsSinkMscData(u2RxMsg)) {
			if ((stSinkCbus.u2RXBufIndex + stSinkCbus.u2RXBuf[1])
				> (my_dev_reg[SINK_CBUS_SCRATCHPAD_SIZE] + 0x40)) {
				vSinkCbusMscErrHandling(SINK_MSC_BAD_OFFSET);
				vResetSinkCbusMSCState();
			} else {
				stSinkCbus.u2RXBuf[stSinkCbus.u2RXBufIndex + 2] = (u2RxMsg & 0xff);
				vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
				stSinkCbus.u2RXBufIndex++;
			}
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	default:
		break;
	}
}
void vSinkCbusRespMsgState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("RespMsg:%d\n",stSinkCbus.stResp.u2State);*/
	switch (stSinkCbus.stResp.u2State) {
	case SINK_CBUS_STATE_S0: /*get msc cmd */
		vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
		stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S1;
		break;

	case SINK_CBUS_STATE_S1: /*get sub cmd */
		if (fgIsSinkMscData(u2RxMsg)) {
			stSinkCbusMscMsg.u2Code = u2RxMsg;
			vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			stSinkCbus.stResp.u2State = SINK_CBUS_STATE_S2;
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	case SINK_CBUS_STATE_S2: /*get data */
		HDMI_LOG(HDMI_LOG_DEBUG, "msg d: %x\n", u2RxMsg);

		if (fgIsSinkMscData(u2RxMsg)) {
			stSinkCbusMscMsg.u1Val = u1GetSinkMscData(u2RxMsg);
			stSinkCbusMscMsg.fgIsValid = TRUE;
			vSinkCbusRespAck();
			vResetSinkCbusMSCState();
		} else if (fgIsSinkMscNACK(u2RxMsg) || fgIsSinkMscAbort(u2RxMsg))
			vResetSinkCbusMSCState();
		else {
			vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
			vResetSinkCbusMSCState();
		}

		break;

	default:
		break;
	}
}
/*********************************************************************/

void vSinkCbusTxRxReset(void)
{
	vSinkWriteCbusMsk(REG_CBUS_LINK_03,
			  (SINK_SW_RESET_RX_MASK | SINK_SW_RESET_TX_MASK),
			  (SINK_SW_RESET_RX_MASK | SINK_SW_RESET_TX_MASK)
			 );
	vSinkWriteCbusMsk(REG_CBUS_LINK_03,
			  0,
			  (SINK_SW_RESET_RX_MASK | SINK_SW_RESET_TX_MASK)
			 );
}

void vSinkCbusReset(void)
{
	/*clear tx*/
	vResetSinkCbusDDCStateAll();
	vResetSinkCbusMSCState();

	u1SinkCbusDdcErrCode = 0;
	u1SinkCbusMscErrCode = 0;
	u1SinkCbusMscErrCodeDelay2S = 0;
	u1SinkCbusDdcErrCodeDelay2S = 0;
	u1SinkCbusMscAbortDelay2S = 0;
	u1SinkCbusDdcAbortDelay2S = 0;

	u1SinkCbusTxHwStatus = 0;
	u1SinkCbusRxHwStatus = 0;

	sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;
	sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_IDLE;
	stSinkCbusRequester.req_state = SINK_CBUS_REQ_IDLE;
	stSinkCbus.u2CbusMscMode = SINK_CBUS_STATE_IDLE;
	vSinkCbusTxRxReset();/*software reset of txrx circuit*/

}
void vSinkCbusIntDisable(void)
{
	vSinkWriteCbus(REG_CBUS_LINK_0D, 0);
}


void vSinkTaskSetPathEn(BOOL en)
{
}

void vSinkCbusRxEnable(void)
{
	vSinkWriteCbusMsk(REG_CBUS_LINK_02, SINK_LINKRX_EN_MASK, SINK_LINKRX_EN_MASK);
}
void vSinkCbusRxDisable(void)
{
	vSinkWriteCbusMsk(REG_CBUS_LINK_02, 0, SINK_LINKRX_EN_MASK);
}



unsigned int sink_100k_r = 3;
SINK_CBUS_R_TYPE  sink_cbus_r_bak = SINK_CBUS_R_NONE;

void vSinkCbusSelectR(SINK_CBUS_R_TYPE type)
{
	unsigned int u4CbusIntEnable = 0;

	if ((sink_efuse_config & 7) == 0)
		sink_100k_r = 3;
	else
		sink_100k_r = sink_efuse_config & 7;

	vSinkWriteCbusMsk(REG_CBUS_LINK_07, 0, SINK_ZCBUS_HW_MASK);

	if (type == SINK_CBUS_R_1K) {
		u4CbusIntEnable = SINK_DISC_DET_INT_CLR_MASK;
		vSinkWriteCbus(REG_CBUS_LINK_0D, u4CbusIntEnable);
		vSinkWriteCbusMsk(REG_CBUS_LINK_0B, (u4CbusIntEnable << SINK_INT_STA), SINK_INT_STA_MASK);
		CBUS_BIT_CLR(REG_CBUS_LINK_07, 31);

		if (!forceRxsence)
			SinkTaskSetPathEn(FALSE);

		vSinkCbusRxDisable();
		/*R : 1K*/
		vSinkWriteCbusMsk(REG_CBUS_LINK_07,
		SINK_ZCBUS_DISCOVER_EN_MASK,
		SINK_ZCBUS_DISCOVER_EN_MASK | SINK_ZCBUS_SINK_ON_CTRL_MASK);
		vSinkWriteCbusMsk(REG_CBUS_LINK_05,
		SINK_WAKEUP_EN_MASK | SINK_DISCOVERY_EN_MASK,
		SINK_WAKEUP_EN_MASK | SINK_DISCOVERY_EN_MASK);
	}

	else if (type == SINK_CBUS_R_100K) {
		/*R : 100K*/
		/*vSinkWriteCbusMsk(REG_CBUS_LINK_07,
		(0x03 << SINK_ZCBUS_SINK_ON_CTRL),
		SINK_ZCBUS_DISCOVER_EN_MASK|SINK_ZCBUS_SINK_ON_CTRL_MASK);*/
		vSinkWriteCbusMsk(REG_CBUS_LINK_07,
		(5 << SINK_ZCBUS_SINK_ON_CTRL),
		SINK_ZCBUS_DISCOVER_EN_MASK | SINK_ZCBUS_SINK_ON_CTRL_MASK);
		vSinkWriteCbusMsk(REG_CBUS_LINK_05, 0, SINK_WAKEUP_EN_MASK | SINK_DISCOVERY_EN_MASK);

		CBUS_BIT_SET(REG_CBUS_LINK_07, 31);  /*enable CBUS low disconnection counter~~*/
		u4CbusIntEnable = (
					  SINK_LINKRX_TIMEOUT_INT_CLR_MASK
					  | SINK_RBUF_TRIG_INT_CLR_MASK
					  /*|SINK_WBUF_TRIG_INT_CLR_MASK*/
					  | SINK_CBUS_LOW_DISCONN_INT_CLR_MASK
					  | SINK_MONITOR_CMP_INT_CLR_MASK
					  /*  |SINK_TX_TRUG_FAIL_INT_CLR_MASK*/
					  /*  |SINK_TX_ARB_FAIL_INT_CLR_MASK*/
					  | SINK_TX_OK_INT_CLR_MASK
					  | SINK_TX_RETRY_TO_INT_CLR_MASK
				  );
		vSinkWriteCbus(REG_CBUS_LINK_0D, u4CbusIntEnable);
		vSinkWriteCbusMsk(REG_CBUS_LINK_0B,
			(u4CbusIntEnable << SINK_INT_STA) | SINK_CBUS_DISCONN_CNT_EN_MASK,
			SINK_INT_STA_MASK);
#if ((!CONFIG_DRV_ONLY) && (!CONFIG_DRV_VERIFY_SUPPORT))
		p_irrx_mhl_fun = sink_get_keycode_from_ir;
#endif
		vSinkCbusRxEnable();
		SinkTaskSetPathEn(TRUE);
	}

	else {
		vSinkWriteCbus(REG_CBUS_LINK_0D, 0);
		vSinkWriteCbusMsk(REG_CBUS_LINK_0B, (0 << SINK_INT_STA), SINK_INT_STA_MASK);

		if (!forceRxsence)
			SinkTaskSetPathEn(FALSE);

		vSinkCbusRxDisable();
#if ((!CONFIG_DRV_ONLY) && (!CONFIG_DRV_VERIFY_SUPPORT))
		p_irrx_mhl_fun = NULL;
#endif
		/*R : float*/
		vSinkWriteCbusMsk(REG_CBUS_LINK_07, 0, SINK_ZCBUS_DISCOVER_EN_MASK | SINK_ZCBUS_SINK_ON_CTRL_MASK);
		vSinkWriteCbusMsk(REG_CBUS_LINK_05, 0, SINK_WAKEUP_EN_MASK | SINK_DISCOVERY_EN_MASK);

		if (is_sink_attached == FALSE)
			vSinkWriteCbusMsk(REG_CBUS_LINK_07, (sink_100k_r << SINK_ZCBUS_SINK_ON_CTRL),
			SINK_ZCBUS_DISCOVER_EN_MASK | SINK_ZCBUS_SINK_ON_CTRL_MASK);
	}

	is_discovery_ok = FALSE;
	is_sink_cbus_stuch_low = FALSE;
	/*clean interrput */
	vSinkWriteCbus(REG_CBUS_LINK_08, 0xFFFFFFFF);
	vSinkWriteCbus(REG_CBUS_LINK_08, 0);

	if (sink_cbus_r_bak != type) {
#if 0

		if (type == SINK_CBUS_R_1K)
			/*printk("[mhls]sel 1k R\n");*/

			else if (type == SINK_CBUS_R_100K)
				/*printk("[mhls]sel 100k R\n");*/

				else if (type == SINK_CBUS_R_FLOAT)
					/*printk("[mhls]sel float R\n");*/

					else
						/*printk("[mhls]sel none R\n");*/
#endif
						sink_cbus_r_bak = type;
	}
}
/**
MHL Cbus Init
*/

void vMHLCbusHwInit(void)
{
	bool ret;
	/*struct pinctrl *pinctrl_hdmi;*/
	struct pinctrl_state *hdmi_hpd_set;
	struct pinctrl_state *mhl_sense_set;

	/*pinctrl_hdmi = devm_pinctrl_get(hdmi_dev);*/
	if(IS_ERR(pinctrl_hdmi))
		HDMI_LOG(HDMI_LOG_INFO, "HDMI_HalGetPwr5V: devm_pinctrl_get fail \r\n");

	/*init cbus hw*/
	/*GPIO_MultiFun_Set(PIN_40_HDMI_HPD_RX, HDMI_HDP_SEL);*/
	hdmi_hpd_set = pinctrl_lookup_state(pinctrl_hdmi, "hdmi_hpd_sel_gpio40_in");
	if(IS_ERR(hdmi_hpd_set))
		HDMI_LOG(HDMI_LOG_INFO, "vMHLCbusHwInit: pinctrl_lookup_state fail \r\n");
	ret = pinctrl_select_state(pinctrl_hdmi, hdmi_hpd_set);
	if(ret)
		HDMI_LOG(HDMI_LOG_INFO, "vMHLCbusHwInit: pinctrl_select_state fail \r\n");

	/*GPIO_MultiFun_Set(PIN_79_MHL_SENSE, MHL_SENSE_SEL);*/
	mhl_sense_set = pinctrl_lookup_state(pinctrl_hdmi, "mhl_sense_sel_gpio79_in");
	if(IS_ERR(mhl_sense_set))
		HDMI_LOG(HDMI_LOG_INFO, "vMHLCbusHwInit: mhl_sense_set = pinctrl_lookup_state fail \r\n");
	ret = pinctrl_select_state(pinctrl_hdmi, mhl_sense_set);
	if(ret)
		HDMI_LOG(HDMI_LOG_INFO, "vMHLCbusHwInit: mhl_sense_set = pinctrl_select_state fail \r\n");


	/*BIT_SET(0x70, 8);*/
	/*BIT_SET(0x70, 3);*/
	BIT_SET(0x368, 13);

	vSinkWriteCbus(REG_CBUS_LINK_0D, 0xc603);
	/*vSinkWriteCbus(REG_CBUS_LINK_0B, 0x);*/


	vSinkWriteCbus(REG_CBUS_LINK_00, 0x08812e02);
	vSinkWriteCbus(REG_CBUS_LINK_01, 0xb3024818);
	vSinkWriteCbus(REG_CBUS_LINK_02, 0x87e50905);
	vSinkWriteCbus(REG_CBUS_LINK_03, 0x04240789);
	vSinkWriteCbus(REG_CBUS_LINK_04, 0x10830946);
	vSinkWriteCbus(REG_CBUS_LINK_05, 0x1f6190a2);
	/*vSinkWriteCbus(REG_CBUS_LINK_06, 0x4e4495b2);*/
	vSinkWriteCbus(REG_CBUS_LINK_06, 0x4ec495b2);
	/*vIO32Write4B(PDWNC_CBUS_LINK_07, 0x00f138f9); //for hw mode*/
	vSinkWriteCbus(REG_CBUS_LINK_07, 0x003138f9); /*Look out*/
	vSinkWriteCbus(REG_CBUS_LINK_08, 0x00000000);
	vSinkWriteCbus(REG_CBUS_LINK_09, 0x74771e47);
	vSinkWriteCbus(REG_CBUS_LINK_0A, 0x2000000f);
	vSinkWriteCbus(REG_CBUS_LINK_0B, 0xfffff02a);
	vSinkWriteCbus(REG_CBUS_LINK_0C, 0xbce6081e);
	udelay(2);
	vSinkWriteCbusMsk(REG_CBUS_LINK_03, 0, 0xf0000000);

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	BASE_WRITE32(0x7c, (BASE_READ32(0x7c) | (1 << 14))); /*hpd multi function pin*/
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	BASE_WRITE32(0x7c, (BASE_READ32(0x7c) | (1 << 18)));
	#endif
	vSinkWriteCbus(REG_CBUS_LINK_BAK, 0x7c7c);

	return;

}

/***********************************************************

    cbus cmd process

**********************************************************/
BOOL fgSinkCbusMscAbort = FALSE;
void vSinkCbusMSCState(unsigned short u2RxMsg)
{
	/*SINK_CBUS_LOG("RxMsg=%x\n",u2RxMsg); */

	if (fgIsSinkMscAbort(u2RxMsg)) {
		fgSinkCbusMscAbort = TRUE;
		/*SINK_CBUS_ERR("msc abort fail\n"); */
		u1SinkCbusMscAbortDelay2S = SINK_MSCDDC_ERR_2S;

		if (u1ForceSinkCbusStop  == 0xA5) {
			fgSinkCbusStop = TRUE;
			vMhlSinkUsePinTrigForDebug(1);
			return;
		}
	}

	if (stSinkCbus.u2CbusMscMode == SINK_CBUS_STATE_REQUESTER) {
		switch (stSinkCbus.stReq.u2Cmd) {
		case SINK_CBUS_MSC_GET_STATE:
			vSinkCbusReqGetStateState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_GET_VENDER_ID:
			vSinkCbusReqGetVendorIDState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_READ_DEVCAP:
			vSinkCbusReqReadDevCapState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_WRITE_STATE:
			vSinkCbusReqWriteStatState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_WRITE_BURST:
			vSinkCbusReqWriteBrustState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_MSC_MSG:
			vSinkCbusReqMscMsgState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_GET_MSC_EC:
			vSinkCbusReqMSCERRCodeDState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_SET_HPD:
			vSinkCbusReqSetHPDState(u2RxMsg);
			break;

		case SINK_CBUS_MSC_CLR_HPD:
			vSinkCbusReqClrHPDState(u2RxMsg);
			break;

		default:
			/*SINK_CBUS_LOG("req error\n");*/
			break;
		}
	} else {
		if (stSinkCbus.u2CbusMscMode == SINK_CBUS_STATE_IDLE) {
			/*if(fgIsSinkRespMscACKNACKABORTCmd(u2RxMsg) == FALSE)*/
			if (fgIsSinkMscAbort(u2RxMsg) == FALSE) { /*6.3.5.6*/
				if (fgIsSinkRespMscCmdValid(u2RxMsg)) {
					stSinkCbus.u2CbusMscMode = SINK_CBUS_STATE_RESPONDER;
					stSinkCbus.stResp.u2State  = SINK_CBUS_STATE_S0;
					stSinkCbus.stResp.u2Cmd = u2RxMsg;
					stSinkCbus.u2RXBuf[0] = u2RxMsg;
					/*SINK_CBUS_LOG("RespNewCmd : %x\n",stSinkCbus.u2RXBuf[0]);*/
					/*printk("RespNewcmd:%x\n", stSinkCbus.u2RXBuf[0]);*/
				} else if (u2RxMsg != NONE_PACKET) {
					/*6.3.5.2/6.3.5.3/6.3.5.4/6.3.5.5/6.3.5.7/*/
					if (fgIsSinkMscOpInvalid(u2RxMsg))
						vSinkCbusMscErrHandling(SINK_MSC_BAD_OPCODE);
					else
						vSinkCbusMscErrHandling(SINK_MSC_PROTOCOL_ERR);
				}
			}
		}

		if (stSinkCbus.u2CbusMscMode == SINK_CBUS_STATE_RESPONDER) {
			switch (stSinkCbus.stResp.u2Cmd) {
			case SINK_CBUS_MSC_GET_STATE:
				vSinkCbusRespGetStateState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_GET_VENDER_ID:
				vSinkCbusRespGetVendorIDState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_GET_DDC_EC:
				vSinkCbusRespGetDdcECState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_GET_MSC_EC:
				vSinkCbusRespGetMscECState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_READ_DEVCAP:
				vSinkCbusRespReadDevCapState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_WRITE_STATE:
				vSinkCbusRespWriteStatState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_WRITE_BURST:
				vSinkCbusRespWriteBrustState(u2RxMsg);
				break;

			case SINK_CBUS_MSC_MSC_MSG:
				vSinkCbusRespMsgState(u2RxMsg);
				break;

			default:
				vResetSinkCbusMSCState();
				/*SINK_CBUS_LOG("resp reserved cmd\n");*/
				break;
			}
		}
	}
}

void vSinkCbusInit(void)
{
	if (u1ForceSinkCbusStop  == 0xA5)
		if (fgSinkCbusStop)
			return;

	/*init cbus register*/
	vSinkWriteCbus(REG_CBUS_LINK_00, 0x08812E00);
	vSinkWriteCbus(REG_CBUS_LINK_01, 0xB362899B);
	vSinkWriteCbus(REG_CBUS_LINK_02, 0x0445CA08);
	vSinkWriteCbus(REG_CBUS_LINK_03, 0x04A48789);
	vSinkWriteCbus(REG_CBUS_LINK_04, 0x10834A64);
	vSinkWriteCbus(REG_CBUS_LINK_05, 0x2341C0B6);
	vSinkWriteCbus(REG_CBUS_LINK_06, 0x58052668);
	vSinkWriteCbus(REG_CBUS_LINK_07, 0x02B16118);
	vSinkWriteCbus(REG_CBUS_LINK_08, 0x0003FFFF);
	vSinkWriteCbus(REG_CBUS_LINK_09, 0x94780290);
	vSinkWriteCbus(REG_CBUS_LINK_0A, 0x2400000F);
	vSinkWriteCbus(REG_CBUS_LINK_0B, 0x0000302A);
	vSinkWriteCbus(REG_CBUS_LINK_0C, 0x3CE6CA21);
	vSinkWriteCbus(REG_CBUS_LINK_0D, 0x0002C7FC);
	vSinkWriteCbus(REG_CBUS_LINK_BAK, 0x00007C84);
	/*sink clk power on, set to 0*/

#if 0       /* Ke Xu*/
	PDWNC_WRITE32(REG_RW_CLKPDN, PDWNC_READ32(REG_RW_CLKPDN) & (~RW_CBUS_SINK_PDN));
	PDWNC_WRITE32(REG_RW_PDCLK, (PDWNC_READ32(REG_RW_PDCLK) & (~(0x07))) | 0x01);
	/*PDWNC_WRITE32(REG_RW_PDCLK, (PDWNC_READ32(REG_RW_PDCLK) & (~(0x07))) |0x09); */

	/*select cbus pin */
	PDWNC_WRITE32(REG_RW_PAD_PINMUX2, (PDWNC_READ32(REG_RW_PAD_PINMUX2) & (~(0x07 << 28))) | (0x00 << 28));
#endif

	vSinkCbusSelectR(SINK_CBUS_R_FLOAT);

	vSinkCbusReset();
}
/****************************************************************/
/* vbus supply power*/
static void sink_vbus_supply_power(BOOL fgen)
{
/*GPIO : PIN_LED0*/
	/*printk("VBUS\n");*/
	if (fgen) {
		#ifdef CONFIG_ATC_PLATFORM_ac83xx
		BASE_WRITE32(0xe8, (BASE_READ32(0xe8) | (1 << 14)));/*supply power for VBUS*/
		#elif defined CONFIG_ATC_PLATFORM_ac823x
		BASE_WRITE32(0xe8, (BASE_READ32(0xe8) | (1 << 18)));
		#endif
	} else {
		#ifdef CONFIG_ATC_PLATFORM_ac83xx
		BASE_WRITE32(0xe8, (BASE_READ32(0xe8) & (~(1 << 14))));/*disable power for VBUS*/
		#elif defined CONFIG_ATC_PLATFORM_ac823x
		BASE_WRITE32(0xe8, (BASE_READ32(0xe8) & (~(1 << 18))));
		#endif
	}
}
static BOOL sink_vbus_over_current_detect(void)
{
	/*printk("over current\n");*/
	return FALSE;

/*GPIO : PIN_LED1*/
	/*if ((BASE_READ32(0x100) & (1 << 30)) == 0)
		return TRUE;    *over current*

	return FALSE;*/
}
/*******************************************************/
/*10ms */
static unsigned char is_sink_attach_bak0;
static unsigned char is_sink_attach_bak1;
static unsigned char is_sink_attach = 0xff;
unsigned int sink_cbus_timer_live = 0;
unsigned int sink_test_mode_timer = 0;
bool sink_test_mode_timer_reset = false;
unsigned int sink_cbus_req_delay_by_discovery = 0;

void sink_reset_attach(void)
{
	is_sink_attach_bak0 = 0;
	is_sink_attach_bak1 = 0;
	is_sink_attach = 0xff;
	vSinkSetAttachStatus(FALSE);
	sink_vbus_supply_power(FALSE);
}
EXPORT_SYMBOL(sink_reset_attach);

void vSinkCbusTimer(void)
{
	unsigned int u4IntStat;
	/*check cable attach */
	sink_cbus_timer_live++;  /*Just for debug  -- Ke Xu */

	u4IntStat = u4SinkReadCbus(REG_CBUS_STA_01);

	if (u4IntStat & SINK_CBUS_CDSENSE_MASK) /*CD Sense on or off */
		is_sink_attach_bak0 = 1;
	else
		is_sink_attach_bak0 = 0;

	if (is_sink_attach_bak0 == is_sink_attach_bak1) {
		if (is_sink_attach != is_sink_attach_bak0) {
			if (is_sink_attach_bak0 == 1) {
				vSinkSetAttachStatus(TRUE);

				if (sink_vbus_over_current_detect() == FALSE)
					sink_vbus_supply_power(TRUE);
			} else {
				vSinkSetAttachStatus(FALSE);
				sink_vbus_supply_power(FALSE);
			}
		}

		is_sink_attach = is_sink_attach_bak0;
	}

	is_sink_attach_bak1 = is_sink_attach_bak0;

	/*cbus timer out */
	if (st_SinkDdcTmrOut.u4MscDdcTmr > 0) {
		st_SinkDdcTmrOut.u4MscDdcTmr--;

		if (st_SinkDdcTmrOut.u4MscDdcTmr == 0) {
			st_SinkDdcTmrOut.fgTmrOut = TRUE;
			vSinkCbusTriggerCmpInterrupt();
			/*SINK_CBUS_ERR("timer_tri,ddc timer out\n");*/
		}
	}

	if (st_SinkMscTmrOut.u4MscDdcTmr > 0) {
		st_SinkMscTmrOut.u4MscDdcTmr--;

		if (st_SinkMscTmrOut.u4MscDdcTmr == 0) {
			st_SinkMscTmrOut.fgTmrOut = TRUE;
			vSinkCbusTriggerCmpInterrupt();
			/*SINK_CBUS_ERR("timer_tri,msc timer out\n");*/
		}
	}

	if (sink_cbus_tx_timer.u4MscDdcTmr > 0) {
		sink_cbus_tx_timer.u4MscDdcTmr--;

		if (sink_cbus_tx_timer.u4MscDdcTmr == 0) {
			sink_cbus_tx_timer.fgTmrOut = TRUE;
			vSinkCbusTriggerCmpInterrupt();
			/*SINK_CBUS_ERR("tx timer out\n");*/
		}
	}

	if (sink_cbus_burst_timer.u4MscDdcTmr > 0) {
		sink_cbus_burst_timer.u4MscDdcTmr--;

		if (sink_cbus_burst_timer.u4MscDdcTmr == 0) {
			sink_cbus_burst_timer.fgTmrOut = TRUE;
			vClrSinkCbusBurstReqWaitTmr();
			st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_NONE;
			/*SINK_CBUS_ERR("burst timer out\n");*/
		}
	}

	/*6.3.6.5*/
	if (u1SinkCbusMscAbortDelay2S > 0)
		u1SinkCbusMscAbortDelay2S--;

	if (u1SinkCbusDdcAbortDelay2S > 0)
		u1SinkCbusDdcAbortDelay2S--;

	if (sink_cbus_req_delay_by_discovery > 0)
		sink_cbus_req_delay_by_discovery--;

	if (sink_vbus_over_current_detect() == TRUE)
		sink_vbus_supply_power(FALSE);

	if (sink_delay_hpd > 0) {
		sink_delay_hpd--;

		if (sink_delay_hpd == 0)
			sink_delay_set_hpd();
	}

	sink_link_timer++;
	sink_test_mode_timer++;
	if (sink_link_timer_reset) {
		sink_link_timer = 0;
		sink_link_timer_reset = false;
	}

	if (sink_test_mode_timer_reset) {
		sink_test_mode_timer_reset = 0;
		sink_test_mode_timer_reset = false;
	}
}
EXPORT_SYMBOL(vSinkCbusTimer);

/*************************************************/
unsigned int sink_cbus_int_live = 0;
unsigned int u4SinkCbusRxCount = 0;
unsigned int u4SinkCbusTest = 0;
unsigned int u4SinkIntStatusBak = 0;
/*************************************************/


static void _Cbus_ClrInt(UINT32 intStat)
{
	intStat = intStat | 0x4000;
	vSinkWriteCbus(REG_CBUS_LINK_08, intStat);
	vSinkWriteCbus(REG_CBUS_LINK_08, 0x0);

	vSinkWriteCbus(0xf8, 0x1);
	vSinkWriteCbus(0xf8, 0x0);

}
/*
static void vCbus_EnableInterrupt(void)
{
    vSinkWriteCbus(REG_CBUS_LINK_0D, 0x2c77c);
}
*/

void vMhlIntProcess(void)
{
	unsigned int u4IntStat;
	unsigned short u2RxMsg = 0;
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned short aRxMsg[SINK_RX_HW_BUF_MAX];

	sink_cbus_int_live++;
	u4SinkCbusRxCount = 0;

	/****************************************************
	  clear hw interrupt
	  ****************************************************/
	u4IntStat = u4SinkReadCbus(REG_CBUS_STA_00) & 0x7FFFF;
	_Cbus_ClrInt(u4IntStat);

	/* printk("irq: 0x%08x\n",u4IntStat); */

	if (u1ForceSinkCbusStop  == 0xA5) {
		if (fgSinkCbusStop)
			return;
	}

	/****************************************************
	  check wake up and discovery
	  ****************************************************/
	if (u4IntStat & SINK_CABLE_DETECT_INT_MASK) {
		/* CBUS_BIT_SET(REG_CBUS_LINK_07, SINK_LDO_SWITCH); */
		/* CBUS_BIT_CLR(REG_CBUS_LINK_07, SINK_LDO_SWITCH_HW); */
		HDMI_LOG(HDMI_LOG_DEBUG, "cable detect\n");
		/* CBUS_BIT_SET(REG_CBUS_LINK_07, 26); */
		/* CBUS_BIT_CLR(REG_CBUS_LINK_07, 23); */
		/* vSinkCbusSelectR(SINK_CBUS_R_1K); */
		/* CBUS_BIT_SET(REG_CBUS_LINK_05, SINK_WAKEUP_EN); */
		/* if(u4SinkReadCbus(REG_CBUS_LINK_07) & 0x00400000) //hw mode */
		{
			/* vIO32WriteFldAlign(REG_CBUS_LINK_05, 1, FLD_WAKEUP_EN); */
			/* vIO32WriteFldAlign(PDWNC_CBUS_LINK_05, 1, FLD_DISCOVERY_EN); */
		}
	}

	if (u4IntStat & SINK_WAKEUP_DET_INT_MASK) {
		/* vIO32WriteFldAlign(REG_CBUS_LINK_05, 1, FLD_DISCOVERY_EN); */
		/* CBUS_BIT_SET(REG_CBUS_LINK_05, SINK_DISCOVERY_EN); */
	}


	if (u4IntStat  & SINK_CBUS_LOW_DISCONN_INT_MASK) {
		/* vSinkCbusSelectR(SINK_CBUS_R_FLOAT); */
		/* vSetSinkCbusStuchLow(); */
		sink_cbus_low_err();
		uDisconInt = TRUE;
		/* SINK_CBUS_ERR(">low\n"); */
	}

	/*
	  if(u4IntStat  & (SINK_WK_TIMEOUT_ST_INT_CLR_MASK |
	  SINK_WAKEUP_ILL_WID_INT_CLR_MASK|SINK_ILL_WAKE2DISC_INT_CLR_MASK))
	  {
	  vSetSinkWakeupErr();
	  }
	  else */
	if (u4IntStat  & SINK_DISC_DET_INT_CLR_MASK) {
		sink_cbus_req_delay_by_discovery = 2;
		/*
		vSinkCbusSelectR(SINK_CBUS_R_100K);
		vSetSinkDiscoveryOk();
		*/
		HDMI_LOG(HDMI_LOG_INFO, "discover ok\n");
		//HdmiTimingEventNotify(3);
		sink_discovery_ok();
		HDMI_LOG(HDMI_LOG_INFO, "discover ok done\n");
		HdmiTimingEventNotify(3);
	}

	/****************************************************
	check hw Rx/Tx status
	****************************************************/
	u1SinkCbusTxHwStatus = 0;
	u1SinkCbusRxHwStatus = 0;

	if (u4IntStat & SINK_LINKRX_TIMEOUT_INT_MASK) {
		u1SinkCbusRxHwStatus |= SINK_LINKRX_TIMEOUT_INT_MASK;
		/* SINK_CBUS_ERR(">RTO\n"); */
#ifdef MHLRX_DBG_BY_PIN_TRIGGER
		vMhlSinkUsePinTrigForDebug(1);
#endif
	}

	if (u4IntStat & SINK_TX_RETRY_TO_INT_MASK) {
		u1SinkCbusTxHwStatus |= SINK_TX_RETRY_TO_INT_MASK;
		/* SINK_CBUS_ERR(">TRTO\n"); */
#ifdef MHLRX_DBG_BY_PIN_TRIGGER
		vMhlSinkUsePinTrigForDebug(1);
#endif
	}

	if (u4IntStat & SINK_TX_OK_INT_MASK)
		u1SinkCbusTxHwStatus |= SINK_TX_OK_INT_MASK;

	if (u4IntStat & SINK_TX_ARB_FAIL_INT_MASK) {
		u1SinkCbusTxHwStatus |= SINK_TX_ARB_FAIL_INT_MASK;
		/* SINK_CBUS_ERR(">TAL\n"); */
#ifdef MHLRX_DBG_BY_PIN_TRIGGER
		vMhlSinkUsePinTrigForDebug(1);
#endif
	}

	if (u4IntStat & SINK_TX_TRIG_FAIL_INT_MASK) {
		u1SinkCbusTxHwStatus |= SINK_TX_TRIG_FAIL_INT_MASK;
		/* SINK_CBUS_ERR(">TTF %x\n",u4IntStat); */
#ifdef MHLRX_DBG_BY_PIN_TRIGGER
		vMhlSinkUsePinTrigForDebug(1);
#endif
	}

	/****************************************************
	  read rx duffer data
	  ****************************************************/
	if ((sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_SEND) || (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_SEND)) {
		/* tx trigger fail, send again */
		if (fgSinkCbusTriFail() == TRUE) {
			vClrSinkCbusTXWaitTmr();

			if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_SEND)
				sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_VALID;

			if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_SEND)
				sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_VALID;

			/* SINK_CBUS_ERR("trig fail,again\n"); */
		} else if (fgSinkCbusTxEvent())
			vClrSinkCbusTXWaitTmr();
		else if (fgSinkCbusTXWaitTmrOut()) {
			vClrSinkCbusTXWaitTmr();

			if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_SEND)
				sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_VALID;

			if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_SEND)
				sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_VALID;

			/* SINK_CBUS_ERR("tx time out,again\n"); */
		}
	}

	/* requester first send data */
	if (stSinkCbusRequester.req_state == SINK_CBUS_REQ_WAIT) {
		if (fgSinkCbusTxEvent() || fgSinkCbusTXWaitTmrOut()) {
			stSinkCbusRequester.req_state = SINK_CBUS_REQ_IDLE;
			/* do not send req msc first data */
			sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;

			if ((fgSinkCbusTriFail() == TRUE) || fgSinkCbusTXWaitTmrOut()) {
				stSinkCbus.stReq.fgOk = FALSE;
				vSinkCbusReqFinish();
				/* SINK_CBUS_ERR("req tx trig fail\n"); */
			} else {
				/* SINK_CBUS_LOG("req next\n"); */
				stSinkCbus.u2CbusMscMode = SINK_CBUS_STATE_REQUESTER;
				stSinkCbus.stReq.u2Cmd = stSinkCbusRequester.u2ReqBuf[0];
				stSinkCbus.stReq.u2State = SINK_CBUS_STATE_S0;
				stSinkCbus.stReq.fgOk = FALSE;
				vSetSinkCbusMSCWaitTmr(SINK_MSCDDC_TMR_OUT_200MS);
			}
		}
	}

	if (fgSinkCbusTxErr()) { /* So recevie the int of tx fail, then SM goto idle. */
		sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;
		sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_IDLE;
		vResetSinkCbusMSCState();
		vResetSinkCbusDDCState();
		vSinkCbusTxRxReset();
		vClrSinkCbusTXWaitTmr();
		stSinkCbus.stReq.fgOk = FALSE;
		vSinkCbusReqFinish();
		/* SINK_CBUS_ERR("tx err\n"); */
	}

	if (fgSinkCbusTxOk()) { /* receive the int of tx success, then set SM to TX_SEND */
		vClrSinkCbusTXWaitTmr();

		if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_SEND)
			sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;

		if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_SEND)
			sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_IDLE;
	}

	if (fgSinkCbusRxErr()) { /*  receive the int of cbus rx fail, reset all SM */
		sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;
		sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_IDLE;
		vResetSinkCbusMSCState();
		vResetSinkCbusDDCState();
		vSinkCbusTxRxReset();
		vClrSinkCbusTXWaitTmr();
		stSinkCbus.stReq.fgOk = FALSE;
		vSinkCbusReqFinish();
		/* SINK_CBUS_ERR("rx err\n"); */
	}

	if (fgSinkCbusDDCWaitTmrOut()) {
		vSinkCbusDdcErrHandling(SINK_CBUS_DDC_INCOMPLETE_ERR);
		stSinkCbus.stDdc.fgOk = FALSE;
		vResetSinkCbusDDCState();
		/* SINK_CBUS_ERR("timer_tri,ddc timer out\n"); */
	}

	if (fgSinkCbusMSCWaitTmrOut()) {
		u1SinkCbusMscErrCode = SINK_MSC_MSG_TIMEOUT;
		vResetSinkCbusMSCState();
		stSinkCbus.stReq.fgOk = FALSE;
		vSinkCbusReqFinish();
		/* SINK_CBUS_ERR("mto\n"); */
	}

	if ((u4IntStat & SINK_RBUF_PULS1_INT_MASK)) {
		u1SinkCbusRxHwStatus |= SINK_RBUF_PULS1_INT_MASK;

		u4SinkCbusRxCount = u4CbusReadRxLen();
		j = u4SinkCbusRxCount;
		i = 0;

		while (j > 0) {
			j--;
			u2RxMsg = u4SinkReadCbus(REG_CBUS_RBUF) & 0x7FF;
			aRxMsg[i] = u2RxMsg;
			i++;
		}

		if (u4SinkCbusRxCount > 0) {
			i = 0;
			j = u4SinkCbusRxCount;

			while (j) {
				j--;
				u2RxMsg = aRxMsg[i];
				i++;

				if (fgIsSinkDdcChannel(u2RxMsg)) {
					/*send ddc data,but recieve ddc data, then ignore tx data */
					if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_SEND)
						sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_IDLE;

					vSinkCbusDDCState(u2RxMsg);
				} else if (fgIsSinkMscChannel(u2RxMsg)) {
					/*send msc data,but recieve msc data, then ignore tx data */
					if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_SEND) {
						sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_IDLE;

					if (stSinkCbusRequester.req_state == SINK_CBUS_REQ_WAIT) {
						stSinkCbusRequester.req_state = SINK_CBUS_REQ_IDLE;
						stSinkCbus.stReq.fgOk = FALSE;
						vSinkCbusReqFinish();
						}
					}

					HDMI_LOG(HDMI_LOG_DEBUG, "> %x ", u2RxMsg);
					vSinkCbusMSCState(u2RxMsg);
				} else
					HDMI_LOG(HDMI_LOG_DEBUG, "[mhl_rx] not cmd\n");
			}
		}

		j = u4SinkCbusRxCount;
		i = 0;

		while (j > 0) {
			j--;
			/*SINK_CBUS_TXRX("R:%X:%d\n",aRxMsg[i],i); */
			i++;
		}
	}

	/********************************************************
	  ********************************************************/
	if ((stSinkCbusRequester.req_state == SINK_CBUS_REQ_BEGIN) && fgSinkCbusMscIdle()) {
		if (fgSinkRxBufEmpty() && fgSinkTxBufEmpty() && fgSinkCbusHwTxRxIdle()) {
			/* SINK_CBUS_LOG("req begin\n"); */
			stSinkCbusRequester.req_state = SINK_CBUS_REQ_WAIT;
			vSinkCbusSendMscMsg(stSinkCbusRequester.u2ReqBuf, stSinkCbusRequester.u4Len);
		}
	}

	sink_cbus_tx_ctrl.u2Len = 0;

	/* msc tx buf */
	if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_VALID) {
		for (i = 0; i < sink_cbus_msc_tx.u2Len; i++)
			sink_cbus_tx_ctrl.u2TxBuf[i] = sink_cbus_msc_tx.u2TxBuf[i];

		sink_cbus_tx_ctrl.u2Len = sink_cbus_msc_tx.u2Len;
	}

	/* ddc tx buf */
	if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_VALID) {
		for (i = 0; i < sink_cbus_ddc_tx.u2Len; i++)
			sink_cbus_tx_ctrl.u2TxBuf[sink_cbus_tx_ctrl.u2Len + i] = sink_cbus_ddc_tx.u2TxBuf[i];

		sink_cbus_tx_ctrl.u2Len = sink_cbus_ddc_tx.u2Len + sink_cbus_tx_ctrl.u2Len;
	}

	/* send tx data */
	if (((sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_VALID) || (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_VALID))
	    && (sink_cbus_msc_tx.tx_sate != SINK_CBUS_TX_SEND)
	    && (sink_cbus_ddc_tx.tx_sate != SINK_CBUS_TX_SEND)) {
		u1SinkCbusSendMsg(sink_cbus_tx_ctrl.u2TxBuf, sink_cbus_tx_ctrl.u2Len);

		if (sink_cbus_msc_tx.tx_sate == SINK_CBUS_TX_VALID) {
			sink_cbus_msc_tx.tx_sate = SINK_CBUS_TX_SEND;
			vSetSinkCbusTXWaitTmr(20 / MHL_LINK_TIME);
		}

		if (sink_cbus_ddc_tx.tx_sate == SINK_CBUS_TX_VALID) {
			sink_cbus_ddc_tx.tx_sate = SINK_CBUS_TX_SEND;
			vSetSinkCbusTXWaitTmr(20 / MHL_LINK_TIME);
		}
	}

}
EXPORT_SYMBOL(vMhlIntProcess);
void vMhlSetDevCapReady(void)
{
	/* set 0x30[0] */
	fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_STATUS_CONNECTED_RDY, SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY);
	/* set 0x20[0] */
	fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT, SINK_CBUS_MSC_RCHANGE_INT_DCAP_CHG);
}

void vSinkCbusCmdStatus(void)
{

	HDMI_LOG(HDMI_LOG_DEBUG, "=========================================\n");
	HDMI_LOG(HDMI_LOG_DEBUG, "mhl_rx_log_on setting:%08X\n", mhl_rx_log_on);
	/* printk("cbus err debug mode by waveform : en=%d,stop=%d\n",u1ForceSinkCbusStop,fgSinkCbusStop); */
	HDMI_LOG(HDMI_LOG_DEBUG, "cbus link stop to floatcbus : en=%d\n", (int)fgSinkCbusFlowStop);
	HDMI_LOG(HDMI_LOG_DEBUG, "u2CbusMscMode=%d\n", stSinkCbus.u2CbusMscMode);
	HDMI_LOG(HDMI_LOG_DEBUG, "stReq.u1Cmd=%d\n", stSinkCbus.stReq.u2Cmd);
	HDMI_LOG(HDMI_LOG_DEBUG, "stReq.u2State=%d\n", stSinkCbus.stReq.u2State);
	/* printk("stReq.fgOk=%d\n",stSinkCbus.stReq.fgOk); */
	HDMI_LOG(HDMI_LOG_DEBUG, "stResp.u2State=%d\n", stSinkCbus.stResp.u2State);
	HDMI_LOG(HDMI_LOG_DEBUG, "sink_cbus_msc_tx.tx_sate=%d\n", sink_cbus_msc_tx.tx_sate);
	HDMI_LOG(HDMI_LOG_DEBUG, "sink_cbus_ddc_tx.tx_sate=%d\n", sink_cbus_ddc_tx.tx_sate);
	HDMI_LOG(HDMI_LOG_DEBUG, "stSinkCbusRequester.req_state=%d\n", stSinkCbusRequester.req_state);
	HDMI_LOG(HDMI_LOG_DEBUG, "stSinkCbusRequester.u4Len=%d\n", stSinkCbusRequester.u4Len);
	/* printk("fgSinkCbusMscIdle()=%d\n",fgSinkCbusMscIdle()); */
	/* printk("fgSinkCbusDdcIdle()=%d\n",fgSinkCbusDdcIdle()); */
	/* printk("fgSinkCbusHwTxRxIdle()=%d\n",fgSinkCbusHwTxRxIdle()); */
	HDMI_LOG(HDMI_LOG_DEBUG, "u1SinkCbusDdcAbortDelay2S=%d\n", u1SinkCbusDdcAbortDelay2S);
	HDMI_LOG(HDMI_LOG_DEBUG, "u1SinkCbusMscAbortDelay2S=%d\n", u1SinkCbusMscAbortDelay2S);
	HDMI_LOG(HDMI_LOG_DEBUG, "u4SinkCbusRxCount=%d\n", u4SinkCbusRxCount);

}

#endif

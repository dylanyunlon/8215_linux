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

#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/memory.h>
#include "x_os.h"
#include "avm_hal.h"
#include "avin_common.h"
#include "tvd_wch_mgr.h"
#include "ac823x_tvd/tvd_data_struct.h"

#define WCH_IDLE		1
#define WCH_STARTED		2
#define WCH_STOPPED		3

#define CVBS_NORMAL		(1 << 0)
#define CVBS_BACKCAR	(1 << 1)
#define CVBS_AVM  (1<<2)
#define WCH_AVM_YBUF_SIZE	(720*576)
#define WCH_AVM_CBUF_SIZE	(720*288)

#ifdef AVM_FULL_MODE
	u32 BUFFER_WIDTH = 720;
	u32 BUFFER_HEIGHT= 480;
#else
	u32 BUFFER_WIDTH = 368;
	u32 BUFFER_HEIGHT= 240;
#endif

struct mutex startLock;

static WCH_BUF_T g_rWchBufferInfo[5];
static WCH_CFG_T g_rWchCtrl[5];
static int mWidth[5];
static int mHeight[5];
static TVD_SIG_STATE_T mSigState[5];
static int videoport = CVBSIN_1P;
static int audioport = CVBSIN_1P;
static u32 wchId[5] = {WCH_1, WCH_2, WCH_3, WCH_4, WCH_5};
static u32 wch_status[5] = {WCH_STOPPED, WCH_STOPPED, WCH_STOPPED, WCH_STOPPED, WCH_STOPPED};
struct avm_data g_avm_data[AVM_MAX_VIDEO_DEVS];
static int g_cvbs_type = 0;
static u32 g_cvbs_mirror = 0;
static u32 g_backcar_mirror = 0;
static u32 g_avm_mirror = 0;
static struct task_struct *g_Thread_task1 = NULL;
static struct task_struct *g_Thread_task2 = NULL;
static struct task_struct *g_Thread_task3 = NULL;
static struct task_struct *g_Thread_task4 = NULL;
static struct task_struct *g_Thread_task5 = NULL;

static HANDLE_T g_hSendBufMsgQ_wch1 = NULL_HANDLE;
static HANDLE_T g_hSendBufMsgQ_wch2 = NULL_HANDLE;
static HANDLE_T g_hSendBufMsgQ_wch3 = NULL_HANDLE;
static HANDLE_T g_hSendBufMsgQ_wch4 = NULL_HANDLE;
static HANDLE_T g_hSendBufMsgQ_wch5 = NULL_HANDLE;
static struct mutex g_Lock;
bool avmInit = false;
bool avmStop[4] = {true,true,true,true};
bool hwReq = false;
//bool gThdCret = false;
u32 cvbsMode[5] = {AV_MODE_NTSC};
bool setSigPort = false;
static u32 g_u4LogCnt = 0;
//bool avmStart = 0;
static s32 wchID = 0;
static u32 stopCount = 0;

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);

static void wch_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;
	int i4Ret = 0;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[0]);
	if ((TVD_SIG_READY == mSigState[0]) || (TVD_SIG_CHANGE_DONE == mSigState[0])) {
		if (NULL_HANDLE != g_hSendBufMsgQ_wch1) {
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch1, pBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret)
			{
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			}
		}else{
			pr_err("[AVIN]%s: g_hSendBufMsgQ_wch1 is Null!\r\n", __func__);
		}
		
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[0]);
	}
}

static void wch_buffer_get1(u32 *pBufIdx) {
	struct capture_priv data;
	int i4Ret = 0;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[1]);
	if ((TVD_SIG_READY == mSigState[1]) || (TVD_SIG_CHANGE_DONE == mSigState[1])) {
		if (NULL_HANDLE != g_hSendBufMsgQ_wch2) {
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch2, pBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret)
			{
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			}
		}else{
			pr_err("[AVIN]%s: g_hSendBufMsgQ_wch2 is Null!\r\n", __func__);
		}
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[1]);
	}
}

static void wch_buffer_get2(u32 *pBufIdx) {
	struct capture_priv data;
	int i4Ret = 0;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[2]);
	if ((TVD_SIG_READY == mSigState[2]) || (TVD_SIG_CHANGE_DONE == mSigState[2])) {
		if (NULL_HANDLE != g_hSendBufMsgQ_wch3) {
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch3, pBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret)
			{
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			}
		}else{
			pr_err("[AVIN]%s: g_hSendBufMsgQ_wch3 is Null!\r\n", __func__);
		}
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[2]);
	}
}

static void wch_buffer_get3(u32 *pBufIdx) {
	struct capture_priv data;
	int i4Ret = 0;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[3]);
	if ((TVD_SIG_READY == mSigState[3]) || (TVD_SIG_CHANGE_DONE == mSigState[3])) {
		if (NULL_HANDLE != g_hSendBufMsgQ_wch4) {
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch4, pBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret)
			{
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			}
		}else{
			pr_err("[AVIN]%s: g_hSendBufMsgQ_wch4 is Null!\r\n", __func__);
		}
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[3]);
	}
}

static void wch_buffer_get4(u32 *pBufIdx) {
	struct capture_priv data;
	int i4Ret = 0;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[4]);
	if ((TVD_SIG_READY == mSigState[4]) || (TVD_SIG_CHANGE_DONE == mSigState[4])) {
		if (NULL_HANDLE != g_hSendBufMsgQ_wch5) {
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch5, pBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret)
			{
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			}
		}else{
			pr_err("[AVIN]%s: g_hSendBufMsgQ_wch5 is Null!\r\n", __func__);
		}
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[4]);
	}
}

static void vdoCloseWch(TVD_CHANNEL_ID channel_id)
{
	pr_info("[AVIN]%s: enter\n", __func__);
	mutex_lock(&g_Lock);
	if(WCH_STOPPED == wch_status[channel_id]) {
		pr_err("[AVIN]%s: The wch have already stopped!\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	
	if (StopWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);
	if (CloseWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: CloseWch success!\r\n", __func__);
	wch_status[channel_id] = WCH_STOPPED;
	mSigState[channel_id] = TVD_SIG_CLOSE_WCH;
	mutex_unlock(&g_Lock);
}

static void vdoSignal(TVD_CHANNEL_ID channel_id, int mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;
	u32 i4Idx = 0;
	u32 u4Size = 0;
	u32 iPhyAddrY = 0;
	u32 iPhyAddrC = 0;
	u64 addy = 0;
	u64 addc = 0;
	
	memset(&g_rWchCtrl[channel_id], 0, sizeof(WCH_CFG_T));
	g_rWchCtrl[channel_id].fgVSyncPolarity = false; /* FALSE is LOW level present sync.*/
	g_rWchCtrl[channel_id].fgHSyncPolarity = true; /* TRUE is High.*/

	g_rWchCtrl[channel_id].eInputFmt = DATA_FMT_YUV444;
	g_rWchCtrl[channel_id].fgProgressive = false;
	g_rWchCtrl[channel_id].eOutputFmt = DATA_FMT_YUV420;
	g_rWchCtrl[channel_id].u4ScanLineMode = 1;
	
	if(channel_id == TVD_CH_0){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD0;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVM_WCH1;
		g_rWchCtrl[channel_id].u1WchId = WCH_1;
	}else if(channel_id == TVD_CH_1){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD1;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVM_WCH2;
		g_rWchCtrl[channel_id].u1WchId = WCH_2;
	}else if(channel_id == TVD_CH_2){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD2;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVM_WCH3;
		g_rWchCtrl[channel_id].u1WchId = WCH_3;
	}else {
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD3;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVM_WCH4;
		g_rWchCtrl[channel_id].u1WchId = WCH_4;
	}
	
	switch (mode) {
	case AV_MODE_PAL:
		mWidth[channel_id] = PAL_FRAME_WIDTH;
		mHeight[channel_id] = PAL_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 2;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 2;
		g_rWchCtrl[channel_id].u4SrcStartX = 0x3B;
		cvbsMode[channel_id] = AV_MODE_PAL;
		pr_info("[AVIN]%s: CVBS signal system is PAL\r\n", __func__);
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_PAL_M:
	case AV_MODE_PAL60:
	case AV_MODE_NTSC:
		mWidth[channel_id] = NTSC_FRAME_WIDTH;
		mHeight[channel_id] = NTSC_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 1;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 0;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 0;
		cvbsMode[channel_id] = AV_MODE_NTSC;
		pr_info("[AVIN]%s: CVBS signal system is NTSC\r\n", __func__);
		break;

	case AV_MODE_SECAM:
		mWidth[channel_id] = SECAM_FRAME_WIDTH;
		mHeight[channel_id] = SECAM_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 1;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 1;
		cvbsMode[channel_id] = AV_MODE_SECAM;
		pr_info("[AVIN]%s: CVBS signal system is SECAM\r\n", __func__);
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		pr_err("[AVIN]%s: Get Error CVBS signal system value\r\n", __func__);
		return;
	}

	mutex_lock(&g_Lock);
	g_rWchCtrl[channel_id].u4SrcWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4SrcHeight = mHeight[channel_id];
	g_rWchCtrl[channel_id].u4DstWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4DstHeight = mHeight[channel_id];
	if(channel_id == TVD_CH_0){
		g_rWchCtrl[channel_id].GetWchBufIndx = wch_buffer_get;
	}else if(channel_id == TVD_CH_1){
		g_rWchCtrl[channel_id].GetWchBufIndx = wch_buffer_get1;
	}else if(channel_id == TVD_CH_2){
		g_rWchCtrl[channel_id].GetWchBufIndx = wch_buffer_get2;
	}else {
		g_rWchCtrl[channel_id].GetWchBufIndx = wch_buffer_get3;
	}
	/*
	if (WCH_SUCCESS != OpenWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);
	*/
	if (ConfigWch(&g_rWchCtrl[channel_id])) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rWchBufferInfo[channel_id], 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo[channel_id].u1WchId= wchId[channel_id];
	if (WchGetBufferAddress(&g_rWchBufferInfo[channel_id])) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		mutex_unlock(&g_Lock);
		return;
	}else{
		for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
			pr_info("[AVIN]%s: Physic Y(%lx) C(%lx)\r\n", __func__,
				g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			
			g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx]);
			
			g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			pr_info("[AVIN]%s: virtual Y(%lx) C(%lx)\r\n", __func__,
				g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			memset(g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx],0x10,720*576);
			memset(g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx],0x80,720*288);
		}
	}
	pr_info("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

	if (StartWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	wch_status[channel_id] = WCH_STARTED;
	
	pr_info("[AVIN]%s: StartWch success!\r\n", __func__);
	
	mSigState[channel_id] = TVD_SIG_READY;
	mutex_unlock(&g_Lock);
	pr_info("[AVIN]%s: vdosignal on leave!\r\n", __func__);
}

static void vdoNoSignal(TVD_CHANNEL_ID channel_id)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	pr_info("[AVIN]%s: enter\n", __func__);
	mutex_lock(&g_Lock);
	if(WCH_STOPPED == wch_status[channel_id]) {
		pr_err("[AVIN]%s: The wch have already stopped!\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	
	if (StopWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);
	/*if (CloseWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		return;
	}
	pr_debug("[AVIN]%s: CloseWch success!\r\n", __func__);*/
	wch_status[channel_id] = WCH_STOPPED;
	
	mSigState[channel_id] = TVD_SIG_LOST;
	mutex_unlock(&g_Lock);
	pr_info("[AVIN]%s: success!\r\n", __func__);
}

static void vdoSignalChangeStart(TVD_CHANNEL_ID channel_id)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	pr_info("[AVIN]%s: enter\n", __func__);
	if (StopWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_debug("[AVIN]%s: StopWch fail!\r\n", __func__);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight[channel_id];
	data.buf_width = mWidth[channel_id];
	data.signal_status = SIGNAL_CHANGE_START;
	pr_info("[AVIN]%s: leave\n", __func__);
}

static void vdoSignalChangeDone(TVD_CHANNEL_ID channel_id, u32 mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;
	u32 i4Idx = 0;
	u32 u4Size = 0;
	
	pr_info("[AVIN]%s: enter\n", __func__);
	switch (mode) {
	case AV_MODE_PAL:
		mWidth[channel_id] = PAL_FRAME_WIDTH;
		mHeight[channel_id] = PAL_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 2;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 2;
		g_rWchCtrl[channel_id].u4SrcStartX = 0x3B;
		pr_debug("[AVIN]%s: CVBS signal system change to PAL\n", __func__);
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_PAL_M:
	case AV_MODE_PAL60:
	case AV_MODE_NTSC:
		mWidth[channel_id] = NTSC_FRAME_WIDTH;
		mHeight[channel_id] = NTSC_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 1;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 0;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 0;
		pr_debug("[AVIN]%s: CVBS signal system change to NTSC\n", __func__);
		break;

	case AV_MODE_SECAM:
		mWidth[channel_id] = SECAM_FRAME_WIDTH;
		mHeight[channel_id] = SECAM_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 1;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 1;
		pr_debug("[AVIN]%s: CVBS signal system change to SECAM\n", __func__);
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		pr_debug("[AVIN]%s: Error CVBS signal system value\n", __func__);
		return;
	}
	
	/*if (StopWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_debug("[AVIN]%s: StopWch fail!\r\n", __func__);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);*/

	g_rWchCtrl[channel_id].u4SrcWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4SrcHeight = mHeight[channel_id];
	g_rWchCtrl[channel_id].u4DstWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4DstHeight = mHeight[channel_id];
	
	pr_debug("[AVIN]%s: wch config width(%d) height(%d)\r\n", __func__, mWidth[channel_id], mHeight[channel_id]);
	if (ConfigWch(&g_rWchCtrl[channel_id])) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		return;
	}
	pr_debug("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rWchBufferInfo[channel_id], 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo[channel_id].u1WchId= wchId[channel_id];
	if (WchGetBufferAddress(&g_rWchBufferInfo[channel_id])) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		return;
	}else{
		for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
			pr_debug("[AVIN]%s: Physic Y(%08x) C(%08x)\r\n", __func__,
				g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx]);
			
			g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			pr_debug("[AVIN]%s: virtual Y(%08x) C(%08x)\r\n", __func__,
				g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx]);
			memset(g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[i4Idx],0x10,720*576);
			memset(g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[i4Idx],0x80,720*288);
		}
	}
	pr_debug("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

	if (StartWch(wchId[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		return;
	}
	wch_status[channel_id] = WCH_STARTED;
	pr_debug("[AVIN]%s: StartWch success!\r\n", __func__);
	mSigState[channel_id] = TVD_SIG_CHANGE_DONE;

	pr_debug("[AVIN]%s: success!\r\n", __func__);
}

static void tvd_signal_status(void *pStatus)
{
	TVD_SIG_INFORMATION *pSigInfo = (TVD_SIG_INFORMATION *)pStatus;

	if(NULL == pSigInfo) {
		pr_err("[AVIN]%s: param pStatus is NULL!\n", __func__);
		return;
	}

	switch (pSigInfo->signal_state) {
	case TVD_SIG_READY:
		vdoSignal(pSigInfo->channel_id, pSigInfo->arg);
		break;

	case TVD_SIG_LOST:
		vdoNoSignal(pSigInfo->channel_id);
		break;

	case TVD_SIG_CHANGE_START:
		vdoSignalChangeStart(pSigInfo->channel_id);
		break;

	case TVD_SIG_CHANGE_DONE:
		vdoSignalChangeDone(pSigInfo->channel_id, pSigInfo->arg);
		break;

	case TVD_SIG_CLOSE_WCH:
		vdoCloseWch(pSigInfo->channel_id);
		break;

	default:
		break;
	}
	pr_info("[AVIN]%s: success with signal_state(%d)!\r\n",
		__func__, pSigInfo->signal_state);
}

static int tvd_wch1dram_thread(void *data){
	int32_t i4Ret1 = -1;
	uint32_t u4ChId = 0;
	uint32_t u4WchBufIdx1 = 0;
	u32 u2MsgIdx1 = 0;
	u32 i = 0;
	u32 z_msg_size = sizeof(uint32_t);
	struct avmbuf avmBuf[AVM_MAX_VIDEO_DEVS];
	pr_info("[AVIN]%s: enter!\r\n", __func__);
	while (true) {
		pr_debug("[AVIN]%s: tvd_wchdram_thread!\r\n", __func__);
		if (kthread_should_stop()) {
			pr_info("[AVIN]%s: kthread_should_stop!\r\n", __func__);
			break;
		}
		
		if (NULL_HANDLE == g_hSendBufMsgQ_wch1) {
			pr_info("[AVIN]%s: avm wait MsgQ Create!\r\n", __func__);
			msleep(5);
			continue;
		}
		
		if(0xFF != u4WchBufIdx1){
			i4Ret1 = x_msg_q_receive(&u2MsgIdx1, &u4WchBufIdx1, &z_msg_size, &g_hSendBufMsgQ_wch1,
				1, X_MSGQ_OPTION_WAIT);
		}else{
			msleep(5);
			continue;
		}

		if (OSR_OK != i4Ret1) {
			pr_info("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
		}else{
			pr_debug("[AVIN]%s: u4WchBufIdx1=%d\n",__func__, u4WchBufIdx1);
			pr_debug("[AVIN]%s: Physic Y(%lx) C(%lx)\r\n", __func__,
				g_rWchBufferInfo[0].tWchBuf.u4YBuf[u4WchBufIdx1], g_rWchBufferInfo[0].tWchBuf.u4CBuf[u4WchBufIdx1]);
			if (u4WchBufIdx1 < WCH_BUF_MAX_CNT){
				avmBuf[0].yAddr = g_rWchBufferInfo[0].tWchBuf.u4YBuf[u4WchBufIdx1];
				avmBuf[0].cAddr = g_rWchBufferInfo[0].tWchBuf.u4CBuf[u4WchBufIdx1];
				avmBuf[0].width = BUFFER_WIDTH;
				avmBuf[0].height = BUFFER_HEIGHT;
				
				if(avin_buffer_complete(AVIN_TYPE_AVM_REAR, (struct capture_priv *)&avmBuf[0])){
					pr_info("[AVIN]%s:buffer complete fial\n", __func__);
					msleep(5);
					continue;
				}
			}
			
		}

	}
	pr_info("[AVIN]%s: thread exit success!\r\n", __func__);

	return 0;
}

static int tvd_wch2dram_thread(void *data){
	int32_t i4Ret = -1;
	uint32_t u4WchBufIdx = 0;
	u32 u2MsgIdx = 0;
	u32 z_msg_size = sizeof(uint32_t);
	struct avmbuf avmBuf[AVM_MAX_VIDEO_DEVS];

	while (true) {
		pr_debug("[AVIN]%s: tvd_wchdram_thread!\r\n", __func__);
		if (kthread_should_stop()) {
			pr_info("[AVIN]%s: kthread_should_stop!\r\n", __func__);
			break;
		}
		
		if (NULL_HANDLE == g_hSendBufMsgQ_wch2) {
				pr_info("[AVIN]%s: avm wait MsgQ Create!\r\n", __func__);
				msleep(5);
				continue;
		}
		if(0xFF != u4WchBufIdx){
			i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &g_hSendBufMsgQ_wch2,
				1, X_MSGQ_OPTION_WAIT);
		}else{
			msleep(5);
			continue;
		}
		
		if (OSR_OK != i4Ret) {
			pr_info("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
		}else{
			if (u4WchBufIdx < WCH_BUF_MAX_CNT){
				pr_debug("[AVIN]%s: u4WchBufIdx = %d!\r\n", __func__, u4WchBufIdx);
				avmBuf[0].yAddr = g_rWchBufferInfo[1].tWchBuf.u4YBuf[u4WchBufIdx];
				avmBuf[0].cAddr = g_rWchBufferInfo[1].tWchBuf.u4CBuf[u4WchBufIdx];
				avmBuf[0].width = BUFFER_WIDTH;
				avmBuf[0].height = BUFFER_HEIGHT;
				
				if(avin_buffer_complete(AVIN_TYPE_AVM_FRONT, (struct capture_priv *)&avmBuf[0])){
					pr_info("[AVIN]%s:buffer complete fial\n", __func__);
					msleep(5);
					continue;
				}
			}
		}
		
	}
	pr_info("[AVIN]%s: thread2 exit success!\r\n", __func__);

	return 0;
}

static int tvd_wch3dram_thread(void *data){
	int32_t i4Ret = -1;
	uint32_t u4WchBufIdx = 0;
	u32 u2MsgIdx = 0;
	u32 z_msg_size = sizeof(uint32_t);
	struct avmbuf avmBuf[AVM_MAX_VIDEO_DEVS];

	while (true) {
		pr_debug("[AVIN]%s: tvd_wchdram_thread!\r\n", __func__);
		if (kthread_should_stop()) {
			pr_info("[AVIN]%s: kthread_should_stop!\r\n", __func__);
			break;
		}
		
		if (NULL_HANDLE == g_hSendBufMsgQ_wch3) {
				pr_info("[AVIN]%s: avm wait MsgQ Create!\r\n", __func__);
				msleep(5);
				continue;
		}
		if(0xFF != u4WchBufIdx){
			i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &g_hSendBufMsgQ_wch3,
				1, X_MSGQ_OPTION_WAIT);
		}else{
			msleep(5);
			continue;
		}

		if (OSR_OK != i4Ret) {
			pr_debug("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
		}else{
			if (u4WchBufIdx < WCH_BUF_MAX_CNT){
				avmBuf[0].yAddr = g_rWchBufferInfo[2].tWchBuf.u4YBuf[u4WchBufIdx];
				avmBuf[0].cAddr = g_rWchBufferInfo[2].tWchBuf.u4CBuf[u4WchBufIdx];
				avmBuf[0].width = BUFFER_WIDTH;
				avmBuf[0].height = BUFFER_HEIGHT;
				
				if(avin_buffer_complete(AVIN_TYPE_AVM_LEFT, (struct capture_priv *)&avmBuf[0])){
					pr_info("[AVIN]%s:buffer complete fial\n", __func__);
					msleep(5);
					continue;
				}
			}
		}

	}
	pr_info("[AVIN]%s: thread3 exit success!\r\n", __func__);

	return 0;
}

static int tvd_wch4dram_thread(void *data){
	int32_t i4Ret = -1;
	uint32_t u4WchBufIdx = 0;
	u32 u2MsgIdx = 0;
	u32 z_msg_size = sizeof(uint32_t);
	struct avmbuf avmBuf[AVM_MAX_VIDEO_DEVS];
	
	while (true) {
		pr_debug("[AVIN]%s: tvd_wchdram_thread!\r\n", __func__);
		if (kthread_should_stop()) {
			pr_info("[AVIN]%s: kthread_should_stop!\r\n", __func__);
			break;
		}
			
		if (NULL_HANDLE == g_hSendBufMsgQ_wch4) {
			pr_info("[AVIN]%s: avm wait MsgQ Create!\r\n", __func__);
			msleep(5);
			continue;
		}
		if(0xFF != u4WchBufIdx){
			i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &g_hSendBufMsgQ_wch4,
				1, X_MSGQ_OPTION_WAIT);
		}else{
			msleep(5);
			continue;
		}

		if (OSR_OK != i4Ret) {
			pr_debug("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
		}else{
			if (u4WchBufIdx < WCH_BUF_MAX_CNT){
				avmBuf[0].yAddr = g_rWchBufferInfo[3].tWchBuf.u4YBuf[u4WchBufIdx];
				avmBuf[0].cAddr = g_rWchBufferInfo[3].tWchBuf.u4CBuf[u4WchBufIdx];
				avmBuf[0].width = BUFFER_WIDTH;
				avmBuf[0].height = BUFFER_HEIGHT;
				if(avin_buffer_complete(AVIN_TYPE_AVM_RIGHT, (struct capture_priv *)&avmBuf[0])){
					pr_info("[AVIN]%s:buffer complete fial\n", __func__);
					msleep(5);
					continue;
				}
			}
		}

	}
	pr_info("[AVIN]%s: thread4 exit success!\r\n", __func__);

	return 0;
}

static int tvd_wch5dram_thread(void *data){
	int32_t i4Ret = -1;
	uint32_t u4WchBufIdx = 0;
	u32 u2MsgIdx = 0;
	u32 z_msg_size = sizeof(uint32_t);
	struct avmbuf avmBuf[AVM_MAX_VIDEO_DEVS];

	while (true) {
		pr_debug("[AVIN]%s: tvd_wchdram_thread!\r\n", __func__);
		
		if (kthread_should_stop()) {
			pr_info("[AVIN]%s: kthread_should_stop!\r\n", __func__);
			break;
		}
		if (NULL_HANDLE == g_hSendBufMsgQ_wch5) {
			pr_info("[AVIN]%s: avm wait MsgQ Create!\r\n", __func__);
			msleep(5);
			continue;
		}
		if(0xFF != u4WchBufIdx){
			i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &g_hSendBufMsgQ_wch5,
				1, X_MSGQ_OPTION_NOWAIT);
		}else{
			msleep(5);
			continue;
		}

		if (OSR_OK != i4Ret || 0xFF == u4WchBufIdx) {
			pr_debug("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
		}else{
			if (u4WchBufIdx < WCH_BUF_MAX_CNT){
				avmBuf[0].yAddr = g_rWchBufferInfo[4].tWchBuf.u4YBuf[u4WchBufIdx];
				avmBuf[0].cAddr = g_rWchBufferInfo[4].tWchBuf.u4CBuf[u4WchBufIdx];
				avmBuf[0].width = 720;
				avmBuf[0].height = 480;
				if(avin_buffer_complete(AVIN_TYPE_AVM_SIGVIEW, (struct capture_priv *)&avmBuf[0])){
					pr_info("[AVIN]%s:buffer complete fial\n", __func__);
					msleep(5);
					continue;
				}
			}
		}
	}
	pr_info("[AVIN]%s: thread4 exit success!\r\n", __func__);
	
	return 0;
}

s32 avm_callback_start(void)
{
	int i4Ret = 0;
	pr_info("[AVIN]%s:\r\n", __func__);
	mutex_lock(&startLock);
	if(avmStop[0] == false || avmStop[1] == false ||
		avmStop[2] == false || avmStop[3] == false || hwReq == false){
		pr_info("[AVIN]%s: avm already start port\r\n", __func__);
		mutex_unlock(&startLock);
		return 0;
	}
	avm_init_video(0);
	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch1, "MSGQ_WCH1", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail wch1!\r\n", __func__);
		mutex_unlock(&startLock);
		return -1;
	}

	g_Thread_task1 = kthread_create(tvd_wch1dram_thread, NULL, "TVD_Wch1Dram_Thread");
	if (IS_ERR(g_Thread_task1))
	{
		i4Ret = PTR_ERR(g_Thread_task1);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task1 = NULL;
		mutex_unlock(&startLock);
		return i4Ret;
	}
	wake_up_process(g_Thread_task1);
	if (WCH_SUCCESS != OpenWch(wchId[0], SRC_APP_AVM_WCH1)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		mutex_unlock(&startLock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);
	avm_tvd_control(TVD_CH_0, TVD_CONTROL_CODE_INIT);

	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch2, "MSGQ_WCH2", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail wch2!\r\n", __func__);
		mutex_unlock(&startLock);
		return -1;
	}

	g_Thread_task2 = kthread_create(tvd_wch2dram_thread, NULL, "TVD_Wch2Dram_Thread");
	if (IS_ERR(g_Thread_task2))
	{
		i4Ret = PTR_ERR(g_Thread_task2);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task2 = NULL;
		mutex_unlock(&startLock);
		return i4Ret;
	}
	wake_up_process(g_Thread_task2);
	if (WCH_SUCCESS != OpenWch(wchId[1], SRC_APP_AVM_WCH2)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		mutex_unlock(&startLock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);	
	avm_tvd_control(TVD_CH_1, TVD_CONTROL_CODE_INIT);

	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch3, "MSGQ_WCH3", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail wch3!\r\n", __func__);
		mutex_unlock(&startLock);
		return -1;
	}

	g_Thread_task3 = kthread_create(tvd_wch3dram_thread, NULL, "TVD_Wch3Dram_Thread");
	if (IS_ERR(g_Thread_task3))
	{
		i4Ret = PTR_ERR(g_Thread_task3);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task3 = NULL;
		mutex_unlock(&startLock);
		return i4Ret;
	}
	wake_up_process(g_Thread_task3);
	if (WCH_SUCCESS != OpenWch(wchId[2], SRC_APP_AVM_WCH3)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		mutex_unlock(&startLock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);	
	avm_tvd_control(TVD_CH_2, TVD_CONTROL_CODE_INIT);

	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch4, "MSGQ_WCH4", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail wch4!\r\n", __func__);
		mutex_unlock(&startLock);
		return -1;
	}

	g_Thread_task4 = kthread_create(tvd_wch4dram_thread, NULL, "TVD_Wch4Dram_Thread");
	if (IS_ERR(g_Thread_task4))
	{
		i4Ret = PTR_ERR(g_Thread_task4);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task4 = NULL;
		mutex_unlock(&startLock);
		return i4Ret;
	}
	wake_up_process(g_Thread_task4);
	if (WCH_SUCCESS != OpenWch(wchId[3], SRC_APP_AVM_WCH4)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		mutex_unlock(&startLock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);
	avm_tvd_control(TVD_CH_3, TVD_CONTROL_CODE_INIT);
	avmStop[0] = false;
	avmStop[1] = false;
	avmStop[2] = false;
	avmStop[3] = false;
	stopCount  = 0;
	mutex_unlock(&startLock);
	/*mutex_init(&g_Wch5Lock);
	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch5, "MSGQ_WCH5", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail wch5!\r\n", __func__);
		return -1;
	}

	g_Thread_task5 = kthread_create(tvd_wch5dram_thread, NULL, "TVD_Wch4Dram_Thread");
	if (IS_ERR(g_Thread_task5))
	{
		i4Ret = PTR_ERR(g_Thread_task5);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task5 = NULL;
		return i4Ret;
	}

	mutex_lock(&g_Wch5Lock);
	g_fgExitThread5 = false;
	wake_up_process(g_Thread_task5);
		
	mutex_unlock(&g_Wch5Lock);*/
}

s32 avm_callback_stop(void)
{
	int i4Ret = 0;
	uint32_t u4WchBufIdx = 0xFF;

	pr_info("[AVIN]%s: enter !\r\n", __func__);
	mutex_lock(&startLock);
	
	if(avmStop[0] == false){
		i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch1, &u4WchBufIdx, sizeof(uint32_t), 1);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			mutex_unlock(&startLock);
			return -1;
		}
		if (!IS_ERR(g_Thread_task1))
		{
			kthread_stop(g_Thread_task1);
		}

		g_Thread_task1 = NULL;
		if (avm_tvd_control(TVD_CH_0, TVD_CONTROL_CODE_STOP)) {
			pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
			mutex_unlock(&startLock);
			return -1;
		}
		if (NULL_HANDLE != g_hSendBufMsgQ_wch1) {
			if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch1)) {
				pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
			}
			g_hSendBufMsgQ_wch1= NULL_HANDLE;
		}
	}
	pr_info("[AVIN]%s: TVD_CH0 stop success!\r\n", __func__);

	if(avmStop[1] == false){
		i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch2, &u4WchBufIdx, sizeof(uint32_t), 1);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			mutex_unlock(&startLock);
			return -1;
		}
		if (!IS_ERR(g_Thread_task2))
		{
			kthread_stop(g_Thread_task2);
		}
		g_Thread_task2 = NULL;
		
		if (avm_tvd_control(TVD_CH_1, TVD_CONTROL_CODE_STOP)) {
			pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
			mutex_unlock(&startLock);
			return -1;
		}
		if (NULL_HANDLE != g_hSendBufMsgQ_wch2) {
			if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch2)) {
				pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
			}
			g_hSendBufMsgQ_wch2= NULL_HANDLE;
		}
	}

	if(avmStop[2] == false){
		i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch3, &u4WchBufIdx, sizeof(uint32_t), 1);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			mutex_unlock(&startLock);
			return -1;
		}
		if (!IS_ERR(g_Thread_task3))
		{
			kthread_stop(g_Thread_task3);
		}
		g_Thread_task3 = NULL;
		
		if (avm_tvd_control(TVD_CH_2, TVD_CONTROL_CODE_STOP)) {
			pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
			mutex_unlock(&startLock);
			return -1;
		}
		if (NULL_HANDLE != g_hSendBufMsgQ_wch3) {
			if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch3)) {
				pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
			}
			g_hSendBufMsgQ_wch3= NULL_HANDLE;
		}
	}

	if(avmStop[3] == false){
		i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch4, &u4WchBufIdx, sizeof(uint32_t), 1);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
			mutex_unlock(&startLock);
			return -1;
		}
		if (!IS_ERR(g_Thread_task4))
		{
			kthread_stop(g_Thread_task4);
		}
		g_Thread_task4 = NULL;
		if (avm_tvd_control(TVD_CH_3, TVD_CONTROL_CODE_STOP)) {
			pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
			mutex_unlock(&startLock);
			return -1;
		}
		if (NULL_HANDLE != g_hSendBufMsgQ_wch4) {
			if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch4)) {
			pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
			}
			g_hSendBufMsgQ_wch4= NULL_HANDLE;
		}
	}
	avmInit = false;
	avmStop[0] = true;
	avmStop[1] = true;
	avmStop[2] = true;
	avmStop[3] = true;
	mutex_unlock(&startLock);
	pr_info("[AVIN]%s: leave !\r\n", __func__);
	/*mutex_lock(&g_Wch5Lock);
	avmInit = false;
	gThdCret = false;
	g_fgExitThread5 = true;
	g_Thread_task5 = NULL;
	mutex_unlock(&g_Wch5Lock);
	vdoCloseWch(WCH_5);
	if (NULL_HANDLE != g_hSendBufMsgQ_wch5) {
		if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch5)) {
		pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
		}
		g_hSendBufMsgQ_wch5= NULL_HANDLE;
	}*/
}

int avm_start_video(TVD_CHANNEL_ID channel_id){
	pr_info("[AVIN]%s: port(%d)\r\n", __func__, channel_id);
	int i4Ret = 0;
	mutex_lock(&startLock);
	if(avmStop[channel_id] == false){
		pr_info("[AVIN]%s: avm already start port(%d)\r\n", __func__, channel_id);
		mutex_unlock(&startLock);
		return 0;
	}
	if(hwReq == false){
		pr_info("[AVIN]%s:HW Request enter  !\r\n", __func__);
		i4Ret = TWMgr_requestHw(SRC_AVM, HW_TVD_ALL, avm_callback_start, avm_callback_stop);
		if(i4Ret < 0){
			pr_err("[AVIN]%s:HW Request Fail !\r\n", __func__);
			mutex_unlock(&startLock);
			return -1;
		}
		avm_init_video(0);
		wchID = i4Ret;
		hwReq = true;
	}
	avmStop[channel_id] = false;
	
	switch(channel_id){
		case TVD_CH_0:
			if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch1, "MSGQ_WCH1", sizeof(uint32_t), 40)) {
				pr_err("[AVIN]%s: x_msg_q_create fail wch1!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}

			g_Thread_task1 = kthread_create(tvd_wch1dram_thread, NULL, "TVD_Wch1Dram_Thread");
			if (IS_ERR(g_Thread_task1))
			{
				i4Ret = PTR_ERR(g_Thread_task1);
				pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
				g_Thread_task1 = NULL;
				mutex_unlock(&startLock);
				return i4Ret;
			}
			wake_up_process(g_Thread_task1);
			if (WCH_SUCCESS != OpenWch(wchId[channel_id], SRC_APP_AVM_WCH1)) {
				pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return;
			}
			pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);
			avm_tvd_control(channel_id, TVD_CONTROL_CODE_INIT);
			break;

		case TVD_CH_1:
			if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch2, "MSGQ_WCH2", sizeof(uint32_t), 40)) {
				pr_err("[AVIN]%s: x_msg_q_create fail wch2!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}

			g_Thread_task2 = kthread_create(tvd_wch2dram_thread, NULL, "TVD_Wch2Dram_Thread");
			if (IS_ERR(g_Thread_task2))
			{
				i4Ret = PTR_ERR(g_Thread_task2);
				pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
				g_Thread_task2 = NULL;
				mutex_unlock(&startLock);
				return i4Ret;
			}
			wake_up_process(g_Thread_task2);
			if (WCH_SUCCESS != OpenWch(wchId[channel_id], SRC_APP_AVM_WCH2)) {
				pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return;
			}
			avm_tvd_control(channel_id, TVD_CONTROL_CODE_INIT);
			break;

		case TVD_CH_2:
			if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch3, "MSGQ_WCH3", sizeof(uint32_t), 40)) {
				pr_err("[AVIN]%s: x_msg_q_create fail wch3!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}

			g_Thread_task3 = kthread_create(tvd_wch3dram_thread, NULL, "TVD_Wch3Dram_Thread");
			if (IS_ERR(g_Thread_task3))
			{
				i4Ret = PTR_ERR(g_Thread_task3);
				pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
				g_Thread_task3 = NULL;
				mutex_unlock(&startLock);
				return i4Ret;
			}
			wake_up_process(g_Thread_task3);
			if (WCH_SUCCESS != OpenWch(wchId[channel_id], SRC_APP_AVM_WCH3)) {
				pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return;
			}
			avm_tvd_control(channel_id, TVD_CONTROL_CODE_INIT);
			break;

		case TVD_CH_3:
			if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch4, "MSGQ_WCH4", sizeof(uint32_t), 40)) {
				pr_err("[AVIN]%s: x_msg_q_create fail wch4!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}

			g_Thread_task4 = kthread_create(tvd_wch4dram_thread, NULL, "TVD_Wch4Dram_Thread");
			if (IS_ERR(g_Thread_task4))
			{
				i4Ret = PTR_ERR(g_Thread_task4);
				pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
				g_Thread_task4 = NULL;
				mutex_unlock(&startLock);
				return i4Ret;
			}
			wake_up_process(g_Thread_task4);
			if (WCH_SUCCESS != OpenWch(wchId[channel_id], SRC_APP_AVM_WCH4)) {
				pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return;
			}
			avm_tvd_control(channel_id, TVD_CONTROL_CODE_INIT);
			break;

		case 4:
			if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ_wch5, "MSGQ_WCH5", sizeof(uint32_t), 40)) {
				pr_err("[AVIN]%s: x_msg_q_create fail wch5!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}

			g_Thread_task5 = kthread_create(tvd_wch5dram_thread, NULL, "TVD_Wch4Dram_Thread");
			if (IS_ERR(g_Thread_task5))
			{
				i4Ret = PTR_ERR(g_Thread_task5);
				pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
				g_Thread_task5 = NULL;
				mutex_unlock(&startLock);
				return i4Ret;
			}
			wake_up_process(g_Thread_task5);
			break;
		default:
			break;
	}

	pr_info("[AVIN]%s: success with cvbs_type\n", __func__);
	mutex_unlock(&startLock);
	return 0;
 }

int avm_stop_video(TVD_CHANNEL_ID channel_id){
	int i4Ret = 0;
	uint32_t u4WchBufIdx = 0xFF;
	pr_info("[AVIN]%s: tvd_channel %d\r\n", __func__,channel_id);
	mutex_lock(&startLock);
	stopCount++;
	if(avmStop[channel_id] == true){
		pr_err("[AVIN]%s: tvd_channel already stoped !\r\n", __func__);
		if(hwReq == true && stopCount == 4){
			hwReq = false;
			stopCount = 0;
			mutex_unlock(&startLock);
			pr_info("[AVIN]%s: TWMgr_releaseHw enter\r\n", __func__);
			wchID = TWMgr_releaseHw(SRC_AVM);
			pr_info("[AVIN]%s: TWMgr_releaseHw leave\r\n", __func__);
			mutex_lock(&startLock);
		}
		mutex_unlock(&startLock);
		return 0;
	}
	
	switch(channel_id){
		case TVD_CH_0:
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch1, &u4WchBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret) {
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
				mutex_unlock(&startLock);
				return -1;
			}
			if (!IS_ERR(g_Thread_task1))
			{
				kthread_stop(g_Thread_task1);
			}
			
			avmInit = false;
			g_Thread_task1 = NULL;
			if (avm_tvd_control(channel_id, TVD_CONTROL_CODE_STOP)) {
				pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}
			
			if (NULL_HANDLE != g_hSendBufMsgQ_wch1) {
				if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch1)) {
					pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
				}
				g_hSendBufMsgQ_wch1= NULL_HANDLE;
			}
			
			break;

		case TVD_CH_1:
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch2, &u4WchBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret) {
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
				mutex_unlock(&startLock);
				return -1;
			}
			if (!IS_ERR(g_Thread_task2))
			{
				kthread_stop(g_Thread_task2);
			}
			
			avmInit = false;
			g_Thread_task2 = NULL;
			if (avm_tvd_control(channel_id, TVD_CONTROL_CODE_STOP)) {
				pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}
			
			if (NULL_HANDLE != g_hSendBufMsgQ_wch2) {
				if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch2)) {
					pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
				}
				g_hSendBufMsgQ_wch2= NULL_HANDLE;
			}
			
			break;

		case TVD_CH_2:
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch3, &u4WchBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret) {
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
				mutex_unlock(&startLock);
				return -1;
			}
			if (!IS_ERR(g_Thread_task3))
			{
				kthread_stop(g_Thread_task3);
			}

			avmInit = false;
			g_Thread_task3 = NULL;
			
			if (avm_tvd_control(channel_id, TVD_CONTROL_CODE_STOP)) {
				pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}
			
			if (NULL_HANDLE != g_hSendBufMsgQ_wch3) {
				if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch3)) {
					pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
				}
				g_hSendBufMsgQ_wch3= NULL_HANDLE;
			}
			
			break;

		case TVD_CH_3:
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch4, &u4WchBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret) {
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
				mutex_unlock(&startLock);
				return -1;
			}

			if (!IS_ERR(g_Thread_task4))
			{
				kthread_stop(g_Thread_task4);
			}
			
			avmInit = false;
			g_Thread_task4 = NULL;
			if (avm_tvd_control(channel_id, TVD_CONTROL_CODE_STOP)) {
				pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
				mutex_unlock(&startLock);
				return -1;
			}
			
			if (NULL_HANDLE != g_hSendBufMsgQ_wch4) {
				if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch4)) {
				pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
				}
				g_hSendBufMsgQ_wch4= NULL_HANDLE;
			}
			pr_info("[AVIN]%s: TVD_CH_3 stop success!\r\n", __func__);
			break;

		case 4:
			i4Ret = x_msg_q_send(g_hSendBufMsgQ_wch5, &u4WchBufIdx, sizeof(uint32_t), 1);
			if (OSR_OK != i4Ret) {
				pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
				mutex_unlock(&startLock);
				return -1;
			}
			
			if (!IS_ERR(g_Thread_task5))
			{
				kthread_stop(g_Thread_task5);
			}

			avmInit = false;
			g_Thread_task5 = NULL;
			vdoCloseWch(WCH_5);
			if (NULL_HANDLE != g_hSendBufMsgQ_wch5) {
				if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ_wch5)) {
				pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
				}
				g_hSendBufMsgQ_wch5= NULL_HANDLE;
			}
			
			break;	
			
		default:
			break;
	}
	avmStop[channel_id] = true;
	if(hwReq == true && stopCount == 4){
		hwReq = false;
		stopCount = 0;
		mutex_unlock(&startLock);
		pr_info("[AVIN]%s: TWMgr_releaseHw enter\r\n", __func__);
		wchID = TWMgr_releaseHw(SRC_AVM);
		mutex_lock(&startLock);
	}
	pr_info("[AVIN]%s: TWMgr_releaseHw leave\r\n", __func__);
	mutex_unlock(&startLock);
	pr_info("[AVIN]%s: success with cvbs_type\r\n", __func__);

	return 0;
}

int avm_set_input(int index){
	u32 i4Idx = 0;
	u32 u4Size = 0;
	u32 iRet = 0;
	
	if(wch_status[4] == WCH_STARTED){
		iRet = StopWch(wchId[4], g_rWchCtrl[4].eSrcId);
		if(iRet){
			pr_debug("[AVIN]%s: StopWch fail!\r\n", __func__);
			return -1;
		}
		wch_status[4] = WCH_STOPPED;
	}
	
	memset(&g_rWchCtrl[4], 0, sizeof(WCH_CFG_T));
	g_rWchCtrl[4].fgVSyncPolarity = false; /* FALSE is LOW level present sync.*/
	g_rWchCtrl[4].fgHSyncPolarity = true; /* TRUE is High.*/

	g_rWchCtrl[4].eInputFmt = DATA_FMT_YUV444;
	g_rWchCtrl[4].fgProgressive = false;
	g_rWchCtrl[4].eOutputFmt = DATA_FMT_YUV420;
	g_rWchCtrl[4].u4ScanLineMode = 1;
	g_rWchCtrl[4].u1WchId = WCH_5;
	g_rWchCtrl[4].eSrcId = SRC_APP_AVM_WCH5;
	switch(index){
		case AVIN_TYPE_AVM_REAR:
			g_rWchCtrl[4].eInputSrc = DATA_SRC_TVD0;
			break;
		case AVIN_TYPE_AVM_FRONT:
			g_rWchCtrl[4].eInputSrc = DATA_SRC_TVD1;
			break;
		case AVIN_TYPE_AVM_LEFT:
			g_rWchCtrl[4].eInputSrc = DATA_SRC_TVD2;
			break;
		case AVIN_TYPE_AVM_RIGHT:
			g_rWchCtrl[4].eInputSrc = DATA_SRC_TVD3;
			break;
		default:
			break;
	}
	
	switch (cvbsMode[index]) {
	case AV_MODE_PAL:
		mWidth[4] = PAL_FRAME_WIDTH;
		mHeight[4] = PAL_FRAME_HEIGHT;
		g_rWchCtrl[4].fgBotFieldFirst = 0;
		g_rWchCtrl[4].u4SrcStartYTop = 2;
		g_rWchCtrl[4].u4SrcStartYBot = 2;
		g_rWchCtrl[4].u4SrcStartX = 0x3B;
		
		pr_info("[AVIN]%s: CVBS signal system is PAL\r\n", __func__);
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_NTSC:
		mWidth[4] = NTSC_FRAME_WIDTH;
		mHeight[4] = NTSC_FRAME_HEIGHT;
		g_rWchCtrl[4].fgBotFieldFirst = 1;
		g_rWchCtrl[4].u4SrcStartYTop = 0;
		g_rWchCtrl[4].u4SrcStartYBot = 0;
		pr_info("[AVIN]%s: CVBS signal system is NTSC\r\n", __func__);
		break;

	case AV_MODE_SECAM:
		mWidth[4] = SECAM_FRAME_WIDTH;
		mHeight[4] = SECAM_FRAME_HEIGHT;
		g_rWchCtrl[4].fgBotFieldFirst = 0;
		g_rWchCtrl[4].u4SrcStartYTop = 1;
		g_rWchCtrl[4].u4SrcStartYBot = 1;
		pr_info("[AVIN]%s: CVBS signal system is SECAM\r\n", __func__);
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		pr_err("[AVIN]%s: Get Error CVBS signal system value\r\n", __func__);
		return;
	}

	//mutex_lock(&g_Lock);

	g_rWchCtrl[4].u4SrcWidth = mWidth[4];
	g_rWchCtrl[4].u4SrcHeight = mHeight[4];
	g_rWchCtrl[4].u4DstWidth = mWidth[4];
	g_rWchCtrl[4].u4DstHeight = mHeight[4];
	g_rWchCtrl[4].GetWchBufIndx = wch_buffer_get4;
	if (WCH_SUCCESS != OpenWch(wchId[4], g_rWchCtrl[4].eSrcId)) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		//mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);
	
	if (ConfigWch(&g_rWchCtrl[4])) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		//mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rWchBufferInfo[4], 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo[4].u1WchId= wchId[4];
	if (WchGetBufferAddress(&g_rWchBufferInfo[4])) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		//mutex_unlock(&g_Lock);
		return;
	}else{
		for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
			pr_debug("[AVIN]%s: Physic Y(%08x) C(%08x)\r\n", __func__,
				g_rWchBufferInfo[4].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[4].tWchBuf.u4CBuf[i4Idx]);
			g_rWchBufferInfo[4].tWchBuf.u4YBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[4].tWchBuf.u4YBuf[i4Idx]);
			
			g_rWchBufferInfo[4].tWchBuf.u4CBuf[i4Idx] = phys_to_virt(g_rWchBufferInfo[4].tWchBuf.u4CBuf[i4Idx]);
			pr_debug("[AVIN]%s: virtual Y(%08x) C(%08x)\r\n", __func__,
				g_rWchBufferInfo[4].tWchBuf.u4YBuf[i4Idx], g_rWchBufferInfo[4].tWchBuf.u4CBuf[i4Idx]);
		}
	}
	pr_info("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

	if (StartWch(wchId[4], g_rWchCtrl[4].eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		//mutex_unlock(&g_Lock);
		return;
	}
	wch_status[4] = WCH_STARTED;
	//mutex_unlock(&g_Lock);
	pr_info("[AVIN]%s: StartWch success!\r\n", __func__);

	pr_info("[AVIN]%s: avm_set_input on leave!\r\n", __func__);
	return 0;
}

int avm_init_video(int index){
	pr_info("[AVIN]%s: Enter\n",  __func__);
	if(avmInit == false){
		tvd_register_notify(tvd_signal_status, NULL);
		avmInit = true;
	} else{
		pr_info("[AVIN]%s: avm already init\n",__func__);
	}
	pr_info("[AVIN]%s: Leave\n",  __func__);
	return 0;
}

int avm_init(void){
	mutex_init(&startLock);
	mutex_init(&g_Lock);
	return 0;
}
int avm_get_std(uint8_t chid, v4l2_std_id *pstd_type){
	
	if (chid >= AVM_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}
	if (NULL == pstd_type) {
		pr_err("[AVIN]%s: param pstd_type is NULL!\r\n", __func__);
		return -1;
	}
	if(cvbsMode[chid]== AV_MODE_NTSC) {
		*pstd_type = V4L2_STD_NTSC;
	}else if(cvbsMode[chid]== AV_MODE_PAL){
		*pstd_type = V4L2_STD_PAL;
	}else {
		*pstd_type = V4L2_STD_UNKNOWN;
		pr_err("[AVIN]%s: param source std is unknow!\r\n", __func__);
	}
	
	return 0;
}

int avm_tvd_control(TVD_CHANNEL_ID channel_id, int CtrlCode)
{
	bool ret = 0;

	switch (CtrlCode) {
	case TVD_CONTROL_CODE_INIT: {
		TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T rTvdDrvInitPara;
		pr_info("[AVIN]%s: TVD_CONTROL_CODE_INIT with tvd_channel(%d)\r\n", __func__, channel_id);
		memset(&rTvdDrvInitPara, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
		rTvdDrvInitPara.source_type = TVD_APP_ID_AVM;
		rTvdDrvInitPara.eVdoInFmt = TVD_VDOFMT_YUV444;
		rTvdDrvInitPara.eVdoOutFmt = TVD_VDOFMT_YUV420;
		rTvdDrvInitPara.u4CHACvbsInxP = CVBSIN_1P;
		rTvdDrvInitPara.u4CHBCvbsInxP = CVBSIN_NONE;
		rTvdDrvInitPara.u4CHAOutDest = TVD_CFG_OUT_DRAM;
		rTvdDrvInitPara.u4CHBOutDest = TVD_CFG_OUT_NONE;
		rTvdDrvInitPara.channel_id = channel_id;
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_INIT, (u8 *)&rTvdDrvInitPara, NULL);
	}
	break;

	case TVD_CONTROL_CODE_CONFIG: {
		TVD_DRV_CAMERA_PREVIEW_CFG_T rTvdDrvCfgPara;

		pr_debug("[AVIN]%s: TVD_CONTROL_CODE_CONFIG\r\n", __func__);
		memset(&rTvdDrvCfgPara, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_CFG_T));
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_CONFIG, (u8 *)&rTvdDrvCfgPara, NULL);
	}
	break;

	case TVD_CONTROL_CODE_START: {
		pr_debug("[AVIN]%s: TVD_CONTROL_CODE_START\r\n", __func__);
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_START, NULL, NULL);
	}
	break;

	case TVD_CONTROL_CODE_STOP: {
		TVD_DRV_STOP_PARA_T rTvdDrvStopPara;

		pr_debug("[AVIN]%s: TVD_CONTROL_CODE_STOP with tvd_channel(%d)\r\n", __func__, channel_id);
		memset(&rTvdDrvStopPara, 0, sizeof(TVD_DRV_STOP_PARA_T));
		rTvdDrvStopPara.stop_channel = TVD_CHA;
		rTvdDrvStopPara.source_type = TVD_APP_ID_AVM;
		rTvdDrvStopPara.channel_id = channel_id;
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_STOP, &rTvdDrvStopPara, NULL);
	}
	break;

	default:
		pr_debug("[AVIN]%s: invalid control code!", __func__);
		return false;
	}
	pr_debug("[AVIN]%s: end with ret(%d)", __func__, ret);

	return (ret > 0);
}

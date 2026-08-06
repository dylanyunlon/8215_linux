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

#include <generated/atc_project.h>
#include "cvbs_hal.h"
#include "audio_hal.h"
#include "avin_common.h"
#include "tvd_wch_mgr.h"
#include "ac823x_tvd/tvd_data_struct.h"
#ifdef CVBS_NR_ENABLE
#include "nr_drv.h"
#include "nr_if.h"
#endif

#define WCH_IDLE		1
#define WCH_STARTED		2
#define WCH_STOPPED		3

#define CVBS_NORMAL		(1 << 0)
#define CVBS_BACKCAR	(1 << 1)
#define CVBS_AVM  (1<<2)

#ifdef CVBS_NR_ENABLE
static NR_ALLC_BUF_T g_rNRBufferInfo[4];
static NR_PRM_T g_rNRParamInfo[4];
#endif
static WCH_BUF_T g_rWchBufferInfo[4];
static WCH_CFG_T g_rWchCtrl[4];
static int mWidth[4];
static int mHeight[4];
static TVD_SIG_STATE_T mSigState[4];
static int videoport = CVBSIN_1P;
static int audioport = CVBSIN_1P;
static u32 wch_status[4] = {WCH_STOPPED, WCH_STOPPED, WCH_STOPPED, WCH_STOPPED};
static u32 tvdType[6] = {HW_TVD0, HW_TVD0, HW_TVD0, HW_TVD1, HW_TVD2, HW_TVD3};
static int g_cvbs_type = 0;
static u32 g_cvbs_mirror = 0;
static u32 g_backcar_mirror = 0;
static u32 g_avm_mirror = 0;
static s32 wchID[4] = {-1};
static bool cvbsStart = false;
static struct mutex g_Lock;

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);

#ifdef CVBS_NR_ENABLE
static void nr_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;
	u32 chIdx = 0;

	if ((NULL == pBufIdx) || (*pBufIdx >= NR_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: nr bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[chIdx]);
	if ((TVD_SIG_READY == mSigState[chIdx]) || (TVD_SIG_CHANGE_DONE == mSigState[chIdx])) {
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rNRBufferInfo[chIdx].u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rNRBufferInfo[chIdx].u4CBuf[*pBufIdx];
		data.buf_height = mHeight[chIdx];
		data.buf_width = mWidth[chIdx];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_0);
		data.need_hide = false;
		if (g_cvbs_type & CVBS_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
	} else {
		pr_debug("[AVIN]%s: the nr data must be discard with signal status is %d\n",
			__func__, mSigState[chIdx]);
	}
}

static void nr_buffer_get1(u32 *pBufIdx) {
	struct capture_priv data;
	u32 chIdx = 1;

	if ((NULL == pBufIdx) || (*pBufIdx >= NR_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: nr bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[chIdx]);
	if ((TVD_SIG_READY == mSigState[chIdx]) || (TVD_SIG_CHANGE_DONE == mSigState[chIdx])) {
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rNRBufferInfo[chIdx].u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rNRBufferInfo[chIdx].u4CBuf[*pBufIdx];
		data.buf_height = mHeight[chIdx];
		data.buf_width = mWidth[chIdx];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_1);
		data.need_hide = false;
		if (g_cvbs_type & CVBS_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
	} else {
		pr_debug("[AVIN]%s: the nr data must be discard with signal status is %d\n",
			__func__, mSigState[chIdx]);
	}
}

static void nr_buffer_get2(u32 *pBufIdx) {
	struct capture_priv data;
	u32 chIdx = 2;

	if ((NULL == pBufIdx) || (*pBufIdx >= NR_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: nr bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[chIdx]);
	if ((TVD_SIG_READY == mSigState[chIdx]) || (TVD_SIG_CHANGE_DONE == mSigState[chIdx])) {
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rNRBufferInfo[chIdx].u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rNRBufferInfo[chIdx].u4CBuf[*pBufIdx];
		data.buf_height = mHeight[chIdx];
		data.buf_width = mWidth[chIdx];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_2);
		data.need_hide = false;
		if (g_cvbs_type & CVBS_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
	} else {
		pr_debug("[AVIN]%s: the nr data must be discard with signal status is %d\n",
			__func__, mSigState[chIdx]);
	}
}

static void nr_buffer_get3(u32 *pBufIdx) {
	struct capture_priv data;
	u32 chIdx = 3;

	if ((NULL == pBufIdx) || (*pBufIdx >= NR_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: nr bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[chIdx]);
	if ((TVD_SIG_READY == mSigState[chIdx]) || (TVD_SIG_CHANGE_DONE == mSigState[chIdx])) {
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rNRBufferInfo[chIdx].u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rNRBufferInfo[chIdx].u4CBuf[*pBufIdx];
		data.buf_height = mHeight[chIdx];
		data.buf_width = mWidth[chIdx];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_3);
		data.need_hide = false;
		if (g_cvbs_type & CVBS_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
	} else {
		pr_debug("[AVIN]%s: the nr data must be discard with signal status is %d\n",
			__func__, mSigState[chIdx]);
	}
}
#endif

static void wch_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;

	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[0]);
	if ((TVD_SIG_READY == mSigState[0]) || (TVD_SIG_CHANGE_DONE == mSigState[0])) {
#ifdef CVBS_NR_ENABLE
		NrReciveWchBuffer(*pBufIdx);
#else
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rWchBufferInfo[0].tWchBuf.u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rWchBufferInfo[0].tWchBuf.u4CBuf[*pBufIdx];
		data.buf_height = mHeight[0];
		data.buf_width = mWidth[0];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_0);
		data.need_hide = false;
		if (g_cvbs_type & CVBS_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
#endif
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[0]);
	}
}

static void wch_buffer_get1(u32 *pBufIdx) {
	struct capture_priv data;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[1]);
	
	if ((TVD_SIG_READY == mSigState[1]) || (TVD_SIG_CHANGE_DONE == mSigState[1])) {
#ifdef CVBS_NR_ENABLE
		NrReciveWchBuffer(*pBufIdx);
#else
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rWchBufferInfo[1].tWchBuf.u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rWchBufferInfo[1].tWchBuf.u4CBuf[*pBufIdx];
		data.buf_height = mHeight[1];
		data.buf_width = mWidth[1];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_1);
		data.need_hide = false;
		//pr_debug("[AVIN]%s: wch bufIdx is %d, yaddr=0x%x caddr=0x%x\n",
		//__func__, *pBufIdx, data.ycaddr.y,data.ycaddr.c);
		if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
#endif
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[1]);
	}
}

static void wch_buffer_get2(u32 *pBufIdx) {
	struct capture_priv data;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[2]);
	
	if ((TVD_SIG_READY == mSigState[2]) || (TVD_SIG_CHANGE_DONE == mSigState[2])) {
#ifdef CVBS_NR_ENABLE
		NrReciveWchBuffer(*pBufIdx);
#else
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rWchBufferInfo[2].tWchBuf.u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rWchBufferInfo[2].tWchBuf.u4CBuf[*pBufIdx];
		data.buf_height = mHeight[2];
		data.buf_width = mWidth[2];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_2);
		data.need_hide = false;
		//pr_debug("[AVIN]%s: wch bufIdx is %d, yaddr=0x%x caddr=0x%x\n",
		//__func__, *pBufIdx, data.ycaddr.y,data.ycaddr.c);
		if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
#endif
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[2]);
	}
}

static void wch_buffer_get3(u32 *pBufIdx) {
	struct capture_priv data;
	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
		return;
	}

	pr_debug("[AVIN]%s: wch bufIdx is %d, signal status is %d\n",
		__func__, *pBufIdx, mSigState[3]);
	
	if ((TVD_SIG_READY == mSigState[3]) || (TVD_SIG_CHANGE_DONE == mSigState[3])) {
#ifdef CVBS_NR_ENABLE
		NrReciveWchBuffer(*pBufIdx);
#else
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = (unsigned int)g_rWchBufferInfo[3].tWchBuf.u4YBuf[*pBufIdx];
		data.ycaddr.c = (unsigned int)g_rWchBufferInfo[3].tWchBuf.u4CBuf[*pBufIdx];
		data.buf_height = mHeight[3];
		data.buf_width = mWidth[3];
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag(TVD_CH_3);
		data.need_hide = false;
		//pr_debug("[AVIN]%s: wch bufIdx is %d, yaddr=0x%x caddr=0x%x\n",
		//__func__, *pBufIdx, data.ycaddr.y,data.ycaddr.c);
		if (g_cvbs_type & CVBS_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else {
			pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
		}
#endif
	} else {
		pr_debug("[AVIN]%s: the wch data must be discard with signal status is %d\n",
			__func__, mSigState[3]);
	}
}

static void vdoCloseWch(TVD_CHANNEL_ID channel_id)
{
	pr_info("[AVIN]%s: enter\n", __func__);
	mutex_lock(&g_Lock);
	if(WCH_STOPPED == wch_status[channel_id]) {
		pr_err("[AVIN]%s: The wch have already stopped channel_id = %d!\n", __func__, channel_id);
		mutex_unlock(&g_Lock);
		return;
	}

	if (StopWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);

#ifdef CVBS_NR_ENABLE
	mutex_unlock(&g_Lock);
	NrDeInit();
	mutex_lock(&g_Lock);
#endif

	if (CloseWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: CloseWch success!\r\n", __func__);
	wch_status[channel_id] = WCH_STOPPED;
	mutex_unlock(&g_Lock);
	mSigState[channel_id] = TVD_SIG_CLOSE_WCH;
}

static void vdoSignal(TVD_CHANNEL_ID channel_id, int mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;
	
	memset(&g_rWchCtrl[channel_id], 0, sizeof(WCH_CFG_T));
	g_rWchCtrl[channel_id].fgVSyncPolarity = false; /* FALSE is LOW level present sync.*/
	g_rWchCtrl[channel_id].fgHSyncPolarity = true; /* TRUE is High.*/

	g_rWchCtrl[channel_id].eInputFmt = DATA_FMT_YUV444;

	g_rWchCtrl[channel_id].fgProgressive = false;
	g_rWchCtrl[channel_id].eOutputFmt = DATA_FMT_YUV420;
	g_rWchCtrl[channel_id].u1WchId = wchID[channel_id];
#ifdef CVBS_NR_ENABLE
	g_rWchCtrl[channel_id].u4ScanLineMode = 0;
#else
	g_rWchCtrl[channel_id].u4ScanLineMode = 1;
#endif
	if(channel_id == TVD_CH_0){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD0;
		if (g_cvbs_type & CVBS_BACKCAR) {
			if (WCH_1 == wchID[channel_id]) {
				g_rWchCtrl[channel_id].eSrcId = SRC_APP_BACKCAR_WCH1;
			} else if (WCH_5 == wchID[channel_id]) {
				g_rWchCtrl[channel_id].eSrcId = SRC_APP_BACKCAR_WCH5;
			}
		}else{
			if (WCH_1 == wchID[channel_id]) {
				g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVIN_WCH1;
			} else if (WCH_5 == wchID[channel_id]) {
				g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVIN_WCH5;
			}
		} 
		
	}else if(channel_id == TVD_CH_1){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD1;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVIN_WCH5;
	}else if(channel_id == TVD_CH_2){
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD2;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVIN_WCH5;
	}else {
		g_rWchCtrl[channel_id].eInputSrc = DATA_SRC_TVD3;
		g_rWchCtrl[channel_id].eSrcId = SRC_APP_AVIN_WCH5;
	}
	
	switch (mode) {
	case AV_MODE_PAL:
		mWidth[channel_id] = PAL_FRAME_WIDTH;
		mHeight[channel_id] = PAL_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 2;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 2;
		g_rWchCtrl[channel_id].u4SrcStartX = 0x3B;
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
		pr_info("[AVIN]%s: CVBS signal system is NTSC\r\n", __func__);
		break;

	case AV_MODE_SECAM:
		mWidth[channel_id] = SECAM_FRAME_WIDTH;
		mHeight[channel_id] = SECAM_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 1;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 1;
		pr_info("[AVIN]%s: CVBS signal system is SECAM\r\n", __func__);
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		pr_err("[AVIN]%s: Get Error CVBS signal system value\r\n", __func__);
		return;
	}

	mutex_lock(&g_Lock);
	if (g_cvbs_type & CVBS_BACKCAR) {
		g_rWchCtrl[channel_id].u4Mirror = g_backcar_mirror;
	} else if (g_cvbs_type & CVBS_NORMAL) {
		g_rWchCtrl[channel_id].u4Mirror = g_cvbs_mirror;
	}else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}

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
	
	if (WCH_SUCCESS != OpenWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: OpenWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: OpenWch success!\r\n", __func__);

#ifdef CVBS_NR_ENABLE
	mutex_unlock(&g_Lock);
	NrInit();
	mutex_lock(&g_Lock);
#endif

	if (ConfigWch(&g_rWchCtrl[channel_id])) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rWchBufferInfo[channel_id], 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo[channel_id].u1WchId= wchID[channel_id];
	if (WchGetBufferAddress(&g_rWchBufferInfo[channel_id])) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_info("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

#ifdef CVBS_NR_ENABLE
	g_rNRParamInfo[channel_id].u1WchId = wchID[channel_id];
	g_rNRParamInfo[channel_id].fgBypassEn = false;
	g_rNRParamInfo[channel_id].u1DemoMode = 0;
	g_rNRParamInfo[channel_id].fgBurstRdEn = false;
	g_rNRParamInfo[channel_id].u1AddrSwapMode = 4;
	g_rNRParamInfo[channel_id].u1FrameMode = 0;
	g_rNRParamInfo[channel_id].u4PicWidth = mWidth[channel_id];
	g_rNRParamInfo[channel_id].u4PicHeight = mHeight[channel_id];
	g_rNRParamInfo[channel_id].u4Strength = 1;
	g_rNRParamInfo[channel_id].u4FNRStrength = 3;
	g_rNRParamInfo[channel_id].u4MNRStrength = 3;
	g_rNRParamInfo[channel_id].u4BNRStrength = 3;
	g_rNRParamInfo[channel_id].fgRangeRemapYEn = false;
	g_rNRParamInfo[channel_id].fgRangeRemapUVEn = false;
	g_rNRParamInfo[channel_id].u4RangeMapY = 0;
	g_rNRParamInfo[channel_id].u4RangeMapUV = 0;
	g_rNRParamInfo[channel_id].fgNoiseMeterEn = false;
	g_rNRParamInfo[channel_id].fgUseBlockMeter = false;
	if(channel_id == TVD_CH_0){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get;
	}else if(channel_id == TVD_CH_1){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get1;
	}else if(channel_id == TVD_CH_2){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get2;
	}else {
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get3;
	}
	NrSetParam(&g_rNRParamInfo[channel_id]);
	NrGetBufferAddress(&g_rNRBufferInfo[channel_id]);
	NrProcess();
#endif

	if (StartWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	wch_status[channel_id] = WCH_STARTED;
	mutex_unlock(&g_Lock);
	pr_info("[AVIN]%s: StartWch success!\r\n", __func__);

	mSigState[channel_id] = TVD_SIG_READY;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight[channel_id];
	data.buf_width = mWidth[channel_id];
	data.signal_status = SIGNAL_READY;
	data.di_flags = cvbs_get_di_flag(channel_id);
	data.need_hide = false;
	if (g_cvbs_type & CVBS_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}
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
	
	if (StopWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);
	if (CloseWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: CloseWch success!\r\n", __func__);
	wch_status[channel_id] = WCH_STOPPED;
	mutex_unlock(&g_Lock);

	mSigState[channel_id] = TVD_SIG_LOST;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight[channel_id];
	data.buf_width = mWidth[channel_id];
	data.signal_status = SIGNAL_LOST;
	data.di_flags = cvbs_get_di_flag(channel_id);
	data.need_hide = false;
	if (g_cvbs_type & CVBS_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}
	pr_info("[AVIN]%s: success!\r\n", __func__);
}

static void vdoSignalChangeStart(TVD_CHANNEL_ID channel_id)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	pr_info("[AVIN]%s: enter\n", __func__);
	if (StopWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_info("[AVIN]%s: StopWch fail!\r\n", __func__);
		return;
	}

#ifdef CVBS_NR_ENABLE
	NrDeInit();
#endif

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight[channel_id];
	data.buf_width = mWidth[channel_id];
	data.signal_status = SIGNAL_CHANGE_START;
	data.di_flags = cvbs_get_di_flag(channel_id);
	data.need_hide = false;
	if (g_cvbs_type & CVBS_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}
	mSigState[channel_id] = TVD_SIG_CHANGE_START;
}

static void vdoSignalChangeDone(TVD_CHANNEL_ID channel_id, u32 mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	pr_info("[AVIN]%s: enter\n", __func__);
	switch (mode) {
	case AV_MODE_PAL:
		mWidth[channel_id] = PAL_FRAME_WIDTH;
		mHeight[channel_id] = PAL_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 2;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 2;
		g_rWchCtrl[channel_id].u4SrcStartX = 0x3B;
		pr_info("[AVIN]%s: CVBS signal system change to PAL\r\n", __func__);
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
		pr_info("[AVIN]%s: CVBS signal system is NTSC\r\n", __func__);
		break;

	case AV_MODE_SECAM:
		mWidth[channel_id] = SECAM_FRAME_WIDTH;
		mHeight[channel_id] = SECAM_FRAME_HEIGHT;
		g_rWchCtrl[channel_id].fgBotFieldFirst = 0;
		g_rWchCtrl[channel_id].u4SrcStartYTop = 1;
		g_rWchCtrl[channel_id].u4SrcStartYBot = 1;
		pr_info("[AVIN]%s: CVBS signal system change to SECAM\r\n", __func__);
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		pr_err("[AVIN]%s: Get Error CVBS signal system value\r\n", __func__);
		return;
	}
	
	/*if (StopWch(WCH_5, g_rWchCtrl[channel_id].eSrcId)) {
		pr_debug("[AVIN]%s: StopWch fail!\r\n", __func__);
		return;
	}*/
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);

	g_rWchCtrl[channel_id].u4SrcWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4SrcHeight = mHeight[channel_id];
	g_rWchCtrl[channel_id].u4DstWidth = mWidth[channel_id];
	g_rWchCtrl[channel_id].u4DstHeight = mHeight[channel_id];
	mutex_lock(&g_Lock);
	if (g_cvbs_type & CVBS_BACKCAR) {
		g_rWchCtrl[channel_id].u4Mirror = g_backcar_mirror;
	} else if (g_cvbs_type & CVBS_NORMAL) {
		g_rWchCtrl[channel_id].u4Mirror = g_cvbs_mirror;
	} else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}

#ifdef CVBS_NR_ENABLE
	NrInit();
#endif

	pr_debug("[AVIN]%s: wch config width(%d) height(%d)\r\n", __func__, mWidth[channel_id], mHeight[channel_id]);
	if (ConfigWch(&g_rWchCtrl[channel_id])) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rWchBufferInfo[channel_id], 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo[channel_id].u1WchId= wchID[channel_id];
	if (WchGetBufferAddress(&g_rWchBufferInfo[channel_id])) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	pr_debug("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

#ifdef CVBS_NR_ENABLE
	g_rNRParamInfo[channel_id].u1WchId = wchID[channel_id];
	g_rNRParamInfo[channel_id].fgBypassEn = false;
	g_rNRParamInfo[channel_id].u1DemoMode = 0;
	g_rNRParamInfo[channel_id].fgBurstRdEn = false;
	g_rNRParamInfo[channel_id].u1AddrSwapMode = 4;
	g_rNRParamInfo[channel_id].u1FrameMode = 0;
	g_rNRParamInfo[channel_id].u4PicWidth = mWidth[channel_id];
	g_rNRParamInfo[channel_id].u4PicHeight = mHeight[channel_id];
	g_rNRParamInfo[channel_id].u4Strength = 1;
	g_rNRParamInfo[channel_id].u4FNRStrength = 3;
	g_rNRParamInfo[channel_id].u4MNRStrength = 3;
	g_rNRParamInfo[channel_id].u4BNRStrength = 3;
	g_rNRParamInfo[channel_id].fgRangeRemapYEn = false;
	g_rNRParamInfo[channel_id].fgRangeRemapUVEn = false;
	g_rNRParamInfo[channel_id].u4RangeMapY = 0;
	g_rNRParamInfo[channel_id].u4RangeMapUV = 0;
	g_rNRParamInfo[channel_id].fgNoiseMeterEn = false;
	g_rNRParamInfo[channel_id].fgUseBlockMeter = false;
	if(channel_id == TVD_CH_0){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get;
	}else if(channel_id == TVD_CH_1){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get1;
	}else if(channel_id == TVD_CH_2){
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get2;
	}else {
	g_rNRParamInfo[channel_id].GetNrBufIndx = nr_buffer_get3;
	}
	NrSetParam(&g_rNRParamInfo[channel_id]);
	NrGetBufferAddress(&g_rNRBufferInfo[channel_id]);
	NrProcess();
#endif

	if (StartWch(wchID[channel_id], g_rWchCtrl[channel_id].eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		mutex_unlock(&g_Lock);
		return;
	}
	wch_status[channel_id] = WCH_STARTED;
	mutex_unlock(&g_Lock);
	pr_debug("[AVIN]%s: StartWch success!\r\n", __func__);
	mSigState[channel_id] = TVD_SIG_CHANGE_DONE;

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = (unsigned int)g_rWchBufferInfo[channel_id].tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight[channel_id];
	data.buf_width = mWidth[channel_id];
	data.signal_status = SIGNAL_CHANGE_DONE;
	data.di_flags = cvbs_get_di_flag(channel_id);
	data.need_hide = false;
	if (g_cvbs_type & CVBS_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else {
		pr_err("[AVIN]%s: g_cvbs_type(%d) error!\n", __func__, g_cvbs_type);
	}
	pr_debug("[AVIN]%s: success!\r\n", __func__);
}

static void atc_tvd_signal_status(void *pStatus)
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
	pr_info("[AVIN]%s: success with signal_state=%d channel_id=%d!\r\n",
		__func__, pSigInfo->signal_state, pSigInfo->channel_id);
}

int cvbs_init_audio(int index)
{
	lineinInit();

	return 0;
}

int cvbs_start_audio(int index)
{
	bool ret = false;

	pr_debug("[AVIN]%s: enter\n", __func__);
	ret = lineinAudStart(index);
	if (ret) {
		pr_debug("[AVIN]%s: lineinAudStart success!\n", __func__);
	} else {
		pr_err("[AVIN]%s: lineinAudStart failed!\n", __func__);
	}

	return ret;
}

int cvbs_stop_audio(int index)
{
	bool ret = false;

	pr_debug("[AVIN]%s: enter\n", __func__);
	ret = lineinAudStop(index);
	if (ret) {
		pr_debug("[AVIN]%s: lineinAudStop success!\n", __func__);
	} else {
		pr_err("[AVIN]%s: lineinAudStop failed!\n", __func__);
	}

	return ret;
}

int cvbs_select_audio(int index)
{
	int ret = 0;

	ret = cvbs_stop_audio(audioport);
	if (ret < 0) {
		pr_err("[AVIN]%s: stop previous audio port(%d) failed!\n", __func__, audioport);
		return ret;
	}

	audioport = index;
	ret = cvbs_start_audio(index);
	if (ret < 0) {
		pr_err("[AVIN]%s: start audio port(%d) failed!\n", __func__, audioport);
		return ret;
	}
	pr_debug("[AVIN]%s: success!\n", __func__);

	return 0;
}


int cvbs_select_video(TVD_APP_ID_ENUM cvbs_type, int index)
{
	pr_info("[AVIN]%s: enter with port(%d)\r\n", __func__, index);
	cvbs_stop_video(cvbs_type, videoport);
	videoport = index;
	//cvbs_init_video(index);
	cvbs_start_video(cvbs_type, index);

	return 0;
}

int cvbs_init_video(int index)
{
	pr_info("[AVIN]%s: Enter",  __func__);
	tvd_register_notify(atc_tvd_signal_status, NULL);
	pr_info("[AVIN]%s: Leave",  __func__);
	return 0;
}

s32 cvbs_callback_start(void)
{
	pr_info("[AVIN]%s:\r\n", __func__);
	cvbs_init_video(0);
	tvdControl(TVD_APP_ID_AVIN, TVD_CONTROL_CODE_INIT);
	g_cvbs_type |= CVBS_NORMAL;
	cvbsStart = true;
	pr_info("[AVIN]%s: success with cvbs_type(%d)\n", __func__, g_cvbs_type);
	return 0;
}

s32 cvbs_callback_stop(void)
{
	pr_info("[AVIN]%s:\r\n", __func__);
	if (tvdControl(TVD_APP_ID_AVIN, TVD_CONTROL_CODE_STOP)) {
		pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
		return -1;
	}
	g_cvbs_type &= (~CVBS_NORMAL);
	g_cvbs_mirror = 0;
	cvbsStart = false;
	pr_info("[AVIN]%s: success with cvbs_type(%d)\r\n", __func__, g_cvbs_type);
	return 0;
}

s32 backcar_callback_start(void)
{
	return 0;
}

s32 backcar_callback_stop(void)
{
	return 0;
}

int cvbs_start_video(TVD_APP_ID_ENUM cvbs_type, int index)
{
	s32 i4Ret = -1;
	pr_info("[AVIN]%s:cvbs_type(%d) port(%d)\r\n", __func__, cvbs_type, index);
	if(index < CVBSIN_1P || index > CVBSIN_6P){
		pr_err("[AVIN]%s:set port index=%d err !\r\n", __func__, index);
	}
	switch(cvbs_type){
		case TVD_APP_ID_AVIN:
			if(cvbsStart == true){
				pr_info("[AVIN]%s: already start\r\n", __func__);
				return 0;
			}
			i4Ret = TWMgr_requestHw(SRC_CVBS, tvdType[index-1], cvbs_callback_start, cvbs_callback_stop);
			if(i4Ret < 0){
				pr_err("[AVIN]%s:HW Request Fail !\r\n", __func__);
				return -1;
			}
			
			if(index >= CVBSIN_1P && index <= CVBSIN_3P){
				wchID[0] = i4Ret;
			}else if(index == CVBSIN_4P){
				wchID[1] = i4Ret;
			}else if(index == CVBSIN_5P){
				wchID[2] = i4Ret;
			}else if(index == CVBSIN_6P){
				wchID[3] = i4Ret;
			}else{
				pr_err("[AVIN]%s:set port index=%d err !\r\n", __func__, index);
			}
			videoport = index;
			break;
		case TVD_APP_ID_BACKCAR:
			pr_info("[AVIN]%s:HW Request enter  !\r\n", __func__);
			i4Ret = TWMgr_requestHw(SRC_BACKCAR, HW_TVD0, backcar_callback_start, backcar_callback_stop);
			if(i4Ret < 0){
				pr_err("[AVIN]%s:HW Request Fail !\r\n", __func__);
				return -1;
			}
			wchID[0] = i4Ret;
			break;
		default:
			pr_err("[AVIN]%s:cvbs src type err !\r\n", __func__);
			break;

	}
	
	cvbs_init_video(index);
	tvdControl(cvbs_type, TVD_CONTROL_CODE_INIT);
	if (TVD_APP_ID_AVIN == cvbs_type) {
		g_cvbs_type |= CVBS_NORMAL;
		cvbsStart = true;
	} else if (TVD_APP_ID_BACKCAR == cvbs_type){
		g_cvbs_type |= CVBS_BACKCAR;
	} else {
		pr_err("[AVIN]%s: cvbs_type error!\r\n", __func__);
		return -1;
	}
	pr_info("[AVIN]%s: success with cvbs_type(%d)\n", __func__, g_cvbs_type);

	return 0;
}

int cvbs_stop_video(TVD_APP_ID_ENUM cvbs_type, int index)
{
	pr_info("[AVIN]%s: cvbs_type(%d) port(%d)\r\n", __func__, cvbs_type, index);
	if (TVD_APP_ID_AVIN == cvbs_type) {
		if(cvbsStart == false) {
			pr_info("[AVIN]%s: already stoped\r\n", __func__);
			if(TWMgr_releaseHw(SRC_CVBS) < 0){
				pr_err("[AVIN]%s:HW Relase Fail !\r\n", __func__);
			}
			return 0;
		}
	}

	if (tvdControl(cvbs_type, TVD_CONTROL_CODE_STOP)) {
		pr_err("[AVIN]%s: tvdControl fail!\r\n", __func__);
		return -1;
	}

	switch(cvbs_type){
		case TVD_APP_ID_AVIN:
			pr_info("[AVIN]%s:AVIN TWM Release !\r\n", __func__);
			if(TWMgr_releaseHw(SRC_CVBS) < 0){
				pr_err("[AVIN]%s:HW Relase Fail !\r\n", __func__);
				return -1;
			}
			cvbsStart = false;
			videoport = CVBSIN_1P;
			break;
		case TVD_APP_ID_BACKCAR:
			pr_info("[AVIN]%s:BackCar TWM Release !\r\n", __func__);
			if(TWMgr_releaseHw(SRC_BACKCAR) < 0){
				pr_err("[AVIN]%s:HW Relase Fail !\r\n", __func__);
				return -1;
			}
			break;
		default:
			pr_err("[AVIN]%s:cvbs src type err !\r\n", __func__);
			break;
	}

	if (TVD_APP_ID_AVIN == cvbs_type) {
		g_cvbs_type &= (~CVBS_NORMAL);
		g_cvbs_mirror = 0;
	} else if (TVD_APP_ID_BACKCAR == cvbs_type) {
		g_cvbs_type &= (~CVBS_BACKCAR);
		g_backcar_mirror = 0;
	} else {
		pr_err("[AVIN]%s: cvbs_type error!\r\n", __func__);
		return -1;
	}
	pr_info("[AVIN]%s: success with cvbs_type(%d)\r\n", __func__, g_cvbs_type);
	return 0;
}

u8 cvbs_get_di_flag(TVD_CHANNEL_ID channel_id)
{
	u32 ret = 0;
	bool di_flag = false;

	ret = tvd_internal_ioctl(GET_DI_FLAG, &channel_id, &di_flag);
	if(ret) {
		pr_err("[AVIN]%s: tvd_internal_ioctl failed!\n", __func__);
		return 0;
	}

	return di_flag;
}

u32 cvbs_get_signal_status(__s32 *pStatus)
{
	u32 ret = 0;
	u32 channel_id = 0;
	pr_info("[AVIN]%s: enter!\n", __func__);
	
	if(videoport >= CVBSIN_1P && videoport <= CVBSIN_3P){
		channel_id = 0;
	}else if(videoport == CVBSIN_4P){
		channel_id = 1;
	}else if(videoport == CVBSIN_5P){
		channel_id = 2;
	}else if(videoport == CVBSIN_6P){
		channel_id = 3;
	}else{
		pr_err("[AVIN]%s:set port index=%d err !\r\n", __func__, videoport);
	}

	if(!(g_cvbs_type&CVBS_NORMAL)||cvbsStart == false){
		pr_info("[AVIN]s%:source type is not cvbs",__func__);
		*pStatus = SIGNAL_READY;
		return ret;
	}
	
	ret = tvd_internal_ioctl(GET_SIGNAL_STATE, &channel_id, pStatus);
	if(ret) {
		pr_err("[AVIN]%s: tvd_internal_ioctl failed!\n", __func__);
	}
	pr_info("[AVIN]%s: leave pStatus = %d!\n", __func__,*pStatus);
	return ret;
}

u32 cvbs_set_mirror(TVD_APP_ID_ENUM cvbs_type, u32 mirror)
{
	if (mirror > 2) {
		pr_err("[AVIN]%s: param error with mirror(%d)!\r\n", __func__, mirror);
		return -1;
	}

	//mutex_lock(&g_Lock);
	if (WCH_STARTED == wch_status[0]) {
		g_rWchCtrl[0].u4Mirror = mirror;
		/*if (WchSetMirror(&g_rWchCtrl)) {
			pr_debug("[AVIN]%s: WchSetMirror(%d) success!\r\n", __func__, mirror);
		} else {
			pr_err("[AVIN]%s: WchSetMirror(%d) error!\r\n", __func__, mirror);
			mutex_unlock(&g_Lock);
			return -1;
		}*/
	} else {
		pr_debug("[AVIN]%s: not in play state, so set in next config!\r\n", __func__);
	}

	if (TVD_APP_ID_AVIN == cvbs_type) {
		g_cvbs_mirror = mirror;
	} else if (TVD_APP_ID_BACKCAR == cvbs_type){
		g_backcar_mirror = mirror;
	} else {
		pr_err("[AVIN]%s: cvbs_type error!\r\n", __func__);
		//mutex_unlock(&g_Lock);
		return -1;
	}
	//mutex_unlock(&g_Lock);

	return 0;
}

int tvdControl(TVD_APP_ID_ENUM cvbs_type, int CtrlCode)
{
	bool ret = 0;

	switch (CtrlCode) {
	case TVD_CONTROL_CODE_INIT: {
		TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T rTvdDrvInitPara;
		pr_debug("[AVIN]%s: TVD_CONTROL_CODE_INIT with type(%d)\r\n", __func__, cvbs_type);
		memset(&rTvdDrvInitPara, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
		rTvdDrvInitPara.source_type = cvbs_type;
		rTvdDrvInitPara.eVdoInFmt = TVD_VDOFMT_YUV444;
		rTvdDrvInitPara.eVdoOutFmt = TVD_VDOFMT_YUV420;
		rTvdDrvInitPara.u4CHBCvbsInxP = CVBSIN_NONE;
		rTvdDrvInitPara.u4CHAOutDest = TVD_CFG_OUT_DRAM;
		rTvdDrvInitPara.u4CHBOutDest = TVD_CFG_OUT_NONE;
		if(cvbs_type == TVD_APP_ID_AVIN){
			rTvdDrvInitPara.u4CHACvbsInxP = videoport;
			switch (videoport) {
				case CVBSIN_1P:
				case CVBSIN_2P:
				case CVBSIN_3P:
					rTvdDrvInitPara.channel_id = TVD_CH_0;
					break;
				case CVBSIN_4P:
					rTvdDrvInitPara.channel_id = TVD_CH_1;
					break;
				case CVBSIN_5P:
					rTvdDrvInitPara.channel_id = TVD_CH_2;
					break;
				case CVBSIN_6P:
					rTvdDrvInitPara.channel_id = TVD_CH_3;
					break;
				default:
					break;

			}
		}else{
			rTvdDrvInitPara.u4CHACvbsInxP = CVBSIN_1P;
			rTvdDrvInitPara.channel_id = TVD_CH_0;
		}
		
		
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

		pr_debug("[AVIN]%s: TVD_CONTROL_CODE_STOP with type(%d)\r\n", __func__, cvbs_type);
		memset(&rTvdDrvStopPara, 0, sizeof(TVD_DRV_STOP_PARA_T));
		rTvdDrvStopPara.stop_channel = TVD_CHA;
		rTvdDrvStopPara.source_type = cvbs_type;
		if(cvbs_type == TVD_APP_ID_AVIN){
			switch (videoport) {
				case CVBSIN_1P:
				case CVBSIN_2P:
				case CVBSIN_3P:
					rTvdDrvStopPara.channel_id = TVD_CH_0;
					break;
				case CVBSIN_4P:
					rTvdDrvStopPara.channel_id = TVD_CH_1;
					break;
				case CVBSIN_5P:
					rTvdDrvStopPara.channel_id = TVD_CH_2;
					break;
				case CVBSIN_6P:
					rTvdDrvStopPara.channel_id = TVD_CH_3;
					break;
				default:
					break;

			}
		}else{
			rTvdDrvStopPara.channel_id = TVD_CH_0;
		}
		
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

int cvbs_init(void)
{
	mutex_init(&g_Lock);
	return 0;
}


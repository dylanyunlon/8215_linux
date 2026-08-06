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


#include <linux/delay.h>
#include <linux/io.h>
#include <generated/atc_project.h>
#include "cvbs_hal.h"
#ifdef CONFIG_AUDIO_ENABLE
#include "audio_hal.h"
#endif
#include "avin_common.h"
#include "tvd_data_struct.h"
#include "avin_log.h"


#define LOG_TAG "cvbs"

#define WCH_IDLE		1
#define WCH_STARTED		2
#define WCH_STOPPED		3


#define DETECT_STD_MAX_TIME		2000/*ms*/
#define DETECT_STD_STEP_TIME	10/*ms*/


static struct wch_ycbuf g_rCameraVirBuf[WCH_BUF_MAX_CNT];
static WCH_BUF_T g_rWchBufferInfo;
static WCH_CTL_PARAM_T g_rWchCtrl;
static int mWidth;
static int mHeight;
static TVD_SIG_STATE_T mSigState;
static int videoport = CVBSIN_1P;
static int audioport = CVBSIN_1P;
static u32 wch_status = WCH_STOPPED;
static int g_cvbs_type = 0;
static u32 g_cvbs_mirror = WCH_CFG_UNMIRROR;
static u32 g_backcar_mirror = WCH_CFG_UNMIRROR;
static int g_cvbs_mode = AV_MODE_NONE;
static struct mutex g_Lock;
struct v4l2_data g_cvbs_camera_data;

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);

static void wch_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;

	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		AVIN_ERROR(LOG_TAG, "param pBufIdx error!\n");
		return;
	}

	AVIN_DEBUG(LOG_TAG, "wch bufIdx is %d, signal status is %d\n",
		*pBufIdx, mSigState);
	if ((TVD_SIG_READY == mSigState) || (TVD_SIG_CHANGE_DONE == mSigState)) {
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[*pBufIdx];
		data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[*pBufIdx];
		data.buf_height = mHeight;
		data.buf_width = mWidth;
		data.signal_status = SIGNAL_NONE;
		data.di_flags = cvbs_get_di_flag();
		data.need_hide = false;
		if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
			avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
		} else if (g_cvbs_type & CVBS_TYPE_NORMAL) {
			avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
		} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
			struct wch_ycbuf ycBuf = {0};
			ycBuf.yAddr = g_rCameraVirBuf[*pBufIdx].yAddr;
			ycBuf.cAddr = g_rCameraVirBuf[*pBufIdx].cAddr;
			/*Cut off the dotted line of the first line of the PAL signal*/
			if (PAL_FRAME_HEIGHT == mHeight) {
				memset((void *)ycBuf.yAddr, 0, mWidth);
			}
#ifdef CONFIG_CVBS_CAMERA_ENABLE
			avin_buffer_complete(AVIN_TYPE_CVBS_CAMERA, (struct capture_priv *)&ycBuf);
#endif
		} else {
			AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
		}
	} else {
		AVIN_DEBUG(LOG_TAG, "the wch data must be discard with signal status is %d\n",
			mSigState);
	}
}

static void vdoCloseWch(void)
{
	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&g_Lock);
	if(WCH_STOPPED == wch_status) {
		AVIN_INFO(LOG_TAG, "The wch have already stopped!\n");
		mutex_unlock(&g_Lock);
		return;
	}

	if (StopWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StopWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");
	if (CloseWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "CloseWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "CloseWch success!\n");
	wch_status = WCH_STOPPED;
	mutex_unlock(&g_Lock);
	mSigState = TVD_SIG_CLOSE_WCH;
}

static void vdoSignal(int mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	memset(&g_rWchCtrl, 0, sizeof(WCH_CTL_PARAM_T));
	g_rWchCtrl.tWchCfg.fgVSyncPolarity = FALSE; /* FALSE is LOW level present sync.*/
	g_rWchCtrl.tWchCfg.fgHSyncPolarity = TRUE; /* TRUE is High.*/

	g_rWchCtrl.tWchCfg.eInputSrc = DATA_SRC_TVD;
	g_rWchCtrl.tWchCfg.eInputFmt = DATA_FMT_YUV422;

	g_rWchCtrl.tWchCfg.fgProgressive = FALSE;
	g_rWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;

	g_rWchCtrl.tWchCfg.u1YSel = 1;/*may change cause of different hw*/
	g_rWchCtrl.tWchCfg.u1USel = 5;/*above*/
	g_rWchCtrl.tWchCfg.u1VSel = 5;/*above*/

	switch (mode) {
	case AV_MODE_PAL:
		mWidth = PAL_FRAME_WIDTH;
		mHeight = PAL_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 1;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system is PAL\n");
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_NTSC:
		mWidth = NTSC_FRAME_WIDTH;
		mHeight = NTSC_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 0;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system is NTSC\n");
		break;

	case AV_MODE_SECAM:
		mWidth = SECAM_FRAME_WIDTH;
		mHeight = SECAM_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 1;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system is SECAM\n");
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		AVIN_ERROR(LOG_TAG, "Get Error CVBS signal system value\n");
		return;
	}

	mutex_lock(&g_Lock);
	g_cvbs_mode = mode;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		g_rWchCtrl.eSrcId = SRC_APP_BACKCAR;
		g_rWchCtrl.tWchCfg.u4Mirror = g_backcar_mirror;
	} else if ((g_cvbs_type & CVBS_TYPE_NORMAL) || (g_cvbs_type & CVBS_TYPE_CAMERA)) {
		g_rWchCtrl.eSrcId = SRC_APP_AVIN;
		g_rWchCtrl.tWchCfg.u4Mirror = g_cvbs_mirror;
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}

	g_rWchCtrl.tWchCfg.u4SrcWidth = mWidth;
	g_rWchCtrl.tWchCfg.u4SrcHeight = mHeight;
	g_rWchCtrl.tWchCfg.u4DstWidth = mWidth;
	g_rWchCtrl.tWchCfg.u4DstHeight = mHeight;
	g_rWchCtrl.tWchCfg.GetWchBufIndx = wch_buffer_get;

	if (ConfigWch(&g_rWchCtrl)) {
		AVIN_ERROR(LOG_TAG, "ConfigWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "ConfigWch success!\n");

	memset(&g_rWchBufferInfo, 0, sizeof(WCH_BUF_T));
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		g_rWchBufferInfo.eSrcId = SRC_APP_BACKCAR;
	} else if ((g_cvbs_type & CVBS_TYPE_NORMAL) || (g_cvbs_type & CVBS_TYPE_CAMERA)) {
		g_rWchBufferInfo.eSrcId = SRC_APP_AVIN;
	} else {
		AVIN_ERROR(LOG_TAG, "srcId error with g_cvbs_type(%d)!\n", g_cvbs_type);
	}
	if (WchGetBufferAddress(&g_rWchBufferInfo)) {
		AVIN_ERROR(LOG_TAG, "WchGetBufferAddress fail!");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "WchGetBufferAddress success!\n");

	if (StartWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StartWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	wch_status = WCH_STARTED;

	if (g_cvbs_type & CVBS_TYPE_CAMERA) {
		u32 u4Idx = 0;
		u32 u4Size = 0;
		for (u4Idx = 0; u4Idx < WCH_BUF_MAX_CNT; u4Idx++) {
			if (!g_rCameraVirBuf[u4Idx].yAddr) {
				u4Size = WCH_SD_YBUF_SIZE;
				g_rCameraVirBuf[u4Idx].yAddr =
					(u32)ioremap(g_rWchBufferInfo.tWchBuf.u4YBuf[u4Idx], u4Size);
				u4Size = WCH_SD_CBUF_SIZE;
				g_rCameraVirBuf[u4Idx].cAddr =
					(u32)ioremap(g_rWchBufferInfo.tWchBuf.u4CBuf[u4Idx], u4Size);
				AVIN_DEBUG(LOG_TAG, "virtual Y(0x%lx) C(0x%lx)\n",
					g_rCameraVirBuf[u4Idx].yAddr, g_rCameraVirBuf[u4Idx].cAddr);
			}
		}
	}

	mutex_unlock(&g_Lock);
	AVIN_DEBUG(LOG_TAG, "StartWch success!\n");

	mSigState = TVD_SIG_READY;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_READY;
	data.di_flags = cvbs_get_di_flag();
	data.need_hide = false;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_TYPE_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
		//avin_buffer_complete(AVIN_TYPE_CVBS_CAMERA, &data);
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}
}

static void vdoNoSignal(void)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&g_Lock);
	if(WCH_STOPPED == wch_status) {
		AVIN_ERROR(LOG_TAG, "The wch have already stopped!\n");
		mutex_unlock(&g_Lock);
		return;
	}

	if (StopWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StopWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");
	if (CloseWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "CloseWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "CloseWch success!\n");
	wch_status = WCH_STOPPED;
	g_cvbs_mode = AV_MODE_NONE;
	mutex_unlock(&g_Lock);

	mSigState = TVD_SIG_LOST;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_LOST;
	data.di_flags = cvbs_get_di_flag();
	data.need_hide = false;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_TYPE_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
		//avin_buffer_complete(AVIN_TYPE_CVBS_CAMERA, &data);
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}
	AVIN_DEBUG(LOG_TAG, "success!\n");
}

static void vdoSignalChangeStart(void)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_CHANGE_START;
	data.di_flags = cvbs_get_di_flag();
	data.need_hide = false;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_TYPE_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
		//avin_buffer_complete(AVIN_TYPE_CVBS_CAMERA, &data);
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}
	mSigState = TVD_SIG_CHANGE_START;
	g_cvbs_mode = AV_MODE_NONE;
}

static void vdoSignalChangeDone(u32 mode)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	switch (mode) {
	case AV_MODE_PAL:
		mWidth = PAL_FRAME_WIDTH;
		mHeight = PAL_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 1;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system change to PAL\n");
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_NTSC:
		mWidth = NTSC_FRAME_WIDTH;
		mHeight = NTSC_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 0;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system change to NTSC\n");
		break;

	case AV_MODE_SECAM:
		mWidth = SECAM_FRAME_WIDTH;
		mHeight = SECAM_FRAME_HEIGHT;
		g_rWchCtrl.tWchCfg.fgBotFieldFirst = 0;
		g_rWchCtrl.tWchCfg.u4SrcStartYTop = 1;
		g_rWchCtrl.tWchCfg.u4SrcStartYBot = 1;
		AVIN_DEBUG(LOG_TAG, "CVBS signal system change to SECAM\n");
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		AVIN_DEBUG(LOG_TAG, "Error CVBS signal system value\n");
		return;
	}

	if (StopWch(g_rWchCtrl.eSrcId)) {
		AVIN_DEBUG(LOG_TAG, "StopWch fail!\n");
		return;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");

	g_rWchCtrl.tWchCfg.u4SrcWidth = mWidth;
	g_rWchCtrl.tWchCfg.u4SrcHeight = mHeight;
	g_rWchCtrl.tWchCfg.u4DstWidth = mWidth;
	g_rWchCtrl.tWchCfg.u4DstHeight = mHeight;
	mutex_lock(&g_Lock);
	g_cvbs_mode = mode;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		g_rWchCtrl.eSrcId = SRC_APP_BACKCAR;
		g_rWchCtrl.tWchCfg.u4Mirror = g_backcar_mirror;
	} else if ((g_cvbs_type & CVBS_TYPE_NORMAL) || (g_cvbs_type & CVBS_TYPE_CAMERA)) {
		g_rWchCtrl.eSrcId = SRC_APP_AVIN;
		g_rWchCtrl.tWchCfg.u4Mirror = g_cvbs_mirror;
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}
	AVIN_DEBUG(LOG_TAG, "wch config width(%d) height(%d)\n", mWidth, mHeight);
	if (ConfigWch(&g_rWchCtrl)) {
		AVIN_ERROR(LOG_TAG, "ConfigWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "ConfigWch success!\n");

	memset(&g_rWchBufferInfo, 0, sizeof(WCH_BUF_T));
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		g_rWchBufferInfo.eSrcId = SRC_APP_BACKCAR;
	} else if ((g_cvbs_type & CVBS_TYPE_NORMAL) || (g_cvbs_type & CVBS_TYPE_CAMERA)) {
		g_rWchBufferInfo.eSrcId = SRC_APP_AVIN;
	} else {
		AVIN_ERROR(LOG_TAG, "srcId error with g_cvbs_type(%d)!\n", g_cvbs_type);
	}
	if (WchGetBufferAddress(&g_rWchBufferInfo)) {
		AVIN_ERROR(LOG_TAG, "WchGetBufferAddress fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "WchGetBufferAddress success!\n");

	if (StartWch(g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StartWch fail!\n");
		mutex_unlock(&g_Lock);
		return;
	}
	wch_status = WCH_STARTED;
	mutex_unlock(&g_Lock);
	AVIN_DEBUG(LOG_TAG, "StartWch success!\n");
	mSigState = TVD_SIG_CHANGE_DONE;

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_CHANGE_DONE;
	data.di_flags = cvbs_get_di_flag();
	data.need_hide = false;
	if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
		avin_buffer_complete(AVIN_TYPE_BACKCAR, &data);
	} else if (g_cvbs_type & CVBS_TYPE_NORMAL) {
		avin_buffer_complete(AVIN_TYPE_CVBS_VIDEO, &data);
	} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
		//avin_buffer_complete(AVIN_TYPE_CVBS_CAMERA, &data);
	} else {
		AVIN_ERROR(LOG_TAG, "g_cvbs_type(%d) error!\n", g_cvbs_type);
	}
	AVIN_DEBUG(LOG_TAG, "success!\n");
}

static void atc_tvd_signal_status(void *pStatus)
{
	TVD_SIG_INFORMATION *pSigInfo = (TVD_SIG_INFORMATION *)pStatus;

	if(NULL == pSigInfo) {
		AVIN_ERROR(LOG_TAG, "param pStatus is NULL!\n");
		return;
	}

	switch (pSigInfo->signal_state) {
	case TVD_SIG_READY:
		vdoSignal(pSigInfo->arg);
		break;

	case TVD_SIG_LOST:
		vdoNoSignal();
		break;

	case TVD_SIG_CHANGE_START:
		vdoSignalChangeStart();
		break;

	case TVD_SIG_CHANGE_DONE:
		vdoSignalChangeDone(pSigInfo->arg);
		break;

	case TVD_SIG_CLOSE_WCH:
		vdoCloseWch();
		break;

	default:
		break;
	}
	AVIN_DEBUG(LOG_TAG, "success with signal_state(%d)!\n",
		pSigInfo->signal_state);
}

#ifdef CONFIG_AUDIO_ENABLE
int cvbs_init_audio(int index)
{
	lineinInit();

	return 0;
}

int cvbs_start_audio(int index)
{
	bool ret = false;

	AVIN_DEBUG(LOG_TAG, "enter with port(%d)\n", index);
	ret = lineinAudStart(index);
	if (ret) {
		AVIN_DEBUG(LOG_TAG, "lineinAudStart success!\n");
		return 0;
	} else {
		AVIN_ERROR(LOG_TAG, "lineinAudStart failed!\n");
		return -1;
	}
}

int cvbs_stop_audio(int index)
{
	bool ret = false;

	AVIN_DEBUG(LOG_TAG, "enter with port(%d)\n", index);
	ret = lineinAudStop(index);
	if (ret) {
		AVIN_DEBUG(LOG_TAG, "lineinAudStop success!\n");
		return 0;
	} else {
		AVIN_ERROR(LOG_TAG, "lineinAudStop failed!\n");
		return -1;
	}
}

int cvbs_select_audio(int index)
{
	int ret = 0;

	AVIN_DEBUG(LOG_TAG, "enter with port(%d)\n", index);
	ret = cvbs_stop_audio(audioport);
	if (ret < 0) {
		AVIN_ERROR(LOG_TAG, "stop previous audio port(%d) failed!\n", audioport);
		return ret;
	}

	audioport = index;
	ret = cvbs_start_audio(index);
	if (ret < 0) {
		AVIN_ERROR(LOG_TAG, "start audio port(%d) failed!\n", audioport);
		return ret;
	}
	AVIN_DEBUG(LOG_TAG, "success!\n");

	return 0;
}
#endif

int cvbs_select_video(CVBS_TYPE_ENUM cvbs_type, int index)
{
	AVIN_DEBUG(LOG_TAG, "enter with port(%d)\n", index);
	cvbs_stop_video(cvbs_type, videoport);
	videoport = index;
	cvbs_init_video(index);
	cvbs_start_video(cvbs_type, index);

	return 0;
}

int cvbs_init_video(int index)
{
	tvd_register_notify(atc_tvd_signal_status, NULL);

	return 0;
}

int cvbs_start_video(CVBS_TYPE_ENUM cvbs_type, int index)
{
	TVD_APP_ID_ENUM tvd_app_id = TVD_APP_ID_NONE;

	AVIN_DEBUG(LOG_TAG, "cvbs_type(%d) port(%d)\n", cvbs_type, index);
	if (((CVBS_TYPE_NORMAL == cvbs_type) || (CVBS_TYPE_CAMERA == cvbs_type)) &&
		(g_cvbs_type & (CVBS_TYPE_NORMAL | CVBS_TYPE_CAMERA))) {
		AVIN_ERROR(LOG_TAG, " error with cvbs_type(%d) port(%d)\n", cvbs_type, index);
		return -1;
	}

	videoport = index;
	if (CVBS_TYPE_NORMAL == cvbs_type) {
		g_cvbs_type |= CVBS_TYPE_NORMAL;
		tvd_app_id = TVD_APP_ID_AVIN;
	} else if (CVBS_TYPE_CAMERA == cvbs_type){
		g_cvbs_type |= CVBS_TYPE_CAMERA;
		tvd_app_id = TVD_APP_ID_AVIN;
	} else if (CVBS_TYPE_BACKCAR == cvbs_type){
		g_cvbs_type |= CVBS_TYPE_BACKCAR;
		tvd_app_id = TVD_APP_ID_BACKCAR;
	} else {
		AVIN_ERROR(LOG_TAG, "cvbs_type error!\n");
		return -1;
	}
	tvdControl(tvd_app_id, TVD_CONTROL_CODE_INIT);
	g_cvbs_mode = AV_MODE_NONE;
	AVIN_DEBUG(LOG_TAG, "success with cvbs_type(%d)\n", g_cvbs_type);

	return 0;
}

int cvbs_stop_video(CVBS_TYPE_ENUM cvbs_type, int index)
{
	TVD_APP_ID_ENUM tvd_app_id = TVD_APP_ID_NONE;

	AVIN_DEBUG(LOG_TAG, "cvbs_type(%d) port(%d)\n", cvbs_type, index);
	if ((CVBS_TYPE_NORMAL == cvbs_type) || (CVBS_TYPE_CAMERA == cvbs_type)) {
		tvd_app_id = TVD_APP_ID_AVIN;
	} else if (CVBS_TYPE_BACKCAR == cvbs_type){
		tvd_app_id = TVD_APP_ID_BACKCAR;
	} else {
		AVIN_ERROR(LOG_TAG, "cvbs_type error!\n");
		return -1;
	}
	if (tvdControl(tvd_app_id, TVD_CONTROL_CODE_STOP)) {
		AVIN_ERROR(LOG_TAG, "tvdControl fail!\n");
		return -1;
	}

	if (CVBS_TYPE_NORMAL == cvbs_type) {
		g_cvbs_type &= (~CVBS_TYPE_NORMAL);
		g_cvbs_mirror = WCH_CFG_UNMIRROR;
	} else if (CVBS_TYPE_CAMERA == cvbs_type){
		g_cvbs_type &= (~CVBS_TYPE_CAMERA);
		g_cvbs_mirror = WCH_CFG_UNMIRROR;
	} else if (CVBS_TYPE_BACKCAR == cvbs_type){
		g_cvbs_type &= (~CVBS_TYPE_BACKCAR);
		g_backcar_mirror = WCH_CFG_UNMIRROR;
	} else {
		AVIN_ERROR(LOG_TAG, "cvbs_type error!\n");
		return -1;
	}

	videoport = CVBSIN_1P;
	g_cvbs_mode = AV_MODE_NONE;
	AVIN_DEBUG(LOG_TAG, "success with cvbs_type(%d)\n", g_cvbs_type);

	return 0;
}

u8 cvbs_get_di_flag(void)
{
	u32 ret = 0;
	bool di_flag = false;

	ret = tvd_internal_ioctl(GET_DI_FLAG, NULL, &di_flag);
	if(ret) {
		AVIN_ERROR(LOG_TAG, "tvd_internal_ioctl failed!\n");
		return 0;
	}

	return di_flag;
}

u32 cvbs_get_signal_status(__s32 *pStatus)
{
	u32 ret = 0;

	ret = tvd_internal_ioctl(GET_SIGNAL_STATE, NULL, pStatus);
	if(ret) {
		AVIN_ERROR(LOG_TAG, "tvd_internal_ioctl failed!\n");
	}

	return ret;
}

u32 cvbs_get_signal_std(v4l2_std_id *pstd_type)
{
	u32 cnt = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	if (!pstd_type) {
		AVIN_ERROR(LOG_TAG, "param pstd_type is NULL!\n");
		return -1;
	}

	for (cnt = 0; cnt < DETECT_STD_MAX_TIME / DETECT_STD_STEP_TIME; cnt++) {
		if (AV_MODE_NONE != g_cvbs_mode) {
			break;
		}
		msleep(DETECT_STD_STEP_TIME);
	}
	if (cnt == (DETECT_STD_MAX_TIME / DETECT_STD_STEP_TIME)) {
		AVIN_INFO(LOG_TAG, "wait for %dms timeout!\n", DETECT_STD_MAX_TIME);
		return -1;
	}

	switch (g_cvbs_mode) {
	case AV_MODE_PAL:
	case AV_MODE_SECAM:
		*pstd_type = V4L2_STD_PAL;
		break;

	case AV_MODE_NTSC443:
	case AV_MODE_NTSC:
		*pstd_type = V4L2_STD_NTSC;
		break;

	case AV_MODE_UNSTABLE:
	case AV_MODE_NONE:
		AVIN_ERROR(LOG_TAG, "cvbs_mode(%d) error!\n", g_cvbs_mode);
		*pstd_type = V4L2_STD_UNKNOWN;
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "leave\n");

	return 0;
}

u32 cvbs_set_mirror(CVBS_TYPE_ENUM cvbs_type, u32 mirror)
{
	if (mirror > WCH_CFG_MIRROR_HV) {
		AVIN_ERROR(LOG_TAG, "param error with mirror(%d)!\n", mirror);
		return -1;
	}

	mutex_lock(&g_Lock);
	if (WCH_STARTED == wch_status) {
		g_rWchCtrl.tWchCfg.u4Mirror = mirror;
		if (WchSetMirror(&g_rWchCtrl)) {
			AVIN_DEBUG(LOG_TAG, "WchSetMirror(%d) success!\n", mirror);
		} else {
			AVIN_ERROR(LOG_TAG, "WchSetMirror(%d) error!\n", mirror);
			mutex_unlock(&g_Lock);
			return -1;
		}
	} else {
		AVIN_DEBUG(LOG_TAG, "not in play state, so set in next config!\n");
	}

	if ((CVBS_TYPE_NORMAL == cvbs_type) || (CVBS_TYPE_CAMERA == cvbs_type)) {
		g_cvbs_mirror = mirror;
	} else if (CVBS_TYPE_BACKCAR == cvbs_type){
		g_backcar_mirror = mirror;
	} else {
		AVIN_ERROR(LOG_TAG, "cvbs_type error!\n");
		mutex_unlock(&g_Lock);
		return -1;
	}
	mutex_unlock(&g_Lock);

	return 0;
}

int tvdControl(TVD_APP_ID_ENUM cvbs_type, int CtrlCode)
{
	bool ret = 0;

	switch (CtrlCode) {
	case TVD_CONTROL_CODE_INIT: {
		TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T rTvdDrvInitPara;
		AVIN_DEBUG(LOG_TAG, "TVD_CONTROL_CODE_INIT with type(%d)\n", cvbs_type);
		memset(&rTvdDrvInitPara, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
		rTvdDrvInitPara.source_type = cvbs_type;
		rTvdDrvInitPara.eVdoInFmt = TVD_VDOFMT_YUV422;
		rTvdDrvInitPara.eVdoOutFmt = TVD_VDOFMT_YUV420;
		rTvdDrvInitPara.u4CHACvbsInxP = videoport;
		rTvdDrvInitPara.u4CHBCvbsInxP = CVBSIN_NONE;
		rTvdDrvInitPara.u4CHAOutDest = TVD_CFG_OUT_DRAM;
		rTvdDrvInitPara.u4CHBOutDest = TVD_CFG_OUT_NONE;
		if (g_cvbs_type & CVBS_TYPE_BACKCAR) {
			rTvdDrvInitPara.u4UVSwap = 0;
		} else if (g_cvbs_type & CVBS_TYPE_CAMERA) {
			AVIN_INFO(LOG_TAG, "set tvd uvSwap to 1\n");
			rTvdDrvInitPara.u4UVSwap = 1;
		}
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_INIT, (u8 *)&rTvdDrvInitPara, NULL);
	}
	break;

	case TVD_CONTROL_CODE_CONFIG: {
		TVD_DRV_CAMERA_PREVIEW_CFG_T rTvdDrvCfgPara;

		AVIN_DEBUG(LOG_TAG, "TVD_CONTROL_CODE_CONFIG\n");
		memset(&rTvdDrvCfgPara, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_CFG_T));
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_CONFIG, (u8 *)&rTvdDrvCfgPara, NULL);
	}
	break;

	case TVD_CONTROL_CODE_START: {
		AVIN_DEBUG(LOG_TAG, "TVD_CONTROL_CODE_START\n");
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_START, NULL, NULL);
	}
	break;

	case TVD_CONTROL_CODE_STOP: {
		TVD_DRV_STOP_PARA_T rTvdDrvStopPara;

		AVIN_DEBUG(LOG_TAG, "TVD_CONTROL_CODE_STOP with type(%d)\n", cvbs_type);
		memset(&rTvdDrvStopPara, 0, sizeof(TVD_DRV_STOP_PARA_T));
		rTvdDrvStopPara.stop_channel = TVD_CHA;
		rTvdDrvStopPara.source_type = cvbs_type;
		ret = tvd_internal_ioctl(TVD_CONTROL_CODE_STOP, &rTvdDrvStopPara, NULL);
	}
	break;

	default:
		AVIN_DEBUG(LOG_TAG, "invalid control code!");
		return false;
	}
	AVIN_DEBUG(LOG_TAG, "end with ret(%d)", ret);

	return (ret > 0);
}

int cvbs_init(void)
{
	mutex_init(&g_Lock);
	memset(g_rCameraVirBuf, 0, sizeof(g_rCameraVirBuf));

	return 0;
}

int cvbs_deinit(void)
{
	int i4Idx = 0;

	for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
		if (g_rCameraVirBuf[i4Idx].yAddr) {
			iounmap((void *)g_rCameraVirBuf[i4Idx].yAddr);
			g_rCameraVirBuf[i4Idx].yAddr = 0;
		}
		if (g_rCameraVirBuf[i4Idx].cAddr) {
			iounmap((void *)g_rCameraVirBuf[i4Idx].cAddr);
			g_rCameraVirBuf[i4Idx].cAddr = 0;
		}
	}
	memset(g_rCameraVirBuf, 0, sizeof(g_rCameraVirBuf));

	return 0;
}



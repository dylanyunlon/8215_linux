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


#include "ybrvga_hal.h"
#include "ybr_vga_drv_if.h"
#include "ac823x/wch_drv.h"
#include "avin_common.h"
#include "avin_log.h"


#define LOG_TAG "ybrvga"

#define WCH_IDLE		1
#define WCH_STARTED		2
#define WCH_STOPPED		3

static WCH_SRC_APP_ID_E    mSrcAppId = SRC_APP_YPBPR;
static WCH_BUFF_INFO_T mWchBufferInfo;
static WCH_BUF_T g_rWchBufferInfo;
static WCH_CFG_T g_rWchCtrl;
static int mWidth;
static int mHeight;
static u32 wch_status = WCH_STOPPED;
static BOOL wchStop = TRUE;
static u8 sigStatus = 0;
YBR_VGA_CFG mYbrVgaCfg;
YBR_VGA_VDO_INFO mYbrVgaVdoInfo;
bool need_di = false;
static struct mutex g_YbrLock;

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);


int ybrControl(int CtrlCode) {
	bool ErrorCode = 0;

	switch (CtrlCode) {
	case IOCTL_YBR_VGA_INIT:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_INIT\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_INIT, NULL, 0, NULL, 0, NULL);
		break;

	case IOCTL_YBR_VGA_START:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_START\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_START, NULL, 0, NULL, 0, NULL);
		break;

	case IOCTL_YBR_VGA_STOP:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_STOP\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_STOP, NULL, 0, NULL, 0, NULL);
		break;

	case IOCTL_YBR_VGA_CONFIG:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_CONFIG\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_CONFIG, (void*)&mYbrVgaCfg,
			sizeof(YBR_VGA_CFG), NULL, 0, NULL);
		break;

	case IOCTL_YBR_VGA_GET_VIDEO_INFO:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_GET_VIDEO_INFO\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_GET_VIDEO_INFO, NULL, 0,
			(void*)&mYbrVgaVdoInfo, sizeof(YBR_VGA_VDO_INFO), NULL);
		break;

	case IOCTL_YBR_VGA_AUTO:
		AVIN_DEBUG(LOG_TAG, "IOCTL_YBR_VGA_AUTO\n");
		ErrorCode = YBR_IOControl(0, IOCTL_YBR_VGA_AUTO, NULL, 0, NULL, 0, NULL);
		break;

	default:
		AVIN_ERROR(LOG_TAG, "invalid control code(%d)", CtrlCode);
		return false;
	}

	return (ErrorCode > 0);
}

static void wch_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;
	bool need_hide = false;

	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		AVIN_ERROR(LOG_TAG, "param pBufIdx error!\n");
		return;
	}

	if (sigStatus == SV_VDO_NOSIGNAL) {
		AVIN_DEBUG(LOG_TAG, "SV_VDO_NOSIGNAL, so need hide vdp\n");
		need_hide = true;
	}
	AVIN_DEBUG(LOG_TAG, "wch bufIdx(%d), signal status(%d)\n",
		*pBufIdx, sigStatus);

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[*pBufIdx];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[*pBufIdx];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_NONE;
	data.need_hide = need_hide;
	data.di_flags = need_di;
	avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
}

void onVdoSignal(int signalstatus) {
	struct capture_priv data;

	AVIN_INFO(LOG_TAG, "enter\n");
	if (!ybrControl(IOCTL_YBR_VGA_GET_VIDEO_INFO)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_GET_VIDEO_INFO fail!\n");
		return;
	}

	memset(&g_rWchCtrl, 0, sizeof(WCH_CFG_T));
	g_rWchCtrl.fgVSyncPolarity = false;// FALSE is LOW level present sync.
	g_rWchCtrl.fgHSyncPolarity = true;// TRUE is High.
	g_rWchCtrl.eInputSrc = DATA_SRC_YPBPR;
	g_rWchCtrl.eInputFmt = DATA_FMT_YUV444;
	g_rWchCtrl.fgProgressive = (mYbrVgaVdoInfo.u1Interlace == 1) ? 0 : 1;
	g_rWchCtrl.eOutputFmt = DATA_FMT_YUV420;
	g_rWchCtrl.u1WchId = WCH_6;
	g_rWchCtrl.u4ScanLineMode = 1;
	g_rWchCtrl.eSrcId = mSrcAppId;
	g_rWchCtrl.u1YSel = 0;
	g_rWchCtrl.u1USel = 1;
	g_rWchCtrl.u1VSel = 2;

	if (mYbrVgaVdoInfo.u1Interlace == 1) {
		need_di = true;
		//g_rWchCtrl.fgBotFieldFirst = 0;
		//mYbrVgaVdoInfo.u2Height = mYbrVgaVdoInfo.u2Height - 3; //68031 modify it
		if (mYbrVgaVdoInfo.u2Height == YBR_VGA_480_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x16;
			g_rWchCtrl.u4SrcStartYBot = 0x16;
		} else if (mYbrVgaVdoInfo.u2Height == YBR_VGA_576_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x18;
			g_rWchCtrl.u4SrcStartYBot = 0x18;
		} else if (mYbrVgaVdoInfo.u2Height == YBR_VGA_1080_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x18;
			g_rWchCtrl.u4SrcStartYBot = 0x18;
		} else {
			AVIN_ERROR(LOG_TAG, "mYbrVgaVdoInfo height(%d) error!\n", mYbrVgaVdoInfo.u2Height);
			return;
		}
	} else {
		need_di = false;
		//g_rWchCtrl.fgBotFieldFirst = 0;
		if (mYbrVgaVdoInfo.u2Height == YBR_VGA_480_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x2C;
			g_rWchCtrl.u4SrcStartYBot = 0x2C;
		} else if (mYbrVgaVdoInfo.u2Height == YBR_VGA_576_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x31;
			g_rWchCtrl.u4SrcStartYBot = 0x31;
		} else if (mYbrVgaVdoInfo.u2Height == YBR_VGA_720_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x1F;
			g_rWchCtrl.u4SrcStartYBot = 0x1F;
		} else if (mYbrVgaVdoInfo.u2Height == YBR_VGA_1080_HEIGHT) {
			g_rWchCtrl.u4SrcStartYTop = 0x2E;
			g_rWchCtrl.u4SrcStartYBot = 0x2E;
		} else {
			AVIN_ERROR(LOG_TAG, "mYbrVgaVdoInfo height(%d) error!\n", mYbrVgaVdoInfo.u2Height);
			return;
		}
	}

	mutex_lock(&g_YbrLock);
	if ((mYbrVgaVdoInfo.u2Height == YBR_VGA_480_HEIGHT) ||
		(mYbrVgaVdoInfo.u2Height == YBR_VGA_576_HEIGHT)) {
		mYbrVgaVdoInfo.u2Width *= 2;
	}
	g_rWchCtrl.u4SrcWidth = mYbrVgaVdoInfo.u2Width;
	g_rWchCtrl.u4DstWidth = mYbrVgaVdoInfo.u2Width;
	g_rWchCtrl.u4SrcHeight = mYbrVgaVdoInfo.u2Height;
	g_rWchCtrl.u4DstHeight = mYbrVgaVdoInfo.u2Height;
	mWidth = mYbrVgaVdoInfo.u2Width;
	mHeight = mYbrVgaVdoInfo.u2Height;
	AVIN_DEBUG(LOG_TAG, "OpenWch srcwidth(%d) dstwidth(%d) height(%d)!\n",
		g_rWchCtrl.u4SrcWidth, g_rWchCtrl.u4DstWidth, g_rWchCtrl.u4SrcHeight);
	g_rWchCtrl.GetWchBufIndx = wch_buffer_get;

	if (WCH_SUCCESS != OpenWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "OpenWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "OpenWch success!\n");
	
	if (ConfigWch(&g_rWchCtrl)) {
		AVIN_ERROR(LOG_TAG, "ConfigWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "ConfigWch success!\n");

	memset(&g_rWchBufferInfo, 0, sizeof(WCH_BUF_T));
	g_rWchBufferInfo.u1WchId = WCH_6;
	if (WchGetBufferAddress(&g_rWchBufferInfo)) {
		AVIN_ERROR(LOG_TAG, "WchGetBufferAddress fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, " WchGetBufferAddress success!\n");

	if (StartWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StartWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	wch_status = WCH_STARTED;
	mutex_unlock(&g_YbrLock);

	sigStatus = SV_VDO_STABLE;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[0];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[0];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_READY;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
}

void onVdoNoSignal(void)
{
	struct capture_priv data;
	u32 buffer_id = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&g_YbrLock);
	if(WCH_STOPPED == wch_status) {
		AVIN_INFO(LOG_TAG, "The wch have already stopped!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	if (StopWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StopWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");
	if (CloseWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "CloseWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return;
	}
	AVIN_DEBUG(LOG_TAG, "CloseWch success!\n");
	wch_status = WCH_STOPPED;
	mutex_unlock(&g_YbrLock);

	sigStatus = SV_VDO_NOSIGNAL;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = g_rWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_LOST;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
}

void atc_ybrvga_isr(void *pArg)
{
	u8 signalStatus = 0;

	if (NULL == pArg) {
		AVIN_ERROR(LOG_TAG, "param pArg error!\n");
		return;
	}

	signalStatus = *((u8 *)pArg);
	switch(signalStatus) {
	case SV_VDO_NOSIGNAL:
		AVIN_DEBUG(LOG_TAG, "no signal\n");
		onVdoNoSignal();
		break;

	case SV_VDO_STABLE:
		AVIN_DEBUG(LOG_TAG, "signal stable\n");
		onVdoSignal(signalStatus);
		break;

	default:
		AVIN_DEBUG(LOG_TAG, "signalStatus invalid\n");
		break;
	}
}

int ybrvga_init_video(int index)
{
	int ret = 0;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	atc_ybr_register_isr(atc_ybrvga_isr, NULL);

	return ret;
}

int ybrvga_set_auto(void )
{
	AVIN_DEBUG(LOG_TAG, "enter\n");
	if (!ybrControl(IOCTL_YBR_VGA_AUTO)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_AUTO fail!\n");
		return -1;
	}

	return 0;
}

int ybrvga_select_video(int index)
{
	AVIN_DEBUG(LOG_TAG, "enter with index(%d)\n", index);
	if(index == 0) {
		mSrcAppId = SRC_APP_YPBPR;
		g_rWchCtrl.eInputSrc = DATA_SRC_YPBPR;
		mYbrVgaCfg.source_type = SRC_YBR;
	} else if(index == 1) {
		mSrcAppId = SRC_APP_VGA;
		g_rWchCtrl.eInputSrc = DATA_SRC_VGA;
		mYbrVgaCfg.source_type = SRC_VGA;
	} else {
		AVIN_ERROR(LOG_TAG, "index(%d) is error\n", index);
		return -1;
	}

	return 0;
}

int ybrvga_start_video(int index)
{
	int mWchId;
	WCH_BUFF_INFO_T bufferInfo;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	ybrvga_select_video(0);
	if (!ybrControl(IOCTL_YBR_VGA_INIT)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_INIT fail!\n");
		return -1;
	}
	if (!ybrControl(IOCTL_YBR_VGA_CONFIG)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_CONFIG fail!\n");
		return -1;
	}
	if (!ybrControl(IOCTL_YBR_VGA_START)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_START fail!\n");
		return -1;
	}

	return 0;
}

int ybrvga_stop_video(void)
{
	bool ret = false;

	AVIN_DEBUG(LOG_TAG, "enter\n");
	mutex_lock(&g_YbrLock);
	if (!ybrControl(IOCTL_YBR_VGA_STOP)) {
		AVIN_ERROR(LOG_TAG, "IOCTL_YBR_VGA_STOP fail!\n");
		mutex_unlock(&g_YbrLock);
		return -1;
	}

	if(WCH_STOPPED == wch_status) {
		AVIN_INFO(LOG_TAG, "The wch have already stopped!\n");
		mutex_unlock(&g_YbrLock);
		return -1;
	}
	if (StopWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StopWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");

	if (CloseWch(WCH_6, g_rWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "CloseWch fail!\n");
		mutex_unlock(&g_YbrLock);
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "CloseWch success!\n");
	wch_status = WCH_STOPPED;
	mutex_unlock(&g_YbrLock);

	return 0;
}

int ybrvga_init(void)
{
	mutex_init(&g_YbrLock);

	return 0;
}


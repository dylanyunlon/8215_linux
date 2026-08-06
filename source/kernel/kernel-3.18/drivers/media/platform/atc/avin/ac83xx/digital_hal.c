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

#include "wch_if.h"
#include "digital_hal.h"
#include "avin_common.h"
#include "avin_log.h"


#define LOG_TAG "digvideo"

static WCH_BUF_T g_rDigWchBufInfo;
static WCH_CTL_PARAM_T g_rDigWchCtrl;
static int mWidth;
static int mHeight;
static int mSrcFmt = 0;
static bool mSigNotify = false;


extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);

static void wch_buffer_get(u32 *pBufIdx) {
	struct capture_priv data;

	if ((NULL == pBufIdx) || (*pBufIdx >= WCH_BUF_MAX_CNT)) {
		AVIN_ERROR(LOG_TAG, "param pBufIdx error!\n");
		return;
	}

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = g_rDigWchBufInfo.tWchBuf.u4YBuf[*pBufIdx];
	data.ycaddr.c = g_rDigWchBufInfo.tWchBuf.u4CBuf[*pBufIdx];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.di_flags = !g_rDigWchCtrl.tWchCfg.fgProgressive;
	data.need_hide = false;
	if (mSigNotify) {
		data.signal_status = SIGNAL_NONE;
		avin_buffer_complete(AVIN_TYPE_DIGITAL_VIDEO, &data);
	} else {
		data.signal_status = SIGNAL_READY;
		avin_buffer_complete(AVIN_TYPE_DIGITAL_VIDEO, &data);
		mSigNotify = true;
	}
}

int digital_select_video(int index)
{
	AVIN_DEBUG(LOG_TAG, "input index=%d\n", index);
	if((index >= DIG_IN_TYPE_601_P_480) && (index < DIG_IN_TYPE_MAX)) {
		mSrcFmt = index;
	} else {
		AVIN_ERROR(LOG_TAG, "input index(%d) error!\n", index);
		return -1;
	}

	return 0;
}

int digital_start_video(int mDigInFmt)
{
	mDigInFmt = mSrcFmt;
	AVIN_DEBUG(LOG_TAG, "enter with mDigInFmt(%d)\n", mDigInFmt);
	switch (mDigInFmt) {
	case DIG_IN_TYPE_601_P_480:
		// 480P601
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 1; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 1; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT601;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0xA0;//may change cause of different hwA0
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0x19;//above19
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0x19;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rDigWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 1;

		g_rDigWchCtrl.tWchCfg.u1YSel = 3;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 3;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_601_P_576:
		// 576P601
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 1; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 1; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputSrc = DATA_SRC_DGI;
		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT601;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0xBB;//may change cause of different hw0xBB
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0x1B;//above0x1B
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0x1B;//above0x1B

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rDigWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 1;

		g_rDigWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 1;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_601_I_480:
		// 480I601
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 1; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 1; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputSrc = DATA_SRC_DGI;
		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT601;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0xCE;//may change cause of different hw0xCE
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 2;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 3;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rDigWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 0;

		g_rDigWchCtrl.tWchCfg.u1YSel = 3;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 3;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_601_I_576:
		// 576I601
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 1; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 1; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT601;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0xE5;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0x7;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0x8;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 0;

		g_rDigWchCtrl.tWchCfg.u1YSel = 3;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 3;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_656_P_480:
		// 480P656
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0x0;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 1;

		g_rDigWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw11
		g_rDigWchCtrl.tWchCfg.u1USel = 1;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_656_P_576:
		// 576P656
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 1;

		g_rDigWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 1;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_656_I_480:
		// 480I656
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 2;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 3;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 0;

		g_rDigWchCtrl.tWchCfg.u1YSel = 3;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 3;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_656_I_576:
		// 576I656
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;
		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 0;

		g_rDigWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 1;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case DIG_IN_TYPE_656_p_800_480:
		// 480P656
		g_rDigWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rDigWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High. 

		g_rDigWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 800; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 800; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rDigWchCtrl.tWchCfg.fgProgressive = 1;

		g_rDigWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u1USel = 1;//above
		g_rDigWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	default:
		AVIN_ERROR(LOG_TAG, "error with mDigInFmt(%d)\n", mDigInFmt);
		return -1;
	}

	g_rDigWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
	g_rDigWchCtrl.tWchCfg.eInputSrc = DATA_SRC_DGI;
	g_rDigWchCtrl.tWchCfg.fgBotFieldFirst = 0;
	g_rDigWchCtrl.eSrcId = SRC_APP_DGI;
	g_rDigWchCtrl.tWchCfg.GetWchBufIndx = wch_buffer_get;
	mWidth = g_rDigWchCtrl.tWchCfg.u4SrcWidth;
	mHeight = g_rDigWchCtrl.tWchCfg.u4SrcHeight;

	if (ConfigWch(&g_rDigWchCtrl)) {
		AVIN_ERROR(LOG_TAG, "ConfigWch fail!\n");
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "ConfigWch success!\n");

	memset(&g_rDigWchBufInfo, 0, sizeof(WCH_BUF_T));
	g_rDigWchBufInfo.eSrcId = SRC_APP_DGI;
	if (WchGetBufferAddress(&g_rDigWchBufInfo)) {
		AVIN_ERROR(LOG_TAG, "WchGetBufferAddress fail!\n");
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "WchGetBufferAddress success!\n");

	if (StartWch(g_rDigWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StartWch fail!\n");
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "StartWch success!\n");

	mSigNotify = false;

	return 0;
}

int digital_stop_video(void)
{
	if (StopWch(g_rDigWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "StopWch fail!\n");
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "StopWch success!\n");

	if (CloseWch(g_rDigWchCtrl.eSrcId)) {
		AVIN_ERROR(LOG_TAG, "CloseWch fail!\n");
		return -1;
	}
	AVIN_DEBUG(LOG_TAG, "CloseWch success!\n");

	return 0;
}


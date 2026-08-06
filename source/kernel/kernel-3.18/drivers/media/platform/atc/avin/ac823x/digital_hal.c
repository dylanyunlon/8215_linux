/*************************************************************************************
 *LEGAL DISCLAIMER
 *
 * (Header of AutoChips Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF AUTOCHIPS SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY AUTOCHIPS SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
*************************************************************************************/


#include "wch_if.h"
#include "digital_hal.h"
#include "avin_common.h"


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
		pr_err("[AVIN]%s: param pBufIdx error!\r\n", __func__);
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
	pr_debug("[AVIN]%s: input index=%d\n", __func__, index);
	if((index >= 0) && (index <= 8)) {
		mSrcFmt = index;
	} else {
		pr_err("[AVIN]%s: input index(%d) error!\n", __func__, index);
		return -1;
	}

	return 0;
}

int digital_start_video(int mDigInFmt)
{
	mDigInFmt = mSrcFmt;
	pr_debug("[AVIN]%s: enter with mDigInFmt(%d)\n", __func__, mDigInFmt);
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
		g_rDigWchCtrl.tWchCfg.u4SrcWidth = 800; //mWidth;
		g_rDigWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rDigWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rDigWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rDigWchCtrl.tWchCfg.u4SrcStartYBot = 0x0;//above

		g_rDigWchCtrl.tWchCfg.u4DstWidth = 800; //mWidth;
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
		pr_err("[AVIN]%s: error with mDigInFmt(%d)\n", __func__, mDigInFmt);
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
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		return -1;
	}
	pr_debug("[AVIN]%s: ConfigWch success!\r\n", __func__);

	memset(&g_rDigWchBufInfo, 0, sizeof(WCH_BUF_T));
	g_rDigWchBufInfo.eSrcId = SRC_APP_DGI;
	if (WchGetBufferAddress(&g_rDigWchBufInfo)) {
		pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
		return -1;
	}
	pr_debug("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);

	if (StartWch(g_rDigWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		return -1;
	}
	pr_debug("[AVIN]%s: StartWch success!\r\n", __func__);

	mSigNotify = false;

	return 0;
}

int digital_stop_video(void)
{
	if (StopWch(g_rDigWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		return -1;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);

	if (CloseWch(g_rDigWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		return -1;
	}
	pr_debug("[AVIN]%s: CloseWch success!\r\n", __func__);

	return 0;
}


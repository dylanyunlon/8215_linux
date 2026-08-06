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


#include "tvd_hal_if.h"
#include "tvd_core.h"
#include "tvd_hw_reg.h"
#include "tvd_log.h"
#include "tdc.h"
#include <generated/atc_project.h>
#ifndef __ARM2__
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#endif

#define LINE_NON_STD_MIN  1695U
#define LINE_NON_STD_MAX  1761U
#define LINE_STD_MIN      1701U
#define LINE_STD_MAX      1749U
#define MODE_STABLE_COUNT        8U
#define IMAGE_STABLE_COUNT      16U
#define IS_LINE_STD(line)      ((((line) < LINE_STD_MAX) && ((line) > LINE_STD_MIN)) ? true : false)
#define IS_LINE_NON_STD(line)  ((((line) > LINE_NON_STD_MAX) || ((line) < LINE_NON_STD_MIN)) ? true : false)
#define IS_FH_NON_STD(tvd_base)        (fgHwTvdFHPos(tvd_base) || fgHwTvdFHNeg(tvd_base))
#define TVDAbsDiff(a, b)  (((a) > (b))?((a) - (b)):((b) - (a)))
#ifdef __ARM2__
#define IRQ_HANDLED
#if defined(CONFIG_ATC_PRJ_ac823x_adas)
unsigned int TDC_DRAM_BASE = 0x21400000;
#else
unsigned int TDC_DRAM_BASE = 0x24400000;
#endif
#else
unsigned long TDC_DRAM_BASE = 0;
#endif


extern unsigned int TDC_DRAM_SIZE;
extern struct device *tvDev;
bool tvd3DComb = true;
unsigned long memory_base = 0;

enum eNASTATE {
	NA_UNKNOWN,
	NA_LOWNOISE,
	NA_HIGHNOISE,
	NA_SUPERHIGHNOISE,
	NA_STOP
};

u32 g_u4RFThreshold       = 50u;
static u32 _u4Noiselevel_new[4];
static u32 _u4NRcnt[4];
static u32 _u4RF_level[4];
static struct tvd_notify_s notify_str = {NULL, NULL};
static const u32 nr_level[] = {
	75u, 86u, 109u, 182u, 394u, 663u, 1162u, 1994u, 3263u,
	5374u, 9530u, 17064u, 28304u, 42895u, 56133u, 72147u, 85691u
};
static const u32 fr_level[] = {
	80u, 75u, 70u, 65u, 60u, 57u, 54u, 51u, 48u, 45u, 42u, 39u,
	36u, 33u, 30u, 27u, 24u
};

#ifndef __ARM2__
typedef struct{
	bool signal_status;
	bool mode_status;
	bool vsync_status;
}TVD_EVENT;
TVD_EVENT tvd_event[4]={{false,false,false},
						{false,false,false},
						{false,false,false},
						{false,false,false}};
#endif

static bool wait_vsyn_16[4];
static bool signal_exist_status[4];
static bool mode_change_status[4];
static u32 vsync_count[4] = {1,1,1,1};
extern unsigned long  tvd_base[4];


void vTvdMeasureNR(TVD_CHANNEL_ID channel_id, bool fgEnable)
{
	u8 i;

	if (!fgEnable) {
		_u4NRcnt[channel_id] = 0u;
		_u4Noiselevel_new[channel_id] = 0u;
		_u4RF_level[channel_id] = 0u;
	} else {
		if (_u4NRcnt[channel_id] < 32u) {
			_u4NRcnt[channel_id]++;
			_u4Noiselevel_new[channel_id] += (u32)(TVD_READ32(tvd_base[channel_id] + REG_STA_REG2B) & VAR_CVBS_CLIP);
		} else {
			_u4NRcnt[channel_id] = 0u;

			for (i = (u8)0; i < (u8)17; i++) {
				if (_u4Noiselevel_new[channel_id] < nr_level[i]) {
					if (i == (u8)0) {
						_u4RF_level[channel_id] = fr_level[i];
						break;
					} else if (i < (u8)5) {
						_u4RF_level[channel_id] = fr_level[i - (u8)1] -
							      (((_u4Noiselevel_new[channel_id] - nr_level[i - (u8)1]) * 5u) /
							       (nr_level[i] - nr_level[i - (u8)1]));
						break;
					} else if (i < (u8)12) {
						_u4RF_level[channel_id] = fr_level[i - (u8)1] -
							      (((_u4Noiselevel_new[channel_id] - nr_level[i - (u8)1]) * 3u) /
							       (nr_level[i] - nr_level[i - (u8)1]));
						break;
					} else {
						_u4RF_level[channel_id] = fr_level[i - (u8)1] - ((((_u4Noiselevel_new[channel_id] >> 4) -
								       (nr_level[i - (u8)1] >> 4)) * 3u) /
								       ((nr_level[i] - nr_level[i - (u8)1]) >> 4));
						break;
					}
				} else {
					_u4RF_level[channel_id] = fr_level[i];
				}
			}

			_u4Noiselevel_new[channel_id] = 0u;
		}
	}
}

/**************************************************************************
* @brief  TVD STD/NSTD Line Setting
* @param
* @return None
**************************************************************************/
void vTVD_NonStandard_Auto_Control(TVD_CHANNEL_ID channel_id)
{
	u32 u4TvdHTotal, u4TvdVTotal;

	u4TvdHTotal = wHwTvdAvgLineCnt(tvd_base[channel_id]);
	u4TvdVTotal = wHwTvdAvgVLen(tvd_base[channel_id]);

	if (0u == fgHwTvdVPresTVD3D(tvd_base[channel_id])) {
		u4TvdHTotal = 1716u;
		u4TvdVTotal = 525u;
	}

	if (IS_LINE_NON_STD(u4TvdHTotal)) {
		TVD_WRITE32_MASK(tvd_base[channel_id] + REG_DFE_01, DFE_BLANK_WIN_START_NSTD_L, BLANK_WIN_START);
		TVD_SET_BIT(tvd_base[channel_id] + REG_TG_0D, HLEN_FHPOS_EN);
		TVD_SET_BIT(tvd_base[channel_id] + REG_CDET_00, MDET_V525_SEL);
		TVD_SET_BIT(tvd_base[channel_id] + REG_TG_04, LF_OFFSET_EN);
	} else if (IS_LINE_STD(u4TvdHTotal)) {
		TVD_WRITE32_MASK(tvd_base[channel_id] + REG_DFE_01, DFE_BLANK_WIN_START_STD_L, BLANK_WIN_START);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_TG_0D, HLEN_FHPOS_EN);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CDET_00, MDET_V525_SEL);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_TG_04, LF_OFFSET_EN);
	} else {
		TVD_LOG(TVD_LOG_LVL_WARN, "it is neither STD nor NON_STD\n");
	}

	if (bHwTvdNAState(tvd_base[channel_id]) == NA_LOWNOISE) {
		if (IS_LINE_NON_STD(u4TvdHTotal)) {
			TVD_SET_BIT(tvd_base[channel_id] + REG_CTG_05, BST_START_SEL);
			TVD_SET_BIT(tvd_base[channel_id] + REG_CTG_00, SOBVLD_MASK_EN);
		} else if (IS_LINE_STD(u4TvdHTotal)) {
			TVD_CLR_BIT(tvd_base[channel_id] + REG_CTG_05, BST_START_SEL);
			TVD_CLR_BIT(tvd_base[channel_id] + REG_CTG_00, SOBVLD_MASK_EN);
		} else {
			TVD_LOG(TVD_LOG_LVL_WARN, "it is neither STD nor NON_STD\n");
		}
	} else {
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CTG_05, BST_START_SEL);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CTG_00, SOBVLD_MASK_EN);
	}

	if (IS_FH_NON_STD(tvd_base[channel_id])) {
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CDET_04, BST_START_SEL);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CDET_04, SOBVLD_MASK_EN);
		TVD_CLR_BIT(tvd_base[channel_id] + REG_CTG_00, CTG_SWLBF);
	} else {
		TVD_SET_BIT(tvd_base[channel_id] + REG_CDET_04, BST_START_SEL);
		TVD_SET_BIT(tvd_base[channel_id] + REG_CDET_04, SOBVLD_MASK_EN);
		TVD_SET_BIT(tvd_base[channel_id] + REG_CTG_00, CTG_SWLBF);
	}

	if (((TVDAbsDiff(u4TvdHTotal, 1716) < 3) || (TVDAbsDiff(u4TvdHTotal, 1728) < 3))
	    && ((TVDAbsDiff(u4TvdVTotal, 525) < 10) || (TVDAbsDiff(u4TvdVTotal, 625) < 10))
	    && (fgHwTvdCoChannel(tvd_base[channel_id]) == 0) && (fgHwTvdTrick(tvd_base[channel_id]) == 0) && (fgHwTvdHeadSwitch(tvd_base[channel_id]) == 0)) {

		if (_u4RF_level[channel_id] > 75) {
			TVD_WRITE32(tvd_base[channel_id] + REG_DFE_1F, 0x55C29325);
		} else if (_u4RF_level[channel_id] < 65) {
			TVD_WRITE32(tvd_base[channel_id] + REG_DFE_1F, 0x2D229325);
		} else {
			TVD_LOG(TVD_LOG_LVL_DBG, "the clamp value is not suitable\n");
		}
	} else {
		TVD_WRITE32(tvd_base[channel_id] + REG_DFE_1F, 0x2D229325);
	}

}


static u32 _MapTvdMode(u32 u4SigSys)
{
	u32 u4CurAVMode = AV_MODE_NONE;

	switch (u4SigSys) {
	case AV_PAL_N:
	case AV_PAL:
		u4CurAVMode = AV_MODE_PAL;
		break;

	case AV_PAL_M:
		u4CurAVMode = AV_MODE_PAL_M;
		break;

	case AV_NTSC:
		u4CurAVMode = AV_MODE_NTSC;
		break;

	case AV_PAL_60:
		u4CurAVMode = AV_MODE_PAL60;
		break;

	case AV_NTSC443:
		u4CurAVMode = AV_MODE_NTSC443;
		break;

	case AV_SECAM:
		u4CurAVMode = AV_MODE_SECAM;
		break;

	default:
		break;
	}

	return u4CurAVMode;
}

static void signal_stable(TVD_CHANNEL_ID channel_id)
{
	u32 signal_detail_mode;
	u32 signal_mode;
	TVD_SIG_INFORMATION signal_infor;

	signal_detail_mode = Tvd_Core_GetMode(channel_id);
	signal_mode        = _MapTvdMode(signal_detail_mode);

	signal_infor.signal_state = TVD_SIG_READY;
	signal_infor.arg = signal_mode;
	signal_infor.channel_id = channel_id;
	
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal ready\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void signal_lost(TVD_CHANNEL_ID channel_id)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_LOST;
	signal_infor.channel_id = channel_id;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal lost\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void signal_close_wch(TVD_CHANNEL_ID channel_id)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_CLOSE_WCH;
	signal_infor.channel_id = channel_id;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal close wch\n");
		notify_str.notify_fun(&signal_infor);
	}
}


static void mode_change_start(TVD_CHANNEL_ID channel_id)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_CHANGE_START;
	signal_infor.channel_id = channel_id;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd mode start change\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void mode_change_done(TVD_CHANNEL_ID channel_id)
{
	u32 u4CurAVMode = AV_MODE_NONE;
	u32 u4SigSys    = AV_NONE;
	TVD_SIG_INFORMATION signal_infor;

	u4SigSys    = Tvd_Core_GetMode(channel_id);
	u4CurAVMode = _MapTvdMode(u4SigSys);

	TVD_LOG(TVD_LOG_LVL_INFO, "start to send tvd mode changed done message and the mode is %d\n", u4CurAVMode);
	Tvd_Core_SetMode(channel_id, u4SigSys);

	signal_infor.signal_state = TVD_SIG_CHANGE_DONE;
	signal_infor.arg = u4CurAVMode;
	signal_infor.channel_id = channel_id;
	
	if (NULL != notify_str.notify_fun) {
		notify_str.notify_fun(&signal_infor);
		TVD_LOG(TVD_LOG_LVL_INFO, "finish sending mode change done message!\n");
	}
}

static void color_kill(TVD_CHANNEL_ID channel_id)
{
	TVD_LOG(TVD_LOG_LVL_INFO, "color_kill enter\n");

	if ((TVD_READ32(tvd_base[channel_id] + REG_STA_CDET_00) & (CKILL)) != 0) {
		TVD_WRITE32_MASK(tvd_base[channel_id] + 0x568u, 0x80000000u, 0xFF000000u);
		TVD_LOG(TVD_LOG_LVL_WARN, "execute color kill operation\n");
	}

	TVD_LOG(TVD_LOG_LVL_INFO, "color_kill leave\n");
}


static void vsync_process(TVD_CHANNEL_ID channel_id)
{
	u32 signal_status = tvd_core_get_signal_state(channel_id);
	u32 u4CurAVMode = AV_MODE_NONE;
	u32 u4SigSys    = AV_NONE;
	/*this condition state that the signal is really lost,so return directlly*/
	if ((TVD_SIG_LOST == signal_status) && (false == wait_vsyn_16[channel_id])) {
		TVD_LOG(TVD_LOG_LVL_DBG, "signal is realy lost!\n");
		signal_exist_status[channel_id] = false;
		mode_change_status[channel_id] = false;
		vsync_count[channel_id] = 1;
		return;
	}

	switch (vsync_count[channel_id]) {
	case MODE_STABLE_COUNT:
		if (false == mode_change_status[channel_id]) {
			vsync_count[channel_id]++;
			return;
		}

		TVD_LOG(TVD_LOG_LVL_INFO, "have already wait 8 vsyncs,so the mode is stable\n");
		mode_change_done(channel_id);
		vsync_count[channel_id] = 1;
		mode_change_status[channel_id] = false;
		break;

	case IMAGE_STABLE_COUNT:
		if (false == wait_vsyn_16[channel_id]) {
			vsync_count[channel_id]++;
			return;
		}

		/*************************************************
		*this condition may be possible when the input line is unpined,so
		*the signal is really lost in the process of waiting for 16 vsync
		*************************************************/
		if (TVD_SIG_LOST == signal_status) {
			TVD_LOG(TVD_LOG_LVL_INFO, "the signal is lost in the process of waiting for 16 vsync\n");
			signal_lost(channel_id);
			vTvdMeasureNR(channel_id, false);
			signal_exist_status[channel_id] = false;
		} else {
			TVD_LOG(TVD_LOG_LVL_INFO, "have already wait 16 vsyncs,so the signal is stable\n");
			u4SigSys    = Tvd_Core_GetMode(channel_id);
			//u4CurAVMode = _MapTvdMode(u4SigSys);
			Tvd_Core_SetMode(channel_id, u4SigSys);
			color_kill(channel_id);
			signal_stable(channel_id);
			vsync_count[channel_id] = 1;
			wait_vsyn_16[channel_id] = false;
			signal_exist_status[channel_id] = true;
		}

		break;

	default:

		/*After signal exist & mode stable, it is not valuable,so recovery it to default value which is one*/
		if ((true == signal_exist_status[channel_id]) && (false == mode_change_status[channel_id])) {
			vsync_count[channel_id] = 1;
		} else {
			vsync_count[channel_id]++;
		}

		break;
	}
}



#ifdef __ARM2__
void _tvd_interrupt_process(s32 irq)
{
	u32 u4IrqStatus;
	u32 signal_status;
	u32 channel_id = 0;
	
	switch(irq){
		case 110:
			channel_id = TVD_CH_0;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD0 channel_id = %d \n", channel_id);
			break;
		case 116:
			channel_id = TVD_CH_1;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD1 channel_id = %d \n", channel_id);
			break;
		case 125:
			channel_id = TVD_CH_2;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD2 channel_id = %d \n", channel_id);
			break;
		case 148:
			channel_id = TVD_CH_3;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD3 channel_id = %d \n", channel_id);
			break;
		default:
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD channel_id = %d channel_id error\n", channel_id);
			break;
	}
	
	u4IrqStatus = Tvd_Core_IrqStatus(channel_id);
         
	if (u4IrqStatus & INTR_VPRES_TVD) {
		signal_status = tvd_core_get_signal_state(channel_id);
		TVD_LOG(TVD_LOG_LVL_INFO, "INTR_VPRES_TVD come,the signal status is %d,wait_vsyn_16 status is %d\n",
				signal_status, wait_vsyn_16[channel_id]);

		switch (signal_status) {
		case TVD_SIG_READY:

			/*this condition can avoid multiple response to signal ready interrupt*/
			if (false == wait_vsyn_16[channel_id]) {
				/*wait 16 vsync until the signal became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
				wait_vsyn_16[channel_id] = true;
			}

			break;

		case TVD_SIG_LOST:

			/*this condition indicate the signal is really lost*/
			if (false == wait_vsyn_16[channel_id]) {
				signal_lost(channel_id);
				vTvdMeasureNR(channel_id, false);
				signal_exist_status[channel_id] = false;
			}

			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "this is signal status is impossible\n");
			break;
		}

	} else if (u4IrqStatus & INTR_VSYNC_TVD) {
		//TVD_LOG(TVD_LOG_LVL_DBG, "vsync irq comming\n");
		vsync_process(channel_id);
	} else if (u4IrqStatus & INTR_MODE_TVD) {
		TVD_LOG(TVD_LOG_LVL_INFO, "vsync irq INTR_MODE_TVD\n");
		u32 u4CurAVMode = AV_MODE_NONE;
		u32 u4SigSys    = AV_NONE;

		u4SigSys    = Tvd_Core_GetMode(channel_id);
		u4CurAVMode = _MapTvdMode(u4SigSys);

		/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
		if ((true == signal_exist_status[channel_id]) && (false == mode_change_status[channel_id])) {
			mode_change_start(channel_id);
			mode_change_status[channel_id] = true;
			/*wait 8 vsync until the mode became stable*/
			TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
		} else {
			TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is %d,mode_change is %d\n",
					signal_exist_status[channel_id], mode_change_status[channel_id]);
		}
	} else {
		TVD_LOG(TVD_LOG_LVL_INFO, "we do not dear this interrupt\n");
	}
	v_clear_bim_irq(irq);
	return IRQ_HANDLED;
}

#else
int _signal_process(void *arg)
{
	unsigned long obj = 0;
	u32 signal_status;
	u32 mode_detail = AV_MODE_NONE;
	u32 tvd_mode    = AV_NONE;
	int ret = 0;
	u32 channel_id = 0;
	
	TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread enter\n");
	while(true) {
		if (kthread_should_stop()) {
			TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread should stop now!\n");
			break;
		}
		if(tvd_event[0].signal_status == true){
			tvd_event[0].signal_status = false;
			signal_status = tvd_core_get_signal_state(0);
				TVD_LOG(TVD_LOG_LVL_INFO, "current signal status is %d,wait_vsyn_16 flag is %d\n",
						signal_status, wait_vsyn_16[0]);
				switch (signal_status) {
				case TVD_SIG_READY:
					/*this condition can avoid multiple response to signal ready interrupt*/
					if (false == wait_vsyn_16[0]) {
						/*wait 16 vsync until the signal became stable*/
						TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
						wait_vsyn_16[0] = true;
					}
					break;

				case TVD_SIG_LOST:
					/*this condition indicate the signal is really lost*/
					if (false == wait_vsyn_16[0]) {
						signal_lost(0);
						vTvdMeasureNR(0, false);
						signal_exist_status[0] = false;
					}
					break;

				default:
					TVD_LOG(TVD_LOG_LVL_WARN, "this signal status is impossible\n");
					break;
				}
		}
		if(tvd_event[1].signal_status == true){
			tvd_event[1].signal_status = false;
			signal_status = tvd_core_get_signal_state(1);
				TVD_LOG(TVD_LOG_LVL_INFO, "current signal status is %d,wait_vsyn_16 flag is %d\n",
						signal_status, wait_vsyn_16[1]);
				switch (signal_status) {
				case TVD_SIG_READY:
					/*this condition can avoid multiple response to signal ready interrupt*/
					if (false == wait_vsyn_16[1]) {
						/*wait 16 vsync until the signal became stable*/
						TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
						wait_vsyn_16[1] = true;
					}
					break;

				case TVD_SIG_LOST:
					/*this condition indicate the signal is really lost*/
					if (false == wait_vsyn_16[1]) {
						signal_lost(1);
						vTvdMeasureNR(1, false);
						signal_exist_status[1] = false;
					}
					break;

				default:
					TVD_LOG(TVD_LOG_LVL_WARN, "this signal status is impossible\n");
					break;
				}
		}
		if(tvd_event[2].signal_status == true){
			tvd_event[2].signal_status = false;
			signal_status = tvd_core_get_signal_state(2);
				TVD_LOG(TVD_LOG_LVL_INFO, "current signal status is %d,wait_vsyn_16 flag is %d\n",
						signal_status, wait_vsyn_16[2]);
				switch (signal_status) {
				case TVD_SIG_READY:
					/*this condition can avoid multiple response to signal ready interrupt*/
					if (false == wait_vsyn_16[2]) {
						/*wait 16 vsync until the signal became stable*/
						TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
						wait_vsyn_16[2] = true;
					}
					break;

				case TVD_SIG_LOST:
					/*this condition indicate the signal is really lost*/
					if (false == wait_vsyn_16[2]) {
						signal_lost(2);
						vTvdMeasureNR(2, false);
						signal_exist_status[2] = false;
					}
					break;

				default:
					TVD_LOG(TVD_LOG_LVL_WARN, "this signal status is impossible\n");
					break;
				}
		}
		if(tvd_event[3].signal_status == true){
			tvd_event[3].signal_status = false;
			signal_status = tvd_core_get_signal_state(3);
				TVD_LOG(TVD_LOG_LVL_INFO, "current signal status is %d,wait_vsyn_16 flag is %d\n",
						signal_status, wait_vsyn_16[3]);
				switch (signal_status) {
				case TVD_SIG_READY:
					/*this condition can avoid multiple response to signal ready interrupt*/
					if (false == wait_vsyn_16[3]) {
						/*wait 16 vsync until the signal became stable*/
						TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
						wait_vsyn_16[3] = true;
					}
					break;

				case TVD_SIG_LOST:
					/*this condition indicate the signal is really lost*/
					if (false == wait_vsyn_16[3]) {
						signal_lost(3);
						vTvdMeasureNR(3, false);
						signal_exist_status[3] = false;
					}
					break;

				default:
					TVD_LOG(TVD_LOG_LVL_WARN, "this signal status is impossible\n");
					break;
				}
		}

		if(tvd_event[0].mode_status == true){
			tvd_event[0].mode_status = false;
			mode_detail = Tvd_Core_GetMode(0);
			tvd_mode = _MapTvdMode(mode_detail);

			/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
			if ((true == signal_exist_status[0]) && (false == mode_change_status[0])) {
				mode_change_start(0);
				mode_change_status[0] = true;
				/*wait 8 vsync until the mode became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
			} else {
				TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is not stable or mode is not stable,signal_exist_status/mode_change_status(%d/%d)\n",
						signal_exist_status[0], mode_change_status[0]);
			}
		}
		if(tvd_event[1].mode_status == true){
			tvd_event[1].mode_status = false;
			mode_detail = Tvd_Core_GetMode(1);
			tvd_mode = _MapTvdMode(mode_detail);

			/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
			if ((true == signal_exist_status[1]) && (false == mode_change_status[1])) {
				mode_change_start(1);
				mode_change_status[1] = true;
				/*wait 8 vsync until the mode became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
			} else {
				TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is not stable or mode is not stable,signal_exist_status/mode_change_status(%d/%d)\n",
						signal_exist_status[1], mode_change_status[1]);
			}
		}
		if(tvd_event[2].mode_status == true){
			tvd_event[2].mode_status = false;
			mode_detail = Tvd_Core_GetMode(2);
			tvd_mode = _MapTvdMode(mode_detail);

			/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
			if ((true == signal_exist_status[2]) && (false == mode_change_status[2])) {
				mode_change_start(2);
				mode_change_status[2] = true;
				/*wait 8 vsync until the mode became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
			} else {
				TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is not stable or mode is not stable,signal_exist_status/mode_change_status(%d/%d)\n",
						signal_exist_status[2], mode_change_status[2]);
			}
		}
		if(tvd_event[3].mode_status == true){
			tvd_event[3].mode_status = false;
			mode_detail = Tvd_Core_GetMode(3);
			tvd_mode = _MapTvdMode(mode_detail);

			/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
			if ((true == signal_exist_status[3]) && (false == mode_change_status[3])) {
				mode_change_start(3);
				mode_change_status[3] = true;
				/*wait 8 vsync until the mode became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
			} else {
				TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is not stable or mode is not stable,signal_exist_status/mode_change_status(%d/%d)\n",
						signal_exist_status[3], mode_change_status[3]);
			}
		}

		if(tvd_event[0].vsync_status == true){
			tvd_event[0].vsync_status = false;
			vsync_process(0);
		}
		if(tvd_event[1].vsync_status == true){
			tvd_event[1].vsync_status = false;
			vsync_process(1);
		}
		if(tvd_event[2].vsync_status == true){
			tvd_event[2].vsync_status = false;
			vsync_process(2);
		}
		if(tvd_event[3].vsync_status == true){
			tvd_event[3].vsync_status = false;
			vsync_process(3);
		}
		msleep(10);
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread leave\n");
	return ret;
}

irqreturn_t _tvd_interrupt_process(s32 irq, void *dev_id)
{
	u32 u4IrqStatus;

	u32 channel_id = 0;
	
	switch(irq){
		case 110:
			channel_id = TVD_CH_0;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD0 channel_id = %d \n", channel_id);
			break;
		case 116:
			channel_id = TVD_CH_1;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD1 channel_id = %d \n", channel_id);
			break;
		case 125:
			channel_id = TVD_CH_2;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD2 channel_id = %d \n", channel_id);
			break;
		case 148:
			channel_id = TVD_CH_3;
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD3 channel_id = %d \n", channel_id);
			break;
		default:
			TVD_LOG(TVD_LOG_LVL_DBG, "TVD channel_id = %d channel_id error\n", channel_id);
			break;
	}
	
	u4IrqStatus = Tvd_Core_IrqStatus(channel_id);
	if (u4IrqStatus & INTR_VPRES_TVD) {
		tvd_event[channel_id].signal_status = true;
	} else if (u4IrqStatus & INTR_MODE_TVD) {
		tvd_event[channel_id].mode_status = true;
	} else if (u4IrqStatus & INTR_VSYNC_TVD) {
		tvd_event[channel_id].vsync_status = true;
	} else {
		TVD_LOG(TVD_LOG_LVL_DBG, "we do not dear this interrupt\n");
	}
	mt33xx_mask_ack_bim_irq(irq);
	return IRQ_HANDLED;
}
#endif


bool tvd_get_di_flag(TVD_CHANNEL_ID channel_id)
{
	bool di_flag = false;

	if (false == signal_exist_status[channel_id]) {
		vTvdMeasureNR(channel_id, false);
	} else {
		vTvdMeasureNR(channel_id, true);
		vTVD_NonStandard_Auto_Control(channel_id);
		di_flag = true;
	}

	return di_flag;
}


bool tvd_channel_port_config(TVD_CHANNEL_ID channel_id, u32 channel, u32 port_num, u32 u4Rear, u32 u4CfgType)
{
	TVD_LOG(TVD_LOG_LVL_INFO, "cvbs channel port config enter!\n");

	if (TVD_ANALOG_CFG_CLAMP == u4CfgType || channel_id != TVD_CH_0) {
		return true;
	}
	
	if (channel == TVD_CHA) {
		switch(port_num){
			case 1:
				TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)&(~(RG_CVBS0P_CHA_SEL_CH0)));
            	TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0)))|(0x8 << 20));
				break;
			case 2:
				TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)|((RG_CVBS0P_CHA_SEL_CH0)));
            	TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0))));
				break;
			case 3:
				TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)&(~(RG_CVBS0P_CHA_SEL_CH0)));
            	TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0)))|(0x4 << 20));
				break;
			default:
				TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)&(~(RG_CVBS0P_CHA_SEL_CH0)));
            	TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0)))|(0x8 << 20));
				break;
		}
	}else {
		TVD_LOG(TVD_LOG_LVL_INFO, "please config tvd to channel_a path!\n");
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "cvbs channel port config leave!\n");
	return true;
	//return CVBS_By_Pass(channel_id, channel, port_num, u4Rear, u4CfgType);
}

void tvd_channel_on_off(TVD_CHANNEL_ID channel_id, u32 channel, bool on_off)
{
	if (TVD_CHA == channel) {
		Tvd_CHA_PowerOn(channel_id, on_off);
	}

	if (TVD_CHB == channel) {
		Tvd_CHB_PowerOn(channel_id, on_off);
	}
}

void tvd_clock_on_off(bool on_off)
{
	Tvd_ClkOnOff(0, on_off);
	Tvd_ClkOnOff(1, on_off);
	Tvd_ClkOnOff(2, on_off);
	Tvd_ClkOnOff(3, on_off);
}

void tvd_hal_notify_close_wch(TVD_CHANNEL_ID channel_id)
{
	signal_close_wch(channel_id);
}

void tvd_hal_open(TVD_CHANNEL_ID channel_id)
{
	bool i4Ret = false;
	TVD_LOG(TVD_LOG_LVL_OFF, "enter\n");

	Tvd_ClkOnOff(channel_id, true);
	if(tvd3DComb == true && channel_id == TVD_CH_0){
		TVD_LOG(TVD_LOG_LVL_INFO, "enable tvd 3D comb filter\n");
		i4Ret = vDrvTDCSetDramBase();
		if(i4Ret == true){
			vDrvTDCInit();
		}else{
			TVD_LOG(TVD_LOG_LVL_INFO, "set tvd 3D dram addr error\n");
		}
		
	}
	Tvd_Register_Rst(channel_id);
	Tvd_Core_Init(channel_id);
	Tvd_Core_IrqClear(channel_id, 0xFFFFFFFF);
	Tvd_Core_IrqEnable(channel_id, true);
	TVD_LOG(TVD_LOG_LVL_OFF, "leave\n");
}


TVD_SIG_STATE_T tvd_hal_get_signal_status(TVD_CHANNEL_ID channel_id)
{
	TVD_SIG_STATE_T signal_status = TVD_SIG_NONE;
	signal_status = tvd_core_get_signal_state(channel_id);
	
	#ifndef __ARM2__
	if(signal_status == TVD_SIG_LOST){
		msleep(80);
		signal_status = tvd_core_get_signal_state(channel_id);
	}
	#endif
	
	return signal_status;
}

void tvd_hal_close(TVD_CHANNEL_ID channel_id)
{
	Tvd_Core_DeInit(channel_id);
	if(tvd3DComb == true && channel_id == TVD_CH_0){
		vDrvTDCOnOff(SV_OFF);
	}
	Tvd_ClkOnOff(channel_id, false);
	wait_vsyn_16[channel_id] = false;
	signal_exist_status[channel_id] = false;
	vsync_count[channel_id] = 1;
	#ifndef __ARM2__
	tvd_event[channel_id].signal_status = false;
	tvd_event[channel_id].mode_status = false;
	tvd_event[channel_id].vsync_status = false;
	#endif
}

s8 _tvd_register_notify(tvd_notify notify_fun, void *arg)
{
	notify_str.notify_fun = notify_fun;
	notify_str.arg = arg;
	return 0;
}


void _tvd_unregister_notify(void)
{
	notify_str.notify_fun = NULL;
	notify_str.arg = NULL;
}

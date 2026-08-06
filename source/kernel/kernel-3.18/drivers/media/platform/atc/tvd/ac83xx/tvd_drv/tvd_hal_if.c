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
#ifndef __ARM2__
#include "x_os.h"
#include "winutil.h"
#include <linux/kthread.h>
#endif

#ifdef __ARM2__
#define IRQ_HANDLED
#endif

#define LINE_NON_STD_MIN  1695U
#define LINE_NON_STD_MAX  1761U
#define LINE_STD_MIN      1701U
#define LINE_STD_MAX      1749U
#define MODE_STABLE_COUNT        8U
#define IMAGE_STABLE_COUNT      16U
#define IS_LINE_STD(line)      ((((line) < LINE_STD_MAX) && ((line) > LINE_STD_MIN)) ? true : false)
#define IS_LINE_NON_STD(line)  ((((line) > LINE_NON_STD_MAX) || ((line) < LINE_NON_STD_MIN)) ? true : false)
#define IS_FH_NON_STD()        (fgHwTvdFHPos() || fgHwTvdFHNeg())
#define TVDAbsDiff(a, b)  (((a) > (b))?((a) - (b)):((b) - (a)))

enum eNASTATE {
	NA_UNKNOWN,
	NA_LOWNOISE,
	NA_HIGHNOISE,
	NA_SUPERHIGHNOISE,
	NA_STOP
};

u32 g_u4RFThreshold       = 50u;
static u32 _u4Noiselevel_new;
static u32 _u4NRcnt;
static u32 _u4RF_level;
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
enum TVD_EVENT {
	TVD_SIGNAL_STATUS_EVENT = 0,
	TVD_MODE_STATUS_EVENT,
	TVD_VSYNC_COUNT_EVENT,
	TVD_EXIT_EVENT,
	TVD_EVENT_MAX
};
HANDLE tvd_event_arr[TVD_EVENT_MAX] = {NULL};
const char *event_name[TVD_EVENT_MAX] = {"signal_status_event", "mode_status_event", "vsync_count_event", "exit_event"};
#endif
static bool wait_vsyn_16;
static bool signal_exist_status;
static bool mode_change_status;
static u32 vsync_count = 1;
static u32 g_u4UVSwap;


void vTvdMeasureNR(bool fgEnable)
{
	u8 i;

	if (!fgEnable) {
		_u4NRcnt = 0u;
		_u4Noiselevel_new = 0u;
		_u4RF_level = 0u;
	} else {
		if (_u4NRcnt < 32u) {
			_u4NRcnt++;
			_u4Noiselevel_new += (u32)(TVD_READ32(REG_STA_REG2B) & VAR_CVBS_CLIP);
		} else {
			_u4NRcnt = 0u;

			for (i = (u8)0; i < (u8)17; i++) {
				if (_u4Noiselevel_new < nr_level[i]) {
					if (i == (u8)0) {
						_u4RF_level = fr_level[i];
						break;
					} else if (i < (u8)5) {
						_u4RF_level = fr_level[i - (u8)1] -
							      (((_u4Noiselevel_new - nr_level[i - (u8)1]) * 5u) /
							       (nr_level[i] - nr_level[i - (u8)1]));
						break;
					} else if (i < (u8)12) {
						_u4RF_level = fr_level[i - (u8)1] -
							      (((_u4Noiselevel_new - nr_level[i - (u8)1]) * 3u) /
							       (nr_level[i] - nr_level[i - (u8)1]));
						break;
					} else {
						_u4RF_level = fr_level[i - (u8)1] - ((((_u4Noiselevel_new >> 4) -
								       (nr_level[i - (u8)1] >> 4)) * 3u) /
								       ((nr_level[i] - nr_level[i - (u8)1]) >> 4));
						break;
					}
				} else {
					_u4RF_level = fr_level[i];
				}
			}

			_u4Noiselevel_new = 0u;
		}
	}
}

/**************************************************************************
* @brief  TVD STD/NSTD Line Setting
* @param
* @return None
**************************************************************************/
void vTVD_NonStandard_Auto_Control(void)
{
	u32 u4TvdHTotal, u4TvdVTotal;

	u4TvdHTotal = wHwTvdAvgLineCnt();
	u4TvdVTotal = wHwTvdAvgVLen();

	if (0u == fgHwTvdVPresTVD3D()) {
		u4TvdHTotal = 1716u;
		u4TvdVTotal = 525u;
	}

	if (IS_LINE_NON_STD(u4TvdHTotal)) {
		TVD_WRITE32_MASK(REG_DFE_01, DFE_BLANK_WIN_START_NSTD_L, BLANK_WIN_START);
		TVD_SET_BIT(REG_TG_0D, HLEN_FHPOS_EN);
		TVD_SET_BIT(REG_CDET_00, MDET_V525_SEL);
		TVD_SET_BIT(REG_TG_04, LF_OFFSET_EN);
	} else if (IS_LINE_STD(u4TvdHTotal)) {
		TVD_WRITE32_MASK(REG_DFE_01, DFE_BLANK_WIN_START_STD_L, BLANK_WIN_START);
		TVD_CLR_BIT(REG_TG_0D, HLEN_FHPOS_EN);
		TVD_CLR_BIT(REG_CDET_00, MDET_V525_SEL);
		TVD_CLR_BIT(REG_TG_04, LF_OFFSET_EN);
	} else {
		TVD_LOG(TVD_LOG_LVL_WARN, "it is neither STD nor NON_STD\n");
	}

	if (bHwTvdNAState() == NA_LOWNOISE) {
		if (IS_LINE_NON_STD(u4TvdHTotal)) {
			TVD_SET_BIT(REG_CTG_05, BST_START_SEL);
			TVD_SET_BIT(REG_CTG_00, SOBVLD_MASK_EN);
		} else if (IS_LINE_STD(u4TvdHTotal)) {
			TVD_CLR_BIT(REG_CTG_05, BST_START_SEL);
			TVD_CLR_BIT(REG_CTG_00, SOBVLD_MASK_EN);
		} else {
			TVD_LOG(TVD_LOG_LVL_WARN, "it is neither STD nor NON_STD\n");
		}
	} else {
		TVD_CLR_BIT(REG_CTG_05, BST_START_SEL);
		TVD_CLR_BIT(REG_CTG_00, SOBVLD_MASK_EN);
	}

	if (IS_FH_NON_STD()) {
		TVD_CLR_BIT(REG_CDET_04, BST_START_SEL);
		TVD_CLR_BIT(REG_CDET_04, SOBVLD_MASK_EN);
		TVD_CLR_BIT(REG_CTG_00, CTG_SWLBF);
	} else {
		TVD_SET_BIT(REG_CDET_04, BST_START_SEL);
		TVD_SET_BIT(REG_CDET_04, SOBVLD_MASK_EN);
		TVD_SET_BIT(REG_CTG_00, CTG_SWLBF);
	}

	if (((TVDAbsDiff(u4TvdHTotal, 1716) < 3) || (TVDAbsDiff(u4TvdHTotal, 1728) < 3))
	    && ((TVDAbsDiff(u4TvdVTotal, 525) < 10) || (TVDAbsDiff(u4TvdVTotal, 625) < 10))
	    && (fgHwTvdCoChannel() == 0) && (fgHwTvdTrick() == 0) && (fgHwTvdHeadSwitch() == 0)) {

		if (_u4RF_level > 75) {
			TVD_WRITE32(REG_DFE_1F, 0x55C29325);
		} else if (_u4RF_level < 65) {
			TVD_WRITE32(REG_DFE_1F, 0x2D229325);
		} else {
			TVD_LOG(TVD_LOG_LVL_DBG, "the clamp value is not suitable\n");
		}
	} else {
		TVD_WRITE32(REG_DFE_1F, 0x2D229325);
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
	case AV_NTSC:
	case AV_PAL_60:
		u4CurAVMode = AV_MODE_NTSC;
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

static void signal_stable(void)
{
	u32 signal_detail_mode;
	u32 signal_mode;
	TVD_SIG_INFORMATION signal_infor;

	signal_detail_mode = Tvd_Core_GetMode();
	signal_mode        = _MapTvdMode(signal_detail_mode);

	signal_infor.signal_state = TVD_SIG_READY;
	signal_infor.arg = signal_mode;

	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal ready\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void signal_lost(void)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_LOST;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal lost\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void signal_close_wch(void)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_CLOSE_WCH;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal close wch\n");
		notify_str.notify_fun(&signal_infor);
	}
}


static void mode_change_start(void)
{
	TVD_SIG_INFORMATION signal_infor;

	signal_infor.signal_state = TVD_SIG_CHANGE_START;
	if (NULL != notify_str.notify_fun) {
		TVD_LOG(TVD_LOG_LVL_INFO, "tvd mode start change\n");
		notify_str.notify_fun(&signal_infor);
	}
}

static void mode_change_done(void)
{
	u32 u4CurAVMode = AV_MODE_NONE;
	u32 u4SigSys    = AV_NONE;
	TVD_SIG_INFORMATION signal_infor;

	u4SigSys    = Tvd_Core_GetMode();
	u4CurAVMode = _MapTvdMode(u4SigSys);

	TVD_LOG(TVD_LOG_LVL_INFO, "start to send tvd mode changed done message and the mode is %d\n", u4CurAVMode);
	Tvd_Core_SetMode(u4CurAVMode);
	if (g_u4UVSwap) {
		Tvd_Core_SetUVSwap(true);
	}

	signal_infor.signal_state = TVD_SIG_CHANGE_DONE;
	signal_infor.arg = u4CurAVMode;

	if (NULL != notify_str.notify_fun) {
		notify_str.notify_fun(&signal_infor);
		TVD_LOG(TVD_LOG_LVL_INFO, "finish sending mode change done message!\n");
	}
}

static void color_kill(void)
{
	TVD_LOG(TVD_LOG_LVL_INFO, "color_kill enter\n");

	if ((TVD_READ32(REG_STA_CDET_00) & (CKILL)) != 0) {
		TVD_WRITE32_MASK(0x568u, 0x80000000u, 0xFF000000u);
		TVD_LOG(TVD_LOG_LVL_WARN, "execute color kill operation\n");
	}

	TVD_LOG(TVD_LOG_LVL_INFO, "color_kill leave\n");
}

UINT32 get_cur_time(void)
{
	UINT32 cur_count = 0;

	cur_count = (*((volatile UINT32 *)(0xFD00814C)));
	return ((0xFFFFFFFF - cur_count)/27000);
}

static void vsync_process(void)
{
	u32 signal_status = tvd_core_get_signal_state();
	u32 u4CurAVMode = AV_MODE_NONE;
	u32 u4SigSys    = AV_NONE;
	/*this condition state that the signal is really lost,so return directlly*/
	if ((TVD_SIG_LOST == signal_status) && (false == wait_vsyn_16)) {
		TVD_LOG(TVD_LOG_LVL_DBG, "signal is realy lost!\n");
		signal_exist_status = false;
		mode_change_status = false;
		vsync_count = 1;
		return;
	}

	switch (vsync_count) {
	case MODE_STABLE_COUNT:
		if (false == mode_change_status) {
			vsync_count++;
			return;
		}

		TVD_LOG(TVD_LOG_LVL_INFO, "have already wait 8 vsyncs,so the mode is stable\n");
		mode_change_done();
		vsync_count = 1;
		mode_change_status = false;
		break;

	case IMAGE_STABLE_COUNT:
		if (false == wait_vsyn_16) {
			vsync_count++;
			return;
		}

		/*************************************************
		*this condition may be possible when the input line is unpined,so
		*the signal is really lost in the process of waiting for 16 vsync
		*************************************************/
		if (TVD_SIG_LOST == signal_status) {
			TVD_LOG(TVD_LOG_LVL_INFO, "the signal is lost in the process of waiting for 16 vsync\n");
			signal_lost();
			vTvdMeasureNR(false);
			signal_exist_status = false;
		} else {
			TVD_LOG(TVD_LOG_LVL_INFO, "have already wait 16 vsyncs,so the signal is stable and consume time is %ums\n", get_cur_time());
			u4SigSys    = Tvd_Core_GetMode();
			u4CurAVMode = _MapTvdMode(u4SigSys);
			Tvd_Core_SetMode(u4CurAVMode);
			if (g_u4UVSwap) {
				Tvd_Core_SetUVSwap(true);
			}
			color_kill();
			signal_stable();
			vsync_count = 1;
			wait_vsyn_16 = false;
			signal_exist_status = true;
		}

		break;

	default:

		/*After signal exist & mode stable, it is not valuable,so recovery it to default value which is one*/
		if ((true == signal_exist_status) && (false == mode_change_status)) {
			vsync_count = 1;
		} else {
			vsync_count++;
		}

		break;
	}
}
#ifdef __ARM2__
#ifdef INIT_TVD_BEFORE_ARM2_START
void wait_signal_stable(void)
{
	u32 pre_time;
	u32 cur_time = 0;
	u32 consume_time = 0;
	u32 lock_status = 0;

	TVD_LOG(TVD_LOG_LVL_INFO, "wait_signal_stable function enter\n");

	pre_time = get_cur_time();
	lock_status = TVD_READ32(0x088U);
	while ((lock_status & 0xF) != 0xF) {
		lock_status = TVD_READ32(0x088U);
	}
	cur_time = get_cur_time();
	consume_time = cur_time- pre_time;
	TVD_LOG(TVD_LOG_LVL_INFO, "wait_signal_stable function leave and consume time is %ums\n", consume_time);
}

void wait_mode_stable(void)
{
	u32 pre_time;
	u32 cur_time = 0;
	u32 consume_time = 0;
	u32 burst_lock = 0;

	TVD_LOG(TVD_LOG_LVL_INFO, "wait_mode_stable function enter\n");

	pre_time = get_cur_time();
	burst_lock = ((TVD_READ32(0x09cU) & (0x1U << 8)) >> 8);
	while (!(burst_lock == 0x1)) {
		burst_lock = ((TVD_READ32(0x09cU) & (0x1U << 8)) >> 8);
	}
	cur_time = get_cur_time();
	consume_time = cur_time- pre_time;
	TVD_LOG(TVD_LOG_LVL_INFO, "wait_mode_stable function leave and consume time is %ums\n", consume_time);
}

void _tvd_interrupt_process(s32 irq)
{
	u32 u4IrqStatus;
	u32 signal_status;

	u32 mode_detail = 0;
	u32 signal_mode = 0;
	static u32 previous_mode = AV_MODE_NONE;

	u4IrqStatus = Tvd_Core_IrqStatus();

	if (u4IrqStatus & INTR_VPRES_TVD) {
		signal_status = tvd_core_get_signal_state();
		TVD_LOG(TVD_LOG_LVL_INFO, "INTR_VPRES_TVD come,the signal status is %d and consume time is %ums\n", signal_status, get_cur_time());

		switch (signal_status) {
		case TVD_SIG_READY:
			TVD_LOG(TVD_LOG_LVL_INFO, "signal ready and now wait signal stable\n");
			wait_signal_stable();
			mode_detail    = Tvd_Core_GetMode();
			signal_mode = _MapTvdMode(mode_detail);
			TVD_LOG(TVD_LOG_LVL_INFO, "the signal is really stable,signal mode is %d and consume time is %ums\n", signal_mode, get_cur_time());
			Tvd_Core_SetMode(signal_mode);
			color_kill();
			signal_stable();
			previous_mode = signal_mode;
			signal_exist_status = true;
			break;

		case TVD_SIG_LOST:
			TVD_LOG(TVD_LOG_LVL_INFO, "signal lost\n");
			/*this condition indicate the signal is really lost*/
			signal_lost();
			vTvdMeasureNR(false);
			previous_mode = AV_MODE_NONE;
			signal_exist_status = false;
			break;

		default:
			TVD_LOG(TVD_LOG_LVL_ERR, "this signal status is impossible\n");
			break;
		}
	} else if (u4IrqStatus & INTR_MODE_TVD) {
		TVD_LOG(TVD_LOG_LVL_INFO, "INTR_MODE_TVD come\n");
		if (signal_exist_status) {
			wait_signal_stable();
			mode_detail    = Tvd_Core_GetMode();
			signal_mode = _MapTvdMode(mode_detail);
			if (previous_mode != signal_mode) {
				TVD_LOG(TVD_LOG_LVL_INFO, "the mode is really stable,current mode is %d\n", signal_mode);
				previous_mode = signal_mode;
				mode_change_start();
				mode_change_done();
			} else {
				TVD_LOG(TVD_LOG_LVL_INFO, "too many mode change interrupts when power on dvd machine, so we discard some\n");
			}
		}else {
			TVD_LOG(TVD_LOG_LVL_INFO, "the signal is not stable, so we discard the mode change interrupt and lock_status is 0x%08x\n", (*((volatile u32 *)(0xFD0A709c))));
		}
	} else {
		TVD_LOG(TVD_LOG_LVL_DBG, "we do not dear this interrupt\n");
	}

	ac83xx_mask_ack_bim_irq(irq);
	return IRQ_HANDLED;
}

#else
void _tvd_interrupt_process(s32 irq)
{
	u32 u4IrqStatus;
	u32 signal_status;

	u4IrqStatus = Tvd_Core_IrqStatus();

	if (u4IrqStatus & INTR_VPRES_TVD) {
		signal_status = tvd_core_get_signal_state();
		TVD_LOG(TVD_LOG_LVL_INFO, "INTR_VPRES_TVD come,the signal status is %d,wait_vsyn_16 status is %d and consume time is %ums\n",
				signal_status, wait_vsyn_16, get_cur_time());

		switch (signal_status) {
		case TVD_SIG_READY:

			/*this condition can avoid multiple response to signal ready interrupt*/
			if (false == wait_vsyn_16) {
				/*wait 16 vsync until the signal became stable*/
				TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
				wait_vsyn_16 = true;
			}

			break;

		case TVD_SIG_LOST:

			/*this condition indicate the signal is really lost*/
			if (false == wait_vsyn_16) {
				signal_lost();
				vTvdMeasureNR(false);
				signal_exist_status = false;
			}

			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "this is signal status is impossible\n");
			break;
		}

	} else if (u4IrqStatus & INTR_WFF_VSYNC_TVD) {
		vsync_process();
	} else if (u4IrqStatus & INTR_MODE_TVD) {
		u32 u4CurAVMode = AV_MODE_NONE;
		u32 u4SigSys    = AV_NONE;

		u4SigSys    = Tvd_Core_GetMode();
		u4CurAVMode = _MapTvdMode(u4SigSys);

		/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
		if ((true == signal_exist_status) && (false ==	mode_change_status)) {
			mode_change_start();
			mode_change_status = true;
			/*wait 8 vsync until the mode became stable*/
			TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
		} else {
			TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is %d,mode_change is %d\n",
					signal_exist_status, mode_change_status);
		}
	} else {
		TVD_LOG(TVD_LOG_LVL_DBG, "we do not dear this interrupt\n");
	}

	ac83xx_mask_ack_bim_irq(irq);
	return IRQ_HANDLED;
}
#endif
#else
int _create_event(void)
{
	int i,j;
	int ret = 0;

	TVD_LOG(TVD_LOG_LVL_INFO, "create event enter\n");
	for (i = 0; i < TVD_EVENT_MAX; i++) {
		tvd_event_arr[i] = x_event_create(NULL, false, false, event_name[i]);
		if (NULL == tvd_event_arr[i]) {
			TVD_LOG(TVD_LOG_LVL_ERR, "create %s event failed\n", event_name[i]);
			for (j = i - 1; j >= 0; j--) {
				x_event_destroy(tvd_event_arr[j]);
				tvd_event_arr[j] = NULL;
			}
			ret = -1;
			return ret;
		}
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "create event leave\n");
	return ret;
}

int _signal_process(void *arg)
{
	unsigned long obj = 0;
	u32 signal_status;
	u32 mode_detail = AV_MODE_NONE;
	u32 tvd_mode    = AV_NONE;
	bool exit_thread = false;
	int ret = 0;

	TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread enter\n");
	while(true) {
		if (kthread_should_stop()) {
			TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread should stop now!\n");
			break;
		}
		if (exit_thread) {
			continue;
		}
		obj = x_event_wait_for_objects(TVD_EVENT_MAX, tvd_event_arr, false , INFINITE);
		switch(obj) {
			case (WAIT_OBJECT_0 + 0):
				signal_status = tvd_core_get_signal_state();
				TVD_LOG(TVD_LOG_LVL_INFO, "current signal status is %d,wait_vsyn_16 flag is %d\n",
						signal_status, wait_vsyn_16);
				switch (signal_status) {
				case TVD_SIG_READY:
					/*this condition can avoid multiple response to signal ready interrupt*/
					if (false == wait_vsyn_16) {
						/*wait 16 vsync until the signal became stable*/
						TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 16 vsync until the signal became stable\n");
						wait_vsyn_16 = true;
					}
					break;

				case TVD_SIG_LOST:
					/*this condition indicate the signal is really lost*/
					if (false == wait_vsyn_16) {
						signal_lost();
						vTvdMeasureNR(false);
						signal_exist_status = false;
					}
					break;

				default:
					TVD_LOG(TVD_LOG_LVL_WARN, "this signal status is impossible\n");
					break;
				}
				break;
			case (WAIT_OBJECT_0 + 1):
				mode_detail = Tvd_Core_GetMode();
				tvd_mode = _MapTvdMode(mode_detail);

				/*the mode change interrupt is valid, only when the signal is stable & the mode is stable*/
				if ((true == signal_exist_status) && (false == mode_change_status)) {
					mode_change_start();
					mode_change_status = true;
					/*wait 8 vsync until the mode became stable*/
					TVD_LOG(TVD_LOG_LVL_INFO, "start to wait 8 vsync until the mode became stable\n");
				} else {
					TVD_LOG(TVD_LOG_LVL_WARN, "the INTR_MODE_TVD discard,because signal is not stable or mode is not stable,signal_exist_status/mode_change_status(%d/%d)\n",
							signal_exist_status, mode_change_status);
				}
				break;
			case (WAIT_OBJECT_0 + 2):
				vsync_process();
				break;
			case (WAIT_OBJECT_0 + 3):
				TVD_LOG(TVD_LOG_LVL_INFO, "want to exit the tvd signal process thread\n");
				exit_thread = true;
				break;
			default:
				break;
		}
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "tvd signal process thread leave\n");
	return ret;
}

void _set_exit_event(void)
{
	TVD_LOG(TVD_LOG_LVL_INFO, "set exit event function enter\n");
	if (tvd_event_arr[TVD_EXIT_EVENT]) {
		x_event_set(tvd_event_arr[TVD_EXIT_EVENT]);
	} else {
		TVD_LOG(TVD_LOG_LVL_ERR, "the tvd exit event is null\n");
	}
	TVD_LOG(TVD_LOG_LVL_INFO, "set exit event function leave\n");
}

irqreturn_t _tvd_interrupt_process(s32 irq, void *dev_id)
{
	u32 u4IrqStatus;

	u4IrqStatus = Tvd_Core_IrqStatus();
	if (u4IrqStatus & INTR_VPRES_TVD) {
		if (tvd_event_arr[TVD_SIGNAL_STATUS_EVENT]) {
			x_event_set(tvd_event_arr[TVD_SIGNAL_STATUS_EVENT]);
		} else {
			TVD_LOG(TVD_LOG_LVL_ERR, "the signal status event is null\n");
		}
	} else if (u4IrqStatus & INTR_MODE_TVD) {
		if (tvd_event_arr[TVD_MODE_STATUS_EVENT]) {
			x_event_set(tvd_event_arr[TVD_MODE_STATUS_EVENT]);
		} else {
			TVD_LOG(TVD_LOG_LVL_ERR, "the tvd mode status event is null\n");
		}
	} else if (u4IrqStatus & INTR_WFF_VSYNC_TVD) {
		if (tvd_event_arr[TVD_VSYNC_COUNT_EVENT]) {
			x_event_set(tvd_event_arr[TVD_VSYNC_COUNT_EVENT]);
		} else {
			TVD_LOG(TVD_LOG_LVL_ERR, "the tvd vsync count event is null\n");
		}
	} else {
		TVD_LOG(TVD_LOG_LVL_DBG, "we do not dear this interrupt\n");
	}
	ac83xx_mask_ack_bim_irq(irq);
	return IRQ_HANDLED;
}
#endif


bool tvd_get_di_flag(void)
{
	bool di_flag = false;

	if (false == signal_exist_status) {
		vTvdMeasureNR(false);
	} else {
		vTvdMeasureNR(true);
		vTVD_NonStandard_Auto_Control();
		di_flag = true;
	}

	return di_flag;
}


bool tvd_channel_port_config(u32 channel, u32 port_num, u32 u4Rear, u32 u4CfgType)
{
	return CVBS_By_Pass(channel, port_num, u4Rear, u4CfgType);
}

void tvd_channel_on_off(u32 channel, bool on_off)
{
	if (TVD_CHA == channel) {
		Tvd_CHA_PowerOn(on_off);
	}

	if (TVD_CHB == channel) {
		Tvd_CHB_PowerOn(on_off);
	}
}

void tvd_clock_on_off(bool on_off)
{
	Tvd_ClkOnOff(on_off);
}

void tvd_hal_notify_close_wch(void)
{
	signal_close_wch();
}

void tvd_hal_open(u32 u4UVSwap)
{
	TVD_LOG(TVD_LOG_LVL_OFF, "enter\n");

	Tvd_ClkOnOff(true);
	Tvd_Register_Rst();
	Tvd_Core_Init();
	Tvd_Core_IrqClear(0xFFFFFFFF);
	Tvd_Core_IrqEnable(true);
	g_u4UVSwap = u4UVSwap;

	TVD_LOG(TVD_LOG_LVL_OFF, "leave\n");
}


TVD_SIG_STATE_T tvd_hal_get_signal_status(void)
{
	return tvd_core_get_signal_state();
}

void tvd_hal_close(void)
{
	int i;

	Tvd_Core_DeInit();
	Tvd_ClkOnOff(false);
	wait_vsyn_16 = false;
	signal_exist_status = false;
	vsync_count = 1;
	g_u4UVSwap = 0;
#ifndef __ARM2__
	for (i = 0; i < TVD_EVENT_MAX; i++) {
		if (NULL != tvd_event_arr[i]) {
			x_event_destroy(tvd_event_arr[i]);
			tvd_event_arr[i] = NULL;
		}
	}
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






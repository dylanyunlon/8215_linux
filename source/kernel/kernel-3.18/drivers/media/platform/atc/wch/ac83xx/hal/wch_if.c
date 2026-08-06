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
#ifndef __ARM2__
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include "windows.h"
#include <linux/dma-mapping.h>
#else
#include "x_types.h"
#endif
#include "winutil.h"
#include "mach/ac83xx_irqs_vector.h"
#include "x_os.h"
#include "atc/wch_drv.h"
#include "wch_log.h"
#include "atc/wch_if.h"
#include "wch_hal.h"
#include "wch_hw.h"
#include <generated/atc_project.h>

#define WCH_WRITED_BUF_IDX_OFT 2U
static DEFINE_SPINLOCK(wch_isr_lock);
#ifndef __ARM2__
static HANDLE hWchThreadEvent[2];
static u32 WchBuffIdx[2];
struct task_struct *hWchInst[2] = {NULL, NULL};
/*add for protecting WchGetBuffIdxThread*/
static DEFINE_SPINLOCK(protect_gWchParam0);
static DEFINE_SPINLOCK(protect_gWchParam1);
static unsigned long fg_gWchParam[2];
u32 _u4WCH_DBG_LVL = (u32)WCH_LOG_LVL_DBG;

#else
u32 _u4WCH_DBG_LVL = (u32)WCH_LOG_LVL_HAL;

#endif
u8 *_pcWchLogLevel[] = {
	"[WCH] [OFF]",
	"[WCH] [ERR]",
	"[WCH] [WARN]",
	"[WCH] [INFO]",
	"[WCH] [HAL]",
	"[WCH] [DBG]",
	"[WCH] [TRACE]",
	"[WCH] [IRQ]",
	"[WCH] [REGRW]",
};
HANDLE _hWchEvent[3] = {NULL, NULL, NULL};

WCH_IF_PARAM_T _gWchParam[3];
bool fgWchIsrInit[2] = {false, false};

u32 u4SkipVysncCnt[2] = {WCH_SKIP_BUFF_CNT, WCH_SKIP_BUFF_CNT};

bool fgGrabWchFront = false;
bool fgWchLockInit = false;
#ifndef __ARM2__
static spinlock_t lock;
#else
static int lock;
#define spin_lock_irqsave(lock, flags)
#define spin_unlock_irqrestore(lock, flags)
#define spin_lock_init(lock)
#endif
unsigned long flags;
u32 u4LockCnt = 0;

#if MONITOR_UPDATEBUF_TIME
u32 u4WchCnt = 0;
struct timeval start_t[2];

#endif

#if WCH_TEST_THREAD
static int WCH_TEST(void *unused);
bool fgStartTestThread = false;
bool fgInitWchCtl = false;
WCH_CTL_PARAM_T tWchCtlParam;

#endif

#if !WCH_HDMI_TIMING_EN
/* For HDMI Control Flow */
u32 uStopParam = 0x1U;
EXPORT_SYMBOL(uStopParam);

u32 uStartParam = 0x1U;
EXPORT_SYMBOL(uStartParam);

DECLARE_WAIT_QUEUE_HEAD(wchqueue);
EXPORT_SYMBOL(wchqueue);

#endif
WCH_TIMING_PARAM_T *prWchRetiming = NULL;
WCH_TIMING_PARAM_T WchTimingTable[] = {

	/* Define: Timing Mode, HsyncInv, HPixel, HActive, VsyncInv, VTopLine, VBotLine, Vactive */
	{WCH_640_480P_60HZ, 1, 48, 640, 1, 33, 33, 480},
	{WCH_1280_720P_60HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_1920_1080I_60HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_720_480P_60HZ, 1, 60, 720, 1, 30, 30, 480},
	{WCH_720_480I_60HZ, 1, 114, 1440, 1, 15, 16, 240},
	{WCH_1280_720P_50HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_1920_1080I_50HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_720_576P_50HZ, 1, 68, 720, 1, 39, 39, 576},
	{WCH_720_576I_50HZ, 1, 138, 1440, 1, 19, 20, 288},
	{WCH_720_240P_60HZ, 1, 114, 1440, 1, 15, 15, 240},
	{WCH_2880_480I_60HZ, 1, 228, 2880, 1, 15, 12, 240},
	{WCH_2880_240P_60HZ, 1, 228, 2880, 1, 15, 15, 240},
	{WCH_1440_480P_60HZ, 1, 120, 1440, 1, 30, 30, 480},
	{WCH_1920_1080P_60HZ, 0, 148, 1920, 0, 36, 36, 1080},
	{WCH_720_288P_50HZ, 1, 138, 1440, 1, 18, 18, 288},
	{WCH_2880_576I_50HZ, 1, 276, 2880, 1, 19, 20, 288},
	{WCH_2880_288P_50HZ, 1, 276, 2880, 1, 19, 19, 288},
	{WCH_1440_576P_50HZ, 1, 136, 1440, 1, 39, 39, 576},
	{WCH_1920_1080P_50HZ, 0, 148, 1920, 0, 36, 36, 1080},
	{WCH_1920_1080P_24HZ, 0, 148, 1920, 0, 36, 36, 1080},
	{WCH_1920_1080P_25HZ, 0, 148, 1920, 0, 36, 36, 1080},
	{WCH_1920_1080P_30HZ, 0, 148, 1920, 0, 36, 36, 1080},
	{WCH_2880_480P_60HZ, 1, 240, 2880, 1, 30, 30, 480},
	{WCH_2880_576P_50HZ, 1, 272, 2880, 1, 39, 39, 576},
	{WCH_1920_1080I_50HZ_1250T, 0, 184, 1920, 1, 57, 58, 540},
	{WCH_1920_1080I_100HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_1280_720P_100HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_720_576P_100HZ, 1, 68, 720, 1, 39, 39, 576},
	{WCH_720_576I_100HZ, 1, 138, 1440, 1, 19, 20, 288},
	{WCH_1920_1080I_120HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_1280_720P_120HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_720_480P_120HZ, 1, 60, 720, 1, 30, 30, 480},
	{WCH_720_480I_120HZ, 1, 114, 1440, 1, 15, 16, 240},
	{WCH_720_576P_200HZ, 1, 68, 720, 1, 39, 39, 576},
	{WCH_720_576I_200HZ, 1, 138, 1440, 1, 19, 20, 288},
	{WCH_720_480P_240HZ, 1, 60, 720, 1, 30, 30, 480},
	{WCH_720_480I_240HZ, 1, 114, 1440, 1, 15, 16, 240},
	{WCH_1440_576I_50HZ, 1, 136, 1440, 1, 39, 39, 576},
	{WCH_MODE_NUM, 0, 0, 0, 0, 0, 0, 0},
};

#if WCH_DUMP_BUFFER_ATTR
u32 u4DumpFrameCnt[2] = {0, 0};

u32 u4DumpBufIdxCnt[2] = {0, 0};
#endif

phys_addr_t wch_tvd_base_pa = 0;
phys_addr_t wch_ypbpr_vga_base_pa = 0;
phys_addr_t wch_hdmi_base_pa = 0;
phys_addr_t wch_dgi_base_pa = 0;
phys_addr_t wch_disp_base_pa = 0;

void WchSetSourceBaseAddr(phys_addr_t wchReservebase)
{
	wch_tvd_base_pa = wchReservebase;
	wch_ypbpr_vga_base_pa = wch_tvd_base_pa + WCH_MEM_TVD_SIZE;
	wch_hdmi_base_pa = wch_ypbpr_vga_base_pa + WCH_MEM_YPBPR_VGA_SIZE;
	wch_dgi_base_pa = wch_hdmi_base_pa + WCH_MEM_HDMI_SIZE;
	wch_disp_base_pa = wch_dgi_base_pa + WCH_MEM_DGI_SIZE;
}

#ifndef __ARM2__/* kernel build branch */

#if WCH_DUMP_BUFF
static bool fgWriteThread[2] = { false, false };
static u32 u4EnDumpCnt[2] = { 0, 0 };
static u32 u4DumpCnt[2] = { 0, 0 };

static bool WchWriteBuff(u8 u1WchId, u32 u4YAddr, u32 u4CAddr)
{
	s8 szDumpFile[256];
	struct file *fp = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	int ret = 0, ysize = WCH_SD_YBUF_SIZE, csize = WCH_SD_CBUF_SIZE;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sprintf(szDumpFile, "/ext_sdcard2/mmcblk1p1/y%02d.raw", u4DumpCnt[u1WchId]);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchWriteBuff] Dump File:%s for wch%d\r\n", szDumpFile,
		 (int)u1WchId);
	fp = filp_open(szDumpFile, O_RDWR | O_CREAT, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchWriteBuff] open file %s error wch%d return %d\n",
			 szDumpFile, (int)u1WchId, ret);
		return false;
	}
	pos = 0;
	ret = vfs_write(fp, MEMRSV_PHY_TO_VIRT(u4YAddr), ysize, &pos);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchWriteBuff] write file addr 0x%x, 0x%x size %d, %d\r\n",
		 (unsigned int)u4YAddr, (unsigned int)MEMRSV_PHY_TO_VIRT(u4YAddr), (int)ysize,
		 (int)pos);
	sprintf(szDumpFile, "/ext_sdcard2/mmcblk1p1/c%02d.raw", u4DumpCnt[u1WchId]);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchWriteBuff] Dump File:%s for wch %d\r\n", szDumpFile,
		 (int)u1WchId);
	fp = filp_open(szDumpFile, O_RDWR | O_CREAT, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchWriteBuff] open file %s error wch%d return %d\n",
			 szDumpFile, (int)u1WchId, ret);
		return false;
	}
	pos = 0;
	ret = vfs_write(fp, MEMRSV_PHY_TO_VIRT(u4CAddr), csize, &pos);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchWriteBuff] write file addr 0x%x, 0x%x size %d, %d\r\n",
		 (unsigned int)u4CAddr, (unsigned int)MEMRSV_PHY_TO_VIRT(u4CAddr), csize, pos);
	u4DumpCnt[u1WchId]++;
	filp_close(fp, NULL);
	set_fs(old_fs);
	return true;
}

static int WchWriteThread(u8 u1WchId)
{
	u32 u4MaxBufCnt = _gWchParam[u1WchId].tWchBuf.u4BufCnt;
	u32 u4BuffIdx = 0;

	pr_debug("WchWriteThread start \r\n");
	for (;;) {
		if (u4DumpCnt[u1WchId] > 10) {
			udelay(1000);
			continue;
		}
		if (u4EnDumpCnt[u1WchId]) {
			u4BuffIdx = u4DumpCnt[u1WchId] % _gWchParam[u1WchId].tWchBuf.u4BufCnt;
			WCH_LOG(WCH_LOG_LVL_INFO, "WchWriteThread wch id %d, buffer id %d\r\n",
				 (int)u1WchId, (int)u4BuffIdx);
			WchWriteBuff(u1WchId, _gWchParam[u1WchId].tWchBuf.u4YBuf[u4BuffIdx],
				      _gWchParam[u1WchId].tWchBuf.u4CBuf[u4BuffIdx]);
			u4EnDumpCnt[u1WchId]--;
		}
		udelay(1000);
	}
}
#endif

HANDLE X_CreateEvent(LPTSTR lpName)
{
	return x_event_create(NULL, false, false, lpName);
}

bool X_DestroyEvent(HANDLE hEvent)
{
	return x_event_destroy(hEvent);
}

bool X_SetEvent(HANDLE hEvent)
{
	return x_event_set(hEvent);
}

bool X_SetEventData(HANDLE hEvent, DWORD dwData)
{
	return x_event_set_data(hEvent, dwData);
}

unsigned int WchRequstIrq(u8 u1WchId, irq_handler_t isr)
{
	return request_irq((u1WchId == (u8)WCH_1) ? wch0irq : wch1irq, isr,
                0, (u1WchId == (u8)WCH_1) ? "ISR_83" : "ISR_87", NULL);
}

bool GetWchIndexFromIsrId(u16 u2Vector, u32 *u4Idx)
{
	if (u2Vector == (u16)wch0irq) {
		*u4Idx = WCH_1;
	}
	else if (u2Vector == (u16)wch1irq) {
		*u4Idx = (u32)WCH_2;
	}
	else
		return false;

	return true;
}

static int wch_0 = 0, wch_1 = 1;

void WchCreateKthread(int (*threadfn)(void *data), const char *namefmt1, const char *namefmt2)
{
	hWchInst[0] = kthread_create(threadfn, (void *)&wch_0, namefmt1);
	hWchInst[1] = kthread_create(threadfn, (void *)&wch_1, namefmt2);
	if (!IS_ERR(hWchInst[0]) && !IS_ERR(hWchInst[1])) {
		WCH_LOG(WCH_LOG_LVL_DBG, "WchCreateKthread create thread success = 0x%p, 0x%p", hWchInst[0], hWchInst[1]);
		wake_up_process(hWchInst[0]);
		wake_up_process(hWchInst[1]);
	}
}

int WchGetBuffIdxThread(void *data)
{
	DWORD dw = 0;
	u32 u4BufIdx = 0;
	int *cnt = data;

	WCH_LOG(WCH_LOG_LVL_DBG, "[yzq] WchGetBuffIdxThread!\r\n");
	for (;;) {
		dw = x_event_wait_for_objects(1, &hWchThreadEvent[*cnt], false , 0xFFFFFFFFU);

		if (WAIT_OBJECT_0 == dw) {
			if (*cnt == 0)
				spin_lock_irqsave(&protect_gWchParam0, fg_gWchParam[0]);
			else
				spin_lock_irqsave(&protect_gWchParam1, fg_gWchParam[1]);
			if (_gWchParam[*cnt].tWchCfg.GetWchBufIndx) {
				u4BufIdx = (WchBuffIdx[*cnt] + _gWchParam[*cnt].tWchBuf.u4BufCnt - WCH_WRITED_BUF_IDX_OFT)
					% _gWchParam[*cnt].tWchBuf.u4BufCnt;

				_gWchParam[*cnt].tWchCfg.GetWchBufIndx(&u4BufIdx);
			}
			if (*cnt == 0)
				spin_unlock_irqrestore(&protect_gWchParam0, fg_gWchParam[0]);
			else
				spin_unlock_irqrestore(&protect_gWchParam1, fg_gWchParam[1]);
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR, "[yzq] wait event error! \r\n");
		}
	}
	complete_and_exit(NULL, 0);

	return 0;
}

void WchEventThreadInit(void)
{
	hWchThreadEvent[0] = X_CreateEvent("wch0KthreadEvent");
	hWchThreadEvent[1] = X_CreateEvent("wch1KthreadEvent");
	if ((hWchThreadEvent[0] == NULL) && (hWchThreadEvent[1] == NULL)) {
		WCH_LOG((u32)WCH_LOG_LVL_ERR,
			"create hWchThreadEvent failed =0x%x, 0x%x\r\n",
			(unsigned int)hWchThreadEvent[0], (unsigned int)hWchThreadEvent[1]);
	} else {
		WCH_LOG((u32)WCH_LOG_LVL_DBG,
			"create hWchThreadEvent success =0x%x, 0x%x\r\n",
			(unsigned int)hWchThreadEvent[0], (unsigned int)hWchThreadEvent[1]);
	}

	WchCreateKthread(WchGetBuffIdxThread, "GetBufferKthread1", "GetBufferKthread2");
}
void WchCloseThreadEvent(void)
{
	u32 cnt = 0;
    
	for (;cnt < 2;cnt++) {
		X_DestroyEvent(hWchThreadEvent[cnt]);
		hWchThreadEvent[cnt] = NULL;
	}
}

#else/* arm2 build branch */

unsigned int WchRequstIrq(u8 u1WchId, irqreturn_t (*isr)(int u2Vector, void *dev_id))
{
#if 0
    x_os_isr_fct pfnOldIsr;

    return x_reg_isr((u1WchId == WCH_1) ? VECTOR_WCHNL : VECTOR_WCHNL2, isr, &pfnOldIsr);
#else 
    return 0;
#endif
}

bool GetWchIndexFromIsrId(u16 u2Vector, u32 *u4Idx)
{
	if (u2Vector == (u16)VECTOR_WCHNL) {
		*u4Idx = WCH_1;
	}
	else if (u2Vector == (u16)VECTOR_WCHNL2) {
		*u4Idx = (u32)WCH_2;
	}
	else
		return false;

	return true;
}

#endif
void SpinLockWch(void)
{
	spin_lock_irqsave(&lock, flags);
	u4LockCnt++;
	WCH_LOG((u32)WCH_LOG_LVL_TRACE, "[SpinLockWch] u4LockCnt=%d\r\n", (int)u4LockCnt);
}
void SpinUnlockWch(void)
{
	u4LockCnt--;
	spin_unlock_irqrestore(&lock, flags);
	WCH_LOG((u32)WCH_LOG_LVL_TRACE, "[SpinUnlockWch] u4LockCnt=%d\r\n", (int)u4LockCnt);
}

#if NEW_WCH_EVENT_NAME
bool WchCreateEvent(u8 u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	TCHAR *tzEventName = "WCH_UNKNOW_EVENT";
	int ret = 1;

	switch (eWchSrcId) {
	case SRC_APP_BACKCAR:
		tzEventName = WCH_BACKCAR_EVENT;
		break;
	case SRC_APP_AVIN:
		tzEventName = WCH_AVIN_EVENT;
		break;
	case SRC_APP_YPBPR:
		tzEventName = WCH_YPBPR_EVENT;
		break;
	case SRC_APP_VGA:
		tzEventName = WCH_VGA_EVENT;
		break;
	case SRC_APP_HDMI:
		tzEventName = WCH_HDMI_EVENT;
		break;
#if WCH_SPECIAL_DGI_ENABLE
	case SRC_APP_SPECIAL_DGI:
#endif
	case SRC_APP_DGI:
		tzEventName = WCH_DGI_EVENT;
		break;
	case SRC_APP_DISPLAY:
		tzEventName = WCH_DVD_EVENT;
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCreateEvent wrong event name = %d", *tzEventName);
		break;
	}

	/* Create write channel vsync event */
	_hWchEvent[u1WchId] = X_CreateEvent(tzEventName);
	if (_hWchEvent[u1WchId] == NULL) {
		WCH_LOG((u32)WCH_LOG_LVL_ERR, "WchCreateEvent fail id = %d, event = %x\r\n",
			 (int)u1WchId, (unsigned int)_hWchEvent[u1WchId]);
		ret = 0;
	} else{
		WCH_LOG((u32)WCH_LOG_LVL_INFO, "WchCreateEvent sucess id = %d, event = %x\r\n",
			 (int)u1WchId, (unsigned int)_hWchEvent[u1WchId]);
	}
	return (ret == 1 ? true:false);
}

bool WchCloseEvent(u8 u1WchId)
{
	WCH_LOG((u32)WCH_LOG_LVL_INFO, "WchCloseEvent id = %d, event = %x\r\n", (int)u1WchId,
		 (unsigned int)_hWchEvent[u1WchId]);
	X_DestroyEvent(_hWchEvent[u1WchId]);
	_hWchEvent[u1WchId] = NULL;
	return true;
}
#else
bool WchCreateEvent(void)
{
	int ret = 1;
	/* Create write channel vsync event */
	_hWchEvent[WCH_1] = X_CreateEvent(WCH1_FRAME_DONE_EVENT);
	_hWchEvent[WCH_2] = X_CreateEvent(WCH2_FRAME_DONE_EVENT);
	if ((_hWchEvent[WCH_1] == NULL) || (_hWchEvent[WCH_2] == NULL)) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCreateEvent fail event1 = %x, event2 = %x\r\n",
			 (unsigned int)_hWchEvent[WCH_1], (unsigned int)_hWchEvent[WCH_2]);
		ret = 0;
	} else{
		WCH_LOG(WCH_LOG_LVL_INFO, "WchCreateEvent sucess event1 = %x, event2 = %x\r\n",
			 (unsigned int)_hWchEvent[WCH_1], (unsigned int)_hWchEvent[WCH_2]);
	}
	return (ret == 1 ? true:false);
}

bool WchCloseEvent(void)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "WchCloseEvent event1 = %x, event2 = %x\r\n",
		 (unsigned int)_hWchEvent[WCH_1], (unsigned int)_hWchEvent[WCH_2]);
	X_DestroyEvent(_hWchEvent[WCH_1]);
	X_DestroyEvent(_hWchEvent[WCH_2]);
	_hWchEvent[WCH_1] = NULL;
	_hWchEvent[WCH_2] = NULL;
	return true;
}


#endif
bool WchSetEventData(u8 u1WchId)
{
	u32 u4BufIdx = _gWchParam[u1WchId].u4BufIdx;
	u32 u4BufCnt = _gWchParam[u1WchId].tWchBuf.u4BufCnt;

#if MONITOR_UPDATEBUF_TIME
	struct timeval end_t = { 0, 0 };
	long usec = 0;

#endif
#ifdef __ARM2__
	if (!_hWchEvent[u1WchId]) {
		WCH_LOG((u32)WCH_LOG_LVL_ERR,
			 "WchSetEventData event is null id = %x, buf id = %d, total buf cnt = %d\r\n",
			 (int)u1WchId, (int)u4BufIdx, (int)u4BufCnt);
		return false;
	}
	u4BufIdx = (u4BufIdx + u4BufCnt - WCH_WRITED_BUF_IDX_OFT) % u4BufCnt;
	X_SetEventData(_hWchEvent[u1WchId], u4BufIdx);
	X_SetEvent(_hWchEvent[u1WchId]);
	WCH_LOG((u32)WCH_LOG_LVL_IRQ, "WchSetEventData id = %x, buf id = %d, total buf cnt = %d\r\n",
		 (int)u1WchId, (int)u4BufIdx, (int)u4BufCnt);

#else
	WchBuffIdx[u1WchId] = _gWchParam[u1WchId].u4BufIdx;
	X_SetEvent(hWchThreadEvent[u1WchId]);
#endif

#if MONITOR_UPDATEBUF_TIME
	do_gettimeofday(&end_t);
	usec =
	    (end_t.tv_sec - start_t[u1WchId].tv_sec) * 1000000 + (end_t.tv_usec -
								  start_t[u1WchId].tv_usec);
	if (usec > 33333) {
		WCH_LOG(WCH_LOG_LVL_DBG, "WchSetEventData id = %x usec = %d ************\r\n",
			 (int)u1WchId, (int)usec);
	}
	do_gettimeofday(&start_t[u1WchId]);

#endif
	return true;
}

bool WchUpdateBuff(u8 u1WchId)
{
	PWCH_BUFF_INFO_T pWchBuff = &_gWchParam[u1WchId].tWchBuf;
	PWCH_IF_PARAM_T pWchParam = &_gWchParam[u1WchId];
	u32 u4YBuff = pWchBuff->u4YBuf[pWchParam->u4BufIdx];
	u32 u4CBuff = pWchBuff->u4CBuf[pWchParam->u4BufIdx];

	if (!u4YBuff || !u4CBuff || (u4YBuff & (u32)WCH_BUFFER_MASK) || (u4CBuff & (u32)WCH_BUFFER_MASK)) {
		WCH_LOG((u32)WCH_LOG_LVL_ERR, "WchUpdateBuff buffer error id=%d, Y=0x%x, C=0x%x\r\n",
			 (int)u1WchId, (unsigned int)u4YBuff, (unsigned int)u4CBuff);
		return false;
	}
	WchHalSetYCAddr(u1WchId, u4YBuff, u4CBuff);
	WchHalSetRegTouch(u1WchId);
	pWchParam->u4BufIdx++;
	WCH_LOG((u32)WCH_LOG_LVL_IRQ, "[yzq] pWchParam->u4BufIdx=%d\r\n", pWchParam->u4BufIdx);
	if (pWchParam->u4BufIdx == pWchBuff->u4BufCnt)
		pWchParam->u4BufIdx = 0;
	return true;
}

bool WchClearBuff(u32 u4YAddr, u32 u4CAddr)
{
	return true;
}

irqreturn_t WchIsr(int u2Vector, void *dev_id)
{
	u32 u4Idx = 0;

	SpinLockWch();
	WCH_LOG((u32)WCH_LOG_LVL_IRQ, "WchIsr interrupt = %d, status = %d\r\n", (int)u2Vector,
		 (int)_gWchParam[u4Idx].u4Status);
	if (!GetWchIndexFromIsrId(u2Vector, &u4Idx)) {
		WCH_LOG((u32)WCH_LOG_LVL_ERR, "WchIsr fail interrupt = %d, satus = %d\r\n",
			 (int)u2Vector, (int)_gWchParam[u4Idx].u4Status);
		SpinUnlockWch();
		return -1;
	}

#if !WCH_HDMI_TIMING_EN
	if ((uStopParam & 0x1) == 0) {
		SpinUnlockWch();
		u32 idx = ((uStopParam & 0x000f0000) >> 16);

		pr_debug("[WCH] HDMI Buffdonw start jiffies : %d, idx : %d\r\n", (int)jiffies,
			  (int)idx);
		uStopParam = 0x1U;
		WchHalDeinit(u4Idx);
		pr_debug("[WCH] HDMI Buffdonw end jiffies : %d\r\n", (int)jiffies);
		wake_up_interruptible(&wchqueue);
		SpinLockWch();
	}

#endif
	if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
		if (u4SkipVysncCnt[u4Idx] > 0) {
			WCH_LOG(WCH_LOG_LVL_IRQ, "WchIsr interrupt = %d, need skip = %d\r\n",
				 (int)u2Vector, (int)u4SkipVysncCnt[u4Idx]);
			u4SkipVysncCnt[u4Idx]--;
		} else {
#if WCH_DUMP_BUFF
			u4EnDumpCnt[u4Idx]++;
#endif
#if WCH_DUMP_BUFFER_ATTR
			u4DumpFrameCnt[u4Idx]++;
#endif
#if WCH_DUMP_FRAMES_BY_ICE
			WCH_LOG(WCH_LOG_LVL_INFO, "WchIsr u4DumpFrameCnt[u4Idx]: %d\r\n",
				(int)u4DumpFrameCnt[u4Idx]);
			if (u4DumpFrameCnt[u4Idx] >= WCH_SD_BUF_CNT) {
				WchHalDeinit(u4Idx);
			}
#endif
			WchUpdateBuff(u4Idx);
			WchSetEventData(u4Idx);
		}
	}
	SpinUnlockWch();
	ac83xx_mask_ack_bim_irq(u2Vector);

	return IRQ_HANDLED;
}

void WchIsrInit(u8 u1WchId)
{

	unsigned long flags;

	spin_lock_irqsave(&wch_isr_lock, flags);

	if (!fgWchIsrInit[u1WchId]) {
		if (WchRequstIrq(u1WchId, WchIsr) == OSR_OK) {
			WCH_LOG((u32)WCH_LOG_LVL_DBG, "vWchIsrInit success idx = %d\r\n", (int)u1WchId);
			fgWchIsrInit[u1WchId] = true;
		} else
			WCH_LOG((u32)WCH_LOG_LVL_ERR, "vWchIsrInit fail idx = %d\r\n", (int)u1WchId);
	}

	spin_unlock_irqrestore(&wch_isr_lock, flags);
}
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__

static struct _wchBuffer bufInfo[2]={{(dma_addr_t)NULL, NULL, 0}, {(dma_addr_t)NULL, NULL, 0}};
void *WchAllocBufferAddr(WCH_SRC_APP_ID_E eWchSrcId, u8 u1WchId, unsigned int size)
{
	if (SRC_APP_BACKCAR != eWchSrcId && SRC_APP_AVIN != eWchSrcId) {
		void *viraddr = NULL;

        if (bufInfo[u1WchId].phyaddr != (dma_addr_t)NULL && 
            bufInfo[u1WchId].viraddr != NULL && 
            bufInfo[u1WchId].size != 0) {
            WCH_LOG(WCH_LOG_LVL_ERR, "free old memory \n");
            dma_free_writecombine(NULL, bufInfo[u1WchId].size,
                bufInfo[u1WchId].viraddr, bufInfo[u1WchId].phyaddr);
            bufInfo[u1WchId].size = 0;
            bufInfo[u1WchId].viraddr = NULL;
            bufInfo[u1WchId].phyaddr = (dma_addr_t)NULL;
        }
        
		size = PAGE_ALIGN(size);
		bufInfo[u1WchId].size = size;
        WCH_LOG(WCH_LOG_LVL_ERR, "*** alloc size: %d \n", size);
		viraddr = dma_alloc_writecombine(NULL, size, &bufInfo[u1WchId].phyaddr, GFP_KERNEL);
		if(!viraddr) {
        		WCH_LOG(WCH_LOG_LVL_ERR, "cma alloc memory fail! \n");
        		bufInfo[u1WchId].size = 0;
        		return NULL;
		}
		bufInfo[u1WchId].viraddr = viraddr;

		return (void *)bufInfo[u1WchId].phyaddr;
	}

	return NULL;
}

void WchFreeBufferAddr(u8 u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	if (eWchSrcId != SRC_APP_BACKCAR && eWchSrcId != SRC_APP_AVIN) {
		dma_free_writecombine(NULL, bufInfo[u1WchId].size,
			bufInfo[u1WchId].viraddr, bufInfo[u1WchId].phyaddr);

		bufInfo[u1WchId].size = 0;
		bufInfo[u1WchId].viraddr = NULL;
		bufInfo[u1WchId].phyaddr = (dma_addr_t)NULL;
	}
}

WCH_BUFF_INFO_T _virtualaddr[3];
void WchAllocVirtualAddr(u8 u1WchId, void* va, u32 ysize, u32 cnt)
{
	int i = 1;

	_virtualaddr[u1WchId].u4BufCnt = cnt;
	_virtualaddr[u1WchId].u4YBuf[0] = (u32)va;
	_virtualaddr[u1WchId].u4CBuf[0] = _virtualaddr[u1WchId].u4YBuf[0] + ysize;
	for (; i < cnt; i++) {
		_virtualaddr[u1WchId].u4YBuf[i] = _virtualaddr[u1WchId].u4CBuf[i-1] + ysize/2;
		_virtualaddr[u1WchId].u4CBuf[i] = _virtualaddr[u1WchId].u4YBuf[i] + ysize;
	}
}
bool WchGetBufVirtualAddr(WCH_SRC_APP_ID_E eWchSrcId, PWCH_BUFF_INFO_T prOutBuffer)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId)
			continue;
		else {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchGetBufVirtualAddr id=%d, eWchSrcId=%d, u4Status=%d\r\n", (int)u4Idx,
				 (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status);
#if 0
			copy_to_user((void __user *)prOutBuffer, &_virtualaddr[u4Idx].tWchBuf, sizeof(WCH_BUFF_INFO_T));/*double check*/
#else
			memcpy(prOutBuffer, &_virtualaddr[u4Idx], sizeof(WCH_BUFF_INFO_T));
#endif
			fgRet = true;
		}
	}
	SpinUnlockWch();
	return fgRet;
}

#endif
#endif

#if defined(CONFIG_ATC_OS_android)
bool WchAllocBuf(u8 u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	PWCH_BUFF_INFO_T pWchBuff = NULL;
	PWCH_IF_PARAM_T pWchParam = NULL;
	int i = 0;

	pWchBuff = &_gWchParam[u1WchId].tWchBuf;
	pWchParam = &_gWchParam[u1WchId];
	switch (eWchSrcId) {
	case SRC_APP_BACKCAR:
	case SRC_APP_AVIN:
	{
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_tvd_base_pa + ((u32)WCH_SD_YBUF_SIZE + (u32)WCH_SD_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_SD_YBUF_SIZE;
			#ifdef __ARM2__
			memset(ARM1PHY2ARM2UCV(pWchBuff->u4YBuf[i]), 0, (u32)WCH_SD_YBUF_SIZE);
			CacheFlush(ARM1PHY2ARM2UCV(pWchBuff->u4YBuf[i]), (u32)WCH_SD_YBUF_SIZE);
			memset(ARM1PHY2ARM2UCV(pWchBuff->u4CBuf[i]), 0x80, (u32)WCH_SD_CBUF_SIZE);
			CacheFlush(ARM1PHY2ARM2UCV(pWchBuff->u4CBuf[i]), (u32)WCH_SD_CBUF_SIZE);
			#endif
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;
	}
	case SRC_APP_YPBPR:
	case SRC_APP_VGA:
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_ypbpr_vga_base_pa + ((u32)WCH_VGA_YBUF_SIZE + (u32)WCH_VGA_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_VGA_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;
	case SRC_APP_HDMI:
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_hdmi_base_pa + ((u32)WCH_HDMI_YBUF_SIZE + (u32)WCH_HDMI_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_HDMI_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;

#if WCH_SPECIAL_DGI_ENABLE
	case SRC_APP_SPECIAL_DGI:
#endif
	case SRC_APP_DGI:
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_dgi_base_pa + ((u32)WCH_SD_YBUF_SIZE + (u32)WCH_SD_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_SD_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;
	case SRC_APP_DISPLAY:
		for (i=0; i<WCH_HD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_disp_base_pa + ((u32)WCH_DISP_YBUF_SIZE + (u32)WCH_DISP_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_DISP_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = WCH_HD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR, "WchAllocBuf unknown src type id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)eWchSrcId);
		return false;
	}
	return true;
}
#elif defined(CONFIG_ATC_OS_linux)
bool WchAllocBuf(u8 u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	PWCH_BUFF_INFO_T pWchBuff = NULL;
	PWCH_IF_PARAM_T pWchParam = NULL;
	unsigned int bufsize = 0;
	WCH_LOG(WCH_LOG_LVL_DBG, "dynamic allocate wch memory version!!!\r\n");
	int i = 0;

	pWchBuff = &_gWchParam[u1WchId].tWchBuf;
	pWchParam = &_gWchParam[u1WchId];
	switch (eWchSrcId) {
	case SRC_APP_BACKCAR:
	case SRC_APP_AVIN:
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)wch_tvd_base_pa + ((u32)WCH_SD_YBUF_SIZE + (u32)WCH_SD_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_SD_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		break;
	case SRC_APP_YPBPR:
	case SRC_APP_VGA:
#ifdef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO,"not to allocate memory.\r\n");
		return false;
#else
		bufsize = (WCH_VGA_YBUF_SIZE + WCH_VGA_CBUF_SIZE) * WCH_SD_BUF_CNT;
		WchAllocBufferAddr(eWchSrcId, u1WchId, bufsize);
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)bufInfo[u1WchId].phyaddr + ((u32)WCH_VGA_YBUF_SIZE + (u32)WCH_VGA_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_VGA_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		WchAllocVirtualAddr(u1WchId, bufInfo[u1WchId].viraddr, WCH_VGA_YBUF_SIZE, WCH_SD_BUF_CNT);
#endif
		break;
	case SRC_APP_HDMI:
#ifdef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO,"not to allocate memory.\r\n");
		return false;
#else
		bufsize = (WCH_HDMI_YBUF_SIZE + WCH_HDMI_CBUF_SIZE) * WCH_SD_BUF_CNT;
		WchAllocBufferAddr(eWchSrcId, u1WchId, bufsize);
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)bufInfo[u1WchId].phyaddr + ((u32)WCH_HDMI_YBUF_SIZE + (u32)WCH_HDMI_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_HDMI_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		WchAllocVirtualAddr(u1WchId, bufInfo[u1WchId].viraddr, WCH_HDMI_YBUF_SIZE, WCH_SD_BUF_CNT);
#endif
		break;

#if WCH_SPECIAL_DGI_ENABLE
	case SRC_APP_SPECIAL_DGI:
#endif
	case SRC_APP_DGI:
#ifdef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO,"not to allocate memory.\r\n");
		return false;
#else
		bufsize = (WCH_SD_YBUF_SIZE + WCH_SD_CBUF_SIZE) * WCH_SD_BUF_CNT;
		WchAllocBufferAddr(eWchSrcId, u1WchId, bufsize);
		for (i=0; i<WCH_SD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)bufInfo[u1WchId].phyaddr + ((u32)WCH_SD_YBUF_SIZE + (u32)WCH_SD_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_SD_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = (u32)WCH_SD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		WchAllocVirtualAddr(u1WchId, bufInfo[u1WchId].viraddr, WCH_SD_YBUF_SIZE, WCH_SD_BUF_CNT);
#endif
		break;
	case SRC_APP_DISPLAY:
#ifdef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO,"not to allocate memory.\r\n");
		return false;
#else
		bufsize = (WCH_DISP_YBUF_SIZE + WCH_DISP_CBUF_SIZE) * WCH_HD_BUF_CNT;
		WchAllocBufferAddr(eWchSrcId, u1WchId, bufsize);
		for (i=0; i<WCH_HD_BUF_CNT; i++) {
			pWchBuff->u4YBuf[i] = (u32)bufInfo[u1WchId].phyaddr + ((u32)WCH_DISP_YBUF_SIZE + (u32)WCH_DISP_CBUF_SIZE) * i;
			pWchBuff->u4CBuf[i] = pWchBuff->u4YBuf[i] + (u32)WCH_DISP_YBUF_SIZE;
		}
		pWchBuff->u4BufCnt = WCH_HD_BUF_CNT;
		pWchParam->u4BufIdx = 0;
		WchAllocVirtualAddr(u1WchId, bufInfo[u1WchId].viraddr, WCH_DISP_YBUF_SIZE, WCH_HD_BUF_CNT);
#endif
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR, "WchAllocBuf unknown src type id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)eWchSrcId);
		return false;
	}

	return true;
}

#endif
bool WchGetRetimingInfo(WCH_TIMING_E eSrcTiming)
{
	u32 u4Idx = 0;
	/*
	if (!eSrcTiming) {
		eSrcTiming = WCH_1280_720P_60HZ;
		WCH_LOG((u32)WCH_LOG_LVL_DBG, "WchGetRetimingInfo modify source timing to %d\r\n",
			 (int)eSrcTiming);
	} */
	for (u4Idx = 0; u4Idx < sizeof(WchTimingTable) / sizeof(WCH_TIMING_PARAM_T); u4Idx++) {
		if (WchTimingTable[u4Idx].eTiming == eSrcTiming) {
			prWchRetiming = &WchTimingTable[u4Idx];
			break;
		}
	}
	if ((u4Idx == (u32)WCH_MODE_NUM) || (prWchRetiming == NULL)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchGetRetimingInfo error table idx=%d, prWchRetiming=%x\r\n", (int)u4Idx,
			 (int)prWchRetiming);
		return false;
	}
	WCH_LOG((u32)WCH_LOG_LVL_DBG, "WchGetRetimingInfo sucess %d, %d, %d, %d, %d, %d, %d, %d\r\n",
		 (int)prWchRetiming->eTiming, (int)prWchRetiming->u2HsyncInv,
		 (int)prWchRetiming->u2HPixel, (int)prWchRetiming->u2HActive,
		 (int)prWchRetiming->u2VsyncInv, (int)prWchRetiming->u2VTopLine,
		 (int)prWchRetiming->u2VBotLine, (int)prWchRetiming->u2VActive);
	return true;
}

bool WchConfigHw(u8 u1WchId, PWCH_CFG_T pWchCfg)
{
	if (!IsAlign(pWchCfg->u4SrcWidth, WCH_WIDTH_ALIGN_SHIFT)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchConfigHw src width is not align id=%d, width=%d, height=%d\r\n",
			 (int)u1WchId, (int)pWchCfg->u4SrcWidth, (int)pWchCfg->u4SrcHeight);
		return false;
	}
	if (u4LockCnt > 0) {
		SpinUnlockWch();
		WchHalInit(u1WchId);
		SpinLockWch();
	} else {
		WchHalInit(u1WchId);
	}
	WchHalSetPinmux(pWchCfg->eInputSrc);
	WchHalSetSrcType(u1WchId, pWchCfg->eInputSrc);
	WchHalSetOutput(u1WchId, pWchCfg->eOutputFmt, pWchCfg->fgProgressive,
			   pWchCfg->u4DstWidth, pWchCfg->u4DstHeight);

#if WCH_HDMI_TIMING_EN
	if (pWchCfg->eInputSrc == DATA_SRC_HDMI) {
		if (WchGetRetimingInfo(pWchCfg->eTiming)) {
			WchHalSetInput(u1WchId, pWchCfg->eInputFmt, prWchRetiming->u2HActive,
					prWchRetiming->u2VActive, prWchRetiming->u2HPixel,
					prWchRetiming->u2VTopLine, prWchRetiming->u2VBotLine,
					pWchCfg->fgProgressive);
			WchHalSetPolarity(u1WchId, !prWchRetiming->u2VsyncInv,
					   !prWchRetiming->u2HsyncInv, pWchCfg->fgBotFieldFirst);
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchConfigHw use write channel re-timing for HDMI id=%d\r\n",
				 (int)u1WchId);
		} else{
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchConfigHw use write channel re-timing for HDMI error id=%d\r\n",
				 (int)u1WchId);
			return false;
		}
	} else
#endif
	{
		WchHalSetInput(u1WchId, pWchCfg->eInputFmt, pWchCfg->u4SrcWidth,
				pWchCfg->u4SrcHeight, pWchCfg->u4SrcStartX,
				pWchCfg->u4SrcStartYTop, pWchCfg->u4SrcStartYBot,
				pWchCfg->fgProgressive);
		WchHalSetPolarity(u1WchId, pWchCfg->fgVSyncPolarity, pWchCfg->fgHSyncPolarity,
				     pWchCfg->fgBotFieldFirst);
	}
	WchHalSetCtrlSignal(u1WchId, pWchCfg->fgProgressive);
	WchHalSetUvYcSwap(u1WchId, WCH_UV_SWAP_MASK, pWchCfg->u1UVSwap, false);
	if (pWchCfg->u1YSel || pWchCfg->u1USel || pWchCfg->u1VSel) {
		WchHalSetYUVDelay(u1WchId, WCH_YUV_SEL_MASK, pWchCfg->u1YSel, pWchCfg->u1USel,
				   pWchCfg->u1VSel);
	}
	WchHalSetMirror(u1WchId, pWchCfg->u4Mirror);
	WchHalSetRegTouch(u1WchId);

	return true;
}

bool WchRecoverSrc(u8 u1WchId, PWCH_IF_PARAM_T pWchParam)
{
	bool fgRet = true;

	switch (pWchParam->u4Status) {
	case WCH_HW_FREE:
		WCH_LOG(WCH_LOG_LVL_DBG, "WchRecoverSrc WCH_HW_FREE id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)pWchParam->eWchSrcId);
		memset(pWchParam, 0, sizeof(WCH_IF_PARAM_T));
#if NEW_WCH_EVENT_NAME
		WchCloseEvent(u1WchId);

#endif
		break;
	case WCH_HW_CONFIG:
		WCH_LOG(WCH_LOG_LVL_DBG, "WchRecoverSrc WCH_HW_CONFIG id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)pWchParam->eWchSrcId);
		if (!WchConfigHw(u1WchId, &pWchParam->tWchCfg)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchRecoverSrc fail src type id=%d, SrcType =%d\r\n", (int)u1WchId,
				 (int)pWchParam->eWchSrcId);
			fgRet = false;
		} else {
			if (!WchUpdateBuff(u1WchId)) {
				WCH_LOG(WCH_LOG_LVL_ERR,
					 "WchRecoverSrc fail src type id=%d, SrcType =%d\r\n",
					 (int)u1WchId, (int)pWchParam->eWchSrcId);
				fgRet = false;
			}
		}
		break;
	case WCH_HW_START:
		WCH_LOG(WCH_LOG_LVL_DBG, "WchRecoverSrc WCH_HW_START id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)pWchParam->eWchSrcId);
		if (!WchConfigHw(u1WchId, &pWchParam->tWchCfg)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchRecoverSrc fail src type id=%d, SrcType =%d\r\n", (int)u1WchId,
				 (int)pWchParam->eWchSrcId);
			fgRet = false;
		} else{
			if (!WchUpdateBuff(u1WchId)) {
				WCH_LOG(WCH_LOG_LVL_ERR,
					 "WchRecoverSrc fail src type id=%d, SrcType =%d\r\n",
					 (int)u1WchId, (int)pWchParam->eWchSrcId);
				fgRet = false;
			}
			WchHalStart(u1WchId);
			WchHalSetRegTouch(u1WchId);
			u4SkipVysncCnt[u1WchId] = WCH_SKIP_BUFF_CNT;
		}
		break;
	case WCH_HW_STOP:
		WCH_LOG(WCH_LOG_LVL_DBG, "WchRecoverSrc WCH_HW_STOP id=%d, SrcType =%d\r\n",
			 (int)u1WchId, (int)pWchParam->eWchSrcId);
		if (u4LockCnt > 0) {
			SpinUnlockWch();
			WchHalDeinit(u1WchId);
			SpinLockWch();
		} else {
			WchHalDeinit(u1WchId);
		}
		break;
	case WCH_HW_READY:
	default:
		break;
	}
	return fgRet;
}

u32 WchOpen(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = (u32)WCH_INVALID;

	if (!fgWchLockInit) {
		spin_lock_init(&lock);
		fgWchLockInit = true;
			WCH_LOG(WCH_LOG_LVL_DBG, "WchOpen init spin lock %p\r\n", &lock);
	}
	SpinLockWch();

#if WCH_SPECIAL_DGI_ENABLE
	if (eWchSrcId == SRC_APP_SPECIAL_DGI) {
		u4Idx = 1U;
		if (_gWchParam[u4Idx].u4Status != (u32)WCH_HW_FREE) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchOpen error status not free id=%d, eWchSrcId=%d, u4Status=%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);
		} else{
			WCH_LOG(WCH_LOG_LVL_DBG, "WchOpen id=%d, eWchSrcId=%d, u4Status=%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);

#if NEW_WCH_EVENT_NAME
			if (WchCreateEvent(u4Idx, eWchSrcId))
#endif
				{
					WchIsrInit(u4Idx);
					_gWchParam[u4Idx].u4Status = (u32)WCH_HW_READY;
					_gWchParam[u4Idx].eWchSrcId = eWchSrcId;
					u4Ret = u4Idx;
				}
		}
		SpinUnlockWch();
		return u4Ret;
	}

#endif
	for (u4Idx = 0; u4Idx < (u32)WCH_BACKUP; u4Idx++) {
		if (_gWchParam[u4Idx].u4Status != (u32)WCH_HW_FREE) {
			if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchOpen status id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				continue;
			} else{
				if (_gWchParam[u4Idx].u4Status == (u32)WCH_HW_READY) {
					WCH_LOG(WCH_LOG_LVL_WARN,
						"WchOpen open already return directly id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						(int)u4Idx, (int)eWchSrcId,
						(int)_gWchParam[u4Idx].u4Status);
					u4Ret = u4Idx;
				} else{
					WCH_LOG(WCH_LOG_LVL_ERR,
						"WchOpen error open already	id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						(int)u4Idx, (int)eWchSrcId,
						(int)_gWchParam[u4Idx].u4Status);
				}
				break;
			}
		} else {
			WCH_LOG(WCH_LOG_LVL_DBG, "WchOpen id=%d, eWchSrcId=%d, u4Status=%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);

#if NEW_WCH_EVENT_NAME
			if (WchCreateEvent(u4Idx, eWchSrcId))
#endif
				{

					/* WchHalInit(u4Idx); */
					WchIsrInit(u4Idx);
					_gWchParam[u4Idx].u4Status = WCH_HW_READY;
					_gWchParam[u4Idx].eWchSrcId = eWchSrcId;
					u4Ret = u4Idx;
				}
			break;
		}
	}

#if WCH_GRAB_FUN_EN
	/* have no free write channel HW for backcar source, and check to grab front write channel source */
	if ((eWchSrcId == SRC_APP_BACKCAR) && (u4Idx == WCH_BACKUP)) {
		for (u4Idx = 0; u4Idx < (u32)WCH_BACKUP; u4Idx++) {
			if (_gWchParam[u4Idx].tWchCfg.fgCanNotGrabbed == false) {/* can grab input source */
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchOpen backup source for backcar id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				if ((_gWchParam[u4Idx].u4Status == WCH_HW_START)
				     || (WchHalIsOn(u4Idx))) {
					/* Disable wch clock first */
					if (u4LockCnt > 0) {
						SpinUnlockWch();
						WchHalDeinit(u4Idx);
						SpinLockWch();
					} else {
						WchHalDeinit(u4Idx);
					}
				}
				/* backup front write channel source */
				memcpy(&_gWchParam[WCH_BACKUP], &_gWchParam[u4Idx],
					   sizeof(WCH_IF_PARAM_T));
				_hWchEvent[WCH_BACKUP] = _hWchEvent[u4Idx];
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
				memcpy(&_virtualaddr[WCH_BACKUP], &_virtualaddr[u4Idx],
					   sizeof(WCH_BUFF_INFO_T));
#endif
#endif
#if NEW_WCH_EVENT_NAME
				if (WchCreateEvent(u4Idx, eWchSrcId))
#endif
				{
					_gWchParam[u4Idx].u4Status = WCH_HW_READY;
					_gWchParam[u4Idx].eWchSrcId = eWchSrcId;
					fgGrabWchFront = true;
					WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchOpen new source id=%d, eWchSrcId=%d,u4Status=%d, fgGrabWchFront=%d\r\n",
					(int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					(int)_gWchParam[u4Idx].u4Status,
					(int)fgGrabWchFront);
					u4Ret = u4Idx;
				}
				break;
			}
		}
	}

#endif
	SpinUnlockWch();
	return u4Ret;
}

bool WchClose(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < (u32)WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchClose status id=%d, eWchSrcId = %d, u4Status =%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);
			continue;
		} else {
			if (_gWchParam[u4Idx].u4Status == WCH_HW_STOP) {
#if NEW_WCH_EVENT_NAME
				WchCloseEvent(u4Idx);
#endif
#if WCH_GRAB_FUN_EN
				if (u4Idx != WCH_BACKUP) {
					/* backcar source exit and recover backup write channel source */
					if (fgGrabWchFront
						&& (_gWchParam[u4Idx].eWchSrcId == SRC_APP_BACKCAR)) {
						WCH_LOG(WCH_LOG_LVL_DBG,
							 "WchClose backcar and recover backup source id=%d, eWchSrcId=%d, u4Status=%d\r\n",
							 (int)u4Idx,
							 (int)_gWchParam[WCH_BACKUP].eWchSrcId,
							 (int)_gWchParam[WCH_BACKUP].u4Status);
						memcpy(&_gWchParam[u4Idx], &_gWchParam[WCH_BACKUP],
							sizeof(WCH_IF_PARAM_T));
						_hWchEvent[u4Idx] = _hWchEvent[WCH_BACKUP];
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
						memcpy(&_virtualaddr[u4Idx], &_virtualaddr[WCH_BACKUP],
							sizeof(WCH_BUFF_INFO_T));
#endif
#endif
						fgRet = WchRecoverSrc(u4Idx, &_gWchParam[u4Idx]);
						{
							if (fgRet) {
								memset(&_gWchParam[WCH_BACKUP], 0,
									sizeof(WCH_IF_PARAM_T));
								_hWchEvent[WCH_BACKUP] = NULL;
								fgGrabWchFront = false;
							}
						}
					} else
#endif
					{
						WCH_LOG(WCH_LOG_LVL_DBG,
							 "WchClose id=%d, eWchSrcId=%d, u4Status=%d\r\n",
							 (int)u4Idx,
							 (int)_gWchParam[u4Idx].eWchSrcId,
							 (int)_gWchParam[u4Idx].u4Status);
#ifndef __ARM2__

						if (u4Idx == 0)
							spin_lock_irqsave(&protect_gWchParam0, fg_gWchParam[0]);
						else
							spin_lock_irqsave(&protect_gWchParam1, fg_gWchParam[1]);
#endif
						memset(&_gWchParam[u4Idx], 0,
							sizeof(WCH_IF_PARAM_T));
#if defined(CONFIG_ATC_OS_linux)
						SpinUnlockWch();
#ifndef __ARM2__
						WchFreeBufferAddr(u4Idx, eWchSrcId);
#endif
						SpinLockWch();
#endif
#ifndef __ARM2__
						if (u4Idx == 0)
							spin_unlock_irqrestore(&protect_gWchParam0, fg_gWchParam[0]);
						else
							spin_unlock_irqrestore(&protect_gWchParam1, fg_gWchParam[1]);
#endif
					}
#if WCH_GRAB_FUN_EN
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "WchClose backup source id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
					_gWchParam[u4Idx].u4Status = WCH_HW_FREE;
				}
#endif
				fgRet = true;
			} else{
				WCH_LOG(WCH_LOG_LVL_WARN,
					 "WchClose status error id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			break;
		}
	}
	SpinUnlockWch();
	return fgRet;
}

bool WchStart(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = 0;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < (u32)WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			continue;
		} else{
			if (_gWchParam[u4Idx].u4Status == WCH_HW_CONFIG) {
#if WCH_GRAB_FUN_EN
				if (u4Idx != WCH_BACKUP) {
#endif
					WCH_LOG(WCH_LOG_LVL_DBG,
						    "WchStart id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						    (int)_gWchParam[u4Idx].u4Status);
#if WCH_HDMI_TIMING_EN
					WchHalStart(u4Idx);
					WchHalSetRegTouch(u4Idx);
#else
					if (eWchSrcId == SRC_APP_HDMI) {
						SpinUnlockWch();
						uStartParam = (u4Idx << 16);
						pr_debug
						    ("[WCH_DRV] HDMI Start startparam : %08x\r\n",
						     (unsigned int)uStartParam);
						u4Ret =
						    wait_event_interruptible_timeout(wchqueue,
										     (uStartParam ==
										      0x1),
										     (100) / (1000 /
											      HZ));

					{
					if (u4Ret == 0) {
						WchHalStart(u4Idx);
						WchHalSetRegTouch(u4Idx);
						pr_debug
						    ("[WCH_DRV] No HDMI Vsync, start wch direct.....\r\n");
					}
					SpinLockWch();
					}

					} else{
						WchHalStart(u4Idx);
						WchHalSetRegTouch(u4Idx);
					}
#endif
#if WCH_GRAB_FUN_EN
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "WchStart backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
				}
#endif
				_gWchParam[u4Idx].u4Status = WCH_HW_START;
				u4SkipVysncCnt[u4Idx] = WCH_SKIP_BUFF_CNT;
#if MONITOR_UPDATEBUF_TIME
				do_gettimeofday(&start_t[u4Idx]);
				u4WchCnt = 0;
#endif
#if WCH_DUMP_BUFF
				u4EnDumpCnt[u4Idx] = 0;
				u4DumpCnt[u4Idx] = 0;
				if (!fgWriteThread[u4Idx]) {
					kernel_thread(WchWriteThread, u4Idx,
						       CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
						       SIGCHLD);
					fgWriteThread[u4Idx] = true;
				}

#endif
#if WCH_DUMP_BUFFER_ATTR
				u4DumpFrameCnt[u4Idx] = 0;
				u4DumpBufIdxCnt[u4Idx] = 0;

#endif
#if WCH_TEST_THREAD
				if (!fgStartTestThread) {
					kernel_thread(WCH_TEST, NULL,
						       CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
						       SIGCHLD);
					fgStartTestThread = true;
				}
#endif
				u4Ret = (u32)true;
			} else {
				WCH_LOG(WCH_LOG_LVL_ERR,
					 "WchStart status error id=%d, eWchSrcId = %d, u4Status =%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			break;
		}
	}
	SpinUnlockWch();
	return u4Ret ? true : false;
}

bool WchStop(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = 0;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < (u32)WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			continue;
		} else{
			if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
#if WCH_GRAB_FUN_EN
				if (u4Idx != WCH_BACKUP) {
#endif
					WCH_LOG(WCH_LOG_LVL_DBG,
						    "WchStop id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						    (int)_gWchParam[u4Idx].u4Status);

#if WCH_HDMI_TIMING_EN
					SpinUnlockWch();
					WchHalDeinit(u4Idx);
					SpinLockWch();
#else
					if (eWchSrcId == SRC_APP_HDMI) {
						SpinUnlockWch();
						uStopParam = (u4Idx << 16);
						pr_debug
						    ("[WCH_DRV] HDMI Stop uStopParam : %08x\r\n",
						     (unsigned int)uStopParam);
						u4Ret = wait_event_interruptible_timeout(wchqueue,
										     (uStopParam ==
										      0x1),
										     (100) / (1000 /
											      HZ));
						{
							if (u4Ret == 0) {
								uStopParam = 0x1U;
								pr_debug
								("[WCH_DRV]Wait buffer down timeout, so we must close it\r\n");
								WchHalDeinit(u4Idx);
							}
						}
						SpinLockWch();
					} else {
						SpinUnlockWch();
						WchHalDeinit(u4Idx);
						SpinLockWch();
					}

#endif
#if WCH_GRAB_FUN_EN
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "WchStop backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
				}
#endif
				_gWchParam[u4Idx].u4Status = WCH_HW_STOP;
			} else if (_gWchParam[u4Idx].u4Status != WCH_HW_FREE)
				_gWchParam[u4Idx].u4Status = WCH_HW_STOP;
			else{
				WCH_LOG(WCH_LOG_LVL_WARN,
					 "WchStop status error id=%d, eWchSrcId = %d, u4Status =%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			u4Ret = (u32)true;
			break;
		}
	}
	SpinUnlockWch();
	return u4Ret ? true : false;
}

bool WchConfig(PWCH_CTL_PARAM_T pWchCtlParam)
{
	u32 u4Idx = 0;
	bool fgRet = true;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < (u32)WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != pWchCtlParam->eSrcId)
			continue;
		else {
			if ((_gWchParam[u4Idx].u4Status != WCH_HW_READY)
			     && (_gWchParam[u4Idx].u4Status !=
				 WCH_HW_STOP) && (_gWchParam[u4Idx].u4Status != WCH_HW_CONFIG)) {

				WCH_LOG(WCH_LOG_LVL_ERR,
					 "WchConfigHw status error src type id=%d, SrcType=%d, status=%d\r\n",
					 (int)u4Idx, (int)pWchCtlParam->eSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				fgRet = false;
				break;
			}
			memcpy(&_gWchParam[u4Idx].tWchCfg, &pWchCtlParam->tWchCfg,
				  sizeof(WCH_CFG_T));
#if WCH_TEST_THREAD
			if (!fgInitWchCtl) {
				memcpy(&tWchCtlParam.tWchCfg, &pWchCtlParam->tWchCfg,
					sizeof(WCH_CFG_T));
				fgInitWchCtl = true;
			}
#endif
#if WCH_GRAB_FUN_EN
			if (u4Idx != WCH_BACKUP) {
#endif
				WCH_LOG(WCH_LOG_LVL_DBG,
					    "WchConfig id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					    (int)_gWchParam[u4Idx].u4Status);
				if (!WchConfigHw(u4Idx, &_gWchParam[u4Idx].tWchCfg)) {
					WCH_LOG(WCH_LOG_LVL_ERR,
						 "WchConfigHw return fail src type id=%d, SrcType =%d\r\n",
						 (int)u4Idx, (int)pWchCtlParam->eSrcId);
					fgRet = false;
				} else{
					if (!WchUpdateBuff(u4Idx)) {
						WCH_LOG(WCH_LOG_LVL_ERR,
							 "WchConfigHw return fail src type id=%d, SrcType =%d\r\n",
							 (int)u4Idx, (int)pWchCtlParam->eSrcId);
						fgRet = false;
					}
				}
#if WCH_GRAB_FUN_EN
			} else{
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchConfig backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
#endif
			if (fgRet)
				_gWchParam[u4Idx].u4Status = WCH_HW_CONFIG;
			break;
		}
	}

	SpinUnlockWch();
	return fgRet;
}

bool WchGetBufAddr(WCH_SRC_APP_ID_E eWchSrcId, PWCH_BUFF_INFO_T prOutBuffer)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId)
			continue;
		else {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchGetBufAddr id=%d, eWchSrcId=%d, u4Status=%d\r\n", (int)u4Idx,
				 (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status);
#if 0
			copy_to_user((void __user *)prOutBuffer, &_gWchParam[u4Idx].tWchBuf, sizeof(WCH_BUFF_INFO_T));/*double check*/
#else
			memcpy(prOutBuffer, &_gWchParam[u4Idx].tWchBuf, sizeof(WCH_BUFF_INFO_T));
#endif
			fgRet = true;
		}
	}
	SpinUnlockWch();
	return fgRet;
}

bool WchSetMirror(PWCH_CTL_PARAM_T pWchCtlParam)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != pWchCtlParam->eSrcId)
			continue;
		else {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchSetMirror id=%d, eWchSrcId=%d, u4Status=%d, u4Mirror=%d\r\n",
				 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status,
				 (int)pWchCtlParam->tWchCfg.u4Mirror);
			_gWchParam[u4Idx].tWchCfg.u4Mirror = pWchCtlParam->tWchCfg.u4Mirror;
			WchHalSetMirror(u4Idx, pWchCtlParam->tWchCfg.u4Mirror);
			fgRet = true;
		}
	}
	SpinUnlockWch();
	return fgRet;
}
EXPORT_SYMBOL(WchSetMirror);

bool _WchSetRearCanNotGrab(PWCH_CTL_PAR_T pWchCtlParam)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (; u4Idx < WCH_BACKUP; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != pWchCtlParam->eSrcId) {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "[yzq]_WchSetRearCanNotGrab id=%d, eWchSrcId=%d, eSrcId=%d, u4Status=%d, fgCanNotGrabbed=%d\r\n",
				 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
				 pWchCtlParam->eSrcId, (int)_gWchParam[u4Idx].u4Status,
				 (int)pWchCtlParam->_fgCanNotGrabbed);
			continue;
		} else{
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "_WchSetRearCanNotGrab id=%d, eWchSrcId=%d, u4Status=%d, fgCanNotGrabbed=%d\r\n",
				 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status,
				 (int)pWchCtlParam->_fgCanNotGrabbed);
			_gWchParam[u4Idx].tWchCfg.fgCanNotGrabbed = pWchCtlParam->_fgCanNotGrabbed;
			fgRet = true;
		}
	}
	SpinUnlockWch();
	return fgRet;
}

bool WchSetRearCanNotGrab(PWCH_CTL_PARAM_T pWchCtlParam)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (; u4Idx < WCH_BACKUP; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != pWchCtlParam->eSrcId)
			continue;
		else {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchSetRearCanNotGrab id=%d, eWchSrcId=%d, u4Status=%d, fgCanNotGrabbed=%d\r\n",
				 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status,
				 (int)pWchCtlParam->tWchCfg.fgCanNotGrabbed);
			_gWchParam[u4Idx].tWchCfg.fgCanNotGrabbed =
			    pWchCtlParam->tWchCfg.fgCanNotGrabbed;
			fgRet = true;
		}
	}
	SpinUnlockWch();
	return fgRet;
}

u32 WchIoControl(u32 u4Context, u32 u4CtlCode, u8 *pInBuffer, u32 u4InSize,
		       u8 *pOutBuffer, u32 u4OutSize, u32 *pOutSize)
{
	u32 u4Ret = WCH_SUCCESS, u4WchId = 0;
	PWCH_CTL_PARAM_T pCtlParam = NULL;
	PWCH_CTL_PAR_T _pCtlParam = NULL;
	WCH_SRC_APP_ID_E eSrcId = SRC_APP_UNKNOWN;

	if (!u4Context) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: context is NULL, control code %x\r\n",
			 (unsigned int)u4CtlCode);
		u4Ret = WCH_CONTEXT_NULL;
		goto Err;
	}
	if ((u4CtlCode == IOCTL_WCH_CONFIG) || (u4CtlCode == IOCTL_WCH_SET_MIRROR)) {
		if ((NULL == pInBuffer) || u4InSize < sizeof(WCH_CTL_PARAM_T)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl: Input param error %x, 0x%x, %d = %d\r\n",
				 (unsigned int)u4CtlCode, (unsigned int)pInBuffer, (int)u4InSize,
				 (int)sizeof(WCH_CTL_PARAM_T));
			u4Ret = WCH_INVALID_INPUT_PARAM;
			goto Err;
		}
	} else if (u4CtlCode == IOCTL_WCH_SET_REARGRAB) {
		if ((NULL == pInBuffer) || u4InSize < sizeof(WCH_CTL_PAR_T)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl: Input param error %x, 0x%x, %d = %d\r\n",
				 (unsigned int)u4CtlCode, (unsigned int)pInBuffer, (int)u4InSize,
				 (int)sizeof(WCH_CTL_PAR_T));
			u4Ret = WCH_INVALID_INPUT_PARAM;
			goto Err;
		}
	} else{
		if ((NULL == pInBuffer) || u4InSize < sizeof(WCH_SRC_APP_ID_E)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl: Input param error %x, 0x%x, %d\r\n",
				 (unsigned int)u4CtlCode, (unsigned int)pInBuffer, (int)u4InSize);
			u4Ret = WCH_INVALID_INPUT_PARAM;
			goto Err;
		}
	}
	if (u4CtlCode == IOCTL_WCH_OPEN) {
		if ((NULL == pOutBuffer) || u4OutSize < sizeof(u32)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl: Output param error %x, 0x%x, %d\r\n",
				 (unsigned int)u4CtlCode, (unsigned int)pOutBuffer,
				 (int)u4OutSize);
			u4Ret = WCH_INVALID_OUTPUT_PARAM;
			goto Err;
			}
	}
#if defined(CONFIG_ATC_OS_linux)
	else if (u4CtlCode == IOCTL_WCH_GET_ADDR || u4CtlCode == IOCTL_WCH_GET_VIRTUAL_ADDR) {
#elif defined(CONFIG_ATC_OS_android)
	else if (u4CtlCode == IOCTL_WCH_GET_ADDR) {
#endif
		if ((NULL == pOutBuffer) || u4OutSize < sizeof(WCH_BUFF_INFO_T)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl: Output param error %x, 0x%x, %d\r\n",
				 (unsigned int)u4CtlCode, (unsigned int)pOutBuffer,
				 (int)u4OutSize);
			u4Ret = WCH_INVALID_OUTPUT_PARAM;
			goto Err;
		}
	}
	switch (u4CtlCode) {
	case IOCTL_WCH_OPEN:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		u4WchId = WchOpen(eSrcId);
		if (u4WchId == WCH_INVALID) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: No free write channel hw %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_NO_FREE_HW;
		} else {
			if (!WchAllocBuf(u4WchId, eSrcId)) {
				WCH_LOG(WCH_LOG_LVL_ERR,
					 "WchIoControl: Alloc buffer fail %d, %d\r\n", (int)u4WchId,
					 (int)eSrcId);
				u4Ret = WCH_ALLOC_BUF_FAIL;
			} else{
#if 0
				copy_to_user((void __user *)pOutBuffer, &u4WchId, sizeof(UINT32));/*double check*/
#else
				memcpy(pOutBuffer, &u4WchId, sizeof(u32));
#endif
			}
		}
		break;
	case IOCTL_WCH_CLOSE:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		if (!WchClose(eSrcId)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchClose return fail %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	case IOCTL_WCH_START:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		if (!WchStart(eSrcId)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchStart return fail %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	case IOCTL_WCH_STOP:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		if (!WchStop(eSrcId)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchStop return fail %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	case IOCTL_WCH_CONFIG:
		pCtlParam = (PWCH_CTL_PARAM_T) pInBuffer;
		if (!WchConfig(pCtlParam)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchConfig return fail %d\r\n",
				 (int)pCtlParam->eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	case IOCTL_WCH_GET_ADDR:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		WchGetBufAddr(eSrcId, (PWCH_BUFF_INFO_T) pOutBuffer);
		if (!pOutBuffer) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchGetBufAddr return fail %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_GET_ADDR_FAIL;
		} else
			WCH_LOG(WCH_LOG_LVL_DBG, "WchIoControl: WchGetBufAddr buffer count %d\r\n",
				 (int)(((PWCH_BUFF_INFO_T) pOutBuffer)->u4BufCnt));
		if (pOutSize)
			*pOutSize = sizeof(WCH_BUFF_INFO_T);
		break;
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
	case IOCTL_WCH_GET_VIRTUAL_ADDR:
		eSrcId = (WCH_SRC_APP_ID_E) *pInBuffer;
		WchGetBufVirtualAddr(eSrcId, (PWCH_BUFF_INFO_T) pOutBuffer);
		if (!pOutBuffer) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchGetBufVirtualAddr return fail %d\r\n",
				 (int)eSrcId);
			u4Ret = WCH_GET_ADDR_FAIL;
		} else
			WCH_LOG(WCH_LOG_LVL_DBG, "WchIoControl: WchGetBufVirtualAddr buffer count %d\r\n",
				 (int)(((PWCH_BUFF_INFO_T) pOutBuffer)->u4BufCnt));
		if (pOutSize)
			*pOutSize = sizeof(WCH_BUFF_INFO_T);
		break;
#endif
#endif
	case IOCTL_WCH_SET_MIRROR:
		pCtlParam = (PWCH_CTL_PARAM_T) pInBuffer;
		if (!WchSetMirror(pCtlParam)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: WchSetMirror return fail %d\r\n",
				 (int)pCtlParam->eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	case IOCTL_WCH_SET_REARGRAB:
		_pCtlParam = (PWCH_CTL_PAR_T) pInBuffer;
		if (!_WchSetRearCanNotGrab(_pCtlParam)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchIoControl:_WchSetRearCanNotGrab return fail %d\r\n",
				 (int)_pCtlParam->eSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
		break;
	default:
		WCH_LOG(WCH_LOG_LVL_ERR, "WchIoControl: Invalid control code %x\r\n",
			 (unsigned int)u4CtlCode);
		u4Ret = WCH_INVALID_CTLCODE;
		break;
	}
Err:	return u4Ret;
}
EXPORT_SYMBOL(WchIoControl);

/* Add for fix input source mode change or signal lost write channel hang DRAM issue */
bool WchSrcMatchAppId(WCH_SRC_APP_ID_E eWchSrcId, WCH_DATA_SRC_E eSrcType)
{
	bool fgRet = false;

	if ((eWchSrcId == SRC_APP_UNKNOWN) || (eSrcType == DATA_SRC_UNKNOWN)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchSrcMatchAppId: unkown source eWchSrcId=%d, SrcType=%d\r\n",
			 (int)eWchSrcId, (int)eSrcType);
	} else{
		switch (eWchSrcId) {
		case SRC_APP_BACKCAR:
		case SRC_APP_AVIN:
			if (eSrcType == DATA_SRC_TVD)
				fgRet = true;
			break;
		case SRC_APP_YPBPR:
		case SRC_APP_VGA:
			if ((eSrcType == DATA_SRC_YPBPR) || (eSrcType == DATA_SRC_VGA))
				fgRet = true;
			break;
		case SRC_APP_HDMI:
			if (eSrcType == DATA_SRC_HDMI)
				fgRet = true;
			break;
#if WCH_SPECIAL_DGI_ENABLE
		case SRC_APP_SPECIAL_DGI:
#endif
		case SRC_APP_DGI:
			if (eSrcType == DATA_SRC_DGI)
				fgRet = true;
			break;
		case SRC_APP_DISPLAY:
			if ((eSrcType == DATA_SRC_DVD) || (eSrcType == DATA_SRC_FMTR) ||
			     (eSrcType == DATA_SRC_FMTF) || (eSrcType == DATA_SRC_MIX))
				fgRet = true;
			break;
		default:
			break;
		}
		WCH_LOG(WCH_LOG_LVL_DBG,
			 "WchSrcMatchAppId: return %d, eWchSrcId=%d, SrcType=%d\r\n", (int)fgRet,
			 (int)eWchSrcId, (int)eSrcType);
	}
	return fgRet;
}

u32 WchCheckStopHwSrc(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = WCH_COMMON_FAIL;
	WCH_DATA_SRC_E eSrcType = DATA_SRC_UNKNOWN;

	for (u4Idx = 0; u4Idx < WCH_BACKUP; u4Idx++) {
		if (WchHalIsOn(u4Idx)) {
			eSrcType = WchHalGetSrcType(u4Idx);
			if (WchSrcMatchAppId(eWchSrcId, eSrcType)) {
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchCheckStopHwSrc: stop wch id=%d, eWchSrcId=%d\r\n",
					 (int)u4Idx, (int)eWchSrcId);
				if (u4LockCnt > 0) {
					SpinUnlockWch();
					WchHalDeinit(u4Idx);
					SpinLockWch();
				} else {
					WchHalDeinit(u4Idx);
				}
				u4Ret = WCH_SUCCESS;
				break;
			}
		}
	}
	return u4Ret;
}

u32 WchStopByInputSrc(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = WCH_SUCCESS;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId)
			continue;
		else {
#if WCH_GRAB_FUN_EN
			if (u4Idx == WCH_BACKUP) {
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "WchStopByInputSrc: back up source not stop eWchSrcId=%d\r\n",
					 (int)eWchSrcId);
				u4Ret = WCH_BACKUP_SOURCE;
			} else
#endif
			{
				if (WchHalIsOn(u4Idx)) {
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "WchStopByInputSrc: stop wch id=%d, eWchSrcId=%d\r\n",
						 (int)u4Idx, (int)eWchSrcId);
					SpinUnlockWch();
					WchHalDeinit(u4Idx);
					SpinLockWch();
				} else{
					WCH_LOG(WCH_LOG_LVL_WARN,
						 "WchStopByInputSrc: not start yet wch id=%d, eWchSrcId=%d\r\n",
						 (int)u4Idx, (int)eWchSrcId);
					u4Ret = WCH_NOT_START;
				}
			}
			break;
		}
	}
	if (u4Idx == WCH_MAX) {
		/* source not match in SW flow and dobule check HW register if the source match */
		if (WchCheckStopHwSrc(eWchSrcId)) {
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchStopByInputSrc: no source match eWchSrcId=%d\r\n",
				 (int)eWchSrcId);
			u4Ret = WCH_COMMON_FAIL;
		}
	}
	SpinUnlockWch();
	return u4Ret;
}
EXPORT_SYMBOL(WchStopByInputSrc);

#if WCH_TEST_THREAD
static int WCH_TEST(void *unused)
{
	u32 u4Cnt = 0;

	msleep(10000U);
	pr_debug("must WCH_TEST start \r\n");
	for (;;) {
		if (WCH_REG_READ(0x42300) & 0x1) {
			pr_debug("STOP %d\r\n", u4Cnt);
			if (u4LockCnt > 0) {
				SpinUnlockWch();
				WchHalDeinit(0);
				SpinLockWch();
			} else {
				WchHalDeinit(0);
			}
			u4Cnt++;
			pr_debug("STOP Done status %x\r\n", (unsigned int)WCH_REG_READ(0x42344));
		} else {
			pr_debug("START %d\r\n", u4Cnt);
			WchConfigHw(0, &tWchCtlParam.tWchCfg);
			WchHalStart(0);
			pr_debug("START Done status %x\r\n", (unsigned int)WCH_REG_READ(0x42344));
			if (WCH_REG_READ(0x42344) & 0x0200) {
				msleep(60000);
				pr_debug("START status error %x\r\n",
					  (unsigned int)WCH_REG_READ(0x42344));
			}
		}
		msleep(1000U);
	}
}
#endif
#ifndef __ARM2__
WCH_CTL_PARAM_T wch_param;
u32 WchCaptureScreenStart(u32* id)
{
	u32 wch_id = WCH_INVALID, ret = 0;

	memset(&wch_param, 0, sizeof(WCH_CTL_PARAM_T));

	wch_param.eSrcId = SRC_APP_DISPLAY;
	ret = WchIoControl(1, IOCTL_WCH_OPEN, (u8 *)&wch_param.eSrcId, sizeof(WCH_SRC_APP_ID_E), 
				(u8 *)&wch_id, sizeof(u32), NULL);
	if (ret) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStart: open error id=%d, return=%d\r\n",
			(int)wch_id, (int)ret);
		goto err;
	}
	*id = wch_id;

	wch_param.tWchCfg.eInputSrc = DATA_SRC_MIX;
	wch_param.tWchCfg.eInputFmt = DATA_FMT_YUV444;
	wch_param.tWchCfg.u4SrcWidth = 1024;
	wch_param.tWchCfg.u4SrcHeight = 600;

	wch_param.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
	wch_param.tWchCfg.u4DstWidth = 1024;
	wch_param.tWchCfg.u4DstHeight = 600;
	wch_param.tWchCfg.fgProgressive = TRUE;
	wch_param.tWchCfg.fgCanNotGrabbed = TRUE;
	wch_param.tWchCfg.fgVSyncPolarity = FALSE; /* FALSE is LOW level present sync.*/   
	wch_param.tWchCfg.fgHSyncPolarity = FALSE; /* TRUE is High.*/
	wch_param.tWchCfg.fgBotFieldFirst = TRUE; /* FALSE is top  first.*/

	wch_param.tWchCfg.u1YUVMask = WCH_YUV_SEL_MASK;/*may change cause of different hw*/
	wch_param.tWchCfg.u1YSel = 1;/*above*/
	wch_param.tWchCfg.u1USel = 4;/*above*/
	wch_param.tWchCfg.u1VSel = 4;/*above*/

	wch_param.tWchCfg.u4SrcStartX = 0x61;
	wch_param.tWchCfg.u4SrcStartYTop = 0xB;
	wch_param.tWchCfg.u4SrcStartYBot = 0;

	ret = WchIoControl(1, IOCTL_WCH_CONFIG, (u8 *)&wch_param, sizeof(WCH_CTL_PARAM_T), NULL, 0, NULL);
	if (ret) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStart: config error id=%d, return=%d\r\n",
			(int)wch_id, (int)ret);
		goto err;
	}

	ret = WchIoControl(1, IOCTL_WCH_START, (u8 *)&wch_param.eSrcId, sizeof(WCH_SRC_APP_ID_E), NULL, 0, NULL);
	if (ret) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStart: start error id=%d, return=%d\r\n",
			(int)wch_id, (int)ret);
		goto err;
	}
err:
	return ret;
}

u32 WchCaptureScreenStop(void)
{
	u32 ret = 0;

	ret = WchIoControl(1, IOCTL_WCH_STOP, (u8 *)&wch_param.eSrcId, sizeof(WCH_SRC_APP_ID_E), NULL, 0, NULL);
	if (ret) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStop: stop error return=%d\r\n", (int)ret);
		goto err;
	}

	ret = WchIoControl(1, IOCTL_WCH_CLOSE, (u8 *)&wch_param.eSrcId, sizeof(WCH_SRC_APP_ID_E), NULL, 0, NULL);
	if (ret) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStop: close error return=%d\r\n", (int)ret);
		goto err;
	}
err:
	return ret;
}

void WchWriteBuffToFile(u32 u4Idx)
{
	u32 u4FrameCnt = 0, u4FrameNum = 3, u4SrcType = 3, u4BuffIdx = 0;
#if defined(CONFIG_ATC_OS_linux)
	PWCH_BUFF_INFO_T prWchBuf = &_virtualaddr[u4Idx];
#elif defined(CONFIG_ATC_OS_android)
	PWCH_BUFF_INFO_T prWchBuf = &_gWchParam[u4Idx].tWchBuf;
#endif
    for (; u4FrameCnt < u4FrameNum; u4FrameCnt++) {
		if (u4DumpFrameCnt[u4Idx]) {
			u4BuffIdx = u4DumpFrameCnt[u4Idx] % prWchBuf->u4BufCnt;
            WchDumpBuffer(u4Idx, prWchBuf->u4YBuf[u4BuffIdx], prWchBuf->u4CBuf[u4BuffIdx], u4SrcType);
            u4DumpFrameCnt[u4Idx]--;
            mdelay(100);
        }
    }
}

bool WchCaptureScreen(void)
{
	u32 u4Idx = 0;
	bool ret = true;

	if (0 == WchCaptureScreenStart(&u4Idx)) {
		mdelay(100);
		WchWriteBuffToFile(u4Idx);
        WCH_LOG(WCH_LOG_LVL_INFO, "WchWriteBuffToFile done! \r\n");
		if (0 != WchCaptureScreenStop()) {
			WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStop stop error!\r\n");
			ret = false;
		}
	} else{
		WCH_LOG(WCH_LOG_LVL_ERR, "WchCaptureScreenStart do not setup success!\r\n");
		ret = false;
	}

	return ret;
}
#endif
//#if defined(CONFIG_ATC_OS_android)
/* new api for android M add by mtk68064 */
void WchLockInit(void)
{
	if (!fgWchLockInit) {
		spin_lock_init(&lock);
		fgWchLockInit = true;
		WCH_LOG(WCH_LOG_LVL_DBG, "WchOpen init spin lock %p\r\n", &lock);
	}
}

static bool _fgInStartState[SRC_APP_MAX] = {false};
bool CheckFreeChannel(u32* index, WCH_SRC_APP_ID_E eWchSrcId)
{
	bool fgReg = false;
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < WCH_BACKUP; u4Idx++) {
		if (_gWchParam[u4Idx].u4Status != WCH_HW_FREE) {
			if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "[CheckFreeChannel] status id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				continue;
			} else{
				if (_gWchParam[u4Idx].u4Status == WCH_HW_CONFIG || 
					_gWchParam[u4Idx].u4Status == WCH_HW_STOP) {//yzq WCH_HW_READY
					WCH_LOG(WCH_LOG_LVL_WARN,
						"[CheckFreeChannel] eWchSrcId=%d was already configed ir stoped id=%d, u4Status=%d\r\n",
						(int)eWchSrcId, (int)u4Idx,
						(int)_gWchParam[u4Idx].u4Status);
					fgReg = true;
					*index = u4Idx;
				} else{
					WCH_LOG(WCH_LOG_LVL_ERR,
						"[CheckFreeChannel] eWchSrcId=%d is running or stop id=%d, u4Status=%d\r\n",
						(int)eWchSrcId, (int)u4Idx,
						(int)_gWchParam[u4Idx].u4Status);
					_fgInStartState[eWchSrcId] = true;
				}
				break;
			}
		} else {
			WCH_LOG(WCH_LOG_LVL_DBG, "[CheckFreeChannel] id=%d, eWchSrcId=%d, u4Status=%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);

			if (WchCreateEvent(u4Idx, eWchSrcId)) {
				WchIsrInit(u4Idx);
				_gWchParam[u4Idx].u4Status = WCH_HW_CONFIG;
				_gWchParam[u4Idx].eWchSrcId = eWchSrcId;
				fgReg = true;
				*index = u4Idx;
			}
			break;
		}
	}

	return fgReg;
}

bool CheckIsBackCar(u32* index, WCH_SRC_APP_ID_E eWchSrcId)
{
	bool fgReg = false;
	u32 u4Idx = 0;

	if (eWchSrcId == SRC_APP_BACKCAR) {
		for (u4Idx = 0; u4Idx < WCH_BACKUP; u4Idx++) {
			if (_gWchParam[u4Idx].tWchCfg.fgCanNotGrabbed == false) {/* can grab input source */
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "[CheckIsBackCar] backup source for backcar id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				if ((_gWchParam[u4Idx].u4Status == WCH_HW_START)
				     || (WchHalIsOn(u4Idx))) {
					/* Disable wch clock first */
					if (u4LockCnt > 0) {
						SpinUnlockWch();
						WchHalDeinit(u4Idx);
						SpinLockWch();
					} else {
						WchHalDeinit(u4Idx);
					}
				}
				/* backup front write channel source */
				memcpy(&_gWchParam[WCH_BACKUP], &_gWchParam[u4Idx],
					   sizeof(WCH_IF_PARAM_T));
				_hWchEvent[WCH_BACKUP] = _hWchEvent[u4Idx];
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
				memcpy(&_virtualaddr[WCH_BACKUP], &_virtualaddr[u4Idx],
					   sizeof(WCH_BUFF_INFO_T));
#endif
#endif
				if (WchCreateEvent(u4Idx, eWchSrcId)) {
					_gWchParam[u4Idx].u4Status = WCH_HW_CONFIG;
					_gWchParam[u4Idx].eWchSrcId = eWchSrcId;
					fgGrabWchFront = true;
					WCH_LOG(WCH_LOG_LVL_DBG,
					 "[CheckIsBackCar] new source id=%d, eWchSrcId=%d,u4Status=%d, fgGrabWchFront=%d\r\n",
					(int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					(int)_gWchParam[u4Idx].u4Status,
					(int)fgGrabWchFront);
					fgReg = true;
					*index = u4Idx;
				}
				break;
			}
		}
	}

	return fgReg;

}

bool CheckConfig(PWCH_CTL_PARAM_T pWchCtlParam)
{
	u32 u4Idx = 0;
	bool fgRet = true;

	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != pWchCtlParam->eSrcId)
			continue;/*warning*/
		else {
			if ((_gWchParam[u4Idx].u4Status != WCH_HW_STOP)
				&& (_gWchParam[u4Idx].u4Status != WCH_HW_CONFIG)) {

				WCH_LOG(WCH_LOG_LVL_ERR,
					 "[CheckConfig] status error src type id=%d, SrcType=%d, status=%d\r\n",
					 (int)u4Idx, (int)pWchCtlParam->eSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
				fgRet = false;
				break;
			}
			memcpy(&_gWchParam[u4Idx].tWchCfg, &pWchCtlParam->tWchCfg,
				  sizeof(WCH_CFG_T));

			if (u4Idx != WCH_BACKUP) {
				WCH_LOG(WCH_LOG_LVL_DBG,
					    "[CheckConfig] id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					    (int)_gWchParam[u4Idx].u4Status);
				if (!WchConfigHw(u4Idx, &_gWchParam[u4Idx].tWchCfg)) {
					WCH_LOG(WCH_LOG_LVL_ERR,
						 "[CheckConfig] return fail src type id=%d, SrcType =%d\r\n",
						 (int)u4Idx, (int)pWchCtlParam->eSrcId);
					fgRet = false;
				} else{
					if (!WchUpdateBuff(u4Idx)) {
						WCH_LOG(WCH_LOG_LVL_ERR,
							 "[CheckConfig] return fail src type id=%d, SrcType =%d\r\n",
							 (int)u4Idx, (int)pWchCtlParam->eSrcId);
						fgRet = false;
					}
				}
			} else{
				WCH_LOG(WCH_LOG_LVL_DBG,
					 "[CheckConfig] backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}

			if (fgRet)
				_gWchParam[u4Idx].u4Status = WCH_HW_CONFIG;
			break;
		}
	}

	return fgRet;
}

u32 ConfigWch(PWCH_CTL_PARAM_T pWchCtlParam)
{
	u32 index = 0;
	WCH_SRC_APP_ID_E eWchSrcId = 0;
	if (NULL == pWchCtlParam) {
		WCH_LOG(WCH_LOG_LVL_ERR,"[ConfigWch] pWchCtlParam is null.\n");
		return WCH_INVALID_INPUT_PARAM;
	}

	eWchSrcId = pWchCtlParam->eSrcId;
	if ((sizeof(pWchCtlParam->eSrcId) + sizeof(pWchCtlParam->tWchCfg)) < sizeof(WCH_CTL_PARAM_T)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "[ConfigWch]: Input param error 0x%x, %d = %d\r\n",
			 (unsigned int)pWchCtlParam, (int)(sizeof(pWchCtlParam->eSrcId) + sizeof(pWchCtlParam->tWchCfg)),
			 (int)sizeof(WCH_CTL_PARAM_T));
		return WCH_INVALID_INPUT_PARAM;
	}

	WchLockInit();

	SpinLockWch();
	/*check free channel*/
	if (!CheckFreeChannel(&index, eWchSrcId) && _fgInStartState[eWchSrcId] == false) {
		if (CheckIsBackCar(&index, eWchSrcId)) {
			WCH_LOG(WCH_LOG_LVL_DBG,"back car grap write channel!\n");
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR,"write channel can not be graped or source is not back car!\n");
			SpinUnlockWch();
			return WCH_NO_FREE_HW;
		}
	} else if (_fgInStartState[eWchSrcId] == true) {
		_fgInStartState[eWchSrcId] = false;
		WCH_LOG(WCH_LOG_LVL_ERR,"[ConfigWch]wch is working!\n");
		SpinUnlockWch();
		return WCH_COMMON_FAIL;
	} else {
	}
    
	SpinUnlockWch();

    if (!WchAllocBuf(index, eWchSrcId)) {
        WCH_LOG(WCH_LOG_LVL_ERR,"allocate buffer fail!\n");
        return WCH_ALLOC_BUF_FAIL;
    }
    
	SpinLockWch();
	/* config timing infomation*/
	if (!CheckConfig(pWchCtlParam)) {
		WCH_LOG(WCH_LOG_LVL_ERR,"[ConfigWch] CheckConfig fail!\n");
		SpinUnlockWch();
		return WCH_COMMON_FAIL;
	}
    SpinUnlockWch();

	return WCH_SUCCESS;
}
EXPORT_SYMBOL(ConfigWch);

u32 StartWch(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = 0;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			continue;
		} else{
			if (_gWchParam[u4Idx].u4Status == WCH_HW_CONFIG) {
				if (u4Idx != WCH_BACKUP) {
					WCH_LOG(WCH_LOG_LVL_DBG,
						    "[StartWch] id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						    (int)_gWchParam[u4Idx].u4Status);
					WchHalStart(u4Idx);
					WchHalSetRegTouch(u4Idx);
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "[StartWch] backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
				}

				_gWchParam[u4Idx].u4Status = WCH_HW_START;
				u4SkipVysncCnt[u4Idx] = WCH_SKIP_BUFF_CNT;
#if WCH_DUMP_BUFFER_ATTR
				u4DumpFrameCnt[u4Idx] = 0;
				u4DumpBufIdxCnt[u4Idx] = 0;
#endif
				u4Ret = true;
			} else {
				WCH_LOG(WCH_LOG_LVL_ERR,
					 "[StartWch] status error id=%d, eWchSrcId = %d, u4Status =%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			break;
		}
	}
	SpinUnlockWch();

	return u4Ret ? WCH_SUCCESS: WCH_COMMON_FAIL;
}
EXPORT_SYMBOL(StartWch);

u32 StopWch(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = 0;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			continue;
		} else{
			if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
				if (u4Idx != WCH_BACKUP) {
					WCH_LOG(WCH_LOG_LVL_DBG,
						    "[StopWch] id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						    (int)_gWchParam[u4Idx].u4Status);
					SpinUnlockWch();
					WchHalDeinit(u4Idx);
					SpinLockWch();
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "[StopWch] backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
				}
				_gWchParam[u4Idx].u4Status = WCH_HW_STOP;
				u4Ret = true;
			} else{
				WCH_LOG(WCH_LOG_LVL_WARN,
					 "[StopWch] status error id=%d, eWchSrcId = %d, u4Status =%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status); 
			}
			break;
		}
	}
	SpinUnlockWch();

	return u4Ret ? WCH_SUCCESS: WCH_COMMON_FAIL;
}
EXPORT_SYMBOL(StopWch);

bool StopHardware(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0, u4Ret = 0;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < (u32)WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			continue;
		} else{
			if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
				if (u4Idx != WCH_BACKUP) {
					WCH_LOG(WCH_LOG_LVL_DBG,
						    "WchStop id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						    (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						    (int)_gWchParam[u4Idx].u4Status);
					SpinUnlockWch();
					WchHalDeinit(u4Idx);
					SpinLockWch();
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "WchStop backup source not update register id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
				}
				u4Ret = true;
				_gWchParam[u4Idx].u4Status = WCH_HW_STOP;
			} else if (_gWchParam[u4Idx].u4Status != WCH_HW_FREE) {
				_gWchParam[u4Idx].u4Status = WCH_HW_STOP;
				u4Ret = true;
			} else{
				WCH_LOG(WCH_LOG_LVL_WARN,
					 "WchStop status error id=%d, eWchSrcId = %d, u4Status =%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			break;
		}
	}
	SpinUnlockWch();

	return u4Ret ? true : false;
}

bool CleanupContext(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Idx = 0;
	bool fgRet = false;

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "[CleanupContext] status id=%d, eWchSrcId = %d, u4Status =%d\r\n",
				 (int)u4Idx, (int)eWchSrcId, (int)_gWchParam[u4Idx].u4Status);
			continue;
		} else {
			if (_gWchParam[u4Idx].u4Status == WCH_HW_STOP) {
				WchCloseEvent(u4Idx);

				if (u4Idx != WCH_BACKUP) {
					/* backcar source exit and recover backup write channel source */
					if (fgGrabWchFront
						&& (_gWchParam[u4Idx].eWchSrcId == SRC_APP_BACKCAR)) {
						WCH_LOG(WCH_LOG_LVL_DBG,
							 "[CleanupContext] backcar and recover backup source id=%d, eWchSrcId=%d, u4Status=%d\r\n",
							 (int)u4Idx,
							 (int)_gWchParam[WCH_BACKUP].eWchSrcId,
							 (int)_gWchParam[WCH_BACKUP].u4Status);
						memcpy(&_gWchParam[u4Idx], &_gWchParam[WCH_BACKUP],
							sizeof(WCH_IF_PARAM_T));
						_hWchEvent[u4Idx] = _hWchEvent[WCH_BACKUP];
#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
						memcpy(&_virtualaddr[u4Idx], &_virtualaddr[WCH_BACKUP],
							sizeof(WCH_BUFF_INFO_T));
#endif
#endif
						fgRet = WchRecoverSrc(u4Idx, &_gWchParam[u4Idx]);
						{
							if (fgRet) {
								memset(&_gWchParam[WCH_BACKUP], 0,
									sizeof(WCH_IF_PARAM_T));
								_hWchEvent[WCH_BACKUP] = NULL;
								fgGrabWchFront = false;
							}
						}
					} else {
						WCH_LOG(WCH_LOG_LVL_DBG,
							 "[CleanupContext] id=%d, eWchSrcId=%d, u4Status=%d\r\n",
							 (int)u4Idx,
							 (int)_gWchParam[u4Idx].eWchSrcId,
							 (int)_gWchParam[u4Idx].u4Status);
#ifndef __ARM2__
						if (u4Idx == 0)
							spin_lock_irqsave(&protect_gWchParam0, fg_gWchParam[0]);
						else
							spin_lock_irqsave(&protect_gWchParam1, fg_gWchParam[1]);
#endif
						memset(&_gWchParam[u4Idx], 0, sizeof(WCH_IF_PARAM_T));
#if defined(CONFIG_ATC_OS_linux)
						SpinUnlockWch();
#ifndef __ARM2__
						WchFreeBufferAddr(u4Idx, eWchSrcId);
#endif
						SpinLockWch();
#endif
#ifndef __ARM2__
						if (u4Idx == 0)
							spin_unlock_irqrestore(&protect_gWchParam0, fg_gWchParam[0]);
						else
							spin_unlock_irqrestore(&protect_gWchParam1, fg_gWchParam[1]);
#endif
					}
				} else{
					WCH_LOG(WCH_LOG_LVL_DBG,
						 "[CleanupContext] backup source id=%d, eWchSrcId=%d, u4Status=%d\r\n",
						 (int)u4Idx, (int)_gWchParam[u4Idx].eWchSrcId,
						 (int)_gWchParam[u4Idx].u4Status);
					_gWchParam[u4Idx].u4Status = WCH_HW_FREE;
				}
				fgRet = true;
			} else{
				WCH_LOG(WCH_LOG_LVL_WARN,
					 "[CleanupContext] status error id=%d, eWchSrcId=%d, u4Status=%d\r\n",
					 (int)u4Idx, (int)eWchSrcId,
					 (int)_gWchParam[u4Idx].u4Status);
			}
			break;
		}
	}
	SpinUnlockWch();

	return fgRet;
}

u32 CloseWch(WCH_SRC_APP_ID_E eWchSrcId)
{
	u32 u4Ret = 0;

	if (StopHardware(eWchSrcId)) {
		if (CleanupContext(eWchSrcId)) {
			u4Ret = true;
			WCH_LOG(WCH_LOG_LVL_DBG,"[CloseWch] success.\n");
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR,"[CloseWch] CleanupContext fail.\n");
		}
	}else {
		WCH_LOG(WCH_LOG_LVL_ERR,"[CloseWch] StopHardware fail.\n");
	}

	return u4Ret ? WCH_SUCCESS: WCH_COMMON_FAIL;
}
EXPORT_SYMBOL(CloseWch);

u32 WchGetBufferAddress(PWCH_BUF_T pWchGetBuf)
{
	u32 u4Idx = 0;
	bool fgRet = false;
	WCH_SRC_APP_ID_E eWchSrcId = 0;
	if (NULL == pWchGetBuf) {
		WCH_LOG(WCH_LOG_LVL_ERR,"[WchGetBufferAddress] pWchGetBuf is null.\n");
		return WCH_INVALID_INPUT_PARAM;
	}
	eWchSrcId = pWchGetBuf->eSrcId;

	if (!(&pWchGetBuf->tWchBuf)) {
		WCH_LOG(WCH_LOG_LVL_ERR,"[WchGetBufferAddress] pWchGetBuf->tWchBuf is null.\n");
		return WCH_INVALID_INPUT_PARAM;
	} else if (sizeof(pWchGetBuf->tWchBuf) < sizeof(WCH_BUFF_INFO_T)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "[WchGetBufferAddress]: Output param error %d, %d\r\n",
			 (unsigned int)sizeof(pWchGetBuf->tWchBuf),
			 (int)sizeof(WCH_BUFF_INFO_T));
		return WCH_INVALID_OUTPUT_PARAM;
	}

	SpinLockWch();
	for (u4Idx = 0; u4Idx < WCH_MAX; u4Idx++) {
		if (_gWchParam[u4Idx].eWchSrcId != eWchSrcId)
			continue;
		else {
			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchGetBufAddr id=%d, eWchSrcId=%d, u4Status=%d\r\n", (int)u4Idx,
				 (int)_gWchParam[u4Idx].eWchSrcId,
				 (int)_gWchParam[u4Idx].u4Status);
			memcpy(&pWchGetBuf->tWchBuf, &_gWchParam[u4Idx].tWchBuf, sizeof(WCH_BUFF_INFO_T));
			fgRet = true;
		}
	}
	SpinUnlockWch();

	return fgRet ? WCH_SUCCESS: WCH_GET_ADDR_FAIL;
}
EXPORT_SYMBOL(WchGetBufferAddress);
//#endif

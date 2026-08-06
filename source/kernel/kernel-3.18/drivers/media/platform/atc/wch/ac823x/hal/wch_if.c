/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */
#ifndef __ARM2__
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>/*wake_up_process*/
#include <linux/kthread.h>/*kthread_create*/
#include <linux/err.h>/*IS_ERR,PTR_ERR*/
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/semaphore.h>/*mutex*/
#include "x_os.h"
#include "x_typedef.h"
#else
#include "x_bim_83xx.h"
#include "reserve_memory.h"
#endif
#include "wch_drv.h"
#include "wch_log.h"
#include "wch_if.h"
#include "wch_hal.h"
#include "wch_priv.h"
//#include <generated/atc_project.h>


#if WCH_DUMP_FIRST_20_FRAMES_BY_ICE
#define WCH_SKIP_BUFF_CNT 0
#else
#define WCH_SKIP_BUFF_CNT 4
#endif //WCH_DUMP_FIRST_20_FRAMES_BY_ICE


#define WCH_DBG_TIME  0
#define WCH_DBG_ARM2_ISR_TIME  0

#define WCH_N_SRC_INTERVAL_MIN		30
#define WCH_PAL_SRC_INTERVAL_MIN	35
#define WCH_WRITED_BUF_IDX_OFT 3U


unsigned int _u4WCH_DBG_LVL = WCH_LOG_LVL_HAL;
#ifndef __ARM2__
int _g_show_isr_interval = 0;
#endif

unsigned char *_pcWchLogLevel[] = {
	"[WCH] [ERR]",
	"[WCH] [WARN]",
	"[WCH] [INFO]",
	"[WCH] [HAL]",
	"[WCH] [DBG]",
	"[WCH] [IRQ]",
};

#if WCH_DUMP_BUFFER_ATTR
int u4DumpFrameCnt[WCH_NUM] = {0};

int u4DumpBufIdxCnt[WCH_NUM] = {0};
#endif

bool fgWchIsrInit[WCH_NUM] = {false, false, false, false, false, false,	false, false, false};
bool fgWchLockInit = false;

//unsigned long wch_reserve = 0x201e0000;/*init for arm2*/
unsigned long wch_reserve = 0;

WCH_IF_PARAM_T _gWchParam[WCH_NUM];
#ifndef __ARM2__
extern spinlock_t wchParamLock[WCH_NUM];
unsigned long fgWchParam[WCH_NUM];
static uintptr_t _hSendBufMsgQ[WCH_NUM] = {(uintptr_t)NULL, (uintptr_t)NULL};
#endif
u32 u4SkipVysncCnt[WCH_NUM] = {
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT,
WCH_SKIP_BUFF_CNT
};
#define WCH5_IRQ  59 //for arm2

static wchLock wchlock;
/*get irq vector from dts*/
int wchirq[WCH_NUM] = {0, 0, 0, 0, 59/*init for arm2*/, 0, 0, 0, 0};

WCH_TIMING_PARAM_T *prWchRetiming = NULL;
WCH_TIMING_PARAM_T WchTimingTable[] = {

	/* Define: Timing Mode, HsyncInv, HPixel, HActive, VsyncInv, VTopLine, VBotLine, Vactive */
	{WCH_640_480P_60HZ, 1, 144, 640, 1, 33, 33, 480},
	{WCH_1280_720P_60HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_1920_1080I_60HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_720_480P_60HZ, 1, 120, 720, 1, 30, 30, 480},
	{WCH_720_480I_60HZ, 1, 228, 1440, 1, 15, 16, 240},
	{WCH_1280_720P_50HZ, 0, 220, 1280, 0, 20, 20, 720},
	{WCH_1920_1080I_50HZ, 0, 148, 1920, 0, 15, 16, 540},
	{WCH_720_576P_50HZ, 1, 136, 720, 1, 39, 39, 576},
	{WCH_720_576I_50HZ, 1, 276, 1440, 1, 19, 20, 288},
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


#ifndef __ARM2__
struct task_struct *hWchInst[WCH_NUM] = {NULL, NULL};
extern struct semaphore wchSem[WCH_NUM];
#endif

#ifdef __ARM2__
#define spin_lock_init(lock)
#define spin_lock_irqsave(lock, flags)
#define spin_unlock_irqrestore(lock, flags)
#endif
void WchLockInit(void)
{
	if (!fgWchLockInit) {
		spin_lock_init(&wchlock.lock);
		fgWchLockInit = true;
		WCH_LOG(WCH_LOG_LVL_DBG, "WchLockInit %p \n", &wchlock.lock);
	}
}

void SpinLockWch(void)
{
	spin_lock_irqsave(&wchlock.lock, wchlock.flags);
	wchlock.u4LockCnt++;
	WCH_LOG(WCH_LOG_LVL_IRQ, "[SpinLockWch] u4LockCnt=%d \n", (int)wchlock.u4LockCnt);
}

void SpinUnlockWch(void)
{
	wchlock.u4LockCnt--;
	spin_unlock_irqrestore(&wchlock.lock, wchlock.flags);
	WCH_LOG(WCH_LOG_LVL_IRQ, "[SpinUnlockWch] u4LockCnt=%d \n", (int)wchlock.u4LockCnt);
}


#ifndef __ARM2__
int WchGetBuffIdxThread(void *data)
{
	unsigned int u4BufIdx = 0;
	unsigned int wchId = 0;
	int32_t i4Ret = 0;
	__u16 u2MsgIdx = 0;
	uint32_t u4WchBufIdx = 0;
	size_t z_msg_size = sizeof(uint32_t);
#if WCH_DBG_TIME
	long usec = 0;
	struct timeval end_t = { 0, 0 };
	struct timeval start_t = { 0, 0 };
#endif

	if (NULL == data) {
		WCH_LOG(WCH_LOG_LVL_INFO, "WchGetBuffIdxThread! parameter error\r\n");
		return 0;
	}

	wchId = *((unsigned int *)data);

	WCH_LOG(WCH_LOG_LVL_INFO, "WchGetBuffIdxThread! WCH_%d enter, &wchSem[%d]: %p\r\n",
		(int)(wchId + 1), (int)(wchId), &wchSem[wchId]);

	for (;;) {

		if (kthread_should_stop()){
			break;
		}
		//down(&wchSem[wchId]);
		if ((uintptr_t)NULL == _hSendBufMsgQ[wchId]){
			WCH_LOG(WCH_LOG_LVL_DBG, " WchGetBuffIdxThread! msgQ %d is NULL\r\n", wchId);
			break;
		}
		i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &(_hSendBufMsgQ[wchId]),
			1, X_MSGQ_OPTION_WAIT);

		spin_lock_irqsave(&wchParamLock[wchId], fgWchParam[wchId]);

		WCH_LOG(WCH_LOG_LVL_DBG, " WchGetBuffIdxThread! spin lock u1WchId: %d\r\n", (int)wchId);

		if (_gWchParam[wchId].tWchCfg.GetWchBufIndx) {

			WCH_LOG(WCH_LOG_LVL_DBG, " WchGetBuffIdxThread! u4WchBufIdx: %d\r\n", u4WchBufIdx);

			u4BufIdx = (u4WchBufIdx + _gWchParam[wchId].tWchBuf.u4BufCnt - WCH_WRITED_BUF_IDX_OFT)
				% _gWchParam[wchId].tWchBuf.u4BufCnt;
			WCH_LOG(WCH_LOG_LVL_DBG, " WchGetBuffIdxThread! u4BufIdx send: %d\r\n", (int)u4BufIdx);
#if WCH_DBG_TIME
			do_gettimeofday(&start_t);
#endif

			_gWchParam[wchId].tWchCfg.GetWchBufIndx(&u4BufIdx);
#if WCH_SYNC_BUFFER
			_gWchParam[wchId].tWchBuf.u4BufFlag |= ((unsigned int)1 << u4BufIdx);
#endif

#if WCH_DBG_TIME
			do_gettimeofday(&end_t);

			if (end_t.tv_usec >= start_t.tv_usec)
			{
				usec =
					(end_t.tv_sec - start_t.tv_sec) * 1000000 + (end_t.tv_usec -
										  start_t.tv_usec);

				if (usec) {
					WCH_LOG(WCH_LOG_LVL_INFO, "GetWchBufIndx usec = %d ************\r\n", (int)usec);
				}
			}
			else {
				usec =
					(end_t.tv_sec - 1 - start_t.tv_sec) * 1000000 + (end_t.tv_usec + 1000000 -
										  start_t.tv_usec);

				if (usec) {
					WCH_LOG(WCH_LOG_LVL_INFO, "1 GetWchBufIndx usec = %d ************\r\n", (int)usec);
				}
			}
#endif
		}

		WCH_LOG(WCH_LOG_LVL_DBG, " WchGetBuffIdxThread! spin unlock u1WchId: %d\r\n", (int)wchId);
		spin_unlock_irqrestore(&wchParamLock[wchId], fgWchParam[wchId]);
	}
	complete_and_exit(NULL, 0);

	return 0;
}

static unsigned int wchId[WCH_NUM] = {0, 1, 2,	3, 4, 5, 6, 7, 8};

void WchCreateKthread(int (*threadfn)(void *data), unsigned int wch_id, const char *namefmt)
{
	WCH_LOG(WCH_LOG_LVL_INFO, " WchCreateKthread enter aa wch_id: %d!\r\n", (int)wch_id);
	hWchInst[wch_id] = kthread_create(threadfn, (void *)&wchId[wch_id], namefmt);


	if (!IS_ERR(hWchInst[wch_id])) {
		WCH_LOG(WCH_LOG_LVL_INFO, "WchCreateKthread create thread for wch%d, success = 0x%p \n", (unsigned int)wch_id, hWchInst[wch_id]);
		wake_up_process(hWchInst[wch_id]);
	}
	else
	{
		WCH_LOG(WCH_LOG_LVL_INFO, " WchCreateKthread kthread_create fail!\r\n");
		hWchInst[wch_id] = NULL;
	}
}

static unsigned char *name_format[WCH_NUM] = {
	"GetBufferKthread1",
	"GetBufferKthread2",
	"GetBufferKthread3",
	"GetBufferKthread4",
	"GetBufferKthread5",
	"GetBufferKthread6",
	"GetBufferKthread7",
	"GetBufferKthread8",
	"GetBufferKthread9"
};

static unsigned char *name_msgq[WCH_NUM] = {
	"Index_MSGQ1",
	"Index_MSGQ2",
	"Index_MSGQ3",
	"Index_MSGQ4",
	"Index_MSGQ5",
	"Index_MSGQ6",
	"Index_MSGQ7",
	"Index_MSGQ8",
	"Index_MSGQ9"
};

void WchEventThreadInit(void)
{
	unsigned int i = 0;

	WCH_LOG(WCH_LOG_LVL_INFO, " WchGetBuffIdxThread!\r\n");
	for (i = 0; i < WCH_NUM; i++)
	{
		hWchInst[i] = NULL;
		_hSendBufMsgQ[i] = (uintptr_t)NULL;

		if (OSR_OK != x_msg_q_create(&(_hSendBufMsgQ[i]), name_msgq[i], sizeof(unsigned int), 10)) {
			WCH_LOG(WCH_LOG_LVL_ERR, "	x_msg_q_create fail!\r\n");
			return;
		}

		WchCreateKthread(WchGetBuffIdxThread, i, name_format[i]);
		if (hWchInst[i] == NULL) {
			WCH_LOG(WCH_LOG_LVL_ERR, "	WchCreateKthread %d fail!\r\n", i);
			return;
		}
	}
}

void WchEventThreadDeinit(void)
{
	unsigned int i = 0;

	for (i = 0; i < WCH_NUM; i++)
	{
		if (NULL != hWchInst[i])
		{
			kthread_stop(hWchInst[i]);
			hWchInst[i] = NULL;
		}

		if (_hSendBufMsgQ[i] != (uintptr_t)NULL)
		{
			x_msg_q_delete(_hSendBufMsgQ[i]);
			_hSendBufMsgQ[i] = (uintptr_t)NULL;
		}
	}
}
#endif

void wchSwitchBuffer(unsigned char u1WchId)
{
	PWCH_BUFF_INFO_T pbuffer = &_gWchParam[u1WchId].tWchBuf;
	PWCH_IF_PARAM_T pparam = &_gWchParam[u1WchId];
	unsigned long u8YAddr = 0;
	unsigned long u8CAddr = 0;

#if WCH_SYNC_BUFFER
	bool bufferOK = 0;
	bufferOK = ((pbuffer->u4BufFlag & ((unsigned int)1 << pparam->u4BufIdx)) == 0);
	if (bufferOK) {
		u8YAddr = pbuffer->u4YBuf[pparam->u4BufIdx];
		u8CAddr = pbuffer->u4CBuf[pparam->u4BufIdx];
		pbuffer->fgUseBackupBuffer = false;
	} else {
		pbuffer->fgUseBackupBuffer = true;
		u8YAddr = WCH_BACKUP_BUFFER_BASE;
		u8CAddr = WCH_BACKUP_BUFFER_BASE + WCH_VDO_YBUF_SIZE;
		WCH_LOG(WCH_LOG_LVL_INFO, "wchSwitchBuffer Buffer not idle, u4BufIdx=%d \n", pparam->u4BufIdx);
	}
#else
	u8YAddr = pbuffer->u4YBuf[pparam->u4BufIdx];
	u8CAddr = pbuffer->u4CBuf[pparam->u4BufIdx];
#endif

	wchHalSetYCAddress(u1WchId, u8YAddr, u8CAddr);


#ifdef __ARM2__
	WCH_LOG(WCH_LOG_LVL_DBG, "wchSwitchBuffer u8YAddr=0x%x ,u8CAddr=%x, u4BufIdx=%d \n",
		(unsigned int)u8YAddr, (unsigned int)u8CAddr, pparam->u4BufIdx);
#else
	WCH_LOG(WCH_LOG_LVL_DBG, "wchSwitchBuffer u8YAddr=%lx ,u8CAddr=%lx, u4BufIdx=%d \n", u8YAddr, u8CAddr, pparam->u4BufIdx);
#endif

#if WCH_SYNC_BUFFER
	if (bufferOK) {
		pparam->u4BufIdx++;
	}
#else
	pparam->u4BufIdx++;
#endif
	if (pparam->u4BufIdx == WCH_BUF_MAX_CNT){
		pparam->u4BufIdx = 0;
	}
}

bool GetWchIndexFromIsrId(int u2Vector, unsigned int *u4Idx)
{
	int i = 0;
	for (i = 0; i< WCH_NUM; i++) {
		if (u2Vector == wchirq[i]){
			*u4Idx = i;
			break;
		}
	}
	if (WCH_NUM == i) {
		WCH_LOG(WCH_LOG_LVL_ERR, "GetWchIndexFromIsrId error u2Vector=%d \n",  u2Vector);
		return false;
	}
	WCH_LOG(WCH_LOG_LVL_DBG, "GetWchIndexFromIsrId id=%d ,u2Vector=%d \n", *u4Idx, u2Vector);
	return true;
}


#ifdef __ARM2__
extern void v_clear_bim_irq(unsigned int u4Id);
#endif

extern void mt33xx_mask_ack_bim_irq(unsigned int virq);

#ifdef __ARM2__
static unsigned int GetBootTime(VOID)
{
	return ((0xFFFFFFFF - (*((volatile uint32_t*)(0x1000814C))))/27000);
}
static unsigned int _g_start = 0;
#else
static struct timeval _g_start_t[WCH_NUM];
#endif

#ifdef __ARM2__
void setIndexToArm2(unsigned int u4wchId)
{
	unsigned int u4BufIdx = 0;
	PWCH_IF_PARAM_T pparam = &_gWchParam[u4wchId];

	if (pparam->tWchCfg.GetWchBufIndx) {
		u4BufIdx = (pparam->u4BufIdx + pparam->tWchBuf.u4BufCnt - WCH_WRITED_BUF_IDX_OFT)
			% pparam->tWchBuf.u4BufCnt;
		WCH_LOG(WCH_LOG_LVL_DBG, "setIndexToArm2 u4BufIdx=%d \n", (int)u4BufIdx);
		pparam->tWchCfg.GetWchBufIndx(&u4BufIdx);
#if WCH_DBG_ARM2_ISR_TIME
		{
			unsigned int tmp = GetBootTime();
			WCH_LOG(WCH_LOG_LVL_INFO, "Wch_Isr isr interval u4BufIdx=%d GetWchBufIndx= %d ms ************\r\n", (int)u4BufIdx, tmp - _g_start);
			_g_start = tmp;
		}
#endif
	}
}
#endif

irqreturn_t Wch_Isr(int u2Vector, void *dev_id)
{
	unsigned int u4Idx = 0;
#ifndef __ARM2__
	struct timeval tmp = {0, 0};
#endif
	long usec = 0;

	//SpinLockWch();
	if (!GetWchIndexFromIsrId(u2Vector, &u4Idx)) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchIsr fail interrupt = %d, status = %d \n",
			 (int)u2Vector, (int)_gWchParam[u4Idx].u4Status);
		//SpinUnlockWch();
		return -1;
	}

#ifdef __ARM2__
	#if WCH_DBG_ARM2_ISR_TIME
		{
			unsigned int tmp = GetBootTime();
			if (_g_start != 0) {
				WCH_LOG(WCH_LOG_LVL_INFO, "Wch_Isr isr interval = %d ms ************\r\n", tmp - _g_start);
			}
			_g_start = tmp;
		}
	#endif
#else
		if (u4Idx >= WCH_1 && u4Idx <= WCH_5) {
			do_gettimeofday(&tmp);
			if (//0 != _g_start_t[u4Idx].tv_usec &&
				0 != _g_start_t[u4Idx].tv_sec &&
				0 == u4SkipVysncCnt[u4Idx]) {
				if (tmp.tv_usec >= _g_start_t[u4Idx].tv_usec)
				{
					usec = ((tmp.tv_sec - _g_start_t[u4Idx].tv_sec) * 1000000 + (tmp.tv_usec - _g_start_t[u4Idx].tv_usec)) / 1000;

					if (usec && _g_show_isr_interval) {
						WCH_LOG(WCH_LOG_LVL_INFO, "Wch_Isr usec = %d ms ************\r\n", (int)usec);
					}
				}
				else {
					usec = ((tmp.tv_sec - 1 - _g_start_t[u4Idx].tv_sec) * 1000000 + (tmp.tv_usec + 1000000 - _g_start_t[u4Idx].tv_usec)) / 1000;

					if (usec && _g_show_isr_interval) {
						WCH_LOG(WCH_LOG_LVL_INFO, "1 Wch_Isr usec = %d ms ************\r\n", (int)usec);
					}
				}
			}
			_g_start_t[u4Idx].tv_usec = tmp.tv_usec;
			_g_start_t[u4Idx].tv_sec = tmp.tv_sec;

			if (0 == u4SkipVysncCnt[u4Idx] &&
				_gWchParam[u4Idx].tWchCfg.u4SrcWidth == 720 &&
				((_gWchParam[u4Idx].tWchCfg.u4SrcHeight == 480 &&
				usec < WCH_N_SRC_INTERVAL_MIN) ||
				(_gWchParam[u4Idx].tWchCfg.u4SrcHeight == 576 &&
				usec < WCH_PAL_SRC_INTERVAL_MIN))) {
				WCH_LOG(WCH_LOG_LVL_INFO, "Wch_Isr DDDDDDDDdrop bad frame, usec = %d ms ************\r\n", (int)usec);

				mt33xx_mask_ack_bim_irq(u2Vector);
				return IRQ_HANDLED;
			}
		}
#endif

	if (u4SkipVysncCnt[u4Idx]) {
		WCH_LOG(WCH_LOG_LVL_INFO, "WchIsr interrupt = %d, need skip = %d\r\n",
			 (int)u2Vector, (int)u4SkipVysncCnt[u4Idx]);
		u4SkipVysncCnt[u4Idx]--;
		if (0 == u4SkipVysncCnt[u4Idx]) {
			/*Before opening WCH shadow, we have switched a buffer to WCH. The first Buffer will be write immediately after WCH be enabled.
			   But after opening WCH shadow, the buffer being switched to WCH will be write in next interrupt.
			   So we need to switch a buffer to WCH before writing the first valid frame to buffer*/
			if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
				wchSwitchBuffer(u4Idx);
			}
		}
	}
	else {
		if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
			wchSwitchBuffer(u4Idx);
		}
#ifdef __ARM2__
		setIndexToArm2(u4Idx);
#else

		//up(&wchSem[u4Idx]);/*sync with WchGetBuffIdxThread*/
		if ((uintptr_t)NULL != _hSendBufMsgQ[u4Idx]
#if WCH_SYNC_BUFFER
			&& !_gWchParam[u4Idx].tWchBuf.fgUseBackupBuffer
#endif
			) {
			x_msg_q_send(_hSendBufMsgQ[u4Idx], &(_gWchParam[u4Idx].u4BufIdx), sizeof(unsigned int), 1);
		}
#endif
	}

	WCH_LOG(WCH_LOG_LVL_DBG, "WchIsr interrupt = %d, u4BufIdx = %d\n",
		(int)u2Vector, (int)_gWchParam[u4Idx].u4BufIdx);

	if (_gWchParam[u4Idx].u4Status == WCH_HW_START) {
		if (0 == u4SkipVysncCnt[u4Idx]) {
#if WCH_DUMP_BUFFER_ATTR
			u4DumpFrameCnt[u4Idx]++;
#endif


#if WCH_DUMP_FIRST_20_FRAMES_BY_ICE
			if (WCH_BUF_MAX_CNT == u4DumpFrameCnt[u4Idx]) {
				wchHalStop(u4Idx);
				wchHalDeInit(u4Idx);
			}
#endif //WCH_DUMP_FIRST_20_FRAMES_BY_ICE
		}
	}

	//SpinUnlockWch();

#ifndef __ARM2__
	mt33xx_mask_ack_bim_irq(u2Vector);
#else//yzq to do
	v_clear_bim_irq(u2Vector);
#endif
	return IRQ_HANDLED;
}

static unsigned char *irq_name[WCH_NUM] = {
	"wch1_ISR",
	"wch2_ISR",
	"wch3_ISR",
	"wch4_ISR",
	"wch5_ISR",
	"wch6_ISR",
	"wch7_ISR",
	"wch8_ISR",
	"wch9_ISR"
};


#ifdef __ARM2__
extern void v_enable_bim_irq(unsigned int u4Id);
extern void v_disable_bim_irq(unsigned int u4Id);
#endif
unsigned int WchRequstIrq(unsigned char u1WchId, irq_handler_t isr)
{
	int irq_vector = 0;

	WCH_LOG(WCH_LOG_LVL_IRQ,"WchRequstIrq! u1WchId = %d \n", u1WchId);
	if (u1WchId >= WCH_NUM) {
		WCH_LOG(WCH_LOG_LVL_IRQ,"WchRequstIrq! error u1WchId = %d \n", u1WchId);
		return -1;
	}

	irq_vector = wchirq[u1WchId];

	WCH_LOG(WCH_LOG_LVL_IRQ,"WchRequstIrq! irq_vector = %d \n", irq_vector);
#ifndef __ARM2__
	return request_irq(irq_vector, isr, 0, irq_name[u1WchId], NULL);
#else//yzq to do
	//v_enable_bim_irq(irq_vector);
	return 0;
#endif
}
#undef OSR_OK
#define OSR_OK (unsigned int)(0)
void WchIsrInit(unsigned char u1WchId)
{
	if (!fgWchIsrInit[u1WchId]) {
		if (WchRequstIrq(u1WchId, Wch_Isr) == OSR_OK) {
			WCH_LOG(WCH_LOG_LVL_INFO, "WchIsrInit success idx = %d \n", u1WchId);
			fgWchIsrInit[u1WchId] = true;
		} else
			WCH_LOG(WCH_LOG_LVL_ERR, "WchIsrInit fail idx = %d \n", u1WchId);
	}
}

void WchSetSourceBaseAddr(unsigned long wchReservebase)
{
	wch_reserve = wchReservebase;
}

static bool WchGetRetimingInfo(WCH_TIMING_E eSrcTiming)
{
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < sizeof(WchTimingTable) / sizeof(WCH_TIMING_PARAM_T); u4Idx++) {
		if (WchTimingTable[u4Idx].eTiming == eSrcTiming) {
			prWchRetiming = &WchTimingTable[u4Idx];
			break;
		}
	}
	if ((u4Idx == (u32)WCH_MODE_NUM) || (prWchRetiming == NULL)) {
		WCH_LOG(WCH_LOG_LVL_ERR,
			 "WchGetRetimingInfo error table idx=%d, prWchRetiming=%p\r\n", (int)u4Idx,
			 prWchRetiming);
		return false;
	}
	WCH_LOG(WCH_LOG_LVL_DBG, "WchGetRetimingInfo sucess %d, %d, %d, %d, %d, %d, %d, %d\r\n",
		 (int)prWchRetiming->eTiming, (int)prWchRetiming->u2HsyncInv,
		 (int)prWchRetiming->u2HPixel, (int)prWchRetiming->u2HActive,
		 (int)prWchRetiming->u2VsyncInv, (int)prWchRetiming->u2VTopLine,
		 (int)prWchRetiming->u2VBotLine, (int)prWchRetiming->u2VActive);

	return true;
}


bool wchAllocBuf(unsigned char u1WchId)
{
	PWCH_BUFF_INFO_T pbuffer = &_gWchParam[u1WchId].tWchBuf;
	PWCH_IF_PARAM_T pparam = &_gWchParam[u1WchId];
	bool ret = true;
	unsigned int i = 0;
	unsigned long base_addr = 0;
	unsigned long y_buffer_size = 0;
	unsigned long c_buffer_size = 0;

	if ((u1WchId < WCH_1) || (u1WchId > WCH_9))
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", u1WchId);
		return false;
	}

	switch (u1WchId){
	case WCH_1:
	{
		switch(pparam->eWchSrcId) {
			case SRC_APP_AVM_WCH1:
				base_addr = WCH_AVM_WCH1_BASE;
				y_buffer_size = WCH_AVM_WCH1_YBUF_SIZE;
				c_buffer_size = WCH_AVM_WCH1_CBUF_SIZE;
				break;

			case SRC_APP_BACKCAR_WCH1:
				base_addr = WCH_BACKCAR_BASE;
				y_buffer_size = WCH_BACKCAR_YBUF_SIZE;
				c_buffer_size = WCH_BACKCAR_CBUF_SIZE;
				break;

			case SRC_APP_AVIN_WCH1:
				base_addr = WCH_AVIN_BASE;
				y_buffer_size = WCH_AVIN_YBUF_SIZE;
				c_buffer_size = WCH_AVIN_CBUF_SIZE;
				break;

			case SRC_APP_DGI656_WCH1:
				base_addr = WCH_BT656_BASE;
				y_buffer_size = WCH_BT656_YBUF_SIZE;
				c_buffer_size = WCH_BT656_CBUF_SIZE;
				break;

			case SRC_APP_DGI601_WCH1:
				base_addr = WCH_BT601_BASE;
				y_buffer_size = WCH_BT601_YBUF_SIZE;
				c_buffer_size = WCH_BT601_CBUF_SIZE;
				break;

			default:
				WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH1, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
				return false;
		}
		break;
	}
	case WCH_2:
		if (SRC_APP_AVM_WCH2 != pparam->eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH2, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
			return false;
		}
		base_addr = WCH_AVM_WCH2_BASE;
		y_buffer_size = WCH_AVM_WCH2_YBUF_SIZE;
		c_buffer_size = WCH_AVM_WCH2_CBUF_SIZE;
		break;
	case WCH_3:
		if (SRC_APP_AVM_WCH3 != pparam->eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH3, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
			return false;
		}
		base_addr = WCH_AVM_WCH3_BASE;
		y_buffer_size = WCH_AVM_WCH3_YBUF_SIZE;
		c_buffer_size = WCH_AVM_WCH3_CBUF_SIZE;
		break;
	case WCH_4:
		if (SRC_APP_AVM_WCH4 != pparam->eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH4, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
			return false;
		}
		base_addr = WCH_AVM_WCH4_BASE;
		y_buffer_size = WCH_AVM_WCH4_YBUF_SIZE;
		c_buffer_size = WCH_AVM_WCH4_CBUF_SIZE;
		break;
	case WCH_5:
		switch(pparam->eWchSrcId) {
			case SRC_APP_AVM_WCH5:
				base_addr = WCH_AVM_WCH5_BASE;
				y_buffer_size = WCH_AVM_WCH5_YBUF_SIZE;
				c_buffer_size = WCH_AVM_WCH5_CBUF_SIZE;
				break;

			case SRC_APP_BACKCAR_WCH5:
				base_addr = WCH_BACKCAR_BASE;
				y_buffer_size = WCH_BACKCAR_YBUF_SIZE;
				c_buffer_size = WCH_BACKCAR_CBUF_SIZE;
				break;

			case SRC_APP_AVIN_WCH5:
				base_addr = WCH_AVIN_BASE;
				y_buffer_size = WCH_AVIN_YBUF_SIZE;
				c_buffer_size = WCH_AVIN_CBUF_SIZE;
				break;

			default:
				WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH5, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
				return false;
		}
		break;

	case WCH_6:
			if (SRC_APP_YPBPR != pparam->eWchSrcId) {
				WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH6, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
				return false;
			}
			base_addr = WCH_YPBPR_BASE;
			y_buffer_size = WCH_YPBPR_YBUF_SIZE;
			c_buffer_size = WCH_YPBPR_CBUF_SIZE;
		break;

	case WCH_7:
			if (SRC_APP_HDMI != pparam->eWchSrcId) {
				WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH6, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
				return false;
			}
			base_addr = WCH_HDMI_BASE;
			y_buffer_size = WCH_HDMI_YBUF_SIZE;
			c_buffer_size = WCH_HDMI_CBUF_SIZE;
		break;
	case WCH_8:
		switch(pparam->eWchSrcId) {
			case SRC_APP_DGI1120:
				base_addr = WCH_BT1120_BASE;
				y_buffer_size = WCH_BT1120_YBUF_SIZE;
				c_buffer_size = WCH_BT1120_CBUF_SIZE;
				break;

			case SRC_APP_DGI656_WCH8:
				base_addr = WCH_BT656_BASE;
				y_buffer_size = WCH_BT656_YBUF_SIZE;
				c_buffer_size = WCH_BT656_CBUF_SIZE;
				break;

			case SRC_APP_DGI601_WCH8:
				base_addr = WCH_BT601_BASE;
				y_buffer_size = WCH_BT601_YBUF_SIZE;
				c_buffer_size = WCH_BT601_CBUF_SIZE;
				break;

			default :
				WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH8, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
				return false;
		}
		break;

	case WCH_9:
		if (SRC_APP_VDO != pparam->eWchSrcId) {
			WCH_LOG(WCH_LOG_LVL_ERR, "SRC_APP Error for WCH6, pparam->eWchSrcId: %d \n", pparam->eWchSrcId);
			return false;
		}
		base_addr = WCH_VDO_BASE;
		y_buffer_size = WCH_VDO_YBUF_SIZE;
		c_buffer_size = WCH_VDO_CBUF_SIZE;
		break;

	default :
		return false;

	}
#if WCH_DUMP_FIRST_20_FRAMES_BY_ICE
	base_addr = wch_reserve;
#endif

#ifdef __ARM2__
	WCH_LOG(WCH_LOG_LVL_INFO, "wchAllocBuf: u1WchId: %d, eWchSrcId: %d, base_addr: 0x%x, y_buffer_size: %d, c_buffer_size: %d\n", 
		u1WchId, pparam->eWchSrcId, (unsigned int)base_addr, (unsigned int)y_buffer_size, (unsigned int)c_buffer_size);
#else
	WCH_LOG(WCH_LOG_LVL_INFO, "wchAllocBuf: u1WchId: %d, eWchSrcId: %d, base_addr: %lx, y_buffer_size: %ld, c_buffer_size: %ld\n", 
		u1WchId, pparam->eWchSrcId, base_addr, y_buffer_size, c_buffer_size);
#endif

	for (i = 0; i < WCH_BUF_MAX_CNT; i++)
	{
		pbuffer->u4YBuf[i] = base_addr + (y_buffer_size + c_buffer_size) * i;
		pbuffer->u4CBuf[i] = pbuffer->u4YBuf[i] + y_buffer_size;

#ifdef __ARM2__
		memset(ARM1PHY2ARM2UCV(pbuffer->u4YBuf[i] + 0x20000000), 0, y_buffer_size);
		CacheFlush(ARM1PHY2ARM2UCV(pbuffer->u4YBuf[i] + 0x20000000), y_buffer_size);
		memset(ARM1PHY2ARM2UCV(pbuffer->u4CBuf[i] + 0x20000000), 0x80, c_buffer_size);
		CacheFlush(ARM1PHY2ARM2UCV(pbuffer->u4CBuf[i] + 0x20000000), c_buffer_size);
#endif

	}
	pbuffer->u4BufCnt = WCH_BUF_MAX_CNT;
#if WCH_SYNC_BUFFER
	pbuffer->u4BufFlag = 0;
	pbuffer->fgUseBackupBuffer = false;
#endif
	pparam->u4BufIdx = 0;

	return ret;
}

unsigned int OpenWch(unsigned char u1wchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	WchLockInit();
#if WCH_SUPPORT_ADAS_ONLY
	if ((u1wchId < WCH_1) || (u1wchId > WCH_5))
#else
	if ((u1wchId < WCH_1) || (u1wchId > WCH_9))
#endif
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", (int)u1wchId);
		return WCH_WRONG_ID;
	}
#ifdef __ARM2__
        wch_reserve = READ64(ARM1PHY2ARM2UCV(WCH_RSV_MEM_START_ADDR_4ARM2));
#endif

	switch (eWchSrcId) {
		case SRC_APP_BACKCAR_WCH1:
		case SRC_APP_AVM_WCH1:
		case SRC_APP_AVIN_WCH1:
		case SRC_APP_DGI656_WCH1:
		case SRC_APP_DGI601_WCH1:
			if (u1wchId != WCH_1){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD0 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD0 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;

		case SRC_APP_BACKCAR_WCH5:
		case SRC_APP_AVM_WCH5:
		case SRC_APP_AVIN_WCH5:
			if (u1wchId != WCH_5){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD0 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD0 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
#ifdef __ARM2__
			v_enable_bim_irq(WCH5_IRQ);
#endif
			break;
		case SRC_APP_AVM_WCH2:
			if (u1wchId != WCH_2){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD1 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD1 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		case SRC_APP_AVM_WCH3:
			if (u1wchId != WCH_3){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD3 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD3 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		case SRC_APP_AVM_WCH4:
			if (u1wchId != WCH_4){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD3 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]TVD3 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		case SRC_APP_YPBPR:
		case SRC_APP_VGA:
			if (u1wchId != WCH_6) {
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]YPBPR/VGA can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]YPBPR/VGA have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		case SRC_APP_HDMI:
			if (u1wchId != WCH_7) {
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]HDMI can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]HDMI have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		case SRC_APP_DGI656_WCH8:
		case SRC_APP_DGI601_WCH8:
		case SRC_APP_DGI1120:
			if (u1wchId != WCH_8){
				WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]BT656/BT601/BT1120 can not use u1WchId=%d \n", u1wchId);
				return WCH_WRONG_ID;
			} else {
				if (_gWchParam[u1wchId].u4Status != WCH_HW_FREE && _gWchParam[u1wchId].u4Status != WCH_HW_STOP){
					WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]BT656/BT601/BT1120 have no free(stop) channel to use u1WchId=%d \n", u1wchId);
					return WCH_NO_FREE_HW;
				}
			}
			break;
		default :
			WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]no such eWchSrcId=%d \n", eWchSrcId);
			return WCH_INVALID_APP_ID;
		}

	/*coming here means having free or stoped channel.*/
	if (_gWchParam[u1wchId].eWchSrcId != SRC_APP_UNKNOWN && _gWchParam[u1wchId].eWchSrcId != eWchSrcId) {
		WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]u1wchId=%d stoped by eWchSrcId=%d \n", u1wchId, eWchSrcId);
		return WCH_STOP_BY_OTHER;
	}

	SpinLockWch();
	_gWchParam[u1wchId].eWchSrcId = eWchSrcId;

	if (_gWchParam[u1wchId].u4Status == WCH_HW_FREE) {
		if (!wchAllocBuf(u1wchId)){
			WCH_LOG(WCH_LOG_LVL_ERR,"[OpenWch]allocate buffer fail for u1wchId=%d \n", u1wchId);
			SpinUnlockWch();
			return WCH_ALLOC_BUF_FAIL;
		}
	}

	SpinUnlockWch();

	WCH_LOG(WCH_LOG_LVL_INFO,"[OpenWch]eWchSrcId=%d try to use u1WchId=%d \n", eWchSrcId, u1wchId);

	return WCH_SUCCESS;
}
EXPORT_SYMBOL(OpenWch);

WCH_SRC_APP_ID_E getWhichSrcUseWch(unsigned char u1wchId)
{
	return _gWchParam[u1wchId].eWchSrcId;
}
EXPORT_SYMBOL(getWhichSrcUseWch);

bool wchHwConfig(unsigned char u1WchId, PWCH_CFG_T pWchCfg)
{
	WCH_INPUT_INFO_T rInputInfo;
	WCH_OUTPUT_INFO_T  rOutputInfo;

	memset(&rInputInfo, 0, sizeof(rInputInfo));
	memset(&rOutputInfo, 0, sizeof(rOutputInfo));

	wchHalInit(u1WchId);

	wchHalTopInit(u1WchId, pWchCfg->eInputSrc);//merge app type and data type?

	if (DATA_SRC_HDMI == pWchCfg->eInputSrc) {
		if (WchGetRetimingInfo(pWchCfg->eTiming)) {
			rInputInfo.u4SrcWidth		= prWchRetiming->u2HActive;
			rInputInfo.u4SrcHeight		= prWchRetiming->u2VActive;
			rInputInfo.u4StartX 		= prWchRetiming->u2HPixel;
			rInputInfo.u4StartYTop		= prWchRetiming->u2VTopLine;
			rInputInfo.u4StartYBot		= prWchRetiming->u2VBotLine;

			WCH_LOG(WCH_LOG_LVL_DBG,
				 "WchConfigHw use write channel re-timing for HDMI id=%d\r\n",
				 (int)u1WchId);
		} else{
			WCH_LOG(WCH_LOG_LVL_ERR,
				 "WchConfigHw use write channel re-timing for HDMI error id=%d\r\n",
				 (int)u1WchId);
			return false;
		}

	}  else {

		rInputInfo.u4SrcWidth 		= pWchCfg->u4SrcWidth;
		rInputInfo.u4SrcHeight 		= pWchCfg->u4SrcHeight;
		rInputInfo.u4StartX 		= pWchCfg->u4SrcStartX;
		rInputInfo.u4StartYTop 		= pWchCfg->u4SrcStartYTop;
		rInputInfo.u4StartYBot 		= pWchCfg->u4SrcStartYBot;
	}

	rInputInfo.u1WchId			= u1WchId;
	rInputInfo.eSrcType 		= pWchCfg->eInputSrc;
	rInputInfo.eSrcFmt			= pWchCfg->eInputFmt;
	rInputInfo.eWchSrcId		= pWchCfg->eSrcId;
	rInputInfo.fgProgressive	= pWchCfg->fgProgressive;
	rInputInfo.u1YSel			= pWchCfg->u1YSel;
	rInputInfo.u1USel			= pWchCfg->u1USel;
	rInputInfo.u1VSel			= pWchCfg->u1VSel;
	rInputInfo.u1CInDelay		= pWchCfg->u1CInDelay;
#if WCH_SUPPORT_AVM_480P
	rInputInfo.fgSupportAVM480P		= pWchCfg->fgSupportAVM480P;
#endif

	wchHalSetInput(&rInputInfo);

	rOutputInfo.u1WchId 		= u1WchId;
	rOutputInfo.eDstFmt			= pWchCfg->eOutputFmt;
	rOutputInfo.u4ScanLineMode 	= pWchCfg->u4ScanLineMode;
	rOutputInfo.fgProgressive 	= pWchCfg->fgProgressive;
	rOutputInfo.u4DstWidth 		= pWchCfg->u4DstWidth;
	rOutputInfo.u4DstHeight 	= pWchCfg->u4DstHeight;

	wchHalSetOutput(&rOutputInfo);

	return true;
}

unsigned int ConfigWch(PWCH_CFG_T pWchCtlParam)
{
	unsigned char u1wchId = pWchCtlParam->u1WchId;

#if WCH_SUPPORT_ADAS_ONLY
	if ((u1wchId < WCH_1) || (u1wchId > WCH_5))
#else
	if ((u1wchId < WCH_1) || (u1wchId > WCH_9))
#endif
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", (int)u1wchId);
		return WCH_WRONG_ID;
	}

	if (sizeof(*pWchCtlParam) < sizeof(WCH_CFG_T)) {
			WCH_LOG(WCH_LOG_LVL_ERR,"ConfigWch sizeof(pWchCtlParam) error! \n");
			return WCH_INVALID_INPUT_PARAM;
	}

	if (_gWchParam[u1wchId].u4Status != WCH_HW_START) {

		SpinLockWch();
		memcpy(&_gWchParam[u1wchId].tWchCfg, pWchCtlParam, sizeof(WCH_CFG_T));
		SpinUnlockWch();
#ifdef __ARM2__
		/*request IRQ*/
		WchIsrInit(u1wchId);
#endif
		if (wchHwConfig(u1wchId, &_gWchParam[u1wchId].tWchCfg)) {
			SpinLockWch();
			_gWchParam[u1wchId].u4Status = WCH_HW_CONFIG;
			SpinUnlockWch();
			WCH_LOG(WCH_LOG_LVL_INFO,"ConfigWch->wchHwConfig success(eWchSrcId:%d--wch%d)! \n",
				_gWchParam[u1wchId].eWchSrcId, u1wchId);
		} else {
			WCH_LOG(WCH_LOG_LVL_ERR,"ConfigWch->wchHwConfig error(eWchSrcId:%d--wch%d)! \n",
				_gWchParam[u1wchId].eWchSrcId, u1wchId);
			return WCH_CONFIG_FAIL;
		}
		wchSwitchBuffer(u1wchId);
	} else {
		WCH_LOG(WCH_LOG_LVL_ERR,"ConfigWch error,because is starting(eWchSrcId:%d--wch%d)! \n",
				_gWchParam[u1wchId].eWchSrcId, u1wchId);
		return WCH_CONFIG_FAIL;
	}

	return WCH_SUCCESS;
}
EXPORT_SYMBOL(ConfigWch);

unsigned int StartWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	unsigned int ret = WCH_SUCCESS;
	WCH_SRC_APP_ID_E eSrcId = SRC_APP_UNKNOWN;

#if WCH_SUPPORT_ADAS_ONLY
	if ((u1WchId < WCH_1) || (u1WchId > WCH_5))
#else
	if ((u1WchId < WCH_1) || (u1WchId > WCH_9))
#endif
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", (int)u1WchId);
		return WCH_WRONG_ID;
	}

	eSrcId = _gWchParam[u1WchId].eWchSrcId;

	WCH_LOG(WCH_LOG_LVL_INFO, "[StartWch]u1WchId=%d,eWchSrcId=%d\n", u1WchId, eWchSrcId);
#if WCH_DUMP_BUFFER_ATTR
	u4DumpFrameCnt[u1WchId] = 0;
	u4DumpBufIdxCnt[u1WchId] = 0;
#endif
#ifdef __ARM2__  //for check interrupt interval
	_g_start = 0;
#else
	_g_start_t[u1WchId].tv_usec = 0;
	_g_start_t[u1WchId].tv_sec = 0;
#endif

	u4SkipVysncCnt[u1WchId] = WCH_SKIP_BUFF_CNT;
	if (eSrcId == eWchSrcId && (_gWchParam[u1WchId].u4Status == WCH_HW_CONFIG ||
								_gWchParam[u1WchId].u4Status == WCH_HW_START)) {
		if (_gWchParam[u1WchId].u4Status == WCH_HW_CONFIG) {
			wchHalStart(u1WchId);
			if (0 == WCH_SKIP_BUFF_CNT) {
			/*Before opening WCH shadow, we have switched a buffer to WCH. The first Buffer will be write immediately after WCH be enabled.
			   But after opening WCH shadow, the buffer being switched to WCH will be write in next interrupt.
			   So we need to switch a buffer to WCH before writing the first valid frame to buffer*/
				wchSwitchBuffer(u1WchId);
			}
		}
		_gWchParam[u1WchId].u4Status = WCH_HW_START;
#if WCH_SYNC_BUFFER
		_gWchParam[u1WchId].tWchBuf.u4BufFlag = 0;
		_gWchParam[u1WchId].tWchBuf.fgUseBackupBuffer = false;
#endif
		WCH_LOG(WCH_LOG_LVL_INFO,"StartWch start success(eWchSrcId:%d--wch%d)! \n", eWchSrcId, u1WchId);
	} else {
		WCH_LOG(WCH_LOG_LVL_ERR,"StartWch start wrong info(eWchSrcId:%d--wch%d--status:%d)! \n",
			eWchSrcId, u1WchId, _gWchParam[u1WchId].u4Status);
		return WCH_START_FAIL;
	}

	return ret;
}
EXPORT_SYMBOL(StartWch);

unsigned int StopWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	unsigned int ret = WCH_SUCCESS;
	WCH_SRC_APP_ID_E eSrcId = SRC_APP_UNKNOWN;

#if WCH_SUPPORT_ADAS_ONLY
	if ((u1WchId < WCH_1) || (u1WchId > WCH_5))
#else
	if ((u1WchId < WCH_1) || (u1WchId > WCH_9))
#endif
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", (int)u1WchId);
		return WCH_WRONG_ID;
	}

	eSrcId = _gWchParam[u1WchId].eWchSrcId;

	WCH_LOG(WCH_LOG_LVL_INFO, "[StopWch]u1WchId=%d,eWchSrcId=%d\n", u1WchId, eWchSrcId);

#if WCH_DUMP_BUFFER_ATTR
	u4DumpFrameCnt[u1WchId] = 0;
	u4DumpBufIdxCnt[u1WchId] = 0;
#endif

	if (eSrcId == eWchSrcId && (_gWchParam[u1WchId].u4Status == WCH_HW_START ||
								_gWchParam[u1WchId].u4Status == WCH_HW_STOP)) {
		wchHalStop(u1WchId);
		_gWchParam[u1WchId].u4Status = WCH_HW_STOP;
		WCH_LOG(WCH_LOG_LVL_INFO,"StopWch stop success(eWchSrcId:%d--wch%d)! \n", eWchSrcId, u1WchId);
	} else {
		WCH_LOG(WCH_LOG_LVL_ERR,"StopWch stop wrong info(eWchSrcId:%d--wch%d--status:%d)! \n",
			eWchSrcId, u1WchId, _gWchParam[u1WchId].u4Status);
		return WCH_STOP_FAIL;
	}


	return ret;
}
EXPORT_SYMBOL(StopWch);

unsigned int CloseWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId)
{
	unsigned int ret = WCH_SUCCESS;
	WCH_SRC_APP_ID_E eSrcId = SRC_APP_UNKNOWN;

#if WCH_SUPPORT_ADAS_ONLY
	if ((u1WchId < WCH_1) || (u1WchId > WCH_5))
#else
	if ((u1WchId < WCH_1) || (u1WchId > WCH_9))
#endif
	{
		WCH_LOG(WCH_LOG_LVL_ERR, "Param Error, u1WchId: %d \n", (int)u1WchId);
		return WCH_WRONG_ID;
	}

	eSrcId = _gWchParam[u1WchId].eWchSrcId;

	WCH_LOG(WCH_LOG_LVL_INFO, "[CloseWch] u1WchId=%d,eWchSrcId=%d\n", u1WchId, eWchSrcId);

#ifdef __ARM2__
	v_disable_bim_irq(WCH5_IRQ);
#endif
	SpinLockWch();
	if (eSrcId == eWchSrcId) {
#ifndef __ARM2__
		spin_lock_irqsave(&wchParamLock[u1WchId], fgWchParam[u1WchId]);
		WCH_LOG(WCH_LOG_LVL_INFO, " CloseWch! spin lock u1WchId: %d\r\n", (int)u1WchId);
#endif
		if (_gWchParam[u1WchId].u4Status == WCH_HW_START) {
			wchHalStop(u1WchId);
		}
		wchHalDeInit(u1WchId);
		memset(&_gWchParam[u1WchId], 0, sizeof(WCH_IF_PARAM_T));
		_gWchParam[u1WchId].u4Status = WCH_HW_FREE;
#ifndef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO, " CloseWch! spin unlock u1WchId: %d\r\n", (int)u1WchId);
		spin_unlock_irqrestore(&wchParamLock[u1WchId], fgWchParam[u1WchId]);
#endif
	} else {
		WCH_LOG(WCH_LOG_LVL_ERR,"CloseWch close wrong info(eWchSrcId:%d--wch%d--status:%d)! \n",
			eWchSrcId, u1WchId, _gWchParam[u1WchId].u4Status);
		SpinUnlockWch();
		return WCH_CLOSE_FAIL;
	}
	SpinUnlockWch();

	return ret;

}
EXPORT_SYMBOL(CloseWch);

unsigned int WchGetBufferAddress(PWCH_BUF_T pWchGetBuf)
{

	unsigned char u1wchId = pWchGetBuf->u1WchId;
	WCH_BUFF_INFO_T wchbuffer;
	int bufcnt = 0;
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchGetBufferAddress]u1WchId=%d \n", u1wchId);

	memset(&wchbuffer, 0, sizeof(WCH_BUFF_INFO_T));

	wchbuffer.u4BufCnt = _gWchParam[u1wchId].tWchBuf.u4BufCnt;
	for (; bufcnt < wchbuffer.u4BufCnt; bufcnt++) {
		wchbuffer.u4YBuf[bufcnt] = _gWchParam[u1wchId].tWchBuf.u4YBuf[bufcnt];
		wchbuffer.u4CBuf[bufcnt] = _gWchParam[u1wchId].tWchBuf.u4CBuf[bufcnt];
#ifdef __ARM2__
		WCH_LOG(WCH_LOG_LVL_INFO, "[WchGetBufferAddress]u8YBuf[%d]: 0x%x, u8CBuf[%d]: 0x%x\n",
			bufcnt, (unsigned int)wchbuffer.u4YBuf[bufcnt], bufcnt, (unsigned int)wchbuffer.u4CBuf[bufcnt]);
#else
		WCH_LOG(WCH_LOG_LVL_INFO, "[WchGetBufferAddress]u8YBuf[%d]: %lx, u8CBuf[%d]: %lx\n",
			bufcnt, wchbuffer.u4YBuf[bufcnt], bufcnt, wchbuffer.u4CBuf[bufcnt]);
#endif
	}

	memcpy(&pWchGetBuf->tWchBuf, &wchbuffer, sizeof(WCH_BUFF_INFO_T));

	return WCH_SUCCESS;
}
EXPORT_SYMBOL(WchGetBufferAddress);

#if WCH_SYNC_BUFFER
unsigned int WchReturnBuffer(unsigned char u1WchId, unsigned int u4BufIdx)
{
	if (u1WchId > WCH_9) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchReturnBuffer Param Error, u1WchId: %d \n", u1WchId);
		return WCH_WRONG_ID;
	}
	if (u4BufIdx >= _gWchParam[u1WchId].tWchBuf.u4BufCnt) {
		WCH_LOG(WCH_LOG_LVL_ERR, "WchReturnBuffer Param Error, u4BufIdx: %d \n", u4BufIdx);
		return WCH_INVALID_INPUT_PARAM;
	}
	spin_lock_irqsave(&wchParamLock[u1WchId], fgWchParam[u1WchId]);

	_gWchParam[u1WchId].tWchBuf.u4BufFlag &= ~((unsigned int)1 << u4BufIdx);
	spin_unlock_irqrestore(&wchParamLock[u1WchId], fgWchParam[u1WchId]);

	return WCH_SUCCESS;
}
EXPORT_SYMBOL(WchReturnBuffer);
#endif

#ifndef __ARM2__
void WchSuspend(void)
{
	int i = 0;

	for (i = WCH_1; i < WCH_NUM; i++) {
		spin_lock_irqsave(&wchParamLock[i], fgWchParam[i]);
		WCH_LOG(WCH_LOG_LVL_INFO, "[WchSuspend]wch%d status: %d \n", i+1, _gWchParam[i].u4Status);
		if (_gWchParam[i].u4Status == WCH_HW_START) {
			wchHalStop(i);
			wchHalDeInit(i);
		}
		memset(&_gWchParam[i], 0, sizeof(WCH_IF_PARAM_T));
		_gWchParam[i].u4Status = WCH_HW_FREE;
		spin_unlock_irqrestore(&wchParamLock[i], fgWchParam[i]);
	}
}

void WchResume(void)
{
}
#endif

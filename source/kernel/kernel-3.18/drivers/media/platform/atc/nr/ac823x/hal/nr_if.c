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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-03-10
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
#endif

#include "nr_log.h"
#include "nr_drv.h"
#include "nr_hal.h"
#include "wch_drv.h"
#include "wch_if.h"

#ifdef __ARM2__
unsigned int _u4NR_DBG_LVL = NR_LOG_LVL_HAL;
#else
unsigned int _u4NR_DBG_LVL = NR_LOG_LVL_DBG;
#endif
unsigned char *_pcNrLogLevel[] = {
	"[NR] [ERR]",
	"[NR] [WARN]",
	"[NR] [INFO]",
	"[NR] [HAL]",
	"[NR] [DBG]",
	"[NR] [IRQ]",
};

static unsigned long nrResMemPa = 0x11e9d0000;/* init for arm2 */
static unsigned long nrResMemVa;
static unsigned long nrResMemSize;
static WCH_BUF_T gWchBuffer;
static NR_PRM_T gNrParam;
static NR_BUFFER_T gNrBuffer;
static NR_ALLC_BUF_T gNrAlloBuf;
//static unsigned int gNrBufferindx = 0;

#ifdef __ARM2__
#define spin_lock_init(lock)
#define spin_lock_irqsave(lock, flags)
#define spin_unlock_irqrestore(lock, flags)
#endif
bool fgWchLockInit = false;
static nrLock nrlock;

void NrLockInit(void)
{
	if (!fgWchLockInit) {
		spin_lock_init(&nrlock.lock);
		fgWchLockInit = true;
		NR_LOG(NR_LOG_LVL_DBG, "NrLockInit %p \n", &nrlock.lock);
	}
}

void SpinLockNr(void)
{
	spin_lock_irqsave(&nrlock.lock, nrlock.flags);
	nrlock.u4LockCnt++;
	NR_LOG(NR_LOG_LVL_IRQ, "[SpinLockNr] u4LockCnt=%d \n", (int)nrlock.u4LockCnt);
}

void SpinUnlockNr(void)
{
	nrlock.u4LockCnt--;
	spin_unlock_irqrestore(&nrlock.lock, nrlock.flags);
	NR_LOG(NR_LOG_LVL_IRQ, "[SpinUnlockNr] u4LockCnt=%d \n", (int)nrlock.u4LockCnt);
}

#ifndef __ARM2__
struct task_struct *hNrInst = NULL;
static HANDLE_T _hNrQueueBufferMsgQ = NULL;
static HANDLE_T _hNrISRMsgQ = NULL;
#define MSG_SIZE sizeof(unsigned int)
#define MSG_CNT 10
#define MSG_QUEUE_BUFFER_NAME "nr queue buffer msgQ"
#define MSG_ISR_NAME "nr ISR msgQ"
#define NR_KERNEL_THREAD_NAME "NR kernel thread"

void NrSwitchBuffer(unsigned int u4NrRdBufIdx, unsigned int u4NrWrBufIdx);
int NrUpdateAddressThread(void* data)
{
	int32_t i4Ret = 0;
	__u16 u2MsgIdx = 0;
	size_t z_msg_size = sizeof(uint32_t);
	unsigned int u4NrRdBufIdx = 0;
	unsigned int u4NrWrBufIdx = 0;
	NR_PRM_T * pNrParam = &gNrParam;

	NR_LOG(NR_LOG_LVL_INFO, "NrUpdateAddressThread! \n");

	for(;;){
		if (kthread_should_stop()){
			break;
		}

		if ((NULL == _hNrQueueBufferMsgQ) || (NULL == _hNrISRMsgQ)) {
			NR_LOG(NR_LOG_LVL_INFO, "NrUpdateAddressThread _hNrQueueBufferMsgQ:%x, _hNrISRMsgQ:%x! \n",
				_hNrQueueBufferMsgQ, _hNrISRMsgQ);
			break;
		}
		//wait wch buffer done
		i4Ret = x_msg_q_receive(&u2MsgIdx, &u4NrRdBufIdx, &z_msg_size, &_hNrQueueBufferMsgQ,
			1, X_MSGQ_OPTION_WAIT);
		//wait nr isr done
		i4Ret = x_msg_q_receive(&u2MsgIdx, &u4NrWrBufIdx, &z_msg_size, &_hNrISRMsgQ,
			1, X_MSGQ_OPTION_WAIT);
		//update nr buffer
		NrSwitchBuffer(u4NrRdBufIdx, u4NrWrBufIdx);

		SpinLockNr();
		if (pNrParam->GetNrBufIndx)
			pNrParam->GetNrBufIndx(&u4NrWrBufIdx);
		SpinUnlockNr();

	}
	complete_and_exit(NULL, 0);

	return 0;
}

static unsigned int NRdate = 168;
void NrKernelThreadInit(void)
{
	NR_LOG(NR_LOG_LVL_INFO, "NrKernelThreadInit begin! \n");

	//queue buffer done
	if (OSR_OK != x_msg_q_create(&_hNrQueueBufferMsgQ, MSG_QUEUE_BUFFER_NAME, MSG_SIZE, MSG_CNT)) {
		NR_LOG(NR_LOG_LVL_ERR, "_hNrQueueBufferMsgQ	x_msg_q_create fail!\r\n");
		return;
	}
	NR_LOG(NR_LOG_LVL_INFO, "  NrKernelThreadInit Enter _hNrQueueBufferMsgQ: %p, *(&_hNrQueueBufferMsgQ): %p!\r\n", 
		_hNrQueueBufferMsgQ, *(&_hNrQueueBufferMsgQ));

	//ISR done
	if (OSR_OK != x_msg_q_create(&_hNrISRMsgQ, MSG_ISR_NAME, MSG_SIZE, MSG_CNT)) {
		NR_LOG(NR_LOG_LVL_ERR, "_hNrISRMsgQ	x_msg_q_create fail!\r\n");
		return;
	}
	NR_LOG(NR_LOG_LVL_INFO, "  NrKernelThreadInit Enter _hNrISRMsgQ: %p, *(&_hNrISRMsgQ): %p!\r\n", 
		_hNrISRMsgQ, *(&_hNrISRMsgQ));

	//kernel thread
	hNrInst = kthread_create(NrUpdateAddressThread, (void*)&NRdate, NR_KERNEL_THREAD_NAME);
	if (!IS_ERR(hNrInst)) {
		NR_LOG(NR_LOG_LVL_INFO, "NrKernelThreadInit create thread success = 0x%p \n", hNrInst);
		wake_up_process(hNrInst);
	} else{
		NR_LOG(NR_LOG_LVL_ERR, " NrKernelThreadInit kthread_create fail!\r\n");
		hNrInst = NULL;
	}
	NR_LOG(NR_LOG_LVL_INFO, "NrKernelThreadInit end! \n");

}
#endif

void setNrReservemem(unsigned long pa, unsigned long va, unsigned int size)
{
	nrResMemPa = pa;
	nrResMemVa = va;
	nrResMemSize = size;

	NR_LOG(NR_LOG_LVL_INFO, "setNrReservemem nrResMemPa:%x, nrResMemSize:%x, nrResMemVa:%x\n",
							nrResMemPa, nrResMemSize, nrResMemVa);
}

int NrAllocateBuff(unsigned int u4FrameWidth, unsigned int u4FrameHeight)
{
	int nrcnt = 0;
	PNR_ALLC_BUF_T pNrAlloBuf = &gNrAlloBuf;

	if (u4FrameWidth == 0 || u4FrameHeight == 0) {
		NR_LOG(NR_LOG_LVL_INFO, "NrAllocateBuff error,width and height is 0 \n");
		return -1;
	}

	pNrAlloBuf->u4BufIdx = 0;
	for (; nrcnt < NR_BUF_MAX_CNT; nrcnt++) {
		pNrAlloBuf->u4YBuf[nrcnt] = nrResMemPa + nrcnt * u4FrameWidth * u4FrameHeight * 2;
		pNrAlloBuf->u4CBuf[nrcnt] = pNrAlloBuf->u4YBuf[nrcnt] + u4FrameWidth * u4FrameHeight;
	}

	NR_LOG(NR_LOG_LVL_INFO, "NR buffer start address: \n", pNrAlloBuf->u4YBuf[0]);
	NR_LOG(NR_LOG_LVL_INFO, "NR buffer  end  address: \n", pNrAlloBuf->u4CBuf[NR_BUF_MAX_CNT - 1]);
	return 0;
}

#ifdef __ARM2__
extern void v_clear_bim_irq(unsigned int u4Id);
#endif
extern void mt33xx_mask_ack_bim_irq(unsigned int virq);
irqreturn_t NrIsr(int u2Vector, void *dev_id)
{
	PNR_ALLC_BUF_T pNrAlloBuf = &gNrAlloBuf;

#ifndef __ARM2__
	if (NULL != _hNrISRMsgQ) {
		x_msg_q_send(_hNrISRMsgQ, &(pNrAlloBuf->u4BufIdx), sizeof(unsigned int), 1);
	}
#endif

	_NrClearInterrupteStatus();

#ifndef __ARM2__
	mt33xx_mask_ack_bim_irq(u2Vector);
#else
	v_clear_bim_irq(u2Vector);
#endif

	return IRQ_HANDLED;
}

unsigned int fgReqirq = 0;
void NrIsrInit(void)
{
	unsigned int IRQVECTOR = 104 + 13;
#ifndef __ARM2__
	if (fgReqirq == 0) {
		request_irq(IRQVECTOR, NrIsr, 0, "NR ISR", NULL);
		fgReqirq = 1;
	}
#else
	/*none*/
#endif
}

unsigned int _u4DramClk = 5;
void NrInit(void)
{
	NrLockInit();

	//Reset NR module
	_NrHwReset();

	//Set NR Clk
	_NrSetClk(_u4DramClk);

	//ParaInit & Set NR Para 
	_NrParaInit(); 
	_NrParaSet(1,3,3,3);

	NR_LOG(NR_LOG_LVL_INFO, "NR Init Done. \n");
}
EXPORT_SYMBOL(NrInit);

void NrDeInit(void)
{
	SpinLockNr();
	memset(&gNrParam, 0, sizeof(NR_PRM_T));
	SpinUnlockNr();
#if 0
	unsigned int IRQVECTOR = 104 + 13;
#ifndef __ARM2__
		if (fgReqirq == 1) {
			free_irq(IRQVECTOR, NULL);
			fgReqirq = 0;
		}
#endif
#endif
	_NrCloseClk();
}
EXPORT_SYMBOL(NrDeInit);

void NrSetParam(NR_PRM_T * ptNrPrm)
{
	NR_PRM_T * pNrParam = &gNrParam;
	PWCH_BUF_T pWchBuffer = &gWchBuffer;
	unsigned int u4WAlign = 16, u4HAlign = 32;
	unsigned int u4AlignWidth, u4AlignHeight;

	if (ptNrPrm == NULL) {
		NR_LOG(NR_LOG_LVL_ERR, "NrSetParam's parameter:ptNrPrm is NULL. \n");
		return;
	}
	memcpy(&gNrParam, ptNrPrm, sizeof(NR_PRM_T));

	//get wch buffer
	pWchBuffer->u1WchId = pNrParam->u1WchId;
	WchGetBufferAddress(&gWchBuffer);

	//align picture size
	u4AlignWidth = ((pNrParam->u4PicWidth + (u4WAlign - 1)) / u4WAlign ) * u4WAlign;
	u4AlignHeight =((pNrParam->u4PicHeight + (u4HAlign - 1)) / u4HAlign ) * u4HAlign;
	pNrParam->u4FrameWidth = u4AlignWidth;
	pNrParam->u4FrameHeight = u4AlignHeight;

	//allocate nr buffer
	NrAllocateBuff(pNrParam->u4FrameWidth, pNrParam->u4FrameHeight);

	_NrSetParam(pNrParam);
}
EXPORT_SYMBOL(NrSetParam);

void NrProcess(void)
{
	NR_BUFFER_T * ptNrBuf = &gNrBuffer;
	PWCH_BUF_T pWchBuffer = &gWchBuffer;
	PNR_ALLC_BUF_T pNrAlloBuf = &gNrAlloBuf;

	ptNrBuf->u4CurrRdYAddr = pWchBuffer->tWchBuf.u4YBuf[0];
	ptNrBuf->u4CurrRdCAddr = pWchBuffer->tWchBuf.u4CBuf[0];
	ptNrBuf->u4LastRdYAddr = pWchBuffer->tWchBuf.u4YBuf[0];
	ptNrBuf->u4LastRdCAddr = pWchBuffer->tWchBuf.u4CBuf[0];
	ptNrBuf->u4CurrWrYAddr = pNrAlloBuf->u4YBuf[0];
	ptNrBuf->u4CurrWrCAddr = pNrAlloBuf->u4CBuf[0];

	_NrSetBufTrigger(ptNrBuf);
	NR_LOG(NR_LOG_LVL_INFO, "NrProcess trigger begin \n");
}
EXPORT_SYMBOL(NrProcess);

void NrReciveWchBuffer(unsigned int u4NrWchIdx)
{
	if (NULL != _hNrQueueBufferMsgQ) {
		x_msg_q_send(_hNrQueueBufferMsgQ, &u4NrWchIdx, sizeof(unsigned int), 1);
	}
}
EXPORT_SYMBOL(NrReciveWchBuffer);

unsigned int NrGetBufferAddress(PNR_ALLC_BUF_T pNrGetBuf)
{
	PNR_ALLC_BUF_T pNrAlloBuf = &gNrAlloBuf;
	NR_ALLC_BUF_T nrbuffer;
	int bufcnt = 0;
	NR_LOG(NR_LOG_LVL_INFO, "[NrGetBufferAddress]enter \n");

	memset(&nrbuffer, 0, sizeof(NR_ALLC_BUF_T));

	nrbuffer.u4BufIdx = pNrAlloBuf->u4BufIdx;
	for (; bufcnt < NR_BUF_MAX_CNT; bufcnt++) {
		nrbuffer.u4YBuf[bufcnt] = pNrAlloBuf->u4YBuf[bufcnt];
		nrbuffer.u4CBuf[bufcnt] = pNrAlloBuf->u4CBuf[bufcnt];
		NR_LOG(NR_LOG_LVL_INFO, "[NrGetBufferAddress]u8YBuf[%d]: %lx, u8CBuf[%d]: %lx\n", 
			bufcnt, nrbuffer.u4YBuf[bufcnt], bufcnt, nrbuffer.u4CBuf[bufcnt]);
	}

	memcpy(pNrGetBuf, &nrbuffer, sizeof(NR_ALLC_BUF_T));

	return 0;
}
EXPORT_SYMBOL(NrGetBufferAddress);

void NrSwitchBuffer(unsigned int u4NrRdBufIdx, unsigned int u4NrWrBufIdx)
{
	NR_BUFFER_T * ptNrBuf = &gNrBuffer;
	PWCH_BUF_T pWchBuffer = &gWchBuffer;
	PNR_ALLC_BUF_T pNrAlloBuf = &gNrAlloBuf;
	unsigned int u4NrRdlastBufIdx = 0;

	if (u4NrRdBufIdx < 0 || u4NrRdBufIdx > WCH_BUF_MAX_CNT || u4NrWrBufIdx < 0 || u4NrWrBufIdx > NR_BUF_MAX_CNT) {
		NR_LOG(NR_LOG_LVL_ERR, "NrSwitchBuffer index err. \n");
		return;
	}

	if (u4NrRdBufIdx == 0) {
		u4NrRdlastBufIdx = WCH_BUF_MAX_CNT - 1;
	} else {
		u4NrRdlastBufIdx = u4NrRdBufIdx - 1;
	}
	NR_LOG(NR_LOG_LVL_DBG, "NrSwitchBuffer u4NrRdBufIdx:%d, u4NrRdlastBufIdx:%d, u4NrWrBufIdx:%d. \n",
		u4NrRdBufIdx, u4NrRdlastBufIdx, u4NrWrBufIdx);

	ptNrBuf->u4CurrRdYAddr = pWchBuffer->tWchBuf.u4YBuf[u4NrRdBufIdx];
	ptNrBuf->u4CurrRdCAddr = pWchBuffer->tWchBuf.u4CBuf[u4NrRdBufIdx];
	ptNrBuf->u4LastRdYAddr = pWchBuffer->tWchBuf.u4YBuf[u4NrRdlastBufIdx];
	ptNrBuf->u4LastRdCAddr = pWchBuffer->tWchBuf.u4CBuf[u4NrRdlastBufIdx];
	ptNrBuf->u4CurrWrYAddr = pNrAlloBuf->u4YBuf[u4NrWrBufIdx];
	ptNrBuf->u4CurrWrCAddr = pNrAlloBuf->u4CBuf[u4NrWrBufIdx];

	_NrSetBufTrigger(ptNrBuf);

	//update nr buffer index
	pNrAlloBuf->u4BufIdx++;
	if (pNrAlloBuf->u4BufIdx == NR_BUF_MAX_CNT){
		pNrAlloBuf->u4BufIdx = 0;
	}
}

int NrBypass(int fgBypass)
{
	if (fgBypass < 0) {
		NR_LOG(NR_LOG_LVL_ERR,"NrBypass fgBypass err:%d \n", fgBypass);
		return -1;
	}
	_NrBypass((unsigned int)fgBypass);

	return 0;
}

int NrEnable3dFunc(int fgBypass)
{
	if (fgBypass < 0) {
		NR_LOG(NR_LOG_LVL_ERR,"NrEnable3dFunc fgBypass err:%d \n", fgBypass);
		return -1;
	}
	_NrEnable3dFunc((unsigned int)fgBypass);

	return 0;
}

int NrSetSwapMode(int swapMode)
{
	if (swapMode < 0 || swapMode > 6){
		NR_LOG(NR_LOG_LVL_ERR,"NrSetSwapMode swapMode err:%d \n", swapMode);
		return -1;
	}
	_NrSetSwapMode((unsigned int)swapMode);

	return 0;
}

int NrSetLevel(unsigned int u4Strength,unsigned int u4FNRStrength,
					unsigned int u4MNRStrength,unsigned int u4BNRStrength)
{
	if (u4Strength < 0 || u4Strength > 3 ||
		u4FNRStrength < 0 || u4FNRStrength > 3 ||
		u4MNRStrength < 0 || u4MNRStrength > 3 ||
		u4BNRStrength < 0 || u4BNRStrength > 3){

		NR_LOG(NR_LOG_LVL_ERR,"NrSetSwapMode swapMode err! \n");
		return -1;
	}
	_NrParaSet(u4Strength, u4FNRStrength, u4MNRStrength, u4BNRStrength);
	_NrLevelSet(u4Strength, u4FNRStrength, u4MNRStrength, u4BNRStrength);

	return 0;
}


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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>


#include "outhw.h"
//#include <mach/ac83xx_memory.h>
#include "aud_pcm_dbg.h"
#include "winutil.h"
#include "GpsMix_mw.h"
#include "GpsMix_if.h"
#include "strmproc.h"
#include "pcm_ac83xx.h"
#include "drv_aud.h"

DspMixOut m_prDspMixOut;

uintptr_t g_hStrmProcMsgQ = 0;
void* g_hStrmProcEvent = NULL;
static void* g_hDspMixEvent;
static struct task_struct *g_hDspMixThread_task;
static struct task_struct *g_hStreamProcThread_task;
extern REAR_AOUT_TYPE g_eRearOut;

extern void DspCfgSetCommBufWptr(u32 u4WptrVal);


static s32 StreamProc_Thread(void *data)
{
    u32 u4Code = 0;
    u16 u2MsgIdx = 0;
    //StrmProc_MSG_T rStrmProcMsg = {0};
    u32 zMsgSize = sizeof(StrmProc_MSG_T);
    StrmProc_MSG_T* rStrmProcMsg = kzalloc(zMsgSize, GFP_KERNEL);
	//zMsgSize = 20;
    StreamProcess *prStrmProc = NULL;
	//u8 msg[20] = {0};
    while (true) {
START:
        u4Code = x_event_wait_for_objects(1, &g_hStrmProcEvent, false, INFINITE);

        if (u4Code != WAIT_OBJECT_0) {
            pr_err("[PCM ERR]StreamProc_Thread: Wait StreamProc Event Failed.\r\n");
            return NORESOURCE;
        }
        pr_debug("[PCM]StreamProc_Thread: Receive StreamProc Event\r\n");
        if (kthread_should_stop()) {
            pr_err("[PCM ERR]StreamProc_Thread: kthread_should_stop\r\n");
            break;
        }
        while (OSR_OK == x_msg_q_receive(&u2MsgIdx, rStrmProcMsg, &zMsgSize,
            &g_hStrmProcMsgQ, 1, X_MSGQ_OPTION_NOWAIT)) {
            pr_debug("[PCM]StreamProc_Thread: chip->m_u4StartCount = %d\r\n", (s32)snd_chip->m_u4StartCount);
            mutex_lock(&snd_chip->m_HeadLock);
            prStrmProc = snd_chip->m_prSPHead;
			
            while (prStrmProc && (prStrmProc->m_u4Idx != rStrmProcMsg->u4StrmProcID)) {
                prStrmProc = prStrmProc->m_prNext;
            }
			
            if (NULL == prStrmProc) {
                pr_err("[PCM ERR]StreamProc_Thread: prStrmProc is NULL!\r\n");
                mutex_unlock(&snd_chip->m_HeadLock);
                goto START;
            }
            if (STREAM_PROC_MSG_START == rStrmProcMsg->u4MsgID) {
                if (STATE_INITED == prStrmProc->m_u4OState) {
                    pr_debug("[PCM]StreamProc_Thread: ");
                    if (STATE_STOPPED == snd_chip->m_pbu4State) {
                        snd_chip->m_pbu4State = STATE_STARTED;
                        snd_chip->m_u4StartCount = 1;
                        DspMixOut_Start();
                        pr_debug("Start with chip->m_u4StartCount = %d\r\n", (s32)snd_chip->m_u4StartCount);
                    } else if (STATE_STARTED == snd_chip->m_pbu4State) {
                        snd_chip->m_u4StartCount++;
                        pr_debug("Started with chip->m_u4StartCount = %d\r\n", (s32)snd_chip->m_u4StartCount);
                    } else {
                        pr_debug("m_pbu4State = %d\r\n", (s32)snd_chip->m_pbu4State);
                    }

                    prStrmProc->m_u4OState = STATE_STARTED;
                    prStrmProc->m_u4OutputTotal = 0;
                    prStrmProc->m_u4UnderrunCount = 0;
                    pr_debug("[PCM]StreamProc_Thread: OState to started\r\n");
                }
                prStrmProc->m_u4BufState = BUF_STATE_NO_FILL;
            } else if ((STREAM_PROC_MSG_STOP == rStrmProcMsg->u4MsgID) &&(STATE_STARTED == prStrmProc->m_u4OState)) {
                prStrmProc->m_u4BufState = BUF_STATE_NO_FILL;
                prStrmProc->m_u4OState = STATE_STOPPED;
                pr_debug("[PCM]StreamProc_Thread: OState to stopped\r\n");
                if ((STATE_STARTED == snd_chip->m_pbu4State) && (snd_chip->m_u4StartCount)) {
                    ASSERT(snd_chip->m_u4StartCount);
                    snd_chip->m_u4StartCount--;
                    if (!snd_chip->m_u4StartCount) {
                        pr_debug("[PCM]StreamProc_Thread: Stop with");
                        pr_debug(" chip->m_u4StartCount = %d\r\n", (u32)snd_chip->m_u4StartCount);
                        DspMixOut_Stop();
                        snd_chip->m_pbu4State = STATE_STOPPED;
                    }
                }
            } else {
                pr_err("[PCM ERR]StreamProc_Thread: StreamProc MsgID(%d), start(%d).\r\n",
                rStrmProcMsg->u4MsgID, prStrmProc->m_u4OState);
            }
            mutex_unlock(&snd_chip->m_HeadLock);
        }
    }
	kfree(rStrmProcMsg);

    return NOERR;
}

static s32 DspMix_Thread(void *data)
{
	u32 u4Time = INFINITE;
	u32 code = 0;

	pr_debug("[PCM]DspMix_Thread: Start\r\n");
	while (true) {

		code = x_event_wait_for_objects(1, &g_hDspMixEvent, false, u4Time);

		if (kthread_should_stop()) {
			pr_err("[PCM ERR]DspMix_Thread: kthread_should_stop\r\n");
			break;
		}

		if (code == WAIT_OBJECT_0) {
			u4Time = DSPMIX_WAIT_TIMEOUT;
		} else if (code == (u32)WAIT_TIMEOUT) {
			u4Time = DspMixOut_InterruptThread();
			/*m_prDspMixOut.m_u4IntrNum++;*/
			if (m_prDspMixOut.m_fgHibernated) {
				u4Time = INFINITE;
			}
		} else {
			pr_err("[PCM ERR]DspMix_Thread: Error for WaitForMultipleObjects\r\n");
			break;
		}
	}
	pr_err("[PCM ERR]DspMix_Thread: DspMix_Thread End\r\n");

	return NOERR;
}

/*==================================//
		#define DspMixOut_BasicCtrl
//==================================*/

s32 DspMixOut_Init(void)
{
	s32 err = 0;

	m_prDspMixOut.m_rFmt.u4BW = DEF_DATA_BITS;
	m_prDspMixOut.m_rFmt.u4Chn = 2;
	m_prDspMixOut.m_rFmt.u4FS = 48000;
	m_prDspMixOut.m_u4CbTime = 20;
	m_prDspMixOut.m_pfnCb = NULL;
	m_prDspMixOut.m_u4CbParam = 0;
	m_prDspMixOut.m_fgHibernated = false;
	m_prDspMixOut.m_u4State = STATE_UNINIT;
	pr_debug("[PCM]DspMixOut_Init: m_u4State change to STATE_UNINIT.\r\n");

	if (STATE_UNINIT == m_prDspMixOut.m_u4State) {
		DspMixOut_initBuf();

		m_prDspMixOut.m_u4State = STATE_INITED;
		pr_debug("[PCM]DspMixOut_Init: m_u4State change to STATE_INITED.\r\n");
		m_prDspMixOut.m_u4WP = 0;
		m_prDspMixOut.m_u4RP = 0;

		g_hStrmProcEvent = x_event_create(NULL, false, false, TEXT("StreamProcEvent"));

		if (!g_hStrmProcEvent) {
			pr_err("[PCM ERR]DspMixOut_Init: Create g_hStrmProcEvent Failed\r\n");
			err = NORESOURCE;
			goto ERROR;
		}

		g_hDspMixEvent = x_event_create(NULL, false, false, TEXT("DspMixEvent"));

		if (!g_hDspMixEvent) {
			pr_err("[PCM ERR]DspMixOut_Init: Create g_hDspMixEvent Failed\r\n");
			err = NORESOURCE;
			goto ERROR;
		}

		g_hDspMixThread_task = kthread_create(DspMix_Thread, NULL, "DspMix_Thread");
		if (IS_ERR(g_hDspMixThread_task)) {
			pr_err("[PCM ERR]DspMixOut_Init: Create g_hDspMixThread_task ERR!\r\n");
			err = (s32)PTR_ERR(g_hDspMixThread_task);
			g_hDspMixThread_task = NULL;
			goto ERROR;
		}
		wake_up_process(g_hDspMixThread_task);

		/*g_hDspMixThread_task = kthread_create(DspMix_Thread, (void *)NULL, "DspMix_Thread");
		if (IS_ERR(g_hDspMixThread_task)) {
			LOG(LOG_CTRLF, TEXT("[vADSPTaskInit]vADSPTaskMain thread create fail \r\n"));
			g_hDspMixThread_task = NULL;
			return;
		}
		else
		{
		    struct sched_param param;
		    s32 ret;

		    param.sched_priority = to_sched_priority(AUD_DRV_THREAD_PRIORITY);
		    ret = sched_setscheduler_nocheck(g_hDspMixThread_task, SCHED_RR, &param);
		    ASSERT(ret == 0);
		}
		wake_up_process(g_hDspMixThread_task);

		g_hStreamProcThread_task = kthread_create(StreamProc_Thread, (void *)NULL, "StreamProc_Thread");
		if (IS_ERR(g_hStreamProcThread_task)) {
			LOG(LOG_CTRLF, TEXT("[vADSPTaskInit]vADSPTaskMain thread create fail \r\n"));
			g_hStreamProcThread_task = NULL;
			return;
		}
		else
		{
		    struct sched_param param;
		    s32 ret;

		    param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
		    ret = sched_setscheduler_nocheck(g_hStreamProcThread_task, SCHED_RR, &param);
		    ASSERT(ret == 0);
		}
		wake_up_process(g_hStreamProcThread_task);*/

		g_hStreamProcThread_task = kthread_create(StreamProc_Thread, NULL, "StreamProc_Thread");
		if (IS_ERR(g_hStreamProcThread_task)) {
			pr_err("[PCM ERR]DspMixOut_Init: Create g_hStreamProcThread_task ERR!\r\n");
			err = (s32)PTR_ERR(g_hStreamProcThread_task);
			g_hStreamProcThread_task = NULL;
			goto ERROR;
		}
		wake_up_process(g_hStreamProcThread_task);

		if (OSR_OK != x_msg_q_create(&g_hStrmProcMsgQ, TEXT("StreamProc_MSGQ"), sizeof(StrmProc_MSG_T), 40)) {
			pr_err("[PCM ERR]DspMixOut_Init: Failed to create StreamProc msg queue\r\n");
			goto ERROR;
		}

		pr_debug("[PCM]DspMixOut_Init: Success!\r\n");
	}

	return NOERR;

ERROR:
	if (g_hStrmProcEvent) {

		x_event_destroy(g_hStrmProcEvent);

		g_hStrmProcEvent = NULL;
	}

	if (g_hDspMixEvent) {

		x_event_destroy(g_hDspMixEvent);

		g_hDspMixEvent = NULL;
	}

	if (g_hDspMixThread_task) {
		kthread_stop(g_hDspMixThread_task);
		g_hDspMixThread_task = NULL;
		pr_err("[PCM ERR]DspMixOut_Init: g_hDspMixThread_task stop\r\n");
	}

	if (g_hStreamProcThread_task) {
		kthread_stop(g_hStreamProcThread_task);
		g_hStreamProcThread_task = NULL;
		pr_err("[PCM ERR]DspMixOut_Init: g_hStreamProcThread_task stop\r\n");
	}

	if (0 != g_hStrmProcMsgQ) {
		if (OSR_OK != x_msg_q_delete(g_hStrmProcMsgQ)) {
			pr_err("[PCM ERR]DspMixOut_Init: Failed to delete StreamProc msg queue\r\n");
		}
		g_hStrmProcMsgQ = 0;
	}

	return err;
}

u32 DspMixOut_UnInit(void)
{
	m_prDspMixOut.m_u4CbTime = 20;
	m_prDspMixOut.m_pfnCb = NULL;
	m_prDspMixOut.m_u4CbParam = 0;

	if ((STATE_INITED == m_prDspMixOut.m_u4State) || (STATE_STOPPED == m_prDspMixOut.m_u4State)) {
		m_prDspMixOut.m_u4State = STATE_UNINIT;
		pr_err("[PCM ERR]DspMixOut_UnInit: m_u4State change to STATE_UNINIT.\r\n");
	}

	if (g_hStrmProcEvent) {

		x_event_destroy(g_hStrmProcEvent);

		g_hStrmProcEvent = NULL;
	}

	if (g_hDspMixEvent) {

		x_event_destroy(g_hDspMixEvent);

		g_hDspMixEvent = NULL;
	}

	if (g_hDspMixThread_task) {
		kthread_stop(g_hDspMixThread_task);
		g_hDspMixThread_task = NULL;
		pr_debug("[PCM]DspMixOut_UnInit: g_hDspMixThread_task stop\r\n");
	}

	if (g_hStreamProcThread_task) {
		kthread_stop(g_hStreamProcThread_task);
		g_hStreamProcThread_task = NULL;
		pr_err("[PCM ERR]DspMixOut_UnInit: g_hStreamProcThread_task stop\r\n");
	}

	if (0 != g_hStrmProcMsgQ) {
		if (OSR_OK != x_msg_q_delete(g_hStrmProcMsgQ)) {
			pr_err("[PCM ERR]DspMixOut_UnInit: Failed to delete StreamProc msg queue\r\n");
		}
		g_hStrmProcMsgQ = 0;
	}

	return NOERR;
}

void DspMixOut_Setup(void)
{
	if ((STATE_INITED == m_prDspMixOut.m_u4State) || (STATE_STOPPED == m_prDspMixOut.m_u4State)) {
		pr_debug("[PCM]DspMixOut_Setup: Start\r\n");
		DspMixOut_setBuffer();
		DspMixOut_initOutputCh();

		m_prDspMixOut.m_u4WP = 0;
		m_prDspMixOut.m_u4RP = 0;
		m_prDspMixOut.m_u4State = STATE_STOPPED;

		pr_debug("[PCM]DspMixOut_Setup: Success!\r\n");
	}
}

s32 DspMixOut_Start(void)
{
	u32 code = 0;

	if ((STATE_STOPPED == m_prDspMixOut.m_u4State) && (!m_prDspMixOut.m_fgHibernated)) {
		AudGpsMix_CmdStart();

		code = x_event_wait_for_objects(1, &m_hGpsMixStartEvent, false, INFINITE);

		if (code != WAIT_OBJECT_0) {
			pr_err("[PCM ERR]DspMixOut_Start: Wait DSP Start Failed.\r\n");
			return NORESOURCE;
		}
		pr_debug("[PCM]DspMixOut_Start: DSP Started %dms\r\n", (s32)(1000 * jiffies / HZ));
		m_prDspMixOut.m_u4RP = 0;
		m_prDspMixOut.m_u4State = STATE_STARTED;
		pr_debug("[PCM]DspMixOut_Start: m_u4State change to STATE_STARTED.\r\n");

		m_prDspMixOut.m_u4IntrNum = 0;
		m_prDspMixOut.m_u4EmptyNum = 0;

		if (m_prDspMixOut.m_u4WP) {
                    DspCfgSetCommBufWptr(m_prDspMixOut.m_u4WP);
		}

		x_event_set(g_hDspMixEvent);

	} else {
		pr_err("[PCM ERR]DspMixOut_Start: State(%d) error!\r\n", (s32)m_prDspMixOut.m_u4State);
		return INVALIDSTATE;
	}

	return NOERR;
}

s32 DspMixOut_Stop(void)
{
	u32 code = 0;

	if (STATE_STARTED == m_prDspMixOut.m_u4State) {
		AudGpsMix_CmdStop();


		code = x_event_wait_for_objects(1, &m_hGpsMixStopEvent, false, INFINITE);

		if (code != WAIT_OBJECT_0) {
			pr_err("[PCM ERR]DspMixOut_Stop: Wait DSP Stop Failed.\r\n");
			return NORESOURCE;
		}

		m_prDspMixOut.m_u4State = STATE_STOPPED;
		pr_debug("[PCM]DspMixOut_Stop: m_u4State change to STATE_STOPPED.\r\n");
		m_prDspMixOut.m_u4WP = 0;
		m_prDspMixOut.m_u4RP = 0;
		x_memset((void *)m_prDspMixOut.m_u4VirAddr, 0, m_prDspMixOut.m_u4MemSize << 1);
	} else {
		pr_err("[PCM ERR]DspMixOut_Stop: DspMixOut is not started!\r\n");
		return INVALIDSTATE;
	}

	return NOERR;
}

void DspMixOut_HibernationCtrl(bool fgWakeUp)
{
	if (fgWakeUp) {
		m_prDspMixOut.m_fgHibernated = false;
		DspMixOut_SwitchOutPut(AUD_BT_CALL_MODE);
		DspMixOut_SwitchOutPut(AUD_NORMAL_MODE);
		x_memset((void *)m_prDspMixOut.m_u4VirAddr, 0, m_prDspMixOut.m_u4MemSize << 1);
	} else {
		DspMixOut_Stop();
		m_prDspMixOut.m_fgHibernated = true;
	}
}

/*==================================//
	#define DspMixOut_Format_OutputCh
//==================================*/

u32 DspMixOut_SetFormat(const PCMFMT_T *prFmt)
{
	x_memcpy(&m_prDspMixOut.m_rFmt, prFmt, sizeof(PCMFMT_T));

	return NOERR;
}

u32 DspMixOut_GetFormat(PCMFMT_T *prFmt)
{
	x_memcpy(prFmt, &m_prDspMixOut.m_rFmt, sizeof(PCMFMT_T));

	return NOERR;
}

u32 DspMixOut_initOutputCh(void)
{
	m_prDspMixOut.m_eCurUsedMode = AUD_OUTPUT_MODE_NUM;
	x_memset(m_prDspMixOut.m_rDspMixCfg, 0, sizeof(m_prDspMixOut.m_rDspMixCfg));
	DspMixOut_SetOutputCh(AUD_NORMAL_MODE, DSP_MIX_FRONT_L_CH|DSP_MIX_FRONT_R_CH);
	DspMixOut_SetOutputCh(AUD_BT_CALL_MODE, DSP_MIX_FRONT_L_CH|DSP_MIX_FRONT_R_CH);
	DspMixOut_SwitchOutPut(AUD_NORMAL_MODE);

	return NOERR;
}

u32 DspMixOut_SetOutputCh(AUD_OUTPUT_MODE eAudOutPutMode, u32 u4Ch)
{
	DSP_MIX_CFG rDspMixCfg;

	pr_debug("[PCM]DspMixOut_SetOutputCh: Set dsp mix to ch(0x%x)\r\n", (u32)u4Ch);
	x_memset(&rDspMixCfg, 0, sizeof(DSP_MIX_CFG));
	rDspMixCfg.Union.u4Value = (u4Ch << 1);

	//rDspMixCfg.Union.u4DspMixEnable = rDspMixCfg.Union.u4DspMixSubwooferCH;
	//rDspMixCfg.Union.u4DspMixSubwooferCH = rDspMixCfg.Union.u4DspMixCh78;
	//rDspMixCfg.Union.u4DspMixCh78 = rDspMixCfg.Union.u4DspMixEnable;
	rDspMixCfg.Union.u4DspMixEnable = 1;

	if (eAudOutPutMode < AUD_OUTPUT_MODE_NUM) {
		x_memcpy(&m_prDspMixOut.m_rDspMixCfg[eAudOutPutMode], &rDspMixCfg, sizeof(rDspMixCfg));
		DspMixOut_SwitchOutPut(eAudOutPutMode);
	}

	return NOERR;
}

u32 DspMixOut_SwitchOutPut(AUD_OUTPUT_MODE eAudOutPutMode)
{
	m_prDspMixOut.m_eCurUsedMode = eAudOutPutMode;
        if(AOUT_FRMR == g_eRearOut)
        {
            m_prDspMixOut.m_rDspMixCfg[m_prDspMixOut.m_eCurUsedMode].Union.u4DspMixCh9 = 1;
	    m_prDspMixOut.m_rDspMixCfg[m_prDspMixOut.m_eCurUsedMode].Union.u4DspMixCh10 = 1;
        }
	pr_debug("[PCM]DspMixOut_SwitchOutPut: Switch to ch(0x%x)\r\n",
		(u32)m_prDspMixOut.m_rDspMixCfg[m_prDspMixOut.m_eCurUsedMode].Union.u4Value);
	DspCfgSetGpsMixCh(m_prDspMixOut.m_rDspMixCfg[m_prDspMixOut.m_eCurUsedMode].Union.u4Value);

	return NOERR;
}

/*==================================//
		#define DspMixOut_BUF_RW
//==================================*/

s32 DspMixOut_initBuf(void)
{
	AUD_GPS_MIX_COMM_BUF_INFO rBufInfo;

	DspCfgGetCommBufInfo(&rBufInfo);
	m_prDspMixOut.m_u4MemSize = rBufInfo.u4CommBufEA - rBufInfo.u4CommBufSA;
	m_prDspMixOut.m_u4VirAddr = rBufInfo.u4CommBufSA;
	if (!m_prDspMixOut.m_u4VirAddr) {
		pr_err("[PCM ERR]DspMixOut_initBuf: Failed to malloc DspMix AOUT Buffer.\r\n");
		return NORESOURCE;
	}
	x_memset((void *)m_prDspMixOut.m_u4VirAddr, 0, m_prDspMixOut.m_u4MemSize);
	m_prDspMixOut.m_u4MemSize >>= 1;
	pr_debug("[PCM]DspMixOut_initBuf: SA(0x%lx) EA(0lx%x) MemSize(%d)\r\n",
		(uintptr_t)rBufInfo.u4CommBufSA, (uintptr_t)rBufInfo.u4CommBufEA,
		(s32)m_prDspMixOut.m_u4MemSize);

	return NOERR;
}

u32 DspMixOut_setBuffer(void)
{
	m_prDspMixOut.m_rOutBuf.u4Chn = 2;
	m_prDspMixOut.m_rOutBuf.u4VirSAdr = m_prDspMixOut.m_u4VirAddr;
	m_prDspMixOut.m_rOutBuf.u4ChBufSz = m_prDspMixOut.m_u4MemSize;
	m_prDspMixOut.m_rOutBuf.u4DataOff = 0;
	m_prDspMixOut.m_rOutBuf.u4DataSize = m_prDspMixOut.m_u4MemSize - 48;

	return NOERR;
}

s32 DspMixOut_GetBuffer(AUD_DATA_BUF_T *prBuffer)
{
	if (prBuffer) {
		x_memcpy(prBuffer, &m_prDspMixOut.m_rOutBuf, sizeof(AUD_DATA_BUF_T));
	} else {
		pr_err("[PCM ERR]DspMixOut_GetBuffer: prBuffer is NULL!\r\n");
		return INVALIDPRAM;
	}

	return NOERR;
}

u32 DspMixOut_GetRP(void)
{
	AUD_GPS_MIX_COMM_BUF_INFO rBufInfo;

	if (STATE_STARTED == m_prDspMixOut.m_u4State) {
		DspCfgGetCommBufInfo(&rBufInfo);
	} else {
		rBufInfo.u4CommBufRptr = 0;
	}

	return rBufInfo.u4CommBufRptr;
}

u32 DspMixOut_GetNextRP(u32 *pu4SLen)
{
	u32 u4Ret = 0;

	if (pu4SLen) {
		*pu4SLen = 0;
	}

	u4Ret = DspMixOut_GetRP();
	if (u4Ret != m_prDspMixOut.m_u4WP) {
		u4Ret += 192U;
		if (u4Ret >= m_prDspMixOut.m_u4MemSize) {
			u4Ret -= m_prDspMixOut.m_u4MemSize;
		}
	}

	return u4Ret;
}

u32 DspMixOut_UpdateWP(u32 u4WP)
{
	if (STATE_STARTED == m_prDspMixOut.m_u4State) {
		if (m_prDspMixOut.m_u4WP == m_prDspMixOut.m_u4RP)
			x_event_set(g_hDspMixEvent);

            DspCfgSetCommBufWptr(u4WP);
	}
	m_prDspMixOut.m_u4WP = u4WP;

	return NOERR;
}
void DspMixNotifyEvent(void)
{
    x_event_set(g_hDspMixEvent);

    return;
}

/*==================================//
	#define DspMixOut_Intr_Reg
//==================================*/

u32	DspMixOut_InterruptThread(void)
{
	if (STATE_STARTED == m_prDspMixOut.m_u4State) {
		u32 u4Time = (u32)1000 * (u32)jiffies / (u32)HZ;
		u32 u4Rptr = DspMixOut_GetRP();
		u32 u4UsedByte = DspMixOut_resetMem(u4Rptr);

		u4UsedByte = (!u4UsedByte) ? (1920) : (u4UsedByte);
		m_prDspMixOut.m_pfnCb(m_prDspMixOut.m_u4CbParam, u4UsedByte);

		u4Time = (u32)1000 * (u32)jiffies / (u32)HZ - u4Time;
		u4Time = (u4Time >= m_prDspMixOut.m_u4CbTime) ? (0) : (m_prDspMixOut.m_u4CbTime - u4Time);

		m_prDspMixOut.m_u4EmptyNum = ((u4Rptr == m_prDspMixOut.m_u4WP) &&
			(m_prDspMixOut.m_u4RP == m_prDspMixOut.m_u4WP)) ? (m_prDspMixOut.m_u4EmptyNum + 1) : (0);
		m_prDspMixOut.m_u4RP = u4Rptr;

		return u4Time;
	}

	return INFINITE;
}

u32 DspMixOut_resetMem(u32 u4Rptr)
{
	u32 u4UsedByte = 0;
	uintptr_t u4VirSAdr2 = 0;

	if (m_prDspMixOut.m_rOutBuf.u4Chn == 1) {
		u4VirSAdr2 = m_prDspMixOut.m_rOutBuf.u4VirSAdr;
	} else {
		u4VirSAdr2 = m_prDspMixOut.m_rOutBuf.u4VirSAdr + m_prDspMixOut.m_rOutBuf.u4ChBufSz;
	}

	if (u4Rptr >= m_prDspMixOut.m_u4RP) {
		u4UsedByte = u4Rptr - m_prDspMixOut.m_u4RP;
		if (u4UsedByte) {
			x_memset((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr + m_prDspMixOut.m_u4RP), 0, u4UsedByte);
			x_memset((void *)(u4VirSAdr2 + m_prDspMixOut.m_u4RP), 0, u4UsedByte);
			
			__flush_dcache_area((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr + m_prDspMixOut.m_u4RP), u4UsedByte);
			__flush_dcache_area((void *)(u4VirSAdr2 + m_prDspMixOut.m_u4RP), u4UsedByte);
		}
	} else {
		u4UsedByte = u4Rptr + m_prDspMixOut.m_u4MemSize - m_prDspMixOut.m_u4RP;
		if (u4UsedByte) {
			x_memset((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr + m_prDspMixOut.m_u4RP), 0,
				m_prDspMixOut.m_u4MemSize - m_prDspMixOut.m_u4RP);
			x_memset((void *)(u4VirSAdr2 + m_prDspMixOut.m_u4RP), 0,
				m_prDspMixOut.m_u4MemSize - m_prDspMixOut.m_u4RP);
			
			__flush_dcache_area((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr + m_prDspMixOut.m_u4RP), m_prDspMixOut.m_u4MemSize - m_prDspMixOut.m_u4RP);
			__flush_dcache_area((void *)(u4VirSAdr2 + m_prDspMixOut.m_u4RP), m_prDspMixOut.m_u4MemSize - m_prDspMixOut.m_u4RP);
			if (u4Rptr) {
				x_memset((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr), 0, u4Rptr);
				x_memset((void *)(u4VirSAdr2), 0, u4Rptr);

				__flush_dcache_area((void *)(m_prDspMixOut.m_rOutBuf.u4VirSAdr), u4Rptr);
				__flush_dcache_area((void *)(u4VirSAdr2), u4Rptr);
			}
		}
	}

	return u4UsedByte;
}


u32 AudioOut_RegISTCB(PFN_ISR_CB pfnCb, u32 u4Param, u32 u4CbTime)
{
	m_prDspMixOut.m_pfnCb = pfnCb;
	m_prDspMixOut.m_u4CbParam = u4Param;
	m_prDspMixOut.m_u4CbTime = u4CbTime;

	return NOERR;
}


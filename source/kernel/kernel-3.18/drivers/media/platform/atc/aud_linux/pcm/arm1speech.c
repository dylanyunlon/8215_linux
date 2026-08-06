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

#include <windows.h>
#include "bt_cfg.h"
#include "bt_osl.h"
#include "bt_speech.h"
#include "bt_arm1_speech_proc.h"
#include "bt_perf_stat.h"
#include "speech_dual.h"
#include "winutil.h"
#include "aud_pcm_dbg.h"
#include "u_os.h"
#include "aud_comm_macros.h"
#include "speechenhance.h"

#include "pcm_debug.h"
#define LOG_TAG "arm1sph"

#define AEC_MSGQ_NAME				TEXT("AEC_MSGQ")
#define BT_MSGQ_NAME				TEXT("BT_MSGQ")
#define ARM1_BT_SEMA_NAME			TEXT("ARM1_BT_SEMA")

#define SPH_MSG_QUEUE_ITEM_NUM	20U
#define SPH_MSG_QUEUE_ITEM_SIZE	16U


static struct semaphore  g_rSpeechSema;

static u32 g_hAECMsgQ;
static u32 g_hBTMsgQ;

struct task_struct *g_hArm2Thread = NULL;
struct task_struct *g_hAECIST = NULL;

static bool g_fgArm2Inited;


static void EnterCriticalSection(void)
{
	down(&g_rSpeechSema);
}

static void InitializeCriticalSection(void)
{
	sema_init(&g_rSpeechSema, (s32)1);
}

static void LeaveCriticalSection(void)
{
	up(&g_rSpeechSema);
}

static void SpeechHandleMsg(u32 u4MsgID, u32 u4P1, u32 u4P2, u32 u4P3)
{
	switch (u4MsgID & 0xFFFFU) {
	case BT_MSG_COMPLETED:
		g_prSpeechEnhance.m_u4AECEvent = u4P1 & 0xFFFFU;
		x_event_set(g_prSpeechEnhance.m_hCompleted);
		g_prSpeechEnhance.m_u4AECState = u4P2;
		break;

	case BT_STATE_CHANGED:
		g_prSpeechEnhance.m_u4AECState = u4P1;
		break;

	case BT_FRAME_COMPLETED:
		SpeechEnhance_PostProcess(u4P1);
		break;

	default:
		break;
	}
}


static s32 AEC_IST(void *data)
{
	bool fgISTExit = false;

	PCM_DEBUG(LOG_TAG, "AEC_IST: start\r\n");
	while (!fgISTExit) {
		s32  i4_res = 0;
		u16 ui2_index = 0;
		u32 z_msg_size = SPH_MSG_QUEUE_ITEM_SIZE;
		u32 *phHandle = (u32 *)&g_hAECMsgQ;
		u32 u4Param[4] = {0};

		if (phHandle == NULL) {
			PCM_ERROR(LOG_TAG, "AEC_IST_tasklet_process: g_hAECMsgQ is NULL\r\n");
			return false;
		}

		i4_res = x_msg_q_receive(&ui2_index, u4Param, &z_msg_size, phHandle, 1U, X_MSGQ_OPTION_WAIT);
		if (i4_res == (s32)OSR_OK) {
			if (SPH_MSG_QUEUE_ITEM_SIZE != z_msg_size) {
				PCM_ERROR(LOG_TAG, "AEC_IST: Read AEC Msg size(%d)\r\n", z_msg_size);
			}
			SpeechHandleMsg(u4Param[0], u4Param[1], u4Param[2], u4Param[3]);
		} else {
			PCM_ERROR(LOG_TAG, "AEC_IST: Read AEC Message Failure %d\r\n", (s32)i4_res);
		}
	}
	PCM_DEBUG(LOG_TAG, "AEC_IST: AEC IST Exit. fgISTExit(%d)\r\n", (s32)fgISTExit);

	return 0;
}

static bool AEC_IST_Init(void)
{
	if (x_msg_q_create(&g_hAECMsgQ, (const char *)AEC_MSGQ_NAME,
		SPH_MSG_QUEUE_ITEM_SIZE, SPH_MSG_QUEUE_ITEM_NUM) != OSR_OK) {
		PCM_ERROR(LOG_TAG, "AEC_IST_Init: Failed to create AEC msg queue\r\n");
		return false;
	}

	g_hAECIST = kthread_create(AEC_IST, (void *)NULL, "AEC_IST");
	if (IS_ERR(g_hAECIST)) {
		PCM_ERROR(LOG_TAG, "AEC_IST_Init: Create AEC_IST_Thread ERR %ld!\r\n", PTR_ERR(g_hAECIST));
		g_hAECIST = NULL;
		return false;
	}
	set_user_nice(g_hAECIST, PCM_TASK_THREAD_NICE_PRIORITY);
	wake_up_process(g_hAECIST);

	return true;
}

static s32 ARM2_EMU_Thread(void *data)
{
	while (true) {
		s32  i4_res = 0;
		u16 ui2_index = 0;
		u32 z_msg_size = SPH_MSG_QUEUE_ITEM_SIZE;
		u32 *phHandle = (u32 *)&g_hBTMsgQ;
		u32 u4Param[4] = {0};

		if (phHandle == NULL) {
			PCM_ERROR(LOG_TAG, "ARM2_EMU_Thread: _hBTMsgQ is NULL!\r\n");
			return false;
		}

		i4_res = x_msg_q_receive(&ui2_index, u4Param, &z_msg_size, phHandle, 1U, X_MSGQ_OPTION_WAIT);
		if (i4_res == (s32)OSR_OK) {
			if (SPH_MSG_QUEUE_ITEM_SIZE != z_msg_size) {
				PCM_ERROR(LOG_TAG, "ARM2_EMU_Thread: Read BT Msg size(%d)\r\n", z_msg_size);
			}
			SpeechCB(u4Param[0], u4Param[1], u4Param[2], u4Param[3]);
			SpeechStateMachine();
		} else {
			PCM_ERROR(LOG_TAG, "ARM2_EMU_Thread: Read BT Message Failure %d\r\n", (s32)i4_res);
		}
	}
	PCM_DEBUG(LOG_TAG, "ARM2_EMU_Thread: end\r\n");

	return 1;
}

static bool ARM2EmulationInit(void)
{
	if (!g_fgArm2Inited) {
		InitializeCriticalSection();
		if (x_msg_q_create(&g_hBTMsgQ, (const char *)BT_MSGQ_NAME,
			SPH_MSG_QUEUE_ITEM_SIZE, SPH_MSG_QUEUE_ITEM_NUM) != OSR_OK) {
			PCM_ERROR(LOG_TAG, "ARM2EmulationInit: Failed to create BT msg queue\r\n");
			return false;
		}
		g_hArm2Thread = kthread_create(ARM2_EMU_Thread, (void *)NULL, "ARM2_EMU_Thread");
		if (IS_ERR(g_hArm2Thread)) {
			PCM_ERROR(LOG_TAG, "ARM2EmulationInit: Create ARM2_EMU_Thread ERR %ld!\r\n",
				PTR_ERR(g_hArm2Thread));
			g_hArm2Thread = NULL;
			return false;
		}
		set_user_nice(g_hArm2Thread, PCM_TASK_THREAD_NICE_PRIORITY);
		wake_up_process(g_hArm2Thread);

		AEC_IST_Init();
		SpeechInit();
		g_fgArm2Inited = true;
		return true;
	}

	return false;
}

/* Speech driver send a message to AEC/NDC/ABF process. */
bool SpeechSendMessage(u32 u4Msg, u32 u4P1, u32 u4P2, u32 u4P3)
{
	u32 dwParam[4];

	dwParam[0] = u4Msg;
	dwParam[1] = u4P1;
	dwParam[2] = u4P2;
	dwParam[3] = u4P3;

	if (g_fgArm2Inited) {
		if (x_msg_q_send(g_hBTMsgQ, dwParam, SPH_MSG_QUEUE_ITEM_SIZE, 1U) != OSR_OK) {
			PCM_ERROR(LOG_TAG, "SpeechSendMessage: Failed to send BT Msg\r\n");
		}
		return true;
	}

	return false;
}

/* AEC/NDC/ABF process send message to speech driver. */
bool AECSendMessage(u32 u4Msg, u32 u4P1, u32 u4P2, u32 u4P3)
{
	u32 dwParam[4];

	dwParam[0] = u4Msg;
	dwParam[1] = u4P1;
	dwParam[2] = u4P2;
	dwParam[3] = u4P3;

	if (x_msg_q_send(g_hAECMsgQ, dwParam, SPH_MSG_QUEUE_ITEM_SIZE, 1U) != OSR_OK) {
		PCM_ERROR(LOG_TAG, "SpeechSendMessage: Failed to send AEC Msg\r\n");
	}

	return true;
}

bool SpeechRegCallback(PFN_Callback pfnCallback)
{
	PFN_Callback pfnSpeech;

	ARM2EmulationInit();
	pfnSpeech = pfnCallback;

	return true;
}

u32 SHARE_MEM(u32 u4VA, u32 u4PA)
{
	return u4VA;
}

bool TAKE_BT_HW_SEMAPHORE(void)
{
	EnterCriticalSection();

	return true;
}

bool RELEASE_BT_HW_SEMAPHORE(void)
{
	LeaveCriticalSection();

	return true;
}



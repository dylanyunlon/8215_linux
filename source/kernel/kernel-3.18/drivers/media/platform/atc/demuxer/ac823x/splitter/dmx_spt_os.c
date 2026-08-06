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
 */

/*!
 * @file dmx_spt_os.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#include "x_os.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_cpsa.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_sema.h"
#include "dmx_spt_os.h"
#include "dmx_spt_if.h"
#include "dmx_spt_main.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_psr.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_psr_decrypt.h"
#include "dmx_gau_if.h"
#include "dmx_pvr.h"
#include "dmx_pfm.h"
#include "dmx_esm_if.h"
#include "dmx_spt_util.h"
#include "cfa_if.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;
EXTERN const char*g_awszDmxSptStatus[];
EXTERN const char*g_awszDmxSptTxStatus[];
EXTERN AU_AUDIO    g_arLogAudioAUs[DMX_MAX_LOG_AUDIO_AU_CNT];
EXTERN u32	g_u4PbBufFlag;

typedef struct {
	SplitterState	eState;
	SplitterTxState eTxState;
} DMX_SPT_STATE_PART_T;

static DMX_SPT_STATE_PART_T _rStatePairs[9] = {
	{SPLITTER_STATE_NONE,	 SPLITTER_TX_STATE_NONE}
	, {SPLITTER_STATE_IDLE,	 SPLITTER_TX_STATE_NONE}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_CHECK}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_PAUSE}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_RSPOFF}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_ABORT}
	, {SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_JUMP}
	};


static char *g_awszCmdName[MAX_OF_DMX_CMD_TYPE_CNT] = {
	TEXT("DMX_CMD_NONE"),
	TEXT("DMX_CMD_PTX_ON"),
	TEXT("DMX_CMD_RSP_OFF"),
	TEXT("DMX_CMD_RSP_REBUF"),
	TEXT("DMX_CMD_RSP_ON"),
	TEXT("DMX_CMD_PTX_PAUSE"),
	TEXT("DMX_CMD_PTX_RESUME"),
	TEXT("DMX_CMD_PTX_OFF"),
	TEXT("DMX_CMD_EXIT"),
	TEXT("DMX_CMD_STM_ENABLE")
};

#ifdef __linux__
static u8 from_sched_priority(int sched_priority)
{
	return (u8)((100 - sched_priority) * 256 / 100);
}

static int to_sched_priority(u8 ui1_priority)
{
	int sched_priority;

	sched_priority = 100 - (int)ui1_priority * 100 / 256;
	if (sched_priority < 1)
		sched_priority = 1;
	if (sched_priority > 99)
		sched_priority = 99;
	return sched_priority;
}
#endif

/*/ Per instance, per task, we may need to chek splitter handle here*/
#ifdef __linux__
static int	SplitterForeverLoop(void *pv_arg);
#else
static void SplitterForeverLoop(void *pv_arg);
#endif

MRESULT SplitterCreateTask(DMX_SPT_INST_T *prSpt, u32 u4SptIdx)
{
	u32  dwTxPriority  = 200;
	struct task_struct *pthread = 0;

	char   szNameBuf[MAX_OF_SPT_NAME_LEN + 1];

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(szNameBuf, 0, sizeof(char) * (MAX_OF_SPT_NAME_LEN + 1));

	strcpy(szNameBuf, SPT_THREAD_NAME);
	szNameBuf[3] = szNameBuf[3] + (char)u4SptIdx;

	prSpt->fgExitThread = FALSE;

	if (NULL != (void *)(prSpt->hSptTask))
		prSpt->hSptTask = (HANDLE)NULL;

	pthread = kthread_create(SplitterForeverLoop, prSpt, szNameBuf);
	if ((struct task_struct *)pthread == ERR_PTR(-ENOMEM)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in kthread_create")
			TEXT("(szNameBuf(%s), u4SptIdx(%d)), prSpt:0x%p, Err: %p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, szNameBuf, u4SptIdx, prSpt, pthread);
		prSpt->fgExitThread = TRUE;
		prSpt->hSptTask = (HANDLE)NULL;
		MM_RETURN(RET_DMX_NO_MEM);
	} else if (IS_ERR(pthread)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in kthread_create")
			TEXT("(szNameBuf(%s), u4SptIdx(%d)), prSpt:0x%p, Err: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, szNameBuf, u4SptIdx, prSpt, pthread);
		prSpt->fgExitThread = TRUE;
		prSpt->hSptTask = (HANDLE)NULL;
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	} else {
		struct sched_param param;
		int ret;

		mm_memset(&param, 0, sizeof(param));
		param.sched_priority = to_sched_priority(dwTxPriority);
		ret = sched_setscheduler_nocheck(pthread,
			SCHED_RR, &param);
		DMX_ASSERT(ret == 0);
		dwTxPriority = from_sched_priority(param.sched_priority);
	}

	init_waitqueue_head(&(prSpt->waitexitqueue));
	prSpt->waitexitqueueflag = 0;

	wake_up_process(pthread);

	prSpt->hSptTask = (HANDLE)pthread;

	DMXLOG_DEBUG(TEXT("[SPT] %s success, exit\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterDeleteTask(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;
	DMX_CMD_INFO_T rCmd;

	DMXLOG_DEBUG(TEXT("[SPT] %s enter, pvSptHdl:0x%p\r\n"),
		DMX_FUNC_NAME, prSpt);

	if (NULL == prSpt)
		MM_RETURN(RET_DMX_OK);

	if (NULL == (void *)(prSpt->hSptTask))
		MM_RETURN(RET_DMX_OK);

	if (prSpt->fgExitThread) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s exit, fgExitThread already is TRUE, prSpt:0x%p\r\n"),
			DMX_FUNC_NAME, prSpt);
		MM_RETURN(RET_DMX_OK);
	}

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmd.eCmd		= DMX_CMD_EXIT;
	rCmd.fgASync	= TRUE;
	rCmd.u4UsrEvts	= SPLITTER_UEV_EXIT;
	rCmd.u4WaitTime = DMX_SPT_WAIT_EXIT_MAXTIME;

	mrRet = SplitterSendCmd(prSpt, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSendCmd(DMX_CMD_")
			TEXT("EXIT), prSpt: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prSpt, mrRet);
		MM_RETURN(mrRet);
	}

	if ((void *)(prSpt->hSptTask) != NULL) {
		if (!IS_ERR((struct task_struct *)(prSpt->hSptTask))) {
			prSpt->waitexitqueueflag = 1;
			wake_up_interruptible(&(prSpt->waitexitqueue));
			kthread_stop((struct task_struct *)(prSpt->hSptTask));
			DMXLOG_TRACE(TEXT("[SPT] %s -- kthread_stop(prSpt: 0x%p, state: %d) ")
				TEXT("success\r\n"),
				DMX_FUNC_NAME, prSpt, prSpt->eSptState);
		} else {
			DMXLOG_TRACE(TEXT("[SPT] %s -- thread task struct is invalid,")
				TEXT(" prSpt:0x%p\r\n"),
				DMX_FUNC_NAME, prSpt);
		}

		prSpt->hSptTask = (HANDLE)NULL;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterSleepTask(DMX_SPT_INST_T *prSpt, u32 u4MiniSec)
{
	UNUSE_PARAMETER(prSpt);

	DMX_THREAD_DELAY(u4MiniSec);

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterCreateSema(DMX_SPT_INST_T *prSpt)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL != prSpt->hSptSemaphore) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for sema already exist, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, prSpt);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	mrRet = dmx_sema_create(&(prSpt->hSptSemaphore), DMX_SEMA_TYPE_BINARY,
		DMX_SEMA_STATE_UNLOCK);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);

	DMXLOG_ERROR(TEXT("[SPT] %s fail in create semaphore, ")
		TEXT("prSpt:0x%p, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, prSpt, mrRet);

	MM_RETURN(mrRet);
}


MRESULT SplitterDeleteSema(DMX_SPT_INST_T *prSpt)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prSpt)
		MM_RETURN(RET_DMX_OK);

	if (NULL != prSpt->hSptSemaphore) {
		mrRet = dmx_sema_delete(prSpt->hSptSemaphore);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in delete semaphore, ")
				TEXT("prSpt:0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			MM_RETURN(mrRet);
		}

		prSpt->hSptSemaphore = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT SplitterLockSema(DMX_SPT_INST_T *prSpt)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = dmx_sema_lock(prSpt->hSptSemaphore, DMX_SEMA_OPTION_WAIT);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);

	DMXLOG_ERROR(TEXT("[SPT] %s fail in lock semaphore, ")
		TEXT("prSpt:0x%p, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, prSpt, mrRet);

	MM_RETURN(mrRet);
}


MRESULT SplitterReleaseSema(DMX_SPT_INST_T *prSpt)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = dmx_sema_unlock(prSpt->hSptSemaphore);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);

	DMXLOG_ERROR(TEXT("[SPT] %s fail in unlock semaphore, ")
		TEXT("prSpt:0x%p, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, prSpt, mrRet);

	MM_RETURN(mrRet);
}

MRESULT SplitterCreateEvent(DMX_SPT_INST_T *prSpt, u32 u4Idx)
{
	s32 i4Ret = OSR_OK;
	char  szNameBuf[MAX_OF_SPT_NAME_LEN];

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt: 0x%p, u4Idx: 0x%x\r\n"),
			DMX_FUNC_NAME, prSpt, u4Idx);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL != prSpt->hSptEvent) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for sema already exist, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, prSpt);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	mm_memset(szNameBuf, 0, sizeof(char) * MAX_OF_SPT_NAME_LEN);
	strcpy(szNameBuf, SPT_EVENT_NAME);
	szNameBuf[3] = szNameBuf[3] + (char)u4Idx;

	i4Ret = x_ev_group_create((uintptr_t *)&(prSpt->hSptEvent),
		(const char *)szNameBuf,
		 SPLITTER_EV_INITIAL);
	if (OSR_OK != i4Ret) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in x_ev_group_create create")
			TEXT(" spt event, prSpt:0x%p, i4Ret: 0x%x\r\n"),
			DMX_FUNC_NAME, prSpt, i4Ret);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	if (NULL != prSpt->hUsrEvent) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for sema already exist, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, prSpt);
		x_ev_group_delete((uintptr_t)prSpt->hSptEvent);
		prSpt->hSptEvent = NULL;
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	mm_memset(szNameBuf, 0, sizeof(char) * MAX_OF_SPT_NAME_LEN);
	strcpy(szNameBuf, SPT_USR_EVENT_NAME);
	szNameBuf[3] = szNameBuf[3] + (char)u4Idx;

	i4Ret = x_ev_group_create((uintptr_t *)&(prSpt->hUsrEvent),
		(const char *)szNameBuf,
		 SPLITTER_UEV_INITIAL);
	if (OSR_OK != i4Ret) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in x_ev_group_create create")
			TEXT(" spt usr event, prSpt:0x%p, i4Ret: 0x%x\r\n"),
			DMX_FUNC_NAME, prSpt, i4Ret);
		x_ev_group_delete((uintptr_t)prSpt->hSptEvent);
		prSpt->hSptEvent = NULL;
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	DMXLOG_TRACE(
		TEXT("[SPT] %s -- prSpt:0x%p, hUsrEvent: 0x%p, hSptEvent: 0x%p\r\n"),
		DMX_FUNC_NAME, prSpt, prSpt->hUsrEvent, prSpt->hSptEvent);

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterDeleteEvent(DMX_SPT_INST_T *prSpt)
{
	s32 i4Ret = OSR_OK;

	if (NULL == prSpt)
		MM_RETURN(RET_DMX_OK);

	if (NULL != prSpt->hSptEvent) {
		i4Ret = x_ev_group_delete((uintptr_t)prSpt->hSptEvent);
		if (OSR_OK != i4Ret) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in x_ev_group_delete ")
				TEXT("delete spt event, prSpt:0x%p, i4Ret: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, i4Ret);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
		prSpt->hSptEvent = NULL;
	}

	/* Splitter User (MPC) Event */
	if (NULL != prSpt->hUsrEvent) {
		i4Ret = x_ev_group_delete((uintptr_t)prSpt->hUsrEvent);
		if (OSR_OK != i4Ret) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in x_ev_group_delete ")
				TEXT("delete spt usr event, prSpt:0x%p, i4Ret: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, i4Ret);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
		prSpt->hUsrEvent = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterGetEvent(
	DMX_SPT_INST_T		  *prSpt,
	EV_GRP_EVENT_T u4WaitOnEvent,
	EV_GRP_EVENT_T *pu4ReceiveEvent,
	u32		   u4WaitTimeout)
{
	s32 i4Ret = OSR_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- x_ev_group_wait_event_timeout(Spt: 0x%p, prSpt->hSptEvent: 0x%p)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->hSptEvent);
	i4Ret = x_ev_group_wait_event_timeout((uintptr_t)prSpt->hSptEvent,
				u4WaitOnEvent, pu4ReceiveEvent,
				X_EV_OP_OR_CONSUME, u4WaitTimeout);
	if (OSR_OK == i4Ret) {
		DMXLOG_DEBUG(TEXT("[SPT] %s -- prSpt:0x%p wait event:0x%x success!\r\n"),
			DMX_FUNC_NAME, prSpt, u4WaitOnEvent);

		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_ERROR(
		TEXT("[SPT] %s -- prSpt:0x%p wait event:0x%x fail, i4Ret: 0x%x!\r\n"),
		DMX_FUNC_NAME, prSpt, u4WaitOnEvent, i4Ret);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}

MRESULT SplitterResetAllEvents(DMX_SPT_INST_T *prSpt)
{
	EV_GRP_EVENT_T u4ReceiveEvent = 0;
	s32 i4Ret = OSR_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	while (1) {
		i4Ret = x_ev_group_wait_event_timeout(
			(uintptr_t)prSpt->hSptEvent,
			SPLITTER_ALL_EVS, &u4ReceiveEvent,
			X_EV_OP_OR_CONSUME, 0);

		if (OSR_OK != i4Ret)
			break;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterSetEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4SetEvent)
{
	s32 i4Ret = OSR_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- x_ev_group_set_event(Spt: 0x%p, prSpt->hSptEvent: 0x%p)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->hSptEvent);
	i4Ret = x_ev_group_set_event((uintptr_t)prSpt->hSptEvent, u4SetEvent, X_EV_OP_OR);
	if (OSR_OK == i4Ret) {
		DMXLOG_DEBUG(TEXT("[SPT] %s -- prSpt:0x%p set event:0x%x success!\r\n"),
			DMX_FUNC_NAME, prSpt, u4SetEvent);

		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_ERROR(
		TEXT("[SPT] %s line %d -- fail to set event:0x%x, i4Ret: 0x%x, prSpt: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4SetEvent, i4Ret, prSpt);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}


MRESULT SplitterGetUsrEvent(
	DMX_SPT_INST_T		  *prSpt,
	EV_GRP_EVENT_T u4WaitOnEvent,
	EV_GRP_EVENT_T *pu4ReceiveEvent,
	u32		   u4WaitTimeout)
{
	s32 i4Ret = OSR_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	i4Ret = x_ev_group_wait_event_timeout((uintptr_t)prSpt->hUsrEvent,
				u4WaitOnEvent, pu4ReceiveEvent,
				X_EV_OP_OR_CONSUME, u4WaitTimeout);
	if (OSR_OK == i4Ret) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s -- prSpt:0x%p wait usr event:0x%x success!\r\n"),
			DMX_FUNC_NAME, prSpt, u4WaitOnEvent);

		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_ERROR(
		TEXT("[SPT] %s -- prSpt:0x%p wait usr event:0x%x fail, i4Ret: 0x%x!\r\n"),
		DMX_FUNC_NAME, prSpt, u4WaitOnEvent, i4Ret);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}


MRESULT SplitterWaitUsrEvent(
	DMX_SPT_INST_T		  *prSpt,
	EV_GRP_EVENT_T u4WaitOnEvent,
	u32		   u4WaitTimeOut)
{
	EV_GRP_EVENT_T u4ReceiveEvent = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = SplitterGetUsrEvent(prSpt, u4WaitOnEvent,
		&u4ReceiveEvent, u4WaitTimeOut);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s -- fail to wait usr event:0x%x, prSpt: 0x%p!\r\n"),
			DMX_FUNC_NAME, u4WaitOnEvent, prSpt);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	if (u4ReceiveEvent & u4WaitOnEvent)
		MM_RETURN(RET_DMX_OK);

	DMXLOG_ERROR(TEXT("[SPT] %s line %d -- fail to wait usr event:0x%x,")
		TEXT(" ReceiveEvent: 0x%x, prSpt: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4WaitOnEvent, u4ReceiveEvent, prSpt);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}

MRESULT SplitterSetUsrEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4SetEvent)
{
	s32  i4Ret = OSR_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	i4Ret = x_ev_group_set_event((uintptr_t)prSpt->hUsrEvent, u4SetEvent, X_EV_OP_OR);
	if (OSR_OK == i4Ret) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s -- prSpt:0x%p set usr event:0x%x success!\r\n"),
			DMX_FUNC_NAME, prSpt, u4SetEvent);

		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_ERROR(
		TEXT("[SPT] %s line %d -- fail to set usr event:0x%x, i4Ret: 0x%x, ")
		TEXT("prSpt: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4SetEvent, i4Ret, prSpt);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}

MRESULT SplitterCreateCmdQ(DMX_SPT_INST_T *prSpt, u32 u4Idx)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = dmx_sema_create(&(prSpt->rCmdCrits), DMX_SEMA_TYPE_BINARY,
		DMX_SEMA_STATE_UNLOCK);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[DECRYPT] %s fail in create semaphore, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	prSpt->u4CmdRdIdx = 0;
	prSpt->u4CmdWrIdx = 0;
	dmx_memset(prSpt->arCmdInfo, 0, sizeof(prSpt->arCmdInfo));

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterDeleteCmdQ(DMX_SPT_INST_T *prSpt)
{
	if (NULL == prSpt)
		MM_RETURN(RET_DMX_OK);


	prSpt->u4CmdRdIdx = 0;
	prSpt->u4CmdWrIdx = 0;
	dmx_memset(prSpt->arCmdInfo, 0, sizeof(prSpt->arCmdInfo));

	if (NULL != prSpt->rCmdCrits) {
		dmx_sema_delete(prSpt->rCmdCrits);
		prSpt->rCmdCrits = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterChangeStateEx(DMX_SPT_INST_T *prSpt,
	SplitterState eState, SplitterTxState eTxState,
	const char *wszFunc, s32 i4Line)
{
	u32 au4ChangeStates[9] = {0};
	u32 u4ChangeStateCnt = 0, u4Idx = 0;

	SPLITTER_LOCK;

	if ((prSpt->eSptState == eState) &&
		(prSpt->eSptTxState == eTxState)) {
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	mm_memset(au4ChangeStates, 0, sizeof(au4ChangeStates));

	if ((SPLITTER_STATE_NONE == prSpt->eSptState) &&
		(SPLITTER_TX_STATE_NONE == prSpt->eSptTxState)) {
		au4ChangeStates[0] = 1;
		u4ChangeStateCnt = 1;
	} else if ((SPLITTER_STATE_IDLE == prSpt->eSptState) &&
		(SPLITTER_TX_STATE_NONE == prSpt->eSptTxState)) {
		au4ChangeStates[0] = 2;
		au4ChangeStates[1] = 3;
		au4ChangeStates[2] = 5;
		au4ChangeStates[3] = 6;
		u4ChangeStateCnt = 4;
	} else if (SPLITTER_STATE_RUNING == prSpt->eSptState) {
		if (SPLITTER_TX_STATE_CHECK == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 4;
			au4ChangeStates[4] = 7;
			u4ChangeStateCnt = 5;
		} else if (SPLITTER_TX_STATE_TXING == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 4;
			au4ChangeStates[4] = 5;
			au4ChangeStates[5] = 7;
			u4ChangeStateCnt = 6;
		} else if (SPLITTER_TX_STATE_PAUSE == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 4;
			au4ChangeStates[4] = 5;
			au4ChangeStates[5] = 6;
			u4ChangeStateCnt = 6;
		} else if (SPLITTER_TX_STATE_RSPOFF == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 4;
			au4ChangeStates[4] = 5;
			au4ChangeStates[5] = 6;
			u4ChangeStateCnt = 6;
		} else if (SPLITTER_TX_STATE_ABORT == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 6;
			u4ChangeStateCnt = 4;
		} else if (SPLITTER_TX_STATE_JUMP == prSpt->eSptTxState) {
			au4ChangeStates[0] = 1;
			au4ChangeStates[1] = 2;
			au4ChangeStates[2] = 3;
			au4ChangeStates[3] = 4;
			au4ChangeStates[4] = 6;
			au4ChangeStates[5] = 7;
			u4ChangeStateCnt = 6;
		} else {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Current Splitter")
				TEXT("(0x%p)'s state (%s, %s) error, (Func: %s, line: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
				((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
					g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
				((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
					g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
				wszFunc, i4Line);
			DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Can't change Splitter")
				TEXT("(0x%p)'s state from (%s, %s) to (%s, %s), (Func: %s, line: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
				((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
					g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
				((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
					g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
				((eState <= SPLITTER_STATE_RUNING) ?
					g_awszDmxSptStatus[eState] : TEXT("UNKNOWN")),
				((eTxState <= SPLITTER_TX_STATE_ERROR) ?
					g_awszDmxSptTxStatus[eTxState] : TEXT("UNKNOWN")),
				wszFunc, i4Line);
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_ERR_STATE);
		}
	}

	for (u4Idx = 0; u4Idx < u4ChangeStateCnt; u4Idx++) {
		if (au4ChangeStates[u4Idx] < 8) {
			if ((_rStatePairs[au4ChangeStates[u4Idx]].eState == eState) &&
				(_rStatePairs[au4ChangeStates[u4Idx]].eTxState == eTxState)) {
				if ((SPLITTER_STATE_IDLE == prSpt->eSptState) &&
					(SPLITTER_TX_STATE_NONE == prSpt->eSptTxState)) {
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- Change Splitter")
						TEXT("(0x%p)'s state from (%s, %s) to (%s, %s), (Func: %s,")
						TEXT(" line: %d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
						((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
						((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
						((eState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[eState] : TEXT("UNKNOWN")),
						((eTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[eTxState] : TEXT("UNKNOWN")),
						wszFunc, i4Line);
				}

				if ((SPLITTER_STATE_RUNING == eState) &&
					(SPLITTER_TX_STATE_JUMP == eTxState)) {
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- Change Splitter")
						TEXT("(0x%p)'s state from (%s, %s) to (%s, %s), (Func: %s,")
						TEXT(" line: %d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
						((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
						((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
						((eState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[eState] : TEXT("UNKNOWN")),
						((eTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[eTxState] : TEXT("UNKNOWN")),
						wszFunc, i4Line);
				}

				if ((SPLITTER_STATE_IDLE == eState) &&
					(SPLITTER_TX_STATE_NONE == eTxState)) {
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- Change Splitter")
						TEXT("(0x%p)'s state from (%s, %s) to (%s, %s), (Func: %s,")
						TEXT(" line: %d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
						((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
						((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
						((eState <= SPLITTER_STATE_RUNING) ?
							g_awszDmxSptStatus[eState] : TEXT("UNKNOWN")),
						((eTxState <= SPLITTER_TX_STATE_ERROR) ?
							g_awszDmxSptTxStatus[eTxState] : TEXT("UNKNOWN")),
						wszFunc, i4Line);
				}
				prSpt->eSptState = eState;
				prSpt->eSptTxState = eTxState;
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_OK);
			}
		}
	}

	DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Can't change Splitter(0x%p)'s")
		TEXT(" state from (%s, %s) to (%s, %s), (Func: %s, line: %d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt,
		((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
			g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
		((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
			g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")),
		((eState <= SPLITTER_STATE_RUNING) ?
			g_awszDmxSptStatus[eState] : TEXT("UNKNOWN")),
		((eTxState <= SPLITTER_TX_STATE_ERROR) ?
			g_awszDmxSptTxStatus[eTxState] : TEXT("UNKNOWN")),
		wszFunc, i4Line);

	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OPERATE_FORBID);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterIdlePsrOn*/
/* Turn on splitter*/
/* 1. Set Cfa parser end flag to be false*/
/* 2. Enable Parser Filter, Set Spt Event: SPLITTER_EV_PTX_CALL*/
/* 3. Set Rsp Log Enable, start to log resplitter AU*/
/* 4. Turn On Cfa, check whether need to sync pbbuf*/
/*/////////////////////////////////////////////////////////////////////////////*/
static MRESULT SplitterIdlePsrOn(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args -- prSpt is NULL!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* Set SplitterCfaPsr End is Flase and set parser CC's fgCfaPsrEnd to be FALSE*/
	mrRet = SplitterSetCfaPsrEnd(prSpt, FALSE, 0);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSetCfaPsrEnd(pvSptHdl: 0x%p, FALSE)!\r\n"),
			DMX_FUNC_NAME, prSpt);
		MM_RETURN(mrRet);
	}

	/* Set Splitter's TxGround Flag to FALSE*/
	prSpt->u8OtherAudioPts		= INVALID_TIMESTAMP;
	prSpt->pvOtherAudioSptHandle = NULL;

	prSpt->ucAudMaxDuration		= 0;

	prSpt->ucRspTxHdrState		= RSP_TXHDRINFO_IDLE;
	prSpt->ucRspTxAStmHdrState	= RSP_TX_AHEADS_IDLE;
	prSpt->u8PtxFromFileOffset	= 0;
	prSpt->pvPtxFromDramAddress = NULL;

	prSpt->fgDivxDRMOn			= FALSE;
	prSpt->u8DivxDRMOffset		= DMX_INVALID_UINT64;
	prSpt->u4DecLen				= 0;
	prSpt->u2FrameKeyIndex		= DMX_DIVXDRM_INVALID_FRAMEIDX;

	prSpt->fgPtxBusy			= FALSE;

	DMXLOG_DEBUG(TEXT("[SPT] %s PSR_Decrypt_Reset(pvSptHdl: 0x%p, pvPsrCC: 0x%p)!\r\n"),
		DMX_FUNC_NAME, prSpt, prSpt->pvPsrCC);
	mrRet = PSR_Decrypt_Reset(prSpt->pvPsrCC);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in PSR_Decrypt_Reset(prSpt: 0x%p)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s PSR_CC_Enable(pvSptHdl: 0x%p, pvPsrCC: 0x%p)!\r\n"),
		DMX_FUNC_NAME, prSpt, prSpt->pvPsrCC);
	mrRet = PSR_CC_Enable(prSpt->pvPsrCC, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in PSR_CC_Enable(prSpt: 0x%p, TRUE)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		MM_RETURN(mrRet);
	}

#if DMX_DISABLE_CFA
	mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUN, TX_CHECK), prSpt: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
		MM_RETURN(mrRet);
	}
#else
	/* Set Rsp Log Enable*/
	DMXLOG_DEBUG(TEXT("[SPT] %s SplitterRspSetLogEnable(pvSptHdl: 0x%p, pvPsrCC: 0x%p)!\r\n"),
		DMX_FUNC_NAME, prSpt, prSpt->pvPsrCC);
	mrRet = SplitterRspSetLogEnable(prSpt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterRspSetLog")
			TEXT("Enable(prSpt: 0x%p, TRUE)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d SplitterChangeState(pvSptHdl: 0x%p, pvPsrCC: 0x%p)!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->pvPsrCC);
	mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_CHECK);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUN, TX_CHECK), prSpt: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
		MM_RETURN(mrRet);
	}

	#if DMX_PFM_TEST
	DmxPfmCfaStart(prSpt);
	#endif /* DMX_PFM_TEST*/

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- SptCfaSetTurnOn(pvSptHdl: 0x%p, cfaType: %d)!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, SplitterGetCfaType(prSpt));
	/* Turn On Cfa, check whether need to sync pbbuf*/
	mrRet = SptCfaSetTurnOn(prSpt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SptCfaSetTurnOn,")
			TEXT(" prSpt: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
		MM_RETURN(mrRet);
	}

	#if DMX_PFM_TEST
	DmxPfmCfaEnd(prSpt);
	#endif /* DMX_PFM_TEST*/
#endif /* DMX_DISABLE_CFA*/

	MM_RETURN(RET_DMX_OK);
}

static void SplitterProcPsrOnCmd(DMX_SPT_INST_T *prSpt,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != pfgReply) && (NULL != prSpt) && (NULL != prCmd))) {
		DMXLOG_TRACE(TEXT("DBGCHK Failed:%s at line %d\r\n"),
			TEXT(__func__), __LINE__);
		DMX_ASSERT(FALSE);
		return;
	}

	switch (SplitterGetState(prSpt)) {
	case SPLITTER_STATE_IDLE:
		{
			mrRet = SplitterIdlePsrOn(prSpt);

			if (DMX_FAILED(mrRet))
				SplitterSetEOSForError((void *)prSpt, mrRet);

			prCmd->mrRet = mrRet;
			*pfgReply = TRUE;

			return;
		}

		break;
	case SPLITTER_STATE_RUNING:
		break;
	default:
		break;
	}
}

static void SplitterProcPsrPauseCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	MRESULT mrRet = RET_DMX_OK;
	DMX_SPT_INST_T *prSpt = NULL;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMX_ASSERT(FALSE);
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	*pfgReply = FALSE;

	prCmd->mrRet = RET_DMX_OK;

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		{
			PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

			DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- CMD_PTX_PAUSE(prSpt")
				TEXT(":0x%p, PsrCC's state: %d, PsrCC's eTxState: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prPsrCC->eState,
				prPsrCC->eTxState);
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
		}
		return;
	case SPLITTER_STATE_RUNING:
		{
			bool fgNeedToPause = FALSE;

			DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- CMD_PTX_PAUSE(prSpt")
				TEXT(":0x%p, state: %d, eTxState: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState,
				prSpt->eSptTxState);
			switch (prSpt->eSptTxState) {
			case SPLITTER_TX_STATE_PAUSE:
			case SPLITTER_TX_STATE_RSPOFF:
			case SPLITTER_TX_STATE_ABORT:
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- CMD_PTX_PAUSE")
					TEXT("(prSpt:0x%p, state: %d, eTxState: %d).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState,
					prSpt->eSptTxState);
				*pfgReply = TRUE;
				break;
			case SPLITTER_TX_STATE_JUMP:
			case SPLITTER_TX_STATE_TXING:
				DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- CMD_PTX_PAUSE")
					TEXT(" (prSpt:0x%p, state: %d, eTxState: %d), Call PSR_CC_PauseTx\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState,
					prSpt->eSptTxState);
				mrRet = PSR_CC_PauseTx(prSpt->pvPsrCC, &fgNeedToPause);
				prCmd->mrRet = mrRet;
				*pfgReply = FALSE;
				if (((DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) ||
					(!fgNeedToPause))) {
					/*skip cfa done until the current tx is done notified.*/
					SplitterSetPauseWithDone(prSpt);

					mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
						SPLITTER_TX_STATE_PAUSE);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s fail in Splitter")
							TEXT("ChangeState(RUNING, PAUSE), pvSptHdl: 0x%p, mrRet:")
							TEXT(" 0x%x\r\n"),
							DMX_FUNC_NAME, pvSptHdl, mrRet);
						SplitterSetEOSForError(pvSptHdl, mrRet);
						prCmd->mrRet = mrRet;
						*pfgReply = TRUE;
						return;
					}

					DMXLOG_TRACE(TEXT("[SPT] %s line %d ---")
						TEXT(" Pause by Done, State changed to be PTX_PAUSE state,")
						TEXT(" and set use event(prSpt:0x%p)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

					prCmd->mrRet = RET_DMX_OK;
					*pfgReply = TRUE;
				}
				break;
			case SPLITTER_TX_STATE_CHECK:
				break;
			default:
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- CMD_PAUSE")
					TEXT("(prSpt:0x%p, Error SptTxState=%d).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptTxState);
				prCmd->mrRet = RET_DMX_ERR_STATE;
				*pfgReply = TRUE;
				break;
			}
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s -- prSpt: 0x%p, Invalid SptState:")
			TEXT(" 0x%x, SptTxState: 0x%x\r\n"),
			DMX_FUNC_NAME, prSpt, prSpt->eSptState, prSpt->eSptTxState);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		*pfgReply = TRUE;
		break;
	}
}

static void SplitterProcPsrResumeCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMX_ASSERT(FALSE);
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

	*pfgReply = FALSE;
	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		prCmd->mrRet = RET_DMX_OK;
		*pfgReply = TRUE;
		break;
	case SPLITTER_STATE_RUNING:
		switch (prSpt->eSptTxState) {
		case SPLITTER_TX_STATE_PAUSE:
			if ((NULL != prPsrCC) &&
				(CCS_INIT != prPsrCC->eState) &&
				(CCS_IDLE != prPsrCC->eState)) {
				DMXLOG_TRACE(
					TEXT("[SPT] ---------->Wake up splitter(0x%p) thread\r\n"),
					pvSptHdl);
				mrRet = PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
				prCmd->mrRet = RET_DMX_ERR_STATE;
				*pfgReply = TRUE;
				break;
			}
			break;
		default:
			DMXLOG_ERROR(TEXT("[SPT] %s fail for Error Splitter State")
				TEXT(" to do PsrOff (pvSptHdl: 0x%p, SptState: 0x%x,  SptTxState: 0x%x)!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prSpt->eSptState, prSpt->eSptTxState);
			prCmd->mrRet = RET_DMX_ERR_STATE;
			*pfgReply = TRUE;
			break;
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s fail for Error Splitter State to")
			TEXT(" do PsrOff (pvSptHdl: 0x%p, SptState: 0x%x,  SptTxState: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prSpt->eSptState, prSpt->eSptTxState);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		*pfgReply = TRUE;
		break;
	}

	DMXLOG_TRACE(TEXT("[SPT] %s line %d -- CMD_RESUME(prSpt:0x%p, ")
		TEXT("Spt's state:%d, etxState: %d).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState);

	*pfgReply = TRUE;
}

static void SplitterProcPsrOffCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	MRESULT mrRet = RET_DMX_OK;
	SplitterState eState = SPLITTER_STATE_NONE;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMX_ASSERT(FALSE);
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	*pfgReply = FALSE;
	prCmd->mrRet = RET_DMX_OK;

	eState = SplitterGetState(prSpt);
	switch (eState) {
	case SPLITTER_STATE_IDLE:
		{
			PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

			DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- CMD_PTX_PAUSE")
				TEXT("(prSpt:0x%p, PsrCC's state: %d, PsrCC's eTxState: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prPsrCC->eState,
				prPsrCC->eTxState);
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
		}
		return;
	case SPLITTER_STATE_RUNING:
		{
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- PSR_OFF (prSpt:0x%p,")
				TEXT(" state: %d, eTxState: %d).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState,
				prSpt->eSptTxState);
			switch (prSpt->eSptTxState) {
			case SPLITTER_TX_STATE_PAUSE:
			case SPLITTER_TX_STATE_RSPOFF:
			case SPLITTER_TX_STATE_TXING:
			case SPLITTER_TX_STATE_ABORT:
				DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- PSR_OFF (prSpt")
					TEXT(":0x%p, state: %d, eTxState: %d), Call PSR_CC_AbortTx\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState,
					prSpt->eSptTxState);
				mrRet = PSR_CC_AbortTx(prSpt->pvPsrCC);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC")
						TEXT("_AbortTx (prSpt: 0x%p, State: 0x%x, mrRet: 0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt, eState, mrRet);
					DMX_ASSERT(FALSE);
				}
				prCmd->mrRet = mrRet;
				break;
			case SPLITTER_TX_STATE_JUMP:
			default:
				DMXLOG_ERROR(TEXT("[SPT] %s fail for Error Splitter ")
					TEXT("State to do PsrOff (pvSptHdl: 0x%p, State: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, eState);
				DMX_ASSERT(FALSE);
				prCmd->mrRet = RET_DMX_ERR_STATE;
				*pfgReply = TRUE;
				break;
			}
		}
		break;

	default:
		DMXLOG_ERROR(TEXT("[SPT] %s fail for Error Splitter State to")
			TEXT(" do PsrOff (pvSptHdl: 0x%p, State: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, eState);
		DMX_ASSERT(FALSE);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		*pfgReply = TRUE;
	}
}

static void SplitterProcExitCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMX_ASSERT(FALSE);
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		{
			prSpt->fgExitThread = TRUE; /* this is used to notify the main thread exit*/
			prCmd->mrRet = mrRet;
			*pfgReply = TRUE;

			return;
		}
		break;
	case SPLITTER_STATE_RUNING:
		break;
	default:
		break;
	}
}

static void SplitterProcRspOffCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	DMX_SPT_RSPOFF_CMDINBUF_T *prRspOffInBuf = NULL;
	PSR_CC *prPsrCC = NULL;
	u8  *pu1RspTxRet = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMXLOG_ERROR(TEXT(" Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		return;
	}

	if ((NULL == prCmd->pvInBuf) ||
		(sizeof(DMX_SPT_RSPOFF_CMDINBUF_T) != prCmd->u4InBufSz) ||
		(NULL == prCmd->pvOutBuf) ||
		(sizeof(u8) != prCmd->u4OutBufSz)) {
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		*pfgReply = TRUE;
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	prRspOffInBuf = (DMX_SPT_RSPOFF_CMDINBUF_T *)(prCmd->pvInBuf);
	pu1RspTxRet = (u8 *)(prCmd->pvOutBuf);
	prCmd->mrRet = RET_DMX_OK;
	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

	DMXLOG_TRACE(TEXT("[SPT] %s line %d -- SptState=%d, SptTxState=%d,")
		TEXT(" fgPsrEnd=%s, prSpt: 0x%p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt->eSptState, prSpt->eSptTxState,
		(SplitterIsCfaPsrEnd(prSpt) ? TEXT("TRUE") : TEXT("FALSE")), prSpt);

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- RSP_OFF in Splitter ")
			TEXT("IDLE State, fgPsrEnd=%s, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(SplitterIsCfaPsrEnd(prSpt) ? TEXT("TRUE") : TEXT("FALSE")), prSpt);

		if (!SplitterRspIsEnabled(prSpt)) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- ReSplitter Fail ")
				TEXT("for RSP is disabled, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			SplitterSetReResplitter(pvSptHdl, FALSE);
			*pu1RspTxRet = 0;
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
			return;
		}

		if (SplitterIsReResplitter(prSpt)) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Re Resplitter, ")
				TEXT("Receive RSP_OFF event, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			SplitterRspTxFinish4ReRsp(prSpt);
		}

		/* To set fgRspStart tobe FALSE, the next flow will not enter into this section*/
		SplitterSetRspOffStart(prSpt, FALSE);

		/* Normal TX Pause after TX done*/
		SplitterSetPauseWithDone(prSpt);

		/* Change Splitter's State*/
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- prSpt(0x%p) Set UEV_PTX")
			TEXT("_PAUSE evt, change Spt's State to TX_RSPOFF!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

		mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_RSPOFF);
		if (DMX_FAILED(mrRet)) {
			*pu1RspTxRet = 0;
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterChangeState")
				TEXT("(RUNING, PAUSE), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, pvSptHdl, mrRet);
			return;
		}

		*pu1RspTxRet = 1;
		prCmd->mrRet = RET_DMX_OK;
		*pfgReply = TRUE;

		return;
	case SPLITTER_STATE_RUNING:
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- RSP_OFF, RUNNING, ")
			TEXT("SptTxState=%d, fgPsrEnd=%s, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->eSptTxState,
			(SplitterIsCfaPsrEnd(prSpt) ? TEXT("TRUE") : TEXT("FALSE")), prSpt);
		switch (prSpt->eSptTxState) {
		case SPLITTER_TX_STATE_CHECK:
		case SPLITTER_TX_STATE_TXING:
			if (!SplitterRspIsEnabled(prSpt)) {

				SplitterSetReResplitter(pvSptHdl, FALSE);
				*pu1RspTxRet = 0;
				prCmd->mrRet = RET_DMX_OK;
				*pfgReply = TRUE;
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- RSP Disabled,")
					TEXT(" So don't support to do resplitter ========!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return;
			}
			SplitterSetRspOffStart(prSpt, TRUE);
			*pu1RspTxRet = 1;
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = FALSE;
			if ((NULL != prPsrCC) &&
				(CCS_TX == prPsrCC->eState) &&
				((TXS_WAIT_FIFO == prPsrCC->eTxState) ||
				 (TXS_WAIT_VFIFO_PTS_THRESHOLD == prPsrCC->eTxState))) {
				PSR_CC_CBSplitter(prSpt->pvPsrCC, E_WAKEUP_ME, NULL);
			}
			break;
		case SPLITTER_TX_STATE_RSPOFF:
			*pu1RspTxRet = 1;
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
			break;
		default:
			break;
		}
		break;
	default:
		SplitterSetReResplitter(pvSptHdl, FALSE);
		*pu1RspTxRet = 0;
		prCmd->mrRet = RET_DMX_OK;
		*pfgReply = TRUE;
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Unsupport RSP for error")
			TEXT(" SptState(0x%x) to do Resplitter Off ========!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->eSptState);
		break;
	}
}

static void SplitterProcRspGetRebufRangeCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	DMX_SPT_GETREBUFRANGE_INBUF_T  *prCmdInBuf = NULL;
	DMX_SPT_GETREBUFRANGE_OUTBUF_T *prCmdOutBuf = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != pfgReply) && (NULL != pvSptHdl) && (NULL != prCmd))) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		return;
	}

	if ((NULL == prCmd->pvInBuf) ||
		(sizeof(DMX_SPT_GETREBUFRANGE_INBUF_T) != prCmd->u4InBufSz) ||
		(NULL == prCmd->pvOutBuf) ||
		(sizeof(DMX_SPT_GETREBUFRANGE_OUTBUF_T) != prCmd->u4OutBufSz)) {
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	prCmdInBuf = (DMX_SPT_GETREBUFRANGE_INBUF_T *)(prCmd->pvInBuf);
	prCmdOutBuf = (DMX_SPT_GETREBUFRANGE_OUTBUF_T *)(prCmd->pvOutBuf);

	prCmdOutBuf->fgNeedRebuf = FALSE;
	prCmd->mrRet = RET_DMX_OK;

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		break;
	case SPLITTER_STATE_RUNING:
		if (SPLITTER_TX_STATE_RSPOFF != prSpt->eSptTxState) {

			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for error splitter")
				TEXT(" state to do Rebuffer(pvSptHdl: 0x%p, State: %d, eTxState: %d).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState,
				prSpt->eSptTxState);
			prCmdOutBuf->fgNeedRebuf = FALSE;
			PSR_CC_GetCurPbbufStartOffset(prSpt->pvPsrCC, &(prCmdOutBuf->u8PbbStartOffset));
			prCmdOutBuf->u8RspStartOffset = prCmdOutBuf->u8PbbStartOffset;
			prCmd->mrRet = RET_DMX_ERR_STATE;
			SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
			*pfgReply = TRUE;
			return;
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for error splitter state")
			TEXT(" to do Rebuffer(pvSptHdl: 0x%p, State: %d, eTxState: %d).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState,
			prSpt->eSptTxState);
		prCmdOutBuf->fgNeedRebuf = FALSE;
		PSR_CC_GetCurPbbufStartOffset(prSpt->pvPsrCC, &(prCmdOutBuf->u8PbbStartOffset));
		prCmdOutBuf->u8RspStartOffset = prCmdOutBuf->u8PbbStartOffset;
		prCmd->mrRet = RET_DMX_ERR_STATE;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	SplitterSetRspStartOffset(pvSptHdl, DMX_INVALID_UINT64);
	SplitterSetRspOffsetDelta(pvSptHdl, 0);

	switch (prSpt->ucRspMode) {/*PTS mode is default*/
	case SPLITTER_PTX_RSP_BY_PTS:
		{
			SplitterSetRspStartPts(pvSptHdl, prCmdInBuf->u8RspStartPts);

			SplitterSetRspPtsDelta(pvSptHdl, prCmdInBuf->u8RspDelta);

			SplitterSetRspStartOffset(pvSptHdl, prCmdInBuf->u8RspStartOffset);

			/* Get the Rsp Entry whose start Pts is little smaller than the designated Playing*/
			/* PTS(u8RspStartPts + u8RspPtsDelta)*/
			/* Get its fileoffset of the Rsp entry*/
			mrRet = SplitterRspGetRspRangeByPts(pvSptHdl, prCmdInBuf->u8RspDelta,
				&(prCmdOutBuf->u8RspStartOffset), &(prCmdOutBuf->u8PbbStartOffset));
		}
		break;
	case SPLITTER_PTX_RSP_BY_OFFSET:
		{
			SplitterSetRspStartOffset(pvSptHdl, prCmdInBuf->u8RspStartOffset);

			SplitterSetRspOffsetDelta(pvSptHdl, prCmdInBuf->u8RspDelta);

			/* Get the Rsp Entry whose start Pts is little smaller than the designated Playing*/
			/* PTS(u8RspStartPts + u8RspPtsDelta)*/
			/* Get its fileoffset of the Rsp entry*/
			mrRet = SplitterRspGetRspRangeByOffset(pvSptHdl, prCmdInBuf->u8RspDelta,
				&(prCmdOutBuf->u8RspStartOffset), &(prCmdOutBuf->u8PbbStartOffset));
		}
		break;
	default:
		break;
		DMXLOG_ERROR(TEXT("[SPT] %s fail for unsupport Resplitter Mode")
			TEXT("(pvSptHdl: 0x%p, RspMode: %d).\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prSpt->ucRspMode);
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	if (DMX_SUCCEED(mrRet) || (RET_DMX_NO_RSP_LOGAU == mrRet)) {
		/* if Rsplitter Start fileOffse is not in the current pbbuf, we need to do ReBuf*/
		if (prCmdOutBuf->u8RspStartOffset < prCmdOutBuf->u8PbbStartOffset) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- RspStartOffset is")
				TEXT(" not in PBBuf data(fgRebuf=TRUE)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			#ifdef __linux__
			DMXLOG_TRACE(
				TEXT("[SPT] RspStartOfst: %lld, PbbufStartOfst: %lld!\r\n"),
				prCmdOutBuf->u8RspStartOffset, prCmdOutBuf->u8PbbStartOffset);
			#else
			DMXLOG_TRACE(
				TEXT("[SPT] RspStartOfst: %I64d, PbbufStartOfst: %I64d!\r\n"),
				prCmdOutBuf->u8RspStartOffset, prCmdOutBuf->u8PbbStartOffset);
			#endif /* #ifdef __linux__*/
			prCmdOutBuf->fgNeedRebuf = TRUE;
		} else {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- RspstartOffset is")
				TEXT(" in PBBuf data(fgRebuf=FALSE)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			prCmdOutBuf->fgNeedRebuf = FALSE;
		}
		mrRet = RET_DMX_OK;
	} else {
		DMXLOG_TRACE(TEXT("[SPT] %s line %d fail in get rsp log au info")
			TEXT(", so set fgNeedRebuf to be false!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		prCmdOutBuf->fgNeedRebuf = FALSE;
	}

	if (prCmdOutBuf->fgNeedRebuf) {
//#if DMX_NEW_PBBUF_MECHANISM
		/* Release All PBBUF.*/
		if (g_u4PbBufFlag) {
	  		mrRet = PBBUF_CancelAllocSlot(pvSptHdl);
	  		if (DMX_FAILED(mrRet)) {
	  			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PBBUF_Cancel")
	  				TEXT("AllocSlot, pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
	  				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
	  			prCmd->mrRet = mrRet;
	  			SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
				}
		}
//#endif
		PSR_CC_ClearAllPBBufInfo(prSpt->pvPsrCC);
		PBBUF_CleanAllSlots(pvSptHdl);
	}

	prCmd->mrRet = mrRet;
	*pfgReply = TRUE;
}

static void SplitterProcRspOnCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_SPT_INST_T *prSpt = NULL;
	CfaIntf *prCfaInterface	= NULL;
	void *pvCfaPrivateData = NULL;
	bool fgRebuf = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prCmd->pvInBuf) ||
		(sizeof(bool) != prCmd->u4InBufSz)) {
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	fgRebuf = *((bool *)(prCmd->pvInBuf));

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		break;
	case SPLITTER_STATE_RUNING:
		if (SPLITTER_TX_STATE_RSPOFF != prSpt->eSptTxState) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for error splitter")
				TEXT(" state to do Rebuffer(pvSptHdl: 0x%p, State: %d, eTxState: %d).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState,
				prSpt->eSptTxState);
			prCmd->mrRet = RET_DMX_ERR_STATE;
			SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
			*pfgReply = TRUE;
			return;
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for error splitter state")
			TEXT(" to do Rebuffer(pvSptHdl: 0x%p, State: %d, eTxState: %d).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState,
			prSpt->eSptTxState);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	prCfaInterface	 = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	pvCfaPrivateData = prSpt->pvCfaPrivateData;

	if (NULL == prCfaInterface) {
		prCmd->mrRet = RET_DMX_NO_CFA_INTERFACE;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	if (NULL == pvCfaPrivateData) {
		prCmd->mrRet = RET_DMX_NO_CFA_PRIV_DATA;
		SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
		*pfgReply = TRUE;
		return;
	}

	/* Inform the cfa which support Rsp that whether need to do rebuffer, if need, the cfa will */
	/*clear some information*/
	/*/ MPG auto-enlarge hdrbuf for performance*/
	if (NULL != prCfaInterface->pfmrRebuf) {
		mrRet = prCfaInterface->pfmrRebuf(pvSptHdl, fgRebuf, pvCfaPrivateData);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s Fail in Cfa->pfmrRebuf, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			prCmd->mrRet = mrRet;
			SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
			*pfgReply = TRUE;
			return;
		}
	}

	prSpt->fgRspRebuf = fgRebuf;

	if (SplitterIsResumeWithDone(prSpt)) {
		SplitterSetPtxNotBusy(prSpt);

		/* Search the first resplitter entry which need to tx*/
		/* Triggle to Tx the entry's data into FIFO*/
		mrRet = SplitterRspSetRspEnable(prSpt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterRsp")
				TEXT("SetRspEnable, prSpt: 0x%p, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
			prCmd->mrRet = mrRet;
			SplitterSetEOSForError(pvSptHdl, prCmd->mrRet);
			*pfgReply = TRUE;
			return;
		}

		SplitterClrPauseWithDone(prSpt);
	} else {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for fgSptPauseWithDone")
			TEXT("=FALSE, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		SplitterSetEOSForError(pvSptHdl, RET_DMX_ERR_STATE);
		*pfgReply = TRUE;
		return;
	}

	prCmd->mrRet = RET_DMX_OK;
	*pfgReply = TRUE;
}

static MRESULT SplitterStmEnable(DMX_STM_INST_T *prStm, bool fgEnable)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid arg!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- Process %s Stream %s, ")
		TEXT("InUsing(%s), pvSptHdl: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, (fgEnable ? TEXT("ENABLE") : TEXT("DISABLE")),
		((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
		((prStm->fgInUsing) ? TEXT("TRUE") : TEXT("FALSE")),
		prStm->pvSptHdl);

	if (fgEnable) {
		mrRet = PSR_Filter_Enable(prStm->pvPsrFtr);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_Enable: hStm")
				TEXT(": 0x%x, StmType: 0x%x!\r\n"),
				DMX_FUNC_NAME, prStm, prStm->u4StmType);
			MM_RETURN(mrRet);
		}

		if (SPT_DATA_A != (E_SPT_DATA_TYPE_T)(prStm->u4StmType)) {
			if ((u32)(prStm->u4GAUHandle) < MAX_GAU_INSTANCE_CNT) {
				mrRet = GAU_Enable((u32)(prStm->u4GAUHandle), TRUE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s(%s) fail in GAU_Enable")
						TEXT("(TRUE), mrRet: 0x%x!\r\n"), DMX_FUNC_NAME,
						((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ?
							g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
						mrRet);
					MM_RETURN(mrRet);
				}
			}
		}

		switch ((E_SPT_DATA_TYPE_T)(prStm->u4StmType)) {
		case SPT_DATA_V:
			/*g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID] = TRUE;*/
			break;
		case SPT_DATA_A:
			/*g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD] = TRUE;*/
			break;
		case SPT_DATA_SP:
			/*g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP] = TRUE; */
			break;
		default:
			break;
		}

		if (SPT_DATA_A == (E_SPT_DATA_TYPE_T)(prStm->u4StmType)) {
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(prStm->pvPsrFtr);

			if ((NULL != prPsrFtr) &&
				(DMX_INVALID_UINT32 != prPsrFtr->u4ESIH)) {
				ESM_CheckFifoClearStatus(prPsrFtr->u4ESIH);
			}
			mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);
		}

		if (prStm->u4StmType != STREAM_DMA &&
			prStm->u4StmType != STREAM_INITIAL) {
			/* We will set CFA UID until the filter enable, to avoid the CFA type is not set*/
			/* This is due to the NV filter issue.*/
			u32 u4CfaStmType = SplitterGetCfaStreamType(prStm->u4StmType,
				GetStreamCntFromType(prStm->pvDmxInst, prStm));
			mrRet = SptCfaSetStmInfo(prStm->pvSptHdl, u4CfaStmType, prStm->u4StmUID);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in SptCfaSetStmInfo")
					TEXT(": prStm: 0x%p, u4CFAStmType: 0x%x, u4StmUID: 0x%x!\r\n"),
					DMX_FUNC_NAME, prStm, u4CfaStmType, prStm->u4StmUID);
				MM_RETURN(mrRet);
			}

			mrRet = SptCfaEnableStream(prStm->pvSptHdl, u4CfaStmType, TRUE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in SptCfaEnableStream")
					TEXT(": prStm: 0x%p, u4CFAStmType: 0x%x!\r\n"),
					DMX_FUNC_NAME, prStm, u4CfaStmType);
				MM_RETURN(mrRet);
			}
		}

		prStm->fgEnable = TRUE;
	} else {
		if (prStm->u4StmType != STREAM_DMA &&
			prStm->u4StmType != STREAM_INITIAL) {
			/* This mainly for clear the corresponding cfa's PrsFlags*/
			u32 u4CfaStmType = SplitterGetCfaStreamType(prStm->u4StmType,
				GetStreamCntFromType(prStm->pvDmxInst, prStm));
			mrRet = SptCfaEnableStream(prStm->pvSptHdl, u4CfaStmType, FALSE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s failed in SptCfaEnableStream")
					TEXT(", prStm: 0x%p, StmType: 0x%x\r\n"),
					DMX_FUNC_NAME, prStm, prStm->u4StmType);
				MM_RETURN(mrRet);
			}
		}

		if ((u32)(prStm->u4GAUHandle) < MAX_GAU_INSTANCE_CNT) {
			mrRet = GAU_Enable((u32)(prStm->u4GAUHandle), FALSE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s(%s) fail in GAU_Enable(")
					TEXT("FALSE), mrRet: 0x%x!\r\n"), DMX_FUNC_NAME,
					((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ?
						g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
					mrRet);
				MM_RETURN(mrRet);
		   }
		}

		mrRet = PSR_Filter_Disable(prStm->pvPsrFtr);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s failed in PSR_Filter_Disable,")
				TEXT(" prStm: 0x%p, StmType: 0x%x\r\n"),
				DMX_FUNC_NAME, prStm, prStm->u4StmType);
			MM_RETURN(mrRet);
		}

		switch ((E_SPT_DATA_TYPE_T)(prStm->u4StmType)) {
		case SPT_DATA_V:
			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
				DmxCloseDumpVFile();
			break;
		case SPT_DATA_A:
			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
				DmxCloseDumpAFile();
			break;
		case SPT_DATA_SP:
			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP])
				DmxCloseDumpSPFile();
		default:
			break;
		}

		prStm->fgEnable = FALSE;
	}

	MM_RETURN(RET_DMX_OK);
}

static void SplitterProcStmEnableCmd(void *pvSptHdl,
	DMX_CMD_INFO_T *prCmd, bool *pfgReply)
{
	DMX_STM_ENABLE_INPUTBUF_T *prInBuf = NULL;
	DMX_SPT_INST_T *prSpt = NULL;
	DMX_STM_INST_T *prStm = NULL;
	PSR_CC	*prPsrCC = NULL;
	bool	fgEnable = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (!((NULL != pfgReply) && (NULL != prSpt) && (NULL != prCmd))) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		return;
	}

	if ((NULL == prCmd->pvInBuf) ||
		(sizeof(DMX_STM_ENABLE_INPUTBUF_T) != prCmd->u4InBufSz)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for invalid params, pvSptHdl: 0x%p!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		*pfgReply = TRUE;
		return;
	}

	prInBuf = (DMX_STM_ENABLE_INPUTBUF_T *)(prCmd->pvInBuf);

	fgEnable = prInBuf->fgEnable;
	prStm = (DMX_STM_INST_T *)(prInBuf->pvStm);

	if (NULL == prStm) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for invalid params,")
			TEXT(" hStm==NULL, pvSptHdl: 0x%p!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		prCmd->mrRet = RET_DMX_PARAM_WRONG;
		*pfgReply = TRUE;
		return;
	}

	DMXLOG_DEBUG(
		TEXT("[SPT] %s line %d -- Process %s Stream %d(%s), pvSptHdl: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, (fgEnable ? TEXT("ENABLE") : "DISABLE"),
		prStm->u4StmType,
		((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
		pvSptHdl);

	switch (prStm->u4StmType) {
	case SPT_DATA_V:
	case SPT_DATA_A:
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		break;
	default:
		prCmd->mrRet = RET_DMX_OK;
		*pfgReply = TRUE;
		return;
	}

	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

	*pfgReply = FALSE;
	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		prCmd->mrRet = RET_DMX_OK;
		*pfgReply = TRUE;
		break;
	case SPLITTER_STATE_RUNING:
		switch (prSpt->eSptTxState) {
		case SPLITTER_TX_STATE_PAUSE:
		case SPLITTER_TX_STATE_RSPOFF:
		case SPLITTER_TX_STATE_ABORT:
		case SPLITTER_TX_STATE_JUMP:
			prCmd->mrRet = RET_DMX_OK;
			*pfgReply = TRUE;
			break;
		case SPLITTER_TX_STATE_TXING:
			{
				prPsrCC = (PSR_CC *)(prStm->pvPsrCC);

				if (NULL != prPsrCC) {
					DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- PsrCC's eState")
						TEXT(": %d, eTxState: %d, ActFilter: 0x%p, PsrFtr: 0x%p\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrCC->eState, prPsrCC->eTxState,
						prPsrCC->pvActFilter, prStm->pvPsrFtr);
				} else {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for Stream's")
						TEXT(" PsrCC ==NULL, pvSptHdl: 0x%p, StmType: %d!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prStm->u4StmType);
					prCmd->mrRet = RET_DMX_UNEXPECT;
					*pfgReply = TRUE;
					return;
				}

				if (!fgEnable) {
					if ((CCS_TX == prPsrCC->eState) &&
						((TXS_TXING == prPsrCC->eTxState) ||
						(TXS_WAIT_IRQ_PROC == prPsrCC->eTxState))) {
						prCmd->mrRet = RET_DMX_OK;
						*pfgReply = FALSE;
						return;
					}

					/* If Parser CC's Current TxState is WAIT_VFIFO_PTS_THRESHOLD, and */
					/*wait pts is 0, Wait Parser Filter is this, set Wake up it*/
					if ((TXS_WAIT_VFIFO_PTS_THRESHOLD == prPsrCC->eTxState) &&
						(INVALID_TIMESTAMP != prPsrCC->u8NormalWaitPts) &&
						(prStm->pvPsrFtr == prPsrCC->pvNormalWaitFtr)) {
						prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
						prPsrCC->pvNormalWaitFtr = NULL;
					}
				}
			}
			break;
		case SPLITTER_TX_STATE_CHECK:
		case SPLITTER_TX_STATE_ERROR:
		default:
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for Error Splitter")
				TEXT(" State to do PsrOff (pvSptHdl: 0x%p, SptState: 0x%x,	SptTxState")
				TEXT(": 0x%x)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState, prSpt->eSptTxState);
			prCmd->mrRet = RET_DMX_ERR_STATE;
			*pfgReply = TRUE;
			return;
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for Error Splitter ")
			TEXT("State to do PsrOff (pvSptHdl: 0x%p, SptState: 0x%x,  SptTxState: 0x%x)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->eSptState, prSpt->eSptTxState);
		prCmd->mrRet = RET_DMX_ERR_STATE;
		*pfgReply = TRUE;
		return;
	}

	mrRet = SplitterStmEnable(prStm, fgEnable);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterStmEnable")
			TEXT("(prStm: 0x%p, fgEnable: %s)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prStm,
			(fgEnable ? TEXT("TRUE") : TEXT("FALSE")));
	}

	prCmd->mrRet = mrRet;

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- CMD_STM_ENABLE(prSpt:0x%p,")
		TEXT(" Spt's state:%d, etxState: %d).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState);

	*pfgReply = TRUE;
}

static DMX_CMD_INFO_T *SplitterGetCmd(DMX_SPT_INST_T *prSpt)
{
	DMX_CMD_INFO_T *prCmd = NULL;

	dmx_sema_lock(prSpt->rCmdCrits, DMX_SEMA_OPTION_WAIT);

	if (0 == DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT)) {
		dmx_sema_unlock(prSpt->rCmdCrits);

		return NULL;
	}

	prCmd = &(prSpt->arCmdInfo[prSpt->u4CmdRdIdx]);

	while (0 == prCmd->u4RefCnt) {
		prSpt->u4CmdRdIdx++;

		if (prSpt->u4CmdRdIdx >= MAX_SPT_CMD_CNT)
			prSpt->u4CmdRdIdx -= MAX_SPT_CMD_CNT;

		if (0 == DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT)) {
			dmx_sema_unlock(prSpt->rCmdCrits);

			return NULL;
		}

		prCmd = &(prSpt->arCmdInfo[prSpt->u4CmdRdIdx]);
	}

	prCmd->u4RefCnt++;

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Get Cmd(eType: %d, Name:")
		TEXT(" %s, RefCnt: %d), CmdRdIdx: %d, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
		(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
			g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
		prCmd->u4RefCnt, prSpt->u4CmdRdIdx, prSpt);

	dmx_sema_unlock(prSpt->rCmdCrits);

	return prCmd;
}

static MRESULT SplitterReleaseCmd(DMX_SPT_INST_T *prSpt, DMX_CMD_INFO_T *prCmd)
{
	dmx_sema_lock(prSpt->rCmdCrits, DMX_SEMA_OPTION_WAIT);

	if (0 == DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT)) {
		dmx_sema_unlock(prSpt->rCmdCrits);

		MM_RETURN(RET_DMX_OK);
	}

	prCmd->u4RefCnt--;

	dmx_sema_unlock(prSpt->rCmdCrits);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT SplitterReplyCmd(DMX_SPT_INST_T *prSpt, DMX_CMD_INFO_T *prCmd)
{
	u32 u4UsrEvts = 0;
	bool   fgAsync = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	dmx_sema_lock(prSpt->rCmdCrits, DMX_SEMA_OPTION_WAIT);

	if (0 == DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT)) {
		dmx_sema_unlock(prSpt->rCmdCrits);

		MM_RETURN(RET_DMX_OK);
	}

	if (prCmd != &(prSpt->arCmdInfo[prSpt->u4CmdRdIdx])) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Cmd(eType: %d, Name: ")
			TEXT("%s, RefCnt: %d) != CmdRdIdx(%d) Cmd(eType: %d, Name: %s, RefCnt:")
			TEXT(" %d), prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
			(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
				g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
			prCmd->u4RefCnt, prSpt->u4CmdRdIdx, prSpt->arCmdInfo[prSpt->u4CmdRdIdx].eCmd,
			(prSpt->arCmdInfo[prSpt->u4CmdRdIdx].eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
				g_awszCmdName[prSpt->arCmdInfo[prSpt->u4CmdRdIdx].eCmd] : TEXT("UNKNOWN"),
			prSpt->arCmdInfo[prSpt->u4CmdRdIdx].u4RefCnt, prSpt);
		dmx_sema_unlock(prSpt->rCmdCrits);

		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prCmd->u4RefCnt--;

	if (0 < prCmd->u4RefCnt) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Cmd(eType: %d, Name: %s,")
			TEXT(" RefCnt: %d) is used by others, CmdRdIdx: %d, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
			(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
				g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
			prCmd->u4RefCnt, prSpt->u4CmdRdIdx, prSpt);
		dmx_sema_unlock(prSpt->rCmdCrits);

		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Reply Cmd(eType: %d, Name:")
		TEXT(" %s), CmdRdIdx: %d, prSpt: 0x%p, mrRet=0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
		(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
			g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
		prSpt->u4CmdRdIdx, prSpt, prCmd->mrRet);

	if (NULL != prCmd->prHdl)
		prCmd->prHdl->mrRet = prCmd->mrRet;

	u4UsrEvts = prCmd->u4UsrEvts;
	fgAsync = prCmd->fgASync;

	smp_mb();

	mm_memset(prCmd, 0, sizeof(DMX_CMD_INFO_T));

	prSpt->u4CmdRdIdx++;

	if (prSpt->u4CmdRdIdx >= MAX_SPT_CMD_CNT)
		prSpt->u4CmdRdIdx -= MAX_SPT_CMD_CNT;

	if (!fgAsync) {
		mrRet = SplitterSetUsrEvent(prSpt, u4UsrEvts);
		if (DMX_FAILED(mrRet))
			SplitterSetEOSForError((void *)prSpt, mrRet);
	}

	/* in case that the CMD_IN signal is reset while there are other cmd need to proc.*/
	if (DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT) > 0) {
		MRESULT mrRet2 = RET_DMX_OK;

		DMXLOG_ERROR(TEXT("[SPT] %s line %d -- u4CmdRdIdx(%d), ")
			TEXT("u4CmdWrIdx(%d), RemainCmdCnt > 0 (prSpt: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, prSpt);
		mrRet2 = SplitterSetEvent(prSpt, SPLITTER_EV_SPT_CMD_IN);
		if (DMX_FAILED(mrRet2)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d failed in Set EV_CMD")
				TEXT("_IN event (prSpt: 0x%p)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			SplitterSetEOSForError((void *)prSpt, mrRet2);
			dmx_sema_unlock(prSpt->rCmdCrits);
			MM_RETURN(mrRet2);
		}
	}

	dmx_sema_unlock(prSpt->rCmdCrits);

	MM_RETURN(mrRet);
}

static MRESULT SplitterClearCmd(DMX_SPT_INST_T *prSpt, DMX_CMD_INFO_T *prCmd)
{
	u32 u4CmdRdIdx = 0;

	dmx_sema_lock(prSpt->rCmdCrits, DMX_SEMA_OPTION_WAIT);

	if (DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx,
		MAX_SPT_CMD_CNT) + 1 >= MAX_SPT_CMD_CNT) {
		dmx_sema_unlock(prSpt->rCmdCrits);
		MM_RETURN(RET_DMX_OK);
	}

	u4CmdRdIdx = prSpt->u4CmdRdIdx;

	while (0 != DMX_DATASIZE(u4CmdRdIdx, prSpt->u4CmdWrIdx, MAX_SPT_CMD_CNT)) {
		if (prSpt->arCmdInfo[u4CmdRdIdx].prHdl == prCmd) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d  -- Clear Cmd(eType:")
				TEXT(" %d, Name: %s, RefCnt: %d), CmdIdx: %d, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
				(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
					g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
			prSpt->arCmdInfo[u4CmdRdIdx].u4RefCnt, u4CmdRdIdx, prSpt);
			if (1 == prSpt->arCmdInfo[u4CmdRdIdx].u4RefCnt) {
				mm_memset(&(prSpt->arCmdInfo[u4CmdRdIdx]), 0, sizeof(DMX_CMD_INFO_T));
				if (u4CmdRdIdx == prSpt->u4CmdRdIdx) {
					prSpt->u4CmdRdIdx++;
					if (prSpt->u4CmdRdIdx >= MAX_SPT_CMD_CNT)
						prSpt->u4CmdRdIdx -= MAX_SPT_CMD_CNT;
				}
				break;
			}
			break;
		}
		u4CmdRdIdx++;
		if (u4CmdRdIdx >= MAX_SPT_CMD_CNT)
			u4CmdRdIdx -= MAX_SPT_CMD_CNT;
	}
	dmx_sema_unlock(prSpt->rCmdCrits);

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterSendCmd(DMX_SPT_INST_T *prSpt, DMX_CMD_INFO_T *prCmd)
{
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;
	EV_GRP_EVENT_T u4Evts = 0;

	if (!((NULL != prSpt) && (NULL != prCmd))) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	dmx_sema_lock(prSpt->rCmdCrits, DMX_SEMA_OPTION_WAIT);

	if (DMX_DATASIZE(prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx,
		MAX_SPT_CMD_CNT) + 1 >= MAX_SPT_CMD_CNT) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d failed for CmdQ has overflow")
			TEXT(" (WrIdx: %d, RdIdx: %d, prSpt: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->u4CmdWrIdx, prSpt->u4CmdRdIdx, prSpt);
		dmx_sema_unlock(prSpt->rCmdCrits);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	if (prSpt->u4CmdWrIdx >= MAX_SPT_CMD_CNT)
		prSpt->u4CmdWrIdx -= MAX_SPT_CMD_CNT;

	prSpt->arCmdInfo[prSpt->u4CmdWrIdx] = *prCmd;

	prSpt->arCmdInfo[prSpt->u4CmdWrIdx].prHdl = prCmd;

	prSpt->arCmdInfo[prSpt->u4CmdWrIdx].u4RefCnt++;

	prSpt->u4CmdWrIdx++;

	if (prSpt->u4CmdWrIdx >= MAX_SPT_CMD_CNT)
		prSpt->u4CmdWrIdx = 0;

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- (Spt(0x%p)'s CmdArray -->")
		TEXT(" WrIdx: %d, RdIdx: %d, CmdName: %s)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->u4CmdWrIdx, prSpt->u4CmdRdIdx,
		(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
			g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));

	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
	if (NULL != prPsrCC) {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- prSpt: 0x%p, SptState: %d, ")
			TEXT("SptTxState: %d, PsrCC's eState: 0x%x, eTxState: 0x%x) Before SendCmd\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState,
			prPsrCC->eState, prPsrCC->eTxState);
	} else {
		DMXLOG_TRACE(TEXT("[SPT] %s line %d  -- prSpt: 0x%p, SptState: %d, ")
			TEXT("SptTxState: %d, PsrCC==NULL) Before SendCmd\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState);
		MM_RETURN(RET_DMX_NO_CC);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- SplitterSetEvent(Spt: 0x%p)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
	mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_SPT_CMD_IN);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d failed in Set EV_CMD_IN event (prSpt: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		SplitterClearCmd(prSpt, prCmd);
		SplitterSetEOSForError((void *)prSpt, mrRet);
		dmx_sema_unlock(prSpt->rCmdCrits);
		MM_RETURN(mrRet);
	}

	dmx_sema_unlock(prSpt->rCmdCrits);

	if (!prCmd->fgASync) {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- SplitterGetUsrEvent(Spt: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		mrRet = SplitterGetUsrEvent(prSpt, (EV_GRP_EVENT_T)(prCmd->u4UsrEvts),
			&u4Evts, prCmd->u4WaitTime);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d  -- (Spt(0x%p)'s CmdArray")
				TEXT(" --> WrIdx: %d, RdIdx: %d, CmdName: %s) process fail\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->u4CmdWrIdx, prSpt->u4CmdRdIdx,
				(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
					g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));

			DMXLOG_ERROR(TEXT("[SPT] %s line %d  -- Spt(0x%p) failed")
				TEXT(" in  Get User Event(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prCmd->u4UsrEvts);
			if (NULL != prPsrCC) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d  -- prSpt: 0x%p, ")
					TEXT("SptState: %d, SptTxState: %d, PsrCC's eState: 0x%x, ")
					TEXT("eTxState: 0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState,
					prPsrCC->eState, prPsrCC->eTxState);
			} else {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d  -- prSpt: 0x%p, ")
					TEXT("SptState: %d, SptTxState: %d, PsrCC==NULL)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState);
			}

			SplitterClearCmd(prSpt, prCmd);
			SplitterSetEOSForError((void *)prSpt, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d	-- Spt(0x%p) Get User Event(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt, u4Evts);
		}

		MM_RETURN(mrRet);
	} else {
		prCmd->mrRet =	RET_DMX_OK;
	}

	MM_RETURN(mrRet);
}

static MRESULT SplitterProcCmd(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;
	DMX_CMD_INFO_T *prCmd = NULL;
	bool	fgReply = FALSE;

	MM_ASSERT(NULL != prSpt);

	prCmd = SplitterGetCmd(prSpt);

	if (NULL == prCmd) {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- NO CMD, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Process Cmd(eType: %d, ")
		TEXT("Name: %s), CmdRdIdx: %d, WrIdx: %d, prSpt: 0x%p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd,
		(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
			g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"),
		prSpt->u4CmdRdIdx, prSpt->u4CmdWrIdx, prSpt);

	switch (prCmd->eCmd) {
	case DMX_CMD_PTX_ON:
		SplitterProcPsrOnCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_RSP_OFF:
		SplitterProcRspOffCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_RSP_REBUF:
		SplitterProcRspGetRebufRangeCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_RSP_ON:
		SplitterProcRspOnCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_PTX_PAUSE:
		SplitterProcPsrPauseCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_PTX_RESUME:
		SplitterProcPsrResumeCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_PTX_OFF:
		SplitterProcPsrOffCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_EXIT:
		SplitterProcExitCmd(prSpt, prCmd, &fgReply);
		break;
	case DMX_CMD_STM_ENABLE:
		SplitterProcStmEnableCmd(prSpt, prCmd, &fgReply);
		break;
	default:
		DMXLOG_ERROR(
			TEXT("[SPT]  %s line %d -- Invalid CmdID(%d) (prSpt:0x%p). --------\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt);
		break;
	}

	if (prCmd->fgASync) {
		SplitterReleaseCmd(prSpt, prCmd);
		SplitterReplyCmd(prSpt, prCmd);
	} else {
		if (fgReply || DMX_FAILED(prCmd->mrRet)) {
			SplitterReleaseCmd(prSpt, prCmd);
			SplitterReplyCmd(prSpt, prCmd);
		} else {
			SplitterReleaseCmd(prSpt, prCmd);
		}
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterSendNfy(DMX_SPT_INST_T *prSpt, E_DMX_SPT_NTY_TYPE_T eNfy)
{
	MRESULT mrRet = RET_DMX_OK;

	if (!(NULL != prSpt)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eNfy) {
	case DMX_SPT_NTY_TX_CONTINUE:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_PTX_CALL);
		break;
	case DMX_SPT_NTY_TX_HW_CB:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_HW_CB);
		break;
	case DMX_SPT_NTY_TX_END:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_PTX_DONE);
		break;
	case DMX_SPT_NTY_TX_JUMP:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_PTX_JUMP);
		break;
	case DMX_SPT_NTY_TX_PAUSE:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_PTX_PAUSE);
		break;
	case DMX_SPT_NTY_TX_ABORT:
		mrRet = SplitterSetEvent(prSpt, SPLITTER_EV_NFY_PTX_ABORT);
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcCmdInHwCBNfy(DMX_SPT_INST_T *prSpt)
{
	DMX_CMD_INFO_T *prCmd = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prCmd = SplitterGetCmd(prSpt);
	if (NULL != prCmd) {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
				g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
		if (DMX_CMD_STM_ENABLE == prCmd->eCmd) {
			DMX_STM_ENABLE_INPUTBUF_T *prInBuf =
				(DMX_STM_ENABLE_INPUTBUF_T *)(prCmd->pvInBuf);

			if (NULL != prInBuf) {
				DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)(prInBuf->pvStm);
				bool fgEnable = prInBuf->fgEnable;

				if (NULL == prStm) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d ")
						TEXT("fail for invalid params, hStm==NULL, ")
						TEXT("prSpt: 0x%p!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					prCmd->mrRet = RET_DMX_PARAM_WRONG;
					SplitterReleaseCmd(prSpt, prCmd);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}

				mrRet = SplitterStmEnable(prStm, fgEnable);

				if (DMX_FAILED(mrRet)) {
					SplitterReleaseCmd(prSpt, prCmd);
					DMXLOG_ERROR(TEXT("[SPT] %s line %d ")
						TEXT("fail in SplitterStmEnable(prStm: 0x%p, ")
						TEXT("fgEnable: %s)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prStm,
						(fgEnable ? TEXT("TRUE") : TEXT("FALSE")));
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}

			prCmd->mrRet = mrRet;
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
					TEXT(" in SplitterReleaseCmd(prSpt, prCmd->eType=%d)")
					TEXT(", Spt(0x%p)'s state: %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			mrRet = SplitterReplyCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
					TEXT(" in SplitterReplyCmd(prSpt, prCmd->eType=%d),")
					TEXT(" Spt(0x%p)'s state: %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		} else {
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
					TEXT(" in SplitterReleaseCmd(prSpt, prCmd->eType=%d)")
					TEXT(", Spt(0x%p)'s state: %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		}
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcHwCBNfy(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		break;
	case SPLITTER_STATE_RUNING:
		{
			PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- SPLITTER_EV_HW_CB\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			if (NULL == prPsrCC) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail for Spt(0x%p)'s PsrCC is NULL \r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				SplitterSetEOSForError((void *)prSpt, RET_DMX_PARAM_WRONG);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			smp_mb();

			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));

				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				DmxDumpFlow(DMX_OPER_HW_PTX_DONE, &rOperInfo);
			}

			PSR_CC_LOCK(prPsrCC->rLock);

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- PSR_CC_LOCK(prPsrCC->rLock)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			smp_mb();

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- PSR_HAL_ParsingIntData\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			mrRet = PSR_HAL_ParsingIntData(prPsrCC->pvHwData);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_HAL_")
					TEXT("ParsingIntData (mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				prPsrCC->fgWakingWaitTx = FALSE;
				SplitterSetEOSForError((void *)prSpt, mrRet);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- PSR_HAL_ParsingIntData Success\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			#if DMX_DRM_DECRYPT_USE_HW
			mrRet = PSR_CC_CheckHWDecryptStatus(prPsrCC, prPsrCC->pvActFilter);
			if (DMX_FAILED(mrRet)) {
				if (!MM_IS_STATE_ERROR(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_")
						TEXT("Filter_IRQ_Proc (mrRet:0x%x, prSpt:0x%p).\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
					prPsrCC->fgWakingWaitTx = FALSE;
				}
				SplitterSetEOSForError((void *)prSpt, mrRet);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}
			#endif /* DMX_DRM_DECRYPT_USE_HW*/

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- PSR_Filter_IRQ_Proc\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			mrRet = PSR_Filter_IRQ_Proc(prPsrCC->pvActFilter);
			if (DMX_FAILED(mrRet)) {
				if (RET_DMX_NEED_JUMP == mrRet) {
					DMXLOG_DEBUG(
						TEXT("[SPT] %s line %d -- Need Jump (prSpt:0x%p).\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					prPsrCC->fgWakingWaitTx = FALSE;
					prPsrCC->pvHwData = NULL;
					SplitterSetEOSForError((void *)prSpt, mrRet);
					PSR_CC_UNLOCK(prPsrCC->rLock);
					MM_RETURN(mrRet);
				} else if (!MM_IS_STATE_ERROR(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_")
						TEXT("Filter_IRQ_Proc (mrRet:0x%x, prSpt:0x%p).\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
					prPsrCC->fgWakingWaitTx = FALSE;
					SplitterSetEOSForError((void *)prSpt, mrRet);
					PSR_CC_UNLOCK(prPsrCC->rLock);
					MM_RETURN(mrRet);
				}
			}

			DMXLOG_DEBUG(
				TEXT("[SPT] ----- %s line %d -- PSR_Filter_IRQ_Proc\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

			prPsrCC->pvHwData = NULL;
			prPsrCC->fgWakingWaitTx = FALSE;
			PSR_CC_UNLOCK(prPsrCC->rLock);

			if (!((CCS_TX == prPsrCC->eState) &&
				(TXS_WAIT_IRQ_PROC == prPsrCC->eTxState)))
				mrRet = SplitterProcCmdInHwCBNfy(prSpt);

			MM_RETURN(mrRet);
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxDoneNfyInRunningState(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;
	DMX_INST_T *prDmxInst = NULL;

	if (NULL == prSpt) {
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;
	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	prDmxInst->fgPsrOff = FALSE;

	switch (prSpt->eSptTxState) {
	case SPLITTER_TX_STATE_RSPOFF:
	case SPLITTER_TX_STATE_PAUSE:
		SplitterSetPauseWithDone(prSpt);
		MM_RETURN(RET_DMX_OK);
	case SPLITTER_TX_STATE_TXING:
		break;
	default:
		DMXLOG_DEBUG(TEXT("[SPT] %s -- Receive PTX_DONE Notify")
			TEXT(" in Spt's state=RUNING, eSptTxState=%d, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, prSpt->eSptTxState, prSpt);
		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_DEBUG(
		TEXT("[SPT] %s -- SPLITTER_EV_PTX_DONE, prSpt: 0x%p\r\n"),
		DMX_FUNC_NAME, prSpt);

	if (SplitterRspIsEnabled(prSpt) &&
		SplitterIsRspOffStart(prSpt)) {
		DMX_CMD_INFO_T *prCmd = NULL;

		DMXLOG_TRACE(
			TEXT("[SPT] %s -- PTX_DONE Notify, RspEnable, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, prSpt);

		if (SplitterIsReResplitter(prSpt)) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Re Resplitter")
				TEXT(", Receive EV_TX_DONE event, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			SplitterRspTxFinish4ReRsp(prSpt);
		} else {
			/* Not to pause parser and hold the cfa tx done, until the next resume */
			/*to send */
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Receive ")
				TEXT("EV_TX_DONE event, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		}

		/* To set fgRspStart tobe FALSE, the next flow will not enter into this section*/
		SplitterSetRspOffStart(prSpt, FALSE);
		/* Normal TX Pause after TX done*/
		SplitterSetPauseWithDone(prSpt);

		/* Change Splitter's State*/
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- prSpt(0x%p) Set")
			TEXT(" UEV_PTX_PAUSE evt, change Spt's State to TX_RSPOFF!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

		mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
			SPLITTER_TX_STATE_RSPOFF);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterChange")
				TEXT("State(RUNING, RSPOFF), prSpt: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			SplitterSetEOSForError((void *)prSpt, mrRet);
		}

		prCmd = SplitterGetCmd(prSpt);

		if (NULL != prCmd) {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				(prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
					g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
			if (DMX_CMD_RSP_OFF == prCmd->eCmd) {
				prCmd->mrRet = mrRet;
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
						TEXT(" in SplitterReleaseCmd(prSpt, prCmd->eType=%d)")
						TEXT(", Spt(0x%p)'s state: %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
				mrRet = SplitterReplyCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
						TEXT(" in SplitterReplyCmd(prSpt, prCmd->eType=%d)")
						TEXT(", Spt(0x%p)'s state: %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			} else {
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
						TEXT(" in SplitterReleaseCmd(prSpt, prCmd->eType=%d)")
						TEXT(", Spt(0x%p)'s state: %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}
		}
		MM_RETURN(mrRet);
	} else {
		u64 u8PtxLen = 0;

		SplitterSetPtxNotBusy(prSpt);

		u8PtxLen = SplitterGetPtxLen(prSpt);

		if (SplitterRspIsEnabled(prSpt) &&
			SplitterRspIsRsping(prSpt)) {
			DMXLOG_DEBUG(
				TEXT("[SPT] %s -- SPLITTER_EV_PTX_DONE, Rsping, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, prSpt);
			/* This is used for do resplitter now, tx the next resplitting au to fifo*/
			/* When we do the resplitter, Cfa is Pause, so video will not tx while*/
			/* resplitting*/
			mrRet = SplitterRspTxDone(prSpt, u8PtxLen);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
					TEXT("SplitterRspTxDone (mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
			}
			MM_RETURN(mrRet);
		} else {
			if (SplitterIsCfaPsrEnd(prSpt)) {
				mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_IDLE,
					SPLITTER_TX_STATE_NONE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s fail in Splitter")
						TEXT("ChangeState(IDLE, TX_NONE), prSpt: 0x%p, mrRet")
						TEXT(": 0x%x\r\n"),
						DMX_FUNC_NAME, prSpt, mrRet);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
				MM_RETURN(RET_DMX_OK);
			}

			DMXLOG_DEBUG(
				TEXT("[SPT] %s -- SPLITTER_EV_PTX_DONE, TX_CHECK, prSpt: 0x%p\r\n"),
				DMX_FUNC_NAME, prSpt);

			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_CHECK);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in Splitter")
					TEXT("ChangeState(RUNING, TX_CHECK), prSpt: 0x%p, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, prSpt, mrRet);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}

			if (SplitterIsReResplitter(prSpt)) {
				DMXLOG_TRACE(
					TEXT("[SPT] %s line %d -- Re-resplitter flag is TRUE\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
			}

		#if DMX_PFM_TEST
			DmxPfmPtxDoneEnd(prSpt);
			DmxPfmCfaStart(prSpt);
		#endif /* DMX_PFM_TEST*/

			mrRet = SptCfaSetTxDone(prSpt, u8PtxLen, FALSE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
					TEXT("SplitterRspTxDone (mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}

			if (SPLITTER_TX_STATE_CHECK == prSpt->eSptTxState) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for")
					TEXT(" eSptTxState = TX_STATE_CHECK after Splitter")
					TEXT("RspTxDone (mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

		#if DMX_PFM_TEST
			DmxPfmCfaEnd(prSpt);
		#endif /* DMX_PFM_TEST*/

			if (MM_IS_FFRW_PLAY(prSpt->i4Rate) &&
				SplitterIstPureAudioPIPE(prSpt)) {
				DMX_THREAD_DELAY(1);
			}

			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxDoneNfy(DMX_SPT_INST_T *prSpt)
{
	DMX_CMD_INFO_T *prCmd = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prCmd = SplitterGetCmd(prSpt);
	if (NULL != prCmd) {
		if (DMX_CMD_STM_ENABLE == prCmd->eCmd) {
			DMX_STM_ENABLE_INPUTBUF_T *prInBuf =
				(DMX_STM_ENABLE_INPUTBUF_T *)(prCmd->pvInBuf);

			if (NULL != prInBuf) {
				bool fgEnable = prInBuf->fgEnable;
				DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)(prInBuf->pvStm);

				if (NULL == prStm) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for ")
						TEXT("invalid params, hStm==NULL, prSpt: 0x%p!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					prCmd->mrRet = RET_DMX_PARAM_WRONG;
					SplitterReleaseCmd(prSpt, prCmd);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}

				mrRet = SplitterStmEnable(prStm, fgEnable);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
						TEXT("SplitterStmEnable(prStm: 0x%p, fgEnable: %s)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prStm,
						(fgEnable ? TEXT("TRUE") : TEXT("FALSE")));
					SplitterReleaseCmd(prSpt, prCmd);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}

			prCmd->mrRet = mrRet;
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state:")
					TEXT(" %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			mrRet = SplitterReplyCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ReplyCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state:")
					TEXT(" %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
					g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state:")
					TEXT(" %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		}
	}

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		DMXLOG_DEBUG(
			TEXT("[SPT] %s line %d -- pvSptHdl: 0x%p Previous TX_DONE in IDLE\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		break;
	case SPLITTER_STATE_RUNING:
		mrRet = SplitterProcPtxDoneNfyInRunningState(prSpt);
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxCallNfy(DMX_SPT_INST_T *prSpt)
{
	SplitterState eState = prSpt->eSptState;
	SplitterTxState eTxState = prSpt->eSptTxState;
	MRESULT mrRet = RET_DMX_OK;

	switch (eState) {
	case SPLITTER_STATE_IDLE:
		mrRet = PSR_CC_MainLoop(prSpt->pvPsrCC);
		if (DMX_FAILED(mrRet))
			SplitterSetEOSForError((void *)prSpt, mrRet);
		break;
	case SPLITTER_STATE_RUNING:
		{
			switch (prSpt->eSptTxState) {
			case SPLITTER_TX_STATE_PAUSE:
			case SPLITTER_TX_STATE_RSPOFF:
			case SPLITTER_TX_STATE_ABORT:
			case SPLITTER_TX_STATE_TXING:
			case SPLITTER_TX_STATE_JUMP:
				mrRet = PSR_CC_MainLoop(prSpt->pvPsrCC);
				if (DMX_FAILED(mrRet))
					SplitterSetEOSForError((void *)prSpt, mrRet);
				break;
			default:
				DMXLOG_TRACE(TEXT("[SPT] %s line %d  -- error SptTxState")
					TEXT(" to handle PTX_CALL Nfy, Spt(0x%p)'s state: %d, TxState")
					TEXT(": %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, eState, eTxState);
				mrRet = RET_DMX_ERR_STATE;
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
				break;
			}
			break;
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxPauseNfy(DMX_SPT_INST_T *prSpt)
{
	SplitterState eState = prSpt->eSptState;
	SplitterTxState eTxState = prSpt->eSptTxState;
	DMX_CMD_INFO_T *prCmd = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(
		TEXT("[SPT] %s line %d	-- Spt(0x%p)'s state: %d, TxState: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, eState, eTxState);

	switch (eState) {
	case SPLITTER_STATE_IDLE:
		mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_PAUSE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterChangeState")
				TEXT("(RUNING, TX_CHECK), prSpt: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			SplitterSetEOSForError((void *)prSpt, mrRet);
		}

		prCmd = SplitterGetCmd(prSpt);
		if (NULL != prCmd) {
			if (DMX_CMD_PTX_PAUSE == prCmd->eCmd) {
				prCmd->mrRet = mrRet;
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
						TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state")
						TEXT(": %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
				mrRet = SplitterReplyCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
						TEXT("ReplyCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state")
						TEXT(": %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			} else {
				DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
						g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
						TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s state")
						TEXT(": %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}
		}

		break;
	case SPLITTER_STATE_RUNING:
		{
			switch (eTxState) {
			case SPLITTER_TX_STATE_TXING:
			case SPLITTER_TX_STATE_JUMP:
			case SPLITTER_TX_STATE_PAUSE:
				DMXLOG_DEBUG(
					TEXT("[SPT] %s line %d -- Receive PTX_PAUSE Notify, prSpt: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				DMXLOG_DEBUG(
					TEXT("[SPT] %s line %d -- Set prSpt(0x%p)'s state to be TX_PAUSE!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
					SPLITTER_TX_STATE_PAUSE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterChange")
						TEXT("State(RUNING, TX_CHECK), prSpt: 0x%p, mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, prSpt, mrRet);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
				break;
			case SPLITTER_TX_STATE_ABORT:
				DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- TX_PAUSE State,")
					TEXT(" Receive PTX_PAUSE Notify, prSpt: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				mrRet = RET_DMX_OK;
				break;
			default:
				DMXLOG_ERROR(TEXT("[SPT] %s line %d  -- error SptTxState")
					TEXT(" to handle PTX_PAUSE Nfy, Spt(0x%p)'s state: %d,")
					TEXT(" TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, eState, eTxState);
				mrRet = RET_DMX_ERR_STATE;
				SplitterSetEOSForError((void *)prSpt, mrRet);
				break;
			}

			prCmd = SplitterGetCmd(prSpt);
			if (NULL != prCmd) {
				if (DMX_CMD_PTX_PAUSE == prCmd->eCmd) {
					prCmd->mrRet = mrRet;
					mrRet = SplitterReleaseCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
							TEXT("SplitterReleaseCmd(prSpt, prCmd->eType=%d),")
							TEXT(" Spt(0x%p)'s state: %d, TxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
					mrRet = SplitterReplyCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
							TEXT("SplitterReplyCmd(prSpt, prCmd->eType=%d),")
							TEXT(" Spt(0x%p)'s state: %d, TxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
				} else {
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, (prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
							g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
					mrRet = SplitterReleaseCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
							TEXT("SplitterReleaseCmd(prSpt, prCmd->eType=%d),")
							TEXT(" Spt(0x%p)'s state: %d, TxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
				}
			}
			break;
		}
		break;
	default:
		break;
	}
	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxAbortNfyInRunningState(DMX_SPT_INST_T *prSpt)
{
	DMX_CMD_INFO_T *prCmd = NULL;
	MRESULT mrRet = RET_DMX_OK;

	switch (prSpt->eSptTxState) {
	case SPLITTER_TX_STATE_TXING:
	case SPLITTER_TX_STATE_CHECK:
	case SPLITTER_TX_STATE_PAUSE:
		{
			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_IDLE,
				SPLITTER_TX_STATE_NONE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in Splitter")
					TEXT("ChangeState(IDLE, TX_NONE), prSpt: 0x%p, mrRet: ")
					TEXT("0x%x\r\n"),
					DMX_FUNC_NAME, prSpt, mrRet);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}

			prCmd = SplitterGetCmd(prSpt);
			if (NULL != prCmd) {
				if (DMX_CMD_PTX_OFF == prCmd->eCmd) {
					mrRet = SplitterReleaseCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d ")
							TEXT("fail in SplitterReleaseCmd(prSpt, ")
							TEXT("prCmd->eType=%d), Spt(0x%p)'s state:")
							TEXT(" %d, TxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
					mrRet = SplitterReplyCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
							TEXT(" in SplitterReplyCmd(prSpt, prCmd->eType")
							TEXT("=%d), Spt(0x%p)'s state: %d, TxState: %d")
							TEXT("\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
				} else {
					DMXLOG_DEBUG(
						TEXT("[SPT] %s line %d	-- Cmd: %s\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
							g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN")));
					mrRet = SplitterReleaseCmd(prSpt, prCmd);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d")
							TEXT(" fail in SplitterReleaseCmd(prSpt, ")
							TEXT("prCmd->eType=%d), Spt(0x%p)'s state: %d,")
							TEXT(" TxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
							prSpt->eSptState, prSpt->eSptTxState);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
				}
			}
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxAbortNfy(DMX_SPT_INST_T *prSpt)
{
	DMX_CMD_INFO_T *prCmd = NULL;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d  -- Spt(0x%p)'s state: %d, ")
		TEXT("TxState: %d, prPsrCC's eState: %d, TxState: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState,
		prPsrCC->eState, prPsrCC->eTxState);

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_IDLE:
		prCmd = SplitterGetCmd(prSpt);
		if (NULL != prCmd) {
			if (DMX_CMD_PTX_OFF == prCmd->eCmd) {
				prCmd->mrRet = mrRet;
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
						TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%p)'s ")
						TEXT("state: %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
				mrRet = SplitterReplyCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d fail in SplitterReplyCmd")
						TEXT("(prSpt, prCmd->eType=%d), Spt(0x%p)'s state")
						TEXT(": %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			} else {
				DMXLOG_DEBUG(
					TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
						g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
				mrRet = SplitterReleaseCmd(prSpt, prCmd);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d fail in SplitterReleaseCmd")
						TEXT("(prSpt, prCmd->eType=%d), Spt(0x%p)'s ")
						TEXT("state: %d, TxState: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
						prSpt->eSptState, prSpt->eSptTxState);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}
		}
		break;
	case SPLITTER_STATE_RUNING:
		mrRet = SplitterProcPtxAbortNfyInRunningState(prSpt);
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterSetHeaderInfoInPtxJumpNfy(DMX_SPT_INST_T *prSpt,
	PSR_CC *prPsrCC, PBBUF_SLOT_HEADER_INFO_T *prHeader, BOOL *pfgContinue)
{
	bool fgExistUnCon = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSpt) || (NULL == prPsrCC) ||
		(NULL == prHeader) || (NULL == pfgContinue))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pfgContinue = FALSE;

	if (prPsrCC->u4TxPBBufJumpIdx < MAX_CACHE_PBBUF) {
		DMX_READ_BUFFER *prRdBuf = NULL;

		DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PSR_CC_RelPbbuf")
			TEXT("2UnCon, u4TxPBBufJumpIdx(%d) (prSpt:0x%p).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufJumpIdx, prSpt);
		mrRet = PSR_CC_RelPbbuf2UnCon(prPsrCC, prPsrCC->u4TxPBBufJumpIdx);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC")
				TEXT("_RelPbbuf2UnCon(TxPBBufJumpIdx: %d), PsrCC: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufJumpIdx, prPsrCC);
			SplitterSetEOSForError((void *)prSpt, mrRet);
			MM_RETURN(mrRet);
		}

		prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;

		mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));

		prRdBuf = &(prPsrCC->arPBBuf[0]);

		mm_memcpy(prHeader, &(prRdBuf->rHeader),
			sizeof(PBBUF_SLOT_HEADER_INFO_T));
	} else if (MAX_CACHE_PBBUF == prPsrCC->u4TxPBBufIdx) {
		DMX_READ_BUFFER *prRdBuf = NULL;
		u32 u4PbbufIdx = 0;

		DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PBBUF_GetFstSent")
			TEXT("SlotHeader, JumpIdx(%d), TxIdx(%d) (prSpt:0x%p).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prPsrCC->u4TxPBBufJumpIdx,
			prPsrCC->u4TxPBBufIdx, prSpt);
		mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));
		mrRet = PSR_CC_GetPBBufSlot(prPsrCC, &u4PbbufIdx);
		if (DMX_FAILED(mrRet)) {
			SplitterPsrSetEOS(prPsrCC->pvSptHdl, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}

		if (u4PbbufIdx >= MAX_CACHE_PBBUF) {
			prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
			prPsrCC->u4TxPBBufIdx = MAX_CACHE_PBBUF;

			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_JUMP);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
					TEXT("SplitterChangeState(RUNING, JUMP), prSpt: 0x%p,")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- exit for ")
				TEXT("u4PbbufIdx >= MAX_CACHE_PBBUF, prSpt: 0x%p.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			MM_RETURN(RET_DMX_OK);
		}

		mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));

		prRdBuf = &(prPsrCC->arPBBuf[u4PbbufIdx]);

		mm_memcpy(prHeader, &(prRdBuf->rHeader), sizeof(PBBUF_SLOT_HEADER_INFO_T));
		if (PBBUF_SLOT_NORMAL == prHeader->eType) {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d WARNING -- ")
				TEXT("the header info obtained by PBBUF_GetFstSentSlotHeader")
				TEXT(" is INVALID(prSpt:0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
			prPsrCC->u4TxPBBufIdx = MAX_CACHE_PBBUF;
			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_JUMP);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
					TEXT("SplitterChangeState(RUNING, JUMP), prSpt: 0x%p,")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			PSR_CC_CBSplitter(prPsrCC, E_TX_JUMP, NULL);
			MM_RETURN(RET_DMX_OK);
		}
	} else {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PSR_CC_Rel")
			TEXT("PbbufAcrossSlot2UnCon, u4TxPBBufIdx(%d) (prSpt:0x%p).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx, prSpt);

		mrRet = PSR_CC_RelPbbufAcrossSlot2UnCon(prPsrCC, prPsrCC->u4TxPBBufIdx,
			&fgExistUnCon);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC")
				TEXT("_RelPbbufAcrossSlot2UnCon (mrRet:0x%x, prSpt:0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
			SplitterSetEOSForError((void *)prSpt, mrRet);
			MM_RETURN(mrRet);
		}

		if (fgExistUnCon) {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PSR_CC_")
				TEXT("RelPbbufAcrossSlot2UnCon(prSpt: 0x%p) fgExistUnCon ")
				TEXT("is TRUE\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			if (prPsrCC->u4TxPBBufJumpIdx < MAX_CACHE_PBBUF) {
				DMX_READ_BUFFER *prRdBuf = NULL;

				DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PSR_CC")
					TEXT("_RelPbbuf2UnCon, u4TxPBBufJumpIdx(%d) (prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufJumpIdx, prSpt);
				mrRet = PSR_CC_RelPbbuf2UnCon(prPsrCC, prPsrCC->u4TxPBBufJumpIdx);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail")
						TEXT(" in PSR_CC_RelPbbuf2UnCon(TxPBBufJumpIdx: %d),")
						TEXT(" PsrCC: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrCC->u4TxPBBufJumpIdx, prPsrCC);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}

				prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;

				mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));

				prRdBuf = &(prPsrCC->arPBBuf[0]);

				mm_memcpy(prHeader, &(prRdBuf->rHeader),
					sizeof(PBBUF_SLOT_HEADER_INFO_T));
			} else {
				DMX_READ_BUFFER *prRdBuf = NULL;
				u32 u4PbbufIdx = 0;

				DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PBBUF")
					TEXT("_GetFstSentSlotHeader, JumpIdx(%d), TxIdx(%d) ")
					TEXT("(prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prPsrCC->u4TxPBBufJumpIdx,
					prPsrCC->u4TxPBBufIdx, prSpt);

				mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));
				mrRet = PSR_CC_GetPBBufSlot(prPsrCC, &u4PbbufIdx);
				if (DMX_FAILED(mrRet)) {
					SplitterPsrSetEOS(prPsrCC->pvSptHdl, GAU_E_FAIL);
					MM_RETURN(mrRet);
				}

				if (u4PbbufIdx >= MAX_CACHE_PBBUF) {
					prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
					prPsrCC->u4TxPBBufIdx = MAX_CACHE_PBBUF;

					mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
						SPLITTER_TX_STATE_JUMP);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d ")
							TEXT("fail in SplitterChangeState(RUNING, ")
							TEXT("JUMP), prSpt: 0x%p, mrRet: 0x%x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
					PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- ")
						TEXT("exit for u4PbbufIdx >= MAX_CACHE_PBBUF, ")
						TEXT("prSpt: 0x%p.\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					MM_RETURN(RET_DMX_OK);
				}

				mm_memset(prHeader, 0, sizeof(PBBUF_SLOT_HEADER_INFO_T));

				prRdBuf = &(prPsrCC->arPBBuf[u4PbbufIdx]);

				mm_memcpy(prHeader, &(prRdBuf->rHeader),
					sizeof(PBBUF_SLOT_HEADER_INFO_T));

				if (PBBUF_SLOT_NORMAL == prHeader->eType) {
					DMXLOG_DEBUG(TEXT("[SPT] %s line %d WARNING")
						TEXT(" -- the header info obtained by PBBUF_GetFst")
						TEXT("SentSlotHeader is INVALID(prSpt:0x%p).\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
					prPsrCC->u4TxPBBufIdx = MAX_CACHE_PBBUF;

					mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
						SPLITTER_TX_STATE_JUMP);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s line %d fail")
							TEXT(" in SplitterChangeState(RUNING, JUMP), ")
							TEXT("prSpt: 0x%p, mrRet: 0x%x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
						SplitterSetEOSForError((void *)prSpt, mrRet);
						MM_RETURN(mrRet);
					}
					PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
					PSR_CC_CBSplitter(prPsrCC, E_TX_JUMP, NULL);
					MM_RETURN(RET_DMX_OK);
				}
			}
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s line %d --> PSR_CC_Rel")
				TEXT("PbbufAcrossSlot2UnCon(prSpt: 0x%p) fgExistUnCon is ")
				TEXT("FALSE\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
			prPsrCC->u4TxPBBufIdx = MAX_CACHE_PBBUF;

			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_JUMP);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
					TEXT("SplitterChangeState(RUNING, JUMP), prSpt: 0x%p,")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			MM_RETURN(RET_DMX_OK);
		}
	}

	*pfgContinue = TRUE;

	MM_RETURN(mrRet);
}

MRESULT SplitterProcPtxJumpNfy(DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;
	DMX_CMD_INFO_T *prCmd = NULL;
	DMX_INST_T *prDmxInst = NULL;
	PBBUF_SLOT_HEADER_INFO_T rHeader;

	if (NULL == prSpt) {
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;
	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	prCmd = SplitterGetCmd(prSpt);
	if (NULL != prCmd) {
		if (DMX_CMD_STM_ENABLE == prCmd->eCmd) {
			DMX_STM_ENABLE_INPUTBUF_T *prInBuf =
				(DMX_STM_ENABLE_INPUTBUF_T *)(prCmd->pvInBuf);

			if (NULL != prInBuf) {
				bool fgEnable = prInBuf->fgEnable;
				DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)(prInBuf->pvStm);

				if (NULL == prStm) {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d fail for invalid params, ")
						TEXT("hStm==NULL, prSpt: 0x%p!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					prCmd->mrRet = RET_DMX_PARAM_WRONG;
					SplitterReleaseCmd(prSpt, prCmd);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}

				mrRet = SplitterStmEnable(prStm, fgEnable);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d fail in Splitter")
						TEXT("StmEnable(hStm: 0x%x, fgEnable: %s)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prStm,
						(fgEnable ? TEXT("TRUE") : TEXT("FALSE")));
					prCmd->mrRet = RET_DMX_PARAM_WRONG;
					SplitterReleaseCmd(prSpt, prCmd);
					SplitterSetEOSForError((void *)prSpt, mrRet);
					MM_RETURN(mrRet);
				}
			}

			prCmd->mrRet = mrRet;
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ReleaseCmd(prSpt, prCmd->eType=%d), Spt(0x%x)'s state:")
					TEXT(" %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
			mrRet = SplitterReplyCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in SplitterReplyCmd")
					TEXT("(prSpt, prCmd->eType=%d), Spt(0x%x)'s state:")
					TEXT(" %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		} else {
			DMXLOG_DEBUG(
				TEXT("[SPT] %s line %d  -- Cmd: %s\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (prCmd->eCmd < MAX_OF_DMX_CMD_TYPE_CNT) ?
					g_awszCmdName[prCmd->eCmd] : TEXT("UNKNOWN"));
			mrRet = SplitterReleaseCmd(prSpt, prCmd);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in SplitterReleaseCmd")
					TEXT("(prSpt, prCmd->eType=%d), Spt(0x%x)'s state")
					TEXT(": %d, TxState: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCmd->eCmd, prSpt,
					prSpt->eSptState, prSpt->eSptTxState);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		}
	}

	switch (prSpt->eSptState) {
	case SPLITTER_STATE_RUNING:
		if (SPLITTER_TX_STATE_JUMP != prSpt->eSptTxState) {
			DMXLOG_ERROR(
				TEXT("[PSR] %s line %d fail for error SptTxState(%d) to do JUMP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt->eSptTxState);
			SplitterSetEOSForError((void *)prSpt, RET_DMX_ERR_STATE);
			MM_RETURN(RET_DMX_ERR_STATE);
		}

		prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

		if (CFA_TYPE_AUDIN == SplitterGetCfaType(prSpt)) {
			if (MM_IS_NORMAL_PLAY(prSpt->i4Rate)) {
				DMXLOG_TRACE(
					TEXT("[SPT] %s line %d -- Jump while normal play, pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			} else {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail for CfaAudIn don't support FF/RW,")
					TEXT(" pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				SplitterSetEOSForError((void *)prSpt, RET_DMX_UNEXPECT);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		} else {
			if (!MM_IS_FFRW_PLAY(prSpt->i4Rate)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d failed for FR/BR")
					TEXT(" rate = %d, so should not JUMP, pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt->i4Rate, prSpt);
				SplitterSetEOSForError((void *)prSpt, RET_DMX_UNEXPECT);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		}

		prDmxInst->fgPsrOff = FALSE;

		DMXLOG_DEBUG(
			TEXT("[SPT] %s line %d -- NEED DO JUMP FLOW, pvSptHdl: 0x%p.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

		if (NULL != prPsrCC) {
			BOOL fgContinue = FALSE;

			PSR_CC_LOCK(prPsrCC->rLock);
			SplitterSetPtxNotBusy(prSpt);

			if (CCS_PAUSE == prPsrCC->eState) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Jump while PsrCC")
					TEXT(" enter to Pause State, pvSptHdl: 0x%p.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_OK);
			}

			fgContinue = FALSE;
			mrRet = SplitterSetHeaderInfoInPtxJumpNfy(prSpt, prPsrCC,
				&rHeader, &fgContinue);
			if (DMX_FAILED(mrRet)) {
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			} else if (!fgContinue) {
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}

			switch (rHeader.eType) {
			case PBBUF_SLOT_NORMAL:
				{
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for the")
						TEXT(" header info obtained by PBBUF_GetFstSentSlotHeader")
						TEXT(" is INVALID(pvSptHdl:0x%x).\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					SplitterSetEOSForError((void *)prSpt, RET_DMX_UNEXPECT);
					PSR_CC_UNLOCK(prPsrCC->rLock);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				break;
			case PBBUF_SLOT_JUMP:
				break;
			case PBBUF_SLOT_END:
				{
					DMXLOG_TRACE(TEXT("[SPT] %s line %d --- prSpt(0x%x)")
						TEXT(" : This file jumps to the first frame, so send EOS")
						TEXT(" to end playback\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
					mrRet = SplitterPsrSetEOS(prSpt, GAU_E_EOS);
					DMXLOG_DEBUG(TEXT("[SPT] %s -- PsrCC's state(%d),")
						TEXT(" etxstate(%d), pvSptHdl: 0x%p\r\n"),
						DMX_FUNC_NAME, prPsrCC->eState, prPsrCC->eTxState, prSpt);

					PSR_CC_UNLOCK(prPsrCC->rLock);
					MM_RETURN(RET_DMX_OK);
				}
				break;
			default:
				{
					DMXLOG_TRACE(TEXT("[SPT] %s line %d --- prSpt(0x%x)")
						TEXT(" : Unknown PBBUF Slot Header Type(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prSpt, rHeader.eType);
					mrRet = SplitterPsrSetEOS(prSpt, GAU_E_EOS);
					PSR_CC_UNLOCK(prPsrCC->rLock);
					MM_RETURN(RET_DMX_OK);
				}
				break;
			}

			if ((NULL != prPsrCC) &&
				(CCS_PAUSE == prPsrCC->eState)) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Jump while ")
					TEXT("PsrCC enter to Pause State, pvSptHdl: 0x%p.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_OK);
			}

			mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_CHECK);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ChangeState(RUNING, TX_CHECK), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}

			DMXLOG_TRACE(
				TEXT("[SPT] %s line %d -- SptCfaSetJumpInfo, pvSptHdl: 0x%p.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

			mrRet = SptCfaSetJumpInfo(prSpt, rHeader.pvParam);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SptCfaSet")
					TEXT("JumpInfo (mrRet:0x%x, pvSptHdl:0x%x).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}

			mrRet = SplitterSetCfaPsrEnd(prSpt, FALSE, 0);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSetCfa")
					TEXT("PsrEnd(pvSptHdl: 0x%p, FALSE)!\r\n"),
					DMX_FUNC_NAME, prSpt);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}

			prSpt->u8OtherAudioPts		= INVALID_TIMESTAMP;
			prSpt->pvOtherAudioSptHandle = NULL;
			prSpt->ucAudMaxDuration		= 0;
			prSpt->ucRspTxHdrState		= RSP_TXHDRINFO_IDLE;
			prSpt->ucRspTxAStmHdrState	= RSP_TX_AHEADS_IDLE;
			prSpt->u8PtxFromFileOffset	= 0;
			prSpt->pvPtxFromDramAddress = NULL;
			prSpt->u8PtxLen				= 0;

			prSpt->fgDivxDRMOn			= FALSE;
			prSpt->u8DivxDRMOffset		= DMX_INVALID_UINT64;
			prSpt->u4DecLen				= 0;
			prSpt->u2FrameKeyIndex		= DMX_DIVXDRM_INVALID_FRAMEIDX;

			prSpt->fgPtxBusy			= FALSE;

			mrRet = PSR_CC_Reset4NonConPbbufSlot(prPsrCC);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC_")
					TEXT("Reset4NonConPbbufSlot (mrRet:0x%x, pvSptHdl:0x%x).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(mrRet);
			}

			PSR_CC_UNLOCK(prPsrCC->rLock);

			mrRet = SptCfaSetTurnOn(prSpt);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PBBUF_")
					TEXT("GetFstSentSlotHeader (mrRet:0x%x, pvSptHdl:0x%x).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for error SptState")
			TEXT("(%s, %s) to do JUMP, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prSpt->eSptState <= SPLITTER_STATE_RUNING) ?
				g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
			((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ?
				g_awszDmxSptTxStatus[prSpt->eSptTxState] : TEXT("UNKNOWN")));
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterProcNfy(EV_GRP_EVENT_T u8Events, DMX_SPT_INST_T *prSpt)
{
	MRESULT mrRet = RET_DMX_OK;

	MM_ASSERT(NULL != prSpt);

	if (0 != (SPLITTER_EV_NFY_HW_CB & u8Events))
		mrRet = SplitterProcHwCBNfy(prSpt);

	if (0 != (SPLITTER_EV_NFY_PTX_DONE & u8Events))
		mrRet = SplitterProcPtxDoneNfy(prSpt);

	if (0 != (SPLITTER_EV_NFY_PTX_CALL & u8Events))
		mrRet = SplitterProcPtxCallNfy(prSpt);

	if (0 != (SPLITTER_EV_NFY_PTX_PAUSE & u8Events))
		mrRet = SplitterProcPtxPauseNfy(prSpt);

	if (0 != (SPLITTER_EV_NFY_PTX_ABORT & u8Events))
		mrRet = SplitterProcPtxAbortNfy(prSpt);

	if (0 != (SPLITTER_EV_NFY_PTX_JUMP & u8Events))
		mrRet = SplitterProcPtxJumpNfy(prSpt);

	MM_RETURN(mrRet);
}

static MRESULT SplitterMainLoop(DMX_SPT_INST_T *prSpt)
{
	EV_GRP_EVENT_T u8Events = 0;
	MRESULT mrRet = RET_DMX_OK;

	/* Do nothing, but wait Parser set event.*/
	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		SplitterSetEOSForError((void *)prSpt, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = SplitterGetEvent(prSpt,
		SPLITTER_ALL_EVS,
		&u8Events,
		DMX_WAIT_INFINITE);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] -------- %s line %d -- Got Events ")
			TEXT("Failed (mrRet:0x%x, prSpt:0x%p). --------\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
		SplitterSetEOSForError((void *)prSpt, mrRet);
	}

	if (0 != (SPLITTER_EV_SPT_CMD_IN & u8Events)) {
		DMXLOG_DEBUG(TEXT("[SPT] -------- %s line %d -- Got SPLITTER")
			TEXT("_EV_SPT_CMD_IN (prSpt:0x%p). --------\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		mrRet = SplitterProcCmd(prSpt);
		if (DMX_FAILED(mrRet)) {
			if (MM_IS_STATE_ERROR(mrRet)) {
				mrRet = RET_DMX_OK;
			} else {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d failed in Splitter")
					TEXT("ProcCmd, (SPT_CMD_IN, mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
			}
		}
	}

	if (0 != (SPLITTER_ALL_NFY_EVS & u8Events)) {
		mrRet = SplitterProcNfy(u8Events, prSpt);
		if (DMX_FAILED(mrRet)) {
			if (MM_IS_STATE_ERROR(mrRet)) {
				mrRet = RET_DMX_OK;
			} else {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d failed in Splitter")
					TEXT("ProcNfy, (mrRet:0x%x, prSpt:0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prSpt);
				SplitterSetEOSForError((void *)prSpt, mrRet);
			}
		}
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterForeverLoop*/
/* Splitter's main thread*/
/*/////////////////////////////////////////////////////////////////////////////*/
#ifdef __linux__
int  SplitterForeverLoop(void *pv_arg)
#else
void SplitterForeverLoop(void *pv_arg)
#endif
{
	DMX_SPT_INST_T *prSpt = ((NULL != pv_arg) ? ((DMX_SPT_INST_T *)pv_arg) : NULL);

	u32 u4MainCounter = 0;
	MRESULT mrRet = RET_DMX_OK;

	#ifndef __linux__
	u32	dwThreadID = GetCurrentThreadId();

	DMXLOG_TRACE(TEXT("[SPT] *********Enter Splitter Thread******* ")
		TEXT("(Thread ID:0x%x, pvSptHdl: 0x%p).\r\n"),
		dwThreadID, prSpt);
	#else
	DMXLOG_TRACE(TEXT("[SPT] *********Enter Splitter Thread******* ")
		TEXT("(pvSptHdl: 0x%p).\r\n"),
		prSpt);
	#endif /* __linux__*/

	if (NULL == prSpt) {
		DMXLOG_ERROR(TEXT("[SPT] SplitterForeverLoop -- prSpt is ")
			TEXT("NULL, so return!!! \r\n"));
#ifdef __linux__
		return 0;
#else
		return;
#endif
	}

	while (!prSpt->fgExitThread) {
		u4MainCounter++;
		mrRet = SplitterMainLoop(prSpt);
		/* if the fail-reason is no memory or no resource, we will exit the thread*/
		/* otherwise, it may causes dead-circularation,*/
		/* e.g. if the function SplitterGetEvent fail in the SplitterIdleState*/
		/* function, dead-circularation will appear.*/
		if (DMX_ERR_IS_NO_RES(mrRet))
			prSpt->fgExitThread = TRUE;
	}

	DMXLOG_DEBUG(TEXT("[SPT] Exit Main thread loop process (pvSptHdl: 0x%p), ")
		TEXT("now wait kthread_should_stop flag.\r\n"),
		prSpt);

#ifdef __linux__
	while (TRUE) {
		prSpt->waitexitqueueflag = 0;
		/*HZ/50=20ms */
		wait_event_interruptible_timeout(prSpt->waitexitqueue,
				prSpt->waitexitqueueflag, HZ / 50);
		if (kthread_should_stop())
				break;
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d wait for kthread_should_stop, ")
			TEXT("(pvSptHdl: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
	}
#endif /* __linux__*/

	DMXLOG_DEBUG(TEXT("[SPT] get kthead_should_stop flag, ")
		TEXT("now *********Exit Splitter Thread******* (pvSptHdl: 0x%p)\r\n"),
		prSpt);

#ifdef __linux__
	return 0;
#else
	return;
#endif
}



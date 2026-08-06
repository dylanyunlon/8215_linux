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
 * @file dmx_spt_os.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Splitter Os macros, events, structures, interfaces declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_SPT_OS_H
#define DMX_INTERNAL_SPT_OS_H

#include "x_typedef.h"
#include "dmx_spt.h"
#include "u_os.h"

#ifdef __cplusplus
extern "C" {

#endif

#define SPT_EVENT_NAME                 "SPT0_EVENT"
#define SPT_USR_EVENT_NAME             "SPT0UEVENT"
#define SPT_THREAD_NAME                "SPT0_TH"

#define SPT_MSGQ_NAME                 _T("SPT0_MSGQ")
#define SPT_MSGQ_SZ                   2
#define SPT_MSGQ_PRIORITY             1
#define SPT_MSGQ_RETRY                20

#define SPT_STACK_SIZE                16384
#ifdef __linux__
#define SPT_PRIORITY                  -6
#else				/* __linux__ */
#define SPT_PRIORITY                  200
#endif				/* __linux__ */
#define MAX_OF_SPT_NAME_LEN           16

/*! @name Splitter OS Type Define (6.3) */
/*! @{ */

/* Splitter Internal Event */
#define SPLITTER_EV_INITIAL                     ((EV_GRP_EVENT_T) 0)
#define SPLITTER_EV_SPT_CMD_IN                  ((EV_GRP_EVENT_T)(1) << 1)
#define SPLITTER_EV_NFY_HW_CB                   ((EV_GRP_EVENT_T)(1) << 10)
#define SPLITTER_EV_NFY_PTX_DONE                ((EV_GRP_EVENT_T)(1) << 11)
#define SPLITTER_EV_NFY_PTX_PAUSE               ((EV_GRP_EVENT_T)(1) << 12)
#define SPLITTER_EV_NFY_PTX_ABORT               ((EV_GRP_EVENT_T)(1) << 13)
#define SPLITTER_EV_NFY_PTX_JUMP                ((EV_GRP_EVENT_T)(1) << 14)
#define SPLITTER_EV_NFY_PTX_CALL                ((EV_GRP_EVENT_T)(1) << 15)
#define SPLITTER_EV_ERROR_FIXED                 ((EV_GRP_EVENT_T)(1) << 63)

#define SPLITTER_ALL_NFY_EVS			(SPLITTER_EV_NFY_HW_CB	| \
						SPLITTER_EV_NFY_PTX_DONE	| \
						SPLITTER_EV_NFY_PTX_JUMP	| \
						SPLITTER_EV_NFY_PTX_ABORT	| \
						SPLITTER_EV_NFY_PTX_PAUSE	| \
						SPLITTER_EV_NFY_PTX_CALL)
#define SPLITTER_ALL_EVS			(SPLITTER_EV_SPT_CMD_IN    | \
						SPLITTER_ALL_NFY_EVS)
/*! Splitter User (MPC) Event */
#define SPLITTER_UEV_INITIAL			((EV_GRP_EVENT_T) 0)
#define SPLITTER_UEV_PTX_ON			((EV_GRP_EVENT_T)(1) << 1)
#define SPLITTER_UEV_PTX_OFF			((EV_GRP_EVENT_T)(1) << 2)
#define SPLITTER_UEV_PTX_PAUSE			((EV_GRP_EVENT_T)(1) << 3)
#define SPLITTER_UEV_PTX_RESUME			((EV_GRP_EVENT_T)(1) << 4)
#define SPLITTER_UEV_EXIT			((EV_GRP_EVENT_T)(1) << 5)
#define SPLITTER_UEV_RSP_OFF			((EV_GRP_EVENT_T)(1) << 6)
#define SPLITTER_UEV_RSP_REBUF			((EV_GRP_EVENT_T)(1) << 7)
#define SPLITTER_UEV_RSP_ON			((EV_GRP_EVENT_T)(1) << 8)
#define SPLITTER_UEV_STM_ENABLE                 ((EV_GRP_EVENT_T)(1) << 16)
#define SPLITTER_UEV_SUSPEND_OK                 ((EV_GRP_EVENT_T)(1) << 24)
#define SPLITTER_UEV_ERROR_FIXED                ((EV_GRP_EVENT_T)(1) << 63)

/*! Splitter Internal Msg for handling MW command request */
typedef struct {
	u32 e_code;
} SPT_MSG_T;

/* Splitter commands */
typedef enum {
SPT_CMD_NONE, SPT_CMD_PAUSE, PT_CMD_RESUME, SPT_CMD_RSP
/* SPT_MSG_CALLPSR // for victor parser send tx_done in the end of a loop, ignore it for too much delay */
} SptCmd;

/*! @} */
MRESULT SplitterCreateEvent(DMX_SPT_INST_T *prSpt, u32 u4Idx);
MRESULT SplitterDeleteEvent(DMX_SPT_INST_T *prSpt);
MRESULT SplitterSetEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4SetEvent);
MRESULT SplitterGetEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4WaitOnEvent,
	EV_GRP_EVENT_T *pu4ReceiveEvent, u32 u4WaitTimeout);
MRESULT SplitterResetAllEvents(DMX_SPT_INST_T *prSpt);
MRESULT SplitterGetUsrEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4WaitOnEvent,
	EV_GRP_EVENT_T *pu4ReceiveEvent, u32 u4WaitTimeout);
MRESULT SplitterSetUsrEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u8SetEvent);
MRESULT SplitterWaitUsrEvent(DMX_SPT_INST_T *prSpt, EV_GRP_EVENT_T u4WaitOnEvent,
	u32 u4TimeOut);
MRESULT SplitterCreateTask(DMX_SPT_INST_T *prSpt, u32 u4SptIdx);
MRESULT SplitterDeleteTask(DMX_SPT_INST_T *prSpt);
MRESULT SplitterCreateCmdQ(DMX_SPT_INST_T *prSpt, u32 u4SptIdx);
MRESULT SplitterDeleteCmdQ(DMX_SPT_INST_T *prSpt);
MRESULT SplitterCreateSema(DMX_SPT_INST_T *prSpt);
MRESULT SplitterDeleteSema(DMX_SPT_INST_T *prSpt);
MRESULT SplitterLockSema(DMX_SPT_INST_T *prSpt);
MRESULT SplitterReleaseSema(DMX_SPT_INST_T *prSpt);
void SplitterSetRspOffTimeout(HANDLE_T hTimer, void *pvTag);
MRESULT SplitterSleepTask(DMX_SPT_INST_T *prSpt, u32 u4MiniSec);
MRESULT SplitterSendCmd(DMX_SPT_INST_T *prSpt, DMX_CMD_INFO_T *prCmd);
MRESULT SplitterSendNfy(DMX_SPT_INST_T *prSpt, E_DMX_SPT_NTY_TYPE_T eNfy);
MRESULT SplitterChangeStateEx(DMX_SPT_INST_T *prSpt, SplitterState eState,
	SplitterTxState eTxState, const char *wszFunc, s32 i4Line);

#define SplitterChangeState(hSpt, eState, eTxState)	\
	SplitterChangeStateEx(hSpt, eState, eTxState, TEXT(__func__), __LINE__)

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef DMX_INTERNAL_SPT_OS_H */

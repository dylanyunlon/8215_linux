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

/*!
 * @file dmx_gau_queue.h
 *
 * @par Project
 *
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef _DMX_GAU_QUEUE_H_
#define _DMX_GAU_QUEUE_H_

#include "windows.h"
#include "drv_win32_if.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/perf_timer.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "drv_esm_if.h"
#include "perf_timer.h"
#include "mm_debug.h"
#endif /* __linux__ */

#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GAU_Q_STATUS_NONE,
	GAU_Q_STATUS_IDLE,
	GAU_Q_STATUS_INUSE,
	GAU_Q_STATUS_INFREE,
	GAU_Q_STATUS_MAX
} E_GAU_Q_STATUE_T;

typedef struct _gqu_q_elem_t {
	u32	u4AuIdx;
	bool	fgInUsing;
	struct _gqu_q_elem_t *prPrev;
	struct _gqu_q_elem_t *prNext;
} GAU_Q_ELEM_T;

typedef struct _gqu_q_elem_list_t {
	u32	u4Cnt;
	GAU_Q_ELEM_T *prTail;
	GAU_Q_ELEM_T *prHead;
} GAU_Q_ELEM_LIST_T;

typedef struct {
	E_GAU_Q_STATUE_T eStatus;
	HANDLE		 hSema;
	u32		 u4Handle;
	u32		 u4MaxCnt;
	GAU_Q_ELEM_T *prElemTable;
	GAU_Q_ELEM_LIST_T rInUseList;
	GAU_Q_ELEM_LIST_T rFreeList;
} GAU_Q_T;

void	GAU_Q_Init(GAU_Q_T *prQueue, u32 u4Handle);
void	GAU_Q_UnInit(GAU_Q_T *prQueue);

MRESULT GAU_Q_Create(GAU_Q_T *prQueue, u32 u4QueueElemCnt);
MRESULT GAU_Q_Release(GAU_Q_T *prQueue);
MRESULT GAU_Q_Flush(GAU_Q_T *prQueue);
MRESULT GAU_Q_GetAU(GAU_Q_T *prQueue, u32 u4AUIdx);
MRESULT GAU_Q_ReleaseAU(GAU_Q_T *prQueue, u32 u4AUIdx);
u32	GAU_Q_GetFstInUseAUIdx(GAU_Q_T *prQueue);
u32 GAU_Q_GetInUseAUIdx(GAU_Q_T *prQueue, GAU_Q_ELEM_T *prElem);
bool	GAU_Q_IsFstInUseAUIdx(GAU_Q_T *prQueue, u32 u4AUIdx);
MRESULT GAU_Q_ReleaseTailInUseAU(GAU_Q_T *prQueue);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef _DMX_GAU_QUEUE_H_ */


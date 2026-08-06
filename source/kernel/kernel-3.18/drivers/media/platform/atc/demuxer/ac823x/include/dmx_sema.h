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
 * @file dmx_sema.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Splitter Structures, Macros declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_SEMA_H
#define DMX_INTERNAL_SEMA_H

#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DMX_SEMA_STATE_LOCK	  0
#define DMX_SEMA_STATE_UNLOCK  1

enum dmx_sema_option {
	DMX_SEMA_OPTION_WAIT = 1,
	DMX_SEMA_OPTION_NOWAIT
};

enum dmx_sema_type {
	DMX_SEMA_TYPE_BINARY = 1,
	DMX_SEMA_TYPE_MUTEX,
	DMX_SEMA_TYPE_COUNTING
};

struct dmx_sema_node {
	enum dmx_sema_type e_type;
	struct semaphore sem;
	struct task_struct *task;
	s16 i2_selfcount;
};

void dmx_sema_init(void);
void dmx_sema_deinit(void);
MRESULT dmx_sema_create(void **sema, enum dmx_sema_type e_type, int initval);
MRESULT dmx_sema_delete(void *sema);

#define DEBUG_DMX_SEMA 0

#if DEBUG_DMX_SEMA
#define dmx_sema_lock(a, b) dmx_sema_lockex(a, b, __func__, __LINE__)
MRESULT dmx_sema_lockex(void *sema, enum dmx_sema_option option,
	const char *szFunc,	int i4line);
#define dmx_sema_lock_timeout(a, b) dmx_sema_lock_timeoutex(a, b, __func__, __LINE__)
MRESULT dmx_sema_lock_timeoutex(void *sema, int waittime,
	const char *szFunc,	int i4line);
#define dmx_sema_unlock(a) dmx_sema_unlockex(a, __func__, __LINE__)
MRESULT dmx_sema_unlockex(void *sema,
	const char *szFunc,	int i4line);
#else
MRESULT dmx_sema_lock(void *sema, enum dmx_sema_option option);
MRESULT dmx_sema_lock_timeout(void *sema, int waittime);
MRESULT dmx_sema_unlock(void *sema);
#endif

#ifdef __cplusplus
}
#endif

#endif


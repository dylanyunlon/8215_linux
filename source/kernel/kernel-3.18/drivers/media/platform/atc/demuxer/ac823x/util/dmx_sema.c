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



#include <asm/current.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include "dmx_def.h"
#include "dmx_errcode.h"
#include "dmx_sema.h"

#define INVALID_TASK ((struct task_struct *)(NULL))

#define pr_fmt(fmt) "[MM]["KBUILD_MODNAME"]" fmt


void dmx_sema_init(void)
{
}

void dmx_sema_deinit(void)
{
}

MRESULT dmx_sema_create(void **sema, enum dmx_sema_type e_type, int initval)
{
	struct dmx_sema_node *pt_sema;

	/* check arguments */
	if ((sema == NULL) ||
	    ((e_type != DMX_SEMA_TYPE_BINARY) && (e_type != DMX_SEMA_TYPE_MUTEX) &&
	     (e_type != DMX_SEMA_TYPE_COUNTING))) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (((e_type == DMX_SEMA_TYPE_BINARY) || (e_type == DMX_SEMA_TYPE_MUTEX)) &&
	    (initval != DMX_SEMA_STATE_LOCK) && (initval != DMX_SEMA_STATE_UNLOCK)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((e_type == DMX_SEMA_TYPE_COUNTING) &&
	    (((s32) initval) < ((s32) DMX_SEMA_STATE_LOCK))) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pt_sema = kcalloc(1, sizeof(struct dmx_sema_node), GFP_KERNEL);
	if (pt_sema == NULL)
		MM_RETURN(RET_DMX_NO_MEM);

	pt_sema->e_type = e_type;
	sema_init(&(pt_sema->sem), initval);

	*sema = (void *) (pt_sema);
	MM_RETURN(RET_DMX_OK);
}

MRESULT dmx_sema_delete(void *sema)
{
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;

	if (sema == NULL)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	kfree(semanode);
	MM_RETURN(RET_DMX_OK);
}

#if DEBUG_DMX_SEMA
MRESULT dmx_sema_lockex(void *sema, enum dmx_sema_option option,
	const char *szFunc,	int i4line)
{
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;

	if ((option != DMX_SEMA_OPTION_WAIT) && (option != DMX_SEMA_OPTION_NOWAIT))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	pr_info("dmx_sema_lock, func: %s, line %d\r\n", szFunc, i4line);
	if (NULL == semanode) {
		pr_err("dmx_sema_lock_timeout return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	down(&(semanode->sem));
	MM_RETURN(RET_DMX_OK);
}

MRESULT dmx_sema_lock_timeoutex(void *sema, int waittime,
	const char *szFunc,	int i4line)
{
	static const int quantum_ms = 1000 / HZ;
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;
	MRESULT ret;

	pr_info("dmx_sema_lock_timeout, func: %s, line %d\r\n", szFunc, i4line);
	if (NULL == semanode) {
		pr_err("dmx_sema_lock_timeout return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	ret = down_timeout(&semanode->sem, waittime / quantum_ms);
	if (ret != 0)
		goto err;

	MM_RETURN(RET_DMX_OK);

err:

	switch (ret) {
	case -ETIME:
		MM_RETURN(RET_DMX_TIMEOUT);

	default:
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
}

MRESULT dmx_sema_unlockex(void *sema,
	const char *szFunc,	int i4line)
{
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;

	pr_info("dmx_sema_unlock, func: %s, line %d\r\n", szFunc, i4line);
	if (NULL == semanode) {
		pr_err("dmx_sema_unlock return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	up(&(semanode->sem));
	MM_RETURN(RET_DMX_OK);
}

#else /* DEBUG_DMX_SEMA */
MRESULT dmx_sema_lock(void *sema, enum dmx_sema_option option)
{
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;

	if ((option != DMX_SEMA_OPTION_WAIT) && (option != DMX_SEMA_OPTION_NOWAIT))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (NULL == semanode) {
		pr_err("dmx_sema_lock return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	down(&(semanode->sem));
	MM_RETURN(RET_DMX_OK);
}

MRESULT dmx_sema_lock_timeout(void *sema, int waittime)
{
	static const int quantum_ms = 1000 / HZ;
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;
	MRESULT ret;

	if (NULL == semanode) {
		pr_err("dmx_sema_unlock return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	ret = down_timeout(&(semanode->sem), waittime / quantum_ms);
	if (ret != 0)
		goto err;

	MM_RETURN(RET_DMX_OK);

err:

	switch (ret) {
	case -ETIME:
		MM_RETURN(RET_DMX_TIMEOUT);

	default:
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
}


MRESULT dmx_sema_unlock(void *sema)
{
	struct dmx_sema_node *semanode = (struct dmx_sema_node *)sema;

	if (NULL == semanode) {
		pr_err("dmx_sema_unlock return for semanode is NULL\r\n");
		MM_RETURN(RET_DMX_OK);
	}
	up(&(semanode->sem));
	MM_RETURN(RET_DMX_OK);
}

#endif /* DEBUG_DMX_SEMA */



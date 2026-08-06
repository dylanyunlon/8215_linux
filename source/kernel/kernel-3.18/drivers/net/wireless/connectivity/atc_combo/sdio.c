/*
* Copyright (c) 2021 AutoChips Inc.
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
#include "wi_begin.h"
#include <linux/module.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/pm_runtime.h>
#include "wi_end.h"

#include "common.h"

int atc_combo_sdio_claim_host_timeout(struct sdio_func *func, int timeout_msecs)
{
	static atomic_t timeout_count = ATOMIC_INIT(0);

	DECLARE_WAITQUEUE(wait, current);
	struct mmc_host *host = NULL;
	unsigned int claimed = 0;
	struct task_struct *claimer = NULL;
	int claim_cnt = 0;
	unsigned long start_time = jiffies;
	unsigned long timeout = 0;
	unsigned long flags;
	bool pm = false;
	bool ok = false;

	if (WARN_ON(!func)) {
		COMBO_ERR("NULL func\n");
		return -ENODEV;
	}
	if (-1 == timeout_msecs) {
		sdio_claim_host(func);
		return 0;
	}
	might_sleep();
	host = func->card->host;
	timeout = jiffies + msecs_to_jiffies((u32)timeout_msecs);
	add_wait_queue(&host->wq, &wait);
	spin_lock_irqsave(&host->lock, flags);
	do {
		set_current_state(TASK_UNINTERRUPTIBLE);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		if (!host->claimed || host->claimer == current)
#else
		if (!host->claimed || (host->claimer && host->claimer->task == current))
#endif
		{
			ok = true;
			break;
		} else if (!timeout_msecs) {
			break;
		}
		spin_unlock_irqrestore(&host->lock, flags);
		schedule_timeout((long)(timeout - jiffies));
		spin_lock_irqsave(&host->lock, flags);

	} while (time_is_after_jiffies(timeout));
	set_current_state(TASK_RUNNING);
	if (ok) {
		host->claimed = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		host->claimer = current;
#else
		if (!host->claimer) {
			host->claimer = &host->default_ctx;
		}
		host->claimer->task = current;
#endif
		host->claim_cnt += 1;
		if (host->claim_cnt == 1) {
			pm = true;
		}
	} else {
		wake_up(&host->wq);
	}
	claimed = host->claimed;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	claimer = host->claimer;
#else
	claimer = host->claimer ? host->claimer->task : NULL;
#endif
	claim_cnt = host->claim_cnt;
	spin_unlock_irqrestore(&host->lock, flags);
	remove_wait_queue(&host->wq, &wait);
	if (pm) {
		pm_runtime_get_sync(mmc_dev(host));
	}
	if (ok) {
		if (atomic_xchg(&timeout_count, 0)) {
			COMBO_INFO("reset timeout_count\n");
		}
		return 0;
	} else if (!timeout_msecs) {
		return -EBUSY;
	} else {
		atomic_inc(&timeout_count);
		COMBO_ERR("timeout %u > %d ms timeout_count %d claimer %u:%s:%d\n",
				jiffies_to_msecs(jiffies - start_time), timeout_msecs,
				atomic_read(&timeout_count), claimed,
				claimer ? claimer->comm : "null", claim_cnt);
		if (claimer) {
			atc_combo_dump_single_task(claimer);
		}
		return -ETIMEDOUT;
	}
}
EXPORT_SYMBOL(atc_combo_sdio_claim_host_timeout);

MODULE_DESCRIPTION("sdio extented/debug api");
MODULE_ALIAS("atc_combo:sdio");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");

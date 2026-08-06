/*
* Copyright (c) 2023 AutoChips Inc.
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
#include <linux/irq.h>
#include <../kernel/irq/internals.h>
#include "wi_end.h"

#include "common.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))

#define __irqd_to_state(d) ((d)->state_use_accessors)

static inline unsigned int irq_desc_get_irq(struct irq_desc *desc)
{
	return desc->irq_data.irq;
}

#else
#define __irqd_to_state(d) ACCESS_PRIVATE((d)->common, \
		state_use_accessors)
#endif

#define SPURIOUS_DEFERRED   0x80000000

void atc_combo_dump_irq(unsigned int irq)
{
	struct irq_desc *desc = NULL;
	struct irqaction *action = NULL;

	COMBO_INFO("====== start irq %u ======\n", irq);

	desc = irq_to_desc(irq);
	if (!desc) {
		COMBO_INFO("NULL irq desc\n");
		return;
	}

	action = desc->action;
	if (!action) {
		COMBO_INFO("NULL irqaction\n");
		return;
	}

	COMBO_INFO("=== irq_desc ===\n");

	COMBO_INFO("name %s irq %u\n",
			desc->name, irq_desc_get_irq(desc));

	COMBO_INFO("irq state: 0x%08x\n",
			__irqd_to_state(&desc->irq_data));

	COMBO_INFO("istate: 0x%08x\n", desc->istate);
	COMBO_INFO("    disabled: %d\n",
			irqd_irq_disabled(&desc->irq_data));
	COMBO_INFO("    IRQD_IRQ_INPROGRESS: %d\n",
			irqd_has_set(&desc->irq_data, IRQD_IRQ_INPROGRESS));

	COMBO_INFO("irq_count: %u\n", desc->irq_count);
	COMBO_INFO("last_unhandled: %lu\n", desc->last_unhandled);
	COMBO_INFO("irqs_unhandled: %u\n", desc->irqs_unhandled);
	COMBO_INFO("threads_active: %d\n",
			atomic_read(&desc->threads_active));
	COMBO_INFO("threads_handled: %d\n",
			atomic_read(&desc->threads_handled));
	COMBO_INFO("threads_handled_last: %u SPURIOUS_DEFERRED: %u\n",
			(unsigned)desc->threads_handled_last & ~SPURIOUS_DEFERRED,
			!!((unsigned)desc->threads_handled_last | SPURIOUS_DEFERRED));

	COMBO_INFO("=== irqaction ===\n");

	COMBO_INFO("name %s irq %u\n",
			action->name, action->irq);
	COMBO_INFO("IRQTF_RUNTHREAD: %d\n",
			test_bit(IRQTF_RUNTHREAD, &action->thread_flags));

	COMBO_INFO("====== end ======\n");
}
EXPORT_SYMBOL(atc_combo_dump_irq);

MODULE_DESCRIPTION("Dump irq info");
MODULE_ALIAS("atc_combo:irq");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");

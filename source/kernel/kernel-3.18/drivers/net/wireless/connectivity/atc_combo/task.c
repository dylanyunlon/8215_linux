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

/* atc_combo_dump_cpu_task
 *
 * Usage:
 * void atc_combo_dump_cpu_task_trigger(bool wait);
 * atc_combo_dump_cpu_task_trigger(wait);
 *
 * Call trace:
    gic_handle_irq();
      __handle_domain_irq();
        generic_handle_irq();
          handle_percpu_devid_irq();
            arch_timer_handler_virt();
              hrtimer_interrupt();
                __hrtimer_run_queues();
                  __run_hrtimer();
                    tick_sched_timer();
                      update_process_times();
                        rcu_check_callbacks();
                          atc_combo_dump_cpu_task();

  void rcu_check_callbacks(int user)
  {
 +       // ATC_AOSP_ENHANCEMENT @{
 +       #if IS_ENABLED(CONFIG_MT6630_CONN)
 +       void atc_combo_dump_cpu_task(void);
 +
 +       atc_combo_dump_cpu_task();
 +       #endif
 +       // ATC_AOSP_ENHANCEMENT @}
 +
 */

#include "wi_begin.h"
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/debug_locks.h>
#include <linux/sched.h>
#include <../kernel/sched/sched.h>
#include "wi_end.h"

#include "common.h"

#ifndef TASK_STATE_TO_CHAR_STR
#define TASK_STATE_TO_CHAR_STR "RSDTtXZxKWPNn"
#endif

static DEFINE_PER_CPU(int, atc_combo_dump_cpu_task_start);
static DEFINE_PER_CPU(int, atc_combo_dump_cpu_task_done);
static DEFINE_PER_CPU(struct task_struct *, atc_combo_dump_cpu_task_curr);

static char atc_combo_task_state(unsigned long state)
{
	static const char stat_nam[] = TASK_STATE_TO_CHAR_STR;
	char state_c = '\0';

	state = state ? __ffs(state) + 1 : 0;
	state_c = (state < (sizeof(stat_nam) - 1)) ? stat_nam[state] : '?';

	return state_c;
}

static void atc_combo_dump_task_state(struct task_struct *tsk)
{
	struct thread_info *ti = NULL;
	unsigned int preempt_count = 0;
	unsigned cpu = 0;
	int prev_debug_locks = 0;

	if (!tsk) {
		COMBO_ERR("NULL task\n");
		return;
	}
	rcu_read_lock();
	cpu = task_cpu(tsk);
	ti = task_thread_info(tsk);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	preempt_count = (unsigned int)ti->preempt_count;
#else
	preempt_count = ti->preempt.count;
#endif
	COMBO_INFO("CPU-%u on_cpu:%d tsk:%p ti:%p "
			"preempt_count: 0x%08x\n",
			cpu, tsk->on_cpu, tsk, ti, preempt_count);

	COMBO_INFO("comm:%s pid:%d tgid:%d state:0x%lx(%c)\n",
			tsk->comm, tsk->pid, tsk->tgid,
			tsk->state, atc_combo_task_state((unsigned long)(tsk->state)));
	rcu_read_unlock();

	prev_debug_locks = debug_locks;
	debug_locks = 1;
	debug_show_held_locks(tsk);
	if (debug_locks != prev_debug_locks) {
		debug_locks = prev_debug_locks;
	}
}

struct task_struct *atc_combo_find_task_by_name(const char *name)
{
	struct task_struct *tsk = NULL;

	if (!name || strlen(name) > 255) {
		COMBO_ERR("name is NULL or too long: '%s'\n", name);
		return NULL;
	}
	rcu_read_lock();
	for_each_process(tsk) { //-V568
		if (!strncmp(tsk->comm, name, TASK_COMM_LEN - 1)) {
			rcu_read_unlock();
			return tsk;
		}
	}
	rcu_read_unlock();

	return NULL;
}
EXPORT_SYMBOL(atc_combo_find_task_by_name);

void atc_combo_dump_single_task(struct task_struct *tsk)
{
	if (!tsk) {
		COMBO_ERR("NULL task\n");
		return;
	}
	atc_combo_dump_task_state(tsk);
	sched_show_task(tsk);
}
EXPORT_SYMBOL(atc_combo_dump_single_task);

void atc_combo_dump_task_by_name(const char *name)
{
	COMBO_INFO("show debug info of '%s'\n", name);
	atc_combo_dump_single_task(
			atc_combo_find_task_by_name(name));
}
EXPORT_SYMBOL(atc_combo_dump_task_by_name);

void atc_combo_dump_cpu_task_simple(void)
{
	int prev_debug_locks = 0;
	unsigned cpu = 0;

	#include "wi_begin.h"
	for_each_possible_cpu(cpu) {
	#include "wi_end.h"
		per_cpu(atc_combo_dump_cpu_task_curr, cpu) = cpu_curr(cpu);
	}

	COMBO_INFO("====== start ======\n");
	dump_stack();

	COMBO_INFO("PREEMPT_MASK: 0x%08lx\n", PREEMPT_MASK);
	COMBO_INFO("SOFTIRQ_MASK: 0x%08lx\n", SOFTIRQ_MASK);
	COMBO_INFO("HARDIRQ_MASK: 0x%08lx\n", HARDIRQ_MASK);
	COMBO_INFO("NMI_MASK    : 0x%08lx\n", NMI_MASK);

	prev_debug_locks = debug_locks;
	debug_locks = 1;
	debug_show_all_locks();
	if (debug_locks != prev_debug_locks) {
		debug_locks = prev_debug_locks;
	}

	#include "wi_begin.h"
	for_each_possible_cpu(cpu) {
	#include "wi_end.h"
		struct task_struct *tsk = NULL;

		tsk = per_cpu(atc_combo_dump_cpu_task_curr, cpu);
		if (tsk) {
			atc_combo_dump_task_state(tsk);
			sched_show_task(tsk);
		} else {
			COMBO_INFO("NULL task on CPU-%u\n", cpu);
		}
	}

	COMBO_INFO("======  end  ======\n");
}
EXPORT_SYMBOL(atc_combo_dump_cpu_task_simple);

void atc_combo_dump_cpu_task_trigger(bool wait)
{
	unsigned cpu = 0;

	atc_combo_dump_cpu_task_simple();

	#include "wi_begin.h"
	for_each_possible_cpu(cpu) {
	#include "wi_end.h"
		int i = 0;

		per_cpu(atc_combo_dump_cpu_task_done, cpu) = 0;
		per_cpu(atc_combo_dump_cpu_task_start, cpu) = 1;

		if (!wait) {
			continue;
		}
		// avoid log crossover from different cpu
		for (i = 0; i < 1000 * 10; i++) {
			if (1 == per_cpu(atc_combo_dump_cpu_task_done, cpu)) {
				break;
			}
			udelay(100);
		}
	}
}
EXPORT_SYMBOL(atc_combo_dump_cpu_task_trigger);

// only call at rcu_check_callbacks()
void atc_combo_dump_cpu_task(void)
{
	struct pt_regs *regs = NULL;
	struct task_struct *tsk = NULL;
	unsigned cpu = (unsigned)smp_processor_id();

	if (!per_cpu(atc_combo_dump_cpu_task_start, cpu)) {
		return;
	} else {
		per_cpu(atc_combo_dump_cpu_task_start, cpu) = 0;
	}

	COMBO_INFO("====== start on CPU-%u ======\n", cpu);
#ifndef MODULE
	print_modules();
#endif
	print_irqtrace_events(current);
	regs = get_irq_regs();
	if (regs)
		show_regs(regs);

	//dump_cpu_task(cpu);
	COMBO_INFO("Task dump for CPU-%u:\n", cpu);
	tsk = per_cpu(atc_combo_dump_cpu_task_curr, cpu);
	atc_combo_dump_single_task(tsk);

	if (tsk != current) {
		COMBO_INFO("Current dump for CPU-%u:\n", cpu);
		atc_combo_dump_single_task(current);
	}

	COMBO_INFO("====== end   on CPU-%u ======\n", cpu);

	per_cpu(atc_combo_dump_cpu_task_done, cpu) = 1;
}

MODULE_DESCRIPTION("Dump each cpu task info");
MODULE_ALIAS("atc_combo:task");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");

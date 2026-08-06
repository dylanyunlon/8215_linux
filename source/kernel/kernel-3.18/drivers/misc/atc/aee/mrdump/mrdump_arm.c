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

#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <asm/ptrace.h>
#include "mrdump_private.h"

void mrdump_save_current_backtrace(struct pt_regs *regs)
{
	asm volatile("stmia %1, {r0 - r15}\n\t"
		     "mrs %0, cpsr\n"
		     : "=r"(regs->uregs[16])
		     : "r" (regs)
		     : "memory");
}

void mrdump_print_crash(struct pt_regs *regs)
{
	unsigned int fp, mode;
	int ok = 1;

	__show_regs(regs);

	/* Print current backtrace */
	printk("Backtrace: ");
	fp = regs->ARM_fp;
	mode = processor_mode(regs);

	if (!fp) {
		printk("no frame pointer");
		ok = 0;
	} else if ((fp < PAGE_OFFSET) || ((high_memory != NULL) && (fp > (unsigned long) high_memory))) {
		printk("invalid frame pointer 0x%08x", fp);
		ok = 0;
	}
	printk("\n");

	if (ok)
		c_backtrace(fp, mode);
}

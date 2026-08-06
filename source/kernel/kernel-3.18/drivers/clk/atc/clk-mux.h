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

#ifndef __DRV_CLK_MUX_H
#define __DRV_CLK_MUX_H

/*
 * This is a private header file. DO NOT include it except clk-*.c.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#define CLK_DEBUG		0
#define DUMMY_REG_TEST		0

extern spinlock_t *get_atc_clk_lock(void);

#define atc_clk_lock(flags)	spin_lock_irqsave(get_atc_clk_lock(), flags)
#define atc_clk_unlock(flags)	\
	spin_unlock_irqrestore(get_atc_clk_lock(), flags)

#define MAX_MUX_GATE_BIT	31
#define INVALID_MUX_GATE_BIT	(MAX_MUX_GATE_BIT + 1)

struct clk *atc_clk_register_mux(
		const char *name,
		const char **parent_names,
		u8 num_parents,
		void __iomem *base_addr,
		u8 shift,
		u8 width,
		u8 gate_bit);

#endif /* __DRV_CLK_MUX_H */

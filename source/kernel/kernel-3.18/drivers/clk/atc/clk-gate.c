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

#include <linux/of.h>
#include <linux/of_address.h>

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/clkdev.h>
#include <linux/ratelimit.h>

#include "clk-mux.h"
#include "clk-gate.h"

/*
 * clk_gate
 */
static void cg_set_mask(void *reg, u32 mask)
{
	u32 r;

	r = readl_relaxed(reg) | mask;
	writel_relaxed(r, reg);

	if (reg != NULL) {
		pr_debug("[CCF]   %s: write [0x%x] to reg[0x%x], mask[0x%x]\n", __func__, r, (unsigned int)reg, mask);
	}
}

static void cg_clr_mask(void *reg, u32 mask)
{
	u32 r;

	r = readl_relaxed(reg) & ~mask;
	writel_relaxed(r, reg);

	if (reg != NULL) {
		pr_debug("[CCF]   %s: write [0x%x] to reg[0x%x], mask[0x%x]\n", __func__, r, (unsigned int)reg, mask);
	}
}

static int cg_prepare(struct clk_hw *hw)
{
	unsigned long flags = 0;
	struct atc_clk_gate *cg = to_clk_gate(hw);
	u32 mask = 0;

	if (cg->onoff_bit >= INVALID_MUX_GATE_BIT) {
		pr_err("[CCF] No on&off for this clock, no need!\n");
		return -1;
	}

	mask = BIT(cg->onoff_bit);

	if (cg != NULL) {
		pr_debug("[CCF] %s: on, shift[%d], flags[%d]\n", __func__, cg->onoff_bit, cg->onoff_flags);
	}

	atc_clk_lock(flags);

	if (cg->onoff_flags & CLK_GATE_INVERSE) {
		cg_set_mask(cg->onoff_addr, mask);
	} else {
		cg_clr_mask(cg->onoff_addr, mask);
	}

	atc_clk_unlock(flags);

	return 0;
}

static void cg_unprepare(struct clk_hw *hw)
{
	unsigned long flags = 0;
	struct atc_clk_gate *cg = to_clk_gate(hw);
	u32 mask = 0;

	if (cg->onoff_bit >= INVALID_MUX_GATE_BIT) {
		pr_err("[CCF] No on&off for this clock, no need!\n");
		return;
	}

	mask = BIT(cg->onoff_bit);

	if (cg != NULL) {
		pr_debug("[CCF] %s: off, shift[%d], flags[%d]\n", __func__, cg->onoff_bit, cg->onoff_flags);
	}

	atc_clk_lock(flags);

	if (cg->onoff_flags & CLK_GATE_INVERSE) {
		cg_clr_mask(cg->onoff_addr, mask);
	} else {
		cg_set_mask(cg->onoff_addr, mask);
	}

	atc_clk_unlock(flags);
}

static int cg_is_prepared(struct clk_hw *hw)
{
	struct atc_clk_gate *cg = to_clk_gate(hw);
	u32 mask;
	u32 val;
	int r;

	mask = BIT(cg->onoff_bit);
	val = mask & readl(cg->onoff_addr);

	r = (cg->onoff_flags & CLK_GATE_INVERSE) ? (val != 0) : (val == 0);

	if (cg != NULL) {
		pr_debug("[CCF] %s: %d, %s, shift[%d]\n", __func__, r, __clk_get_name(hw->clk), (int)cg->onoff_bit);
	}

	return r;
}

static int cg_enable(struct clk_hw *hw)
{
	unsigned long flags = 0;
	struct atc_clk_gate *cg = to_clk_gate(hw);
	u32 mask = 0;

	if (cg->rst_bit >= INVALID_MUX_GATE_BIT) {
		pr_err("[CCF] No reset for this clock, no need!\n");
		return -1;
	}

	mask = BIT(cg->rst_bit);

	if (cg != NULL) {
		pr_debug("[CCF] %s: do reset, shift[%d], flags[%d]\n", __func__, cg->rst_bit, cg->rst_flags);
	}

	atc_clk_lock(flags);

	if (cg->rst_flags & CLK_GATE_INVERSE) {
		cg_set_mask(cg->rst_addr, mask);
	} else {
		cg_clr_mask(cg->rst_addr, mask);
	}

	atc_clk_unlock(flags);

	return 0;
}

static void cg_disable(struct clk_hw *hw)
{
	unsigned long flags = 0;
	struct atc_clk_gate *cg = to_clk_gate(hw);
	u32 mask = 0;

	if (cg->rst_bit >= INVALID_MUX_GATE_BIT) {
		pr_err("[CCF] No reset for this clock, no need!\n");
		return;
	}

	mask = BIT(cg->rst_bit);

	if (cg != NULL) {
		pr_debug("[CCF] %s: do reset, shift[%d], flags[%d]\n", __func__, cg->rst_bit, cg->rst_flags);
	}

	atc_clk_lock(flags);

	if (cg->rst_flags & CLK_GATE_INVERSE) {
		/*cg_set_mask(cg->rst_addr, mask);*/
		cg_clr_mask(cg->rst_addr, mask);
	} else {
		cg_set_mask(cg->rst_addr, mask);
		/*cg_clr_mask(cg->rst_addr, mask);*/
	}

	atc_clk_unlock(flags);

}

static const struct clk_ops atc_clk_gate_ops = {
	.is_prepared	= cg_is_prepared,
	.enable			= cg_enable,
	.disable		= cg_disable,
	.prepare		= cg_prepare,
	.unprepare		= cg_unprepare,
};

struct clk *atc_clk_register_gate(
	const char *name,
	const char *parent_name,
	void __iomem *onoff_addr,
	void __iomem *rst_addr,
	unsigned int onoff_bit,
	unsigned int onoff_flags,
	unsigned int rst_bit,
	unsigned int rst_flags)
{
	struct atc_clk_gate *cg;
	struct clk *clk;
	struct clk_init_data init;

	cg = kzalloc(sizeof(*cg), GFP_KERNEL);

	if (!cg) {
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.flags = CLK_IGNORE_UNUSED;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;
	init.ops = &atc_clk_gate_ops;

	cg->onoff_addr = onoff_addr;
	cg->onoff_bit  = onoff_bit;
	cg->onoff_flags = onoff_flags;
	cg->rst_addr = rst_addr;
	cg->rst_bit = rst_bit;
	cg->rst_flags = rst_flags;

	cg->hw.init = &init;

	clk = clk_register(NULL, &cg->hw);

	if (IS_ERR(clk)) {
		kfree(cg);
	}

	return clk;
}










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
#include "clk-divider.h"

// for audio special case
#define AUD_K1_RATIO_REG  0xFD00001C

/*
 * clk_divider
 */
static void cg_set_value(void *reg, unsigned int bit, unsigned int width, unsigned int rate)
{
	unsigned int r = 0;
	unsigned int value = 0;

	r = readl_relaxed(reg);

	if ((reg == AUD_K1_RATIO_REG) && ((bit == 0) || (bit == 8))) {
		value = ~((1U << 16) - 1);
		r = r & value;
		value = rate;
		value = value & ((1U << 16) - 1);
		r = r | value;
	} else {
		value = ~(((1U << width) - 1) << bit);
		r = r & value;
		value = rate << bit;
		r = r | value;
	}

	writel_relaxed(r, reg);

	if (reg != 0) {
		pr_info("[CCF]   %s: write [0x%x] to reg[0x%x], rate[0x%x]\n", __func__, r, (unsigned int)reg, rate);
	}
}

static unsigned long clk_divider_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{

	return 0;
}

static long clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long *prate)
{

	return rate;
}

static int clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct atc_clk_divider *cg = to_clk_divider(hw);
	unsigned long flags = 0;
	unsigned int rate_tmp = 0;

	atc_clk_lock(flags);

	rate_tmp = (unsigned int)rate;

	if (cg->bit >= INVALID_MUX_GATE_BIT) {
		pr_err("[CCF] wrong parameter, bit\n");
		return -1;
	}

	if (!((cg->addr == AUD_K1_RATIO_REG) && (cg->bit == 0 || cg->bit == 8))) {
		if (rate_tmp >= (1 << cg->width)) {
			pr_err("[CCF] wrong parameter, rate\n");
			return -1;
		}
	}

	if (cg != 0) {
		pr_info("[CCF]   %s: addr [0x%x], bit [%d], width [%d], value [0x%x]\n",
			__func__,
			(unsigned int)(cg->addr),
			cg->bit,
			cg->width, rate_tmp);
	}

	cg_set_value(cg->addr, cg->bit, cg->width, rate_tmp);

	atc_clk_unlock(flags);

	return 0;
}


static const struct clk_ops atc_clk_divider_ops = {
	.recalc_rate = clk_divider_recalc_rate,
	.round_rate = clk_divider_round_rate,
	.set_rate = clk_divider_set_rate,
};

struct clk *atc_clk_register_divider(
	const char *name,
	const char *parent_name,
	void __iomem *addr,
	unsigned int bit,
	unsigned int width)

{
	struct atc_clk_divider *cg;
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
	init.ops = &atc_clk_divider_ops;

	cg->addr = addr;
	cg->bit  = bit;
	cg->width = width;

	cg->hw.init = &init;

	clk = clk_register(NULL, &cg->hw);

	if (IS_ERR(clk)) {
		pr_err("[CCF] register clk fail\n");
		kfree(cg);
	}

	return clk;
}










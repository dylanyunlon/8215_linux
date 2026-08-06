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

#ifndef __DRV_CLK_GATE_H
#define __DRV_CLK_GATE_H

/*
 * This is a private header file. DO NOT include it except clk-*.c.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>

struct atc_clk_gate {
	struct clk_hw	hw;
	void __iomem	*onoff_addr;
	unsigned int		onoff_bit;
	unsigned int		onoff_flags;
	void __iomem	*rst_addr;
	unsigned int		rst_bit;
	unsigned int		rst_flags;
};

#define to_clk_gate(_hw) container_of(_hw, struct atc_clk_gate, hw)

#define CLK_GATE_INVERSE	BIT(0)
#define CLK_GATE_NO_SETCLR_REG	BIT(1)

struct clk *atc_clk_register_gate(
		const char *name,
		const char *parent_name,
		void __iomem *onoff_addr,
		void __iomem *rst_addr,
		unsigned int onoff_bit,
		unsigned int onoff_flags,
		unsigned int rst_bit,
		unsigned int rst_flags);

#endif /* __DRV_CLK_GATE_H */


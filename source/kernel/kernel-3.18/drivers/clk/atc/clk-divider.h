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

#ifndef __DRV_CLK_DIVIDER_H
#define __DRV_CLK_DIVIDER_H

/*
 * This is a private header file. DO NOT include it except clk-*.c.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>

struct atc_clk_divider {
	struct clk_hw	hw;
	void __iomem	*addr;
	unsigned int	bit;
	unsigned int	width;
};

#define to_clk_divider(_hw) container_of(_hw, struct atc_clk_divider, hw)

#define CLK_DIVIDER_INVERSE	BIT(0)
#define CLK_DIVIDER_NO_SETCLR_REG	BIT(1)

struct clk *atc_clk_register_divider(
		const char *name,
		const char *parent_name,
		void __iomem *addr,
		unsigned int bit,
		unsigned int width);

#endif /* __DRV_CLK_DIVIDER_H */


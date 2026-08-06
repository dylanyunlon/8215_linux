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

/* system header files */


#ifndef __ARCH_ARM_MTK_NAND_H
#define __ARCH_ARM_MTK_NAND_H

#include <linux/mtd/nand.h>

struct atc_nand_pdata {
	/* NAND chip information */
	unsigned short		page_size;
	unsigned short		data_width;

	/* RD/WR strobe delay timing information, all times in SCLK cycles */
	unsigned short		rd_dly;
	unsigned short		wr_dly;

	/* NAND MTD partition information */
	int                     nr_partitions;
	struct mtd_partition    *partitions;
};

#endif	/* __ARCH_ARM_DAVINCI_NAND_H */

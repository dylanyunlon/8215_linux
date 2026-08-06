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

#ifndef ATC_NAND_H
#define ATC_NAND_H

#include "drv_polling.h"
#include "atc_ecc.h"

#define NAND_MAX_OOBSIZE	640
#define NAND_MAX_PAGESIZE	8192
#define NAND_ATC_BB_VER 0x0000001
#define NAND_ATC_MAX_BB_NUM 100
#define NAND_ATC_MAX_RB_NUM 6
#define NAND_ATE_BBT_SIGNITURE 0xAA00BB00
#define ATC_RB_FREE  0x10000
#define ATC_RB_BAD   0x20000



struct nand_atc_bbt {
    u16 badblocks[NAND_ATC_MAX_BB_NUM];
    u32 realnum; // real bad block
    u32 maxbb; // max number bad block
    u32 rsvbn; //Real reserved blocks 
    u32 phyidx[NAND_ATC_MAX_RB_NUM];
    u32 logidx[NAND_ATC_MAX_RB_NUM];
    u32 version;
    u32 signiture;
};
struct nand_log2phy_tbl {
	u32 page2blkshift;
	u32 page2blkmask;
	u32 blocknum;
	u16 log2phytbl[1];
};

/* atc nand info */
struct atc_nand_info {
	struct mtd_info mtd;
	//struct nand_hw_control controller;
	struct nand_chip nand;
	struct clk *nand_clk;
	struct clk *clk_onoff;
	struct device *device;
	struct platform_device *pdev;
	struct completion dma_completion;
	struct completion erase_completion;
	struct completion program_completion;
	struct completion reset_completion;
	int datalen;
	int data_pos;
	dma_addr_t dmaaddr;
	unsigned char *dmabuf;

	void __iomem *nfi_regs;
	void __iomem *ecc_regs;
	ECC_Level_t ecc_level;
	u32 para_encode_config;
	u32 para_decode_config;
	u8 fdm_size;
	u8 fdm_ecc_size;
	u8 nand_sec_shift;
};


 int atc_nand_init_att(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_set_bad_block(struct nand_chip *chip, u16 blockid);
 u32 atc_pagel2p(struct nand_chip *chip, u32 page);
 u32 atc_blockl2p(struct nand_chip *chip, u32 page);
 int atc_nand_read_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_write_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_init_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);

 
#define ATC_NAND_TIMEOUT	10 // 10 ms

#define CMD_BUFF_SIZE  (256)
#define CMD_MAX_SIZE   (32)


#endif  // ATC_NAND_H


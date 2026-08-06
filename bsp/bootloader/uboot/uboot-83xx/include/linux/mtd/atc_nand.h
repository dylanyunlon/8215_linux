/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef AC83XX_NAND_H
#define AC83XX_NAND_H
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
 int atc_nand_init_att(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_set_bad_block(struct nand_chip *chip, u16 blockid);
 u32 atc_pagel2p(struct nand_chip *chip, u32 page);
 u32 atc_blockl2p(struct nand_chip *chip, u32 page);
 int atc_nand_read_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_write_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);
 int atc_nand_init_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt);

#endif  // AC83XX_NAND_H


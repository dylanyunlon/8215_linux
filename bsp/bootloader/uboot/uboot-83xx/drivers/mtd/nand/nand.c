/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <linux/mtd/atc_nfi.h>
#include <asm/arch/ac83xx_upg_ddr_layout.h>

#ifndef CONFIG_SYS_NAND_BASE_LIST
#define CONFIG_SYS_NAND_BASE_LIST { CONFIG_SYS_NAND_BASE }
#endif

DECLARE_GLOBAL_DATA_PTR;
extern unsigned int _sdagentflag;

int nand_curr_device = -1;
nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];
static ulong base_address[CONFIG_SYS_MAX_NAND_DEVICE] = CONFIG_SYS_NAND_BASE_LIST;

static const char default_nand_name[] = "nand";
static __attribute__((unused)) char dev_name[CONFIG_SYS_MAX_NAND_DEVICE][8];

static void nand_init_chip(struct mtd_info *mtd, struct nand_chip *nand,ulong base_addr)
{
	int maxchips = CONFIG_SYS_NAND_MAX_CHIPS;
	int __attribute__((unused)) i = 0;

	if (maxchips < 1)
		maxchips = 1;
	mtd->priv = nand;

	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)base_addr;
	if (board_nand_init(nand) == 0) {
		if (nand_scan(mtd, maxchips) == 0) {
			if (!mtd->name)
				mtd->name = (char *)default_nand_name;
			else
				mtd->name += gd->reloc_off;

#ifdef CONFIG_MTD_DEVICE
			/*
			 * Add MTD device so that we can reference it later
			 * via the mtdcore infrastructure (e.g. ubi).
			 */
			sprintf(dev_name[i], "nand%d", i);
			mtd->name = dev_name[i++];
			add_mtd_device(mtd);
#endif
		} else
			mtd->name = NULL;
	} else {
		mtd->name = NULL;
		mtd->size = 0;
	}

}
ulong res_blk_addr;
ulong res_blk_size;
const char *valid_flag="1234abcd";
void nand_init(void)
{
	int i;
	unsigned int size = 0;
	struct mtd_oob_ops ops;
	int val;

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		nand_init_chip(&nand_info[i], &nand_chip[i], base_address[i]);
		size += nand_info[i].size / 1024;
		if (nand_curr_device == -1)
			nand_curr_device = i;
	}
	printf("nand: %u MiB\n", size / 1024);

	res_blk_addr= (nand_info[nand_curr_device].size>>nand_chip[nand_curr_device].phys_erase_shift)-6;
	res_blk_addr=res_blk_addr<<nand_chip[nand_curr_device].phys_erase_shift;
	res_blk_size=nand_info[nand_curr_device].erasesize;
	printf("res_blk_addr %x  size %x \n",res_blk_addr,res_blk_size);

	uint8_t *ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
	uint8_t *ext_oob_buffer=ATC_UPG_BASE_ADDR+ATC_NAND_UPG_OOB_OFFSET;

	memset(ext_buffer, 0xFF, nand_info[nand_curr_device].writesize);
	memset(ext_oob_buffer, 0xFF, nand_info[nand_curr_device].oobsize);
	char *nfi_val;
	char flag_head[9];

#if 1
	if (0){
		*NFI_ACCCON=0x121;
		nand_tune_timing_test(&nand_info[nand_curr_device], res_blk_addr, res_blk_size, 2);
		val=*NFI_ACCCON;
		printf("set acccon %x \n",*NFI_ACCCON);
		*nfi_val=val&0xff;
		memcpy(ext_buffer,nfi_val,1);
		ops.len = nand_info[nand_curr_device].writesize;
		ops.mode = MTD_OOB_PLACE;
		ops.datbuf = ext_buffer;
		ops.ooblen = 4;
		ops.oobbuf = ext_oob_buffer;
		ops.ooboffs = 0;

		if(0 != nand_info[nand_curr_device].write_oob(&nand_info[nand_curr_device], res_blk_addr, &ops)) {
				printf("MTD writeoob failure \n");
		}
	}
	else{

		ops.len = nand_info[nand_curr_device].writesize;
		ops.mode = MTD_OOB_PLACE;
		ops.datbuf = ext_buffer;
		ops.ooblen = 4;
		ops.oobbuf = ext_oob_buffer;
		ops.ooboffs = 0;

		if(0 != nand_info[nand_curr_device].read_oob(&nand_info[nand_curr_device], res_blk_addr, &ops)) {
				printf("MTD read oob failure \n");
		}else{
			memcpy(flag_head,ext_buffer,8);
			if(0==strncmp(flag_head,valid_flag,8)){
				memcpy(nfi_val,ext_buffer+8,1);
				val=*NFI_ACCCON;
				val&=0xffffff00;
				val|= (*nfi_val);

				*NFI_ACCCON=val;

				printf("val %x set acccon %x \n",val,*NFI_ACCCON);
			}
		}
	}
#endif
#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	/*
	 * Select the chip in the board/cpu specific driver
	 */
	board_nand_select_device(nand_info[nand_curr_device].priv, nand_curr_device);
#endif
}

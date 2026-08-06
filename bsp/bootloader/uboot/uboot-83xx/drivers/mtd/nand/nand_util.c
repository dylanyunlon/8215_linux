/*
 * drivers/mtd/nand/nand_util.c
 *
 * Copyright (C) 2006 by Weiss-Electronic GmbH.
 * All rights reserved.
 *
 * @author:	Guido Classen <clagix@gmail.com>
 * @descr:	NAND Flash support
 * @references: borrowed heavily from Linux mtd-utils code:
 *		flash_eraseall.c by Arcom Control System Ltd
 *		nandwrite.c by Steven J. Hill (sjhill@realitydiluted.com)
 *			       and Thomas Gleixner (tglx@linutronix.de)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <div64.h>
#include <asm/arch/ac83xx_upg_ddr_layout.h>
#include <asm/errno.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <jffs2/jffs2.h>
#include "nand_ext.h"
#if !defined(CONFIG_SYS_64BIT_VSPRINTF)
#warning Please define CONFIG_SYS_64BIT_VSPRINTF for correct output!
#endif

typedef struct erase_info erase_info_t;
typedef struct mtd_info	  mtd_info_t;

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

extern void flush_cache(unsigned int start, unsigned int size);
extern void flush_invalid_cache(unsigned int start, unsigned int size);
/*****************************************************************************/
static int nand_block_bad_scrub(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	return 0;
}

/**
 * nand_erase_opts: - erase NAND flash with support for various options
 *		      (jffs2 formating)
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	erase_info_t erase;
	unsigned long erase_length, erased_length; /* in blocks */
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	int (*nand_block_bad_old)(struct mtd_info *, loff_t, int) = NULL;
	const char *mtd_device = meminfo->name;
	struct mtd_oob_ops oob_opts;
	struct nand_chip *chip = meminfo->priv;

	if ((opts->offset & (meminfo->writesize - 1)) != 0) {
		printf("Attempt to erase non page aligned data\n");
		return -1;
	}
	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = lldiv(opts->length + meminfo->erasesize - 1,
			     meminfo->erasesize);
	cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen = cpu_to_je32(8);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		struct nand_chip *priv_nand = meminfo->priv;

		nand_block_bad_old = priv_nand->block_bad;
		priv_nand->block_bad = nand_block_bad_scrub;
		/* we don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (priv_nand->bbt) {
			kfree(priv_nand->bbt);
		}
		priv_nand->bbt = NULL;
	}

	for (erased_length = 0;
	     erased_length < erase_length;
	     erase.addr += meminfo->erasesize) {

		WATCHDOG_RESET ();

        if (erase.addr >= meminfo->size) //the total device is erased all
        {
          printf("\nErase position pass end of device, erase.addr = 0x%08x, flash size = 0x%08x\n",
		  	        erase.addr, meminfo->size);
          break;
        }

		if (!opts->scrub && bbtest) {
			int ret = meminfo->block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					printf("\rSkipping bad block at  "
					       "0x%08llx                 "
					       "                         \n",
					       erase.addr);

				if (!opts->spread){
					erased_length++;
					printf("spread erased_length++\r\n");
				}

				printf("continue\r\n");

				continue;

			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		erased_length++;
		result = meminfo->erase(meminfo, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);
                            result = meminfo->block_markbad(meminfo, erase.fail_addr);
                            printf("\n%s: MTD mark bad block at address 0x%x: %d\n",
			       mtd_device, erase.fail_addr, result);
			continue;
		}

		/* format for JFFS2 ? */
		if (opts->jffs2 && chip->ecc.layout->oobavail >= 8) {
			chip->ops.ooblen = 8;
			chip->ops.datbuf = NULL;
			chip->ops.oobbuf = (uint8_t *)&cleanmarker;
			chip->ops.ooboffs = 0;
			chip->ops.mode = MTD_OOB_AUTO;

			result = meminfo->write_oob(meminfo,
			                            erase.addr,
			                            &chip->ops);
			if (result != 0) {
				printf("\n%s: MTD writeoob failure: %d\n",
				       mtd_device, result);
				continue;
			}
		}

		if (!opts->quiet) {
			unsigned long long n = erased_length * 100ULL;
			int percent;

			percent = (int)lldiv(n, erase_length);

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;

				printf("\rErasing at 0x%x -- %3d%% complete.",
				       (unsigned int)erase.addr, percent);

				if (opts->jffs2 && result == 0)
					printf(" Cleanmarker written at 0x%llx.",
					       erase.addr);
			}
		}
	}
	if (!opts->quiet)
		printf("\n");

	if (nand_block_bad_old) {
		struct nand_chip *priv_nand = meminfo->priv;

		priv_nand->block_bad = nand_block_bad_old;
		priv_nand->scan_bbt(meminfo);
	}

	return 0;
}

/* XXX U-BOOT XXX */
#if 0

#define MAX_PAGE_SIZE	2048
#define MAX_OOB_SIZE	64

/*
 * buffer array used for writing data
 */
static unsigned char data_buf[MAX_PAGE_SIZE];
static unsigned char oob_buf[MAX_OOB_SIZE];

/* OOB layouts to pass into the kernel as default */
static struct nand_ecclayout none_ecclayout = {
	.useecc = MTD_NANDECC_OFF,
};

static struct nand_ecclayout jffs2_ecclayout = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 0, 1, 2, 3, 6, 7 }
};

static struct nand_ecclayout yaffs_ecclayout = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 8, 9, 10, 13, 14, 15}
};

static struct nand_ecclayout autoplace_ecclayout = {
	.useecc = MTD_NANDECC_AUTOPLACE
};
#endif

/* XXX U-BOOT XXX */
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK

/******************************************************************************
 * Support for locking / unlocking operations of some NAND devices
 *****************************************************************************/

#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_LOCK_TIGHT	0x2c
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24
#define NAND_CMD_LOCK_STATUS	0x7a

/**
 * nand_lock: Set all pages of NAND flash chip to the LOCK or LOCK-TIGHT
 *	      state
 *
 * @param mtd		nand mtd instance
 * @param tight		bring device in lock tight mode
 *
 * @return		0 on success, -1 in case of error
 *
 * The lock / lock-tight command only applies to the whole chip. To get some
 * parts of the chip lock and others unlocked use the following sequence:
 *
 * - Lock all pages of the chip using nand_lock(mtd, 0) (or the lockpre pin)
 * - Call nand_unlock() once for each consecutive area to be unlocked
 * - If desired: Bring the chip to the lock-tight state using nand_lock(mtd, 1)
 *
 *   If the device is in lock-tight state software can't change the
 *   current active lock/unlock state of all pages. nand_lock() / nand_unlock()
 *   calls will fail. It is only posible to leave lock-tight state by
 *   an hardware signal (low pulse on _WP pin) or by power down.
 */
int nand_lock(struct mtd_info *mtd, int tight)
{
	int ret = 0;
	int status;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd,
		      (tight ? NAND_CMD_LOCK_TIGHT : NAND_CMD_LOCK),
		      -1, -1);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);

	/* see if device thinks it succeeded */
	if (status & 0x01) {
		ret = -1;
	}

	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_get_lock_status: - query current lock state from one page of NAND
 *			   flash
 *
 * @param mtd		nand mtd instance
 * @param offset	page address to query (muss be page aligned!)
 *
 * @return		-1 in case of error
 *			>0 lock status:
 *			  bitfield with the following combinations:
 *			  NAND_LOCK_STATUS_TIGHT: page in tight state
 *			  NAND_LOCK_STATUS_LOCK:  page locked
 *			  NAND_LOCK_STATUS_UNLOCK: page unlocked
 *
 */
int nand_get_lock_status(struct mtd_info *mtd, loff_t offset)
{
	int ret = 0;
	int chipnr;
	int page;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chipnr = (int)(offset >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);


	if ((offset & (mtd->writesize - 1)) != 0) {
		printf ("nand_get_lock_status: "
			"Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Status */
	page = (int)(offset >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, page & chip->pagemask);

	ret = chip->read_byte(mtd) & (NAND_LOCK_STATUS_TIGHT
					  | NAND_LOCK_STATUS_LOCK
					  | NAND_LOCK_STATUS_UNLOCK);

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_unlock: - Unlock area of NAND pages
 *		  only one consecutive area can be unlocked at one time!
 *
 * @param mtd		nand mtd instance
 * @param start		start byte address
 * @param length	number of bytes to unlock (must be a multiple of
 *			page size nand->writesize)
 *
 * @return		0 on success, -1 in case of error
 */
int nand_unlock(struct mtd_info *mtd, ulong start, ulong length)
{
	int ret = 0;
	int chipnr;
	int status;
	int page;
	struct nand_chip *chip = mtd->priv;
	printf ("nand_unlock: start: %08x, length: %d!\n",
		(int)start, (int)length);

	/* select the NAND device */
	chipnr = (int)(start >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	/* check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	if (!(chip->read_byte(mtd) & NAND_STATUS_WP)) {
		printf ("nand_unlock: Device is write protected!\n");
		ret = -1;
		goto out;
	}

	if ((start & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Start address must be beginning of "
			"nand block!\n");
		ret = -1;
		goto out;
	}

	if (length == 0 || (length & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Length must be a multiple of nand block "
			"size %08x!\n", mtd->erasesize);
		ret = -1;
		goto out;
	}

	/*
	 * Set length so that the last address is set to the
	 * starting address of the last block
	 */
	length -= mtd->erasesize;

	/* submit address of first page to unlock */
	page = (int)(start >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK1, -1, page & chip->pagemask);

	/* submit ADDRESS of LAST page to unlock */
	page += (int)(length >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK2, -1, page & chip->pagemask);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);
	/* see if device thinks it succeeded */
	if (status & 0x01) {
		/* there was an error */
		ret = -1;
		goto out;
	}

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}
#endif

/**
 * get_len_incl_bad
 *
 * Check if length including bad blocks fits into device.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @return image length including bad blocks
 */
static size_t get_len_incl_bad (nand_info_t *nand, loff_t offset,
				const size_t length)
{
	size_t len_incl_bad = 0;
	size_t len_excl_bad = 0;
	size_t block_len;

	while (len_excl_bad < length) {
		block_len = nand->erasesize - (offset & (nand->erasesize - 1));

		if (!nand_block_isbad (nand, offset & ~(nand->erasesize - 1)))
			len_excl_bad += block_len;

		len_incl_bad += block_len;
		offset       += block_len;

		if ((offset + len_incl_bad) >= nand->size)
			break;
	}

	return len_incl_bad;
}

/************************ help variables used by our code ******************************/
/** only used by mtklib */
#define COLLECT_SPARE 1 //close collect spare just change it to 0
#if COLLECT_SPARE
#define RESERVED_SECTOR_IN_BLOCK        0xFFFFFA
u32 g_bReservedSectorNumInBlock = 1;
#endif

/* CacualateLogicalRange: cacualte logical blocks range from physical blocks range
 * pRegion: flash region which has physical blocks range information
 * dwDiskIndex: disk number( should be removed)
 *
 * This function is used to help us caculate logical blocks range from physical blocks range,
 * reserved blocks which is physical blocks range sub logical blocks range is used for our
 * compactor
 */
#define MINIMUM_FLASH_BLOCKS_TO_RESERVE		2
#define BASIC_RESERVED_BLOCK_NUM  			2
#define EXTRA_RESERVED_BLOCK_RATIO_0  		16
#define RESERVED_BLOCK_NUM_FROM_PHY_0 (5)
#define RESERVED_BLOCK_NUM_FROM_PHY_1 (10)
ulong caculate_part_size(nand_info_t *nand, ulong part_size, u_char *name){
	printf("part size %dMB\r\n", part_size/1024/1024);
	if(strcmp(name, "system")==0)
		part_size -= RESERVED_BLOCK_NUM_FROM_PHY_0;
	else if(strcmp(name, "data")==0)
		part_size -= RESERVED_BLOCK_NUM_FROM_PHY_1;
	else
		part_size -= (MINIMUM_FLASH_BLOCKS_TO_RESERVE  + (part_size/nand->erasesize + EXTRA_RESERVED_BLOCK_RATIO_0 - 1)/EXTRA_RESERVED_BLOCK_RATIO_0)*nand->erasesize;
	printf("log part size %dMB\r\n", part_size/1024/1024);
	return part_size;
}

void printBM(u_char *buf,uint32 blocksize){
	uint32 i = 0;
	printf("BM:");
	for(i = 0; i<blocksize; i++){
		if(i%16 == 0){
			printf("\r\n%02x ", buf[i]);
		}else{
			printf("%02x ", buf[i]);
		}
	}
	printf("\r\n");
}


/**
 * nand_write_ext_help:
 *
 *Write ext2/3/4 image to NAND flash(help function).
 *Adjustment partition size in img.
* @param nand	   NAND device
* @param start_offset partition start offset
* @param offset    offset, physical address
* @param length    buffer length
* @param buf	   buffer to read from
* @return	   0 in case of success
*/
static uint8_t *ext_buffer = 0;
static uint8_t *ext_oob_buffer = 0;
static uint32_t last_phy_block = -1;
static uint32_t last_length = 0;
static int nand_write_ext_help(nand_info_t *nand, ulong start_offset, ulong offset, size_t length,
			u_char *buffer, int end)
{
	int rval;
	uint32_t log_sector;
	size_t len_incl_bad;
	uint32_t offset1;
	size_t sectors_per_block = nand->erasesize/nand->writesize;
	size_t sectors_per_block_in_log = sectors_per_block - g_bReservedSectorNumInBlock;
	size_t block_size_in_log = nand->erasesize - nand->writesize*g_bReservedSectorNumInBlock;
	struct 	mtd_oob_ops ops;
	PSectorInfo sector_info;
#ifdef CONFIG_NAND_UPDATE_CHECK
	u_char *read_buff;
	u_char *read_oob_buff;

	read_buff = malloc(nand->writesize * 2);
	read_oob_buff = malloc(sizeof(SectorInfo)*2);

	if((read_buff == NULL) || (read_oob_buff == NULL)) {
		printf("%s: malloc fail\n", __func__);
		rval = -ENOMEM;
		goto WRITE_ERROR;
	}
#endif

	if(!ext_buffer){
		//ext_buffer = vmalloc(nand->erasesize);
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		if(!ext_buffer){
			printf("No memory for ext buffer!\r\n");
			rval = -ENOMEM;
			goto WRITE_ERROR;
		}
		printf("ext_buffer %x\r\n", (unsigned int)ext_buffer);
		memset(ext_buffer, 0xFF, nand->erasesize);
	}

	if(!ext_oob_buffer){
		//ext_oob_buffer = vmalloc(nand->oobsize);
		ext_oob_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_OOB_OFFSET;
		if(!ext_oob_buffer){
			printf("No memory for ext oob buffer!\r\n");
			rval = -ENOMEM;
			goto WRITE_ERROR;
		}
		printf("ext_oob_buffer %x\r\n", (unsigned int)ext_oob_buffer);
		memset(ext_oob_buffer, 0xFF, nand->oobsize);
	}

	if(last_phy_block == -1)
		last_phy_block = start_offset/nand->erasesize;

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
	    (length & (nand->writesize - 1)) != 0) {
		printf ("nand_write_ext_help: Attempt to write non page aligned data %llx, %d\n", offset, (unsigned int)length);
		rval = -EINVAL;
		goto WRITE_ERROR;
	}

	if(last_length>block_size_in_log){
		printf("Something wrong!\r\n");
		rval = -EINVAL;
		goto WRITE_ERROR;
	}

	log_sector = offset/nand->writesize;
	memcpy(ext_buffer + last_length, buffer, length);
	flush_cache(ext_buffer + last_length, length);
	sector_info = (PSectorInfo)(ext_oob_buffer + last_length/nand->writesize*sizeof(SectorInfo));
	sector_info->bBadBlock = 0xFFFF;
	sector_info->wReserved2 = 0;
	sector_info->dwReserved1 = log_sector;
	last_length += nand->writesize;

	//write into flash
	if(last_length == block_size_in_log){
		//printf("Collecting spare for %d\r\n", last_phy_block);
		memcpy(ext_buffer + last_length, ext_oob_buffer, sectors_per_block_in_log*sizeof(SectorInfo));
		flush_cache(ext_buffer + last_length, sectors_per_block_in_log * sizeof(SectorInfo));
		sector_info = (PSectorInfo)(ext_oob_buffer + sectors_per_block_in_log*sizeof(SectorInfo));
		sector_info->bBadBlock = 0xFFFF;
		sector_info->wReserved2 = 0;
		sector_info->dwReserved1 = RESERVED_SECTOR_IN_BLOCK;
		ops.datbuf = ext_buffer;
		ops.oobbuf = ext_oob_buffer;
		ops.ooblen = sizeof(SectorInfo);
		ops.len = nand->erasesize;
		ops.mode = MTD_OOB_PLACE;
		ops.ooboffs = 0;
		while(nand_block_isbad (nand, last_phy_block*nand->erasesize)) {
			printf ("Skip bad block 0x%08llx\n",last_phy_block);
			last_phy_block++;
		}
		rval = -1;
		while(rval != 0)
		{
			flush_cache(ops.oobbuf, ops.ooblen);
			rval = nand->write_oob(nand, last_phy_block*nand->erasesize, &ops);
			//printf("write ext4 blk num %x \n",last_phy_block);
			if(rval != 0){
				nand->block_markbad(nand,last_phy_block*nand->erasesize);
				printf ("NAND write to offset %llx failed %d\n", (last_phy_block*nand->erasesize), rval);
			}
#ifdef CONFIG_NAND_UPDATE_CHECK
			else {
				uint i = 0;
				ops.datbuf = read_buff;
				ops.oobbuf = read_oob_buff;
				ops.ooblen = sizeof(SectorInfo);
				ops.len = nand->writesize;
				ops.mode = MTD_OOB_PLACE;
				ops.ooboffs = 0;
				for(i=0; i<sectors_per_block; i++)  {
					flush_invalid_cache(ops.datbuf, ops.len);
					if(nand->read_oob(nand, last_phy_block * nand->erasesize + i * nand->writesize, &ops)) {
						printf("read back fail\n");
						rval = -1;
						goto WRITE_ERROR;
					}
					if((memcmp(read_buff, ext_buffer + i * nand->writesize, nand->writesize) != 0) ||
						(memcmp(read_oob_buff, ext_oob_buffer + i * sizeof(SectorInfo), sizeof(SectorInfo)) != 0)) {
						printf("read back check 1 fail\n");
						rval = -1;
						goto WRITE_ERROR;
					}
				}

			}
#endif
			last_phy_block++;
		}
		memset(ext_buffer, 0xFF, nand->erasesize);
		memset(ext_oob_buffer, 0xFF, nand->writesize);
		last_length = 0;
	}

	if(end){
		memcpy(ext_buffer + block_size_in_log, ext_oob_buffer, sectors_per_block_in_log*sizeof(SectorInfo));
		flush_cache(ext_buffer + block_size_in_log, sectors_per_block_in_log*sizeof(SectorInfo));
//		sector_info = (PSectorInfo)(ext_oob_buffer + sectors_per_block_in_log*sizeof(SectorInfo));
//		sector_info->bBadBlock = 0xFFFF;
//		sector_info->wReserved2 = 0;
//		sector_info->dwReserved1 = RESERVED_SECTOR_IN_BLOCK;
		if(last_length != 0){
			ops.datbuf = ext_buffer;
			ops.len =   last_length;                   //nand->erasesize;;
			ops.oobbuf = ext_oob_buffer;
			ops.ooblen = sizeof(SectorInfo);
			ops.ooboffs = 0;
			ops.mode = MTD_OOB_PLACE;
			printf("Write block %d with last length %d\r\n", last_phy_block, last_length);
			while(nand_block_isbad (nand, last_phy_block*nand->erasesize)) {
				printf ("Skip bad block 0x%08llx\n",last_phy_block);
				last_phy_block++;
			}
			rval = -1;
			while(rval != 0)
			{
				flush_cache(ops.datbuf, ops.len);
				flush_cache(ops.oobbuf, ops.ooblen);
				rval = nand->write_oob(nand, last_phy_block*nand->erasesize, &ops);
				//printf("write ext4 blk num %x \n",last_phy_block);
				if(rval != 0){
					nand->block_markbad(nand,last_phy_block*nand->erasesize);
					printf ("NAND write to offset %llx failed %d\n", (last_phy_block*nand->erasesize), rval);
				}
#ifdef CONFIG_NAND_UPDATE_CHECK
				else {
					uint i = 0;
					ops.datbuf = read_buff;
					ops.oobbuf = read_oob_buff;
					ops.ooblen = sizeof(SectorInfo);
					ops.len = nand->writesize;
					ops.mode = MTD_OOB_PLACE;
					ops.ooboffs = 0;
					for(i=0; i < last_length/nand->writesize; i++)  {
						flush_invalid_cache(ops.datbuf, ops.len);
						if(nand->read_oob(nand, last_phy_block * nand->erasesize + i * nand->writesize, &ops)) {
							printf("read back fail\n");
							rval = -1;
							goto WRITE_ERROR;
						}
						if((memcmp(read_buff, ext_buffer + i * nand->writesize, nand->writesize) != 0) ||
								(memcmp(read_oob_buff, ext_oob_buffer + i * sizeof(SectorInfo), sizeof(SectorInfo)) != 0)) {
							printf("read back check 2 fail\n");
							rval = -1;
							goto WRITE_ERROR;
						}
					}
				}
#endif
				last_phy_block++;
			}
		}else{
			printf("No valid data, throw the buffer!\r\n");
		}
		printf("end write done\r\n");
		printf("ext_buffer %x\r\next_oob_buffer %x\r\n", (unsigned int)ext_buffer,  (unsigned int)ext_oob_buffer);
		#if 0
		if(ext_buffer)
			vfree(ext_buffer);
		if(ext_oob_buffer);
			vfree(ext_oob_buffer);
		printf("free buffer for ext done\r\n");
		ext_buffer = 0;
		ext_oob_buffer = 0;
		#else
		memset(ext_buffer, 0xFF, nand->erasesize);
		memset(ext_oob_buffer, 0xFF, nand->writesize);
		#endif
		last_phy_block = -1;
		last_length = 0;
	}

	rval = 0;

WRITE_ERROR:
#ifdef CONFIG_NAND_UPDATE_CHECK
	free(read_buff);
	free(read_oob_buff);
#endif
	return rval;
}

/*offset is logical addr
 * nand write ext help2 is a wrapper function to reduce the complex of nand write ext help function
 * @param nand			nand device information structure
 * @param start_offset 	partition offset, physical address
 * @param offset			partition offset, logical address
 * @param length			how long should we write into flash
 * @param buffer 			data buffer
 * we use a buffer to composite the block size data into nand->erasesize.
*/

static u_char *wrapper_buffer = 0;
static uint32_t last_log_sector = -1;
static int nand_write_ext_help2(nand_info_t *nand, ulong start_offset, ulong offset, size_t length, u_char *buffer)
{
	int rval;
	int offset1;
	nand_write_ext_help(nand, start_offset, offset, nand->writesize, buffer, 0);
	return 0;
WRITE_ERROR:
	return rval;
}

static int flush(nand_info_t *nand, ulong start_offset){
	printf("Flushing data into flash!\r\n");
	nand_write_ext_help(nand, start_offset, last_log_sector*nand->writesize, nand->writesize, wrapper_buffer, 1);
	printf("Flush done\r\n");
	printf("wrapper buffer %x\r\n", (unsigned int)wrapper_buffer);
	#if 0
	if(wrapper_buffer)
		vfree(wrapper_buffer);
	wrapper_buffer = 0;
	#else
	memset(wrapper_buffer, 0xFF, nand->writesize);
	#endif
	last_log_sector = -1;
	return 0;
}

/*****************************************************************************/


/*******************************support vfat*************************************/
/** fat boot section struct */
#pragma pack(push ,1)
typedef struct _IMAGE_BPB{
	u16 BPB_BytesPerSec;
	u8 	BPB_SecPerClus;
	u16	BPB_RsvdSecCnt;
	u8	BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_ToSec16;
	u8 	BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32	BPB_HidSec;
	u32	BPB_ToSec32;
	u32	BPB_FATSz32;
	u16 BPB_Flags;
	u16 BPB_FSVer;
	u32 BPB_RootClus;
	u16	BPB_FSInfo;
	u16 BPB_Reserved;
	u8 	reserved[12];
} IMAGE_BPB, *PIMAGE_BPB;

typedef struct _IMAGE_EXTERN_BPB{
	u8 BS_DrvNum;
	u8 BS_Reserved1;
	u8 BS_BootSig;
	u32	BS_VSN;
	u8 BS_VolumeLabel[11];
	u8 BS_SystemID[8];
} IMAGE_EXTERN_BPB, *PIMAGE_EXTERN_BPB;

typedef struct _IMAGE_DBP_SECTOR{
	u8 JmpCnd[3];
	char OSVersion[8];
	IMAGE_BPB bpb;
	IMAGE_EXTERN_BPB bpb_extern;
} IMAGE_DBP_SECTOR, *PIMAGE_DBP_SECTOR;

typedef struct _IMAGE_FSINFO{
	u32 FI_BootSig;
	u8 	FI_Reserved1[480];
	u32 FI_Signature;
	u32 FI_FreeClus;
	u32 FI_NextClus;
	u8 	FI_Reserved2[14];
	u16 FI_EndSig;
} IMAGE_FSINFO, *PIMAGE_FSINFO;

#pragma pack(pop)

/********************************vfat format helper function************/
#define VFAT_BLOCK_SIZE 512
void full_bpb_extern(IMAGE_EXTERN_BPB *bpb_extern){
	char type[] = "FAT32";
	char *p;
	bpb_extern->BS_DrvNum  = 0x80;
	bpb_extern->BS_Reserved1 = 0;
	bpb_extern->BS_BootSig = 0x29;
	bpb_extern->BS_VSN = 0x94525487;
	p = (char *)bpb_extern->BS_VolumeLabel;
	p[0] = 'F';
	p[1] = 'e';
	p[2] = 'n';
	p[3] = 'g';
	p[4] = 'X';
	p[5] = 'i';
	p[6] = 'n';
	p[7] = 'g';
	p[8] = 'Y';
	p[9] = 'i';
	p[10] = 'n';
	memcpy(bpb_extern->BS_SystemID, type, 6);
}

void full_bpb(IMAGE_BPB *bpb){
	bpb->BPB_BytesPerSec = VFAT_BLOCK_SIZE;
	bpb->BPB_SecPerClus = 4096/VFAT_BLOCK_SIZE;
	bpb->BPB_RsvdSecCnt = 32;
	bpb->BPB_NumFATs = 2;
	bpb->BPB_RootEntCnt = 0;
	bpb->BPB_ToSec16 = 0;
	bpb->BPB_Media = 0xF8;
	bpb->BPB_FATSz16 = 0;
	bpb->BPB_SecPerTrk = 0x3F;
	bpb->BPB_NumHeads = 0xFF;
	bpb->BPB_HidSec = 0;
	bpb->BPB_ToSec32 = 0; //
	bpb->BPB_FATSz32 = 0; //
	bpb->BPB_Flags = 0;
	bpb->BPB_FSVer = 0;
	bpb->BPB_RootClus = 2;
	bpb->BPB_FSInfo = 1;
	bpb->BPB_Reserved = 0;
	memset(bpb->reserved, 0x00, 12);
}

void full_dbp(IMAGE_DBP_SECTOR *dbp){
	char version[] = "MSDOS5.0";
	dbp->JmpCnd[0] = 0xEB;
	dbp->JmpCnd[1] = 0x58;
	dbp->JmpCnd[2] = 0x90;
	memcpy(dbp->OSVersion, version, 8);
	full_bpb(&(dbp->bpb));
	full_bpb_extern(&(dbp->bpb_extern));
}

void full_fsinfo(IMAGE_FSINFO *fsinfo){
	fsinfo->FI_BootSig = 0x41615252;
	fsinfo->FI_Signature = 0x61417272;
	fsinfo->FI_NextClus = 2;
	fsinfo->FI_FreeClus = -1;
	fsinfo->FI_EndSig = 0x55AA;
}

ulong caculate_fat(u32 total_sect, u32 resv_sect, u32 fat_num, u32 SecPerClus){
	ulong fat_sect = 0;
	ulong inval_sect = 0;
	ulong tmp = 0;
	while(1){
		fat_sect++;
		inval_sect = total_sect - resv_sect - fat_sect*fat_num;
		tmp = (inval_sect/SecPerClus + 2)*4;
		if(tmp <= fat_sect*VFAT_BLOCK_SIZE)
			return fat_sect;
	}
}

/**
 * nand_format_userdata:
 *
 * format usrdata partition as vfat
 * @param nand 	NAND device
 * @param offset	offset in flash
 * @param length	partition length
 */
int nand_format_userdata(nand_info_t *nand, ulong offset, ulong part_size){
	ulong sector_id = 0;
	int i = 0;
	IMAGE_DBP_SECTOR *block_buffer = vmalloc(VFAT_BLOCK_SIZE);
	IMAGE_BPB *bpb = &(block_buffer->bpb);
	IMAGE_FSINFO *fsinfo;
	u_char *p = vmalloc(VFAT_BLOCK_SIZE);
	if(block_buffer == NULL||p==NULL){
		printf("Can not alloc memory for userdata format function\n");
		return -1;
	}
	ulong log_size = caculate_part_size(nand, part_size,"usrdata");
	printf("Format usrdata partition(physical size 0x%x, logical size 0x%x)\n", part_size, log_size);
	memset(block_buffer, 0x00, VFAT_BLOCK_SIZE);
	memset(p, 0x00, VFAT_BLOCK_SIZE);
	full_dbp((IMAGE_DBP_SECTOR *)block_buffer);
	bpb->BPB_ToSec32 = log_size/VFAT_BLOCK_SIZE;
	bpb->BPB_FATSz32 = caculate_fat(bpb->BPB_ToSec32, bpb->BPB_RsvdSecCnt,bpb->BPB_NumFATs, bpb->BPB_SecPerClus);//(bpb->BPB_ToSec32*4)/VFAT_BLOCK_SIZE;
	((u_char *)block_buffer)[510] = 0x55;
	((u_char *)block_buffer)[511] = 0xAA;
	printf("total sector 0x%x, fat size 0x%x\n", bpb->BPB_ToSec32, bpb->BPB_FATSz32);
	/** write boot section */
	//printBM(block_buffer, VFAT_BLOCK_SIZE);
	nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)block_buffer);
	sector_id = 1;
	fsinfo = (IMAGE_FSINFO *)p;
	full_fsinfo(fsinfo);
	nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
	sector_id++;
	memset(p, 0x00, VFAT_BLOCK_SIZE);
	for(i = 2; i<bpb->BPB_RsvdSecCnt; i++){
		nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
		sector_id++;
	}
	/** write FAT table */
	((u32 *)p)[0] = 0x0FFFFFF8;
	((u32 *)p)[1] = 0x0FFFFFFF;
	((u32 *)p)[2] = 0x0FFFFFFF;
	sector_id = bpb->BPB_RsvdSecCnt;
	//printBM(p, VFAT_BLOCK_SIZE);
	nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
	sector_id++;
	memset(p, 0x00, 12);
	for(i = 1; i<bpb->BPB_FATSz32; i++){
		nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
		sector_id++;
	}
	/** write 2th fat table */
	((u32 *)p)[0] = 0x0FFFFFF8;
	((u32 *)p)[1] = 0x0FFFFFFF;
	((u32 *)p)[2] = 0x0FFFFFFF;
	nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
	sector_id++;
	memset(p, 0x00, 12);
	for(i = 1; i<bpb->BPB_FATSz32; i++){
		nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
		sector_id++;
	}

	nand_write_ext_help2(nand, offset, sector_id*VFAT_BLOCK_SIZE, VFAT_BLOCK_SIZE, (u_char *)p);
	flush(nand, offset);
	vfree(block_buffer);
	vfree(p);
	return 0;
}


/**
 * nand_write_with_lognum:
 *
 * Write image to NAND flash.(used to write fat image)
 * Blocks which are marked as bad are skipped and the data is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks, and when we writing the image to flash, the data
 * used by mtkftl is added to oob zone
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param buf           buffer to read from with oob data
 * @return		0 in case of success
 */

int nand_write_with_lognum(nand_info_t *nand, loff_t offset, size_t *length,
			u_char *buffer)
{
	int rval, i;
	size_t left_to_write = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;
	size_t log_num = 0;
	struct nand_chip *chip = nand->priv;
	size_t sectors_per_block =1<<(chip->phys_erase_shift - chip->page_shift);
	SectorInfo sector_info;
	size_t cnt = 0;
	struct 	mtd_oob_ops ops;
#if COLLECT_SPARE
	size_t reserved_size = nand->writesize*g_bReservedSectorNumInBlock;
	uint8_t *temp_buf = kzalloc(nand->erasesize, GFP_KERNEL);
	int print_flag = 0;
	printk("USING COLLECT_SPARE ON!\r\n");
#endif

	printf("%s is called with nand %d, offset 0x%x, length %d, buffer 0x%x\n", __FUNCTION__, nand->index, (unsigned int)offset,  (unsigned int)(*length), (unsigned int)buffer);

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
		(*length & (nand->writesize - 1)) != 0) {
		printf ("Attempt to write non page aligned data\n");
		return -EINVAL;
	}

	/*Reject writes, which are not block aligned. we will support this condition in the future*/
	if((offset&(nand->erasesize - 1)) != 0){
		printf("Attempt to write non block align data, now we just does not support it!\r\n");
		return -EINVAL;
	}

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) >= nand->size) {
		printf ("Attempt to write outside the flash area\n");
		return -EINVAL;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;
#if COLLECT_SPARE
		int collect_flag = 0;
		block_offset += reserved_size;
		memset(temp_buf, 0xff, nand->erasesize);
#endif
		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n",offset & ~(nand->erasesize - 1));
			//align with the next block
			offset += nand->erasesize - block_offset + reserved_size;
			//escapt the data should be write in this bad block
			left_to_write -= (nand->erasesize - block_offset);
			continue;
		}

		if(left_to_write <(nand->erasesize - block_offset)){
			write_size = left_to_write;
#if COLLECT_SPARE
			//not full block, do not collect spare data
			collect_flag = 0;
#endif
		}
		else{
			write_size = nand->erasesize - block_offset;
#if COLLECT_SPARE
			//full block, collect spare data :)
			collect_flag = 1;
#endif
		}

		cnt = write_size>>chip->page_shift;
		ops.len = write_size;
		ops.mode = MTD_OOB_RAW;
		ops.datbuf = p_buffer;
		ops.ooblen = sizeof(sector_info);
		ops.oobbuf = (uint8_t *)malloc(sectors_per_block*sizeof(sector_info));
		ops.ooboffs = 0;
		for(i = 0; i<cnt; i++){
			sector_info.bBadBlock = 0xffff;
			sector_info.dwReserved1 = log_num++;
			sector_info.wReserved2 = 0x1;
			memcpy(ops.oobbuf + i*sizeof(sector_info), &sector_info, sizeof(sector_info));
		}
#if COLLECT_SPARE
		//now if this full block, we should collect spare data.
		if(collect_flag == 1){
			//make sure again.
			if(cnt == sectors_per_block - g_bReservedSectorNumInBlock){
				for(i = 0; i<g_bReservedSectorNumInBlock; i++){
					sector_info.bBadBlock = 0xffff;
					sector_info.dwReserved1 = RESERVED_SECTOR_IN_BLOCK;
					sector_info.wReserved2 = 0x0;
					memcpy(ops.oobbuf + (cnt + i)*sizeof(sector_info), &sector_info, sizeof(sector_info));
					cnt++;
				}
				memcpy(temp_buf, p_buffer, write_size);
				memcpy(temp_buf + write_size, ops.oobbuf, sectors_per_block*sizeof(sector_info));
				ops.datbuf = temp_buf;
				ops.len = nand->erasesize;
				if(!print_flag)
					printk("(cnt = %d(write_size %x, page_shift %d), right = %d, nand->erasesize %x, nand->writesize %x, block_offset %d, left_to_write %x!\r\n", cnt, write_size, chip->page_shift,
						sectors_per_block-g_bReservedSectorNumInBlock, nand->erasesize, nand->writesize, block_offset, left_to_write);
				print_flag = 1;
			}
			else{
				printk("Something wrong, just exit(cnt = %d(write_size %x, page_shift %d), right = %d, nand->erasesize %x, nand->writesize %x, block_offset %d, left_to_write %x!\r\n", cnt, write_size, chip->page_shift,
					sectors_per_block-g_bReservedSectorNumInBlock, nand->erasesize, nand->writesize, block_offset, left_to_write);
				free(ops.oobbuf);
				free(temp_buf);
				return -1;
			}
		}
#endif

		rval = nand->write_oob(nand, offset, &ops);
		if(rval != 0){
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			free(ops.oobbuf);
			free(temp_buf);
			return rval;
		}

		left_to_write -= write_size;
#if COLLECT_SPARE
		if(collect_flag == 1)
			offset += nand->erasesize;
		else
			offset += write_size;
#else
		offset += write_size;
#endif
		p_buffer += write_size;
		free(ops.oobbuf);
	}

	free(temp_buf);
	return 0;
}

/***************************************support ext3******************************/

//is block empty
//0 means yes, -1 means no.
int is_block_empty(int index, u_char *buffer){
	int ind = index/8;
	int offset = index%8;
	return buffer[ind]&(1<<offset)?-1:0;
	//return -1;
}

void set_block_bit(int index, u_char *buffer){
	int ind = index/8;
	int offset = index%8;
	buffer[ind] |= 1<<offset;
}

/**
 * nand_write_ext_image:
 *
 *Write ext2/3/4 image to NAND flash.
 *Adjustment partition size in img.
* @param nand	   NAND device
* @param offset    offset in flash
* @param length    buffer length
* @param buf		   buffer to read from
* @return	   0 in case of success
*/
#define EXT_BLOCK_SIZE_1024
//#define EXT_BLOCK_SIZE_4096
#ifdef EXT_BLOCK_SIZE_1024
#define EXT_BLOCK_SIZE 1024
#endif

#ifdef EXT_BLOCK_SIZE_4096
#define EXT_BLOCK_SIZE 4096
#endif

typedef struct sparse_header {
  __le32	magic;		/* 0xed26ff3a */
  __le16	major_version;	/* (0x1) - reject images with higher major versions */
  __le16	minor_version;	/* (0x0) - allow images with higer minor versions */
  __le16	file_hdr_sz;	/* 28 bytes for first revision of the file format */
  __le16	chunk_hdr_sz;	/* 12 bytes for first revision of the file format */
  __le32	blk_sz;		/* block size in bytes, must be a multiple of 4 (4096) */
  __le32	total_blks;	/* total blocks in the non-sparse output image */
  __le32	total_chunks;	/* total chunks in the sparse input image */
  __le32	image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
				/* as 0. Standard 802.3 polynomial, use a Public Domain */
				/* table implementation */
} sparse_header_t;

#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3
#define CHUNK_TYPE_CRC32    0xCAC4

typedef struct chunk_header {
  __le16	chunk_type;	/* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  __le16	reserved1;
  __le32	chunk_sz;	/* in blocks in output image */
  __le32	total_sz;	/* in bytes of chunk input file including chunk header and data */
} chunk_header_t;

#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))

int nand_write_ext4_image(nand_info_t *nand, ulong offset, size_t *length, u_char *buffer)
{
    int   ival = 0;
    ulong start_offset = offset;
    uint32_t block_size = 0;
    uchar *pfileBuffer = NULL;
    int32_t filelen = (int32_t)(*length);
    size_t datalen = 0;
    uint32_t chunk_cnt = 0;
    uint32_t fsoffset = 0;
    uint32_t dump_ext4_blockcnt = 10;
    uint32_t uLoop = 0;

    if ((offset & (nand->writesize - 1)) != 0)
    {
	printf ("nand_write_ext4_img: Attempt to write non page aligned data offset %llx\n", offset);
	return -EINVAL;
    }
    sparse_header_t *ptSparseHeader = NULL;
    chunk_header_t  *ptChunkHeader = NULL;

    ptSparseHeader = (sparse_header_t *)buffer;
    block_size = ptSparseHeader->blk_sz;
    chunk_cnt = ptSparseHeader->total_chunks;

   // printf ("nand_write_ext4_img:ptSparseHeader->blk_sz = 0X%x\r\n", ptSparseHeader->blk_sz);
   // printf ("nand_write_ext4_img:ptSparseHeader->total_chunks = 0X%x\r\n", ptSparseHeader->total_chunks);
   // printf ("nand_write_ext4_img:ptSparseHeader->total_blks = 0X%x\r\n", ptSparseHeader->total_blks);
   // printf ("nand_write_ext4_img:filelen = 0X%x\r\n", filelen);
    pfileBuffer = buffer + sizeof(sparse_header_t);
    filelen -= sizeof(sparse_header_t);
    while ((chunk_cnt > 0) && (filelen > 0))
    {
        ptChunkHeader = (chunk_header_t *)pfileBuffer;
        datalen = ptChunkHeader->chunk_sz * block_size;
        if (CHUNK_TYPE_RAW == ptChunkHeader->chunk_type)
        {
            pfileBuffer += CHUNK_HEADER_LEN;
            for (uLoop = 0; uLoop < ptChunkHeader->chunk_sz; uLoop++)
            {
                nand_write_ext_help2(nand, start_offset, fsoffset, block_size, pfileBuffer);
                fsoffset += block_size;
                pfileBuffer += block_size;
            }
            filelen -= ptChunkHeader->total_sz;
            chunk_cnt--;

        }
        else if (CHUNK_TYPE_DONT_CARE == ptChunkHeader->chunk_type)
        {
            fsoffset += datalen;
            pfileBuffer += ptChunkHeader->total_sz;
            filelen -= ptChunkHeader->total_sz;
            chunk_cnt--;
        }
        else
        {
            printf ("nand_write_ext4_img:ptChunkHeader->chunk_type error\r\n");
            return -EINVAL;
        }
     //   printf ("nand_write_ext4_img:chunk_cnt = 0X%x; filelen = 0X%x\r\n", chunk_cnt, filelen);
     //   printf ("nand_write_ext4_img:ptChunkHeader->chunk_type = 0X%x\r\n", ptChunkHeader->chunk_type);
     //   printf ("nand_write_ext4_img:ptChunkHeader->chunk_sz = 0X%x\r\n", ptChunkHeader->chunk_sz);
     //   printf ("nand_write_ext4_img:ptChunkHeader->total_sz = 0X%x\r\n\r\n", ptChunkHeader->total_sz);
    }
    if (0 != chunk_cnt)
    {
        printf ("nand_write_ext4_img:chunk_cnt error\r\n");
        return -EINVAL;
    }

    flush(nand, start_offset);
    printf("Write %x bytes\r\n", ptSparseHeader->total_blks * block_size);
    return 0;
}
int nand_write_ext_image(nand_info_t *nand, u_char *part_name, ulong offset, ulong part_size, size_t *length,
							u_char *buffer)
{
	int rval;
	int i = 0;
	ulong start_offset = offset;
	u_char *pbuffer = buffer;
	u_char *gd_buffer = 0;  //group descriptor buffer copied to every blockgroup
	u_char *ext_bm = 0;  //block bitmap used for extends block group
	u_char *block_bitmap = 0; //block bitmap used for extends block group
	u_char *inode_bitmap = 0; //inode bitmap used for extends block group
	u_char *start_point = 0;
	uint32_t block_count = 0, group_count_old = 0, group_count_new = 0;
	uint32_t block_id = 0;
	uint32_t block_size = EXT_BLOCK_SIZE;
	uint32_t write_size = 0;
	uint32_t boot_sector_size = 0;
	uint32_t org_block_count = 0;
	uint32_t group_des_blocks_new = 0, group_des_blocks_old = 0, inodetable_blocks_per_group = 0;
	uint32_t free_blocks_per_group = 0;
	groupdescriptor *temp_buffer = NULL;

	printf("Write offset %x, part_length %x, length %x\r\n", (unsigned int)offset, (unsigned int)part_size, (unsigned int)(*length));
	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0) {
		printf ("nand_write_ext_img: Attempt to write non page aligned data offset %llx\n", offset);
		rval = -EINVAL;
		goto WRITE_ERROR;
	}

#ifdef EXT_BLOCK_SIZE_1024   //ext has not boot sector when the block size is 4096
	nand_write_ext_help2(nand, start_offset, block_id*block_size, block_size, pbuffer);
	//skip boot sector
	pbuffer += block_size;
	block_id++;
#endif

	start_point = pbuffer;
	superblock *sb;
#ifdef EXT_BLOCK_SIZE_1024
	sb = (superblock *)start_point;
#endif

#ifdef EXT_BLOCK_SIZE_4096
	sb = (superblock *)(start_point + 1024);
#endif
	group_count_old = sb->s_blocks_count/sb->s_blocks_per_group;
	if(group_count_old == 0)
		group_count_old += sb->s_blocks_count%sb->s_blocks_per_group?1:0;
	group_des_blocks_old = group_count_old*sizeof(groupdescriptor)/block_size;
	group_des_blocks_old += (group_count_old*sizeof(groupdescriptor))%block_size?1:0;
	block_count = sb->s_blocks_per_group;
	gd_buffer = start_point + block_size; // the second block is one of group descriptor sectors
	temp_buffer = (groupdescriptor *)gd_buffer;
	//original data
	printf("org data:\r\n");
	printf("+inode count = %d\r\n", sb->s_inodes_count);
	printf("+block count = %d\r\n", sb->s_blocks_count);
	printf("+reserved block count = %d\r\n", sb->s_r_blocks_count);
	printf("+free blocks count = %d\r\n", sb->s_free_blocks_count);
	printf("+free inodes count = %d\r\n", sb->s_free_inodes_count);
	printf("+logical block size = %d\r\n", sb->s_log_block_size);
	printf("+logical frag size = %d\r\n", sb->s_log_frag_size);
	printf("+blocks per group = %d\r\n", sb->s_blocks_per_group);
	printf("+frags per group = %d\r\n", sb->s_frags_per_group);
	printf("+inodes  per group = %d\r\n", sb->s_inodes_per_group);
	//new data (common super block for all of the block groups)
#if 1
	org_block_count = sb->s_blocks_count;
	sb->s_blocks_count = caculate_part_size(nand, part_size, part_name)/block_size;
	group_count_new = (sb->s_blocks_count/sb->s_blocks_per_group);
	if(group_count_new == 0)
		group_count_new += (sb->s_blocks_count%sb->s_blocks_per_group?1:0);
	group_des_blocks_new = group_count_new*sizeof(groupdescriptor)/block_size;
	group_des_blocks_new += (group_count_new*sizeof(groupdescriptor))%block_size?1:0;
	if(group_des_blocks_new != group_des_blocks_old){
		printf("group des sectors no equal, now we do not support it.(old %d, new %d(part size %x, block count %d, %d))\r\n", group_des_blocks_old, group_des_blocks_new, (unsigned int)part_size, (unsigned int)caculate_part_size(nand, part_size, part_name), sb->s_blocks_count);
		group_des_blocks_new = group_des_blocks_old;
		group_count_new = group_des_blocks_new*block_size/sizeof(groupdescriptor);
	}
	if(sb->s_blocks_count > group_count_new*sb->s_blocks_per_group)
		sb->s_blocks_count = group_count_new*sb->s_blocks_per_group;
	sb->s_inodes_count = group_count_new*sb->s_inodes_per_group;
	inodetable_blocks_per_group = sb->s_inodes_per_group*sizeof(inode)/block_size;
	inodetable_blocks_per_group += (sb->s_inodes_per_group*sizeof(inode)%block_size?1:0);
	free_blocks_per_group = sb->s_blocks_per_group - (1 + group_des_blocks_new + 2 + inodetable_blocks_per_group); //super block + group description sectors + 2 maps + inode table sectors
	sb->s_free_blocks_count = sb->s_free_blocks_count + (group_count_new-group_count_old)*free_blocks_per_group;
	sb->s_free_inodes_count = sb->s_free_inodes_count + (group_count_new-group_count_old)*sb->s_inodes_per_group;

	if(org_block_count%sb->s_blocks_per_group != 0){
		if(group_count_new == group_count_old){
			sb->s_free_blocks_count += (sb->s_blocks_count - org_block_count);
		}else{
			sb->s_free_blocks_count += (sb->s_blocks_per_group - org_block_count%sb->s_blocks_per_group);
		}
	}

	//new data(common group descriptors for all of the block groups)
	printf("group count old %d\r\n", group_count_old);
	printf("group count new %d\r\n", group_count_new);
	{
		printf("fill new block group descriptor!\r\n");
		//fill new group descriptors
		uint32_t i;
		#if 1
		for(i = 0; i<group_count_old; i++){
			printf("block group %d\r\n", i);
			printf("block bitmap %d\r\n", temp_buffer[i].bg_block_bitmap);
			printf("inode bitmap %d\r\n", temp_buffer[i].bg_inode_bitmap);
			printf("free block count %d\r\n", temp_buffer[i].bg_free_blocks_count);
			printf("free inodes count %d\r\n", temp_buffer[i].bg_free_inodes_count);
		}
		#endif
		for(i = group_count_old; i<group_count_new; i++){
			//memcpy(&(temp_buffer[i]),&(temp_buffer[0]), sizeof(groupdescriptor));
			temp_buffer[i].bg_block_bitmap = temp_buffer[i-1].bg_block_bitmap + sb->s_blocks_per_group;     //i*sb->s_blocks_per_group + temp_buffer[0].bg_block_bitmap + group_des_blocks_new;
			temp_buffer[i].bg_inode_bitmap = temp_buffer[i-1].bg_inode_bitmap + sb->s_blocks_per_group;    //temp_buffer[i].bg_block_bitmap + 1;
			temp_buffer[i].bg_inode_table =  temp_buffer[i-1].bg_inode_table + sb->s_blocks_per_group;     //temp_buffer[i].bg_inode_bitmap + 1;
			temp_buffer[i].bg_free_blocks_count = free_blocks_per_group;
			temp_buffer[i].bg_free_inodes_count = sb->s_inodes_per_group;
			temp_buffer[i].bg_used_dirs_count = 0;
			#if 1
			printf("block group %d\r\n", i);
			printf("block bitmap %d\r\n", temp_buffer[i].bg_block_bitmap);
			printf("inode bitmap %d\r\n", temp_buffer[i].bg_inode_bitmap);
			printf("free block count %d\r\n", temp_buffer[i].bg_free_blocks_count);
			printf("free inodes count %d\r\n", temp_buffer[i].bg_free_inodes_count);
			#endif
		}
	}
	//new data(common blocks bitmap and inode bitmap for free block groups)
	block_bitmap = vmalloc(block_size);
	memset(block_bitmap, 0x00, block_size);
	{
		int cnt;
		for(cnt = 0; cnt<(temp_buffer[0].bg_block_bitmap + 2 + inodetable_blocks_per_group); cnt++){
			set_block_bit(cnt, block_bitmap);
		}
	}
	inode_bitmap = vmalloc(block_size);
	memset(inode_bitmap, 0x00, block_size);
	printf("new data:\r\n");
	printf("+inode count = %d\r\n", sb->s_inodes_count);
	printf("+block count = %d\r\n", sb->s_blocks_count);
	printf("+reserved block count = %d\r\n", sb->s_r_blocks_count);
	printf("+free blocks count = %d\r\n", sb->s_free_blocks_count);
	printf("+free inodes count = %d\r\n", sb->s_free_inodes_count);
	printf("+logical block size = %d\r\n", sb->s_log_block_size);
	printf("+logical frag size = %d\r\n", sb->s_log_frag_size);
	printf("+blocks per group = %d\r\n", sb->s_blocks_per_group);
	printf("+frags per group = %d\r\n", sb->s_frags_per_group);
	printf("+inodes  per group = %d\r\n", sb->s_inodes_per_group);
	//start ...
#endif
	for(i = 0; i<group_count_old; i++){
		int j = 0;
		int count = 0;
		//write super block
		printf("writing the %d block group\r\n", i);
		nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, (u_char *)start_point);
		printf("write super block %d\r\n", block_id);
		block_id++;
		pbuffer += block_size;
		//write group descriptors
		printf("write group des block %d\r\n", block_id);
		for(j=0; j<group_des_blocks_old; j++){
			nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, gd_buffer + j*block_size);
			block_id++;
			pbuffer += block_size;
		}

		ext_bm = block_size*temp_buffer[i].bg_block_bitmap + buffer; //start_point - block_size;
		printf("Block Mapping sector %d\r\n", temp_buffer[i].bg_block_bitmap);
		printBM(ext_bm, block_size);
		for(j = group_des_blocks_old + 1; j<block_count; j++){
			//not empty, write it.
			if(is_block_empty(j , ext_bm)){
				nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, pbuffer);
				write_size += block_size;
				count++;
			}
			block_id++;
			pbuffer += block_size;
		}
		printf("print count %d\r\n", count);
	}
#if 1
	//write new block groups
	for(i = group_count_old; i<group_count_new; i++){
		int j;
		int org_block_id = block_id;
		//super block
		nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, (u_char *)start_point);
		block_id++;
		//group descriptors block
		for(j = 0; j<group_des_blocks_new; j++){
			nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, gd_buffer+j*block_size);
			block_id++;
		}
		//block bitmap
		nand_write_ext_help2(nand, start_offset, temp_buffer[i].bg_block_bitmap*block_size + boot_sector_size, block_size, block_bitmap);;
		//inode bitmap
		nand_write_ext_help2(nand, start_offset, temp_buffer[i].bg_inode_bitmap*block_size + boot_sector_size, block_size, inode_bitmap);
		//inode table blocks
		block_id = temp_buffer[i].bg_inode_table;
		for(j = 0; j<inodetable_blocks_per_group; j++){
			nand_write_ext_help2(nand, start_offset, block_id*block_size + boot_sector_size, block_size, inode_bitmap);
			block_id++;
		}
		block_id = org_block_id + sb->s_blocks_per_group;
	}
#endif
	flush(nand, start_offset);
	vfree(block_bitmap);
	vfree(inode_bitmap);
	block_bitmap = 0;
	inode_bitmap = 0;
	printf("Write %x bytes\r\n", write_size);
	return 0;
WRITE_ERROR:
	vfree(block_bitmap);
	vfree(inode_bitmap);
	block_bitmap = 0;
	inode_bitmap = 0;
	return rval;
}

/*************************************support yaffs2******************************/

#define YAFFS2_OOB_SIZE 64
int nand_write_yaffs2_image(nand_info_t *nand, loff_t offset, size_t *length,
			u_char *buffer)
{
	int rval;
	size_t left_to_write = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;
	size_t oobsize = YAFFS2_OOB_SIZE; // nand->oobsize;  //should be change.
	size_t datasize = nand->writesize;
	int datapages = 0;
	int skip_firstblock = 1;
	u_char *oob_buffer = vmalloc(oobsize*(nand->erasesize/nand->writesize));
	u_char *data_buffer = vmalloc(datasize*(nand->erasesize/nand->writesize));

	if(oob_buffer == 0 || data_buffer == 0){
		printf("Can not alloc memory for the function of nand_write_yaffs2_image\r\n");
		return -EINVAL;
	}

	//adjustment the length of image
	*length = *length/(datasize+oobsize)*(datasize+oobsize);
	printf("length %lld, datasize %lld, oobsize %lld\r\n", *length, datasize, oobsize);

	/* make sure its appropriate length */
	if(*length%(datasize + oobsize) != 0){
		printf("Unappropriate length of the image.\r\n");
		return -EINVAL;
	}

	datapages = *length/(datasize + oobsize);
	*length = datapages*datasize;
	left_to_write = *length;

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
	    (*length & (nand->writesize - 1)) != 0) {
		printf ("Attempt to write non page aligned data\n");
		return -EINVAL;
	}

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) >= nand->size) {
		printf ("Attempt to write outside the flash area\n");
		return -EINVAL;
	}

#if 0
	if (len_incl_bad == *length) {
		rval = nand_write (nand, offset, length, buffer);
		if (rval != 0)
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);

		return rval;
	}
#endif

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;
		int page_count = 0;
		int i;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			left_to_write -= nand->erasesize - block_offset;
			continue;
		}

		if(skip_firstblock == 1){
			skip_firstblock = 0;
			continue;
		}

		if (left_to_write < (nand->erasesize - block_offset))
			write_size = left_to_write;
		else
			write_size = nand->erasesize - block_offset;

		page_count = write_size/nand->writesize;

		for(i = 0; i<page_count; i++){
			memcpy(data_buffer+i*datasize, p_buffer + i*(datasize+oobsize), datasize);
			memcpy(oob_buffer+i*oobsize, p_buffer+datasize+i*(datasize+oobsize), oobsize);
		}

		struct mtd_oob_ops ops;
		ops.datbuf = data_buffer;
		ops.len = write_size;
		ops.mode = MTD_OOB_RAW;
		ops.oobbuf = oob_buffer;
		ops.ooblen = oobsize;
		ops.ooboffs = 2;
		//printf("Write %lld bytes to offset %lld\r\n", write_size, offset);
		rval = nand->write_oob(nand, offset, &ops);
		if (rval != 0) {
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
		offset        += write_size;
		p_buffer      += (datasize + oobsize)*page_count;
	}
	return 0;
}
#ifdef CONFIG_NAND_UPDATE_CHECK
int nand_data_check_skipbad(nand_info_t *nand, loff_t offset, size_t length, u_char *write_data)
{
	int rval = -1;
	u_char *read_buf;
	size_t left_to_read = length;
	u_char *p_buffer = write_data;

	read_buf = malloc(nand->erasesize);
	if(read_buf == NULL) {
		printf("%s:malloc fail\n", __func__);
		rval = -1;
		goto OUT;
	}
	memset(read_buf, 0, nand->erasesize);

	while(left_to_read > 0) {

		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_size;
		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n", offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			left_to_read -= nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_size = left_to_read;
		else
			read_size = nand->erasesize - block_offset;

		rval = nand_read(nand, offset, &read_size, read_buf);
		if (rval != 0) {
			printf ("NAND write to offset %llx failed %d\n", offset, rval);
			length -= left_to_read;
			goto OUT;
		}

		if(memcmp(read_buf, p_buffer, read_size) != 0) {
			printf("Read back check fail\n");
			rval = -2;
			goto OUT;
		}
		left_to_read  -= read_size;
		offset        += read_size;
		p_buffer      += read_size;
	}
	rval = 0;
OUT:
	free(read_buf);
	return rval;
}
#endif
/*************************************************************************/

/**
 * nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param buf           buffer to read from
 * @return		0 in case of success
 */
int nand_write_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
			u_char *buffer)
{
	int rval;
	size_t left_to_write = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;
#ifdef CONFIG_NAND_UPDATE_CHECK
	size_t arg_offset = offset;
	size_t arg_length = *length;
#endif

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
	    (*length & (nand->writesize - 1)) != 0) {
		printf ("Attempt to write non page aligned data\n");
		return -EINVAL;
	}

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) >= nand->size) {
		printf ("Attempt to write outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = nand_write (nand, offset, length, buffer);
		if (rval != 0)
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);

		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (nand->erasesize - block_offset))
			write_size = left_to_write;
		else
			write_size = nand->erasesize - block_offset;

		rval = nand_write (nand, offset, &write_size, p_buffer);
		if (rval != 0) {
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
		offset        += write_size;
		p_buffer      += write_size;
	}
#ifdef CONFIG_NAND_UPDATE_CHECK
	if(nand_data_check_skipbad(nand, arg_offset, arg_length, buffer)) {
		printf("check fail\r\n");
		return -1;
	}
	else
		return 0;

#else
	return 0;
#endif
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is readen
 * instead as long as the image is short enough to fit even after skipping the
 * bad blocks.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds remaining bytes to read
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		       u_char *buffer)
{
	int rval;
	size_t left_to_read = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) >= nand->size) {
		printf ("Attempt to read outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = nand_read (nand, offset, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;
		printf ("NAND read from offset %llx failed %d\n",
			(ulong)offset, rval);
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skipping bad block 0x%08llx\n",
				(ulong)(offset & ~(nand->erasesize - 1)));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = nand_read (nand, offset, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			printf ("NAND read from offset %llx failed %d\n",
				(ulong)offset, rval);
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

int nand_rw_offset(nand_info_t *nand, size_t *offset, size_t *length)
{
	int rval;
	size_t len_left = *length;
	size_t off = *offset;

	while(nand_block_isbad (nand, off & ~(nand->erasesize - 1))){
		size_t block_offset = off & (nand->erasesize - 1);
		printf ("Skip bad block 0x%08llx\n",
				off & ~(nand->erasesize - 1));
			off += nand->erasesize - block_offset;
			len_left -= nand->erasesize;
			if(len_left <= 0){
				printf ("All bad block\r\n");
			}
	}

	return off;
}



static uchar *common_buffer = NULL;
//offset: data's offset to partition start address.
//type: partition type,raw or ext4 or vfat
//end: last time write of the partition
int write_nand_ex(uchar* buf,ulong length,char * partitionName,ulong offset,char* type,int end)
{
   nand_info_t *nand = &nand_info[nand_curr_device];
   ulong uCommonBufSize=nand->writesize*(nand->erasesize/nand->writesize-1);
   ulong datalength,i,size,off;
   uchar *databuf;
   uchar real_end = 0;
   struct mtd_device *dev;
   struct part_info *mtd_part=NULL;
   u8 pnum;
   int ret;

   if(!mtd_part){
   	if((find_dev_and_part(partitionName, &dev, &pnum, &mtd_part) != 0)){
		printf("find dev and part fail\r\n");
     		return -1;
   	}
   }

   //alloc memory
   if(!common_buffer){
	//common_buffer = vmalloc(uCommonBufSize);
	common_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_COM_OFFSET;
   	if(!common_buffer){
		printf("Can not alloc memory for common_buffer\r\n");
		return -1;
   	}
	printf("common_buffer %x\r\n", common_buffer);
	memset(common_buffer, 0xFF, uCommonBufSize);
   }

   //start write data
   if(0 == strcmp(type,"raw")){
	char buf1[10] = {0};
	char buf2[10] = {0};
	char buf3[16] = {0};
	char *argv[7] = {"nand", "write"};

	sprintf(buf1, "%x", buf);
	argv[2] = buf1;
	argv[3] = partitionName;
	sprintf(buf2, "%x", length);
	argv[4] = buf2;
	sprintf(buf3, "%x", offset);
	argv[5] = buf3;
	//argv[0]="nand"
	//argv[1]="write"
	//argv[2]="buf"
	//argv[3]="partitionName"
	//argv[4]="length"
	//argv[5]="offset inner partition"
	return do_nand(NULL, 0, 6, argv);
   }else if(0 == strcmp(type,"ext4")){
   	datalength=length;
	databuf=buf;
	off=offset;
	while(datalength>0){
		//1.copy data to buffer
		if(datalength>uCommonBufSize)
		{
			size=uCommonBufSize;
		}
		else
		{
			size=datalength;
			if(end == 1)
				real_end = 1;
		}
		memcpy(common_buffer,databuf,size);
		datalength-=size;
		databuf+=size;

		//2.write data to nand
		for(i=0;i<size/nand->writesize;i++){
			if(real_end == 1 &&(i == (size/nand->writesize -1)))
			{
				ret = nand_write_ext_help(nand,  mtd_part->offset, off, nand->writesize,common_buffer+i*nand->writesize, 1);
			}
			else
			{
				ret = nand_write_ext_help(nand,  mtd_part->offset, off, nand->writesize,common_buffer+i*nand->writesize, 0);
			}
			//nand_write_ext_help2(nand, mtd_part->offset, off, nand->writesize, common_buffer+i*nand->writesize);
			if(ret)
				return -1;
			off+=nand->writesize;
		}
		memset(common_buffer, 0xFF, uCommonBufSize);
	}

   }else{
	printf("[write_nand_ex] partition type error\r\n");
	return -1;
   }

   return 0;
}

uint64_t r_bdoffset = 0;
uint64_t r_ext4respage = 0;
uint32_t reserved_logical_num;
uint32_t res_offset;

#define  RE_FAIL  (-1)

int nand_rw_start()
{
	int sectors_per_block;
	nand_info_t *nand = &nand_info[nand_curr_device];

	r_bdoffset = 0;
	r_ext4respage = 0;
	res_offset = 0;
	reserved_logical_num = nand->erasesize/ nand->writesize - g_bReservedSectorNumInBlock;
	sectors_per_block = nand->erasesize/ nand->writesize;

	return 0;
}

int nand_rw_end(void)
{
	r_bdoffset = 0;
	res_offset = 0;
	r_ext4respage = 0;
	return 0;
}

int64_t nand_read_raw_image(void* buf, uint64_t offset, uint64_t size)
{
	uint64_t ofs, end_addr = 0;
	long long blockstart = 1;
	int i, bs, badblock = 0;

	int firstblock = 1;

	unsigned char *readbuf = NULL;

	int ret;
	size_t *len;
	uint64_t start_addr;
	uint64_t already_read = 0;
	nand_info_t *nand = &nand_info[nand_curr_device];


	start_addr = offset + r_bdoffset;

	/* Allocate buffers */
	readbuf = (unsigned char *)malloc(sizeof(readbuf) * nand->writesize);

	if(!readbuf){
		printf(" malloc readbuf fail (%d)",nand->writesize);
		return RE_FAIL;
	}
	/* Initialize start/end addresses and block size */
	if (start_addr & (nand->writesize - 1)) {
		printf("the start address (%llx) is not page-aligned!\n",start_addr);
		goto closeall;
	}

	end_addr = start_addr + size;
	bs = nand->writesize;
	*len=nand->writesize;

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr; ofs += bs) {
		memset(readbuf, 0xff, bs);
		/* Check for bad block */
		if (blockstart != (ofs & (~(nand->erasesize) + 1)) || firstblock) {
			blockstart = ofs & (~(nand->erasesize) + 1);
			firstblock = 0;
			badblock = nand_block_isbad(nand, blockstart);
		}

		if (badblock) {
			/* skip bad block, increase end_addr */
			end_addr += (nand->erasesize);
			ofs += (nand->erasesize) - bs;
			r_bdoffset += nand->erasesize;
			continue;

		} else {
			flush_invalid_cache(readbuf, (unsigned int)len);
			ret = nand_read (nand, ofs,len, readbuf);
			if (ret && ret != -EUCLEAN) {
				printf ("NAND read from offset %llx failed %d\n",offset, ret);
				goto closeall;
			}

		}

		memcpy(buf + already_read, readbuf,
			(size - already_read) > nand->writesize ? nand->writesize:size - already_read);
		already_read += bs;

	}

closeall:
	free(readbuf);
	if(already_read > size)
		already_read = size;
	return already_read;
}



int64_t nand_read_ext4_image(void* buf, uint64_t offset, uint64_t size)
{
	uint64_t ofs, end_addr = 0;
	long long blockstart = 1;
	int i, bs, badblock = 0;

	int firstblock = 1;
	unsigned char *readbuf = NULL;

	int ret;
	uint64_t start_addr;
	uint64_t already_read = 0;
	size_t *len;
	nand_info_t *nand = &nand_info[nand_curr_device];

	start_addr = offset + r_bdoffset + r_ext4respage;

	/* Allocate buffers */
	readbuf = (unsigned char *)malloc(sizeof(readbuf) * nand->writesize);
	if(!readbuf){
		printf(" malloc readbuf fail (%d)",nand->writesize);
		return RE_FAIL;
	}


	/* Initialize start/end addresses and block size */
	if (start_addr & (nand->writesize - 1)) {
		printf("the start address (%llx) is not page-aligned!\n",start_addr);
		goto closeall;
	}


	end_addr = start_addr + size;


	bs = nand->writesize;
	*len=nand->writesize;


	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr; ofs += bs) {

		memset(readbuf, 0xff, bs);
		/* Check for bad block */
		if (blockstart != (ofs & (~(nand->erasesize) + 1)) || firstblock) {
			blockstart = ofs & (~(nand->erasesize) + 1);
			firstblock = 0;
			badblock = nand_block_isbad(nand, blockstart);
		}

		/*skipp the reserved page*/
		if(res_offset == reserved_logical_num){
			r_ext4respage += nand->writesize;
			res_offset = 0;
			end_addr += nand->writesize;
			continue;
		}

		if (badblock) {
			/* skip bad block, increase end_addr */
			end_addr += nand->erasesize;
			r_bdoffset += nand->erasesize;
			ofs += nand->erasesize - bs;
			continue;
		} else {
			/* Read page data and exit on failure */
			flush_invalid_cache(readbuf, (unsigned int)len);
			ret = nand_read (nand, ofs,len, readbuf);
			if (ret && ret != -EUCLEAN) {
				printf ("NAND read from offset %llx failed %d\n",offset, ret);
				goto closeall;
			}
		}

		memcpy(buf + already_read, readbuf,
			(size - already_read) > nand->writesize ? nand->writesize:size - already_read);
		already_read += bs;
		res_offset ++;
	}

closeall:
	free(readbuf);
	if(already_read > size)
		already_read = size;
	return already_read;
}
int get_nand_info(struct nand_dev_info *nand_dev)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	nand_dev->block_size=nand->erasesize;
	nand_dev->page_size=nand->writesize;
	return 0;
}


#ifdef  CONFIG_NAND_DEBUG_VERSION
#include <linux/mtd/atc_nfi.h>

typedef struct _SectorMappingInfo
{
    uint16_t    bBadBlock;
    uint16_t    fDataStatus;
    uint32_t    logicalSectorAddr;
} SectorMappingInfo, *PSectorMappingInfo;
#define GET_LOGICAL_SECTOR_ADDR(x)                       ((x).logicalSectorAddr&0x00FFFFFF)
#define GET_REFRESH_COUNTER(x)                   ((x).fDataStatus+(((x).logicalSectorAddr>>8)&0x00FF0000))
#define SECONDARY_TABLE_SECTOR_MASK             0xFFFF00
#define SECONDARY_TABLE_SECTOR                  0xFFFA00
int nand_check_ftl_after_update_by_sd(nand_info_t *nand, uint32_t start_offset, uint32_t length)
{
	uint32_t log_sector;
	uint32_t sectors_per_block = nand->erasesize/nand->writesize;
	uint32_t sectors_per_block_in_log = sectors_per_block - g_bReservedSectorNumInBlock;
	uint32_t block_size_in_log = nand->erasesize - nand->writesize*g_bReservedSectorNumInBlock;
	struct 	mtd_oob_ops ops;
	SectorMappingInfo sector_info;
	SectorMappingInfo *tmp_secInfo;
	uint32_t start_phy_block;
	uint32_t num_phy_block;
	uint32_t phy_blk_addr;
	uint32_t blk_num;
	uint32_t sector_num;
	uint32_t addr;
	uint32_t last_block;
	uint8_t met_free_sector = 0;
	unsigned int err = 0;
	uint32_t valid_map_cnt = 0;

	if(!ext_buffer){
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		//printf("ext_buffer %x\n", (unsigned int)ext_buffer);
		memset(ext_buffer, 0xFF, nand->writesize);
	}
	start_phy_block = start_offset/nand->erasesize;//get start phy block
	num_phy_block = length/nand->erasesize;
	log_sector = 0;

	for(blk_num = 0; blk_num < num_phy_block; blk_num ++)
	{
		last_block = start_offset + blk_num * nand->erasesize;
		if(nand_block_isbad (nand, last_block)) {
			printf ("Skip bad block 0x%x\n",last_block);
			continue;
		}

		/* read reserve sector */
		memset(&sector_info, 0, sizeof(sector_info));
		ops.datbuf = ext_buffer;
		ops.len = nand->writesize;
		ops.oobbuf = &sector_info;
		ops.ooblen = sizeof(SectorInfo);
		ops.mode = MTD_OOB_PLACE;
		ops.ooboffs = 0;
		if (nand->read_oob(nand, last_block + sectors_per_block_in_log * nand->writesize, &ops)) {
			printf("[FMD]: read reserve sector oob error\n");
			goto READ_ERROR;
		}
		if((GET_LOGICAL_SECTOR_ADDR(sector_info) != RESERVED_SECTOR_IN_BLOCK)) {
			//printf("[FTL]blk %d doesn't have reserverd sector\n", blk_num);
			tmp_secInfo = NULL;
		} else {
			tmp_secInfo = (SectorMappingInfo *)ext_buffer;
			if(met_free_sector) {
				printf("[FTL]error: blk %d should have no reserve sector\n", blk_num);
				err ++;
			}
		}

		for(sector_num = 0; sector_num < sectors_per_block_in_log; sector_num++) {
			memset(&sector_info, 0, sizeof(sector_info));
			addr = 	last_block + sector_num * nand->writesize;
			ops.datbuf = NULL;
			ops.oobbuf = &sector_info;
			ops.ooblen = sizeof(SectorInfo);
			ops.len = 0;
			ops.mode = MTD_OOB_PLACE;
			ops.ooboffs = 0;

			if (nand->read_oob(nand, addr, &ops)) {
				printf("[FMD]read oob error\n");
				goto READ_ERROR;
			}

			if((GET_LOGICAL_SECTOR_ADDR(sector_info) == 0xffffff) && (GET_REFRESH_COUNTER(sector_info) == 0xffffff)) {
				//printf("[FTL]blk:%d,sec:%d<=>free\n", blk_num, sector_num);
				met_free_sector = 0xff;
			} else {
				if(met_free_sector) {
					printf("[FTL]error: blk:%d,sec:%d not free\n", blk_num, sector_num);
					err ++;
				}

				if((GET_LOGICAL_SECTOR_ADDR(sector_info) & SECONDARY_TABLE_SECTOR_MASK) == SECONDARY_TABLE_SECTOR) {
					//printf("[FTL]blk:%d,sec:%d is ST sector\n", blk_num, sector_num);
					printf("[FTL]:error: find ST sector @blk(%d),sec(%d)\n", blk_num, sector_num);
					err ++;
				}
				else {
					valid_map_cnt ++;
					//printf("[FTL]blk:%d,sec:%d<=>log:0x%x,ref:0x%x\n",
					//blk_num, sector_num, GET_LOGICAL_SECTOR_ADDR(sector_info), GET_REFRESH_COUNTER(sector_info));
					if(GET_LOGICAL_SECTOR_ADDR(sector_info) != blk_num * sectors_per_block_in_log + sector_num) {
						printf("[FTL]:mapping error @blk(%d),sec(%d)\n", blk_num, sector_num);
						err ++;
					}
					if(GET_REFRESH_COUNTER(sector_info) != 0) {
						printf("[FTL]:refcnt is not zero @blk(%d),sec(%d), ref=0x%x\n", blk_num, sector_num, GET_REFRESH_COUNTER(sector_info));
						err ++;
					}
				}

			}

			if(tmp_secInfo) {//check reserve sector
				if(0 != memcmp(&sector_info, tmp_secInfo + sector_num, sizeof(SectorMappingInfo))) {
					printf("[FTL]error: (blk:%d,sec:%d)mapping info not match with reserve sector\n", blk_num, sector_num);
					err ++;
				}
			}
		}
	}
	printf("=======result=====\n");

	if(valid_map_cnt == 0) {
		printf("No mapping sector fount, maybe is not a ext4 partition\n");
		return 0;
	}
	if(!err)
		printf("check ext4 image OK\n");
	else {
		printf("check ext4 image FAIL\n");
		goto READ_ERROR;
	}

	return 0;
READ_ERROR:
	return -1;
}

#define MAX_ENTRY 4
struct mappingtable {
uint16_t mapping_cnt;
struct map_info {
uint16_t blk;
uint16_t sector;
uint16_t refcnt;
}info[MAX_ENTRY];
};

struct mappingtable* nand_read_ftl_log(nand_info_t *nand, uint32_t start_phy_offset, uint32_t phy_size)
{
	uint32_t sectors_per_block = nand->erasesize / nand->writesize;
	uint32_t sectors_per_block_in_log = sectors_per_block - g_bReservedSectorNumInBlock;
	uint32_t block_size_in_log = nand->erasesize - nand->writesize * g_bReservedSectorNumInBlock;
	struct 	mtd_oob_ops ops;
	SectorMappingInfo sector_info;
	SectorMappingInfo *tmp_secInfo;
	uint32_t start_phy_block;
	uint32_t num_phy_block;
	uint32_t phy_blk_addr;
	uint32_t blk_num;
	uint32_t sector_num;
	uint32_t addr;
	uint32_t last_block;
	struct mappingtable *map;
	struct mappingtable *tmp_map;

	if(!ext_buffer){
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		memset(ext_buffer, 0xFF, nand->writesize);
	}
	map = (struct mappingtable *)(ext_buffer + nand->writesize);
	start_phy_block = start_phy_offset / nand->erasesize;//get start phy block
	num_phy_block = phy_size / nand->erasesize;
	printf ("nand block size is 0x%x, nand page size is %d,\n", nand->erasesize, nand->writesize);

	memset(map, 0, sizeof(struct mappingtable) * sectors_per_block_in_log * num_phy_block);

	for(blk_num = 0; blk_num < num_phy_block; blk_num ++)
	{
		last_block = start_phy_offset + blk_num * nand->erasesize;
		if(nand_block_isbad (nand, last_block)) {
			printf ("Skip bad block 0x%x\n", last_block);
			continue;
		}

		/* read reserve sector */
		memset(&sector_info, 0, sizeof(sector_info));
		ops.datbuf = ext_buffer;
		ops.len = nand->writesize;
		ops.oobbuf = &sector_info;
		ops.ooblen = sizeof(SectorInfo);
		ops.mode = MTD_OOB_PLACE;
		ops.ooboffs = 0;
		if (nand->read_oob(nand, last_block + sectors_per_block_in_log * nand->writesize, &ops)) {
			printf("[FMD]: read reserve sector oob error\n");
			goto READ_ERROR;
		}
		if((GET_LOGICAL_SECTOR_ADDR(sector_info) != RESERVED_SECTOR_IN_BLOCK)) {
			tmp_secInfo = NULL;
		} else {
			tmp_secInfo = (SectorMappingInfo *)ext_buffer;
		}

		for(sector_num = 0; sector_num < sectors_per_block_in_log; sector_num ++) {
			memset(&sector_info, 0, sizeof(sector_info));
			addr = 	last_block + sector_num * nand->writesize;
			ops.datbuf = NULL;
			ops.oobbuf = &sector_info;
			ops.ooblen = sizeof(SectorInfo);
			ops.len = 0;
			ops.mode = MTD_OOB_PLACE;
			ops.ooboffs = 0;

			if (nand->read_oob(nand, addr, &ops)) {
				printf("[FMD]read oob error\n");
				goto READ_ERROR;
			}
			if((GET_LOGICAL_SECTOR_ADDR(sector_info) == 0xffffff) && (GET_REFRESH_COUNTER(sector_info) == 0xffffff)) {
				//printf("[FTL]blk:%d,sec:%d<=>free\n", blk_num, sector_num);
			}
			else if((GET_LOGICAL_SECTOR_ADDR(sector_info) & SECONDARY_TABLE_SECTOR_MASK) == SECONDARY_TABLE_SECTOR){
				//printf("[FTL]blk:%d,sec:%d is ST sector\n", blk_num, sector_num);
			}else {
				//printf("[FTL]blk:%d,sec:%d<=>log:0x%x,ref:0x%x, tmp_map->mapping_cnt is %d\n",
					//blk_num, sector_num, GET_LOGICAL_SECTOR_ADDR(sector_info), GET_REFRESH_COUNTER(sector_info),
					//tmp_map->mapping_cnt);
				tmp_map = map;
				tmp_map += GET_LOGICAL_SECTOR_ADDR(sector_info);
				if(tmp_map->mapping_cnt < MAX_ENTRY) {
					tmp_map->info[tmp_map->mapping_cnt].blk = blk_num;
					tmp_map->info[tmp_map->mapping_cnt].sector = sector_num;
					tmp_map->info[tmp_map->mapping_cnt].refcnt = GET_REFRESH_COUNTER(sector_info);
				}
				else if(GET_REFRESH_COUNTER(sector_info) > tmp_map->info[MAX_ENTRY-1].refcnt) {
                    tmp_map->info[MAX_ENTRY-1].blk = blk_num;
                    tmp_map->info[MAX_ENTRY-1].sector = sector_num;
                    tmp_map->info[MAX_ENTRY-1].refcnt = GET_REFRESH_COUNTER(sector_info);
				}
				tmp_map->mapping_cnt ++;
			}

			if(tmp_secInfo) {//check reserve sector
				if(0 != memcmp(&sector_info, tmp_secInfo + sector_num, sizeof(SectorMappingInfo))) {
					printf("[FTL]error: (blk:%d,sec:%d)mapping info not match with reserve sector\n", blk_num, sector_num);
				}
			}
		}
	}

	return map;
READ_ERROR:
	return NULL;
}


int nand_check_ftl(nand_info_t *nand, uint32_t start_offset, uint32_t length)
{
	uint32_t sectors_per_block = nand->erasesize/nand->writesize;
	uint32_t sectors_per_block_in_log = sectors_per_block - g_bReservedSectorNumInBlock;
	uint32_t block_size_in_log = nand->erasesize - nand->writesize*g_bReservedSectorNumInBlock;
	struct 	mtd_oob_ops ops;
	SectorMappingInfo sector_info;
	SectorMappingInfo *tmp_secInfo;
	uint32_t start_phy_block;
	uint32_t num_phy_block;
	uint32_t phy_blk_addr;
	uint32_t blk_num;
	uint32_t sector_num;
	uint32_t addr;
	uint32_t last_block;
	struct mappingtable *map;
	struct mappingtable *tmp_map;

	if(!ext_buffer){
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		//printf("ext_buffer %x\n", (unsigned int)ext_buffer);
		memset(ext_buffer, 0xFF, nand->writesize);
	}
	map = (struct mappingtable *)(ext_buffer + nand->writesize);
	start_phy_block = start_offset/nand->erasesize;//get start phy block
	num_phy_block = length/nand->erasesize;

	memset(map, 0, sizeof(struct mappingtable) * sectors_per_block_in_log * num_phy_block);

	for(blk_num = 0; blk_num < num_phy_block; blk_num ++)
	{
		last_block = start_offset + blk_num * nand->erasesize;
		if(nand_block_isbad (nand, last_block)) {
			printf ("Skip bad block 0x%x\n",last_block);
			continue;
		}

		/* read reserve sector */
		memset(&sector_info, 0, sizeof(sector_info));
		ops.datbuf = ext_buffer;
		ops.len = nand->writesize;
		ops.oobbuf = &sector_info;
		ops.ooblen = sizeof(SectorInfo);
		ops.mode = MTD_OOB_PLACE;
		ops.ooboffs = 0;
		if (nand->read_oob(nand, last_block + sectors_per_block_in_log * nand->writesize, &ops)) {
			printf("[FMD]: read reserve sector oob error\n");
			goto READ_ERROR;
		}
		if((GET_LOGICAL_SECTOR_ADDR(sector_info) != RESERVED_SECTOR_IN_BLOCK)) {
			//printf("[FTL]blk %d doesn't have reserverd sector\n", blk_num);
			tmp_secInfo = NULL;
		} else {
			tmp_secInfo = (SectorMappingInfo *)ext_buffer;
		}

		for(sector_num = 0; sector_num < sectors_per_block_in_log; sector_num++) {
			memset(&sector_info, 0, sizeof(sector_info));
			addr = 	last_block + sector_num * nand->writesize;
			ops.datbuf = NULL;
			ops.oobbuf = &sector_info;
			ops.ooblen = sizeof(SectorInfo);
			ops.len = 0;
			ops.mode = MTD_OOB_PLACE;
			ops.ooboffs = 0;

			if (nand->read_oob(nand, addr, &ops)) {
				printf("[FMD]read oob error\n");
				goto READ_ERROR;
			}
			if((GET_LOGICAL_SECTOR_ADDR(sector_info) == 0xffffff) && (GET_REFRESH_COUNTER(sector_info) == 0xffffff)) {
				//printf("[FTL]blk:%d,sec:%d<=>free\n", blk_num, sector_num);
			}
			else if((GET_LOGICAL_SECTOR_ADDR(sector_info) & SECONDARY_TABLE_SECTOR_MASK) == SECONDARY_TABLE_SECTOR)
				printf("[FTL]blk:%d,sec:%d is ST sector\n", blk_num, sector_num);
			else {
				//printf("[FTL]blk:%d,sec:%d<=>log:0x%x,ref:0x%x\n",
				//	blk_num, sector_num, GET_LOGICAL_SECTOR_ADDR(sector_info), GET_REFRESH_COUNTER(sector_info));
				tmp_map = map;
				tmp_map += GET_LOGICAL_SECTOR_ADDR(sector_info);
				if(tmp_map->mapping_cnt < MAX_ENTRY) {
					tmp_map->info[tmp_map->mapping_cnt].blk = blk_num;
					tmp_map->info[tmp_map->mapping_cnt].sector = sector_num;
					tmp_map->info[tmp_map->mapping_cnt].refcnt = GET_REFRESH_COUNTER(sector_info);
				}
				else if(GET_REFRESH_COUNTER(sector_info) > tmp_map->info[MAX_ENTRY-1].refcnt) {
                                        tmp_map->info[MAX_ENTRY-1].blk = blk_num;
                                        tmp_map->info[MAX_ENTRY-1].sector = sector_num;
                                        tmp_map->info[MAX_ENTRY-1].refcnt = GET_REFRESH_COUNTER(sector_info);
				}
				tmp_map->mapping_cnt ++;
			}

			if(tmp_secInfo) {//check reserve sector
				if(0 != memcmp(&sector_info, tmp_secInfo + sector_num, sizeof(SectorMappingInfo))) {
					printf("[FTL]error: (blk:%d,sec:%d)mapping info not match with reserve sector\n", blk_num, sector_num);
				}
			}
		}
	}

//dump mapping record
	printf("================result=================\n");
	int i;
	tmp_map = map;
	for(sector_num = 0; sector_num < sectors_per_block_in_log * num_phy_block; sector_num ++) {
		if(tmp_map->mapping_cnt == 0)//not mapped
			continue;
		if(tmp_map->mapping_cnt == 1)//just one mapping
			continue;
		printf("[FTL] log addr 0x%x has %d mappings, dump some of them:(the last is the newest)\n", sector_num, tmp_map->mapping_cnt);
		for(i=0; i < min(tmp_map->mapping_cnt, MAX_ENTRY); i++)
			printf("map[%d]:blk %d, sec %d, ref 0x%x\n",
				i, tmp_map->info[i].blk, tmp_map->info[i].sector, tmp_map->info[i].refcnt);
		tmp_map++;
	}

	return 0;
READ_ERROR:
	return -1;
}

static int nand_erase_blk_test(nand_info_t *nand,  uint32_t blk_addr)
{
	struct erase_info einfo;

	memset(&einfo, 0, sizeof(struct erase_info));
	einfo.mtd = nand;
	einfo.len = nand->erasesize;
	einfo.addr = blk_addr;
	if(0 != nand->erase(nand, &einfo)) {
		printf("MTD Erase failure\n");
		//if(0 != nand->block_markbad(meminfo, einfo.fail_addr))
		//	printf("markbad fail\n");
		return -1;
	}
	return 0;
}

static int nand_erase_test(nand_info_t *nand, uint32_t start_offset, uint32_t length)
{
	uint32_t blk_num;
	uint32_t last_block;
	uint32_t addr;
	uint32_t num_phy_block = length/nand->erasesize;;
	uint32_t sectors_per_block = nand->erasesize/nand->writesize;

	for(blk_num = 0; blk_num < num_phy_block; blk_num++) {
		last_block = start_offset + blk_num * nand->erasesize;
		if(nand_block_isbad(nand, last_block)) {
			printf("ERASE:Skip bad block 0x%x\n",last_block);
			continue;
		}

		if(0 != nand_erase_blk_test(nand, last_block)) {
			printf("Erase 0x%x fail, skip\n",last_block);
			continue;
		}

	}
	return 0;
}

int nand_w_test(nand_info_t *nand, uint32_t start_offset, uint32_t length, unsigned char test_value)
{
	uint32_t blk_num;
	uint32_t sector_num;
	uint32_t last_block;
	uint32_t addr;
	uint32_t num_phy_block = length/nand->erasesize;;
	uint32_t sectors_per_block = nand->erasesize/nand->writesize;
	struct mtd_oob_ops ops;

	memset(ext_buffer, test_value, nand->writesize);
	memset(ext_oob_buffer, test_value, nand->oobsize);

	ops.len = nand->writesize;
	ops.mode = MTD_OOB_PLACE;
	ops.datbuf = ext_buffer;
	ops.ooblen = sizeof(SectorMappingInfo);
	ops.oobbuf = ext_oob_buffer;
	ops.ooboffs = 0;

	for(blk_num = 0; blk_num < num_phy_block; blk_num++) {
		last_block = start_offset + blk_num * nand->erasesize;
		if(nand_block_isbad (nand, last_block)) {
			printf("WRITE:Skip bad block 0x%x\n",last_block);
			continue;
		}

		for(sector_num = 0; sector_num < sectors_per_block; sector_num++) {
			addr = last_block + sector_num * nand->writesize;
			if(0 != nand->write_oob(nand, addr, &ops)) {
				printf("MTD writeoob failure(blk %d, sec %d), quit test ... \n", blk_num, sector_num);
				goto fail;
			}

		}

	}
	return 0;
fail:
	return -1;

}

int nand_r_test(nand_info_t *nand, int raw, uint32_t start_offset, uint32_t length, uint8_t test_value)
{
	uint32_t blk_num;
	uint32_t sector_num;
	uint32_t last_block;
	uint32_t addr;
	uint32_t num_phy_block = length/nand->erasesize;;
	uint32_t sectors_per_block = nand->erasesize/nand->writesize;
	struct mtd_oob_ops ops;
	uint32_t diff = 0;
	uint32_t i;
	SectorMappingInfo rinfo;
	struct nand_chip *chip = nand->priv;

	memset(&rinfo, 0xff, sizeof(SectorMappingInfo));
	memset(ext_buffer, 0xff, nand->writesize);

	ops.len = nand->writesize;
	ops.datbuf = ext_buffer;
	ops.mode = MTD_OOB_PLACE;
	ops.oobbuf = &rinfo;
	ops.ooblen = sizeof(SectorMappingInfo);
	ops.ooboffs = 0;

	for(blk_num = 0; blk_num < num_phy_block; blk_num++) {
		last_block = start_offset + blk_num * nand->erasesize;
		if(nand_block_isbad (nand, last_block)) {
			printf("READ:Skip bad block 0x%x\n",last_block);
			continue;
		}

		for(sector_num = 0; sector_num < sectors_per_block; sector_num++) {
			addr = last_block + sector_num * nand->writesize;
			if(raw) {
				chip->select_chip(nand, 0);
				if(0 != atc_nand_read_raw_with_autofdm(nand, addr >> chip->page_shift, ops.datbuf)) {
					printf("%s: MTD readraw fail, quit test...\n", nand->name);
					goto fail;
				}
				chip->select_chip(nand, -1);
			} else {
				if(0 != nand->read_oob(nand, addr, &ops)) {
					printf("%s: MTD readoob fail, quit test...\n", nand->name);
					goto fail;
				}
			}
			for(i = 0; i<nand->writesize; i++) {
				if(ops.datbuf[i] != test_value) {
					diff++;
					printf("[blk%d,sec%d,%s]data diff:[%d]=0x%x(should 0x%x)\n",
						blk_num, sector_num, raw?"raw":"ecc", i, ops.datbuf[i], test_value);
				}
			}
			for(i = 0; i<sizeof(SectorMappingInfo); i++) {
				if(raw) {
					if(chip->oob_poi[i] != test_value) {

						diff++;
						printf("[blk%d,sec%d,raw]oob diff:[%d]=0x%x(should 0x%x)\n",
							blk_num, sector_num, i, chip->oob_poi[i], test_value);

					}
				} else {
					if(ops.oobbuf[i] != test_value) {

						diff++;
						printf("[blk%d,sec%d,ecc]oob diff:[%d]=0x%x(should 0x%x)\n",
							blk_num, sector_num, i, ops.oobbuf[i], test_value);
					}
				}


			}

			if(diff) {
				printf("[blk%d,sec%d,%s]total diff = %d\n", blk_num, sector_num, raw?"raw":"ecc",diff);
				goto fail;
			}

		}

	}

	return 0;
fail:
	return -1;

}

int nand_rw_test(nand_info_t *nand, uint32_t start_offset, uint32_t length, ulong cycle)
{
	ulong i;
	struct nand_chip *chip = nand->priv;
	uint32_t offset, size;
	uint32_t diff_cnt = 0;

	if(!ext_buffer){
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		//printf("ext_buffer %x\n", (unsigned int)ext_buffer);
		memset(ext_buffer, 0xFF, nand->writesize);
	}

	if(!ext_oob_buffer){
		ext_oob_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_OOB_OFFSET;
		//printf("ext_oob_buffer %x\r\n", (unsigned int)ext_oob_buffer);
		memset(ext_oob_buffer, 0xFF, nand->oobsize);
	}

	offset = start_offset >> chip->phys_erase_shift;
	offset = offset << chip->phys_erase_shift;
	size = length >> chip->phys_erase_shift;
	if(size == 0)
		size = 1;
	size = size << chip->phys_erase_shift;

	if((start_offset != offset) || (length != size)) {
		printf("After alignment: offset=0x%x, size=0x%x\r\n", offset, size);
	}

	for(i=0; i<cycle; i++) {
		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xff))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0xff))//read with ecc on
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0xff)) {//read with ecc off
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xaa))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0xaa))
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0xaa)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x55))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0x55))
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0x55)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x00))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0x00))
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0x00)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xf0))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0xf0))
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0xf0)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xf))
			goto fail;
		if(nand_r_test(nand, 0, offset, size, 0xf))
			goto fail;
		if(nand_r_test(nand, 1, offset, size, 0xf)) {
			diff_cnt ++;
		}

		printf("#Cycle %d done, diff_cnt = %d\r\n", i, diff_cnt);
	}
	printf("###NAND TEST: [OK](diff %d)\r\n", diff_cnt);
	nand_erase_test(nand, offset, size);//clean
	return 0;
fail:

	printf("###NAND TEST: [FAIL](diff %d)\r\n", diff_cnt);
	nand_erase_test(nand, offset, size);//clean
	return -1;

}

int nand_tune_test(nand_info_t *nand, uint32_t offset, uint32_t size, uint cycle)
{
	ulong i;
	uint32_t diff_cnt = 0;
	int ret=0;

	for(i=0; i<cycle; i++) {
		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xff)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0xff)){//read with ecc on
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0xff)) {//read with ecc off
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xaa)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0xaa)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0xaa)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x55)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x55)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x55)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x00)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x00)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x00)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xf0)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0xf0)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0xf0)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0xf)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0xf)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0xf)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x12)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x12)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x12)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x34)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x34)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x34)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x56)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x56)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x56)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x78)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x78)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x78)) {
			diff_cnt ++;
		}

		nand_erase_test(nand, offset, size);
		if(nand_w_test(nand, offset, size, 0x89)){
			ret=1;
			goto fail;
			}
		if(nand_r_test(nand, 0, offset, size, 0x89)){
			ret=2;
			goto fail;
			}
		if(nand_r_test(nand, 1, offset, size, 0x89)) {
			diff_cnt ++;
		}

		printf("#Cycle %d done, diff_cnt = %d\r\n", i, diff_cnt);
	}
	printf("###NAND TEST: [OK](diff %d)\r\n", diff_cnt);
	nand_erase_test(nand, offset, size);//clean
	return 0;
fail:

	printf("###NAND TEST: [FAIL](diff %d)\r\n", diff_cnt);
	nand_erase_test(nand, offset, size);//clean
	return ret;
}

#define mdelay(n)	udelay((n)*1000);

int nand_tune_timing_test(nand_info_t *nand, uint32_t start_offset, uint32_t length, uint cycle)
{
	ulong i;
	struct nand_chip *chip = nand->priv;
	uint32_t offset, size;
	uint32_t diff_cnt = 0;
	char rlt;
	char wst;
	int  nfi_val;
	int ret;

	if(!ext_buffer){
		ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
		//printf("ext_buffer %x\n", (unsigned int)ext_buffer);
		memset(ext_buffer, 0xFF, nand->writesize);
	}

	if(!ext_oob_buffer){
		ext_oob_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_OOB_OFFSET;
		//printf("ext_oob_buffer %x\r\n", (unsigned int)ext_oob_buffer);
		memset(ext_oob_buffer, 0xFF, nand->oobsize);
	}

	nfi_val=*NFI_ACCCON;
	rlt= nfi_val&0xf;
	wst= (nfi_val>>4)&0xf;

	offset = start_offset >> chip->phys_erase_shift;
	offset = offset << chip->phys_erase_shift;
	size = length >> chip->phys_erase_shift;
	if(size == 0)
		size = 1;
	size = size << chip->phys_erase_shift;

	if((start_offset != offset) || (length != size)) {
		printf("After alignment: offset=0x%x, size=0x%x\r\n", offset, size);
	}

	while(1){
		ret=nand_tune_test(nand,offset,size,cycle);

		if(ret==1){
			rlt++;
		}else if(ret==2){
			wst++;
		}else{
			break;
		}
		nfi_val&=0xffffff00;
		nfi_val|=((rlt)|(wst<<4));
		printf("rlt %d wst %d  nfi %x \n",rlt,wst,nfi_val);
		*NFI_ACCCON=nfi_val;
		mdelay(1000);
	}
	return 0;
}

int nand_scan_read_all(void)
{

	int i,ret;
	nand_info_t *nand;
	struct nand_chip *chip;
	ulong addr, length;
	size_t size;
	char *buff;
	int block_nums;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];
	chip = nand->priv;
	block_nums=(nand->size >> chip->phys_erase_shift) -4 ;
	length=nand->erasesize;
	addr=0;

	printf("blocks 0x%x , length 0x%x \n",block_nums,length);
	buff = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;

	for(i=0;i<block_nums;i++){
		if(nand_read_skip_bad(nand, addr, &length,buff))
			printf("read fail at address %08x \n",addr);
		addr+=nand->erasesize;
	}
	printf("read scan done \n");
}

int nand_stress_test(ulong cycle)
{
	nand_info_t *nand;
	struct nand_chip *chip;
	uint32_t length;

	char *buff;
	uint32_t block_nums;


	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];
	chip = nand->priv;
	block_nums = (uint32_t)(nand->size >> chip->phys_erase_shift) -4;//skip bbt
	length = block_nums << chip->phys_erase_shift;

	printf("test cycle=%d test len = %x  nand size 0x%lx \n",cycle, length,  nand->size);

	nand_rw_test(nand, 0, length, cycle);
	printf("nand stress done \n");
}

extern ulong res_blk_addr;
extern const char *valid_flag;

int nand_save_timing(int timing)
{
	int i,ret;
	nand_info_t *nand;
	struct nand_chip *chip;
	ulong addr, length;
	size_t size;
	char *buff;
	int block_nums;
	struct mtd_oob_ops ops;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];
	chip = nand->priv;


	uint8_t *ext_buffer = ATC_UPG_BASE_ADDR+ATC_NAND_UPG_EXT_OFFSET;
	uint8_t *ext_oob_buffer=ATC_UPG_BASE_ADDR+ATC_NAND_UPG_OOB_OFFSET;

	*buff= timing&0xff;

	memset(ext_buffer, 0xFF, nand_info[nand_curr_device].writesize);
	memset(ext_oob_buffer, 0xFF, nand_info[nand_curr_device].oobsize);

	nand_erase_test(nand,res_blk_addr,nand->erasesize);

	memcpy(ext_buffer,valid_flag,8);
	memcpy(ext_buffer+8,buff,1);
	ops.len = nand_info[nand_curr_device].writesize;
	ops.mode = MTD_OOB_PLACE;
	ops.datbuf = ext_buffer;
	ops.ooblen = 4;
	ops.oobbuf = ext_oob_buffer;
	ops.ooboffs = 0;

	if(0 != nand->write_oob(nand, res_blk_addr, &ops)) {
			printf("MTD writeoob failure \n");
	}
}

void nand_clean_timing(void)
{
	int i,ret;
	nand_info_t *nand;
	struct nand_chip *chip;
	ulong addr, length;
	size_t size;
	char *buff;
	int block_nums;
	struct mtd_oob_ops ops;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];
	chip = nand->priv;

	nand_erase_test(nand,res_blk_addr,nand->erasesize);

}

#endif

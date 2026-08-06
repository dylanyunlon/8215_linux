/*
 *  nandupgrade.c
 *
 *  Copyright (C) 2018 Dandan Liu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define PROGRAM_NAME "nandupgrade"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>

#include <asm/types.h>
#include <mtd/mtd-user.h>
#include <common.h>
#include <libmtd.h>
#include "nandupgrade.h"

#define   NAND_RW_COMPARE  0

static unsigned int g_bReservedSectorNumInBlock = 1;

static const char	*standard_input = "-";
static const char	*img;
static char mtd_device[11] = "/dev/mtd0";

static long long	inputskip = 0;
static long long	inputsize = 0;
static bool		quiet =0;
static bool		writeoob = false;
static bool		onlyoob = false;
static bool		markbad = false;
static bool		noecc = false;
static bool		autoplace = false;
static bool		skipallffs = false;
static bool		noskipbad = false;
static bool		pad = false;
static bool		skip_bad_blocks_to_start = false;
static int		blockalign = 1; /* default to using actual block size */
int fd = -1;
long long image_len = 0;
unsigned long long image_offset = 0;
unsigned long long  image_logical_num;
unsigned char *res_pagebuf = NULL;
struct mtd_dev_info mtd;
libmtd_t mtd_desc;
unsigned int reserved_logical_num;
unsigned long long partiton_start_addr;
unsigned long long	mtdoffset = 0;
uint32_t res_offset;
uint32_t all_nand_flag;

unsigned long long w_bdoffset = 0;
unsigned long long r_bdoffset = 0;
unsigned long long r_ext4respage = 0;

static void erase_buffer(void *buffer, size_t size)
{
	const uint8_t kEraseByte = 0xff;

	if (buffer != NULL && size > 0)
		memset(buffer, kEraseByte, size);
}

static int is_virt_block_bad(struct mtd_dev_info *mtd, int fd,
				long long offset)
{
	int i, ret = 0;

	for (i = 0; i < blockalign; ++ i) {
		ret = mtd_is_bad(mtd, fd, offset / mtd->eb_size + i);
		if (ret)
			break;
	}

	return ret;
}

static void show_progress(struct mtd_dev_info *mtd, off_t start, int eb,
			  int eb_start, int eb_cnt)
{
	bareverbose(!quiet, "\rErasing %d Kibyte @ %"PRIxoff_t" -- %2i %% complete ",
		mtd->eb_size / 1024, start, ((eb - eb_start) * 100) / eb_cnt);
	fflush(stdout);
}


int64_t nand_write_raw_image(void* buf, uint64_t offset, uint64_t size)
{
	
	int pagelen;
	long long imglen = 0;
	long long already_write = 0;
	long long blockstart = -1;
	int ret;
	
	/* points to the current page inside filebuf */
	unsigned char *writebuf = NULL;
	int ebsize_aligned;
	uint8_t write_mode;

	mtdoffset = offset + w_bdoffset;
	quiet = 0;	
	markbad = 1;

	image_len = image_len - size;

	if(image_len < 0){
		fprintf(stderr, "write beyond partition size\n");
		return RE_FAIL;
	}

	/*
	 * Pretend erasesize is specified number of blocks - to match jffs2
	 *	 (virtual) block size
	 * Use this value throughout unless otherwise necessary
	 */
	ebsize_aligned = mtd.eb_size * blockalign;

	if (mtdoffset & (mtd.min_io_size - 1)){
		errmsg_die("The start address is not page-aligned !\n"
			   "The pagesize of this NAND Flash is 0x%x.\n",
			   mtd.min_io_size);
		return RE_FAIL;
	}
	pagelen = mtd.min_io_size;
	
	imglen = size;


	/* Check, if length fits into device */
	if ((imglen / pagelen) * mtd.min_io_size > mtd.size - mtdoffset) {
		fprintf(stderr, "Image %lld bytes, NAND page %d bytes, OOB area %d"
				" bytes, device size %lld bytes\n",
				imglen, pagelen, mtd.oob_size, mtd.size);
		sys_errmsg("Input file does not fit into device");
		return RE_FAIL;
	}

	/*
	 * Allocate a buffer big enough to contain all the data (OOB included)
	 * for one eraseblock. The order of operations here matters; if ebsize
	 * and pagelen are large enough, then "ebsize_aligned * pagelen" could
	 * overflow a 32-bit data type.
	 */
	
	writebuf = (unsigned char *)malloc(pagelen);
	if(!writebuf){
		sys_errmsg(" malloc writebuf fail (%d)", pagelen);
		return RE_FAIL;
	}

	#if  NAND_RW_COMPARE
	unsigned char *readbuf = NULL;
	readbuf = (unsigned char *)malloc(pagelen);
	if(!readbuf){
		sys_errmsg(" malloc writebuf fail (%d)", pagelen);
		return RE_FAIL;
	}
	#endif
	already_write = 0;

	/*
	 * Get data from input and write to the device while there is
	 * still input to read and we are still within the device
	 * bounds. Note that in the case of standard input, the input
	 * length is simply a quasi-boolean flag whose values are page
	 * length or zero.
	 */
	while ((already_write < imglen) && mtdoffset < mtd.size) {
		erase_buffer(writebuf, pagelen);
		#if  NAND_RW_COMPARE
		erase_buffer(readbuf, pagelen);
		#endif
		memcpy(writebuf, buf + already_write,
			(imglen-already_write) > pagelen ? pagelen : imglen-already_write);
		/*
		 * New eraseblock, check for bad block(s)
		 * Stay in the loop to be sure that, if mtdoffset changes because
		 * of a bad block, the next block that will be written to
		 * is also checked. Thus, we avoid errors if the block(s) after the
		 * skipped block(s) is also bad (number of blocks depending on
		 * the blockalign).
		 */
		while (blockstart != (mtdoffset & (~ebsize_aligned + 1))) {
			blockstart = mtdoffset & (~ebsize_aligned + 1);

			if (!quiet)
				fprintf(stdout, "Writing data to block %lld at offset 0x%llx\n",
						 blockstart / ebsize_aligned, blockstart);

			ret = is_virt_block_bad(&mtd, fd, blockstart);

			if (ret < 0) {
				sys_errmsg("%s: MTD get bad block failed", mtd_device);
				goto closeall;
			} else if (ret == 1) {
				if (!quiet)
					fprintf(stderr,
						"Bad block at %llx, %u block(s) "
						"will be skipped\n",
						blockstart, blockalign);
				w_bdoffset += ebsize_aligned;
				image_len  -= ebsize_aligned;
				mtdoffset = blockstart + ebsize_aligned;

				if (mtdoffset > mtd.size) {
					errmsg("too many bad blocks, cannot complete request");
					goto closeall;
				}
			}
		}

		ret = 0;
		
		/* Write out data  */
		ret = mtd_write(mtd_desc, &mtd, fd, mtdoffset / mtd.eb_size,
				mtdoffset % mtd.eb_size, writebuf, mtd.min_io_size,NULL, 0, write_mode);
		
		if (ret) {
			if (errno != EIO) {
				sys_errmsg("%s: MTD write failure", mtd_device);
				goto closeall;
			}

			fprintf(stderr, "Erasing failed write from %#08llx to %#08llx\n",
				blockstart, blockstart + ebsize_aligned - 1);

			if (mtd_erase(mtd_desc, &mtd, fd, blockstart / mtd.eb_size)) {
				int errno_tmp = errno;
				sys_errmsg("%s: MTD Erase failure", mtd_device);
				if (errno_tmp != EIO)
					goto closeall;
			}

			if (markbad) {
				fprintf(stderr, "Marking block at %08llx bad\n",
						mtdoffset & (~mtd.eb_size + 1));
				if (mtd_mark_bad(&mtd, fd, mtdoffset / mtd.eb_size)) {
					sys_errmsg("%s: MTD Mark bad block failure", mtd_device);
					goto closeall;
				}
			}
			mtdoffset = blockstart + ebsize_aligned;

			continue;
		}

#if  NAND_RW_COMPARE
		if (mtd_read(&mtd, fd, mtdoffset / mtd.eb_size, mtdoffset % mtd.eb_size, readbuf,mtd.min_io_size)) {
				errmsg("mtd_read");
		}

		if(memcmp(writebuf,readbuf, mtd.min_io_size)){
				errmsg("compare the readbuf and writebuf fail ===== dandadn ");
		}
#endif
		mtdoffset += mtd.min_io_size;
		already_write += mtd.min_io_size;
	}

closeall:
	
	free(writebuf);
	#if   NAND_RW_COMPARE
	free(readbuf);
	#endif
	if(already_write > size)
		already_write = size;
	return already_write;

}

int64_t nand_write_ext4_image(void* buf, uint64_t offset, uint64_t size)
{
	int pagelen;
	uint64_t imglen = 0;
	uint64_t already_write = 0;
	long long blockstart = -1;	
	int ret;
	bool failed = true;

	/* points to the current page inside filebuf */
	unsigned char *writebuf = NULL;
	/* points to the OOB for the current page in filebuf */
	unsigned char *oobbuf = NULL;

	int ebsize_aligned;
	uint8_t write_mode;
	int oob_size;

	int sectors_per_block  ;
	int sectors_per_block_in_log  ;
	int block_size_in_log ;
	PSectorInfo sector_info;

	uint64_t off;
	uint32_t page_num;

	int  sectormgr_size = sizeof(SectorInfo);
	
	quiet = 0;
	markbad = 1;
	
	off = offset;

	if(!(image_len > 0)){
		errmsg_die("the image length is %d ", image_len);
		return RE_FAIL;
	}

	if(all_nand_flag){
		if(partiton_start_addr == 0){
			partiton_start_addr = offset;
			mtdoffset = offset;
			printf("the start add is 0x%llx \n", mtdoffset);
		}
	}
	else{
		partiton_start_addr = 0;

	}
	ebsize_aligned = mtd.eb_size * blockalign;
	oob_size = mtd.oob_size;
	pagelen = mtd.min_io_size ;
	sectors_per_block = mtd.eb_size / mtd.min_io_size;
	sectors_per_block_in_log = sectors_per_block - g_bReservedSectorNumInBlock;
	block_size_in_log = sectors_per_block_in_log*pagelen;
	imglen = size;

	if (mtdoffset & (mtd.min_io_size - 1))
		errmsg_die("The start address is not page-aligned !\n"
			   "The pagesize of this NAND Flash is 0x%x.\n",
			   mtd.min_io_size);


	write_mode = MTD_OPS_PLACE_OOB;

	/* Check, if length fits into device */
	if ((imglen / pagelen) * mtd.min_io_size > mtd.size - mtdoffset) {
		fprintf(stderr, "Image %lld bytes, NAND page %d bytes, OOB area %d"
				" bytes, device size %lld bytes\n",
				imglen, pagelen, mtd.oob_size, mtd.size);
		sys_errmsg("Input file does not fit into device");
		
		return RE_FAIL;
	}

	/*
	 * Allocate a buffer big enough to contain all the data (OOB included)
	 * for one eraseblock. The order of operations here matters; if ebsize
	 * and pagelen are large enough, then "ebsize_aligned * pagelen" could
	 * overflow a 32-bit data type.
	 */
	writebuf = (unsigned char *)malloc(pagelen);
	if(!writebuf){
		sys_errmsg(" malloc writebuf fail (%d)",pagelen);
		return RE_FAIL;
	}
	oobbuf = (unsigned char *)malloc(oob_size);
	if(!oobbuf){
		sys_errmsg(" malloc oobbuf fail (%d)",oob_size);
		free(writebuf);
		return RE_FAIL;
	}

	#if  NAND_RW_COMPARE
	unsigned char *oobbuf2 = NULL;
	oobbuf2 = (unsigned char *)malloc(pagelen);
	if(!oobbuf2){
		sys_errmsg(" malloc writebuf fail (%d)",pagelen);
		return RE_FAIL;
	}
	#endif
	
	already_write = 0;


	/*
	 * Get data from input and write to the device while there is
	 * still input to read and we are still within the device
	 * bounds. Note that in the case of standard input, the input
	 * length is simply a quasi-boolean flag whose values are page
	 * length or zero.
	 */
	while((already_write < imglen)&& mtdoffset < mtd.size) {
		erase_buffer(writebuf, pagelen);
		memcpy(writebuf,buf+already_write,
			(imglen-already_write)>pagelen ? pagelen:imglen-already_write);
		ret = 0;
		/*
		 * New eraseblock, check for bad block(s)
		 * Stay in the loop to be sure that, if mtdoffset changes because
		 * of a bad block, the next block that will be written to
		 * is also checked. Thus, we avoid errors if the block(s) after the
		 * skipped block(s) is also bad (number of blocks depending on
		 * the blockalign).
		 */
		while (blockstart != (mtdoffset & (~ebsize_aligned + 1))) {
			blockstart = mtdoffset & (~ebsize_aligned + 1);
			if (!quiet)
				fprintf(stdout, "Writing data to block %lld at offset 0x%llx\n",
						 blockstart / ebsize_aligned, blockstart);


			ret = is_virt_block_bad(&mtd, fd, blockstart);

			if (ret < 0) {
				sys_errmsg("%s: MTD get bad block failed", mtd_device);
				goto closeall;
			} else if (ret == 1) {
				if (!quiet)
					fprintf(stderr,
						"Bad block at %llx, %u block(s) "
						"will be skipped\n",
						blockstart, blockalign);

				mtdoffset = blockstart + ebsize_aligned;

				if (mtdoffset > mtd.size) {
					errmsg("too many bad blocks, cannot complete request");
					goto closeall;
				}
			}
		}

		sector_info = (PSectorInfo)oobbuf;
		
		sector_info->bBadBlock = 0xFFFF;
		sector_info->wReserved2 = 0;
		
		page_num = (off - partiton_start_addr) / pagelen;
		sector_info->dwReserved1 = page_num;

		//printf("==dd page num %d res_offset %d <==> mtd_off 0x%llx off 0x%llx\n",page_num,
		//	res_offset,mtdoffset,off);
	//	memcpy(res_pagebuf+sectormgr_size*res_offset,oobbuf,sectormgr_size);
		
		if(res_offset == reserved_logical_num){
			sector_info = (PSectorInfo)oobbuf;
			sector_info->bBadBlock = 0xFFFF;
			sector_info->wReserved2 = 0;
			sector_info->dwReserved1 = RESERVED_SECTOR_IN_BLOCK;
			//memcpy(res_pagebuf+sectormgr_size*res_offset,oobbuf,sectormgr_size);
			/*write  reserved sector*/
			ret = mtd_write(mtd_desc, &mtd, fd, mtdoffset / mtd.eb_size,mtdoffset % mtd.eb_size,
							 res_pagebuf, mtd.min_io_size, oobbuf, sectormgr_size, write_mode);
			image_logical_num = 0;
			erase_buffer(res_pagebuf, mtd.min_io_size);
			//off +=mtd.min_io_size;
			mtdoffset += mtd.min_io_size;
			res_offset = 0;
			continue;
		}
		else{
			memcpy(res_pagebuf + sectormgr_size*res_offset, oobbuf, sectormgr_size);
			/* Write  data */
			ret = mtd_write(mtd_desc, &mtd, fd, mtdoffset / mtd.eb_size, mtdoffset % mtd.eb_size,
					 writebuf, mtd.min_io_size, oobbuf, sectormgr_size, write_mode);
		}


		if (ret) {
			if (errno != EIO) {
				sys_errmsg("%s: MTD write failure", mtd_device);
				goto closeall;
			}

			fprintf(stderr, "Erasing failed write from %#08llx to %#08llx\n",
				blockstart, blockstart + ebsize_aligned - 1);

			if (mtd_erase(mtd_desc, &mtd, fd, blockstart / mtd.eb_size)) {
				int errno_tmp = errno;
				sys_errmsg("%s: MTD Erase failure", mtd_device);
				if (errno_tmp != EIO)
					goto closeall;
			}

			if (markbad) {
				fprintf(stderr, "Marking block at %08llx bad\n",
						mtdoffset & (~mtd.eb_size + 1));
				if (mtd_mark_bad(&mtd, fd, mtdoffset / mtd.eb_size)) {
					sys_errmsg("%s: MTD Mark bad block failure", mtd_device);
					goto closeall;
				}
			}
			mtdoffset = blockstart + ebsize_aligned;

			continue;
		}
		mtdoffset += mtd.min_io_size;
		off += mtd.min_io_size;
		already_write += mtd.min_io_size;
		image_logical_num ++;
		image_offset ++;
		res_offset ++;
	}

	failed = false;

closeall:
	
	free(writebuf);
	free(oobbuf);

	if(already_write > size)
		already_write = size;
	return already_write;
}

int64_t nand_read_raw_image(void* buf, uint64_t offset, uint64_t size)
{
	uint64_t ofs, end_addr = 0;
	long long blockstart = 1;
	int i, bs, badblock = 0;
	
	int firstblock = 1;

	unsigned char *readbuf = NULL;

	int err;
	uint64_t start_addr;
	uint64_t already_read = 0;

	if(!(image_len > 0)){
		errmsg_die("the image length is %d ", image_len);
		return RE_FAIL;
	}

	start_addr = offset + r_bdoffset;
	
	/* Allocate buffers */
	readbuf = (unsigned char *)xmalloc(sizeof(readbuf) * mtd.min_io_size);

	if(!readbuf){
		sys_errmsg(" malloc readbuf fail (%d)", mtd.min_io_size);
		return RE_FAIL;
	}
	/* Initialize start/end addresses and block size */
	if (start_addr & (mtd.min_io_size - 1)) {
		fprintf(stderr, "the start address (-s parameter) is not page-aligned!\n"
				"The pagesize of this NAND Flash is 0x%x.\n",
				mtd.min_io_size);
		goto closeall;
	}

	 
	end_addr = start_addr + size;
	if (end_addr > mtd.size)
		end_addr = mtd.size;


	bs = mtd.min_io_size;

	/* Print informative message */
	 
	fprintf(stderr, "Block size %d, page size %d, OOB size %d\n",
			mtd.eb_size, mtd.min_io_size, mtd.oob_size);
	fprintf(stderr,
			"read data starting at 0x%08llx and ending at 0x%08llx...\n",
			start_addr, end_addr);
	 

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr; ofs += bs) {
		memset(readbuf, 0xff, bs);
		/* Check for bad block */
		if (blockstart != (ofs & (~mtd.eb_size + 1)) || firstblock) {
			blockstart = ofs & (~mtd.eb_size + 1);
			firstblock = 0;
			if ((badblock = mtd_is_bad(&mtd, fd, ofs / mtd.eb_size)) < 0) {
				errmsg("libmtd: mtd_is_bad");
				goto closeall;
			}
		}

		if (badblock) {
			/* skip bad block, increase end_addr */	
			end_addr += mtd.eb_size;
			ofs += mtd.eb_size - bs;
			r_bdoffset += mtd.eb_size;
			if (end_addr > mtd.size)
				end_addr = mtd.size;
			continue;	
			
		} else {
			/* Read page data and exit on failure */
			if (mtd_read(&mtd, fd, ofs / mtd.eb_size, ofs % mtd.eb_size, readbuf, bs)) {
				errmsg("mtd_read");
				goto closeall;
			}
		}
		
		memcpy(buf + already_read, readbuf,
			(size - already_read) > mtd.min_io_size ? mtd.min_io_size:size - already_read);
		already_read += bs;
		
	}
	fprintf(stderr,
			"read data starting at 0x%08llx and ending at 0x%08llx...\n",
			start_addr, end_addr);
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
	bool eccstats = false;
	unsigned char *readbuf = NULL;

	int err;
	uint64_t start_addr;
	uint64_t already_read = 0;
	uint32_t page_num;
	start_addr = offset + r_bdoffset + r_ext4respage;
	
	/* Allocate buffers */
	readbuf = (unsigned char *)xmalloc(sizeof(readbuf) * mtd.min_io_size);


	/* Initialize start/end addresses and block size */
	if (start_addr & (mtd.min_io_size - 1)) {
		fprintf(stderr, "the start address (-s parameter) is not page-aligned!\n"
				"The pagesize of this NAND Flash is 0x%x.\n",
				mtd.min_io_size);
		goto closeall;
	}

	end_addr = start_addr + size;
	if (end_addr > mtd.size)
		end_addr = mtd.size;

	bs = mtd.min_io_size;

	/* Print informative message */
	fprintf(stderr, "Block size %d, page size %d, OOB size %d\n",
			mtd.eb_size, mtd.min_io_size, mtd.oob_size);
	fprintf(stderr,
			"read data starting at 0x%08llx and ending at 0x%08llx...\n",
			start_addr, end_addr);

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr; ofs += bs) {
		memset(readbuf, 0xff, bs);
		/* Check for bad block */
		if (blockstart != (ofs & (~mtd.eb_size + 1)) || firstblock) {
			blockstart = ofs & (~mtd.eb_size + 1);
			firstblock = 0;
			if ((badblock = mtd_is_bad(&mtd, fd, ofs / mtd.eb_size)) < 0) {
				errmsg("libmtd: mtd_is_bad");
				goto closeall;
			}
		}
		page_num = (ofs - partiton_start_addr) / mtd.min_io_size;

		/*skipp the reserved page*/
		if(res_offset == reserved_logical_num){
			r_ext4respage += mtd.min_io_size;
			image_logical_num = 0;
			res_offset = 0;
			end_addr += mtd.min_io_size;
			if (end_addr > mtd.size)
				end_addr = mtd.size;
			continue;
		}
		if (badblock) {
			/* skip bad block, increase end_addr */
			end_addr += mtd.eb_size;
			r_bdoffset += mtd.eb_size;
			ofs += mtd.eb_size - bs;
			if (end_addr > mtd.size)
				end_addr = mtd.size;
			fprintf(stderr, "badblock\n");
			continue;
		} else {
			/* Read page data and exit on failure */
			if (mtd_read(&mtd, fd, ofs / mtd.eb_size, ofs % mtd.eb_size, readbuf, bs)) {
				errmsg("mtd_read");
				goto closeall;
			}
		}
		memcpy(buf + already_read, readbuf,
			(size - already_read) > mtd.min_io_size ? mtd.min_io_size:size - already_read);
		already_read += bs;
		image_logical_num ++;
		res_offset ++;
	}

closeall:
	free(readbuf);
	if(already_read > size)
		already_read = size;
	return already_read;
}

int64_t nand_ext4_write_endoffset(void)
{
	return mtdoffset;
}

int nand_erase(uint64_t start, uint32_t cnt)
{
	unsigned int eb, eb_start, eb_cnt;
	bool isNAND;
	int error = 0;
	off_t offset = 0;

	eb_cnt = cnt;
	eb_start = start / mtd.eb_size;

	/*
	 * Now do the actual erasing of the MTD device
	 */
	if (eb_cnt == 0)
		eb_cnt = (mtd.size / mtd.eb_size) - eb_start;

	for (eb = eb_start; eb < eb_start + eb_cnt; eb++) {
		offset = (off_t)eb * mtd.eb_size;

		int ret = mtd_is_bad(&mtd, fd, eb);
		if (ret > 0) {
			sys_errmsg( "Skipping bad block at %08",offset);
			continue;
		} else if (ret < 0) {
			return sys_errmsg("%s: MTD get bad block failed", mtd_device);
		}

		if (mtd_erase(mtd_desc, &mtd, fd, eb) != 0) {
			sys_errmsg("%s: MTD Erase failure", mtd_device);
			continue;
		}

	}
	show_progress(&mtd, offset, eb, eb_start, eb_cnt);

	return 0;
}


int nand_all_erase(void)
{
	return nand_erase(0,0);
}

int get_nand_info(struct nand_dev_info *nand_info)
{
	libmtd_t mtd_desc_info;
	struct mtd_dev_info mtd1;
	int sectors_per_block;
	int fd1;
	
	mtd_desc_info = libmtd_open();
	if (mtd_desc_info == NULL)
		return errmsg("can't initialize libmtd");

	if ((fd1 = open(mtd_device, O_RDWR)) < 0)
		return sys_errmsg("%s", mtd_device);

	if (mtd_get_dev_info(mtd_desc_info, mtd_device, &mtd1) < 0)
		return errmsg("mtd_get_dev_info failed");

	nand_info->block_cnt = mtd1.eb_cnt;
	nand_info->block_size = mtd1.eb_size;
	nand_info->page_size = mtd1.min_io_size;
	nand_info->oob_size = mtd1.oob_size;
	printf("block cnt %x , block size %x  page size %x   oob size %x\n",nand_info->block_cnt,
			nand_info->block_size, nand_info->page_size, nand_info->oob_size);
	
	close(fd1);
	libmtd_close(mtd_desc_info);

	return 0;
}

int nand_rw_start(int part, uint64_t size)
{
	
	int sectors_per_block;
	mtd_desc = libmtd_open();
	if (mtd_desc == NULL){
		errmsg("can't initialize libmtd");
		return RE_FAIL;
	}
	if(part == ALLNAND){
		strcpy(mtd_device, "/dev/mtd0");
		all_nand_flag = 1;
	}else if(part > ALLNAND){
		errmsg("partition error");
		return RE_FAIL;
	}else {
		sprintf(mtd_device, "%s%d", "/dev/mtd", part);
		all_nand_flag = 0;
	}
	if ((fd = open(mtd_device, O_RDWR)) < 0){
		sys_errmsg("%s", mtd_device);
		return RE_FAIL;
		}
	if (mtd_get_dev_info(mtd_desc, mtd_device, &mtd) < 0){
		errmsg("mtd_get_dev_info failed");
		return RE_FAIL;
	}

	image_len = size;
	w_bdoffset = 0;
	r_bdoffset = 0;
	r_ext4respage = 0;
	image_offset = 0;
	image_logical_num = 0;
	reserved_logical_num = mtd.eb_size / mtd.min_io_size - g_bReservedSectorNumInBlock;
	sectors_per_block = mtd.eb_size / mtd.min_io_size;
	partiton_start_addr = 0;
	
	res_pagebuf = (unsigned char *)malloc(mtd.min_io_size);
	erase_buffer(res_pagebuf,mtd.min_io_size);
	if(!res_pagebuf){
		errmsg("malloc res_pagebuf (%d) failed", sizeof(SectorInfo) * sectors_per_block);
		return RE_FAIL;
	}
	res_offset = 0;
	mtdoffset = 0;
	//printf("=====dandan ==start to nand rw start %llx  \n", size);
	return 0;
}

int nand_rw_end(void)
{
	image_len = 0;
	w_bdoffset = 0;
	r_bdoffset = 0;
	r_ext4respage = 0;
	mtdoffset = 0;
	close(fd);
	libmtd_close(mtd_desc);
	if(res_pagebuf)
		free(res_pagebuf);
	return 0;
}


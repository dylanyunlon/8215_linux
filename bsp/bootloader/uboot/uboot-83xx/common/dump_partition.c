#ifndef CONFIG_BOOT_MMC
#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <linux/mtd/mtd.h>
#include <mmc.h>

extern nand_info_t nand_info[];
extern int nand_curr_device;
extern void flush_cache(unsigned int start, unsigned int size);

#define DUMP_SDCARD_ADDR 0x800000
/*
 * dump_partition:
 *  - startaddr: NAND offset in bytes
 *  - size: number of bytes to dump (will be rounded up to NAND page size)
 *  - mode: 0 => only data; non-zero => include OOB per page
 *
 * Writes to SD at fixed address 0x800000 (byte offset). Each page is
 * written as: [page data][optional oob], and the SD write size is
 * rounded up to 512-byte blocks.
 */
int dump_partition(struct mmc *mmc, int dev_num, unsigned int startaddr, unsigned int size, unsigned int bEnableDumpOob)
{
	nand_info_t *nand;
	unsigned int page_size;
	unsigned int aligned_size;
	unsigned int total_pages;
	unsigned int pages_per_block;
	unsigned int pages_left;
	unsigned int p;
	loff_t addr;
	void *page_buf = NULL;
	void *block_oob_buf = NULL; /* stores concatenated 8-byte oobs for a block */
	void *oob_tmp = NULL;       /* temporary per-page oob buffer (8 bytes) */
	unsigned int write_blocks;
	unsigned long sd_sector = (unsigned long)(DUMP_SDCARD_ADDR / 512); /* fixed target sector */
	const unsigned int OOB_READ_LEN = sizeof(SectorInfo);
	//unsigned int high, low;

	printf("Ready to dump data to SD at 0x%x\n", DUMP_SDCARD_ADDR);
	printf("[%s]: startaddr=0x%x, size=0x%x, bEnableDumpOob=%u\n", __func__, startaddr, size, bEnableDumpOob);

	if (nand_curr_device < 0)
		return -1;

	nand = &nand_info[nand_curr_device];
	page_size = nand->writesize;

	printf("[%s]: nand page_size=%u, erasesize=%u\n", __func__, page_size, nand->erasesize);

	if (page_size == 0) {
		printf("dump_partition: invalid nand page size\n");
		return -1;
	}

	/* align size to page */
	aligned_size = (size + page_size - 1) & ~(page_size - 1);
	total_pages = aligned_size / page_size;
	if (total_pages == 0) {
		printf("dump_partition: nothing to dump\n");
		return -1;
	}

	pages_per_block = nand->erasesize / page_size;
	if (pages_per_block == 0) {
		printf("dump_partition: invalid pages_per_block\n");
		return -1;
	}

	//printf("dump_partition: aligned_size=0x%x, total_pages=%u, pages_per_block=%u\n", 
	//aligned_size, total_pages, pages_per_block);

	page_buf = malloc(page_size);
	if (!page_buf) {
		printf("dump_partition: alloc page_buf failed\n");
		return -1;
	}
	//printf("dump_partition: allocated page_buf at %p, size=%u\n", page_buf, page_size);

	/* If mode!=0, need to read OOB data */
	if (bEnableDumpOob != 0) {
		block_oob_buf = malloc(pages_per_block * OOB_READ_LEN);
		if (!block_oob_buf) {
			printf("dump_partition: alloc block_oob_buf failed\n");
			free(page_buf);
			return -1;
		}
		oob_tmp = malloc(OOB_READ_LEN);
		if (!oob_tmp) {
			printf("dump_partition: alloc oob_tmp failed\n");
			free(page_buf);
			free(block_oob_buf);
			return -1;
		}
		//printf("dump_partition: allocated block_oob_buf at %p (size=%u) and oob_tmp at %p (size=%u)\n", 
		//block_oob_buf, pages_per_block * OOB_READ_LEN, oob_tmp, OOB_READ_LEN);
	}

	addr = (loff_t)startaddr;
	pages_left = total_pages;

	while (pages_left) {
		unsigned int pages_in_block = (pages_left > pages_per_block) ? pages_per_block : pages_left;

		/* clear block oob buffer to 0xFF to avoid residual data */
		if (block_oob_buf)
			memset(block_oob_buf, 0, pages_per_block * OOB_READ_LEN);

		/* read and write each page in the block */
		for (p = 0; p < pages_in_block; p++) {
			struct mtd_oob_ops ops;
			int ret;

			memset(&ops, 0, sizeof(ops));

			ops.datbuf = page_buf;
			ops.len = page_size;

			if (oob_tmp) {
				ops.oobbuf = oob_tmp;
				ops.ooblen = OOB_READ_LEN;
				ops.mode = MTD_OOB_PLACE;
				ops.ooboffs = 0;
				//printf("dump_partition: OOB enabled, reading %u bytes OOB data\n", OOB_READ_LEN);
			} else {
				ops.oobbuf = NULL;
				ops.ooblen = 0;
				ops.mode = MTD_OOB_PLACE; /* no oob requested */
				//printf("dump_partition: OOB disabled\n");
			}

			/* Using nand->read_oob to read data 与 oob；If no need oob，ops.oobbuf=NULL，only return data */
			ret = nand->read_oob(nand, addr, &ops);
			if (ret) {
				printf("dump_partition: nand read oob failed at 0x%08llx ret=%d\n", (unsigned long long)addr, ret);
				goto cleanup_err;
			}

			if (oob_tmp) {
				memcpy((char *)block_oob_buf + p * OOB_READ_LEN, oob_tmp, OOB_READ_LEN);
			}

			/* write page data (round up to 512-byte blocks) */
			flush_cache((unsigned int)page_buf, page_size);
			write_blocks = (page_size + 511) / 512;
			//printf("dump_partition: writing page data to SD, sd_sector=%lu, write_blocks=%u, page_size=%u\n", 
			//sd_sector, write_blocks, page_size);
			if (mmc->block_dev.block_write(dev_num, sd_sector, write_blocks, (char *)page_buf) != write_blocks) {
				printf("dump_partition: mmc write failed at sector 0x%lx\n", sd_sector);
				goto cleanup_err;
			}

			sd_sector += write_blocks;
			addr += page_size;
			//u64_to_u32(addr, &high, &low);
			//printf("dump_partition: updated addr:(0x%x%08x ~ 0x%x%08x), sd_sector=%lu\n", (unsigned long long)addr, sd_sector);
		}

		/* write aggregated OOB for this block, if any */
		if (block_oob_buf) {
			unsigned int oob_bytes = pages_in_block * OOB_READ_LEN;
			unsigned int oob_padded = (oob_bytes + 511) & ~511;
			void *oob_write_buf = malloc(oob_padded);
			if (!oob_write_buf) {
				printf("dump_partition: alloc oob_write_buf failed\n");
				goto cleanup_err;
			}
			//printf("dump_partition: preparing OOB data, oob_bytes=%u, oob_padded=%u, sd_sector=%lu\n", 
			//oob_bytes, oob_padded, sd_sector);
			memset(oob_write_buf, 0xFF, oob_padded);
			memcpy(oob_write_buf, block_oob_buf, oob_bytes);

			flush_cache((unsigned int)oob_write_buf, oob_padded);
			write_blocks = oob_padded / 512;
			//printf("dump_partition: writing OOB data to SD, sd_sector=%lu, write_blocks=%u\n", 
			//sd_sector, write_blocks);
			if (mmc->block_dev.block_write(dev_num, sd_sector, write_blocks, (char *)oob_write_buf) != write_blocks) {
				printf("dump_partition: mmc write failed for block oob at sector 0x%lx\n", sd_sector);
				free(oob_write_buf);
				goto cleanup_err;
			}
			sd_sector += write_blocks;
			free(oob_write_buf);
			printf("[%s]: OOB written successfully, updated sd_sector=%lu\n", __func__, sd_sector);
		}

		pages_left -= pages_in_block;
		printf("[%s]: finished one block, pages_left=%u\n", __func__, pages_left);
	}

	printf("[%s]: completed success, total sectors used: %lu\n", __func__, sd_sector - (DUMP_SDCARD_ADDR / 512));

	/* success cleanup */
	free(page_buf);
	if (block_oob_buf) free(block_oob_buf);
	if (oob_tmp) free(oob_tmp);
	return 0;

cleanup_err:
	printf("[%s]: error occurred, cleaning up\n", __func__);
	free(page_buf);
	if (block_oob_buf) free(block_oob_buf);
	if (oob_tmp) free(oob_tmp);
	return -1;
}
#endif

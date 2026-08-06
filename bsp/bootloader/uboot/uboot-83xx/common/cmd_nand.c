/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>


/*
 *
 * New NAND support
 *
 */
#include <common.h>
#include <linux/mtd/mtd.h>

#if defined(CONFIG_CMD_NAND)

#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/x_typedef.h>
#include <linux/mtd/atc_nand.h>
#include <mmc.h>
#include <linux/time.h>


//#include <mtd.h>

struct nand_atc_bbt g_bbt;

extern BOOL _fgAUTO_FMT;
#define false 0
#define true 1
#if defined(CONFIG_CMD_MTDPARTS)

/* parition handling routines */
int mtdparts_init(void);
int id_parse(const char *id, const char **ret_id, u8 *dev_type, u8 *dev_num);
int find_dev_and_part(const char *id, struct mtd_device **dev,
		      u8 *part_num, struct part_info **part);
#endif


#ifdef  CONFIG_NAND_DEBUG_VERSION

//Partition address
ulong partition_size = 0;

#define MAX_ENTRY 4
struct mappingtable {
uint16_t mapping_cnt;
struct map_info {
uint16_t blk;
uint16_t sector;
uint16_t refcnt;
}info[MAX_ENTRY];
};

static unsigned int boot_time_ms(void)
{
	volatile unsigned int time = 0;

	/***
	* Register F000814C, which was triggered by BootROM,
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - (*((volatile uint32_t*)(0xF000814C)))) / 27000;
	return time;
}


int nand_dump_ext4data(nand_info_t* nand, struct mmc* dst_mmc, ulong dst_offset, ulong start_phy_addr,
	ulong start_log_addr, ulong size)
{
    u8* buf = (u8*)malloc(nand->writesize);
	ulong start_addr_in_page = (start_phy_addr >> 4) << 4;
	ulong size_in_page = size * nand->writesize;

	u32 dst_offset_in_block = 0;
	u32 i = 0;
	struct mappingtable *map = NULL;
	struct mappingtable *tmp_map = NULL;
	ulong read_phy_addr;
	int m = 0, refcnt = 0, tmp = 0;

	if (buf == NULL)
	{
		printf("nand =====> allocate buffer for RW failed <=====\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	if ((nand == NULL) || (dst_mmc == NULL))
	{
		free(buf);
		printf("nand =====> bad mmc pointer for dump <=====\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	// Convert address and size to block units
	dst_offset_in_block = ALIGN(dst_offset, 512) / 512;
	printf("nand =====> destination(mmc%d): dest address : 0x%08X %08X (%d) <=====\n", dst_mmc->host_id, (u32)(dst_offset >> 32),(u32)(dst_offset & 0xFFFFFFFF), dst_offset_in_block);

	if(partition_size > 0){
		map = nand_read_ftl_log(nand, (uint32_t)start_phy_addr, (uint32_t)partition_size);
	}

	if(map == NULL){
		printf("map is NULL\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	// Read data from  source address
	tmp_map = map + start_log_addr;
	for(i = 0; i < size; i ++){
		memset(buf, 0, sizeof(buf));
		if(tmp_map->mapping_cnt == 0){//not mapped
			printf("Not mapped");
		}else if(tmp_map->mapping_cnt >= 1){//have mapping
			tmp = 0;
		    refcnt = tmp_map->info[0].refcnt;
			for(m = 1; m < min(tmp_map->mapping_cnt, MAX_ENTRY); m ++){
				if(refcnt < tmp_map->info[m].refcnt){
					refcnt = tmp_map->info[m].refcnt;
					tmp = m;
				}
			}
			read_phy_addr = start_phy_addr + tmp_map->info[tmp].blk * nand->erasesize
				+ tmp_map->info[tmp].sector * (nand->writesize);
			//printf("tmp:%d, start_phy_addr:0x%x, blk:%d, sec: %d, read_phy_addr: 0x%x\n",
				//tmp, start_phy_addr,tmp_map->info[tmp].blk, tmp_map->info[tmp].sector, read_phy_addr);
			nand_read_skip_bad(nand, read_phy_addr, &nand->writesize, buf);


			// Write data to destination device with special address (dst_offset)
			mmc_bwrite(dst_mmc->host_id, dst_offset_in_block, nand->writesize / 512, buf);
			dst_offset_in_block += nand->writesize / 512;
		}
		tmp_map ++;
	}

	if (buf)
	{
		free(buf);
		buf = NULL;
	}

	printf("============> dump ext4data completed <============\n");

	return 0;

}

int nand_dump_data(nand_info_t* nand, struct mmc* dst_mmc, ulong dst_offset, ulong start_addr, ulong size)
{

    u8* buf = (u8*)malloc(nand->erasesize);
	ulong start_addr_in_page = (start_addr >> 4) << 4;
	ulong size_in_page = ALIGN(size, nand->writesize);
	ulong read_size = 0;
	printf("start_addr_in_page is 0x%08x\, size_in_page is 0x%08x\n", start_addr_in_page, size_in_page);

	u32 ret = 0;
	u32 dst_offset_in_block = 0;
	u32 size_in_block = 0;
	u32 trans_size_in_block = 0;
	u32 i = 0;

	if (buf == NULL)
	{
		printf("nand =====> allocate buffer for RW failed <=====\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	if ((nand == NULL) || (dst_mmc == NULL))
	{
		free(buf);
		printf("nand =====> bad mmc pointer for dump <=====\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	// Convert address and size to block units
	dst_offset_in_block = ALIGN(dst_offset, 512) / 512;
	size_in_block = ALIGN(size, nand->writesize) / 512;
	printf("nand =====> source     : start address: 0x%08X %08X  <=====\n", (u32)(start_addr_in_page >> 32),(u32)(start_addr_in_page & 0xFFFFFFFF));
	printf("nand =====> destination(mmc%d): dest address : 0x%08X %08X (%d) <=====\n", dst_mmc->host_id, (u32)(dst_offset >> 32),(u32)(dst_offset & 0xFFFFFFFF), dst_offset_in_block);
	printf("nand =====> dump data size : 0x%08X %08X (%d) <=====\n", (u32)(size >> 32),(u32)(size & 0xFFFFFFFF), size_in_block);

	if (0 == size_in_block) {
		printf("dump block count is 0\n");
		return NAND_CMD_STATUS_ERROR0;
	}

	while (size_in_block)
	{
		read_size = nand->erasesize;
		trans_size_in_block = nand->erasesize / 512;
		//if (size_in_block > MSDC_DUMP_BUF_BLOCKS){
		//	trans_size_in_block = MSDC_DUMP_BUF_BLOCKS;
		//}else{
		//	trans_size_in_block = size_in_block;
		//}
		//read_size = trans_size_in_block * 512;

		// Read data from  source address
		if (nand_block_isbad (nand, start_addr_in_page)) {
			printf ("Skipping bad block 0x%x\n",start_addr_in_page);
			start_addr_in_page += nand->erasesize;
			printf("start_addr_in_page:0x%x\n", start_addr_in_page);
			continue;
		}
		memset(buf, 0, sizeof(buf));
		nand_read_skip_bad(nand, start_addr_in_page, &read_size, buf);

		// Write data to destination device with special address (dst_offset)
		mmc_bwrite(dst_mmc->host_id, dst_offset_in_block, trans_size_in_block, buf);

		size_in_block -= trans_size_in_block;
		size_in_page -= trans_size_in_block * 512;
		start_addr_in_page += trans_size_in_block * 512;
		dst_offset_in_block += trans_size_in_block;

		i++;
		if ((i%64 == 0) && (i != 0))
		{
			printf("\r\n");
		}
	}

	if (buf)
	{
		free(buf);
		buf = NULL;
	}

	printf("============> dump data completed <============\n");

	return 0;

}

#endif


static int nand_dump(nand_info_t *nand, ulong off, int only_oob)
{
	int i;
	u_char *datbuf, *oobbuf, *p;

	datbuf = malloc(nand->writesize + nand->oobsize);
	oobbuf = malloc(nand->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = nand->writesize;
	ops.ooblen = nand->oobsize;
	ops.mode = MTD_OOB_RAW;
	i = nand->read_oob(nand, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
	printf("Page %08lx dump:\n", off);
	i = nand->writesize >> 4;
	p = datbuf;

	while (i--) {
		if (!only_oob)
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

	return 0;
}

/* ------------------------------------------------------------------------- */

static inline int str2long(char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

static int mtdparts_inited = 0;
int mtdparts_inited_once(void)
{
	if (!mtdparts_inited) {
		if (!mtdparts_init()) {
			mtdparts_inited = 1;
		}
	}
	return mtdparts_inited ? 0: -1;
}

int
arg_off_size(int argc, char *argv[], nand_info_t *nand, ulong *off, size_t *size)
{
	int idx = nand_curr_device;
#if defined(CONFIG_CMD_MTDPARTS)
#endif
#if 1
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (argc >= 1 && !(str2long(argv[0], off))) {
		if ((mtdparts_inited_once() == 0) &&
		    (find_dev_and_part(argv[0], &dev, &pnum, &part) == 0)) {
			if (dev->id->type != MTD_DEV_TYPE_NAND) {
				puts("not a NAND device\n");
				return -1;
			}
			*off = part->offset;
			if (argc >= 3)
			{
				if (!(str2long(argv[2], (ulong *)off))) {
					printf("'%s' is not a number\n", argv[2]);
					*off = 0;
				}
				*off += part->offset;
			}
			if (argc >= 2) {
				if (!(str2long(argv[1], (ulong *)size))) {
					printf("'%s' is not a number\n", argv[1]);
					return -1;
				}
				if (*size > part->size)
					*size = part->size;
			} else {
				*size = part->size;
			}
			idx = dev->id->num;
			*nand = nand_info[idx];
			goto out;
		}
	}
#endif

	if (argc >= 1) {
		if (!(str2long(argv[0], off))) {
			printf("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
	}

	if (argc >= 2) {
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("'%s' is not a number\n", argv[1]);
			return -1;
		}
	} else {
		*size = nand->size - *off;
	}

#if defined(CONFIG_CMD_MTDPARTS)
out:
#endif
	printf("device %d ", idx);
	if (*size == nand->size)
		puts("whole chip\n");
	else
		printf("offset 0x%lx, size 0x%zx\n", *off, *size);
	return 0;
}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
static void print_status(ulong start, ulong end, ulong erasesize, int status)
{
	printf("%08lx - %08lx: %08lx blocks %s%s%s\n",
		start,
		end - 1,
		(end - start) / erasesize,
		((status & NAND_LOCK_STATUS_TIGHT) ?  "TIGHT " : ""),
		((status & NAND_LOCK_STATUS_LOCK) ?  "LOCK " : ""),
		((status & NAND_LOCK_STATUS_UNLOCK) ?  "UNLOCK " : ""));
}

static void do_nand_status(nand_info_t *nand)
{
	ulong block_start = 0;
	ulong off;
	int last_status = -1;

	struct nand_chip *nand_chip = nand->priv;
	/* check the WP bit */
	nand_chip->cmdfunc(nand, NAND_CMD_STATUS, -1, -1);
	printf("device is %swrite protected\n",
		(nand_chip->read_byte(nand) & 0x80 ?
		"NOT " : ""));

	for (off = 0; off < nand->size; off += nand->erasesize) {
		int s = nand_get_lock_status(nand, off);

		/* print message only if status has changed */
		if (s != last_status && off != 0) {
			print_status(block_start, off, nand->erasesize,
					last_status);
			block_start = off;
		}
		last_status = s;
	}
	/* Print the last block info */
	print_status(block_start, off, nand->erasesize, last_status);
}
#endif

static void nand_print_info(int idx)
{
	nand_info_t *nand = &nand_info[idx];
	struct nand_chip *chip = nand->priv;
	printf("Device %d: ", idx);
	if (chip->numchips > 1)
		printf("%dx ", chip->numchips);
	printf("%s, sector size %u KiB, page size %u Bytes, oob size %u Bytes\n",
	       nand->name, nand->erasesize >> 10, nand->writesize, nand->oobsize);
}
extern int scan_block_fast(struct mtd_info *mtd, struct nand_bbt_descr *bd,
			  loff_t offs, uint8_t *buf, int len);
struct bootloader_message {
	char command[32];
	char status[32];
	char recovery[1024];
};

void dumpHex(uint8_t *buf, int len){
	int i = 0;
	printf("DumpHex");
	for(; i<len; i++){
		if(i%16==0){
			printf("\r\n");
		}
		printf("%08x ", buf[i]);
	}
	printf("\r\n");
}
int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, dev, ret = 0;
	ulong addr, off;
	size_t size;
	ulong from, to;
	char *cmd, *s;
	nand_info_t *nand;
	int loop, read_len, loop1;

#ifdef CONFIG_SYS_NAND_QUIET
	int quiet = CONFIG_SYS_NAND_QUIET;
#else
	int quiet = 0;
#endif
	const char *quiet_str = getenv("quiet");

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	if (quiet_str)
		quiet = simple_strtoul(quiet_str, NULL, 0) != 0;

	cmd = argv[1];

	if (strcmp(cmd, "info") == 0) {

		putc('\n');
		for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
			if (nand_info[i].name)
				nand_print_info(i);
		}
		return 0;
	}

	if (strcmp(cmd, "device") == 0) {

		if (argc < 3) {
			putc('\n');
			if ((nand_curr_device < 0) ||
			    (nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE))
				puts("no devices available\n");
			else
				nand_print_info(nand_curr_device);
			return 0;
		}
		dev = (int)simple_strtoul(argv[2], NULL, 10);
		if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[dev].name) {
			puts("No such device\n");
			return 1;
		}
		printf("Device %d: %s", dev, nand_info[dev].name);
		puts("... is now current device\n");
		nand_curr_device = dev;

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
		/*
		 * Select the chip in the board/cpu specific driver
		 */
		board_nand_select_device(nand_info[dev].priv, dev);
#endif

		return 0;
	}
	/* the following commands operate on the current device */
	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];

	if (strcmp(cmd, "bad") == 0) {
		printf("\nDevice %d bad blocks:\n", nand_curr_device);
		for (off = 0; off < nand->size; off += nand->erasesize)
			if (nand_block_isbad(nand, off))
				printf("  %08lx\n", off);
		return 0;
	}

	//recovery cmd: nand recovery misc
	if(strcmp(cmd, "recovery")==0){
		printf("\nDevice %d check recovery mode\r\n", nand_curr_device);
		u_char *buf = malloc(nand->writesize + nand->oobsize);
		size_t len = nand->writesize;
		if(buf==NULL){
			printf("\nCan't alloc memory for check\r\n");
			return -1;
		}else{
			memset(buf, 0xff, nand->writesize+nand->oobsize);
		}

		if (arg_off_size(argc - 2, argv + 2, nand, &off, &size) != 0){
			free(buf);
			return -1;
		}

		ret = nand_read_skip_bad(nand,off+nand->writesize, &len, buf);
		if(ret != 0){
			printf("recovery check: nand read error\r\n");
			free(buf);
			return -1;
		}

		struct bootloader_message *buffer = (struct bootloader_message *)buf;
		//dumpHex(buffer, sizeof(*buffer));
		if(strncmp(buffer->recovery, "recovery", 8)==0){
			free(buf);
			return 0;
		}else{
			free(buf);
			return -1;
		}
	}
	/*
	 * Syntax is:
	 *   0    1     2       3    4
	 *   nand erase [clean] [off size]
	 */
	if (strcmp(cmd, "erase") == 0 || strcmp(cmd, "scrub") == 0) {


		nand_erase_options_t opts;
		/* "clean" at index 2 means request to write cleanmarker */
		int clean = argc > 2 && !strcmp("clean", argv[2]);
		int o = clean ? 3 : 2;
		int scrub = !strcmp(cmd, "scrub");
		int part = 0;
		int chip = 0;
		int spread = 0;
		int args = 2;

		if (cmd[5] != 0) {
			if (!strcmp(&cmd[5], ".spread")) {
				spread = 1;
			} else if (!strcmp(&cmd[5], ".part")) {
				part = 1;
				args = 1;
			} else if (!strcmp(&cmd[5], ".chip")) {
				chip = 1;
				args = 0;
			} else {
				goto usage;
			}
		}

		printf("\nNAND %s: ", scrub ? "scrub" : "erase");
		/* skip first two or three arguments, look for offset and size */
		if (arg_off_size(argc - o, argv + o, nand, &off, &size) != 0)
			return 1;

		memset(&opts, 0, sizeof(opts));
		opts.offset = off;
		opts.length = size;
		opts.jffs2  = clean;
		opts.quiet  = quiet;
		opts.spread = spread;

		if (scrub) {
#ifndef  CONFIG_NAND_DEBUG_VERSION
			puts("Warning: "
			     "scrub option will erase all factory set "
			     "bad blocks!\n"
			     "         "
			     "There is no reliable way to recover them.\n"
			     "         "
			     "Use this command only for testing purposes "
			     "if you\n"
			     "         "
			     "are sure of what you are doing!\n"
			     "\nReally scrub this NAND flash? <y/N>\n");

			if (getc() == 'y' && getc() == '\r') {
				opts.scrub = 1;
			} else {
				puts("scrub aborted\n");
				return -1;
			}
#else
		puts("Warning:scrub option will erase all factory set bad blocks!\n");
		opts.scrub = 1;
#endif
		}
		ret = nand_erase_opts(nand, &opts);
		printf("%s\n", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;

	}

	if (strncmp(cmd, "dump", 4) == 0) {
		if (argc < 3)
			goto usage;

		s = strchr(cmd, '.');
		off = (int)simple_strtoul(argv[2], NULL, 16);

		if (s != NULL && strcmp(s, ".oob") == 0)
			ret = nand_dump(nand, off, 1);
		else
			ret = nand_dump(nand, off, 0);

		return ret == 0 ? 1 : 0;

	}

#ifdef  CONFIG_NAND_DEBUG_VERSION

    if(strncmp(cmd, "rawdump", 7) == 0){

		if(argc < 6){
			printf("Usage:nand rawdump [sd slot] [start addr] [size] [dst addr]\n");
			return 0;
		}

		// dst device
		int dst_dev = simple_strtoul(argv[2], NULL, 10);

		ulong start_addr = 0, size = 0, dst_offset = 0;
		start_addr = (int)simple_strtoul(argv[3], NULL, 16);
		size = (int)simple_strtoul(argv[4], NULL, 16);
		dst_offset = (int)simple_strtoul(argv[5], NULL, 16);
		printf("dst_dev is %d, start_addr is 0x%08X, size is 0x%08X, dst_offset is 0x%08X\n",
			dst_dev, start_addr, size, dst_offset);
		struct mmc *dst_mmc = find_mmc_device(dst_dev);
		int err = mmc_init(dst_mmc);
		if (err)
			return err;

		nand_dump_data(nand, dst_mmc, dst_offset, start_addr, size);
        return 0;
    }

    if(strncmp(cmd, "ext4dump", 8) == 0){

		if(argc < 7){
			printf("Usage:nand ext4dump [sd slot] [ext4 partition name] [start logic addr] [size] [dst addr]\n");
			return 0;
		}
		// dst device
		int dst_dev = simple_strtoul(argv[2], NULL, 10);
		ulong start_phy_addr = 0, start_log_addr = 0, size = 0, dst_offset = 0;

		//Partition address
		if(strncmp(argv[3], "system_ext4", 11) != 0 &&
			strncmp(argv[3], "app_ext4", 8) != 0 && strncmp(argv[3], "data_ext4", 9) != 0){
			printf("Not ext4 partition\n");
			return -1;
		}
		if(arg_off_size(1, argv + 3, nand, &start_phy_addr, &partition_size) != 0){
			printf("Get offset error\r\n");
			return -1;
		}
		printf("off 0x%x, partition_size 0x%x\r\n", start_phy_addr, partition_size);

		start_log_addr = (int)simple_strtoul(argv[4], NULL, 16);
		size = (int)simple_strtoul(argv[5], NULL, 16);
		dst_offset = (int)simple_strtoul(argv[6], NULL, 16);
		printf("dst_dev is %d, start_phy_addr is 0x%08X, start_log_addr is 0x%08X, size is 0x%08X, dst_offset is 0x%08X\n",
			dst_dev, start_phy_addr, start_log_addr, size, dst_offset);
		struct mmc *dst_mmc = find_mmc_device(dst_dev);
		int err = mmc_init(dst_mmc);
		if (err)
			return err;

		nand_dump_ext4data(nand, dst_mmc, dst_offset, start_phy_addr, start_log_addr, size);
        return 0;
    }

	if(strncmp(cmd, "testperf", 8) == 0){
		int rw = simple_strtoul(argv[2], NULL, 16);
		ulong startaddr = simple_strtoul(argv[3], NULL, 16);
		size = (int)simple_strtoul(argv[4], NULL, 16);
		ulong start_addr_in_page = (startaddr >> 4) << 4;
		ulong size_in_page = ALIGN(size, nand->writesize);
		u8* buf = (u8*)malloc(size_in_page);
		ulong size_mb = size_in_page /(1024 * 1024);
		ulong start_addr_in_block = ALIGN(startaddr, nand->erasesize);
		memset(buf, 0, sizeof(buf));
		int time1 = boot_time_ms();
		if(rw){
			nand_read_skip_bad(nand, start_addr_in_page, &size_in_page, buf);
		}else{
			nand_erase_options_t opts;
			memset(&opts, 0, sizeof(opts));
			opts.offset = start_addr_in_block;
			opts.length = size;
			opts.quiet  = quiet;
			ret = nand_erase_opts(nand, &opts);
			printf("%s\n", ret ? "ERROR" : "OK");
			nand_write_skip_bad(nand, start_addr_in_page, &size_in_page, buf);
		}
		int time2 = boot_time_ms();
		printf(" === %d MB Data %s cost %lld ms and speed is %d MB\\S ===\n",size_mb,
			rw? "Read":"Write",time2 - time1,(size_mb * 1000/(time2-time1)));
	}

#endif

	static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

	if (strncmp(cmd, "mtktool", 7) == 0) {

		struct nand_chip *chip = nand->priv;
		struct erase_info instr;
		int ret;

		for(loop1 = 2; loop1 < (chip->chipsize >> (chip->bbt_erase_shift)); loop1++ ){

			instr.mtd = nand;
			instr.addr = (u32)(1<<chip->bbt_erase_shift) * (u32)loop1;
			instr.len = (1<<chip->bbt_erase_shift) * 1;
			instr.callback = 0;
			ret = nand_erase_nand_ext(nand, &instr, 0);
			if (ret){
				if (nand->block_markbad(nand, NAND_MAX_PAGESIZE * loop1 * (1<<(chip->bbt_erase_shift-chip->page_shift)))) {
					printf("block 0x%08lx NOT marked "
						"as bad! ERROR %d\n",
						loop1, ret);
					ret = 1;
				} else {
					printf("block 0x%08lx successfully "
						"marked as bad\n",
						loop1);
				}
			}else{
				printf("Block %d is a good block\r\n", loop1);
			}
		}

		printf("do_nand: mtktool \r\n");
		printf("do_nand: scan bbt\r\n");
		chip->options |= NAND_BBT_SCANNED;
		chip->scan_bbt(nand);
	}

	if(strncmp(cmd, "format" , 6)==0){
		printf("Format usrdata partition");
		u32 part_size;
		u32 offset;
		{
			struct part_info *rw_part;
			struct mtd_device *dev;
			u8 part_num;
			if(find_dev_and_part("usrdata", &dev, &part_num, &rw_part)){
				printf("Can not get part %s information\r\n", "usrdata");
				return -1;
			}
			printf("part %s information: size %x\r\n", "usrdata",rw_part->size);
			part_size = rw_part->size;
			offset = rw_part->offset;
		}
		nand_format_userdata(nand, offset, part_size);
		return 0;
	}

	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {


		char data[NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE];
		char write_data[NAND_MAX_PAGESIZE];
		char oob[NAND_MAX_OOBSIZE];
		struct erase_info instr;
// test erase--read--write--read--erase

#if 1
		int read;

		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

		read = strncmp(cmd, "read", 4) == 0; /* 1 = read, 0 = write */
		printf("\nNAND %s %s: \n", read ? "read" : "write", argv[3]);

#if 1
		if (arg_off_size(argc - 3, argv + 3, nand, &off, &size) != 0)
			return 1;

#if 0   //to fixup nand read/write ram_addr off size   command
		size_t part_size;
		struct part_info *rw_part;
		{
			struct mtd_device *dev;
			u8 part_num;
			if(find_dev_and_part(argv[3], &dev, &part_num, &rw_part)){
				printf("Can not get part %s information\r\n", argv[3]);
				return -1;
			}
			printf("part %s information: size %x\r\n", argv[3],rw_part->size);
			part_size = rw_part->size;
		}
#endif
		s = strchr(cmd, '.');

		if (!s || !strcmp(s, ".jffs2") ||
		    !strcmp(s, ".e") || !strcmp(s, ".i")) {
			if (read)
				ret = nand_read_skip_bad(nand, off, &size,
							 (u_char *)addr);
			else{
#if 0
					if ((memcmp(argv[3], "system", 6) == 0)|| (memcmp(argv[3], "data", 4) == 0)||(memcmp(argv[3], "cache", 5)==0)){
					#if SUPPORT_YAFFS2
					ret = nand_write_yaffs2_image(nand,off, &size, (u_char *)addr);
					#endif
					#if SUPPORT_EXT3
					ret = nand_write_ext_image(nand, rw_part->name, off, part_size, &size, (u_char *)addr);
					#endif
                    #if SUPPORT_EXT4
                    ret = nand_write_ext4_image(nand, off, &size, (u_char *)addr);
                    #endif
					#if SUPPORT_VFAT
					ret = nand_write_with_lognum(nand, off, &size, (u_char *)addr);
					#endif
				}else if(memcmp(argv[3], "usrdata", 7) == 0){
					ret = nand_format_userdata(nand, off, part_size);
				}else{
#endif
					ret = nand_write_skip_bad(nand, off, &size,
							  (u_char *)addr);
//				}
			}
		}else if (!strcmp(s, ".oob")) {
			/* out-of-band data */
			mtd_oob_ops_t ops = {
				.oobbuf = (u8 *)addr,
				.ooblen = size,
				.mode = MTD_OOB_RAW
			};

			if (read)
				ret = nand->read_oob(nand, off, &ops);
			else
				ret = nand->write_oob(nand, off, &ops);
		} else {
			printf("Unknown nand command suffix '%s'.\n", s);
			return 1;
		}
#endif
		printf(" %zu bytes %s: %s\n", size,
		       read ? "read" : "written", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
#endif
	}

	if (strcmp(cmd, "markbad") == 0) {
		argc -= 2;
		argv += 2;

		if (argc <= 0)
			goto usage;

		while (argc > 0) {
			addr = simple_strtoul(*argv, NULL, 16);

			if (nand->block_markbad(nand, addr)) {
				printf("block 0x%08lx NOT marked "
					"as bad! ERROR %d\n",
					addr, ret);
				ret = 1;
			} else {
				printf("block 0x%08lx successfully "
					"marked as bad\n",
					addr);
			}
			--argc;
			++argv;
		}
		return ret;
	}

	if (strcmp(cmd, "biterr") == 0) {
		/* todo */
		return 1;
	}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	if (strcmp(cmd, "lock") == 0) {
		int tight = 0;
		int status = 0;
		if (argc == 3) {
			if (!strcmp("tight", argv[2]))
				tight = 1;
			if (!strcmp("status", argv[2]))
				status = 1;
		}
		if (status) {
			do_nand_status(nand);
		} else {
			if (!nand_lock(nand, tight)) {
				puts("NAND flash successfully locked\n");
			} else {
				puts("Error locking NAND flash\n");
				return 1;
			}
		}
		return 0;
	}

	if (strcmp(cmd, "unlock") == 0) {
		if (arg_off_size(argc - 2, argv + 2, nand, &off, &size) < 0)
			return 1;

		if (!nand_unlock(nand, off, size)) {
			puts("NAND flash successfully unlocked\n");
		} else {
			puts("Error unlocking NAND flash, "
			     "write and erase will probably fail\n");
			return 1;
		}
		return 0;
	}
#endif

	if (strcmp(cmd, "compare") == 0) {
		unsigned int addr_src;
		unsigned int addr_dest;
		unsigned int len;
		unsigned int flag = 0, num = 0;
		if (argc < 5 )
			return 1;
		addr_src = (ulong)simple_strtoul(argv[2], NULL, 16);
		addr_dest = (ulong)simple_strtoul(argv[3], NULL, 16);
		len = (ulong)simple_strtoul(argv[4], NULL, 16);
		for(flag = 0; flag < len; flag++){
			if ((*((char*)(addr_src + flag))) != (*((char*)(addr_dest + flag)))){
				printf("src[%d] = 0x%x, dest[%d] = 0x%x\r\n",
					flag, (*((char*)(addr_src + flag))),
					flag, (*((char*)(addr_dest + flag))));
				num++;
			}

			flag++;

		}

		printf("Notice: There are %d bytes error\r\n", num);
	}

	if(strncmp(cmd, "badblock", 8) == 0){
		unsigned int offset = 0;
		if (argc < 4)
			goto usage;

		if(argc > 4){
			offset = (ulong)simple_strtoul(argv[4], NULL, 16);
		}

		if (arg_off_size(argc - 2, argv + 2, nand, &off, &size) != 0)
			return -1;

		ret = nand_rw_offset(nand, &off, &size) - off + offset;

		return ret;
	}
usage:
	cmd_usage(cmdtp);
	return 1;
}


int atc_restore_nand_att(void)
{
     struct nand_chip *chip = (struct nand_chip *)(nand_info[nand_curr_device].priv);
	 if (0 != atc_nand_read_bbt(chip, &g_bbt))
	 {
		 printf("nand_read_atc_bbt failed\r\n");
		 return (-1);
	 }

	 return (0);
}

int atc_nand_read_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt)
{
	char *argv[5] = {"nand", "read", "0x0FE00000", "boot", "0x10000"};
	int  ret;
	struct nand_atc_bbt *save_bbt;

	printf("nand_read_atc_bbt start\r\n");
    ret = do_nand(NULL, 0, 5, argv);
	if (0 != ret)
	{
		printf("nand_read_atc_bbt read boot partition error\r\n");
		return (-1);
	}
	save_bbt = (struct nand_atc_bbt *)(0x0FE00000 + 0x8800);
	if (NAND_ATE_BBT_SIGNITURE != save_bbt->signiture)
	{
		printf("Not bbt in boot partition\r\n");
		return (-1);
	}
	if (NAND_ATC_BB_VER != save_bbt->version)
	{
		printf("bbt version is not match. Save version(0x%x), uboot version(0x%x)\r\n", save_bbt->version, NAND_ATC_BB_VER);
		return (-1);
	}
	memcpy(atcbbt, save_bbt, sizeof(struct nand_atc_bbt));
	{
		int loop;
		for(loop = 0; loop < atcbbt->realnum; loop  ++ )
		{
		     printf("Bad block %d\r\n", atcbbt->badblocks[loop]);
		}
	}
	printf("nand_read_atc_bbt end\r\n");
	return (0);

}

int atc_nand_write_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt)
{
	char *rargv[5] = {"nand", "read", "0x0FE00000", "boot"};
	char *eargv[4] = {"nand", "erase", "boot"};
	char *wargv[5] = {"nand", "write", "0x0FE00000", "boot"};
	char buf1[10] = {0};
	int  ret;
	struct nand_atc_bbt *save_bbt;

	printf("nand_write_atc_bbt start\r\n");
	sprintf(buf1, "%x", (1 << chip->bbt_erase_shift));
	rargv[4] = buf1;
    ret = do_nand(NULL, 0, 5, rargv);
	if (0 != ret)
	{
		printf("nand_write_atc_bbt read boot partition error\r\n");
		return (-1);
	}
	save_bbt = (struct nand_atc_bbt *)(0x0FE00000 + 0x8800);
	if (NAND_ATE_BBT_SIGNITURE != atcbbt->signiture)
	{
		printf("Not bbt in boot partition\r\n");
		return (-1);
	}
	if (NAND_ATC_BB_VER != atcbbt->version)
	{
		printf("bbt version is not match. Save version(0x%x), uboot version(0x%x)\r\n", save_bbt->version, NAND_ATC_BB_VER);
		return (-1);
	}
	memcpy( save_bbt, atcbbt, sizeof(struct nand_atc_bbt));


	// erase boot partition
	eargv[3] = buf1;
    ret = do_nand(NULL, 0, 4, eargv);
	if (0 != ret)
	{
		printf("nand_write_atc_bbt erase boot partition error\r\n");
		return (-1);
	}

	// rewrite boot partition
	wargv[4] = buf1;
    ret = do_nand(NULL, 0, 5, wargv);
	if (0 != ret)
	{
		printf("nand_write_atc_bbt re-write boot partition error\r\n");
		return (-1);
	}

	printf("nand_write_atc_bbt end\r\n");
	return (0);
}

int atc_nand_init_bbt(struct nand_chip *chip, struct nand_atc_bbt * atcbbt)
{
	u32  blocks;
	u32  loop;
	if (!chip)
		chip = (struct nand_chip *)(nand_info[nand_curr_device].priv);
	printf("nand_init_atc_bbt start\r\n");
	blocks = chip->chipsize >> chip->bbt_erase_shift;
	memset(atcbbt, 0, sizeof(struct nand_atc_bbt));
	atcbbt->rsvbn = NAND_ATC_MAX_RB_NUM;

	for (loop = 0; loop < NAND_ATC_MAX_RB_NUM; loop ++)
	{
	    atcbbt->phyidx[loop] = blocks - NAND_ATC_MAX_RB_NUM + loop;
		atcbbt->logidx[loop] = ATC_RB_FREE;
	}

	atcbbt->maxbb = NAND_ATC_MAX_BB_NUM;
	atcbbt->realnum = 0;
	atcbbt->signiture = NAND_ATE_BBT_SIGNITURE;
	atcbbt->version = NAND_ATC_BB_VER;

	printf("nand_init_atc_bbt end\r\n");
	return (0);
}



U_BOOT_CMD(nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"NAND sub-system",
	"info - show available NAND devices\n"
	"nand device [dev] - show or set current device\n"
	"nand read - addr off|partition size\n"
	"nand write - addr off|partition size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"nand erase [clean] [off size] - erase 'size' bytes from\n"
	"    offset 'off' (entire device if not specified)\n"
	"nand bad - show bad blocks\n"
	"nand dump[.oob] off - dump page\n"
	"nand scrub - really clean NAND erasing bad blocks (UNSAFE)\n"
	"nand markbad off [...] - mark bad block(s) at offset (UNSAFE)\n"
	"nand biterr off - make a bit error at offset (UNSAFE)"
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	"\n"
	"nand lock [tight] [status]\n"
	"    bring nand to lock state or display locked pages\n"
	"nand unlock [offset] [size] - unlock section\n"
	"nand mtktool\n"
#endif
);

static int nand_load_image(cmd_tbl_t *cmdtp, nand_info_t *nand,
			   ulong offset, ulong addr, char *cmd)
{
	int r;
	char *ep, *s;
	size_t cnt;
	image_header_t *hdr;
#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	s = strchr(cmd, '.');
	if (s != NULL &&
	    (strcmp(s, ".jffs2") && strcmp(s, ".e") && strcmp(s, ".i"))) {
		printf("Unknown nand load suffix '%s'\n", s);
		show_boot_progress(-53);
		return 1;
	}

	printf("\nLoading from %s, offset 0x%lx\n", nand->name, offset);

	cnt = nand->writesize;
	r = nand_read_skip_bad(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		puts("** Read error\n");
		show_boot_progress (-56);
		return 1;
	}
	show_boot_progress (56);

	switch (genimg_get_format ((void *)addr)) {
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *)addr;

		show_boot_progress (57);
		image_print_contents (hdr);

		cnt = image_get_image_size (hdr);
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *)addr;
		puts ("Fit image detected...\n");

		cnt = fit_get_size (fit_hdr);
		break;
#endif
	default:
		show_boot_progress (-57);
		puts ("** Unknown image type\n");
		return 1;
	}
	show_boot_progress (57);

	r = nand_read_skip_bad(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		puts("** Read error\n");
		show_boot_progress (-58);
		return 1;
	}
	show_boot_progress (58);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier, we need complete FIT image in RAM first */
	if (genimg_get_format ((void *)addr) == IMAGE_FORMAT_FIT) {
		if (!fit_check_format (fit_hdr)) {
			show_boot_progress (-150);
			puts ("** Bad FIT image format\n");
			return 1;
		}
		show_boot_progress (151);
		fit_print_contents (fit_hdr);
	}
#endif

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep, "yes") == 0)) {
		char *local_args[2];
		extern int do_bootm(cmd_tbl_t *, int, int, char *[]);

		local_args[0] = cmd;
		local_args[1] = NULL;

		printf("Automatic boot of image at addr 0x%08lx ...\n", addr);

		do_bootm(cmdtp, 0, 1, local_args);
		return 1;
	}
	return 0;
}

int do_nandboot(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	int idx;
	ulong addr, offset = 0;
#if defined(CONFIG_CMD_MTDPARTS)
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (argc >= 2) {
		char *p = (argc == 2) ? argv[1] : argv[2];
		if (!(str2long(p, &addr)) && (mtdparts_init() == 0) &&
		    (find_dev_and_part(p, &dev, &pnum, &part) == 0)) {
			if (dev->id->type != MTD_DEV_TYPE_NAND) {
				puts("Not a NAND device\n");
				return 1;
			}
			if (argc > 3)
				goto usage;
			if (argc == 3)
				addr = simple_strtoul(argv[1], NULL, 16);
			else
				addr = CONFIG_SYS_LOAD_ADDR;
			return nand_load_image(cmdtp, &nand_info[dev->id->num],
					       part->offset, addr, argv[0]);
		}
	}
#endif

	show_boot_progress(52);
	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
#if defined(CONFIG_CMD_MTDPARTS)
usage:
#endif
		cmd_usage(cmdtp);
		show_boot_progress(-53);
		return 1;
	}

	show_boot_progress(53);
	if (!boot_device) {
		puts("\n** No boot device **\n");
		show_boot_progress(-54);
		return 1;
	}
	show_boot_progress(54);

	idx = simple_strtoul(boot_device, NULL, 16);

	if (idx < 0 || idx >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[idx].name) {
		printf("\n** Device %d not available\n", idx);
		show_boot_progress(-55);
		return 1;
	}
	show_boot_progress(55);

	return nand_load_image(cmdtp, &nand_info[idx], offset, addr, argv[0]);
}

U_BOOT_CMD(nboot, 4, 1, do_nandboot,
	"boot from NAND device",
	"[partition] | [[[loadAddr] dev] offset]"
);
#endif

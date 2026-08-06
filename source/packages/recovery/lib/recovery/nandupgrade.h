/*
 *  nandupgrade.c
 *
 *  Copyright (C) 2018 Dandan Liu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _NANDUPGRADE_H
#define _NANDUPGRADE_H

#define RESERVED_SECTOR_IN_BLOCK        0xFFFFFA
//byqy unsigned int g_bReservedSectorNumInBlock = 1;

struct nand_dev_info {
	int block_cnt;
	int block_size;
	int page_size;
	int oob_size;
};

typedef struct _SectorInfo {
	uint16_t bBadBlock;
	uint16_t wReserved2;
	uint32_t dwReserved1;
} SectorInfo, *PSectorInfo;

#define  RE_FAIL  (-1)

typedef enum PARTITION {
	PRELOADER = 0,
	PRELOADER_BK,
	DATAZONE,
	DATAZONE_BK,
	UBOOT,
	TRUSTZONE,
	ARM2,
	DTB,
	LOGO,
	BOOT_MISC,
	VBA,
	METAZONE,
	KERNEL,
	SYSTEM,
	USRDATA,
	RECOVERY,
	ALLNAND
} PARTITION;

int64_t nand_write_raw_image(void* buf,uint64_t offset,uint64_t size);
int64_t nand_write_ext4_image(void* buf,uint64_t offset,uint64_t size);
int64_t nand_read_raw_image(void* buf,uint64_t offset,uint64_t size);
int64_t nand_read_ext4_image(void* buf,uint64_t offset,uint64_t size);
int nand_erase(uint64_t start,uint32_t cnt);
int nand_all_erase(void);
int get_nand_info(struct nand_dev_info *nand_info);
int nand_rw_start(int partition,uint64_t size);
int nand_rw_end(void);
int64_t nand_ext4_write_endoffset(void);

#endif /* _NANDUPGRADE_H */


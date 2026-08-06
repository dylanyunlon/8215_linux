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

#ifndef _METAZONE_INTER_H_
#define _METAZONE_INTER_H_
#include <asm-arm/types.h>
#include <asm-arm/arch-ac83xx/x_typedef.h>
/* ---------------------------------------------------------- */
typedef struct _CRC16{
    unsigned short crc_dword;
	unsigned short crc_binary;
	unsigned short crc_reserved;
	unsigned short crc_header;
}CRCflag;

typedef struct _MetaZone {
	u32 dwVersion;
	u32 dwSignature;
	u32 dwDataSize;		/* Metazone size (in bytes) */
	u32 dwReserveSize;
	u32 dwValueOffset;
	u32 dwValueNum;
	u32 dwBinaryOffset;	/*  */
	u32 dwBinaryNum;	/* Number of binary data */
	u32 dwBinaryItemSize;	/* Max bytes of one item of binary data. */
} TMetaZone, *PTMetaZone;	/* limited size is 3 page size, that might be 3*512 bytes */


typedef struct _SlotHeader {
	u32 magic;		/* METAZONE_SLOT_MAGIC */
	u32 seq;		/* Sequence number for latest detection */
	u8 state;		/* 0x01: writing, 0xFF: valid */
	u8 reserved[3];
	CRCflag crc;	/* CRC of the metazone data */
} SlotHeader;

typedef struct _Flag{//for emmc
	u32 write_before;
	u32 write_after;
}Wflag;

typedef enum {//for emmc
	BACKUP_NONE,
	BACKUP_MTZ1,
	BACKUP_MTZ2
 }backup_mtz;
 

#define METAZONE_SIGNATURE 0xabcdef01
#define METAZONE_SIZE_MAX  0x10000U
#define METAZONE_VERSION  0x00010000
#define METAZONE_SLOT_MAGIC 0x12345678U

#define MTZ_SLOT_SIZE       (METAZONE_SIZE_MAX)  /*64KB*/
#define MTZ_MAX_SLOTS       16          /* 1MB / 64KB */
#define MTZ_ACTIVE_BLOCKS   2  /* Use 2 blocks for cycling */
#define MTZ_SLOT_HEADER_SIZE 512

/* MTZ_BLOCK_SIZE will be dynamic from mtd->erasesize, use mtz_block_size variable */

#define ERROR_INAVLID_METAZONE_SIGNATURE 100L
#define ERROR_INAVLID_METAZONE_SIZE      101L
u32 metazone_read(u32 u4Idx, u32 *pu4Data);
u32 metazone_readreserved(char *pbData, u32 u4Size);
u32 metazone_readbinary(u32 u4Idx, char *pbData, u32 u4Size);
u32 metaZone_write(u32 u4Idx, u32 u4Data);

u32 metazone_writereserved(char *pbData, u32 u4Size);
u32 metazone_writebinary(u32 u4Idx, char *pbData, u32 u4Size);
u32 metazone_flush(BOOL fgSync);
BOOL metazone_init(UINT32 mode);
/*index*/
#define  MZ_DISPLAY_RESOLUTION    ( 0x10040)

#endif				/* _METAZONE_INTER_H_ */

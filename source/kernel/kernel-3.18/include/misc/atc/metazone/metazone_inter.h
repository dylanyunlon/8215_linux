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

typedef struct _Flag{
	u32 write_before;
	u32 write_after;
}Wflag;

typedef struct _SlotHeader {
	u32 magic;		/* METAZONE_SLOT_MAGIC */
	u32 seq;		/* Sequence number for latest detection */
	u8 state;		/* 0x01: writing, 0xFF: valid */
	u8 reserved[3];
	CRCflag crc;	/* CRC of the metazone data */
} SlotHeader;

typedef enum {
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
u32 _MetaZone_Read(u32 u4Idx, u32 *pu4Data);
u32 _MetaZone_ReadReserved(char *pbData, u32 u4Size);
u32 _MetaZone_ReadBinary(u32 u4Idx, char *pbData, u32 u4Size);
u32 _MetaZone_Write(u32 u4Idx, u32 u4Data);

u32 _MetaZone_WriteReserved(char *pbData, u32 u4Size);
u32 _MetaZone_WriteBinary(u32 u4Idx, char *pbData, u32 u4Size);
int mtz_proc_init(void);

#endif				/* _METAZONE_INTER_H_ */

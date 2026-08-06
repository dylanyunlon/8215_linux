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



#include "metazone_inter.h"
#include "metazone.h"
#include "metazone_ioctl.h"
#include "printf.h"
#include <asm-arm/arch-ac83xx/x_typedef.h>
#include <asm-arm/types.h>
#include <linux/mtd/atc_nand.h>
#include "reserved_memory.h"  
#include <nand.h>

BOOL _fgWritableZoneInited = FALSE;
VOID  *_pMetaZone = NULL;
TMetaZone *_pMetaHeader = NULL;
PBYTE _pbReserve = NULL;
UINT32 *_pu4Value = NULL;
UINT32 _u4BinaryStart = 0;

extern unsigned int _sdagentflag;
extern unsigned long long g_metazonePartitonAddr;
extern unsigned long long g_metazonePartitonSize;

#undef MEMRSV_PHY_TO_VIRT
#define MEMRSV_PHY_TO_VIRT(x) (x)
//now we use 256KB to store metazone & backup (256KB*2) by mtk68080
unsigned long long g_metazone_mem_addr;
#ifndef CONFIG_BOOT_MMC
static UINT8 g_mtz_header_buf[MTZ_SLOT_HEADER_SIZE] __attribute__((aligned(4)));
static UINT32 g_mtz_current_slot = 0;
static UINT32 g_mtz_current_seq = 0;
static BOOL g_mtz_slot_ready = FALSE;
static UINT32 g_mtz_slot_count = 0;
static UINT32 g_mtz_slots_per_block = 0;
static UINT32 g_mtz_active_blocks[2];
static UINT32 g_mtz_active_count = 0;
nand_info_t *nand;
static UINT32 g_mtz_seqs[MTZ_MAX_SLOTS];
static UINT32 g_mtz_valid[MTZ_MAX_SLOTS];
#define MTZ_PARTITION_SIZE  (0x200000)  /*2MB*/
static void mtz_set_mem_ptrs(void);
static UINT32 mtz_slot_data_addr(UINT32 slot_idx);
static inline BOOL mtz_seq_after(UINT32 seq_a, UINT32 seq_b)
{
	return ((INT32)(seq_a - seq_b) > 0) ? TRUE : FALSE;
}

static unsigned short calculate_CRC16(unsigned char* pbuff, unsigned short length)
{
	unsigned short shift, data, val;
	int i;
	shift = 0xffff;
	if (pbuff == NULL || length == 0) {
		printf("[MTZ][UBOOT] calculate_CRC input is invalid\r\n");
		return (unsigned short)0xFFFF;
	}
	for (i = 0; i < length * 8; i++) {
		if ((i % 8) == 0)
			data = (*pbuff++) << 8;
		val = shift ^ data;
		shift = shift << 1;
		data = data << 1;
		if (val & 0x8000)
			shift = shift ^ 0x1021;
	}

	return shift;
}

static SlotHeader *mtz_get_slot_header_rsv_addr(void)
{
	SlotHeader *header;

	if (_pMetaZone == NULL) {
		printf("[MTZ][UBOOT] mtz_get_slot_header_rsv_addr: _pMetaZone is NULL\r\n");
		return NULL;
	}

	header = (SlotHeader *)((char *)_pMetaZone + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE);
	return header;
}

static unsigned short recount_header_crc(const TMetaZone *pMetaHeader)
{
	unsigned short reCRC;
	SlotHeader *header;

	if (pMetaHeader == NULL) {
		printf("[MTZ][UBOOT] header CRC input is invalid\r\n");
		return (unsigned short)-1;
	}

	reCRC = calculate_CRC16((unsigned char *)pMetaHeader, (unsigned short)sizeof(TMetaZone));
	header = mtz_get_slot_header_rsv_addr();
	if (header)
		header->crc.crc_header = reCRC;
	printf("[MTZ][UBOOT] Header CRC recount (%u)\r\n", reCRC);
	return reCRC;
}

static int recount_CRC_value(int type)
{
	unsigned short reCRC;
	SlotHeader *header;

	if (_pMetaZone == NULL) {
		printf("[MTZ][UBOOT] CRC recount failed: metazone memory is NULL\r\n");
		return -1;
	}

	header = mtz_get_slot_header_rsv_addr();
	if (!header) {
		printf("[MTZ][UBOOT] CRC recount failed: slot header invalid\r\n");
		return -1;
	}

	switch (type) {
	case 1:
		reCRC = calculate_CRC16((char *)(_pu4Value + 1), (unsigned short)(_pMetaHeader->dwValueNum - 1) * 4);
		header->crc.crc_dword = reCRC;
		printf("[MTZ][UBOOT] Dword CRC recount (%u)\r\n", reCRC);
		break;
	case 2:
		reCRC = calculate_CRC16((char *)_u4BinaryStart,
			(unsigned short)((_pMetaHeader->dwBinaryItemSize + 4) * _pMetaHeader->dwBinaryNum));
		header->crc.crc_binary = reCRC;
		printf("[MTZ][UBOOT] Binary CRC recount (%u)\r\n", reCRC);
		break;
	case 3:
		reCRC = calculate_CRC16(_pbReserve, (unsigned short)(_pMetaHeader->dwReserveSize));
		header->crc.crc_reserved = reCRC;
		printf("[MTZ][UBOOT] Reserved CRC recount (%u)\r\n", reCRC);
		break;
	default:
		printf("[MTZ][UBOOT] CRC recount type [%d] is wrong\r\n", type);
		return -1;
	}

	return 0;
}

static unsigned short mtz_calc_header_crc(const TMetaZone *pMetaHeader)
{
	if (pMetaHeader == NULL) {
		printf("[MTZ][UBOOT] mtz_calc_header_crc: null input\r\n");
		return (unsigned short)0xFFFF;
	}

	return calculate_CRC16((unsigned char *)pMetaHeader, (unsigned short)sizeof(TMetaZone));
}

static BOOL mtz_check_header_crc(void)
{
	SlotHeader *header = mtz_get_slot_header_rsv_addr();
	unsigned short calc;

	if (_pMetaHeader == NULL || header == NULL) {
		printf("[MTZ][UBOOT] mtz_check_header_crc: header or meta pointer NULL\r\n");
		return FALSE;
	}

	calc = mtz_calc_header_crc(_pMetaHeader);
	if (calc != header->crc.crc_header) {
		printf("[MTZ][UBOOT] Header CRC mismatch (calc=%u stored=%u)\r\n", calc, header->crc.crc_header);
		return FALSE;
	}
	return TRUE;
}

static BOOL mtz_check_dword_crc(void)
{
	SlotHeader *header = mtz_get_slot_header_rsv_addr();
	unsigned short calc;

	if (_pMetaHeader == NULL || header == NULL || _pu4Value == NULL) {
		printf("[MTZ][UBOOT] mtz_check_dword_crc: invalid pointers\r\n");
		return FALSE;
	}

	if (_pMetaHeader->dwValueNum <= 1) /* nothing to check (only count field present) */
		return TRUE;

	calc = calculate_CRC16((unsigned char *)(_pu4Value + 1), (unsigned short)((_pMetaHeader->dwValueNum - 1) * 4));
	if (calc != header->crc.crc_dword) {
		printf("[MTZ][UBOOT] Dword CRC mismatch (calc=%u stored=%u)\r\n", calc, header->crc.crc_dword);
		return FALSE;
	}
	return TRUE;
}

static BOOL mtz_check_binary_crc(void)
{
	SlotHeader *header = mtz_get_slot_header_rsv_addr();
	unsigned short calc;

	if (_pMetaHeader == NULL || header == NULL) {
		printf("[MTZ][UBOOT] mtz_check_binary_crc: invalid pointers\r\n");
		return FALSE;
	}

	if (_pMetaHeader->dwBinaryNum == 0)
		return TRUE;

	calc = calculate_CRC16((unsigned char *)_u4BinaryStart,
						   (unsigned short)((_pMetaHeader->dwBinaryItemSize + 4) * _pMetaHeader->dwBinaryNum));
	if (calc != header->crc.crc_binary) {
		printf("[MTZ][UBOOT] Binary CRC mismatch (calc=%u stored=%u)\r\n", calc, header->crc.crc_binary);
		return FALSE;
	}
	return TRUE;
}

static BOOL mtz_check_reserved_crc(void)
{
	SlotHeader *header = mtz_get_slot_header_rsv_addr();
	unsigned short calc;

	if (_pMetaHeader == NULL || header == NULL || _pbReserve == NULL) {
		printf("[MTZ][UBOOT] mtz_check_reserved_crc: invalid pointers\r\n");
		return FALSE;
	}

	if (_pMetaHeader->dwReserveSize == 0)
		return TRUE;

	calc = calculate_CRC16(_pbReserve, (unsigned short)(_pMetaHeader->dwReserveSize));
	if (calc != header->crc.crc_reserved) {
		printf("[MTZ][UBOOT] Reserved CRC mismatch (calc=%u stored=%u)\r\n", calc, header->crc.crc_reserved);
		return FALSE;
	}
	return TRUE;
}
static void mtz_set_mem_ptrs(void)
{
	_pMetaZone = (void *)MEMRSV_PHY_TO_VIRT((u32)g_metazone_mem_addr);
	_pMetaHeader = (TMetaZone *)_pMetaZone;
	_pbReserve = (BYTE *)((UINT32) _pMetaHeader + sizeof(TMetaZone));
	_pu4Value = (UINT32 *)((UINT32) _pMetaHeader + _pMetaHeader->dwValueOffset);
	_u4BinaryStart = (UINT32) _pMetaHeader + _pMetaHeader->dwBinaryOffset;
}


/* Check all CRCs; returns TRUE only when all individual checks pass */
static BOOL mtz_check_all_crcs(void)
{
	mtz_set_mem_ptrs();
	if (!mtz_check_header_crc())
		return FALSE;
	if (!mtz_check_dword_crc())
		return FALSE;
	if (!mtz_check_binary_crc())
		return FALSE;
	if (!mtz_check_reserved_crc())
		return FALSE;
	return TRUE;
}

static int mtz_read_storage(void *dst, UINT32 size, UINT32 offset)
{
	char szAddr[17];
	int ret;

	sprintf(szAddr, "0x%x", (u32)dst);
	char szSize[17];
	char szOffset[17];
	char *argv[6] = {"nand", "read", szAddr, "metazone", szSize, szOffset};
	sprintf(szSize, "0x%x", size);
	sprintf(szOffset, "0x%x", offset);
	ret = do_nand(NULL, 0, 6, argv);


	return ret;
}

static int mtz_write_storage(void *src, UINT32 size, UINT32 offset)
{
	char szAddr[17];
	int ret;

	sprintf(szAddr, "0x%x", (u32)src);
	char szSize[17];
	char szOffset[17];
	char *argv[6] = {"nand", "write", szAddr, "metazone", szSize, szOffset};
	sprintf(szSize, "0x%x", size);
	sprintf(szOffset, "0x%x", offset);
	ret = do_nand(NULL, 0, 6, argv);

	return ret;
}

static int mtz_read_slot_header_uboot(UINT32 slot_idx, SlotHeader *header)
{
	UINT32 slot_offset;

	if (!header)
		return -1;

	if (g_mtz_active_count == 0)
		return -1;

	slot_offset = mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE;
	memset(g_mtz_header_buf, 0, sizeof(g_mtz_header_buf));
	if (mtz_read_storage(g_mtz_header_buf, MTZ_SLOT_HEADER_SIZE, slot_offset) != 0)
		return -1;

	memcpy(header, g_mtz_header_buf, sizeof(SlotHeader));
	return 0;
}

static int mtz_read_slot_data_uboot(UINT32 slot_idx, void *dst)
{
	UINT32 slot_offset;
	if (g_mtz_active_count == 0)
		return -1;
	slot_offset = mtz_slot_data_addr(slot_idx);
	return mtz_read_storage(dst, MTZ_SLOT_SIZE, slot_offset);
}

/* Helper: set pointers into the reserved metazone buffer after loading
 * metazone data into physical reserved memory at g_metazone_mem_addr.
 */

static int mtz_write_slot_data_uboot(UINT32 slot_idx, void *src)
{
	UINT32 slot_offset;
	if (g_mtz_active_count == 0)
		return -1;
	slot_offset = mtz_slot_data_addr(slot_idx);
	return mtz_write_storage(src, MTZ_SLOT_SIZE, slot_offset);
}

static int mtz_write_slot_header_page_uboot(UINT32 slot_idx, const SlotHeader *header)
{
	UINT32 slot_offset;
	UINT8 page_buf[nand->writesize];
	UINT32 pagesize = nand->writesize;

	if (!header)
		return -1;

	if (g_mtz_active_count == 0)
		return -1;

	slot_offset = mtz_slot_data_addr(slot_idx);
	memset(page_buf, 0, sizeof(page_buf));
	memcpy(page_buf + (pagesize - MTZ_SLOT_HEADER_SIZE), header, sizeof(SlotHeader));
	return mtz_write_storage(page_buf, pagesize, slot_offset + MTZ_SLOT_SIZE - pagesize);
}

static int mtz_write_slot_uboot(UINT32 slot_idx, void *src)
{
	UINT32 slot_offset;
	UINT32 data_size = MTZ_SLOT_SIZE - nand->writesize;

	if (!src)
		return -1;

	if (g_mtz_active_count == 0)
		return -1;
	slot_offset = mtz_slot_data_addr(slot_idx);
	// write data region (exclude last header page)
	return mtz_write_storage(src, data_size, slot_offset);
}

static UINT32 mtz_get_partition_size(void)
{
	printf("[MTZ UBOOT] mtz_get_partition_size:0x%x\n", MTZ_PARTITION_SIZE);
	return MTZ_PARTITION_SIZE;
}

static UINT32 mtz_get_block_size(void)
{
	printf("metazone block size:0x%x\n", nand->erasesize);
	return nand->erasesize;
}

static UINT32 mtz_get_slots_per_block(void)
{
	UINT32 block_size = mtz_get_block_size();
	UINT32 slots = block_size / MTZ_SLOT_SIZE;

	return (slots == 0) ? 1 : slots;
}

/* Map logical slot index to NAND offset using selected active blocks. */
static UINT32 mtz_slot_data_addr(UINT32 slot_idx)
{
	UINT32 block_size = mtz_get_block_size();
	UINT32 slots_per_block = mtz_get_slots_per_block();
	UINT32 slot_in_block;
	UINT32 slot_block;

	if (g_mtz_active_count == 0 || slots_per_block == 0)
		return 0;

	slot_in_block = slot_idx % slots_per_block;
	if (g_mtz_active_count == 1 || slot_idx < slots_per_block)
		slot_block = g_mtz_active_blocks[0];
	else
		slot_block = g_mtz_active_blocks[1];

	return slot_block * block_size + slot_in_block * MTZ_SLOT_SIZE;
}



static UINT32 mtz_build_active_blocks(UINT32 *blocks, UINT32 max_blocks)
{
	UINT32 part_size = mtz_get_partition_size();
	UINT32 block_size = mtz_get_block_size();
	UINT32 total_blocks = part_size / block_size;
	UINT32 found = 0;
	UINT32 i;
	pr_info("[MTZ UBOOT]  total_blocks:%d, part_size:%d, block_size:%d, g_metazonePartitonAddr:0x%x\n",total_blocks, part_size, block_size, g_metazonePartitonAddr);
	for (i = 0; i < total_blocks && found < max_blocks; i++) {
		UINT32 block_offset = i * block_size + g_metazonePartitonAddr;
		if (nand_block_isbad(nand, block_offset & ~(nand->erasesize - 1))) {
			pr_info("[MTZ UBOOT] metazone block:%d is bad, block offset:0x%x\n",i, block_offset);
			continue;
		}

		blocks[found++] = i;
	}
	g_mtz_active_count = found;
	if (found > 0)
		g_mtz_active_blocks[0] = blocks[0];
	if (found > 1)
		g_mtz_active_blocks[1] = blocks[1];

	pr_info("mtz_build_active_blocks found:%d, [%d] and [%d] \n", found, blocks[0],blocks[1]);
	return found;
}

static int mtz_erase_block_for_slot(UINT32 slot_idx)
{

	UINT32 block_size = mtz_get_block_size();
	UINT32 slots_per_block = mtz_get_slots_per_block();
	UINT32 block_idx;
	UINT32 block_offset;
	char szSize[17];
	char szOffset[17];
	char *argv[6] = {"nand", "erase", "metazone", szSize, szOffset};
	int ret;
	if (g_mtz_active_count == 0 || slots_per_block == 0) {
		printf("[MTZ UBOOT] erase block failed: no active block\r\n");
		return -1;
	}

	block_idx = (g_mtz_active_count == 1 || slot_idx < slots_per_block)
		? g_mtz_active_blocks[0]
		: g_mtz_active_blocks[1];
	block_offset = block_idx * block_size;
	printf("[MTZ UBOOT] erase block  szSize=0x%x, offset=0x%x\r\n", nand->erasesize, block_offset);
	sprintf(szSize, "0x%x", MTZ_SLOT_SIZE);
	sprintf(szOffset, "0x%x", block_offset);
	ret = do_nand(NULL, 0, 5, argv);
	if (ret != 0)
		printf("[MTZ UBOOT] erase block fail, slot=%u offset=0x%x\r\n", slot_idx, block_offset);
	return ret;
}

static int mtz_find_latest_slot( UINT32 *best_idx, UINT32 *best_seq, BOOL *best_valid)
{
	UINT32 i;
	SlotHeader hdr;
	UINT32 slot_count = 0;
	UINT32 slots_per_block = mtz_get_slots_per_block();
	UINT32 active_blocks[2];
	UINT32 active_count = 0;

	active_count = mtz_build_active_blocks(active_blocks, 2);
	if (active_count == 0) {
		printf("[MTZ][UBOOT] no good block found\r\n");
		return FALSE;
	}
	if (active_count == 1)
		printf("[MTZ][UBOOT] single block mode\r\n");

	slot_count = active_count * slots_per_block;
	if (slot_count == 0 || slot_count > MTZ_MAX_SLOTS)
		slot_count = MTZ_MAX_SLOTS;

	g_mtz_slot_count = slot_count;
	g_mtz_slots_per_block = slots_per_block;
	if (!best_idx || !best_seq || !best_valid)
		return -1;

	*best_idx = 0;
	*best_seq = 0;
	*best_valid = FALSE;

	for (i = 0; i < slot_count; i++) {
		if (mtz_read_slot_header_uboot(i, &hdr) == 0 && hdr.magic == METAZONE_SLOT_MAGIC && hdr.state == 0xFF) {
			if (!*best_valid || mtz_seq_after(hdr.seq, *best_seq)) {
				g_mtz_valid[i] = 1; // mark as valid
				g_mtz_seqs[i] = hdr.seq;
				*best_idx = i;
				*best_seq = hdr.seq;
				*best_valid = TRUE;
			}
		} else {
				g_mtz_valid[i] = 0;
				g_mtz_seqs[i] = 0;
		}
	}
	
	g_mtz_current_slot = best_valid ? *best_idx : 0;
	g_mtz_current_seq = best_valid ? *best_seq : 0;
	return 0;
}

/*
 * Try to load previous slots' data in descending sequence order.
 * slot_count: number of slots to consider (typically g_mtz_slot_count)
 * best_idx/best_seq: in/out parameters. On entry these should contain the
 *    index and sequence of the newest slot to try first. The function will
 *    iterate older candidates until a slot with valid CRCs is found.
 * Returns TRUE if a valid slot was loaded into reserved memory, FALSE otherwise.
 */
static BOOL mtz_load_prev_slots(UINT32 slot_count, UINT32 *best_idx, UINT32 *best_seq)
{
	UINT32 i, j;
	BOOL found_good = FALSE;

	if (!best_idx || !best_seq || slot_count == 0)
		return FALSE;

	for (i = 0; i < slot_count; i++) {
		/* try this candidate (best_idx/best_seq) */
		if (g_mtz_valid[*best_idx]) {
			if (mtz_read_slot_data_uboot(*best_idx, (void *)g_metazone_mem_addr) == 0) {
				if (mtz_check_all_crcs()) {
					printf("[MTZ][UBOOT] loaded valid previous slot %u seq %u\r\n", *best_idx, *best_seq);
					found_good = TRUE;
					g_mtz_valid[*best_idx] = 0;
					break;
				}
			}
		}

		/* find next candidate: highest seq among remaining valid[] entries */
		{
			BOOL any = FALSE;
			UINT32 best_local_seq = 0;
			UINT32 best_local_idx = 0;
			for (j = 0; j < slot_count; j++) {
				if (!g_mtz_valid[j])
					continue;
				if (!any || mtz_seq_after(g_mtz_seqs[j], best_local_seq)) {
					best_local_seq = g_mtz_seqs[j];
					best_local_idx = j;
					any = TRUE;
				}
			}
			if (!any)
				break; /* no more candidates */
			*best_idx = best_local_idx;
			*best_seq = best_local_seq;
		}
	}

	return found_good;
}
#endif
u32 metazone_read(u32 u4Idx, u32 *pu4Data)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			*pu4Data = _pu4Value[u4Idx];
			return MZ_SUCCESS;
		}
		u4Idx += MZ_WR_IDX_START;
	}
	return MZ_FAILURE;
}

u32 metazone_readbinary(u32 u4Idx, char *pbData, u32 u4Size)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			u32 u4Tmp =
			    *(u32 *) (_u4BinaryStart +
				      (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx);
			if (u4Size > u4Tmp)
				u4Size = u4Tmp;
			memcpy(pbData,
			       (char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), u4Size);
			return u4Size;
		}
	}
	return MZ_FAILURE;
}


u32 metazone_readreserved(char *pbData, u32 u4Size)
{
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;
	memcpy(pbData, _pbReserve, u4Size);

	return u4Size;
}

u32 metazone_write(u32 u4Idx, u32 u4Data)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			_pu4Value[u4Idx] = u4Data;
#ifndef CONFIG_BOOT_MMC
			recount_CRC_value(1);
#endif
			return MZ_SUCCESS;
		}
	}

	return MZ_FAILURE;
}

u32 metazone_writebinary(u32 u4Idx, char *pbData, u32 u4Size)
{
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			if (u4Size > _pMetaHeader->dwBinaryItemSize)
				u4Size = _pMetaHeader->dwBinaryItemSize;
			*(u32 *) (_u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx) = u4Size;
			memcpy((char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), pbData,u4Size);
#ifndef CONFIG_BOOT_MMC
			recount_CRC_value(2);
#endif
			return MZ_SUCCESS;
		}
	}
	return MZ_FAILURE;
}


u32 metaZone_writereserved(char *pbData, u32 u4Size)
{
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;
	memcpy(_pbReserve, pbData, u4Size);
#ifndef CONFIG_BOOT_MMC
	recount_CRC_value(3);
#endif
	return MZ_SUCCESS;
}


#ifndef CONFIG_BOOT_MMC
BOOL metazone_init(UINT32 mode)
{

	RSV_MEM_T *metazone_rsv = NULL;
	nand = &nand_info[0];
	metazone_rsv = get_rsv_mem_by_name("metazone");
	if (NULL == metazone_rsv) {
		printf("Metazone_rsv get failed ,Please check\r\n");
		return FALSE;
	}
	g_metazone_mem_addr = metazone_rsv->start_addr;
	if (mode == 0) {//for sdupgrade
		printf("Metazone enter upgrade mode\r\n");
		mtz_set_mem_ptrs();
		_fgWritableZoneInited = TRUE;
		return TRUE;
	} else if (mode == 1) { //normal boot
		UINT32 best_idx = 0;
		UINT32 best_seq = 0;
		BOOL best_valid = FALSE;
		mtz_find_latest_slot(&best_idx, &best_seq, &best_valid);
		/* If no valid header at all, fallback to slot 0 */
		if (best_valid) {
		/* Try newest first, then iterate older slots in descending seq order */
			BOOL found_good = FALSE;
			if (g_mtz_valid[best_idx]) {
				if ((mtz_read_slot_data_uboot(best_idx, (void *)g_metazone_mem_addr) == 0) && mtz_check_all_crcs()) {
					found_good = TRUE;
					g_mtz_valid[best_idx] = 0;
					printf("[MTZ][UBOOT] Active slot %u seq %u\r\n", best_idx, best_seq);
				} else {
					printf("[MTZ][UBOOT] newest slot %u seq %u failed CRC check\r\n", best_idx, best_seq);
					g_mtz_valid[best_idx] = 0; // mark as used even if CRC failed, to avoid retrying same bad slot
				}
			}

			/* If newest failed, try previous slots (helper will load into reserve on success) */
			if (!found_good) {
				if (!mtz_load_prev_slots(g_mtz_slot_count, &best_idx, &best_seq)) {
					/* fallback to slot 0 and trigger recount */
					printf("[MTZ][UBOOT] no previous valid slot found, fallback slot 0\r\n");
					if (mtz_read_slot_data_uboot(0, (void *)g_metazone_mem_addr) != 0) {
						printf("[MTZ][UBOOT] read slot 0 failed\r\n");
						return FALSE;
					}
					best_valid = FALSE;
				}
			}
		} else {
			printf("[MTZ][UBOOT] first boot, fallback slot 0\r\n");
			if (mtz_read_slot_data_uboot(0, (void *)g_metazone_mem_addr) != 0) {
				printf("[MTZ][UBOOT] read slot 0 failed\r\n");
				return FALSE;
			}
			mtz_set_mem_ptrs();
			recount_header_crc(_pMetaHeader);
			recount_CRC_value(1);
			recount_CRC_value(2);
			recount_CRC_value(3);
		}
		mtz_set_mem_ptrs();
		if (_pMetaHeader->dwSignature != METAZONE_SIGNATURE ||
			_pMetaHeader->dwVersion != METAZONE_VERSION ||
			_pMetaHeader->dwDataSize > METAZONE_SIZE_MAX) {
			printf("[MTZ][UBOOT] metazone header invalid, signature=0x%x version=0x%x size=0x%x\r\n",
				_pMetaHeader->dwSignature, _pMetaHeader->dwVersion, _pMetaHeader->dwDataSize);
			return FALSE;
		}
		g_mtz_slot_ready = TRUE;
		printf("[MTZ] WritableZone Infomation:\n");
		printf("[MTZ] \tDataSize = 0x%x, Binary Item Max Size = %d\n",
			_pMetaHeader->dwDataSize, _pMetaHeader->dwBinaryItemSize);
		printf("[MTZ] \tDWORD Value Number = %d, offset = 0x%x\n", _pMetaHeader->dwValueNum,
			_pMetaHeader->dwValueOffset);
		printf("[MTZ] \tBinary Value Number = %d, offset = 0x%x\n",
			_pMetaHeader->dwBinaryNum, _pMetaHeader->dwBinaryOffset);
		printf("[MTZ] MTZ_InitWritableZone End\n");

	_fgWritableZoneInited = TRUE;
	}

	return TRUE;

}

UINT32 metazone_flush(BOOL fgSync)
{
	UINT32 slot_count = 0;
	UINT32 slots_per_block = 0;
	UINT32 next_idx = 0;
	SlotHeader *local_header;
	int ret;

	printf("[MTZ UBOOT] Flush Sync(%d).\r\n", fgSync);
	if (_pMetaZone == NULL)
		return 0;

	slot_count = g_mtz_slot_count;
	slots_per_block = g_mtz_slots_per_block;
	if (slot_count == 0 || slots_per_block == 0) {
		printf("[MTZ UBOOT] Flush: slot count not initialized\r\n");
		return 0;
	}

	if (!g_mtz_slot_ready)
		printf("[MTZ UBOOT] Flush: slot not initialized, fallback to slot 0\r\n");

	next_idx = (g_mtz_current_slot + 1) % slot_count;
	// printf("[MTZ UBOOT] Flush:g_mtz_current_slot:%d, next_idx:%d\r\n", g_mtz_current_slot, next_idx);
	local_header = mtz_get_slot_header_rsv_addr();
	if (!local_header) {
		printf("[MTZ UBOOT] Flush: slot header is NULL\r\n");
		return 0;
	}
	local_header->magic = METAZONE_SLOT_MAGIC;
	local_header->seq = g_mtz_current_seq + 1;
	local_header->state = 0x01;

	if ((next_idx % slots_per_block) == 0) {
		printf("[MTZ UBOOT] Flush: erase block for slot %u\r\n", next_idx);
		if (mtz_erase_block_for_slot(next_idx) != 0) {
			printf("[MTZ UBOOT] Flush: erase block failed\r\n");
			return 0;
		}
	}

	printf("[MTZ UBOOT] Flush: write slot %u seq %u\r\n", next_idx, local_header->seq);
	ret = mtz_write_slot_uboot(next_idx, _pMetaZone);
	if (ret != 0) {
		printf("[MTZ UBOOT] Flush: write slot %u failed\r\n", next_idx);
		return 0;
	}

	local_header->state = 0xFF;
	if (mtz_write_slot_header_page_uboot(next_idx, local_header) != 0) {
		printf("[MTZ UBOOT] Flush: update header for slot %u failed\r\n", next_idx);
		return 0;
	}

	g_mtz_current_slot = next_idx;
	g_mtz_current_seq = local_header->seq;
	g_mtz_slot_ready = TRUE;

	return 0;
}

#else
#define MTZ_PARTITION_SIZE  (0x20000)  /*256KB*/
Wflag *p1;
Wflag *p2;
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
int Metazone_Wflag_Compare()
{
	printf("Metazone_Wflag_Compare enter\n");

	int mtz1ret;
	int mtz2ret;
	int flag1,flag2;
	int ret=BACKUP_NONE;
	unsigned int w_b1,w_a1,w_b2,w_a2;
	char W_flag1[17];
	char W_flag2[17];
	char buff1_addr[17];
	char buff2_addr[17];
	char buff1=NULL;
	char buff2=NULL;
	Wflag *p1=NULL;
	Wflag *p2=NULL;
	p1=(Wflag *)malloc(sizeof(Wflag));
	p2=(Wflag *)malloc(sizeof(Wflag));
	if(!p1||!p2){
		printf("malloc wflag fail \n");
		return -1;
	}

	buff1=(char *)malloc(256);
	if(!buff1){
		printf("malloc buff1 fail \n");
		return -1;
		}
	buff2=(char *)malloc(256);
	if(!buff2){
		printf("malloc buff1 fail \n");
		return -1;
		}
	memset(W_flag1,0,17);
	memset(W_flag2,0,17);
	memset(buff1_addr,0,17);
	memset(buff2_addr,0,17);
	memset(buff1,0,128);
	memset(buff2,0,128);
	uitostr_hex(W_flag1,(unsigned int)((g_metazonePartitonAddr+ MTZ_PARTITION_SIZE/2-0x200)/512));
	uitostr_hex(W_flag2,(unsigned int)((g_metazonePartitonAddr+ MTZ_PARTITION_SIZE-0x200)/512));
	uitostr_hex(buff1_addr,(unsigned int)(buff1));
	uitostr_hex(buff2_addr,(unsigned int)(buff2));

#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	char *argv1[]= {"mmc","read","0",buff1_addr,W_flag1,"0x1"};
	char *argv2[]={"mmc","read","0",buff2_addr,W_flag2,"0x1"};
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	char *argv1[]= {"mmc","read","2",buff1_addr,W_flag1,"0x1"};
	char *argv2[]={"mmc","read","2",buff2_addr,W_flag1,"0x1"};
#endif

	mtz1ret = do_mmcops(NULL, 0, 6, argv1);
	mtz2ret = do_mmcops(NULL, 0, 6, argv2);
	if(mtz1ret||mtz2ret){
		printf("do_mmcops fail mtz1%d mtz2%d \n",mtz1ret,mtz2ret);
		return -1;
	}
	memcpy(p1,buff1,sizeof(Wflag));
	memcpy(p2,buff2,sizeof(Wflag));
	w_a1=p1->write_after;
	w_b1=p1->write_before;
	w_a2=p2->write_after;
	w_b2=p2->write_before;
	printf("w_b1 (%d) w_a1 (%d) w_b2 (%d) w_a2 (%d) \n",w_b1,w_a1,w_b2,w_a2);
	if(w_b1 != w_a1){
		printf("metazone1 broken \n");
		flag1=0;
	} else {
		printf("metazone1 OK \n");
		flag1=1;
	}

	if (w_b2 != w_a2){
		printf("metazone2 broken \n");
		flag2=0;
	} else {
		printf("metazone2 OK \n");
		flag2=1;
	}

	if((flag1==1)&&((flag2==1))){
			if(w_a1==w_a2)
				ret=BACKUP_NONE;
			else if(w_a1>w_a2)
				ret=BACKUP_MTZ2;
			else if(w_a1<w_a2){
				printf("a critical error (1,1)\n");
				ret=BACKUP_MTZ1;
			}
	} else if((flag1==1)&&((flag2==0))){
		ret=BACKUP_MTZ2;
	} else if((flag1==0)&&((flag2==1))){
		ret=BACKUP_MTZ1;
	} else if((flag1==0)&&((flag2==0))){
		printf("a critical error (0,0)");
		ret=-1;
	}

	if((ret!=BACKUP_NONE)||(w_a1>0xFFFFFFF0)){
	/*clean the wflag*/
		memset(buff1,0,256);
		memset(buff2,0,256);
		uitostr_hex(buff1_addr,(unsigned int)(buff1));
		uitostr_hex(buff2_addr,(unsigned int)(buff2));

	#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
		char *argv3[]= {"mmc","write","0",buff1_addr,W_flag1,"0x1"};
		char *argv4[]= {"mmc","write","0",buff2_addr,W_flag2,"0x1"};
	#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
		char *argv3[]= {"mmc","write","2",buff1_addr,W_flag1,"0x1"};
		char *argv4[]= {"mmc","write","2",buff2_addr,W_flag2,"0x1"};
	#endif

		printf("before do_mmcops and consume time is %ums\n", boot_time_ms());
		mtz1ret = do_mmcops(NULL, 0, 6, argv3);
		mtz2ret = do_mmcops(NULL, 0, 6, argv4);
		printf("after do_mmcops and consume time is %ums\n", boot_time_ms());
		if(mtz1ret||mtz2ret){
			printf("clean wflag fail %d %d\n",mtz1ret,mtz2ret);
		}
		printf("Metazone_Wflag_Compare leave and consume time is %ums\n", boot_time_ms());
	}
		return ret;
	return 0;

}

BOOL metazone_init(UINT32 mode)
{

	int ret;
	char *szValAddr1 = (char *)malloc(17);
	char *szValAddr2 = (char *)malloc(17);
	RSV_MEM_T *metazone_rsv = NULL;

	memset(szValAddr1,0,17);
	memset(szValAddr2,0,17);
	uitostr_hex(szValAddr1,(unsigned int)(g_metazonePartitonAddr/512));
	uitostr_hex(szValAddr2,(unsigned int)((g_metazonePartitonAddr+ MTZ_PARTITION_SIZE/2)/512));
	printf("Metazone_Read szValAddr1=%s, szValAddr2=%s\r\n",szValAddr1, szValAddr2);

	metazone_rsv = get_rsv_mem_by_name("metazone");
	if (NULL == metazone_rsv) {
		 printf("Metazone_rsv get failed ,Please check\r\n");
		 return FALSE;
	}
	g_metazone_mem_addr = metazone_rsv->start_addr;
	printf("current system mode :%d\r\n", _sdagentflag);
	if (_sdagentflag == 1) {
		printf("Metazone enter upgrade mode\r\n");
		_pMetaZone = (void *)MEMRSV_PHY_TO_VIRT((u32)g_metazone_mem_addr);
		_pMetaHeader = (TMetaZone *)_pMetaZone;
		_pbReserve = (BYTE *)((UINT32) _pMetaHeader + sizeof(TMetaZone));
		_pu4Value = (UINT32 *)((UINT32) _pMetaHeader + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (UINT32) _pMetaHeader + _pMetaHeader->dwBinaryOffset;
		_fgWritableZoneInited = TRUE;
		free(szValAddr1);
		free(szValAddr2);
		return TRUE;
	}
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	char *argv1[]= {"mmc","read","0",g_metazone_mem_addr,szValAddr1,"0x80"};
	char *argv2[]= {"mmc","read","0",g_metazone_mem_addr + 0x10000,szValAddr2,"0x80"};
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	char *argv1[]= {"mmc","read","2",g_metazone_mem_addr,szValAddr1,"0x80"};
	char *argv2[]={"mmc","read","2",g_metazone_mem_addr + 0x10000,szValAddr2,"0x80"};
#endif

	int mtz1ret;
	int mtz2ret;
	mtz1ret = do_mmcops(NULL, 0, 6, argv1);
	mtz2ret = do_mmcops(NULL, 0, 6, argv2);
	if ( (0 != mtz1ret) && (0 != mtz2ret))
	{
		printf("ERROR: Unable to read metazone\r\n");
		return FALSE;
	}
	BYTE  *pbMtz = (BYTE *)MEMRSV_PHY_TO_VIRT((u32)g_metazone_mem_addr);
	_pMetaZone = (void *)pbMtz;
	/*default use metazone in block one*/
	ret=Metazone_Wflag_Compare();
	if(ret==BACKUP_MTZ1){
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
		char *argv4[]= {"mmc","write","0",g_metazone_mem_addr,szValAddr1,"0x80"};
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
		char *argv4[]= {"mmc","write","2",g_metazone_mem_addr,szValAddr1,"0x80"};
#endif
		// Write metazone to block one.
		printf("[MTZ UBOOT] Write Metazone data to block one\r\n");
		mtz2ret = do_mmcops(NULL, 0, 6, argv4);
		if (0 != mtz2ret)
		{
			printf("[MTZ UBOOT] Failed to write Metazone data to block one\r\n");
		}
	}
	if(ret==BACKUP_MTZ2){

#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
		char *argv3[]= {"mmc","write","0",g_metazone_mem_addr,szValAddr2,"0x80"};
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
		char *argv3[]= {"mmc","write","2",g_metazone_mem_addr,szValAddr2,"0x80"};
#endif
		printf("[MTZ UBOOT] Write Metazone data to block two\r\n");
		// Write metazone to block two.
		mtz2ret = do_mmcops(NULL, 0, 6, argv3);
		if (0 != mtz2ret)
		{
			printf("[MTZ UBOOT] Failed to write Metazone data to block two\r\n");
		}
	}

	_pMetaHeader = (TMetaZone *)_pMetaZone;
	_pbReserve = (BYTE *)((UINT32) _pMetaZone + sizeof(TMetaZone));
	_pu4Value = (UINT32 *)((UINT32) _pMetaZone + _pMetaHeader->dwValueOffset);
	_u4BinaryStart = (UINT32) _pMetaZone + _pMetaHeader->dwBinaryOffset;

	_fgWritableZoneInited = TRUE;
	free(szValAddr1);
	free(szValAddr2);
	return TRUE;

}

UINT32 metazone_flush(BOOL fgSync)
{
	char *szValAddr1 = (char *)malloc(17);
	char *szValAddr2 = (char *)malloc(17);
	char *szMtzddr = (char *)malloc(17);
	int ret;

	printf("[MTZ UBOOT] Flush Sync(%d).\r\n", fgSync);

	memset(szValAddr1,0,17);
	memset(szValAddr2,0,17);
	memset(szMtzddr,0,17);

	uitostr_hex(szValAddr1,(unsigned int)(g_metazonePartitonAddr/512));
	uitostr_hex(szValAddr2,(unsigned int)((g_metazonePartitonAddr+ MTZ_PARTITION_SIZE/2)/512));
	uitostr_hex(szMtzddr,(unsigned int)_pMetaZone);

	printf("Metazone szValAddr1=%s\r\n", szValAddr1);
	printf("Metazone szValAddr2=%s\r\n", szValAddr2);
	printf("Metazone szMtzddr=%s\r\n",   szMtzddr);


#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	char *argv4[]= {"mmc","write","0",szMtzddr, szValAddr1,"0x80"};
	char *argv3[]= {"mmc","write","0",szMtzddr, szValAddr2,"0x80"};
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	char *argv4[]= {"mmc","write","2",szMtzddr, szValAddr1,"0x80"};
	char *argv3[]= {"mmc","write","2",szMtzddr, szValAddr2,"0x80"};
#endif

	// Write metazone to block one.
	printf("[MTZ UBOOT] Write Metazone data to block one\r\n");
	ret = do_mmcops(NULL, 0, 6, argv4);
	if (0 != ret)
	{
		printf("[MTZ UBOOT] Failed to write Metazone data to block one\r\n");
	}

	// Write metazone to block two.
	printf("[MTZ UBOOT] Write Metazone data to block two\r\n");
	ret = do_mmcops(NULL, 0, 6, argv3);
	if (0 != ret)
	{
		printf("[MTZ UBOOT] Failed to write Metazone data to block two\r\n");
	}
	free(szValAddr1);
	free(szValAddr2);
	free(szMtzddr);
	return (0);
}
#endif
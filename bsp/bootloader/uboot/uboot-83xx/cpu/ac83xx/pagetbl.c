//
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Copyright (c) MediaTek Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

#include "mmu.h"
#include "configs/ac83xx_evb.h"

extern volatile unsigned int _sdagentflag;

/**********************************************************************

Memory mapping of ARM2

00000000 - 00100000   -   -          0M -      1M        1M    DRAM-B not allowed to access
00100000 - 3FFFFFFF     C   B         1M - 1024M   1023M    DRAM-B
40000000 - 403FFFFF     -   -      1024M - 1028M      4M    - (IO)
40400000 - 4FFFFFFF     -   -      1028M - 1280M    252M    -
50000000 - 503FFFFF     -   -      1280M - 1284M      4M    - (IO Extension)
50400000 - 5FFFFFFF     -   -      1284M - 1536M    252M    -
60000000 - 60FFFFFF     -   -      1536M - 1552M     16M    PBI-A (Flash)
61000000 - 6FFFFFFF     -   -      1552M - 1792M    240M    -
70000000 - 703FFFFF     -   -      1792M - 1796M      4M    IO
70400000 - 7FFFFFFF     -   -      1796M - 2048M    252M    -
80000000 - 80FFFFFF     C   B      2048M - 2064M     16M    PBI-B (Flash)
81000000 - 8FFFFFFF     -   -      2064M - 2304M    240M    -
90000000 - BFFFFFFF     -   -      2304M - 3072M    768M    -
C0000000 - FFFFFFFF     -   -      3072M - 4096M   1024M    DRAM-A

**********************************************************************/

/*----------------------------------------------------------------------------
 * CreatePageTable() Create page table
 *  @param pagetable[in] - Address of page table, shall be aligned to 16K boundary
 *---------------------------------------------------------------------------*/

extern unsigned int _start;
extern unsigned int _end;
#define ALIGN_UP(addr, align)       (((addr) + (align) - 1) & ~((align) - 1))

void CreatePageTable(unsigned int* pagetable)
{
	unsigned int i;//, ca, pa;
	unsigned int start, end;

	// Note
	// 1. Unused entries must be reserved.
	// 2. All accessible regions are set to domain 0

	memset( pagetable, 0, sizeof(unsigned int) * 4096 );

	// Map vector table
	pagetable[0] = SECTION_DESC(
		0 << 20,
		EXECUTABLE,
		APX_NO_LIMIT,
		AP_USR_NO_LIMIT,
		TEX_STD,
		NOCACHE_ORDERED,
		0,
		0,
		0,
		NONSEC,
		DOMAIN00);

	/* Map arm2 area: should be released after arm2 bootup */
	start = CFG_ARM2_RESERVED_ADDR >> 20;
	end = (CFG_ARM2_RESERVED_ADDR + CFG_ARM2_RESERVED_SIZE) >> 20;
	for(i = start; i < end; i++) {
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			CACHE_WRITEBACK,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	/* Map rsv info area */
	pagetable[CFG_ARGS_RESERVED_ADDR >> 20] = SECTION_DESC(
		CFG_ARGS_RESERVED_ADDR,
		EXECUTABLE,
		APX_NO_LIMIT,
		AP_USR_NO_LIMIT,
		TEX_STD,
		CACHE_WRITEBACK,
		0,
		0,
		0,
		NONSEC,
		DOMAIN00);

	/* Map mapping table itself */
	start = (unsigned int)pagetable >> 20;
	pagetable[start] = SECTION_DESC(
		(unsigned int)pagetable,
		EXECUTABLE,
		APX_NO_LIMIT,
		AP_USR_NO_LIMIT,
		TEX_STD,
		CACHE_WRITEBACK,
		0,
		0,
		0,
		NONSEC,
		DOMAIN00);

	/* Map: upgrade temp area(BASE_ADDR/NAND_WRITE_BASE_ADDR)
	 * uboot/uboot-83xx/include/configs/ac83xx_evb.h
	 *      BASE_ADDR / NAND_WRITE_BASE_ADDR: 0x0110_0000 ~ 0x0190_0000
	 * uboot/uboot-83xx/include/asm-arm/arch-ac83xx/ac83xx_upg_ddr_layout.h
	 *      ATC_UPG_BASE_ADDR (COM/EXT/OOB) : 0x0190_0000 ~ 0x01C0_0000
	 */
	for(i = 0x11; i < 0x1C; i++)
	{
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			CACHE_WRITEBACK,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	/* Map: Uboot runtime area(Stack + Heap + Code)
	 * +--------------+-----------------------+------------+
	 * |     Stack    |          Heap         |     Code   |
	 * +--------------+-----------------------+------------+
	 *  Max 1M default  CONFIG_SYS_MALLOC_LEN _start    _end
	 *
	 */
	start = ((unsigned int)&_start) >> 20;
	start -= (8 + 1);
	end = ALIGN_UP(((unsigned int)&_end), 0x100000) >> 20;
	for(i = start; i < end; i++)
	{
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			CACHE_WRITEBACK,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	/* Map framebuffer: show upgrade process */
	start = CFG_FRAMEBUFFER_RESERVED_ADDR >> 20;
	end = (CFG_FRAMEBUFFER_RESERVED_ADDR + CFG_FRAMEBUFFER_RESERVED_SIZE) >> 20;
	for(i = start; i < end; i++)
	{
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			CACHE_WRITEBACK,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	/* Map animation show upgrade process */
	start = CFG_ANIMATION_RESERVED_ADDR >> 20;
	end = (CFG_ANIMATION_RESERVED_ADDR + CFG_ANIMATION_RESERVED_SIZE) >> 20;
	for(i = start; i < end; i++)
	{
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			CACHE_WRITEBACK,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	if (_sdagentflag == 0) {
		/* If not sd upragde mode, enable DTB region: MAX DTB SIZE is 2MB */
		start = FDT_LOAD_ADDR >> 20;
		end = start + 2;
		for(i = start; i < end; i++)
		{
			pagetable[i] = SECTION_DESC(
				i << 20,
				EXECUTABLE,
				APX_NO_LIMIT,
				AP_USR_NO_LIMIT,
				TEX_STD,
				CACHE_WRITEBACK,
				0,
				0,
				0,
				NONSEC,
				DOMAIN00);
		}
	}

	// Map the non-cachable & non-bufferable memory space 0xF000 0000 -- F03FFFFF  4M IO
	for(i = 0xf00; i < 0xf04; i++)  //0x700 - 0x704
	{
		pagetable[i] = SECTION_DESC(
			i << 20,
			EXECUTABLE,
			APX_NO_LIMIT,
			AP_USR_NO_LIMIT,
			TEX_STD,
			NOCACHE_ORDERED,
			0,
			0,
			0,
			NONSEC,
			DOMAIN00);
	}

	return;
}

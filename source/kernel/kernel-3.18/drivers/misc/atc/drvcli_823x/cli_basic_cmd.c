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

#include <linux/mm.h>

#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "base_regs.h"
//#include "mach/ac83xx_memory.h"

#include <linux/types.h>
#if 1

#define MIN_MEM_ADDR 0x100000
#define MAX_MEM_ADDR 0x20000000
#define MAX_IO_ADDR 0x400000

/* IO by Lingling */
#define CLIMEMRSV_PHY_TO_VIRT(x)       (IO_VIRT + (x) - IO_BASE)
/* reserved memory for OSE by Lingling */
//#define CLIMEMOCAL_PHYTOVIRT(x)        MEMRSV_PHY_TO_VIRT(x)
#define CLIMEMOCAL_PHYTOVIRT(x)        CLIMEMRSV_PHY_TO_VIRT(x)    //now we only support IO, not support reserve memory
static void _MemoryRead(u32 u4Addr, s32 i4Argc, const s8 **szArgv)
{
	u32 u4Size = 4;
	u32 u4Mode = 32;
	u32 u4Loop;

	if (i4Argc >= 1)
		u4Size = StrToInt(szArgv[0]);

	if (i4Argc >= 2)
		u4Mode = StrToInt(szArgv[1]);

	switch (u4Mode) {
	case 32:
		{
			ulong u4VirAddr;

			u4Size &= 0xFFFFFFFC;
			u4Addr &= 0xFFFFFFFC;
			u4VirAddr = u4Addr + CLI_IO_BASE;//u4Addr; //CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Read  %d DWORDs from address 0x%lx\n"),
					u4Size >> 2, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 4) {
				if (!(u4Loop % 16))
					pr_err(TEXT("0x%08lx:\n"), u4Addr + u4Loop);

				pr_err(TEXT("  0X%08lx\n"), readl((u4VirAddr + u4Loop)));
			}
			//pr_err(TEXT("\n"));
		}
		break;
	case 16:
		{
			ulong u4VirAddr;

			u4Size &= 0xFFFFFFFE;
			u4Addr &= 0xFFFFFFFE;
			u4VirAddr = u4Addr + CLI_IO_BASE; //CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Read  %d DWORDs from address 0x%lx\n"),
					u4Size >> 1, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 2) {
				if (!(u4Loop % 16))
					pr_err(TEXT("0x%08lx:\n"), u4Addr + u4Loop);
				pr_err(TEXT("  0X%04lx\n"), readw((const volatile void *)(u4VirAddr + u4Loop)));
			}
			//pr_err(TEXT("\r\n"));
		}
		break;
	default:
		{
			ulong u4VirAddr;

			u4VirAddr = u4Addr + CLI_IO_BASE; //CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Read  %d BYTEs from address 0x%lx\n"),
					u4Size, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop++) {
				if (!(u4Loop % 16))
					pr_err(TEXT("0x%08lx:\n"), u4Addr + u4Loop);
				pr_err(TEXT("  0X%02lx:\n"), readb((const volatile void *)(u4VirAddr + u4Loop)));
			}
			//pr_err(TEXT("\r\n"));
		}
		break;
	}
}
static void _CMemoryRead(u32 u4Addr, s32 i4Argc, const s8 **szArgv)
{
	u32 u4Size = 4;
	u32 u4Mode = 32;
	u32 u4Loop;

	if (i4Argc >= 1)
		u4Size = StrToInt(szArgv[0]);

	if (i4Argc >= 2)
		u4Mode = StrToInt(szArgv[1]);

	switch (u4Mode) {
	case 32:
		{
			u32 u4VirAddr;

			u4Size &= 0xFFFFFFFC;
			u4Addr &= 0xFFFFFFFC;
			u4VirAddr = CLIMEMOCAL_PHYTOVIRT(u4Addr);
			pr_err(TEXT("\r\n Lingling1 Read  %d DWORDs from address 0x%x"), u4Size >> 2, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 4) {
				if (!(u4Loop % 16))
					pr_err(TEXT("\r\n 0x%08x: "), u4Addr + u4Loop);
				pr_err(TEXT("\r\n 0x%08x: "), __raw_readl((const volatile void *)(u4VirAddr + u4Loop)));
			}
			pr_err(TEXT("\r\n"));
		}
		break;
	case 16:
		{
			u32 u4VirAddr;

			u4Size &= 0xFFFFFFFE;
			u4Addr &= 0xFFFFFFFE;
			u4VirAddr = CLIMEMOCAL_PHYTOVIRT(u4Addr);
			pr_err(TEXT("\r\n Lingling2 Read  %d DWORDs from address 0x%x"), u4Size >> 1, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 2) {
				if (!(u4Loop % 16))
					pr_err(TEXT("\r\n 0x%08x: "), u4Addr + u4Loop);
				pr_err(TEXT("\r\n 0x%04x: "), __raw_readw((const volatile void *)(u4VirAddr + u4Loop)));
			}

			pr_err(TEXT("\r\n"));

		}
		break;
	default:
		{
			u32 u4VirAddr;

			u4VirAddr = CLIMEMOCAL_PHYTOVIRT(u4Addr);
			pr_err(TEXT("\r\n Lingling 3 Read  %d BYTEs from address 0x%x\r\n ") , u4Size, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop++) {
				if (!(u4Loop % 16))
					pr_err(TEXT("\r\n 0x%08x: "), u4Addr + u4Loop);
				pr_err(TEXT("\r\n 0x%02x: "), __raw_readb((const volatile void *)(u4VirAddr + u4Loop)));
			}
			pr_err(TEXT("\r\n"));
		}
		break;
	}
}

static void _MemoryWrite(u32 u4Addr, s32 i4Argc, const s8 **szArgv)
{
	u32 u4Mode = 32;
	u32 u4Size = 4;
	u32 u4Value;
	u32 u4Loop;

	if (i4Argc >= 2) {
		u4Mode = StrToInt(szArgv[0]);
		i4Argc--;
		szArgv++;
	}
	u4Size = i4Argc * u4Mode / 8;
	switch (u4Mode) {
	case 32:
		{
			ulong u4VirAddr;

			u4Size &= 0xFFFFFFFC;
			u4Addr &= 0xFFFFFFFC;
			u4VirAddr = u4Addr + CLI_IO_BASE;//CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Write %d DWORDs from address 0x%lx\n"), u4Size >> 2, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 4, szArgv++) {
				u4Value = StrToInt(*szArgv);
				writel(u4Value, (u4VirAddr + u4Loop));
				//*(u32 *)(u4VirAddr + u4Loop) = u4Value;
			}
		}
		break;
	case 16:
		{
			ulong u4VirAddr;

			u4Size &= 0xFFFFFFFE;
			u4Addr &= 0xFFFFFFFE;
			u4VirAddr = u4Addr + CLI_IO_BASE; //CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Write %d DWORDs from address 0x%x\n"), u4Size >> 1, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop += 2, szArgv++) {
				u4Value = StrToInt(*szArgv);
				writew(u4Value, (u4VirAddr + u4Loop));
				//*(UINT16 *)(u4VirAddr + u4Loop) = u4Value;
			}
		}
		break;
	default:
		{
			ulong u4VirAddr;

			u4VirAddr = u4Addr + CLI_IO_BASE; //CLIMEMRSV_PHY_TO_VIRT(u4Addr);
			pr_err(TEXT("Write  %d BYTEs from address 0x%lx\n"), u4Size, u4Addr);
			for (u4Loop = 0; u4Loop < u4Size; u4Loop ++, szArgv++) {
				u4Value = StrToInt(*szArgv);
				//*(UINT8 *)(u4VirAddr + u4Loop) = u4Value;
				writeb(u4Value, (u4VirAddr + u4Loop));
			}
		}
		break;
	}
}

static s32 _CLI_MemRead(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Addr = 0;

	if (i4Argc < 2)
		return (-1);

	u4Addr = StrToInt(szArgv[1]);
	if ((u4Addr < MIN_MEM_ADDR) || (u4Addr >= MAX_MEM_ADDR))
		return -1;

	_CMemoryRead(u4Addr, i4Argc - 2, szArgv + 2);

	return 0;
}

static s32 _CLI_MemWrite(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Addr = 0;

	if (i4Argc < 3)
		return -1;

	u4Addr = StrToInt(szArgv[1]);
	if ((u4Addr < MIN_MEM_ADDR) || (u4Addr >= MAX_MEM_ADDR))
		return -1;

	_MemoryWrite(u4Addr, i4Argc - 2, szArgv + 2);

	return 0;
}

static s32 _CLI_IOR(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Addr = 0;

	if (i4Argc < 2)
		return -1;

	u4Addr = StrToInt(szArgv[1]);

	if (u4Addr >= MAX_IO_ADDR)
		return -1;
	//u4Addr += CLI_IO_BASE;

	_MemoryRead(u4Addr, i4Argc - 2, szArgv + 2);
	return 0;
}

static s32 _CLI_IOW(s32 i4Argc, const s8 **szArgv)
{
	u32 u4Addr = 0;

	if (i4Argc < 3)
		return -1;

	u4Addr = StrToInt(szArgv[1]);
	if (u4Addr >= MAX_IO_ADDR)
		return -1;
	//u4Addr += IO_BASE;

	_MemoryWrite(u4Addr, i4Argc - 2, szArgv + 2);
	return 0;
}

static s32 _CLI_MemFill(s32 i4Argc, const s8 **szArgv)
{
	s32 i;

	pr_err(TEXT("\r\n No support memory fill\r\n"));
	for (i = 0; i < i4Argc ; i++)
		pr_err(TEXT("%s "), szArgv[i]);
	pr_err(TEXT("\r\n"));
	return 0;
}

static s32 _CLI_MemCopy(s32 i4Argc, const s8 **szArgv)
{
	s32 i;

	pr_err(TEXT("\r\n No support memory copy\r\n"));
	for (i = 0; i < i4Argc ; i++)
		pr_err(TEXT("%s "), szArgv[i]);
	pr_err(TEXT("\r\n"));
	return 0;
}
#endif

/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arMemoryCmdTbl[] = {
#if 1
	#if 0
	{
		TEXT("MemRead"),                    /* pszCmdStr */
		TEXT("r"),
		_CLI_MemRead,                       /* execution function */
		NULL,
		TEXT("r PhyAddr size bits(eg:r 0x300000 0x10 16)"),
		CLI_GUEST
	},
	{
		TEXT("MemWrite"),                  /* pszCmdStr */
		TEXT("w"),
		_CLI_MemWrite,              /* execution function */
		NULL,
		TEXT("w PhyAddr bits data1 data2 ..."),
		CLI_GUEST
	},
	#endif
	{
		TEXT("RegRead"),                    /* pszCmdStr */
		TEXT("ior"),
		_CLI_IOR,                       /* execution function */
		NULL,
		TEXT("ior offset size bits(eg:mr 0xa804c 0x10 16)"),
		CLI_GUEST
	},
	{
		TEXT("RegWrite"),                  /* pszCmdStr */
		TEXT("iow"),
		_CLI_IOW,              /* execution function */
		NULL,
		TEXT("iow offset bits data1 data2 ..."),
		CLI_GUEST
	},
	#if 0
	{
		TEXT("MemFill"),                  /* pszCmdStr */
		TEXT("f"),
		_CLI_MemFill,              /* execution function */
		NULL,
		TEXT("f PhyAddr data size mode"),
		CLI_GUEST
	},
	{
		TEXT("MemCopy"),                  /* pszCmdStr */
		TEXT("mc"),
		_CLI_MemCopy,              /* execution function */
		NULL,
		TEXT("mc DstPhyAddr SrcPhyAddr size"),
		CLI_GUEST
	},
	#endif
#endif
	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};


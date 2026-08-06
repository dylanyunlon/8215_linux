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


#ifndef __linux__
#include "stdlib.h"
#else
#include "winutil.h"
#include "linux/kernel.h"
#define _ttoi(a)   StrToInt(a)
#endif
#include "metazone.h"
#define TEXT

static _CLI_MetazoneRead(INT32 i4Argc, const TCHAR **szArgv)
{
	UINT32 u4Index = 0;
	UINT32 u4Value = 0;

	if (i4Argc < 2) {
		pr_err(TEXT("\r\n Please input index."));
		return -1;
	}

	u4Index = _ttoi(szArgv[1]);
	pr_err(TEXT("MetaZone Index(0x%x) \r\n"), u4Index);
	if (MZ_FAILURE == MetaZone_Read(u4Index, &u4Value)) {
		pr_err(TEXT("Read MetaZone index(0x%x) failed..\r\n"), u4Index);
		return -1;
	}
	pr_err(TEXT("MetaZone Index(0x%x) = %d \r\n"), u4Index, u4Value);
	return 0;
}

#define PRINT_INTERVAL 1000


static INT32 _CLI_MetazoneWrite(INT32 i4Argc, const TCHAR **szArgv)
{
	UINT32 u4Index = 0;
	UINT32 u4Value = 0;
	UINT32 u4Time;
	UINT32 u4Loop = 1;
	UINT32 u4LastTime;
	UINT32 u4CurTime;
	BOOL fgSync = FALSE;

	if (i4Argc < 3) {
		pr_err(TEXT("\r\n Please input index & data."));
		return -1;
	}

	u4Index = _ttoi(szArgv[1]);
	u4Value = _ttoi(szArgv[2]);
	if (i4Argc > 3) {
		u4Loop = _ttoi(szArgv[3]);
	}
	if (i4Argc > 4) {
		fgSync = (BOOL)_ttoi(szArgv[4]);
	}
#if 0
	if (4 != MetaZone_Read(u4Index, &u4Value)) {
		CLI_Printf(KERN_ERR "Read MetaZone index(%d) failed..\r\n"), u4Index));
		return -1;
	}
#endif

	pr_err(TEXT("Write MetaZone Index(0x%x) = value(%d) \r\n"),
			u4Index, u4Value);
	RETAILMSG((u4Loop > 1) , (L"[CLI MTZ] Loop count = %d\r\n", u4Loop));

	/*    u4Time = GetTickCount(); */
	u4LastTime = u4Time;
	while (u4Loop--) {
		/* u4CurTime = GetTickCount(); */
		if (PRINT_INTERVAL < (u4CurTime - u4LastTime)) {
			RETAILMSG(u4Loop, (L"[CLI MTZ] Left loop count = %d\r\n", u4Loop));
			u4LastTime = u4CurTime;
		}

		if (MZ_SUCCESS != MetaZone_Write(u4Index, u4Value)) {
			pr_err(TEXT("Write MetaZone index(0x%x) failed..\r\n"), u4Index);
			return -1;
		}
		MetaZone_Flush(fgSync);
	}
	/*    u4Time = GetTickCount() - u4Time; */
	RETAILMSG(1, (L"[CLI MTZ] Write MetaZone Cost %d ms\r\n", u4Time));

	return 0;
}

static INT32 _CLI_MtzReadBinary(INT32 i4Argc, const TCHAR **szArgv)
{
	UINT32 u4Index = 0;
	UINT32 u4Size = 100;
	BYTE bData[100];

	if (i4Argc < 2) {
		pr_err(TEXT("\r\n Please input binary data index."));
		return -1;
	}

	u4Index = _ttoi(szArgv[1]);
	u4Size =  MetaZone_ReadBinary(u4Index, bData, 100);
	if (!u4Size) {
		pr_err(TEXT("Read binary data index(0x%x) size is zero..\r\n"), u4Index);
		return -1;
	}
	pr_err(TEXT("Binary data index(0x%x) is:"), u4Index);
	for (u4Index = 0; u4Index < u4Size; u4Index++) {
		if (!(u4Index % 16)) {
			pr_err(TEXT("\r\n"));
		}
		pr_err(TEXT("0x%02x "), bData[u4Index]);
	}
	return 0;
}

static _CLI_MtzWriteBinary(INT32 i4Argc, const TCHAR **szArgv)
{
	UINT32 u4Index = 0;
	UINT32 u4Size = 100;
	UINT32 u4Offset = 0;
	INT32 i4ArgcIdx = 1;
	BYTE bData[100];

	if (i4Argc < 3) {
		pr_err(TEXT("\r\n Please input binary data index & data."));
		return (-1);
	}
	memset(bData, 0, 100);

	u4Index = _ttoi(szArgv[i4ArgcIdx]);
	i4ArgcIdx++;
	u4Size =  MetaZone_ReadBinary(u4Index, bData, 100);
	if (i4Argc > 4) {
		u4Offset = _ttoi(szArgv[i4ArgcIdx]);
		i4ArgcIdx++;
	}

	for (; (u4Offset < 100) && (i4ArgcIdx < i4Argc);
			u4Offset++, i4ArgcIdx++) {
		bData[u4Offset] = (BYTE)_ttoi(szArgv[i4ArgcIdx]);
	}
	if (u4Size < u4Offset)
		u4Size = u4Offset;
	MetaZone_Flush(FALSE);
	if (MZ_SUCCESS == MetaZone_WriteBinary(u4Index, bData, u4Size)) {
		pr_err(TEXT("Modify Binary data index(0x%x) offset (%d) succeed.\r\n:"),
				u4Index, u4Offset);
	} else {
		pr_err(TEXT("Modify Binary data index(0x%x) offset (%d) failed..\r\n:"),
				u4Index, u4Offset);
	}
	return 0;

}
#if 0

#include "linux/fs.h"
#include <linux/preempt.h>
#include <asm/uaccess.h>

static _CLI_WriteLogo(INT32 i4Argc, const TCHAR **szArgv)
{
	struct file *f;
	mm_segment_t old_fs;
	loff_t lsize;
	loff_t loff;
	loff_t lwsize;
	char *buffer;

	if (i4Argc < 2) {
		CLI_Printf(KERN_ERR "\r\n Please input file name(full path).");
		return 0;
	}
	CLI_Printf(KERN_ERR "\r\n Open file(%s)\r\n", szArgv[1]);
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	f = filp_open(szArgv[1], O_RDWR, 0);
	if (IS_ERR(f)) {
		CLI_Printf(KERN_ERR "Open file failed.\r\n");
		set_fs(old_fs);
		return 0;
	}

	lsize = vfs_llseek(f, 0, SEEK_END);

	CLI_Printf(KERN_ERR "file size(%d=0x%x)\r\n",
			(UINT32)lsize, (UINT32)lsize);

	if (lsize <= 0) {
		CLI_Printf(KERN_ERR "File size is error!\r\n");
		filp_close(f, NULL);
		set_fs(old_fs);
		return 0;
	}
	lwsize = (lsize + 0x2000 - 1) /  0x2000;
	lwsize *= 0x2000;
	buffer  = kmalloc(lwsize , GFP_KERNEL);

	if (NULL == buffer) {
		CLI_Printf(KERN_ERR "\r\n kmalloc  %d bytes buffer failed ",
				(UINT32)lwsize);
		filp_close(f, NULL);  
		set_fs(old_fs);
		return 0;
	}
	loff = 0;
	lsize = vfs_read(f, buffer, lsize, &loff);

	CLI_Printf(KERN_ERR "_CLI_WriteLogo Logo size(%d = 0x%x)\r\n",
			(UINT32)lwsize, (UINT32)lwsize);

	if (lsize > 0) {
		MetaZone_WriteLogo((BYTE *)buffer, (UINT32)lwsize);
	}
	filp_close(f, NULL);
	set_fs(old_fs);
	kfree(buffer);

	CLI_Printf(KERN_ERR "Write logo success!\r\n");
	return 0;
}

#endif

/******************************************************************************
* Variable      : cli default table
******************************************************************************/
CLI_EXEC_T _arMetazoneCmdTbl[] = {
	{
		TEXT("Read"),                    /* pszCmdStr */
		TEXT("r"),
		_CLI_MetazoneRead,                       /* execution function */
		NULL,
		TEXT("r index(eg:r 2)"),
		CLI_GUEST
	},
	{
		TEXT("Write"),                  /* pszCmdStr */
		TEXT("w"),
		_CLI_MetazoneWrite,              /* execution function */
		NULL,
		TEXT("w index value"),
		CLI_GUEST
	},
	{
		TEXT("ReadBinary"),                    /* pszCmdStr */
		TEXT("rb"),
		_CLI_MtzReadBinary,                       /* execution function */
		NULL,
		TEXT("rb index(eg:r 2)"),
		CLI_GUEST
	},
	{
		TEXT("WriteBinary"),                  /* pszCmdStr */
		TEXT("wb"),
		_CLI_MtzWriteBinary,              /* execution function */
		NULL,
		TEXT("wb index offset value1 value2 ..."),
		CLI_GUEST
	},
	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};


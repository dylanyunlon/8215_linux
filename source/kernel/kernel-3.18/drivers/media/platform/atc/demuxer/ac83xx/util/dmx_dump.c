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

/*!
 * @file dmx_spt_dump.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */


#include "x_os.h"
#include "drv_config.h"
#ifdef __linux__
#include "windows.h"
#include "mach/ac83xx.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
#include <media/atc/drv_esm_if.h>
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "drv_esm_if.h"
#include "mm_debug.h"
#endif /* __linux__ */

#include "x_ckgen.h"
#include "x_hal_ic.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_sema.h"
#include "dmx_stream.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_esm_if.h"
#include "dmx_hal_if.h"

#ifndef __linux__
#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant */
#endif

#ifdef CONFIG_ANDROID_ENABLE
#define DMX_DUMP_FILE_PATH_1	  "/mnt/udisk1/"
#define DMX_DUMP_FILE_PATH_2	  "/mnt/m_external_sd/"
#else
#define DMX_DUMP_FILE_PATH_1	  "/media/udisk1/"
#define DMX_DUMP_FILE_PATH_2	  "/media/ext_sdcard2/"
#endif

const char *_azSptDataTypeName[MAX_SPT_DATA_TYPE_CNT] = {
	"Unknown",
	"Video",
	"Audio",
	"SP/CC",
	"Section",
	"BUF",
	"GRD",
};

const char *g_awszDmxSptStatus[] = {
	TEXT("[NONE]"),
	TEXT("[IDLE]"),
	TEXT("[RUNING]"),
};

const char *g_awszDmxSptTxStatus[] = {
	TEXT("[TX_NONE]"),
	TEXT("[TX_CHECK]"),
	TEXT("[TX_TXING]"),
	TEXT("[TX_PAUSE]"),
	TEXT("[TX_RSPOFF]"),
	TEXT("[TX_ABORT]"),
	TEXT("[TX_JUMP]"),
	TEXT("[TX_ERROR]")
};

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;

#define DMXDUMPLOCKINIT(mrRet)	 \
	do { \
		mrRet = dmx_sema_create(&(g_rDmxDumpMan.hSema), DMX_SEMA_TYPE_BINARY, \
			DMX_SEMA_STATE_UNLOCK); \
		if (DMX_FAILED(mrRet)) { \
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,\
			TEXT("[DECRYPT] %s fail in create semaphore, mrRet: 0x%x\r\n"), \
				DMX_FUNC_NAME, mrRet); \
		} \
	} while (0)

#define DMXDUMPLOCK()	 \
	dmx_sema_lock(g_rDmxDumpMan.hSema, DMX_SEMA_OPTION_WAIT)

#define DMXDUMPUNLOCK()   \
	dmx_sema_unlock(g_rDmxDumpMan.hSema)

#define DMXDUMPLOCKDEINIT(mrRet)  \
	dmx_sema_delete(g_rDmxDumpMan.hSema); \
	g_rDmxDumpMan.hSema = NULL; \
	mrRet = RET_DMX_OK

#ifdef __linux__
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <asm/unistd.h>

#define MAX_PATH 256
typedef struct {
	char  cFileName[MAX_PATH];
	u32 dwFileAttributes;
} WIN32_FIND_DATA;

#define FILE_ATTRIBUTE_UNKNOWN	 0
#define FILE_ATTRIBUTE_DIRECTORY 1
#define FILE_ATTRIBUTE_FILE	2
#define INVALID_HANDLE_VALUE	 (-1)

static HANDLE FindFirstFile(char *filedir, WIN32_FIND_DATA *filedata)
{
	struct file *pfile = NULL;
	struct kstat statbuf;
	char dirName[256];
	char *szEndChar = NULL;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS);

	memset(dirName, 0 , MAX_PATH);

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] FindFirstFile filedir %s !\r\n"), filedir);
	strcpy(dirName, filedir);

	szEndChar = strrchr(dirName, '/');
	if (NULL == szEndChar) {
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] invalid dir or file name : %s\r\n"), filedir);
	set_fs(fs);
		return (HANDLE)INVALID_HANDLE_VALUE;
	}
	pfile = filp_open(dirName, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		int errorno = 0;

		errorno = PTR_ERR(pfile);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("line %d -- no directory or file %s error, errorno: %d\r\n"),
			__LINE__, dirName, errorno);
		set_fs(fs);
		return (HANDLE)INVALID_HANDLE_VALUE;
	}

	if (0 != vfs_stat(dirName, &statbuf)) {
		int errorno = 0;

		errorno = PTR_ERR(pfile);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("line %d -- vfs_stat fail, no directory ")
			TEXT("or file %s error, errorno: %d\r\n"),
			__LINE__, dirName, errorno);
		filp_close(pfile, NULL);
		set_fs(fs);
		return (HANDLE)INVALID_HANDLE_VALUE;
	}

	if (S_ISDIR(statbuf.mode))
		filedata->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	else if (S_ISREG(statbuf.mode))
		filedata->dwFileAttributes = FILE_ATTRIBUTE_FILE;
	else
		filedata->dwFileAttributes = FILE_ATTRIBUTE_UNKNOWN;

	filp_close(pfile, NULL);

	set_fs(fs);

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("%s is filedata->cFileName\r\n"), dirName);

	return (HANDLE)1;
}

static void FindClose(HANDLE hFile)
{
}

HANDLE CreateDumpFile(
	const char *lpFileName,
	u32 dwDesiredAccess,
	u32 dwShareMode,
	void *lpSecurityAttributes,
	u32 dwCreationDispostion,
	u32 dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	struct file *pfile = NULL;
	int flags = 0;
	HANDLE ret;
	struct inode *pinode = NULL;
	loff_t t_cur_pos = 0;

	switch (dwDesiredAccess) {
	case GENERIC_READ:
		flags = O_RDONLY;
		break;
	case GENERIC_WRITE:
		flags = O_WRONLY;
		break;
	case GENERIC_RW:
		flags = O_RDWR;
		break;
	default:
		break;
	}

	if (dwCreationDispostion == CREATE_ALWAYS)
		flags |= O_TRUNC | O_CREAT;

	pfile = filp_open(lpFileName, flags, 0666);
	if (IS_ERR(pfile)) {
		int errorno = PTR_ERR(pfile);

		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d filp_open %s error, errorno: %d\r\n"),
			__func__, __LINE__, lpFileName, errorno);
		ret = (HANDLE)INVALID_HANDLE_VALUE;
		return ret;
	}
	ret = (HANDLE)pfile;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("%s line %d -- success, file name: %s\r\n"),
		__func__, __LINE__, lpFileName);

	t_cur_pos = vfs_llseek(pfile, (loff_t)0, SEEK_SET);
	t_cur_pos = vfs_llseek(pfile, (loff_t)t_cur_pos, SEEK_SET);
	pinode = pfile->f_dentry->d_inode;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("*%s line %d --> create file %s success, ")
		TEXT("pfile: 0x%lx, curPos: %lld, f_pos: %lld\r\n"),
		__func__, __LINE__, lpFileName,
		(u32)pfile, t_cur_pos, pfile->f_pos);

	return ret;
}

bool CloseDumpFile(HANDLE hObject)
{
	struct file *pfile = (struct file *)hObject;

	if (IS_ERR(pfile))
		return FALSE;
	filp_close(pfile, NULL);
	return TRUE;
}

static char _szDmxWriteFileTmpbuf[512 * 1024] = {0};
bool WriteFile(
	HANDLE hFile,
	const void *lpBuffer,
	u32 nNumberOfBytesToWrite,
	u32 *lpNumberOfBytesWritten,
	void *lpOverlapped
)
{
	struct file *pfile = (struct file *)hFile;
	ssize_t u4Size = 0;
	struct inode *pinode = NULL;
	loff_t t_cur_pos = 0, datapos = 0;
	mm_segment_t old_fs;
	u32 dwWriteTotalSz = 0, dwWriteSz;

	if (NULL == lpBuffer) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("************************Dmx WriteFile err, ")
			TEXT("invalid buffer address ****************\r\n"));
		return FALSE;
	}

	pinode = pfile->f_dentry->d_inode;


	old_fs = get_fs();
	set_fs(KERNEL_DS);

	dwWriteTotalSz = 0;

	datapos = (loff_t)(pinode->i_size);

	while (dwWriteTotalSz < nNumberOfBytesToWrite) {
		if (dwWriteTotalSz + (512 * 1024) <= nNumberOfBytesToWrite)
			dwWriteSz = 512 * 1024;
		else
			dwWriteSz = nNumberOfBytesToWrite - dwWriteTotalSz;

		t_cur_pos = vfs_llseek(pfile, datapos, SEEK_SET);

		mm_memcpy(_szDmxWriteFileTmpbuf, lpBuffer + dwWriteTotalSz, dwWriteSz);

		u4Size = vfs_write(pfile, _szDmxWriteFileTmpbuf, dwWriteSz, &t_cur_pos);
	  if (u4Size < 0) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("hFile: 0x%lx, Dmx vfs_write err, ")
				TEXT("ret = %i, i_size: %lld, t_cur_pos: %lld, f_pos: %lld\r\n"),
				(u32)hFile, u4Size, pinode->i_size, t_cur_pos, pfile->f_pos);
			*lpNumberOfBytesWritten = dwWriteTotalSz;
			set_fs(old_fs);
		return FALSE;
	  }

		datapos += dwWriteSz;
		dwWriteTotalSz += dwWriteSz;
	}

	set_fs(old_fs);

	if (dwWriteTotalSz < nNumberOfBytesToWrite)
		return FALSE;

	if (NULL != lpNumberOfBytesWritten)
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

	return TRUE;
}

u32 GetFileSize(HANDLE hFile, u32 *pdwHighSize)
{
	struct file *pfile = (struct file *)hFile;
	struct inode *pinode = NULL;
	u64 u8Sz = 0;

	pinode = pfile->f_dentry->d_inode;
	u8Sz = (u64)(pinode->i_size);

	*pdwHighSize = (u32)((u32)(u8Sz >> 32));

	return (u32)((u32)(u8Sz));
}

u32 GetTickCount(void)
{
	return ((u32)(1000*jiffies/HZ));
}

#endif /* #ifdef __linux__ */

void DmxDumpInit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	mm_memset(&g_rDmxDumpMan, 0, sizeof(DMX_DUMP_MAN_T));

	DMXDUMPLOCKINIT(mrRet);
}

void DmxDumpDeInit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DmxDumpCloseAllFile();

	DMXDUMPLOCKDEINIT(mrRet);

	mm_memset(&g_rDmxDumpMan, 0, sizeof(DMX_DUMP_MAN_T));
}

void DmxDumpCloseAllFile(void)
{
	u32 u4Idx = 0;

	DmxCloseDumpVFile();
	DmxCloseDumpAFile();
	DmxCloseDumpPbbufFile();
	DmxCloseDumpFlowFile();

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++)
		DmxCloseDumpRspFile(u4Idx);
}

void DmxCloseDumpStmFile(E_DMX_CLI_STM_TYPE_T eCliStmType, char *wszStmType)
{
	if (((HANDLE)INVALID_HANDLE_VALUE !=
		g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile) &&
		(NULL != g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile)) {
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile =
			(HANDLE)INVALID_HANDLE_VALUE;
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close %s Fifo Dump Files!\r\n"), wszStmType);
	}

	if (((HANDLE)INVALID_HANDLE_VALUE !=
		g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile) &&
		(NULL != g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile)) {
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile =
			(HANDLE)INVALID_HANDLE_VALUE;
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close %s AUInfo Dump Files!\r\n"), wszStmType);
	}

	if (((HANDLE)INVALID_HANDLE_VALUE !=
		g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile) &&
		(NULL != g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile)) {
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile =
			(HANDLE)INVALID_HANDLE_VALUE;
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close %s DmaInfo Dump Files!\r\n"), wszStmType);
	}
}

bool DmxCreateDumpStmFile(char *wszDir,
	E_DMX_CLI_STM_TYPE_T eCliStmType, char *wszStmType)
{
#ifndef __linux__
	char wszDirName[100] = {0};
#endif
	char wszFileName[150] = {0};
	WIN32_FIND_DATA rData;
	char *wszPath = NULL;
	HANDLE	hFile;
	u32 u4Idx = 0;
	bool  fgCreateSucc = FALSE;

	DmxCloseDumpStmFile(eCliStmType, wszStmType);

	u4Idx = 1;
	fgCreateSucc = FALSE;
	while (u4Idx < 3) {
		if (u4Idx == 1)
			wszPath = DMX_DUMP_FILE_PATH_1;
		if (u4Idx == 2)
			wszPath = DMX_DUMP_FILE_PATH_2;

		hFile = FindFirstFile(wszPath, &rData);
		if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
			FindClose(hFile);
		else {
			u4Idx++;
			continue;
		}

#ifdef __linux__
		sprintf(wszFileName, "%s%s_FifoData", wszPath, wszDir);
#else
		vsnprintf(wszDirName, 100 * sizeof(char), TEXT("%s%s"), wszPath, wszDir);
		hFile = FindFirstFile(wszDirName, &rData);

		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(0 == (FILE_ATTRIBUTE_DIRECTORY & rData.dwFileAttributes))) {
			if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
				FindClose(hFile);

			if (!CreateDirectory(wszDirName, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create %s Dump Directory(%s)Failed!\r\n"),
					wszStmType, wszDirName);

				return FALSE;
			}
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create %s Dump Directory(%s) Success!\r\n"),
				wszStmType, wszDirName);
		} else
			FindClose(hFile);

		mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
		vsnprintf(wszFileName, 150 * sizeof(char),
			TEXT("%s\\FifoData"), wszDirName);
#endif /* __linux__ */

		g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile =
			CreateDumpFile(wszFileName,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (((HANDLE)INVALID_HANDLE_VALUE ==
			g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile) ||
			(NULL == g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create %s FifoData File %s fail!, err: %d\r\n"),
				wszStmType, wszFileName, DMX_GET_LASTERR);
			u4Idx++;
			continue;
		}

		break;
	}

	if (u4Idx >= 3)
		return FALSE;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create %s FifoData File %s success!\r\n"),
		wszStmType, wszFileName);

	mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
#ifdef __linux__
	sprintf(wszFileName, "%s%s_AuInfo", wszPath, wszDir);
#else
	vsnprintf(wszFileName, 150 * sizeof(char),
	TEXT("%s\\AuInfo"), wszDirName);
#endif /* __linux__ */

	 g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile =
		CreateDumpFile(wszFileName,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile) ||
			(NULL == g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Create %s AuInfo File %s fail!, err: %d\r\n"),
			wszStmType, wszFileName, DMX_GET_LASTERR);
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile = NULL;
		return FALSE;
	}


	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create %s AuInfo File %s success!\r\n"),
		wszStmType, wszFileName);

	mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
#ifdef __linux__
	sprintf(wszFileName, "%s%s_DmaInfo", wszPath, wszDir);
#else
	vsnprintf(wszFileName, 150 * sizeof(char),
	TEXT("%s\\DmaInfo"), wszDirName);
#endif /* __linux__ */

	 g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile =
		CreateDumpFile(wszFileName,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile) ||
			(NULL == g_rDmxDumpMan.arDmxStms[eCliStmType].hDmaInfoFile)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Create %s DmaInfo File %s fail!, err: %d\r\n"),
			wszStmType, wszFileName, DMX_GET_LASTERR);
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hFifoFile = NULL;
		CloseDumpFile(g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile);
		g_rDmxDumpMan.arDmxStms[eCliStmType].hAUInfoFile = NULL;
		return FALSE;
	}

	g_rDmxDumpMan.arDmxStms[eCliStmType].u4FifoDataEa = 0;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create %s AuInfo File %s success!\r\n"),
		wszStmType, wszFileName);

	return TRUE;
}

bool DmxCreateDumpVFile(char *wszVDirName)
{
#ifdef __linux__
	return DmxCreateDumpStmFile(wszVDirName, DMX_CLI_STM_VID, "Video");
#else
	return DmxCreateDumpStmFile(wszVDirName, DMX_CLI_STM_VID, L"Video");
#endif
}

void DmxCloseDumpVFile(void)
{
#ifdef __linux__
	DmxCloseDumpStmFile(DMX_CLI_STM_VID, "Video");
#else
	DmxCloseDumpStmFile(DMX_CLI_STM_VID, L"Video");
#endif
	g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_VID]++;
	mm_memset((void *)(&(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID])), 0,
		sizeof(DMX_DUMP_STM_INFO_T));
}

void DmxDumpVFrame(AU_VPic *prVidAU, uintptr_t ptrVFifoVSa, uintptr_t ptrVFifoVEa)
{
	PicInfo *prPicInfo = NULL;
	u32	dwWriteBytes = 0;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};

		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "VFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_VID]);
		if (!DmxCreateDumpVFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create Video dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile)) {
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hAUInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hAUInfoFile)) {
		return;
	}

	if (NULL == prVidAU)
		return;

	if (AU_DATA != prVidAU->eAuType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid AU Type: %d!\r\n"),
			DMX_FUNC_NAME, prVidAU->eAuType);
		return;
	}

	DMXDUMPLOCK();

	prPicInfo = &(prVidAU->rAUInfo.rInfo);

	if (prPicInfo->ptrSAddr < prPicInfo->ptrEAddr) {
		u32	dwLowSize1 = 0, dwLowSize2 = 0;
		u32	dwHighSize1 = 0, dwHighSize2 = 0;

		dwLowSize1 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(u32 *)(&dwHighSize1));

		if (0xFFFFFFFF == dwLowSize1) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get VFIFO File's size 1, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(void *)(prPicInfo->ptrSAddr),
			(prPicInfo->ptrEAddr - prPicInfo->ptrSAddr), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write VFIFO File 1, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
		dwLowSize2 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(u32 *)(&dwHighSize2));
		if (0xFFFFFFFF == dwLowSize2) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get VFIFO File's size 1.2, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"FrameIdx: %d, PicType: 0x%x, RawFileOfst: 0x%08x%08x, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, Pts: %lld, Size: %d\r\n",
			(s32)(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u4FrameCnt),
			prPicInfo->u4VType,
			(u32)((prPicInfo->u8Offset) >> 32),
			(u32)(prPicInfo->u8Offset),
			(u32)dwHighSize1, (u32)dwLowSize1,
			(u32)dwHighSize2, (u32)dwLowSize2,
			(s64)(prPicInfo->u8Pts),
			(s32)(DMX_DATASIZE(prPicInfo->ptrSAddr, prPicInfo->ptrEAddr,
				(ptrVFifoVEa - ptrVFifoVSa))));

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hAUInfoFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in VAUInfo file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else {
		u32	dwLowSize1 = 0, dwLowSize2 = 0;
		u32	dwHighSize1 = 0, dwHighSize2 = 0;

		dwLowSize1 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(u32 *)(&dwHighSize1));
		if (0xFFFFFFFF == dwLowSize1) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get VFIFO File's size 2, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(void *)(prPicInfo->ptrSAddr),
			(ptrVFifoVEa - prPicInfo->ptrSAddr), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write VFIFO File 2.1, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(void *)ptrVFifoVSa,
			(prPicInfo->ptrEAddr - ptrVFifoVSa), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write VFIFO File 2.2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
		dwLowSize2 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hFifoFile,
			(u32 *)(&dwHighSize2));
		if (0xFFFFFFFF == dwLowSize2) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get VFIFO File's size 2.2, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"FrameIdx: %d, PicType: 0x%x, RawFileOfst: 0x%08x%08x, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, Pts: %lld, Size: %d\r\n",
			(s32)(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u4FrameCnt),
			prPicInfo->u4VType,
			(u32)((prPicInfo->u8Offset) >> 32),
			(u32)(prPicInfo->u8Offset),
			(u32)dwHighSize1, (u32)dwLowSize1,
			(u32)dwHighSize2, (u32)dwLowSize2,
			(s64)(prPicInfo->u8Pts),
			(s32)(DMX_DATASIZE(prPicInfo->ptrSAddr, prPicInfo->ptrEAddr,
			(ptrVFifoVEa - ptrVFifoVSa))));
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hAUInfoFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in VAUInfo file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	}

	g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u4FrameCnt++;

	DMXDUMPUNLOCK();
}

void DmxDumpVDmaInfo(u64 u8FileOfst, void *pvBuf, u64 u8Len)
{
	u32	dwWriteBytes;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "VFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_VID]);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"VFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_VID]);
#endif /* __linux__ */
		if (!DmxCreateDumpVFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create Video dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile)) {
		return;
	}

	DMXDUMPLOCK();

	if (0 != u8FileOfst) {
		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		#ifdef __linux__
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Pbb2Fifo -- Ofst: %lld, Len: %lld, Before Dma's VFifoDataSz: %lld\r\n",
			u8FileOfst, u8Len,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz);
		#else
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Pbb2Fifo -- Ofst: %I64d, Len: %I64d, Before Dma's VFifoDataSz: %I64d\r\n",
			u8FileOfst, u8Len,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz);
		#endif /* #ifdef __linux__ */
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write VDmainfo File 1, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}

		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz += u8Len;
	} else if (NULL != pvBuf) {
		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));
		#ifdef __linux__
		sprintf(g_rDmxDumpMan.szDumpInfo, "Mem2Fifo -- Len: %lld, Before Dma's VFifoDataSz: %lld\r\n",
			u8Len, g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz);
		#else
		sprintf(g_rDmxDumpMan.szDumpInfo, "Mem2Fifo -- Len: %I64d, Before Dma's VFifoDataSz: %I64d\r\n",
			u8Len, g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz);
		#endif /* #ifdef __linux__ */
		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write VDmainfo File 2, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}

		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].u8FifoDataSz += u8Len;
		{
			u64 u8Idx = 0;
			u8  *pu1Bytes = (u8 *)pvBuf;
			u8  au1Byte[4] = {0};
			u8  u1Char = 0;

			for (u8Idx = 0; u8Idx < u8Len; u8Idx++, pu1Bytes++) {
				u1Char = (((*pu1Bytes) >> 4) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[0] = (char)u1Char;

				u1Char = ((*pu1Bytes) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[1] = (char)u1Char;
				au1Byte[2] = ',';
				au1Byte[3] = ' ';
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile,
					au1Byte,
					4, &dwWriteBytes, NULL)) {
					#ifdef __linux__
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%lld] to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#else
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%I64d] to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#endif /* #ifdef __linux__ */
				}
				if ((u8Idx % 10) == 9) {
					au1Byte[0] = '\r';
					au1Byte[1] = '\n';
				}
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile,
					au1Byte,
					2, &dwWriteBytes, NULL)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write \\r\\n")
						TEXT(" to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
				}
			}
			au1Byte[0] = '\r';
			au1Byte[1] = '\n';
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_VID].hDmaInfoFile,
				au1Byte, 2, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write \\r\\n to VDma ")
					TEXT("file 4, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
			}
		}
	}

	DMXDUMPUNLOCK();

}

bool DmxCreateDumpAFile(char *wszADirName)
{
#ifdef __linux__
	return DmxCreateDumpStmFile(wszADirName, DMX_CLI_STM_AUD, "Audio");
#else
	return DmxCreateDumpStmFile(wszADirName, DMX_CLI_STM_AUD, L"Audio");
#endif
}

void DmxCloseDumpAFile(void)
{
#ifdef __linux__
	DmxCloseDumpStmFile(DMX_CLI_STM_AUD, "Audio");
#else
	DmxCloseDumpStmFile(DMX_CLI_STM_AUD, L"Audio");
#endif
	g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_AUD]++;
	mm_memset((void *)(&(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD])),
		0, sizeof(DMX_DUMP_STM_INFO_T));
}

void DmxDumpASample(AU_AUDIO *prAudAU, u32 u4AFifoVSa,
	u32 u4AFifoVEa, u32 u4TxUID, bool fgRsping)
{
	u32	dwWriteBytes = 0;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "AFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_AUD]);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"AFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_AUD]);
#endif /* __linux__ */
		if (!DmxCreateDumpAFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create Audio dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile))
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hAUInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hAUInfoFile))
		return;

	if (NULL == prAudAU)
		return;

	if (AU_DATA != prAudAU->eAuType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid AU Type: %d!\r\n"),
			DMX_FUNC_NAME, prAudAU->eAuType);
		return;
	}

	DMXDUMPLOCK();

	if (0 == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa)
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa = u4AFifoVSa;

	if (!fgRsping) {
		if (g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa < prAudAU->ptrEAddr) {
			u32	dwLowSize1 = 0, dwLowSize2 = 0;
			u32	dwHighSize1 = 0, dwHighSize2 = 0;

			dwLowSize1 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
				(u32 *)(&dwHighSize1));
			if (0xFFFFFFFF == dwLowSize1) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get AFIFO File's size 1,")
					TEXT(" err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
				(void *)(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa),
				(prAudAU->ptrEAddr - g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa),
				&dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write AFIFO File 1, ")
					TEXT("err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
			dwLowSize2 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
					(u32 *)(&dwHighSize2));
			if (0xFFFFFFFF == dwLowSize2) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get AFIFO File's ")
					TEXT("size 1.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

			sprintf(g_rDmxDumpMan.szDumpInfo,
				"u4TxUID: %d, Rsping: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, Pts: %lld, Sa: 0x%x, Ea: 0x%x\r\n",
				(s32)u4TxUID, (fgRsping ? 1 : 0),
				(u32)dwHighSize1, (u32)dwLowSize1,
				(u32)dwHighSize2, (u32)dwLowSize2,
				(s64)prAudAU->rAUInfo.rInfo.u8Pts, (u32)prAudAU->ptrSAddr, (u32)prAudAU->ptrEAddr);

			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hAUInfoFile,
				(void *)g_rDmxDumpMan.szDumpInfo,
				strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write Aud AUInfo ")
					TEXT("file 1, err: %d!\r\n"),
					 DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		} else {
			u32	dwLowSize1 = 0, dwLowSize2 = 0;
			u32	dwHighSize1 = 0, dwHighSize2 = 0;

			dwLowSize1 =
				GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
							(u32 *)(&dwHighSize1));
			if (0xFFFFFFFF == dwLowSize1) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get AFIFO File's ")
					TEXT("size 2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
				(void *)(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa),
				(u4AFifoVEa - g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa),
					&dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write AFIFO ")
					TEXT("File 2.1, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
				(void *)u4AFifoVSa,
				(prAudAU->ptrEAddr - u4AFifoVSa), &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write AFIFO ")
					TEXT("File 2.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
			dwLowSize2 =
				GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
							(u32 *)(&dwHighSize2));
			if (0xFFFFFFFF == dwLowSize2) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get AFIFO File's ")
					TEXT("size 2.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			mm_memset(g_rDmxDumpMan.szDumpInfo, 0,
				sizeof(g_rDmxDumpMan.szDumpInfo));

			sprintf(g_rDmxDumpMan.szDumpInfo,
				"u4TxUID: %d, Rsping: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, Pts: %lld, Sa: 0x%x, Ea: 0x%x\r\n",
				(s32)u4TxUID, (fgRsping ? 1 : 0),
				(u32)dwHighSize1, (u32)dwLowSize1,
				(u32)dwHighSize2, (u32)dwLowSize2,
				(s64)prAudAU->rAUInfo.rInfo.u8Pts,
				(u32)prAudAU->ptrSAddr, (u32)prAudAU->ptrEAddr);

			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hAUInfoFile,
				(void *)g_rDmxDumpMan.szDumpInfo,
				strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Aud AUInfo file 2,")
					TEXT("err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		}

		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u4FifoDataEa = prAudAU->ptrEAddr;

	} else {
		u32	dwLowSize1 = 0, dwLowSize2 = 0;
		u32	dwHighSize1 = 0, dwHighSize2 = 0;

		dwLowSize1 =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
			(u32 *)(&dwHighSize1));
		if (0xFFFFFFFF == dwLowSize1) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get AUDIO FIFO File's")
					TEXT(" size 2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		dwLowSize2 = dwLowSize1;
		dwHighSize2 = dwHighSize1;

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0,
			sizeof(g_rDmxDumpMan.szDumpInfo));

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"TXUID: %d, Rsping: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, Pts: %lld\r\n",
			(s32)u4TxUID, (fgRsping ? 1 : 0),
			(u32)dwHighSize1, (u32)dwLowSize1,
			(u32)dwHighSize2, (u32)dwLowSize2,
			(s64)prAudAU->rAUInfo.rInfo.u8Pts);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hAUInfoFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in AUDIO AUInfo file 2,")
				TEXT(" err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	}

	DMXDUMPUNLOCK();
}

void DmxDumpADmaInfo(void *pvSptHdl, u64 u8FileOfst,
	void *pvBuf, u64 u8Len, u32 u4TxUID)
{
	u32	u4CurUID   = 0;
	u32	dwWriteBytes;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		return;

	if (NULL == pvSptHdl) {
		DMX_ASSERT(FALSE);
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "AFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_AUD]);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"AFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_AUD]);
#endif /* __linux__ */
		if (!DmxCreateDumpAFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create Audio dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile))
		return;

	DMXDUMPLOCK();

	if (0 != u8FileOfst) {
		u32	dwLowSize = 0, dwHighSize = 0;
		u64	u8AFifoFileSz = 0;

		dwLowSize =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
			(u32 *)(&dwHighSize));
		if (0xFFFFFFFF == dwLowSize) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get AFIFO File's size 1, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		u8AFifoFileSz = (u64)((((u64)dwHighSize) << 32) | dwLowSize);

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		u4CurUID = GetStmUIDByType(pvSptHdl, SPT_DATA_A);
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Pbb2Fifo -- FileOfst: 0x%llx, Len: 0x%llx, TxUID: %d, CurTxUID: %d, BeforeDma's FifoDataSz: 0x%llx, AFifoFileSz: 0x%llx\r\n",
			u8FileOfst, u8Len, (s32)u4TxUID, (s32)u4CurUID,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u8FifoDataSz,
			u8AFifoFileSz);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write ADmainfo File, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}

		if (u4CurUID == u4TxUID)
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u8FifoDataSz += u8Len;
	} else if (NULL != pvBuf) {
		u32	dwLowSize = 0, dwHighSize = 0;
		u64	u8AFifoFileSz = 0;

		dwLowSize =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hFifoFile,
			(u32 *)(&dwHighSize));
		if (0xFFFFFFFF == dwLowSize) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get AFIFO File's ")
				TEXT("size 2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		u8AFifoFileSz = (u64)((((u64)dwHighSize) << 32) | dwLowSize);

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		u4CurUID = GetStmUIDByType(pvSptHdl, SPT_DATA_A);

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Mem2Fifo -- Len: %lld, TxUID: %d, CurTxUID: %d, BeforeDma's FifoDataSz: 0x%llx, AFifoFileSz: 0x%llx\r\n",
			(s64)u8Len, (s32)u4TxUID, (s32)u4CurUID,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u8FifoDataSz,
			u8AFifoFileSz);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write ADmainfo File,")
				TEXT(" err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	{
			u64 u8Idx = 0;
			u8  *pu1Bytes = (u8 *)pvBuf;
			u8  au1Byte[4] = {0};
			u8  u1Char = 0;

			for (u8Idx = 0; u8Idx < u8Len; u8Idx++, pu1Bytes++) {
				u1Char = (((*pu1Bytes) >> 4) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[0] = (char)u1Char;

				u1Char = ((*pu1Bytes) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[1] = (char)u1Char;
				au1Byte[2] = ',';
				au1Byte[3] = ' ';
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile,
					au1Byte,
					4, &dwWriteBytes, NULL)) {
					#ifdef __linux__
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%lld] to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#else
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%I64d] to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#endif /* #ifdef __linux__ */
				}
				if ((u8Idx % 10) == 9) {
					au1Byte[0] = '\r';
					au1Byte[1] = '\n';
				}
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile,
					au1Byte,
					2, &dwWriteBytes, NULL)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write \\r\\n")
						TEXT(" to VDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				}
			}
			au1Byte[0] = '\r';
			au1Byte[1] = '\n';
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].hDmaInfoFile,
				au1Byte, 2, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write \\r\\n to ")
					TEXT("VDma file 4, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
			}
		}

		if (u4CurUID == u4TxUID)
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_AUD].u8FifoDataSz += u8Len;

	} else
		DMX_ASSERT(FALSE);

	DMXDUMPUNLOCK();

}

bool DmxCreateDumpSPFile(char *wszSPDirName)
{
#ifdef __linux__
	return DmxCreateDumpStmFile(wszSPDirName, DMX_CLI_STM_SP, "Audio");
#else
	return DmxCreateDumpStmFile(wszSPDirName, DMX_CLI_STM_SP, L"Audio");
#endif
}

void DmxCloseDumpSPFile(void)
{
#ifdef __linux__
	DmxCloseDumpStmFile(DMX_CLI_STM_SP, "Audio");
#else
	DmxCloseDumpStmFile(DMX_CLI_STM_SP, L"Audio");
#endif
	g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_SP]++;
	mm_memset((void *)(&(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP])),
		0, sizeof(DMX_DUMP_STM_INFO_T));
}

void DmxDumpSPSample(AU_SP *prSPAU, u32 u4SPFifoVSa,
	u32 u4SPFifoVEa, u32 u4TxUID, bool fgRsping)
{
	u32	dwWriteBytes = 0;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP])
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "SPFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_SP]);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"SPFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_SP]);
#endif /* __linux__ */
		if (!DmxCreateDumpSPFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create SP dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile)) {
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hAUInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hAUInfoFile)) {
		return;
	}

	if (NULL == prSPAU)
		return;

	if (AU_DATA != prSPAU->eAuType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid AU Type: %d!\r\n"),
			DMX_FUNC_NAME, prSPAU->eAuType);
		return;
	}

	DMXDUMPLOCK();

	if (!fgRsping) {
		if (prSPAU->rAUInfo.rInfo.ptrAddr +
			prSPAU->rAUInfo.rInfo.u4Size <= u4SPFifoVEa) {
			u32	dwLowSize1 = 0, dwLowSize2 = 0;
			u32	dwHighSize1 = 0, dwHighSize2 = 0;

			dwLowSize1 =
				GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
							(u32 *)(&dwHighSize1));
			if (0xFFFFFFFF == dwLowSize1) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get SP FIFO File's")
					TEXT(" size 1, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
				(void *)(prSPAU->rAUInfo.rInfo.ptrAddr),
				prSPAU->rAUInfo.rInfo.u4Size, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write SP FIFO ")
					TEXT("File 1, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}

			dwLowSize2 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
							(u32 *)(&dwHighSize2));
			if (0xFFFFFFFF == dwLowSize2) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get SP FIFO ")
					TEXT("File's size 1.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

			sprintf(g_rDmxDumpMan.szDumpInfo,
				"TXUID: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, StartPts: %lld, EndPts: %lld, Offst: 0x%llx\r\n",
				(s32)u4TxUID,
				(u32)dwHighSize1, (u32)dwLowSize1,
				(u32)dwHighSize2, (u32)dwLowSize2,
				(s64)prSPAU->rAUInfo.rInfo.u8StartPts,
				(s64)prSPAU->rAUInfo.rInfo.u8EndPts,
				prSPAU->rAUInfo.rInfo.u8Offset);
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hAUInfoFile,
				(void *)g_rDmxDumpMan.szDumpInfo,
				strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write SP AUInfo file 1,")
					TEXT(" err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		} else {
			u32	dwLowSize1 = 0, dwLowSize2 = 0;
			u32	dwHighSize1 = 0, dwHighSize2 = 0;
			u32	u4EndSz = 0;

			dwLowSize1 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
							(u32 *)(&dwHighSize1));
			if (0xFFFFFFFF == dwLowSize1) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get SP FIFO File's ")
					TEXT("size 2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			u4EndSz = u4SPFifoVEa - prSPAU->rAUInfo.rInfo.ptrAddr;
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
				(void *)(prSPAU->rAUInfo.rInfo.ptrAddr),
				u4EndSz, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write SP FIFO ")
					TEXT("File 2.1, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
				(void *)u4SPFifoVSa,
				(prSPAU->rAUInfo.rInfo.u4Size - u4EndSz),
				&dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write SP FIFO ")
					TEXT("File 2.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}

			dwLowSize2 = GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
							(u32 *)(&dwHighSize2));
			if (0xFFFFFFFF == dwLowSize2) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in Get SP FIFO ")
					TEXT("File's size 2.2, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				DMXDUMPUNLOCK();
				return;
			}

			mm_memset(g_rDmxDumpMan.szDumpInfo, 0,
				sizeof(g_rDmxDumpMan.szDumpInfo));

			sprintf(g_rDmxDumpMan.szDumpInfo,
				"TXUID: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, StartPts: %lld, EndPts: %lld, Offst: 0x%llx\r\n",
				(s32)u4TxUID,
				(u32)dwHighSize1, (u32)dwLowSize1,
				(u32)dwHighSize2, (u32)dwLowSize2,
				(s64)prSPAU->rAUInfo.rInfo.u8StartPts,
				(s64)prSPAU->rAUInfo.rInfo.u8EndPts,
				prSPAU->rAUInfo.rInfo.u8Offset);

			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hAUInfoFile,
				(void *)g_rDmxDumpMan.szDumpInfo,
				strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in SP AUInfo file 2,")
					TEXT(" err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		}
	} else {
		u32	dwLowSize1 = 0, dwLowSize2 = 0;
		u32	dwHighSize1 = 0, dwHighSize2 = 0;

		dwLowSize1 =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
			(u32 *)(&dwHighSize1));
		if (0xFFFFFFFF == dwLowSize1) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get SP FIFO File's ")
					TEXT("size 2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		dwLowSize2 = dwLowSize1;
		dwHighSize2 = dwHighSize1;

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"TXUID: %d, StartPos: 0x%08x%08x, EndPos: 0x%08x%08x, StartPts: %lld, EndPts: %lld, Offst: 0x%llx\r\n",
			(s32)u4TxUID,
			(u32)dwHighSize1, (u32)dwLowSize1,
			(u32)dwHighSize2, (u32)dwLowSize2,
			(s64)prSPAU->rAUInfo.rInfo.u8StartPts,
			(s64)prSPAU->rAUInfo.rInfo.u8EndPts,
			prSPAU->rAUInfo.rInfo.u8Offset);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hAUInfoFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in SP AUInfo file 2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	}

	DMXDUMPUNLOCK();
}

void DmxDumpSPDmaInfo(void *pvSptHdl, u64 u8FileOfst,
	void *pvBuf, u64 u8Len, u32 u4TxUID)
{
	u32	u4CurUID   = 0;
	u32	dwWriteBytes;

	if (!g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP])
		return;

	if (NULL == pvSptHdl) {
		DMX_ASSERT(FALSE);
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "SPFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_SP]);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"SPFIFO%d",
			g_rDmxDumpMan.u4DmxStmDumpCnt[DMX_CLI_STM_SP]);
#endif /* __linux__ */
		if (!DmxCreateDumpSPFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create SP dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile) ||
		(NULL == g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile))
		return;

	DMXDUMPLOCK();

	if (0 != u8FileOfst) {
		u32	dwLowSize = 0, dwHighSize = 0;
		u64	u8SPFifoFileSz = 0;

		dwLowSize =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
			(u32 *)(&dwHighSize));
		if (0xFFFFFFFF == dwLowSize) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get SPFIFO File's")
				TEXT(" size 1, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		u8SPFifoFileSz = (u64)((((u64)dwHighSize) << 32) | dwLowSize);

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		u4CurUID = GetStmUIDByType(pvSptHdl, SPT_DATA_SP);
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Pbb2Fifo -- FileOfst: 0x%llx, Len: 0x%llx, TxUID: %d, CurTxUID: %d, BeforeDma's FifoDataSz: 0x%llx, AFifoFileSz: 0x%llx\r\n",
			u8FileOfst, u8Len, (s32)u4TxUID, (s32)u4CurUID,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].u8FifoDataSz,
			u8SPFifoFileSz);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write SPDmainfo File,")
				TEXT(" err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}

		u4CurUID = GetStmUIDByType(pvSptHdl, SPT_DATA_SP);
		if (u4CurUID == u4TxUID)
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].u8FifoDataSz += u8Len;
	} else if (NULL != pvBuf) {
		u32	dwLowSize = 0, dwHighSize = 0;
		u64	u8AFifoFileSz = 0;

		dwLowSize =
			GetFileSize(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hFifoFile,
			(u32 *)(&dwHighSize));
		if (0xFFFFFFFF == dwLowSize) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get SPFIFO File's ")
				TEXT("size 2, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		u8AFifoFileSz = (u64)((((u64)dwHighSize) << 32) | dwLowSize);

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		u4CurUID = GetStmUIDByType(pvSptHdl, SPT_DATA_SP);

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Mem2Fifo -- Len: 0x%llx, TxUID: %d, CurTxUID: %d, BeforeDma's FifoDataSz: 0x%llx, AFifoFileSz: 0x%llx\r\n",
			u8Len, (s32)u4TxUID, (s32)u4CurUID,
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].u8FifoDataSz, u8AFifoFileSz);

		if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write SPDmainfo File,")
				TEXT(" err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
		{
			u64 u8Idx = 0;
			u8  *pu1Bytes = (u8 *)pvBuf;
			u8  au1Byte[4] = {0};
			u8  u1Char = 0;

			for (u8Idx = 0; u8Idx < u8Len; u8Idx++, pu1Bytes++) {
				u1Char = (((*pu1Bytes) >> 4) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[0] = (char)u1Char;

				u1Char = ((*pu1Bytes) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[1] = (char)u1Char;
				au1Byte[2] = ',';
				au1Byte[3] = ' ';
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile,
					au1Byte, 4, &dwWriteBytes, NULL)) {
					#ifdef __linux__
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%lld] to SPDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#else
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's")
						TEXT(" byte[%I64d] to SPDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#endif /* #ifdef __linux__ */
				}
				if ((u8Idx % 10) == 9) {
					au1Byte[0] = '\r';
					au1Byte[1] = '\n';
				}
				if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile,
					au1Byte, 2, &dwWriteBytes, NULL)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write \\r\\n")
						TEXT(" to SPDma file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				}
			}
			au1Byte[0] = '\r';
			au1Byte[1] = '\n';
			if (!WriteFile(g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].hDmaInfoFile,
				au1Byte, 2, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write \\r\\n to ")
					TEXT("SPDma file 4, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		}

		if (u4CurUID == u4TxUID)
			g_rDmxDumpMan.arDmxStms[DMX_CLI_STM_SP].u8FifoDataSz += u8Len;
	} else
		DMX_ASSERT(FALSE);

	DMXDUMPUNLOCK();

}

bool DmxCreateDumpPbbufFile(u64 u8SrcOfst, char *wszFilePrev)
{
#ifndef __linux__
	char wszDirName[100] = {0};
#endif /* __linux__ */
	char wszFileName[150] = {0};
	WIN32_FIND_DATA rData;
	char *wszPath = NULL;
	HANDLE	hFile;
	u32 u4Idx = 0;
	bool  fgCreateSucc = FALSE;

	DmxCloseDumpPbbufFile();

	u4Idx = 1;
	fgCreateSucc = FALSE;
	while (u4Idx < 3) {
		if (u4Idx == 1)
			wszPath = DMX_DUMP_FILE_PATH_1;
		if (u4Idx == 2)
			wszPath = DMX_DUMP_FILE_PATH_2;

		hFile = FindFirstFile(wszPath, &rData);
		if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
			FindClose(hFile);
		else {
			u4Idx++;
			continue;
		}

#ifdef __linux__
		sprintf(wszFileName, "%sPBBUF_%s_%lld", wszPath,
			wszFilePrev, u8SrcOfst);
#else
		vsnprintf(wszDirName, 100 * sizeof(char), TEXT("%sPBBUF"), wszPath);
		hFile = FindFirstFile(wszDirName, &rData);
		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(0 == (FILE_ATTRIBUTE_DIRECTORY & rData.dwFileAttributes))) {
			if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
				FindClose(hFile);

			if (!CreateDirectory(wszDirName, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create PBBUF Dump Directory(%s)Failed!\r\n"),
					wszDirName);
				return FALSE;
			}
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create PBBUF Dump Directory(%s) Success!\r\n"),
					wszDirName);
		} else
			FindClose(hFile);

		mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
		#ifdef __linux__
		vsnprintf(wszFileName, 150 * sizeof(char),
			TEXT("%s\\%s_%I64d"), wszDirName, wszFilePrev, u8SrcOfst);
		#else
		vsnprintf(wszFileName, 150 * sizeof(char), TEXT("%s\\%s_%lld"),
			wszDirName, wszFilePrev, u8SrcOfst);
		#endif /* #ifdef __linux__ */
#endif /* __linux__ */

		g_rDmxDumpMan.hPbbufFifoFile = CreateDumpFile(wszFileName,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (((HANDLE)INVALID_HANDLE_VALUE == g_rDmxDumpMan.hPbbufFifoFile) ||
			(NULL == g_rDmxDumpMan.hPbbufFifoFile)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create Pbbuf Dump File %s fail!, err: %d\r\n"),
				wszFileName, DMX_GET_LASTERR);
			u4Idx++;
			continue;
		}
		break;
	}

	if (u4Idx >= 3)
		return FALSE;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create Pbbuf Dump File %s success!\r\n"),
		wszFileName);

	return TRUE;
}

void DmxCloseDumpPbbufFile(void)
{
	if (((HANDLE)INVALID_HANDLE_VALUE != g_rDmxDumpMan.hPbbufFifoFile) &&
		(NULL != g_rDmxDumpMan.hPbbufFifoFile)) {
		CloseDumpFile(g_rDmxDumpMan.hPbbufFifoFile);
		g_rDmxDumpMan.hPbbufFifoFile = (HANDLE)INVALID_HANDLE_VALUE;

		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close PBBUF Dump Files!\r\n"));
	}
}

void DmxDumpPbbufData(u64 u8FileOfst, void *pvBuf, u64 u8Len)
{
	u32	dwWriteBytes;

	if (((HANDLE)INVALID_HANDLE_VALUE == g_rDmxDumpMan.hPbbufFifoFile) ||
		(NULL == g_rDmxDumpMan.hPbbufFifoFile))
		return;

	DMXDUMPLOCK();

	if (NULL != pvBuf) {
		u32	dwLowSize = 0, dwHighSize = 0;
		u64	u8SPFifoFileSz = 0;

		dwLowSize = GetFileSize(g_rDmxDumpMan.hPbbufFifoFile,
			(u32 *)(&dwHighSize));
		if (0xFFFFFFFF == dwLowSize) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Get Pbbuf File's size 1, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			DMXDUMPUNLOCK();
			return;
		}

		u8SPFifoFileSz = (u64)((((u64)dwHighSize) << 32) | dwLowSize);

		mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

		sprintf(g_rDmxDumpMan.szDumpInfo,
			"Pbbuf -- FileOfst: 0x%llx, Len: 0x%llx, PbbufSlotHandle: 0x%x\r\n",
			u8FileOfst, u8Len, (u32)pvBuf);

		if (!WriteFile(g_rDmxDumpMan.hPbbufFifoFile,
			(void *)(g_rDmxDumpMan.szDumpInfo),
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write Pbbuf File, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	{
			u64 u8Idx = 0;
			u8  *pu1Bytes = (u8 *)pvBuf;
			u8  au1Byte[4] = {0};
			u8  u1Char = 0;

			for (u8Idx = 0; u8Idx < u8Len; u8Idx++, pu1Bytes++) {
				u1Char = (((*pu1Bytes) >> 4) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[0] = (char)u1Char;

				u1Char = ((*pu1Bytes) & 0x0F);
				if (u1Char < 0x0A)
					u1Char = u1Char + '0';
				else
					u1Char = u1Char - 0x0A + 'A';

				au1Byte[1] = (char)u1Char;
				au1Byte[2] = ',';
				au1Byte[3] = ' ';
				if (!WriteFile(g_rDmxDumpMan.hPbbufFifoFile, au1Byte,
					4, &dwWriteBytes, NULL)) {
					#ifdef __linux__
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's ")
						TEXT("byte[%lld] to Pbbuf file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#else
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write pvBuf's ")
						TEXT(" byte[%I64d] to Pbbuf file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, u8Idx, DMX_GET_LASTERR);
					#endif /* #ifdef __linux__ */
				}
				if ((u8Idx % 10) == 9) {
					au1Byte[0] = '\r';
					au1Byte[1] = '\n';
				}
				if (!WriteFile(g_rDmxDumpMan.hPbbufFifoFile, au1Byte,
					2, &dwWriteBytes, NULL)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[DUMP] %s line %d failed in write \\r\\n")
						TEXT(" to Pbbuf file 3, err: %d!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
				}
			}
			au1Byte[0] = '\r';
			au1Byte[1] = '\n';
			if (!WriteFile(g_rDmxDumpMan.hPbbufFifoFile, au1Byte,
				2, &dwWriteBytes, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] %s line %d failed in write \\r\\n to Pbbuf ")
					TEXT("file 4, err: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
			}
		}
	}

	DMXDUMPUNLOCK();

}

bool DmxCreateDumpRspFile(u32 u4InstId)
{
#ifndef __linux__
	char wszDirName[100] = {0};
#endif /* __linux__ */
	char wszFileName[150] = {0};
	WIN32_FIND_DATA rData;
	char *wszPath = NULL;
	HANDLE	hFile;
	u32 u4Idx = 0;
	bool  fgCreateSucc = FALSE;

	if (u4InstId >= DMX_MAX_SPT_INST_CNT) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid args (u4InstId: %d)\r\n"),
			DMX_FUNC_NAME, u4InstId);
		return FALSE;
	}

	DmxCloseDumpPbbufFile();

	u4Idx = 1;
	fgCreateSucc = FALSE;
	while (u4Idx < 3) {
		if (u4Idx == 1)
			wszPath = DMX_DUMP_FILE_PATH_1;
		if (u4Idx == 2)
			wszPath = DMX_DUMP_FILE_PATH_2;

		hFile = FindFirstFile(wszPath, &rData);
		if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
			FindClose(hFile);
		else {
			u4Idx++;
			continue;
		}

#ifdef __linux__
	sprintf(wszFileName, "%s_Rsp_%d_Inst%d", wszPath,
		(s32)(g_rDmxDumpMan.u4RspDumpFileCnt), (s32)u4InstId);
#else
		vsnprintf(wszDirName, 100 * sizeof(char), TEXT("%sRsp"), wszPath);
		hFile = FindFirstFile(wszDirName, &rData);
		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(0 == (FILE_ATTRIBUTE_DIRECTORY & rData.dwFileAttributes))) {
			if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
				FindClose(hFile);

			if (!CreateDirectory(wszDirName, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create Resplitter Dump Directory(%s)Failed!\r\n"),
					wszDirName);
				return FALSE;
			}
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create Resplitter Dump Directory(%s) Success!\r\n"),
				wszDirName);
		} else
			FindClose(hFile);

		mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
		vsnprintf(wszFileName, 150 * sizeof(char),
			TEXT("%s\\Rsp_%d_Inst%d"), wszDirName,
			g_rDmxDumpMan.u4RspDumpFileCnt, u4InstId);
#endif /* __linux__ */

		if (((HANDLE)INVALID_HANDLE_VALUE ==
			g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]) ||
			(NULL == g_rDmxDumpMan.hRspSampleHdrFile[u4InstId])) {
			g_rDmxDumpMan.hRspSampleHdrFile[u4InstId] = CreateDumpFile(wszFileName,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_WRITE | FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
			if (((HANDLE)INVALID_HANDLE_VALUE ==
				g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]) ||
				(NULL == g_rDmxDumpMan.hRspSampleHdrFile[u4InstId])) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create Resplitter Dump File %s fail!, err: %d\r\n"),
					wszFileName, DMX_GET_LASTERR);
				u4Idx++;
				continue;
			}
		}
		break;
	}

	if (u4Idx >= 3)
		return FALSE;

	g_rDmxDumpMan.u4RspDumpFileCnt++;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create Resplitter Dump File %s success!\r\n"),
		wszFileName);

	return TRUE;
}

void DmxCloseDumpRspFile(u32 u4InstId)
{
	if (u4InstId >= DMX_MAX_SPT_INST_CNT) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid args (u4InstId: %d)\r\n"),
			DMX_FUNC_NAME, u4InstId);
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE !=
		g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]) &&
		(NULL != g_rDmxDumpMan.hRspSampleHdrFile[u4InstId])) {
		CloseDumpFile(g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]);
		g_rDmxDumpMan.hRspSampleHdrFile[u4InstId] = (HANDLE)INVALID_HANDLE_VALUE;

		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close Resplitter Dump Files (u4InstId: %d)!\r\n"),
			u4InstId);
	}
}

void DmxDumpRspData(u32 u4InstId, RSP_HDR_MEM_NODE *prNode,
	RSP_HDR_MEM_LIST *prHdrMemList, bool fgAdd)
{
	u32	dwWriteBytes;

	if (!g_rDmxCliMan.fgDumpRspInfo)
		return;

	if ((u4InstId >= DMX_MAX_SPT_INST_CNT) ||
		(NULL == prNode) ||
		(NULL == prHdrMemList)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s fail for invalid args (u4InstId: %d)\r\n"),
			DMX_FUNC_NAME, u4InstId);
		return;
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
			g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]) ||
		(NULL == g_rDmxDumpMan.hRspSampleHdrFile[u4InstId])) {
		if (!DmxCreateDumpRspFile(u4InstId)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[SPT] %s line %d fail in DmxCreateDumpRspFile, SptId(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4InstId);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
			g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]) ||
		(NULL == g_rDmxDumpMan.hRspSampleHdrFile[u4InstId]))
		return;

	DMXDUMPLOCK();

	mm_memset(g_rDmxDumpMan.szDumpInfo, 0, sizeof(g_rDmxDumpMan.szDumpInfo));

	sprintf(g_rDmxDumpMan.szDumpInfo,
		"%s -- NodeAddr: 0x%x, Addr: %p, Size: 0x%x, Prev: 0x%x, Next: 0x%x, Head: 0x%x, Tail: 0x%x, Rp: %p, Wp: %p, Sa: %p, Ea: %p\r\n",
		(fgAdd ? "ADD" : "Remove"),
		(u32)prNode, prNode->pvAddr, prNode->u4Size,
		(u32)(prNode->prPrev), (u32)(prNode->prNext),
		(u32)(prHdrMemList->prHead), (u32)(prHdrMemList->prTail),
		prHdrMemList->pvHdrRp,
		prHdrMemList->pvHdrWp, prHdrMemList->pvSa, prHdrMemList->pvEa);

	if (!WriteFile(g_rDmxDumpMan.hRspSampleHdrFile[u4InstId],
		(void *)g_rDmxDumpMan.szDumpInfo,
		strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] %s line %d failed in write Rsp Dump file ,")
			TEXT(" err: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
	}

	DMXDUMPUNLOCK();
}

MRESULT DmxDumpSptInfo(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = NULL;

	if (NULL == pvSptHdl)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("++++++++++++++++++++++++++++++++++++++++++++++\r\n"));

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("DmxInst(0x%x) Splitter(0x%x)'s current info as follow: \r\n"),
		prSpt->pvDmxInst, prSpt);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("SptState: %s(0x%x), SptTxState: %s(0x%x), StmHdlNum: 0x%x\r\n"),
		((prSpt->eSptState <= SPLITTER_STATE_RUNING) ? g_awszDmxSptStatus[prSpt->eSptState] : TEXT("UNKNOWN")),
		prSpt->eSptState,
		((prSpt->eSptTxState <= SPLITTER_TX_STATE_ERROR) ? g_awszDmxSptTxStatus[prSpt->eSptTxState] :
		TEXT("UNKNOWN")),
		prSpt->eSptTxState, prSpt->u4StmHandleNs);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("----------------------------------------------\r\n"));

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxDumpPsrCCInfo(void *pvPsrCC)
{
	PSR_CC	   *prPsrCC = NULL;
	u32	   u4Idx = 0;
	u32	   u4FtrType = 0;

	static const char * const _aCCStatus[] = {
		TEXT("[CCS_IDLE]"),
		TEXT("[CCS_INIT]"),
		TEXT("[CCS_TX]"),
		TEXT("[CCS_PAUSE]"),
		TEXT("[CCS_ABORT]")
	};

	static const char * const _aCCTxStatus[] = {
		TEXT("[TXS_WAIT_PBBUF]"),
		TEXT("[TXS_PBBUF_OK]"),
		TEXT("[TXS_WAIT_FIFO]"),
		TEXT("[TXS_WAIT_VFIFO_PTS_THRESHOLD]"),
		TEXT("[TXS_FIFO_OK]"),
		TEXT("[TXS_WAIT_DECRYPT]"),
		TEXT("[TXS_WAIT_HW]"),
		TEXT("[TXS_WAIT_IRQ_PROC]"),
		TEXT("[TXS_TXING]"),
		TEXT("[TXS_TX_OK]"),
		TEXT("[TXS_TX_JUMP]")
	};

	if (NULL == pvPsrCC) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[CLI] %s fail for invalid param --> ptrPsrCC == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrCC = (PSR_CC *)pvPsrCC;

	if (0 == (prPsrCC->u4Flag & CCF_USED)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[CLI] %s fail for unused PsrCC: 0x%x !! Psr flag: 0x%x\r\n"),
			DMX_FUNC_NAME, prPsrCC, prPsrCC->u4Flag);
		MM_RETURN(RET_DMX_OK);
	}

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("++++++++++++++++++++++++++++++++++++++++++++++\r\n"));

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("pvDmxInst(0x%x) pvSptHdl(0x%x) PsrCC(0x%x)'s current info as follow:\r\n"),
		prPsrCC->pvDmxInst, prPsrCC->pvSptHdl, prPsrCC);

	u4FtrType = ((NULL != prPsrCC->pvActFilter) ? (((PSR_FILTER *)(prPsrCC->pvActFilter))->eType) :
			SPT_DATA_UNDEFINE);
	if (u4FtrType >= MAX_SPT_DATA_TYPE_CNT) {
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("Param Error:  u4FtrType >= MAX_SPT_DATA_TYPE_CNT\r\n"));
		u4FtrType = SPT_DATA_UNDEFINE;
	}

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("pvActFilter --> Handle: 0x%x, eType: %d(%s)\r\n"),
		prPsrCC->pvActFilter, u4FtrType,
		DMX_SPTDATATYPE_STR(u4FtrType));

	#ifdef __linux__
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
	TEXT("eState: %s, eTxState: %s, Flag: 0x%x, PbbufSaOfst: %lld\r\n"),
		_aCCStatus[prPsrCC->eState], _aCCTxStatus[prPsrCC->eTxState],
		prPsrCC->u4Flag, prPsrCC->arPBBuf[0].u8SrcOffset);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("AVStmType: 0x%x, AVStmPlayFlag: 0x%x\r\n"),
		prPsrCC->u4AVStmFlags, prPsrCC->u4AVStmPlayFlags);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("TxStartOfst: %lld, TxLen: 0x%llx\r\n"),
		prPsrCC->u8TxStartOffset,
		prPsrCC->u8TxLen);
	#else
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("eState: %s, eTxState: %s, Flag: 0x%x, PbbufSaOfst: %I64d\r\n"),
		_aCCStatus[prPsrCC->eState], _aCCTxStatus[prPsrCC->eTxState],
		prPsrCC->u4Flag, prPsrCC->arPBBuf[0].u8SrcOffset);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("AVStmType: 0x%x, AVStmPlayFlag: 0x%x\r\n"),
		prPsrCC->u4AVStmFlags, prPsrCC->u4AVStmPlayFlags);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("TxStartOfst: %I64d, TxLen: 0x%llx\r\n"),
		prPsrCC->u8TxStartOffset,
		prPsrCC->u8TxLen);
	#endif /* #ifdef __linux__ */
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("TxCurrSa: 0x%x, TxCurrOfst: 0x%llx, TxCurrLen: 0x%llx\r\n"),
		prPsrCC->ptrTxCurrSa,
		prPsrCC->u8TxCurrOffset,
		prPsrCC->u8TxCurrLen);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("++++++ DECRYPT MANGER INFO List As Follow:+++++++++++++ \r\n"));

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("eDecryptType: %d, eDataType: %d, fgHasDecrypted: %d, ")
		TEXT("pvInst: 0x%x, pvPrivData: 0x%x\r\n"),
		prPsrCC->rDecryptMan.eDecryptType,
		prPsrCC->rDecryptMan.eDataType,
		((DECRYPT_UNCOMPLETE != prPsrCC->rDecryptMan.eStatus) ? 1 : 0),
		prPsrCC->rDecryptMan.pvInst,
		prPsrCC->rDecryptMan.pvPrivData);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("u8DecryptStOft: 0x%llx, u4DecryptLen: 0x%x, u4AlignSize: %d\r\n"),
		prPsrCC->rDecryptMan.u8DecryptStOft,
		prPsrCC->rDecryptMan.u4DecryptLen,
		prPsrCC->rDecryptMan.u4AlignSize);
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("ptrTxMemAddr: 0x%x, u4TxMemSize: 0x%x, ptrTxMemWPtr: 0x%x, ")
		TEXT("ptrTxMemRPtr: 0x%x\r\n"),
		prPsrCC->rDecryptMan.ptrTxMemAddr,
		prPsrCC->rDecryptMan.u4TxMemSize,
		prPsrCC->rDecryptMan.ptrTxMemWPtr,
		prPsrCC->rDecryptMan.ptrTxMemRPtr);

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("------ DECRYPT MANGER INFO List END -------------------- \r\n"));

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("CLI] fgUseCmdQ: %d, CmdEntryBufSa: 0x%x, CmdCnt: %d, ")
		TEXT("u2CurTxRngSIdx: %d, u2CurTxRngEIdx: %d\r\n"),
		prPsrCC->fgUseCmdQ, prPsrCC->u4CmdQTxEntryBuffer, prPsrCC->u2TxEntryCnt,
		prPsrCC->rCmdQTxInf.u2CurTxRngSIdx,
		prPsrCC->rCmdQTxInf.u2CurTxRngEIdx);

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[CLI] fgChkedAndWaitTx: %d, u4CurTxRngSIdxOfst: %d, ")
		TEXT("u4CurTxRngSIdxLen: %d\r\n"),
		prPsrCC->fgChkedAndWaitTx, prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst,
		prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen);

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[CLI] u4CurTxRngEIdxOfst: %d, u4CurTxRngEIdxLen: %d\r\n"),
	   prPsrCC->rCmdQTxInf.u4CurTxRngEIdxOfst,
	   prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen);

	#ifdef __linux__
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
	TEXT("[CLI] u4CurTxRngEIdxRmnLen: %d, u8RmnTotalRealTxLen: %lld\r\n"),
		prPsrCC->rCmdQTxInf.u4CurTxRngEIdxRmnLen,
		prPsrCC->rCmdQTxInf.u8RmnTotalRealTxLen);
	#else
	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
	TEXT("[CLI] u4CurTxRngEIdxRmnLen: %d, u8RmnTotalRealTxLen: %I64d\r\n"),
		prPsrCC->rCmdQTxInf.u4CurTxRngEIdxRmnLen,
		prPsrCC->rCmdQTxInf.u8RmnTotalRealTxLen);
	#endif /* #ifdef __linux__ */
	{
		DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;

		prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->u4CmdQTxEntryBuffer);
		for (u4Idx = 0; u4Idx < prPsrCC->u2TxEntryCnt; u4Idx++, prTxEntry++) {
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] CmdIdx(%d) --> SkipLen: %d, PayloadLen: %d\r\n"),
				u4Idx, prTxEntry->u4TxOfst, prTxEntry->u4TxLen);
		}
	}

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[CLI] Pbbuf Cache TxPbbufIdx: %d\r\n"),
		prPsrCC->u4TxPBBufIdx);

	for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
		DMX_READ_BUFFER *prReadBuf = &(prPsrCC->arPBBuf[u4Idx]);
		#ifdef __linux__
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[CLI] arPBBuf[%d]-- SrcOfst: %lld, DataSz: 0x%x,")
			TEXT(" PlayOfst: 0x%x, PlaySz: 0x%x, ptrSlot: 0x%x\r\n"),
			u4Idx,
			prReadBuf->u8SrcOffset,
			prReadBuf->u4DataSize,
			prReadBuf->u4PlayOffset,
			prReadBuf->u4PlaySize,
			prReadBuf->pvSlot);
		#else
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[CLI] arPBBuf[%d]-- SrcOfst: %I64d, DataSz: 0x%x,")
			TEXT(" PlayOfst: 0x%x, PlaySz: 0x%x, ptrSlot: 0x%x\r\n"),
			u4Idx,
			prReadBuf->u8SrcOffset,
			prReadBuf->u4DataSize,
			prReadBuf->u4PlayOffset,
			prReadBuf->u4PlaySize,
			prReadBuf->pvSlot);
		#endif /* #ifdef __linux__ */
	}

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("--------------------------------------------\r\n"));

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxDumpPsrFilterInfo(void *pvPsrFtr)
{
	PSR_FILTER *prPsrFtr = NULL;
	u32	u4AUFreeCnt = 0;
	u32	u4FifoSize	= 0;
	u32	u4FifoAvailSize = 0;
	u32	u4AUCnt = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvPsrFtr) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[CLI] %s fail for invalid param --> ptrPsrFtr == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrFtr = (PSR_FILTER *)pvPsrFtr;

	if ((prPsrFtr->u4Flag) & FF_USED) {
		if ((SPT_DATA_V == prPsrFtr->eType)  ||
			(SPT_DATA_A == prPsrFtr->eType)  ||
			(SPT_DATA_SP == prPsrFtr->eType) ||
			(SPT_DATA_SECTION == prPsrFtr->eType)) {
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("+++++++++++++++++++++++++++++++++++++++++")
				TEXT("+++++++++++++++++++++++++++++++++++++\r\n"));
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] DmxInst(0x%p), PsrCC(0x%p), PsrFilter(0x%p)'s")
				TEXT(" current info as follow: \r\n"),
				prPsrFtr->pvDmxInst, prPsrFtr->pvPsrCC, prPsrFtr);

			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] eType: %d(%s), Flag: 0x%x, StmType: 0x%x, StmUID: 0x%x\r\n"),
				prPsrFtr->eType,
				DMX_SPTDATATYPE_STR(prPsrFtr->eType),
				prPsrFtr->u4Flag, prPsrFtr->u4StmType, prPsrFtr->u4StmUID);
			#ifdef __linux__
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] PsrCCHdl: 0x%x, TxCurOst: %lld, u8TotalAULen: %lld,")
				TEXT(" u8CurAULen: %lld\r\n"),
				prPsrFtr->pvPsrCC,
				prPsrFtr->u8TxCurrOffset,
				prPsrFtr->u8TotalAULen,
				prPsrFtr->u8CurAULen);
			#else
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] PsrCCHdl: 0x%x, TxCurOst: %I64d,")
				TEXT(" u8TotalAULen: %I64d, u8CurAULen: %I64d\r\n"),
				prPsrFtr->ptrPsrCC,
				prPsrFtr->u8TxCurrOffset,
				prPsrFtr->u8TotalAULen,
				prPsrFtr->u8CurAULen);
			#endif /* #ifdef __linux__ */
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] fgAUCtrlByLen: %d, fgAUCtrlByEnd: %d, fgAUEnd: %d,")
				TEXT(" u8HWDevId: %u\r\n"),
				(prPsrFtr->fgAUCtrlByLen ? 1 : 0),
				(prPsrFtr->fgAUCtrlByEnd ? 1 : 0),
				(prPsrFtr->fgAUEnd ? 1 : 0),
				prPsrFtr->ucHwDevId);

			mrRet = ESM_AUTableGetFreeCount(prPsrFtr->u4ESIH, &u4AUFreeCnt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[CLI] %s line %d fail in")
					TEXT(" ESM_AUTableGetFreeCount(u4Handle: 0x%x),")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
				MM_RETURN(mrRet);
			}
			mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoAvailSize);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[CLI] %s line %d fail in")
					TEXT(" ESM_FifoGetAvailDataSize(u4Handle: 0x%x),")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
				MM_RETURN(mrRet);
			}
			u4FifoSize = prPsrFtr->u4ESFifoSize;
			mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCnt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[CLI] %s line %d fail in")
					TEXT(" ESM_AUTableGetTotalCount(u4Handle: 0x%x),")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
				MM_RETURN(mrRet);
			}

			if ((0 < u4FifoSize) &&
				(0 < (u4AUCnt + u4AUFreeCnt))) {
				DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[CLI] fifo (Avail/Total) = %d/%d (%d%%),")
					TEXT(" au (Avail/Total): %d/%d (%d%%), ")
					TEXT("Fifo Reserve %d \r\n"),
					u4FifoAvailSize, u4FifoSize,
					u4FifoAvailSize * 100 / u4FifoSize,
					u4AUCnt, (u4AUCnt + u4AUFreeCnt),
					(u4AUCnt * 100 / (u4AUCnt + u4AUFreeCnt)),
					PSR_RESERVE_FIFO_SPACE);
			} else {
				DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[CLI] fifo - AvailSz: %d, TotalSz: %d,")
					TEXT(" AU -- AvailCnt: %d, FreeCnt: %d, ")
					TEXT("Fifo Reserve %d \r\n"),
					u4FifoAvailSize, u4FifoSize,
					u4AUCnt, u4AUFreeCnt,
					PSR_RESERVE_FIFO_SPACE);
			}

			ESM_PrintFifoInfo(prPsrFtr->u4ESIH);
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("---------------------------------------------")
				TEXT("----------------------------------------------\r\n"));
		}
	}
	MM_RETURN(RET_DMX_OK);
}

bool DmxCreateDumpFlowFile(char *wszVDirName)
{
#ifndef __linux__
	char wszDirName[100] = {0};
#endif /* __linux__ */
	char wszFileName[150] = {0};
	WIN32_FIND_DATA rData;
	char *wszPath = NULL;
	HANDLE	hFile;
	u32 u4Idx = 0;
	bool  fgCreateSucc = FALSE;

	DmxCloseDumpFlowFile();

	u4Idx = 1;
	fgCreateSucc = FALSE;
	while (u4Idx < 3) {
		if (u4Idx == 1)
			wszPath = DMX_DUMP_FILE_PATH_1;
		if (u4Idx == 2)
			wszPath = DMX_DUMP_FILE_PATH_2;

		hFile = FindFirstFile(wszPath, &rData);
		if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
			FindClose(hFile);
		else {
			u4Idx++;
			continue;
		}

#ifdef __linux__
		sprintf(wszFileName, TEXT("%s%s_Flow"), wszPath, wszVDirName);
#else
		vsnprintf(wszDirName, 100 * sizeof(char),
			TEXT("%s%s"), wszPath, wszVDirName);
		hFile = FindFirstFile(wszDirName, &rData);

		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(0 == (FILE_ATTRIBUTE_DIRECTORY & rData.dwFileAttributes))) {
			if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
				FindClose(hFile);

			if (!CreateDirectory(wszDirName, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create DMX Dump Flow Directory(%s)Failed!\r\n"),
					wszDirName);

				return FALSE;
			}
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create DMX Dump Flow Directory(%s) Success!\r\n"),
				wszDirName);
		} else
			FindClose(hFile);

		mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
		vsnprintf(wszFileName, 150 * sizeof(char),
			TEXT("%s\\Flow"), wszDirName);
#endif /* __linux__ */

		if (((HANDLE)INVALID_HANDLE_VALUE ==
			g_rDmxDumpMan.rFlowInfo.hFlowFile) ||
			(NULL == g_rDmxDumpMan.rFlowInfo.hFlowFile)) {
			g_rDmxDumpMan.rFlowInfo.hFlowFile = CreateDumpFile(wszFileName,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_WRITE | FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
			if (((HANDLE)INVALID_HANDLE_VALUE ==
				g_rDmxDumpMan.rFlowInfo.hFlowFile) ||
				(NULL == g_rDmxDumpMan.rFlowInfo.hFlowFile)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create DMX Dump Flow File %s fail!, err: %d\r\n"),
					wszFileName, DMX_GET_LASTERR);
				u4Idx++;
				continue;
			}
		}

		break;
	}

	if (u4Idx >= 3)
		return FALSE;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create DMX Dump Flow File %s success!\r\n"),
		wszFileName);

	return TRUE;
}

void DmxCloseDumpFlowFile(void)
{
	if (((HANDLE)INVALID_HANDLE_VALUE != g_rDmxDumpMan.rFlowInfo.hFlowFile) &&
		(NULL != g_rDmxDumpMan.rFlowInfo.hFlowFile)) {
		CloseDumpFile(g_rDmxDumpMan.rFlowInfo.hFlowFile);
		g_rDmxDumpMan.rFlowInfo.hFlowFile = (HANDLE)INVALID_HANDLE_VALUE;
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close DMX Dump Flow File!\r\n"));
	}
}

void DmxDumpFlow(E_DMX_OPER_TYPE_T eOperType,
	DMX_DUMP_FLOW_OPER_INFO_T *prOperInfo)
{
	u32	dwWriteBytes;

	if ((!g_rDmxCliMan.fgDumpFlow) ||
		(DMX_MEMCHECK_START_VAL != g_rDmxCliMan.u4CheckStart) ||
		(DMX_MEMCHECK_END_VAL != g_rDmxCliMan.u4CheckEnd) ||
		(NULL == prOperInfo))
		return;

	if (((HANDLE)INVALID_HANDLE_VALUE == g_rDmxDumpMan.rFlowInfo.hFlowFile) ||
		(NULL == g_rDmxDumpMan.rFlowInfo.hFlowFile)) {
		char wszDirName[DMX_MAX_PATH_LEN] = {0};
#ifdef __linux__
		snprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), "FLOW%d",
			g_rDmxDumpMan.rFlowInfo.u4FlowDumpCnt);
#else
		vsnprintf(wszDirName, DMX_MAX_PATH_LEN * sizeof(char), L"FLOW%d",
			g_rDmxDumpMan.rFlowInfo.u4FlowDumpCnt);
#endif /* __linux__ */
		if (!DmxCreateDumpFlowFile(wszDirName)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[CLI] ++++++++ %s fail for create flow dump files\r\n"),
				DMX_FUNC_NAME);
		}
	}

	if (((HANDLE)INVALID_HANDLE_VALUE ==
		g_rDmxDumpMan.rFlowInfo.hFlowFile) ||
		(NULL == g_rDmxDumpMan.rFlowInfo.hFlowFile)) {
		return;
	}

	DMXDUMPLOCK();

	if (DMX_OPER_PSR_ON == eOperType) {
		g_rDmxDumpMan.rFlowInfo.u4TickCount = GetTickCount();
		sprintf(g_rDmxDumpMan.szDumpInfo, "[0] PSR_ON--> pvSptHdl: 0x%x\r\n",
			(u32)(prOperInfo->pvSptHdl));

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_PSR_PAUSE == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo, "[%d] PSR_PAUSE--> pvSptHdl: 0x%x\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl));

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_PSR_OFF == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo, "[%d] PSR_OFF--> pvSptHdl: 0x%x\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl));

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_SYNC == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] SyncPbbuf--> pvSptHdl: 0x%x, Ofst: 0x%llx, Len: 0x%llx\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl), prOperInfo->unFlow.rSync.u8FileOfst,
			prOperInfo->unFlow.rSync.u8Len);

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_SYNCEX == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] SyncPbbufEx--> pvSptHdl: 0x%x, Ofst: 0x%llx, Len: 0x%llx\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl), prOperInfo->unFlow.rSync.u8FileOfst,
			prOperInfo->unFlow.rSync.u8Len);

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_SW_START_PTX == eOperType) {
		if (NULL == prOperInfo->unFlow.rDma.pvBuf) {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] SW_START_DMA--> pvSptHdl: 0x%x, StmType: %s, (PBBUF), Ofst: 0x%llx, Len: 0x%llx\r\n",
				 (s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
				 (u32)(prOperInfo->pvSptHdl),
				 DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDma.u4StmType),
				 prOperInfo->unFlow.rDma.u8FileOfst, prOperInfo->unFlow.rDma.u8Len);
		} else {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] SW_START_DMA--> pvSptHdl: 0x%x, StmType: %s, (BUF), Ofst: 0, Len: 0x%llx\r\n",
				 (s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
				 (u32)(prOperInfo->pvSptHdl),
				DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDma.u4StmType),
				 prOperInfo->unFlow.rDma.u8Len);
		}

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_SET_DECRYPT_INFO == eOperType) {
		if (0 < prOperInfo->unFlow.rDecryptSetting.u4DecryptLen) {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] SET_DECRYPT--> pvSptHdl: 0x%x, TURN_ON, (Type: %s), StOfst: 0x%llx, Len: 0x%x, FrameKeyIdx: 0x%02x\r\n",
				(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
				(u32)(prOperInfo->pvSptHdl),
				DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDecryptSetting.eDataType),
				prOperInfo->unFlow.rDecryptSetting.u8DecryptOfst,
				prOperInfo->unFlow.rDecryptSetting.u4DecryptLen,
				prOperInfo->unFlow.rDecryptSetting.u2FrameKeyIdx);
		} else {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] SET_DECRYPT--> pvSptHdl: 0x%x, TURN_OFF\r\n",
				(s32)((u32)GetTickCount() -
					g_rDmxDumpMan.rFlowInfo.u4TickCount),
				(u32)(prOperInfo->pvSptHdl));
		}
		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_COMPOSE_DECRYPT_DATA == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] COMPOSE_DECRYPT--> pvSptHdl: 0x%x, StmType: %s, (StOfst: 0x%llx, Len: 0x%x), CurSlot(StOfst: 0x%llx, DataSz: 0x%x), Pos: %d\r\n",
			 (s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			 (u32)(prOperInfo->pvSptHdl),
			DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rComposeDecrypt.u4StmType),
			 prOperInfo->unFlow.rComposeDecrypt.u8FileOfst,
			 prOperInfo->unFlow.rComposeDecrypt.u4Len,
			 prOperInfo->unFlow.rComposeDecrypt.u8SlotSrcOfst,
			prOperInfo->unFlow.rComposeDecrypt.u4SlotDataSz,
			 prOperInfo->unFlow.rComposeDecrypt.ePosition);

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_DECRYPT == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] DECRYPT--> pvSptHdl: 0x%x, StmType: %s, Ofst: 0x%llx, Len: 0x%x\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl),
			DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDecrypt.u4StmType),
			prOperInfo->unFlow.rDecrypt.u8FileOfst,
			prOperInfo->unFlow.rDecrypt.u4Len);

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_HW_PTX == eOperType) {
		if (NULL == prOperInfo->unFlow.rDma.pvBuf) {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] HW_PTX--> pvSptHdl: 0x%x, StmType: %s, (PBBUF), Ofst: 0x%llx, Len: 0x%llx\r\n",
				(s32)((u32)GetTickCount() -
				g_rDmxDumpMan.rFlowInfo.u4TickCount),
				(u32)(prOperInfo->pvSptHdl),
				DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDma.u4StmType),
				prOperInfo->unFlow.rDma.u8FileOfst,
				prOperInfo->unFlow.rDma.u8Len);
		} else {
			sprintf(g_rDmxDumpMan.szDumpInfo,
				"[%d] HW_PTX--> pvSptHdl: 0x%x, StmType: %s, (BUF), Ofst: 0, Len: 0x%llx\r\n",
				(s32)((u32)GetTickCount() -
				g_rDmxDumpMan.rFlowInfo.u4TickCount),
				(u32)(prOperInfo->pvSptHdl),
				DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rDma.u4StmType),
				prOperInfo->unFlow.rDma.u8Len);
		}

		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_HW_PTX_DONE == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] HW_PTX_Done-> pvSptHdl: 0x%x\r\n",
			(s32)((u32)GetTickCount() - g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl));
		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_FILLAU == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] FillAU-> pvSptHdl: 0x%x, StmType: %s, AUIdx: %d\r\n",
			(s32)((u32)GetTickCount() -
				g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl),
			DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rFillAU.u4StmType),
			(s32)(prOperInfo->unFlow.rFillAU.u4AUIdx));
		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file,")
				TEXT(" err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_SW_PTX_DONE == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] SW_PTX_Done-> pvSptHdl: 0x%x\r\n",
			(s32)((u32)GetTickCount() -
				g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl));
		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else if (DMX_OPER_GETAU == eOperType) {
		sprintf(g_rDmxDumpMan.szDumpInfo,
			"[%d] GETAU-> pvSptHdl: 0x%x, StmType: %s, AUIdx: %d\r\n",
			(s32)((u32)GetTickCount() -
				g_rDmxDumpMan.rFlowInfo.u4TickCount),
			(u32)(prOperInfo->pvSptHdl),
			DMX_SPTDATATYPE_STR(prOperInfo->unFlow.rGetAU.u4StmType),
			(s32)(prOperInfo->unFlow.rGetAU.u4AUIdx));
		if (!WriteFile(g_rDmxDumpMan.rFlowInfo.hFlowFile,
			(void *)g_rDmxDumpMan.szDumpInfo,
			strlen(g_rDmxDumpMan.szDumpInfo), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in Dmx Dump Flow file, ")
				TEXT("err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	}

	DMXDUMPUNLOCK();

}

bool DmxCreateDumpFile(char *wszVDirName, HANDLE *phFile)
{
#ifndef __linux__
	char wszDirName[100] = {0};
#endif /* __linux__ */
	char wszFileName[150] = {0};
	WIN32_FIND_DATA rData;
	char *wszPath = NULL;
	HANDLE hFile = NULL;
	u32 u4Idx = 1;
	bool   fgCreateSucc = FALSE;

	*phFile = NULL;

	u4Idx = 1;
	fgCreateSucc = FALSE;
	while (u4Idx < 3) {
		if (u4Idx == 1)
			wszPath = DMX_DUMP_FILE_PATH_1;
		if (u4Idx == 2)
			wszPath = DMX_DUMP_FILE_PATH_2;

		hFile = FindFirstFile(wszPath, &rData);
		if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
			FindClose(hFile);
		else {
			u4Idx++;
			continue;
		}

#ifdef __linux__
		sprintf(wszFileName, "%s%s_FifoData", wszPath, wszVDirName);
#else
		vsnprintf(wszDirName, 100 * sizeof(char),
		TEXT("%s%s"), wszPath, wszVDirName);
		hFile = FindFirstFile(wszDirName, &rData);

		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(0 == (FILE_ATTRIBUTE_DIRECTORY & rData.dwFileAttributes))) {
			if ((HANDLE)INVALID_HANDLE_VALUE != hFile)
				FindClose(hFile);

			if (!CreateDirectory(wszDirName, NULL)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("[DUMP] Create Dump Directory(%s)Failed!\r\n"),
					wszDirName);

				return FALSE;
			}
			DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create Dump Directory(%s) Success!\r\n"),
				wszDirName);
		} else
			FindClose(hFile);

		mm_memset((void *)wszFileName, 0, sizeof(char) * 150);
		vsnprintf(wszFileName, 150 * sizeof(char),
			TEXT("%s\\FifoData"), wszDirName);
#endif /* __linux__ */

		hFile = CreateDumpFile(wszFileName,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
			(NULL == hFile)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] Create FifoData File %s fail!, err: %d\r\n"),
				wszFileName, DMX_GET_LASTERR);
			u4Idx++;
			continue;
		}

		break;
	}

	if (u4Idx >= 3)
		return FALSE;

	DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DUMP] Create FifoData File %s success!\r\n"),
		wszFileName);

	*phFile = hFile;

	return TRUE;
}

void DmxCloseDumpFile(HANDLE hFile)
{
	if (((HANDLE)INVALID_HANDLE_VALUE != hFile) &&
		(NULL != hFile)) {
		CloseDumpFile(hFile);
		hFile = NULL;
		DmxLogI(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DUMP] Close Fifo Dump Files!\r\n"));
	}
}

void DmxDumpData(HANDLE hFile, u32 u4FifoRP,
	u32 u4FifoWP, u32 u4FifoSa, u32 u4FifoEa)
{
	u32	dwWriteBytes = 0;

	if (((HANDLE)INVALID_HANDLE_VALUE == hFile) ||
		(NULL == hFile))
		return;

	if (u4FifoRP < u4FifoWP) {
		if (!WriteFile(hFile, (void *)(u4FifoRP),
			(u4FifoWP - u4FifoRP), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write FIFO File, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	} else {
		if (!WriteFile(hFile, (void *)(u4FifoRP),
			(u4FifoEa - u4FifoRP), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write FIFO File, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
		if (!WriteFile(hFile, (void *)u4FifoSa,
			(u4FifoWP - u4FifoSa), &dwWriteBytes, NULL)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[DUMP] %s line %d failed in write FIFO File, err: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}
	}

}

void DMXDumpRegisters(u32 u4StartAddr, u32 u4RegCnt)
{
	DMX_HAL_PVR_DumpRegisters(u4StartAddr, u4RegCnt);
}


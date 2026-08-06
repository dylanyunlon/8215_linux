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

#ifdef __linux__
#include "windows.h"
#endif
#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"
#include "x_typedef.h"

#include <linux/types.h>

#include "x_vid_dec.h"
#include "ioctl_vdec.h"
#include "linux/string.h"
#ifndef __linux__
#include "stdlib.h"
#else
#include "winutil.h"
#include "linux/kernel.h"
#endif

#ifndef __linux__
#define VID_DEV_NAME             L"VDE1:"
#endif

VID_DEC_CLI_CFG g_rVdecCliCfg;

#ifndef __linux__

static HANDLE g_hVdecCliCmdIOCTL = INVALID_HANDLE_VALUE;
static bool _CLI_VDEC_OpenDrv(void);
#define CLI_VDEC_CHECK_DRV_HANDLE()      \
{                                        \
	if (!_CLI_VDEC_OpenDrv()) {          \
		return -1;                       \
	}                                    \
}

#define CLI_VDEC_CLOSE_DRV()                     \
{                                                \
	RETAILMSG(0, (TEXT("[VDEC_CLI] CLI_VDEC_CLOSE_DRV close vdec driver\r\n"))); \
	CloseHandle(g_hVdecCliCmdIOCTL);             \
	g_hVdecCliCmdIOCTL = INVALID_HANDLE_VALUE;   \
	return 0;                                    \
}

#define CLI_VDEC_IOCTL()                                                   \
{                                                                          \
	if (!DeviceIoControl(g_hVdecCliCmdIOCTL, VDEC_IOCTL_SET_CLI_CMD_INFO,  \
			&g_rVdecCliCfg, sizeof(g_rVdecCliCfg), NULL, 0, NULL, NULL)) { \
		RETAILMSG(1, (TEXT(": VDEC_IOCTL_SET_CLI_CMD_INFO err = %ld\r\n"), \
				GetLastError()));                                          \
	}                                                                      \
}

static bool _CLI_VDEC_OpenDrv(void)
{
	if (g_hVdecCliCmdIOCTL == INVALID_HANDLE_VALUE) {
		RETAILMSG(0, (TEXT("[VDEC_CLI] _CLI_VDEC_OpenDrv open vdec driver\r\n")));
		g_hVdecCliCmdIOCTL = CreateFile(DMX_DEV_NAME,
				GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
		if (INVALID_HANDLE_VALUE == g_hVdecCliCmdIOCTL) {
			RETAILMSG(1, (TEXT("[VDEC_CLI] CreateFile Failed: INVALID_HANDLE_VALUE\r\n")));
			return FALSE;
		}
	}
	return TRUE;
}

#else

#define _ttoi(a)   simple_strtol(a, NULL, 10)
#define CLI_VDEC_CHECK_DRV_HANDLE()
#define CLI_VDEC_CLOSE_DRV()
extern void _VidCliCmd(VID_DEC_CLI_TYPE eVidCli, u32 u4arg1, u32 u4arg2,
		u32 u4arg3, u32 u4arg4, const s8 **pfilename);
#define CLI_VDEC_IOCTL() _VidCliCmd(g_rVdecCliCfg.eVidCliType, \
				g_rVdecCliCfg.u4arg1, g_rVdecCliCfg.u4arg2,    \
				g_rVdecCliCfg.u4arg3, g_rVdecCliCfg.u4arg4,    \
				(const s8 **)g_rVdecCliCfg.ptParam)
#endif


static s32 _CLI_VidDumpAU(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	if (i4Argc > 3) {
		RETAILMSG(1, (TEXT("[Command]drv.vid.au [PicCnt]\r\n")));
		return -1;
	} else if (1 == i4Argc) {
		g_rVdecCliCfg.u4arg1 = 0;
		g_rVdecCliCfg.u4arg2 = (u32)-1;
	} else if (2 == i4Argc) {
		g_rVdecCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		g_rVdecCliCfg.u4arg2 = (u32)_ttoi(szArgv[1]);
	} else if (3 == i4Argc) {
		g_rVdecCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		g_rVdecCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
	}

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_DUMP_AU;

	CLI_VDEC_IOCTL();

	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidPrintPts(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	if (i4Argc != 1) {
		RETAILMSG(1, (TEXT("[Command]drv.vid.pts\r\n")));
		return -1;
	}

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_PRINT_PTS;
	CLI_VDEC_IOCTL();

	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidDumpSPS(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_DUMP_SPS;
	CLI_VDEC_IOCTL();

	CLI_VDEC_CLOSE_DRV();
}


static s32 _CLI_VidDumpPPS(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_DUMP_PPS;
	CLI_VDEC_IOCTL();

	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidDumpFrmBuf(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_DUMP_FRMBUF;

	CLI_VDEC_IOCTL();
	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidDumpYUV(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	if (i4Argc > 4) {
		RETAILMSG(1, (TEXT("[Command]drv.vid.yuv [0/1] [StartFrm] [EndFrm]\r\n")));
		return -1;
	} else if (2 == i4Argc) {
		g_rVdecCliCfg.u4arg2 = 0;
		g_rVdecCliCfg.u4arg3 = (u32)-1;
	} else if (3 == i4Argc) {
		g_rVdecCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
		g_rVdecCliCfg.u4arg3 = (u32)_ttoi(szArgv[2]);
	} else if (3 == i4Argc) {
		g_rVdecCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
		g_rVdecCliCfg.u4arg3 = (u32)_ttoi(szArgv[3]);
	}

	g_rVdecCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_DUMP_YUV;

	CLI_VDEC_IOCTL();
	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidQueryVdecState(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_QUERY_VDEC_STATE;

	CLI_VDEC_IOCTL();
	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidSetVdecLogLevel(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	if (i4Argc != 2) {
		RETAILMSG(1, (TEXT("[Command]drv.vid.loglevel [loglevel]\r\n")));
		return -1;
	}
	g_rVdecCliCfg.ptParam = *(void **)&szArgv[1];
	RETAILMSG(1, (TEXT("[Command]drv.vid.loglevel %s\r\n"),
			g_rVdecCliCfg.ptParam));

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_LOG_LEVEL;

	CLI_VDEC_IOCTL();
	CLI_VDEC_CLOSE_DRV();
}

static s32 _CLI_VidSetFileName(s32 i4Argc, const s8 **szArgv)
{
	CLI_VDEC_CHECK_DRV_HANDLE();

	if (i4Argc != 2) {
		RETAILMSG(1, (TEXT("[Command]drv.vid.filename [filename]\r\n")));
		return -1;
	}
	g_rVdecCliCfg.ptParam = *(void **)&szArgv[1];
	RETAILMSG(1, (TEXT("[Command]drv.vid.filename %s\r\n"),
			g_rVdecCliCfg.ptParam));

	g_rVdecCliCfg.eVidCliType =  VID_DEC_CLI_FILE_NAME;

	CLI_VDEC_IOCTL();
	CLI_VDEC_CLOSE_DRV();
}

CLI_EXEC_T _arVdecCmdTbl[] = {
	{TEXT("dumpau"), TEXT("drv.vid.dumpau [1/0]"), _CLI_VidDumpAU,
		NULL, TEXT("Dump VFIFO Data, [1:ON|0:OFF] [PicCnt]"), CLI_GUEST},
	{TEXT("filename"), TEXT("drv.vid.filename [FileName]"),
		_CLI_VidSetFileName,
		NULL, TEXT("Set File Name for Dumping"), CLI_GUEST},
	{TEXT("pts"), TEXT("drv.vid.pts"), _CLI_VidPrintPts,
		NULL, TEXT("Dump PTS Info, [1:ON|0:OFF]"), CLI_GUEST},
	{TEXT("sps"), TEXT("drv.vid.sps"), _CLI_VidDumpSPS,
		NULL, TEXT("Dump SPS Info, only for AVC Codec"), CLI_GUEST},
	{TEXT("pps"), TEXT("drv.vid.pps"), _CLI_VidDumpPPS,
		NULL, TEXT("Dump PPS Info, only for AVC Codec"), CLI_GUEST},
	{TEXT("frmbuf"), TEXT("drv.vid.frmbuf"), _CLI_VidDumpFrmBuf,
		NULL, TEXT("Dump FrmBuf, only for AVC Codec"), CLI_GUEST},
	{TEXT("dumpyuv"), TEXT("drv.vid.dumpyuv [1/0] [startfrm] [endfrm]"),
		_CLI_VidDumpYUV, NULL,
		TEXT("Dump YUV Data, [1:Display|0:Decode Order][Start][End]"),
		CLI_GUEST},
	{TEXT("state"), TEXT("drv.vid.state"), _CLI_VidQueryVdecState,
		NULL, TEXT("Query VideoDecoder State"), CLI_GUEST},
	{TEXT("loglevel"), TEXT("drv.vid.loglevel [Loglevel]"),
		_CLI_VidSetVdecLogLevel, NULL,
		TEXT("Set VideoDecoder Log Level, [loglevel: 'Debug','Trace', 'Error', 'Fatal']"),
		CLI_GUEST},
	/* last cli command record, NULL */
	{NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR}
};



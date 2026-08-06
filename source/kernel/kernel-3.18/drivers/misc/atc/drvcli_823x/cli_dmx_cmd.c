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
#include "x_stl_lib.h"
#include "x_typedef.h"

#include <linux/types.h>

#include "x_dmx.h"
#include "ioctl_dmx.h"

#include "winutil.h"
#include <linux/kernel.h>
#include <linux/ctype.h>
//#include "dmx_def.h"
//#include "dmx_cli.h"

#ifndef __linux__
#define DMX_DEV_NAME             L"DMX1:"
#endif

DMX_CLI_CFG g_rDmxCliCfg;

char *g_wszDmxCliLogLvl[4] = {
	TEXT("Debug Level"),
	TEXT("Warning Level"),
	TEXT("Error Level"),
	TEXT("Fatal Level")
};

static char *g_wszDmxCliStmName[4] = {
	TEXT("Video"),
	TEXT("Audio"),
	TEXT("SP"),
	TEXT("Section")
};

#define _ttoi(a)   simple_strtol(a, NULL, 10)
#define _ttoi16(a) simple_strtol(a, NULL, 16)
#define CLI_DMX_CHECK_DRV_HANDLE()
#define CLI_DMX_CLOSE_DRV()
#define CLI_DMX_IOCTL() _DmxCliCmd(g_rDmxCliCfg.eDmxCliType, \
				g_rDmxCliCfg.u4arg1, g_rDmxCliCfg.u4arg2, \
				g_rDmxCliCfg.u4arg3, g_rDmxCliCfg.u4arg4, \
				(const char **)g_rDmxCliCfg.ptParam)

extern void _DmxCliCmd(DMX_CLI_TYPE eDmxCliType, u32 arg1, u32 arg2,
				u32 arg3, u32 arg4, const char **pfilename);

/******************************************************************************
 *_CLI_DMXTurnOnLog
 * Function: Turn On Demuxer's Log
 * u4arg1 -- Turn On or Off
 * 1) =0. Turn off
 * 2) =1. Turn on
 * u4arg2 -- Log level ID, it should be the following value:
 * 1) =1. Debug Level
 * 2) =2. Warning Level
 * 3) =3. Error Level
 * 4) =4. Fatal Level
 * 5) =none. All Level Log
******************************************************************************/
static s32 _CLI_DMXTurnOnOffLog(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 5) {
		pr_info("[Command] drv.dmx.log [TurnOnOff] [loglvl] [Module] [ModuleLoglevel]\r\n");
		pr_info("[Command] TurnOnOff: 1-On, 0-Off\r\n");
		pr_info("[Command] loglvl: 0-debug, 1-Warning, 2-Trace, 3-Error\r\n");
		pr_info("[Command] Module: Cfa_Audio, Cfa_Ape, Cfa_Flv, Cfa_Rm, Cfa_Audin,");
		pr_info(" Cfa_Mkv, Cfa_Mpg, Cfa_Mp4, Cfa_Asf, Cfa_Ts, Cfa_Sub, Hw, Other, All\r\n");
		pr_info("[Command] ModuleLoglevel: 0 ~ 31, or -1, defined by Module\r\n");
	} else {
		char szModName[20];
		u32 u4Idx = 0;
		u32 u4ModNameLen = 0;
		char *pcChar = NULL;

		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		g_rDmxCliCfg.u4arg4 = (u32)_ttoi(szArgv[4]);

		pcChar = (char *)szArgv[2];
		if (isdigit(*pcChar)) {
			g_rDmxCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
		} else {
			memset((void *)szModName, 0, sizeof(szModName));
			u4ModNameLen = strlen(szArgv[2]);

			pcChar = (char *)szArgv[2];

			for (u4Idx = 0; u4Idx < u4ModNameLen; u4Idx++, pcChar++) {
				szModName[u4Idx] = toupper(*pcChar);
			}

			if (0 == strcmp(szModName, TEXT("D"))) {
				g_rDmxCliCfg.u4arg3 = 0;
			} else if (0 == strcmp(szModName, TEXT("W"))) {
				g_rDmxCliCfg.u4arg3 = 1;
			} else if (0 == strcmp(szModName, TEXT("T"))) {
				g_rDmxCliCfg.u4arg3 = 2;
			} else if (0 == strcmp(szModName, TEXT("E"))) {
				g_rDmxCliCfg.u4arg3 = 3;
			}
		}

		if (3 < g_rDmxCliCfg.u4arg2) {
			pr_info("[Command] drv.dmx.log [TurnOnOff] [loglvl 0-3 | 'D','W','T','E'] [Module] [ModuleLoglevel]\r\n");
			pr_info("[Command] TurnOnOff: 1-On, 0-Off\r\n");
			pr_info("[Command] loglvl: 0|D-debug, 1|W-Warning, 2|T-Trace, 3|E-Error\r\n");
			pr_info("[Command] Module: Cfa_Audio, Cfa_Ape, Cfa_Rm, Cfa_Avi, Cfa_Ogm, Cfa_Flv, Cfa_Audin, Cfa_Mkv, Cfa_Mpg, Cfa_Mp4, Cfa_Asf, Cfa_Ts, Cfa_Sub, Hw, Other\r\n");
			pr_info("[Command] ModuleLoglevel: 0 ~ 31, or -1, defined by Module\r\n");
			CLI_DMX_CLOSE_DRV();
			return 0;
		}

		if (g_rDmxCliCfg.u4arg4 > 31) {
			g_rDmxCliCfg.u4arg4 = -1;
		}

		memset(szModName, 0, sizeof(szModName));
		u4ModNameLen = strlen(szArgv[3]);

		pcChar = (char *)szArgv[3];

		for (u4Idx = 0; u4Idx < u4ModNameLen; u4Idx++, pcChar++) {
			szModName[u4Idx] = toupper(*pcChar);
		}

		if (0 == strcmp(szModName, TEXT("CFA_AUDIO"))) {
			g_rDmxCliCfg.u4arg3 = 0;
		} else if (0 == strcmp(szModName, TEXT("CFA_APE"))) {
			g_rDmxCliCfg.u4arg3 = 1;
		} else if (0 == strcmp(szModName, TEXT("CFA_RM"))) {
			g_rDmxCliCfg.u4arg3 = 2;
		} else if (0 == strcmp(szModName, TEXT("CFA_MPG"))) {
			g_rDmxCliCfg.u4arg3 = 3;
		} else if (0 == strcmp(szModName, TEXT("CFA_AVI"))) {
			g_rDmxCliCfg.u4arg3 = 4;
		} else if (0 == strcmp(szModName, TEXT("CFA_MP4"))) {
			g_rDmxCliCfg.u4arg3 = 5;
		} else if (0 == strcmp(szModName, TEXT("CFA_ASF"))) {
			g_rDmxCliCfg.u4arg3 = 6;
		} else if (0 == strcmp(szModName, TEXT("CFA_OGM"))) {
			g_rDmxCliCfg.u4arg3 = 7;
		} else if (0 == strcmp(szModName, TEXT("CFA_MKV"))) {
			g_rDmxCliCfg.u4arg3 = 8;
		} else if (0 == strcmp(szModName, TEXT("CFA_FLV"))) {
			g_rDmxCliCfg.u4arg3 = 9;
		} else if (0 == strcmp(szModName, TEXT("CFA_TS"))) {
			g_rDmxCliCfg.u4arg3 = 10;
		} else if (0 == strcmp(szModName, TEXT("CFA_SUB"))) {
			g_rDmxCliCfg.u4arg3 = 11;
		} else if (0 == strcmp(szModName, TEXT("CFA_AUDIN"))) {
			g_rDmxCliCfg.u4arg3 = 12;
		} else if (0 == strcmp(szModName, TEXT("HW"))) {
			g_rDmxCliCfg.u4arg3 = 16;
		} else if (0 == strcmp(szModName, TEXT("RSP"))) {
			g_rDmxCliCfg.u4arg3 = 17;
		} else if (0 == strcmp(szModName, TEXT("GAU"))) {
			g_rDmxCliCfg.u4arg3 = 18;
		} else if (0 == strcmp(szModName, TEXT("ESM"))) {
			g_rDmxCliCfg.u4arg3 = 19;
		} else if (0 == strcmp(szModName, TEXT("OTH"))) {
			g_rDmxCliCfg.u4arg3 = 20;
		}

		pr_info("[Command] drv.dmx.log [%s] [LoglLevel=%d] [Module=%s(%d)] [ModuleLoglevel=%d]\r\n",
				((0 == g_rDmxCliCfg.u4arg1) ? TEXT("OFF") : TEXT("ON")),
				g_rDmxCliCfg.u4arg2,
				szArgv[3],
				g_rDmxCliCfg.u4arg3,
				g_rDmxCliCfg.u4arg4);
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_TURN_ONOFF_LOG;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpFifoInfo
 * Function: Dump Demuxer's one or several stream's Fifo and AU info
 * u4arg1 -- Dump Stream ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =3. SP/CC
 * 3) =4. Section
 * 4) =none. All Streams
******************************************************************************/
static s32 _CLI_DMXDumpFifoInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		g_rDmxCliCfg.u4arg1 = (u32)(-1);
		pr_info("[Command] DUMP DMX ALL FIFO INFO (drv.dmx.fifo): \r\n");
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		if ((g_rDmxCliCfg.u4arg1 >= 5) ||
				(g_rDmxCliCfg.u4arg1 < 1)) {
			g_rDmxCliCfg.u4arg1 = (u32)(-1);
			pr_info("[Command] DUMP DMX ALL FIFO INFO (drv.dmx.fifo): \r\n");
		} else {
			pr_info("[Command] DUMP DMX (%s) FIFO INFO (drv.dmx.fifo [0x%x]): \r\n",
			g_wszDmxCliStmName[g_rDmxCliCfg.u4arg1 - 1],
			g_rDmxCliCfg.u4arg1);
		}
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_FIFO_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpPbbufInfo
 * Function: Dump Demuxer's Pbbufs' Info
 * No Param
******************************************************************************/
static s32 _CLI_DMXDumpPbbufInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	pr_info("[Command] DUMP DMX PBBUF INFO (drv.dmx.pbbuf [fgdumpdata(1) or not(0/none)]): \r\n");

	if (i4Argc < 2) {
		pr_info("[Command] DUMP DMX PBBUF INFO (drv.dmx.pbbuf): \r\n");
		g_rDmxCliCfg.u4arg1 = 0;
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		pr_info("[Command] DUMP DMX PBBUF INFO (drv.dmx.pbbuf [%sDumpData ]): \r\n",
				(0 != g_rDmxCliCfg.u4arg1) ? TEXT(" ") : TEXT(" No"));
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_PBBUF_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpGAUInfo
 * Function: Dump Demuxer's Pbbufs' Info
 * No Param
******************************************************************************/
static s32 _CLI_DMXDumpGAUInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	pr_info("[Command] DUMP DMX GAU INFO (drv.dmx.gau): \r\n");

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_GAU_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpThresholdInfo
 * Function: Dump Demuxer's one or several stream's threshold info
 * u4arg1 -- Dump Stream ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =none. Both Video and Audio
******************************************************************************/
static s32 _CLI_DMXDumpThresholdInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		g_rDmxCliCfg.u4arg1 = (u32)(-1);
		pr_info("[Command] DUMP DMX ALL STREAMS' THRESHOLD INFO (drv.dmx.threshold [stmtype(1,2,-1)]): \r\n");
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		if ((g_rDmxCliCfg.u4arg1 >= 3) ||
				(g_rDmxCliCfg.u4arg1 < 1)) {
			g_rDmxCliCfg.u4arg1 = (u32)(-1);
			pr_info("[Command] DUMP DMX ALL STREAMS' THRESHOLD INFO (drv.dmx.threshold [stmtype(1,2,-1)]): \r\n");
		} else {
			pr_info("[Command] DUMP DMX (%s) STREAM's THRESHOLD INFO (drv.dmx.threshold [stmtype(1,2,-1): 0x%x]): \r\n",
			g_wszDmxCliStmName[g_rDmxCliCfg.u4arg1 - 1],
			g_rDmxCliCfg.u4arg1);
		}
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_THRESHOLD_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXEnableThreshold
 * Function: Eanble Demuxer's one or several stream's threshold info
 * u4StmTypeID -- Designated Stream Type ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * u4Enable    -- Designated whether to enable or disable stream's threshold
 * 1) =0. Disable
 * 2) =!0. Enable
******************************************************************************/
static s32 _CLI_DMXEnableThreshold(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info("[Command] ONLY SUPPORT (drv.dmx.enthreshold [stmtype(1-video,2-audio)])\r\n");
		CLI_DMX_CLOSE_DRV();
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		g_rDmxCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);
		if ((g_rDmxCliCfg.u4arg1 >= 3) ||
				(g_rDmxCliCfg.u4arg1 < 1)) {
			pr_info("[Command] ONLY SUPPORT (drv.dmx.enthreshold [stmtype(1-video,2-audio)])\r\n");
			CLI_DMX_CLOSE_DRV();
		} else {
			pr_info("[Command] %s DMX (%s) STREAM's THRESHOLD INFO(drv.dmx.enthreshold [stmtype(1,2): 0x%x])\r\n",
					((0 == g_rDmxCliCfg.u4arg2) ? TEXT("DISABLE") : TEXT("ENABLE")),
					g_wszDmxCliStmName[g_rDmxCliCfg.u4arg1 - 1],
					g_rDmxCliCfg.u4arg1);
		}
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_ENABLE_THRESHOLD;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXPrintAUGetLog
 * Function: Print Get AU Log(include AUIdx, PTS, AUSA, AUEA)
 * u4arg1 -- Stream ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =3. SP/CC
 * 3) =4. Section
 * 4) =none. All Streams
******************************************************************************/
static s32 _CLI_DMXPrintAUGetLog(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info("[Command] drv.dmx.getaulog [Stmtype(1~4)] [logon(1) or off(0)]\r\n");
		CLI_DMX_CLOSE_DRV();
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		g_rDmxCliCfg.u4arg2 = (u32)_ttoi(szArgv[2]);

		if ((g_rDmxCliCfg.u4arg1 < 1) || (5 <= g_rDmxCliCfg.u4arg1)) {
			g_rDmxCliCfg.u4arg1 = (u32)(-1);
		}

		pr_info("[Command] %s DMX (%s) STREAM's GETAU INFO(drv.dmx.getaulog 0x%x 0x%x)\r\n",
				((0 == g_rDmxCliCfg.u4arg2) ? TEXT("DISABLE") : TEXT("ENABLE")),
				((g_rDmxCliCfg.u4arg1 < 4) ? g_wszDmxCliStmName[g_rDmxCliCfg.u4arg1 - 1] : TEXT("ALL")),
				g_rDmxCliCfg.u4arg1,
				g_rDmxCliCfg.u4arg2);
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_PRINT_AUGET_LOG;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpState
 * Function: Dump Demuxer's Splitter, Parser CC, Parser Filter' Info
 * u4arg1 -- Log level ID, it should be the following value:
 * 1) =1. Splitter's Current Info
 * 2) =2. Parser CC's Current Info
 * 3) =3. Parser Filter's Current Info
 * 3) =none. All Info
******************************************************************************/
static s32 _CLI_DMXDumpState(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		g_rDmxCliCfg.u4arg1 = -1;
		pr_info("[Command] DUMP DMX All MAIN INFO (drv.dmx.main)\r\n");
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		pr_info("[Command] DUMP DMX MAIN INFO (drv.dmx.main [ModuleID(1-3, -1 or none): 0x%x])\r\n",
			g_rDmxCliCfg.u4arg1);
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_INSTS_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

static s32 _CLI_DMXDumpHWInfo(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 2) {
		g_rDmxCliCfg.u4arg1 = -1;
		pr_info("[Command] DUMP DMX HW INFO (drv.dmx.hw [PidIdx(0-3, 10) or none])\r\n");
	} else {
		g_rDmxCliCfg.u4arg1 = (u32)_ttoi(szArgv[1]);
		pr_info("[Command] DUMP DMX HW INFO (drv.dmx.hw [PidIdx(0-3, 10) or none : 0x%x])\r\n",
				g_rDmxCliCfg.u4arg1);
	}

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_HW_INFO;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpAllAUData
 * Function: Dump All Stream's fifo data info designated file
 * u4arg1 -- StmTypeValue, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =3. CC
 * u4arg2 -- Dump or not, it should be the following value:
 * 1) =0. Close Dump File
 * 2) =1. Create Dump File and begin to dump
 * ptParam -- the name of the dir to be created
******************************************************************************/
static s32 _CLI_DMXDumpAllAUData(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 4) {
		pr_info("[Command]drv.dmx.audata [1 ~ 3(stmtype)] [0 or 1(enable)] [dirname]\r\n");
		CLI_DMX_CLOSE_DRV();
		return -1;
	}

	g_rDmxCliCfg.u4arg1  = (u32)_ttoi(szArgv[1]);
	g_rDmxCliCfg.u4arg2  = (u32)_ttoi(szArgv[2]);
	g_rDmxCliCfg.ptParam = *(void **)&szArgv[3];

	pr_info("[Command]drv.dmx.audata [0x%x(%s)] [0x%x(%s)] [dirname: %s]\r\n",
		g_rDmxCliCfg.u4arg1,
		g_wszDmxCliStmName[g_rDmxCliCfg.u4arg1 - 1],
		g_rDmxCliCfg.u4arg2,
		((0 == g_rDmxCliCfg.u4arg2) ? TEXT("CloseDump") : TEXT("EnableDump")),
		(s8 *)(g_rDmxCliCfg.ptParam));

	g_rDmxCliCfg.eDmxCliType =  DMX_CLI_CMD_DUMP_ALLAUDATA;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpFlow
 * Function: Dump Flow
 * u4arg1 -- Dump or not, it should be the following value:
 * 1) =0. Close Dump File
 * 2) =1. Create Dump File and begin to dump
 * ptParam -- the name of the dir to be created
******************************************************************************/
static s32 _CLI_DMXDumpFlow(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info("[Command]drv.dmx.flow [0 or 1(enable)] [dirname]\r\n");
		CLI_DMX_CLOSE_DRV();
		return -1;
	}

	g_rDmxCliCfg.u4arg1  = (u32)_ttoi(szArgv[1]);
	g_rDmxCliCfg.ptParam = *(void **)&szArgv[2];

	pr_info("[Command]drv.dmx.flow [0x%x(%s)] [dirname: %s]\r\n",
		g_rDmxCliCfg.u4arg1,
		((0 == g_rDmxCliCfg.u4arg1) ? TEXT("CloseDump") : TEXT("EnableDump")),
		(s8 *)(g_rDmxCliCfg.ptParam));

	g_rDmxCliCfg.eDmxCliType =  DMX_CLI_CMD_DUMP_FLOW;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

/******************************************************************************
 *_CLI_DMXDumpRegisters
 * Function: Dump Flow
 * u4arg1 -- Dump or not, it should be the following value:
 * 1) =0. Close Dump File
 * 2) =1. Create Dump File and begin to dump
 * ptParam -- the name of the dir to be created
******************************************************************************/
static s32 _CLI_DMXDumpRegisters(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	if (i4Argc < 3) {
		pr_info("[Command]drv.dmx.regs [1stRegVAddr] [RegCnt]\r\n");
		CLI_DMX_CLOSE_DRV();
		return -1;
	}

	g_rDmxCliCfg.u4arg1  = (u32)_ttoi16(szArgv[1]);
	g_rDmxCliCfg.u4arg2  = (u32)_ttoi16(szArgv[2]);

	pr_info("[Command]drv.dmx.regs [0x%x] [0x%x]\r\n",
			g_rDmxCliCfg.u4arg1, g_rDmxCliCfg.u4arg2);

	g_rDmxCliCfg.eDmxCliType =  DMX_CLI_CMD_DUMP_REGISTERS;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

static s32 _CLI_DMXDumpMemUsage(s32 i4Argc, const s8 **szArgv)
{
	CLI_DMX_CHECK_DRV_HANDLE();

	pr_info("[Command] DUMP DMX MEM USAGE (drv.dmx.mem): \r\n");

	g_rDmxCliCfg.eDmxCliType = DMX_CLI_CMD_DUMP_MEM_USAGE;

	CLI_DMX_IOCTL();

	CLI_DMX_CLOSE_DRV();

	return 0;
}

CLI_EXEC_T _arDmxDrvCmdTbl[] = {
	/* DMX_CLI_CMD_TURN_OFF_LOG */
	{
		TEXT("TurnOnOffLog"),
		TEXT("log"),
		_CLI_DMXTurnOnOffLog,
		NULL,
		TEXT("Turn On or Off Dmx Log"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_FIFO_INFO */
	{
		TEXT("DumpFifoInfo"),
		TEXT("fifo"),
		_CLI_DMXDumpFifoInfo,
		NULL,
		TEXT("Dump Fifo Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_PBBUF_INFO */
	{
		TEXT("DumpPbbufInfo"),
		TEXT("pbbuf"),
		_CLI_DMXDumpPbbufInfo,
		NULL,
		TEXT("Dump Pbbuf Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_DMYDEC_INFO */
	{
		TEXT("DumpGAUInfo"),
		TEXT("gau"),
		_CLI_DMXDumpGAUInfo,
		NULL,
		TEXT("Dump GAU Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_THRESHOLD_INFO */
	{
		TEXT("DumpThresholdInfo"),
		TEXT("threshold"),
		_CLI_DMXDumpThresholdInfo,
		NULL,
		TEXT("Dump Threshold Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_ENABLE_THRESHOLD */
	{
		TEXT("EnableThreshold"),
		TEXT("enthreshold"),
		_CLI_DMXEnableThreshold,
		NULL,
		TEXT("Enable Threshold"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_PRINT_AUGET_LOG */
	{
		TEXT("Print AU Get Log"),
		TEXT("getaulog"),
		_CLI_DMXPrintAUGetLog,
		NULL,
		TEXT("Enable Threshold"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_INSTS_INFO */
	{
		TEXT("DumpMainInfo"),
		TEXT("main"),
		_CLI_DMXDumpState,
		NULL,
		TEXT("Dump Dmx Splitter/PsrCC/PsrFilter Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_HW_INFO */
	{
		TEXT("DumpHWInfo"),
		TEXT("hw"),
		_CLI_DMXDumpHWInfo,
		NULL,
		TEXT("Dump Dmx HW Info"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_ALLAUDATA */
	{
		TEXT("DumpAllAUData"),
		TEXT("audata"),
		_CLI_DMXDumpAllAUData,
		NULL,
		TEXT("Dump ALL AU Data"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_FLOW */
	{
		TEXT("DumpFlow"),
		TEXT("flow"),
		_CLI_DMXDumpFlow,
		NULL,
		TEXT("Dump Flow"),
		CLI_GUEST
	},

	{
		TEXT("DumpRegs"),
		TEXT("regs"),
		_CLI_DMXDumpRegisters,
		NULL,
		TEXT("Dump Registers"),
		CLI_GUEST
	},

	/* DMX_CLI_CMD_DUMP_MEM_USAGE */
	{
		TEXT("DumpMemUsage"),
		TEXT("mem"),
		_CLI_DMXDumpMemUsage,
		NULL,
		TEXT("Dump Mem Usage"),
		CLI_GUEST
	},

	/* last cli command record, NULL */
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		CLI_SUPERVISOR
	}
};



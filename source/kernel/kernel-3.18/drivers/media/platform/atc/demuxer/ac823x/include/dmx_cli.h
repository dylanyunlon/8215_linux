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
 * @file dmx_cli.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Cli structures, macros, interfaces declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_CLI_H
#define DMX_INTERNAL_CLI_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/x_dmx.h>
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "drv_esm_if.h"
#include "x_dmx.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "dmx_def.h"

#ifdef __linux__
extern void _DmxCliCmd(DMX_CLI_TYPE eDmxCliType, u32 arg1, u32 arg2,
				u32 arg3, u32 arg4, const char **pfilename);

#endif				/* __linux__ */

#ifdef __cplusplus
extern "C" {

#endif
typedef enum{
	DMX_CLI_STM_VID, DMX_CLI_STM_AUD, DMX_CLI_STM_SP,
	DMX_CLI_STM_SECTION, MAX_OF_DMX_CLI_SMT_CNT = 4
} E_DMX_CLI_STM_TYPE_T;
typedef struct{
	bool fgDumpRspInfo;
	bool fgDumpFlow;
	bool fgUsed;
	bool fgAVDumpFrameData[MAX_OF_DMX_CLI_SMT_CNT];
	uintptr_t pvDmxInst;
} DMX_CLI_MAN_T;
void DmxCliInit(void);
void DmxCliDeInit(void);
MRESULT DmxCliTurnOnOffLog(u32 u4OnOff, u32 u4LogLevel,
	u32 u4Module, u32 u4ModLogLvl);
MRESULT DmxCliDumpFifoInfo(u32 u4StmTypeID);
MRESULT DmxCliDumpPbbufInfo(u32 u4DumpData);
MRESULT DmxCliDumpGAUInfo(void);
MRESULT DmxCliDumpThresholdInfo(u32 u4StmTypeID);
MRESULT DmxCliEnableThreshold(u32 u4StmTypeID, u32 u4Enabled);
MRESULT DmxCliPrintGetAULog(u32 u4StmType, u32 u4Enabled);
MRESULT DmxCliDumpInstsInfo(u32 u4DumpLevel);
MRESULT DmxCliDumpPidStructure(u32 u4Pidx);
MRESULT DmxCliDumpFlow(u32 u4Enable, char *wszDirname);
MRESULT DmxCliDumpAllAUData(u32 u4StmType, u32 u4Enable,
	char *pdirname);
MRESULT DmxCliDumpMemUsage(void);
MRESULT DmxCliDumpRspInfo(u32 u4DumpData);
MRESULT DmxCliDumpAUInfo(u32 u4StmType, u32 u4StartIdx,
	u32 u4EndIdx);
MRESULT DmxCliPrintPerfInfo(void);

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef DMX_INTERNAL_CLI_H */

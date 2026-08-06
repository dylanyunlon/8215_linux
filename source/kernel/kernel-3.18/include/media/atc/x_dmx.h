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

#ifndef _X_DMX_H_
#define _X_DMX_H_

/*-----------------------------------------------------------------------------
										include files
-----------------------------------------------------------------------------*/
#include "x_common.h"
#include "x_typedef.h"

/*-----------------------------------------------------------------------------
										ClI Related
-----------------------------------------------------------------------------*/
typedef enum _DMX_CLI_TYPE_ {
	DMX_CLI_CMD_TURN_ONOFF_LOG,
	DMX_CLI_CMD_DUMP_FIFO_INFO,
	DMX_CLI_CMD_DUMP_PBBUF_INFO,
	DMX_CLI_CMD_DUMP_GAU_INFO,
	DMX_CLI_CMD_DUMP_THRESHOLD_INFO,
	DMX_CLI_CMD_ENABLE_THRESHOLD,
	DMX_CLI_CMD_PRINT_AUGET_LOG,
	DMX_CLI_CMD_DUMP_INSTS_INFO,
	DMX_CLI_CMD_DUMP_HW_INFO,
	DMX_CLI_CMD_DUMP_ALLAUDATA,
	DMX_CLI_CMD_DUMP_MEM_USAGE,
	DMX_CLI_CMD_DUMP_FLOW,
	DMX_CLI_CMD_DUMP_REGISTERS,
	DMX_CLI_CMD_DUMP_AU_INFO,
	DMX_CLI_CMD_PRINT_PERF_INFO,
	MAX_CNT_OF_DMX_CLI_CMD_TYPE
} DMX_CLI_TYPE;

typedef struct {
	DMX_CLI_TYPE eDmxCliType;
	__u32 u4arg1;	/* u4InputID; */
	__u32 u4arg2;		/* u4Len; */
	__u32 u4arg3;		/* u4Size; */
	__u32 u4arg4;		/* u4Value; */
	void **ptParam;		/* filename */
} DMX_CLI_CFG;

#endif				/* _X_DMX_H_ */

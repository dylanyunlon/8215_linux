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

#ifndef X_DRV_CLI_H
#define X_DRV_CLI_H


//remove driver cli module.

/******************************************************************************
* cli command respond value
******************************************************************************/
#define CLI_COMMAND_OK					0
#define CLI_UNKNOWN_CMD					-2147483647

/******************************************************************************
* cli command access right
******************************************************************************/
/* duplicate define with u_cli.h*/
typedef enum
{
	CLI_SUPERVISOR = 0,
	CLI_ADMIN,
	CLI_GUEST,
	CLI_HIDDEN
} CLI_ACCESS_RIGHT_T;


/******************************************************************************
* cli command structure
******************************************************************************/
/* duplicate define with u_cli.h*/
typedef struct _CLI_EXEC
{
	CHAR*				pszCmdStr;													// command string
	CHAR*				pszCmdAbbrStr;												// command abbreviation
	INT32				(*pfExecFun) (INT32 i4Argc, const CHAR ** szArgv);			// execution function
	struct _CLI_EXEC	*prCmdNextLevel;											// next level command table
	CHAR*				pszCmdHelpStr;												// command description string
	CLI_ACCESS_RIGHT_T	eAccessRight;												// command access right
} CLI_EXEC_T;


/******************************************************************************
* cli command table get function
******************************************************************************/
typedef CLI_EXEC_T* (*CLI_GET_CMD_TBL_FUNC)(void);
typedef void (* VER_FUNC)(void);


#endif /* X_DRV_CLI_H */



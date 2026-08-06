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

#ifndef _DSP_TASK_H_
#define _DSP_TASK_H_
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
// IC Type Distinguishing Config
#include "aud_drv_config.h"
#include "chip_ver.h"

#include "drv_thread.h"
#include "x_bim.h"

/******************************************************************************
*    Constant definition
******************************************************************************/
#define DSPA_INT_VECTOR                     VECTOR_DSP
#define DSPC_INT_VECTOR                     VECTOR_DSPC
#define DSPB_INT_VECTOR                     VECTOR_DSPB2RISC

extern void vADSPTaskInit(void);

#endif //_DSP_TASK_H_


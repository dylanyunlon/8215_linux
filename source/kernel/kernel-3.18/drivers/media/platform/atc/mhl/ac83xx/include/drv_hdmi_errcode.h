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

#ifndef _DRV_HDMI_ERRCODE_H_
#define _DRV_HDMI_ERRCODE_H_

#include "x_typedef.h"
#include "u_uerrcode.h"


#define HDMI_HAL_OK_CODE_START_OFFSET    30
#define HDMI_HAL_ERROR_CODE_START_OFFSET    30


#define S_HDMI_OK                                            UOKCODE(DRL_MODULE_HDMI, 0)


#define E_HDMI_TASK_INIT_FAIL                                UERRCODE(DRL_MODULE_HDMI, 0)
#define E_HDMI_TASK_UNINIT_FAIL                              UERRCODE(DRL_MODULE_HDMI, 1)
#define E_CEC_TASK_INIT_FAIL                                 UERRCODE(DRL_MODULE_HDMI, 2)

#endif

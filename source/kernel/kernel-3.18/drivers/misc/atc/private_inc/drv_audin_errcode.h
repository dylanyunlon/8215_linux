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

#include "x_typedef.h"
#include "u_uerrcode.h"

#ifndef _DRV_AUDIN_ERRCODE_H_
#define _DRV_AUDIN_ERRCODE_H_


#define S_AUDIN_OK                                            UOKCODE(DRL_MODULE_AUDIN, 0)


#define E_AUDIN_TASK_INIT_FAIL                                UERRCODE(DRL_MODULE_AUDIN, 0)
#define E_AUDIN_TASK_UNINIT_FAIL                              UERRCODE(DRL_MODULE_AUDIN, 1)

#endif

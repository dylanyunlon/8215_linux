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

#ifndef _DRV_GCPU_ERRCODE_H_
#define _DRV_GCPU_ERRCODE_H_

#define GCPU_HAL_ERROR_CODE_START_OFFSET    30

#define S_GCPU_OK                                            UOKCODE(DRL_MODULE_GCPU, 0)

#define E_GCPU_UNEXPECT                                UERRCODE(DRL_MODULE_GCPU, 0)
#define E_GCPU_INVALID_HANDLE                    UERRCODE(DRL_MODULE_GCPU, 1)
#define E_GCPU_PARAM_WRONG                       UERRCODE(DRL_MODULE_GCPU, 2)
#define E_GCPU_OS_OPERA_FAIL                      UERRCODE(DRL_MODULE_GCPU, 3)
#define E_GCPU_NO_INIT                                  UERRCODE(DRL_MODULE_GCPU, 4)
#define E_GCPU_OVER_LIMIT                            UERRCODE(DRL_MODULE_GCPU, 5)
#define E_GCPU_OPERATE_FORBID                    UERRCODE(DRL_MODULE_GCPU, 6)
#define E_GCPU_NO_MEM                                  UERRCODE(DRL_MODULE_GCPU, 7)
#define E_GCPU_CMD_FAIL                                UERRCODE(DRL_MODULE_GCPU, 8)
#define E_GCPU_UNIMPLEMENT                         UERRCODE(DRL_MODULE_GCPU, 9)

#define E_GCPU_FAIL                UERRCODE(DRL_MODULE_GCPU, GCPU_HAL_ERROR_CODE_START_OFFSET)
#define E_GCPU_INV_ARG             UERRCODE(DRL_MODULE_GCPU, (GCPU_HAL_ERROR_CODE_START_OFFSET+1))
#define E_GCPU_NO_RES              UERRCODE(DRL_MODULE_GCPU, (GCPU_HAL_ERROR_CODE_START_OFFSET+2))


#endif /* #ifndef _DRV_GCPU_ERRCODE_H_ */



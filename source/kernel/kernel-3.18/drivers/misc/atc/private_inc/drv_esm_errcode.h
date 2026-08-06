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

#ifndef _DRV_ESM_ERRCODE_H_
#define _DRV_ESM_ERRCODE_H_

#define S_ESM_OK                                            UOKCODE(DRL_MODULE_ESM, 0)
  
#define E_ESM_UNEXPECT                                UERRCODE(DRL_MODULE_ESM, 0)
#define E_ESM_INVALID_HANDLE                    UERRCODE(DRL_MODULE_ESM, 1)
#define E_ESM_NO_AUTABLE                           UERRCODE(DRL_MODULE_ESM, 2)
#define E_ESM_PARAM_WRONG                       UERRCODE(DRL_MODULE_ESM, 3)
#define E_ESM_OS_OPERA_FAIL                      UERRCODE(DRL_MODULE_ESM, 4)
#define E_ESM_NO_HANDLE                             UERRCODE(DRL_MODULE_ESM, 5)
#define E_ESM_NO_MEMORY                             UERRCODE(DRL_MODULE_ESM, 6)
#define E_ESM_NO_INIT                                  UERRCODE(DRL_MODULE_ESM, 7)
#define E_ESM_NO_ESI_EXIST                         UERRCODE(DRL_MODULE_ESM, 8)
#define E_ESM_SET_TWICE                             UERRCODE(DRL_MODULE_ESM, 9)
#define E_ESM_NO_SET_FTYPE                         UERRCODE(DRL_MODULE_ESM, 10)


#endif // #ifndef _DRV_ESM_ERRCODE_H_



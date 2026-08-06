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

#ifndef _DRV_FBM_ERRCODE_H_
#define _DRV_FBM_ERRCODE_H_

#define S_FBM_OK                               UOKCODE(DRL_MODULE_FBM, 0)
  
#define E_FBM_UNEXPECT                         UERRCODE(DRL_MODULE_FBM, 0)
#define E_FBM_INVALID_INDEX                    UERRCODE(DRL_MODULE_FBM, 1)
#define E_FBM_PARAM_WRONG                      UERRCODE(DRL_MODULE_FBM, 2)
#define E_FBM_OS_OPERA_FAIL                    UERRCODE(DRL_MODULE_FBM, 3)
#define E_FBM_NO_MEMORY                        UERRCODE(DRL_MODULE_FBM, 4)
#define E_FBM_NO_INIT                          UERRCODE(DRL_MODULE_FBM, 5)
#define E_FBM_NO_FBUF                          UERRCODE(DRL_MODULE_FBM, 6)
#define E_FBM_OVER_COUNT                       UERRCODE(DRL_MODULE_FBM, 7)
#define E_FBM_RW_REJECT                        UERRCODE(DRL_MODULE_FBM, 8)
#define E_FBM_QUEUE_FULL                       UERRCODE(DRL_MODULE_FBM, 9)
#define E_FBM_NO_QUEUE                         UERRCODE(DRL_MODULE_FBM, 10)
#define E_FBM_NO_PU                            UERRCODE(DRL_MODULE_FBM, 11)
#define E_FBM_NO_SUPPORT                       UERRCODE(DRL_MODULE_FBM, 12)
#define E_FBM_NO_CCU                           UERRCODE(DRL_MODULE_FBM, 13)
#define E_FBM_OPERATE_FORBID                   UERRCODE(DRL_MODULE_FBM, 14)
#define E_FBM_RESOURE_BUSY                     UERRCODE(DRL_MODULE_FBM, 15)
#define E_FBM_NO_DSFBUF                        UERRCODE(DRL_MODULE_FBM, 16)
#define E_FBM_FBG_DESTROYED                    UERRCODE(DRL_MODULE_FBM, 17)
  


#endif // #ifndef _DRV_FBM_ERRCODE_H_



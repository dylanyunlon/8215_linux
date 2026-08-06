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

#ifndef _DRV_SYNCCTRL_ERRCODE_H_
#define _DRV_SYNCCTRL_ERRCODE_H_

/*! \addtogroup ErrCode
* @{
*/

#define S_SYNCCTRL_OK                          UOKCODE(DRL_MODULE_SYNCCTRL, 0)
  
#define E_SYNCCTRL_UNEXPECTED                  UERRCODE(DRL_MODULE_SYNCCTRL, 1)
#define E_SYNCCTRL_OS_OPERA_FAIL             UERRCODE(DRL_MODULE_SYNCCTRL, 2)
#define E_SYNCCTRL_PARAM_WORNG              UERRCODE(DRL_MODULE_SYNCCTRL, 3)
#define E_SYNCCTRL_NO_INIT                         UERRCODE(DRL_MODULE_PSR, 4)
#define E_SYNCCTRL_OPERATE_FORBID          UERRCODE(DRL_MODULE_SYNCCTRL, 5)

/*! @} */

#endif // #ifndef _DRV_SYNCCTRL_ERRCODE_H_


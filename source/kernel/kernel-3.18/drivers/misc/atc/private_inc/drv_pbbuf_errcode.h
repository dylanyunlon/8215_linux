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

#ifndef _DRV_PBBUF_ERRCODE_H_
#define _DRV_PBBUF_ERRCODE_H_

/*! \addtogroup ErrCode
* @{
*/

#define S_PBBUF_OK                          UOKCODE(DRL_MODULE_PBBUF, 0)
  
#define E_PBBUF_BUSY                        UERRCODE(DRL_MODULE_PBBUF, 0)
#define E_PBBUF_MEM_ALLOC_FAIL              UERRCODE(DRL_MODULE_PBBUF, 1)
/*! @} */

#endif // #ifndef _DRV_PBBUF_ERRCODE_H_



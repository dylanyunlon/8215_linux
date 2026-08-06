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

//============================================================================
// Include files
//============================================================================
#include <linux/module.h>      //Must be included header file
#include "x_ckgen.h"
#include "x_hal_ic.h"
#include "x_bim.h"
#include <mach/base_regs.h>
#include "chip_ver.h"
#include "x_bsp_linux.h"
#include <linux/spinlock_types.h>

static DEFINE_SPINLOCK(ac83xx_bsp_lock);

#define FL_STATUS_A2_FINISHED	0x3
BSP_GET_FUNCTION_ID	_gBspGetFuncID;

/*----------------------------------------------------------------------------
 * BSP_GetIcBounding() Get IC Bounding
 *  @return The Bounding 0/1
 *---------------------------------------------------------------------------*/
BOOL BSP_GetIcBounding(UINT32 u4PROT)
{
  UINT32 u4Tmp1;
  unsigned long flags;

  spin_lock_irqsave(&ac83xx_bsp_lock, flags);
  _gBspGetFuncID.CallBackfunc = 0;
  _gBspGetFuncID.FuncID = 0;
  _gBspGetFuncID.ret_val = 0;
  _gBspGetFuncID.GetIcVerFunc = 0;

  u4Tmp1 = CKGEN_READ32(REG_RW_RST_CFG);
  spin_unlock_irqrestore(&ac83xx_bsp_lock, flags);

  u4Tmp1 = (u4Tmp1 & ((1U << u4PROT) << RST_CFG_PROT_OFFSET));

  if(u4Tmp1 == 0)
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

EXPORT_SYMBOL(_gBspGetFuncID);

EXPORT_SYMBOL(BSP_GetIcBounding);

CHAR pc_IC_SUB_VER_UNKNOWN[] =      "UNKNOWN";     // UNKNOWN
void _BSP_GetIcSubVersion(UINT32* u4IcSubVerAddr)
{
    CHAR** pcIcSubVer = (CHAR**)u4IcSubVerAddr;
    *pcIcSubVer = pc_IC_SUB_VER_UNKNOWN;
}
EXPORT_SYMBOL(_BSP_GetIcSubVersion);

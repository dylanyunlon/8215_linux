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

#ifndef GFX_COMMON_H
#define GFX_COMMON_H


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

/*lint -save -e961 */
//#include "x_common.h"
#include "x_typedef.h"
#include "x_timer.h"
#include "x_assert.h"
#include "x_os.h"
#include "x_bim.h"
#include "chip_ver.h"
#include "x_gfx.h"
#include "x_debug.h"

//#define GFX_3D_HOR_ROTATE 0


/*lint -restore */


//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
//#define GFX_SW_FX_ENTRY_NOTIFY
#define GFX_HW_FX_ENTRY_NOTIFY

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------

/** ignore return value
 *
 */
#define GFX_UNUSED_RET(X)           \
    {                               \
        INT32 i4Ignore;             \
        i4Ignore = (INT32)(X);      \
        UNUSED(i4Ignore);           \
    }

/* DBG INFO RELATED */
//#define GFX_DBG_API(_stmt)  x_dbg_stmt _stmt
//#define GFX_DBG_ERR(_stmt)  x_dbg_stmt _stmt
//#define GFX_DBG_INFO(_stmt) x_dbg_stmt _stmt
//#define GFX_DBG_STMT(_stmt) x_dbg_stmt _stmt

/* use software mode instead */
#define GFX_ENABLE_SW_MODE
//#define x_dbg_stmt(argu)

#ifdef GFX_SW_FX_ENTRY_NOTIFY
    #define GFX_SW_FX_ENTRY    //x_dbg_stmt("[GFX_SW] %s\n", __FUNCTION__);
#else
    #define GFX_SW_FX_ENTRY
#endif

#ifdef GFX_HW_FX_ENTRY_NOTIFY
    #define GFX_HW_FX_ENTRY    //x_dbg_stmt("[GFX_DRV] %s\n", __FUNCTION__);
#else
    #define GFX_HW_FX_ENTRY UTIL_Printf("[GFX_DRV] %s\n", __FUNCTION__);
#endif


#endif // GFX_COMMON_H



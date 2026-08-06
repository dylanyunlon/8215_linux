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
 
#ifndef GFX_DIF_H
#define GFX_DIF_H

#include "x_assert.h"
#include "x_os.h"
#include "x_drv_cli.h"
//#include "x_serial.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_printf.h"
#include "x_util.h"
#include "x_stl_lib.h"
#include "x_bim.h"
#include "chip_ver.h"

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "gfx_dif_reg.h"



//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

// u4Config
#define MA_DIF_HAVE_GFX_SW              \
    (GFX_DifGetData(u4GfxHwId)->u4Config & GFX_HAVE_SW_MOD)
    
#define MA_DIF_HAVE_GFX_HW8520          \
    (GFX_DifGetData(u4GfxHwId)->u4Config & GFX_HAVE_HW_8520_MOD)
    
#define MA_DIF_HAVE_GFX_FB              \
    (GFX_DifGetData(u4GfxHwId)->u4Config & GFX_HAVE_FB_MOD)

// u4ModInit
#define MA_DIF_GFX_SW_MOD_OK(u4GfxHwId)          \
    (GFX_DifGetData(u4GfxHwId)->u4ModInit |= GFX_HAVE_SW_MOD)
    
#define MA_DIF_GFX_HW8520_MOD_OK(u4GfxHwId)      \
    (GFX_DifGetData(u4GfxHwId)->u4ModInit |= GFX_HAVE_HW_8520_MOD)
    
#define MA_DIF_GFX_FB_MOD_OK(u4GfxHwId)          \
    (GFX_DifGetData(u4GfxHwId)->u4ModInit |= GFX_HAVE_FB_MOD)


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

/** gfx system capabilities
 *  uiConfig - SW_MOD, HW_MOD, etc
 *  uiModInit - init bit flag
 */
typedef struct _MI_DIF_T
{
    UINT32 u4Config;        // configuration bits, modules
    UINT32 u4GfxMode;       // operation mode / module
    UINT32 u4ModInit;       // configuration bits, modules status
    UINT32 *pu4CrBase;      // control register base address
    INT32 i4DifIdle;                            /// Resize complete flag.
    BOOL fgFlushing;                                   /// flushing
} MI_DIF_T;


//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_DifInit(UINT32 u4GfxHwId);
extern INT32 GFX_DifUninit(UINT32 u4GfxHwId);

extern INT32 GFX_DifSetRegBase(UINT32 u4GfxHwId, UINT32 *pu4Base);

extern INT32 GFX_DifGetRegBase(UINT32 u4GfxHwId, UINT32 *pu4Base);

extern INT32 GFX_DifReset(UINT32 u4GfxHwId, UINT32 u4Reset);

extern void GFX_DifSetMode(UINT32 u4GfxHwId, UINT32 u4GfxMode);

extern INT32 GFX_DifSetCR(UINT32 u4GfxHwId, UINT32 u4CrName, UINT32 u4Val);

extern INT32 GFX_DifGetCR(UINT32 u4GfxHwId, UINT32 u4CrName, UINT32 *pu4Val);

extern INT32 GFX_DifGetIdle(UINT32 u4GfxHwId);

extern void GFX_DifSetIdle(UINT32 u4GfxHwId, INT32 u4Idle);

extern void GFX_DifSetNotify(UINT32, void (*pfnNotify)(UINT32));

extern MI_DIF_T *GFX_DifGetData(UINT32 u4GfxHwId);

extern INT32 (*pfnGFX_DifAction)(UINT32);

extern INT32 (*pfnGFX_DifGetInternalIdle)(UINT32);

extern void (*pfnGFX_DifWait)(void);

extern void (*pfnGFX_DifFinNotify)(UINT32);
extern INT32 GFX_DifResetWT(UINT32 u4GfxHwId, UINT32 u4Reset);
#endif // GFX_DIF_H



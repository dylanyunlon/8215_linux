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
    uint32_t u4Config;        // configuration bits, modules
    uint32_t u4GfxMode;       // operation mode / module
    uint32_t u4ModInit;       // configuration bits, modules status
    unsigned long *pu4CrBase;      // control register base address
    INT32 i4DifIdle;                            /// Resize complete flag.
    BOOL fgFlushing;                                   /// flushing
} MI_DIF_T;


//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_DifInit(uint32_t u4GfxHwId);
extern INT32 GFX_DifUninit(uint32_t u4GfxHwId);

extern INT32 GFX_DifSetRegBase(uint32_t u4GfxHwId, unsigned long *pu4Base);

extern INT32 GFX_DifGetRegBase(uint32_t u4GfxHwId, unsigned long *pu4Base);

extern INT32 GFX_DifReset(uint32_t u4GfxHwId, uint32_t u4Reset);

extern void GFX_DifSetMode(uint32_t u4GfxHwId, uint32_t u4GfxMode);

extern INT32 GFX_DifSetCR(uint32_t u4GfxHwId, uint32_t u4CrName, uint32_t u4Val);

extern INT32 GFX_DifGetCR(uint32_t u4GfxHwId, uint32_t u4CrName, uint32_t *pu4Val);

extern INT32 GFX_DifGetIdle(uint32_t u4GfxHwId);

extern void GFX_DifSetIdle(uint32_t u4GfxHwId, INT32 u4Idle);

extern void GFX_DifSetNotify(uint32_t, void (*pfnNotify)(uint32_t));

extern MI_DIF_T *GFX_DifGetData(uint32_t u4GfxHwId);

extern INT32 (*pfnGFX_DifAction)(uint32_t);

extern INT32 (*pfnGFX_DifGetInternalIdle)(uint32_t);

extern void (*pfnGFX_DifWait)(void);

extern void (*pfnGFX_DifFinNotify)(uint32_t);
extern INT32 GFX_DifResetWT(uint32_t u4GfxHwId, uint32_t u4Reset);
#endif // GFX_DIF_H



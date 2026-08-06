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


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_dif.h"
#include "gfx_hw.h"
#include "gfx_sw.h"
#include "gfx_cmdque.h"
#include "gfx_if.h"
#include "dram_model.h"
#include "chip_ver.h"
#include "drv_def.h"
#include <linux/string.h>

//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------
//#if ((1 ==CONFIG_DRAM256_MODEL) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8550) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560))
#if (2 != GFX_HAL_HW_INST_NUM)
static MI_DIF_T _rDifData[GFX_HAL_HW_INST_NUM] =
{
   { (UINT32)(GFX_HAVE_SW_MOD + GFX_HAVE_HW_8520_MOD),
    (UINT32)E_GFX_HW_8520_MOD,
    (UINT32)0,
    (UINT32 *)NULL,
    (UINT32)TRUE,
    (BOOL)FALSE
   }/*,
   { (UINT32)(GFX_HAVE_SW_MOD + GFX_HAVE_HW_8520_MOD),
    (UINT32)E_GFX_HW_8520_MOD,
    (UINT32)0,
    (UINT32 *)NULL,
    (UINT32)TRUE,
    (BOOL)FALSE 
   }*/
};
#else
static MI_DIF_T _rDifData[GFX_HAL_HW_INST_NUM] =
{
   { (UINT32)(GFX_HAVE_SW_MOD + GFX_HAVE_HW_8520_MOD),
    (UINT32)E_GFX_HW_8520_MOD,
    (UINT32)0,
    (UINT32 *)NULL,
    (UINT32)TRUE,
    (BOOL)FALSE
   },
   { (UINT32)(GFX_HAVE_SW_MOD + GFX_HAVE_HW_8520_MOD),
    (UINT32)E_GFX_HW_8520_MOD,
    (UINT32)0,
    (UINT32 *)NULL,
    (UINT32)TRUE,
    (BOOL)FALSE 
   }
};

#endif

/** _i4DifIdle
 *  gfx engine idle flag
 */
//static INT32 _i4DifIdle = TRUE;


//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------


#if defined(GFX_ENABLE_SW_MODE)
//-------------------------------------------------------------------------
/** _GfxReturnVoid
 *  void return function
 *
 */
//-------------------------------------------------------------------------
static void _GfxReturnVoid(void)
{
}


//-------------------------------------------------------------------------
/** _GfxReturn1
 *  function returns 1
 *
 */
//-------------------------------------------------------------------------
static INT32 _GfxReturn1(UINT32 u4GfxHwId)
{
    return 1;
}
#endif // #if defined(GFX_ENABLE_SW_MODE)


//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------
/** pfnGFX_DifAction
 *  gfx command queue action function pointer
 */
//-------------------------------------------------------------------------
INT32 (*pfnGFX_DifAction)(UINT32);


//-------------------------------------------------------------------------
/** pfnGFX_DifGetInternalIdle
 *  gfx get internal idle state function pointer
 */
//-------------------------------------------------------------------------
INT32 (*pfnGFX_DifGetInternalIdle)(UINT32);


//-------------------------------------------------------------------------
/** pfnGFX_DifWait
 *  gfx wait for complete function pointer
 */
//-------------------------------------------------------------------------
void (*pfnGFX_DifWait)(void);


//-------------------------------------------------------------------------
/** pfnGFX_DifFinNotify
 *  gfx complete notification function pointer
 */
//-------------------------------------------------------------------------
void (*pfnGFX_DifFinNotify)(UINT32);


//-------------------------------------------------------------------------
/** GFX_DifSetIdle
 *  gfx set ide state
 */
//-------------------------------------------------------------------------
void GFX_DifSetIdle(UINT32 u4GfxHwId, INT32 i4Idle)
{
    _rDifData[u4GfxHwId].i4DifIdle = i4Idle;
}


//-------------------------------------------------------------------------
/** GFX_DifGetIdle
 *  gfx get idle state
 */
//-------------------------------------------------------------------------
INT32 GFX_DifGetIdle(UINT32 u4GfxHwId)
{
    return _rDifData[u4GfxHwId].i4DifIdle ;
}


//-------------------------------------------------------------------------
/** GFX_DifGetData
 *  get dif data (pointer)
 */
//-------------------------------------------------------------------------
MI_DIF_T *GFX_DifGetData(UINT32 u4GfxHwId)
{
    return &_rDifData[u4GfxHwId];
}


//-------------------------------------------------------------------------
/** GFX_DifSetNotify
 *  set notification function
 */
//-------------------------------------------------------------------------
void GFX_DifSetNotify(UINT32 u4GfxHwId, void (*pfnNotify)(UINT32))
{
    pfnGFX_DifFinNotify = pfnNotify;
}


//-------------------------------------------------------------------------
/** GFX_DifInit
 *  gfx dif init
 *  init software module
 *  init hardware module
 *  set initial state (idle)
 */
//-------------------------------------------------------------------------
INT32 GFX_DifInit(UINT32 u4GfxHwId)
{
    INT32 i4Ret;

    // initialize SW engine first, because if the hardware engine,
    // hw_init shall overide some configuration directly without notification.
    // reset counter and control register file
    GFX_DifSetIdle(u4GfxHwId, TRUE);

#if defined(GFX_ENABLE_SW_MODE)
    i4Ret = GFX_SwInit();
    if ((INT32)E_GFX_OK == i4Ret)
    {
        GFX_DifSetMode(u4GfxHwId, (UINT32)E_GFX_SW_MOD);
        MA_DIF_GFX_SW_MOD_OK(u4GfxHwId);
    }
#endif  //#if defined(GFX_ENABLE_SW_MODE)

    // setup ISR
    // reset hw and all counters
    // status check
    if (MA_DIF_HAVE_GFX_HW8520)
    {
        i4Ret = GFX_HwInit();
        if ((INT32)E_GFX_OK == i4Ret)
        {
            MA_DIF_GFX_HW8520_MOD_OK(u4GfxHwId);
            GFX_DifSetMode(u4GfxHwId, (UINT32)E_GFX_HW_8520_MOD);
        } 
    }

    // fb module does not need initialization
    MA_DIF_GFX_FB_MOD_OK(u4GfxHwId);

    return (INT32)E_GFX_OK;
}

INT32 GFX_DifUninit(UINT32 u4GfxHwId)
{
    if (MA_DIF_HAVE_GFX_HW8520)
    {
        GFX_HwUninit();
    }

    return (INT32)E_GFX_OK;
}

//-------------------------------------------------------------------------
/** GFX_DifSetRegBase
 *  gfx set dif register base address
 */
//-------------------------------------------------------------------------
INT32 GFX_DifSetRegBase(UINT32 u4GfxHwId, UINT32 *pu4Base)
{
    if (pu4Base != NULL)
    {
        _rDifData[u4GfxHwId].pu4CrBase = pu4Base;
        return (INT32)E_GFX_OK;
    }

    return -(INT32)E_GFX_INV_ARG;
}


//-------------------------------------------------------------------------
/** GFX_DifGetRegBase
 *  gfx get dif register base address
 */
//-------------------------------------------------------------------------
INT32 GFX_DifGetRegBase(UINT32 u4GfxHwId, UINT32 *pu4Base)
{
    if (pu4Base != NULL)
    {
        *pu4Base = (UINT32)(_rDifData[u4GfxHwId].pu4CrBase);
        return (INT32)E_GFX_OK;
    }
    
    return -(INT32)E_GFX_INV_ARG;
}


//-------------------------------------------------------------------------
/** GFX_DifReset
 *  gfx dif reset
 *  reset hardware module and software module
 */
//-------------------------------------------------------------------------
INT32 GFX_DifReset(UINT32 u4GfxHwId, UINT32 u4Reset)
{
    // if an HW gfx engine is available, just reset it
   
    if (MA_DIF_HAVE_GFX_HW8520)
    {
        #if 0//(( 1 == CONFIG_GFX_WT_SUPPORT)||(1 == CONFIG_DRV_ONLY))
        //#ifdef CONFIG_GFX_WT_SUPPORT
        if (0 == u4GfxHwId)
        {
            GFX_HwResetWT(0,TRUE);
        }
        #endif
        return GFX_HwReset(u4GfxHwId, u4Reset);
       
    }
    else
    {
        // else erase software control register file
        GFX_UNUSED_RET(x_memset(_rDifData[u4GfxHwId].pu4CrBase, 0, 
                (sizeof(UINT32) * GREG_FILE_SIZE)))
    }
	printk("leave difreset\n");

    return (INT32)E_GFX_OK;
}

INT32 GFX_DifResetWT(UINT32 u4GfxHwId, UINT32 u4Reset)
{
    // if an HW gfx engine is available, just reset it
    if (MA_DIF_HAVE_GFX_HW8520)
    {
        if (0 == u4GfxHwId)
        {
            GFX_HwResetWT(0,TRUE);
        }
        return GFX_HwReset(u4GfxHwId, u4Reset);
    }
    else
    {
        // else erase software control register file
        GFX_UNUSED_RET(x_memset(_rDifData[u4GfxHwId].pu4CrBase, 0, 
                (sizeof(UINT32) * GREG_FILE_SIZE)))
    }

    return (INT32)E_GFX_OK;
}

//-------------------------------------------------------------------------
/** GFX_DifSetMode
 *  gfx set operation mode (hardware/software)
 *  change function pointers and re-init command queue
 */
//-------------------------------------------------------------------------
void GFX_DifSetMode(UINT32 u4GfxHwId, UINT32 u4GfxMode)
{
    INT32 i4Ret;
    UINT32 u4GfxRegBase;
    UINT32 *pu4GfxRegBase = &u4GfxRegBase;

#if defined(GFX_ENABLE_SW_MODE)
    // sw mode setting
    if ((UINT32)E_GFX_SW_MOD == u4GfxMode)
    {
        pfnGFX_DifAction = GFX_SwAction;
        pfnGFX_DifWait            = _GfxReturnVoid;
        pfnGFX_DifGetInternalIdle = _GfxReturn1;
        
        i4Ret = GFX_SwGetRegBase(u4GfxHwId, &pu4GfxRegBase);
        VERIFY((INT32)E_GFX_OK == i4Ret);
        
        i4Ret = GFX_DifSetRegBase(u4GfxHwId, pu4GfxRegBase);
        VERIFY((INT32)E_GFX_OK == i4Ret);
    }
#endif  //#if defined(GFX_ENABLE_SW_MODE)

    // hw mode setting
    if ((UINT32)E_GFX_HW_8520_MOD == u4GfxMode)
    {
        pfnGFX_DifAction = GFX_HwAction;
        pfnGFX_DifWait            = GFX_HwWait;
        pfnGFX_DifGetInternalIdle = GFX_HwGetIdle;
        
        i4Ret = GFX_HwGetRegBase(u4GfxHwId, &pu4GfxRegBase);
        VERIFY((INT32)E_GFX_OK == i4Ret);
        
        i4Ret = GFX_DifSetRegBase(u4GfxHwId, pu4GfxRegBase);
        VERIFY((INT32)E_GFX_OK == i4Ret);
    }

    _rDifData[u4GfxHwId].u4GfxMode = u4GfxMode;
   // GFX_SET_ENG_EXE_MOD(u4GfxHwId, (INT32)u4GfxMode);  // for debug use
    i4Ret = GFX_CmdQueInit();
    VERIFY((INT32)E_GFX_OK == i4Ret);
}


//-------------------------------------------------------------------------
/** GFX_DifSetCR
 *  set control register (via dif base)
 */
//-------------------------------------------------------------------------
INT32 GFX_DifSetCR(UINT32 u4GfxHwId, UINT32 u4CrName, UINT32 u4Val)
{
    if (_rDifData[u4GfxHwId].pu4CrBase != NULL)
    {
        _rDifData[u4GfxHwId].pu4CrBase[u4CrName] = u4Val;
        return (INT32)E_GFX_OK;
    }
    
    return -(INT32)E_GFX_INV_ARG;
}


//-------------------------------------------------------------------------
/** GFX_DifGetCR
 *  get control register (via dif base)
 */
//-------------------------------------------------------------------------
INT32 GFX_DifGetCR(UINT32 u4GfxHwId, UINT32 u4CrName, UINT32 *pu4Val)
{
    if (pu4Val && _rDifData[u4GfxHwId].pu4CrBase)
    {
        *pu4Val = _rDifData[u4GfxHwId].pu4CrBase[u4CrName];
        return (INT32)E_GFX_OK;
    }
    
    return -(INT32)E_GFX_INV_ARG;
}



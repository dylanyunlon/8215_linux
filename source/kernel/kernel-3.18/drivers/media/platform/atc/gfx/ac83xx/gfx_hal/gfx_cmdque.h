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

#ifndef GFX_CMDQUE_H
#define GFX_CMDQUE_H


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_common.h"


//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

/** que capacity
 * each command occupys 8 bytes
 * there are always even commands in que when gfx engine start
 * 32768 * 8 = 262144 = 256 KB
 *  4096 * 8 =  32768 =  32 KB
 *  2048 * 8 =  16384 =  16 KB
 *  1024 * 8 =   8192 =   8 KB
 *   512 * 8 =   4096 =   4 KB
 *   256 * 8 =   2048 =   2 KB
 */
enum EGFX_CMDQUE_CAPACITY
{
    EGFX_CPT_256KB = 32768,
    EGFX_CPT_128KB = 16384,
    EGFX_CPT_64KB  =  8192,
    EGFX_CPT_32KB  =  4096,
    EGFX_CPT_16KB  =  2048,
    EGFX_CPT_8KB   =  1024,
    EGFX_CPT_4KB   =   512,
    EGFX_CPT_2KB   =   256,

    // cyc_size config
    EGFX_CQCFG_256KB = 0x3,
    EGFX_CQCFG_128KB = 0x2,
    EGFX_CQCFG_64KB  = 0x1,
    EGFX_CQCFG_32KB  = 0x0,
    EGFX_CQCFG_16KB  = 0x3,
    EGFX_CQCFG_8KB   = 0x2,
    EGFX_CQCFG_4KB   = 0x1,
    EGFX_CQCFG_2KB   = 0x0
};


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

/** gfx command queue info
 *  que capacity, que size, is idle, que top, next que (if multiple),
 *  previous index, read index, write index.
 *  basically, it is a software maintained hardware que information
 */
typedef struct _GFX_CMDQUE_T
{
    INT32 i4QueCapacity;
    INT32 i4QueSize;
    INT32 i4PrevIndex;
    INT32 i4ReadIndex;
    INT32 i4WriteIndex;
    INT32 i4Idle;
    INT32 i4ShortCmdque;
    INT32 i4CqSizeCfg;
    UINT32 bNeedFlushAll;   // flush flag, 2 - need flush src/dest data
//    BOOL  bNeedFlushAll;
//    UINT8 *pu1PrevAddr;
//    UINT8 *pu1ReadAddr;
//    UINT8 *pu1WriteAddr;
    volatile UINT64 *pu8QueTop;
    struct _GFX_CMDQUE_T *prNext;
} GFX_CMDQUE_T;


//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

extern UINT32 _GFX_GetFlushCount(UINT32 u4GfxHwId);


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------


extern INT32 GFX_CmdQueInit(void);

extern INT32 GFX_CmdQueUninit(UINT32 u4GfxHwId);

extern INT32 GFX_CmdQueReset(UINT32 u4GfxHwId);

extern INT32 GFX_CmdQueAction(UINT32 u4GfxHwId);

extern INT32 GFX_RiscPushBack(UINT32 u4GfxHwId, UINT32 u4Reg, UINT32 u4Val);

extern INT32 GFX_CmdQuePushBack(UINT32 u4GfxHwId, UINT32 u4Reg, UINT32 u4Val);
extern void GFX_CmdQueSetCqCapacity(UINT32 u4GfxHwId, INT32 i4Capacity);

extern VOID  GFX_CmdQueSetFlushAllFlag(UINT32 u4GfxHwId, UINT32 ui4FlushAllFlag);

extern void GFX_CmdQueDbgInfo(UINT32 u4GfxHwId);
// for debug use
#if defined(GFX_DEBUG_MODE)

#endif // #if defined(GFX_DEBUG_MODE)


#endif // GFX_CMDQUE_H



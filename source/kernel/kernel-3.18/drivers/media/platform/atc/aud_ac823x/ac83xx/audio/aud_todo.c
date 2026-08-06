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
//av_d_if.c
#include <windows.h>
//#include "x_typedef.h"
#include <linux/types.h>
//#include "drv_dmx.h"
//#include "drv_common.h"
//#include "drv_ibc.h"


#include "aud_drv.h"
#include "aud_config.h"
#include "aud_oal.h"
#if CONFIG_DRV_AUD_AC83XX
#include <mach/base_regs.h>
#endif

#if 0
/// This structure represents the path information.
struct _IBC_PATH_INFO
{
    DRV_InbandCmd *pOwner;      ///< The owner object.
    IBC_ID_UNION uPathId;   ///< The path ID.
};

/// This structure represents the driver layer in-band command.
struct _DRV_INBAND_CMD
{
    IBC_InbandCmd rIbcInbandCmd;    ///< The in-band command structure.
    s32 i4Id;                         ///< The in-band command ID.
    s32 i4RefCount;                   ///< The reference count.
    s32 i4NumPaths;                   ///< The number of paths.
    IBC_PathInfo rIbcPathInfo[1];       ///< The path information.
};

struct _IBC_DRIVER_OBJECT
{
    bool fgInitialized;
    uintptr_t hMutex;
} _rIbcDriverObject;
#endif

void BSP_FlushDCacheRange(uintptr_t u4Start, u32 u4Len)
{
#ifndef __linux__
    CacheRangeFlush((void *)u4Start, u4Len, CACHE_SYNC_DISCARD);
#endif // #ifndef __linux__
}

void BSP_CleanDCacheRange(uintptr_t u4Start, u32 u4Len)
{
#ifndef __linux__
    CacheRangeFlush((void *)u4Start, u4Len, CACHE_SYNC_DISCARD);
#endif // #ifndef __linux__
}

void BSP_InvDCacheRange(uintptr_t u4Start, u32 u4Len)
{
#ifndef __linux__
    CacheRangeFlush((void *)u4Start, u4Len, CACHE_SYNC_DISCARD);
#endif // #ifndef __linux__
}
#if 0
void DmxControlDumpStatus(DmxControl *pInst)
{

}

DmxControl *DmxGetControlInst(u16 ui2_id)
{
    return NULL;
}


s32 i4IbcPathRelease(IBC_PathInfo *pInst)
{
    return S_IBC_OK;
}


const IBC_InbandCmd *prIbcPathGetInbandCmd(
    IBC_PathInfo *pInst ///< [in] The instance.
    )
{
    return &pInst->pOwner->rIbcInbandCmd;
}

s32 i4IbcGetInbandCmdId(
    DRV_InbandCmd *pInst    ///< [in] The instance.
    )
{
    return pInst->i4Id;
}

s32 i4IbcPathGetInbandCmdId(
    IBC_PathInfo *pInst ///< [in] The instance.
    )
{
    return i4IbcGetInbandCmdId(pInst->pOwner);
}
#endif


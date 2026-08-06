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

#ifndef __DRV_IBC_H
#define __DRV_IBC_H

#include "x_drv_comm.h"

#define DRV_HANDLE_IBC_CALLBACK_RULE 1
#define DRV_IBC_PID_SAME_FILTERID 1

#define S_IBC_OK                (0)
#define E_IBC_OUT_OF_MEMORY     (-1)
#define E_IBC_ALREADY_RELEASED  (-2)

/// This structure represents the driver layer in-band command.
typedef struct _DRV_INBAND_CMD DRV_InbandCmd;


/// This structure represents the path information.
typedef struct _IBC_PATH_INFO IBC_PathInfo;


/// For playback buffer:
///     Allocates an in-band command.
/// \return This function returns negative value if failed.
INT32 i4IbcAllocate(
    DRV_InbandCmd **ppInst,         ///< [out] The instance.
    INT32 i4NumPaths,               ///< [in] The number of paths.
    IBC_ID_UNION uPathId[],     ///< [in] The path ID for each path.
    const IBC_InbandCmd *pCmd,  ///< [in] The in-band command.
    INT32 i4Id                      ///< [in] The in-band command ID.
    );


/// For demux:
///     Release an in-band command.
/// \return This function returns negative value if failed.
INT32 i4IbcRelease(
    DRV_InbandCmd *pInst    ///< [in] The instance.
    );


/// For demux:
///     Gets the number of paths.
/// \return This function returns the number of paths.
INT32 i4IbcGetNumPaths(
    DRV_InbandCmd *pInst    ///< [in] The instance.
    );


/// For demux:
///     Gets the path instance.
/// \return This function returns the path instance.
IBC_PathInfo *prIbcGetPathInst(
    DRV_InbandCmd *pInst,   ///< [in] The instance.
    INT32 i4Index           ///< [in] The path index.
    );


/// For demux:
///     Gets the in-band command structure.
/// \return This function returns pointer to in-band command structure.
const IBC_InbandCmd *prIbcGetInbandCmd(
    DRV_InbandCmd *pInst    ///< [in] The instance.
    );


/// For demux:
///     Gets in-band command ID.
/// \return This function returns in-band command ID.
INT32 i4IbcGetInbandCmdId(
    DRV_InbandCmd *pInst    ///< [in] The instance.
    );


/// For demux:
///     Gets the path ID from a path instance.
/// \return This function returns the path ID.
IBC_ID_UNION *puIbcPathGetPathId(
    IBC_PathInfo *pInst ///< [in] The instance.
    );


/// For path handler:
///     Gets in-band command structure from a path instance.
/// \return This function returns pointer to in-band command structure.
const IBC_InbandCmd *prIbcPathGetInbandCmd(
    IBC_PathInfo *pInst ///< [in] The instance.
    );


/// For path handler/callback:
///     Gets in-band command ID from a path instance.
/// \return This function returns in-band command ID.
INT32 i4IbcPathGetInbandCmdId(
    IBC_PathInfo *pInst ///< [in] The instance.
    );


/// For path handler:
///     Release a path.
/// \return This function returns negative value if failed.
INT32 i4IbcPathRelease(
    IBC_PathInfo *pInst ///< [in] The path instance.
    );


/// Initialize IBC driver.
/// \return This function returns negative value if failed.
INT32 i4IbcInit(void);

/// Uninitialize IBC driver.
/// \return None.
void vIbcUninit(void);

#endif // __DRV_IBC_H

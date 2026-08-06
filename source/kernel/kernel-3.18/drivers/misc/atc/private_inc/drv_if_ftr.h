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

#ifndef __DRV_IF_FTR_H
#define __DRV_IF_FTR_H


#include "drv_if.h"

/// This interface represents IID_IFilter interface.
/// This interface contains Filter functions.
typedef struct _IFilter
{
  INT32 (*pi4GetFilterType)(
    void *pvUserPrivate,        ///< [IN] user private data
    UINT32 *pu4FilterType       ///< [OUT] Filter type, refer the FilterStreamType in drv_ftr.h
  );
  INT32 (*pi4GetSupportComponentId)(
    void *pvUserPrivate,        ///< [IN] user private data
    UINT16 eComponentType,      ///< [IN] component type
    UINT16 *u2ComponentId       ///< [OUT] component ID
  );
  INT32 (*pi4LockFtr)(
    void *pvUserPrivate,        ///< [IN] user private data
    BOOL fgLock                 ///< [IN] 1: lock, 0: unlock
  );
  INT32 (*pi4ChangeFifo)(
    void *pvUserPrivate,        ///< [IN] user private data
    BOOL fgChange               ///< [IN] 1: New fifo in ESM, 0: resume fifo
  );
  INT32 (*pi4MoveFifoWpAUWIdx)(  ///< should be call during filter lock status
    void *pvUserPrivate,        ///< [IN] user private data
    UINT32 u4NewFifoWp,         ///< [IN] Fifo wp
    UINT32 u4NewAUWrIdx         ///< [IN] AU Write Idx
  );
  BOOL (*pfgIsESIFull)(
    void *pvUserPrivate         ///< [IN] user private data
  );
  INT32 (*pi4FtrFlush)(
    void *pvUserPrivate        ///< [IN] user private data
  );
} IFilter;



#endif // __DRV_IF_FTR_H

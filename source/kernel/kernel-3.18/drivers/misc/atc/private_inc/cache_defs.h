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
//
#ifndef __CACHE_DEFS_H
#define __CACHE_DEFS_H


#if __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <ceddk.h>
#include <oal_cache.h>


#define CACHE_SYNC_INVALIDATE   0x080   /* invalidate L1 Data Cache */

//------------------------------------------------------------------------------
//
//  Function:  OALInvalidateDCache/OALInvalidateDCacheLines
//
//  This functions invalidate data cache (= invalidate cache location). 
//
VOID OALInvalidateDCache();
VOID OALInvalidateDCacheLines(VOID* pAddress, UINT32 size);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif


#endif // __OAL_CACHE_H


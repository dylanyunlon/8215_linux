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

#ifndef X_LIST_H
#define X_LIST_H

#include "x_typedef.h"

//=====================================================================
// Type definitions

#define NULL_INDEX                      (-1)

typedef struct
{
    INT32           i4Size;
    INT32           i4Count;
    INT32           i4Head;
    INT32           i4Tail;
    INT32*          arNext;
    INT32*          arPrev;
} SLIST_T;


//=====================================================================
// Interface

extern BOOL SLIST_Init(SLIST_T* plist, INT32 u4ListSize);

extern BOOL SLIST_Release(SLIST_T* plist);

extern BOOL SLIST_IsEmpty(SLIST_T* plist);

extern BOOL SLIST_IsFull(SLIST_T* plist);

extern INT32 SLIST_GetCount(SLIST_T* plist);

extern INT32 SLIST_GetSize(SLIST_T* plist);

extern BOOL SLIST_IsValidIndex(SLIST_T* plist, INT32 i4Index);

extern INT32 SLIST_AddTail(SLIST_T* plist);

extern INT32 SLIST_AddHead(SLIST_T* plist);

extern INT32 SLIST_RemoveHead(SLIST_T* plist);

extern INT32 SLIST_RemoveTail(SLIST_T* plist);

extern BOOL SLIST_RemoveAt(SLIST_T* plist, INT32 i4Index);

extern BOOL SLIST_RemoveAll(SLIST_T* plist);

extern INT32 SLIST_GetHeadIndex(SLIST_T* plist);

extern INT32 SLIST_GetTailIndex(SLIST_T* plist);

extern INT32 SLIST_GetNextIndex(SLIST_T* plist, INT32 i4Index);

extern INT32 SLIST_GetPrevIndex(SLIST_T* plist, INT32 i4Index);



#endif  // X_LIST_H

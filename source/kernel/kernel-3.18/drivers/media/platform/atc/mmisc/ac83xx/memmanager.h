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

#ifndef MEMMANAGER_H
#define MEMMANAGER_H

#include <linux/types.h>

EXTERN void MM_MemDbg_Init(void);
EXTERN void MM_MemDbg_Deinit(void);
EXTERN bool MM_MemDbg_Open(void);
EXTERN bool MM_MemDbg_Close(void);
EXTERN bool MM_MemDbg_InsertNode(void *pvInsertNode);
EXTERN bool MM_MemDbg_RemoveNode(void *pvNode, void **pvAddr);
EXTERN bool MM_MemDbg_RemoveNode32(void *pvNode, void **pvAddr);
EXTERN bool MM_MemDbg_Dump(void);
EXTERN bool MM_MemDbg_Flush(void);
EXTERN bool MM_MemDbg_GetNodeSize(void *pvNode, u32 *pu4Sz);
EXTERN void BTreeInsertLib(alloc_node *pnode);
EXTERN bool BTreeDelBufNotFree(void *p);
EXTERN void *MemDbgAlloc(size_t nSize, char *lpszFileName, int nLine);
EXTERN void MemDbgFree(void *p, char *lpszFileName, int nLine);
EXTERN bool MemDBGInit(void);
EXTERN void MemDBGUninit(void);

#endif				/*  */

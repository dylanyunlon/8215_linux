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

#ifndef __X_QUEUE_H__
#define __X_QUEUE_H__

#include "x_typedef.h"

//Queue API
s32 x_queue_create(HANDLE_T*  phQueue, UINT32 u4MaxSize);
s32 x_queue_delete(HANDLE_T hQueue);
s32 x_queue_pop_head(HANDLE_T hQueue, void **ppData);
s32 x_queue_push_tail(HANDLE_T  hQueue, void *pData);
s32 x_queue_peek_nth(HANDLE_T  hQueue, UINT32 u4Index, void **ppData);
s32 x_queue_get_length(HANDLE_T  hQueue, UINT32 *pLength);

#endif 


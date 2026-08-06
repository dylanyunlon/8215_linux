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

#ifndef X_RTOS_H
#define X_RTOS_H

#include "x_typedef.h"

//=====================================================================
// Function prototypes

extern void *addr_user_to_kernel(void *addr);
extern void *addr_kernel_to_user(void *addr);

void* x_alloc_aligned_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_mem(void *pUser);
void* x_alloc_aligned_ch1_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_ch1_mem(void *pUser);
void* x_alloc_aligned_ch2_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_ch2_mem(void *pUser);

void* x_alloc_dma_mem(UINT32 u4Size);
void x_free_dma_mem(void *pUser);
void* x_alloc_aligned_dma_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_dma_mem(void *pUser);

void* x_alloc_nc_mem(UINT32 u4Size);
void x_free_nc_mem(void *pUser);
void* x_alloc_aligned_nc_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_nc_mem(void *pUser);

void* x_alloc_phy_mem(UINT32 u4Size);
void x_free_phy_mem(void *pUser);
void* x_alloc_aligned_phy_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_phy_mem(void *pUser);

void* x_alloc_dtcm_mem(UINT32 u4Size);
void x_free_dtcm_mem(void* p);
void* x_realloc_dtcm_mem(void* pv_mem,UINT32 z_new_size);

void* x_alloc_vmem(UINT32 u4Size);
void x_free_vmem(void *pUser);

#endif	// X_RTOS_H


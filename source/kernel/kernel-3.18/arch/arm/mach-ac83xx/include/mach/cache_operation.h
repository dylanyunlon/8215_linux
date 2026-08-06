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

#ifndef _CACHE_OPERATION_H_
#define _CACHE_OPERATION_H_

//#include <x_typedef.h>

/*
 * BSP_FlushDCacheRange(uint32_t u4Start, uint32_t u4End)
 *
 *    Clean and Invalidate Data Cache Range
 *    - u4Start : virtual start address (inclusive)
 *    - u4Len : length
 */
extern void BSP_FlushDCacheRange(uint32_t u4Start, uint32_t u4Len);
extern void BSP_CleanDCacheRange(uint32_t u4Start, uint32_t u4Len);
extern void BSP_InvDCacheRange(uint32_t u4Start, uint32_t u4Len);
extern asmlinkage long sys_imb(void);
extern asmlinkage long sys_imb_range(unsigned long u4Start, unsigned long u4Len);

#endif // _CACHE_OPERATION_H_

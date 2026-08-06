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

/*!
 * @file dmx_psr_util.h
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */


#ifndef DMX_PSR_UTIL_H
#define DMX_PSR_UTIL_H

#include "x_typedef.h"
#ifdef __linux__
/* #include <media/atc/mm_debug.h> */
#else  /* __linux__*/
#include "mm_debug.h"
#endif /* __linux__*/
#include "dmx_psr_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

bool PSR_IsNonHdrVideoType(VCodeC eVCodeC);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_PSR_UTIL_H*/


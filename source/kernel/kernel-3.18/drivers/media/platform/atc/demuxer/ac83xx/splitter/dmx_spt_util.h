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
*/

/*!
 * @file dmx_spt_util.h
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_SPT_UTIL_H
#define DMX_SPT_UTIL_H

#include "x_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif


u32 SplitterGetCfaStreamType(u32 u4SptStreamType, u32 u4SptStreamNo);

u32 Spt4CfaGetPitureType(u32 u4CfaPictureType);

u32 Spt4CfaGetAudioCodec(u32 u4CfaAudioCodec);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_SPT_UTIL_H*/


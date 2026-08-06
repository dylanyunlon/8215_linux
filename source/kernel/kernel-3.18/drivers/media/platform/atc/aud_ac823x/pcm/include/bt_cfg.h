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

#ifndef BT_CFG_H
#define BT_CFG_H


#include <windows.h>


#define IN
#define OUT

#define ENABLE_AEC				1U
#define ENABLE_NDC				1U
#define ENABLE_DMNR				1U

#define BT_FRAME_SAMPLE			160U
#define BT_FRAME_LEN			320U
#define BT_FRAME_LEN2			640U

#if defined(__cplusplus)
extern "C" {
#endif


#define DL_FILE "/data/fe.wav"
#define UL_FILE "/data/ne.wav"
#define UL2_FILE "/data/ne2.wav"

#if defined(__cplusplus)
}
#endif

#endif

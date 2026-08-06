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

#ifndef H_VARDEF
#define H_VARDEF

#include "chip_ver.h"
#include <linux/types.h>

#ifdef _DSP_GLOBAL_VARIABLES
#define EXTERN_DSP
#else
#define EXTERN_DSP extern
#endif //_DSP_GLOBAL_VARIABLES

EXTERN_DSP bool   g_fgDspSInt;      // for compile
EXTERN_DSP u32 g_u4DspSIntAddr;  // for compile
EXTERN_DSP u32 g_u4DspSIntSD;    // for compile
EXTERN_DSP u32 g_u4DspSIntLD;    // for compile

EXTERN_DSP u32 g_u4DspRIntSD;
EXTERN_DSP u32 g_u4DspRIntLD;
EXTERN_DSP u32 g_fgDspId;
EXTERN_DSP u32 g_u4DspUop;
EXTERN_DSP u32 g_u4DspHUop;
EXTERN_DSP u32 g_u4DspTimerCnt;

EXTERN_DSP bool g_fgDspUop;
EXTERN_DSP bool g_fgDspHUop;
EXTERN_DSP bool g_fgDspASInt;
EXTERN_DSP bool g_fgDspBSInt;
EXTERN_DSP u32  g_u4DspASIntAddr;
EXTERN_DSP u32 g_u4DspASIntSD;
EXTERN_DSP u32 g_u4DspASIntLD;
EXTERN_DSP u32 g_u4DspBSIntAddr;
EXTERN_DSP u32 g_u4DspBSIntSD;
EXTERN_DSP u32 g_u4DspBSIntLD;

EXTERN_DSP bool   g_fgDspARInt;
EXTERN_DSP bool   g_fgDspBRInt;

EXTERN_DSP bool g_fgDspPtsSet;
EXTERN_DSP bool g_fgIECRAWOff;
EXTERN_DSP bool g_fgIecMuteState;
EXTERN_DSP bool g_fgIecMuteStateHdmi;
EXTERN_DSP bool g_fgIsAoutConnected;
EXTERN_DSP u8 g_u1DspCurrSampRate;

#endif //H_VARDEF

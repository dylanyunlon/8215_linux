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

#ifndef VGA_DEBUG_H_
#define VGA_DEBUG_H_

/***Macro Define***/
#define SUPPORT_VGA_AMBIGUOUS_H_DETECT 1
#define HDTV_240P_SUPPORT	0  // TG39  Interlace / Noninterlace switch
#define HDTV_540P_SUPPORT	0
#define HDTV_288P_SUPPORT	0  // TG39  Interlace / Noninterlace switch
#ifndef NEW_SYNC_SLICER_CIRCUIT
#define ADAPTIVE_SLICER_ENABLE 1
#endif
#if ADAPTIVE_SLICER_ENABLE
#define SUPPORT_SET_SLICER 0           //wu add
#define ADAPTIVE_SLICER_PLLERR_CHK 1
#define ADAPTIVE_SLICER_FILED_CHK 0
#define ADAPTIVE_MONITOR_SLICER_MEASURE 1
#if ADAPTIVE_MONITOR_SLICER_MEASURE
#define ADAPTIVE_MONITOR_SLICER_MEASURE_DEBUG 1
#define ADAPTIVE_MONITOR_SLICER_MEASURE_ONLINE 1
#define ADAPTIVE_SLICER_DEFAULT_SETTING 0
#else
#define ADAPTIVE_MONITOR_SLICER_MEASURE_DEBUG 0
#define ADAPTIVE_MONITOR_SLICER_MEASURE_ONLINE 0
#define ADAPTIVE_SLICER_DEFAULT_SETTING 1
#endif
#else
#define SUPPORT_SET_SLICER 1    //wu add
#define ADAPTIVE_SLICER_PLLERR_CHK 0
#define ADAPTIVE_SLICER_FILED_CHK 0
#define ADAPTIVE_SLICER_DEFAULT_SETTING 0
#define ADAPTIVE_MONITOR_SLICER_MEASURE 0
#if ADAPTIVE_MONITOR_SLICER_MEASURE
#define ADAPTIVE_MONITOR_SLICER_MEASURE_DEBUG 0
#define ADAPTIVE_MONITOR_SLICER_MEASURE_ONLINE 0
#define ADAPTIVE_SLICER_DEFAULT_SETTING 0
#else
#define ADAPTIVE_MONITOR_SLICER_MEASURE_DEBUG 0
#define ADAPTIVE_MONITOR_SLICER_MEASURE_ONLINE 0
#define ADAPTIVE_SLICER_DEFAULT_SETTING 0
#endif
#endif

#ifdef __MODEL_slt__
#define DECODER_ADD_WIDTH   0
#else
#define DECODER_ADD_WIDTH   0
#endif

#endif




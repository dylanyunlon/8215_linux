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

#ifndef VIDEO_TIMING_H_
#define VIDEO_TIMING_H_

#include "vga_table.h"


#define SUPPORT_VGA_AMBIGUOUS_H_DETECT 1


#define HDTV_SEARCH_START   (u8)1
#define HDTV_SEARCH_END     (bHdtvTimings-(u8)1)

#define VGA_SEARCH_START        (bHdtvTimings)  //1
#define VGA_SEARCH_END      (bAllTimings-(u8)1)

#define DVI_SEARCH_START        (bHdtvTimings)
#define DVI_SEARCH_END      (bHdtvTimings+bVgaTimings-(u8)1)

#define MAX_TIMING_FORMAT   (bAllTimings)

enum
{
    MODE_NOSIGNAL = 0,        // No signal
    MODE_525I_OVERSAMPLE = 1,      //SDTV 
    MODE_625I_OVERSAMPLE,       //
    MODE_480P_OVERSAMPLE,       //SDTV
    MODE_576P_OVERSAMPLE,
    MODE_720p_50,               //HDTV 
    MODE_720p_60,               //HDTV   
    MODE_1080i_48,              //HDTV  
    MODE_1080i_50,              //HDTV  
    MODE_1080i,                 //HDTV
    MODE_1080p_24,              //HDTV 
    MODE_1080p_25,
    MODE_1080p_30,
    MODE_1080p_50,              //HDTV 
    MODE_1080p_60,
    MODE_525I,
    MODE_625I,
    MODE_480P,
    MODE_576P,    
    MODE_720p_24,
    MODE_720p_25,    
    MODE_720p_30,        
    MODE_240P,
    MODE_540P,
    MODE_288P,    
    MODE_480P_24,    
    MODE_480P_30,        
    MODE_576P_25,    
    MODE_HDMI_640_480P,
    MODE_HDMI_720p_24,
    MODE_3D_720p_50_FP,
    MODE_3D_720p_60_FP,
    MODE_3D_1080p_24_FP,
    MODE_3D_1080I_60_FP,
    MODE_3D_480p_60_FP,
    MODE_3D_576p_50_FP,
    MODE_3D_720p_24_FP,
    MODE_3D_720p_30_FP,
    MODE_3D_1080p_30_FP,
    MODE_3D_480I_60_FP,
    MODE_3D_576I_60_FP,
    MODE_3D_1080I_50_FP,
    MODE_3D_1080p_50_FP,    
    MODE_3D_1080p_60_FP,
    MODE_3D_1650_750_60_FP,
    MODE_3D_1650_1500_30_FP,
    MODE_3D_640_480p_60_FP,
    MODE_3D_1440_240p_60_FP,
    MODE_3D_1440_288p_50_FP,
    MODE_3D_1440_576p_50_FP,
    MODE_3D_720p_25_FP,
    MODE_3D_1080p_25_FP,
    MODE_3D_1080I_1250TOTAL_50_FP,
    MODE_3D_1080p_24_SBS_FULL,
    MODE_3D_1080p_25_SBS_FULL,
    MODE_3D_1080p_30_SBS_FULL,
    MODE_3D_1080I_50_SBS_FULL,
    MODE_3D_1080I_60_SBS_FULL,
    MODE_3D_720p_24_SBS_FULL,
    MODE_3D_720p_30_SBS_FULL,
    MODE_3D_720p_50_SBS_FULL,
    MODE_3D_720p_60_SBS_FULL,
    MODE_3D_480p_60_SBS_FULL,
    MODE_3D_576p_50_SBS_FULL,
    MODE_3D_480I_60_SBS_FULL,
    MODE_3D_576I_50_SBS_FULL,
    MODE_3D_640_480p_60_SBS_FULL,
    MODE_3D_640_480p_60_LA,
    MODE_3D_240p_60_LA,
    MODE_3D_288p_50_LA,
    MODE_3D_480p_60_LA,
    MODE_3D_576p_50_LA,
    MODE_3D_720p_24_LA,
    MODE_3D_720p_60_LA,
    MODE_3D_720p_50_LA,
    MODE_3D_1080p_24_LA,
    MODE_3D_1080p_25_LA,
    MODE_3D_1080p_30_LA,
    MODE_3D_480I_60_FA,
    MODE_3D_576I_50_FA,
    MODE_3D_1080I_60_FA,
    MODE_3D_1080I_50_FA,
    MODE_3D_MASTER_1080I_60_FA,
    MODE_3D_MASTER_1080I_50_FA,
    MODE_3D_480I_60_SBS_HALF,
    MODE_3D_576I_50_SBS_HALF,
    MODE_3D_1080I_60_SBS_HALF,
    MODE_3D_1080I_50_SBS_HALF,
    MODE_1080i_50_VID39,           //HDTV  
    MODE_1080P_30_2640H,           //HDTV  
    MODE_240P_60_3432H,
    MODE_576i_50_3456H_FP,
    MODE_576P_50_1728H_FP,
    MODE_480P_60_3432H,
    MODE_2576P_60_3456H,
    MODE_3D_1440_480p_60_FP,
    MODE_MAX,
    MODE_DE_MODE = 252,
    MODE_NODISPLAY = 253,
    MODE_NOSUPPORT = 254,      // Signal out of range
    MODE_WAIT = 255

};


enum
{
    B2R_CLK_MODE_27,
    B2R_CLK_MODE_74,
    B2R_CLK_MODE_148,
    B2R_CLK_MODE_MAX
};

#define fgIsUserModeTiming(bMode) (((bMode) >= bUserVgaTimingBegin) && ((bMode) < bAllTimings))
#define fgIsVgaTiming(bMode)   ((((bMode) >= DVI_SEARCH_START) && ((bMode) <= DVI_SEARCH_END)) || ((bMode) == MODE_DE_MODE) || fgIsUserModeTiming(bMode))
#define fgIsVideoTiming(bMode) (((bMode) >= HDTV_SEARCH_START) && ((bMode) <= HDTV_SEARCH_END))
#define fgIsValidTiming(bMode) (fgIsVgaTiming(bMode) || fgIsVideoTiming(bMode) ||fgIsUserModeTiming(bMode))

extern u8 _bHdtvTiming;

#define fgIsOversampleTiming() ((((_bHdtvTiming) >= MODE_525I_OVERSAMPLE) && ((_bHdtvTiming) <= MODE_625I_OVERSAMPLE)) \
                                ||(((_bHdtvTiming) >= MODE_480P_OVERSAMPLE) && ((_bHdtvTiming) <= MODE_576P_OVERSAMPLE)))


#define fgIsPScanTiming() ((_bHdtvTiming==MODE_576P) || (_bHdtvTiming==MODE_576P_OVERSAMPLE) || \
                           (_bHdtvTiming==MODE_480P) || (_bHdtvTiming==MODE_480P_OVERSAMPLE) || \
                           (_bHdtvTiming==MODE_240P) || (_bHdtvTiming==MODE_540P) || \
                           (_bHdtvTiming==MODE_720p_50) || (_bHdtvTiming==MODE_720p_60) || \
                           (_bHdtvTiming==MODE_720p_24) || (_bHdtvTiming==MODE_720p_25) || \
                           (_bHdtvTiming==MODE_720p_30))
#define fgIsVBISupportTiming() ((((_bHdtvTiming) >= MODE_525I_OVERSAMPLE) && ((_bHdtvTiming) <= MODE_576P_OVERSAMPLE)) \
                               ||(((_bHdtvTiming) >= MODE_525I) && ((_bHdtvTiming) <= MODE_576P)) \
                               ||(((_bHdtvTiming) >= MODE_480P_24) && ((_bHdtvTiming) <= MODE_576P_25)) \
                               ||((_bHdtvTiming) == MODE_720p_60) || ((_bHdtvTiming) == MODE_1080i))  //for wss hd timing
                               

#define fgIsVBISupportCCTiming() ((_bHdtvTiming == MODE_525I_OVERSAMPLE)||(_bHdtvTiming == MODE_525I))

/////////////////////////////////////////////////////////////////////
// #define MODE_NOSIGNAL 0         // No signal
// #define MODE_DE_MODE 252
// #define MODE_NODISPLAY 253
// #define MODE_NOSUPPORT 254      // Signal out of range
// #define MODE_WAIT 255

#define PROGRESSIVE 0           //Progressive Mode
#define INTERLACE 1             //Interlace Mode

#endif


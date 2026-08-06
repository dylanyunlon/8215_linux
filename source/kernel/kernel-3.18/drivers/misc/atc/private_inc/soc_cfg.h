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

//------------------------------------------------------------------------------
//
//  File:  soc_cfg.h
//
//  This file contains system constant specific for ac83xx board.
//
#ifndef __SOC_CFG_H
#define __SOC_CFG_H

//MT3360 SW Feature
#define FEATURE_DTS              (1<<1)
#define FEATURE_AC3              (1<<2)
#define FEATURE_DIVXHT           (1<<3)
#define FEATURE_VC1              (1<<4)
#define FEATURE_AVS              (1<<5)
#define FEATURE_RM               (1<<6)
#define FEATURE_H264             (1<<7)
#define FEATURE_WMA              (1<<8)
#define FEATURE_WMV              (1<<9)
#define FEATURE_FLASH            (1<<10)
#define FEATURE_3D               (1<<11)
#define FEATURE_AAC              (1<<13)
#define FEATURE_RESERVE          (1<<15)

//AC8317 SW Feature
#define FEATURE_MHL              (1<<0)
#define FEATURE_DTS              (1<<1)
#define FEATURE_DOLBY            (1<<2)
#define FEATURE_DIVX             (1<<3)
#define FEATURE_RM               (1<<6)
#define FEATURE_H265             (1<<7)
#define FEATURE_WMC              (1<<8)
#define FEATURE_3D               (1<<11)
#define FEATURE_CD               (1<<14)
#define FEATURE_DVD              (1<<15)
#define FEATURE_SUPPORT          (1<<16)
#define FEATURE_ATS              (1<<17)

extern void featureInit(unsigned int u4Feature, unsigned int p2, unsigned int p3, unsigned int p4);
extern bool fgGetChipFeature(unsigned int u4Feature);


#endif // __SOC_CFG_H


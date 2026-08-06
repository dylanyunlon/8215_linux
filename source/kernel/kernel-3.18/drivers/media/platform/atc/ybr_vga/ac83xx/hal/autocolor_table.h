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

#ifndef AUTOCOLOR_TABLE_H__
#define AUTOCOLOR_TABLE_H__
#include <linux/types.h>
/*#include "x_typedef.h"*/

extern s16  bEFuseCompensation[4][3]; 
extern u8  bVGADefaultGainTABLE_100[4][3];

extern u16  bVGADefaultGainTABLE_100_DIGITAL[4][3];
    
extern u16  bVGADefaultOffsetTABLE_100_DIGITAL[4][3];


extern u8  bVGADefaultOffsetTABLE_100[4][3];

extern u8  bVGADefaultGainTABLE_120[4][3];
extern u8  bVGADefaultOffsetTABLE_120[4][3];

extern u8  bVGADefaultGainTABLE_75_100[4][3];
extern u8  bVGADefaultOffsetTABLE_75_100[4][3];

extern u16  wColorBlankValueNew[4][3];

/*Table for 100% input signal*/
extern u8  bColorMaxType1[4][3]; 

/*Table for 120% input signal*/
extern u8  bColorMaxType2[4][3];
/*Table for SONY input signal*/
extern u8  bColorMaxType3[4][3]; 

extern u16  wOnChipColorMaxType[4][3];

extern u16  wOnChipColorGainTable[4][3];
extern u16  wOnChipColorGainTable_75[4][3];

extern u16  wYPbPrMappingVgaGainTable[4][3];
extern u8  bOnChipCalibrateTolerance[5][4];

extern u8  bOnChipCheckTolerance[5][2];
/* support 100% with 18/56 ohm*/
extern u8  GAIN_HIGH_LIMIT_100[4][3];
extern u8  GAIN_LOW_LIMIT_100[4][3]; 
/*DIGITAL_NEW_GAIN*/
extern u16  GAIN_HIGH_LIMIT_100_DIGITAL[4][3];
extern u16  GAIN_LOW_LIMIT_100_DIGITAL[4][3];

/*support 120% with 18/56 ohm*/
extern u8  GAIN_HIGH_LIMIT_120[4][3];
extern u8  GAIN_LOW_LIMIT_120[4][3]; 

extern u8  GAIN_HIGH_LIMIT_75_100[4][3];

extern u8  GAIN_LOW_LIMIT_75_100[4][3];

#endif


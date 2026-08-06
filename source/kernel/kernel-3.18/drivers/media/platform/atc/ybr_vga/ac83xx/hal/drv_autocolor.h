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

#ifndef DRV_AUTOCOLOR_H__
#define DRV_AUTOCOLOR_H__

#include "ybr_vga_util.h"
#include "drv_vga.h"
#include "drv_hdtv.h"
#include <linux/types.h>

/***Macro Define***/
#define COMPENSATE_GAIN         0
#define FULLY_HW_AUTO_CALIBRATION   1  /*fully total cancel blank level shift by H/W*/
#define DIGITAL_NEW_OFFSET    1
/*using digital line average method must make sure the analog signal is >0*/
#define BLANK_RESET 0x08 /*rock20060220*/
#define BLANK_HOLD 0x04
#define BLANK_ALWAYS 0x00
#define vSetAutoClrState(bState)    (_bAutoColorState0 = bState)
#define bGetAutoClrState()  (_bAutoColorState0)

/***Enum Define***/
enum
{
    eVFE_HW_075ohm,
    eVFE_HW_SPECIAL_TARGET,
    eYPBPR_ADC_SUPPORT_120,
    eEFuseAutocolor
};

enum
{
	Pixel_16_per_line,
	Pixel_32_per_line,
	Pixel_64_per_line,
	Pixel_128_per_line
};

enum
{
	Start_line_0,
	Start_Lline_8,
	Start_line_16,
	Start_line_24,
	Start_line_32,
	Start_line_40,
	Start_line_48,
	Start_line_56,
	Start_line_64,
	Start_line_72,
	Start_line_80,
	Start_line_88,
	Start_line_96,
	Start_line_104,
	Start_line_112,
	Start_line_120,
	Start_line_128,
};

enum
{
	Per_1_line,
	Per_2_line,
	Per_4_line,
	Per_8_line,
	Per_16_line,
	Per_32_line,
	Per_64_line,
	Per_128_line
};
enum
{
	Average_128_line,
	Average_256_line,
	Average_512_line,
	Average_1024_line
};

enum
{
	UPDATE_PER_1_FIELD,
	UPDATE_PER_2_FIELD,
	UPDATE_PER_4_FIELD,
	UPDATE_PER_8_FIELD
};

enum
{
	AFTER_ADJ,
	BEFORE_ADJ
};

enum
{
	CALI_DISABLE,
	CALI_ENABLE
};

enum
{
    VDO_AUTO_COLOR_NOT_BEGIN = 0,
    VDO_AUTO_COLOR_START,
    VDO_AUTO_COLOR_1_START,
    VDO_AUTO_COLOR_2_START,
    VDO_AUTO_COLOR_3_START,
    VDO_AUTO_COLOR_3_START_Delay1,
    VDO_AUTO_COLOR_4_START,
    VDO_AUTO_COLOR_5_START,
    VDO_AUTO_COLOR_CLAMP_DELAY,
    VDO_AUTO_COLOR_END,
    VDO_AUTO_COLOR_BLANK1,
    VDO_AUTO_COLOR_2_START_Interation1,
    VDO_AUTO_COLOR_2_START_Interation_End,
    VDO_AUTO_COLOR_1P0_START,
};

enum
{
    ON_CHIP_AUTO_COLOR_DO_NOTHING,
    ON_CHIP_AUTO_COLOR_INITIAL_STATE,
    ON_CHIP_AUTO_COLOR_SEARCH_TARGET,
    ON_CHIP_AUTO_COLOR_WAIT_VALUE,
    ON_CHIP_AUTO_COLOR_CHECK_VALUE,
    ON_CHIP_AUTO_COLOR_FINISH,
    ON_CHIP_AUTO_COLOR_END
};

enum
{
   BLK_NOTHING,
   BLK_START,
   BLK_LEVEL_STABLE,
   BLK_OFFSET_ADJ,
   BLK_OFFSET_DELAY,  /* [SA7_Michael] 080828 for adjusting blanking level stable*/
   BLK_OFFSET_DELAY1,
   BLK_OFFSET_DELAY2,
   BLK_OFFSET_CHECK,
   BLK_GAIN_ADJ,
   BLK_DELAY,
   BLK_MEASURE,
   BLK_CLAMP_DELAY,
   BLK_APICMD,
   BLK_END  
};

enum
{
    ON_CHIP_GAIN_MODE,
    ON_CHIP_OFFSET_MODE,
    OFFSET_MODE,
    GAIN_MODE,
    DIGITAL_OFFSET_MODE
};

/***Function Declaration***/
void vDrvIntAutoColorStart(void) ;
void bDrvSetHDADCGain(u8 bOrder,u8 value);
void bDrvSetHDADCOffset(u8 bOrder,u8 value);
void vDrvSetHDADCDigitalOffset(u8 bOrder,u16 value);
void bDrvSetHDADCGain_Digital(u8 bOrder, u16 value);
u8 bDrvOnChipGetVFESignalType(u8 bAutoInput);
u8 bDrvOnChipAutoColorCheckSignalReady(void);
u16 bDrvGetBlankVal(u8 bOrder);
u8 bDrvOnChipGetADCMaxMinValue(u8 bOrder, u8 bMode);
u8 bDrvOnChipGetADCMaxValueRGBYPBPR(u8 bOrder);
void vDrvOnChipAutoColor_GainOffset(u8 bMode); /*for ON_CHIP_AUTO_COLOR*/
void vDrvOnChipAutoColorIteration(void);
void vDrvAdjustBlankLevel(void);
void vDrvEnableBlankLevelAdjust(void);
void vDrvEnableBlankLevelMeasure(void);
void vDrvEnableADCLinearityVerify(u8 bType);
void vDrvEnablePGALinearityVerify(void);
void vDrvPGALinearityVerify(void);
void vDrvADCLinearityVerify(void);
void vDrvHDTVMeasureSetting(u8 bField_Number);
void vDrvHDTVClampMethodDefaultSetting(u8 bType);
u8 vCust_Current_Cal_Status(void);
void vDrvHDTV_HW_AUTO_ONOFF(u8 bMode, u8 bType);

/***Extern Declaration***/
extern u8   _bAutoColorState0;
extern u16 _bAutoColorGain[4][3];
extern u16  _bAutoColorOffset[4][3];
extern u16  _bAutoColorGain_for_EFuse[4][3];
extern u8  _bAutoColorHistory[4];
extern u8 _bOffsetCaliDone[4];

extern u16  wColorBlankValueNew[4][3];
extern u8  bColorMaxType1[4][3];
extern u8  bColorMaxType2[4][3];
extern u8  bColorMaxType3[4][3];
extern u16  wOnChipColorMaxType[4][3];
extern u16  wOnChipColorGainTable[4][3];
extern u16  wOnChipColorGainTable_75[4][3];
extern u16  wYPbPrMappingVgaGainTable[4][3];
/*extern u16  wOnChipColorGainTable_SONY[4][3];*/
extern u8  bOnChipCalibrateTolerance[5][4];
extern u8  bOnChipCheckTolerance[5][2];
extern u8  GAIN_HIGH_LIMIT_100[4][3];
extern u8  GAIN_LOW_LIMIT_100[4][3];
extern u8  GAIN_HIGH_LIMIT_120[4][3];
extern u8  GAIN_LOW_LIMIT_120[4][3];
extern u8  GAIN_HIGH_LIMIT_75_100[4][3];
extern u8  GAIN_LOW_LIMIT_75_100[4][3];
extern u8  bVGADefaultOffsetTABLE_100[4][3];

extern u16  GAIN_HIGH_LIMIT_100_DIGITAL[4][3];
extern u16  GAIN_LOW_LIMIT_100_DIGITAL[4][3];

#endif

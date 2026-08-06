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

#ifndef PANEL_H
#define PANEL_H

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include "x_lint.h"
LINT_EXT_HEADER_BEGIN
#include "x_typedef.h"
LINT_EXT_HEADER_END


/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * PANEL LIST, PLEASE FOLLOW NAMING FORMAT WHEN ADDING NEW PANEL:
 * 1.add model suffix to avoid the different resolution in same panel size
 * 2.add resolution info for user friendly
 * 3.DO NOT change panel index, use PANEL_TOTAL_NUMBER as new panel's index
 *   and increment PANEL_TOTAL_NUMBER
 * 4.If panel use SW PWM, add PMW suffix like PANEL_LG_26_WX2_PWM
 *---------------------------------------------------------------------------*/

#define PANEL_PWM_MASK 0x80
#define PANEL_INVALID 0xff

#define PANEL_XGA 0 /* 1024x768*/
#define PANEL_SXGA 1 /* 1280x1024*/

#define PANEL_CMO_154_C1_P1 53 /* 1440x900*/
#define PANEL_CMO_154_I2_L2 54 /* 1280x800*/
#define PANEL_CMO_19_A1 2 /* 1440x900*/
#define PANEL_CMO_22_Z1 22 /* 1680x1050*/
#define PANEL_CMO_26_B1 24 /* 1366x768 (LVDS=5-V, BL=24-V)*/
#define PANEL_CMO_26_B1_PWM (PANEL_CMO_26_B1 | PANEL_PWM_MASK)
#define PANEL_CMO_27_W1 3 /* 1280x720*/
#define PANEL_CMO_27_B1 4 /* 1366x768*/
#define PANEL_CMO_315_B1 28 /* 1366x768*/
#define PANEL_CMO_32_B1 5 /* 1366x768*/
#define PANEL_CMO_37_H1 6 /* 1920x1080*/
#define PANEL_CMO_42_H1 7 /* 1920x1080*/
#define PANEL_CMO_42_H1_L5 52 /* 1920x1080*/

#define PANEL_AUO_20_XW2 8 /* 1366x768*/
#define PANEL_AUO_26_XW3 30 /* 1366x768 (LVDS=5-V, BL=24-V)*/
#define PANEL_AUO_315_XW2_V0 31 /* 1366x768 (LVDS=5-V, BL=24-V)*/
#define PANEL_AUO_315_XW2_V1 32 /* 1366x768 (LVDS=5-V, BL=24-V)*/
#define PANEL_AUO_315_XW2_V3 48 /* 1366x768*/
#define PANEL_AUO_315_XW2_V6 35 /* 1366x768 (LVDS=5-V, BL=24-V) 120Hz*/
#define PANEL_AUO_32_XW1 9 /* 1366x768*/
#define PANEL_AUO_32_XW2 27 /* 1366x768*/
#define PANEL_AUO_37_XW1_V1 45 /* 1366x768*/
#define PANEL_AUO_37_XW1_V2 46 /* 1366x768*/
#define PANEL_AUO_37_XW2 20 /* 1366x768*/
#define PANEL_AUO_37_XW2_V5 42 /* 1366x768*/
#define PANEL_AUO_37_HW1 10 /* 1920x1080*/
#define PANEL_AUO_42_XW1_V3 43 /* 1366x768*/
#define PANEL_AUO_42_HW1_V0 50 /* 1920x1080*/

#define PANEL_LG_26_W1 11 /* 1280x768*/
#define PANEL_LG_26_WX2 19 /* 1366x768*/
#define PANEL_LG_26_WX2_PWM (PANEL_LG_26_WX2 | PANEL_PWM_MASK)
#define PANEL_LG_32_W1 12 /* 1366x768*/
#define PANEL_LG_32_W1_PWM (PANEL_LG_32_W1 | PANEL_PWM_MASK)
#define PANEL_LG_32_WX3_SLB1 36 /* 1366x768 (LVDS=12-V, BL=24-V)*/
#define PANEL_LG_37_WX1 13 /* 1366x768*/
#define PANEL_LG_37_WX1_SL2 41 /* 1366x768*/
#define PANEL_LG_37_WX1_SLA1 37 /* 1366x768 (LVDS=12-V, BL=24-V)*/
#define PANEL_LG_37_WU1 14 /* 1920x1080*/
#define PANEL_LG_42_W2 15 /* 1366x768*/
#define PANEL_LG_42_W2_SLA2 47 /* 1366x768*/
#define PANEL_LG_42_WU2 25 /* 1920x1080 (LVDS=12-V, BL=24-V)*/
#define PANEL_LG_42_WU3_SLA1 51 /* 1920x1080*/
#define PANEL_LG_47_WU4 21 /* 1920x1080*/
#define PANEL_LG_47_WU4_PWM (PANEL_LG_47_WU4 | PANEL_PWM_MASK)
#define PANEL_LG_50_X4_PDP 29 /* 1366x768*/

#define PANEL_SS_19_M2 23 /* 1440x900 (LVDS=5-V, BL=12-V)*/
#define PANEL_SS_23_W2_L1 34 /* 1366x768*/
#define PANEL_SS_40_WT 16 /* 1366x768*/

#define PANEL_CLAA_15_XP3 17 /* 1024x768*/
#define PANEL_CLAA_32_WB2 38 /* 1366x768*/
#define PANEL_CLAA_37_WA3 55 /* 1366x768*/

#define PANEL_QD_32_HL1 18 /* 1366x768*/
#define PANEL_QD_32_HL1_W 44 /* 1366x768*/
#define PANEL_HSD_19_MGW1 26 /* 1440x900 (LVDS=5-V, BL=12-V)*/

#define PANEL_PV_154_LCM_C01 33 /* 1440x900*/
#define PANEL_PV_154_LCM_C04 39 /* 1280x800*/
#define PANEL_PV_201_TVM_C01 40 /* 1400x1050*/
#define PANEL_PV_260_TVM_A01H 56 /* 1366x768*/
#define PANEL_PV_260_TVM_C01H 57 /* 1366x768*/
#define PANEL_PV_320_TVM_A01H 49 /* 1366x768*/
#define PANEL_PV_370_TVM_A01H 58 /* 1366x768*/


#define PANEL_TOTAL_NUMBER 59


/* for backward compatible...but not recommended*/
#define PANEL_DEFAULT PANEL_XGA
#define PANEL_CMO_19 PANEL_CMO_19_A1
#define PANEL_CMO_27_S1 PANEL_CMO_27_W1
#define PANEL_CMO_32_S1 PANEL_CMO_32_B1
#define PANEL_CMO_42 PANEL_CMO_42_H1
#define PANEL_AUO_32 PANEL_AUO_32_XW1
#define PANEL_LG_26_S1 PANEL_LG_26_W1
#define PANEL_LG_32 PANEL_LG_32_W1
#define PANEL_LG_37 PANEL_LG_37_WU1
#define PANEL_SS_40 PANEL_SS_40_WT


/*----------------------------------------------------------------------------
 * If any one want to change PANEL_SELECT settings, you can have following
 * methods to change PANEL_SELECT settings.
 * 1. Build image by "make EXTRA_DEFINES=-DPANEL_SELECT=PANEL_CMO_27_W1"
 * 2. Edit project_x/target/mt5371/drv_opt/common.def to add one
 *   more line "-DPANEL_SELECT=PANEL_CMO_27_W1 \" for middleware program.
 *---------------------------------------------------------------------------*/
#ifndef PANEL_SELECT    /* NO CHANGE */
#define PANEL_SELECT PANEL_DEFAULT /* DO NOT CHANGE THIS SETTING!!! */
#endif /* PANEL_SELECT */


/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/* display mode*/
#define DISP_MODE_OFF 0
#define DISP_MODE_ON 1
#define DISP_MODE_USE DISP_MODE_OFF

/* lvds*/
#define LVDS_OUT_OFF 0
#define LVDS_OUT_ON 1
#define LVDS_OUT_USE LVDS_OUT_ON
#define LVDS_RESERVED_LOW 0
#define LVDS_RESERVED_HIGH 1
#define LVDS_RESERVED_FIELD 2
#define LVDS_RESERVED_TYPE LVDS_RESERVED_LOW

/* no tube in 537x*/
#define DISPLAY_LCD 0
#define DISPLAY_TUBE 1
#define USE_DISPLAY_TYPE DISPLAY_LCD

/* no interlace mode in 537x*/
#define PROGRESSIVE_MODE 0
#define INTERLACE_MODE 1
#define INTERLACE_ENABLE PROGRESSIVE_MODE

/* dac*/
#define DAC_OUT_OFF 0
#define DAC_OUT_ON 1
#define DAC_OUT_USE DAC_OUT_ON
#define DA_CLOCK_NORMAL 0
#define DA_CLOCK_INVERSE 1
#define DA_CLOCK_POARITY DA_CLOCK_NORMAL
#define DACCLK_DELAY_SEL 0

/* svm*/
#define SVM_DISABLE 0
#define SVM_ENABLE 1
#define SUPPORT_SVM SVM_DISABLE

/* ttl*/
#define TTL_OUT_OFF 0
#define TTL_OUT_ON 1
#define TTL_OUT_USE TTL_OUT_OFF
#define TTL_CLOCK_NORMAL 0
#define TTL_CLOCK_INVERSE 1
#define TTL_CLOCK_POARITY TTL_CLOCK_NORMAL
#define TTLCLK_DELAY_SEL 0
#define TTL_DRIVING_SEL 0x1

/* scpos*/
#define SRGB_EN_OFF 0
#define SRGB_EN_ON 1
#define SRGB_FUNCTION_EN SRGB_EN_OFF
#define DEN_NO_ACIVE 0
#define DEN_ACIVE 1
#define DEN_TYPE DEN_NO_ACIVE
#define DEN_HIGH 0
#define DEN_LOW 1
#define DEN_POLARITY DEN_HIGH
#define FIELD_HIGH 0
#define FIELD_LOW 1
#define FIELD_POLARITY FIELD_HIGH
#define HSYNC_HIGH 0
#define HSYNC_LOW 1
#define HSYNC_POLARITY HSYNC_HIGH
#define VSYNC_HIGH 0
#define VSYNC_LOW 1
#define VSYNC_POLARITY VSYNC_HIGH
#define RB_CHANNEL_INVERSE_OFF 0
#define RB_CHANNEL_INVERSE_ON 1
#define RB_CHANNEL_INVERSE RB_CHANNEL_INVERSE_OFF
#define BIT_ORDER_INVERSE_OFF 0
#define BIT_ORDER_INVERSE_ON 1
#define BIT_ORDER_INVERSE BIT_ORDER_INVERSE_OFF

/* dither*/
#define NO_DITHER 0 /*no dither*/
#define DITHER_1210 1 /*12-bit dither to 10-bit*/
#define DITHER_128 2 /*12-bit dither to 8-bit*/
#define DITHER_126 3 /*12-bit dither to 6-bit*/
#define DITHER_MODE NO_DITHER
#define ROUND_OFF 0
#define ROUND_ON 1
#define ROUND_FUNCTION_EN ROUND_OFF
#define DITHER_ROUND_OFF 0
#define DITHER_ROUND_ON 1
#define DITHER_ROUND_EN DITHER_ROUND_OFF
#define ERROR_DITHER_OFF 0
#define ERROR_DITHER_ON 1
#define ERROR_DITHER_EN ERROR_DITHER_OFF
#define FPHASE_VALUE 0x00

/* dump flag*/
#define PANEL_DUMP_CURRENT 0
#define PANEL_DUMP_ALL 1
#define PANEL_DUMP_TITLE 2


/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

EXTERN void ReadPanelIndexFromEeprom(void);
EXTERN void WritePanelIndexToEeprom(__u32 u4PanelIndex);
EXTERN void SelectPanel(__u32 u4Index);
EXTERN __s8 *GetCurrentPanelName(void);
EXTERN __u32 GetCurrentPanelIndex(void);
EXTERN __s8 *GetPanelName(__u32 u4Index);
EXTERN void DumpPanelAttribute(__u32 u4Flag);
EXTERN bool IsUseSwPwm(void);
EXTERN __u32 IsLvdsMsbSwOn(void);
EXTERN __u32 IsPwmLowPanelBright(void);
EXTERN __u32 PANEL_GetBacklightHigh(void);
EXTERN __u32 PANEL_GetBacklightMiddle(void);
EXTERN __u32 PANEL_GetBacklightLow(void);

EXTERN __u32 PANEL_GetPanelWidth(void);
EXTERN __u32 PANEL_GetPanelHeight(void);
EXTERN __u32 PANEL_GetPixelClkMax(void);
EXTERN __u32 PANEL_GetPixelClk60Hz(void);
EXTERN __u32 PANEL_GetPixelClk50Hz(void);
EXTERN __u32 PANEL_GetPixelClkMin(void);
EXTERN __u32 PANEL_GetHTotalMax(void);
EXTERN __u32 PANEL_GetHTotal60Hz(void);
EXTERN __u32 PANEL_GetHTotal50Hz(void);
EXTERN __u32 PANEL_GetHTotalMin(void);
EXTERN __u32 PANEL_GetHSyncLen60Hz(void);
EXTERN __u32 PANEL_GetHSyncLen50Hz(void);
EXTERN __u32 PANEL_GetVTotalMax(void);
EXTERN __u32 PANEL_GetVTotal60Hz(void);
EXTERN __u32 PANEL_GetVTotal50Hz(void);
EXTERN __u32 PANEL_GetVTotalMin(void);
EXTERN __u32 PANEL_GetVClkMax(void);
EXTERN __u32 PANEL_GetVClkMin(void);
EXTERN __u32 PANEL_GetHSyncWidth(void);
EXTERN __u32 PANEL_GetVSyncWidth(void);
EXTERN __u32 PANEL_GetHPosition(void);
EXTERN __u32 PANEL_GetVPosition(void);
EXTERN __u32 PANEL_GetControlWord(void);
EXTERN __u32 PANEL_GetBacklightHighHwPwm(void);
EXTERN __u32 PANEL_GetBacklightMiddleHwPwm(void);
EXTERN __u32 PANEL_GetBacklightLowHwPwm(void);
EXTERN __u32 PANEL_GetDimmingFrequency60Hz(void);
EXTERN __u32 PANEL_GetBacklightHighSwPwm(void);
EXTERN __u32 PANEL_GetBacklightMiddleSwPwm(void);
EXTERN __u32 PANEL_GetBacklightLowSwPwm(void);
EXTERN __u32 PANEL_GetDimmingFrequency50Hz(void);
EXTERN __u32 PANEL_GetLvdsOnDalay(void);
EXTERN __u32 PANEL_GetBacklightOnDelay(void);
EXTERN __u32 PANEL_GetBacklightOffDelay(void);
EXTERN __u32 PANEL_GetLvdsOffDalay(void);
EXTERN __u32 PANEL_GetMaxOverscan(void);

EXTERN void PANEL_SetPanelWidth(__u32 u4Value);
EXTERN void PANEL_SetPanelHeight(__u32 u4Value);
EXTERN void PANEL_SetPixelClkMax(__u32 u4Value);
EXTERN void PANEL_SetPixelClk60Hz(__u32 u4Value);
EXTERN void PANEL_SetPixelClk50Hz(__u32 u4Value);
EXTERN void PANEL_SetPixelClkMin(__u32 u4Value);
EXTERN void PANEL_SetHTotalMax(__u32 u4Value);
EXTERN void PANEL_SetHTotal60Hz(__u32 u4Value);
EXTERN void PANEL_SetHTotal50Hz(__u32 u4Value);
EXTERN void PANEL_SetHTotalMin(__u32 u4Value);
EXTERN void PANEL_SetHSyncLen60Hz(__u32 u4Value);
EXTERN void PANEL_SetHSyncLen50Hz(__u32 u4Value);
EXTERN void PANEL_SetVTotalMax(__u32 u4Value);
EXTERN void PANEL_SetVTotal60Hz(__u32 u4Value);
EXTERN void PANEL_SetVTotal50Hz(__u32 u4Value);
EXTERN void PANEL_SetVTotalMin(__u32 u4Value);
EXTERN void PANEL_SetVClkMax(__u32 u4Value);
EXTERN void PANEL_SetVClkMin(__u32 u4Value);
EXTERN void PANEL_SetHSyncWidth(__u32 u4Value);
EXTERN void PANEL_SetVSyncWidth(__u32 u4Value);
EXTERN void PANEL_SetHPosition(__u32 u4Value);
EXTERN void PANEL_SetVPosition(__u32 u4Value);
EXTERN void PANEL_SetControlWord(__u32 u4Value);
EXTERN void PANEL_SetBacklightHighHwPwm(__u32 u4Value);
EXTERN void PANEL_SetBacklightMiddleHwPwm(__u32 u4Value);
EXTERN void PANEL_SetBacklightLowHwPwm(__u32 u4Value);
EXTERN void PANEL_SetDimmingFrequency60Hz(__u32 u4Value);
EXTERN void PANEL_SetBacklightHighSwPwm(__u32 u4Value);
EXTERN void PANEL_SetBacklightMiddleSwPwm(__u32 u4Value);
EXTERN void PANEL_SetBacklightLowSwPwm(__u32 u4Value);
EXTERN void PANEL_SetDimmingFrequency50Hz(__u32 u4Value);
EXTERN void PANEL_SetLvdsOnDalay(__u32 u4Value);
EXTERN void PANEL_SetBacklightOnDelay(__u32 u4Value);
EXTERN void PANEL_SetBacklightOffDelay(__u32 u4Value);
EXTERN void PANEL_SetLvdsOffDalay(__u32 u4Value);
EXTERN void PANEL_SetMaxOverscan(__u32 u4Value);


/*---------------------------------------------------------------------------*/
/* Macro definitions*/
/*---------------------------------------------------------------------------*/

/* replace these legacy global variable with function call*/

#define TV_WIDTH PANEL_GetPanelWidth()
#define TV_HEIGHT PANEL_GetPanelHeight()

#define HLEN_TOTAL_TYPI PANEL_GetHTotal60Hz()
#define VLEN_TOTAL_TYPI PANEL_GetVTotal60Hz()
#define HSYNCLEN_TYPI PANEL_GetHSyncLen60Hz()
#define LEFT_MASK_TYPI 0
#define TOP_MASK_TYPI 0
#define RIGHT_MASK_TYPI (UINT16)(((HLEN_TOTAL_TYPI-HSYNCLEN_TYPI)-PANEL_GetPanelWidth())-LEFT_MASK_TYPI)
#define BOTTOM_MASK_TYPI (UINT16)((VLEN_TOTAL_TYPI-PANEL_GetPanelHeight())-TOP_MASK_TYPI)

#define HLEN_TOTAL_50HZ PANEL_GetHTotal50Hz()
#define VLEN_TOTAL_50HZ PANEL_GetVTotal50Hz()
#define HSYNCLEN_50HZ PANEL_GetHSyncLen50Hz()
#define LEFT_MASK_50HZ 0
#define TOP_MASK_50HZ 0
#define RIGHT_MASK_50HZ (UINT16)(((HLEN_TOTAL_50HZ-HSYNCLEN_50HZ)-PANEL_GetPanelWidth())-LEFT_MASK_50HZ)
#define BOTTOM_MASK_50HZ (UINT16)((VLEN_TOTAL_50HZ-PANEL_GetPanelHeight())-TOP_MASK_50HZ)

#define HS_WIDTH PANEL_GetHSyncWidth()
#define VS_WIDTH PANEL_GetVSyncWidth()
#define H_POS PANEL_GetHPosition()
#define V_POS PANEL_GetVPosition()

/* panel limitation, for display mode use*/
#define DCLK_MIN PANEL_GetPixelClkMin()
#define DCLK_MAX PANEL_GetPixelClkMax()
#define DHS_TOTAL_MIN PANEL_GetHTotalMin()
#define DHS_TOTAL_MAX PANEL_GetHTotalMax()
#define DVS_TOTAL_MIN PANEL_GetVTotalMin()
#define DVS_TOTAL_MAX PANEL_GetVTotalMax()
#define DVS_FREQ_MIN PANEL_GetVClkMin()
#define DVS_FREQ_MAX PANEL_GetVClkMax()

/* lvds*/
/* msb switch*/
#define LVDS_MSB_SW_OFF 0 /* {MSB 7,6,5,4,3,2,1,0 LSB}*/
#define LVDS_MSB_SW_ON 1 /* {MSB 5,4,3,2,1,0,7,6 LSB}*/
#define LVDS_DISP_MSB_SW IsLvdsMsbSwOn()

/* odd/even pixel stitch*/
#define LVDS_ODD_SW_OFF 0 /* even in A0~A3, odd in A4~A7*/
#define LVDS_ODD_SW_ON (1 << 1)
#define LVDS_DISP_ODD_SW (PANEL_GetControlWord() & LVDS_ODD_SW_ON)

/* lvds port*/
#define SINGLE_PORT 0
#define DUAL_PORT (1 << 2)
#define LVDS_OUTPUT_PORT (PANEL_GetControlWord() & DUAL_PORT)

/* dynamic sync frame rate, frame rate will stay 60Hz when WFB_MODE_OFF*/
#define WFB_MODE_ON 0
#define WFB_MODE_OFF (1 << 3)
#define WFB_LOCK_ENABLE (PANEL_GetControlWord() & WFB_MODE_OFF)

/* display bit*/
#define DISP_18BIT 0 /* 18-bit RGB output*/
#define DISP_24BIT (1 << 4) /* 24-bit RGB output*/
#define DISP_30BIT (2 << 4) /* 30-bit RGB output*/
#define DISP_BIT_MSK (3 << 4)
#define DISP_BIT (PANEL_GetControlWord() & DISP_BIT_MSK)

/* PWM_HIGH_PANEL_BRIGHT : panel will be brighter when duty cycle is higher*/
#define PWM_HIGH_PANEL_BRIGHT 0
#define PWM_LOW_PANEL_BRIGHT (1 << 6)
#define PANEL_BRIGHT_SETTING IsPwmLowPanelBright()

#define PANEL_BACKLIGHT_HIGH bApiGetBackLightTbl(BACKLIGHT_HIGH)
#define PANEL_BACKLIGHT_MIDDLE bApiGetBackLightTbl(BACKLIGHT_MIDDLE)
#define PANEL_BACKLIGHT_LOW bApiGetBackLightTbl(BACKLIGHT_LOW)

/* reset panel when switch frame rate*/
#define WFB_PANEL_RESET_OFF 0
#define WFB_PANEL_RESET_ON (1 << 7)
#define WFB_PANEL_RESET (PANEL_GetControlWord() & WFB_PANEL_RESET_ON)

#endif /* PANEL_H */



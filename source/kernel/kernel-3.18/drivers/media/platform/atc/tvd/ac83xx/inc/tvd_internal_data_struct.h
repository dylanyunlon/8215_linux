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

#ifndef TVD_INTERNAL_DATA_STRUCT_H_
#define TVD_INTERNAL_DATA_STRUCT_H_


enum  {
	TVD_ANALOG_CFG_INPUT,
	TVD_ANALOG_CFG_OUTPUT,
	TVD_ANALOG_CFG_CLAMP,
	TVD_ANALOG_CFG_ALL
};


typedef struct {
	u8            u1HueLvl;            /* hue gain level */
	u8            u1SaturationLvl;     /* saturation level */
	u8            u1BrightnessLvl;     /* brightness gain level */
#if SUPPORT_CALIBRATE_BRIGHTNESS
	u8            u1Mask;
	u8            u1YGain;
	u8            u1YOffset;
	u8            u1UCosGain;
	u8            u1VCosGain;
	u8            u1USinGain;
	u8            u1VSinGain;
	u8            u1UOffset;
	u8            u1VOffset;
#endif
} TVD_CORE_PREVIEW_CFG_T, *PTVD_CORE_PREVIEW_CFG_T;

#if defined(__ARM2__)
#define IO_UCV_BASE                     0xFD000000
#define REG_RW_CLKGATE_CFG6             0x00b4       /* vdout clock gated*/
#define REG_RW_SYNC_RESET_CFG6          0x00d0       /* vdout tvd sync reset*/
#define REG_RW_CLKGATE_CFG3             0x00a8       /* audio peripher*/
#define REG_RW_SYNC_RESET_CFG3          0x00c4       /* audio prepher sync reset*/

#define CLK_PDN_TVD1                    (1U << 1)/*57*/
#define CLK_PDN_TVD2                    (1U << 2)/*58*/

#define CLK_RESET_TVD1                  (1U << 1)/*57*/


#define CLK_PDN_WRITE_CHANEL            (1U << 8)	/*64*/
#define CLK_PDN_WRITE_CHANEL2			(1U << 10)	/*64*/

#define CLK_RESET_WRITE_CHANEL          (1U << 8)/*64*/
#define CLK_RESET_WRITE_CHANEL2         (1U << 10)/*64*/

#define CLK_PDN_PWM0                     (1U << 24)/*33*/
#define CLK_PDN_PWM1                     (1U << 25)/*34*/
#define CLK_PDN_PWM2                     (1U << 26)/*35*/
#define CLK_PDN_PWM3                     (1U << 27)/*36*/

#define CLK_RESET_PWM0                    (1U << 24)/*33*/
#define CLK_RESET_PWM1                    (1U << 25)/*34*/
#define CLK_RESET_PWM2                    (1U << 26)/*35*/
#define CLK_RESET_PWM3                    (1U << 27)/*36*/



#endif

#endif


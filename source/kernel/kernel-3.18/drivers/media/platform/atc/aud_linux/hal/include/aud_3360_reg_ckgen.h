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




#include "x_ckgen.h"

#ifndef _AUD_3360_REG_CKGEN_H_
#define _AUD_3360_REG_CKGEN_H_

//////////////////////////////////////////////////////////////
//      CKGEN Register
//////////////////////////////////////////////////////////////
#define REG_RISC_CFG0                       0x0004
#define REG_DVD_REG1                        0x0008
#define REG_AP_REG0                         0x000C
    #define AP_DSP_CLOCK_27M                0
    #define AP_DSP_CLOCK_SYSPLL_D3          1
    #define AP_DSP_CLOCK_ARMPLL_D2          2
    #define AP_DSP_CLOCK_APLL2_D2           3
    #define AP_DSP_CLOCK_USBPLL_D2          4
    #define AP_DSP_CLOCK_APLL2              5
    #define AP_DSP_CLOCK_DMPLL              6
    #define AP_DSP_CLOCK_SYSPLL_D2          7
#define REG_AP_REG1                         0x0010
#define REG_AP_REG2                         0x0014
#define REG_AP_REG3                         0x0018
#define REG_AP_REG4                         0x001C
    #define AUD_REG_CFG_K3_START            (16)
    #define AUD_REG_CFG_K3_NUM              (8)
#define REG_AP_REG5                         0x0020
#define REG_AP_REG6                         0x0024
    #define AUD_REG_SEL_APLL2_K3_START      (14)
    #define AUD_REG_SEL_APLL2_K3_NUM        (1)
#define REG_AP_REG7                         0x0028
#define REG_AP_REG8                         0x002C
#define REG_AP_REG9                         0x0030
    #define AUD_MICIN_MLINE_SEL_START       (26)
    #define AUD_MICIN_MLINE_SEL_NUM         (1)
#define REG_AP_REG10                        0x0034
#define DVD_AUD_MCLK_SEL                    0xA4
#define REG_TST_CLK_CFG                     0x0188
#define REG_TST_CLK_CFG1                    0x018C


//////////////////////////////////////////////////////////////
//      Multi Func Register
//////////////////////////////////////////////////////////////
#define REG_PADMUX0                         0x0054
    //[bit 0-2]define
    #define AUD_MICIN_PINMUX_START          (0)
    #define AUD_MICIN_PINMUX_NUM            (3)
    #define AUD_MICIN_GROUP_GPIO             1
    #define AUD_MICIN_GROUP_NLD              2
    #define AUD_MICIN_GROUP_AIN0             3
    #define AUD_MICIN_GROUP_DE               4
    #define AUD_MICIN_GROUP_AIN2             5

#define REG_PADMUX1                         0x0058
#define REG_PADMUX2                         0x005C
#define REG_PADMUX3                         0x0060
    //[bit 8-9]define
    #define AUD_LINEIN_PINMUX_START         (8)
    #define AUD_LINEIN_PINMUX_NUM           (2)
    #define AUD_LINEIN_GROUP_AIN0            1
    #define AUD_LINEIN_GROUP_AIN2            2
    #define AUD_LINEIN_GROUP_SP1             3

#define REG_PADMUX4                         0x0064
#define REG_PADMUX5                         0x0068
#define REG_PADMUX6                         0x006C

#define REG_AUDHW_RST_I                     0x00A8
#define REG_AUDHW_RST_II                    0x00C4


//////////////////////////////////////////////////////////////
//      PLL GP Register
//////////////////////////////////////////////////////////////
#define ANA7_PLLGP_CFG0                     0x0280
#define ANA7_PLLGP_CFG1                     0x0284
#define ANA7_PLLGP_CFG2                     0x0288
#define ANA7_PLLGP_CFG3                     0x028C
#define ANA7_PLLGP_CFG4                     0x0290
#define ANA7_PLLGP_CFG5                     0x0294
#define ANA7_PLLGP_CFG6                     0x0298
#define ANA7_PLLGP_CFG7                     0x029C
#define ANA7_PLLGP_CFG8                     0x02A0
#define ANA7_PLLGP_CFG9                     0x02A4
#define ANA7_PLLGP_CFG10                    0x02A8
#define ANA7_PLLGP_CFG11                    0x02AC
#define ANA7_PLLGP_CFG12                    0x02B0
#define ANA7_PLLGP_CFG13                    0x02B4
#define ANA7_PLLGP_CFG14                    0x02B8
#define ANA7_PLLGP_CFG15                    0x02BC
    #define BIT_RG_APLL_RESERVE_MASK        (0x3F << 18)
    #define BIT_RG_APLL_PCW_NCPO_CHG        (1 << 26)
    #define BIT_RG_APLL_DDS_RSTB            (1 << 25)
    #define BIT_RG_APLL_DDS_PWDB            (1 << 24)
    #define BIT_RG_APLL_DDSEN               (1 << 17)
    #define BIT_RG_APLL_VODEN               (1 << 16)
    #define BIT_RG_APLL_AUTOK_LOAD          (1 << 13)
    #define BIT_RG_APLL_AUTOK_VCO           (1 << 12)
#define ANA7_PLLGP_CFG16                    0x02C0
#define ANA7_PLLGP_CFG17                    0x02C4
    #define BIT_RG_APLL_DDS_NCPO_EN         (1 << 19)
    #define BIT_RG_APLL_DDS_CLK_PH_INV      (1 << 17)
    #define BIT_RG_APLL_FIFO_START_MAN      (1 << 13)
#define ANA7_PLLGP_CFG18                    0x02C8
#define ANA7_PLLGP_CFG19                    0x02CC
    #define BIT_RG_PLL_RESERVE_MASK         (0xFF)
#define ANA7_PLLGP_CFG20                    0x02D0
#define ANA7_PLLGP_CFG21                    0x02D4
#define ANA7_PLLGP_CFG22                    0x02D8
#define ANA7_PLLGP_CFG23                    0x02DC
#define MONITOR_PLLGP_STATUS                0x02E4
#define ANA7_PLLGP_CFG25                    0x02E8
/* defined in other header file.
    #define BIT_AD_RGS_APLL270_VCOCAL_FAIL  (1 << 15)
    #define BIT_AD_RGS_APLL270_VCOCAL_CPLT  (1 << 14)
    #define BIT_AD_RGS_APLL294_VCOCAL_FAIL  (1 << 7)
    #define BIT_AD_RGS_APLL294_VCOCAL_CPLT  (1 << 6)
*/
#define ANA7_PLLGP_CFG26                    0x02EC
    #define BIT_RG_APLL_PWD                 (1 << 0)
#define ANA7_PLLGP_CFG27                    0x02D0

#define AUD_REG_RST_1                       0x00A8
#define AUD_REG_RST_2                       0x00C4

#define AUD_MPHONE_CLK_SEL_START            (6)
#define AUD_MPHONE_CLK_SEL_NUM              (2)
#define AUD_MPHONE_CLK_K6_SEL               (1)

#define AUD_REG_SEL_APLL2_K6_START          (17)
#define AUD_REG_SEL_APLL2_K6_NUM            (1)

#define AUD_REG_CFG_K6_START                (8)
#define AUD_REG_CFG_K6_NUM                  (8)


#endif // #ifndef _AUDIO_3360_REG_CKGEN_H_

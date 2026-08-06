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

#ifndef __AC823X_PINMUX_TABLE_H
#define __AC823X_PINMUX_TABLE_H


#include "ac823x_gpio_pinmux.h"
#include "x_types.h"
//#include <debug.h>


#define PINMUX_LEVEL_GPIO_END_FLAG 0xFF
#define PINMUX_LEVEL_INVALID_FLAG 0xFE
#define PINMUX_GROUP_INVALID_FLAG 0xFE
#define PINMUX_GPIO_LEVEL_MAX 12
#define PINMUX_GPIO_PARAM 2
#define GPIO_PINMUX_TOTAL_FUNC (PINMUX_GPIO_PARAM*PINMUX_GPIO_LEVEL_MAX)
#define INVALID_PUD_GPIO 0xFF
#define INVALID_PIN_FUNCTION 0xFF

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
//GPIO 47,68,69,174,175,195 need tread special
static const unsigned char _au1PinmuxFunctionSel[221][GPIO_PINMUX_TOTAL_FUNC] =
{
    //#define PIN_GPIO0           0  //DONE
    {   
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        SP0_SEL,                                   PINMUX_FUNCTION5,
        SP1_SEL,                                   PINMUX_FUNCTION5,        
        USB_OTG_SEL,                               PINMUX_FUNCTION1,
        PWM0_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO1           1 //Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        SP0_SEL,                                   PINMUX_FUNCTION5,
        SP1_SEL,                                   PINMUX_FUNCTION5,        
        USB_OTG_SEL,                               PINMUX_FUNCTION1,
        PWM1_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO2           2//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        SP0_SEL,                                   PINMUX_FUNCTION5,
        SP1_SEL,                                   PINMUX_FUNCTION5,        
        PWM1_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO3           3//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION4,
		I2S_LINE0_IN_SEL, 						   PINMUX_FUNCTION4,
        I2S_LINE1_IN_SEL, 						   PINMUX_FUNCTION4,
        USB_OTG_SEL, 						       PINMUX_FUNCTION2,
        LVDS_SEL,                                  PINMUX_FUNCTION1,
        SP0_SEL,                                   PINMUX_FUNCTION2,
        SP0_SEL,                                   PINMUX_FUNCTION5,
        SP1_SEL,                                   PINMUX_FUNCTION2,
        SP1_SEL,                                   PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO4           4  //doing
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION4,
		I2S_LINE0_IN_SEL, 						   PINMUX_FUNCTION4,
        I2S_LINE1_IN_SEL, 						   PINMUX_FUNCTION4,
        SGM_MIC_IN_SEL, 						   PINMUX_FUNCTION1,
        USB_OTG_SEL, 						       PINMUX_FUNCTION2,
        SP0_SEL,                                   PINMUX_FUNCTION2,
        SP1_SEL,                                   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG 
    },
    //#define PIN_GPIO5           5//doing
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION4,
		I2S_LINE0_IN_SEL, 						   PINMUX_FUNCTION4,
        I2S_LINE1_IN_SEL, 						   PINMUX_FUNCTION4,
        SGM_MIC_IN_SEL, 						   PINMUX_FUNCTION1,
        SP0_SEL,                                   PINMUX_FUNCTION2,
        SP1_SEL,                                   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO6           6//doing
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION4,
		I2S_LINE0_IN_SEL, 						   PINMUX_FUNCTION4,
        I2S_LINE1_IN_SEL, 						   PINMUX_FUNCTION4,
        SGM_MIC_IN_SEL, 						   PINMUX_FUNCTION1,
        SP0_SEL,                                   PINMUX_FUNCTION2,
        SP1_SEL,                                   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO7           7//doing
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION2,
		I2S_LINE1_IN_SEL, 						   PINMUX_FUNCTION2,
        I2S_LINE0_IN_SEL, 						   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN0_L              8//done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION2,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN0_R                  9//done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN1_L                 10//done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION2,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION2,
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN1_R                 11//done
    {
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN2_L                 12//done
    {
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN2_R                 13//done
    {
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN3_L                 14//done
    {
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_ANI3_R                 15//done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION3,
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN4_L                 16//done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AIN4_R                 17//done
    {
        AMUTE_F_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AL0                    18//DONE
    {
        AMUTE_R_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AL1                    19//Done
    {
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AL2                    20//Done
    {
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AL3                    21//Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION3,			
        I2S_OUT0_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AL4                    22//Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AMUTE_F                23//Done    
    {
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    }, 
    //#define PIN_AMUTE_GPS              24 //done    
    {
        //DVIN1_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_DCLK_IN_2                25 //Done
    {
        DVIN2_CLK_SEL,                             PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_DE                    26//Done
    {
        TTL_DE_SEL,                                PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_EINT0                    27//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION2,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION2,
        CA7_DFD_SEL,                               PINMUX_FUNCTION3,
        CA7_MBIST_SEL,							   PINMUX_FUNCTION1,
        //MP1_CA7_MBIST_SEL,						   PINMUX_FUNCTION1,
        EINT0_SEL,								   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_EINT1                 28//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION2,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION2,
        CA7_DFD_SEL,                               PINMUX_FUNCTION3,
        CA7_MBIST_SEL,							   PINMUX_FUNCTION1,
        //MP1_CA7_MBIST_SEL,						   PINMUX_FUNCTION1,
        EINT1_SEL,								   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_EINT2                 29 //Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION2,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION2,
        CA7_DFD_SEL,                               PINMUX_FUNCTION3,
        CA7_MBIST_SEL,							   PINMUX_FUNCTION1,
        //MP1_CA7_MBIST_SEL,						   PINMUX_FUNCTION1,
        EINT2_SEL,								   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO30                 30 //Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        CA7_DFD_SEL,                               PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO31                 31//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },

    //#define PIN_EINT3                    32//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION2,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION2,
        CA7_DFD_SEL,                               PINMUX_FUNCTION3,
        CA7_MBIST_SEL,							   PINMUX_FUNCTION1,
        //MP1_CA7_MBIST_SEL,						   PINMUX_FUNCTION1,
        EINT3_SEL,								   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HDMI_CEC_RX                    33//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION7,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION7,
        CA7_DFD_SEL,                               PINMUX_FUNCTION1,
        HDMI_CEC_SEL,							   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HDMI_HDP_RX                   34//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION7,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION7,
        CA7_DFD_SEL,                               PINMUX_FUNCTION1,
        //HDMI_HPD_SEL,							   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HDMI_SCL_RX                   35//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION7,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION7,
        CA7_DFD_SEL,                               PINMUX_FUNCTION1,
        //HDMI_I2C_SEL,                              PINMUX_FUNCTION1,
        USB3_SP_I2C_SEL,                           PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HDMI_SDA_RX                36//DONE
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION7,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION7,
        CA7_DFD_SEL,                               PINMUX_FUNCTION1,
        //HDIMI_I2C_SEL,                             PINMUX_FUNCTION1,
        USB3_SP_I2C_SEL,                           PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_DE                     37//Done
    {
        TTL_SYNC_SEL,                              PINMUX_FUNCTION1,
        TEST_BUS_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HSYNC_IN_1                  38    //Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION10,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //DV1_TIMING_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_HSYNC_IN_2              39 //Done 
    {
        //DV2_TIMING_SEL,                            PINMUX_FUNCTION1,
        PWM3_SEL,                                  PINMUX_FUNCTION2,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_IN1_BCK              40//going
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION1,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION1,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION1,
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_OUT1_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_IN1_D              41//Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION1,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION1,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION1,
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_OUT1_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO42                 42//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        EINT7_SEL,                                 PINMUX_FUNCTION3,
        PWM2_SEL,                                  PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO43                 43//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG, 
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO44                 44//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        CA7_DFD_SEL,                               PINMUX_FUNCTION3,
        CA7_MBIST_SEL,                             PINMUX_FUNCTION1,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION1,
        LVDS_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_IN1_LRCK              45 //Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION1,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION1,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION1,
        I2S_OUT1_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
       //#define PIN_I2S_IN1_MCLK        46//Done    
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION1,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION1,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION1,
        SGM_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_OUT1_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_BCK            47 //Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION2,
        I2S_OUT0_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_D0             48//Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION2,
        I2S_OUT0_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_D1            49//Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_D2            50//Done
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_LRCK         51//Done      
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION2,
        I2S_OUT0_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_I2S_OUT0_MCLK               52 //Done       
    {
        I2S_OUT1_SEL,                              PINMUX_FUNCTION2,
        I2S_OUT0_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO53                 53//done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION1,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION1,
        CA7_DFD_SEL,                               PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO54                 54//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION1,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION1,
        CA7_DFD_SEL,                               PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO55                 55//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION1,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION1,
        CA7_DFD_SEL,                               PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO56                 56//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION1,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION1,
        CA7_DFD_SEL,                               PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO57                 57//Done
    {
        PINMUX_LEVEL_INVALID_FLAG,                 PINMUX_GROUP_INVALID_FLAG,
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION1,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION1,
        CA7_DFD_SEL,                               PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE0N                     58 //dONE     
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION6,
		I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION6,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION6,        
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE0P             59//Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION6,
		I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION6,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION6,      
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE1N             60 //Done 
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION6,
		I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION6,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION6,    
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE1P           61//Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION6,
		I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION6,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION6,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE2N           62 //Done
    {
        //AP_RS232_SEL,							   PINMUX_FUNCTION9,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE2P                        63//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION9,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE3N           64 //Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION6,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION6,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AE3P             65//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION6,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION6,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AECKN            66//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION6,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION6,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AECKP            67//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION5,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO0N            68//Done
    {
        EINT7_SEL,                    			   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO0P           69//Done
    {
        EINT6_SEL,                    			   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO70                 70//dONE
    {
        CA7_MBIST_SEL,                             PINMUX_FUNCTION2,
        EINT4_SEL,                                 PINMUX_FUNCTION3,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION2,        
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO71                 71//DONE
    {
        CA7_MBIST_SEL,                             PINMUX_FUNCTION2,
        EINT5_SEL,                                 PINMUX_FUNCTION3,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION2,  
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO72                 72//Done
    {
        EINT6_SEL,                                 PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO1N            73//Done
    {
        EINT5_SEL,                                 PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO74                 74//DONE
    {
        RTC_OUT_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO1P             75//Done
    {
        EINT4_SEL,                            	   PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO2N             76//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION8,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO2P            77//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION8,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO3N            78//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION5,
	    ARM9_JTAG_SEL,                             PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_LVDS_AO3P            79//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION5,
	    ARM9_JTAG_SEL,                             PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define  PIN_LVDS_AOCKN         80//DONE
    {
    	ARM11_JTAG_SEL,                            PINMUX_FUNCTION5,
	    ARM9_JTAG_SEL,                             PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_AOCKP                  81//Done
    {
    	ARM11_JTAG_SEL,                            PINMUX_FUNCTION5,
	    ARM9_JTAG_SEL,                             PINMUX_FUNCTION5,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_MHL_PWR_EN                82//Done  
    {
        CA7_MBIST_SEL,                			   PINMUX_FUNCTION5,
        //MP1_CA7_MBIST_SEL,                		   PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_MHL_SENSE                 83 //Done
    {
    	CA7_MBIST_SEL,                			   PINMUX_FUNCTION3,
		MHL_SENSE_SEL,							   PINMUX_FUNCTION1,
		//MP1_CA7_MBIST_SEL,                		   PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD1_D0                  84//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD1_D1                  85//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD2_D2                 86//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD1_D3                  87//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD2_D0                  88//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD2_D1                   89 //Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD2_D2                   90//Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD2_D3                  91 //Done
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //----------------------------------------------------------------------
    //#define PIN_NFALE                   92//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,                 
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NFCEN0                   93//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NFCEN1                   94 //Going
    {
        //NAND_FLASH_2ND_SEL,                      PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NCFLE                   95 //Done 
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NFRBN                   96//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    
    //#define PIN_NFRBN2                97//Going
    {
        // NAND_FLASH_2ND_SEL,                     PINMUX_FUNCTION1,//not find
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NFREN                 98//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NFWEN                99//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD0                 100//Done    
    {
        SP0_SEL,                                   PINMUX_FUNCTION3,
        SP1_SEL,                                   PINMUX_FUNCTION3,
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
   
    //#define PIN_NLD1                  101//Done
    {
        SP0_SEL,                                   PINMUX_FUNCTION3,
        SP1_SEL,                                   PINMUX_FUNCTION3,
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD2                  102//Done
    {
        SP0_SEL,                                   PINMUX_FUNCTION3,
        SP1_SEL,                                   PINMUX_FUNCTION3,
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD3                  103//Done
    {
        SP0_SEL,                                   PINMUX_FUNCTION3,
        SP1_SEL,                                   PINMUX_FUNCTION3,
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },

    //#define PIN_NLD4                  104//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD5               105//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD6                106//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_NLD7                107//Done
    {
        NAND_FLASH_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_PCM_CLK                108//Done
    {
        PCM_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_PCM_IN              109//Done
    {
        PCM_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },   
    //#define PIN_PCM_OUT                110//Done
    {
        PCM_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },   
    //#define PIN_PCM_SYNC                111 //Done
    {
        PCM_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },   
    //#define PIN_SCL0                112//Done
    {
        USB_I2C_SEL,                               PINMUX_FUNCTION1,
		USB3_I2C_SEL,                              PINMUX_FUNCTION2,
		USB3_SP_I2C_SEL,                           PINMUX_FUNCTION2,
        I2C0_SEL,                                  PINMUX_FUNCTION1,
        I2C1_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },   
    //#define PIN_SCL1                 113//Done
    {
        USB3_I2C_SEL,                              PINMUX_FUNCTION1,
        USB_I2C_SEL,                               PINMUX_FUNCTION2,
        //HDMI_I2C_SEL,                              PINMUX_FUNCTION2,
        //VGA_I2C_SEL,                               PINMUX_FUNCTION2,
        I2C0_SEL,                                  PINMUX_FUNCTION2,
        I2C1_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD_V33_18_SW0              114//Done
    {
        CA7_MBIST_SEL,                             PINMUX_FUNCTION2,
        CA7_MBIST_SEL,                             PINMUX_FUNCTION3,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION2,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD_V33_18_SW1                 115//Done
    {
        CA7_MBIST_SEL,                             PINMUX_FUNCTION2,
        CA7_MBIST_SEL,                             PINMUX_FUNCTION3,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION2,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SD_V33_18_SW2                 116//Done
    {
        CA7_MBIST_SEL,                             PINMUX_FUNCTION2,
        CA7_MBIST_SEL,                             PINMUX_FUNCTION3,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION2,
        //MP1_CA7_MBIST_SEL,                         PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SDA0                 117//Done
    {
        USB_I2C_SEL,                               PINMUX_FUNCTION1,
		USB3_I2C_SEL,                              PINMUX_FUNCTION2,
        USB3_SP_I2C_SEL,                           PINMUX_FUNCTION2,
        I2C0_SEL,                                  PINMUX_FUNCTION1,
        I2C1_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SDA1                 118//Done
    {
        USB3_I2C_SEL,                              PINMUX_FUNCTION1,
	    USB_I2C_SEL,                               PINMUX_FUNCTION2,
        //HDMI_I2C_SEL,                              PINMUX_FUNCTION2,
        //VGA_I2C_SEL,                               PINMUX_FUNCTION2,
        I2C0_SEL,                                  PINMUX_FUNCTION2,
        I2C1_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP0_CLK                 119//Done
    
    {
        SP0_SEL,                                   PINMUX_FUNCTION1,
        SP1_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP0_CS                 120//Done
    {
        SP0_SEL,                                   PINMUX_FUNCTION1,
        SP1_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP0_SI                 121//done
    {
        SP0_SEL,                                   PINMUX_FUNCTION1,
        SP1_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP0_SO                 122//DONE
    {
        SP0_SEL,                                   PINMUX_FUNCTION1,
        SP1_SEL,                                   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP1_CLK                123//Done
    {
        AP_SF_SEL,                                 PINMUX_FUNCTION1,
		I2S_MIC_IN_SEL,                            PINMUX_FUNCTION7,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION7,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION7,
        SP0_SEL,                                   PINMUX_FUNCTION4,
        SP1_SEL,                                   PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO124               124//Done
    {
        UART3_SEL,                                 PINMUX_FUNCTION2,
        PWM2_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO_125              125//Done
    {
        UART3_SEL,                                 PINMUX_FUNCTION2,
        PWM3_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP1_CS               126//Done
    {
        AP_SF_SEL,                                 PINMUX_FUNCTION1,
		I2S_MIC_IN_SEL,                            PINMUX_FUNCTION7,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION7,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION7,
        SP0_SEL,                                   PINMUX_FUNCTION4,
        SP1_SEL,                                   PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SP1_SI                 127//Done
    {
        AP_SF_SEL,                                 PINMUX_FUNCTION1,
		I2S_MIC_IN_SEL,                            PINMUX_FUNCTION7,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION7,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION7,
        SP0_SEL,                                   PINMUX_FUNCTION4,
        SP1_SEL,                                   PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },

    //#define PIN_SP1_SO                 128//Done
    {
        AP_SF_SEL,                                 PINMUX_FUNCTION1,
		I2S_MIC_IN_SEL,                            PINMUX_FUNCTION7,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION7,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION7,
        SP0_SEL,                                   PINMUX_FUNCTION4,
        SP1_SEL,                                   PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_SPDIF                 129//Done
    {
        SPDIF_SEL,                                 PINMUX_FUNCTION1,
        HDMI_SPDIF_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_URAT0_CTS                 130//Done
    { 
        UART0_FLWCTRL_SEL,                         PINMUX_FUNCTION1,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION3,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_URAT0_RTS                 131//Done
    {
        UART0_FLWCTRL_SEL,                         PINMUX_FUNCTION1,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION3,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_URAT1_CTS                132//Done
    {
		UART1_FLWCTRL_SEL,                         PINMUX_FUNCTION1,
		I2S_MIC_IN_SEL,                            PINMUX_FUNCTION3,
		I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION3,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_URAT1_RTS             133 //Done   
    {
        UART1_FLWCTRL_SEL,                         PINMUX_FUNCTION1,
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION3,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION3,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },         
    //#define PIN_URXD0             134 //Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION1,
        UART0_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },         
    //#define PIN_URXD1             135 //Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION2,
        UART1_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },         
    //#define PIN_URXD2             136 //Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION3,
        UART2_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },         
    //#define PIN_URXD3                 137//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION4,
        UART3_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_URXD4                 138//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION5,
        UART4_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_USB_DM_P0                 139//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION4,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_USB_DM_P1                 140//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION4,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_USB_DP_P0                 141//done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION4,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_USB_DP_P1                142//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION4,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION4,        
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_UTXD0              143//done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION1,
        UART0_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_UTXD1                  144//done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION2,
        UART1_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_UTXD2                   145//done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION3,
        UART2_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_UTXD3                   146 //done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION4,
        UART3_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_UTXD4                   147//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION5,
        UART4_SEL,                                 PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB0                   148//Done
    {
        PWM0_SEL,                            	   PINMUX_FUNCTION3,
        TTL_8B_SEL,                                PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB1               149//Done
    {
        PWM1_SEL,                           	   PINMUX_FUNCTION3,
		TTL_8B_SEL,                                PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO150               150//done
    {
        PWM1_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB2               151//Done
    {
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    
    //#define PIN_VB3                   152//Done
    {
        PWM3_SEL,                            	   PINMUX_FUNCTION4,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION4,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VD4                  153//Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION5,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION5,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION5,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB5                   154//done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION5,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION5,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION5,
        TTL_6_8B_SEL,               			   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB6                   155//DONE
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION5,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION5,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION5,
        TTL_6_8B_SEL,               			   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VB7                   156//Done
    {
        I2S_MIC_IN_SEL,                            PINMUX_FUNCTION5,
        I2S_LINE1_IN_SEL,                          PINMUX_FUNCTION5,
        I2S_LINE0_IN_SEL,                          PINMUX_FUNCTION5,
        TTL_6_8B_SEL,               			   PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG0                   157//done
    {
        EINT4_SEL,                             	   PINMUX_FUNCTION4,
        TTL_8B_SEL,                                PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG1                   158 //done
    {
        EINT5_SEL,                                 PINMUX_FUNCTION1,
        TTL_8B_SEL,                                PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG2                   159//Done
    {   
        EINT6_SEL,                                 PINMUX_FUNCTION1,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG3                   160//Done
    {
        EINT6_SEL,                                 PINMUX_FUNCTION1,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG4                   161//Done
    {
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        TEST_BUS_SEL,                              PINMUX_FUNCTION1,
        TEST_OUT_SEL,                              PINMUX_FUNCTION1,
        LVDS_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO162               162//Done
    {
        PWM0_SEL,                                  PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG5                 163//Done
    {
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        TEST_BUS_SEL,                              PINMUX_FUNCTION1,
        TEST_OUT_SEL,                              PINMUX_FUNCTION1,
        LVDS_SEL,                                  PINMUX_FUNCTION2,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VG6                  164//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION8,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        TEST_BUS_SEL,                              PINMUX_FUNCTION1,
        TEST_OUT_SEL,                              PINMUX_FUNCTION1,
        LVDS_SEL,                                  PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },          
    //#define PIN_VG7                  165  //Done 
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION8,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        TEST_BUS_SEL,                              PINMUX_FUNCTION1,
        TEST_OUT_SEL,                              PINMUX_FUNCTION1,
        LVDS_SEL,                                  PINMUX_FUNCTION3,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },          
    //#define PIN_VGA_HSYNC0                  166//Done
    {
        //VGA_HSYNC0_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VGA_SCL                  167//Done
    {
        //VGA_I2C_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VGA_SDA                  168//Done
    {
        //VGA_I2C_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VGA_VSYNC0                 169//Done
    {
        //VGA_VSYNC0_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN0                 170//Done
    {
        UART0_SEL,                                 PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN1                   171//Done
    {
        UART0_SEL,                                 PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN2                   172//Done
    {
        UART1_SEL,                                 PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1, 
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN3                   173//Done
    {
        UART1_SEL,                                 PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,  
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN4                  174//DONE
    {
        UART2_SEL,                    			   PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,  
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN5                   175//DONE
    {
        UART2_SEL,                    			   PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN6                   176//Done
    {
        UART4_SEL,                    			   PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VIN7                   177//Done
    {
        UART4_SEL,                    			   PINMUX_FUNCTION2,
        //CCIR656_601_DATAIN_SEL,                    PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        //TEST_ABIST_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR0                   178//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION7,
        TTL_8B_SEL,                                PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR1                 179//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION7,
        TTL_8B_SEL,                                PINMUX_FUNCTION1,
        TEST_IN_SEL,                               PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR2              180//Done
    {
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },

    //#define PIN_VR3         181//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION3,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION3,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR4         182 //Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION3,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION3,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR5         183//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION3,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION3,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,       
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR6         184//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION3,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION3,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,        
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VR7      185//Done
    {
        ARM11_JTAG_SEL,                            PINMUX_FUNCTION3,
        ARM9_JTAG_SEL,                             PINMUX_FUNCTION3,
        TTL_6_8B_SEL,                              PINMUX_FUNCTION1,  
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VSYNC      186//Done
    {
        TTL_SYNC_SEL,                              PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VSYNC_IN_1     187//Done
    {
        //AP_RS232_SEL,                              PINMUX_FUNCTION10,
        //UART5_SEL,                                 PINMUX_FUNCTION2,
        //DVIN1_TIMING_SEL,                          PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_VSYNC_IN_2      188//Done
    {
        DVIN2_TIMING_SEL,                          PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#define    PIN_YIN0                             189//Done
    {   
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define     PIN_YIN1                       190//Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define   PIN_YIN2                         191 //Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define   PIN_YIN3                        192 //Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#define PIN_YIN4                       193//Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_YIN5                    194//Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_YIN6                   195//Done
    {
        DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_YIN7                   196//
    {
    	DVIN2_DATA_SEL,                            PINMUX_FUNCTION1,
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#define PIN_GPIO7          197
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_ADDC_L0               198// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_ADDC_R0              199// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_ADDC_L1             200 // not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_ADDC_R1                    201// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_EMMC_RCLK             202// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_EMMC_RST                 203// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
	//#PIN_SD1_CLK                   204// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD1_CMD                 205// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD2_CLK                 206// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD2_CMD                 207// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_                 208// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_                 209// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_                 210// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_CLK                211// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_CMD                 212// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_D0                213// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_D1                 214// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_D2                 215// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_SD3_D3                216// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_OPWRSB                 217// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_IR                 218// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_GPIO8                 219// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
    //#PIN_GPIO9                 220// not GPIO
    {
        PINMUX_LEVEL_GPIO_END_FLAG,                PINMUX_GROUP_INVALID_FLAG
    },
       
};


static const unsigned char _au1PinmuxFunctionMasks[256] = 
{
//--------------0x54--------------------
  7,  0,  0,  3,  0,  1,  1,  0,  1,  0, 
  0,  0,  0,  0,  0,  0,  3,  0,  3,  0,
  3,  0,  3,  0,  1,  3,  0,  0,  7,  0,
  0,  0,  
//--------------0x58--------------------
  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,
  0,  0,  1,  3,  0,  7,  0,  0,  3,  0,
  0,  0,
//--------------0x5c--------------------
  3,  0,  1,  1,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  0,  0,  7,  0,  0,  7,
  0,  0,  1,  1,  0,  0,  0,  0,  0,  0,
  1,  1,
//--------------0x60--------------------
  1,  0,  0,  0,  0,  0,  0,  0,  3,  0,
  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,
//--------------0x64--------------------
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  0,  0,  0,  0,  0,  1,  1,
  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,
  0,  0,
//--------------0x68--------------------
  1,  0,  1,  0,  1,  0,  1,  0,  3,  0,
  0,  0,  3,  0,  0,  0,  3,  0,  0,  3,
  0,  0,  3,  0,  0,  3,  0,  0,  0,  0,
  0,  0,
//--------------0x6c--------------------
  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,
  0,  3,  0,  3,  0,  0,  3,  0,  3,  0,
  3,  0,  0,  0,  0,  0,  3,  0,  0,  0,
  0,  0,
//--------------0x70--------------------
  3,  0,  0,  1,  1,  1,  3,  0,  0,  0,
  0,  1,  3,  0,  3,  0,  7,  0,  0,  0,
  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0
};

//137 pad_port not find
static const unsigned char _au1PinPullUDOffset[205] =
{
   //0~31 
    0, 1, 2, 3, 4, 5, 6, 7, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,    
    255, 255, 123, 124, 255, 255, 255, 255, 63, 109,
    8, 9,
   //32~63
    62, 108, 29, 66, 67, 68, 69, 112, 115, 113,   //41 
    10, 11, 12, 114, 110, 111, 119, 121, 120, 118, //51
    125, 13, 14, 15, 16, 17, 126, 127, 128, 129,   //61
    130, 255,
   //64~95
    255, 255, 255, 255, 255, 255, 18, 19, 20, 255,  //73
    21, 255, 255, 255, 117, 116, 255, 255, 255, 255,//83
    255, 255, 255, 255, 255, 255, 255, 255, 86, 84, //93
    85, 255, 
   //96~127
    83, 255, 82, 87, 255, 255, 255, 255, 255, 255,  //105
    255, 255, 72, 71, 73, 70, 133, 135, 255, 255,   //115
    255, 134, 136, 74, 77, 75, 76, 78, 22, 23,      //125
    81, 79,
   //128~159
    80, 122, 26, 30, 31, 32, 33, 34, 35, 36,        //137
    37, 27,  28, 88, 89, 90, 91, 92, 93, 255,//147
    255, 255, 24, 255, 255, 94, 95, 96, 97, 98,//157
    99, 54,
   //160~191
    55, 56, 25, 57, 58, 59, 60, 61, 46, 47,//169
    48, 49, 50, 51, 52, 53, 132, 131, 100, 101,//179
    102, 103, 104, 105, 106, 107, 38, 39, 40, 41,//189
    42, 43,
   //192~204
    44, 45, 65, 64, 255, 255, 255, 255, 255, 255,
    255, 255, 255
};


static const unsigned char _au1IntSel[205][3] =
{
    //#define PIN_GPIO0           0  //DONE
    {0,0,24},{1,0,25},{2,0,30},{3,0,31},{4,0,34},{5,0,38},{6,0,44},{7,0,0},{8,0,1},{9,0,2},
    {10,0,3},{11,0,4},{12,0,5},{13,0,6},{14,0,7},{15,0,8},{16,0,9},{17,0,10},{18,0,11},{19,0,12},
    {20,0,13},{21,0,14},{22,0,15},{23,0,16},{24,1,17},{25,1,18},{26,0,19},{27,0,20},{28,0,21},{29,0,22},
    {30,0,32},{31,0,33},{32,0,23},{33,0,49},{34,0,50},{35,0,51},{36,0,52},{37,0,53},{38,0,54},{39,0,55},
    {40,0,56},{41,0,57},{42,0,35},{43,0,36},{44,0,37},{45,0,58},{46,0,59},{47,0,60},{48,0,61},{49,0,62},
    {50,0,63},{51,0,64},{52,0,65},{53,0,39},{54,0,40},{55,0,41},{56,0,42},{57,0,43},{58,0,66},{59,0,67},
    {60,0,68},{61,0,66},{62,0,69},{63,0,66},{64,0,70},{65,0,71},{66,0,53},{67,0,54},{68,0,76},{69,0,77},
    {70,0,45},{71,0,46},{72,0,47},{73,0,78},{74,0,48},{75,0,79},{76,0,80},{77,0,81},{78,0,82},{79,0,83},
    {80,1,28},{81,1,29},{82,0,86},{83,0,87},{84,0,88},{85,0,89},{86,0,90},{87,0,91},{88,0,92},{89,0,93},
    {90,0,94},{91,0,95},{92,1,0},{93,1,1},{94,1,2},{95,1,3},{96,1,4},{97,1,5},{98,1,6},{99,1,7},
    {100,1,8},{101,1,9},{102,1,10},{103,1,11},{104,1,12},{105,1,13},{106,1,14},{107,1,15},{108,1,16},{109,1,17},
    {110,1,18},{111,1,19},{112,1,20},{113,1,21},{114,1,22},{115,1,23},{116,1,24},{117,1,25},{118,1,26},{119,1,27},
    {120,1,28},{121,1,29},{122,1,30},{123,1,31},{124,0,26},{125,0,27},{126,1,32},{127,1,33},{128,1,34},{129,1,35},
    {130,1,36},{131,1,37},{132,1,38},{133,1,39},{134,1,40},{135,1,41},{136,1,42},{137,1,43},{138,1,44},{139,1,45},
    {140,1,46},{141,1,47},{142,1,48},{143,1,49},{144,1,50},{145,1,51},{146,1,52},{147,1,53},{148,1,54},{149,1,55},
    {150,0,28},{151,1,56},{152,1,57},{153,1,58},{154,1,59},{155,1,60},{156,1,61},{157,1,62},{158,1,63},{159,1,64},
    {160,1,65},{161,1,66},{162,0,29},{163,1,67},{164,1,68},{165,1,69},{166,1,73},{167,1,71},{168,1,72},{169,1,70},
    {170,1,74},{171,1,75},{172,1,76},{173,1,77},{174,1,78},{175,1,79},{176,1,80},{177,1,81},{178,1,82},{179,1,83},
    {180,1,84},{181,1,85},{182,1,86},{183,1,87},{184,1,88},{185,1,89},{186,1,90},{187,1,91},{188,0,92},{189,1,93},
    {190,1,94},{191,1,95},{192,2,0},{193,2,1},{194,2,2},{195,2,3},{196,2,4},{197,2,12},{198,2,6},{199,2,7},
    {200,2,8},{201,2,9},{202,1,46},{203,2,10},{204,2,11}
   

};




//137 pad_port not find
static const unsigned char PIN_PULL_UP_OR_DOWN_OFFSET[205] =
{
   //0~31 
    0, 1, 2, 3, 4, 5, 6, 255, 255, 255,     //9
    255, 255, 255, 255, 255, 255, 255, 125, 124, 255,   //19   
    255, 255, 123, 50, 107, 110, 49, 53, 54, 55,
    7, 8,
   //32~63
    56, 113, 116, 114, 115, 52, 109, 112, 120, 122,     //41 
    9, 10, 11, 121, 119, 126, 127, 128, 129, 130,  //51
    131, 12, 13, 14, 15, 16, 126, 127, 128, 129,    //61
    130, 255,
   //64~95
    255, 255, 255, 255, 255, 255, 17, 18, 19, 255,  //73
    20, 255, 255, 255, 117, 116, 255, 255, 118, 117,//83
    255, 255, 255, 255, 255, 255, 255, 255, 74, 72, //93
    73, 255, 
   //96~127
    70, 71, 69, 75, 255, 255, 255, 255, 255, 255,  //105
    255, 255, 57, 59, 60, 58, 134, 135, 138, 139,   //115
    140, 135, 137, 61, 62, 63, 64, 65, 21, 22,      //125
    68, 66,
   //128~159
    67, 123, 76, 77, 78, 79, 80, 81, 82, 83,        //137
    84, 27,  28, 88, 89, 90, 136, 86, 87, 88,       //147
    89, 42, 23, 43, 44, 45, 46, 47, 48, 33,     //157
    34, 35,
   //160~191
    36, 37, 24, 38, 39, 40, 142, 131, 132, 141,         //169
    98, 99, 100, 101, 102, 103, 104, 105, 25, 26,     //179
    27, 28, 29, 30, 31, 32, 51, 39, 111, 90,   //189
    91, 92,
   //192~204
    93, 94, 95, 96, 97, 255, 255, 255, 255, 255,   //201
    255, 255, 255
};

#endif /* __AC823X_PINMUX_TABLE_H */


/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef X_CKGEN_3360_H
#define X_CKGEN_3360_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"



#define REG_RW_ANA7_PLLGP_CFG1      0x284
#define REG_RW_ANA7_PLLGP_CFG7      0x29C
#define REG_RW_ANA7_PLLGP_CFG8      0x2A0

#define REG_RW_ANA7_PLLGP_CFG9      0x2A4
#define REG_RW_ANA7_PLLGP_CFG14     0x2B8
#define REG_RW_ANA7_PLLGP_CFG25     0x2E8
  #define BIT_RG_APLL_PWD           (1 << 0)
  
#define REG_RW_ANA7_PLLGP_CFG10     0x2A8
#define REG_RW_ANA7_PLLGP_CFG15     0x2BC
  #define BIT_RG_APLL_RESERVE_MASK (0x3F << 18)
  #define BIT_RG_APLL_PCW_NCPO_CHG (1 << 26)
  #define BIT_RG_APLL_DDS_RSTB     (1 << 25)
  #define BIT_RG_APLL_DDS_PWDB     (1 << 24)
  #define BIT_RG_APLL_DDSEN        (1 << 17)
  #define BIT_RG_APLL_VODEN        (1 << 16)
  #define BIT_RG_APLL_AUTOK_LOAD   (1 << 13)
  #define BIT_RG_APLL_AUTOK_VCO    (1 << 12)
  
#define REG_RW_ANA7_PLLGP_CFG11     0x2AC
#define REG_RW_ANA7_PLLGP_CFG16     0x2C0
  
#define REG_RW_ANA7_PLLGP_CFG12     0x2B0
#define REG_RW_ANA7_PLLGP_CFG17     0x2C4
  #define BIT_RG_APLL_BR(v)         (v << 24)
  #define BIT_RG_APLL_BR_MASK         (0x7 << 24)
  #define BIT_RG_APLL_DDS_NCPO_EN     (1 << 19)
  #define BIT_RG_APLL_DDS_CLK_PH_INV  (1 << 17)
  #define BIT_RG_APLL_FIFO_START_MAN  (1 << 13)


  
#define REG_RW_ANA7_PLLGP_CFG13     0x2B4
#define REG_RW_ANA7_PLLGP_CFG18     0x2C8

#define REG_RW_ANA7_PLLGP_CFG19     0x2CC
  #define BIT_RG_PLL_RESERVE_MASK   (0xFF)

#define REG_RO_MONITOR_PLLGP_STATUS 0x2E4
  #define BIT_AD_RGS_APLL270_VCOCAL_FAIL   (1 << 15)
  #define BIT_AD_RGS_APLL270_VCOCAL_CPLT   (1 << 14)
  #define BIT_AD_RGS_APLL294_VCOCAL_FAIL   (1 << 7)
  #define BIT_AD_RGS_APLL294_VCOCAL_CPLT   (1 << 6)


#define REG_RW_MEMPLL0      (0x5AA90)
#define REG_RW_MEMPLL1      (0x5aa94)

//for USBPLL
#define REG_RW_USBPLLCTRL   (0xF800)
#define REG_RW_USBPHY       (0xF860)

#define REG_RW_BIM_MISC2    (0x80AC)
  
//============================================================================
// Register definitions
//============================================================================
#define REG_RW_VERSION                  0x0000              // 0x8550
#define REG_RW_ARMPLL_CFG0              0x0004              //ARMPLL Configuration 0 Register
  #define ARMPLL_ARMPLL_VCOCALSEL_MASK           0x60000000
  #define ARMPLL_ARMPLL_VCOCALSEL_OFFSET         29         //VCOCAL period selection    00:256 reference cycle  01:512 cycle  10:1024 cycle  11:2048 cycle
  #define ARMPLL_ARMPLL_RELATCH_EN               (1U << 28) //Feedback divider relatch function   0:disable  1:enable
  #define ARMPLL_ARMPLL_VCOCAL_EN                (1U << 27) //VCOBAND calibration enable  0:VCOBAND_OUT[3:0] = VCOBAND[3:0]  1:VCOCAL enable
  #define ARMPLL_ARMPLL_POSTDIV_MASK             0x07000000
  #define ARMPLL_ARMPLL_POSTDIV_OFFSET           24         //000:/1  001:/2  010:/4  011:/8  100:/16  101:/32
  #define ARMPLL_ARMPLL_VCOBAND_MASK             0x00F00000
  #define ARMPLL_ARMPLL_VCOBAND_OFFSET           20         //VCOBAND selection (manul mode)
  #define ARMPLL_ARMPLL_CP_P_MASK                0x000E0000
  #define ARMPLL_ARMPLL_CP_P_OFFSET              17         //PLL BW Control
  #define ARMPLL_ARMPLL_DIV1                     (1U << 16) //Feedback divider  0:/RG_ARMPLL_DIV[7:0]+2)  1:/1
  #define ARMPLL_ARMPLL_DIV_MASK                 0x0000FE00
  #define ARMPLL_ARMPLL_DIV_OFFSET               9          // /(RG_ARMPLL_DIV[7:0]+2)
  #define ARMPLL_ARMPLL_CMDIV1                   (1U << 8)  //Capacitor Multiplication Number = 1
  #define ARMPLL_ARMPLL_CM_MASK                  0x000000FC
  #define ARMPLL_ARMPLL_CM_OFFSET                2          //Capacitor Multiplication Number = RG_ARMPLL_CM[7:0]+2)
  #define ARMPLL_ARMPLL_PREDIV_MASK              0x00000003
  #define ARMPLL_ARMPLL_PREDIV_OFFSET            0          //00:/1  01:/2  10:/4  11:/8
  //#define ARMPLL_ARMPLL_CKSEL_MASK               0x00000003
  //#define ARMPLL_ARMPLL_CKSEL_OFFSET             0          //00:REFCLK 27M  01:REFCLK_DUM[0]  10:REFCLK_DUM[1]  11:REFCLK_DUM[2]
#define REG_RW_ARMPLL_CFG1              0x0008              //ARMPLL Configuration 1 Register
  #define ARMPLL_ARMPLL_PWD                      (1U << 11) //PLL power down   0:normal  1:power down
  #define ARMPLL_ARMPLL_RSTB                     (1U << 10) //PLL reset
  #define ARMPLL_ARMPLL_VCOVTSEL_MASK            0x00000300
  #define ARMPLL_ARMPLL_VCOVTSEL_OFFSET          8          //VCOCAL slicer voltage    00:0.72V  01:0.6V  10:0.48V
  #define ARMPLL_ARMPLL_REV_MASK                 0x000000FF
  #define ARMPLL_ARMPLL_REV_OFFSET               0          //Reserved r/g
#define REG_RW_DMPLL_CFG0               0x000C              //DMPLL Configuration 0 Register
  #define DMPLL_DMPLL_RELATCH_EN                 (1U << 31) //Feedback divider relatch function    0:disable  1:enable
  #define DMPLL_DMPLL_VCOCAL_EN                  (1U << 30) //VCOBAND calibration enable    0:VCOBAND_OUT[3:0] = VCOBAND[3:0]  1:VCOCAL enable
  #define DMPLL_DMPLL_POSTDIV_MASK               0x38000000
  #define DMPLL_DMPLL_POSTDIV_OFFSET             27         //000:/1  001:/2  010:/4  011:/8  100:/16  101:/32
  #define DMPLL_DMPLL_CP_P_MASK                  0x07000000
  #define DMPLL_DMPLL_CP_P_OFFSET                24         //PLL BW Control
  #define DMPLL_DMPLL_CMDIV1                     (1U << 22) //Capacitor Multiplication Number = 1
  #define DMPLL_DMPLL_CM_MASK                    0x003F0000
  #define DMPLL_DMPLL_CM_OFFSET                  16         //Capacitor Multiplication Number =DMPLL_CM[7:0]+2)
  #define DMPLL_DMPLL_DIV1                       (1U << 14)  //Feedback divider  0:/(DMPLL_DIV[7:0]+2)  1:/1
  #define DMPLL_DMPLL_DIV_MASK                   0x00003F00
  #define DMPLL_DMPLL_DIV_OFFSET                 8          // /(DMPLL_DIV[7:0]+2)
  #define DMPLL_DMPLL_VCOBAND_MASK               0x000000F0
  #define DMPLL_DMPLL_VCOBAND_OFFSET             4          //VCOBAND selection (manul mode)
  #define DMPLL_DMPLL_PREDIV_MASK                0x0000000C
  #define DMPLL_DMPLL_PREDIV_OFFSET              2          //00:/1  01:/2  10:/4  11:/8
  #define DMPLL_DMPLL_CKSEL_MASK                 0x00000003
  #define DMPLL_DMPLL_CKSEL_OFFSET               0          //00:REFCLK 27M  01:REFCLK_DUM[0]  10:REFCLK_DUM[1]  11:REFCLK_DUM[2]
#define REG_RW_DMPLL_CFG1               0x0010              //DMPLL Configuration 1 Register
  #define DMPLL_NCO_DMSS_TEST_MASK               0x001F0000
  #define DMPLL_NCO_DMSS_TEST_OFFSET             16         //DMSS control word fractional part
  #define DMPLL_NCO_DMPLL_DIR_MASK               0x00006000
  #define DMPLL_NCO_DMPLL_DIR_OFFSET             13         //DMSS spreading type    0:center  1:down  2:up
  #define DMPLL_NCO_DMPLL_CW_MASK                0x00001F00
  #define DMPLL_NCO_DMPLL_CW_OFFSET              8          //DMSS nco frequency control
  #define DMPLL_CNCO_DMPLL_RSTB                  (1U << 7)  //DMSS nco resetb control
  #define DMPLL_NCO_DMPLL_INIT                   (1U << 6)  //DMSS nco initial control
  #define DMPLL_DMPLL_PWD                        (1U << 5)  //PLL power down    0:normal  1:power down
  #define DMPLL_DMPLL_RSTB                       (1U << 4)  //PLL reset
  #define DMPLL_DMPLL_VCOVTSEL_MASK              0x0000000C
  #define DMPLL_DMPLL_VCOVTSEL_OFFSET            2          //VCOCAL slicer voltage    00: 0.72V  01: 0.6V  10: 0.48V
  #define DMPLL_DMPLL_VCOCALSEL_MASK             0x00000003
  #define DMPLL_DMPLL_VCOCALSEL_OFFSET           0          //VCOCAL period selection    00:256 reference cycle  01:512 cycle  10:1024 cycle  11:2048 cycle
#define REG_RW_DMPLL_CFG2               0x0014              //DMPLL Configuration 2 Register
  #define DMPLL_NCO_DMPLL_STEP_MASK              0xFFFF0000
  #define DMPLL_NCO_DMPLL_STEP_OFFSET            16         //Spread step in 27MHz domain
  #define DMPLL_NCO_DMPLL_PERIOD_MASK            0x00000FFF
  #define DMPLL_NCO_DMPLL_PERIOD_OFFSET          0          //Spread period in 27MHz domain
#define REG_RW_AP_REG3               0x0018              //DMPLL Configuration 3 Register
  
  #define CPU1_AP_SEL_MASK             0x0C
  #define CPU1_AP_SEL_27M              (0 << 2)
  #define CPU1_AP_SEL_ARMPLL           (1 << 2)
  #define CPU1_AP_SEL_SYSPLL           (2 << 2)
  #define CPU1_AP_SEL_DMPLL            (3 << 2)
  #define AUD2_AP_SEL_MASK             0x00000003
  #define AUD2_AP_SEL_OFFSET           0          //Clock phase adjustment
#define REG_RW_MEMPLL_CFG0              0x001C              //MEMPLL Configuration 0 Register
  #define MEMPLL_MEMPLL_VCOCALSEL_MASK           0x60000000
  #define MEMPLL_MEMPLL_VCOCALSEL_OFFSET         29         //VCOCAL period selection    00:256 reference cycle  01:512 cycle  10:1024 cycle  11:2048 cycle
  #define MEMPLL_MEMPLL_RELATCHEN                (1U << 28) //Feedback divider relatch function    0:disable  1:enable
  #define MEMPLL_MEMPLL_VCOCAL_EN                (1U << 27) //VCOBAND calibration enable    0:VCOBAND_OUT[3:0]=VCOBAND[3:0]  1:VCOCAL enable
  #define MEMPLL_MEMPLL_POSTDIV_MASK             0x07000000
  #define MEMPLL_MEMPLL_POSTDIV_OFFSET           24         //000:/1  001:/2  010:/4  011:/8  100:/16  101:/32
  #define MEMPLL_MEMPLL_CMDIV1                   (1U << 22) //Capacitor Multiplication Number = 1
  #define MEMPLL_MEMPLL_CM_MASK                  0x003F0000
  #define MEMPLL_MEMPLL_CM_OFFSET                16         //Capacitor Multiplication Number =DMPLL_CM[7:0]+2)
  #define MEMPLL_MEMPLL_VCOBAND_MASK             0x0000F000
  #define MEMPLL_MEMPLL_VCOBAND_OFFSET           12         //VCOBAND selection (manul mode)
  #define MEMPLL_MEMPLL_CP_P_MASK                0x00000E00
  #define MEMPLL_MEMPLL_CP_P_OFFSET              9          //PLL BW Control
  #define MEMPLL_MEMPLL_DIV1                     (1U << 8)  //Feedback divider  0:/(DMPLL_DIV[7:0]+2)  1:/1
  #define MEMPLL_MEMPLL_DIV_MASK                 0x000000FC
  #define MEMPLL_MEMPLL_DIV_OFFSET               2          // /(DMPLL_DIV[7:0]+2)
  #define MEMPLL_MEMPLL_PREDIV_MASK              0x00000003
  #define MEMPLL_MEMPLL_PREDIV_OFFSET            0          //00:/1  01:/2  10:/4  11:/8
  //#define MEMPLL_MEMPLL_CKSEL_MASK               0x00000003
  //#define MEMPLL_MEMPLL_CKSEL_OFFSET             0          //00:REFCLK 27M  01:REFCLK_DUM[0]  10:REFCLK_DUM[1]  11:REFCLK_DUM[2]
#define REG_RW_MEMPLL_CFG1              0x0020              //MEMPLL Configuration 1 Register
  //#define MEMPLL_MEMPLL_TV_EN                    (1U << 21) //voltage test mode
  //#define MEMPLL_MEMPLL_TI_EN                    (1U << 20) //current test mode
  //#define MEMPLL_MEMPLL_MONCLKEN                 (1U << 15) //PLL test mode AIO enable(controlled by RG_MEMPLL_MONSEL)
  //#define MEMPLL_MEMPLL_MONAIOEN                 (1U << 14) //PLL test mode CLK enable(controlled by RG_MEMPLL_MONSEL)
  #define MEMPLL_MEMPLL_ABIST_PWD                (1U << 31) //PLL group ABIST power down    0:normal  1:power down
  #define MEMPLL_MEMPLL_ABIST_DIV1               (1U << 30) //Selected clock    0:/(RG_MEMPLL_ABIST_DIV[7:0]+2)  1:/1
  #define MEMPLL_MEMPLL_ABIST_DIV_MASK           0x3F000000
  #define MEMPLL_MEMPLL_ABIST_DIV_OFFSET         24         //Selected clock    /(RG_MEMPLL_ABIST_DIV[7:0]+2)
  #define MEMPLL_MEMPLL_MONSEL_MASK              0x00E00000
  #define MEMPLL_MEMPLL_MONSEL_OFFSET            21         //MEMPLL monitor control
  #define MEMPLL_MEMPLL_SEL_CK                   (1U << 20) //MEMPLL test mode control    0:AIO mode  1:CLK mode
  #define MEMPLL_MEMPLL_TEST_EN                  (1U << 19) //MEMPLL test mode enable    0:disable  1:enable
  #define MEMPLL_MEMPLL_BG_PWD                   (1U << 18) //PLL BG power down    0:normal  1:power down
  #define MEMPLL_MEMPLL_IBIAS_PLL_MASK           0x00030000
  #define MEMPLL_MEMPLL_IBIAS_PLL_OFFSET         16         //PLL bias R    00:21k  01:17.5k  10:27.75k  11:none
  #define MEMPLL_MEMPLL_CKDL_MASK                0x0000F000
  #define MEMPLL_MEMPLL_CKDL_OFFSET              12         //MEMPLL zero phase control
  #define MEMPLL_MEMPLL_PWD                      (1U << 11) //PLL power down    0:normal  1:power down
  #define MEMPLL_MEMPLL_RSTB                     (1U << 10) //PLL reset
  #define MEMPLL_MEMPLL_VCOVTSEL_MASK            0x00000300
  #define MEMPLL_MEMPLL_VCOVTSEL_OFFSET          8          //VCOCAL slicer voltage    00:0.72V  01:0.6V  10:0.48V
  #define MEMPLL_MEMPLL_REV_MASK                 0x000000FF
  #define MEMPLL_MEMPLL_REV_OFFSET               0          //Reserved r/g
#define REG_RW_MEMPLL_CFG2              0x0024              //MEMPLL Configuration 2 Register
  #define MEMPLL_MEMPLL_ZF_FLAG_RSTB             (1U << 0)  //Reset MEMPLL zero phase monitor flag
#define REG_RW_PLL_STA0                 0x0028              //PLL Group Status 0
#define REG_RW_PLL_STA1					        0x002C              //PLL Group Status 1
  #define PLL_STA1_SYSPLL2_VCOCALOK				       (1U << 20) 
  #define PLL_STA1_SYSPLL1_VCOCALOK				       (1U << 12) 
#define REG_R_PLL_STA2					        0x0030              //PLL Group Status 2
#define REG_RW_SYSPLL1_CFG0             0x0040              //SYSPLL1 Configuration 0 Register
  #define SYSPLL1_SYSPLL1_PWD                    (1U << 31) //PLL power down    0:normal  1:power down
  #define SYSPLL1_SYSPLL1_RSTB                   (1U << 30) //PLL reset
  #define SYSPLL1_SYSPLL1_VCOCALSEL_MASK         0x30000000
  #define SYSPLL1_SYSPLL1_VCOCALSEL_OFFSET       28         //VCOCAL period selection    00:256 reference cycle  01:512 cycle  10:1024 cycle  11:2048 cycle
  #define SYSPLL1_SYSPLL1_RELATCH_EN             (1U << 27) //Feedback divider relatch function    0:disable  1:enable
  #define SYSPLL1_SYSPLL1_POSTDIV_MASK           0x03000000
  #define SYSPLL1_SYSPLL1_POSTDIV_OFFSET         24         //000:/1  001:/2  010:/4  011:/8  100:/16  101:/32
  #define SYSPLL1_SYSPLL1_VCOCAL_EN              (1U << 23) //VCOBAND calibration enable    0:VCOBAND_OUT[3:0]=VCOBAND[3:0]  1:VCOCAL enable
  #define SYSPLL1_SYSPLL1_CMDIV1                 (1U << 22) //Capacitor Multiplication Number=SYSPLL1_CMDIV1[7:0]+2)
  #define SYSPLL1_SYSPLL1_CM_MASK                0x003F0000
  #define SYSPLL1_SYSPLL1_CM_OFFSET              16         ///(SYSPLL1_CM[7:0]+2)
  #define SYSPLL1_SYSPLL1_VCOBAND_MASK           0x0000F000
  #define SYSPLL1_SYSPLL1_VCOBAND_OFFSET         12         //VCOBAND selection (manul mode)
  #define SYSPLL1_SYSPLL1_CP_P_MASK              0x00000E00
  #define SYSPLL1_SYSPLL1_CP_P_OFFSET            9          //PLL BW Control
  #define SYSPLL1_SYSPLL1_DIV1                   (1U << 8)  //Feedback divider  0:/(SYSPLL_DIV[7:0]+2)  1:/1
  #define SYSPLL1_SYSPLL1_DIV_MASK               0x000000FC
  #define SYSPLL1_SYSPLL1_DIV_OFFSET             2          // /(SYSPLL_DIV[7:0]+2)
  #define SYSPLL1_SYSPLL1_PREDIV_MASK            0x00000003
  #define SYSPLL1_SYSPLL1_PREDIV_OFFSET          0          //00:/1  01:/2  10:/4  11:/8
  //#define SYSPLL1_SYSPLL1_CKSEL_MASK             0x00000003
  //#define SYSPLL1_SYSPLL1_CKSEL_OFFSET           0          //00:REFCLK 27M  01:REFCLK_DUM[0]  10:REFCLK_DUM[1]  11:REFCLK_DUM[2]
#define REG_RW_SYSPLL1_CFG1             0x0044              //SYSPLL1 Configuration 1 Register
  #define SYSPLL1_USB_DIV_MASK                   0x00001C00
  #define SYSPLL1_USB_DIV_OFFSET                 10         //To usb clock= 432M/(RG_USB_DIV[5:0]+2)
  #define SYSPLL1_SYSPLL1_VCOVTSEL_MASK          0x00000300
  #define SYSPLL1_SYSPLL1_VCOVTSEL_OFFSET        8          //VCOCAL slicer voltage    00:0.72V  01:0.6V  10:0.48V
  #define SYSPLL1_SYSPLL1_REV_MASK               0x000000FF
  #define SYSPLL1_SYSPLL1_REV_OFFSET             0          ///(Reserved r/g
#define REG_RW_SYSPLL2_CFG0             0x0048              //SYSPLL2 Configuration 0 Register
  #define SYSPLL2_SYSPLL2_VCOCALSEL_MASK         0x60000000
  #define SYSPLL2_SYSPLL2_VCOCALSEL_OFFSET       29         //VCOCAL period selection    00:256 reference cycle  01:512 cycle  10:1024 cycle  11:2048 cycle
  #define SYSPLL2_SYSPLL2_RELATCH_EN             (1U << 28) //Feedback divider relatch function    0:disable  1:enable
  #define SYSPLL2_SYSPLL2_VCOCAL_EN              (1U << 27) //VCOBAND calibration enable    0:VCOBAND_OUT[3:0]=VCOBAND[3:0]  1:VCOCAL enable
  #define SYSPLL2_SYSPLL2_POSTDIV_MASK           0x07000000
  #define SYSPLL2_SYSPLL2_POSTDIV_OFFSET         24         //000:/1  001:/2  010:/4  011:/8  100:/16  101:/32
  #define SYSPLL2_SYSPLL2_CMDIV1                 (1U << 22) //Capacitor Multiplication Number=SYSPLL2_CMDIV1[7:0]+2)
  #define SYSPLL2_SYSPLL2_CM_MASK                0x003F0000
  #define SYSPLL2_SYSPLL2_CM_OFFSET              16         ///(SYSPLL2_CM[7:0]+2)
  #define SYSPLL2_SYSPLL2_VCOBAND_MASK           0x0000F000
  #define SYSPLL2_SYSPLL2_VCOBAND_OFFSET         12         //VCOBAND selection (manul mode)
  #define SYSPLL2_SYSPLL2_CP_P_MASK              0x00000E00
  #define SYSPLL2_SYSPLL2_CP_P_OFFSET            9          //PLL BW Control
  #define SYSPLL2_SYSPLL2_DIV1                   (1U << 8)  //Feedback divider  0:/(SYSPLL_DIV[7:0]+2)  1:/1
  #define SYSPLL2_SYSPLL2_DIV_MASK               0x000000FC
  #define SYSPLL2_SYSPLL2_DIV_OFFSET             2          // /(SYSPLL_DIV[7:0]+2)
  #define SYSPLL2_SYSPLL2_PREDIV_MASK            0x00000003
  #define SYSPLL2_SYSPLL2_PREDIV_OFFSET          0          //00:/1  01:/2  10:/4  11:/8
  //#define SYSPLL2_SYSPLL2_CKSEL_MASK             0x00000003
  //#define SYSPLL2_SYSPLL2_CKSEL_OFFSET           0          //00:REFCLK 27M  01:REFCLK_DUM[0]  10:REFCLK_DUM[1]  11:REFCLK_DUM[2]
#define REG_RW_SYSPLL2_CFG1             0x004C              //SYSPLL2 Configuration 1 Register
  #define SYSPLL2_SYSPLL2_PWD                    (1U << 11) //PLL power down    0:normal  1:power down
  #define SYSPLL2_SYSPLL2_RSTB                   (1U << 10) //PLL reset
  #define SYSPLL2_SYSPLL2_VCOVTSEL_MASK          0x00000300
  #define SYSPLL2_SYSPLL2_VCOVTSEL_OFFSET        8          //VCOCAL slicer voltage    00:0.72V  01:0.6V  10:0.48V
  #define SYSPLL2_SYSPLL2_REV_MASK               0x000000FF
  #define SYSPLL2_SYSPLL2_REV_OFFSET             0          ///(Reserved r/g
#define REG_RW_PLL_CFG0                 0x0050
  #define PLL_CFG0_SATA_CLK_SEL         (1U << 28)   //8555 add
  #define PLL_CFG0_RG_MEMPLL_CKSEL      (1U << 25)   //8555 add
  #define PLL_CFG0_RG_PLL_ABIST_DIV_MASK 0x007E0000
  #define PLL_CFG0_RG_PLL_ABIST_DIV_OFFSET 17
#define REG_RW_PLL_CFG1                 0x0054
#define REG_RW_MSC_CFG                  0x0058
  #define MSC_CFG_TEST_VDAC             (1U <<16)    // 0:Normal work mode //8555 add
  #define MSC_CFG_SATAPLL_TEST          (1U <<13)    // Wwitch clock from SATAPLL to external pin (PAD_NFD7) //8555 add
  #define MSC_CFG_SATAPLL_RST_DIVIDER   (1U <<12)    // Reset of SATAPLL divider,active high //8555 add
#define REG_RW_TST_CFG0                 0x005C              //Test Configuration 0
  #define TST_CFG0_AXICLK_RATIO_MASK             0x001C0000
  #define TST_CFG0_AXICLK_RATIO_OFFSET           18
  #define TST_CFG0_BCLK_RATIO_MASK               0x00000070
  #define TST_CFG0_BCLK_RATIO_OFFSET             4
#define REG_RW_TST_CFG1                 0x0060              //Test Configuration Register 1 
  #define TST_CFG1_RISCCLK_RATIO_MASK            0x00001C00 //ARMCK RATIO Mask
  #define TST_CFG1_RISCCLK_RATIO_OFFSET          10         //ARMCK RATIO Offset
  //#define TST_CFG1_RISCCLK2_RATIO_MASK           0x0000E000 //ARM2CK RATIO Mask
  //#define TST_CFG1_RISCCLK2_RATIO_OFFSET         13         //ARM2CK RATIO Offset
  #define TST_CFG1_AXICLK2_RATIO_MASK            0x00070000 //AXI2 RATIO Mask (backward compatibility
  #define TST_CFG1_AXICLK2_RATIO_OFFSET          16         //AXI2 RATIO Offset (backward compatibility
#define REG_RW_RST_CFG                  0x0064              //Reset Configuration Register
  #define RST_CFG_TRAP_MAKS                      0x03FF0000
  #define RST_CFG_TRAP_OFFSET                    16
  #define RST_CFG_PROT_MASK                      0x000003FF
  #define RST_CFG_PROT_OFFSET                    0
//#define REG_RW_RST_RISC_PD              0x006C            //8555 delete
#define REG_RW_CLK_CFG0                 0x0070              //Clock Selection Configuration 0
  #define CLK_PDN_VFD                            (1U << 31) //turn off VFD
  #define CLK_SLOW_RISC_SEL_MASK                 0x70000000
  #define CLK_SLOW_RISC_SEL_OFFSET               28         //Select divisor used to divide 27MHz clock, for RISC clock slow option
  #define CLK_SLOW_RISC_SEL_27M_1_2048           7
  #define CLK_SLOW_RISC_SEL_27M_1_1024           6
  #define CLK_SLOW_RISC_SEL_27M_1_512            5
  #define CLK_SLOW_RISC_SEL_27M_1_256            4
  #define CLK_SLOW_RISC_SEL_27M_1_128            3
  #define CLK_SLOW_RISC_SEL_27M_1_64             2
  #define CLK_SLOW_RISC_SEL_27M_1_32             1
  #define CLK_SLOW_RISC_SEL_27M_1_16             0
  #define CLK_PDN_RISC2                          (1U << 27) //turn off RISC2
  #define CLK_CLK_RISC2_SEL_MASK                 0x07000000 // 8555 add
  #define CLK_CLK_RISC2_SEL_OFFSET               24         //Selection of RISC2 clock frequency
  #define CLK_CLK_RISC2_SEL_DMPLL                5
  #define CLK_CLK_RISC2_SEL_SLOW_RISC_CLK        4
  #define CLK_CLK_RISC2_SEL_SYSPLL2              3
  #define CLK_CLK_RISC2_SEL_SYSPLL1              2
  #define CLK_CLK_RISC2_SEL_ARMPLL               1
  #define CLK_CLK_RISC2_SEL_27M                  0
  #define CLK_PDN_SACD                           (1U << 23) //turn off SACD
  #define CLK_CLK_SACD_SEL_MASK                  0x00700000
  #define CLK_CLK_SACD_SEL_OFFSET                20         //Selection of SACD clock frequency
  #define CLK_CLK_SACD_SEL_27M                   0
  #define CLK_CLK_SACD_SEL_SYSPLL1_1_6           1
  #define CLK_CLK_SACD_SEL_SYSPLL1_1_8           2
  #define CLK_CLK_SACD_SEL_SYSPLL1_1_10          3
  #define CLK_CLK_SACD_SEL_SYSPLL1_1_12          4
  #define CLK_CLK_SACD_SEL_SYSPLL2_1_8           5
  #define CLK_CLK_SACD_SEL_SYSPLL2_1_12          6
  #define CLK_CLK_SACD_SEL_SYSPLL2_1_18          7
  #define CLK_PDN_NFLASH                         (1U << 19) //turn off Nand FLASH
  #define CLK_CLK_NFLASH_SEL_MASK                0x00070000
  #define CLK_CLK_NFLASH_SEL_OFFSET              16         //Selection of NAND FLASH clock frequency
  #define CLK_CLK_NFLASH_SEL_27M                 0
  #define CLK_CLK_NFLASH_SEL_ARMPLL_1_3          1
  #define CLK_CLK_NFLASH_SEL_SYSPLL1_1_2         2
  #define CLK_CLK_NFLASH_SEL_SYSPLL1_1_3         3
  #define CLK_CLK_NFLASH_SEL_SYSPLL1_1_4         4
  #define CLK_CLK_NFLASH_SEL_SYSPLL2_1_3         5
  #define CLK_CLK_NFLASH_SEL_DMPLL_1_2           6
  #define CLK_CLK_NFLASH_SEL_USBPLL_240M         7
  #define CLK_PDN_FLASH                          (1U << 15) //turn off FLASH
  #define CLK_CLK_FLASH_SEL_MASK                 0x00007000
  #define CLK_CLK_FLASH_SEL_OFFSET               12         //Selection of FLASH clock frequency
  #define CLK_CLK_FLASH_SEL_27M_1_2              0
  #define CLK_CLK_FLASH_SEL_27M                  1
  #define CLK_CLK_FLASH_SEL_SYSPLL1_1_6          2
  #define CLK_CLK_FLASH_SEL_SYSPLL1_1_8          3
  #define CLK_CLK_FLASH_SEL_SYSPLL1_1_10         4
  #define CLK_CLK_FLASH_SEL_SYSPLL1_1_12         5
  #define CLK_CLK_FLASH_SEL_SYSPLL2_1_6          6
  #define CLK_CLK_FLASH_SEL_DMPLL_1_8            7
  #define CLK_PDN_ADSP2                          (1U << 11) //turn off ADSP
  #define CLK_CLK_ADSP2_SEL_MASK                 0x00000700  //8555 change
  #define CLK_CLK_ADSP2_SEL_OFFSET               8          //Selection of ADSP clock frequency
  #define CLK_CLK_ADSP2_SEL_27M                  0
  #define CLK_CLK_ADSP2_SEL_ADSP                 1
  #define CLK_CLK_ADSP2_SEL_DMPLL                2
  #define CLK_PDN_ADSP                           (1U << 7)  //turn off ADSP(if clk_ADSP!=clk_DRAM)
  #define CLK_CLK_ADSP_SEL_MASK                  0x00000070
  #define CLK_CLK_ADSP_SEL_OFFSET                4          //Selection of ADSP clock frequency
  #define CLK_CLK_ADSP_SEL_27M                   0
  #define CLK_CLK_ADSP_SEL_SYSPLL2_2_3           1
  #define CLK_CLK_ADSP_SEL_ARMPLL_1_2            2
  #define CLK_CLK_ADSP_SEL_SYSPLL1_1_2           3
  #define CLK_CLK_ADSP_SEL_SYSPLL1_1_3           4
  #define CLK_CLK_ADSP_SEL_SYSPLL2_1_2           5
  #define CLK_CLK_ADSP_SEL_SYSPLL2_1_3           6
  #define CLK_CLK_ADSP_SEL_USBPLL_240M           7
  #define CLK_PDN_RISC                           (1U << 3)  //turn off RISC
  #define CLK_CLK_RISC_SEL_MASK                  0x00000007
  #define CLK_CLK_RISC_SEL_OFFSET                0          //Selection of RISC clock frequency
  #define CLK_CLK_RISC_SEL_27M                   0
  #define CLK_CLK_RISC_SEL_ARMPLL                1
  #define CLK_CLK_RISC_SEL_SYSPLL1               2
  #define CLK_CLK_RISC_SEL_SYSPLL2               3
  #define CLK_CLK_RISC_SEL_SLOW_RISC_CLK         4
  #define CLK_CLK_RISC_SEL_DMPLL                 5
#define REG_RW_CLK_CFG1                 0x0074              //Clock Selection Configuration 1
  #define CLK_PDN_DEMUX                          (1U << 31) //turn off DEMUX
  #define CLK_CLK_DEMUX_SEL_MASK                 0x70000000
  #define CLK_CLK_DEMUX_SEL_OFFSET               28         //Selection of DEMUX clock frequency
  #define CLK_CLK_DEMUX_SEL_27M                  0
  #define CLK_CLK_DEMUX_SEL_ARMPLL_1_3           1
  #define CLK_CLK_DEMUX_SEL_SYSPLL1_1_2          2
  #define CLK_CLK_DEMUX_SEL_SYSPLL1_1_3          3
  #define CLK_CLK_DEMUX_SEL_SYSPLL1_1_4          4
  #define CLK_CLK_DEMUX_SEL_SYSPLL2_1_3          5
  #define CLK_CLK_DEMUX_SEL_DMPLL_1_2            6
  #define CLK_CLK_DEMUX_SEL_USBPLL_240M          7
  #define CLK_PDN_NR                             (1U << 27) //turn off NR
  #define CLK_CLK_NR_SEL_MASK                    0x07000000
  #define CLK_CLK_NR_SEL_OFFSET                  24         //Selection of NR clock frequency
  #define CLK_CLK_NR_SEL_27M                     0
  #define CLK_CLK_NR_SEL_SYSPLL1_1_3             1
  #define CLK_CLK_NR_SEL_SYSPLL1_1_4             2
  #define CLK_CLK_NR_SEL_SYSPLL2_1_3             3
  #define CLK_CLK_NR_SEL_DMPLL_1_2               4
  #define CLK_PDN_GRAPH                          (1U << 23) //turn off GRAPH
  #define CLK_CLK_GRAPH_SEL_MASK                 0x00700000
  #define CLK_CLK_GRAPH_SEL_OFFSET               20         //Selection of GRAPH clock frequency
  #define CLK_CLK_GRAPH_SEL_27M                  0
  #define CLK_CLK_GRAPH_SEL_APLL                 1
  #define CLK_CLK_GRAPH_SEL_ARMPLL_1_2           2
  #define CLK_CLK_GRAPH_SEL_ARMPLL_1_3           3
  #define CLK_CLK_GRAPH_SEL_SYSPLL1_1_2          4
  #define CLK_CLK_GRAPH_SEL_SYSPLL2_1_2          5
  #define CLK_CLK_GRAPH_SEL_DMPLL                6
  #define CLK_CLK_GRAPH_SEL_USBPLL_240M          7
  #define CLK_PDN_RESZ                           (1U << 19) //turn off RESZ
  #define CLK_CLK_RESZ_SEL_MASK                  0x00070000
  #define CLK_CLK_RESZ_SEL_OFFSET                16         //Selection of RESZ clock frequency
  #define CLK_CLK_RESZ_SEL_27M                   0
  #define CLK_CLK_RESZ_SEL_SYSPLL1_1_2           1
  #define CLK_CLK_RESZ_SEL_SYSPLL2_1_2           2
  #define CLK_CLK_RESZ_SEL_USBPLL_240M           3
  #define CLK_CLK_RESZ_SEL_DMPLL_1_2             4
  #define CLK_PDN_PNG                            (1U << 15) //turn off PNG
  #define CLK_CLK_PNG_SEL_MASK                   0x00007000
  #define CLK_CLK_PNG_SEL_OFFSET                 12         //Selection of PNG clock frequency
  #define CLK_CLK_PNG_SEL_27M                    0
  #define CLK_CLK_PNG_SEL_APLL                   1
  #define CLK_CLK_PNG_SEL_ARMPLL_1_2             2
  #define CLK_CLK_PNG_SEL_ARMPLL_1_3             3
  #define CLK_CLK_PNG_SEL_SYSPLL1_1_2            4
  #define CLK_CLK_PNG_SEL_SYSPLL2_1_2            5
  #define CLK_CLK_PNG_SEL_DMPLL                  6
  #define CLK_CLK_PNG_SEL_USBLL_240M             7
  #define CLK_PDN_OSD                            (1U << 11) //turn off OSD
  #define CLK_CLK_OSD_SEL_MASK                   0x00000700
  #define CLK_CLK_OSD_SEL_OFFSET                 8          //Selection of OSD clock frequency
  #define CLK_CLK_OSD_SEL_27M                    0
  #define CLK_CLK_OSD_SEL_SYSPLL1_1_2            1
  #define CLK_CLK_OSD_SEL_SYSPLL2_1_3            2
  #define CLK_CLK_OSD_SEL_USBPLL_240M            3
  #define CLK_CLK_OSD_SEL_DMPLL_1_2              4
  #define CLK_CLK_OSD_SEL_APLL                   5
  #define CLK_CLK_OSD_SEL_ARMPLL_1_3             6
  #define CLK_CLK_OSD_SEL_SYSPLL2_1_2            7
  #define CLK_PDN_MC                             (1U << 7)  //turn off MC
  #define CLK_CLK_MC_SEL_MASK                    0x00000070
  #define CLK_CLK_MC_SEL_OFFSET                  4          //Selection of MC clock frequency
  #define CLK_CLK_MC_SEL_27M                     0
  #define CLK_CLK_MC_SEL_APLL                    1
  #define CLK_CLK_MC_SEL_DMPLL                   2
  #define CLK_CLK_MC_SEL_ARMPLL_1_3              3
  #define CLK_CLK_MC_SEL_SYSPLL1_1_2             4
  #define CLK_CLK_MC_SEL_SYSPLL2_1_2             5
  #define CLK_CLK_MC_SEL_SYSPLL2_1_3             6
  #define CLK_CLK_MC_SEL_USBPLL_240M             7
  #define CLK_PDN_VDEC                           (1U << 3)  //turn off VDEC
  #define CLK_CLK_VDEC_SEL_MASK                  0x00000007
  #define CLK_CLK_VDEC_SEL_OFFSET                0          //Selection of VDEC clock frequency
  #define CLK_CLK_VDEC_SEL_27M                   0
  #define CLK_CLK_VDEC_SEL_SYSPLL1_1_2           1
  #define CLK_CLK_VDEC_SEL_SYSPLL2_1_3           2
  #define CLK_CLK_VDEC_SEL_USBPLL                3
  #define CLK_CLK_VDEC_SEL_DMPLL_1_2             4
#define REG_RW_CLK_CFG2                 0x0078              //Clock Selection Configuration 2
  #define CLK_PDN_ABIST                          (1U << 31) //turn off ABIST frequency meter clock
  #define CLK_CLK_ABIST_SEL_MASK                 0x70000000
  #define CLK_CLK_ABIST_SEL_OFFSET               28         //Selection of ABIST clock frequency
  #define CLK_CLK_ABIST_SEL_27M                  0
  #define CLK_CLK_ABIST_SEL_AD_XTAL27M_CK        1
  #define CLK_CLK_ABIST_SEL_AD_MEMPLL_MONCK      2
  #define CLK_CLK_ABIST_SEL_AD_PLL_MONCK         3
  #define CLK_CLK_ABIST_SEL_AD_HDMI_LBOUT        4
  #define CLK_CLK_ABIST_SEL_USBPHY_CK30          5
  #define CLK_CLK_ABIST_SEL_USBPHY_CK240         6
  #define CLK_CLK_ABIST_SEL_CLK_ABIST2           7
  
  #define CLK_PDN_ABIST2                           (1U <<27)
  #define CLK_CLK_ABIST2_SEL_MASK                 0x07000000  
  #define CLK_CLK_ABIST2_SEL_OFFSET                  24 
  #define CLK_CLK_ABIST2_SEL_27M                  0
  #define CLK_CLK_ABIST2_SEL_AD_USB_MONCLK        1
  #define CLK_CLK_ABIST2_SEL_AD_USB_MONCLK_0      2
  #define CLK_CLK_ABIST2_SEL_AD_USB_MONCLK_1      3
  #define CLK_CLK_ABIST2_SEL_AD_USB_ABIST_CK      4
  
  
  #define CLK_PDN_MS                             (1U << 23) //turn off MS
  #define CLK_CLK_MS_SEL_MASK                    0x007F0000
  #define CLK_CLK_MS_SEL_OFFSET                  16         //Selection of MS clock frequency
  #define CLK_CLK_MS_SEL_27M                     0x00
  #define CLK_CLK_MS_SEL_SYSPLL1_1_8             0x01  // 54Mhz    //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL1_1_10            0x02 // 43.2Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL1_1_12            0x03  // 36Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL2_1_12            0x04   // 50Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_DMPLL_1_8               0x05   // 37.5Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL2_1_18            0x06   // 33.33Mhz  //mhzhang
  //added for 8550 //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL2_1_26       0x07  //23.08Mhz //mhzhang
  #define CLK_CLK_MS_SEL_SYSPLL1_1_24         0x08  // 18Mhz //mhzhang
  
  #define CLK_CLK_MS_SEL_27M_1_2                 0x09   // 13.5Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_4                 0x19    // 6.75Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_8                 0x29   // 3.38Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_16                0x39   // 1.69Mhz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_32                0x49   // 847.75Khz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_64                0x59   // 421.885Khz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_128               0x69   // 210.94Khz  //mhzhang
  #define CLK_CLK_MS_SEL_27M_1_256               0x79   // 105.47Khz  //mhzhang
  #define CLK_PDN_SD                             (1U << 15) //turn off SD
  #define CLK_CLK_SD_SEL_MASK                    0x00007F00
  #define CLK_CLK_SD_SEL_OFFSET                  8          //Selection of SD clock frequency
  #define CLK_CLK_SD_SEL_27M                     0x00
  #define CLK_CLK_SD_SEL_SYSPLL1_1_8             0x01    // 54Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_SYSPLL1_1_10            0x02    // 43.2Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_SYSPLL1_1_12            0x03    // 36Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_SYSPLL2_1_12            0x04    // 50Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_DMPLL_1_8               0x05    // 37.5Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_SYSPLL2_1_18            0x06    // 33.33Mhz  //mhzhang
  //added for 8550 //mhzhang
  #define CLK_CLK_SD_SEL_SYSPLL2_1_26        0x07  // 23.08 Mhz 
  #define CLK_CLK_SD_SEL_SYSPLL1_1_24        0x08 // 18 Mhz
  
  #define CLK_CLK_SD_SEL_27M_1_2                 0x09    // 13.5Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_4                 0x19    // 6.75Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_8                 0x29    // 3.38Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_16                0x39    // 1.69Mhz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_32                0x49    // 843.75 Khz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_64                0x59    // 421.885 Khz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_128               0x69    // 210.94 Khz  //mhzhang
  #define CLK_CLK_SD_SEL_27M_1_256               0x79    // 105.47 Khz  //mhzhang
  #define CLK_PDN_IR                             (1U << 7)  //turn off IR
  #define CLK_CLK_IR_DIV_SEL_MASK                0x0000000F
  #define CLK_CLK_IR_DIV_SEL_OFFSET              0          //Select divisor used to divide 27MHz clock, for IR clock
  #define CLK_CLK_IR_DIV_SEL_27M                 0
  #define CLK_CLK_IR_DIV_SEL_27M_1_2             1
  #define CLK_CLK_IR_DIV_SEL_27M_1_4             2
  #define CLK_CLK_IR_DIV_SEL_27M_1_8             3
  #define CLK_CLK_IR_DIV_SEL_27M_1_16            4
  #define CLK_CLK_IR_DIV_SEL_27M_1_32            5
  #define CLK_CLK_IR_DIV_SEL_27M_1_64            6
  #define CLK_CLK_IR_DIV_SEL_27M_1_128           7
  #define CLK_CLK_IR_DIV_SEL_27M_1_256           8
#define REG_RW_CLK_CFG3                 0x007C              //Clock Selection Configuration 3
  #define CLK_CFG3_PDN_BCLK                  (1U << 31)  //8555 add
  #define CLK_CFG3_CLK_BCLK_SEL_MASK          0x70000000  //8555 add
  #define CLK_CFG3_CLK_BCLK_SEL_OFFSET        28          //8555 add
  #define CLK_CFG3_CLK_BCLK_DEFAULT_CLK_RISC      0x00
  #define CLK_CFG3_CLK_BCLK_SEL_DMPLL_1_1         0x01  
  #define CLK_CFG3_CLK_BCLK_SEL_SYSPLL1_1_2       0x02  
  #define CLK_CFG3_CLK_BCLK_SEL_SYSPLL2_1_2       0x03  
  #define CLK_CFG3_CLK_BCLK_SEL_SVOIPLL_1_2       0x04
  #define CLK_CFG3_CLK_BCLK_SEL_SATAPLL_1_2       0x05
  #define CLK_CFG3_CLK_BCLK_SEL_27M               0x06
  //#define CLK_CFG3_CLK_BCLK_SEL_27M               0x07

  #define CLK_CFG3_PDN_VENC_2F                  (1U << 27)  //8555 add
  #define CLK_CFG3_CLK_VENC_2F_SEL_MASK          0x07000000  //8555 add
  #define CLK_CFG3_CLK_VENC_2F_SEL_OFFSET        24          //8555 add
  #define CLK_CFG3_CLK_VENC_2F_SEL_27M               0x00
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL1_1_1       0x01    
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL1_1_2       0x02    
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL1_1_3       0x03    
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL1_1_4       0x04    
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL1_1_6       0x05   
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL2_10_15     0x06 
  #define CLK_CFG3_CLK_VENC_2F_SEL_SYSPLL2_10_30     0x07

  #define CLK_CFG3_PDN_SIF                      (1U << 23)  //8555 add
  #define CLK_CFG3_CLK_SIF_SEL_MASK              0x00700000  //8555 add
  #define CLK_CFG3_CLK_SIF_SEL_OFFSET            20          //8555 add
  #define CLK_CFG3_CLK_SIF_SEL_27M               0x00
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL1_1_8       0x01    
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL1_1_10      0x02    
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL1_1_12      0x03    
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL1_1_24      0x04    
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL2_1_6       0x05   
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL2_1_18      0x06 
  #define CLK_CFG3_CLK_SIF_SEL_SYSPLL2_1_26      0x07

  #define CLK_CFG3_PDN_SPI                      (1U << 19)  //8555 add
  #define CLK_CFG3_CLK_SPI_SEL_MASK              0x00070000  //8555 add
  #define CLK_CFG3_CLK_SPI_SEL_OFFSET            16          //8555 add
  #define CLK_CFG3_CLK_SPI_SEL_27M               0x00
  #define CLK_CFG3_CLK_SPI_SEL_SYSPLL1_1_3       0x01    
  #define CLK_CFG3_CLK_SPI_SEL_SYSPLL1_1_4       0x02    
  #define CLK_CFG3_CLK_SPI_SEL_SYSPLL1_1_6       0x03    
  #define CLK_CFG3_CLK_SPI_SEL_SYSPLL1_1_8       0x04    
  #define CLK_CFG3_CLK_SPI_SEL_SYSPLL1_1_12      0x05   
  #define CLK_CFG3_CLK_SPI_SEL_27M_2             0x06
  #define CLK_CFG3_CLK_SPI_SEL_27M_4             0x07
  
  #define CLK_CFG3_PDN_TSOUT                    (1U << 15)  //8555 add
  #define CLK_CFG3_CLK_TSOUT_SEL_MASK            0x00007000  //8555 add
  #define CLK_CFG3_CLK_TSOUT_SEL_OFFSET          12
  #define CLK_CFG3_CLK_TSOUT_SEL_27M             0x00
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_3     0x01    
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_4     0x02    
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_6     0x03    
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_8     0x04    
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_10    0x05   
  #define CLK_CFG3_CLK_TSOUT_SEL_SYSPLL1_1_12    0x06    
  
  
  #define CLK_PDN_MVDO2                          (1U << 11)  //turn off clk_mvdo2
  #define CLK_CLK_MVDO2_SEL_MASK                 0x00000700
  #define CLK_CLK_MVDO2_SEL_OFFSET               8          //Select divisor used to divide 27MHz clock, for mvdo2 clock
  #define CLK_CLK_MVDO2_SEL_27M                  0
  #define CLK_CLK_MVDO2_SEL_SYSPLL1_1_3          1
  #define CLK_CLK_MVDO2_SEL_SYSPLL2_1_4          2
  #define CLK_PDN_GCPU                           (1U << 7)  //turn off clk_gcpu
  #define CLK_CLK_GCPU_SEL_MASK                  0x00000070
  #define CLK_CLK_GCPU_SEL_OFFSET                4          //Select divisor used to divide 27MHz clock, for gcpu clock
  #define CLK_CLK_GCPU_SEL_27M                   0
  #define CLK_CLK_GCPU_SEL_DRAM_CLK              1
  #define CLK_CLK_GCPU_SEL_SYSPLL1_1_2           2
  #define CLK_CLK_GCPU_SEL_SYSPLL2_1_2           3
  #define CLK_CLK_GCPU_SEL_SVOIPLL_1_2           4
  #define CLK_PDN_SVO_STDBY                      (1U << 3)  //turn off clk_svo_stdby
  #define CLK_CLK_SVO_STDBY_SEL_MASK             0x00000007
  #define CLK_CLK_SVO_STDBY_SEL_OFFSET           0          //Select divisor used to divide 27MHz clock, for svo_stdby clock
  #define CLK_CLK_SVO_STDBY_SEL_27M              0
  #define CLK_CLK_SVO_STDBY_SEL_SVOIPLL_1_27     1

#define REG_RW_CLK_CFG4                 0x0080              //Clock Selection Configuration 4 ,8555 add FOR HDMI RX
  #define CLK_CFG4_PCLK_IN_SEL_MASK     0x1E000000
  #define CLK_CFG4_PCLK_IN_SEL_OFFSET   25
  #define CLK_CFG4_PCLK_MUX_SELECT      (1U << 28)   //1:from RX PLL; 0:from pin
  #define CLK_CFG4_PCLK_INVERT          (1U << 27)   //1:invert
  #define CLK_CFG4_PCLK_MUX             (1U << 26)   //1:clk_27m
  #define CLK_CFG4_PCLK_ON_OFF          (1U << 25)   //1:on
  
  
  #define CLK_CFG4_MCLK_IN_SEL_MASK     0x001C0000
  #define CLK_CFG4_MCLK_IN_SEL_OFFSET   22
  #define CLK_CFG4_MCLK_SELECT          (1U << 24)   //1:from RX digital; 0:from pin
  #define CLK_CFG4_MCLK_MUX             (1U << 23)   //1:clk_27m
  #define CLK_CFG4_MCLK_ON_OFF          (1U << 22)   //1:on
  
  
  #define CLK_CFG4_VCOCLK_IN_SEL_MASK     0x00380000
  #define CLK_CFG4_VCOCLK_IN_SEL_OFFSET   19
  #define CLK_CFG4_VCOCLK_IN_SELECT    (1U << 21)   //1:from RX APLL; 0:from clk_27m
  #define CLK_CFG4_VCOCLK_MUX           (1U << 20)   //0: clk_27m
  #define CLK_CFG4_VCOCLK_ON_OFF          (1U << 19)   //1: ON
  
  
  #define CLK_CFG4_PDTCLK_IN_SEL_MASK     0x00060000
  #define CLK_CFG4_PDTCLK_IN_SEL_OFFSET   17
  #define CLK_CFG4_PDTCLK_SELECT   (1U << 18)  //1:from RXPLL ; 0:from clk_27m
  #define CLK_CFG4_PDTCLK_ON_OFF   (1U << 17)  //1:on
  
  #define CLK_CFG4_PLLCLK_IN_SEL_MASK     0x0001C000
  #define CLK_CFG4_PLLCLK_IN_SEL_OFFSET   14
  #define CLK_CFG4_PLLCLK_SELECT          (1U << 16)   //1:from RX PLL ; 0: from pin
  #define CLK_CFG4_PLLCLK_MUX             (1U << 15)   //1:from RX PLL or pin ; 0: from clk_27m
  #define CLK_CFG4_PLLCLK_ON_OFF          (1U << 14)   //1:on
  
  
  #define CLK_CFG4_DPCLK_IN_SEL_MASK     0x00003E00
  #define CLK_CFG4_DPCLK_IN_SEL_OFFSET   9
  #define CLK_CFG4_DPCLK_FROM_PIN          (1U << 13)   //0:dpclk from pin (test use)
  #define CLK_CFG4_DPCLK_SELECT_PIXEL_CLOCK      (1U << 12)   //1:pixel clock ; 0:tmds clock
  #define CLK_CFG4_DPCLK_INVERT      (1U << 11)   //1:invert
  #define CLK_CFG4_DPCLK_SELECT      (1U << 10)   //0:clk_27m
  #define CLK_CFG4_DPCLK_ON_OFF      (1U << 9)   //1:on
  
  #define CLK_CFG4_XCLK_IN_SEL_MASK     0x00000180
  #define CLK_CFG4_XCLK_IN_SEL_OFFSET   7
  #define CLK_CFG4_XCLK_CLOCK_MUX      (1U << 8)  //1:from RX APLL ; 0: from clk_27m
  #define CLK_CFG4_XCLK_CLOCK_ON_OFF      (1U << 7)//1:on
    
  #define CLK_CFG4_FCLK_IN_SEL_MASK     0x00000070
  #define CLK_CFG4_FCLK_IN_SEL_OFFSET   4
  #define CLK_CFG4_FCLK_SELECT_FROM_RXAPLL          (1U << 6) //1:from RX APLL ; 0: from pin
  #define CLK_CFG4_FCLK_SELECT          (1U << 5) //1:normal fclk in ; 0:clk_27m
  #define CLK_CFG4_FCLK_ON_OFF          (1U << 4) //1:on
  
  #define CLK_CFG4_CSCL_IN_SEL_MASK   0x0000000C
  #define CLK_CFG4_CSCL_IN_SEL_OFFSET   2
  #define CLK_CFG4_DSCL_IN_SEL_MASK   0x00000003
  #define CLK_CFG4_DSCL_IN_SEL_OFFSET   0  
  
  #define CLK_CFG4_DSCL_CLOCK_SELECT    (1U << 1)   //1:select DDC_SCL ; 0:clk_27m
  #define CLK_CFG4_DSCL_CLOCK_ON_OFF    (1U << 0)   //1:select dscl in ; 0:pull up
  
#define REG_RW_CLK_CFG5                 0x0084              //Clock Selection Configuration 5 ,8555 add FOR HDMI RX
  #define CLK_CFG5_RX_CH2_CK_IN_SEL_MASK     0x00001C00
  #define CLK_CFG5_RX_CH2_CK_IN_SEL_OFFSET   10  
  #define CLK_CFG5_RX_CH2_CK_SELECT    (1U << 12)   //1:select channel 2 clock ; 0:pclk of rx pll
  #define CLK_CFG5_RX_CH2_CK_MUX       (1U << 11)   //1:from RX PLL ; 0: clk_27m
  #define CLK_CFG5_RX_CH2_CK_ON        (1U << 10)   //1:on
  
  #define CLK_CFG5_RX_CH1_CK_IN_SEL_MASK     0x00000380
  #define CLK_CFG5_RX_CH1_CK_IN_SEL_OFFSET   7
  #define CLK_CFG5_RX_CH1_CK_SELECT    (1U << 9)   //1:select channel 1 clock ; 0:pclk of rx pll
  #define CLK_CFG5_RX_CH1_CK_MUX       (1U << 8)   //1:from RX PLL ; 0: clk_27m
  #define CLK_CFG5_RX_CH1_CK_ON        (1U << 7)   //1:on
  
  #define CLK_CFG5_RX_CH0_CK_IN_SEL_MASK     0x00000070
  #define CLK_CFG5_RX_CH0_CK_IN_SEL_OFFSET   4
  #define CLK_CFG5_RX_CH0_CK_SELECT    (1U << 6)   //1:select channel 0 clock ; 0:pclk of rx pll
  #define CLK_CFG5_RX_CH0_CK_MUX       (1U << 5)   //1:from RX PLL ; 0: clk_27m
  #define CLK_CFG5_RX_CH0_CK_ON        (1U << 4)   //1:on
  #define CLK_CFG5_SRAM_TEST_MODE         (1U << 3)
  #define CLK_CFG5_C_DPCLK_SEL_XCK        (1U << 2)
  #define CLK_CFG5_C_CKDT_XCLK            (1U << 1)
  #define CLK_CFG5_BIST_PCLK_SEL          (1U << 0)
  
#define REG_RW_CLK_CFG6                 0x0088              //Clock Selection Configuration 6 ,8555 add FOR IP_YA_HIER
  #define CLK_CFG6_PDN_ESIF          (1U << 12)
  #define CLK_CFG6_PDN_MUXED_TS1     (1U << 11)
  #define CLK_CFG6_MUXED_TS1_SEL_2   (1U << 10)    //1: clock phase inverte ; 0:normal clock   
  #define CLK_CFG6_PDN_SPDIFIN     (1U << 3)
  #define CLK_CFG6_CLK_SPDIFIN_SEL_MASK     0x00000007
  #define CLK_CFG6_CLK_SPDIFIN_SEL_OFFSET   0

#define REG_RW_CLK27CALI0               0x00A0              //CLK27 Calibration Register 0  //8555 change address
#define REG_RW_CLK27CALI1               0x00A4              //CLK27 Calibration Register 1  //8555 change address
#define REG_RW_CLK27CALI2               0x00A8              //CLK27 Calibration Register 2  //8555 change address
#define REG_RW_CLK27CALI3               0x00AC              //CLK27 Calibration Register 3  //8555 change address
#define REG_RW_CLK27CALI4               0x00B0              //CLK27 Calibration Register 4  //8555 change address
#define REG_RW_CLK27CALI5               0x00B4              //CLK27 Calibration Register 5  //8555 change address
#define REG_RW_CLK27CALI6               0x00B8              //CLK27 Calibration Register 6  //8555 add

#define REG_RW_PAD_CFG_0                0x00C0              //Pad Multifunction Configuration Register 0
    #define SPDATA0_SPDATA                       (1<<15)
    #define SPDATA0_AOSDATA3                     (2<<15)
    #define SPDATA0_AOSDATA4                     (3<<15)
    #define SPDATA0_AOSDATA5                     (4<<15)
    #define SPDATA0_MCIN                         (5<<15)
    #define SPDATA0_MASK                         (7<<15)
    #define LINEIN_SPDATA_SEL_MASK		(0x03<<2)
    #define LINEIN_SPDATA_SEL_VOUTD15		(0x01<<2)
    #define LINEIN_SPDATA_SEL_AOSDATA5		(0x02<<2)
    #define LINEIN_SPDATA_SEL_VOUTCLK1		(0x03<<2)
    #define AUD_SDATA5_SEL_MASK		(0x01<<11)
    
#define REG_RW_PAD_CFG_1                0x00C4        //Pad Multifunction Configuration Register 1
#define REG_RW_PAD_CFG_2                0x00C8        //Pad Multifunction Configuration Register 2
    #define EXT_INT1_PIN_MASK_                   (0xF<<12)
    #define EXT_INT1_PIN_GPIO0_                  (1<<12)
    #define EXT_INT1_PIN_GPIO1_                  (2<<12)
    #define EXT_INT1_PIN_GPIO2_                  (3<<12)
    #define EXT_INT1_PIN_GPIO3_                  (4<<12)
    #define EXT_INT1_PIN_GPIO4_                  (5<<12)
    #define EXT_INT1_PIN_GPIO5_                  (6<<12)
    #define EXT_INT1_PIN_GPIO6_                  (7<<12)
    #define EXT_INT1_PIN_GPIO7_                  (8<<12)
    #define EXT_INT1_PIN_VOUTCLK1_               (9<<12)
    #define EXT_INT1_PIN_SFDI_                   (0xA<<12)
    #define EXT_INT1_PIN_VOUTHSYNC_              (0xB<<12)
    #define EXT_INT1_PIN_VOUTD4_                 (0xC<<12)
    #define EXT_INT1_PIN_VOUTD6_                 (0xD<<12)
    #define EXT_INT1_PIN_VOUTD11_                (0xE<<12)
    #define EXT_INT2_PIN_MASK_                   (0xF<<16)
    #define EXT_INT2_PIN_VOUTD0_                 (1<<16)
    #define EXT_INT2_PIN_VOUTD12_                (2<<16)
    #define EXT_INT2_PIN_AOSDATA3_               (3<<16)
    #define EXT_INT2_PIN_AOSDATA4_               (4<<16)
    #define EXT_INT2_PIN_AOSDATA5_               (5<<16)
    #define EXT_INT2_PIN_SPMCLK_                 (6<<16)
    #define EXT_INT2_PIN_SPBCK_                  (7<<16)
    #define EXT_INT2_PIN_SPLRCK_                 (8<<16)
    #define EXT_INT2_PIN_SPDATA_                 (9<<16)
    #define EXT_INT2_PIN_MCIN_                   (0xA<<16)
    #define EXT_INT2_PIN_VOUTD5_                 (0xB<<16) 
    #define EXT_INT2_PIN_VOUTD7_                 (0xC<<16)
    #define EXT_INT2_PIN_VOUTVSYNC_              (0xD<<16)
    #define EXT_INT2_PIN_GPIO7_                  (0xE<<16)

#define REG_RW_PAD_CFG_3                0x00CC        //Pad Multifunction Configuration Register 3
#define REG_RW_PAD_CFG_4                0x00D0        //Pad Multifunction Configuration Register 4
#define REG_RW_PAD_CFG_5                0x00D4        //Pad Multifunction Configuration Register 5
#define REG_RW_PAD_CFG_6                0x00D8        //Pad Multifunction Configuration Register 6
#define REG_RW_PAD_CFG_7                0x00DC        //Pad Multifunction Configuration Register 7
#define REG_RW_PAD_CFG_8                0x00E0        //Pad Multifunction Configuration Register 8

#define REG_RW_PAD_CTRL_0               0x0100        //Pad PU/PD/SMT/SR/E2/E4 Control Register 0
  #define SPDIF_DV_MASK                          (3<<28)
  #define SPDIF_DV_CUR_0                         (0<<28)
  #define SPDIF_DV_CUR_1                         (1<<28)
  #define SPDIF_DV_CUR_2                         (2<<28)
  #define SPDIF_DV_CUR_3                         (3<<28)
  #define AOMCLK_DRV_CUR_0 			                 (0<<0) // 4mA
  #define AOMCLK_DRV_CUR_1 			                 (1<<0) // 8mA
  #define AOMCLK_DRV_CUR_2 			                 (2<<0) // 12mA
  #define AOMCLK_DRV_CUR_3 			                 (3<<0) // 16mA

#define REG_RW_PAD_CTRL_1               0x0104       //Pad PU/PD/SMT/SR/E2/E4 Control Register 1
#define REG_RW_PAD_CTRL_2               0x0108       //Pad PU/PD/SMT/SR/E2/E4 Control Register 2
#define REG_RW_PAD_CTRL_3               0x010C       //Pad PU/PD/SMT/SR/E2/E4 Control Register 3
#define REG_RW_PAD_CTRL_4               0x0110       //Pad PU/PD/SMT/SR/E2/E4 Control Register 4
#define REG_RW_PAD_CTRL_5               0x0114       //Pad PU/PD/SMT/SR/E2/E4 Control Register 5
#define REG_RW_PAD_CTRL_6               0x0118       //Pad PU/PD/SMT/SR/E2/E4 Control Register 6
#define REG_RW_PAD_CTRL_7               0x011C       //Pad PU/PD/SMT/SR/E2/E4 Control Register 7
#define REG_RW_PAD_CTRL_8               0x0120       //Pad PU/PD/SMT/SR/E2/E4 Control Register 8
#define REG_RW_PAD_CTRL_9               0x0124       //Pad PU/PD/SMT/SR/E2/E4 Control Register 9
#define REG_RW_PAD_CTRL_10              0x0128       //Pad PU/PD/SMT/SR/E2/E4 Control Register 10
#define REG_RW_PAD_CTRL_11              0x012C       //Pad PU/PD/SMT/SR/E2/E4 Control Register 11
#define REG_RW_PAD_CTRL_12              0x0130       //Pad PU/PD/SMT/SR/E2/E4 Control Register 12
#define REG_RW_PAD_CTRL_13              0x0134       //Pad PU/PD/SMT/SR/E2/E4 Control Register 13
#define REG_RW_PAD_CTRL_14              0x0138       //Pad PU/PD/SMT/SR/E2/E4 Control Register 14
#define REG_RW_PAD_CTRL_15              0x013C       //Pad PU/PD/SMT/SR/E2/E4 Control Register 15
#define REG_RW_PAD_CTRL_16              0x0140       //Pad PU/PD/SMT/SR/E2/E4 Control Register 16
#define REG_RW_PAD_CTRL_17              0x0144       //Pad PU/PD/SMT/SR/E2/E4 Control Register 17
                                        
#define REG_RW_GPIO_EN0                 0x0180       //GPIO Output Enable Control Register 0
#define REG_RW_GPIO_EN1                 0x0184       //GPIO Output Enable Control Register 1
#define REG_RW_GPIO_EN2                 0x0188       //GPIO Output Enable Control Register 2
#define REG_RW_GPIO_EN3                 0x018C       //GPIO Output Enable Control Register 3
                                        
#define REG_RW_GPIO_OUT0                0x01A0       //GPIO Output Register 0
#define REG_RW_GPIO_OUT1                0x01A4       //GPIO Output Register 1
#define REG_RW_GPIO_OUT2                0x01A8       //GPIO Output Register 2
#define REG_RW_GPIO_OUT3                0x01AC       //GPIO Output Register 3
                                        
#define REG_RW_GPIO_IN0                 0x01C0       //GPIO Input Register 0
#define REG_RW_GPIO_IN1                 0x01C4       //GPIO Input Register 1
#define REG_RW_GPIO_IN2                 0x01C8       //GPIO Input Register 2
#define REG_RW_GPIO_IN3                 0x01CC       //GPIO Input Register 3

#define REG_RW_MON_REG                  0x0200       //Monitor Bus Register 0

#define REG_RW_PAD_FECTL_0              0x0280       // FE Pad Control Selection 0
#define REG_RW_PAD_FECTL_1              0x0284       // FE Pad Control Selection 1
#define REG_RW_PAD_PWMCTL_0             0x0288       // PWM Pad Control Selection 0

#define REG_RW_PAD_DUTY_0               0x02C0       // Pad TDSEL/RDSEL Control 0
#define REG_RW_PAD_DUTY_1               0x02C4       // Pad TDSEL/RDSEL Control 1
#define REG_RW_PAD_DUTY_2               0x02C8       // Pad TDSEL/RDSEL Control 2
#define REG_RW_PAD_DUTY_3               0x02CC       // Pad TDSEL/RDSEL Control 3
#define REG_RW_PAD_DUTY_4               0x02D0       // Pad TDSEL/RDSEL Control 4
#define REG_RW_PAD_DUTY_5               0x02D4       // Pad TDSEL/RDSEL Control 5
#define REG_RW_PAD_DUTY_6               0x02D8       // Pad TDSEL/RDSEL Control 6
#define REG_RW_PAD_DUTY_7               0x02DC       // Pad TDSEL/RDSEL Control 7
#define REG_RW_PAD_DUTY_8               0x02E0       // Pad TDSEL/RDSEL Control 8
#define REG_RW_PAD_DUTY_9               0x02E4       // Pad TDSEL/RDSEL Control 9
#define REG_RW_PAD_DUTY_10              0x02E8       // Pad TDSEL/RDSEL Control 10

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Type definitions
//============================================================================
typedef enum
{
    SRC_CK_APLL,
    SRC_CK_ARMPLL,
    SRC_CK_VDPLL,
    SRC_CK_DMPLL,
    SRC_CK_SYSPLL1,
    SRC_CK_SYSPLL2,
    SRC_CK_USBCK,
    SRC_CK_MEMPLL
} SRC_CK_T;

typedef enum
{
	  e_CLK_RISC,	             //0  
    e_CLK_ADSP,	             //1  
    e_CLK_ADSP2,	           //2  
    e_CLK_FLASH,	           //3  
    e_CLK_NFLASH,	           //4  
    e_CLK_SACD,              //5  
    e_CLK_RISC2,	           //6  
    e_CLK_SLOW_RISC,         //7  
    e_CLK_VDEC,	             //8  
    e_CLK_MC,	               //9  
    e_CLK_OSD,	             //10 
    e_CLK_PNG,	             //11 
    e_CLK_RESZ,	             //12 
    e_CLK_GRAPH,	           //13 
    e_CLK_NR,	               //14 
    e_CLK_DEMUX,             //15 
    e_CLK_IR_DIV,	           //16 
    e_CLK_SD,	               //17 
    e_CLK_MS,	               //18 
    e_CLK_ABIST2,	           //19 
    e_CLK_ABIST,	           //20 
    e_CLK_SVO_STDBY,         //21 
    e_CLK_GCPU,              //22 
    e_CLK_MVDO2,             //23 
    e_CLK_TSOUT,             //24 
    e_CLK_SPI,               //25 
    e_CLK_SIF,               //26 
    e_CLK_VENC_2F,           //27 
    e_CLK_BCLK,              //28 
    e_CLK_SPDIFIN,           //29 
    e_CLK_MUXED_TS0,         //30 
    e_CLK_MUXED_TS1,         //31 
    e_CLK_ESIF,              //32 
    e_CLK_MAX                //33  

} e_CLK_T;

//============================================================================
// Interface
//============================================================================
extern BOOL BSP_Calibrate(SRC_CK_T eSource, UINT32 u4Clock);
extern UINT32 BSP_GetClock(SRC_CK_T eSource);
extern BOOL CKGEN_SetPLL(SRC_CK_T eSource, UINT32 u4Clock0, UINT32 u4Clock1);
extern BOOL CKGEN_AgtOnClk(e_CLK_T eAgt);
extern BOOL CKGEN_AgtOffClk(e_CLK_T eAgt);
extern BOOL CKGEN_AgtSelClk(e_CLK_T eAgt, UINT32 u4Sel);
extern UINT32 CKGEN_AgtGetClk(e_CLK_T eAgt);
#endif  // X_CKGEN_8555_H

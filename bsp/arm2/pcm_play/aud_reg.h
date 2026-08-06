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


#ifndef _AUD_REG_H
#define _AUD_REG_H

#include "x_bim.h"
#include "base_regs.h"
#include "x_ckgen.h"

    
#ifdef __cplusplus
    extern "C"
    {
#endif

/**********************************************
*
* Register RW
*
**********************************************/
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
#define IO_BASE_VA                              0xFD000000
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
#define IO_BASE_VA                              0x1D000000
#endif

#define AUDREG_WRITE(addr, val)                (*((volatile UINT32*)(IO_BASE_VA + addr)) = (val))
#define AUDREG_READ(addr)                      (*((volatile UINT32*)(IO_BASE_VA + addr)))

#define AUDREG_MASK(start, bitNum)             (((1 << (bitNum)) - 1) << (start))

#define AUDREG_BITS_VAL(val, start, num)                           \
    ((val & AUDREG_MASK(start, num)) >> (start))

#define AUDREG_BITS_W(addr, start, bitNum, val)                \
    AUDREG_WRITE(addr, (AUDREG_READ(addr) & ~(AUDREG_MASK(start, bitNum))) | ((val) << start))

#define AUDREG_BITS_R(addr, start, bitNum)                      \
    ((AUDREG_READ(addr) & AUDREG_MASK(start, bitNum)) >> start)

#define AUD_CKGEN_SETBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define AUD_CKGEN_CLRBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))


//=============================================================================================//

/**********************************************
*
* CKGEN BASE Register 
*
**********************************************/

#define AUD_REG_CKGEN_BASE                  (0x0000)

#define AUD_REG_AP_REG3                     (AUD_REG_CKGEN_BASE + 0x0018)
        
    #define BIT_STR_AUD2_AP_SEL             0 //(0 - 1)
    #define BIT_NUM_AUD2_AP_SEL             2 //aud2 clock select
        #define AUD2_AP_ACLK_K4     1

#define AUD_REG_AP_REG4                     (AUD_REG_CKGEN_BASE + 0x001C)

    #define BIT_STR_CFG_REG_K4              24 //(24 - 31)
    #define BIT_NUM_CFG_REG_K4              8  //audio k4 clock dirvider ratio

#define AUD_REG_AP_REG7                     (AUD_REG_CKGEN_BASE + 0x0028)

    #define BIT_STR_SEL_APLL_K4             22 //(22 - 23)
    #define BIT_NUM_SEL_APLL_K4             2 //select apll1 or apll2 as k4 divider source
        #define CKGEN_APPL1         0
        #define CKGEN_APLL2         1

#define AUD_REG_PAD_MUX1                    (AUD_REG_CKGEN_BASE + 0x0058)
           
#define AUD_REG_PAD_MUX2                    (AUD_REG_CKGEN_BASE + 0x005C)
    #define BIT_STR_AMUTE_R_SEL             31//(31)
    #define BIT_NUM_AMUTE_R_SEL             1 //amute rear pad select
    
    #define BIT_STR_SPDIF_SEL               30//(30)
    #define BIT_NUM_SPDIF_SEL               1 //spdif pad select
    
#define AUD_REG_PAD_MUX3                    (AUD_REG_CKGEN_BASE + 0x0060)
    #define BIT_STR_I2S_OUT0_SEL            8 //(8 - 9)
    #define BIT_NUM_I2S_OUT0_SEL            2 //i2s out pad select
    
    #define BIT_STR_AMUTE_F_SEL             0 //(0)
    #define BIT_NUM_AMUTE_F_SEL             1 //amute front pad select
    
#define AUD_REG_PAD_MUX4                    (AUD_REG_CKGEN_BASE + 0x0064)
    
#define AUD_REG_PAD_MUX6                    (AUD_REG_CKGEN_BASE + 0x006C)
    #define BIT_STR_I2S_OUT1_SEL            3 //(3 - 4)
    #define BIT_NUM_I2S_OUT1_SEL            2 //i2s out1 pad select


/**********************************************
*
* PWM DAC BASE Register 
*
**********************************************/

#define AUD_REG_PWM_DAC_BASE                (0x600)
    
#define BIT_NUM_PWM_2CH_CTL                 2
        
#define AUD_REG_PWM_CFG0                    (AUD_REG_PWM_DAC_BASE + 0x28)
    #define BIT_STR_HB_CLK_294M_SEL         16 //(16 - 23)
    #define BIT_NUM_HB_CLK_294M_SEL         8  //pwm clock select
        #define PWM_294M    1
        #define PWM_270M    0
        
#define AUD_REG_PWM_CFG1                    (AUD_REG_PWM_DAC_BASE + 0x2C)
    #define BIT_STR_HB_PS_CLK_294M_SEL      16 //(16 - 23)
    #define BIT_NUM_HB_PS_CLK_294M_SEL      8  //Phase-Shift PWM Clock Select
        
#define AUD_REG_PWM_CFG2                    (AUD_REG_PWM_DAC_BASE + 0x30)
    #define BIT_STR_HB_ENVO_CH(idx)         (17 + ((idx) << 2)) //(17, 21, 25, 29)
    #define BIT_NUM_HB_ENVO_CH              1  //Activate Amplifier Mode of channel(X) amplifiter
        #define PWM_ENVO_OFF        0
        #define PWM_ENVO_ON         1
        
    #define BIT_STR_HB_DFC_CH               11 //(11 - 14)   ->default : 0
    #define BIT_NUM_HB_DFC_CH               4  //Enable damping factor control option for channel(X) amplifier
        
#define AUD_REG_PWM_CFG3                    (AUD_REG_PWM_DAC_BASE + 0x34)
    #define BIT_STR_HB_ENPWRDET             21 //(21)        -> default : 0
    #define BIT_NUM_HB_ENPWRDET             1  //Power supply detecton for dc/ac off
            
#define AUD_REG_PWM_CFG5                    (AUD_REG_PWM_DAC_BASE + 0x3C)
    #define BIT_STR_HB_REV0                 16 //(16 - 31)   ->default : 0
    #define BIT_NUM_HB_REV0                 16 //
        
    #define BIT_STR_GPIO_PWM_EN             8  //(8 - 15)
    #define BIT_NUM_GPIO_PWM_EN             8  //GPIO enable
        #define PWM_GPI_EN          0
        #define PWM_GPO_EN          1
        
#define AUD_REG_PWM_CFG7                    (AUD_REG_PWM_DAC_BASE + 0x44)
    #define BIT_STR_GPIO_PWM_G              16 //(16 - 23)
    #define BIT_NUM_GPIO_PWM_G              8  //GPIO analog/gpio function control
        #define PWM_ANALOG_FUNCTON  0
        #define PWM_GPIO_FUNCTION   1
        
#define AUD_REG_PWM_CFG8                    (AUD_REG_PWM_DAC_BASE + 0x48)
    #define BIT_STR_AUD_PWMDAC_REV0         16 //(16 - 31)    -> default : 0
    #define BIT_NUM_AUD_PWMDAC_REV0         16 //
        
    #define BIT_STR_AUD_PWMDAC_REV1         0  //(0 - 15)     -> default : 0
    #define BIT_NUM_AUD_PWMDAC_REV1         16 //
        
#define AUD_REG_PWM_CFG9                    (AUD_REG_PWM_DAC_BASE + 0x4C)
    #define BIT_STR_ADAC_VCM_EN             7  //(7)
    #define BIT_NUM_ADAC_VCM_EN             1  //
        
#define AUD_REG_PWM_CFG10                   (AUD_REG_PWM_DAC_BASE + 0x50)
    #define BIT_STR_HB_HIZ                  24 //(24 - 31)   -> gpio: 1 , analog: 0
    #define BIT_NUM_HB_HIZ                  8  //Enable high impedance mode


/**********************************************
*
* PWMIP BASE 1 Register 
*
**********************************************/

#define AUD_REG_PWMIP_BASE              (0xF00)
    
#define AUD_REG_PWMIP_PGCTRL0           (AUD_REG_PWMIP_BASE + 0x01)
    
#define AUD_REG_PWMIP_PGCTRL1           (AUD_REG_PWMIP_BASE + 0x02)
    
#define AUD_REG_PWMIP_POE               (AUD_REG_PWMIP_BASE + 0x04)
    
#define AUD_REG_PWMIP_PIC               (AUD_REG_PWMIP_BASE + 0x05)
    
#define AUD_REG_PWMIP_PCADDR            (AUD_REG_PWMIP_BASE + 0x08)
    
#define AUD_REG_PWMIP_PCDATA            (AUD_REG_PWMIP_BASE + 0x09)
    
#define AUD_REG_PWMIP_P0PIN             (AUD_REG_PWMIP_BASE + 0x0A)
    
#define AUD_REG_PWMIP_P1PIN             (AUD_REG_PWMIP_BASE + 0x0B)
    
#define AUD_REG_PWMIP_P2PIN             (AUD_REG_PWMIP_BASE + 0x0C)
    
#define AUD_REG_PWMIP_P3PIN             (AUD_REG_PWMIP_BASE + 0x0D)
         

/**********************************************
*
* ENV BASE 1 Register 
*
**********************************************/

#define AUD_REG_ENV_BASE                    (0x5000)

#define REGENV_RWD_BLK67                    (AUD_REG_ENV_BASE + 0x064)

    #define BIT_STR_DRAM_SBLK7              16  //(16 - 31)
    #define BIT_NUM_DRAM_SBLK7              16 //

#define REGENV_AOUT_CFG1                    (AUD_REG_ENV_BASE + 0x0C8)

    #define BIT_NUM_AOSDATA                 2 //
    #define BIT_STR_AOSDATA(idx)            (22 + BIT_NUM_AOSDATA * idx) //      

#define REGENV_MISC_CTRL                    (AUD_REG_ENV_BASE + 0x0CC)
    
    #define BIT_STR_AOUT2_A2BCKX            12//(12 - 15)
    #define BIT_NUM_AOUT2_A2BCKX            4 //
    
    #define BIT_STR_AOUT2_LRCK_CYC          17//(17 - 18)
    #define BIT_NUM_AOUT2_LRCK_CYC          2 //
    
    #define BIT_STR_AOUT2_DELAY             19//(19)
    #define BIT_NUM_AOUT2_DELAY             1 //
    
    #define BIT_STR_AOUT2_LEFT              20//(20)
    #define BIT_NUM_AOUT2_LEFT              1 //
    
    #define BIT_STR_INV_BCK2                21//(21)
    #define BIT_NUM_INV_BCK2                1 //
    
    #define BIT_STR_INV_LRCK2               22//(22)
    #define BIT_NUM_INV_LRCK2               1 //
    
    #define BIT_STR_AOUT2_DA_BNUM           23//(23 - 28)
    #define BIT_NUM_AOUT2_DA_BNUM           6 //
   
#define REGENV_PWMIP_S_DAC_CH_CFG           (AUD_REG_ENV_BASE + 0x260)

    #define BIT_NUM_PWM2_CH_CFG             4   
    #define BIT_STR_PWM2_CH_CFG(idx)        ((idx) * BIT_NUM_PWM2_CH_CFG) //(ch12, ch34, ch56, ch910)

#define REGENV_AUD_DRAM_BANK                (AUD_REG_ENV_BASE + 0x3A0)

    #define BIT_STR_ADSP_BANK               0 //(0 - 10)
    #define BIT_NUM_ADSP_BANK               11//
  

/**********************************************
*
* ENV BASE 2 Register 
*
**********************************************/

#define AUD_REG_ENV_BASE2                   (0xA8000)    

#define REGENV_PWMTOP_CFG                   (AUD_REG_ENV_BASE2 + 0x000)

    #define BIT_STR_PWM_FRNT_APLL_MUX       0 //(0)
    #define BIT_NUM_PWM_FRNT_APLL_MUX       1 //
        #define PWM_APLL1               0
        #define PWM_APLL2               1

    #define BIT_STR_PWM_REAR_APLL_MUX       4 //(4)
    #define BIT_NUM_PWM_REAR_APLL_MUX       1 //

    #define BIT_STR_FRNT_PWM_IN_SEL         8 //(8 - 9)
    #define BIT_NUM_FRNT_PWM_IN_SEL         2 //pwm frnt in selection
        #define PWM_SEL_AOUT1           0
        #define PWM_SEL_AOUT2           1
        #define PWM_SEL_DVP             2
        #define PWM_SEL_DVP2            3
    
    #define BIT_STR_REAR_PWM_IN_SEL         10//(10 - 11)
    #define BIT_NUM_REAR_PWM_IN_SEL         2 //pwm rear in selection

#define REGENV_PWMCFG0_PG1                  (AUD_REG_ENV_BASE2 + 0x00C)
#define REGENV_PWMCFG0_PG2                  (AUD_REG_ENV_BASE2 + 0x010)
#define REGENV_PWMCFG0_PG3                  (AUD_REG_ENV_BASE2 + 0x014)
#define REGENV_PWMCFG0_PG4                  (AUD_REG_ENV_BASE2 + 0x018)

#define REGENV_PWMIP_MISC1                  (AUD_REG_ENV_BASE2 + 0x038)

#define REGENV_PWMIP_NDRECT_ADDR(idx)       (AUD_REG_ENV_BASE2 + 0x0240 + ((idx) << 3)) 
#define REGENV_PWMIP_NDRECT_DATA(idx)       (AUD_REG_ENV_BASE2 + 0x0244 + ((idx) << 3)) 

//-----------------------------------------------------------------------------//

#define AUD_REG_RGBK2_CFG1                  (AUD_REG_ENV_BASE2 + 0x84)

#define REGENV_RGBK2_CFG5                   (AUD_REG_ENV_BASE2 + 0x094)
    
    #define BIT_STR_AO2_INT_CLR             1 //(1)
    #define BIT_NUM_AO2_INT_CLR             1 //audout2 interrupt clear 
     
#define REGENV_RGBK2_AOUT_CFG2              (AUD_REG_ENV_BASE2 + 0x0C8)  
    
    #define BIT_STR_AP_AOUT2_ARM_CTRL_CFG   8 //(8 - 15)
    #define BIT_NUM_AP_AOUT2_ARM_CTRL_CFG   8 //EQ 'h36 enable arm contorl
        
    #define BIT_STR_FRNT_SRC_SEL            16 //(16 - 17)
    #define BIT_NUM_FRNT_SRC_SEL            2 //frnt sout select
        
    #define BIT_STR_REAR_SRC_SEL            18 //(18 - 19)
    #define BIT_NUM_REAR_SRC_SEL            2 //rear sout select
       
//-------------------------------------------------------------------------------------//     

#define REGENV_AOUT2_CH1_BUF_SADR           (AUD_REG_ENV_BASE2 + 0x0500)

    #define BIT_STR_AOUT2_CH1_BUF_SADR      0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH1_BUF_SADR      20//channel 1 buffer start address
        
#define REGENV_AOUT2_CH1_BUF_SIZE           (AUD_REG_ENV_BASE2 + 0x0504) 

    #define BIT_STR_AOUT2_CH1_BUF_SIZE      0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH1_BUF_SIZE      20//channel 1 buffer size register
       
#define REGENV_AOUT2_CH_NSADR(idx)          (AUD_REG_ENV_BASE2 + 0x0508 + ((idx) << 2))  // idx: 0 - 11

    #define BIT_STR_AOUT2_CH_NSADR          0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH_NSADR          20//set the next start address for channeL
        
#define REGENV_AOUT2_NSNUM                  (AUD_REG_ENV_BASE2 + 0x0538)

    #define BIT_STR_AOUT2_NSNUM             8 //(15 - 31)
    #define BIT_NUM_AOUT2_NSNUM             16//next audio output sample number per channel
        
#define REGENV_AOUT2_INTRSIZE               (AUD_REG_ENV_BASE2 + 0x053C)

    #define BIT_STR_AOUT2_INTRSIZE          8 //(15 - 31)
    #define BIT_NUM_AOUT2_INTRSIZE          16//generating interrupt when how many samples remained to be send out
            
#define REGENV_AOUT2_CTRL                   (AUD_REG_ENV_BASE2 + 0x0540)

    #define BIT_STR_AOUT2_EN_PRE            8 //(8)
    #define BIT_NUM_AOUT2_EN_PRE            1 //
    
#define REGENV_AOUT2_CH_CFG(idx)            (AUD_REG_ENV_BASE2 + 0x0544 + ((idx) << 4))  // idx: 0-2
       
#define REGENV_AOUT2_CH_NUM                 (AUD_REG_ENV_BASE2 + 0x0550)

    #define BIT_STR_AOUT2_CH_NUM            8 //(8 - 11)
    #define BIT_NUM_AOUT2_CH_NUM            2 //       

        

//==================================================================================//


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_REG_H


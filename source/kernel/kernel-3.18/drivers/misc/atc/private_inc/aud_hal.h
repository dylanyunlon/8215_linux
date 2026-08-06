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
#ifndef _AUD_HAL_H_
#define _AUD_HAL_H_
#include "x_hal_ic.h"
#include "drv_config.h"
#if CONFIG_DRV_AUD_AC83XX
#include <mach/base_regs.h>
#else
#include "hardware.h"
#endif


// Audio Output Register Configuration
#define AUD_AOUTCFG                     0xC0
#define AOUT1_LRCK_CYC16                (0 << 0)        // 16 cycles
#define AOUT1_LRCK_CYC24                (1 << 0)        // 24 cycles
#define AOUT1_LRCK_CYC32                (2 << 0)        // 32 cycles
#define LRCK_PAD_OUT_CH1112             (1 << 2)        // LRCK pad output ch11, 12 signal  
#define HDMI_OUT_PHASE_MODULATION       ( 1<< 3)        // HDMI audio output phase modulation
#define AOUT1_FMT_RJ                    (0 << 4)        // Right aligned with LRCK
#define AOUT1_FMT_LJ                    (2 << 4)        // Left aligned with LRCK
#define AOUT1_FMT_I2S                   (3 << 4)        // I2S interface
#define AOUT1_INV_BCK                   (1 << 6)        // Invert bit clock
#define AOUT1_INV_LRCK                  (1 << 7)        // Invert LRCK
#define AOUT1_DAC_16BIT                 (0x10 << 8)     // Audio DAC 16-Bit
#define AOUT1_DAC_18BIT                 (0x12 << 8)     // Audio DAC 18-Bit
#define AOUT1_DAC_20BIT                 (0x14 << 8)     // Audio DAC 20-Bit
#define AOUT1_DAC_24BIT                 (0x18 << 8)     // Audio DAC 24-Bit
#define DAC_BNUM_MASK                   (0x3F << 8)     // Audio DAC bit number
#define AOUT1_INV_SD                    (1 << 14)       // Invert the Sdata output
#define HDMI_DSD_OUTPUT                 (1 << 15)       // Audio output 2 output DSD data
#define A2BCKX_MASK                     (0x0F << 16)    // Times of ACK/2 to BCK
#define AOUT1_A2BCKX_REG_BIT_POS        16              // Bit position for Times of ACK/2 to BCK
#define AOUT1_ACK_EXSEL                 (1 << 20)       // External ACK select
#define IEC2_TO_IEC                     (1 << 21)       //IEC958 Configuration
#define IEC_TO_IEC2                     (1 << 22)
#define IEC2_NO_SRC                     (0 << 23)       // IEC2 no down sample
#define IEC2_HALF_SRC                   (1 << 23)       // IEC2 1/2 down sample
#define IEC2_QUARTER_SRC                (3 << 23)       // IEC2 1/4 down sample
#define HDMI_DSD_CTRL_BY_LRCK           (1 << 25)       // HDMI DSD output L/R  channel output by LRCK
#define IEC2_SWP                        (1 << 26)       // Swap IEC2 raw data output high & low bytes
#define IEC_SWP                         (1 << 27)       // Swap IEC raw data output high & low bytes
#define IEC_SSLEW                       (1 << 28)       // IEC958 output slow slew rate
#define IEC_CUR_4                       (1 << 29)       // IEC958 driving current 4mA
#define HDMI_DSD_CTRL_BY_BCK            ((UINT32)1 << 31)        // HDMI DSD output L/R  channel output by BCK

// Audio Input Configuration
#define AUD_AINCFG                      0xC4  
// Micro Phone ADC Interface Configuration
#define MP_BNUM_MASK                    (0x1F << 0)      // (numbers of bits - 1) for microphone ADC
#define MP_16BIT                        (0x0F << 0)      // 16 bits
#define MP_18BIT                        (0x11 << 0)      // 18 bits
#define MP_20BIT                        (0x13 << 0)      // 20 bits
#define MP_24BIT                        (0x17 << 0)      // 24 bits
#define MP_FMT_RJ                       (0 << 5)         // Right aligned with LRCK
#define MP_FMT_LJ                       (1 << 5)         // Left aligned with LRCK
#define MP_FMT_I2S                      (3 << 5)         // I2S interface
#define MP_LRCK_INV                     (1 << 7)         // Invert LRCK for microphone
// SPDIF/line in configuration
#define SPDIF_BNUM_MASK_POS             8
#define SPLIN_BNUM_MASK                 (0x1F << 8)       // (number of bits - 1) for line in ADC or SPDIF
#define SPLIN_16BIT                     (0x0F << 8)     // 16 bits
#define SPLIN_18BIT                     (0x11 << 8)     // 18 bits
#define SPLIN_20BIT                     (0x13 << 8)     // 20 bits
#define SPLIN_24BIT                     (0x17 << 8)     // 24 bits
#define SPLIN_FORMAT_MASK_POS           13
#define SPLIN_FORMAT_MASK               (3 << 13)
#define SPLIN_RJ                        (0 << 13)          // Right aligned with LRCK
#define SPLIN_LJ                        (1 << 13)          // Left aligned with LRCK
#define SPLIN_I2S                       (3 << 13)          // I2S interface
#define SPLIN_INV                       (1 << 15)          // SPDIF/line in LRCK is inverted
#define SPDIF_CYC_MASK_POS              16
#define SPDIF_CYC_MASK                  (3 << 16)
#define SPDIF_CYC16                     (0 << 16)          // 16 cycles
#define SPDIF_CYC24                     (1 << 16)          // 24 cycles
#define SPDIF_CYC32                     (2 << 16)          // 32 cycles
#define MPBCK_INV	                    (1 << 18)   //MP inverse
#define MPCLK_IND	                    (1 << 19)   //Microphone use different sampling rate
#define SPDIF_IN                        (1 << 20)          // The interface is spdif in, set to 0 is line in
#define AOUT1_BCK_EXSEL                  (1 << 21)          // External bck select, default is 0: internal  
#define AOUT1_LRCK_EXSEL                 (1 << 21)          // External LRCK select, default is 0: internal  
//Audio Pad control register
#define SDATA_SSLEW                     (1 << 23)        // output slow slew rate for sdata
#define SDATA_CUR_4                     (1 << 24)        // Sdata driving current 4mA
#define BCK_SSLEW                       (1 << 26)        // output slow slew rate for sdata
#define BCK_CUR_4                       (1 << 27)        // Sdata driving current 4mA
#define ACK_SSLEW                       (1 << 29)        // output slow slew rate for sdata
#define ACK_CUR_4                       (1 << 30)        // Sdata driving current 4mA

// Audio output config1 
#define AUD_AOUT1CFG                    0xC8          
#define AO_PHASE_MOD                    (1 << 0)         //audio output phase modulation
#define DSD_OUTPUT                      (1 << 1)         //SACD DSD output
#define DSD_CTRL_BY_LRCK                (1 << 2)         //DSD output L/R by LRCK
#define DSD_CTRL_BY_BCK                 (1 << 3)         //DSD output L/R by BCK
#define SDATA0_CFG_MASK                 (0x07 << 4)        //Sdata0 configuration 
#define SDATA1_CFG_MASK                 (0x07 << 7)        //Sdata1 configuration 
#define SDATA2_CFG_MASK                 (0x07 << 10)    //Sdata2 configuration 
#define SDATA3_CFG_MASK                 (0x07 << 13)    //Sdata3 configuration 
#define SDATA4_CFG_MASK                 (0x07 << 16)    //Sdata4 configuration 
#define SDATA5_CFG_MASK                 (0x07 << 19)    //Sdata5 configuration 
#define SDATA6_CFG_MASK                 (0x07 << 22)    //Sdata6 configuration(HDMI) 
#define SDATA7_CFG_MASK                 (0x07 << 24)    //Sdata7 configuration(HDMI) 
#define SDATA8_CFG_MASK                 (0x07 << 26)    //Sdata8 configuration(HDMI)
#define SDATA9_CFG_MASK                 ((UINT32)0x07 << 28)    //Sdata9 configuration(HDMI)
#define Ch1Ch2_OUT                      0 
#define Ch3Ch4_OUT                      1 
#define Ch5Ch6_OUT                      2 
#define Ch7Ch8_OUT                      3 
#define Ch9Ch10_OUT                     4 
#define Ch11Ch12_OUT                    5 
#define Ch_OUT_MUTE                     6 
#define SDATA0_MUTE                     (Ch_OUT_MUTE << 4)        //Sdata0 mute 
#define SDATA1_MUTE                     (Ch_OUT_MUTE << 7)        //Sdata1 mute 
#define SDATA2_MUTE                     (Ch_OUT_MUTE << 10)    //Sdata2 mute 
#define SDATA3_MUTE                     (Ch_OUT_MUTE << 13)    //Sdata3 mute 
#define SDATA4_MUTE                     (Ch_OUT_MUTE << 16)    //Sdata4 mute 
#define SDATA5_MUTE                     (Ch_OUT_MUTE << 19)    //Sdata5 mute 
#define SDATA0_OUT                      (Ch1Ch2_OUT << 4)        //Sdata0 output on ch1ch2
#define SDATA1_OUT                      (Ch3Ch4_OUT << 7)        //Sdata1 output on ch3ch4 
#define SDATA2_OUT                      (Ch5Ch6_OUT << 10)    //Sdata2 output on ch5ch6 
#define SDATA3_OUT                      (Ch7Ch8_OUT << 13)    //Sdata3 output  on ch7ch8
#define SDATA4_OUT                      (Ch9Ch10_OUT << 16)    //Sdata4 output on ch9ch10 
#define SDATA5_OUT                      (Ch11Ch12_OUT << 19)    //Sdata5 output on ch11ch12 
#define AOUT_TO_AOUT2                   (1 << 30)
#define AOUT2_TO_AOUT                   ((u32)1 << 31)

// Audio output2 config(HDMI)
#define AUD_AOUT2CFG                    0xCC
#define MPHONE_A2BCKX_MASK              0x0F
#define MPHONE_A2BCKX_REG_BIT_POS       0               // Bit position for Times of ACK/2 to BCK
#define MPHONE_LRCK_MASK                (0x03 <<4)    
#define MPHONE_LRCK_CYC16               (0 << 4)        // 16 cycles
#define MPHONE_LRCK_CYC24               (1 << 4)        // 24 cycles
#define MPHONE_LRCK_CYC32               (2 << 4)        // 32 cycles
#define AOUT2_ACK_EXSEL                 (1 << 11)       // HDMI External ACK select
#define AOUT2_A2BCKX_MASK               (0x0F << 12)    // Times of ACK/2 to BCK
#define AOUT2_A2BCKX_REG_BIT_POS        12              // Bit position for Times of ACK/2 to BCK
#define AOUT2_BCK_EXSEL                 (1 << 16)       // HDMI external BCK select
#define AOUT2_LRCK_CYC16                (0 << 17)       // 16 cycles
#define AOUT2_LRCK_CYC24                (1 << 17)       // 24 cycles
#define AOUT2_LRCK_CYC32                (2 << 17)       // 32 cycles
#define AOUT2_FMT_RJ                    (0 << 19)       // Right aligned with LRCK
#define AOUT2_FMT_LJ                    (2 << 19)       // Left aligned with LRCK
#define AOUT2_FMT_I2S                   (3 << 19)       // I2S interface
#define AOUT2_INV_BCK                   (1 << 21)       // Invert bit clock
#define AOUT2_INV_LRCK                  (1 << 22)       // Invert LRCK
#define AOUT2_DAC_BNUM_MASK             (0x3F << 23)    // Audio DAC bit number
#define AOUT2_DAC_16BIT                 (0x10 << 23)    // Audio DAC 16-Bit
#define AOUT2_DAC_18BIT                 (0x12 << 23)    // Audio DAC 18-Bit
#define AOUT2_DAC_20BIT                 (0x14 << 23)    // Audio DAC 20-Bit
#define AOUT2_DAC_24BIT                 (0x18 << 23)    // Audio DAC 24-Bit
#define AOUT2_INV_SD                    (1 << 29)       // Invert the Sdata output
#define AOUT2_LRCK_EXSEL                ((u32)1 << 31)         // HDMI external LRCK select

#define AOUT_TDM_CTRL                   0xD0
#define AOUT_TDM_BCK_DIV_MSK            0x3

//**************************************************************************
// APLL
//**************************************************************************

#define APLL_CAL_CNT                    0x200

#define APLL_CFG0                       0x204
#define APLL_PREDIV_MASK	            (0x03 << 0)
#define APLL_PREDIV_1                   0
#define APLL_PREDIV_2                   1
#define APLL_PREDIV_4                   2
#define APLL_PREDIV_8                   3
#define APLL_CURRENTSEL_MASK            (0x07 << 2)
#define APLL_CURRENTSEL_5UA             (0 << 2)
#define APLL_CURRENTSEL_10UA            (1 << 2)
#define APLL_CURRENTSEL_15UA            (2 << 2)
#define APLL_CURRENTSEL_20UA            (3 << 2)
#define APLL_CURRENTSEL_25UA            (4 << 2)
#define APLL_CURRENTSEL_30UA            (5 << 2)
#define APLL_CURRENTSEL_40UA            (6 << 2)
#define APLL_CURRENTSEL_50UA            (7 << 2)
#define APLL_CP_P_MASK                  (0x07 << 5)
#define APLL_CP_P_30U                   (0 << 5)
#define APLL_CP_P_60U                   (1 << 5)
#define APLL_CP_P_90U                   (2 << 5)
#define APLL_CP_P_120U                  (3 << 5)
#define APLL_CP_P_15U                   (4 << 5)
#define APLL_CP_P_30U_                  (5 << 5)
#define APLL_CP_P_45U                   (6 << 5)
#define APLL_CP_P_60U_                  (7 << 5)
#define APLL_DIV1                       (1 << 8)
#define APLL_RELATCH_EN                 (1 << 9)
#define APLL_VCOBAND                    (1 << 10) //VCOBAND Selection for Manual mode
#define APLL_VCOBAND_DEFAULT            (2 << 10)
#define APLL_PD                         (1 << 14)
#define APLL_DIV56                      (1 << 15)
#define APLL_DIV5_86M                   (0 << 15)
#define APLL_DIV6_72M                   (1 << 15) //default suggested
#define APLL_A2_K2_RESET_B              (1 << 16)    
#define APLL_CLK_A2                     (0 << 17)
#define APLL_APLL_CLK                   (1 << 17)
#define APLL_CKSEL_MASK                 (3 << 18)
#define APLL_REF_SEL_72M                (0 << 18)
#define APLL_REF_SEL_DIN                (1 << 18)
#define APLL_REV_MASK                   (0xFF << 20)
#define APLL_REV                        (0x01 << 20)
#define APLL_RVE_VER_E                  (0xC0 << 20)
#define APLL_RST_B_1                    (1 << 28)
#define APLL_RESET_B_CORE_1             (1 << 29)
#define APLL_POSTDIVSEL_MASK            (1 << 30)
#define APLL_POSTDIVSEL_FRACTIONAL_N    (0 << 30)
#define APLL_POSTDIVSEL_INTERGER        (1 << 30)
#define DSP_CTRL_APLL_MODIN             (1 << 31)  
#define RISC_CTRL_APLL_MODIN            (0 << 31)

#define APLL_CFG1                       0x208
#define APLL_MODDIV_MASK                ((unsigned)0xFF << 0) 
#define  APLL_MODDIV_3                  (2 << 0)                       //divisor=APLL_MODDIV+1
#define  APLL_MODDIV_4                  (3 << 0)                       //divisor=APLL_MODDIV+1
#define APLL_MODIN_MASK                 ((unsigned)0xFFFFFF << 8)   //APLL modulator parameter for fine tuning APLL frequency
#define APLL_FS_48K                     ((u32)0xC49BA << 8)        //  modify
#define APLL_FS_48K_PP1                 ((u32)0xCCFF2 << 8)        // + 0.1 %
#define APLL_FS_48K_NP1                 ((u32)0xBC382 << 8)        // - 0.1 %
#define APLL_FS_48K_P1                  ((u32)0x1187E7 << 8)        // + 1 %
#define APLL_FS_48K_N1                  ((u32)0x70B8C << 8)        // - 1 %
#define APLL_FS_44K                     ((u32)0xE1B08A << 8)        //  modify
#define APLL_FS_44K_PP1                 ((u32)0xE22BDA << 8)        // + 0.1 %
#define APLL_FS_44K_NP1                 ((u32)0xE1353A << 8)        // - 0.1 %
#define APLL_FS_44K_P1                  ((u32)0xE681AB << 8)        // + 1 %
#define APLL_FS_44K_N1                  ((u32)0xDCDF6A << 8)        // - 1 %
#define APLL_FS_44K_VDPLL               (0x47e001 << 8)

#define APLL_CFG2                       0x20C
#define APLL_A2K2_RST_1                 (1 << 0) 
#define APLL_CLK_A2_SEL_1               (1 << 1)         //APLL CLK A2 Sel
#define APLL_TEST_CLK_SEL               (1 << 2) 
#define APLL_VCOCAL_EN                  (1 << 3)  //  NEW
#define APLL_VCOCALSEL_256              (0 << 4)  //  NEW
#define APLL_VCOCALSEL_512              (1 << 4)  //  NEW
#define APLL_VCOCALSEL_1024             (2 << 4)  //  NEW
#define APLL_VCOCALSEL_2048             (3 << 4)  //  NEW
#define APLL_VCOVTSEL_0P8V              (0 << 6)  //  NEW
#define APLL_VCOVTSEL_0P7V              (1 << 6)  //  NEW
#define APLL_VCOVTSEL_0P6V              (2 << 6)  //  NEW
#define APLL_K1_MASK                    (0xFF << 8)     //APLL K1 divider for IEC_CLK
#define APLL_K2_MASK                    (0xFF << 16)    //APLL K2 divider for Aud output 1 CLK
#define APLL_K3_MASK                    (0xFF << 24)    //APLL K3 divider for IEC2_CLK

#define APLL_CFG3                       0x210
#define APLL_K4_MASK                    (0xFF << 0)    // APLL K4 divider for AUD2 output CLK
#define APLL_K5_MASK                    (0xFF << 8)    // APLL K5 divider for IIR CLK
#define APLL_K6_MASK                    (0xFF << 16)    //APLL K2 divider for Aud output 1 CLK
#define APLL_K4_POS                     0
#define APLL_K5_POS                     8
#define APLL_K6_POS                     16

#define AOMCLK_CFG                      0x224
#define AOMCLK_CHG_MASK                 (0x03 << 4) 
#define AOMCLK_FROM_XTAL                (0 << 4)
#define AOMCLK_FROM_APLL                (1 << 4)
#define AOMCLK_FROM_EXT                 (2 << 4)
#define MPHCLK_CHG_MASK                 (0x03 <<20) 
#define MPHCLK_FROM_XTAL                (0 << 20)
#define MPHCLK_FROM_APLL                (1 << 20)
#define MPHCLK_FROM_EXT                 (2 << 20)
#define AOCLKS_PD_MASK                  (0x07 << 0) 
#define AOMCLK_PD_EN                    (1 << 0)
#define AOMCLK_PD_DIS	                (0 << 0)
#define AOBCK_PD_EN                     (1 << 1)
#define AOBCK_PD_DIS                    (0 << 1)
#define AOLRCK_PD_EN                    (1 << 2)
#define AOLRCK_PD_DIS                   (0 << 2)
#define AOMCLK_FROM_SPDIF_IN            (3 << 4)
#define AO2MCLK_CHG_MASK                (0x03 << 12) 
#define AO2MCLK_FROM_XTAL               (0 << 12)
#define AO2MCLK_FROM_APLL               (1 << 12)
#define AO2MCLK_FROM_EXT                (2 << 12)
#define AO2MCLK_FROM_SPDIF_IN           (3 << 12)

#define AUD_SPDIFIN_CFG0	            0x280
#define SPDIFRX_EN	                    (0x01<<0) //SPDIFRX enabled (soft reset)
#define SPDIFRX_DIS	                    (0x00<<0)
#define SPDIFRX_FLIP	                (0x01<<1) //SPDIFRX enabled (soft reset)
#define SPDIFRX_NOFLIP	                (0x00<<1)
#define SPDIFRX_INT_EN	                (0x01<<6) //SPDIFRX interrupt enabled
#define SPDIFRX_INT_DIS	                (0x00<<6) 
#define SPDIFRX_MAX_LEN_32KHz           (0xBE << 16) // 32kHz
#define SPDIFRX_MAX_LEN_48KHz           (0x8C << 16) // 48kHz
#define SPDIFRX_BITCELL_NUM256CYC	    (0x03 << 24) // 256 cycle
#define SPDIFRX_INV_LRCK	            (0x01 << 31) // Inverse LRCK

#define AUD_SPDIFIN_CFG1	            0x284
#define SEL_SPDIFIN_EN	                (0x01<<0) // data
#define SEL_SPDIFIN_CLK_EN	            (0x01<<1) // clock
#define FIFOSTARTPOINT_5	            (0x01<<4)
#define LRCK_POLARITY_CHECK	            (0x01<<7)
#define SEL_BCK_SPDIFIN	                (0x01<<16)
#define PINMUX_SPMCLK	                (0x00<<17)
#define PINMUX_SPBCK	                (0x01<<17)
#define PINMUX_MASK	                    (0x01<<17)
#define PRE_ERR_NON_EN	                (0x01<<20)
#define PRE_ERR_B_EN	                (0x01<<21)
#define PRE_ERR_M_EN	                (0x01<<22)
#define PRE_ERR_W_EN	                (0x01<<23)
#define PRE_ERR_BITCNT_EN	            (0x01<<24)
#define CHAN_STS_EN	                    (0x01<<29) // channel status and emphasis
#define CHECK_STATUS_EN	                (PRE_ERR_NON_EN|PRE_ERR_B_EN|PRE_ERR_M_EN|PRE_ERR_W_EN|PRE_ERR_BITCNT_EN|CHAN_STS_EN)

#define AUD_SPDIFIN_CHSTS1	            0x288
#define CHSTS_PCM	                    (0x00<<1)
#define CHSTS_NONPCM	                (0x01<<1)
#define CHSTS_PRE_EMP	                (0x01<<3)
#define CHSTS_NO_PRE_EMP	            (0x00<<3)

#define AUD_SPDIFIN_DEBUG1	            0x2A0

#define AUD_SPDIFIN_DEBUG2	            0x2A4

#define AUD_SPDIFIN_DEBUG3	            0x2A8
#define PRE_ERR_NON_STS	                (0x01<<0)
#define PRE_ERR_B_STS	                (0x01<<1)
#define PRE_ERR_M_STS	                (0x01<<2)
#define PRE_ERR_W_STS	                (0x01<<3)
#define PRE_ERR_BITCNT_STS	            (0x01<<4)
#define CHAN_STS_CHG	                (0x01<<7)
#define CHECK_STATUS_ERR	            (PRE_ERR_NON_STS|PRE_ERR_B_STS|PRE_ERR_M_STS|PRE_ERR_W_STS|PRE_ERR_BITCNT_STS)

#define AUD_SPDIFIN_DEBUG4	            0x2AC
#define AUD_SPDIFIN_EC	                0x2B0 // error clear
#define PRE_ERR_NON_EC	                (0x01<<0)
#define PRE_ERR_B_EC	                (0x01<<1)
#define PRE_ERR_M_EC	                (0x01<<2)
#define PRE_ERR_W_EC	                (0x01<<3)
#define PRE_ERR_BITCNT_EC	            (0x01<<4)
#define CHAN_STS_EC	                    (0x01<<9)
  
#define AUD_SPDIFIN_BR	                0x2C0
#define BIT_RECOVER_EN	                (0x01<<0) //Bitclk recovery enable bit
#define BIT_CLK_FS_256	                (0x03<<4)
#define BIT_CLK_SUBF128	                (0x07<<8)
#define BIT_CLK_SUBF256	                (0x08<<8)
#define BIT_CLK_SUBF512	                (0x09<<8)

#define AUD_SPDIFIN_BR_DBG1	            0x2C4 //FS Change Information

#define AIN_ACK_CFG_Multi               0x3F0
#define AIN_MUTLI_BNUM_MASK             (0x3FF << 8)      // (numbers of bits - 1) for multiple line in h/w
#define AIN_MUTLI_16BIT                 (0x0F << 8)      // 16 bits
#define AIN_MUTLI_24BIT                 (0x17 << 8)      // 24 bits
#define AIN_MUTLI_FMT_RJ                (0 << 13)           // Right aligned with LRCK 
#define AIN_MUTLI_FMT_LJ                (1 << 13)          // Left aligned with LRCK
#define AIN_MUTLI_FMT_I2S               (3 << 13)          // I2S interface
#define AIN_MUTLI_LRCK_INV              (1 << 15)         // Invert LRCK for multiple line in
#define AIN_MUTLI_LRCK_CYC_16           (0 << 16)         // LRCK selection 16
#define AIN_MUTLI_LRCK_CYC_24           (1 << 16)         // LRCK selection 24
#define AIN_MUTLI_LRCK_CYC_32           (2<< 16)         // LRCK selection 32

#define APLL_K1_POS                     8
#define APLL_K2_POS                     16
#define APLL_K3_POS                     24

#define APLL_K1                         (0x1 << 12) // of 0x24
#define APLL_K2                         (0x1 << 13) // 0x 0x24
#define APLL_K3                         (0x1 << 14) // 0x 0x24
#define APLL_K4                         (0x1 << 15) // 0x 0x24
#define APLL_K5                         (0x1 << 16) // 0x 0x24
#define APLL_K6                         (0x1 << 17) // 0x 0x24
#define APLL_K7                         (0x1 << 18) // 0x 0x24
#define APLL_K8                         (0x1 << 19) // 0x 0x24
#define APLL_K9                         (0x1 << 20) // 0x 0x24
#define APLL_K10                        (0x1 << 21) // 0x 0x24
#define APLL_K11                        (0x1 << 7)  // of 0x28
#define APLL_K12                        (0x1 << 16) // of 0x28
#define APLL_K13                        (0x1 << 0)  // of 0x2C
#define APLL_K14                        (0x1 << 23) // of 0x24
#define APLL_A1                         (0x1 << 16) // of 0x2C
#define APLL_A2                         (0x1 << 31) // 0f 0x18
#define APLL_A3                         (0x1 << 19) // of 0x2C


#define APLL_256FS_6K                   192
#define APLL_256FS_8K                   144
#define APLL_256FS_12K                  96
#define APLL_256FS_16K                  72
#define APLL_256FS_24K                  48
#define APLL_256FS_32K                  36
#define APLL_256FS_36K	                32
#define APLL_256FS_44K                  24
#define APLL_256FS_48K                  24
#define APLL_256FS_64K                  18
#define APLL_256FS_88K                  12
#define APLL_256FS_96K                  12
#define APLL_256FS_192K                 6

#define APLL_384FS_48K                  (APLL_256FS_48K * 2 / 3)

#define APLL_512FS_44K                  (APLL_256FS_44K >> 1)


// *********************************************************************
// Macros
// *********************************************************************
#define AUDIO_CFG_REG_OFFSET            0x5000
#define MULTIN_CFG_REG_OFFSET           0x5300

#define DSP_REG_BASE (IO_UCV_BASE + AUDIO_CFG_REG_OFFSET)

#define vWriteAUD(dAddr, dVal)    *(volatile u32 *)(IO_UCV_BASE + AUDIO_CFG_REG_OFFSET  + (dAddr)) = dVal
#define dReadAUD(dAddr)           *(volatile u32 *)(IO_UCV_BASE + AUDIO_CFG_REG_OFFSET  + (dAddr))
#define vWriteMULTIN(dAddr, dVal)    *(volatile u32 *)(IO_UCV_BASE + MULTIN_CFG_REG_OFFSET  + (dAddr)) = dVal
#define dReadMULTIN(dAddr)           *(volatile u32 *)(IO_UCV_BASE + MULTIN_CFG_REG_OFFSET  + (dAddr))

#define vWriteAUDMsk(dAddr, dVal, dMsk) vWriteAUD((dAddr), (dReadAUD(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define SetBitAUD(Reg, Bit)      vWriteAUD(Reg, dReadAUD(Reg) |   (Bit) )
#define ClrBitAUD(Reg, Bit)      vWriteAUD(Reg, dReadAUD(Reg) & (~(Bit)))

#endif

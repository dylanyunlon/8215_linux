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


/**
 * @file aud_reg_env.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_ENV_H
#define _AUD_REG_ENV_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif



#define AUD_REG_ENV_BASE        (0x5000)


//Audio Configuration
#define REGENV_AUDIO_CFG        (AUD_REG_ENV_BASE + 0x014)
    #define BIT_STR_IEC2_OUT_SEL    0 //(0 - 2)
    #define BIT_NUM_IEC2_OUT_SEL    3 //IEC2 output channel selection

    #define BIT_STR_IEC2_MUTE       3 //(3)
    #define BIT_NUM_IEC2_MUTE       1 //Writing 1 to this bit will tie the IEC2 output pin to ground

    #define BIT_STR_IEC2_INV        4 //(4)
    #define BIT_NUM_IEC2_INV        1 //IEC2 clock inverse control register

    #define BIT_STR_CLR_BS          5 //(5)
    #define BIT_NUM_CLR_BS          1 //Clear get bit stream count control.

    #define BIT_STR_IEC_BNUM        6 //(6 - 7)
    #define BIT_NUM_IEC_BNUM        2 //Number of bits for IEC60958 output cooked PCM data
        #define REGIEC_24BIT    0
        #define REGIEC_20BIT    2
        #define REGIEC_16BIT    3

    #define BIT_STR_IEC_OUT_SEL     8 //(8 - 10)
    #define BIT_NUM_IEC_OUT_SEL     3 // IEC output channel selection
        #define REG_IEC_CHLR    0
        #define REG_IEC_CHLSRS  1
        #define REG_IEC_CHCLFE  2
        #define REG_IEC_CH78    3
        #define REG_IEC_CHSPLIN 4
        #define REG_IEC_CH910   5
        #define REG_IEC_CH1112  6
    #define BIT_STR_IEC_HB          11 //(11)
    #define BIT_NUM_IEC_HB          1  //IEC output high bit rate control register

    #define BIT_STR_IEC_MUTE        12 //(12)
    #define BIT_NUM_IEC_MUTE        1  //Writing 1 to this bit will tie the IEC output pin to ground

    #define BIT_STR_IEC_DS          13 //(13 - 14)
    #define BIT_NUM_IEC_DS          2  //IEC output down sample
        #define REGIEC_DS_NONE  0//Without down sample
        #define REGIEC_DS_2     1//1/2 down sample
        #define REGIEC_DS_4     1//1/4 down sample

    #define BIT_STR_IEC_CK_INV      15 //(15)
    #define BIT_NUM_IEC_CK_INV      1  // IEC clock inverse control register


    #define BIT_STR_AOUT_BNUM       16 //(16 - 17)
    #define BIT_NUM_AOUT_BNUM       2  // Number of bits for analog audio output cooked PCM data
        #define REGAOUT_24BIT   0
        #define REGAOUT_20BIT   2
        #define REGAOUT_16BIT   3
    
    #define BIT_STR_AOUT2_BNUM      18 //(18 - 19)
    #define BIT_NUM_AOUT2_BNUM      2  // Number of bits for analog audio output cooked PCM data
        #define REGAOUT2_24BIT  0
        #define REGAOUT2_20BIT  2
        #define REGAOUT2_16BIT  4

    #define BIT_STR_IEC_DS_CTRL     20 //(20)
    #define BIT_NUM_IEC_DS_CTRL     1  //IEC downsample controol
        #define REG_IEC_DS_RISC 0//controlled by RISC register 0x5014 [14:13]
        #define REG_IEC_DS_DSP  1//controlled by ADSP register 0x2a1 [23:22]

    #define BIT_STR_IEC2_DS_CTRL    21 //(21)
    #define BIT_NUM_IEC2_DS_CTRL    1  //IEC2 downsample controol
        #define REG_IEC2_DS_RISC 0//controlled by RISC register 0x50c0 [24:23]
        #define REG_IEC2_DS_DSP  1//controlled by ADSP register 0x2b1 [23:22]

    #define BIT_STR_IEC2_BNUM       22 //(22 - 23)
    #define BIT_NUM_IEC2_BNUM       2 //Number of bits for IEC60958 output cooked PCM data
        #define REGIEC2_24BIT   0
        #define REGIEC2_20BIT   2
        #define REGIEC2_16BIT   3

    #define BIT_STR_MIC_LR_INV      24 //(24)
    #define BIT_NUM_MIC_LR_INV      1  //option to avoid microphone L/R inverse problem

    #define BIT_STR_SPL_LR_INV      25 //(25)
    #define BIT_NUM_SPL_LR_INV      1  //option to avoid SPDIF/Line-in L/R inverse problem

    #define BIT_STR_IEC2_HB         26 //(26)
    #define BIT_NUM_IEC2_HB         1  //IEC2 output high bit rate control register

    #define BIT_STR_IEC2_DST        27 //(27)
    #define BIT_NUM_IEC2_DST        1  //IEC2 output SACD DST packet control register

    #define BIT_STR_ADDR_CFG_MP     28 //(28)
    #define BIT_NUM_ADDR_CFG_MP     1  //How to update DRAM address for microphone
        #define REG_DRAM_ALE    0//with DRAM ale
        #define REG_SAMPLE      1//with input sample coun

    #define BIT_STR_ADDR_CFG_SL     29 //(29)
    #define BIT_NUM_ADDR_CFG_SL     1  //How to update DRAM address for SPDIF/Line input

//SPDIF Sample Frequency Detection
#define REGENV_SPDFIN_FS        (AUD_REG_ENV_BASE + 0x018)
    #define BIT_STR_SPDF_LRCK_DIV   0  //(0)
    #define BIT_NUM_SPDF_LRCK_DIV   13 //


//SPDIF/Line In Buffer Block
#define REGENV_SPLIN_BLK        (AUD_REG_ENV_BASE + 0x01C)
    #define BIT_STR_SPLIN_EBLK      0  //(0 - 15)
    #define BIT_NUM_SPLIN_EBLK      16 //The lower two-byte registers, plus 8 bits of zero as its LSB, 
                                       //form a 24-bit byte address of the end point of DRAM used by SPDIF/Line in buffer.
    
    #define BIT_STR_SPLIN_SBLK      16 //(16 - 31)
    #define BIT_NUM_SPLIN_SBLK      16 //The higher two-byte registers, plus 8 bits of zero as its LSB
                                       //form a 24-bit byte address of the start point of DRAM used by SPDIF/Line in buffe
//SPDIF/Line In Control
#define REGENV_SPLIN_CTRL       (AUD_REG_ENV_BASE + 0x020)
    #define BIT_STR_SPLIN_EN        0  //(0)
    #define BIT_NUM_SPLIN_EN        1  // Enable the SPDIF/Line-in buffering data to DRAM between SPLIN_SBLK and SPLIN_EBLK.

    #define BIT_STR_SPLIN_BIT24     1  //(1)
    #define BIT_NUM_SPLIN_BIT24     1  // Store 24 bits or 16 bits of input data to DRAM buffer.

    #define BIT_STR_SPLIN_RAW16     2  //(2)
    #define BIT_NUM_SPLIN_RAW16     1  //

    #define BIT_STR_SPLIN_SWAP      3  //(3)
    #define BIT_NUM_SPLIN_SWAP      1  //When BIT24 is set false, high and low bytes in a word are swapped or not before stored to DRAM.

    #define BIT_STR_SPLIN_INT_PRD   4  //(4 -5)
    #define BIT_NUM_SPLIN_INT_PRD   2  //RISC interrupt generating period  for SPDIF mode.
        #define REGSPLIN_INT_PRD_NONE  0//disable
        #define REGSPLIN_INT_PRD_64DW  1//64 double words
        #define REGSPLIN_INT_PRD_128DW 2//128 double words
        #define REGSPLIN_INT_PRD_256DW 3//256 double words

    #define BIT_STR_SPLIN_NEG_SPACE 6  //(6)
    #define BIT_NUM_SPLIN_NEG_SPACE 1  //neglect IEC61937 burst spacing

    #define BIT_STR_SPLIN_USE_IND_PAD 7 //(7)
    #define BIT_NUM_SPLIN_USE_IND_PAD 1 //The line-in use separated pins to SPDIF in.

    #define BIT_STR_SPLIN_PPNT0     8  //(8)
    #define BIT_NUM_SPLIN_PPNT0     1  // Audio parser pointer0 selection
                                       // 0 Select RISC parser pointer as getbs parser pointer
                                       // 1 Select SPDIF/Line-in write pointer as getbs parser pointer
    #define BIT_STR_SPLIN_PPNT1     9  //(9)
    #define BIT_NUM_SPLIN_PPNT1     1  // Audio parser pointer1 selection

    #define BIT_STR_SPLIN_PPNT2     10 //(10)
    #define BIT_NUM_SPLIN_PPNT2     1  // Audio parser pointer2 selection

    #define BIT_STR_SPDFLIN_BANK    16 //(16 -26)
    #define BIT_NUM_SPDFLIN_BANK    11 //SPDIF-in/Line-in bank (in 1M bytes unit)

//SPDIF Type Detection
#define REGENV_SPDF_TYPE        (AUD_REG_ENV_BASE + 0x024)
    #define BIT_STR_SPDF_DETAIL     0  //(0 - 4)
    #define BIT_NUM_SPDF_DETAIL     5  //Detail type for IEC61937 RAW data

    #define BIT_STR_SPDF_BSNUM      5  //(5 - 7)
    #define BIT_NUM_SPDF_BSNUM      3  //Bit stream number for IEC61937 RAW data

    #define BIT_STR_SPDF_ROUGH      8  //(8 - 9)
    #define BIT_NUM_SPDF_ROUGH      2  //Rough type of the SPDIF input bit stream
        #define REGSPDF_PCM     0
        #define REGSPDF_RAW     1//Detail types will be in bits 4~0.
        #define REGSPDF_16BIT_DTSCD 2
        #define REGSPDF_14BIT_DTSCD 3
    
    #define BIT_STR_SPDF_DEC        10 //(10)
    #define BIT_NUM_SPDF_DEC        1  // SPDIF bit stream type decided or not.


//ADSP_ENV Reserved Register
#define REGENV_AENV_BAK         (AUD_REG_ENV_BASE + 0x02C)
    #define BIT_STR_MPHONE_SLAVE    18
    #define BIT_NUM_MPHONE_SLAVE    1

    #define BIT_STR_AIN_INV_BCK     31 //(31)
    #define BIT_NUM_AIN_INV_BCK     1 //
    
//DSP Read/Write DRAM Start Block Page 0/1
#define REGENV_RWD_BLK01        (AUD_REG_ENV_BASE + 0x058)

//DSP Read/Write DRAM Start Block Page 2/3
#define REGENV_RWD_BLK23        (AUD_REG_ENV_BASE + 0x05C)

//DSP Read/Write DRAM Start Block Page 4/5
#define REGENV_RWD_BLK45        (AUD_REG_ENV_BASE + 0x060)
    #define BIT_STR_DRAM_SBLK5        16 //(16 - 31)
    #define BIT_NUM_DRAM_SBLK5        16 //

//DSP Read/Write DRAM Start Block Page 6/7
#define REGENV_RWD_BLK67        (AUD_REG_ENV_BASE + 0x064)
    #define BIT_STR_DRAM_SBLK7        16 //(16 - 31)
    #define BIT_NUM_DRAM_SBLK7        16 //

//Bit Stream Buffer 0 Start Block
#define REGENV_SW_BS0_SBLK      (AUD_REG_ENV_BASE + 0x080)

//Bit Stream Buffer 0 End Block
#define REGENV_SW_BS0_EBLK      (AUD_REG_ENV_BASE + 0x084)

//Bit Stream Buffer 0 Parser Pointer
#define REGENV_SW_BS0_PPNT      (AUD_REG_ENV_BASE + 0x088)

//Bit Stream Buffer 1 Start Block
#define REGENV_SW_BS1_SBLK      (AUD_REG_ENV_BASE + 0x08C)

//Bit Stream Buffer 1 End Block
#define REGENV_SW_BS1_EBLK      (AUD_REG_ENV_BASE + 0x090)

//Bit Stream Buffer 1 Parser Pointer
#define REGENV_SW_BS1_PPNT      (AUD_REG_ENV_BASE + 0x094)


//Audio Output Configuration
#define REGENV_AOUT_CFG         (AUD_REG_ENV_BASE + 0x0C0)
    #define BIT_STR_LRCK_CYC        0  //(0 - 1)
    #define BIT_NUM_LRCK_CYC        2  //Number of clock cycles of BCK that half LRCK duration is.
        #define REG_LRCK_CYC16  0
        #define REG_LRCK_CYC24  1
        #define REG_LRCK_CYC32  2
    
    #define BIT_STR_LRCK_PAD        2 //(2)
    #define BIT_NUM_LRCK_PAD        1 //Selection of output signal for pad of LRCK

    #define BIT_STR_AOUT2_PM        3 //(3)
    #define BIT_NUM_AOUT2_PM        1 //

    #define BIT_STR_DAT_DLY         4 //(4)
    #define BIT_NUM_DAT_DLY         1 // 

    #define BIT_STR_LEFT_ALN        5 //(5)
    #define BIT_NUM_LEFT_ALN        1 // 

    #define BIT_STR_INV_BCK         6 //(6)
    #define BIT_NUM_INV_BCK         1 //AOBCK is inverted or not at the output port.

    #define BIT_STR_INV_LRCK        7 //(7)
    #define BIT_NUM_INV_LRCK        1 //AOLRCK is inverted or not. 
      
    #define BIT_STR_DA_BNUM         8 //(8 - 13)
    #define BIT_NUM_DA_BNUM         6 //Audio DAC bit number

    #define BIT_STR_INV_SDATA       14 //(14)
    #define BIT_NUM_INV_SDATA       1  //Invert audio output serial data for OP phase inverting

    #define BIT_STR_AOUT2_DSD       15 //(15)
    #define BIT_NUM_AOUT2_DSD       1  //Audio output 2 output DSD data

    #define BIT_STR_A2BCKX          16 //(16 - 19)
    #define BIT_NUM_A2BCKX          4  // The ratio of half BCK cycle period to ACK
    
    #define BIT_STR_ACK_SEL         20 //(20)
    #define BIT_NUM_ACK_SEL         1  //ACK source selection
                    //0: ACK is from internal PLL and outputted to the external pad.
                    //1: ACK is from the external pad. 
    #define BIT_STR_SPDF_SEL        21 //(21)
    #define BIT_NUM_SPDF_SEL        1  //SPDIF output selection

    #define BIT_STR_SPDF2_SEL       22 //(22)
    #define BIT_NUM_SPDF2_SEL       1  //SPDIF2 output selection   

    #define BIT_STR_IEC2_DS         23 //(23 - 24)
    #define BIT_NUM_IEC2_DS         2  //IEC2 output down sample 

    #define BIT_STR_DSD2_LR_LRCK    25 //(25)
    #define BIT_NUM_DSD2_LR_LRCK    1  //IEC2 output down sample 
    
    #define BIT_STR_IEC2_SWAP       26 //(26)
    #define BIT_NUM_IEC2_SWAP       1  //

    #define BIT_STR_IEC_SWAP        27 //(27)
    #define BIT_NUM_IEC_SWAP        1  //Swap the high & low bytes of audio bit stream for IEC raw data output

    #define BIT_STR_SLEW            28 //(28)
    #define BIT_NUM_SLEW            1  //IEC60958 output pad (SPDIF output) slew rate control

    #define BIT_STR_DRV_CUR         29 //(29 - 30)
    #define BIT_NUM_DRV_CUR         2  //IEC60958 output pad (SPDIF output) driving current ;n (2n + 2) mA

//Audio Input Hardware Configuration
#define REGENV_AIN_CFG          (AUD_REG_ENV_BASE + 0x0C4)
    #define BIT_STR_MP_BNUM         0 //(0 - 4)
    #define BIT_NUM_MP_BNUM         5 //Number of bits for microphone ADC resolution less 1

    #define BIT_STR_M_LEFT_A        5 //(5)
    #define BIT_NUM_M_LEFT_A        1 //

    #define BIT_STR_M_DAT_D         6 //(6)
    #define BIT_NUM_M_DAT_D         1 //    

    #define BIT_STR_M_INV_LRCK      7 //(7)
    #define BIT_NUM_M_INV_LRCK      1 //    

    #define BIT_STR_SPL_BNUM        8 //(8 - 12)
    #define BIT_NUM_SPL_BNUM        5 //Number of bits for Line-in ADC or SPDIF DIR less 1
   
    #define BIT_AIN_STR_LEFT_ALN    13//(13)
    #define BIT_AIN_NUM_LEFT_ALN    1 //

    #define BIT_AIN_STR_DAT_DLY     14//(14)
    #define BIT_AIN_NUM_DAT_DLY     1 //

    #define BIT_STR_AIN_INV_LRCK        15//(15)
    #define BIT_NUM_AIN_INV_LRCK        1 //    

    #define BIT_AIN_STR_LRCK_CYC        16//(16 - 17)
    #define BIT_AIN_NUM_LRCK_CYC        2 //    

    #define BIT_STR_MPBCK_INV       18//(18)
    #define BIT_NUM_MPBCK_INV       1 //    

    #define BIT_STR_MPCLK_IND       19//(19)
    #define BIT_NUM_MPCLK_IND       1 //Switch for Microphone use different sampling rate.

    #define BIT_STR_SPL_SEL         20//(20)
    #define BIT_NUM_SPL_SEL         1 //Select the interface is for SPDIF or Line-in  
                //For SPDIF, audio clock MCLK, BCK and LRCK are from external (SPMCLK, SPBCK and SPLRCK)
                //For Line-in, these signals are all outputs (AO1MCLK, AO1BCK and AO1LRCK).
        #define LIN_INT_CLK   0
        #define LIN_EXT_CLK   1

    #define BIT_STR_EXT_BCK_SEL     21//(21)
    #define BIT_NUM_EXT_BCK_SEL     1 //BCK source selection 
        #define BCK_INT     0
        #define BCK_EXT     1

    #define BIT_STR_EXT_LRCK_SEL    22//(22)
    #define BIT_NUM_EXT_LRCK_SEL    1 // LRCK source selection
        #define LRCK_INT    0
        #define LRCK_EXT    1

    #define BIT_STR_SDATA_DRIV      23//(23)
    #define BIT_NUM_SDATA_DRIV      1 //  AOSDATA* output slew rate control

    #define BIT_STR_SDATA_SLEW      24//(24 - 25)
    #define BIT_NUM_SDATA_SLEW      2 //  AOSDATA* output pad driving current

    #define BIT_STR_BCK_SLEW        26//(26)
    #define BIT_NUM_BCK_SLEW        2 //AOBCK output slew rate control

    #define BIT_STR_BCK_CUR         27//(27 - 28)
    #define BIT_NUM_BCK_CUR         2 // AOBCK output pad driving current

    #define BIT_STR_ACK_SLEW        29//(29)
    #define BIT_NUM_ACK_SLEW        1 // AOMCLK output slew rate control

    #define BIT_STR_ACK_CUR         30//(30 - 31)
    #define BIT_NUM_ACK_CUR         2 // AOMCLK output pad driving 
    
//Additional Audio Output Configuration
#define REGENV_AOUT_CFG1        (AUD_REG_ENV_BASE + 0x0C8)
    #define BIT_STR_AOUT_PM         0 //(0)
    #define BIT_NUM_AOUT_PM         1 //Audio output phase modulation select 

    #define BIT_STR_AOUT_DSD        1 //(1)
    #define BIT_NUM_AOUT_DSD        1 // Audio output DSD data

    #define BIT_STR_DSD_LR_LRCK     2 //(2)
    #define BIT_NUM_DSD_LR_LRCK     1 //

    #define BIT_STR_DSD_LR_BCK      3 //(3)
    #define BIT_NUM_DSD_LR_BCK      1 //    

    #define BIT_STR_AOSDATA0        4 //(4 - 6)
    #define BIT_NUM_AOSDATA0        3 //  Aosdata0 output channel configuration
        #define REGAO_CH1CH2    0
        #define REGAO_CH3CH4    1
        #define REGAO_CH5CH6    2
        #define REGAO_CH7CH8    3
        #define REGAO_CH9CH10   4
        #define REGAO_CH11CH12  5
    #define BIT_STR_AOSDATA1        7 //(7 - 9)
    #define BIT_NUM_AOSDATA1        3 //  Aosdata1 output channel configuration

    #define BIT_STR_AOSDATA2        10//(10 - 12)
    #define BIT_NUM_AOSDATA2        3 //  Aosdata2 output channel configuration

    #define BIT_STR_AOSDATA3        13//(13 - 15)
    #define BIT_NUM_AOSDATA3        3 //  Aosdata3 output channel configuration

    #define BIT_STR_AOSDATA4        16//(16 - 18)
    #define BIT_NUM_AOSDATA4        3 //  Aosdata4 output channel configuration

    #define BIT_STR_AOSDATA5        19//(19 - 21)
    #define BIT_NUM_AOSDATA5        3 //  Aosdata5 output channel configuration

    #define BIT_STR_AOSDATA6        22//(22 - 23)
    #define BIT_NUM_AOSDATA6        2 //  Aosdata6 output channel configuration
        #define REGAO2_CH1CH2   0
        #define REGAO2_CH3CH4   1
        #define REGAO2_CH5CH6   2
        #define REGAO2_CH7CH8   3
    #define BIT_STR_AOSDATA7        24//(24 - 25)
    #define BIT_NUM_AOSDATA7        2 //  Aosdata7 output channel configuration

    #define BIT_STR_AOSDATA8        26//(26 - 27)
    #define BIT_NUM_AOSDATA8        2 //  Aosdata8 output channel configuration

    #define BIT_STR_AOSDATA9        28//(28 - 29)
    #define BIT_NUM_AOSDATA9        2 //  Aosdata9 output channel configuration

    #define BIT_STR_AOUT_AOUT2      30//(30)
    #define BIT_NUM_AOUT_AOUT2      1 //  Analog audio output selection

    #define BIT_STR_AOUT2_AOUT      31//(31)
    #define BIT_NUM_AOUT2_AOUT      1 //  Analog audio output2 selection

//Audio misc control
#define REGENV_MISC_CTRL        (AUD_REG_ENV_BASE + 0x0CC)
    #define BIT_STR_MPHONE_BCK_DIV  0 //(0 - 3)
    #define BIT_NUM_MPHONE_BCK_DIV  4 // Decide how many mphone_mclk cycles will make half period of mphone_bck.

    #define BIT_STR_MPHONE_LRCK_DIV_SEL  4 //(4 - 5)
    #define BIT_NUM_MPHONE_LRCK_DIV_SEL  2 // Decide the how many mphone_bck cycle in a lrck cycle. 
    
    #define BIT_STR_DSPA_MCLK_EN    6 //(6)
    #define BIT_NUM_DSPA_MCLK_EN    1 //DSPA AUDIO_MCLK_EN
    
    #define BIT_STR_DSPB_MCLK_EN    7 //(7)
    #define BIT_NUM_DSPB_MCLK_EN    1 //DSPB AUDIO_MCLK_EN

    #define BIT_STR_PUTBS_DSP_BUF   8 //(8)
    #define BIT_NUM_PUTBS_DSP_BUF   1 //putbs data output DSPA or DSPB DRAM cache

    #define BIT_STR_SACD_DSP_BUF    9 //(9)
    #define BIT_NUM_SACD_DSP_BUF    1 //SACD data output DSPA or DSPB DRAM cache

    #define BIT_STR_WDMA_DSP_BUF    10//(10)
    #define BIT_NUM_WDMA_DSP_BUF    1 //wdma data output DSPA or DSPB DRAM cache

    #define BIT_STR_ACK2_SEL        11//(11)
    #define BIT_NUM_ACK2_SELF       1 //ACK2 source selection

    #define BIT_STR_AOUT2_A2BCKX    12//(12 - 15)
    #define BIT_NUM_AOUT2_A2BCKX    4 //The ratio of half BCK cycle period to ACK. BCK is generated dividing ACK with this number

    #define BIT_STR_EXT_BCK2_SEL    16//(16)
    #define BIT_NUM_EXT_BCK2_SEL    1 //BCK2 source selection

    #define BIT_STR_AOUT2_LRCK_CYC  17//(17 - 18)
    #define BIT_NUM_AOUT2_LRCK_CYC  2 //Number of clock cycles of BCK2 that half LRCK2 duration is for audio output.

    #define BIT_STR_AOUT2_DELAY     19//(19)
    #define BIT_NUM_AOUT2_DELAY     1 //

    #define BIT_STR_AOUT2_LEFT      20//(20)
    #define BIT_NUM_AOUT2_LEFT      1 //

    #define BIT_STR_INV_BCK2        21//(21)
    #define BIT_NUM_INV_BCK2        1 //

    #define BIT_STR_INV_LRCK2       22//(22)
    #define BIT_NUM_INV_LRCK2       1 //

    #define BIT_STR_AOUT2_DA_BNUM   23//(23 - 28)
    #define BIT_NUM_AOUT2_DA_BNUM   6 //

    #define BIT_STR_AOUT2_INV_SDATA 29//(29)
    #define BIT_NUM_AOUT2_INV_SDATA 1 //

    #define BIT_STR_MLP_DSP         30//(30)
    #define BIT_NUM_MLP_DSP         1 // DSPA or DSPB access MLP huffman decoder hardware

    #define BIT_STR_EXT_LRCK2_SEL   31//(31)
    #define BIT_NUM_EXT_LRCK2_SEL   1 //LRCK2 source selection

//AOUT TDM control
#define REGENV_AOUT_TDM_CTRL    (AUD_REG_ENV_BASE + 0x0D0)

//AOUT TDM channel configuration
#define REGENV_AOUT_TDM_CHCFG   (AUD_REG_ENV_BASE + 0x0D4)


//apll_calibation
#define REGENV_APLL_CAL         (AUD_REG_ENV_BASE + 0x200)
    #define BIT_STR_APLL_CALI       0 //(0 - 11)
    #define BIT_NUM_APLL_INV_BCK2   12//Write this register to start APLL calibration, count LRCK / 27M = RREG_APLL_CAL

//APLL_CFG0
#define REGENV_APLL_CFG0        (AUD_REG_ENV_BASE + 0x204)
    #define BIT_STR_APLL_PREDIV     0 //(0 - 1)
    #define BIT_NUM_APLL_PREDIV     2 //PLL Pre-div setting 
                                      //00 :1, 01 :1, 10:4, 11:8
    #define BIT_STR_APLL_CURRENTSEL 2 //(2 - 4)
    #define BIT_NUM_APLL_CURRENTSEL 2 //Charge Pump current 

    #define BIT_STR_APLL_POSTDIV    5 //(5 - 7)
    #define BIT_NUM_APLL_POSTDIV    3 //Charge Pump current    

    #define BIT_STR_APLL_DIV1       8 //(8)
    #define BIT_NUM_APLL_DIV1       1 //Feedback divider  

    #define BIT_STR_APLL_RELATCHEN  9 //(8)
    #define BIT_NUM_APLL_RELATCHEN  1 //Feedback divider relatch function

    #define BIT_STR_APLL_VCOBAND    10//(10 - 13)
    #define BIT_NUM_APLL_VCOBAND    4 //VCOBAND selection (manul mode)

    #define BIT_STR_APLL_PWD        14//(14)
    #define BIT_NUM_APLL_PWD        1 //PLL power down 
        #define REG_PLL_NORMAL      0
        #define REG_PLL_POWER_DOWN  1
    
    #define BIT_STR_APLL_DIV56      15//(15)
    #define BIT_NUM_APLL_DIV56      1 //
                                    //0: 432MHz / 5 = 86.4MHz
                                    //1: 432MHz / 6 = 72MHz
    #define BIT_STR_APLL_A2_K2_RESET_B 16//(16)
    #define BIT_NUM_APLL_A2_K2_RESET_B 1 //

    #define BIT_STR_APLL_CLK_A2_SEL 17//(17)
    #define BIT_NUM_APLL_CLK_A2_SEL 1 //APLL_CLK_A2 output frequency

    #define BIT_STR_APLL_CKSEL      18//(18 - 19)
    #define BIT_NUM_APLL_CKSEL      2 //PLL reference clock selection
                    //00: 432MHz / 6 = 72MHz, 01: DA_APLLCK
                    //10: REFCLK_DUM[1],      11: REFCLK_DUM[2]
    #define BIT_STR_APLL_REV        20//(20 - 27)
    #define BIT_NUM_APLL_REV        8 //Reserved     

    #define BIT_STR_APLL_RSTB       28//(28)
    #define BIT_NUM_APLL_RSTB       1 //PLL reset
    
    #define BIT_STR_APLL_RSTB_B_CORE 29//(29)
    #define BIT_NUM_APLL_RSTB_B_CORE 1 //APLL modulator & post divider reset_b

    #define BIT_STR_APLL_POSDIVSEL  30//(30)
    #define BIT_NUM_APLL_POSDIVSEL  1 //APLL post divider select
                              //0 : fractional-N, 1 : integer-N
    #define BIT_STR_DSP_CTRL_APLL_MODIN  31//(31)
    #define BIT_NUM_DSP_CTRL_APLL_MODIN  1 //DSP control APLL_MODIN     

//APLL_CFG1
#define REGENV_APLL_CFG1        (AUD_REG_ENV_BASE + 0x208)
    #define BIT_STR_APLL_MODDIV     0 //(0 - 7)
    #define BIT_NUM_APLL_MODDIV     8 //APLL post divider code select
    //00000000:    no use (default); 00000001:    posdiv = /3; 00000010:    posdiv = /4
    #define BIT_STR_APLL_MODIN      8 //(8 - 31)
    #define BIT_NUM_APLL_MODIN      24//APLL modulator input Freq is selected by APLL_DIV56

//APLL_CFG2
#define REGENV_APLL_CFG2        (AUD_REG_ENV_BASE + 0x20C)
    #define BIT_STR_APLL_RSEL       0 //(0 - 2)
    #define BIT_NUM_APLL_RSEL       3 //Resistance
    
    #define BIT_STR_APLL_VCOCAL_EN  3 //(3)
    #define BIT_NUM_APLL_VCOCAL_EN  1 //VCOBAND calibration enable

    #define BIT_STR_APLL_VCOCALSEL  4 //(4 - 5)
    #define BIT_NUM_APLL_VCOCALSEL  2 //VCOCAL period selection

    #define BIT_STR_APLL_VCOVTSEL   6 //(6 - 7)
    #define BIT_NUM_APLL_VCOVTSEL   2 //VCOCAL slicer voltage   

    #define BIT_STR_APLL_AUDIO_K1   8 //(8 - 15)
    #define BIT_NUM_APLL_AUDIO_K1   8 //IEC_CLK = APLL_CLK/(APLL_AUDIO_K1 + 1) 

    #define BIT_STR_APLL_AUDIO_K2   16//(16 - 23)
    #define BIT_NUM_APLL_AUDIO_K2   8 //AUD_CLK = APLL_CLK/(APLL_AUDIO_K2 + 1)

    #define BIT_STR_APLL_AUDIO_K3   24//(24 - 31)
    #define BIT_NUM_APLL_AUDIO_K3   8 //IEC2_CLK = APLL_CLK/(APLL_AUDIO_K3 + 1)

//APLL_CFG3
#define REGENV_APLL_CFG3        (AUD_REG_ENV_BASE + 0x210)

#define REGENV_DRAM_MON0        (AUD_REG_ENV_BASE + 0x214)
#define REGENV_DRAM_MON1        (AUD_REG_ENV_BASE + 0x218)
#define REGENV_CACHE_MON0       (AUD_REG_ENV_BASE + 0x21C)
#define REGENV_CACHE_MON1       (AUD_REG_ENV_BASE + 0x220)

#define REGENV_AOMCLK_CFG       (AUD_REG_ENV_BASE + 0x224)
    #define BIT_STR_AOMCLK_PD       0 //(0)
    #define BIT_NUM_AOMCLK_PD       1 //AOMCLK pad power down
        //0: normal function, 1: tie pad AOMCLK to low
    
    #define BIT_STR_AOBCK_PD        1 //(1)
    #define BIT_NUM_AOBCK_PD        1 //AOBCK pad power down

    #define BIT_STR_AOLRCK_PD       2 //(2)
    #define BIT_NUM_AOLRCK_PD       1 //AOLRCK pad power down

    #define BIT_STR_AOMCLK_OK       3 //(3)
    #define BIT_NUM_AOMCLK_OK       1 //AOMCLK switch ok

    #define BIT_STR_AOMCLK_CFG      4 //(4 - 6)
    #define BIT_NUM_AOMCLK_CFG      3 //AOMCLK change configuration
        #define REG_AOMCLK_XTAL_CK      0
        #define REG_AOMCLK_K2           1
        #define REG_AOMCLK_EXT          2
        #define REG_AOMCLK_SPIN_CLK     3
        #define REG_AOMCLK_INT_SPDIF_RX 4
        #define REG_AOMCLK_EXT2         5
        #define REG_AOMCLK_HDMI_RX_CK   6
        
    #define BIT_STR_AO2MCLK_PD      8 //(8)
    #define BIT_NUM_AO2MCLK_PD      1 //AO2MCLK pad power down
    
    #define BIT_STR_AO2BCK_PD       9 //(9)
    #define BIT_NUM_AO2BCK_PD       1 //AO2BCK pad power down

    #define BIT_STR_AO2LRCK_PD      10//(10)
    #define BIT_NUM_AO2LRCK_PD      1 //AO2LRCK pad power down

    #define BIT_STR_AO2MCLK_OK      11//(11)
    #define BIT_NUM_AO2MCLK_OK      1 //AO2MCLK switch ok

    #define BIT_STR_AO2MCLK_CFG     12//(12 - 13)
    #define BIT_NUM_AO2MCLK_CFG     2 //AO2MCLK change configuration

    #define BIT_STR_MPHMCLK_PD      16//(16)
    #define BIT_NUM_MPHMCLK_PD      1 //MPHMCLK pad power down
    
    #define BIT_STR_MPHBCK_PD       17//(17)
    #define BIT_NUM_MPHBCK_PD       1 //MPHBCK pad power down

    #define BIT_STR_MPHLRCK_PD      18//(18)
    #define BIT_NUM_MPHLRCK_PD      1 //MPHLRCK pad power down

    #define BIT_STR_MPHCLK_CFG      20//(20 - 21)
    #define BIT_NUM_MPHCLK_CFG      2 //MPHCLK change configuration

//deep configuration register
#define REGENV_DEEP_CFG         (AUD_REG_ENV_BASE + 0x228)
    #define BIT_STR_AOUT_128        0 //(0)
    #define BIT_NUM_AOUT_128        1 //AOUT 128 bits mode select

    #define BIT_STR_IEC_128         1 //(1)
    #define BIT_NUM_IEC_128         1 //IEC 128 bits mode select
    
    #define BIT_STR_MPHONE_128      2 //(2)
    #define BIT_NUM_MPHONE_128      1 //MPHONE 128 bits mode select

    #define BIT_STR_AOUT2_128       3 //(3)
    #define BIT_NUM_AOUT2_128       1 //AOUT2 128 bits mode select

    #define BIT_STR_ADD_LATENCY     4 //(4 - 5)
    #define BIT_NUM_ADD_LATENCY     2 //add latency control

//ADSP_ENV reserved Register2
#define REGENV_DADSP_AENV_BAK2      (AUD_REG_ENV_BASE + 0x230)    
    #define BIT_STR_BCK2_EXT_SEL    6 //(6)
    #define BIT_NUM_BCK2_EXT_SEL    1 //aud2 external bck select
        //0: from external ao2bck, 1: from aud bck

    #define BIT_STR_READ_STC_SEL    7 //(7 - 8)
    #define BIT_NUM_READ_STC_SEL    2 //this register controls which STC is read. 

    #define BIT_STR_PDMA_DEST_BANK_SEL    9 //(9)
    #define BIT_NUM_PDMA_DEST_BANK_SEL    1 //Parser dma destination bank control.
      //0: use DSP working area bank.   1: use AFIFO bank.

    #define BIT_STR_PDMA_SRC_BANK_SEL     10//(10)
    #define BIT_NUM_PDMA_SRC_BANK_SEL     1 //Parser dma destination bank control.

    #define BIT_STR_READ_STC_CTRL_SEL     11//(11)
    #define BIT_NUM_READ_STC_CTRL_SEL     1 //STC read selection control.

    #define BIT_STR_MULTI_CH_CFG0       14//(14 - 16)
    #define BIT_NUM_MULTI_CH_CFG0       3 //Multiple line in sdata 0 source
        #define SPLINE2_SDATA   0
        #define SPLINE_SDATA1   1
        #define SPLINE_SDATA2   2
        #define SPLINE_SDATA3   3
        #define SPLINE_SDATA4   4
        #define SPLINE_SDATA5   5
        #define SPLINE_SDATA6   6
        #define SPLINE_SDATA7   7

    #define BIT_STR_MULTI_CH_CFG1       17//(17 - 19)
    #define BIT_NUM_MULTI_CH_CFG1       3 //Multiple line in sdata 1 source

    #define BIT_STR_MULTI_CH_CFG2       20//(20 - 22)
    #define BIT_NUM_MULTI_CH_CFG2       3 //Multiple line in sdata 2 source

    #define BIT_STR_MULTI_CH_CFG3       23//(23 - 25)
    #define BIT_NUM_MULTI_CH_CFG3       3 //Multiple line in sdata 3 source

    #define BIT_STR_MULTI_CH_CFG4       26//(26 - 28)
    #define BIT_NUM_MULTI_CH_CFG4       3 //Multiple line in sdata 4 source

    #define BIT_STR_MULTI_CH_CFG5       29//(29 - 31)
    #define BIT_NUM_MULTI_CH_CFG5       3 //Multiple line in sdata 5 source

//PWMIP channel config
#define REGENV_AENV_BAK2            (AUD_REG_ENV_BASE + 0x240)    
    #define BIT_STR_PWM1_CH_CFGA       0 //(0 - 3)
    #define BIT_NUM_PWM1_CH_CFGA       4 //aout1 pwm ch12 config others:ch12
        #define PWM_CH12    0
        #define PWM_CH34    1
        #define PWM_CH56    2
        #define PWM_CH78    3
        #define PWM_CH910   4

    #define BIT_STR_PWM1_CH_CFGB       4 //(4 - 7)
    #define BIT_NUM_PWM1_CH_CFGB       4 //aout1 pwm ch34 config others:ch34

    #define BIT_STR_PWM1_CH_CFGC       8 //(8 - 11)
    #define BIT_NUM_PWM1_CH_CFGC       4 //aout1 pwm ch56 config others:ch34

    #define BIT_STR_PWM1_CH_CFGD       12//(12 - 15)
    #define BIT_NUM_PWM1_CH_CFGD       4 //aout1 pwm ch910 config others:ch34

//PWM update configure
#define REGENV_PWM_UPDATE_CFG1      (AUD_REG_ENV_BASE + 0x248)    

#define REGENV_SW_BS2_RPTR          (AUD_REG_ENV_BASE + 0x24C)  
//Bit Stream Buffer 2 Start Block
#define REGENV_SW_BS2_SBLK          (AUD_REG_ENV_BASE + 0x250)  

//Bit Stream Buffer 2 End Block
#define REGENV_SW_BS2_EBLK          (AUD_REG_ENV_BASE + 0x254)  

//Bit Stream Buffer 2 Parser Pointer
#define REGENV_SW_BS2_PPNT          (AUD_REG_ENV_BASE + 0x258)

//PWM2 channel config
#define REGENV_PWMIP_S_DAC_CH_CFG   (AUD_REG_ENV_BASE + 0x260)
    #define BIT_STR_PWM2_CH_CFGA       0 //(0 - 3)
    #define BIT_NUM_PWM2_CH_CFGA       4 //aout1 pwm ch12 config others:ch12

    #define BIT_STR_PWM2_CH_CFGB       4 //(4 - 7)
    #define BIT_NUM_PWM2_CH_CFGB       4 //aout1 pwm ch34 config others:ch34

    #define BIT_STR_PWM2_CH_CFGC       8 //(8 - 11)
    #define BIT_NUM_PWM2_CH_CFGC       4 //aout1 pwm ch56 config others:ch34

    #define BIT_STR_PWM2_CH_CFGD       12//(12 - 15)
    #define BIT_NUM_PWM2_CH_CFGD       4 //aout1 pwm ch910 config others:ch34

//2ND  SPDIF/Line In Buffer Block
#define REGENV_SPLIN_BLK_LIN2       (AUD_REG_ENV_BASE + 0x264)
    #define BIT_STR_SPLIN2_EBLK             0 //(0 - 15)
    #define BIT_NUM_SPLIN2_EBLK             16//

    #define BIT_STR_SPLIN2_SBLK             16//(16 - 31)
    #define BIT_NUM_SPLIN2_SBLK             16//

//2nd  Line In control
#define REGENV_SPLIN_CTL_LIN2       (AUD_REG_ENV_BASE + 0x268)
    #define BIT_STR_LIN2_EN                 0 //(0)
    #define BIT_NUM_LIN2_EN                 1 //

    #define BIT_STR_LIN2_BIT24              1 //(1)
    #define BIT_NUM_LIN2_BIT24              1 //

    #define BIT_STR_LIN2_RAW16              2 //(2)
    #define BIT_NUM_LIN2_RAW16              1 //

    #define BIT_STR_LIN2_SWAP               3 //(3)
    #define BIT_NUM_LIN2_SWAP               1 //

    #define BIT_STR_LIN2_INTR_PERIOD        4 //(4 - 5)
    #define BIT_NUM_LIN2_INTR_PERIOD        2 //
        #define LIN2_INTR_PERIOD_32DWRD 0
        #define LIN2_INTR_PERIOD_64DWRD 1
        #define LIN2_INTR_PERIOD_128DWRD 2
        #define LIN2_INTR_PERIOD_256DWRD 3
    
    #define BIT_STR_LIN2_PSU_BS             6 //(6)
    #define BIT_NUM_LIN2_PSU_BS             1 //Pseudo raw data detect mode


//2nd Audio Input Hardware Configuration
#define REGENV_AIN_CFG_LIN2         (AUD_REG_ENV_BASE + 0x26C)
    #define BIT_STR_LIN2_BNUM               8 //(8 - 12)
    #define BIT_NUM_LIN2_BNUM               5 //Number of bits for Line-in ADC or SPDIF DIR less 1  

    #define BIT_STR_LIN2_LEFT_ALN           13 //(13)
    #define BIT_NUM_LIN2_LEFT_ALN           1 //

    #define BIT_STR_LIN2_DAT_DLY            14 //(14)
    #define BIT_NUM_LIN2_DAT_DLY            1 //

    #define BIT_STR_LIN2_INV_LRCK           15 //(15)
    #define BIT_NUM_LIN2_INV_LRCK           1 //

    #define BIT_STR_LIN2_LRCK_CYC           16 //(16 - 17)
    #define BIT_NUM_LIN2_LRCK_CYC           2 //

    #define BIT_STR_LIN2_SPL_SEL            20 //(20)
    #define BIT_NUM_LIN2_SPL_SEL            1 //
        //0: The interface is Line-in.  1: The interface is SPDIF in.

    #define BIT_STR_LIN2_EXT_BCK_SEL        21 //(21)
    #define BIT_NUM_LIN2_EXT_BCK_SEL        1 //
        //0: BCK is from internal PLL and . 1: BCK is from the external pad.

    #define BIT_STR_LIN2_EXT_LRCK_SEL       22 //(22)
    #define BIT_NUM_LIN2_EXT_LRCK_SEL       1 //

    #define BIT_STR_LIN2_BCK_INV            23 //(23)
    #define BIT_NUM_LIN2_BCK_INV            1 //

    #define BIT_STR_LIN2_USE_AO2_TIM        24 //(24)
    #define BIT_NUM_LIN2_USE_AO2_TIM        1 //

    #define BIT_STR_LIN2_SEL_MLINCKGEN      25 //(25)
    #define BIT_NUM_LIN2_SEL_MLINCKGEN      1 //   

    #define BIT_STR_LIN2_BCK_DIV            26 //(26 - 29)
    #define BIT_NUM_LIN2_BCK_DIV            4 //   

    #define REGENV_SPLIN_WRADR          (AUD_REG_ENV_BASE + 0x2D8)  //3363 ver

#define REGENV_SPMULTI_WRADR        (AUD_REG_ENV_BASE + 0x2DC)
#define REGENV_SPLIN2_WRADR         (AUD_REG_ENV_BASE + 0x2E0)
    #define BIT_STR_LIN_WP                  0
    #define BIT_NUM_LIN_WP                  32
    
//Compact-Aout CRC config
#define REGENV_AOUT_CRC_CTRL        (AUD_REG_ENV_BASE + 0x360)
//Aout CRC start address
#define REGENV_AOUT_CRC_STRADR      (AUD_REG_ENV_BASE + 0x364)
//Aout CRC end address
#define REGENV_AOUT_CRC_ENDADR      (AUD_REG_ENV_BASE + 0x368)
//Aout CRC record address
#define REGENV_AOUT_CRC_RECADR      (AUD_REG_ENV_BASE + 0x36C)
//Aout CRC result
#define REGENV_AOUT_CRC_RESULT      (AUD_REG_ENV_BASE + 0x370)

//Audio DRAM bank
#define REGENV_AUD_DRAM_BANK        (AUD_REG_ENV_BASE + 0x3A0)
    #define BIT_STR_ADSP_BANK               0 //(0 - 10)
    #define BIT_NUM_ADSP_BANK               11//
    
    #define BIT_STR_AFIFO_BANK              12//(12 - 22)
    #define BIT_NUM_AFIFO_BANK              11//

//MPHONE_WADR
#define REGENV_MPHONE_WADR          (AUD_REG_ENV_BASE + 0x3DC)

//Line In Multi Cfg
#define REGENV_LINBLK_MULTI         (AUD_REG_ENV_BASE + 0x3E8)
    #define BIT_STR_SPLIN_EBLK_MULTI        0 //(0 - 15)
    #define BIT_NUM_SPLIN_EBLK_MULTI        16//

    #define BIT_STR_SPLIN_SBLK_MULTI        16//(16 - 31)
    #define BIT_NUM_SPLIN_SBLK_MULTI        16//

//Line In Multi
#define REGENV_LIN_MULTI            (AUD_REG_ENV_BASE + 0x3EC)
    #define BIT_STR_MULTI_EN                0 //(0)
    #define BIT_NUM_MULTI_EN                1 //

    #define BIT_STR_MULTI_BIT24             1 //(1)
    #define BIT_NUM_MULTI_BIT24             1 //

    #define BIT_STR_MULTI_RAW16             2 //(2)
    #define BIT_NUM_MULTI_RAW16             1 //

    #define BIT_STR_MULTI_SWAP              3 //(3)
    #define BIT_NUM_MULTI_SWAP              1 //

    #define BIT_STR_MULTI_INTR_PERIOD       4 //(4 - 5)
    #define BIT_NUM_MULTI_INTR_PERIOD       2 //
        #define MULTI_INT_32DWRD    0
        #define MULTI_INT_64DWRD    1
        #define MULTI_INT_128DWRD   2
        #define MULTI_INT_256DWRD   3
    #define BIT_STR_MULTI_PSU_BS            6
    #define BIT_NUM_MULTI_PSU_BS            1

    #define BIT_STR_MULTI_PNT_SEL0          8
    #define BIT_NUM_MULTI_PNT_SEL0          1

    #define BIT_STR_MULTI_PNT_SEL1          9
    #define BIT_NUM_MULTI_PNT_SEL1          1

    #define BIT_STR_MULTI_BCKLRCK_SEL_1     10
    #define BIT_NUM_MULTI_BCKLRCK_SEL_1     1

    #define BIT_STR_MULTI_SDATA0_SEL_1      11
    #define BIT_NUM_MULTI_SDATA0_SEL_1      1

    #define BIT_STR_MULTI_BCKLRCK_SEL_0     12
    #define BIT_NUM_MULTI_BCKLRCK_SEL_0     1
    
    #define BIT_STR_MULTI_SDATA0_SEL_0      13
    #define BIT_NUM_MULTI_SDATA0_SEL_0      1   

    #define BIT_STR_MULTI_DA_REF_CK_SEL     14
    #define BIT_NUM_MULTI_DA_REF_CK_SEL     1 

    #define BIT_STR_MULTI_AO_LOOP_SEL       15
    #define BIT_NUM_MULTI_AO_LOOP_SEL       1 

//Multiple Line In Hardware Configuration
#define REGENV_AIN_ACK_CFG_MULTI    (AUD_REG_ENV_BASE + 0x3F0)
    #define BIT_STR_MULTI_CHNUM             0 //(0 - 1)
    #define BIT_NUM_MULTI_CHNUM             2 //
        #define MULTI_2CH   0
        #define MULTI_4CH   1
        #define MULTI_6CH   2
        #define MULTI_8CH   3
        
    #define BIT_STR_MULTI_DOWN              2 //(2 - 3)
    #define BIT_NUM_MULTI_DOWN              2 //
        #define MULTI_DW_NONE   0
        #define MULTI_DW_2      1
        #define MULTI_DW_4      2
        #define MULTI_DW_8      3

    #define BIT_STR_MULTI_DEEPER_MODE       4 //(4)
    #define BIT_NUM_MULTI_DEEPER_MODE       1 //

    #define BIT_STR_MULTI_ADDR_UPDATE       5 //(5)
    #define BIT_NUM_MULTI_ADDR_UPDATE       1 //How to update DRAM address

    #define BIT_STR_MULTI_INTR_SEL          6 //(6)
    #define BIT_NUM_MULTI_INTR_SEL          1 //    

    #define BIT_STR_MULTI_SPL_BNUM          8 //(8 - 12)
    #define BIT_NUM_MULTI_SPL_BNUM          5 //  

    #define BIT_STR_MULTI_LEFT_ALN          13//(13)
    #define BIT_NUM_MULTI_LEFT_ALN          1 // 

    #define BIT_STR_MULTI_DAT_DLY           14//(14)
    #define BIT_NUM_MULTI_DAT_DLY           1 // 

    #define BIT_STR_MULTI_INV_LRCL          15//(15)
    #define BIT_NUM_MULTI_INV_LRCL          1 // 

    #define BIT_STR_MULTI_LRCK_CYCLE_SEL    16//(16 - 17)
    #define BIT_NUM_MULTI_LRCK_CYCLE_SEL    2 //

    #define BIT_STR_MULTI_HBR_MODE          18//(18)
    #define BIT_NUM_MULTI_HBR_MODE          1 //
        //0: not HBR mode (L0L1L2L3R0R1R2R3), 1: HBR mode (L0R0L1R1L2R2L3R3)

    #define BIT_STR_MULTI_NON_CMPT_MODE     19//(19)
    #define BIT_NUM_MULTI_NON_CMPT_MODE     1 //

    #define BIT_STR_MULTI_DSD_MODE          20//(20)
    #define BIT_NUM_MULTI_DSD_MODE          1 //Multi-channel line-in  received data format
        //0: PCM mode .  1: DSD mode

    #define BIT_STR_MULTI_AOUT1_MULTI_DSD_MODE  21//(21)
    #define BIT_NUM_MULTI_AOUT1_MULTI_DSD_MODE  1 //Set 1 to make AOUT1 output multi-channel line-in DSD format

    #define BIT_STR_MULTI_AOUT2_MULTI_DSD_MODE  22//(22)
    #define BIT_NUM_MULTI_AOUT2_MULTI_DSD_MODE  1 //Set 1 to make AOUT2 output multi-channel line-in DSD format

    #define BIT_STR_MULTI_INV_BCK           23//(23)
    #define BIT_NUM_MULTI_INV_BCK           1 //

    #define BIT_STR_MULTI_SYNC_ON           24//(24)
    #define BIT_NUM_MULTI_SYNC_ON           1 //option to avoid L/R inverse problem

//Multi-CH SPDIF Type Detection
#define REGENV_MULTI_SPDF_TYPE      (AUD_REG_ENV_BASE + 0x3F4)
    #define BIT_STR_MULTI_DETAIL            0 //(0 - 4)
    #define BIT_NUM_MULTI_DETAIL            5 //Detail type for IEC61937 RAW data

    #define BIT_STR_MULTI_BSNUM             5 //(5 - 7)
    #define BIT_NUM_MULTI_BSNUM             3 //Bit stream number for IEC61937 RAW data   

    #define BIT_STR_MULTI_ROUGH             8 //(8 - 9)
    #define BIT_NUM_MULTI_ROUGH             2 //Rough type of the SPDIF input bit stream
        #define MULTI_PCM           0
        #define MULTI_RAW           1
        #define MULTI_16BIT_DTSCD   2
        #define MULTI_14BIT_DTSCD   3

    #define BIT_STR_MULTI_DEC             10//(10)
    #define BIT_NUM_MULTI_DEC             1 // SPDIF bit stream type decided or not.
        //0: The bit stream type is not decided yet.
        //1: The bit stream type is decided. Detail information is in bits 9~0.

//HDMI Sample Frequency detection
#define REGENV_HDMI_FREQ_DET        (AUD_REG_ENV_BASE + 0x3F8)
    #define BIT_STR_HDMI_LRCK_DIV           0 //(0 - 12)
    #define BIT_NUM_HDMI_LRCK_DIV           13// These registers represent that a HDMI LRCK cycle equals to how many number of 27MHz clock cycles  


#ifdef __cplusplus
        }
#endif
                            
#endif // _AUD_REG_ENV_H
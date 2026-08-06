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
 * @file aud_reg_rgbk2.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_RGBK2_H
#define _AUD_REG_RGBK2_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif



#define AUD_REG_ENV_BASE2           (0xA8000)

//pwm top configuration
#define REGENV_PWMTOP_CFG       (AUD_REG_ENV_BASE2 + 0x000)
    #define BIT_STR_PWM_FRNT_APLL_MUX       0 //(0)
    #define BIT_NUM_PWM_FRNT_APLL_MUX       1 //
        #define PWM_APLL1       0
        #define PWM_APLL2       1

    #define BIT_STR_PWM_REAR_APLL_MUX       4 //(4)
    #define BIT_NUM_PWM_REAR_APLL_MUX       1 //


    #define BIT_STR_FRNT_PWM_IN_SEL         8 //(8 - 9)
    #define BIT_NUM_FRNT_PWM_IN_SEL         2 //pwm frnt in selection
        #define PWM_SEL_AOUT1       0
        #define PWM_SEL_AOUT2       1
        #define PWM_SEL_DVP         2
        #define PWM_SEL_DVP2        3
    
    #define BIT_STR_REAR_PWM_IN_SEL         10//(10 - 11)
    #define BIT_NUM_REAR_PWM_IN_SEL         2 //pwm rear in selection

    #define BIT_STR_PWMIPG1_FIFO_EN         24//(24 - 25)
    #define BIT_NUM_PWMIPG1_FIFO_EN         2 //

    #define BIT_STR_PWMIPG2_FIFO_EN         26//(26 - 27)
    #define BIT_NUM_PWMIPG2_FIFO_EN         2 //

    #define BIT_STR_PWMIPG3_FIFO_EN         28//(28 - 29)
    #define BIT_NUM_PWMIPG3_FIFO_EN         2 // 

    #define BIT_STR_PWMIPG4_FIFO_EN         30//(30 - 31)
    #define BIT_NUM_PWMIPG5_FIFO_EN         2 // 


//frnt pwm anaif configure
#define REGENV_FRNT_ANAIF_CFG       (AUD_REG_ENV_BASE2 + 0x004)

//pwm DP pad mux configure
#define REGENV_PWMIP_PAD_DP_CFG     (AUD_REG_ENV_BASE2 + 0x008)

//pwmIP g1(frnt pwm LR) cfg0
#define REGENV_PWMCFG0_PG1          (AUD_REG_ENV_BASE2 + 0x00C)

//pwmIP g2(frnt pwm SLR) cfg0
#define REGENV_PWMCFG0_PG2          (AUD_REG_ENV_BASE2 + 0x010)

//pwmIP g3(frnt pwm clfe) cfg0
#define REGENV_PWMCFG0_PG3          (AUD_REG_ENV_BASE2 + 0x014)

//pwmIP g4(rear pwm LR) cfg0
#define REGENV_PWMCFG0_PG4          (AUD_REG_ENV_BASE2 + 0x018)

//pwmIP abist channel select
#define REGENV_PWMIP_ABIST_CH       (AUD_REG_ENV_BASE2 + 0x01C)

//pwm IP monitor selection
#define REGENV_PWMIP_MON_SEL        (AUD_REG_ENV_BASE2 + 0x020)

//pwm pad mux cfg
#define REGENV_PWMIP_PAD_CFG        (AUD_REG_ENV_BASE2 + 0x024)

//pwm IP g1(frnt LR) misc0
#define REGENV_PWMIP_MISC0_PG1      (AUD_REG_ENV_BASE2 + 0x028)

//pwm IP g2(frnt SLR) misc0
#define REGENV_PWMIP_MISC0_PG2      (AUD_REG_ENV_BASE2 + 0x02C)

//pwm IP g3(frnt CLFE) misc0
#define REGENV_PWMIP_MISC0_PG3      (AUD_REG_ENV_BASE2 + 0x030)

//pwm IP g4(rear LR) misc0
#define REGENV_PWMIP_MISC0_PG4      (AUD_REG_ENV_BASE2 + 0x034)

//pwmIP misc1
#define REGENV_PWMIP_MISC1          (AUD_REG_ENV_BASE2 + 0x038)

//pwm IPg1 monitor
#define REGENV_PWMIP_MON_PG1        (AUD_REG_ENV_BASE2 + 0x03C)

//pwm IPg2 monitor
#define REGENV_PWMIP_MON_PG2        (AUD_REG_ENV_BASE2 + 0x040)

//pwm IPg3 monitor
#define REGENV_PWMIP_MON_PG3        (AUD_REG_ENV_BASE2 + 0x044)

//pwm IPg4 monitor
#define REGENV_PWMIP_MON_PG4        (AUD_REG_ENV_BASE2 + 0x048)

//pwm IPg1 IDD monitor
#define REGENV_PWMIP_IDD_MON_PG1    (AUD_REG_ENV_BASE2 + 0x04C)

//pwm IPg2 IDD monitor
#define REGENV_PWMIP_IDD_MON_PG2    (AUD_REG_ENV_BASE2 + 0x050)

//pwm IPg3 IDD monitor
#define REGENV_PWMIP_IDD_MON_PG3    (AUD_REG_ENV_BASE2 + 0x054)

//pwm IPg4 IDD monitor
#define REGENV_PWMIP_IDD_MON_PG4    (AUD_REG_ENV_BASE2 + 0x058)

//AOUT ARM control
#define REGENV_AOUT_ARM_CTRL        (AUD_REG_ENV_BASE2 + 0x05C)
    #define BIT_STR_AP_AOUT_ARM_CTRL_CFG    0 //(0 - 7)
    #define BIT_NUM_AP_AOUT_ARM_CTRL_CFG    8 //EQ 'h36 enable arm contorl

//rgbk2 configuration0
#define REGENV_RGBK2_CFG0           (AUD_REG_ENV_BASE2 + 0x080)
    #define BIT_STR_GPS_APLL_SEL            7 //(7)
    #define BIT_NUM_GPS_APLL_SEL            1 //
    
//rgbk2 configuration1
#define REGENV_RGBK2_CFG1           (AUD_REG_ENV_BASE2 + 0x084)
    #define BIT_STR_MIC_IN_DSPC_CTRL_EN     0 //(0)
    #define BIT_NUM_MIC_IN_DSPC_CTRL_EN     1 //

    #define BIT_STR_MIC_IN_ARM_CTRL_EN      1 //(1)
    #define BIT_NUM_MIC_IN_ARM_CTRL_EN      1 //

    #define BIT_STR_SPDF_DVD_SEL            8 //(8)
    #define BIT_NUM_SPDF_DVD_SEL            1 //spdf out selection 
        #define SPIDF_AP    0
        #define SPDIF_DVD   1

    #define BIT_STR_CLK_AXI_PDN             9 //(9)
    #define BIT_NUM_CLK_AXI_PDN             1 //clk_axi power down

    #define BIT_STR_STC_BCK_PDN             10//(10)
    #define BIT_NUM_STC_BCK_PDN             1 //stc bck power down

    #define BIT_STR_DSP2BIM_CLK_PDN         11//(11)
    #define BIT_NUM_DSP2BIM_CLK_PDN         1 //dsp2bim dsp clock power down

    #define BIT_STR_MPHONE_CLK_PDN          12//(12)
    #define BIT_NUM_MPHONE_CLK_PDN          1 //mphone mclk clock power down

    #define BIT_STR_MLIN_CLK_PDN            13//(13)
    #define BIT_NUM_MLIN_CLK_PDN            1 //mline in clock power down   

    #define BIT_STR_IEC_CLK_PDN             14//(14)
    #define BIT_NUM_IEC_CLK_PDN             1 //iec dram clock power dow

    #define BIT_STR_APLL_ADJ_CLK_PDN        16//(16)
    #define BIT_NUM_APLL_ADJ_CLK_PDN        1 //apll_adj aplll clocksource power down

    #define BIT_STR_APLL2_ADJ_CLK_PDN       17//(17)
    #define BIT_NUM_APLL2_ADJ_CLK_PDN       1 //apll_adj apll2 clocksource power down

    #define BIT_STR_AFE1_26M_CLK_PDN        18//(18)
    #define BIT_NUM_AFE1_26M_CLK_PDN        1 //afe1 26m clock power down
    
    #define BIT_STR_AFE2_26M_CLK_PDN        19//(19)
    #define BIT_NUM_AFE2_26M_CLK_PDN        1 //afe2 26m clock power down

//rgbk2 configuration2
#define REGENV_RGBK2_CFG2           (AUD_REG_ENV_BASE2 + 0x088)
    #define BIT_STR_MPBUF1_SADR             0 //(0 - 15)
    #define BIT_NUM_MPBUF1_SADR             16//

    #define BIT_STR_MPBUF2_SADR             16//(16 - 31)
    #define BIT_NUM_MPBUF2_SADR             16//

//rgbk2 configuration3
#define REGENV_RGBK2_CFG3           (AUD_REG_ENV_BASE2 + 0x08C)
    #define BIT_STR_MPBUF3_SADR             0 //(0 - 15)
    #define BIT_NUM_MPBUF3_SADR             16//
    
    #define BIT_STR_MPHONE_BUF_EN           16//(16)
    #define BIT_NUM_MPHONE_BUF_EN           1 //

    #define BIT_STR_AFE_27M_ECO_CFG         26//(26 - 31)
    #define BIT_NUM_AFE_27M_ECO_CFG         6 //

//rgbk2 configuration4
#define REGENV_RGBK2_CFG4           (AUD_REG_ENV_BASE2 + 0x090)
    #define BIT_STR_MLIN_BCK_DIV            0 //(0 - 3)
    #define BIT_NUM_MLIN_BCK_DIV            4 //internal mlin bck divider when mlinckgen selected

    #define BIT_STR_SEL_MLINCKGEN           4 //(4)
    #define BIT_NUM_SEL_MLINCKGEN           1 //

    #define BIT_STR_MPHONE_BANK             8 //(8)
    #define BIT_NUM_MPHONE_BANK             11//micphone arm control dram bank

    #define BIT_STR_PCM_DRAM_BANK           20//(20 - 30)
    #define BIT_NUM_PCM_DRAM_BANK           11//BT_PCM dram bank address(<<20 to byte address ) 
 
//rgbk2 configuration5
#define REGENV_RGBK2_CFG5           (AUD_REG_ENV_BASE2 + 0x094)
    #define BIT_STR_PCM_LOOP_MODE           0 //(0)
    #define BIT_NUM_PCM_LOOP_MODE           1 //pcm loopback mode(pcm tx to pcm rx)

    #define BIT_STR_AO2_INT_CLR             1 //(1)
    #define BIT_NUM_AO2_INT_CLR             1 //audout2 interrupt clear 

    #define BIT_STR_AO_INT_CLR              2 //(2)
    #define BIT_NUM_AO_INT_CLR              1 //audout interrupt clear 

    #define BIT_STR_ASRC_CALI_SIG4_SEL      6 //(6 - 7)
    #define BIT_NUM_ASRC_CALI_SIG4_SEL      2 //asrc cali sig4 selection 

    #define BIT_STR_ASRC_CALI_SIG3_SEL      8 //(8 - 9)
    #define BIT_NUM_ASRC_CALI_SIG3_SEL      2 //asrc cali sig3 selection 

    #define BIT_STR_ASRC_CALI_SIG2_SEL      10 //(10 - 11)
    #define BIT_NUM_ASRC_CALI_SIG2_SEL      2 //asrc cali sig2 selection 

    #define BIT_STR_ASRC_CALI_SIG1_SEL      12 //(12 - 13)
    #define BIT_NUM_ASRC_CALI_SIG1_SEL      2 //asrc cali sig1 selection

    #define BIT_STR_ASRC_CALI_SIG0_SEL      14 //(14 - 15)
    #define BIT_NUM_ASRC_CALI_SIG0_SEL      2 //asrc cali sig0 selection

    #define BIT_STR_ARM_MIC_BLK6            16 //(16 - 31)
    #define BIT_NUM_ARM_MIC_BLK6            16//micphone arm control dram blk6

//rgbk2 aout configuration2
#define REGENV_RGBK2_AOUT_CFG2      (AUD_REG_ENV_BASE2 + 0x0C8)
    #define BIT_STR_AP_ASRC_ARM_CTRL_CFG    0 //(0 - 7)
    #define BIT_NUM_AP_ASRC_ARM_CTRL_CFG    8 //EQ 'h36 enable arm contorl

    #define BIT_STR_AP_AOUT2_ARM_CTRL_CFG   8 //(8 - 15)
    #define BIT_NUM_AP_AOUT2_ARM_CTRL_CFG   8 //EQ 'h36 enable arm contorl

    #define BIT_STR_FRNT_SRC_SEL            16 //(16 - 17)
    #define BIT_NUM_FRNT_SRC_SEL            2 //frnt sout select
        #define SOUT_AOUT1      0
        #define SOUT_AOUT2      1
        #define SOUT_DVP        2
        #define SOUT_DVP2       3

    #define BIT_STR_REAR_SRC_SEL            18 //(18 - 19)
    #define BIT_NUM_REAR_SRC_SEL            2 //rear sout select

    #define BIT_STR_FRNT_SFT_MUTE           20 //(20)
    #define BIT_NUM_FRNT_SFT_MUTE           1 //frnt software mute

    #define BIT_STR_REAR_SFT_MUTE           21 //(21)
    #define BIT_NUM_REAR_SFT_MUTE           1 //rear software mute

    #define BIT_STR_GPS_SFT_MUTE            22 //(22)
    #define BIT_NUM_GPS_SFT_MUTE            1 //gps software mute

    #define BIT_STR_I2S_IN1_MASTER          23 //(23)
    #define BIT_NUM_I2S_IN1_MASTER          1 //

    #define BIT_STR_I2S_IN2_MASTER          24 //(24)
    #define BIT_NUM_I2S_IN2_MASTER          1 //

//afe top configuration 0
#define REGENV_AFE_TOP_CFG0         (AUD_REG_ENV_BASE2 + 0x0D0)
    #define BIT_STR_AFEXX_MIC_SEL           0 //(0)
    #define BIT_NUM_AFEXX_MIC_SEL           1 //digital mic fifo timing selection 
        //0:from afe1, 1:from afe2

    #define BIT_STR_MPH_16BIT_MODE          1 //(1)
    #define BIT_NUM_MPH_16BIT_MODE          1 //mphone 16bit mode selection
        // 0:24bit, 1:mphone in 16bit compact mode to DRAM

    #define BIT_STR_AFE1_BCK_SEL            2 //(2 - 3)
    #define BIT_NUM_AFE1_BCK_SEL            2 //afe1 ff clock selectionneed program same as afe1_ff_dsel  
        #define AFE_SEL_CLK_AOUT1   0
        #define AFE_SEL_CLK_AOUT2   1
        #define AFE_SEL_CLK_LINEIN  2
        #define AFE_SEL_CLK_MPH     3

    #define BIT_STR_AFE2_BCK_SEL            4 //(4 - 5)
    #define BIT_NUM_AFE2_BCK_SEL            2 //afe2 ff clock selectionneed program same as afe1_ff_dsel  

    #define BIT_STR_MIC_AFEX_SEL            6 //(6)
    #define BIT_NUM_MIC_AFEX_SEL            1 //mic in AFE selection 
        //0:AFE1, 1:AFE2

    #define BIT_STR_LIN_AFEX_SEL            7 //(7)
    #define BIT_NUM_LIN_AFEX_SEL            1 //line in AFE selection    

    #define BIT_STR_AOUT1_BYPA_ADC          8 //(8)
    #define BIT_NUM_AOUT1_BYPA_ADC          1 //aout1 bypass mode select
        //0:aout1 source from DRAM, 1:aout1 source from ADC

    #define BIT_STR_AOUT2_BYPA_ADC          9 //(9)
    #define BIT_NUM_AOUT2_BYPA_ADC          1 //aout2 bypass mode select
        //0:aout2 source from DRAM, 1:aout2 source from ADC

    #define BIT_STR_MIC_USE_AO2_GIM         13//(13)
    #define BIT_NUM_MIC_USE_AO2_GIM         1 //mph in clock selection
        //0: normal, 1: mic_use ao2 tim
    
    #define BIT_STR_LIN_USE_AO2_GIM         14//(14)
    #define BIT_NUM_LIN_USE_AO2_GIM         1 //line in clock selection
        //0: normal, 1:line in use ao2 tim

    #define BIT_STR_AFE1_LRCK_SEL           16//(16 - 17)
    #define BIT_NUM_AFE1_LRCK_SEL           2 //afe1 ff clock selectionneed program same as afe1_ff_dsel  

    #define BIT_STR_AFE2_LRCK_SEL           18//(18 - 19)
    #define BIT_NUM_AFE2_LRCK_SEL           2 //afe2 ff clock selectionneed program same as afe1_ff_dsel 

    #define BIT_STR_AFE1_CLK_USE_LIN2       20//(20 - 21)
    #define BIT_NUM_AFE1_CLK_USE_LIN2       2 //

    #define BIT_STR_AFE2_CLK_USE_LIN2       22//(20 - 21)
    #define BIT_NUM_AFE2_CLK_USE_LIN2       2 //

    #define BIT_STR_LIN2_AFE_MODE           24 //(4)
    #define BIT_NUM_LIN2_AFE_MODE           1  //line2 source in selection

    #define BIT_STR_LIN2_DFMT_SEL           25 //(25)
    #define BIT_NUM_LIN2_DFMT_SEL           1  //line2 in data format selection 

    #define BIT_STR_LIN2_WE_SEL             26 //(26)
    #define BIT_NUM_LIN2_WE_SEL             1 //line in2 confige

    #define BIT_STR_LIN2_AFEX_SEL           28 //(28)
    #define BIT_NUM_LIN2_AFEX_SEL           1 //line2 in AFE selection 
    
//bypasss volume control configuration 0
#define REGENV_BYPS_VLUM_CFG0       (AUD_REG_ENV_BASE2 + 0x0D4)
    #define BIT_STR_ADC_BYPS_VOLUME         0 //(0 - 23)
    #define BIT_NUM_ADC_BYPS_VOLUME         24//adc bypass volume value   

    #define BIT_STR_ADC_BYPS_VOLUME_EN      24//(24)
    #define BIT_NUM_ADC_BYPS_VOLUME_EN      1 //adc bypass volume control enable

    #define BIT_STR_ADC_BYPS_FDIN           25//(25)
    #define BIT_NUM_ADC_BYPS_FDIN           1 //adc bypass fade in enable

    #define BIT_STR_ADC_BYPS_FDOUT          26//(26)
    #define BIT_NUM_ADC_BYPS_FDOUT          1 //adc bypass fade out enable

    #define BIT_STR_ADC_BYPS_MODE_SEL       27//(27)
    #define BIT_NUM_ADC_BYPS_MODE_SEL       1 //adc bypass mode selection
        //0:section liner, 1:liner

    #define BIT_STR_VOLUME_SRC_SEL          28//(28)
    #define BIT_NUM_VOLUME_SRC_SEL          1 //volume source selection
        //0: from AFE1,  0: from AFE2

//bypasss volume control configuration 1
#define REGENV_BYPS_VLUM_CFG1       (AUD_REG_ENV_BASE2 + 0x0D8)
    #define BIT_STR_ADC_BYPA_SCALE          0 //(0 - 3)
    #define BIT_NUM_ADC_BYPA_SCALE          4 //how many samples a step for fade in/out

    #define BIT_STR_LIN_AFE_MODE            4 //(4)
    #define BIT_NUM_LIN_AFE_MODE            1 //line source in selection
        #define LIN_EXT_ADC     0
        #define LIN_INT_ADC     1
        //0:external ADC, 1:internal ADC

    #define BIT_STR_LIN_WE_SEL              5 //(5)
    #define BIT_NUM_LIN_WE_SEL              1 //line in confige  
    
    #define BIT_STR_LIN_LRDO_SWAP           6 //(6)
    #define BIT_NUM_LIN_LRDO_SWAP           1 //line in to volume control lr swap

    #define BIT_STR_LIN_DFMT_SEL            7 //(7)
    #define BIT_NUM_LIN_DFMT_SEL            1 //line in data format selection
        //0:16bits left alignment, 1:16bits right alignment

    #define BIT_STR_MPH_AFE_MODE            8 //(8)
    #define BIT_NUM_MPH_AFE_MODE            1 //micphone source in selection
        //0:external ADC, 1:internal ADC

    #define BIT_STR_MPH_WE_SEL              9 //(9)
    #define BIT_NUM_MPH_WE_SEL              1 //micphone in confige

    #define BIT_STR_MPH_LRDO_SWAP           10//(10)
    #define BIT_NUM_MPH_LRDO_SWAP           1 //micphone in to volume control lr swap

    #define BIT_STR_LIN_AFE2SPDIF           11//(11)
    #define BIT_NUM_LIN_AFE2SPDIF           1 //adc test mode for debug 

    #define BIT_STR_LIN2_AFE2SPDIF          11//(12)
    #define BIT_NUM_LIN2_AFE2SPDIF          1 //adc test mode for debug 

//bt_pcm block address
#define REGENV_BT_PCM_BLK_CFG       (AUD_REG_ENV_BASE2 + 0x0E0)
    #define BIT_STR_PCM_RX_DRAM_BLK         0 //(0 - 15)
    #define BIT_NUM_PCM_RX_DRAM_BLK         16//BT_PCM RX block address(<<8 t o byte address)

    #define BIT_STR_PCM_TX_DRAM_BLK         16//(16 - 31)
    #define BIT_NUM_PCM_TX_DRAM_BLK         16//BT_PCM TX block address(<<8 t o byte address)

//apll adjust configuration 0
#define REGENV_APLL_ADJ_CFG0        (AUD_REG_ENV_BASE2 + 0x01E0)
    #define BIT_STR_PH_RANGE                0 //(0 - 15)
    #define BIT_NUM_PH_RANGE                16//the pahse range, when phase diff meet to this range will intrrupt   

    #define BIT_STR_PH_MIDDLE               16//(16 - 31)
    #define BIT_NUM_PH_MIDDLE               16//the midle of 48k apllclk phase

//apll adjust configuration 1
#define REGENV_APLL_ADJ_CFG1        (AUD_REG_ENV_BASE2 + 0x01E4)
    #define BIT_STR_APLL_ADJ_EN             0 //(0)
    #define BIT_NUM_APLL_ADJ_EN             1 //

    #define BIT_STR_APLL_ADJ_CNT_RST        1 //(1)
    #define BIT_NUM_APLL_ADJ_CNT_RST        1 //soft reset

    #define BIT_STR_APLL_ADJ_INTR_CFG       2 //(2 - 3)
    #define BIT_NUM_APLL_ADJ_INTR_CFG       2 //interrupt config

    #define BIT_STR_APLL_ADJ_TIMER          8 //(8 - 15)
    #define BIT_NUM_APLL_ADJ_TIMER          8 //when out of phase range will generator interrupt by this time unit 48k
   
    #define BIT_STR_APLL_ADJ_MODE2_TIMER    16//(16 - 27)
    #define BIT_NUM_APLL_ADJ_MODE2_TIMER    27//timer for recoder apll2 conter period

    #define BIT_STR_APLL2_ADJ_CLK_SEL       28//(28)
    #define BIT_NUM_APLL2_ADJ_CLK_SEL       1 //apll clock select 

//apll adjust status 0
#define REGENV_APLL_ADJ_STATUS0     (AUD_REG_ENV_BASE2 + 0x01E8)
    #define BIT_STR_PHASE_LATCH             0 //(0 - 15)
    #define BIT_NUM_PHASE_LATCH             16//phase latched value

    #define BIT_STR_APLL_PH_DIRECTORY       16//(16)
    #define BIT_NUM_APLL_PH_DIRECTORY       1 //apll phdiff directory 
        //0: means apll fast, 1: means 26Mclk fast

    #define BIT_STR_OUT_OF_RANGE            17//(17)
    #define BIT_NUM_OUT_OF_RANGE            1 //out of range status
        //0: in range, 1: out of range

//apll adjust status 1
#define REGENV_APLL_ADJ_STATUS1     (AUD_REG_ENV_BASE2 + 0x01EC)    
    #define BIT_STR_APLL_8K_CNT             0 //(0 - 7)
    #define BIT_NUM_APLL_8K_CNT             8 //APLL 8k counter

    #define BIT_STR_AFE_8K_CNT              16//(16 - 23)
    #define BIT_NUM_AFE_8K_CNT              8 //AFE 8k count

//apll adjust status 2
#define REGENV_APLL_ADJ_STATUS2     (AUD_REG_ENV_BASE2 + 0x01F0)     
    #define BIT_STR_APLL2_COUNT             0 //(0 - 31)
    #define BIT_NUM_APLL2_COUNT             32//APLL2_cnt

//indrect pwmIP g1 address
#define REGENV_PWMIP_NDRECT_ADDR_G1 (AUD_REG_ENV_BASE2 + 0x0240) 
//indrect pwmIP g1 data
#define REGENV_PWMIP_NDRECT_DATA_G1 (AUD_REG_ENV_BASE2 + 0x0244) 

#define REGENV_PWMIP_NDRECT_ADDR_G2 (AUD_REG_ENV_BASE2 + 0x0248) 
#define REGENV_PWMIP_NDRECT_DATA_G2 (AUD_REG_ENV_BASE2 + 0x024C)

#define REGENV_PWMIP_NDRECT_ADDR_G3 (AUD_REG_ENV_BASE2 + 0x0250) 
#define REGENV_PWMIP_NDRECT_DATA_G3 (AUD_REG_ENV_BASE2 + 0x0254)

#define REGENV_PWMIP_NDRECT_ADDR_G4 (AUD_REG_ENV_BASE2 + 0x0258) 
#define REGENV_PWMIP_NDRECT_DATA_G4 (AUD_REG_ENV_BASE2 + 0x025C) 

//afe1 configuration0
#define REGENV_AFE1_CFG0            (AUD_REG_ENV_BASE2 + 0x0284)
#define REGENV_AFE1_CFG19           (AUD_REG_ENV_BASE2 + 0x02d0)
    #define BIT_STR_CH2_MANU_GAIN_SEL   15
    #define BIT_NUM_CH2_MANU_GAIN_SEL   1
        #define AUTO_GAIN   0
        #define MANU_GAIN   1

    #define BIT_STR_CH1_MANU_GAIN_SEL   16
    #define BIT_NUM_CH1_MANU_GAIN_SEL   1

    #define BIT_STR_CH2_MANU_GAIN       17
    #define BIT_NUM_CH2_MANU_GAIN       6

    #define BIT_STR_CH1_MANU_GAIN       23
    #define BIT_NUM_CH1_MANU_GAIN       6
    
#define REGENV_AFE1_CFG20           (AUD_REG_ENV_BASE2 + 0x02d4)
    #define BIT_STR_MCU_AFE_ON      0
    #define BIT_NUM_MCU_AFE_ON      1

    #define BIT_STR_MCU_UL_SRC_ON   6
    #define BIT_NUM_MCU_UL_SRC_ON   1

    #define BIT_STR_MCU_UL_VOICE_MODE_CH1   14
    #define BIT_NUM_MCU_UL_VOICE_MODE_CH1   2

    #define BIT_STR_MCU_UL_VOICE_MODE_CH2   16
    #define BIT_NUM_MCU_UL_VOICE_MODE_CH2   2

    #define BIT_STR_AFE_FS_SEL              21
    #define BIT_NUM_AFE_FS_SEL              2

    #define BIT_STR_ANA_FIFO_CS             24
    #define BIT_NUM_ANA_FIFO_CS             1

    #define BIT_STR_MPH_FIFO_CS             25
    #define BIT_NUM_MPH_FIFO_CS             1

//........
#define REGENV_AFE1_CFG22           (AUD_REG_ENV_BASE2 + 0x02DC)

//afe2 configuration0
#define REGENV_AFE2_CFG0            (AUD_REG_ENV_BASE2 + 0x0304)
#define REGENV_AFE2_CFG19           (AUD_REG_ENV_BASE2 + 0x0350)
#define REGENV_AFE2_CFG20           (AUD_REG_ENV_BASE2 + 0x0354)


//........
#define REGENV_AFE2_CFG22           (AUD_REG_ENV_BASE2 + 0x035C)

//channel1 buffer start ADR
#define REGENV_AOUT1_CH1_BUF_SADR   (AUD_REG_ENV_BASE2 + 0x0400)
    #define BIT_STR_AOUT1_CH1_BUF_SADR      0 //(0 - 19)
    #define BIT_NUM_AOUT1_CH1_BUF_SADR      20//channel 1 buffer start address

//channel1 buffer size
#define REGENV_AOUT1_CH1_BUF_SIZE   (AUD_REG_ENV_BASE2 + 0x0404)   
    #define BIT_STR_AOUT1_CH1_BUF_SIZE      0 //(0 - 19)
    #define BIT_NUM_AOUT1_CH1_BUF_SIZE      20//channel 1 buffer size register

//Channel1 next start ADR
#define REGENV_AOUT1_CH1_NSADR      (AUD_REG_ENV_BASE2 + 0x0408)   
    #define BIT_STR_AOUT1_CH_NSADR          0 //(0 - 19)
    #define BIT_NUM_AOUT1_CH_NSADR          20//set the next start address for channeL

#define REGENV_AOUT1_CH2_NSADR      (AUD_REG_ENV_BASE2 + 0x040C)
#define REGENV_AOUT1_CH3_NSADR      (AUD_REG_ENV_BASE2 + 0x0410)
#define REGENV_AOUT1_CH4_NSADR      (AUD_REG_ENV_BASE2 + 0x0414)
#define REGENV_AOUT1_CH5_NSADR      (AUD_REG_ENV_BASE2 + 0x0418)
#define REGENV_AOUT1_CH6_NSADR      (AUD_REG_ENV_BASE2 + 0x041C)
#define REGENV_AOUT1_CH7_NSADR      (AUD_REG_ENV_BASE2 + 0x0420)
#define REGENV_AOUT1_CH8_NSADR      (AUD_REG_ENV_BASE2 + 0x0424)
#define REGENV_AOUT1_CH9_NSADR      (AUD_REG_ENV_BASE2 + 0x0428)
#define REGENV_AOUT1_CH10_NSADR     (AUD_REG_ENV_BASE2 + 0x042C)
#define REGENV_AOUT1_CH11_NSADR     (AUD_REG_ENV_BASE2 + 0x0430)
#define REGENV_AOUT1_CH12_NSADR     (AUD_REG_ENV_BASE2 + 0x0434)

//AOUT next sample number
#define REGENV_AOUT1_NSNUM          (AUD_REG_ENV_BASE2 + 0x0438)
    #define BIT_STR_AOUT1_NSNUM             8 //(8 - 23)
    #define BIT_NUM_AOUT1_NSNUM             16//next audio output sample number per channel

//AOUT interrupt generate size
#define REGENV_AOUT1_INTRSIZE       (AUD_REG_ENV_BASE2 + 0x043C)
    #define BIT_STR_AOUT1_INTRSIZE          8 //(8 - 23)
    #define BIT_NUM_AOUT1_INTRSIZE          16//generating interrupt when how many samples remained to be send out
    
//AOUT control register
#define REGENV_AOUT1_CTRL           (AUD_REG_ENV_BASE2 + 0x0440)
    #define BIT_STR_AOUT1_EN_PRE            8 //(8)
    #define BIT_NUM_AOUT1_EN_PRE            1 //audio output enable

    #define BIT_STR_AOUT1_PAUSE_PRE         9 //(9)
    #define BIT_NUM_AOUT1_PAUSE_PRE         1 //audio output pause

//output channel source configuration0
#define REGENV_AOUT1_CH_CFG0        (AUD_REG_ENV_BASE2 + 0x0444)
    #define BIT_STR_AOUT1_L_CFG             8 //(8 - 11)
    #define BIT_NUM_AOUT1_L_CFG             4 //channel left
    
    #define BIT_STR_AOUT1_R_CFG             12 //(12 - 15)
    #define BIT_NUM_AOUT1_R_CFG             4 //channel right

    #define BIT_STR_AOUT1_C_CFG             16 //(16 - 19)
    #define BIT_NUM_AOUT1_C_CFG             4 //channle center

    #define BIT_STR_AOUT1_CH7_CFG           20 //(20 - 23)
    #define BIT_NUM_AOUT1_CH7_CFG           4 //channle 7

//output channel source configuration1
#define REGENV_AOUT1_CH_CFG1        (AUD_REG_ENV_BASE2 + 0x0448)
    #define BIT_STR_AOUT1_SL_CFG            8 //(8 - 11)
    #define BIT_NUM_AOUT1_SL_CFG            4 //channel surround left
        
    #define BIT_STR_AOUT1_SR_CFG            12 //(12 - 15)
    #define BIT_NUM_AOUT1_SR_CFG            4 //channel surround right
    
    #define BIT_STR_AOUT1_LFE_CFG           16 //(16 - 19)
    #define BIT_NUM_AOUT1_LFE_CFG           4 //channel lfe
    
    #define BIT_STR_AOUT1_CH8_CFG           20 //(20 - 23)
    #define BIT_NUM_AOUT1_CH8_CFG           4 //channle 8

//output channel source configuration2
#define REGENV_AOUT1_CH_CFG2        (AUD_REG_ENV_BASE2 + 0x044C)
    #define BIT_STR_AOUT1_CH9_CFG           8 //(8 - 11)
    #define BIT_NUM_AOUT1_CH9_CFG           4 //channel 9
            
    #define BIT_STR_AOUT1_CH10_CFG          12 //(12 - 15)
    #define BIT_NUM_AOUT1_CH10_CFG          4 //channel 10
        
    #define BIT_STR_AOUT1_CH11_CFG          16 //(16 - 19)
    #define BIT_NUM_AOUT1_CH11_CFG          4 //channel 11
        
    #define BIT_STR_AOUT1_CH12_CFG          20 //(20 - 23)
    #define BIT_NUM_AOUT1_CH12_CFG          4 //channel 12

//Channel number
#define REGENV_AOUT1_CH_NUM         (AUD_REG_ENV_BASE2 + 0x0450)
    #define BIT_STR_AOUT1_CH_NUM            8 //(8 - 11)
    #define BIT_NUM_AOUT1_CH_NUM            4 //of channels in DRAM source

//AOUT switch register
#define REGENV_AOUT1_SWITCH         (AUD_REG_ENV_BASE2 + 0x0454)

//LEV & DET configuration register
#define REGENV_AOUT1_LEV_DET_CFG    (AUD_REG_ENV_BASE2 + 0x0458) 
    #define BIT_STR_AOUT1_LEVEL_AVG_SEL     8 //(8 - 11)
    #define BIT_NUM_AOUT1_LEVEL_AVG_SEL     4 //of channels
    //0:left, 1:right, 2:sur left, 3:sur right, 4:cente, 5:lfe, 8:ch9, 9:ch10

    #define BIT_STR_AOUT1_AVG_SIGNAL_SEL    12 //(12)
    #define BIT_NUM_AOUT1_AVG_SIGNAL_SEL    1 // 
    //0:128 samples,  1:256 samples

//AOUT averager level
#define REGENV_AOUT1_LEV            (AUD_REG_ENV_BASE2 + 0x045C) 
    #define BIT_STR_AOUT1_LEVEL_AVG_LEV     0 //(0 - 23)
    #define BIT_NUM_AOUT1_LEVEL_AVG_LEV     24//

//Buffer data mixing configuration
#define REGENV_AOUT1_BUF_MIX_CFG    (AUD_REG_ENV_BASE2 + 0x0460) 
    #define BIT_STR_AOUT1_BUF_MIX_EN        8 //(8)
    #define BIT_NUM_AOUT1_BUF_MIX_EN        1 //buffer mix enable

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_LR  9 //(9)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_LR  1 //mixing data to left & right output

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_SLR 10 //(10)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_SLR 1 //mixing data to surround right & left output

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_C   11 //(11)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_C   1 //mixing data to center output

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_CH78    12 //(12)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_CH78    1 //mixing data to channel7 & channel8

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_CH910   13 //(13)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_CH910   1 //mixing data to channel9 & channel10

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_CH1112  14 //(14)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_CH1112  1 //mixing data to channel11 & channel12

    #define BIT_STR_AOUT1_MIX_BUFFER_EN_LFE     15 //(15)
    #define BIT_NUM_AOUT1_MIX_BUFFER_EN_LFE     1 //mixing data to subwoofer output

    #define BIT_STR_AOUT1_MIX_BUFFER_CFG        16 //(16)
    #define BIT_NUM_AOUT1_MIX_BUFFER_CFG        1 // 

//Buffer1 mix NSADR
#define REGENV_AOUT1_MIX2_NSADR     (AUD_REG_ENV_BASE2 + 0x0468)
    #define BIT_STR_AOUT1_MIX1_NSADR            8 //(8 - 23)
    #define BIT_NUM_AOUT1_MIX1_NSADR            16//buffer1 mix data next start address

    #define BIT_STR_AOUT1_MIX2_NSADR            8 //(8 - 23)
    #define BIT_NUM_AOUT1_MIX2_NSADR            16//buffer2 mix data next start address

//pcm control register
#define REGENV_PCM_CTRL             (AUD_REG_ENV_BASE2 + 0x04C4)
    #define BIT_STR_PCM_EN                      0 //(0)
    #define BIT_NUM_PCM_EN                      1 //

    #define BIT_STR_RX_EN                       1 //(1)
    #define BIT_NUM_RX_EN                       1 //

    #define BIT_STR_TX_EN                       2 //(2)
    #define BIT_NUM_TX_EN                       1 //

    #define BIT_STR_MODE_SEL                    3 //(3)
    #define BIT_NUM_MODE_SEL                    1 //
        //0 : master mode, 1 : slave mode

    #define BIT_STR_INV_PCM_CLK_IN              4 //(4)
    #define BIT_NUM_INV_PCM_CLK_IN              1 //invert input pcm_clk phase

    #define BIT_STR_INV_PCM_CLK_OUT             5 //(5)
    #define BIT_NUM_INV_PCM_CLK_OUT             1 //invert output pcm_clk phase

    #define BIT_STR_SYNC_CYCLE                  6 //(6)
    #define BIT_NUM_SYNC_CYCLE                  1 //pcm sync cycle select
        //0 : 32 pcm_clk cycle,  1 : 64 pcm_clk cycle

    #define BIT_STR_SYNC_MODE_SEL               7 //(6)
    #define BIT_NUM_SYNC_MODE_SEL               1 //pcm sync mode select                                                                                                                  
        //0 : 32 pcm_clk cycle  , 1 : 64 pcm_clk cycle

    #define BIT_STR_SYNC_LENGTH                 8 //(8 - 9)
    #define BIT_NUM_SYNC_LENGTH                 2 //pcm sync pulse width select : 
        //00 : 1 pcm_clk length pulse, 01 : 2 pcm_clk length pulse   
        //10 : 3 pcm_clk length pulse, 11 : 4 pcm_clk length pulse  

    #define BIT_STR_PCM_16BIT                   10//(10)
    #define BIT_NUM_PCM_16BIT                   1 //format select between linear and law.  
        //1 : linear 16bit     ,  0 : law 8bit
 
    #define BIT_STR_BIT_NUM_SEL                 11//(11 - 12)
    #define BIT_NUM_BIT_NUM_SEL                 2 //  pcm bit number select, when format is linear                                                                                                                                                            
        //00 : 16_bits,  01 : 15_bits,  10 : 14_bits, 11 : 13_bits

    #define BIT_STR_BIT_SIGN_EN                 13//(13)
    #define BIT_NUM_BIT_SIGN_EN                 1 // enable expand sign for linear format, MSB->LSB less than 16bits.
        //1: enable         ,   0: disable

    #define BIT_STR_BIT_DATA_ORDER              14//(14)
    #define BIT_NUM_BIT_DATA_ORDER              1 // during receive & transmiter ,bitstream order select.    
        //1 : LSB -> MSB , 0 : MSB -> LSB

//PCM RX DRAM start address
#define REGENV_PCMRX_DRAM_SADR      (AUD_REG_ENV_BASE2 + 0x04CC)
    #define BIT_STR_PCMRX_DRAM_SADR             0 //(0 - 19)
    #define BIT_NUM_PCMRX_DRAM_SADR             20//set dram start addr for BT in (128-bit align)

//PCM RX DRAM end address
#define REGENV_PCMRX_DRAM_EADR      (AUD_REG_ENV_BASE2 + 0x04D0)
    #define BIT_STR_PCMRX_DRAM_EADR             0 //(0 - 19)
    #define BIT_NUM_PCMRX_DRAM_EADR             20//set dram end addr for BT in (128-bit align)
    
//PCM TX DRAM start address
#define REGENV_PCMTX_DRAM_SADR      (AUD_REG_ENV_BASE2 + 0x04D8)
    #define BIT_STR_PCMTX_DRAM_SADR             0 //(0 - 19)
    #define BIT_NUM_PCMTX_DRAM_SADR             20//set dram start addr for BT out (128-bit align)
    
//PCM TX DRAM end address
#define REGENV_PCMTX_DRAM_EADR      (AUD_REG_ENV_BASE2 + 0x04DC)
    #define BIT_STR_PCMTX_DRAM_EADR             0 //(0 - 19)
    #define BIT_NUM_PCMTX_DRAM_EADR             20//set dram end addr for BT out(128-bit align)

//set dram next start addr for BT out
#define REGENV_PCMTX_DRAM_NSADR     (AUD_REG_ENV_BASE2 + 0x04E0)
    #define BIT_STR_PCM_TX_DRAM_NSADR           0 //(0 - 19)
    #define BIT_NUM_PCM_TX_DRAM_NSADR           20//set dram next start addr for BT out

//pcm tx intr num
#define REGENV_PCMTX_INTR           (AUD_REG_ENV_BASE2 + 0x04E8)
    #define BIT_STR_TX_SAMPLE_NUM               0 //(0 - 7)
    #define BIT_NUM_TX_SAMPLE_NUM               8 //pcm tx output data number and remain the value of this number

    #define BIT_STR_TX_INTR_RENUM               8 //(8 - 15)
    #define BIT_NUM_TX_INTR_RENUM               8 //BT out remain the value of sample to gen interrupt

//PCM RX write addr
#define REGENV_PCMRX_WRADR          (AUD_REG_ENV_BASE2 + 0x04EC)
    #define BIT_STR_RX_ADR_MON                  0 //(0 - 19)
    #define BIT_NUM_RX_ADR_MON                  20//monitor the BT in address    


//channel1 buffer start ADR
#define REGENV_AOUT2_CH1_BUF_SADR   (AUD_REG_ENV_BASE2 + 0x0500)
    #define BIT_STR_AOUT2_CH1_BUF_SADR      0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH1_BUF_SADR      20//channel 1 buffer start address
    
//channel1 buffer size
#define REGENV_AOUT2_CH1_BUF_SIZE   (AUD_REG_ENV_BASE2 + 0x0504)   
    #define BIT_STR_AOUT2_CH1_BUF_SIZE      0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH1_BUF_SIZE      20//channel 1 buffer size register
    
    //Channel1 next start ADR
#define REGENV_AOUT2_CH1_NSADR      (AUD_REG_ENV_BASE2 + 0x0508)   
    #define BIT_STR_AOUT2_CH_NSADR          0 //(0 - 19)
    #define BIT_NUM_AOUT2_CH_NSADR          20//set the next start address for channeL
    
#define REGENV_AOUT2_CH2_NSADR      (AUD_REG_ENV_BASE2 + 0x050C)
#define REGENV_AOUT2_CH3_NSADR      (AUD_REG_ENV_BASE2 + 0x0510)
#define REGENV_AOUT2_CH4_NSADR      (AUD_REG_ENV_BASE2 + 0x0514)
#define REGENV_AOUT2_CH5_NSADR      (AUD_REG_ENV_BASE2 + 0x0518)
#define REGENV_AOUT2_CH6_NSADR      (AUD_REG_ENV_BASE2 + 0x051C)
#define REGENV_AOUT2_CH7_NSADR      (AUD_REG_ENV_BASE2 + 0x0520)
#define REGENV_AOUT2_CH8_NSADR      (AUD_REG_ENV_BASE2 + 0x0524)
#define REGENV_AOUT2_CH9_NSADR      (AUD_REG_ENV_BASE2 + 0x0528)
#define REGENV_AOUT2_CH10_NSADR     (AUD_REG_ENV_BASE2 + 0x052C)
#define REGENV_AOUT2_CH11_NSADR     (AUD_REG_ENV_BASE2 + 0x0530)
#define REGENV_AOUT2_CH12_NSADR     (AUD_REG_ENV_BASE2 + 0x0534)
    
//AOUT next sample number
#define REGENV_AOUT2_NSNUM          (AUD_REG_ENV_BASE2 + 0x0538)
    #define BIT_STR_AOUT2_NSNUM             8 //(8 - 23)
    #define BIT_NUM_AOUT2_NSNUM             16//next audio output sample number per channel
    
    //AOUT interrupt generate size
#define REGENV_AOUT2_INTRSIZE       (AUD_REG_ENV_BASE2 + 0x053C)
    #define BIT_STR_AOUT2_INTRSIZE          8 //(8 - 23)
    #define BIT_NUM_AOUT2_INTRSIZE          16//generating interrupt when how many samples remained to be send out
        
    //AOUT control register
#define REGENV_AOUT2_CTRL           (AUD_REG_ENV_BASE2 + 0x0540)
    #define BIT_STR_AOUT2_EN_PRE            8 //(8)
    #define BIT_NUM_AOUT2_EN_PRE            1 //audio output enable
    
    #define BIT_STR_AOUT2_PAUSE_PRE         9 //(9)
    #define BIT_NUM_AOUT2_PAUSE_PRE         1 //audio output pause
    
    //output channel source configuration0
#define REGENV_AOUT2_CH_CFG0        (AUD_REG_ENV_BASE2 + 0x0544)
    #define BIT_STR_AOUT2_L_CFG             8 //(8 - 11)
    #define BIT_NUM_AOUT2_L_CFG             4 //channel left
        
    #define BIT_STR_AOUT2_R_CFG             12 //(12 - 15)
    #define BIT_NUM_AOUT2_R_CFG             4 //channel right
    
    #define BIT_STR_AOUT2_C_CFG             16 //(16 - 19)
    #define BIT_NUM_AOUT2_C_CFG             4 //channle center
    
    #define BIT_STR_AOUT2_CH7_CFG           20 //(20 - 23)
    #define BIT_NUM_AOUT2_CH7_CFG           4 //channle 7
    
    //output channel source configuration1
#define REGENV_AOUT2_CH_CFG1        (AUD_REG_ENV_BASE2 + 0x0548)
    #define BIT_STR_AOUT2_SL_CFG            8 //(8 - 11)
    #define BIT_NUM_AOUT2_SL_CFG            4 //channel surround left
            
    #define BIT_STR_AOUT2_SR_CFG            12 //(12 - 15)
    #define BIT_NUM_AOUT2_SR_CFG            4 //channel surround right
        
    #define BIT_STR_AOUT2_LFE_CFG           16 //(16 - 19)
    #define BIT_NUM_AOUT2_LFE_CFG           4 //channel lfe
        
    #define BIT_STR_AOUT2_CH8_CFG           20 //(20 - 23)
    #define BIT_NUM_AOUT2_CH8_CFG           4 //channle 8
    
    //output channel source configuration2
#define REGENV_AOUT2_CH_CFG2        (AUD_REG_ENV_BASE2 + 0x054C)
    #define BIT_STR_AOUT2_CH9_CFG           8 //(8 - 11)
    #define BIT_NUM_AOUT2_CH9_CFG           4 //channel 9
                
    #define BIT_STR_AOUT2_CH10_CFG          12 //(12 - 15)
    #define BIT_NUM_AOUT2_CH10_CFG          4 //channel 10
            
    #define BIT_STR_AOUT2_CH11_CFG          16 //(16 - 19)
    #define BIT_NUM_AOUT2_CH11_CFG          4 //channel 11
            
    #define BIT_STR_AOUT2_CH12_CFG          20 //(20 - 23)
    #define BIT_NUM_AOUT2_CH12_CFG          4 //channel 12
    
    //Channel number
#define REGENV_AOUT2_CH_NUM         (AUD_REG_ENV_BASE2 + 0x0550)
    #define BIT_STR_AOUT2_CH_NUM            8 //(8 - 11)
    #define BIT_NUM_AOUT2_CH_NUM            4 //of channels in DRAM source
    
    //AOUT switch register
#define REGENV_AOUT2_SWITCH         (AUD_REG_ENV_BASE2 + 0x0554)
    
    //LEV & DET configuration register
#define REGENV_AOUT2_LEV_DET_CFG    (AUD_REG_ENV_BASE2 + 0x0558) 
    #define BIT_STR_AOUT2_LEVEL_AVG_SEL     8 //(8 - 11)
    #define BIT_NUM_AOUT2_LEVEL_AVG_SEL     4 //of channels
        //0:left, 1:right, 2:sur left, 3:sur right, 4:cente, 5:lfe, 8:ch9, 9:ch10
    
    #define BIT_STR_AOUT2_AVG_SIGNAL_SEL    12 //(12)
    #define BIT_NUM_AOUT2_AVG_SIGNAL_SEL    1 // 
        //0:128 samples,  1:256 samples
    
    //AOUT averager level
#define REGENV_AOUT2_LEV            (AUD_REG_ENV_BASE2 + 0x055C) 
    #define BIT_STR_AOUT2_LEVEL_AVG_LEV     0 //(0 - 23)
    #define BIT_NUM_AOUT2_LEVEL_AVG_LEV     24//
    
    //Buffer data mixing configuration
#define REGENV_AOUT2_BUF_MIX_CFG    (AUD_REG_ENV_BASE2 + 0x0560) 
    #define BIT_STR_AOUT2_BUF_MIX_EN        8 //(8)
    #define BIT_NUM_AOUT2_BUF_MIX_EN        1 //buffer mix enable
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_LR  9 //(9)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_LR  1 //mixing data to left & right output
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_SLR 10 //(10)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_SLR 1 //mixing data to surround right & left output
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_C   11 //(11)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_C   1 //mixing data to center output
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_CH78    12 //(12)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_CH78    1 //mixing data to channel7 & channel8
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_CH910   13 //(13)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_CH910   1 //mixing data to channel9 & channel10
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_CH1112  14 //(14)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_CH1112  1 //mixing data to channel11 & channel12
    
    #define BIT_STR_AOUT2_MIX_BUFFER_EN_LFE     15 //(15)
    #define BIT_NUM_AOUT2_MIX_BUFFER_EN_LFE     1 //mixing data to subwoofer output
    
    #define BIT_STR_AOUT2_MIX_BUFFER_CFG        16 //(16)
    #define BIT_NUM_AOUT2_MIX_BUFFER_CFG        1 // 
    
    //Buffer1 mix NSADR
#define REGENV_AOUT2_MIX2_NSADR     (AUD_REG_ENV_BASE2 + 0x0568)
    #define BIT_STR_AOUT2_MIX1_NSADR            8 //(8 - 23)
    #define BIT_NUM_AOUT2_MIX1_NSADR            16//buffer1 mix data next start address
    
    #define BIT_STR_AOUT2_MIX2_NSADR            8 //(8 - 23)
    #define BIT_NUM_AOUT2_MIX2_NSADR            16//buffer2 mix data next start address


#ifdef __cplusplus
            }
#endif
                                
#endif // _AUD_REG_RGBK2_H    

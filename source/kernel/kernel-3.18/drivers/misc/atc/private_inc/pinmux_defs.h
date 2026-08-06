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

#ifndef PINMUX_DEFS_H
#define PINMUX_DEFS_H

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
#define PINMUX_FUNCTION0		0
#define PINMUX_FUNCTION1		1
#define PINMUX_FUNCTION2		2
#define PINMUX_FUNCTION3		3
#define PINMUX_FUNCTION4		4
#define PINMUX_FUNCTION5		5
#define PINMUX_FUNCTION6		6
#define PINMUX_FUNCTION7		7
#define PINMUX_FUNCTION8		8
#define PINMUX_FUNCTION9		9
#define PINMUX_FUNCTION10	 	10
#define PINMUX_FUNCTION11		11
#define PINMUX_FUNCTION12		12
#define PINMUX_FUNCTION13		13
#define PINMUX_FUNCTION14		14
#define PINMUX_FUNCTION15		15
#define MAX_PINMUX_FUNCTION     15

//83xx Pin mux function definitions
#define AUD_MCLK_SEL            0
#define SEL_AOMCLK              0
#define SEL_AOBCK               0
#define SEL_AOLRCK              0

#define AUD_SDATA0_SEL          6
#define SEL_AOSDATA0            6

#define AUD_SDATA1_SEL          7
#define SEL_AOSDATA1            7

#define AUD_SDATA2_SEL          8
#define SEL_AOSDATA2            8

#define AUD_SDATA3_SEL          9
#define SEL_AOSDATA3            9

#define AUD_SDATA4_SEL          10
#define SEL_AOSDATA4            10

#define AUD_SDATA5_SEL          11
#define SEL_AOSDATA5            11

#define AUD_MUTE_SEL            12
#define SEL_AMUTE               12

#define AUD_MCIN_SEL            13 
#define SEL_MCIN                13

#define AUD_SPDIF_SEL           14 
#define SEL_SPDIF               14

#define AUD_SPDATA_SEL          15 
#define SEL_SPDATA0             15

#define AUD_SPLRCK_SEL          18
#define SEL_SPLRCK              18

#define AUD_SPBCK_SEL           19
#define SEL_SPBCK               19

#define AUD_SPMCLK_SEL          20
#define SEL_SPMCLK              20

#define FESF_SEL                21
#define SEL_FESFCS              21
#define SEL_FESFCK              21
#define SEL_FESFDI              21
#define SEL_FESFDO              21

#define I2C_SEL                 22
#define SEL_SCL                 22 
#define SEL_SDA                 22

#define VINCLK_SEL              23
#define SEL_VINCLK              23

#define VINHSYNC_SEL            24 
#define SEL_VINHSYNC            24

#define VINVSYNC_SEL            25 
#define SEL_VINVSYNC            25

#define VIND2_7_SEL             26 
#define SEL_VIND2               26
#define SEL_VIND3               26
#define SEL_VIND4               26
#define SEL_VIND5               26
#define SEL_VIND6               26
#define SEL_VIND7               26

#define VIND8_9_SEL             27
#define SEL_VIND8               27
#define SEL_VIND9               27

#define VIND12_17_SEL           28 
#define SEL_VIND12              28
#define SEL_VIND13              28
#define SEL_VIND14              28
#define SEL_VIND15              28
#define SEL_VIND16              28
#define SEL_VIND17              28

#define VIND18_19_SEL           29
#define SEL_VIND18              29
#define SEL_VIND19              29

#define NAND_SEL                30
#define SEL_NFREN               30
#define SEL_NFCLE               30 
#define SEL_NFCEN               30 
#define SEL_NFWEN               30
#define SEL_NFALE               30
#define SEL_NFRBN               30
#define SEL_NFD0                30
#define SEL_NFD1                30
#define SEL_NFD2                30
#define SEL_NFD3                30
#define SEL_NFD4                30
#define SEL_NFD5                30
#define SEL_NFD6                30
#define SEL_NFD7                30

#define NAND_RDY2_SEL           31
#define SEL_NFRBN2              31 

#define VOUTCLK2_SEL            33
#define SEL_VOUTCLK2            33

#define VOUTHSYNC_SEL           34
#define SEL_VOUTHSYNC           34 

#define VOUTVSYNC_SEL           35
#define SEL_VOUTVSYNC           35

#define VOUTD2_7_SEL            36
#define SEL_VOUTD2              36
#define SEL_VOUTD3              36
#define SEL_VOUTD4              36
#define SEL_VOUTD5              36
#define SEL_VOUTD6              36
#define SEL_VOUTD7              36

#define VOUTD8_9_SEL            37 
#define SEL_VOUTD8              37
#define SEL_VOUTD9              37

#define VOUTD12_17_SEL          38
#define SEL_VOUTD12             38
#define SEL_VOUTD13             38
#define SEL_VOUTD14             38
#define SEL_VOUTD15             38
#define SEL_VOUTD16             38
#define SEL_VOUTD17             38

#define VOUTD18_19_SEL          39
#define SEL_VOUTD18             39
#define SEL_VOUTD19             39

#define RS232_D2_SEL            40 
#define SEL_UATXD2              40
#define SEL_UARXD2              40

#define AUD2_SEL                42
#define SEL_AO2MCLK             42 
#define SEL_AO2BCK              42
#define SEL_AO2LRCK             42
#define SEL_AO2SDATA0           42
#define SEL_AO2SDATA1           42
#define SEL_AO2SDATA2           42
#define SEL_AO2SDATA3           42
#define SEL_SPDIF2              42

#define AUD2_SACD_SEL           43
#define SEL_AO2SDATA4           43
#define SEL_AO2SDATA5           43 

#define MSSER_SEL               46
#define SEL_MSBS                46
#define SEL_MSD0                46 
#define SEL_MSCLK               46

#define MSPAL_SEL               49 
#define SEL_MSD1                49
#define SEL_MSD2                49
#define SEL_MSD3                49

#define M27_OUT_SEL             52
#define SEL_MJC                 52

#define VIND0_1_SEL             55 
#define SEL_VIND0               55
#define SEL_VIND1               55

#define SF_CS2_SEL              56  
#define SEL_SFCS2               56

#define PD_RS232_SEL            60 
#define SEL_UATXD1              60
#define SEL_UARXD1              60

#define VIND20_23_SEL           62 
#define SEL_VIND20              62
#define SEL_VIND21              62
#define SEL_VIND22              62
#define SEL_VIND23              62

#define NAND_CEN2_SEL           64
#define SEL_NFCEN2              64 

#define NAND_CEN3_SEL           68
#define SEL_NFCEN3              68 

#define NAND_CEN4_SEL           72
#define SEL_NFCEN4              72 

#define EXT_INTR1_SEL           76
#define SEL_EXT_INTR1           76

#define EXT_INTR2_SEL           80
#define SEL_EXT_INTR2           80

#define RS232_D3_SEL            84
#define SEL_UATXD3              84
#define SEL_UARXD3              84

#define HDMI_MSCK_EN_SEL        87
#define SEL_HDMI_MSCK_EN        87

#define SI2C_SEL                88 
#define SEL_SCLS                88 
#define SEL_SDAS                88

#define HDMI_I2C_SEL            92 
#define SEL_MSD                 92
#define SEL_MSCK                92 

#define VOUTDE_SEL              96
#define SEL_VOUTDE              96

#define TEST_FRONTEND_SEL       98
#define SEL_FEWDLE              98 
#define SEL_FEDLE               98
#define SEL_FEALE               98
#define SEL_FEBE1               98
#define SEL_FEBE0               98
#define SEL_FERW                98
#define SEL_FELAST              98
#define SEL_FEREQ               98
#define SEL_FECLK               98
#define SEL_FEADDR19            98
#define SEL_FEADDR18            98
#define SEL_FEADDR17            98
#define SEL_FEADDR16            98
#define SEL_FEADDR15            98
#define SEL_FEADDR14            98
#define SEL_FEADDR13            98
#define SEL_FEADDR12            98
#define SEL_FEADDR11            98
#define SEL_FEADDR10            98
#define SEL_FEADDR9             98
#define SEL_FEADDR8             98
#define SEL_FEADDR7             98
#define SEL_FEADDR6             98
#define SEL_FEADDR5             98
#define SEL_FEADDR4             98
#define SEL_FEADDR3             98
#define SEL_FEADDR2             98
#define SEL_FEADDR1             98
#define SEL_FEADDR0             98
#define SEL_FEDATAOUT15         98
#define SEL_FEDATAOUT14         98
#define SEL_FEDATAOUT13         98
#define SEL_FEDATAOUT12         98
#define SEL_FEDATAOUT11         98
#define SEL_FEDATAOUT10         98
#define SEL_FEDATAOUT9          98
#define SEL_FEDATAOUT8          98
#define SEL_FEDATAOUT7          98
#define SEL_FEDATAOUT6          98
#define SEL_FEDATAOUT5          98
#define SEL_FEDATAOUT4          98
#define SEL_FEDATAOUT3          98
#define SEL_FEDATAOUT2          98
#define SEL_FEDATAOUT1          98
#define SEL_FEDATAOUT0          98
#define SEL_FEDATAIN15          98
#define SEL_FEDATAIN14          98
#define SEL_FEDATAIN13          98
#define SEL_FEDATAIN12          98
#define SEL_FEDATAIN11          98
#define SEL_FEDATAIN10          98
#define SEL_FEDATAIN9           98
#define SEL_FEDATAIN8           98
#define SEL_FEDATAIN7           98
#define SEL_FEDATAIN6           98
#define SEL_FEDATAIN5           98
#define SEL_FEDATAIN4           98
#define SEL_FEDATAIN3           98
#define SEL_FEDATAIN2           98
#define SEL_FEDATAIN1           98
#define SEL_FEDATAIN0           98

#define TEST_IN_SEL             99 
#define SEL_TIND31              99
#define SEL_TIND30              99 
#define SEL_TIND29              99
#define SEL_TIND28              99
#define SEL_TIND27              99
#define SEL_TIND26              99
#define SEL_TIND25              99
#define SEL_TIND24              99
#define SEL_TIND23              99
#define SEL_TIND22              99
#define SEL_TIND21              99
#define SEL_TIND20              99
#define SEL_TIND19              99
#define SEL_TIND18              99
#define SEL_TIND17              99
#define SEL_TIND16              99
#define SEL_TIND15              99
#define SEL_TIND14              99
#define SEL_TIND13              99
#define SEL_TIND12              99
#define SEL_TIND11              99
#define SEL_TIND10              99
#define SEL_TIND9               99
#define SEL_TIND8               99
#define SEL_TIND7               99
#define SEL_TIND6               99
#define SEL_TIND5               99
#define SEL_TIND4               99
#define SEL_TIND3               99
#define SEL_TIND2               99
#define SEL_TIND1               99
#define SEL_TIND0               99

#define MONITOR_SEL             100 
#define SEL_MOUTB31             100
#define SEL_MOUTB30             100
#define SEL_MOUTB29             100
#define SEL_MOUTB28             100
#define SEL_MOUTB27             100
#define SEL_MOUTB26             100
#define SEL_MOUTB25             100
#define SEL_MOUTB24             100
#define SEL_MOUTB23             100
#define SEL_MOUTB22             100
#define SEL_MOUTB21             100
#define SEL_MOUTB20             100
#define SEL_MOUTB19             100
#define SEL_MOUTB18             100
#define SEL_MOUTB17             100
#define SEL_MOUTB16             100
#define SEL_MOUTB15             100
#define SEL_MOUTB14             100
#define SEL_MOUTB13             100
#define SEL_MOUTB12             100
#define SEL_MOUTB11             100
#define SEL_MOUTB10             100
#define SEL_MOUTB9              100
#define SEL_MOUTB8              100
#define SEL_MOUTB7              100
#define SEL_MOUTB6              100
#define SEL_MOUTB5              100
#define SEL_MOUTB4              100
#define SEL_MOUTB3              100
#define SEL_MOUTB2              100
#define SEL_MOUTB1              100
#define SEL_MOUTB0              100

#define VIND10_11_SEL           101  
#define SEL_VIND10              101
#define SEL_VIND11              101 

#define VOUTD0_1_SEL            102 
#define SEL_VOUTD0              102
#define SEL_VOUTD1              102

#define VOUTD10_11_SEL          103  
#define SEL_VOUTD10             103 
#define SEL_VOUTD11             103 

#define AOMCLK_PWM_SEL          104      
#define SEL_AOMCLK_PWM          104

#define AOBCK_PWM_SEL           105  
#define SEL_AOBCK_PWM           105

#define AOLRCK_PWM_SEL          106  
#define SEL_AOLRCK_PWM          106

#define AOSDATA0_PWM_SEL        107 
#define SEL_AOSDATA0_PWM        107

#define AOSDATA1_PWM_SEL        108 
#define SEL_AOSDATA1_PWM        108

#define AOSDATA2_PWM_SEL        109 
#define SEL_AOSDATA2_PWM        109

#define AOSDATA3_PWM_SEL        110 
#define SEL_AOSDATA3_PWM        110

#define AOSDATA4_PWM_SEL        111 
#define SEL_AOSDATA4_PWM        111

#define SPMCLK_PWM_SEL          112 
#define SEL_SPMCLK_PWM          112

#define SPBCK_PWM_SEL           113
#define SEL_SPBCK_PWM           113 

#define APLL_CLK_INV_SEL        114
#define SEL_SPBCK_INV           114
#define SEL_SPMCLK_INV          115
#define SEL_AOSDATA4_INV        116
#define SEL_AOSDATA3_INV        117
#define SEL_AOSDATA2_INV        118
#define SEL_AOSDATA1_INV        119
#define SEL_AOSDATA0_INV        120
#define SEL_AOLRCK_INV          121
#define SEL_AOBCK_INV           122
#define SEL_AOMCLK_INV          123

#define EFVDDQ_SEL              124
#define SEL_EFVDDQ              124

#define EFSRC_SEL               125 
#define SEL_EFSRC               125

#define RTC_TST_SEL             126
#define SEL_RTC_TST_CLK         126

#define WAVEOUT_SEL             127
#define SEL_WAVEOUT             127

#define MONITOR_MODE_SEL        128
#define SEL_MONITOR_MODE        128

#define MONITOR_DRAM_SEL        136        
#define SEL_MONITOR_DRAM        136

#define VDOUT0_5_SEL            144 
#define SEL_VOUTVSYNC_FIFO      144
#define SEL_VOUTHSYNC_FIFO      145
#define SEL_VOUTD0_1            146
#define SEL_VOUTD2_7            147
#define SEL_VOUTD8_9            148
#define SEL_VOUTD10_11          149

#define NAND_16BIT_SEL          150  
#define SEL_NFD8                150 
#define SEL_NFD9                150 
#define SEL_NFD10               150 
#define SEL_NFD11               150 
#define SEL_NFD12               150 
#define SEL_NFD13               150 
#define SEL_NFD14               150 
#define SEL_NFD15               150 

#define PWM_SEL                 152
#define SEL_PWM                 152

#define VDOUT6_8_SEL            157 
#define SEL_VDOUT12_17          157 
#define SEL_VDOUT18_19          158
#define SEL_VINHSYNC_VINVSYNC_VIND0_19   159

#define USBPHY_CK_SEL           160 
#define SEL_USBPHY              160

#define HDMI_TST_SEL            161 
#define SEL_HDMI_TST            161

#define SF_SEL                  162
#define SEL_SFCS                162
#define SEL_SFCK                162
#define SEL_SFDI                162
#define SEL_SFDO                162

#define VDO_444_SEL             163
#define SEL_VDO_444             163

#define SD0_OE_SEL              174
#define SEL_SD0                 174

#define SDSER_SEL               175
#define SEL_SDD0                175
#define SEL_SDCLK               175
#define SEL_SDCMD               175

#define SD1PAL_SEL              178 
#define SEL_SDD1                178

#define SD23PAL_SEL             181 
#define SEL_SDD3                181
#define SEL_SDD2                181

#define VOUTCLK1_SEL            184 
#define SEL_VOUTCLK1            184

#define SPDATA1_SEL             186 
#define SEL_SPDATA1             186 

#define SPDATA2_SEL             188 
#define SEL_SPDATA2             188

#define SPDATA3_SEL             190 
#define SEL_SPDATA3             190

#define PROT_SEL                192 
#define SEL_PROT                192

#define MAX_PINMUX_SEL          224


#endif // PINMUX_DEFS_H

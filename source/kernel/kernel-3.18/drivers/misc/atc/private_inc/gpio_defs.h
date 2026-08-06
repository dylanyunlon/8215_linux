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
#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
#define TOTAL_GPIO_IDX          5
#define GPIO_INDEX_MASK         ((1 << 5) - 1)

// GPIO name definitions
#define PIN_GPIO0           0
#define PIN_GPIO1           1
#define PIN_GPIO2           2
#define PIN_GPIO3           3
#define PIN_GPIO4           4
#define PIN_GPIO5           5
#define PIN_GPIO6           6
#define PIN_GPIO7           7
#define PIN_AMUTE           8
#define PIN_AOBCK           9
#define PIN_AOLRCK          10
#define PIN_AOMCLK          11
#define PIN_AOSDATA0        12
#define PIN_AOSDATA1        13
#define PIN_AOSDATA2        14
#define PIN_AOSDATA3        15
#define PIN_AOSDATA4        16
#define PIN_AOSDATA5        17
#define PIN_FESFCK          18
#define PIN_FESFCS          19
#define PIN_FESFDI          20
#define PIN_FESFDO          21
#define PIN_MCIN            22
#define PIN_NFALE           23
#define PIN_NFCEN           24     
#define PIN_NFCEN2          25     
#define PIN_NFCLE           26
#define PIN_NFD0            27
#define PIN_NFD1            28
#define PIN_NFD2            29
#define PIN_NFD3            30       
#define PIN_NFD4            31 

#define PIN_NFD5            32      
#define PIN_NFD6            33
#define PIN_NFD7            34
#define PIN_NFRBN           35
#define PIN_NFRBN2          36
#define PIN_NFREN           37
#define PIN_NFWEN           38
#define PIN_SCL             39
#define PIN_SDA             40   
#define PIN_SFCK            41
#define PIN_SFCS            42
#define PIN_SFDI            43
#define PIN_SFDO            44
#define PIN_SPBCK           45
#define PIN_SPDATA          46
#define PIN_SPDIF           47
#define PIN_SPLRCK          48      
#define PIN_SPMCLK          49
#define PIN_VINCLK          50
#define PIN_VIND0           51
#define PIN_VIND1           52  
#define PIN_VIND10          53   
#define PIN_VIND11          54
#define PIN_VIND12          55
#define PIN_VIND13          56
#define PIN_VIND14          57
#define PIN_VIND15          58
#define PIN_VIND16          59      
#define PIN_VIND17          60
#define PIN_VIND18          61      
#define PIN_VIND19          62
#define PIN_VIND2           63     

#define PIN_VIND3           64
#define PIN_VIND4           65      
#define PIN_VIND5           66
#define PIN_VIND6           67
#define PIN_VIND7           68
#define PIN_VIND8           69
#define PIN_VIND9           70
#define PIN_VINHSYNC        71
#define PIN_VINVSYNC        72
#define PIN_VOUTCLK1        73
#define PIN_VOUTCLK2        74
#define PIN_VOUTD0          75
#define PIN_VOUTD1          76
#define PIN_VOUTD10         77
#define PIN_VOUTD11         78
#define PIN_VOUTD12         79
#define PIN_VOUTD13         80
#define PIN_VOUTD14         81
#define PIN_VOUTD15         82
#define PIN_VOUTD16         83
#define PIN_VOUTD17         84
#define PIN_VOUTD18         85
#define PIN_VOUTD19         86
#define PIN_VOUTD2          87
#define PIN_VOUTD3          88
#define PIN_VOUTD4          89
#define PIN_VOUTD5          90
#define PIN_VOUTD6          91
#define PIN_VOUTD7          92
#define PIN_VOUTD8          93
#define PIN_VOUTD9          94
#define PIN_VOUTHSYNC       95
                        
#define PIN_VOUTVSYNC       96
#define PIN_TDO             97
#define PIN_TRST            98
#define PIN_TMS             99
#define PIN_TDI             100
#define PIN_TCK             101

//pdwnc
#define PDWNC_PAD_PINMUX1   0x0F4
#define PDWNC_PAD_PINMUX2   0x0F8
#define PDWNC_PAD_PINMUX3   0x0FC
#define PDWNC_GPIOIN        0x0D0
#define PDWNC_GPIOEN        0x0D4
#define PDWNC_GPIOOUT       0x0D8

#define PIN_VFD_STB         102
#define PIN_VFD_CLK         103 
#define PIN_VFD_DATA        104
#define PIN_LCDRD           105
//                                             106
#define PIN_ETMDIO          107
#define PIN_ETMDC           108
#define PIN_ETRXER          109
#define PIN_ETTXER          110
#define PIN_ETRXDV          111
#define PIN_ETTXEN          112 
#define PIN_ETCRS           113 
#define PIN_ETCOL           114
#define PIN_ETRXD3          115
#define PIN_ETRXD2          116
#define PIN_ETRXD1          117
#define PIN_ETRXD0          118
#define PIN_ETTXD3          119
#define PIN_ETTXD2          120
#define PIN_ETTXD1          121  
#define PIN_ETTXD0          122
#define PIN_ETRXCLK         123 
#define PIN_ETTXCLK         124
#define PIN_HDMI_SCK        125
#define PIN_HDMI_SD         126 
#define PIN_HDMI_HTPLG      127
#define PIN_HDMI_CEC        128 
   
#define PIN_UNKNOWN         255

// Total GPIO is 128.
#ifndef TOTAL_GPIO_NUM
#define TOTAL_GPIO_NUM      128
#endif

#define AUD_DAC_RESET  	     PIN_GPIO3    	//Audio DAC Reset pin
#define PCM_DSD_SEL 	PIN_GPIO6		//Audio DAC PCM/DSD selection
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/* mtk40043
#if SUPPORT_WP
#define NAND_WP         PIN_UNKNOWN
#endif

#if SUPPORT_UPG_STATUS
#define UPG_STATUS_PIN  PIN_GPIO6 
#endif

#if SUPPORT_AMUTE
#define AUDIO_MUTE      PIN_AMUTE
#endif

#if SUPPORT_JIG
#define START_BIT       PIN_GPIO1

#define VIDEO_MUTE      PIN_VIND6      // Control for Video Mute 0:Mute, 1:Output

#define JIG_MODE0       PIN_GPIO4      //JIG mode flag input 0
#define JIG_MODE1       PIN_GPIO5      //JIG mode flag input 1

#define USB_PCONT1      PIN_VIND5
#define USB_PCONT2      PIN_GPIO3
#define USB_PCONT3 		PIN_UNKNOWN
#endif
*/

#define GPIO_OUT_ONE            1
#define GPIO_OUT_ZERO           0
#define GPS_RST 			    PIN_VOUTD0
#define GPS_STANDBY			    PIN_VOUTD1
#define GPS_POWER_PIN           PIN_VOUTD2

#endif //GPIO_DEFS_H

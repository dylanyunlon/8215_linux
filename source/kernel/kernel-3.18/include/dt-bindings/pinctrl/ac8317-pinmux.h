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
#ifndef __AC8317_PINMUX_H
#define __AC8317_PINMUX_H
#define OUTPUT        1
#define INPUT         0
//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
//CKGEN
#define GPIO_EN0_OFFSET         0x074 
#define GPIO_OUT0_OFFSET        0x0E0
#define GPIO_IN0_OFFSET         0x100
//#define PAD_FECTL_0               0x280
//#define PAD_PWMCTL_0            0x288
#define TOTAL_GPIO_IDX          5
#define GPIO_INDEX_MASK         ((1 << 5) - 1)
//-----------------------------------Gen GPIO--------------------------------------------
#define PIN_0_GPIO0                   0
#define PIN_1_GPIO1                   1
#define PIN_2_GPIO2                   2
#define PIN_3_GPIO3                   3
#define PIN_4_GPIO4                   4
#define PIN_5_GPIO5                   5
#define PIN_6_GPIO6                   6
#define PIN_7_GPIO7                   7
#define PIN_8_AADC_L0                 8
#define PIN_9_AADC_R0                 9
#define PIN_10_AADC_L1                10
#define PIN_11_AADC_R1                11
#define PIN_12_AADC_L2                12
#define PIN_13_AADC_R2                13
#define PIN_14_AADC_L3                14
#define PIN_15_AADC_R3                15
#define PIN_16_AADC_L4                16
#define PIN_17_AADC_R4                17
#define PIN_18_AL0                    18
#define PIN_19_AL1                    19
#define PIN_20_AL2                    20
#define PIN_21_AL3                    21
#define PIN_22_AMUTE_R                22
#define PIN_23_AMUTE_F                23     
#define PIN_24_AR0                    24     
#define PIN_25_AR1                    25
#define PIN_26_AR2                    26
#define PIN_27_AR3                    27
#define PIN_28_DCLK                   28
#define PIN_29_DCLK_IN                29       
#define PIN_30_GPIO30                 30 
#define PIN_31_GPIO31                 31
#define PIN_32_DE                     32
#define PIN_33_DE_IN                  33
#define PIN_34_DEMOD_RST              34
#define PIN_35_EINT0                  35
#define PIN_36_EINT1                  36
#define PIN_37_EINT2                  37
#define PIN_38_EINT3                  38
#define PIN_39_HDMI_CEC_RX            39   
#define PIN_40_HDMI_HPD_RX            40
#define PIN_41_HDMI_SCL_RX            41
#define PIN_42_GPIO42                 42
#define PIN_43_GPIO43                 43
#define PIN_44_GPIO44                 44
#define PIN_45_HDMI_SDA_RX            45
#define PIN_46_VSYNC_IN               46
#define PIN_47_HSYNC_IN               47  
#define PIN_48_I2S_IN1_BCK            48
#define PIN_49_I2S_IN1_D              49
#define PIN_50_I2S_IN1_LRCK           50
#define PIN_51_I2S_IN1_MCLK           51  
#define PIN_52_I2S_OUT0_BCK           52
#define PIN_53_GPIO53                 53
#define PIN_54_GPIO54                 54
#define PIN_55_GPIO55                 55
#define PIN_56_GPIO56                 56
#define PIN_57_GPIO57                 57
#define PIN_58_I2S_OUT0_D0            58      
#define PIN_59_I2S_OUT0_D1            59
#define PIN_60_I2S_OUT0_D2            60 
#define PIN_61_I2S_OUT0_LRCK          60  
#define PIN_62_I2S_OUT0_MCLK          62     
#define PIN_63_IR                     63
#define PIN_64_AO0N                   64      
#define PIN_65_AO0P                   65
#define PIN_66_AO1N                   66
#define PIN_67_AO1P                   67
#define PIN_68_AO2N                   68
#define PIN_69_AO2P                   69
#define PIN_70_GPIO70                 70
#define PIN_71_GPIO71                 71
#define PIN_72_GPIO72                 72
#define PIN_73_A03N                   73
#define PIN_74_GPIO74                 74
#define PIN_75_A03P                   75
#define PIN_76_A0CKN                  76
#define PIN_77_A0CKP                  77
#define PIN_78_MHL_PWR_EN             78
#define PIN_79_MHL_SENSE              79
#define PIN_80_SD0_DO                 80
#define PIN_81_SD0_D1                 81
#define PIN_82_SD0_D2                 82  
#define PIN_83_SD0_D3                 83   
#define PIN_84_SD1_DO                 84
#define PIN_85_SD1_D1                 85
#define PIN_86_SD1_D2                 86
#define PIN_87_SD1_D3                 87
#define PIN_88_SD2_D1                 88
#define PIN_89_SD2_D2                 89      
#define PIN_90_SD2_D3                 90
#define PIN_91_SD2_D4                 91
#define PIN_92_NFALE                  92
#define PIN_93_NFCEN0                 93     
#define PIN_94_NFCEN1                 94     
#define PIN_95_NFCLE                  95     
#define PIN_96_NFRBN                  96
#define PIN_97_NFRBN2                 97
#define PIN_98_NFREN                  98
#define PIN_99_NFWEN                  99
#define PIN_100_NLD0                  100
#define PIN_101_NLD1                  101
#define PIN_102_NLD2                  102
#define PIN_103_NLD3                  103
#define PIN_104_NLD4                  104
#define PIN_105_NLD5                  105
#define PIN_106_NLD6                  106
#define PIN_107_NLD7                  107
#define PIN_108_PCM_CLK               108
#define PIN_109_PCM_IN                109
#define PIN_110_PCM_OUT               110
#define PIN_111_PCM_SYNC              111    
#define PIN_112_SCL0                  112
#define PIN_113_SCL1                  113//
#define PIN_114_SD_V33_18_SW0         114
#define PIN_115_SD_V33_18_SW1         115
#define PIN_116_SD_V33_18_SW2         116
#define PIN_117_SDA0                  117
#define PIN_118_SDA1                  118
#define PIN_119_SP0_CLK               119
#define PIN_120_SP0_CS                120
#define PIN_121_SP0_SI                121
#define PIN_122_SP0_SO                122
#define PIN_123_SP1_CLK               123
#define PIN_124_GPIO124               124
#define PIN_125_GPIO125               125
#define PIN_126_SP1_CS                126
#define PIN_127_SP1_SI                127
#define PIN_128_SP1_SO                128
#define PIN_129_SPDIF                 129
#define PIN_130_TS_CLKIN              130
#define PIN_131_TS_D0                 131
#define PIN_132_TS_D1                 132
#define PIN_133_TS_D2                 133             
#define PIN_134_TS_D3                 134
#define PIN_135_TS_D4                 135
#define PIN_136_TS_D5                 136
#define PIN_137_TS_D6                 137
#define PIN_138_TS_D7                 138
#define PIN_139_TS_DEN                139
#define PIN_140_TS_SYNC               140
#define PIN_141_URXD0                 141
#define PIN_142_URXD1                 142
#define PIN_143_URXD2                 143
#define PIN_144_URXD3                 144
#define PIN_145_URXD4                 145
#define PIN_146_URXD5                 146
#define PIN_147_USB_DM_P0             147
#define PIN_148_USB_DM_P1             148
#define PIN_149_GPIO149               149
#define PIN_150_GPIO150               150
#define PIN_151_USB_DP_P0             151
#define PIN_152_USB_DP_P1             152
#define PIN_153_UTXD0                 153
#define PIN_154_UTXD1                 154
#define PIN_155_UTXD2                 155
#define PIN_156_UTXD3                 156
#define PIN_157_UTXD4                 157
#define PIN_158_UTXD5                 158
#define PIN_159_VB0                   159
#define PIN_160_VB1                   160
#define PIN_161_VB2                   161
#define PIN_162_GPIO162               162
#define PIN_163_VB3                   163
#define PIN_164_VB4                   164
#define PIN_165_VB5                   165             
#define PIN_166_VB6                   166
#define PIN_167_VB7                   167
#define PIN_168_VG0                   168
#define PIN_169_VG1                   169
#define PIN_170_VG2                   170
#define PIN_171_VG3                   171
#define PIN_172_VG4                   172
#define PIN_173_VG5                   173
#define PIN_174_VG6                   174
#define PIN_175_VG7                   175
#define PIN_176_VGA_SCL               176
#define PIN_177_VGA_SDA               177
#define PIN_178_VIN0                  178
#define PIN_179_VIN1                  179
#define PIN_180_VIN2                  180
#define PIN_181_VIN3                  181
#define PIN_182_VIN4                  182
#define PIN_183_VIN5                  183
#define PIN_184_VIN6                  184
#define PIN_185_VIN7                  185
#define PIN_186_VR0                   186
#define PIN_187_VR1                   187
#define PIN_188_VR2                   188
#define PIN_189_VR3                   189
#define PIN_190_VR4                   190
#define PIN_191_VR5                   191
#define PIN_192_VR6                   192
#define PIN_193_VR7                   193
#define PIN_194_HSYNC                 194
#define PIN_195_VSYNC                 195//GPIO only end here
#define PIN_196_MSDC_0_CLK            196
#define PIN_197_MSDC_0_CMD            197
#define PIN_198_MSDC_1_CLK            198
#define PIN_199_MSDC_1_CMD            199
#define PIN_200_MSDC_2_CLK            200
#define PIN_201_MSDC_2_CMD            201
#define PIN_202_SD_8BIT_CLK           202
#define PIN_203_VGA_HSYNC_GPI         203
#define PIN_204_VGA_VSYNC_GPI         204
//------------------------End Gen GPIO---------------------------------
   
#define PIN_UNKNOWN         255
#define TOTAL_GPIO_NUM      205
//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------
// Macros for register read/write
//#define PAD_CFG0_OFFSET			0xC0
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
//------------------ padmux0 0X54 ---------------------------------
//PADMUX 0 -- 0X54
#define I2S_MIC_IN_SEL            0
#define I2C0_SEL                  3 
#define AP_SF_SEL                 8 
#define USB_I2C_SEL               18
#define TS_SEL0                   24
#define TS_SEL1                   25
#define TS_SEL2                   26
#define TS_SEL3                   27
#define ARM11_JTAG_SEL            28
//------------------ padmux1 0X58 ---------------------------------
//PADMUX 1 -- 0X58
#define SGM_MIC_IN_SEL            (32 + 0)
#define DVD_RS232_SEL             (32 + 15)
#define AP_RS232_SEL              (32 + 18)
#define DVD_RS232_SEL_ADDITION    (32 + 21)
#define AP_RS232_SEL_ADDITION     (32 + 22)
#define PCM_SEL                   (32 + 24)
#define ARM9_JTAG_SEL             (32 + 25)
#define PWM0_SEL                  (32 + 28)
//------------------ padmux2 0X5C ---------------------------------
//PADMUX 2 -- 0X5C
#define PWM1_SEL                  (64 + 0)
#define CCIR601_TIMING_VSIN_SEL   (64 + 4)
#define TTL_6_8B_SEL              (64 + 6)
#define TTL_8B_SEL                (64 + 7)
#define TTL_SYNC_SEL              (64 + 8)
#define TTL_DE_SEL                (64 + 9)
#define CCIR656_601_DATAIN_SEL    (64 + 10)
#define CCIR601_TIMING_HSIN_SEL   (64 + 12)
#define CCIR601_TIMING_DEIN_SEL   (64 + 14)
#define SP0_SEL                   (64 + 16)
#define SP1_SEL                   (64 + 19)
#define NAND_FLASH_SEL            (64 + 22)
#define SPDIF_SEL                 (64 + 30)
#define AMUTE_R_SEL               (64 + 31)
//------------------ padmux3 0X60 ---------------------------------
//PADMUX 3 -- 0X60
#define AMUTE_F_SEL               (96 + 0)
#define I2S_OUT0_SEL              (96 + 8)
#define TEST_BUS_SEL              (96 + 10)
#define DVD_T8032_UP0_SEL         (96 + 12)
#define DVD_T8032_UP1_SEL         (96 + 16)
#define DVD_T8032_UP2_SEL         (96 + 18)
#define DVD_T8032_UP3_SEL         (96 + 22)
#define T8032_EINT1_SEL           (96 + 24)
#define T8032_EINT2_SEL           (96 + 26)
#define T8032_EINT3_SEL           (96 + 28)
#define T8032_EINT4_SEL           (96 + 30)
//------------------ padmux4 0X64 ---------------------------------
//PADMUX 4 -- 0X64
#define T8032_EINT5_SEL           (128 + 0)
#define PT110_EINT1_SEL           (128 + 2)
#define PT110_EINT2_SEL           (128 + 4)
#define PT110_EINT3_SEL           (128 + 6)
#define PT110_EINT4_SEL           (128 + 8)
#define RTC_OUT_SEL               (128 + 10)
#define TEST_IN_SEL               (128 + 11)
#define TEST_OUT_SEL              (128 + 12)
#define USB_GPIO_MODE             (128 + 18)
#define USB_GPIO_MODE_1P          (128 + 19)
#define LVDS_SEL                  (128 + 24)
//------------------ padmux5 0X68 ---------------------------------
//PADMUX 5 -- 0X68
#define EINT0_SEL                 (160 + 0)
#define EINT1_SEL                 (160 + 2)
#define EINT2_SEL                 (160 + 4)
#define EINT3_SEL                 (160 + 6)
#define EINT4_SEL                 (160 + 8)
#define EINT5_SEL                 (160 + 12)
#define EINT6_SEL                 (160 + 16)
#define EINT7_SEL                 (160 + 19)
#define PWM2_SEL                  (160 + 22)
#define PWM3_SEL                  (160 + 25)
//------------------ padmux6 0X6C ---------------------------------
//PADMUX 6 -- 0X6C
#define I2S_OUT1_SEL              (192 + 3)
#define UART0_SEL                 (192 + 11)
#define UART1_SEL                 (192 + 13)
#define UART2_SEL                 (192 + 16)
#define UART3_SEL                 (192 + 18)
#define UART4_SEL                 (192 + 20)
#define UART5_SEL                 (192 + 23)
#define I2C1_SEL                  (192 + 26)
//------------------ padmux7 0x70 (ADD)---------------------------------
#define HDMI_I2C_SEL               (224 + 0)
#define MHL_SENSE_SEL              (224 + 3)
#define HDMI_CEC_SEL               (224 + 4)
#define HDMI_HDP_SEL               (224 + 5)
#define VGA_I2C_SEL                (224 + 6)
#define DVD_TEST_CPUM              (224 + 8)
#define HDMI_SPDIF_SEL             (224 + 11)
#define CA7_DFD_SEL                (224 + 12)
#define CA7_MBIST_SEL              (224 + 14)
#define I2S_LINE0_IN_SEL           (224 + 16)
#define I2S_LINE1_IN_SEL           (224 + 20)
#define DVD_RS232_SEL_ADDITION_CLEAR    (32 + 21)
#define AP_RS232_SEL_ADDITION_CLAER     (32 + 22)
#define PINMUX_LEVEL_GPIO_END_FLAG          (0xFF)
//-------------------------END PADMUX--------------------------------
#define MAX_PINMUX_SEL   (I2S_LINE1_IN_SEL + 1)
#define	GPIO_IRQTYPE_RISINGEDGE  0
#define	GPIO_IRQTYPE_FALLINGEDGE 1
#define	GPIO_IRQTYPE_TWOEDGE     2
#define	GPIO_IRQTYPE_HIGHLEVEL   3
#define	GPIO_IRQTYPE_LOWLEVEL    4
#define CURRENT_2MA             0
#define CURRENT_4MA             1
#define CURRENT_6MA             2
#define CURRENT_8MA             3
#define    NORMAL 0
#define    PULLUP 1
#define    PULLDOWN  2
#define PIN_42_GPIO42__TS_SEL0                 (PIN_42_GPIO42<<8|TS_SEL0)
#define PIN_43_GPIO43__TS_SEL1                 (PIN_43_GPIO43<<8|TS_SEL1)
#define PIN_44_GPIO44__TS_SEL2                 (PIN_44_GPIO44<<8|TS_SEL2)
#define PIN_31_GPIO31__TS_SEL3                 (PIN_31_GPIO31<<8|TS_SEL3)
#define PIN_31_GPIO31__GPIO31                 (PIN_31_GPIO31<<8|PINMUX_LEVEL_GPIO_END_FLAG)
/*MHL*/
#define PIN_40_HDMI_HPD_RX__HDMI_HDP_SEL         (PIN_40_HDMI_HPD_RX<<8 | HDMI_HDP_SEL)
#define PIN_40_HDMI_HPD_RX__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_40_HDMI_HPD_RX<<8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_79_MHL_SENSE__MHL_SENSE_SEL             (PIN_79_MHL_SENSE<<8 | MHL_SENSE_SEL)
/*BKL*/
#define PIN_124_GPIO124__GPIO124                (PIN_124_GPIO124<<8|PINMUX_LEVEL_GPIO_END_FLAG)
/*WCH*/
#define PIN_46_VSYNC_IN__VSIN_SEL               (PIN_46_VSYNC_IN<<8|CCIR601_TIMING_VSIN_SEL)
#define PIN_178_VIN0__DATAIN_SEL                (PIN_178_VIN0<<8|CCIR656_601_DATAIN_SEL)
#define PIN_179_VIN1__DATAIN_SEL                (PIN_179_VIN1<<8|CCIR656_601_DATAIN_SEL)
#define PIN_180_VIN2__DATAIN_SEL                (PIN_180_VIN2<<8|CCIR656_601_DATAIN_SEL)
#define PIN_181_VIN3__DATAIN_SEL                (PIN_181_VIN3<<8|CCIR656_601_DATAIN_SEL)
#define PIN_182_VIN4__DATAIN_SEL                (PIN_182_VIN4<<8|CCIR656_601_DATAIN_SEL)
#define PIN_183_VIN5__DATAIN_SEL                (PIN_183_VIN5<<8|CCIR656_601_DATAIN_SEL)
#define PIN_184_VIN6__DATAIN_SEL                (PIN_184_VIN6<<8|CCIR656_601_DATAIN_SEL)
#define PIN_185_VIN7__DATAIN_SEL                (PIN_185_VIN7<<8|CCIR656_601_DATAIN_SEL)
#define PIN_29_DCLK_IN__DATAIN_SEL              (PIN_29_DCLK_IN<<8|CCIR656_601_DATAIN_SEL)
#define PIN_47_HSYNC_IN__HSIN_SEL                (PIN_47_HSYNC_IN<<8|CCIR601_TIMING_HSIN_SEL)
#define PIN_33_DE_IN__DEIN_SEL                   (PIN_33_DE_IN<<8|CCIR601_TIMING_DEIN_SEL)
/*FB*/
#define PIN_32_DE__TTL_DE_SEL			(PIN_32_DE << 8 | TTL_DE_SEL)
#define PIN_194_HSYNC__TTL_SYNC_SEL		(PIN_194_HSYNC << 8 | TTL_SYNC_SEL)
#define PIN_195_VSYNC__TTL_SYNC_SEL		(PIN_195_VSYNC << 8 | TTL_SYNC_SEL)
#define PIN_159_VB0__TTL_8B_SEL			(PIN_159_VB0 << 8 | TTL_8B_SEL)
#define PIN_160_VB1__TTL_8B_SEL			(PIN_160_VB1 << 8 | TTL_8B_SEL)
#define PIN_168_VG0__TTL_8B_SEL			(PIN_168_VG0 << 8 | TTL_8B_SEL)
#define PIN_169_VG1__TTL_8B_SEL			(PIN_169_VG1 << 8 | TTL_8B_SEL)
#define PIN_186_VR0__TTL_8B_SEL			(PIN_186_VR0 << 8 | TTL_8B_SEL)
#define PIN_187_VR1__TTL_8B_SEL			(PIN_187_VR1 << 8 | TTL_8B_SEL)
#define PIN_161_VB2__TTL_6_8B_SEL		(PIN_161_VB2 << 8 | TTL_6_8B_SEL)
#define PIN_163_VB3__TTL_6_8B_SEL		(PIN_163_VB3 << 8 | TTL_6_8B_SEL)
#define PIN_164_VB4__TTL_6_8B_SEL		(PIN_164_VB4 << 8 | TTL_6_8B_SEL)
#define PIN_165_VB5__TTL_6_8B_SEL		(PIN_165_VB5 << 8 | TTL_6_8B_SEL)
#define PIN_166_VB6__TTL_6_8B_SEL		(PIN_166_VB6 << 8 | TTL_6_8B_SEL)
#define PIN_167_VB7__TTL_6_8B_SEL		(PIN_167_VB7 << 8 | TTL_6_8B_SEL)
#define PIN_170_VG2__TTL_6_8B_SEL		(PIN_170_VG2 << 8 | TTL_6_8B_SEL)
#define PIN_171_VG3__TTL_6_8B_SEL		(PIN_171_VG3 << 8 | TTL_6_8B_SEL)
#define PIN_172_VG4__TTL_6_8B_SEL		(PIN_172_VG4 << 8 | TTL_6_8B_SEL)
#define PIN_173_VG5__TTL_6_8B_SEL		(PIN_173_VG5 << 8 | TTL_6_8B_SEL)
#define PIN_174_VG6__TTL_6_8B_SEL		(PIN_174_VG6 << 8 | TTL_6_8B_SEL)
#define PIN_175_VG7__TTL_6_8B_SEL		(PIN_175_VG7 << 8 | TTL_6_8B_SEL)
#define PIN_188_VR2__TTL_6_8B_SEL		(PIN_188_VR2 << 8 | TTL_6_8B_SEL)
#define PIN_189_VR3__TTL_6_8B_SEL		(PIN_189_VR3 << 8 | TTL_6_8B_SEL)
#define PIN_190_VR4__TTL_6_8B_SEL		(PIN_190_VR4 << 8 | TTL_6_8B_SEL)
#define PIN_191_VR5__TTL_6_8B_SEL		(PIN_191_VR5 << 8 | TTL_6_8B_SEL)
#define PIN_192_VR6__TTL_6_8B_SEL		(PIN_192_VR6 << 8 | TTL_6_8B_SEL)
#define PIN_193_VR7__TTL_6_8B_SEL		(PIN_193_VR7 << 8 | TTL_6_8B_SEL)
//i2c0
#define PIN_112_SCL0__I2C0_SEL                          (PIN_112_SCL0 << 8 | I2C0_SEL)
#define PIN_112_SCL0__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_112_SCL0 << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_117_SDA0__I2C0_SEL                          (PIN_117_SDA0 << 8 | I2C0_SEL)
#define PIN_117_SDA0__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_117_SDA0 << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
//i2c1
#define PIN_113_SCL1__I2C1_SEL                          (PIN_113_SCL1 << 8 | I2C1_SEL)
#define PIN_113_SCL1__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_113_SCL1 << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_118_SDA1__I2C1_SEL                          (PIN_118_SDA1 << 8 | I2C1_SEL)
#define PIN_118_SDA1__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_118_SDA1 << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
//spi0
#define PIN_119_SP0_CLK__SP0_SEL                           (PIN_119_SP0_CLK << 8 | SP0_SEL)
#define PIN_119_SP0_CLK__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_119_SP0_CLK << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_120_SP0_CS__SP0_SEL                            (PIN_120_SP0_CS << 8 | SP0_SEL)
#define PIN_120_SP0_CS__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_120_SP0_CS << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_121_SP0_SI__SP0_SEL                            (PIN_121_SP0_SI << 8 | SP0_SEL)
#define PIN_121_SP0_SI__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_121_SP0_SI << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_122_SP0_SO__SP0_SEL                            (PIN_122_SP0_SO << 8 | SP0_SEL)
#define PIN_122_SP0_SO__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_122_SP0_SO << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
//spi1
#define PIN_123_SP1_CLK__SP1_SEL                           (PIN_123_SP1_CLK << 8 | SP1_SEL)
#define PIN_123_SP1_CLK__PINMUX_LEVEL_GPIO_END_FLAG        (PIN_123_SP1_CLK << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_126_SP1_CS__SP1_SEL                            (PIN_126_SP1_CS << 8 | SP1_SEL)
#define PIN_126_SP1_CS__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_126_SP1_CS << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_127_SP1_SI__SP1_SEL                            (PIN_127_SP1_SI << 8 | SP1_SEL)
#define PIN_127_SP1_SI__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_127_SP1_SI << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_128_SP1_SO__SP1_SEL                            (PIN_128_SP1_SO << 8 | SP1_SEL)
#define PIN_128_SP1_SO__PINMUX_LEVEL_GPIO_END_FLAG         (PIN_128_SP1_SO << 8 | PINMUX_LEVEL_GPIO_END_FLAG)
//touch 
#define PIN_37_EINT2__EINT2_SEL                  (PIN_37_EINT2<<8|EINT2_SEL)
#define PIN_125_GPIO125__PWM3_SEL                (PIN_125_GPIO125<<8|PWM3_SEL)
#define PIN_125_GPIO125__PINMUX_LEVEL_GPIO_END_FLAG (PIN_125_GPIO125<<8|PINMUX_LEVEL_GPIO_END_FLAG)
#define PIN_150_GPIO150__PWM1_SEL                (PIN_150_GPIO150<<8|PWM1_SEL)
#define PIN_150_GPIO150__PINMUX_LEVEL_GPIO_END_FLAG  (PIN_150_GPIO150<<8|PINMUX_LEVEL_GPIO_END_FLAG)
#endif /* __AC8317_PINMUX_H */
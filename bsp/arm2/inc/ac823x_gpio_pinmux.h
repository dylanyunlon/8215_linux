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

#ifndef __AC823X_GPIO_PINMUX_H
#define __AC823X_GPIO_PINMUX_H


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

//-----------------------------------Gen GPIO, 221 pins--------------------------------------------
#define PIN_0_GPIO0                   0
#define PIN_1_GPIO1                   1
#define PIN_2_GPIO2                   2
#define PIN_3_GPIO3                   3
#define PIN_4_GPIO4                   4
#define PIN_5_GPIO5                   5
#define PIN_6_GPIO6                   6
#define PIN_7_AADC_L2                 7
#define PIN_8_AADC_R2                 8
#define PIN_9_AADC_L3                 9
#define PIN_10_AADC_R3                10
#define PIN_11_AADC_L4                11
#define PIN_12_AADC_R4                12
#define PIN_13_AL0                    13
#define PIN_14_AL1                    14
#define PIN_15_AL2                    15
#define PIN_16_AL3                    16
#define PIN_17_AMUTE_F                17
#define PIN_18_AMUTE_R                18
#define PIN_19_AR0                    19
#define PIN_20_AR1                    20
#define PIN_21_AR2                    21
#define PIN_22_AR3                    22
#define PIN_23_DCLK                   23     
#define PIN_24_DCLK_IN_1              24     
#define PIN_25_DCLK_IN_2              25
#define PIN_26_DE                     26
#define PIN_27_EINT0                  27
#define PIN_28_EINT1                  28
#define PIN_29_EINT2                  29       
#define PIN_30_GPIO30                 30 
#define PIN_31_GPIO31                 31

#define PIN_32_EINT3                  32
#define PIN_33_HDMI_CEC_RX            33
#define PIN_34_HDMI_HDP_RX            34
#define PIN_35_HDMI_SCL_RX            35
#define PIN_36_HDMI_SDA_RX            36
#define PIN_37_HSYNC                  37
#define PIN_38_HSYNC_IN_1             38
#define PIN_39_HSYNC_IN_2             39   
#define PIN_40_I2S_IN1_BCK            40
#define PIN_41_I2S_IN1_D              41
#define PIN_42_GPIO42                 42
#define PIN_43_GPIO43                 43
#define PIN_44_GPIO44                 44
#define PIN_45_I2S_IN1_LRCK           45
#define PIN_46_I2S_IN1_MCLK           46
#define PIN_47_I2S_OUT0_BCK           47  
#define PIN_48_I2S_OUT0_D0            48
#define PIN_49_I2S_OUT0_D1            49
#define PIN_50_I2S_OUT0_D2            50
#define PIN_51_I2S_OUT0_LRCK          51  
#define PIN_52_I2S_OUT0_MCLK          52
#define PIN_53_GPIO53                 53
#define PIN_54_GPIO54                 54
#define PIN_55_GPIO55                 55
#define PIN_56_GPIO56                 56
#define PIN_57_GPIO57                 57
#define PIN_58_AE0N                   58      
#define PIN_59_AE0P                   59
#define PIN_60_AE1N                   60 
#define PIN_61_AE1P                   61  
#define PIN_62_AE2N                   62     
#define PIN_63_AE2P                   63

#define PIN_64_AE3N                   64      
#define PIN_65_AE3P                   65
#define PIN_66_AECKN                  66
#define PIN_67_AECKP                  67
#define PIN_68_AO3N                   68
#define PIN_69_AO3P                   69
#define PIN_70_GPIO70                 70
#define PIN_71_GPIO71                 71
#define PIN_72_GPIO72                 72
#define PIN_73_AOCKN                   73
#define PIN_74_GPIO74                 74
#define PIN_75_AOCKP                   75
#define PIN_76_AO2N                   76
#define PIN_77_A02P                   77
#define PIN_78_AO0N                   78
#define PIN_79_AO0P                   79

#define PIN_80_AOCKN                  80
#define PIN_81_AOCKP                  81
#define PIN_82_MHL_PWR_EN             82  
#define PIN_83_MHL_SENSE              83   
#define PIN_84_SD1_DO                 84
#define PIN_85_SD1_D1                 85
#define PIN_86_SD1_D2                 86
#define PIN_87_SD1_D3                 87
#define PIN_88_SD2_D0                 88
#define PIN_89_SD2_D1                 89      
#define PIN_90_SD2_D2                 90
#define PIN_91_SD2_D3                 91

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
#define PIN_113_SCL1                  113

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
#define PIN_130_URAT0_CTS             130
#define PIN_131_URAT0_RTS             131
#define PIN_132_URAT1_CTS             132
#define PIN_133_URAT1_RTS             133             
#define PIN_134_URXD0                 134
#define PIN_135_URXD1                 135
#define PIN_136_URXD2                 136
#define PIN_137_URXD3                 137
#define PIN_138_URXD4                 138
#define PIN_139_USB_DM_P0             139
#define PIN_140_USB_DM_P1             140

#define PIN_141_USB_DP_P0             141
#define PIN_142_USB_DP_P1             142
#define PIN_143_UTXD0                 143
#define PIN_144_UTXD1                 144
#define PIN_145_UTXD2                 145
#define PIN_146_UTXD3                 146
#define PIN_147_UTXD4                 147
#define PIN_148_VB0                   148
#define PIN_149_VB1                   149
#define PIN_150_GPIO150               150
#define PIN_151_VB2                   151
#define PIN_152_VB3                   152

#define PIN_153_VB4                   153
#define PIN_154_VB5                   154
#define PIN_155_VB6                   155
#define PIN_156_VB7                   156
#define PIN_157_VG0                   157
#define PIN_158_VG1                   158

#define PIN_159_VG2                   159
#define PIN_160_VG3                   160
#define PIN_161_VG4                   161
#define PIN_162_GPIO162               162
#define PIN_163_VG5                   163
#define PIN_164_VG6                   164
#define PIN_165_VG7                   165             
#define PIN_166_VGA_HYNCO             166
#define PIN_167_VGA_SCL               167
#define PIN_168_VGA_SDA               168
#define PIN_169_VGA_VSYNCO            169
#define PIN_170_VIN0                  170
#define PIN_171_VIN1                  171
#define PIN_172_VIN2                  172
#define PIN_173_VIN3                  173
#define PIN_174_VIN4                  174
#define PIN_175_VIN5                  175
#define PIN_176_VIN6                  176
#define PIN_177_VIN7                  177
#define PIN_178_VR0                   178
#define PIN_179_VR1                   179
#define PIN_180_VR2                   180
#define PIN_181_VR3                   181
#define PIN_182_VR4                   182
#define PIN_183_VR5                   183
#define PIN_184_VR6                   184
#define PIN_185_VR7                   185
#define PIN_186_VSYNC                 186
#define PIN_187_VSYNC_IN_1            187
#define PIN_188_VSYNC_IN_2            188
#define PIN_189_YIN0                  189
#define PIN_190_YIN1                  190
#define PIN_191_YIN2                  191
#define PIN_192_YIN3                  192
#define PIN_193_YIN4                  193

#define PIN_194_YIN5                  194
#define PIN_195_YIN6                  195

#define PIN_196_YIN7                  196
#define PIN_197_GPIO7                 197
#define PIN_198_AADC_L0               198
#define PIN_199_AADC_R0               199
#define PIN_200_AADC_L1               200
#define PIN_201_AADC_R1               201
#define PIN_202_NONE                  202    //0x90[10]
#define PIN_203_NONE                  202	 //0x90[11]
#define PIN_204_SD0_CLK               204
#define PIN_205_SD0_CMD               205
#define PIN_206_SD1_CLK               206
#define PIN_207_SD1_CMD               207
#define PIN_208_NONE                  208   //0x90[16]
#define PIN_209_NONE                  209   //0x90[17]
#define PIN_210_NONE                  210   //0x90[18]
#define PIN_211_SD2_CLK               211
#define PIN_212_SD2_CMD               212
#define PIN_213_SD2_D0                213
#define PIN_214_SD2_D1                214
#define PIN_215_SD2_D2                215
#define PIN_216_SD2_D3                216
#define PIN_217_IR                    217
#define PIN_218_OPWSRB                218
#define PIN_219_GPIO8                 219
#define PIN_220_GPIO9                 220


//------------------------End Gen GPIO---------------------------------

   
#define PIN_UNKNOWN         255
//#define TOTAL_GPIO_NUM      205
#define TOTAL_GPIO_NUM      221




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
#define UART0_FLWCTRL_SEL		  5
#define UART1_FLWCTRL_SEL         6
#define AP_SF_SEL                 8
#define USB3_I2C_SEL			  16
#define USB_I2C_SEL               18
#define USB_OTG_SEL				  20
#define USB3_SP_I2C_SEL			  22
#define VGA_OUT_SEL               24
#define MP1_MBIST_SEL             25
#define ARM11_JTAG_SEL            28

//------------------ padmux1 0X58 ---------------------------------
//PADMUX 1 -- 0X58
#define SGM_MIC_IN_SEL            (32 + 0)
#define AP_RS232_SEL_2_0          (32 + 18)
#define AP_RS232_SEL_3     		  (32 + 22)
#define PCM_SEL                   (32 + 23)
#define ARM9_JTAG_SEL             (32 + 25)
#define PWM0_SEL                  (32 + 28)

//------------------ padmux2 0X5C ---------------------------------
//PADMUX 2 -- 0X5C
#define PWM1_SEL                  (64 + 0)
#define DVIN_CLK_SEL			  (64 + 2)
#define DVIN_TIMING_SEL			  (64 + 3)
#define TTL_6_8B_SEL              (64 + 6)
#define TTL_8B_SEL                (64 + 7)
#define TTL_SYNC_SEL              (64 + 8)
#define TTL_DE_SEL                (64 + 9)
#define DVIN2_DATA_SEL            (64 + 10)
#define DVIN_DATA_SEL			  (64 + 11)
#define DVIN2_CLK_SEL			  (64 + 12)
#define DVIN2_TIMING_SEL		  (64 + 13)
#define SP0_SEL                   (64 + 16)
#define SP1_SEL                   (64 + 19)
#define NAND_FLASH_SEL            (64 + 22)
#define NAND_2ND_FLASH_SEL		  (64 + 23)
#define SPDIF_SEL                 (64 + 30)
#define AMUTE_R_SEL               (64 + 31)

//------------------ padmux3 0X60 ---------------------------------
//PADMUX 3 -- 0X60
#define AMUTE_F_SEL               (96 + 0)
#define I2S_OUT0_SEL              (96 + 8)
#define TEST_BUS_SEL              (96 + 10)
//------------------ padmux4 0X64 ---------------------------------

//PADMUX 4 -- 0X64
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
#define I2C1_SEL                  (192 + 26)

//------------------ padmux7 0x70 (ADD)---------------------------------
#define HDMI_I2C_SEL_1_0           (224 + 0)
#define MHL_SENSE_SEL              (224 + 3)
#define HDMI_CEC_SEL               (224 + 4)
#define HDMI_HDP_SEL               (224 + 5)
#define VGA_I2C_SEL_1_0            (224 + 6)
#define HDMI_SPDIF_SEL             (224 + 11)
#define CA7_DFD_SEL                (224 + 12)
#define CA7_MBIST_SEL              (224 + 14)
#define I2S_LINE0_IN_SEL           (224 + 16)
#define I2S_LINE1_IN_SEL           (224 + 20)
//-------------------------END PADMUX--------------------------------

#define MAX_PINMUX_SEL   (I2S_LINE1_IN_SEL + 1)

#define DEBUG_WR32(_reg_, _val_)  (*((volatile uint32_t*)((0xF0000000) + (_reg_))) = (_val_))
#define DEBUG_RE32(_reg_)         (*((volatile uint32_t*)((0xF0000000) + (_reg_))))


enum GPIO_IRQTYPE {
	GPIO_IRQTYPE_RISINGEDGE = 0,
	GPIO_IRQTYPE_FALLINGEDGE,
	GPIO_IRQTYPE_TWOEDGE,
	GPIO_IRQTYPE_HIGHLEVEL,
	GPIO_IRQTYPE_LOWLEVEL,
};

typedef enum{
    NORMAL = 0,
    PULLUP,
    PULLDOWN
}GPIO_PUD;


static const char * const mtk_gpio_functions[] = {
	"func0",   "func1",   "func2",   "func3",   "func4",   "func5",  "func6",   "func7",   "func8",   "func9", 
    "func10",  "func11",  "func12",  "func13",  "func14",  "func15", "func16",  "func17",  "func18",  "func19",
    "func20",  "func21",  "func22",  "func23",  "func24",  "func25", "func26",  "func27",  "func28",  "func29", 
    "func30",  "func31",  "func32",  "func33",  "func34",  "func35", "func36",  "func37",  "func38",  "func39",
	"func40",  "func41",  "func42",  "func43",  "func44",  "func45", "func46",  "func47",  "func48",  "func49", 
    "func50",  "func51",  "func52",  "func53",  "func54",  "func55", "func56",  "func57",  "func58",  "func59",
    "func60",  "func61",  "func62",  "func63",  "func64",  "func65", "func66",  "func67",  "func68",  "func69", 
    "func70",  "func71",  "func72",  "func73",  "func74",  "func75", "func76",  "func77",  "func78",  "func79",
    "func80",  "func81",  "func82",  "func83",  "func84",  "func85", "func86",  "func87",  "func88",  "func89", 
    "func90",  "func91",  "func92",  "func93",  "func94",  "func95", "func96", " func97",  "func98",  "func99",
    "func100", "func101", "func102", "func103", "func104", "func105","func106", "func107", "func108", "func109", 
    "func110", "func111", "func112", "func113", "func114", "func115","func116", "func117", "func118", "func119",
    "func120", "func121", "func122", "func123", "func124", "func125","func126", "func127", "func128", "func129", 
    "func130", "func131", "func132", "func133", "func134", "func135","func136", "func137", "func138", "func139",
    "func140", "func141", "func142", "func143", "func144", "func145","func146", "func147", "func148", "func149", 
    "func150", "func151", "func152", "func153", "func154", "func155","func156", "func157", "func158", "func159",
    "func160", "func161", "func162", "func163", "func164", "func165","func166", "func167", "func168", "func169", 
    "func170", "func171", "func172", "func173", "func174", "func175","func176", "func177", "func178", "func179",
    "func180", "func181", "func182", "func183", "func184", "func185","func186", "func187", "func188", "func189",
    "func190", "func191", "func192", "func193", "func194", "func195","func196", "func197", "func198", "func199", 
    "func200", "func201", "func202", "func203", "func204", "func205","func206", "func207", "func208", "func209", 
    "func210", "func211", "func212", "func213", "func214", "func215","func216", "func217", "func218", "func219",
    "func220", "func221", "func222", "func223", "func224", "func225","func226", "func227", "func228", "func229", 
    "func230", "func231", "func232", "func233", "func234", "func235","func236", "func237", "func238", "func239",
    "func240", "func241", "func242", "func243", "func244", "func245","func246", "func247", "func248", "func249", 
    "func250", "func251", "func252", "func253", "func254", "func255",
};

#define IO_VBASE_ADDR (0x10000000)
 
#define HAL_WRITE32(_reg_, _val_)  (*((volatile uint32_t*)((unsigned long)_reg_)) = (_val_))
#define HAL_READ32(_reg_)          (*((volatile uint32_t*)((unsigned long)_reg_)))

#define GPIOIO_READ32(base, offset)    HAL_READ32((base) + (offset))
#define GPIOIO_WRITE32(base, offset, value)  \
        HAL_WRITE32((base) + (offset), (value))

#define GPIO_823X_READ32(offset)       GPIOIO_READ32(IO_VBASE_ADDR, (offset))
#define GPIO_823X_WRITE32(offset, value)  \
        GPIOIO_WRITE32(IO_VBASE_ADDR, (offset), (value))

int GPIO_MultiFun_Set(int i4GpioNum,  int i4FuncSel);    //set gpio pinmux
void GPIO_Pull_UpDown(int i4GpioNum, unsigned PullUpOrDown);    //set gpio pull up or down
int ac823x_gpio_inout_sel_reg(unsigned gpio, int dir);    //set gpio direction
int ac823x_gpio_get_value_reg(unsigned gpio);    //get gpio value
int ac823x_gpio_set_value_reg(unsigned gpio, int value);    //set gpio value


  
#endif /* __AC823X_GPIO_PINMUX_H */



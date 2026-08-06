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
#ifndef  _AC823X_AUXADC_H
#define _AC823X_AUXADC_H

#define IO_BASE_VA          IO_UCV_BASE_FOR_KP

#define AUXADC_BIM_MODE 0

/*AUXADC_CON3*/
#define AUXADC_CON3_STATUS_mask         (0x0001)
#define AUXADC_STATUS_BUSY          	(0x01)
#define AUXADC_STATUS_IDLE          	(0x00) 


/*AUX_TS_CON*/
#define TS_CON_STATUS_MASK     			0x0002
#define TS_CON_STATUS_offset   			1
#define TS_CON_STATUS_NOTOUCH 			0x0000
#define TS_CON_STATUS_TOUCHED 			0x0002

#define TS_CON_SPL_MASK        			0x0001
#define TS_CON_SPL_offset      			0
#define TS_CON_SPL_NOACTION  			0x0000
#define TS_CON_SPL_TRIGGER    			0x0001


//------------------------------------------------------------------------------
// External Variables
//

//------------------------------------------------------------------------------
// External Functions
//

//------------------------------------------------------------------------------
// Global Variables
//

//------------------------------------------------------------------------------
// Defines
#define AUXADC_CON0                     0x000
#define AUXADC_CON1                     0x004
#define AUXADC_CON2                     0x008
#define AUXADC_CON3                     0x00C
#define AUXADC_DAT0                     0x010
#define AUXADC_DAT1                     0x014
#define AUXADC_DAT2                     0x018
#define AUXADC_DAT3                     0x01C
#define AUXADC_DAT4                     0x020
#define AUXADC_DAT5                     0x024

//Added for AC8317
#define AUXADC_DAT6                     0x028
#define AUXADC_DAT7                     0x02C
#define AUXADC_DAT8                     0x030
#define AUXADC_DAT9                     0x034
#define AUXADC_DAT10                    0x038
#define AUXADC_DAT11                    0x03C
#define AUXADC_DAT12                    0x040
#define AUXADC_DAT13                    0x044

#define AUXADC_TS_DEBT0                 0x050
#define AUXADC_TS_DEBT1                 0x054
#define AUXADC_TS_CMD                   0x058
#define AUXADC_TS_ADDR                  0x05C
#define AUXADC_TS_CON0                  0x060
#define AUXADC_TS_CON1                  0x064
#define AUXADC_TS_CON2                  0x068
#define AUXADC_TS_CON3                  0x06C
#define AUXADC_TS_DAT0                  0x070
#define AUXADC_TS_DAT1                  0x074
#define AUXADC_TS_DAT2                  0x078
#define AUXADC_TS_DAT3                  0x07C

#define AUXADC_TS_TEST			0x080
#define AUXADC_DET_VOLT                 0x088
#define AUXADC_DET_SEL			0X08C
#define AUXADC_DET_PERIOD		0X090
#define AUXADC_DET_DEBT			0X094
#define AUXADC_MISC			0X098
#define AUXADC_ECC			0X09C
#define AUXADC_SAMPLE_LIST		0X0A0
#define AUXADC_ABIST_PERIOD		0X0A4

#define AUXADC_TS_AUTO_CON		0X0A8
#define AUXADC_TS_AUTO_TIME_INTVL	0X0AC

#define AUXADC_TS_AUTO_X_DATn		0X200
#define AUXADC_TS_AUTO_Y_DATn		0X240
#define AUXADC_TS_AUTO_Z1_DATn		0X280
#define AUXADC_TS_AUTO_Z2_DATn		0X2C0

#define AUXADC_THERM_CON		0X180
#define AUXADC_THERM_DATA		0X184
#define AUXADC_F13M_EN			0X188
#define AUXADC_TMMA0_CNT		0X18C
#define AUXADC_TDMA1_CNT                0x190
#define AUXADC_PDN_CON                  0x194
#define AUXADC_KEY_PWMFB_EN             0x198
#define AUXADC_KEY_MASK			0X19C
#define AUXADC_PWM_CMPH			0X1A0
#define AUXADC_PWM_CMPL			0X1A4
#define AUXADC_KEY_PWM_IRQ_STA		0X1A8
#define AUXADC_MONI_SEL0		0X1B0
#define AUXADC_MONI_SEL1		0X1B4
#define AUXADC_MONI_SEL2		0X1B8
#define AUXADC_TP_RESERVE		0X1BC
//
// AUX_TS_CMD
#define AUX_TS_CMD_ADDRESS_Y  		0x01
#define AUX_TS_CMD_ADDRESS_Z1 		0x03
#define AUX_TS_CMD_ADDRESS_Z2 		0x04
#define AUX_TS_CMD_ADDRESS_X  		0x05

#define AUX_TS_CMD_12BIT_RES 		0x0
#define AUX_TS_CMD_10BIT_RES 		0x1

#define AUX_TS_CMD_MODE_DF 		0x0 //Differential mode
#define AUX_TS_CMD_MODE_SE 		0x1 //Single-end mode

#define AUX_TS_CMD_PD_YDRV_SH 	0x00
#define AUX_TS_CMD_PD_IRQ_SH 	0x01
#define AUX_TS_CMD_PD_IRQ 		0x03


// AUX_TS_CON
#define AUX_TS_CON_SPL_TRIGGER 		0x01
#define AUX_TS_CON_PRESS_STATUS_MASK 	0x02

#define AUX_BASE_ADDR			0xA9000

//------------------------------------------------------------------------------
#define ADC_WRITE32(offset, value)  	IO_WRITE32(IO_BASE_VA+AUX_BASE_ADDR, (offset), (value))
#define ADC_READ32(offset)          	IO_READ32(IO_BASE_VA+AUX_BASE_ADDR, (offset))
#define WR(addr, v) 			ADC_WRITE32(addr, v)
#define RR(addr) 			ADC_READ32(addr)


#define ADCKEY_VOLTAGE_RANGE_INVALID      (0x20)
#define ADCKEY_VOLTAGE_RANGE_A            (0xe3)  //227
#define ADCKEY_VOLTAGE_RANGE_B            (0x1ba) //442
#define ADCKEY_VOLTAGE_RANGE_C            (0x26c) //620//658
#define ADCKEY_VOLTAGE_RANGE_D            (0x2D0) //720//873
#define ADCKEY_VOLTAGE_RANGE_E            (0x422) //1058//1089
#define ADCKEY_VOLTAGE_RANGE_F            (0x4c4) //1220//1303
#define ADCKEY_VOLTAGE_RANGE_G            (0x5a5) //1445//1510
#define ADCKEY_VOLTAGE_RANGE_H            (0x681) //1665//1730
#define ADCKEY_VOLTAGE_RANGE_I            (0x758) //1880//1949
#define ADCKEY_VOLTAGE_RANGE_J            (0x834) //2100//2167
#define ADCKEY_VOLTAGE_RANGE_K            (0x90b) //2315//2382
#define ADCKEY_VOLTAGE_RANGE_L            (0x9c4) //2500//2593
#define ADCKEY_VOLTAGE_RANGE_M            (0xaf0) //2800//2813
#define ADCKEY_VOLTAGE_RANGE_N            (0xb9f) //2975//3029
#define ADCKEY_VOLTAGE_RANGE_O            (0xc4e) //3150//3245
#define ADCKEY_VOLTAGE_RANGE_P            (0xd80) //3456


#define ADCKEY_KEYCODE_A                   (30)
#define ADCKEY_KEYCODE_B                   (48)
#define ADCKEY_KEYCODE_C                   (46)
#define ADCKEY_KEYCODE_D                   (32)
#define ADCKEY_KEYCODE_E                   (18)
#define ADCKEY_KEYCODE_F                   (33)
#define ADCKEY_KEYCODE_G                   (34)
#define ADCKEY_KEYCODE_H                   (35)
#define ADCKEY_KEYCODE_I                   (23)
#define ADCKEY_KEYCODE_J                   (36)
#define ADCKEY_KEYCODE_K                   (37)
#define ADCKEY_KEYCODE_L                   (38)
#define ADCKEY_KEYCODE_M                   (50)
#define ADCKEY_KEYCODE_N                   (49)
#define ADCKEY_KEYCODE_O                   (24)
#define ADCKEY_KEYCODE_P                   (25)


#endif // _AC823X_AUXADC_H

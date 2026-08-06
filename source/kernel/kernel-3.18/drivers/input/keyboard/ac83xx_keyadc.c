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

#include "ac83xx_keyadc.h"
#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <linux/kernel.h>
#include <ac83xx_auxadc.h>
#include <linux/printk.h>
#include <linux/types.h>

u32 key_value[MAX_KEYS] = {
    ADCKEY_KEYCODE_A, 
    ADCKEY_KEYCODE_B, 
    ADCKEY_KEYCODE_C, 
    ADCKEY_KEYCODE_D,
    ADCKEY_KEYCODE_E,
    ADCKEY_KEYCODE_F,
    ADCKEY_KEYCODE_G,
    ADCKEY_KEYCODE_H,
    ADCKEY_KEYCODE_I,
    ADCKEY_KEYCODE_J,
    ADCKEY_KEYCODE_K, 
    ADCKEY_KEYCODE_L,
    ADCKEY_KEYCODE_M,
    ADCKEY_KEYCODE_N,
    ADCKEY_KEYCODE_O,
    ADCKEY_KEYCODE_P,
};
u32 key_voltage[MAX_KEYS] = {
    ADCKEY_VOLTAGE_RANGE_A,
    ADCKEY_VOLTAGE_RANGE_B,
    ADCKEY_VOLTAGE_RANGE_C,
    ADCKEY_VOLTAGE_RANGE_D,
    ADCKEY_VOLTAGE_RANGE_E,
    ADCKEY_VOLTAGE_RANGE_F,
    ADCKEY_VOLTAGE_RANGE_G,
    ADCKEY_VOLTAGE_RANGE_H,
    ADCKEY_VOLTAGE_RANGE_I,
    ADCKEY_VOLTAGE_RANGE_J,
    ADCKEY_VOLTAGE_RANGE_K,
    ADCKEY_VOLTAGE_RANGE_L,
    ADCKEY_VOLTAGE_RANGE_M,
    ADCKEY_VOLTAGE_RANGE_N,
    ADCKEY_VOLTAGE_RANGE_O,
    ADCKEY_VOLTAGE_RANGE_P,

};



bool AuxADCInitKeypad(void)
{
    u32 tmp = 0;

    {
        tmp = IO_READ32(IO_BASE_VA, 0XCC);
        tmp |= 0X00000002;
        IO_WRITE32(IO_BASE_VA, 0XCC, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XCC);

        tmp = IO_READ32(IO_BASE_VA, 0XB0);
        tmp |= 0X0000000E;
        IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XB0);


        tmp = IO_READ32(IO_BASE_VA, 0X6A0);
        tmp = tmp & (0xfffdffff);
        IO_WRITE32(IO_BASE_VA, 0X6A0, tmp);

    }

  
	tmp = ADC_READ32(AUXADC_PDN_CON);
	ADC_WRITE32(AUXADC_PDN_CON, 0);
	tmp = ADC_READ32(AUXADC_PDN_CON);
	tmp = ADC_READ32(AUXADC_MISC);
	tmp |= 0x200;
	ADC_WRITE32(AUXADC_MISC, tmp); //enable  ECC
  	tmp = ADC_READ32(AUXADC_MISC);
	
    tmp = 0x1118;
  	ADC_WRITE32(AUXADC_ECC, tmp); 

	ADC_WRITE32(AUXADC_PWM_CMPH, 0);
    ADC_WRITE32(AUXADC_PWM_CMPL, 0xfff);

	ADC_WRITE32(AUXADC_KEY_MASK, 0);
	
    ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x3fc);
    //ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x3 << 4);
    //ADC_WRITE32(AUXADC_CON0, 0x1 << 2);
    ////ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x3FF);
    
    ADC_WRITE32(AUXADC_CON0, 0x1f);
    ADC_WRITE32(AUXADC_TDMA1_CNT,0x460);//3ms

/*
    //ADC_WRITE32(AUXADC_TDMA1_CNT,0xBB8);//30ms
    //ADC_WRITE32(AUXADC_TDMA1_CNT,0x4b0);//5ms
    tmp = ADC_READ32(AUXADC_MISC);
    tmp |= 0x0200;
    ADC_WRITE32(AUXADC_MISC, tmp);

    ADC_WRITE32(AUXADC_ECC, 0x1118);
 */ 
    return true;
}

bool AuxADCDeInitKeypad(void)
{
    u32 tmp = 0;
    {
        tmp = IO_READ32(IO_BASE_VA, 0XCC);
        tmp = tmp & 0XFFFFFFFD;
        IO_WRITE32(IO_BASE_VA, 0XCC, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XCC);

        tmp = IO_READ32(IO_BASE_VA, 0XB0);
        tmp = tmp & 0XFFFFFFF1;
        IO_WRITE32(IO_BASE_VA, 0XB0, tmp);
        tmp = IO_READ32(IO_BASE_VA, 0XB0);

        tmp = ADC_READ32(AUXADC_PDN_CON);
        ADC_WRITE32(AUXADC_PDN_CON, 3);
        tmp = ADC_READ32(AUXADC_PDN_CON);
    }
  
    ADC_WRITE32(AUXADC_KEY_PWMFB_EN, 0x0);
    tmp = ADC_READ32(AUXADC_CON0);
    tmp = tmp & 0xFC0;
    ADC_WRITE32(AUXADC_CON0, tmp);
    ADC_WRITE32(AUXADC_TDMA1_CNT,0x1E0);//15ms
    return true;
}

u32 GET_KEY(u32 sample_value)
{
    u32 key;
    if (sample_value < ADCKEY_VOLTAGE_RANGE_INVALID) {//instable voltage
        key = 0xfff;
    } else if(sample_value > key_voltage[2]&&  
                    sample_value < key_voltage[3]) {
        key = 229;//recent apps
    }  else if(sample_value > key_voltage[4]&&
                    sample_value < key_voltage[5]) {
        key = 106;//right
    }  else if(sample_value > key_voltage[5]&&
                    sample_value < key_voltage[6]) {
        key = 115;//voice up
    } else if(sample_value > key_voltage[6]&& 
                   sample_value < key_voltage[7]) {
        key = 102;//home
    }  else if(sample_value > key_voltage[7]&&
                    sample_value < key_voltage[8]) {
        key = 103;//up
    } else if(sample_value > key_voltage[8]&&
                    sample_value < key_voltage[9]) {
        key = 28;//enter
    }else if(sample_value > key_voltage[9]&&
                    sample_value < key_voltage[10]) {
        key = 108;//down
    }else if(sample_value > key_voltage[10]&&
                   sample_value < key_voltage[11]) {
        key = 158;//back
    } else if(sample_value > key_voltage[12]&&
                   sample_value < key_voltage[13]) {
        key = 105;//left
    } else if(sample_value > key_voltage[13]&&
                   sample_value < key_voltage[14]) {
        key = 114;//voice down
    } else {
        key = 0xfff;
    }
    return key;
}

u32  ac83xx_knob_init(void)
{
    int tmp = 0;
	
    pr_debug("[KP]ac83xx_knob_init  START !\r\n");
    tmp = IO_READ32(IO_BASE_VA, 0X68);
    tmp |= 0x55;
    IO_WRITE32(IO_BASE_VA, 0X68, tmp);
    tmp = IO_READ32(BIM_UCV_BASE, 0Xa8);
    tmp |= 0x100000;
    IO_WRITE32(BIM_UCV_BASE, 0Xa8, tmp);
    tmp = IO_READ32(IO_BASE_VA, 0X550);
    tmp |= 0x6000;
    IO_WRITE32(IO_BASE_VA, 0X550, tmp);
    pr_debug("[KP]ac83xx_knob_init  DONE !\r\n");
	
    return 0;
}


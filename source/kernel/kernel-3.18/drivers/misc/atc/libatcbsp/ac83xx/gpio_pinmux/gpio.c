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
#include "x_bim.h"
//#include "x_pinmux.h"
//#include "x_gpio.h"
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>

#include "x_ckgen.h"
#include "gpio_debug.h"
#include "x_hal_ic.h"
#include "x_assert.h"
#include "x_os.h"
#include "drv_common.h"
#include "drv_uart.h" 
#include "drv_config.h"
#include "chip_ver.h"
#include "mach/gpio.h"

#if (CONFIG_DRV_LINUX)
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "x_major.h"
#include "x_module.h"
#endif

struct gpio_desc;

extern int gpio_configure(unsigned gpio, int dir, int value);
extern int gpio_inout_sel(unsigned gpio, int dir);
extern int gpiod_get_value(const struct gpio_desc *desc);
extern void gpiod_set_value(struct gpio_desc *desc, int value);
extern void gpio_table_check(void);
extern void gpio_verify(unsigned refergpio, unsigned gpio);
extern void gpio_related(unsigned gpio, int showall);


/******************************************************************************
* Local variable
******************************************************************************/

/****************************************************************************
** Function prototypes
****************************************************************************/
void GPIO_Config(INT32 i4GpioNum, INT32 i4Output, INT32 i4High);
void GPIO_InOut_Sel(INT32 i4GpioNum, INT32 i4Output);

//void GPIO_Output(struct gpio_desc *desc, int value);
//unsigned int GPIO_Input(const struct gpio_desc *desc);

 /*----------------------------------------------------------------------------
 * Function: GPIO_Init
 * Description:
 * 	 Initialize all GPIO pin.
 * Inputs:  
 * Outputs: 
 * Returns: 
 *---------------------------------------------------------------------------*/
#if (CONFIG_DRV_LINUX)
INT32 __init GPIO_Init(void)
#else
INT32 GPIO_Init(void)
#endif
{
#ifdef _TODO_
  	//Audio GPIO config
    GPIO_Config(AUD_DAC_RESET, OUTPUT, LOW);
    GPIO_Config(PCM_DSD_SEL, OUTPUT, LOW);
  	GPIO_Config(AUD_IN_RESET, OUTPUT, LOW);
  	//SIF GPIO config
	GPIO_Config(SIF_SDA, OUTPUT, LOW);
  	GPIO_Config(SIF_SCL, OUTPUT, LOW);
  
    GPIO_Config(NAND_WP, OUTPUT, HIGH);
  
  #ifdef HDMI_CEC_AS_GPIO
  GPIO_Config(MUX_SWITCH, OUTPUT, HIGH);
  #endif
  
  #ifdef USB_VBUS_CONT_P1_P2
    //USB VBus GPIO config
	GPIO_Config(USB_VBUS_PCONT1, OUTPUT, HIGH);
	GPIO_Config(USB_VBUS_PCONT2, OUTPUT, HIGH);
  #endif 

#endif

    return 0;
}

#if (CONFIG_DRV_LINUX)
 /*----------------------------------------------------------------------------
 * Function: i4GPIO_Uninit
 * Description:
 * 	 UnInitialize all GPIO pin.
 * Inputs:  
 * Outputs: 
 * Returns: 
 *---------------------------------------------------------------------------*/
INT32 __exit i4GPIO_Uninit(UINT32 u4Case)
{   
    return 0;
}
#endif

/*----------------------------------------------------------------------------
 * Function: GPIO_Config
 * Description:
 * 	 Configure GPIO pin. It sets pin as gpio¡Boutput or input mode and 
 *   set output value if output mode.
 * Inputs:
 *      i4GpioNum: the gpio number to be set.
 *      i4Output:  If the integer is 0, this function will set the mode of the
 *				   gpio number as input mode, otherwise set as output mode. 
 *      i4High: In output mode,if the integer is 0, this function will set the 
 *              bit of the gpio number as 0, otherwise set as 1. 
                If input mode, ignore it. 
 * Outputs:     
 * Returns:
 *---------------------------------------------------------------------------*/
void GPIO_Config(INT32 i4GpioNum, INT32 i4Output, INT32 i4High)
{
	gpio_configure(i4GpioNum, i4Output, i4High);
}

/*----------------------------------------------------------------------------
 * Function: GPIO_InOut_Sel     
 * Description:
 *      The GPIO input/output mode setting functions. It will check the
 *  i4GpioNum and set to related register bit as 0 or 1.  In this function, 0
 *  is input mode and 1 is output mode.
 *      
 * Inputs:
 *      i4GpioNum: the gpio number to set or read.
 *      i4Output:  If the integer is 0, this function will set the mode of the
 *				   gpio number as input mode, otherwise set as output mode.     
 * Outputs:     
 * Returns:
 *---------------------------------------------------------------------------*/
void GPIO_InOut_Sel(INT32 i4GpioNum, INT32 i4Output)
{       
    gpio_inout_sel(i4GpioNum, i4Output);
}  
       
/*----------------------------------------------------------------------------
 * Function: GPIO_Output    
 * Description:
 *      The GPIO output value setting functions. It will check the i4GpioNum
 *  and set to related register bit as 0 or 1.     
 * Inputs:
 *      i4GpioNum: the gpio number to set.
 *      i4High: If the integer is 0, this function will set the bit of the gpio
 *  			number as 0, otherwise set as 1.     
 * Outputs: 
 * Returns: 
 *---------------------------------------------------------------------------*/
void GPIO_Output(struct gpio_desc *desc, int value)
{
    gpiod_set_value(desc, value);
}

/*----------------------------------------------------------------------------
 * Function: GPIO_Input
 * Description:
 *      The GPIO input reading functions. It will check the i4GpioNum and read
 *  	related register bit to return.
 * Inputs:
 *      i4GpioNum: the gpio number to read.
 * Outputs: 
 * Returns: 
 *      GPIO input value.
 *---------------------------------------------------------------------------*/
unsigned int GPIO_Input(const struct gpio_desc *desc)
{
    return gpiod_get_value(desc);
}

/*----------------------------------------------------------------------------
 * Function: GPIO_Verify
 *---------------------------------------------------------------------------*/
void GPIO_Verify(INT32 i4ReferGpioNum, INT32 i4GpioNum)
{
    gpio_verify(i4ReferGpioNum, i4GpioNum);
}

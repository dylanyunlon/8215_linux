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

#ifndef __ASM_ARCH_GPIO_REG_H
#define __ASM_ARCH_GPIO_REG_H



int atc_set_gpio_dir(unsigned gpio, int dir);
int atc_get_gpio_in(unsigned gpio);
int atc_set_gpio_out(unsigned gpio, int value);
int ac83xx_gpio_direction_input_reg(unsigned gpio);
int ac83xx_gpio_direction_output_reg(unsigned gpio, int value);
void GPIO_Pull_UpDown(int gpionum, unsigned PullUpOrDown);


#endif

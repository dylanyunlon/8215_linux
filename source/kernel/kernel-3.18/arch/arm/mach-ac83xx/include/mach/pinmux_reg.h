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
#ifndef __ASM_ARCH_PINMUX_REG_H
#define __ASM_ARCH_PINMUX_REG_H


struct gpio_chip;

int __bsp_pinset(unsigned pinmux_sel, unsigned function);
int __bsp_pinget(unsigned pinmux_sel);
int GPIO_MultiFun_Set(int i4GpioNum,  int i4FuncSel);
unsigned int GPIO_MultiFun_Get(unsigned int i4GpioNum);

void GPIO_Pull_UpDown(int i4GpioNum, unsigned PullUpOrDown);
void GPIO_DriveCurrent_Set(int i4GpioNum,  int i4Current);
int ac83xx_gpio_direction_input(struct gpio_chip *chip, unsigned gpio);
int ac83xx_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int value);
int ac83xx_gpio_get_value(struct gpio_chip *chip, unsigned gpio);
void ac83xx_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value);
int ac83xx_gpio_to_irq(struct gpio_chip *chip, unsigned gpio);

#endif



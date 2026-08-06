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

#ifndef IRQ_PRIORITY_H
#define IRQ_PRIORITY_H

//#include <x_typedef.h>

#define CONFIG_AC83XX_IRQ_PRIORITY_ENABLE  1

//----------------------------------------------------------------------------
// IRQ Priority
typedef enum {
    IRQ_PRIORITY_NONE = 0,
    IRQ_PRIORITY_LOW,
    IRQ_PRIORITY_MID,
    IRQ_PRIORITY_HIGH
} IRQ_PRIORITY;

extern IRQ_PRIORITY BSP_GetIrqPriority(uint32_t irq_num);
extern uint32_t BSP_SetIrqPriority(uint32_t irq_num, IRQ_PRIORITY ePri);

extern void ac83xx_IrqEnter(unsigned int irq_num);
extern void ac83xx_IrqExit(unsigned int irq_num);

#endif /*IRQ_PRIORITY_H*/


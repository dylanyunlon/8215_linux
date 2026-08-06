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

#ifndef _BIM_H_
#define _BIM_H_

void mt33xx_mask_bim_irq(unsigned int virq);
void mt33xx_mask_ack_bim_irq(unsigned int virq);
void mt33xx_unmask_bim_irq(unsigned int virq);
u32 mt33xx_ismask_bim_irq(unsigned int virq);
u32 mt33xx_pending_bim_irq(unsigned int virq);

#endif /* _BIM_H_ */

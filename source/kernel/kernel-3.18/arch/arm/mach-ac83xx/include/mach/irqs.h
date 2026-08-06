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

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

#include <asm/mach-types.h>


#include <mach/83xx_irqs_vector.h>

#define CPU_BRINGUP_SGI 1
#define FIQ_SMP_CALL_SGI 13
#define FIQ_DBG_SGI 14

#ifndef NR_IRQS
#define	NR_IRQS		MAX_VECTOR_NUM
#endif


#endif

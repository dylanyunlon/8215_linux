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

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#include <mach/memory.h>

#define IO_SPACE_LIMIT 0xffffffff

#define __io(a)         ((void __iomem *)(PCI0_BASE + (a)))
#define __mem_pci(a)    (a)
#define __mem_isa(a)    (a)

#define __bim_writel(val,add)  __raw_writel(val,__io(BIM_BASE_VA+add))
#define __bim_readl(add)       __raw_readl(__io(BIM_BASE_VA+add))

#endif

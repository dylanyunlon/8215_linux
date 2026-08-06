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
#ifndef _CP_DEF_H

#define _CP_DEF_H

#include "drv_config.h"
#ifndef __ARM2__
#include <linux/clk.h>
#endif

#define IO_BASE_ADDRESS  0xFD000000

#ifndef __ARM2__
extern void __iomem *vcp_sysreg_base;
extern struct clk *clk_ac8317_vcp;
#endif

#if (CONFIG_DRV_LINUX)
extern void *x_alloc_aligned_dma_mem(u32 u4Size, u32 u4Align);
extern void x_free_aligned_dma_mem(void *pUser);

#else
extern void *x_alloc_aligned_nc_mem(u32 u4Size, u32 u4Align);
extern void x_free_aligned_nc_mem(void *pUser);

#endif


/* ********************************************************************* */
/* Module Related Base Address define										 */
/* ********************************************************************* */

#define CLR_PROC_OPERATION_EN (1)

#ifdef VCP_FOR_ANDROID
#define CLR_PROC_REG_ADDR   ( IO_BASE_ADDRESS + 0x42600 )

#define vWriteCP(dAddr, dVal)  (*(volatile DWRD *)(CLR_PROC_REG_ADDR + (dAddr)) = dVal)
#define dReadCP(dAddr)         (*(volatile DWRD *)(CLR_PROC_REG_ADDR + (dAddr)))
#define vWriteCPMsk(dAddr, dVal, dMsk) vWriteCP((dAddr), (dReadCP(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#endif

/* RGB Color Procession */
#define RGB_GAIN               (0x00)
#define RGB_OFFSET             (0x01)
#define RGB_GAMMA_ON_OFF       (0x02)
#define RGB_GAMMATABLE_WRITE   (0x03)
#define RGB_GAMMATABLE_READ    (0x04)
#define RGB_GAMA_READBACK_LOOP (0x05)
#define vWriteReg(dAddr, dVal)              (*(volatile u32*)(IO_BASE_ADDRESS + (dAddr)) = dVal)
#define dReadReg(dAddr)                     (*(volatile u32*)(IO_BASE_ADDRESS + (dAddr)))



#endif

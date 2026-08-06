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

#include "gpio_debug.h"
#include "x_hal_ic.h"
#include "x_assert.h"
#include "x_os.h"
#include "drv_common.h"
#include "drv_uart.h"
#include "drv_config.h"
#include "chip_ver.h"

#if (CONFIG_DRV_LINUX)
#include <linux/module.h>
#endif

extern int bsp_pinset(unsigned pinmux_sel, unsigned function);
extern int bsp_pinget(unsigned pinmux_sel);

extern INT32 BSP_PinSet(INT32 i4FuncSel, INT32 i4Func);
extern INT32 BSP_PinGet(INT32 i4FuncSel);

 /*----------------------------------------------------------------------------
 * Function: Pinmux_Init
 * Description:
 * 	 Initialize pin function.
 * Inputs:  
 * Outputs: 
 * Returns: 
 *---------------------------------------------------------------------------*/
#if (CONFIG_DRV_LINUX)
void __init Pinmux_Init(void)
#else
void Pinmux_Init(void)
#endif
{
#ifdef _TODO_
    #ifdef SUPPORT_EXT_HDMI
    if ((BSP_GetIcVersion() == IC_VER_C)||(BSP_GetIcVersion() >= IC_VER_E))
    {
		BSP_PinSet(AUD2_2CH_SEL2, 1); 		//2nd Audio output
		BSP_PinSet(AUD2_5CH_SEL2, 1); 		//2nd Audio output
		BSP_PinSet(AUD2_7CH_SEL2, 1); 		//2nd Audio output
		BSP_PinSet(AUD2_10CH_SEL2, 1); 		//2nd Audio output
		BSP_PinSet(AUD2_12CH_SEL2, 1); 		//2nd Audio output
		BSP_PinSet(AUD2_SPDIF_SEL2, 1); 	//2nd Audio SPDIF output
    }
    #endif

#endif
}
/*
INT32 BSP_PinSet(INT32 i4FuncSel, INT32 i4Func)
{
	return bsp_pinset(i4FuncSel, i4Func);
}
EXPORT_SYMBOL(BSP_PinSet);

INT32 BSP_PinGet(INT32 i4FuncSel)
{
    return bsp_pinget(i4FuncSel);
}
EXPORT_SYMBOL(BSP_PinGet);*/

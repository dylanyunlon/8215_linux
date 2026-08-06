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

#ifndef _DRV_S_SECTION_H_
#define _DRV_S_SECTION_H_

 /*--------------------------------------------------------------------------------------------
 * Description      - include "drv_s_section" will compiler the object into secure dram
 *------------------------------------------------------------------------------------------*/
#pragma arm section code="custom_sdram_info",rodata="custom_sdram_info",rwdata="custom_sdram_info" //compiler into Secure-D RAM

#endif //#ifndef _DRV_ESM_IF_H_


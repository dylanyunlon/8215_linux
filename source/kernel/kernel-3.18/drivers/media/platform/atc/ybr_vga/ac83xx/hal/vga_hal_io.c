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

#include "vga_hal_io.h"


u32 startbit(u32 fld)
{
	u32 i = 0;

	while ((fld & (u32)(0x1U << (i++))) == 0U)
		;

	return (i - 1U);
}

void vIO32Write1BMsk(u32 reg32, u8 val8, u8 msk8)
{
	u32 u4Val, u4Msk;
	u8 bu8;

	bu8 = (u8)(reg32 & 3U);
	reg32 &= ~3U;
	val8 &= msk8;
	u4Msk = ~(u32)(msk8 << ((u32)bu8 << 3));

	u4Val = (*(volatile u32 *)(reg32));
	u4Val = ((u4Val & u4Msk) | (u32)((u32)val8 << (u32)(bu8 << 3)));
	(*(volatile u32 *)(reg32) = (u4Val));

}

void vIO32Write2BMsk(u32 reg32, u16 val16, u16 msk16)
{
	u32 u4Val, u4Msk;
	u8 bu8;

	bu8 = (u8)(reg32 & 3U);
	/*ASSERT(bu8<3);*/

	reg32 &= ~3U;
	val16 &= msk16;
	u4Msk = ~(u32)(msk16 << ((u32)bu8 << 3));

	u4Val = (*(volatile u32 *)(reg32));
	u4Val = ((u4Val & u4Msk) | (u32)((u32)val16 << (u32)(bu8 << 3)));
	(*(volatile u32 *)(reg32) = (u4Val));
}

void vIO32Write4BMsk(u32 reg32, u32 val32, u32 msk32)
{
	/*ASSERT((reg32&3)==0);*/
	val32 &= msk32;
	(*(volatile u32 *)(reg32) = ((*(volatile u32 *)(reg32)) & ~msk32) | val32);

}



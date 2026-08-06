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


#include "x_hal_io.h"
#include "hal_io.h"
#include "x_assert.h"

#include "x_os.h"


static DEFINE_SPINLOCK(ac83xx_hdmi_hal_lock);

UINT16 u2HdmiRxIO32Read2B(UINT32 reg32)
{
	UINT32 addr = reg32 & ~3;

	switch (reg32 & 3) {
	/*default:*/
	case 0:
	case 2:
		return (*(volatile UINT16 *)(reg32));

	case 1:
		return ((*(volatile UINT32 *)(addr)) >> 8) & 0xffff;

	case 3:
		ASSERT((reg32 & 3) < 3);
		return ((*(volatile UINT32 *)(addr)) >> 24) & 0xff;
	default:
		return (*(volatile UINT16 *)(reg32));
	}
}

void vHdmiRxIO32Write1BMsk(UINT32 reg32, UINT32 val8, UINT8 msk8)
{	
	UINT32 u4Val, u4Msk;
	UINT8 bByte;
	unsigned long flags;

	bByte = reg32 & 3;
	reg32 &= ~3;
	val8 &= msk8;
	u4Msk = ~(UINT32)(msk8 << ((UINT32)bByte << 3));

	spin_lock_irqsave(&ac83xx_hdmi_hal_lock, flags);
	u4Val = (*(volatile UINT32 *)(reg32));
	u4Val = ((u4Val & u4Msk) | ((UINT32)val8 << (bByte << 3)));
	(*(volatile UINT32 *)(reg32) = (u4Val));
	spin_unlock_irqrestore(&ac83xx_hdmi_hal_lock, flags);

}

void vHdmiRxIO32Write2BMsk(UINT32 reg32, UINT32 val16, UINT16 msk16)
{
    unsigned long flags;
	UINT32 u4Val, u4Msk;
	UINT8 bByte;

	bByte = reg32 & 3;
	ASSERT(bByte < 3);

	reg32 &= ~3;
	val16 &= msk16;
	u4Msk = ~(UINT32)(msk16 << ((UINT32)bByte << 3));

	spin_lock_irqsave(&ac83xx_hdmi_hal_lock, flags);
	u4Val = (*(volatile UINT32 *)(reg32));
	u4Val = ((u4Val & u4Msk) | ((UINT32)val16 << (bByte << 3)));
	(*(volatile UINT32 *)(reg32) = (u4Val));
	spin_unlock_irqrestore(&ac83xx_hdmi_hal_lock, flags);

}

void vHdmiRxIO32Write4BMsk(UINT32 reg32, UINT32 val32, UINT32 msk32)
{
    unsigned long flags;
	ASSERT((reg32 & 3) == 0);

	val32 &= msk32;

	spin_lock_irqsave(&ac83xx_hdmi_hal_lock, flags);
	(*(volatile UINT32 *)(reg32) = ((*(volatile UINT32 *)(reg32)) & ~msk32) | val32);
	spin_unlock_irqrestore(&ac83xx_hdmi_hal_lock, flags);
}

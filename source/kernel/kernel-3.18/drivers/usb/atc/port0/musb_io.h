/*
 * MUSB OTG driver register I/O
 *
 * Copyright 2005 Mentor Graphics Corporation
 * Copyright (C) 2005-2006 by Texas Instruments
 * Copyright (C) 2006-2007 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __MUSB_LINUX_PLATFORM_ARCH_H__
#define __MUSB_LINUX_PLATFORM_ARCH_H__

#include <linux/io.h>
#include <linux/spinlock.h>
#include <generated/atc_project.h>

extern bool atc_usb_power;
extern bool usb_enable_clock(bool enable);
extern spinlock_t usb_io_lock;

#ifdef CONFIG_ATC_PLATFORM_ac83xx
static inline u16 musb_readw(const void __iomem *addr, unsigned offset)
{
	u16 rc = 0;

	if (atc_usb_power) {
		rc = ioread16((void *)(addr + offset));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		rc = ioread16((void *)(addr + offset));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
	return rc;
}

static inline u32 musb_readl(const void __iomem *addr, unsigned offset)
{
	u32 rc = 0;

	if (atc_usb_power) {
		rc = ioread32((void *)(addr + offset));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		rc = ioread32((void *)(addr + offset));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
	return rc;
}


static inline void musb_writew(void __iomem *addr, unsigned offset, u16 data)
{
	volatile uint32_t i4TmpVar;
	if (atc_usb_power) {
		i4TmpVar = ioread32((void *)(addr + ((offset) & 0xFFFFFFFC)));
		i4TmpVar &= ~(((uint32_t)0xFFFF) << (8*((offset) & 0x03)));
		i4TmpVar |= (uint32_t)((data) << (8*((offset) & 0x03)));
		iowrite32(i4TmpVar, (void *)(((uint32_t)addr) + ((offset) & 0xFFFFFFFC)));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		i4TmpVar = ioread32((void *)(addr + ((offset) & 0xFFFFFFFC)));
		i4TmpVar &= ~(((uint32_t)0xFFFF) << (8*((offset) & 0x03)));
		i4TmpVar |= (uint32_t)((data) << (8*((offset) & 0x03)));
		iowrite32(i4TmpVar, (void *)(((uint32_t)addr) + ((offset) & 0xFFFFFFFC)));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
}

static inline void musb_writel(void __iomem *addr, unsigned offset, u32 data)
{
	if (atc_usb_power) {
		iowrite32(data, (void *)(addr + offset));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		iowrite32(data, (void *)(addr + offset));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
}

static inline u8 musb_readb(const void __iomem *addr, unsigned offset)
{
	u8 rc = 0;

	if (atc_usb_power) {
		//rc = readb(addr + offset);
		rc = ioread8((void *)(addr + offset));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		//rc = readb(addr + offset);
		rc = ioread8((void *)(addr + offset));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
	return rc;
}

static inline void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
{
	volatile uint32_t i4TmpVar;

	if (atc_usb_power) {
		i4TmpVar = ioread32((void *)(addr + ((offset) & 0xFFFFFFFC)));
		i4TmpVar &= ~(((uint32_t)0xFF) << (8*((offset) & 0x03)));
		i4TmpVar |= (uint32_t)(((data) & 0xFF) << (8*((offset) & 0x03)));
		iowrite32(i4TmpVar, (void *)(((uint32_t)addr) + ((offset) & 0xFFFFFFFC)));
	} else {
		unsigned long flags = 0;

		spin_lock_irqsave(&usb_io_lock, flags);
		usb_enable_clock(true);
		DBG(1, "[MUSB]:access %s function when usb clock is off 0x%X\n", __func__, offset);
		i4TmpVar = ioread32((void *)(addr + ((offset) & 0xFFFFFFFC)));
		i4TmpVar &= ~(((uint32_t)0xFF) << (8*((offset) & 0x03)));
		i4TmpVar |= (uint32_t)(((data) & 0xFF) << (8*((offset) & 0x03)));
		iowrite32(i4TmpVar, (void *)(((uint32_t)addr) + ((offset) & 0xFFFFFFFC)));
		usb_enable_clock(false);
		spin_unlock_irqrestore(&usb_io_lock, flags);
	}
}
#else
#define MUSB_FADDR		0x00	/* 8-bit */
#define MUSB_POWER		0x01	/* 8-bit */

#define MUSB_INTRTX		0x02	/* 16-bit */
#define MUSB_INTRRX		0x04
#define MUSB_INTRTXE		0x06
#define MUSB_INTRRXE		0x08
#define MUSB_INTRUSB		0x0A	/* 8 bit */

static inline void musb_writel(void __iomem *addr, unsigned offset, u32 data)
{
	//DBG(1, "[MUSB]:data 0x%08x access %s 0x%X\n", data,__func__, offset);
	*((volatile uint32_t *)((addr)  + offset)) = data;
}

static inline u32 musb_readl(const void __iomem *addr, unsigned offset)
{

	u32 rc = 0;
	rc = *((volatile uint32_t *)((addr) + offset));
	//DBG(1, "[MUSB]:data 0x%08x access %s 0x%X\n", rc,__func__, offset);
	return rc;
}


static inline u16 musb_readw(const void __iomem *addr, unsigned offset)
{
	u16 rc = 0;
	rc =  *((volatile uint16_t *)((addr)  + offset));
	//DBG(1, "[MUSB]:data 0x%04x access %s 0x%X\n", rc,__func__, offset);
	return rc;
}

static inline void musb_writew(void __iomem *addr, unsigned offset, u16 data)
{
	volatile uint32_t u4TmpVar;  
	//DBG(1, "[MUSB]:data 0x%04x access %s 0x%X\n", data,__func__, offset);
	u4TmpVar = *((volatile uint32_t*)((addr)  + ((offset) & 0xFFFFFFFC))); 
	u4TmpVar &= ~(((uint32_t)0xFFFF) << (8*((offset) & 0x03)));                                 
	u4TmpVar |= (data) << (8*((offset) & 0x03)); 
	*((volatile uint32_t*)((addr)  + ((offset) & 0xFFFFFFFC))) = u4TmpVar; 
}

	
static inline u8 musb_readb(const void __iomem *addr, unsigned offset)
{
	u8 rc = 0;
	rc = *((volatile uint8_t *)((addr)  + offset));
	//DBG(1, "[MUSB]:data 0x%02x access %s 0x%X\n", rc,__func__, offset);
	return rc;
}
	
static inline void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
{

	volatile uint32_t u4TmpVar;  
	u4TmpVar = *((volatile uint32_t*)((addr)  + ((offset) & 0xFFFFFFFC))); 
	u4TmpVar &= ~(((uint32_t)0xFF) << (8*((offset) & 0x03)));									
	u4TmpVar |= (uint32_t)(((data) & 0xFF) << (8*((offset) & 0x03)));	
	*((volatile uint32_t*)((addr)  + ((offset) & 0xFFFFFFFC))) = u4TmpVar; 
	//DBG(1, "[MUSB]:data 0x%02x access %s 0x%X\n", data,__func__, offset);
}


#endif
#endif

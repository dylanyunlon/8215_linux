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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   $Workfile: rtc_hal.h $
 *
 * Description:
 * ------------
 *   Real Time Clock header file
 *
 * Author:
 * -------
 *   
 *
****************************************************************************/
#ifndef _HW_RTC_HW_H_
#define _HW_RTC_HW_H_


#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>

#ifndef IO_UCV_BASE
#define IO_UCV_BASE  0xFD000000
#endif

//RTC Registers define
#define RTC_BBPU                    0x00
#define RTC_IRQ_STA                 0x04
#define RTC_IRQ_EN                  0x08
#define RTC_CII_EN                  0x0c
#define RTC_AL_MASK                 0x10
#define RTC_TC_SEC                  0x14
#define RTC_TC_MIN                  0x18
#define RTC_TC_HOU                  0x1c
#define RTC_TC_DOM                  0x20
#define RTC_TC_DOW                  0x24
#define RTC_TC_MTH                  0x28
#define RTC_TC_YEA                  0x2c
#define RTC_AL_SEC                  0x30
#define RTC_AL_MIN                  0x34
#define RTC_AL_HOU                  0x38
#define RTC_AL_DOM                  0x3c
#define RTC_AL_DOW                  0x40
#define RTC_AL_MTH                  0x44
#define RTC_AL_YEA                  0x48
#define RTC_POWERKEY1               0x50
#define RTC_POWERKEY2               0x54
#define RTC_PDN1                    0x58
#define RTC_PDN2                    0x5c
#define RTC_SPAR1                   0x64
#define RTC_PROT                    0x68
#define RTC_DIFF                    0x6c
#define ANA_OUT                     0x70
#define RTC_WRTGR                   0x74
#define RTC_ANA                     0x7c

#define ALSTA                       (1 << 0)
#define TCSTA                       (1 << 1)

#ifndef RTC_UCV_BASE
#define RTC_UCV_BASE                        	(IO_UCV_BASE + 0x51000)
#endif

#define RTC_IO_READ32(offset)			IO_READ32(IO_UCV_BASE, (offset))
#define RTC_IO_WRITE32(offset, value)		IO_WRITE32(IO_UCV_BASE, (offset), (value))

#define RTC_READ32(offset)			IO_READ32(RTC_UCV_BASE, (offset))
#define RTC_WRITE32(offset, value)	    	IO_WRITE32(RTC_UCV_BASE, (offset), (value))

#define RTC_SET_BIT(offset, Bit)        	RTC_WRITE32(offset, RTC_READ32(offset) | (Bit))
#define RTC_CLR_BIT(offset, Bit)        	RTC_WRITE32(offset, RTC_READ32(offset) & (~(Bit)))


#endif //_HW_RTC_HW_H_

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

#ifndef X_HAL_IO_H
#define X_HAL_IO_H

//#include <generated/atc_project.h>
#include "x_typedef.h"

#include "x_ioopt.h"
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "ac83xx_io_macros.h"
#endif

#include "x_typedef.h"

//===========================================================================

// Type of IO register, for recording the address of an IO register
typedef volatile UINT32             HAL_IO_REGISTER;
#if 0
// Macros of register read
#define HAL_READ8(_reg_)            (*((volatile UINT8*)(_reg_)))
#define HAL_READ16(_reg_)           (*((volatile UINT16*)(_reg_)))
#define HAL_READ32(_reg_)           (*((volatile UINT32*)(_reg_)))

// Macros of register write
#define HAL_WRITE8(_reg_, _val_)    (*((volatile UINT8*)(_reg_)) = (_val_))
#define HAL_WRITE16(_reg_, _val_)   (*((volatile UINT16*)(_reg_)) = (_val_))
#define HAL_WRITE32(_reg_, _val_)   (*((volatile UINT32*)(_reg_)) = (_val_))

// Macros for read/write access
#define HAL_REG8(_reg_)				HAL_READ8((_reg_))
#define HAL_REG16(_reg_)			HAL_READ16((_reg_))
#define HAL_REG32(_reg_)			HAL_READ32((_reg_))
#else
#endif
//===========================================================================

// Bit field definitions
#define BIT0                        0x00000001
#define BIT1                        0x00000002
#define BIT2                        0x00000004
#define BIT3                        0x00000008
#define BIT4                        0x00000010
#define BIT5                        0x00000020
#define BIT6                        0x00000040
#define BIT7                        0x00000080
#define BIT8                        0x00000100
#define BIT9                        0x00000200
#define BIT10                       0x00000400
#define BIT11                       0x00000800
#define BIT12                       0x00001000
#define BIT13                       0x00002000
#define BIT14                       0x00004000
#define BIT15                       0x00008000
#define BIT16                       0x00010000
#define BIT17                       0x00020000
#define BIT18                       0x00040000
#define BIT19                       0x00080000
#define BIT20                       0x00100000
#define BIT21                       0x00200000
#define BIT22                       0x00400000
#define BIT23                       0x00800000
#define BIT24                       0x01000000
#define BIT25                       0x02000000
#define BIT26                       0x04000000
#define BIT27                       0x08000000
#define BIT28                       0x10000000
#define BIT29                       0x20000000
#define BIT30                       0x40000000
#define BIT31                       0x80000000

#endif  // X_HAL_IO_H

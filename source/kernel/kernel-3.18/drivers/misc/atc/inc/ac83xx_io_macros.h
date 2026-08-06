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

#ifndef __AC83XX_IO_MACROS_H
#define __AC83XX_IO_MACROS_H


#define IO_VIRT_N             0xFD000000 
#define BIM_UCV_BASE_N                            (IO_VIRT_N + 0x08000)
#define CKGEN_VIRT_N                              (IO_VIRT_N + 0x00000)

//x_hal_io.h

// Macros of register read
#define HAL_READ8(_reg_)            (*((volatile UINT8*)(_reg_)))
#define HAL_READ16(_reg_)           (*((volatile UINT16*)(_reg_)))
//#define HAL_READ32(_reg_)           (*((volatile UINT32*)(_reg_)))
#if 0
// Macros of register write
#define HAL_WRITE8(_reg_, _val_)    (*((volatile UINT8*)(_reg_)) = (_val_))
#define HAL_WRITE16(_reg_, _val_)   (*((volatile UINT16*)(_reg_)) = (_val_))
#define HAL_WRITE32(_reg_, _val_)   (*((volatile UINT32*)(_reg_)) = (_val_))
#endif
// Macros for read/write access
#define HAL_REG8(_reg_)				HAL_READ8((_reg_))
#define HAL_REG16(_reg_)			HAL_READ16((_reg_))



//ac83xx.h
#ifdef __ARM2__
#define HAL_WRITE32(_reg_, _val_)   (*((volatile UINT32*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)           (*((volatile UINT32*)(_reg_)))
#define HAL_REG32(_reg_)			HAL_READ32((_reg_))
#else
#define HAL_WRITE32(_reg_, _val_)   		(*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)           		(*((volatile uint32_t*)(_reg_)))
#endif



#define IO_READ32(base, offset)                 HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)         HAL_WRITE32((base) + (offset), (value))
#define	IO_REG32(base, offset)									HAL_READ32((base) + (offset))

#define CKGEN_READ32(offset)           		IO_READ32(CKGEN_VIRT_N, (offset))
#define CKGEN_WRITE32(offset, value)   		IO_WRITE32(CKGEN_VIRT_N, (offset), (value))
#if 0
#define BIM_READ32(offset)           		IO_READ32(BIM_VIRT, (offset))
#define BIM_WRITE32(offset, value)   		IO_WRITE32(BIM_VIRT, (offset), (value))
#define BIM_REG32(offset)   						IO_REG32(BIM_VIRT, (offset))

#define BIM2_READ32(offset)           		IO_READ32(BIM2_VIRT, (offset))
#define BIM2_WRITE32(offset, value)   		IO_WRITE32(BIM2_VIRT, (offset), (value))
#define BIM2_REG32(offset)   						IO_REG32(BIM2_VIRT, (offset))
#endif
#define PDWNC_READ32(offset)           		IO_READ32(PDWNC_VIRT, (offset))
#define PDWNC_WRITE32(offset, value)   		IO_WRITE32(PDWNC_VIRT, (offset), (value))

//x_hal_ic.h
#if 0
//============================================================================
// Macros for register read
//============================================================================
#define IO_READ8(base, offset)                          HAL_READ8((base) + (offset))
#define IO_READ16(base, offset)                         HAL_READ16((base) + (offset))
#define IO_READ32(base, offset)                         HAL_READ32((base) + (offset))

//============================================================================
// Macros for register write
//============================================================================
#define IO_WRITE8(base, offset, value)                  HAL_WRITE8((base) + (offset), (value))
#define IO_WRITE16(base, offset, value)                 HAL_WRITE16((base) + (offset), (value))
#define IO_WRITE32(base, offset, value)                 HAL_WRITE32((base) + (offset), (value))
#endif
//============================================================================
// Macros for register read/write access
//============================================================================
#define IO_REG8(base, offset)                           HAL_REG8((base) + (offset))
#define IO_REG16(base, offset)                          HAL_REG16((base) + (offset))
//#define IO_REG32(base, offset)                          HAL_REG32((base) + (offset))


//ac83xx_basic.h
#if 0
#define HAL_WRITE32(_reg_, _val_)   		(*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)           		(*((volatile uint32_t*)(_reg_)))

#define IO_READ32(base, offset)                 HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)         HAL_WRITE32((base) + (offset), (value))
#define	IO_REG32(base, offset)									HAL_READ32((base) + (offset))

#define CKGEN_READ32(offset)           		IO_READ32(CKGEN_VIRT_N, (offset))
#define CKGEN_WRITE32(offset, value)   		IO_WRITE32(CKGEN_VIRT_N, (offset), (value))

#define BIM_READ32(offset)           		IO_READ32(BIM_VIRT, (offset))
#define BIM_WRITE32(offset, value)   		IO_WRITE32(BIM_VIRT, (offset), (value))
#define BIM_REG32(offset)   						IO_REG32(BIM_VIRT, (offset))

#define BIM2_READ32(offset)           		IO_READ32(BIM2_VIRT, (offset))
#define BIM2_WRITE32(offset, value)   		IO_WRITE32(BIM2_VIRT, (offset), (value))
#define BIM2_REG32(offset)   						IO_REG32(BIM2_VIRT, (offset))

#define PDWNC_READ32(offset)           		IO_READ32(PDWNC_VIRT, (offset))
#define PDWNC_WRITE32(offset, value)   		IO_WRITE32(PDWNC_VIRT, (offset), (value))
#endif
#define __bim_writel(val,add)  __raw_writel(val,__io(BIM_BASE_VA+add))
#define __bim_readl(add)       __raw_readl(__io(BIM_BASE_VA+add))

//x_bim_83xx.h
//============================================================================
// Macros for register read/write
//============================================================================
#if 1
#define BIM_READ8(offset)            IO_READ8(BIM_UCV_BASE_N, offset)
#define BIM_READ16(offset)           IO_READ16(BIM_UCV_BASE_N, offset)
#define BIM_READ32(offset)           IO_READ32(BIM_UCV_BASE_N, offset)

#define BIM_WRITE8(offset, value)    IO_WRITE8(BIM_UCV_BASE_N, offset, (value))
#define BIM_WRITE16(offset, value)   IO_WRITE16(BIM_UCV_BASE_N, offset, (value))
#define BIM_WRITE32(offset, value)   IO_WRITE32(BIM_UCV_BASE_N, offset, (value))

#define BIM_REG8(offset)             IO_REG8(BIM_UCV_BASE_N, offset)
#define BIM_REG16(offset)            IO_REG16(BIM_UCV_BASE_N, offset)
#define BIM_REG32(offset)            IO_REG32(BIM_UCV_BASE_N, offset)

#define BIM2_READ8(offset)           IO_READ8(BIM_1_UCV_BASE, offset)
#define BIM2_READ16(offset)          IO_READ16(BIM_1_UCV_BASE, offset)
#define BIM2_READ32(offset)          IO_READ32(BIM_1_UCV_BASE, offset)

#define BIM2_WRITE8(offset, value)   IO_WRITE8(BIM_1_UCV_BASE, offset, (value))
#define BIM2_WRITE16(offset, value)  IO_WRITE16(BIM_1_UCV_BASE, offset, (value))
#define BIM2_WRITE32(offset, value)  IO_WRITE32(BIM_1_UCV_BASE, offset, (value))

#define BIM2_REG8(offset)            IO_REG8(BIM_1_UCV_BASE, offset)
#define BIM2_REG16(offset)           IO_REG16(BIM_1_UCV_BASE, offset)
#define BIM2_REG32(offset)           IO_REG32(BIM_1_UCV_BASE, offset)
#endif
#define SBIM_READ8(offset)           IO_READ8(SBIM_UCV_BASE_N, offset)
#define SBIM_READ16(offset)          IO_READ16(SBIM_UCV_BASE_N, offset)
#define SBIM_READ32(offset)          IO_READ32(SBIM_UCV_BASE_N, offset)

#define SBIM_WRITE8(offset, value)   IO_WRITE8(SBIM_UCV_BASE_N, offset, (value))
#define SBIM_WRITE16(offset, value)  IO_WRITE16(SBIM_UCV_BASE_N, offset, (value))
#define SBIM_WRITE32(offset, value)  IO_WRITE32(SBIM_UCV_BASE_N, offset, (value))
//============================================================================
// Macros for memory read/write
//============================================================================
#define WRITEMEM8(Address, Value)       *(volatile UINT8 *)(Address) = Value
#define READMEM8(Address)               *(volatile UINT8 *)(Address)

#define WRITEMEM16(Address, Value)      *(volatile UINT16 *)(Address) = Value
#define READMEM16(Address)              *(volatile UINT16 *)(Address)

#define WRITEMEM32(Address, Value)      *(volatile UINT32 *)(Address) = Value
#define READMEM32(Address)              *(volatile UINT32 *)(Address)

//x_ckgen.h
#if 0
#define CKGEN_READ8(offset)            IO_READ8(CKGEN_UCV_BASE, (offset))
#define CKGEN_READ16(offset)           IO_READ16(CKGEN_UCV_BASE, (offset))
#define CKGEN_READ32(offset)           IO_READ32(CKGEN_UCV_BASE, (offset))

#define CKGEN_WRITE8(offset, value)    IO_WRITE8(CKGEN_UCV_BASE, (offset), (value))
#define CKGEN_WRITE16(offset, value)   IO_WRITE16(CKGEN_UCV_BASE, (offset), (value))
#define CKGEN_WRITE32(offset, value)   IO_WRITE32(CKGEN_UCV_BASE, (offset), (value))

#define CKGEN_REG8(offset)             IO_REG8(CKGEN_UCV_BASE, (offset))
#define CKGEN_REG16(offset)            IO_REG16(CKGEN_UCV_BASE, (offset))
#define CKGEN_REG32(offset)            IO_REG32(CKGEN_UCV_BASE, (offset))

#define CKGEN_SETBIT(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define CKGEN_CLRBIT(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))
#endif

#endif /* __AC83XX_IO_MACROS_H */


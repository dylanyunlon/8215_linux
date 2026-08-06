#ifndef X_BIM_H
#define X_BIM_H

//============================================================================
// Include files
//============================================================================
#include "targetConfig.h"
#include "x_hal_ic.h"
#include "x_typedef.h"

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360) 
#include "x_bim_3360.h"
#include "x_sbim_3360.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580) 
#include "x_bim_8580.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356) 
#include "x_bim_3356.h"
#include "x_sbim_3356.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363) 
#include "x_bim_3363.h"
#endif


//============================================================================
// Macros for register read/write
//============================================================================
#define BIM_READ8(offset)            IO_READ8(BIM_BASE, offset)
#define BIM_READ16(offset)           IO_READ16(BIM_BASE, offset)
#define BIM_READ32(offset)           IO_READ32(BIM_BASE, offset)

#define BIM_WRITE8(offset, value)    IO_WRITE8(BIM_BASE, offset, (value))
#define BIM_WRITE16(offset, value)   IO_WRITE16(BIM_BASE, offset, (value))
#define BIM_WRITE32(offset, value)   IO_WRITE32(BIM_BASE, offset, (value))

#define BIM_REG8(offset)             IO_REG8(BIM_BASE, offset)
#define BIM_REG16(offset)            IO_REG16(BIM_BASE, offset)
#define BIM_REG32(offset)            IO_REG32(BIM_BASE, offset)

#define BIM2_READ8(offset)           IO_READ8(BIM_1_BASE, offset)
#define BIM2_READ16(offset)          IO_READ16(BIM_1_BASE, offset)
#define BIM2_READ32(offset)          IO_READ32(BIM_1_BASE, offset)

#define BIM2_WRITE8(offset, value)   IO_WRITE8(BIM_1_BASE, offset, (value))
#define BIM2_WRITE16(offset, value)  IO_WRITE16(BIM_1_BASE, offset, (value))
#define BIM2_WRITE32(offset, value)  IO_WRITE32(BIM_1_BASE, offset, (value))

#define BIM2_REG8(offset)            IO_REG8(BIM_1_BASE, offset)
#define BIM2_REG16(offset)           IO_REG16(BIM_1_BASE, offset)
#define BIM2_REG32(offset)           IO_REG32(BIM_1_BASE, offset)

#define SBIM_READ32(offset)          IO_READ32(SBIM_BASE, offset)
#define SBIM_WRITE32(offset, value)  IO_WRITE32(SBIM_BASE, offset, (value))
//============================================================================
// Macros for memory read/write
//============================================================================
#define WRITEMEM8(Address, Value)    *(volatile UINT8 *)(Address) = Value
#define READMEM8(Address)            *(volatile UINT8 *)(Address)

#define WRITEMEM16(Address, Value)   *(volatile UINT16 *)(Address) = Value
#define READMEM16(Address)           *(volatile UINT16 *)(Address)

#define WRITEMEM32(Address, Value)   *(volatile UINT32 *)(Address) = Value
#define READMEM32(Address)           *(volatile UINT32 *)(Address)


#endif


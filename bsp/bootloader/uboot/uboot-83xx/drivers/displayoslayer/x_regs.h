#ifndef X_REGS_H
#define X_REGS_H

#include "x_typedef.h"

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

//============================================================================
// Macros for register read/write access
//============================================================================
#define IO_REG8(base, offset)                           HAL_REG8((base) + (offset))
#define IO_REG16(base, offset)                          HAL_REG16((base) + (offset))
#define IO_REG32(base, offset)                          HAL_REG32((base) + (offset))

#endif  // X_REGS_H
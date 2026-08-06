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

#ifndef VGA_HAL_IO_H__
#define VGA_HAL_IO_H__

#include "x_typedef.h"
#include "ybr_vga_oal.h"
#include <linux/types.h>
#include "x_hal_ic.h"
/*
// Macros of register read
#define HAL_READ8(_reg_)            (*((volatile u8*)(_reg_)))
#define HAL_READ16(_reg_)           (*((volatile u16*)(_reg_)))
#define HAL_READ32(_reg_)           (*((volatile u32*)(_reg_)))

// Macros of register write
#define HAL_WRITE8(_reg_, _val_)    (*((volatile u8*)(_reg_)) = (_val_))
#define HAL_WRITE16(_reg_, _val_)   (*((volatile u16*)(_reg_)) = (_val_))
#define HAL_WRITE32(_reg_, _val_)   (*((volatile u32*)(_reg_)) = (_val_))

// Macros for read/write access
#define HAL_REG8(_reg_)         HAL_READ8((_reg_))
#define HAL_REG16(_reg_)        HAL_READ16((_reg_))
#define HAL_REG32(_reg_)        HAL_READ32((_reg_))
*/

#define u80(x) (u8)(x)
#define u81(x) (u8)((x) >> 8)
#define u82(x) (u8)((x) >> 16)
#define u83(x) (u8)((x) >> 24)

extern u32 startbit(u32 fld);

#define P_Fld(val,msk) (((val) << (startbit((msk)))) & (msk))

#define WriteRegMsk(addr, val, msk) HAL_WRITE32((addr), (u32)(((u32)(HAL_READ32(addr) & (~(msk)))) | ((u32)((val) << startbit(msk)))))
#define ReadRegMsk(addr,msk) ((u32)((u32)(((HAL_READ32((addr))) &(msk)) )>> (startbit(msk))))

extern void vIO32Write1BMsk(u32 reg32, u8 val8, u8 msk8);
extern void vIO32Write2BMsk(u32 reg32, u16 val16, u16 msk16);
extern void vIO32Write4BMsk(u32 reg32, u32 val32, u32 msk32);


#define IO32ReadFld(reg32,fld)   \
   ReadRegMsk((reg32), (fld))
   
#define IO32ReadFldAlign(reg32,fld)  \
    ReadRegMsk((reg32), (fld))

#define vIO32WriteFld(reg32,val,fld) \
   WriteRegMsk((reg32), (val) <<(startbit((fld))), (fld))

#define vIO32WriteFldAlign(reg32,val,fld) \
        WriteRegMsk((reg32), (val) ,(fld))
        
#define vIO32WriteFldMulti(reg32,list,fld) \
    HAL_WRITE32((reg32), (HAL_READ32((reg32)) &(~(fld))) | (list))
    
#endif  //VGA_HAL_IO_H


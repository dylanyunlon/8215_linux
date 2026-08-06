/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef X_HAL_IO_H
#define X_HAL_IO_H

#include "x_typedef.h"

//===========================================================================

// Type of IO register, for recording the address of an IO register
//typedef volatile UINT32             HAL_IO_REGISTER;

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

//===========================================================================
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

#define IO_MASK(base,offset,mask,value)      IO_WRITE32(base,offset,((IO_READ32(base,offset)&(~(mask)))|(value)))           

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

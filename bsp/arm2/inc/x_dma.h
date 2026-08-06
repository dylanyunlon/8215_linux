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

#ifndef X_DMA_H
#define X_DMA_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"

//============================================================================
// Constant definitions
//============================================================================
#define DMA_DEBUG							0
#define DMA_RETURN_AFTER_SUCCESS			1

#define DMA_ADDRESS_ALIGNMENT_MASK		0x3
#define DMA_LENGTH_ALIGNMENT_MASK		0x3
//============================================================================
// Macros for register read/write
//============================================================================
#define DMA_READ8(offset)			IO_READ8(DDMA_BASE, offset)
#define DMA_READ16(offset)			IO_READ16(DDMA_BASE, offset)
#define DMA_READ32(offset)			IO_READ32(DDMA_BASE, offset)

#define DMA_WRITE8(offset, value)		IO_WRITE8(DDMA_BASE, offset, (value))
#define DMA_WRITE16(offset, value)	IO_WRITE16(DDMA_BASE, offset, (value))
#define DMA_WRITE32(offset, value)	IO_WRITE32(DDMA_BASE, offset, (value))

#define DMA_REG8(offset)				IO_REG8(DDMA_BASE, offset)
#define DMA_REG16(offset)			IO_REG16(DDMA_BASE, offset)
#define DMA_REG32(offset)			IO_REG32(DDMA_BASE, offset)

//============================================================================
// DMA Registers
//============================================================================
#define REG_RW_DDMA_STR			0x0000       //DRAM DMA Start
  #define RW_DDMA_STR_EN			(1U << 0)  //Start DRAM DMA 

#define REG_RW_DDMA_LEN			0x0004       //DRAM DMA Length
  #define RW_DDMA_LEN_MASK		0x3FFFFFFC //DRAM DMA Length Mask
  
#define REG_RW_DDMA_SBADR			0x0008       //Source Begin Address
  #define RW_DDMA_SBADR_MASK		0x3FFFFFFC //Source Begin Address Mask
  
#define REG_RW_DDMA_SEADR			0x000C       //Source End Address
  #define RW_DDMA_SEADR_MASK		0x3FFFFFFC //Source End Address Mask 
  
#define REG_RW_DDMA_TBADR			0x0010       //Target Begin Address
  #define RW_DDMA_TBADR_MASK		0x3FFFFFFC //Target Begin Address Mask
    
#define REG_RW_DDMA_TEADR			0x0014       //Target End Address
  #define RW_DDMA_TEADR_MASK		0x3FFFFFFC //Target End Address Mask 
   
#define REG_RW_DDMA_INTR			0x0018       //DRAM DMA Interrupt
  #define RW_DDMA_INTR_EN			(1U << 0)  //DRAM DMA Interrupt Enable
  
//============================================================================
// Public functions
//============================================================================
EXTERN BOOL fgEnableDMA(UINT32 u4SourceAddress, UINT32 u4TargetAddress, UINT32 u4Length);

#endif  // X_DMA_H


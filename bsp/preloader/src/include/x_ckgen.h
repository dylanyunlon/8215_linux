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

#ifndef X_CKGEN_H
#define X_CKGEN_H

//============================================================================
// Include files
//============================================================================
#include "targetConfig.h"
#include "x_hal_ic.h"
#include "x_typedef.h"



#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360) 
#include "x_ckgen_3360.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560) 
#include "x_ckgen_8560.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580) 
#include "x_ckgen_8580.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356) 
#include "x_ckgen_3356.h"
#endif

//============================================================================
// Macros for register read/write
//============================================================================
#define CKGEN_READ8(offset)            IO_READ8(CKGEN_BASE, (offset))
#define CKGEN_READ16(offset)           IO_READ16(CKGEN_BASE, (offset))
#define CKGEN_READ32(offset)           IO_READ32(CKGEN_BASE, (offset))

#define CKGEN_WRITE8(offset, value)    IO_WRITE8(CKGEN_BASE, (offset), (value))
#define CKGEN_WRITE16(offset, value)   IO_WRITE16(CKGEN_BASE, (offset), (value))
#define CKGEN_WRITE32(offset, value)   IO_WRITE32(CKGEN_BASE, (offset), (value))

#define CKGEN_REG8(offset)             IO_REG8(CKGEN_BASE, (offset))
#define CKGEN_REG16(offset)            IO_REG16(CKGEN_BASE, (offset))
#define CKGEN_REG32(offset)            IO_REG32(CKGEN_BASE, (offset))

#define CKGEN_MASK32(offset,value,mask)   CKGEN_WRITE32(offset,((CKGEN_READ32(offset)&(~(mask)))|value))

#define CKGEN_SETBIT(offset,bit)        CKGEN_WRITE32(offset,((CKGEN_READ32(offset)|bit)))
#define CKGEN_CLRBIT(offset,bit)        CKGEN_WRITE32(offset,(CKGEN_READ32(offset)&(~(bit))))

#define CKGEN_ISBITSET(offset,bit)    ((CKGEN_READ32(offset)&bit)!=0)

#endif  // X_CKGEN_H


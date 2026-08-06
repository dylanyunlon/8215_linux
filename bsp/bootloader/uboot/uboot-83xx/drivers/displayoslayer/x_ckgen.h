/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef X_CKGEN_H
#define X_CKGEN_H

//============================================================================
// Include files
//============================================================================
#include "x_typedef.h"
//#include "x_hal_ic.h"
//#include "drv_config.h"
#include "chip_ver.h"
#include "x_regs.h"

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
#include "x_ckgen_83xx.h"
#endif

//============================================================================
// Macros for register read/write
//============================================================================
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

#endif  // X_CKGEN_H


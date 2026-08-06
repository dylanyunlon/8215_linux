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
#include <asm/arch/x_typedef.h>
#include <asm/arch/ac83xx_basic.h>

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Macros for register read/write
//============================================================================
#define CKGEN_READ32(offset)           (*((volatile UINT32*)(CKGEN_BASE + offset)))
#define CKGEN_WRITE32(offset, value)   (*((volatile UINT32*)(CKGEN_BASE + offset)) = (value))
#define CKGEN_REG32(offset)            (*((volatile UINT32*)(CKGEN_BASE + offset)))

//============================================================================
// CKGEN Registers
//============================================================================
#define REG_RW_TST_CFG0                0x005C

#define REG_RW_PINMUX_OFFSET		   0xC0
#define REG_RW_GPIO_EN_OFFSET          0x180
#define REG_RW_GPIO_OUT_OFFSET         0x1A0
#define REG_RW_GPIO_IN_OFFSET          0x1C0
#define REG_RW_PAD_FECTL_0             0x280

//============================================================================
// Macros for pinmux mask read/write
//============================================================================

//============================================================================
// Boot Loader Version Define
//============================================================================

//============================================================================
// Type definitions
//============================================================================

//============================================================================
// Public functions
//============================================================================

#endif  // X_CKGEN_H


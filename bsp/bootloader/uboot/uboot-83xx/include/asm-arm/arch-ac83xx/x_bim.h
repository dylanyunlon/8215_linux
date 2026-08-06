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

#ifndef X_BIM_H
#define X_BIM_H

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
#define BIM_READ32(offset)           (*((volatile UINT32*)(BIM_BASE + offset)))
#define BIM_WRITE32(offset, value)   (*((volatile UINT32*)(BIM_BASE + offset)) = (value))
#define BIM_REG32(offset)            (*((volatile UINT32*)(BIM_BASE + offset)))

//============================================================================
// Macros for memory read/write
//============================================================================


//============================================================================
// Boot Loader Version Define
//============================================================================


//============================================================================
// BIM Registers
//============================================================================

//----------------------------------------------------------------------------
// General Purpose Register
#define REG_RW_GPRB0        0x00E0        //RISC Byte General Purpose Register 0
#define REG_RW_GPRB1        0x00E4        //RISC Byte General Purpose Register 1
#define REG_RW_GPRB2        0x00E8        //RISC Byte General Purpose Register 2
#define REG_RW_GPRB3        0x00EC        //RISC Byte General Purpose Register 3
#define REG_RW_GPRB4        0x00F0        //RISC Byte General Purpose Register 4
#define REG_RW_GPRB5        0x00F4        //RISC Byte General Purpose Register 5
#define REG_RW_GPRB6        0x00F8        //RISC Byte General Purpose Register 6
#define REG_RW_GPRB7        0x00FC        //RISC Byte General Purpose Register 7
#define REG_RW_GPRDW0       0x0100        //RISC Double Word General Purpose Register 0
#define REG_RW_GPRDW1       0x0104        //RISC Double Word General Purpose Register 1
#define REG_RW_GPRDW2       0x0120        //RISC Double Word General Purpose Register 2
#define REG_RW_GPRDW3       0x0124        //RISC Double Word General Purpose Register 3
#define REG_RW_GPRDW4       0x0110        //RISC Double Word General Purpose Register 4
#define REG_RW_GPRDW5       0x0114        //RISC Double Word General Purpose Register 5
#define REG_RW_GPRDW6       0x0118        //RISC Double Word General Purpose Register 6
#define REG_RW_GPRDW7       0x011C        //RISC Double Word General Purpose Register 7

//----------------------------------------------------------------------------
// MISC Register
#define REG_RW_MISC2		0x00AC
	#define REG_SAFEMODE_OFST	6
	#define REG_SAFEMODE_MASK	(1<<REG_SAFEMODE_OFST)

//----------------------------------------------------------------------------
// 64b_TIMER	
#define REG_RW_T64b_LO_0   0x0728

//============================================================================
// Type definitions
//============================================================================

//============================================================================
// Public functions
//============================================================================

// DUAL CORE
#define REG_RW_HSMPHE       0x01B4        //Hardware Semaphore Register
#define HSMPHE_UART1        (1U << 0)   	//Hardware Semaphore 0
#define HSMPHE_DC_SMPHE     (1U << 1)   	//Hardware Semaphore 1 (for Dual Communication Semaphore)
#define HSMPHE_NAND         (1U << 2)   	// NAND semaphore
#define HSMPHE_LZHS         (1U << 3)   	// LZHS semaphore

extern BOOL BIM_GETHWSemaphore(UINT32 u4Number, UINT32 u4TimeOut);
extern BOOL BIM_ReleaseHWSemaphore(UINT32 u4Number);

#endif  // X_BIM_H


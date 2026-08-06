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


#ifndef _AC83XX_H_
#define _AC83XX_H_

#include "x_bim_83xx.h"
/*************************************************************************/
/*   SYSTEM DEFINE                                                       */
/*************************************************************************/
#if 0
/* Address define */
#define DRAMA_BASE          0xC0000000
#define IO_BASE             0x70000000

#define HEAP_BOTTOM         0x00100000
//#define STACK_HEAD	    0x0FF00000 

#define SRAM_OFFSET0		0x4

#define BIM_BASE            (IO_BASE  + 0x8000)
#define ETHERNET_BASE                   (IO_BASE + 0x33000) 
#define CKGEN_BASE                      (IO_BASE + 0x0000)
#define SERIAL_BASE         	    (IO_BASE + 0xc000)
#define PDWNC_BASE         	    (IO_BASE + 0x24000)	

#define IQR_BASE	    (BIM_BASE + 0xc000)

#define CTL_BASE            (IO_BASE  + 0xD000)

#define REG_RO_ICEMODE      (BIM_BASE + 0x0000)
#define BIT_RISCICE         1

#define REG_RW_REMAP        (BIM_BASE + 0x001C)
#define BIT_REMAP           1

#define REG_RW_DBOA         (BIM_BASE + 0x0020)

#define RW_BIM_7LED         (BIM_BASE + 0x010C)

#define DRAM_PARM           0x70007000

/*************************************************************************/
/*   UART                                                             */
/*************************************************************************/ 
#define UART_BASE               (IO_BASE + 0xC000)
#define REG_UART0_DATA          0x0
#define REG_STATUS              0x4
#define REG_STATUS_WR_ALLOW     0x2
#endif

/*************************************************************************/
/*   TIMER                                                             */
/*************************************************************************/
#define REG_TIMER_LIMIT		(*((volatile UINT32*)(BIM_1_BASE + 0x60)))
#define REG_TIMER_COUNT     (*((volatile UINT32*)(BIM_1_BASE + 0x64)))
#define REG_TIMER_CONTROL	(*((volatile UINT32*)(BIM_1_BASE + 0x78)))

#define REG_TIMER1_LIMIT		(*((volatile UINT32*)(BIM_1_BASE + 0x68)))
#define REG_TIMER1_COUNT     (*((volatile UINT32*)(BIM_1_BASE + 0x6c)))
#define REG_TIMER1_CONTROL	(*((volatile UINT32*)(BIM_1_BASE + 0x78)))

#if 0
/**************************************************************************/
/*   IRQ                                                                  */
/**************************************************************************/
//----------------------------------------------------------------------------
// IRQ/FIQ
#define REG_IRQST           0x0030        //RISC L1 IRQ Status Register
#define REG_IRQEN           0x0034        //RISC L1 IRQ Enable Register
#define REG_IRQCL           0x0038        //RISC L1 IRQ Clear Register
#define REG_FIQST           0x003C        //RISC L1 FIQ Status Register
#define REG_FIQEN           0x0040        //RISC L1 FIQ Enable Register
#define REG_FIQCL           0x0044        //RISC L1 FIQ Clear Register
#define REG_IRQST2          0x0138        //RISC L2 IRQ Status Register
#define REG_IRQEN2          0x013C        //RISC L2 IRQ Enable Register
#define REG_IRQCL2          0x0140        //RISC L2 IRQ Clear Register
#define REG_FIQST2          0x0144        //RISC L2 FIQ Status Register
#define REG_FIQEN2          0x0148        //RISC L2 FIQ Enable Register
#define REG_FIQCL2          0x014C        //RISC L2 FIQ Clear Register
#define REG_IRQST3          0x0154        //RISC L3 IRQ Status Register
#define REG_IRQEN3          0x0158        //RISC L3 IRQ Enable Register
#define REG_IRQCL3          0x015C        //RISC L3 IRQ Clear Register
#define REG_FIQST3          0x0160        //RISC L3 FIQ Status Register
#define REG_FIQEN3          0x0164        //RISC L3 FIQ Enable Register
#define REG_FIQCL3          0x0168        //RISC L3 FIQ Clear Register

#endif

#define REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_1_BASE + REG_IRQST)))
#define REG_IRQ_CLEAR   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL)))
#define REG_IRQ_ENABLE     (*((volatile UINT32*)(BIM_1_BASE + REG_IRQEN)))

#define REG_IRQ_STATUS2  (*((volatile UINT32*)(BIM_1_BASE + REG_IRQST2)))
#define REG_IRQ_CLEAR2   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL2)))
#define REG_IRQ_ENABLE2     (*((volatile UINT32*)(BIM_1_BASE + REG_IRQEN2)))

#define REG_IRQ_STATUS3  (*((volatile UINT32*)(BIM_1_BASE + REG_IRQST3)))
#define REG_IRQ_CLEAR3   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL3)))
#define REG_IRQ_ENABLE3     (*((volatile UINT32*)(BIM_1_BASE + REG_IRQEN3)))

#define REG_IRQ_STATUS4  (*((volatile UINT32*)(BIM_1_BASE + REG_IRQST4)))
#define REG_IRQ_CLEAR4   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL4)))
#define REG_IRQ_ENABLE4     (*((volatile UINT32*)(BIM_1_BASE + REG_IRQEN4)))

#define REG_ARM1_IRQST           0x030        //arm1 RISC L1 IRQ Status Register

#define ARM1_REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_BASE + REG_ARM1_IRQST)))

#define REG_IRQ_ARM2_CLEAR   (*((volatile UINT32*)(BIM_1_BASE + REG_RW_IRQCLR)))



/* vector id */
  //#define VECTOR_T0             3         //Timer 0
  //#define VECTOR_RS232_1        17        //RS232 1
  
#define IRQ_VECTOR_MAX_NUM              128  // 0~ 95

#if 0
/* irq bits */
#define IRQ_SERIAL	0x1<<VECTOR_RS232_1
#define IRQ_TIMER0	0x1<<VECTOR_T0
#endif

void __attribute__((weak)) abort(void) {
	return;
}
#endif

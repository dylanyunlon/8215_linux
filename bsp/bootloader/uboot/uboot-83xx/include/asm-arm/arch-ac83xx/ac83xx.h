/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 *  Copyright(C) 2006 NXP BV, All rights reserved.
 *  by Jean-Paul Saman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _AC83XX_SYS_H_
#define _AC83XX_SYS_H_

/*************************************************************************/
/*   SYSTEM DEFINE                                                       */
/*************************************************************************/
/* Address define */
#define DRAMA_BASE          0xC0000000
#define IO_BASE             0xf0000000

#define HEAP_BOTTOM         0x00100000
//#define STACK_HEAD	    0x0FF00000 

#define SRAM_OFFSET0		0x4

#define BIM_BASE            (IO_BASE  + 0x8000)
#define ETHERNET_BASE                   (IO_BASE + 0x33000) 
#define CKGEN_BASE                      (IO_BASE + 0x0000)
#define SERIAL_BASE         	    (IO_BASE + 0xc000)
#define PDWNC_BASE         	    (IO_BASE + 0x24000)	
#define TIMER_BASE                       (IO_BASE + 0x8148)

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

/*************************************************************************/
/*   TIMER                                                             */
/*************************************************************************/
#define REG_TIMER_LIMIT		(*((volatile UINT32*)(TIMER_BASE + 0x00)))
#define REG_TIMER_CNT    	(*((volatile UINT32*)(TIMER_BASE + 0x04)))
#define REG_TIMER_CONTROL	(*((volatile UINT32*)(TIMER_BASE + 0x1c)))

/**************************************************************************/
/*   IRQ                                                                  */
/**************************************************************************/
//----------------------------------------------------------------------------
// IRQ/FIQ
#if 0
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
#else
// IRQ/FIQ
#define REG_IRQST           0x0030        //RISC L1 IRQ Status Register
#define REG_IRQEN           0x0034        //RISC L1 IRQ Enable Register
#define REG_IRQCL           0x0038        //RISC L1 IRQ Clear Register
#define REG_FIQST           0x003C        //RISC L1 FIQ Status Register
#define REG_FIQEN           0x0040        //RISC L1 FIQ Enable Register
#define REG_FIQCL           0x0044        //RISC L1 FIQ Clear Register

#define REG_IRQST2          0x0048        //RISC L2 IRQ Status Register
#define REG_IRQEN2          0x004C        //RISC L2 IRQ Enable Register
#define REG_IRQCL2          0x0050        //RISC L2 IRQ Clear Register
#define REG_FIQST2          0x0054        //RISC L2 FIQ Status Register
#define REG_FIQEN2          0x0058        //RISC L2 FIQ Enable Register
#define REG_FIQCL2          0x005C        //RISC L2 FIQ Clear Register

#define REG_IRQST3          0x0060        //RISC L3 IRQ Status Register
#define REG_IRQEN3          0x0064        //RISC L3 IRQ Enable Register
#define REG_IRQCL3          0x0068        //RISC L3 IRQ Clear Register
#define REG_FIQST3          0x006C        //RISC L3 FIQ Status Register
#define REG_FIQEN3          0x0070        //RISC L3 FIQ Enable Register
#define REG_FIQCL3          0x0074        //RISC L3 FIQ Clear Register

#define REG_IRQST4          0x0230        //RISC L4 IRQ Status Register
#define REG_IRQEN4          0x0234        //RISC L4 IRQ Enable Register
#define REG_IRQCL4          0x0238        //RISC L4 IRQ Clear Register
#define REG_FIQST4          0x023C        //RISC L4 FIQ Status Register
#define REG_FIQEN4          0x0240        //RISC L4 FIQ Enable Register
#define REG_FIQCL4          0x0244        //RISC L4 FIQ Clear Register

#endif
#define REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_BASE + REG_IRQST)))
#define REG_IRQ_CLEAR   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL)))
#define REG_IRQ_ENABLE     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN)))

#define REG_IRQ_STATUS2  (*((volatile UINT32*)(BIM_BASE + REG_IRQST2)))
#define REG_IRQ_CLEAR2   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL2)))
#define REG_IRQ_ENABLE2     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN2)))

#define REG_IRQ_STATUS3  (*((volatile UINT32*)(BIM_BASE + REG_IRQST3)))
#define REG_IRQ_CLEAR3   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL3)))
#define REG_IRQ_ENABLE3     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN3)))

/* vector id */
  #define VECTOR_T0             (59+32)         //Timer 0
  #define VECTOR_RS232_1        (46+32)        //RS232 1
  
#define IRQ_VECTOR_MAX_NUM              256  // 0~ 159

/* irq bits */
//#define IRQ_SERIAL	0x1<<VECTOR_RS232_1
//#define IRQ_TIMER0	0x1<<VECTOR_T0

#endif

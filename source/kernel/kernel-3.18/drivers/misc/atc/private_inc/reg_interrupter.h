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

 #ifndef _REG_INTERRUPTER_H
 #define _REG_INTERRUPTER_H

#include "base_regs.h"


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

#ifdef CHIP_VER_AC83XX

//for AC83XX
#define REG_IRQST4          0x0230        //RISC L4 IRQ Status Register
#define REG_IRQEN4          0x0234        //RISC L4 IRQ Enable Register
#define REG_IRQCL4          0x0238        //RISC L4 IRQ Clear Register
#define REG_FIQST4          0x023C        //RISC L4 FIQ Status Register
#define REG_FIQEN4          0x0240        //RISC L4 FIQ Enable Register
#define REG_FIQCL4          0x0244        //RISC L4 FIQ Clear Register

#endif


#define REG_RW_INTSTA				  0x140                     //PDWNC INTERRUPT STATUS REGISTER
    #define RW_INTSTA_SFFE_INT				  (1U << 16)	
    #define RW_INTSTA_DDCCI_INT				  (1U << 15)
    #define RW_INTSTA_SIFM_INT				  (1U << 14)
    #define RW_INTSTA_IR_INT					  (1U << 13)
    #define RW_INTSTA_ETNET_INT				  (1U << 12)
    #define RW_INTSTA_CEC_INT				  (1U << 11)
    #define RW_INTSTA_SIFS_INT				  (1U << 10)
    #define RW_INTSTA_DBG_UART_INT			  (1U << 9)
    #define RW_INTSTA_GPIO_INT7				  (1U << 7)
    #define RW_INTSTA_GPIO_INT6				  (1U << 6)
    #define RW_INTSTA_GPIO_INT5				  (1U << 5)
    #define RW_INTSTA_GPIO_INT4				  (1U << 4)
    #define RW_INTSTA_GPIO_INT3				  (1U << 3)
    #define RW_INTSTA_GPIO_INT2				  (1U << 2)
    #define RW_INTSTA_GPIO_INT1				  (1U << 1)
    #define RW_INTSTA_GPIO_INT0				  (1U << 0)    
#define REG_RW_INTEN				  0x144                     //PDWNC INTERRUPT ENABLE REGISTER
    #define RW_INTEN_SFFE_INTEN				  (1U << 16)	
    #define RW_INTEN_DDCCI_INTEN			  (1U << 15)
    #define RW_INTEN_SIFM_INTEN				  (1U << 14)
    #define RW_INTEN_IR_INTEN				  (1U << 13)
    #define RW_INTEN_ETNET_INTEN				  (1U << 12)
    #define RW_INTEN_CEC_INTEN				  (1U << 11)
    #define RW_INTEN_SIFS_INTEN				  (1U << 10)
    #define RW_INTEN_DBG_UART_INTEN			  (1U << 9)
    #define RW_INTEN_GPIO_INTEN7			  (1U << 7)
    #define RW_INTEN_GPIO_INTEN6			  (1U << 6)
    #define RW_INTEN_GPIO_INTEN5			  (1U << 5)
    #define RW_INTEN_GPIO_INTEN4			  (1U << 4)
    #define RW_INTEN_GPIO_INTEN3			  (1U << 3)
    #define RW_INTEN_GPIO_INTEN2			  (1U << 2)
    #define RW_INTEN_GPIO_INTEN1			  (1U << 1)
    #define RW_INTEN_GPIO_INTEN0			  (1U << 0)    
#define REG_RW_INTCLR				  0x148                     //PDWNC INTERRUPT CLEAR REGISTER
    #define RW_INTCLR_SFFE_INTCLR				  (1U << 16)	
    #define RW_INTCLR_DDCCI_INTCLR				  (1U << 15)
    #define RW_INTCLR_SIFM_INTCLR				  (1U << 14)
    #define RW_INTCLR_IR_INTCLR					  (1U << 13)
    #define RW_INTCLR_ETNET_INTCLR				  (1U << 12)
    #define RW_INTCLR_CEC_INTCLR				  (1U << 11)
    #define RW_INTCLR_SIFS_INTCLR				  (1U << 10)
    #define RW_INTCLR_DBG_UART_INTCLR			  (1U << 9)
    #define RW_INTCLR_GPIO_INTCLR7				  (1U << 7)
    #define RW_INTCLR_GPIO_INTCLR6				  (1U << 6)
    #define RW_INTCLR_GPIO_INTCLR5				  (1U << 5)
    #define RW_INTCLR_GPIO_INTCLR4				  (1U << 4)
    #define RW_INTCLR_GPIO_INTCLR3				  (1U << 3)
    #define RW_INTCLR_GPIO_INTCLR2				  (1U << 2)
    #define RW_INTCLR_GPIO_INTCLR1				  (1U << 1)
    #define RW_INTCLR_GPIO_INTCLR0				  (1U << 0)  


typedef volatile struct {
	UINT32 IRQST;
	UINT32 IRQEN;
	UINT32 IRQCL;
	UINT32 FIQST;
	UINT32 FIQEN;
	UINT32 FIQCL;
	UINT32 REVA[0x3c];
	UINT32 IRQST2;
	UINT32 IRQEN2;
	UINT32 IRQCL2;
	UINT32 FIQST2;
	UINT32 FIQEN2;
	UINT32 FIQCL2;
	UINT32 REV;
	UINT32 IRQST3;
	UINT32 IRQEN3;
	UINT32 IRQCL3;
	UINT32 FIQST3;
	UINT32 FIQEN3;
	UINT32 FIQCL3;
#ifdef CHIP_VER_AC83XX  
  UINT32 REVB[0x31];
  UINT32 IRQST4;
	UINT32 IRQEN4;
	UINT32 IRQCL4;
	UINT32 FIQST4;
	UINT32 FIQEN4;
	UINT32 FIQCL4;
#endif  
}AC83XX_INT_REG;

typedef volatile struct {
	UINT32 IRQST;
	UINT32 IRQEN;
	UINT32 IRQCL;
}AC83XX_PDWNC_INT_REG;



#define REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_BASE + REG_IRQST)))
#define REG_IRQ_CLEAR   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL)))
#define REG_IRQ_ENABLE     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN)))

#define REG_IRQ_STATUS2  (*((volatile UINT32*)(BIM_BASE + REG_IRQST2)))
#define REG_IRQ_CLEAR2   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL2)))
#define REG_IRQ_ENABLE2     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN2)))

#define REG_IRQ_STATUS3  (*((volatile UINT32*)(BIM_BASE + REG_IRQST3)))
#define REG_IRQ_CLEAR3   (*((volatile UINT32*)(BIM_BASE + REG_IRQCL3)))
#define REG_IRQ_ENABLE3     (*((volatile UINT32*)(BIM_BASE + REG_IRQEN3)))

#define ARM2_REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_1_UCV_BASE + REG_ARM2_IRQST)))

#define ARM2_REG_IRQ_STATUS3  (*((volatile UINT32*)(BIM_1_UCV_BASE + REG_IRQST3)))


/* vector id */
#ifdef CHIP_VER_AC83XX
  //#define VECTOR_T0             27         //Timer 0
  //#define   VECTOR_RS232_1              14
#endif

#ifdef CHIP_VER_AC83XX

#define IRQ_VECTOR_MAX_NUM              160  // 0~ 159  

#else
  
//#define IRQ_VECTOR_MAX_NUM              96  // 0~ 95
#define IRQ_VECTOR_MAX_NUM              128  // 0~ 127

#endif

/* irq bits */
#define IRQ_SERIAL	0x1<<VECTOR_RS232_1
#define IRQ_TIMER0	0x1<<VECTOR_T0

#endif


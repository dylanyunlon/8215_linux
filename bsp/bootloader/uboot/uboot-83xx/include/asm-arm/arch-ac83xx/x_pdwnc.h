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

#ifndef X_PDWNC_H
#define X_PDWNC_H

//============================================================================
// Include files
//============================================================================
#include <asm/arch/x_typedef.h>
#include <asm/arch/ac83xx_basic.h>

//============================================================================
// Constant definitions
//============================================================================
#define SRAM_MASTER_RISC				0x0	 
#define SRAM_MASTER_T8032				0x1
#define SRAM_MASTER_FE					0x2
#define SRAM_CLR_VALUE				      0x0 

#define ENGINE_MASTER_RISC				0	 
#define ENGINE_MASTER_T8032			1	 
#define ENGINE_ALL						0
#define ENGINE_ETHE						1
#define ENGINE_UART						2
#define ENGINE_CEC						3

//============================================================================
// Macros for register read/write
//============================================================================
#define PDWNC_READ8(offset)                       (*((volatile UINT8*)(PDWNC_BASE + offset)))
#define PDWNC_READ16(offset)                     (*((volatile UINT16*)(PDWNC_BASE + offset)))
#define PDWNC_READ32(offset)                     (*((volatile UINT32*)(PDWNC_BASE + offset)))

#define PDWNC_WRITE8(offset, value)               (*((volatile UINT8*)(PDWNC_BASE + offset)) = (value))
#define PDWNC_WRITE16(offset, value)              (*((volatile UINT16*)(PDWNC_BASE + offset)) = (value))
#define PDWNC_WRITE32(offset, value)		(*((volatile UINT32*)(PDWNC_BASE + offset)) = (value))

#define PDWNC_REG32(offset)					(*((volatile UINT32*)(PDWNC_BASE + offset)))

//============================================================================
// Macros for memory read/write
//============================================================================



//============================================================================
// PDWNC Registers
//============================================================================
#define REG_RW_WKRSC				  0x000                     //WAKE UP RESET COUNTER REGISTER 
    #define RW_WKRSC_WKRSC_MASK	                  0xFFFFFFFF	   
    #define RW_WKRSC_WKRSC_OFFSET	          0		
#define REG_RW_WAKEN				  0x080                     // POWER DOWN WAKE UP ENABLE
    #define RW_WAKEN_UNOR_WAKEN			  (1U << 11)	 
    #define RW_WAKEN_T8032_WAKEN		  (1U << 10)	 
    #define RW_WAKEN_HTPLG_WAKEN		  (1U << 9)	 
    #define RW_WAKEN_IR_WAKEN			  (1U << 8)	
    #define RW_WAKEN_GPIO_WAKEN_MASK	          0x000000FF	   
    #define RW_WAKEN_GPIO_WAKEN_OFFSET	          0		
#define REG_RW_PDCODE				  0x084                     //POWER DOWN ENREY CODE REGISTER
    #define RW_PDCODE_PDCODE_MASK	          0x000000FF	   
    #define RW_PDCODE_PDCODE_OFFSET	          0	
#define REG_RW_PDSTAT				  0x088                     // POWER DOWN WAKE UP STATUS
    #define RW_PDSTAT_UART_WAK			   (1U << 23)	 
    #define RW_PDSTAT_UNOR_WAK			   (1U << 11)	 
    #define RW_PDSTAT_T8032_WAK		   (1U << 10)	 
    #define RW_PDSTAT_HTPLG_WAK		   (1U << 9)	 
    #define RW_PDSTAT_IR_WAK			   (1U << 8)	
    #define RW_PDSTAT_GPIO_WAK_MASK	    0x000000FF	   
    #define RW_PDSTAT_GPIO_WAK_OFFSET	    0		
#define REG_RW_UPWAK				  0x090                     //UP WAKE UP CODE REGISTER
    #define RW_UPWAK_PDCODE_MASK	          0x000000FF	   
    #define RW_UPWAK_PDCODE_OFFSET	          0		
#define REG_RW_PAD_PD				  0x0F0                     //IO PAD PULL DOWN REGISTER
    #define RW_PAD_PD_OPWRSB_PD			  (1U << 4)	
    #define RW_PAD_PD_OPWRSB_PD_OFFSET	  4	
    
#define REG_RW_PAD_PU                0x0EC                     //IO PAD PULL UP REGISTER
   #define RW_PAD_PD_OPWRSB_PU           (1U << 4) 
   #define RW_PAD_PD_OPWRSB_PU_OFFSET    4     
    
#define REG_RW_PAD_PINMUX1				0x0F4
	#define RW_PAD_PINMUX1_HDMI_CEC		  (1U << 26)	
    
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

#define REG_RW_RESRV0				  0x160                    
#define REG_RW_RESRV1				  0x164                    
#define REG_RW_RESRV2				  0x168                    
#define REG_RW_RESRV3				  0x16C                    

#define REG_RW_UP_ADDR				  0x300180     // 0x324180
    #define RW_UP_ADDR_UP_ADDR_MASK		  0x00007FFF	   
    #define RW_UP_ADDR_UP_ADDR_OFFSET		  0			

#define REG_RW_UP_DATA				  0x300184     // 0x324184
    #define RW_UP_DATA_UP_DATA_MASK		  0x000000FF	   
    #define RW_UP_DATA_UP_DATA_OFFSET		  0			

#define REG_RW_UP_CFG	    			  0x188                     //UP CONFIGURATION REGISTER    
    #define RW_UP_CFG_ETNET_EN			  28	
    #define RW_UP_CFG_DBG_UART_EN		  24	 
    #define RW_UP_CFG_FAST_CK_EN		  20	 
    #define RW_UP_CFG_ENGINE_EN			  16	 
    #define RW_UP_CFG_CEC_EN			  12	 
    #define RW_UP_CFG_RAM_CK_SEL_MASK             0x00000300
    #define RW_UP_CFG_RAM_CK_SEL_OFFSET	          8			
    #define RW_UP_CFG_RAM_SPL_SEL_MASK            0x000000c0
    #define RW_UP_CFG_RAM_SPL_SEL_OFFSET	  6			
    #define RW_UP_CFG_RAM_SP_SEL_MASK             0x0000000c
    #define RW_UP_CFG_RAM_SP_SEL_OFFSET	          2					
    #define RW_UP_CFG_T8032_RST                   (1U << 0)	 

#define REG_RW_ARM_IND_ADDR	       	          0x1c0                     //ARM INDIRECT ADDRESS REGISTER
    #define RW_ARM_IND_ADDR_ARM_IND_ADDR_MASK     0x0000FFFF
    #define RW_ARM_IND_ADDR_ARM_IND_ADDR_OFFSET	  0	

#define REG_RW_ARM_IND_DATA0	    		  0x1c4                     //ARM INDIRECT DATA REGISTER 0
    #define RW_ARM_IND_DATA0_ARM_IND_DATA0_MASK   0xFFFFFFFF
    #define RW_ARM_IND_DATA0_ARM_IND_DATA0_OFFSET 0	
     #define RW_ARM_IND_DATA0_BIT31                   (1U << 31)    //demo use, ac on standby
     #define RW_ARM_IND_DATA0_BIT30                   (1U << 30)    //demo use, for standby flag
     #define RW_ARM_IND_DATA0_BIT29                   (1U << 29)    //reserve
     #define RW_ARM_IND_DATA0_BIT28                   (1U << 28)    //demo no use, only  power on key
     #define RW_ARM_IND_DATA0_BIT27                   (1U << 27)    //demo no use, only  power on key
     #define RW_ARM_IND_DATA0_BIT26                   (1U << 26)    //demo no use, only  power on key
     #define RW_ARM_IND_DATA0_BIT25                   (1U << 25)    //demo no use, only  use for Trade mode  
     #define RW_ARM_IND_DATA0_BIT24                   (1U << 24)    //demo no use, only  use for cec switch
     #define RW_ARM_IND_DATA0_BIT23                   (1U << 23)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT22                   (1U << 22)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT21                   (1U << 21)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT20                   (1U << 20)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT19                   (1U << 19)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT18                   (1U << 18)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT17                   (1U << 17)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT16                   (1U << 16)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT15                   (1U << 15)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT14                   (1U << 14)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT13                   (1U << 13)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT12                   (1U << 12)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT11                   (1U << 11)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT10                   (1U << 10)	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT9                    (1U << 9)	    //demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT8                    (1U << 8)   	//demo no use, only  use for CEC_PA
     #define RW_ARM_IND_DATA0_BIT7                    (1U << 7)  	//demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT6                    (1U << 6)	    //demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT5                    (1U << 5)	    //demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT4                    (1U << 4)  	//demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT3                    (1U << 3)  	//demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT2                    (1U << 2)  	//demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT1                    (1U << 1)	    //demo no use, only  use for CEC_LA
     #define RW_ARM_IND_DATA0_BIT0                    (1U << 0)     //demo no use, only  use for CEC_LA

#define REG_RW_ARM_IND_DATA1	    		  0x1c8                     //ARM INDIRECT DATA REGISTER 1
    #define RW_ARM_IND_DATA1_ARM_IND_DATA1_MASK   0xFFFFFFFF
    #define RW_ARM_IND_DATA1_ARM_IND_DATA1_OFFSET 0	

#define REG_RW_ARM_IND_INT	    		  0x1cc                     //ARM INDIRECT CONTROL REGISTER 1
    #define RW_ARM_IND_INT_AUXINT_STA		  (1U << 3)	 
    #define RW_ARM_IND_INT_AUXINT_CLR		  (1U << 2)	 
    #define RW_ARM_IND_INT_ARM_RW		  (1U << 1)	 
    #define RW_ARM_IND_INT_ARM_INT		  (1U << 0)

#define RW_ARM_IND_DATA0_KEY_MASK                     0x1C000000
#define RW_ARM_IND_DATA0_KEY_POWER                    (RW_ARM_IND_DATA0_BIT26)
#define RW_ARM_IND_DATA0_KEY_EJECT                    (RW_ARM_IND_DATA0_BIT27)
#define RW_ARM_IND_DATA0_KEY_PLAY                     (RW_ARM_IND_DATA0_BIT27 | RW_ARM_IND_DATA0_BIT26)
#define RW_ARM_IND_DATA0_KEY_DISC                     (RW_ARM_IND_DATA0_BIT28)
#define RW_ARM_IND_DATA0_KEY_HOME                     (RW_ARM_IND_DATA0_BIT28 | RW_ARM_IND_DATA0_BIT26)

#define REG_RW_AUX_IND_DATA0	    		  0x1d0                     //DATA REGISTER 0 FROM T8032
    #define RW_AUX_IND_DATA0_AUX_DATA0_MASK       0x000000FF
    #define RW_AUX_IND_DATA0_AUX_DATA0_OFFSET	  0	
    #define RW_AUX_IND_DATA0_BIT7                 (1U << 7)	//demo no use, only  use for power on
    #define RW_AUX_IND_DATA0_BIT6                 (1U << 6)	//demo no use, only  use for power down
    #define RW_AUX_IND_DATA0_BIT5                 (1U << 5)	//demo no use, only  use for power down
    #define RW_AUX_IND_DATA0_KEY_HOME             (1U << 4)
    #define RW_AUX_IND_DATA0_KEY_DISC             (1U << 3)
    #define RW_AUX_IND_DATA0_KEY_PLAY             (1U << 2)
    #define RW_AUX_IND_DATA0_KEY_EJECT            (1U << 1)
    #define RW_AUX_IND_DATA0_KEY_POWER            (1U << 0)	

/* watch dog timer */
#define 	REG_RW_WDTSET									0x004
#define 	REG_RW_WDT										0x008

//============================================================================
// Type definitions
//============================================================================

//============================================================================
// Public functions
//============================================================================


#endif  // X_PDWNC_H


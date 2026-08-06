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

#ifndef X_PDWNC_H
#define X_PDWNC_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"

//============================================================================
// Constant definitions
//============================================================================
#define SRAM_MASTER_RISC				0x0	 
#define SRAM_MASTER_T8032				0x1
#define SRAM_MASTER_FE					0x2

#define ENGINE_MASTER_RISC				0	 
#define ENGINE_MASTER_T8032			1	 
#define ENGINE_ALL						0
#define ENGINE_ETHE						1
#define ENGINE_UART						2
#define ENGINE_CEC						3
//============================================================================
// Macros for register read/write
//============================================================================
#define PDWNC_READ8(offset)                       IO_READ8(PDWNC_BASE, offset)
#define PDWNC_READ16(offset)                      IO_READ16(PDWNC_BASE, offset)
#define PDWNC_READ32(offset)                      IO_READ32(PDWNC_BASE, offset)

#define PDWNC_WRITE8(offset, value)               IO_WRITE8(PDWNC_BASE, offset, (value))
#define PDWNC_WRITE16(offset, value)              IO_WRITE16(PDWNC_BASE, offset, (value))
#define PDWNC_WRITE32(offset, value)              IO_WRITE32(PDWNC_BASE, offset, (value))

#define PDWNC_REG8(offset)                        IO_REG8(PDWNC_BASE, offset)
#define PDWNC_REG16(offset)                       IO_REG16(PDWNC_BASE, offset)
#define PDWNC_REG32(offset)                       IO_REG32(PDWNC_BASE, offset)

#define T8032_READ8(offset)                       IO_READ8(0, offset)
#define T8032_READ16(offset)                      IO_READ16(0, offset)
#define T8032_READ32(offset)                      IO_READ32(0, offset)

#define T8032_WRITE8(offset, value)               IO_WRITE8(0, offset, (value))
#define T8032_WRITE16(offset, value)              IO_WRITE16(0, offset, (value))
#define T8032_WRITE32(offset, value)              IO_WRITE32(0, offset, (value))

#define T8032_REG8(offset)                        IO_REG8(0, offset)
#define T8032_REG16(offset)                       IO_REG16(0, offset)
#define T8032_REG32(offset)                       IO_REG32(0, offset)

//============================================================================
// Macros for register read/write
//============================================================================
#define REG_RW_WKRSC				  0x000                     //WAKE UP RESET COUNTER REGISTER 
    #define RW_WKRSC_WKRSC_MASK	                  0xFFFFFFFF	   
    #define RW_WKRSC_WKRSC_OFFSET	          0		

#define REG_RW_WDTSET				  0x004                     //WDT MODE SET REGISTER
    #define RW_WDTSET_WDTMODE_MASK	          0x00000070	   
    #define RW_WDTSET_WDTMODE_OFFSET	          4		
    #define RW_WDTSET_DBG_STOP	  	          (1U << 1)	 
    #define RW_WDTSET_WDT_EN	  	          (1U << 0)	 

#define REG_RW_WDT				  0x008                     //WDT COUNTER REGISTER
    #define RW_WDT_WDT_MASK	                  0xFFFFFFFF	   
    #define RW_WDT_WDT_OFFSET	                  0		

#define REG_RW_WDTLMT             0x00C    
#define REG_RW_WDTSTA             0x010
    #define RW_WDT_STA_OFFSET           8

#define REG_RW_WDT_POWKEY1        0x014
#define REG_RW_WDT_POWKEY2        0x018
#define REG_RW_WDT_KEYMASK1       0x01C
#define REG_RW_WDT_KEYMASK2       0x020

#define REG_RW_CLKPDN             0x040
    #define RW_CLKPDN_DECODE_CLK_STOP   (0x1<<2)
    #define RW_CLKPDN_IRRX_CLK_STOP     (0x1<<1)
    #define RW_CLKPDN_IO_CLK_STOP       (0x1<<0)

#define REG_RW_PDMISC				  0x048                     //PDWNC MISCELLANEOUS REGISTER 
    #define RW_PDMISC_DEBUG_OUT_EN	          (1U << 31)
    #define RW_PDMISC_DEBUG_CORE_SEL_OFFSET	  28		
    #define RW_PDMISC_DEBUG_SEL_OFFSET            24
    #define RW_PDMISC_CORE_CLK_EN                 (1U << 4)
    #define RW_PDMISC_PDWN_POL			  (1U << 0)
#define REG_RW_PDCKSEL                            0x04C
    #define RW_PDCKSEL_3M                         (0U << 5)
    #define RW_PDCKSEL_27M                        (1U << 5)
    #define RW_PDCKSEL_2M                         (2U << 5)
    #define RW_PDCKSEL_1M                         (3U << 5)
    #define RW_PDCKSEL_500K                       (4U << 5)
    #define RW_PDCKSEL_250K                       (5U << 5)
    #define RW_PDCKSEL_6P75M                      (6U << 5)
    #define RW_PDCKSEL_13P5M                      (7U << 5)

#define REG_RW_WAKEN				  0x080                     // POWER DOWN WAKE UP ENABLE
    #define RW_WAKEN_IR_WAKEN			  (1U << 8)	
    #define RW_WAKEN_GPIO_WAKEN_MASK	          0x00000003	   
    #define RW_WAKEN_GPIO_WAKEN_OFFSET	          0
    #define RW_WAKEN_GPIO_IR                     (1U << 0)
    #define RW_WAKEN_GPIO_WAKEUPSRC              (1U << 1)
    #define RW_WAKEN_GPIO_WAKEUPSTS              (1U << 2) 
#define REG_RW_PDCODE				  0x084                     //POWER DOWN ENREY CODE REGISTER
    #define RW_PDCODE_PDCODE_MASK	          0x000000FF	   
    #define RW_PDCODE_PDCODE_OFFSET	          0	
#define REG_RW_PDSTAT				  0x088                     // POWER DOWN WAKE UP STATUS
    #define RW_PDSTAT_IR_WAK			   (1U << 8)	
    #define RW_PDSTAT_GPIO_WAK_MASK	    0x000000FF	   
    #define RW_PDSTAT_GPIO_WAK_OFFSET	    0	
    #define RW_PDSTAT_GPIO_IR                     (1U << 0)
    #define RW_PDSTAT_GPIO_WAKEUPSRC              (1U << 1)
    #define RW_PDSTAT_GPIO_WAKEUPSTS              (1U << 2) 

#define REG_RW_PDSTCLR                            0x08C


#define REG_RW_SRAM_CTL				  0x090                     //UP WAKE UP CODE REGISTER
    #define RW_UPWAK_PDCODE_MASK	          0x000000FF	   
    #define RW_UPWAK_PDCODE_OFFSET	          0		

#define REG_RW_EXINTCFG			  0xc0    //External interrupt configuration register
#define REG_RW_PDIO			  0xc4    //External interrupt configuration register
  #define REG_RW_PDIO_GPIO_PDWN_IR_EN             (1U << 24)
  #define REG_RW_PDIO_GPIO_PDWN__WAKEUPSRC_EN      (1U << 25) 
  #define REG_RW_PDIO_GPIO_PDWN_WAKEUPSTS_EN      (1U << 26)
  #define REG_RW_PDIO_GPIO_PDWN_POL_IR_OFFSET            (16)
  #define REG_RW_PDIO_GPIO_PDWN_POL_WAKEUPSRC_OFFSET            (17)
  #define REG_RW_PDIO_GPIO_PDWN_POL_WAKESTS_OFFSET            (18)
  #define REG_RW_PDIO_GPIO_WAK_IR_EN             (1U << 0)
  #define REG_RW_PDIO_GPIO_WAK__WAKEUPSRC_EN      (1U << 1) 
  #define REG_RW_PDIO_GPIO_WAK_WAKEUPSTS_EN      (1U << 2)


#define REG_RW_PDIOCNT                    0xC8

#define REG_RW_PAD_PD				  0x0F0                     //IO PAD PULL DOWN REGISTER
    #define RW_PAD_PD_OPWRSB_PD			  (1U << 4)	
    #define RW_PAD_PD_OPWRSB_PD_OFFSET	  4	
    
#define REG_RW_PAD_PU                0x0EC                     //IO PAD PULL UP REGISTER
   #define RW_PAD_PD_OPWRSB_PU           (1U << 4) 
   #define RW_PAD_PD_OPWRSB_PU_OFFSET    4     
    
#define REG_RW_PAD_PINMUX1				0x0F4
	#define RW_PAD_PINMUX1_HDMI_CEC		  (1U << 26)	
	

#define REG_RW_GPIOIN                 0x0D0
#define REG_RW_GPIOEN                 0x0D4
#define REG_RW_GPIOOUT                0x0D8
    
//#define  REG_RW_GPIOEN                                          0x0F8
  #define RW_GPIOEN_ETMDIO_EN  (1U << 5)
      
#if ((CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)) // mtk40184 add
#define REG_RW_PAD_PINMUX2            0x0F8
#define REG_RW_PAD_PINMUX3            0x0FC
#endif

#if ((CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)) // mtk40184 add
#define REG_RW_PAD_PINMUX4            0x108
#endif

//============================================================================
// The register REG_RW_PWDC_XTAL_CFG(0x0120) included in x_ckgen_mt8561.h
// Do not do operation in pdwnc.
//============================================================================
/*********************************************************************
#define REG_RW_PWDC_XTAL_CFG                  0x0120 

	#define REG_PWM_GPIO_PD_OFFSET				(29)
	#define REG_PWM_GPIO_PD_MASK				(0x3<<29)
	#define REG_PWM_GPIO_PU_OFFSET				(27)
	#define REG_PWM_GPIO_PU_MASK				(0x3<<27)
	#define REG_PWM_GPIO_G_OFFSET				(14)
	#define REG_PWM_GPIO_G_MASK 				(0x3<<14)
	#define REG_PWM_GPIO_EN_OFFSET				(12)
	#define REG_PWM_GPIO_EN_MASK				(0x3<<12)
	#define REG_PWM_PWMDAC_LDO_EN_OFFSET		(8)
	#define REG_PWM_PWMDAC_LDO_EN_MASK			(0x3<<8)
	#define REG_PWM_PWMDAC_STBY_REV_OFFSET		(0)
	#define REG_PWM_PWMDAC_STBY_REV_MASK		(0xFF<<0)

	#define REG_PWM_SYSPLL1_PWD 				(0x1<<26)
	#define REG_PWM_SYSPLL1_DDS_PWDB			(0x1<<25)
	#define REG_PWM_HDMIPLL_PWD 				(0x1<<24)
	#define REG_PWM_HDMIPLL_DDS_PWDB			(0x1<<23)
	#define REG_PWM_SYSPLL2_PWD 				(0x1<<22)
	#define REG_PWM_ARMPLL_PWD					(0x1<<21)
	#define REG_PWM_APLL_PWD					(0x1<<20)
	#define REG_PWM_APLL_DDS_PWDB				(0x1<<19)
	#define REG_PWM_PLLGP_BIAS_PWD				(0x1<<18)
	#define REG_PWM_HA_DDS_PWDB 				(0x1<<17)
	#define REG_PWM_BRCKTX_PD					(0x1<<16)
	#define REG_PWM_MUTE_CTRL_PWD				(0x1<<11)
	#define REG_PWM_PUD_PWD 					(0x1<<10)
****************************************************************/


#define REG_RW_INTSTA				  0x140                     //PDWNC INTERRUPT STATUS REGISTER
	#define RW_INTSTA_CEC2_INT				  (1U << 19)
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
	#define RW_INTEN_CEC2_INTEN				  (1U << 19)
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
	#define RW_INTCLR_CEC2_INTCLR				  (1U << 19)
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


#define REG_RW_SYSSTA                             0x174
     #define REG_RW_SYSSTA_WAKEUP_OFFSET                 ( 1 )
     #define REG_RW_SYSSTA_ACDC_OFFSET                   (0 )

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550) // mtk40184 add
#define REG_RW_UP_ADDR				  0x300180     // 0x324180
#else
#define REG_RW_UP_ADDR				  0x180                     //UP MEN ADDRESS REGISTER 
#endif
    #define RW_UP_ADDR_UP_ADDR_MASK		  0x0000FFFF	   
    #define RW_UP_ADDR_UP_ADDR_OFFSET		  0			

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550) // mtk40184 add
#define REG_RW_UP_DATA				  0x300184     // 0x324184
#else
#define REG_RW_UP_DATA				  0x184                     //UP MEN DATA REGISTER    
#endif                    //UP MEN DATA REGISTER    
    #define RW_UP_DATA_UP_DATA_MASK		  0x000000FF	   
    #define RW_UP_DATA_UP_DATA_OFFSET		  0			

#define REG_RW_UP_CFG	    			  0x188                     //UP CONFIGURATION REGISTER    
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580) ||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561) ||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)
	#define RW_UP_CFG_ETNET_EN			  26
#else
	#define RW_UP_CFG_ETNET_EN			  28
#endif		
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
    #define RW_UP_CFG_RAM_NR_SEL_OFFSET	          (1U << 1)
    #define RW_UP_CFG_RAM_PP_SEL_OFFSET	          (1U << 29)
    #define RW_UP_CFG_T8032_RST                   (1U << 0)	 

#define REG_RW_UP_DBG	    			  0x18c                     //T8032 DEBUG REGISTER
    #define RW_UP_DBG_UP_DBG_MASK                 0x0000FFFF
    #define RW_UP_DBG_UP_DBG_OFFSET               0			

#define REG_RW_ARM_IND_ADDR	       	          0x1c0                     //ARM INDIRECT ADDRESS REGISTER
    #define RW_ARM_IND_ADDR_ARM_IND_ADDR_MASK     0x0000FFFF
    #define RW_ARM_IND_ADDR_ARM_IND_ADDR_OFFSET   0
    #define RW_ARM_IND_DATA0_VFD_DIMMER_STANDBY             (1U << 2) //for T8032 vfd dimmer Standby
    #define RW_ARM_IND_DATA0_DEMO_MODE_STANDBY              (1U << 1) //for T8032 DEMO or Child Mode Standby
    #define RW_ARM_IND_DATA0_FORCE_POWER_ON                 (1U << 0) //for T8032 Force Power on

#define REG_RW_ARM_IND_DATA0	    		  0x1c4                        //bit0-bit15 for philips  mtk70199_for_aap_data           //ARM INDIRECT DATA REGISTER 0
    #define RW_ARM_IND_DATA0_ARM_IND_DATA0_MASK   0xFFFFFFFF
    #define RW_ARM_IND_DATA0_ARM_IND_DATA0_OFFSET 0	
     #define RW_ARM_IND_DATA0_BIT31                   (1U << 31)    //for mcu upg status, 1 fail, 0 success
     #define RW_ARM_IND_DATA0_BIT30                   (1U << 30)    //for suspend mode
     #define RW_ARM_IND_DATA0_BIT29                   (1U << 29)    //for One UX
     #define RW_ARM_IND_DATA0_BIT28                   (1U << 28)    //for power on key
     #define RW_ARM_IND_DATA0_BIT27                   (1U << 27)    //for power on key
     #define RW_ARM_IND_DATA0_BIT26                   (1U << 26)	//for power on key
     #define RW_ARM_IND_DATA0_BIT25                   (1U << 25)	//for Trade mode	
     #define RW_ARM_IND_DATA0_BIT24                   (1U << 24)    //for cec switch
     #define RW_ARM_IND_DATA0_BIT23                   (1U << 23)	//dfor CEC_PA
     #define RW_ARM_IND_DATA0_BIT22                   (1U << 22)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT21                   (1U << 21)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT20                   (1U << 20)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT19                   (1U << 19)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT18                   (1U << 18)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT17                   (1U << 17)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT16                   (1U << 16)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT15                   (1U << 15)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT14                   (1U << 14)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT13                   (1U << 13)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT12                   (1U << 12)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT11                   (1U << 11)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT10                   (1U << 10)	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT9                    (1U << 9)	    //for CEC_PA
     #define RW_ARM_IND_DATA0_BIT8                    (1U << 8)   	//for CEC_PA
     #define RW_ARM_IND_DATA0_BIT7                    (1U << 7)  	//for CEC_LA
     #define RW_ARM_IND_DATA0_BIT6                    (1U << 6)	    //for CEC_LA
     #define RW_ARM_IND_DATA0_BIT5                    (1U << 5)	    //for CEC_LA
     #define RW_ARM_IND_DATA0_BIT4                    (1U << 4)  	//for CEC_LA
     #define RW_ARM_IND_DATA0_BIT3                    (1U << 3)  	//dfor CEC_LA
     #define RW_ARM_IND_DATA0_BIT2                    (1U << 2)  	//for CEC_LA
     #define RW_ARM_IND_DATA0_BIT1                    (1U << 1)	    //for CEC_LA
     #define RW_ARM_IND_DATA0_BIT0                    (1U << 0)     //for CEC_LA

#define RW_ARM_IND_ONE_UX_MASK                        0x3C000000//MTK70199 for One UX 
#define RW_ARM_IND_DATA0_KEY_MASK                     0x1C000000
#define RW_ARM_IND_DATA0_KEY_POWER                    (RW_ARM_IND_DATA0_BIT26)
#define RW_ARM_IND_DATA0_KEY_EJECT                    (RW_ARM_IND_DATA0_BIT27)
#define RW_ARM_IND_DATA0_KEY_PLAY                     (RW_ARM_IND_DATA0_BIT27 | RW_ARM_IND_DATA0_BIT26)
#define RW_ARM_IND_DATA0_KEY_DISC                     (RW_ARM_IND_DATA0_BIT28)
#define RW_ARM_IND_DATA0_KEY_HOME                     (RW_ARM_IND_DATA0_BIT28 | RW_ARM_IND_DATA0_BIT26)
    
#define RW_ARM_IND_DATA0_MCU_UG_STATUS_MASK         0x80000000
#define RW_ARM_IND_DATA0_MCU_UG_STATUS_FAIL         RW_ARM_IND_DATA0_BIT31
    

#define REG_RW_ARM_IND_DATA1	    		  0x1c8                     //ARM INDIRECT DATA REGISTER 1
    #define RW_ARM_IND_DATA1_ARM_IND_DATA1_MASK   0xFFFFFFFF
    #define RW_ARM_IND_DATA1_ARM_IND_DATA1_OFFSET 0	
    #define RW_ARM_IND_DATA1_BIT31                   (1U << 31)    //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT30                   (1U << 30)    //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT29                   (1U << 29)    //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT28                   (1U << 28)    //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT27                   (1U << 27)    //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT26                   (1U << 26)	   //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT25                   (1U << 25)	   //for STR mode,please don't use it 
    #define RW_ARM_IND_DATA1_BIT24                   (1U << 24)    //for STR mode,please don't use it 
    
#define REG_RW_ARM_IND_INT	    		  0x1cc                     //ARM INDIRECT CONTROL REGISTER 1
    #define RW_ARM_IND_INT_AUXINT_STA		  (1U << 3)	 
    #define RW_ARM_IND_INT_AUXINT_CLR		  (1U << 2)	 
    #define RW_ARM_IND_INT_ARM_RW		  (1U << 1)	 
    #define RW_ARM_IND_INT_ARM_INT		  (1U << 0)

#define REG_RW_AUX_IND_DATA0	    		  0x1d0                     //DATA REGISTER 0 FROM T8032
    #define RW_AUX_IND_DATA0_AUX_DATA0_MASK       0x000000FF
    #define RW_AUX_IND_DATA0_AUX_DATA0_OFFSET	  0	

#if CONFIG_DRV_CUSTOM_JSN
#define RW_AUX_IND_DATA0_FORCE_EJECT          (1U << 7)    //Sony Force Eject mode
#define RW_AUX_IND_DATA0_KEY_DIAG             (1U << 6) //Sony Diag mode
#define RW_AUX_IND_DATA0_KEY_SERVICE          (1U << 5) //Sony Service mode
#else
#define RW_AUX_IND_DATA0_BIT7                 (1U << 7)	//for OneUX
#define RW_AUX_IND_DATA0_BIT6                 (1U << 6)	//reserve
#define RW_AUX_IND_DATA0_BIT5                 (1U << 5)	//reserve
#endif

    #define RW_AUX_IND_DATA0_KEY_HOME             (1U << 4)
    #define RW_AUX_IND_DATA0_KEY_DISC             (1U << 3)
    #define RW_AUX_IND_DATA0_KEY_PLAY             (1U << 2)
    #define RW_AUX_IND_DATA0_KEY_EJECT            (1U << 1)
    #define RW_AUX_IND_DATA0_KEY_POWER            (1U << 0)	


#define REG_RW_AUX_IND_DATA1	    		  0x1d1                     //DATA REGISTER 1 FROM T8032
    #define RW_AUX_IND_DATA1_AUX_DATA1_MASK       0x000000FF
    #define RW_AUX_IND_DATA1_AUX_DATA1_OFFSET	  0	
    #define RW_AUX_IND_DATA1_BIT0                     (1U << 0)	//demo no use, only philips use for power on	

#define REG_RW_AUX_IND_DATA2	    		  0x1d2                     //DATA REGISTER 2 FROM T8032
    #define RW_AUX_IND_DATA2_AUX_DATA2_MASK       0x000000FF
    #define RW_AUX_IND_DATA2_AUX_DATA2_OFFSET	  0	

#define REG_RW_AUX_IND_DATA3	    		  0x1d3                     //DATA REGISTER 3 FROM T8032
    #define RW_AUX_IND_DATA3_AUX_DATA3_MASK       0x000000FF
    #define RW_AUX_IND_DATA3_AUX_DATA3_OFFSET	  0	

//@0x241d4 for STR mode,please don't use it 	
#define REG_RW_AUX_IND_DATA4	    		  0x1d4                     //DATA REGISTER 0 FROM T8032
    #define RW_AUX_IND_DATA4_AUX_DATA0_MASK       0x000000FF           
    #define RW_AUX_IND_DATA4_AUX_DATA0_OFFSET	  0	

#define REG_RW_AUX_IND_DATA5	    		  0x1d5                     //DATA REGISTER 1 FROM T8032
    #define RW_AUX_IND_DATA5_AUX_DATA1_MASK       0x000000FF
    #define RW_AUX_IND_DATA5_AUX_DATA1_OFFSET	  0	

#define REG_RW_AUX_IND_DATA6	    		  0x1d6                     //DATA REGISTER 2 FROM T8032
    #define RW_AUX_IND_DATA6_AUX_DATA2_MASK       0x000000FF
    #define RW_AUX_IND_DATA6_AUX_DATA2_OFFSET	  0	

#define REG_RW_AUX_IND_DATA07	    		  0x1d7                     //DATA REGISTER 3 FROM T8032
    #define RW_AUX_IND_DATA7_AUX_DATA3_MASK       0x000000FF
    #define RW_AUX_IND_DATA7_AUX_DATA3_OFFSET	  0	

#define REG_RW_AUX_IND_INT	    		  0x1d8                     //CONTROL REGISTER FROM T8032
#define RW_AUX_IND_INT_AUX_INT			  (1U << 0)

#define DDCCI_EDID0_ENABLE              0x600
#define DDCCI_EDID_CHECKSUM              0x604
#define DDCCI_EDID0_CHECKSUM              0x604
#define DDCCI_EDID1_CHECKSUM              0x60C

#define DDCCI_EDID1_ENABLE              0x608
#define DDCCI_EDID_DOWNLOAD_MODE  0x61c
#define DDCCI_EDID_DATA                       0x650
#define DDCCI_EDID0_DATA_REMAP                       0x660
#define DDCCI_EDID1_DATA_REMAP                       0x668



#define REG_RW_STAB	      			  0x804                     //UART_PD STATUS REGISTER

#define REG_RW_RS232_MODE	    		  0x818                     //RS232 MODE CONFIGURATION REGISTERS
    #define RW_RS232_MODE_PDGPIO_AS_UART	  (1U << 2)	
    #define RW_RS232_MODE_RS232_MODE_MASK         0x00000003
    #define RW_RS232_MODERS232_MODE_OFFSET	  0


#define REG_RW_IRCKSEL	 0x044
#define REG_RW_IRCFGL	 0x210
#define REG_IREXP_EN	 0x240
#define REG_PDWN_CNT	 0x250


#define REG_IREXP_MASK_LOW	 0x244 
#define REG_IREXP_MASK_HIGH	 0x248


	
#define REG_RW_EIOINTCFG	 0x360

	
#define REG_EXP_IRM0	0x280
#define REG_EXP_IRM1	0x288
#define REG_EXP_IRM2	0x290
#define REG_EXP_IRM3	0x298
#define REG_EXP_IRM4	0x2a0
#define REG_EXP_IRM5	0x2a8
#define REG_EXP_IRM6	0x2b0
#define REG_EXP_IRM7	0x2b8
#define REG_EXP_IRM8	0x2c0
#define REG_EXP_IRM9	0x2c8

	#define PIN_MAX 		3
	#define PIN_INT_MAX 	16
	
#define REG_RW_EGPIOEN	0x320
#define REG_RW_EGPIOIN	0x324
#define REG_RW_EGPIOOUT	0x328
#define REG_RW_EGPIOSEL	0x32c


#define REG_RW_PDEIO	0x364




//----------------------------------------------------------------------------
// 8530Remove
// PWM
#define REG_RW_PWM0         0x02A0        //PWM0 Register  
#define PWM0_PWME           (1U << 0)   //Enable  
#define PWM0_PWMP_MASK      0x0000FF00  //Prescaler Mask  
#define PWM0_PWMP_OFFSET    8           //Prescaler Offset  
#define PWM0_PWMH_MASK      0x00FF0000  //High Period Mask  
#define PWM0_PWMH_OFFSET    16          //High Period Offset  
#define PWM0_PWMRSN_MASK    0xFF000000  //Resolution Byte Mask  
#define PWM0_PWMRSN_OFFSET  24          //Resolution Byte Offset  
#define REG_RW_PWM1         0x02F0        //PWM1 Register  
#define PWM1_PWME           (1U << 0)   //Enable  
#define PWM1_ALD            (1U << 1)   //PWM Auto Load PWMH From Demux  
#define PWM1_PWMP_MASK      0x0000FF00  //Prescaler Mask  
#define PWM1_PWMP_OFFSET    8           //Prescaler Offset  
#define PWM1_PWMH_MASK      0x00FF0000  //High Period Mask  
#define PWM1_PWMH_OFFSET    16          //High Period Offset  
#define PWM1_PWMRSN_MASK    0xFF000000  //Resolution Byte Mask  
#define PWM1_PWMRSN_OFFSET  24          //Resolution Byte Offset  
#define REG_RW_PWMSWTCH    0x02F4         //PWM Switch Register  
#define PWMSWTCH_PWME      (1U << 0)    //Switch
#endif  // X_PDWNC_H


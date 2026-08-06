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

#include "targetConfig.h"


#include "def3363.h"




.macro REG_MASK, reg, value, mask
	ldr r3,[\reg,#0]
	bic r3,r3,\mask
	orr r3,r3,\value
	str r3,[\reg,#0]
.endm
	
/** need enable timer**/ 

.macro DELAY_N37ns, value
  LDR      r0,=\value	
  BL  Timer_Delay
.endm
 

	.extern cmain
	.extern CPU1_Main
	.extern SLAVE_CORE
  .global reset
  .global JumpCmd

/*
 * CKGEN
 */
/*PLL*/
#define	REG_RW_SYSPLL1_CFG0	  (CKGEN_BASE + 0x580)
#define	BIT_SYSPLL1_CFG0_648MHZ	    0x170C0320
#define   BIT_SYSPLL1_CFG0_MASK  0x7FFFFFFE


#define	REG_RW_ARMPLL_CFG	    (CKGEN_BASE + 0x594)
#define   BIT_ARMPLL_CFG_1296MHZ  0x5F4C2022
#define   BIT_ARMPLL_CFG_1188MHZ  0x574C2022
#define   BIT_ARMPLL_CFG_1377MHZ  0x654C2022
#define   BIT_ARMPLL_CFG_1215MHZ  0x594C2022
#define   BIT_ARMPLL_CFG_1026MHZ  0x4B4C2022
#define   BIT_ARMPLL_CFG_1107MHZ  0x514C2022
#define   BIT_ARMPLL_CFG_1012MHZ  0x4A4C2022
#define   BIT_ARMPLL_CFG_999MHZ  0x494C2022
#define	BIT_ARMPLL_CFG_945MHZ	   0x454C2022
#define   BIT_ARMPLL_CFG_904MHZ  0x424C2022
#define	BIT_ARMPLL_CFG_850MHZ	   0x3E4C2022
#define   BIT_ARMPLL_CFG_702MHZ    0x334C2022
#define   BIT_ARMPLL_CFG_MASK      0x7FFFFFFF

#define	REG_RW_ARM9PLL_CFG	  (CKGEN_BASE + 0x598)
#define   BIT_ARM9PLL_CFG_459MHZ   0x215D2020
#define BIT_ARM9PLL_CFG_405MHZ	   0x1D5D2020
#define   BIT_ARM9PLL_CFG_351MHZ	 0x195D2020
#define	  BIT_ARM9PLL_CFG_MASK	   0x7FFFFFFF

/*CLOCK*/
#define	REG_MCUSYS_AXI        (IO_BASE + 0x55060)
#define	BIT_RG_MCUSYS_SET_AXI       0x00000012         
#define	BIT_RG_MCUSYS_SET_AXI_MASK	0x0000001F
#define	REG_MCUSYS_APB        (IO_BASE + 0x55064)
#define	BIT_RG_MCUSYS_SET_APB       0x00000012         
#define	BIT_RG_MCUSYS_SET_APB_MASK	0x0000001F

#define	REG_ARMPLL_DIV        (IO_BASE + 0x8448)
#define	BIT_ARMPLL_DIV_CPUCK_SEL	    0x00010000
#define	BIT_ARMPLL_DIV_CPUCK_SEL_MASK	0x00030000

#define	REG_RW_AP_REG9		    (CKGEN_BASE + 0x30)
#define	BIT_CLK_SRAMIF_SYSPLL_D2	      0x00001000
#define	BIT_CLK_SRAMIF_SYSPLL_D2_MASK	  0x00007000

#define	REG_RW_AP_REG3		    (CKGEN_BASE + 0x18)
#define	BIT_RISC9_SEL_ARM9PLL	      0x10
#define	BIT_RISC9_SEL_ARM9PLL_MASK	0xFFFFFFCF
#define	REG_RW_AP_ARM9_EN		  (CKGEN_BASE + 0xA0)
#define	BIT_RISC9_SEL_ARM9PLL_EN	    0x00008000
#define	BIT_RISC9_SEL_ARM9PLL_EN_MASK	0x00008000

#define	REG_RW_AP_REG1		    (CKGEN_BASE + 0x10)
#define	BIT_BCLK_SEL_SYSPLLD3	 0x1
#define	  BIT_BCLK_SEL_SYSPLLD3_MASK	0xFFFFFFF8	 
#define   BIT_BCLK_SEL_SYSPLLD4    0x2
#define	  BIT_BCLK_SEL_SYSPLLD4_MASK	0xFFFFFFF8
#define BIT_CLK_AXIM_SYSPLL_D2        0x01000000
#define	  BIT_CLK_AXIM_SYSPLL_D2_MASK	0x07000000
#define BIT_FLASH_AP_SEL_SYSPLL_D9        0x00000100  //[8:6]=100
#define	  BIT_FLASH_AP_SEL_SYSPLL_D9_MASK	0x000001C0

#define	REG_RW_DVD_REG1		    (CKGEN_BASE + 0x08)
#define	BIT_DRAM_SLOW_SEL_CLK_APLL1	 0x2            /*   APLL1 need config*/
#define	  BIT_DRAM_SLOW_SEL_CLK_APLL1_MASK	0xFFFFFFF8
#define   BIT_DRAM_SLOW_SEL_CLK_SYSPLLD3    0x3
#define	  BIT_DRAM_SLOW_SEL_CLK_SYSPLLD3_MASK	0xFFFFFFF8

/*
 * PDWNC
 */
#define	PDWNC_BASE			  (IO_BASE + 0x24000) 
#define PDWNC_PLLGP_CTL		(PDWNC_BASE + 0x180) /* Configuration pwd register */
#define   BIT_RG_SYSPLL1_PWD           (0x00000001)
#define   BIT_RG_SYSPLL1_PWD_MASK      (0x00000001)
#define   BIT_RG_SYSPLL1_DDS_PWDB      (0x00000002)
#define   BIT_RG_SYSPLL1_DDS_PWDB_MASK (0x00000002)
#define   BIT_RG_ARMPLL_PWD            (0x00000004)
#define   BIT_RG_ARMPLL_PWD_MASK 			 (0x00000004)
#define   BIT_RG_ARM9PLL_PWD           (0x00000008)
#define   BIT_RG_ARM9PLL_PWD_MASK 		 (0x00000008)
#define   BIT_RG_APLL294_PWD           (0x00000010)
#define   BIT_RG_APLL294_PWD_MASK 		 (0x00000010)
#define   BIT_RG_APLL294_DDS_PWDB      (0x00000020)
#define   BIT_RG_APLL294_DDS_PWDB_MASK (0x00000020)
#define   BIT_RG_APLL270_PWD           (0x00000040)
#define   BIT_RG_APLL270_PWD_MASK      (0x00000040)
#define   BIT_RG_APLL270_DDS_PWDB      (0x00000080)
#define   BIT_RG_APLL270_DDS_PWDB_MASK (0x00000080)
#define   BIT_RG_BRCKTX_PD             (0x00000100)
#define   BIT_RG_BRCKTX_PD_MASK        (0x00000100)
#define   BIT_RG_PLLGP_BIAS_PWD        (0x00000200)
#define   BIT_RG_PLLGP_BIAS_PWD_MASK   (0x00000200)
#define   BIT_RG_DDDS1_PWDB            (0x00000400)
#define   BIT_RG_DDDS1_PWDB_MASK       (0x00000400)
#define   BIT_RG_DDDS2_PWDB            (0x00000800)
#define   BIT_RG_DDDS2_PWDB_MASK       (0x00000800)
#define   BIT_RG_APLL26_PWD            (0x00001000)
#define   BIT_RG_APLL26_PWD_MASK       (0x00001000)
#define   BIT_RG_APLL26_BIAS_PWD       (0x00002000)
#define   BIT_RG_APLL26_BIAS_PWD_MASK  (0x00002000)
#define   BIT_RG_HADDS2_PWD            (0x00004000)
#define   BIT_RG_HADDS2_PWD_MASK       (0x00004000)
#define   BIT_RG_HADDS2_DDS_PWDB       (0x00008000)
#define   BIT_RG_HADDS2_DDS_PWDB_MASK  (0x00008000)
#define   BIT_RG_MSDCPLL_PWD           (0x00010000)
#define   BIT_RG_MSDCPLL_PWD_MASK      (0x00010000)
#define   BIT_RG_VGAPLL_PWD            (0x00020000)
#define   BIT_RG_VGAPLL_PWD_MASK       (0x00020000)
#define   BIT_RG_XDDS_PWDB             (0x00040000)
#define   BIT_RG_XDDS_PWDB_MASK        (0x00040000)
#define   BIT_RG_VGATL_BIAS_PWD        (0x00080000)
#define   BIT_RG_VGATL_BIAS_PWD_MASK   (0x00080000)

#define	REG_PLLGP_CFG16		    (CKGEN_BASE + 0x5C0)
#define	BIT_RG_APLL_REFCK_EN	        0x00002000          
#define	BIT_RG_APLL_REFCK_EN_MASK	    0x00002000

#define	REG_PLLGP_CFG1		    (CKGEN_BASE + 0x584)
#define	BIT_RG_SYSPLL1_DDS_RSTB       0x00000001          
#define	BIT_RG_SYSPLL1_DDS_RSTB_MASK	0x00000001

#define	REG_PLLGP_CFG25		    (CKGEN_BASE + 0x5E4)
#define	BIT_RG_HADDS2_DDS_RSTB            0x00400000          
#define	BIT_RG_HADDS2_DDS_RSTB_MASK	      0x00400000
#define	BIT_RG_HADDS2_CLK_PH_INV          0x00080000          
#define	BIT_RG_HADDS2_CLK_PH_INV_MASK	    0x00080000
#define	BIT_RG_HADDS2_FIFO_START_MAN      0x00800000          
#define	BIT_RG_HADDS2_FIFO_START_MAN_MASK	0x00800000
#define	BIT_RG_HADDS2_NCPO_EN             0x00200000          
#define	BIT_RG_HADDS2_NCPO_EN_MASK	      0x00200000

#define	REG_PLLGP_CFG2		    (CKGEN_BASE + 0x588)
#define	BIT_RG_SYSPLL1_PCW_NCPO_CHG         0x00040000          
#define	BIT_RG_SYSPLL1_PCW_NCPO_CHG_MASK	  0x00040000
#define	BIT_RG_SYSPLL1_FIFO_START_MAN       0x00010000          
#define	BIT_RG_SYSPLL1_FIFO_START_MAN_MASK	0x00010000

#define	REG_PLLGP_CFG4		    (CKGEN_BASE + 0x590)
#define	BIT_RG_SYSPLL1_DDS_CLK_PH_INV       0x00000800          
#define	BIT_RG_SYSPLL1_DDS_CLK_PH_INV_MASK	0x00000800
#define	BIT_RG_SYSPLL1_DDS_NCPO_EN          0x00000200          
#define	BIT_RG_SYSPLL1_DDS_NCPO_EN_MASK	    0x00000200

//set apll
#define	REG_PLLGP_CFG8		    (CKGEN_BASE + 0x5A0)
#define	BIT_RG_APLL294_DDS_RSTB             0x00000040          
#define	BIT_RG_APLL294_DDS_RSTB_MASK	      0x00000040
#define	BIT_RG_APLL294_PCW_NCPO_CHG         0x00000020          
#define	BIT_RG_APLL294_PCW_NCPO_CHG_MASK	  0x00000020
#define	BIT_RG_APLL294_DDSEN	              0x00004000
#define	BIT_RG_APLL294_DDSEN_MASK           0x00004000
#define	BIT_RG_APLL294_BAND	                0x04000000
#define	BIT_RG_APLL294_BAND_MASK            0xFC000000
#define	BIT_RG_APLL294_AUTOK_VCO            0x00080000          
#define	BIT_RG_APLL294_AUTOK_VCO_MASK	      0x00080000

#define	REG_PLLGP_CFG10		    (CKGEN_BASE + 0x5A8)
#define	BIT_RG_APLL294_DDS_CLK_PH_INV       0x00004000          
#define	BIT_RG_APLL294_DDS_CLK_PH_INV_MASK	0x00004000
#define	BIT_RG_APLL294_FIFO_START_MAN       0x00040000          
#define	BIT_RG_APLL294_FIFO_START_MAN_MASK	0x00040000
#define	BIT_RG_APLL294_DDS_NCPO_EN          0x00001000          
#define	BIT_RG_APLL294_DDS_NCPO_EN_MASK	    0x00001000


#define	REG_PLLGP_CFG12		    (CKGEN_BASE + 0x5B0)
#define	BIT_RG_APLL270_BAND                 0x00000400          
#define	BIT_RG_APLL270_BAND_MASK	          0x0000FC00
#define	BIT_RG_APLL270_AUTOK_VCO            0x00000008          
#define	BIT_RG_APLL270_AUTOK_VCO_MASK	      0x00000008

#define	REG_PLLGP_CFG13		    (CKGEN_BASE + 0x5B4)
#define	BIT_RG_APLL270_DDS_RSTB             0x00400000          
#define	BIT_RG_APLL270_DDS_RSTB_MASK	      0x00400000
#define	BIT_RG_APLL270_PCW_NCPO_CHG         0x00200000          
#define	BIT_RG_APLL270_PCW_NCPO_CHG_MASK	  0x00200000
#define	BIT_RG_APLL270_DDSEN	              0x40000000
#define	BIT_RG_APLL270_DDSEN_MASK           0x40000000

#define	REG_PLLGP_CFG15		    (CKGEN_BASE + 0x5BC)
#define	BIT_RG_APLL270_DDS_CLK_PH_INV       0x00004000          
#define	BIT_RG_APLL270_DDS_CLK_PH_INV_MASK	0x00004000
#define	BIT_RG_APLL270_FIFO_START_MAN       0x00040000          
#define	BIT_RG_APLL270_FIFO_START_MAN_MASK	0x00040000
#define	BIT_RG_APLL270_DDS_NCPO_EN          0x00001000          
#define	BIT_RG_APLL270_DDS_NCPO_EN_MASK	    0x00001000

#define REG_EFUSE_FEATURE     (IO_BASE + 0x54668)
#define BIT_RG_HP_MASK             (0x1 << 13)

reset:
	// core 0 work only
	MOV    R10,#1                         // 1:Hold cpu1, 0: release cpu1
	MRC    p15, 0, r0, c0, c0, 5        //read Multiprocessor Affinity Register MPIDR   	     
	TST     r0, #(1<<30)		     //Check for uniprocessor variant
	BNE     arm_init             
	TST     r0, #3                           //Check for CPU 0
	BNE     SLAVE_CORE 
	//BNE    wloop           
	B        arm_init                


wloop:
	TST  r10,#1
       BNE   wloop
       B      set_cpu1_stack 

SLAVE_CORE:
  WFI 
   B SLAVE_CORE
 
arm_init:
  mov     r0, #INIT_CPSR
  msr     cpsr,r0

pdwnc_init:
	LDR r0,=(IO_BASE + 0x24040)
	mov r1,#0x0
        str r1,[R0]


enable_timer0:
  LDR	r0,=0xFFFFFFFF  
  LDR	r1,=(BIM_BASE+0x148)  
  STR	r0,[r1,#0]  
  MOV	r0,#1  
  LDR	r1,=(BIM_BASE+0x164)  
  STR	r0,[r1,#0]

/*check_boot_type:
    ldr r0,=(PDWNC_BASE+0x160)
    ldr r1,[r0,#0]
    ldr r2,=0x6D617273
    cmp r1,r2
    beq set_arm_pll*/

#ifdef config_TARGET_REALCHIP
Enable_Demux_SRAM:
  MOV	r0,#0x80
  LDR      r1,=(IO_BASE+0xBC)
  LDR	r2,[r1,#0]
  ORR      r2,r2,r0
  STR       r2,[r1,#0]
  #ifndef config_TARGET_SIM         
  DELAY_N37ns 81                       //wait 3us = 3*27=  81
  #endif    
  LDR	r1,=(IO_BASE+0x120C0)
  LDR	r2,[r1,#0]
  MOV      r3,#3
  MOV      r0,r3,LSL #26
  BIC       r2,r2,r0
  MOV      r3,#1
  MOV      r0,r3,LSL #26
  ORR      r2,r2,r0
  STR	r2,[r1,#0]
#endif

//  DELAY_N37ns 27     //delay 1us
  
/*
 *setup pll
 */
#if(CLK_SETTING_ENABLE)

set_arm_pll:
        /* For MT3363 CKGen */
        /* Setup ARMPLL */


        /* Step.1 Setup ARMPLL,ARM9PLL*/
        /*
        BIT_ARMPLL_CFG_850MHZ
        BIT_ARM9PLL_CFG_405MHZ
        */
        
	      ldr  r0,=REG_RW_ARMPLL_CFG
				ldr  r1,=BIT_ARMPLL_CFG_850MHZ

				/* check efuse HP bit, enable high frequece */
				ldr  r4,=REG_EFUSE_FEATURE
				ldr  r5, [r4]    
				tst  r5, #(1<<13)        
        bne  normal_clock
	      ldr  r1,=BIT_ARMPLL_CFG_1026MHZ

normal_clock:
	      ldr  r2,=BIT_ARMPLL_CFG_MASK
	      REG_MASK r0,r1,r2
        
   	    ldr  r0,=REG_RW_ARM9PLL_CFG
	      ldr  r1,=BIT_ARM9PLL_CFG_405MHZ
	      ldr  r2,=BIT_ARM9PLL_CFG_MASK
	      REG_MASK r0,r1,r2
       
        

set_bus_pll:

        /* Step.1 Setup SYSPLL1  648MHz */
        ldr  r0,=REG_RW_SYSPLL1_CFG0
	      ldr  r1,=BIT_SYSPLL1_CFG0_648MHZ
	      ldr  r2,=BIT_SYSPLL1_CFG0_MASK
	      REG_MASK r0,r1,r2
	      
/*
 *setup power on sequence
 */
        //RG_PLLGP_BIAS_PWD | RG_VGATL_BIAS_PWD | RG_APLL26_BIAS_PWD : 0 power on
        ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_PLLGP_BIAS_PWD | BIT_RG_VGATL_BIAS_PWD | BIT_RG_APLL26_BIAS_PWD)
        bic     r2, r2,r1
        str     r2, [r3, #0]

       
        //Delay 40/27=1.5us
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 40         
	      #endif

        //RG_ARMPLL_PWD | RG_ARM9PLL_PWD | RG_HADDS2_PWD | RG_APLL26_PWD | RG_SYSPLL1_PWD : 0 power on
        ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_ARMPLL_PWD | BIT_RG_ARM9PLL_PWD | BIT_RG_HADDS2_PWD | BIT_RG_APLL26_PWD | BIT_RG_SYSPLL1_PWD)
        bic     r2, r2,r1
        str     r2, [r3, #0]
        
        //Delay 700/27=26us
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 700         
	      #endif    

        // BIT_RG_APLL_REFCK_EN : setup apll_reference_CK
        
        ldr  r0,=REG_PLLGP_CFG16
	      ldr  r1,=BIT_RG_APLL_REFCK_EN
	      ldr  r2,=BIT_RG_APLL_REFCK_EN_MASK
	      REG_MASK r0,r1,r2
	      
        //RG_SYSPLL1_DDS_PWDB | RG_DDDS1_PWDB | RG_DDDS2_PWDB | RG_XDDS_PWDB : 1 power on
        ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_SYSPLL1_DDS_PWDB | BIT_RG_DDDS1_PWDB | BIT_RG_DDDS2_PWDB | BIT_RG_XDDS_PWDB)
        orr     r2, r2,r1
        str     r2, [r3, #0]

        //RG_SYSPLL1_DDS_RSTB | RG_HADDS2_DDS_RSTB  write 0 reset
        ldr     r3, =REG_PLLGP_CFG1
        ldr     r2, [r3, #0]
        ldr     r1, =BIT_RG_SYSPLL1_DDS_RSTB
        bic     r2, r2,r1
        str     r2, [r3, #0]
        
        ldr     r3, =REG_PLLGP_CFG25
        ldr     r2, [r3, #0]
        ldr     r1, =BIT_RG_HADDS2_DDS_RSTB
        bic     r2, r2,r1
        str     r2, [r3, #0]

       //delay >40 ns, 2/27=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif    
        
       //RG_SYSPLL1_PCW_NCPO_CHG write 1 sync enable; NO RG_HADDS2_PCW_NCPO_CHG REGISTER
        ldr  r0,=REG_PLLGP_CFG2
	      ldr  r1,=BIT_RG_SYSPLL1_PCW_NCPO_CHG
	      ldr  r2,=BIT_RG_SYSPLL1_PCW_NCPO_CHG_MASK
	      REG_MASK r0,r1,r2
       

       //BIT_RG_SYSPLL1_DDS_CLK_PH_INV | BIT_RG_HADDS2_CLK_PH_INV write 1 clock edge inverter
        ldr  r0,=REG_PLLGP_CFG4
	      ldr  r1,=BIT_RG_SYSPLL1_DDS_CLK_PH_INV
	      ldr  r2,=BIT_RG_SYSPLL1_DDS_CLK_PH_INV_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG25
	      ldr  r1,=BIT_RG_HADDS2_CLK_PH_INV
	      ldr  r2,=BIT_RG_HADDS2_CLK_PH_INV_MASK
	      REG_MASK r0,r1,r2

       //delay >40 ns, 2/27=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif    

       //BIT_RG_SYSPLL1_FIFO_START_MAN | BIT_RG_HADDS2_FIFO_START_MAN write 1 DDS FIFO manual start
        ldr  r0,=REG_PLLGP_CFG2
	      ldr  r1,=BIT_RG_SYSPLL1_FIFO_START_MAN
	      ldr  r2,=BIT_RG_SYSPLL1_FIFO_START_MAN_MASK
	      REG_MASK r0,r1,r2

        ldr  r0,=REG_PLLGP_CFG25
	      ldr  r1,=BIT_RG_HADDS2_FIFO_START_MAN
	      ldr  r2,=BIT_RG_HADDS2_FIFO_START_MAN_MASK
	      REG_MASK r0,r1,r2
	      
	     //BIT_RG_SYSPLL1_DDS_NCPO_EN | BIT_RG_HADDS2_NCPO_EN write 1 DDS NCPO enable
	      ldr  r0,=REG_PLLGP_CFG2
	      ldr  r1,=BIT_RG_SYSPLL1_DDS_NCPO_EN
	      ldr  r2,=BIT_RG_SYSPLL1_DDS_NCPO_EN_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG25
	      ldr  r1,=BIT_RG_HADDS2_NCPO_EN
	      ldr  r2,=BIT_RG_HADDS2_NCPO_EN_MASK
	      REG_MASK r0,r1,r2
	     	            
       //delay >100 ns form apll_reference_CK,74+74+74=222ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif    
       
       //open APLL294,APLL270 BAND
        ldr  r0,=REG_PLLGP_CFG8
	      ldr  r1,=BIT_RG_APLL294_BAND
	      ldr  r2,=BIT_RG_APLL294_BAND_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG12
	      ldr  r1,=BIT_RG_APLL270_BAND
	      ldr  r2,=BIT_RG_APLL270_BAND_MASK
	      REG_MASK r0,r1,r2
       
       //delay >0us 37*2=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif   
	      
       //RG_APLL294_PWD | RG_APLL270_PWD   write 0 power on 
        ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_APLL294_PWD | BIT_RG_APLL270_PWD)
        bic     r2, r2,r1
        str     r2, [r3, #0]
     
       //delay >100 ns form BIT_RG_SYSPLL1_DDS_NCPO_EN,74+74+74=222ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 4         
	      #endif   
     
       //RG_MSDCPLL_PWD | RG_VGAPLL_PWD write 0 power on
        ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_MSDCPLL_PWD | BIT_RG_VGAPLL_PWD)
        bic     r2, r2,r1
        str     r2, [r3, #0]
/*
 * set APLL power on sequence
*/     
        //Delay >500us 37*25000=740us
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 25000         
	      #endif 

        //set BIT_RG_APLL294_AUTOK_VCO | BIT_RG_APLL270_AUTOK_VCO  write 0 disable 
        ldr     r3, =REG_PLLGP_CFG8
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_APLL294_AUTOK_VCO)
        bic     r2, r2,r1
        str     r2, [r3, #0]
	      
	ldr     r3, =REG_PLLGP_CFG12
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_APLL270_AUTOK_VCO)
        bic     r2, r2,r1
        str     r2, [r3, #0]
        
        //Delay >0us 37*2=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2        
	      #endif 

       //set BIT_RG_APLL270_DDS_RSTB | BIT_RG_APLL294_DDS_RSTB
        ldr  r0,=REG_PLLGP_CFG13
	      ldr  r1,=BIT_RG_APLL270_DDS_RSTB
	      ldr  r2,=BIT_RG_APLL270_DDS_RSTB_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG8
	      ldr  r1,=BIT_RG_APLL294_DDS_RSTB
	      ldr  r2,=BIT_RG_APLL294_DDS_RSTB_MASK
	      REG_MASK r0,r1,r2
	      
	     //set BIT_RG_APLL270_DDS_PWDB | BIT_RG_APLL294_DDS_PWDB write 1 power on
	      ldr     r3, =PDWNC_PLLGP_CTL
        ldr     r2, [r3, #0]
        ldr     r1, =(BIT_RG_APLL270_DDS_PWDB | BIT_RG_APLL294_DDS_PWDB)
        orr     r2, r2,r1
        str     r2, [r3, #0]
        
       //delay >40 ns, 2/27=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif 
	      
       //set BIT_RG_APLL270_PCW_NCPO_CHG | BIT_RG_APLL294_PCW_NCPO_CHG
	      ldr  r0,=REG_PLLGP_CFG13
	      ldr  r1,=BIT_RG_APLL270_PCW_NCPO_CHG
	      ldr  r2,=BIT_RG_APLL270_PCW_NCPO_CHG_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG8
	      ldr  r1,=BIT_RG_APLL294_PCW_NCPO_CHG
	      ldr  r2,=BIT_RG_APLL294_PCW_NCPO_CHG_MASK
	      REG_MASK r0,r1,r2
    
       //set BIT_RG_APLL270_DDS_CLK_PH_INV | BIT_RG_APLL294_DDS_CLK_PH_INV
        ldr  r0,=REG_PLLGP_CFG15
	      ldr  r1,=BIT_RG_APLL270_DDS_CLK_PH_INV
	      ldr  r2,=BIT_RG_APLL270_DDS_CLK_PH_INV_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG10
	      ldr  r1,=BIT_RG_APLL294_DDS_CLK_PH_INV
	      ldr  r2,=BIT_RG_APLL294_DDS_CLK_PH_INV_MASK
	      REG_MASK r0,r1,r2

       //delay >40 ns, 2/27=74ns
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 2         
	      #endif 
	      
	     //set BIT_RG_APLL270_FIFO_START_MAN | BIT_RG_APLL294_FIFO_START_MAN

        ldr  r0,=REG_PLLGP_CFG15
	      ldr  r1,=BIT_RG_APLL270_FIFO_START_MAN
	      ldr  r2,=BIT_RG_APLL270_FIFO_START_MAN_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG10
	      ldr  r1,=BIT_RG_APLL294_FIFO_START_MAN
	      ldr  r2,=BIT_RG_APLL294_FIFO_START_MAN_MASK
	      REG_MASK r0,r1,r2
	      
	      //set BIT_RG_APLL270_DDS_NCPO_EN |BIT_RG_APLL294_DDS_NCPO_EN
	      ldr  r0,=REG_PLLGP_CFG15
	      ldr  r1,=BIT_RG_APLL270_DDS_NCPO_EN
	      ldr  r2,=BIT_RG_APLL270_DDS_NCPO_EN_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG10
	      ldr  r1,=BIT_RG_APLL294_DDS_NCPO_EN
	      ldr  r2,=BIT_RG_APLL294_DDS_NCPO_EN_MASK
	      REG_MASK r0,r1,r2
	      
	      //delay >1u, 6/27=1.4us
        #ifndef config_TARGET_SIM         
         DELAY_N37ns 4         
	      #endif 
	      
	      //set BIT_RG_APLL270_DDSEN | BIT_RG_APLL294_DDSEN
	      ldr  r0,=REG_PLLGP_CFG13
	      ldr  r1,=BIT_RG_APLL270_DDSEN
	      ldr  r2,=BIT_RG_APLL270_DDSEN_MASK
	      REG_MASK r0,r1,r2
	      
	      ldr  r0,=REG_PLLGP_CFG8
	      ldr  r1,=BIT_RG_APLL294_DDSEN
	      ldr  r2,=BIT_RG_APLL294_DDSEN_MASK
	      REG_MASK r0,r1,r2
	      
/*
 *setup clk
 */

set_arm_clk:

      	/* Step.3 Before switching CPU clock to ARMPLL,
      	 *programmer must configure mcu_biu_ck to 1/2 CPU clock 
      	*/
      	ldr  r0,=REG_MCUSYS_AXI
	      ldr  r1,=BIT_RG_MCUSYS_SET_AXI
	      ldr  r2,=BIT_RG_MCUSYS_SET_AXI_MASK
	      REG_MASK r0,r1,r2
      	
      	ldr  r0,=REG_MCUSYS_APB
	      ldr  r1,=BIT_RG_MCUSYS_SET_APB
	      ldr  r2,=BIT_RG_MCUSYS_SET_APB_MASK
	      REG_MASK r0,r1,r2
      	
        /* Step.3 Change CA7 clock source to ARMPLL */
        ldr  r0,=REG_ARMPLL_DIV
	      ldr  r1,=BIT_ARMPLL_DIV_CPUCK_SEL
	      ldr  r2,=BIT_ARMPLL_DIV_CPUCK_SEL_MASK
	      REG_MASK r0,r1,r2
	      
	      /* Step.3 setup sramif clock syspll/2 = 324MHz*/
        ldr  r0,=REG_RW_AP_REG9
	      ldr  r1,=BIT_CLK_SRAMIF_SYSPLL_D2
	      ldr  r2,=BIT_CLK_SRAMIF_SYSPLL_D2_MASK
	      REG_MASK r0,r1,r2
	      
	      /* Step.3 setup AXI clock syspll/2 = 324MHz*/
        ldr  r0,=REG_RW_AP_REG1
	      ldr  r1,=BIT_CLK_AXIM_SYSPLL_D2
	      ldr  r2,=BIT_CLK_AXIM_SYSPLL_D2_MASK
	      REG_MASK r0,r1,r2

        /* Step.3 Change RISC9 clock source to ARM9PLL */
        LDR     r1, =REG_RW_AP_REG3
        LDR     r0, [r1, #0]
        LDR     r2, =BIT_RISC9_SEL_ARM9PLL_MASK
        AND     r0, r0, r2
        LDR     r2, =BIT_RISC9_SEL_ARM9PLL
        ORR     r0, r0, r2
        STR     r0, [r1, #0]
        
        /* Step.3 RISC9 clock source enable */
        ldr  r0,=REG_RW_AP_ARM9_EN
	      ldr  r1,=BIT_RISC9_SEL_ARM9PLL_EN
	      ldr  r2,=BIT_RISC9_SEL_ARM9PLL_EN_MASK
	      REG_MASK r0,r1,r2
        

        
set_bus_clk:
        /* Step.3 bus clk ref to syspll/3 = 216MHz*/
	      LDR     r1, =REG_RW_AP_REG1
	      LDR     r0, [r1, #0]
        LDR     r2, =BIT_BCLK_SEL_SYSPLLD3_MASK
        AND     r0, r0, r2
        LDR     r2, =BIT_BCLK_SEL_SYSPLLD3
        ORR     r0, r0, r2
        STR     r0, [r1, #0]
        
#ifdef config_TARGET_SIM   
        /*simulation serial flash 72Mhz, realchip 27/8=3.375Mhz */
set_ap_serial_flash_clk:
        ldr  r0,=REG_RW_AP_REG1
	      ldr  r1,=BIT_FLASH_AP_SEL_SYSPLL_D9
	      ldr  r2,=BIT_FLASH_AP_SEL_SYSPLL_D9_MASK
	      REG_MASK r0,r1,r2
#endif  
        
set_dram_slow_clk:

         /* Step.3 dvd dram slow clock SYSPLL_D3=216Mhz-----APLL1 = 270MHz*/
        LDR     r1, =REG_RW_DVD_REG1
        LDR     r0, [r1, #0]
        LDR     r2, =BIT_DRAM_SLOW_SEL_CLK_SYSPLLD3_MASK
        AND     r0, r0, r2
        LDR     r2, =BIT_DRAM_SLOW_SEL_CLK_SYSPLLD3
        ORR     r0, r0, r2
        STR     r0, [r1, #0]
        

#endif









DisableMMU:    
	//Disable caches and MMU    
 	MRC     p15, 0, r0, c1, c0, 0       //Read CP15 control register    
	BIC     r0, r0, #CR_ICACHE    
	ORR     r0, r0, #CR_ICACHE    
	BIC     r0, r0, #CR_DCACHE    
	BIC     r0, r0, #CR_MMU    
	ORR     r0, r0, #CR_ALIGNMENT       //Enable alignment fault checking    
	MCR     p15, 0, r0, c1, c0, 0       //Write back control register    

	//For MT3363 CORTEX-A7    
	MRC     p15, 0, r0, c1, c0, 1       //Read CP15 auxiliary control register    
	ORR     r0, r0, #0x40               //Enables coherent requests to the processor    
 	MCR     p15, 0, r0, c1, c0, 1       //Write back control register
       

       /* run @0xF4000000 base*/
	ldr r0,=Remap_System
	mov  pc,r0
	nop
	nop
	nop
Remap_System:
        ldr  r0, =0xF0024174
	mov r1,#0
	str r1,[r0]
	LDR     r0, =REG_RW_REMAP    
	LDR     r1, [r0, #0]    
	ORR     r1, r1 ,#BIT_REMAP    
	STR     r1, [r0, #0]

set_cpu0_stack:      
  /*  SVC Mode */        
	ldr     r0, = (MODE_SVC|BIT_I|BIT_F)        
	msr     cpsr,r0
	ldr     r1, =  CPU0_STACK_TOP - 8        
	mov     sp, r1
	ldr     r2, =SVC_STACK_SIZE	
	
relocate:
        /* relocate to tcm */
        mov  r5,#0
        mov	r0, pc
        ldr	r1, =_start
        ldr	r2, =.
        ldr	r3, =_bss_start
        sub	r0, r2, r0
        cmp r5,r0
        beq zero_bss
        sub	r2, r3, r0
        sub	r0, r1, r0

copy_loop:
        ldmia   r0!, {r3-r10}           /* copy from source address [r0]    */
        stmia   r1!, {r3-r10}           /* copy to   target address [r1]    */
        cmp     r0, r2                  /* until source end addreee [r2]    */
        ble     copy_loop

rom_boot:
	/* formal boot up flow */

zero_bss:
        /* zero bss */
        ldr	r0, =_bss_start
        ldr	r1, =_end
        ldr	r3, =0x0
        ldr	r4, =0x0
        ldr	r5, =0x0
        ldr	r6, =0x0
        ldr	r7, =0x0
        ldr	r8, =0x0
        ldr	r9, =0x0
        ldr	r10, =0x0

zero_loop:
        cmp	r0, r1
        stmcc   r0!, {r3-r10}           /* zeroing target address [r0]    */
        bcc	zero_loop



       
GotoCMain:
	MOV   R10,#1
        ldr     pc, =cmain

/*
GotoUBoot:
        ldr	pc, =UBOOT_ADDR
*/

/*
 * private functions
 */
DELAY_LOOP:
        SUBS    R1, R1, #1
        BNE     DELAY_LOOP
        BX      lr

;/*************************************************************************/
;/*   JumpCmd                                                           */
;/*   for BootROM , this code must be put into Boot ROM                                        */
;/*************************************************************************/

set_cpu1_stack:      
  /*  IRQ Mode */        
	ldr     r0, = (MODE_IRQ|BIT_I|BIT_F)        
	msr     cpsr,r0
	ldr     r1, =  CPU1_STACK_TOP - 8        
	mov     sp, r1
	ldr     r2, =IRQ_STACK_SIZE  	
	/* FIQ Mode*/        
	ldr     r0, = (MODE_FIQ|BIT_I|BIT_F)        
	msr     cpsr,r0       
	mov     sp, r1
	sub     r1, r1,r2
	ldr     r2, =FIQ_STACK_SIZE              
	/* UND Mode*/        
	ldr     r0, = (MODE_UND|BIT_I|BIT_F)        
	msr     cpsr,r0       
	mov     sp, r1
	sub     r1, r1,r2
	ldr     r2, =UND_STACK_SIZE     
	/* ABT Mode*/        
	ldr     r0, = (MODE_ABT|BIT_I|BIT_F)        
	msr     cpsr,r0       
	mov     sp, r1
	sub     r1, r1,r2
	ldr     r2, =ABT_STACK_SIZE   
	/* SYS Mode*/        
	ldr     r0, = (MODE_SYS|BIT_I|BIT_F)        
	msr     cpsr,r0       
	mov     sp, r1
	sub     r1, r1,r2
	ldr     r2, =SYS_STACK_SIZE   
	/* SVC Mode*/        
	ldr     r0, = (MODE_SVC|BIT_I|BIT_F)        
	msr     cpsr,r0       
	mov     sp, r1
	sub     r1, r1,r2
	ldr     r2, =SVC_STACK_SIZE  


GotoCpu1Main:
        ldr     pc, =CPU1_Main

JumpCmd:
      mov  pc, r0


Timer_Delay:
  	LDR	r1,=(BIM_BASE+0x14C)
  	LDR	r2,[r1,#0]
  	SUB      r3,r2,r0
DLoop: 	
	LDR      r2,[r1,#0]
  	CMP      r3,r2
  	BCC     DLoop	
     	MOV     pc, lr
        .end


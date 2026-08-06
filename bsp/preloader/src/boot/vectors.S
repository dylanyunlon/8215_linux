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

#if(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356)
#include "def3356.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360)
#include "def3360.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#include "def3363.h"
#endif

	 .extern irq_vector
	 .global SLAVE_CORE

        .text
        .section .resident.text, "ax"

        b       _reset
undefined_instruction:
        b       undefined_instruction
software_interrupt:
#if(WAKEUP_TEST_ENABLE)
	ldr  pc,=swi_handler
#else
        b       software_interrupt
#endif
prefetch_abort:
        b       prefetch_abort
data_abort:   
        b       data_abort
not_used:         
        b       not_used
irq: 
#if(IRQ_ENABLE)
        ldr   pc,=irq_handler
#else
        b      irq
#endif
fiq:
        b       fiq
_suspend:
        b      suspend_entry

_reset:

        MOV    R10,#1                         // 1:Hold cpu1, 0: release cpu1
	MRC    p15, 0, r0, c0, c0, 5        //read Multiprocessor Affinity Register MPIDR   	     
	TST     r0, #(1<<30)		     //Check for uniprocessor variant
	BNE     reset             
	TST     r0, #3                           //Check for CPU 0
	BNE     SLAVE_CORE
	b  reset
SLAVE_CORE:
	LDR   r5,=REG_GICC_PMR            //mask   
	MOV   r6,#0xF0
	STR   r6,[r5]
	LDR   r5,=REG_GICC_CTRL           //enable
	MOV   r6,#0x01
	STR   r6,[r5]
1:	
	 WFI
	 LDR   r5,=REG_CORE1_MAGIC
	 LDR   r6,[r5]
         LDR   r7,=CORE1_MAGIC_NUM
         CMP   r6,r7
	 BNE   1b
	 LDR   r5,=REG_RW_SLAVE_START
	 LDR   r6,[r5]
	 MOV   pc,r6




#if(IRQ_ENABLE)

irq_handler:
       sub lr,lr,#4
       stmfd  sp!,{r0-r5,r12,lr}
	mrs  r0,spsr
       stmfd sp!,{r0}

       ldr     r1, =handle_arch_irq
	mov   lr,pc
	ldr      pc,[r12]
	
	ldmfd  sp!,{r0}
	msr    spsr,r0
       ldmfd  sp!,{r0-r5,r12,pc}^


#endif
	.globl	handle_arch_irq
handle_arch_irq:	
	.space	4
       
        .end


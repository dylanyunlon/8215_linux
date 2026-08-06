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
#include "arm_cpu.h" 
#define DRAM_BACKUP_REG_SIZE   0x1000
	.extern core_suspend
	.extern _loader_start
	.global suspend_entry
	.global cpu_SaveSuspendReturn
	.global cpu_Resume
	.global cpu_ResumeMMUOn
	.global ac8317_power_off
	.global ac8317_power_off_end
	.global power_off
    .global GotoTZ
    
suspend_entry:
            b cpu_SaveSuspendReturn
cpu_SaveSuspendReturn:
   
    @ OS Resume start PC.
    adr r2,quickboot_param_addr
    str r0,[r2]
    add r0,r0,#0x8
    ldr r1,[r0]
    ldr  r0,=_loader_start  //load loader(nfi or emmc) start address
    add r0,r0,#(DRAM_BACKUP_REG_SIZE)
    str  r1, [r0], #4

    @ Save return registers in cpu state buffer
    @ This stackfram will be destroied by following DRAMC
    @ suspend functions. So put it in cpu state buffer.
    stmia   r0!, {r4-r11}

    @ Save current CPU stats
    mrc     p15, 0, r4, c1, c0, 0   @ control register
    mrc     p15, 0, r5, c1, c0, 2   @ co-processor access control
    mrc     p15, 0, r6, c3, c0, 0   @ Domain ID
    mrc     p15, 0, r7, c2, c0, 0   @ Translation table base 0
    mrc     p15, 0, r8, c2, c0, 1   @ Translation table base 1
    mrc     p15, 0, r9, c1, c0, 1   @ auxiliary control register
    mrc     p15, 0, r10, c13, c0, 0 @ FCSE/PID
    mrc     p15, 0, r11, c13, c0, 1 @ Context ID
    stmia   r0!, {r4-r11}

    mrc     p15, 0, r5, c10, c2, 0  @ PRRR
    mrc     p15, 0, r6, c10, c2, 1  @ NRRR
    mrc     p15, 0, r7, c13, c0, 3  @ User r/o thread ID
    stmia   r0!, {r5-r7}

    @ Save current mode....
    mrs     r4, CPSR
    str     r4, [r0], #4

    @ USER registers (using SYS mode)
    cps     #MODE_SYS
    stmia   r0!, {sp, lr}

    @ SVC registers
    cps     #MODE_SVC
    mrs     r5, SPSR
    stmia   r0!, {r5, sp, lr}

    @ ABT registers
    cps     #MODE_ABT
    mrs     r5, SPSR
    stmia   r0!, {r5, sp, lr}

    @ UNDEF registers
    cps     #MODE_UDF
    mrs     r5, SPSR
    stmia   r0!, {r5, sp, lr}

    @ IRQ registers
    cps     #MODE_IRQ
    mrs     r5, SPSR
    stmia   r0!, {r5, sp, lr}

    @ FIQ registers
    cps     #MODE_FIQ
    mrs     r5, SPSR
    stmia   r0!, {r5, r8-r14}

    @ Switch back to original mode.
    msr     CPSR_cxsf, r4



/*arm_init:*/
  mov     r0, #INIT_CPSR
  msr     cpsr,r0



/*enable_timer0:*/
  LDR	r0,=0xFFFFFFFF  
  LDR	r1,=(BIM_BASE+0x148)  
  STR	r0,[r1,#0]  
  MOV	r0,#1  
  LDR	r1,=(BIM_BASE+0x164)  
  STR	r0,[r1,#0]


/*set_cpu0_stack:*/      
  /*  IRQ Mode */       
	ldr     r0, = (MODE_IRQ|BIT_I|BIT_F)        
	msr     cpsr,r0
	ldr     r1, =  CPU0_STACK_TOP - 8        
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
       
GotoSuspend:
	MRC     p15, 0, r1, c1, c0, 0       //Read CP15 control register    
	BIC     r1, r1, #CR_ICACHE    
	MCR     p15, 0, r1, c1, c0, 0       //Write back control register    

	MOV		R10,#1
	adr r1, quickboot_param_addr
	ldr r0,[r1]
	ldr     pc, =core_suspend

#ifdef __AndroidM__
GotoTZ:
    ldr r0,=cpu_Resume
    ldr r1,=0x201000
    ldr r2,=0x40
    b   trustzone_resume_v7
#else
GotoTZ:
    ldr r3,=cpu_Resume
    ldr r2,=0x100000
    mov pc,r2
#endif

cpu_Resume:
  # We're back, Restore current CPU stats
    ldr  r0,=_loader_start  //load loader(nfi or emmc) start address
    add r0,r0,#(DRAM_BACKUP_REG_SIZE)


    /*skip resume pc*/
    add     r2,r0, #4                  
    ldr     r3, [r0]               @ R3 point to resume registers
    #bic     r3, r3, #0xc0000000     @ address translation from physical to virtual
    #add     r3,r3,#0x1000000
    add     r0, r0, #(32+4)         @ Skip resume registers
    ldmia   r0!, {r4-r11}
    mov     r1, #0
    mcr     p15, 0, r1, c7, c5, 0   @ invalidate I cache
    mcr     p15, 0, r1, c7, c10, 4  @ drain write buffer
    mcr     p15, 0, r11, c13, c0, 1 @ Context ID
    mcr     p15, 0, r10, c13, c0, 0 @ FCSE/PID
    mcr     p15, 0, r6, c3, c0, 0   @ Domain ID
    mcr     p15, 0, r1, c8, c7, 0   @ invalidate I + D TLBs
    isb
    dsb
    isb
    mcr     p15, 0, r7, c2, c0, 0   @ Translation table base 0
    mcr     p15, 0, r8, c2, c0, 1   @ Translation table base 1
    mcr     p15, 0, r9, c1, c0, 1   @ auxiliary control register @ for init ACTLR.SMP
    mcr     p15, 0, r5, c1, c0, 2   @ co-processor access control
    mcr     p15, 0, r1, c2, c0, 2   @ TTB control register

    ldmia   r0!, {r5-r7}
    mcr     p15, 0, r5, c10, c2, 0  @ PRRR
    mcr     p15, 0, r6, c10, c2, 1  @ NRRR
    mcr     p15, 0, r7, c13, c0, 3  @ User r/o thread ID


    # Now, enable Cache & MMU
    mcr     p15, 0, r1, c7, c5, 4   @ ISB
    mcr     p15, 0, r4, c1, c0, 0   @ turn on MMU, I-cache, D-cache, etc
    nop
    nop
    nop
    nop

cpu_ResumeMMUOn:
    @mcr     p15, 0, r1, c7, c14, 0 @ clean+invalidate D cache
    mcr     p15, 0, r1, c7, c5, 0   @ invalidate I cache
    @mcr     p15, 0, r1, c7, c15, 0 @ clean+invalidate cache
    @                               @ TODO: invalidate D cache
    mcr     p15, 0, r1, c8, c7, 0   @ invalidate I + D TLBs
    isb
    dsb
    isb
    nop

#    orr     r0, r0, #0xc0000000    @ address translation from physical to virtual
    @ Load current mode....
    ldr     r4, [r0], #4

    @ USER registers (using SYS mode)
    cps     #MODE_SYS
    ldmia   r0!, {sp, lr}

    @ SVC registers
    cps     #MODE_SVC
    ldmia   r0!, {r5, sp, lr}
    msr     SPSR_cxsf, r5

    @ ABT registers
    cps     #MODE_ABT
    ldmia   r0!, {r5, sp, lr}
    msr     SPSR_cxsf, r5

    @ UNDEF registers
    cps     #MODE_UDF
    ldmia   r0!, {r5, sp, lr}
    msr     SPSR_cxsf, r5

    @ IRQ registers
    cps     #MODE_IRQ
    ldmia   r0!, {r5, sp, lr}
    msr     SPSR_cxsf, r5

    @ FIQ registers
    cps     #MODE_FIQ
    ldmia   r0!, {r5, r8-r14}
    msr     SPSR_cxsf, r5

    @ Switch back to original mode.
    msr     CPSR_cxsf, r4

    @ We're done, returning
    @ Because stackframe is corrupted, just skip them.
    mov     r0, #0
    ldmia   r2, {r4-r11}
    bx   r3


ac8317_power_off:
    mov r0,pc 
    add r0,r0,#(28-8)
    ldr r1,[r0]
    mov r0,#0x14
    str r0, [R1]
    mov r0,#0x04
    str r0,[R1]
    #b  .
    .word 0xF0024084
ac8317_power_off_end:

quickboot_param_addr:
	.word  0x5A5A5A5A

	.end


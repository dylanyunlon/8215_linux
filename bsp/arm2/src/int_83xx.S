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

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  .align 2; \
  name:
#endif

#ifndef END
#define END(name) \
  .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
  .type name, %function; \
  END(name)
#endif


    .section  .resident.text
#        .text


#include  "../inc/83xx.inc"

ENTRY(reset)
        /*
        * set the cpu to SVC32 mode
        */
        mov     r0, #INIT_CPSR
        msr     cpsr_cxsf,r0

        ldr     r0, =set_stack
        BX      r0                 // & jump to new virtual address
        nop

set_stack:
        /* SVC Mode Only */
        LDR  sp, =SYS_STACK_BASE  //at 2M  //ARM2_OFFSET_BASE + 2M

       /* IRQ Mode */
        ldr     r2, =ARM2_IRQ_STACK_BASE        /* Pickup IRQ stack size in bytes*/
        bic     r2,r2,#3                        /* Insure word alignment         */
        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear the mode bits           */
        orr     r0,r0,#MODE_IRQ                 /* Set the IRQ mode bits         */
        msr     CPSR_cxsf,r0                    /* Move to IRQ mode              */
        mov     sp,r2                           /* Setup IRQ stack pointer       */

        ldr     r2, =ARM2_EXCPTION_STACK_BASE        /* Pickup IRQ stack size in bytes*/
        bic     r2,r2,#3                        /* Insure word alignment         */
        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear the mode bits           */
        orr     r0,r0,#MODE_ABT                 /* Set the ABT mode bits         */
        msr     CPSR_cxsf,r0                    /* Move to ABT  mode              */
        mov     sp,r2                           /* Setup IRQ stack pointer       */

        ldr     r2, =ARM2_EXCPTION_STACK_BASE        /* Pickup IRQ stack size in bytes*/
        bic     r2,r2,#3                        /* Insure word alignment         */
        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear the mode bits           */
        orr     r0,r0,#MODE_UDF                /* Set the UDF mode bits         */
        msr     CPSR_cxsf,r0                    /* Move to UDF  mode              */
        mov     sp,r2                           /* Setup IRQ stack pointer       */

        /* Return to supervisor mode. */

        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear mode bits               */
        orr     r0,r0,#MODE_SVC                 /* Set the supervisor mode bits  */
        orr     r0,r0,#LOCKOUT                  /* Insure IRQ/FIQ interrupts are */
                                                /* locked out                    */
        msr     CPSR_cxsf,r0                    /* All interrupt stacks are setup*/
        ldr     sp, =SVC_STACK_BASE  //at 2M  //ARM2_OFFSET_BASE + 2M

        bl      func_table_init
zero_bss:
        /* zero bss */
        ldr    r0, =_bss_start
        ldr    r1, =_end
        ldr    r3, =0x0
        ldr    r4, =0x0
        ldr    r5, =0x0
        ldr    r6, =0x0
        ldr    r7, =0x0
        ldr    r8, =0x0
        ldr    r9, =0x0
        ldr    r10, =0x0

zero_loop:
        cmp    r0, r1
        stmcc   r0!, {r3-r10}           /* zeroing target address [r0]    */
        bcc    zero_loop

        mov     r0, #0x0
        ldr     r1,=VectorInstructions
        mov     r2,#0x40
        bl      memcpy

GotoCMain:
        bl     cmain
        b .

ENDPROC(reset)


# void Enable_IRQ
#
ENTRY(Enable_IRQ)
        mrs      r1, cpsr
        bic      r1, r1, #BIT_I
        msr      cpsr_c, r1
        bx       lr
ENDPROC(Enable_IRQ)

.macro cache_range_op, crm op
        add     r2, r0, r1                  // calculate the end address
        bic     r3, r0, #(32 - 1)           // align the start with a cache line
.Lcache_range_op_loop\@:
        mcr     p15, 0, r3, c7, \crm, \op
        add     r3, r3, #32
        cmp     r3, r2
        blo     .Lcache_range_op_loop\@

        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4      // data sync barrier (formerly drain write buffer)
.endm

#void Flush_Cache(UINT32 u4Start, UINT32 u4Len)
#
ENTRY(Flush_Cache)
        push    {r11, r14}
        cache_range_op c10 1

        pop     {r11, r15}
ENDPROC(Flush_Cache)

#void Flush_Invalid_Cache(UINT32 u4Start, UINT32 u4Len)
#
ENTRY(Flush_Invalid_Cache)
        push    {r11, r14}
        cache_range_op c6 1

        pop     {r11, r15}
ENDPROC(Flush_Invalid_Cache)

#void Get_SysSP(unsigned int mask)
#
ENTRY(Get_SysSP)
        mrs     r1, cpsr
        bic     r2, r1, #0x1F
        orr     r2, r2, r0
        msr     cpsr_c, r2

        mov     r0, sp

        msr     cpsr_c, r1
        bx      lr
ENDPROC(Get_SysSP)

#void Eisable_IRQ
#
ENTRY(Disable_IRQ)
        mrs      r1, cpsr
        orr      r1, r1, #BIT_I
        msr      cpsr_c, r1
        bx      lr
ENDPROC(Disable_IRQ)


ENTRY(IRQDisableStatus)
        mrs      r0, cpsr
        and      r0, r0, #BIT_I
        bx      lr
ENDPROC(IRQDisableStatus)


#
#  Function:  EnterSleepMode
#
ENTRY(EnterSleepMode)
        mov     r1, #0
        mcr     p15, 0, r1, c7, c0, 4
        mov     pc, lr
ENDPROC(EnterSleepMode)


        .end


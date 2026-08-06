/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

    .text
    .arm
    .syntax unified

    .set SYS_MODE,   0x1f
    .set SVC_MODE,   0x13
    .set IRQ_MODE,   0x12
    .set CPSR_I_BIT, 0x80

    /* Variables and functions. */
    .extern pxCurrentTCB
    .extern vTaskSwitchContext
    .extern vApplicationIRQHandler
    .extern ulPortInterruptNesting
    .extern ulPortTaskHasFPUContext
    .extern ulPortYieldRequired
    .extern vApplicationIRQHandler

    .global FreeRTOS_IRQ_Handler
    .global vPortYield
    .global vPortStartFirstTask
    .global vPortYieldProcessor

/*-----------------------------------------------------------*/

.macro portSAVE_CONTEXT

    /* Save the LR and SPSR onto the system mode stack before switching to
     * system mode to save the remaining system mode registers. */
    STMDB    SP!, {R0}

    STMDB    SP, {SP}^
    NOP
    SUB      SP, SP, #4
    LDMIA    SP!, {R0}

    STMDB     R0!, {LR}
    MOV       LR,  R0
    LDMIA     SP!, {R0}

    STMDB     LR, {R0-LR}^
    NOP
    SUB       LR, LR, #60

    MRS       R0, SPSR
    STMDB     LR!, {R0}

    /* Push the critical nesting count. */
    LDR     R0, =ulCriticalNesting
    LDR     R0, [R0]
    STMDB   LR!, {R0}

    /* Save the stack pointer in the TCB. */
    LDR     R0, =pxCurrentTCB
    LDR     R1, [R0]
    STR     LR, [R1]

    .endm

/*-----------------------------------------------------------*/

.macro portRESTORE_CONTEXT

    /* Set the SP to point to the stack of the task being restored. */
    LDR     R0, =pxCurrentTCB
    LDR     R0, [R0]
    LDR     LR, [R0]

    /* Restore the critical section nesting depth. */
    LDR     R0, =ulCriticalNesting
    LDMFD   LR!, {R1}
    STR     R1, [R0]

    LDMFD   LR!, {R0}
    MSR     SPSR_cxsf, R0

    LDMFD    LR, {R0-R14}^
    NOP

    LDR      LR, [LR, #+60]

    SUBS     PC, LR, #4 

    .endm

/*-----------------------------------------------------------*/
.align 4
.type vPortStartFirstTask, %function
vPortStartFirstTask:
    portRESTORE_CONTEXT

/*-----------------------------------------------------------*/
.align 4
.type vPortYield, %function
vPortYield:
    SVC  0
    bx  lr


.align 4
.type vPortYieldProcessor, %function
vPortYieldProcessor:
    ADD LR, LR, #4
    portSAVE_CONTEXT
    LDR  R0, =vTaskSwitchContext
    MOV  LR, PC
    BX   R0
    portRESTORE_CONTEXT

/*-----------------------------------------------------------*/

.align 4
.type FreeRTOS_IRQ_Handler, %function
FreeRTOS_IRQ_Handler:
    /* Return to the interrupted instruction. */
    portSAVE_CONTEXT

    /* Call the function that selects the new task to execute.
     * vTaskSwitchContext() if vTaskSwitchContext() uses LDRD or STRD
     * instructions, or 8 byte aligned stack allocated data.  LR does not need
     * saving as a new LR will be loaded by portRESTORE_CONTEXT anyway. */
    BL     arm2_ISR

    /* Restore the context of, and branch to, the task selected to execute
     * next. */
    portRESTORE_CONTEXT
/*-----------------------------------------------------------*/

.end


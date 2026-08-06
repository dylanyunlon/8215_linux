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

        .global  VectorTable
        .global  VectorInstructions

        .section   .resident.text

        b       reset
        b       undefined_instruction
        b       software_interrupt
        b       prefetch_abort
        b       data_abort
        b       not_used
        b       vIrqProcWrap
        b       fiq


undefined_instruction:
        b       undef_handler
        b .
software_interrupt:
        b       vPortYieldProcessor
prefetch_abort:
        b       prefetch_abort_handler
        b .
data_abort:
        b       data_abort_handler
        b .
not_used:
        b       not_used
fiq:
        b       fiq

vIrqProcWrap:
        b  FreeRTOS_IRQ_Handler
VectorInstructions:
        ldr     pc, [pc, #0x20 -8]            /* reset*/
        ldr     pc, [pc, #0x20 -8]               /*undefined instruction*/
        ldr     pc, [pc, #0x20 -8]               /*SVC*/
        ldr     pc, [pc, #0x20 -8]               /* Prefetch abort*/
        ldr     pc, [pc, #0x20 -8]              /* data abort*/
        ldr     pc, [pc, #0x20 -8]              /*unused vector location*/
        ldr     pc, [pc, #0x20 -8]              /* IRQ*/
        ldr     pc, [pc, #0x20 -8]              /*FIQ  */

VectorTable:
         .long     -1                              /*reset*/
         .long     undefined_instruction                  /*undefined instruction*/
         .long     software_interrupt                      /*SVC*/
         .long     prefetch_abort                   /* Prefetch abort*/
         .long     data_abort                /* data abort*/
         .long     -1                              /*unused vector*/
         .long     vIrqProcWrap                     /* IRQ*/
         .long     fiq                      /* FIQ        */

       .end


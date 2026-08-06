/*
 *  armboot - Startup Code for ARM1167 CPU-core
 *
 *  Copyright (c) 2004	Texas Instruments <r-woodruff2@ti.com>
 *  Copyright (c) 2001	Marius Grger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Zpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/sizes.h>

.globl _start
_start: b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq
.global _sdagentflag
_sdagentflag:	.word 0x0
.global _logosize
_logosize:	.word 0x400000
.global _arm2size
_arm2size:	.word 0x200000
.global _dtbsize
_dtbsize:	.word 0x200000
#ifdef NEW_PARTITION_DESIGN
.global _aut_version
_aut_version:	.word 0x0
#endif
.global _bootmisc_size
_bootmisc_size:	.word 0x40000
.global _mtz_size
_mtz_size: .word 0x10000


_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq
_pad:			.word 0x12345678 /* now 16*4=64 */
.global _end_vect
_end_vect:

	.balignl 16,0xdeadbeef
/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_U_BOOT_STR_SIG:
  .word    0x9B10ADE4

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word   TEXT_BASE

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

_U_BOOT_END_SIG:
  .word    0x4EDA01B9

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word 0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif

/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

/*
loophere:
	b	loophere
*/

#if 0
rom_vector_copy:
	/* Next bits originate from OMAP2420H4 */
	/* Copy vectors to mask ROM indirect addr */
	adr	r0, _start		/* r0 <- current position of code   */
	add     r0, r0, #4		/* skip reset vector */
	mov	r2, #64			/* r2 <- size to copy  */
	add	r2, r0, r2		/* r2 <- source end address */
	mov	r1, #SRAM_OFFSET0	/* build vect addr */

next:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	bne	next			/* loop until equal */
//#else
rom_vector_copy:
	/* Next bits originate from OMAP2420H4 */
	/* Copy vectors to mask ROM indirect addr */
	adr	r0, _start		/* r0 <- current position of code   */
	add     r0, r0, #4		/* skip reset vector */
	mov	r2, #48			/* r2 <- size to copy  */
	add	r2, r0, r2		/* r2 <- source end address */
	mov	r1, #SRAM_OFFSET0	/* build vect addr */

next:
	ldmia	r0!, {r3-r8}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r8}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	bne	next
#endif
	bl	cpu_init_crit

#if 0
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp	r0, r1			/* don't reloc during debug	    */
	beq	stack_setup

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot	    */
	add	r2, r0, r2		/* r2 <- source end address	    */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
#endif

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN /* malloc area			    */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo			    */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

#if 1
clear_bss:
	/* using memset */
	ldr r0, _bss_start
	ldr r2, _bss_end
	mov r1, #0x0
	sub r2, r2, r0

	bl  memset
#else
	ldr	r0, _bss_start		/* find start of bss segment	    */
	ldr	r1, _bss_end		/* stop here			    */
	mov	r2, #0x00000000		/* clear			    */

clbss_l:
	str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l
#endif

	/* setup vector table */
	ldr r0, =#0x0
	adr r1, _start
	ldr r2, =#0x100
	bl  memcpy

	ldr r10, =CFG_PAGETABLE_ADDRESS
	mov r0, r10

	bl CreatePageTable

	/*The page tables and exception vectors are setup.
	 *Initialize the MMU and turn it on.
	 */
	mov r1, #0
	mov r1, #1
	mcr p15, 0, r1, c3, c0, 0   //setup access to domain 0
	mcr p15, 0, r10, c2, c0, 0

	ldr r0, _sdagentflag
	cmp r0, #0
	mov r1, #0x0079
	orrne r1, r1, #0x4      //Enable the DCache in SD-Upgrade mode
	orr r1, r1, #0x1800     //Enable the ICache and Branch Prediction

	ldr r0, =JUMP_START_ARMBOOT
	mcr p15, 0, r1, c1, c0, 0
	bx  r0                 // & jump to new virtual address
	nop
	nop
	nop
JUMP_START_ARMBOOT:

	ldr	pc, _start_armboot

_start_armboot: .word start_armboot

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	//mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
/*	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align */
    bic r0, r0, #0x00000002	@ disable bit 2 (A) Align
    orr r0, r0, #0x00400000 @ set bit 22 (U) Un-align support
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Jump to board specific initialization... The Mask ROM will have already initialized
	 * basic memory.  Go here to bump up clock rate and handle wake up conditions.
	 */
	mov	ip, lr		/* persevere link reg across call */
	mov	lr, ip		/* restore link */
	mov	pc, lr		/* back to my caller */

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */
@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC	0x13
#define I_BIT	 	0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE		@ carve out a frame on current user stack
	stmia	sp, {r0 - r12}			@ Save user registers (now in svc mode) r0-r12

	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_SYS_MALLOC_LEN)
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
	ldmia	r2, {r2 - r3}			@ get values for "aborted" pc and cpsr (into parm regs)
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp				@ save current stack into r0 (param register)
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r8, sp, #S_PC			@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	stmdb	r8, {sp, lr}^			@ Calling SP, LR
	str	lr, [r8, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]			@ Save CPSR
	str	r0, [r8, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack (enter in banked mode)
	sub	r13, r13, #(CONFIG_SYS_MALLOC_LEN)	@ move past malloc pool
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8) @ move to reserved a couple spots for abort stack

	str	lr, [r13]			@ save caller lr in position 0 of saved stack
	mrs	lr, spsr			@ get the spsr
	str	lr, [r13, #4]			@ save spsr in position 1 of saved stack

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13			@ switch modes, make sure moves will execute
	mov	lr, pc				@ capture return pc
	movs	pc, lr				@ jump to next instruction & switch modes.
	.endm

	.macro get_bad_stack_swi
	sub	r13, r13, #4			@ space on current stack for scratch reg.
	str	r0, [r13]			@ save R0's value.
	ldr	r0, _armboot_start		@ get data regions start
	sub	r0, r0, #(CONFIG_SYS_MALLOC_LEN)	@ move past malloc pool
	sub	r0, r0, #(CONFIG_SYS_GBL_DATA_SIZE+8)	@ move past gbl and a couple spots for abort stack
	str	lr, [r0]			@ save caller lr in position 0 of saved stack
	mrs	r0, spsr			@ get the spsr
	str	lr, [r0, #4]			@ save spsr in position 1 of saved stack
	ldr	r0, [r13]			@ restore r0
	add	r13, r13, #4			@ pop stack entry
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

	.macro warm_cpu_stop            @ if not cold boot cpu, then stop it
	ldr	r13, _armboot_start		@ setup our mode stack (enter in banked mode)
	sub	r13, r13, #(CONFIG_SYS_MALLOC_LEN)	@ move past malloc pool
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8) @ move to reserved a couple spots for abort stack

	push {r0}
	mrc p15, 0, r0, c0, c0, 5
	and r0, r0, #0xf
	cmp r0, #0
	bne .
	pop {r0}
	.endm
/*
 * exception handlers
 */
	.align	5
undefined_instruction:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	warm_cpu_stop
	get_bad_stack_swi
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	warm_cpu_stop
	get_irq_stack
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	warm_cpu_stop
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	warm_cpu_stop
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq

#endif
	.align 5
.global arm1176_cache_flush
arm1176_cache_flush:
	mcr	p15, 0, r1, c7, c5, 0	@ invalidate I cache
	mov	pc, lr			@ back to caller

	.align	5
.globl reset_cpu
reset_cpu:
_loop_forever:
	b	_loop_forever


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
        cache_range_op c10 1

        bx      lr
ENDPROC(Flush_Cache)

#void Flush_Invalid_Cache(UINT32 u4Start, UINT32 u4Len)
#
ENTRY(Flush_Invalid_Cache)
        cache_range_op c14 1

        bx      lr
ENDPROC(Flush_Invalid_Cache)

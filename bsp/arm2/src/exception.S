.syntax unified
    .cpu arm9
    .text
    .global undef_handler
    .global prefetch_abort_handler
    .global data_abort_handler

/* -----------------------------------------------------
 * Save ARM9 exception mode register frame
 * Order must be consistent with arm_fault_frame_t:
 * [0-12]: r0-r12
 * [13]  : lr 
 * [14]  : spsr
 * [15]  : fiq_sp
 * [16]  : fiq_lr
 * [17]  : irq_sp
 * [18]  : irq_lr
 * [19]  : svc_sp
 * [20]  : svc_lr
 * [21]  : abt_sp
 * [22]  : abt_lr
 * [23]  : und_sp
 * [24]  : und_lr
 * [25]  : sys_sp
 * [26]  : sys_lr
 * ----------------------------------------------------- */

    .macro SAVE_FRAME
        sub     sp, sp, #(4 * 27)

        /* r0-r12 */
        stm     sp, {r0-r12}

        /* lr */
        str     lr, [sp, #(4 * 13)]

        /* spsr */
        mrs     r0, spsr
        str     r0, [sp, #(4 * 14)]

        mrs     r1, cpsr

        /* Save current mode SP to temporary register */
        mov     r2, sp                   /* Save current SP to r2 for subsequent offset access */

        /* FIQ mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x11   /* FIQ mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 15)]
        str     lr, [r2, #(4 * 16)]

        /* IRQ mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x12   /* IRQ mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 17)]
        str     lr, [r2, #(4 * 18)]

        /* SVC mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x13   /* SVC mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 19)]
        str     lr, [r2, #(4 * 20)]

        /* ABT mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x17   /* ABT mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 21)]
        str     lr, [r2, #(4 * 22)]

        /* UND mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x1b   /* UND mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 23)]
        str     lr, [r2, #(4 * 24)]

        /* SYS mode */
        mrs     r0, cpsr
        bic     r0, r0, #0x1f
        orr     r0, r0, #0x1f   /* SYS mode */
        msr     cpsr_c, r0
        str     sp, [r2, #(4 * 25)]
        str     lr, [r2, #(4 * 26)]

        /* Restore original CPSR */
        msr     cpsr_c, r1
    .endm

/* -----------------------------------------------------
 * Undefined instruction
 * ----------------------------------------------------- */
undef_handler:
    SAVE_FRAME
    mov     r0, sp
    mov     r1, #1              /* FAULT_UNDEF */
    bl      fault_entry_c
    b       .

/* -----------------------------------------------------
 * Prefetch Abort
 * ----------------------------------------------------- */
prefetch_abort_handler:
    SAVE_FRAME
    mov     r0, sp
    mov     r1, #2              /* FAULT_PREFETCH */
    bl      fault_entry_c
    b       .

/* -----------------------------------------------------
 * Data Abort
 * ----------------------------------------------------- */
data_abort_handler:
    SAVE_FRAME
    mov     r0, sp
    mov     r1, #3              /* FAULT_DATA */
    bl      fault_entry_c
    b       .
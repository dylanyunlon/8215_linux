/*************************************************************************/
/*****************           ATC CONFIDENTIAL            *****************/
/*****************                                       *****************/
/*****************   Description : AC83xx trustzone      *****************/
/*****************                                       *****************/
/*****************                                       *****************/
/*****************       Company : Aucochips Inc.        *****************/
/*****************       Programmer : Emily Zhang        *****************/
/*************************************************************************/

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
				
#include  "../inc/83xx.inc"

.globl _start 

ENTRY(_start)

       /*
        * Read Multiprocessor ID register (MPIDR)
        * get the id of the cpu running the code
        */
       mrc     p15, 0, r8, c0, c0, 5					
       and     r8, r8, #3
       cmp     r8, #0
       bne     _secondary_cpu_monitorInit             

       /*
        * save call back func address and param
        * for core0 to start the kernel
        */
       ldr r7, =CALL_BACK
       str r0, [r7], #4     //save param: 0
       str r1, [r7], #4     //save param: mach id
       str r2, [r7], #4     //save param: fdt address
       str r3, [r7]         //save param: call back fun address, to boot linux kernel       
       
       /*
        * set the cpu to SVC32 mode
        */
        mrs	r0,cpsr
        bic	r0,r0,#0x1f
        orr	r0,r0,#0xd3
        msr	cpsr,r0
        ldr     sp, =TZ_SVC_STACK_BASE

        #b .

        /*Create PageTable*/
        ldr r10, =TZ_PAGETABLE_ADDRESS
        mov r0, r10
        bl CreatePageTable

        /*The page tables and exception vectors are setup.
        *Initialize the MMU and turn it on. 
        */
        mov    r1, #0
       // mcr    p15,0,r1,c7,c6,0          //Invalidate D-Cache
        mov     r1, #1
        mcr     p15, 0, r1, c3, c0, 0   //setup access to domain 0
        mcr     p15, 0, r10, c2, c0, 0

        #mcr     p15, 0, r0, c8, c7, 0   // flush I+D TLBs

        //mov    r1, #0x007D
        mov    r1, #0x007C   //don't enable mmu
        orr	r1, r1, #0x1800     //Enable the ICache and Branch Prediction
        #orr  r1, r1, #(1<<13)    //enable high vectors 0xffff0000 - 0xffff001c

       // ldr     r0, =set_stack
      //  mcr     p15, 0, r1, c1, c0, 0
      //  BX      r0                 // & jump to new virtual address
        b set_stack
        nop
        nop
        nop

set_stack:
        /* SVC Mode Only */
        //LDR  sp, =0x400000  //at 4M  //ARM2_OFFSET_BASE + 4M
        LDR  sp, =TZ_SVC_STACK_BASE  //at 2M  //ARM2_OFFSET_BASE + 2M

       /* IRQ Mode */
       // ldr     r1, =ARM2_IRQ_STACK_SIZE        /* Pickup IRQ stack size in bytes*/
       // add     r2,sp,r1                        /* Allocate IRQ stack area       */
        ldr     r2, =TZ_MON_STACK_BASE        /* Pickup IRQ stack size in bytes*/
        bic     r2,r2,#3                        /* Insure word alignment         */
        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear the mode bits           */
        orr     r0,r0,#MODE_MON                 /* Set the IRQ mode bits         */
        msr     CPSR_cxsf,r0                    /* Move to IRQ mode              */
        mov     sp,r2                           /* Setup IRQ stack pointer       */

        /* Return to supervisor mode. */

        mrs     r0,CPSR                         /* Pickup current CPSR           */
        bic     r0,r0,#MODE_MASK                /* Clear mode bits               */
        orr     r0,r0,#MODE_SVC                 /* Set the supervisor mode bits  */
        orr     r0,r0,#LOCKOUT                  /* Insure IRQ/FIQ interrupts are */
                                                /* locked out                    */
        msr     CPSR_cxsf,r0                    /* All interrupt stacks are setup*/

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

flushcache:
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 6                 /* BPIALL - Invalidate entire branch predictor array */
             
enablePrediction:
        mrc     p15, 0, r0, c1, c0, 0                 /* Read SCTLR */
        orr     r0, r0, #(1 << 11)                    /* Set the Z bit (bit 11) */
        mcr     p15, 0,r0, c1, c0, 0                  /* Write SCTLR */

enableCaches:
        mrc     p15, 0, r0, c1, c0, 0                /* Read System Control Register configuration data */
        orr     r0, r0, #(1 << 2)                    /* Set C bit */
        orr     r0, r0, #(1 << 12)                   /* Set I bit */
        mcr     p15, 0, r0, c1, c0, 0                /* Write System Control Register configuration data */

/*
*   set jump instruction to address 0 for secondary cpu to jump into tz when power on
*/
SetupSecondaryCPUBootupInstruction:

        ldr	r4, =_start  //set trustzone entry addr to dram 0x8
        mov	r5, #8
        str	r4, [r5]

        ldr	r7, =JumpAddressForSecondaryCPU 
        ldr	r0, [r7], #4
        ldr	r1, [r7], #4

        mov	r4, #0
        str	r0, [r4], #4
        str	r1, [r4]


GotoMonitor:
        #ldr     pc, =_primary_cpu_monitorInit
        b	_primary_cpu_monitorInit
        
ENDPROC(_start)

.global CALL_BACK
CALL_BACK:  
        .long 0,0,0,0


/*
 *   set jump instruction to address 0 for secondary cpu to jump into tz when power on
 */

JumpAddressForSecondaryCPU:
        mov	r0, #8
        ldr	pc, [r0]
         
.end
        


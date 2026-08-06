/*************************************************************************/
/*****************           ATC CONFIDENTIAL            *****************/
/*****************                                       *****************/
/*****************   Description : AC83xx trustzone      *****************/
/*****************                                       *****************/
/*****************                                       *****************/
/*****************       Company : Aucochips Inc.        *****************/
/*****************       Programmer : Emily Zhang        *****************/
/*************************************************************************/

/* ------------------------------------------------------------
;Secure Monitor
;
; Implement a basic monitor
; ------------------------------------------------------------*/
    
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

.macro read_scr reg
mrc p15, 0, \reg, c1, c1, 0
.endm

.macro write_scr reg
mcr p15, 0, \reg, c1, c1, 0
.endm

.macro LOCAL_FUNC name colon
	.type \name , %function
	\name \colon
	.endm

.macro END_FUNC name
    .size \name , .-\name
    .endm


#include  "../inc/83xx.inc"

/* ------------------------------------------------------------
; Monitor mode vector table
; ------------------------------------------------------------*/
.p2align 5
.globl monitor_vector
monitor_vector:
  nop     /* Reset      - not used by Monitor */
  nop     /* Undef      - not used by Monitor */
  b       SMC_Handler
  nop     /* Prefetch   - can by used by Monitor */
  nop     /* Data abort - can by used by Monitor */
  nop     /* RESERVED */
  nop     /* IRQ        - can by used by Monitor */
  nop     /* FIQ        - can by used by Monitor */

LOCAL_FUNC sm_save_modes_regs , :
  /* User mode registers has to be saved from system mode */
  cps #MODE_SYS
  stm r0!, {sp, lr}

  cps #MODE_IRQ
  mrs r2, spsr
  stm r0!, {r2, sp, lr}

  cps #MODE_SVC
  mrs r2, spsr
  stm r0!, {r2, sp, lr}

  cps #MODE_ABT
  mrs r2, spsr
  stm r0!, {r2, sp, lr}

  cps #MODE_UNDEF
  mrs r2, spsr
  stm r0!, {r2, sp, lr}

  cps #MODE_MON
  ldm r1, {r2-r3}     /* Load SPSR and LR from the stack */
  stm r0!, {r2-r3}        /* Store SPSR and LR in context */
  bx  lr
END_FUNC sm_save_modes_regs

/* Restores the mode specific registers */
LOCAL_FUNC sm_restore_modes_regs , :
  /* User mode registers has to be saved from system mode */
  cps #MODE_SYS
  ldm r0!, {sp, lr}

  cps #MODE_IRQ
  ldm r0!, {r2, sp, lr}
  msr spsr_fsxc, r2

  cps #MODE_SVC
  ldm r0!, {r2, sp, lr}
  msr spsr_fsxc, r2

  cps #MODE_ABT
  ldm r0!, {r2, sp, lr}
  msr spsr_fsxc, r2

  cps #MODE_UNDEF
  ldm r0!, {r2, sp, lr}
  msr spsr_fsxc, r2

  cps #MODE_MON
  ldm r0!, {r2-r3}        /* Load SPSR and LR from context */
  stm r1, {r2-r3}     /* Store SPSR and LR in stack */
  bx  lr
END_FUNC sm_restore_modes_regs


/* ------------------------------------------------------------
; SMC Handler
;
; - Detect which world executed SMC
; - Saves state to appropiate stack
; - Restores other worlds state
; - Switches world
; - Performs exception return
; ------------------------------------------------------------*/

ENTRY(SMC_Handler)
   srsdb   sp!, #MODE_MON
   push   {r0-r3}           /* R0-r3 contain args to be passed between worlds */

   clrex
  /* Which world have we come from */
  mrc     p15, 0, r0, c1, c1, 0        /* Read Secure Configuration Register data */
  tst     r0, #NS_BIT                  /* Is the NS bit set? */
  bne     .smc_ret_to_sec
  
.smc_ret_to_nsec:
    /* Save secure context */
    bl  sm_get_sec_ctx
    add r1, sp, #SMC_ENTRY_SRS_OFFS /* Where srsdb wrote */
    bl  sm_save_modes_regs

    mov r0, sp
    mov r1, r4
    bl  sm_set_nsec_ret_vals

    /* Restore non-secure context */
    bl  sm_get_nsec_ctx
    add r1, sp, #SMC_ENTRY_SRS_OFFS /* Where srsdb wrote */
    bl  sm_restore_modes_regs
    ldm r0!, {r4-r12}

    /* Update SCR */
    read_scr r0
    eor     r0, r0, #NS_BIT  /* Set NS and FIQ bit in SCR */
    write_scr r0

    b   .smc_exit

.smc_ret_to_sec:
    eor     r0, r0, #NS_BIT              /* Toggle NS bit */
    mcr     p15, 0, r0, c1, c1, 0        /* Write Secure Configuration Register data */

	/* Save non-secure context */
	push	{r12, lr}
	bl	sm_get_nsec_ctx
	pop	{r12, lr}
	add	r1, sp, #SMC_ENTRY_SRS_OFFS /* Where srsdb wrote */
	bl	sm_save_modes_regs
	stm	r0!, {r4-r12}

	/*
	 * Update secure context with vector depending on SMC function,
	 * also updates entry reason
	 */
	mov	r0, sp
	bl	sm_set_sec_smc_entry

	/* Restore secure context */
	bl	sm_get_sec_ctx
	add	r1, sp, #SMC_ENTRY_SRS_OFFS /* Where srsdb wrote */
	bl	sm_restore_modes_regs  
//end of smc_rec_to_sec

 #if 0 
  /* Load save to pointer */
  ldreq   r0, =S_STACK_SP             /* If NS bit set, was in Normal world.  So restore Secure state */
  ldrne   r0, =NS_STACK_SP
  ldr     r2, [r0]

  /* Load restore from pointer */
  ldreq   r1, =NS_STACK_SP
  ldrne   r1, =S_STACK_SP
  ldr     r3, [r1]

  /* r2  <-- save to */
  /* r3  <-- restore from */

  /* Save general purpose registers, SPSR and LR */
  stmfd   r2!, {r4-r12}               /* Save r4 to r12 */
  mrs     r4, spsr                    /* Also get a copy of the SPSR */
  stmfd   r2!, {r4, lr}               /* Save original SPSR and LR */

  str     r2, [r0]                    /* Save updated pointer back, r0 and r2 now free */

  /* Restore other worlds registers, SPSR and LR */
  ldmfd   r3!, {r0, lr}               /* Get SPSR and LR from */
  /* ADD SUPPORT FOR SPs */
  msr     spsr_cxsf, r0               /* Restore SPSR */
  ldmfd   r3!, {r4-r12}               /* Restore registers r4 to r12 */

  str     r3, [r1]                    /* Save updated pointer back, r1 and r3 now free */

  /* Clear local monitor */
  clrex                               /* Not strictly required in this example, as not using LDREX/STREX */
                                      /* However, architecturally should execute CLREX on a context switch */

  /* Now restore args (r0-r3) */
  pop     {r0-r3}


  /* Perform exception return */
  movs    pc, lr
#endif

.smc_exit:
	pop	{r0-r3}
	rfefd	sp!

ENDPROC(SMC_Handler)

/* ------------------------------------------------------------
; Monitor Initialization
;
; This is called the first time the Secure world wishes to
; move to the Normal world.
; ------------------------------------------------------------*/
ENTRY(_primary_cpu_monitorInit)
	ldr     r0, =monitor_vector          /* Get address of Monitors vector table */
	mcr     p15, 0, r0, c12, c0, 1       /* Write Monitor Vector Base Address Register */

	/*Set Non-secure Access Control register*/
	mrc     p15, 0, r4, c1, c1, 2
	ldr     r5, =0x00060c00 
	orr     r4, r5  
	mcr     p15, 0, r4, c1, c1, 2
    
	/*Set Secure Debug Enable Register*/
	mrc     p15, 0, r4, c1, c1, 1
	orr     r4, #0x3   
	mcr     p15, 0, r4, c1, c1, 1


	/*Set protection of trustzone memory */
	bl	set_dram_protection_for_tz
	
   	
        /*config gic*/
	bl      gic_secure_config	
	
	//msr     spsr_cxsf, #MODE_SVC      //Set SPSR to be SVC mode
	mrs     r4, spsr                    //**** we must make sure spsr value is correct before we use it
	bic     r4, r4, #MODE_MASK
	orr     r4, r4, #MODE_SVC
	orr     r4, r4, #0xC0               //mask F I

 	cps     #MODE_MON 
 	
 	msr     spsr_cxsf, r4               //spsr value is from svc mode             
	 /* Switch to Normal world */
	 
	mrc	 p15, 0, r4, c1, c1, 0		  /* Read Secure Configuration Register data */
	orr	 r4, #NS_BIT				  /* Set NS bit */
	orr	 r4, #0x30					  /*enable A F bit writable in non sec mode*/
	mcr	 p15, 0, r4, c1, c1, 0		  /* Write Secure Configuration Register data */

	
	ldr     r5, =GICD_IGROUPR0   //igroup0
	ldr     r6, =0xFE00FFFF
	str     r6, [r5]
	nop

    /*set param and call back func address to pc*/
	ldr r7, =CALL_BACK
	ldr r0, [r7], #4
	ldr r1, [r7], #4
	ldr r2, [r7], #4
	ldr r3, [r7]

	dsb
	isb
    
	movs pc, r3      //restore cpsr and switch to normal,then boot kernel

ENDPROC(_primary_cpu_monitorInit)


/*
*  init monitary for secondary cpu
*/
ENTRY(_secondary_cpu_monitorInit)  

	ldr 	r0, =monitor_vector 		 /* Get address of Monitors vector table */
	mcr 	p15, 0, r0, c12, c0, 1		 /* Write Monitor Vector Base Address Register */

	
	/*Set Non-secure Access Control register*/
	mrc 	p15, 0, r4, c1, c1, 2
	ldr 	r5, =0x00060c00 
	orr 	r4, r5	
	mcr 	p15, 0, r4, c1, c1, 2
	
	/*Set Secure Debug Enable Register*/
	mrc 	p15, 0, r4, c1, c1, 1
	orr 	r4, #0x3   
	mcr 	p15, 0, r4, c1, c1, 1

	mrs 	r4, spsr					//**** we must make sure spsr value is correct before we use it
	bic 	r4, r4, #MODE_MASK
	orr 	r4, r4, #MODE_SVC
	orr 	r4, r4, #0xC0				//mask F I

	cps 	#MODE_MON 
	
	msr 	spsr_cxsf, r4				//spsr value is from svc mode	

	// set mon stack for core1/2/3
	mrc 	p15, 0, r0, c0, c0, 5		 //Read Multiprocessor ID register (MPIDR)			 
	and 	r0, r0, #3
	
	cmp 	r0, #1
	ldreq	sp, =TZ_MON_STACK1_BASE

	cmp 	r0, #2
	ldreq	sp, =TZ_MON_STACK2_BASE

	cmp 	r0, #3
	ldreq	sp, =TZ_MON_STACK3_BASE

	/* Switch to Normal world */
	mrc  p15, 0, r4, c1, c1, 0		  /* Read Secure Configuration Register data */
	orr  r4, #NS_BIT				  /* Set NS bit */
	orr  r4, #0x30					  /*enable A F bit writable in non sec mode*/
	mcr  p15, 0, r4, c1, c1, 0		  /* Write Secure Configuration Register data */ 

	
	ldr 	r5, =GICD_IGROUPR0	 //igroup0 is banked for each cpu
	ldr 	r6, =0xFE00FFFF
	str 	r6, [r5]
	nop
 
	//get the jump address from PC_INITR, which is set in kernel booting secondary cpu
	ldr r5, =PC_INITR
	ldr r6, [r5]

 	dsb
 	isb

	movs pc, r6	 //restore cpsr and switch to normal,then boot kernel

ENDPROC(_secondary_cpu_monitorInit)

 .end
#if 0
  ; Install Secure Monitor
  ; -----------------------
  LDR     r0, =monitor                 ; Get address of Monitors vector table
  MCR     p15, 0, r0, c12, c0, 1       ; Write Monitor Vector Base Address Register

  ; Save Secure state
  ; ------------------
  LDR     r0, =S_STACK_LIMIT          ; Get address of Secure state stack
  STMFD   r0!, {r4-r12}               ; Save general purpose registers
  ; ADD support for SPs
  MRS     r1, cpsr                    ; Also get a copy of the CPSR
  STMFD   r0!, {r1, lr}               ; Save CPSR and LR


  ; Switch to Monitor mode
  ; -----------------------
  CPS     #Mode_MON                   ; Move to Monitor mode after saving Secure state


  ; Save Secure state stack pointer
  ; --------------------------------
  LDR     r1, =S_STACK_SP              ; Get address of global
  STR     r0, [r1]                     ; Save pointer
  

  ; Set up initial NS state stack pointer
  ; --------------------------------------
  LDR     r0, =NS_STACK_SP             ; Get address of global
  LDR     r1, =NS_STACK_LIMIT          ; Get top of Normal state stack (assuming FD model)
  STR     r1, [r0]                     ; Save pointer


  ; Set up execption return information
  ; ------------------------------------
  IMPORT  ns_image
  LDR     lr, =ns_image
  MSR     spsr_cxsf, #Mode_SVP         ; Set SPSR to be SVC mode


  ; Switch to Normal world
  ; -----------------------
  MRC     p15, 0, r4, c1, c1, 0        ; Read Secure Configuration Register data
  ORR     r4, #NS_BIT                  ; Set NS bit
  MCR     p15, 0, r4, c1, c1, 0        ; Write Secure Configuration Register data


  ; Clear general purpose registers
  ; --------------------------------
  MOV     r0,  #0
  MOV     r1,  #0
  MOV     r2,  #0
  MOV     r3,  #0
  MOV     r4,  #0
  MOV     r5,  #0
  MOV     r6,  #0
  MOV     r7,  #0
  MOV     r8,  #0
  MOV     r9,  #0
  MOV     r10, #0
  MOV     r11, #0
  MOV     r12, #0

  MOVS    pc, lr


; ------------------------------------------------------------
; Space reserved for stacks
; ------------------------------------------------------------

  AREA  Monitor_Data, DATA, ALIGN=8, READWRITE

NS_STACK_BASE
  DCD     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NS_STACK_LIMIT

S_STACK_BASE
  DCD     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
S_STACK_LIMIT

NS_STACK_SP
  DCD     0

S_STACK_SP
  DCD     0


  END

; ------------------------------------------------------------
; End
; ------------------------------------------------------------
#endif


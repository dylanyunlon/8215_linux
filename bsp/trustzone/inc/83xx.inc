
/*
 * CPU related
 */

 
/* - Standard definitions of mode bits and interrupt (I&F) flags in PSRs */
#define MODE_USR     0x10
#define MODE_FIQ     0x11
#define MODE_IRQ     0x12
#define MODE_SVC     0x13
#define MODE_ABT     0x17
#define MODE_UNDEF   0x1B
#define MODE_SYS     0x1F
#define MODE_MON     0x16
#define MODE_MASK    0x1F 
#define I_Bit        0x80 /* when I bit is set, IRQ is disabled */
#define F_Bit        0x40 /* when F bit is set, FIQ is disabled */
#define NS_BIT	     0x1


#define TTB_COHERENT      0x00014c06  /* Template descriptor for coherent memory */
#define TTB_NONCOHERENT   0x00000c1e  /* Template descriptor for non-coherent memory */
#define TTB_DEVICE        0x00000c06  /* Template descriptor for device memory */

 
#define LOCKOUT    0xC0               /*Interrupt lockout value      */
#define LOCK_MSK   0xC0               /*Interrupt lockout mask value */
 
 
#define INIT_CPSR      0xD3   @( MODE_SVC | I_Bit | F_Bit )

/*CP15 control register (r1) definitions*/

#define CR_MMU                               (1 << 0)           @ MMU enable
#define CR_ALIGNMENT                         (1 << 1)           @Alignment fault checking
#define CR_DCACHE                            (1 << 2)           @ D-cache enable
#define CR_ICACHE                            (1 << 12)          @I-cache enable
#define CR_UNALIGNMENT_SUPPORT               (1 << 22)          @ Unalignment Support


/*IO Base*/

#define IO_BASE		     0xF0000000
#define CKGEN_BASE	   (IO_BASE + 0x0000)
#define HDMI_REG_BASE      (IO_BASE + 0x22C00)

/*BIM*/

/*Some register for wakeup cpu1/2/3 and some gic register*/
#define EFUSER        0xF0054664     //for get real cpu core num
#define SYNCR         0xF0008118     //use to sync between core0 and core1/2/3
#define MAGIC_NUMR    0xF0008114     //register to store magicnum for wakeup cpu1/2/3
#define MAGIC_NUM     0x4C48462E     //magic num for wakeup
#define PC_INITR      0xF0008110     //core1/2/3 pc is init from it
#define GICD_SGIR     0xF1001F00     //soft generate interrupt register
#define GICD_IGROUPR0 0xF1001080
#define GICC_IAR      0xF100200C
#define GICC_EOIR     0xF1002010
#define GICC_DIR      0xf1003000
#define GICC_CTLR     0xF1002000


/*stack*/

#define TZ_SVC_STACK_BASE        (0x200000-0x1000)
#define TZ_SVC_STACK_SIZE        (124 * 1024)

#define TZ_MON_STACK_BASE     (TZ_SVC_STACK_BASE - TZ_SVC_STACK_SIZE)
#define TZ_MON_STACK_SIZE     (112 * 1024)

#define TZ_MON_STACK1_BASE     (TZ_MON_STACK_BASE - TZ_MON_STACK_SIZE)
#define TZ_MON_STACK2_BASE     (TZ_MON_STACK1_BASE - TZ_MON_STACK_SIZE)
#define TZ_MON_STACK3_BASE     (TZ_MON_STACK2_BASE - TZ_MON_STACK_SIZE)


#define TZ_PAGETABLE_ADDRESS     (TZ_MON_STACK3_BASE - TZ_MON_STACK_SIZE - 4096*4)

#define S_STACK_SP 	(TZ_PAGETABLE_ADDRESS-32)
#define NS_STACK_SP 	(S_STACK_SP-32)

/* Positions relative to stack pointer */
#define SMC_ENTRY_R0R3_OFFS	0
#define SMC_ENTRY_SRS_OFFS	(4 * 4 + SMC_ENTRY_R0R3_OFFS)

	

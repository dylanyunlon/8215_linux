/*
 * CPU related
 */

#ifndef _DEF3360_H_
#define _DEF3360_H_

#define	ITCM_BASE_ADDRESS	0x40000000
#define	ITCM_SIZE		0x4000
#define	ITCM_HALF_SIZE		0x2000
#define	DTCM_BASE_ADDRESS	ITCM_BASE_ADDRESS + ITCM_SIZE
#define	DTCM_SIZE		0x4000
#define	DTCM_HALF_SIZE		0x2000
 
#define MODE_IRQ  0x12 
#define	MODE_SVC	0x13
#define MODE_FIQ  0x11
#define MODE_SYS  0x1F
#define MODE_USR  0x10
#define MODE_UND  0x1B
#define MODE_ABT  0x17

#define	BIT_I		0x80
#define	BIT_F		0x40

#define INIT_CPSR (MODE_SVC|BIT_I|BIT_F)

#define CPU0_STACK_TOP					0xF4007F00
#define CPU1_STACK_TOP					0xF4007FF8

#define IRQ_STACK_SIZE      0x100
#define FIQ_STACK_SIZE     0x100
#define UND_STACK_SIZE     0x100
#define ABT_STACK_SIZE      0x100
#define USR_STACK_SIZE      0x2000
#define SYS_STACK_SIZE      0x2000
#define SVC_STACK_SIZE      0x600



/*
 *
 */
#define CR_MMU                  (1 << 0)
#define CR_ALIGNMENT            (1 << 1)
#define CR_DCACHE               (1 << 2)
#define CR_ICACHE               (1 << 12)
#define CR_UNALIGNMENT_SUPPORT  (1 << 22)




#define MEM_BUF_MTD_IMG_INFO		0xC3000000
#define MEM_BUF_MTD_PART_NFB_TBL	0xC4000000	// size:0x00800000 (8M)
#define MEM_BUF_MTD_PT_NVM_TBL		0xC4800000	// size:0x00800000 (8M)
#define MEM_BUF_MTD_TMP_MEM_PTR		0xC5000000	// size:0x00800000 (8M)
#define MEM_BUF_MTD_CACHE_MEM_PTR	0xC5800000	// size:0x00800000 (8M)
#define MEM_BUF_SECURE_BOOT_1		0xC6000000	// size:0x03C00000 (60M)
#define MEM_BUF_SECURE_BOOT_2		0xC9C00000	// size:0x03C00000 (60M)
#define MEM_BUF_SECURE_SHA1_CHKSUM	0xCD800000	// size:0x00080000 (512K)
#define MEM_BUF_SECURE_AES_CMAC		0xCD880000	// size:0x00080000 (512K)

#define MEM_BUF_SIT_BOOT			0xCD900000  // size:0x00001000 (4K)
#define MEM_BUF_SIT_ACTIVE			0xCD901000  // size:0x00001000 (4K)
#define MEM_BUF_SIT_BACKUP			0xCD902000  // size:0x00001000 (4K)
#define MEM_BUF_SIT_SZ				0x00001000 
#define MEM_BUF_PIT					0xCD903000	// size:0x00001000 (4K)
#define MEM_BUF_PIT_SZ				0x00001000 
/*
 * stack
 */




#define IO_BASE        0xF0000000
#define BIM_BASE       (IO_BASE+0x8000)
#define CKGEN_BASE     (IO_BASE+0x0)
#define REG_RW_REMAP   (IO_BASE+0x3801C)
#define BIT_REMAP      1



/*
   delete
*/
#define UBOOT_ADDR		0x80100000


#define HEAP_BOTTOM         0x00100000
#define STACK_HEAD          0x10000000
//#define SVC_STACK_SIZE      8192

#define IRQST0    (BIM_BASE + 0x30)
#define IRQST1    (BIM_BASE + 0x138)
#define IRQST2    (BIM_BASE + 0x154)

#define IRQEN0    (BIM_BASE + 0x34)
#define IRQEN1    (BIM_BASE + 0x13C)
#define IRQEN2    (BIM_BASE + 0x158)

#define IRQCL0    (BIM_BASE + 0x38)
#define IRQCL1    (BIM_BASE + 0x140)
#define IRQCL2    (BIM_BASE + 0x144)


// SMP CORE BOOT
#define REG_CORE1_MAGIC  (BIM_BASE+0x114)
#define REG_RW_SLAVE_START  (BIM_BASE+0x110)
#define CORE1_MAGIC_NUM    0x4c48462e



#define REG_GICD_BASE			0xf1001000
#define REG_GICC_BASE			0xf1002000

#define REG_GICC_CTRL			(REG_GICC_BASE+0x0)
#define REG_GICC_PMR			(REG_GICC_BASE+0x4)

#endif

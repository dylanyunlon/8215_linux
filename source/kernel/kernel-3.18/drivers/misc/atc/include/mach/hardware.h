#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/io.h>
#include <mach/chip_ver.h>

extern unsigned long io_virt_base;
//extern unsigned long get_io_virt_base(void);

#define __io(a)	((void __iomem *)((unsigned long)a))

#define REG_IRQST		0x0030	//RISC L1 IRQ Status Register
#define REG_IRQEN		0x0034	//RISC L1 IRQ Enable Register
#define REG_IRQCL		0x0038	//RISC L1 IRQ Clear Register
#define REG_FIQST		0x003C	//RISC L1 FIQ Status Register
#define REG_FIQEN		0x0040	//RISC L1 FIQ Enable Register
#define REG_FIQCL		0x0044	//RISC L1 FIQ Clear Register

#define REG_IRQST2		0x0048	//RISC L2 IRQ Status Register
#define REG_IRQEN2		0x004C	//RISC L2 IRQ Enable Register
#define REG_IRQCL2		0x0050	//RISC L2 IRQ Clear Register
#define REG_FIQST2		0x0054	//RISC L2 FIQ Status Register
#define REG_FIQEN2		0x0058	//RISC L2 FIQ Enable Register
#define REG_FIQCL2		0x005C	//RISC L2 FIQ Clear Register

#define REG_IRQST3		0x0060	//RISC L3 IRQ Status Register
#define REG_IRQEN3		0x0064	//RISC L3 IRQ Enable Register
#define REG_IRQCL3		0x0068	//RISC L3 IRQ Clear Register
//#define REG_FIQST3		0x006c	//RISC L3 FIQ Status Register
#define REG_FIQEN3		0x0070	//RISC L3 FIQ Enable Register
#define REG_FIQCL3		0x0074	//RISC L3 FIQ Clear Register

#define REG_IRQST4		0x0230	//RISC L4 IRQ Status Register
#define REG_IRQEN4		0x0234	//RISC L4 IRQ Enable Register
#define REG_IRQCL4		0x0238	//RISC L4 IRQ Clear Register
#define REG_FIQST4		0x023C	//RISC L4 FIQ Status Register
#define REG_FIQEN4		0x0240	//RISC L4 FIQ Enable Register
#define REG_FIQCL4		0x0244	//RISC L4 FIQ Clear Register

#define REG_IRQST5		0x0310	//RISC L5 IRQ Status Register
#define REG_IRQEN5		0x0314	//RISC L5 IRQ Enable Register
#define REG_IRQCL5		0x0318	//RISC L5 IRQ Clear Register
#define REG_FIQST5		0x031C	//RISC L5 FIQ Status Register
#define REG_FIQEN5		0x0320	//RISC L5 FIQ Enable Register
#define REG_FIQCL5 		0x0324	//RISC L5 FIQ Clear Register

//----------------------------------------------------------------------------
//ckgen
#define REG_RW_PAD_CFG_S	0x0C0
#define REG_RW_GPIO_EN_S	0x180
#define REG_RW_GPIO_OUT_S	0x1A0
#define REG_RW_GPIO_IN_S	0x1C0
#define REG_RW_PAD_FECTL_S	0x280
#define REG_RW_PAD_PWMCTL_S	0x288

// pdwnc
#define REG_RW_GPIO_WAKEN	0x080
#define REG_RW_GPIO_PDSTAT	0x088
#define REG_RW_GPIO_PDSTCLR	0x08C
//#define REG_RW_PDIO		0x0C4
#define REG_RW_GPIOIN		0x0D0
#define REG_RW_GPIOEN		0x0D4
#define REG_RW_GPIOOUT		0x0D8
#define REG_RW_PAD_PINMUX1	0x0F4
#define REG_RW_PAD_PINMUX2	0x0F8
#define REG_RW_PAD_PINMUX3	0x0FC
#define REG_RW_PAD_PINMUX4	0x108
#define REG_RW_PAD_PINMUX5	0x10c
#define REG_RW_XTAL_CFG		0x120
#define REG_RW_INTSTA		0x140	//PDWNC INTERRUPT STATUS REGISTER
#define REG_RW_INTEN		0x144	//PDWNC INTERRUPT ENABLE REGISTER
#define REG_RW_INTCLR		0x148	//PDWNC INTERRUPT CLEAR REGISTER
#define REG_RW_EGPIO_SEL	0x32c

/*
 * Tile-specific addresses
 */

// Timer settings
#define REG_RW_TIMER0_LIM_OFFSET	0x148
#define REG_RW_TIMER0_VALUE		0x14C
#define REG_RW_TIMER_CTRL_OFFSET	0x164

#define VAL_T0_AUTOLOAD			0x2
#define VAL_T0_ENABLE			0x1

#define MT85XX_TILE_L220_BASE		0x50002000      /* L220 registers */

#define CKGEN_OFFSET			0x0000
#define BIM_OFFSET			0x8000
#define UART_OFFSET			0xc000
#define PDWNC_OFFSET			0x00024000
#define AUD_OFFSET			0x5000

#define REG_RW_HSMPHE			0x01B4		//Hardware Semaphore Register
#define HSMPHE_UART1			(1U << 0)		//Hardware Semaphore 0
#define HSMPHE_DC_SMPHE			(1U << 1)		//Hardware Semaphore 1 (for Dual Communication Semaphore)
#define HSMPHE_NAND			(1U << 2)		// NAND semaphore
#define HSMPHE_LZHS			(1U << 3)		// LZHS semaphore
#define HSMPHE_SINFO4			(1U << 4)		// SINFO4_REG semaphore
#define HSMPHE_PNG			(1U << 5)		// PNG semaphore
#define HSMPHE_PWR_UP_CFG		(1U << 6)		// POWER UP REG semaphore

// General Purpose Register
#define REG_RW_GPRB0			0x00E0	//RISC Byte General Purpose Register 0
										// boot loader version
#define REG_RW_GPRB1			0x00E4	//RISC Byte General Purpose Register 1
													  // ne upg version
#define REG_RW_GPRB2			0x00E8	//RISC Byte General Purpose Register 2
#define REG_RW_GPRB3			0x00EC	//RISC Byte General Purpose Register 3
#define REG_RW_GPRB4			0x00F0	//RISC Byte General Purpose Register 4
#define REG_RW_GPRB5			0x00F4	//RISC Byte General Purpose Register 5
#define REG_RW_GPRB6			0x00F8	//RISC Byte General Purpose Register 6
#define REG_RW_GPRB7			0x00FC	//RISC Byte General Purpose Register 7
#define REG_RW_GPRDW0			0x0100	//RISC Double Word General Purpose Register 0
#define REG_RW_GPRDW1			0x0104	//RISC Double Word General Purpose Register 1
#define REG_RW_GPRDW2			0x0120	//RISC Double Word General Purpose Register 2
#define REG_RW_GPRDW3			0x0124	//RISC Double Word General Purpose Register 3
#define REG_RW_GPRDW4			0x0110	//RISC Double Word General Purpose Register 4
#define REG_RW_GPRDW5			0x0114	//RISC Double Word General Purpose Register 5
#define REG_RW_GPRDW6			0x0118	//RISC Double Word General Purpose Register 6
#define REG_RW_GPRDW7			0x011C	//RISC Double Word General Purpose Register 7

//#define REG_RW_SINFO4_REG		0x0210	//Dual Core Share Info Register 4

#define FB_MEDIA_PRESENT		(1U << 0)		//[Fastboot]ARM2->ARM1 Detect Media Present
#define FB_ARM2_ALIVE			(1U << 1)		//[Fastboot]ARM2->ARM1 Fastboot Function is Alive
#define FB_ARM1_SATA_INIT		(1U << 2)		//[Fastboot]ARM1->ARM2 SATA Driver Init
#define FB_ARM1_LIRC_INIT		(1U << 3)		//[Fastboot]ARM1->ARM2 LIRC Driver Init
#define FB_ARM1_EJECT_KEY		(1U << 4)		//[Fastboot]ARM1->ARM2 Eject Key is Pressed
#define FB_ARM1_IRRX_INIT		(1U << 5)		//[Fastboot]ARM1->ARM2 IRRX Driver Init
#define FB_ARM1_PINMUX_INIT		(1U << 6)		//[Fastboot]ARM1->ARM2 PINMUX Driver Init
#define FB_ARM2_TRAY_ST_SHIFT		(8)			//[Fastboot]ARM2->ARM1 Tray Status Shift
#define FB_ARM2_TRAY_ST_MASK		(0x0F)		//[Fastboot]ARM2->ARM1 Tray Status Mask

#define IO_VIRT_BASE			(io_virt_base)
//#define IO_VIRT_BASE			({get_io_virt_base();})

#define IO_PHYS_BASE			(0x10000000)
#define MT33XX_VA_UART			(IO_VIRT_BASE + UART_OFFSET)
#define MT33XX_UART0_1_VA		MT33XX_VA_UART	/* UART 0 and 1 */
#define UART_VIRT			MT33XX_VA_UART
#define CKGEN_VIRT			(IO_VIRT_BASE + CKGEN_OFFSET)
#define CKGEN_BASE_VA			CKGEN_VIRT
#define BIM_VIRT			(IO_VIRT_BASE + BIM_OFFSET)
#define AUD_VIRT			(IO_VIRT_BASE + AUD_OFFSET)
#define PDWNC_VIRT			(IO_VIRT_BASE + PDWNC_OFFSET)

#define BIM_BASE_PA			(IO_PHYS_BASE + 0x00008000)
#define BIM_BASE_VA			(IO_VIRT_BASE + 0x00008000)
#define RS232_BASE_PA			(IO_PHYS_BASE + 0x0000C000)
#define RS232_BASE_VA			(IO_VIRT_BASE + 0x0000C000)
//#define CKGEN_BASE_VA			(IO_VIRT_BASE + 0x00000)
#define NFI24_BASE_VA			(IO_VIRT_BASE + 0x1E400)
#define NFIECC24_BASE_VA		(IO_VIRT_BASE + 0x1EC00)
#define SPM_BASE_VA			(IO_VIRT_BASE + 0x48000)


/*
//#define MACRO_HAL_WRITE32
#define HAL_WRITE32(_reg_, _val_)	__raw_writel(_val_, __io(_reg_))

//#define MACRO_HAL_READ32
#define HAL_READ32(_reg_)		__raw_readl(__io(_reg_))

//#define MACRO_IO_READ32
#define IO_READ32(base, offset)		HAL_READ32((base) + (offset))

//#define MACRO_IO_WRITE32
#define IO_WRITE32(base, offset, value)	HAL_WRITE32((base) + (offset), (value))

//#define MACRO_IO_REG32
#define IO_REG32(base, offset)		HAL_READ32((base) + (offset))

//#define MACRO_CKGEN_READ32
//#define CKGEN_READ32(offset)		IO_READ32(CKGEN_VIRT, (offset))

//#define MACRO_CKGEN_WRITE32
//#define CKGEN_WRITE32(offset, value)	IO_WRITE32(CKGEN_VIRT, (offset), (value))

//#define MACRO_CKGEN_MskWrite
//#define CKGEN_MskWrite(offset, value, mask)	CKGEN_WRITE32((offset), ((CKGEN_READ32((offset)) & (~(mask))) | ((value) & (mask))))

//#define MACRO_BIM_READ32
#define BIM_READ32(offset)		IO_READ32(BIM_VIRT, (offset))

//#define MACRO_BIM_WRITE32
#define BIM_WRITE32(offset, value)	IO_WRITE32(BIM_VIRT, (offset), (value))

//#define MACRO_BIM_REG32
#define BIM_REG32(offset)		IO_REG32(BIM_VIRT, (offset))

//#define UART_READ32(offset)		IO_READ32(UART_VIRT, (offset))
//#define UART_WRITE32(offset, value)	IO_WRITE32(UART_VIRT, (offset), (value))

//#define BIM2_READ32(offset)		IO_READ32(BIM2_VIRT, (offset))
//#define BIM2_WRITE32(offset, value)	IO_WRITE32(BIM2_VIRT, (offset), (value))

//#define PDWNC_READ32(offset)		IO_READ32(PDWNC_VIRT, (offset))
//#define PDWNC_WRITE32(offset, value)	IO_WRITE32(PDWNC_VIRT, (offset), (value))

*/

#endif

/*
 * (C) Copyright 2004 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/system.h>




int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	DECLARE_GLOBAL_DATA_PTR;

	IRQ_STACK_START = _armboot_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
       printf("enable icache\r\n");
        icache_enable();
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	unsigned long i;

	disable_interrupts ();

#if 0
#ifdef CONFIG_LCD
	{
		extern void lcd_disable(void);
		extern void lcd_panel_disable(void);

		lcd_disable(); /* proper disable of lcd & panel */
		lcd_panel_disable();
	}
#endif
#endif

	/* turn off I/D-cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	//i &= ~(CR_C | CR_I);
	i &= ~(CR_C | CR_I | CR_M);
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush I/D-cache */
	i = 0;
	//asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (i));  /* invalidate both caches and flush btb */
	asm ("mcr p15, 0, %0, c7, c10, 4": :"r" (i)); /* mem barrier to sync things */
	return(0);
}

//Power Down Watchdog
#define REG_WDT_EN			0x4
#define WDT_EN				0x1
#define WDT_MODE_00 		0x0
#define REG_WDT_COUNTER  	0x8
#define REG_WDT_LIMIT    	0xc
#define WDT_CLOCK        	3000000  //3Mhz
#define IO_UCV_BASE         0xFD000000
#define PDWNC_UCV_BASE		(IO_UCV_BASE + 0x24000)

#define WriteReg32(addr,data)   ((*(unsigned long *)(addr)) = (unsigned long)(data))//volatile
#define PDWC_WRITE32(offset, value)     WriteReg32(PDWNC_UCV_BASE + (offset), (value))

static void boot_reset_cpu()
{
	PDWC_WRITE32(REG_WDT_LIMIT, WDT_CLOCK);
    PDWC_WRITE32(REG_WDT_EN, WDT_EN);
	while(1); 
}
int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern void reset_cpu (ulong addr);

	disable_interrupts ();
	boot_reset_cpu();
	reset_cpu (0);
	/*NOTREACHED*/
	return(0);
}


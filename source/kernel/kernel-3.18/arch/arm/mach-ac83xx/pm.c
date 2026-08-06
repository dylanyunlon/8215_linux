/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <mach/chip_ver.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <mach/ac83xx_basic.h>
#include <linux/syscalls.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <mach/ac83xx_system.h>
#include <asm/cacheflush.h>
#include <linux/slab.h>
#include <mach/quickboot.h>

#define SPM_READ32(REG)            __raw_readl(__io(SPM_BASE_VA + REG))
#define SPM_WRITE32(VAL, REG)      __raw_writel(VAL, __io(SPM_BASE_VA + REG))

static struct quickboot_param qb_param; 

extern void ac83xx_power_off(void);
extern void quickboot_resume(void);
extern void enter_sleep_mode_coreoff(uint32_t *buf);
extern void enter_sleep_mode_coreon(void);
extern void ac83xx_mask_ack_bim_irq(uint32_t irq);
static uint32_t   clk_reg[0x80];
uint32_t   quickboot_suspend_state= 0 ; //0:core off, 1:core on
#define REGDUMP(offset)  printk("0x%x : 0x%x\n",base + offset,HAL_READ32(base+offset));
static void check_topregister(void)
{

	uint32_t base = 0xFD000000;
	uint32_t offset = 0;
	for(offset = 0x9C;offset <= 0xD0;offset+=4)
		REGDUMP(offset);
	return;

}
void save_clkgate(void)
{

   	uint32_t base = 0xFD000000;
	uint32_t offset = 0;
	for(offset = 0x0;offset < 0x200;offset+=4)
		clk_reg[offset/4] = HAL_READ32(base+offset);




}
void restore_clkgate(void)
{

   	uint32_t base = 0xFD000000;
	uint32_t offset = 0;
	for(offset = 0x0;offset < 0x200;offset+=4)
               HAL_WRITE32(base+offset,clk_reg[offset/4]);
    

}
 void (*ac83xx_sram_suspend)(void);
extern void ac83xx_smp_resume(void); 

#ifdef CONFIG_TRUSTY
#include <linux/trusty/smcall.h>
#include <asm/compiler.h>

#define SMC_ARG0		"r0"
#define SMC_ARG1		"r1"
#define SMC_ARG2		"r2"
#define SMC_ARG3		"r3"
#define SMC_ARCH_EXTENSION	".arch_extension sec\n"
#define SMC_REGISTERS_TRASHED	"ip"

static inline ulong smc(ulong r0, ulong r1, ulong r2, ulong r3)
{
#if 1
	register ulong _r0 asm(SMC_ARG0) = r0;
	register ulong _r1 asm(SMC_ARG1) = r1;
	register ulong _r2 asm(SMC_ARG2) = r2;
	register ulong _r3 asm(SMC_ARG3) = r3;

	asm volatile(
		__asmeq("%0", SMC_ARG0)
		__asmeq("%1", SMC_ARG1)
		__asmeq("%2", SMC_ARG2)
		__asmeq("%3", SMC_ARG3)
		__asmeq("%4", SMC_ARG0)
		__asmeq("%5", SMC_ARG1)
		__asmeq("%6", SMC_ARG2)
		__asmeq("%7", SMC_ARG3)
#endif
	SMC_ARCH_EXTENSION
		"smc	#0"	/* switch to secure world */
		: "=r" (_r0), "=r" (_r1), "=r" (_r2), "=r" (_r3)
		: "r" (_r0), "r" (_r1), "r" (_r2), "r" (_r3)
		: SMC_REGISTERS_TRASHED);
	return _r0;
}
#endif
static int ac8317_suspend(void)
{
#ifdef CONFIG_TRUSTY
	smc(SMC_FC_CPU_DORMANT, qb_param.nw_resume_entry, 0, 0);
#endif
	PDWNC_WRITE32(0x160,0x6D617273);
	check_topregister();
	flush_cache_all();
	save_clkgate();
#if     1
       if(quickboot_suspend_state == 0)
	  enter_sleep_mode_coreoff((uint32_t *)virt_to_phys(&qb_param));
       else
         enter_sleep_mode_coreon();
//	restore_clkgate();
//
	HAL_WRITE32(0xFD008164,0x0);
	HAL_WRITE32(0xFD008148,0x6978);
	HAL_WRITE32(0xFD008164,0x3);
	
        SPM_WRITE32(0x02860001, 0);

        //ac83xx_smp_resume();   //bin yang
#else


         ((ac83xx_sram_suspend)(0xFC000020))(0xFC005000,0x0,0x0,0x0);
         
#endif

		return 0;

}
static int ac83xx_pm_valid(suspend_state_t state)
{
	int fgResult;
	fgResult = ((state == PM_SUSPEND_STANDBY) || (state == PM_SUSPEND_MEM));
	printk(KERN_INFO "ac83xx_pm_valid %s (%u) %s\n", \
			(!state)   ?"PM_SUSPEND_ON":\
			(1==state) ?"PM_SUSPEND_STANDBY":\
			(3==state) ?"PM_SUSPEND_MEM":"UNKNOW",
			state,
			fgResult?"support":"unsupport");

	return fgResult;
}

#ifdef CONFIG_TRUSTY	
extern unsigned int trusty_get_resume_entry(void);
#endif
static int ac83xx_pm_begin(suspend_state_t state)
{
	printk(KERN_INFO "ac83xx_pm_begin: Nothing to do!\n");

#ifdef CONFIG_TRUSTY	
	long sw_resume_entry = 0;    
	sw_resume_entry = trusty_get_resume_entry();
	if( (sw_resume_entry<0) || (sw_resume_entry==0))
	{
		printk("ac83xx_pm_begin: cannot get sw_resume_entry\n");
		while (1);
	}
	else
	{
		unsigned int virt_addr = 0;
		
		qb_param.sw_resume_entry = (unsigned int)sw_resume_entry;

		printk("ac83xx_pm_begin: get sw_resume_entry 0x%x\n", qb_param.sw_resume_entry);
		virt_addr = ioremap(0x8, sizeof(unsigned int));
		printk("%s, virt_addr = 0x%x\n",__FUNCTION__, virt_addr);
		*(volatile unsigned int *)virt_addr = sw_resume_entry;
		iounmap(virt_addr);
		dsb();
	}
#endif

	return 0;
}

static int ac83xx_pm_prepare( void)
{
	printk(KERN_INFO "ac83xx_pm_prepare\n");

	return 0;
}

static int ac83xx_pm_prepare_late(void)
{

	printk(KERN_INFO "ac83xx_pm_prepare_late");

	return 0;
}


static int ac83xx_pm_enter(suspend_state_t state)
{
	printk(KERN_INFO "ac83xx_pm_enter %s (%u)\n",\
			(!state)   ?"PM_SUSPEND_ON":\
			(1==state) ?"PM_SUSPEND_STANDBY":\
			(3==state) ?"PM_SUSPEND_MEM":"UNKNOW", state);

	switch(state)
	{
		case PM_SUSPEND_MEM:
			ac8317_suspend();
			break;
		case PM_SUSPEND_STANDBY:
			break;
		case PM_SUSPEND_ON:
			break;
		default:
			break;
	}
	return 0;
}

static void ac83xx_pm_wake(void)
{
	printk(KERN_INFO "ac83xx_pm_wake\n");
}

static void ac83xx_pm_finish(void)
{
	printk(KERN_INFO "ac83xx_pm_finish");

}

static void ac83xx_pm_end(void)
{
	printk(KERN_INFO "ac83xx_pm_end: Nothing to do!\n");
	HAL_WRITE32(0xFD008164,0x0);
        ac83xx_mask_ack_bim_irq(VECTOR_T0);
	HAL_WRITE32(0xFD008148,0x6978);
	HAL_WRITE32(0xFD008164,0x3);

}

static void ac83xx_pm_recover(void)
{
	printk(KERN_INFO "ac83xx_pm_recover: Nothing to do!\n");
}

// begin->*prepare->*prepare_late->*enter->*wake->*finish->end
static struct platform_suspend_ops ac83xx_pm_ops =
{
	.valid        = ac83xx_pm_valid,
	.begin        = ac83xx_pm_begin,
	.prepare      = ac83xx_pm_prepare,
	.prepare_late = ac83xx_pm_prepare_late,
	.enter        = ac83xx_pm_enter,
	.wake         = ac83xx_pm_wake,
	.finish       = ac83xx_pm_finish,
	.end          = ac83xx_pm_end,
	.recover      = ac83xx_pm_recover,
};

static int __init ac83xx_pm_init(void)
{
        unsigned char *buf;
#ifdef CONFIG_PM
	printk(KERN_INFO "ac83xx_pm_init\n");
	suspend_set_ops(&ac83xx_pm_ops);
#endif
        //MOD_VERSION_INFO("POWER", 1, 0, 0);        
        buf =(unsigned char *)kmalloc(16*1024,GFP_ATOMIC);
         
        printk("calibration virtual address:%p phsical address:%p\n", buf, (void *)virt_to_phys(buf));
        qb_param.ddr_cal_addr  = virt_to_phys(buf);
	qb_param.version = 0x1;
	qb_param.nw_resume_entry = virt_to_phys(quickboot_resume);
	qb_param.wakeup_src_gpio = GPIO_WAKEUP_SRC;
	qb_param.wakeup_src_polarity = GPIO_POLARITY_HIGH;
	qb_param.wakeup_sts_gpio = GPIO_WAKEUP_STS;
	qb_param.wakeup_sts_polarity = GPIO_POLARITY_HIGH; 
	printk("ac83xx_pm_init: sw_resume_entry: 0x%x, nw_resume_entry: 0x%x\n",qb_param.sw_resume_entry, qb_param.nw_resume_entry);
        pm_power_off = ac83xx_power_off;
        return 0;
}


late_initcall(ac83xx_pm_init);

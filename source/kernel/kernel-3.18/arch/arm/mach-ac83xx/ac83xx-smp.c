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

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/delay.h>

//#include <asm/hardware/gic.h>
//#include <linux/irqchip/arm-gic.h>

#include "mach/sync_write.h"
#include <mach/hardware.h>
#include <mach/ac83xx_system.h>


#define REG_CORE1_MAGIC  (BIM_BASE_VA+0x114)
#define REG_RW_SLAVE_START  (BIM_BASE_VA+0x110)
#define CORE1_MAGIC_NUM    0x4c48462e

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

static cpumask_t dead_cpus;

extern void gic_send_sgi(unsigned int cpu_id, unsigned int irq);

extern void ac83xx_secondary_startup(void);
void ac83xx_smp_resume(void)
{

 ac83xx_reg_sync_writel(CORE1_MAGIC_NUM, REG_CORE1_MAGIC);
 ac83xx_reg_sync_writel(virt_to_phys(ac83xx_secondary_startup), REG_RW_SLAVE_START);

}


void ac83xx_cpu_powerup(unsigned int cpu)
{
	if(cpu == 1)
	{
		spm_set_power(SPM_MODULE_CPU1,true);
	}
	else if (cpu == 2)
	{
		spm_set_power(SPM_MODULE_CPU2,true);
	}
	else if (cpu == 3)
	{
		spm_set_power(SPM_MODULE_CPU3,true);
	}
}


void ac83xx_cpu_powerdown(unsigned int cpu)
{
	if(cpu == 1)
	{
		spm_set_power(SPM_MODULE_CPU1,false);
	}
	else if (cpu == 2)
	{
		spm_set_power(SPM_MODULE_CPU2,false);
	}
	else if (cpu == 3)
	{
		spm_set_power(SPM_MODULE_CPU3,false);
	}
}

//kernel 3.12
/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen".
 */



/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */


void __cpuinit ac83xx_secondary_init(unsigned int cpu)
{
   printk("%s, cpu = %d\n",__FUNCTION__, cpu);
}

#if 0
int __cpuinit ac83xx_boot_secondary(unsigned int cpu, struct task_struct *idle)
{

		printk("%s, cpu = %d\n",__FUNCTION__, cpu);

    switch(cpu)
    {
        case 1:
        case 2:
        case 3:
            ac83xx_reg_sync_writel(CORE1_MAGIC_NUM, REG_CORE1_MAGIC);
            break;
    }
    //arch_send_wakeup_ipi_mask(cpumask_of(cpu));
    dsb_sev();

    return 0;
}
#endif

void __init ac83xx_smp_init_cpus(void)
{
		printk("%s\n",__FUNCTION__);
}

void __init ac83xx_smp_prepare_cpus(unsigned int max_cpus)
{
		printk("%s\n",__FUNCTION__);
    ac83xx_reg_sync_writel(virt_to_phys(ac83xx_secondary_startup), REG_RW_SLAVE_START);
}


//kernel  3.4.35


/*
 * Setup the set of possible CPUs (via set_cpu_possible)
 */
#if 0
void smp_init_cpus(void)
{
    //set_smp_cross_call(gic_raise_softirq);
}
#endif

/*
 * Perform platform specific initialisation of the specified CPU.
 */
void platform_secondary_init(unsigned int cpu)
{
    /*
     * If any interrupts are already enabled for the primary
     * core (e.g. timer irq), then they will not have been enabled
     * for us: do so
     */
    //gic_secondary_init(0);

		printk("%s, cpu = %d\n",__FUNCTION__, cpu);
}

/*
 * Initialize cpu_possible map, and enable coherency
 */
void platform_smp_prepare_cpus(unsigned int max_cpus)
{
		printk("%s\n",__FUNCTION__);
    ac83xx_reg_sync_writel(virt_to_phys(ac83xx_secondary_startup), REG_RW_SLAVE_START);
}


/*
 * Boot a secondary CPU, and assign it the specified idle task.
 * This also gives us the initial stack to use for this CPU.
 */
#define CONFIG_TRUSTZONE_SUPPORT

 #ifdef CONFIG_TRUSTZONE_SUPPORT
 int __cpuinit ac83xx_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    
    printk("%s, cpu = %d\n",__FUNCTION__, cpu);

	#ifdef CONFIG_HOTPLUG_CPU
	     //spm_set_power(SPM_MODULE_CPU1,true);
		 //	 ac83xx_cpu_powerup(cpu);
	#endif    
#ifdef CONFIG_TRUSTY	
		smc(SMC_FC_CPU_ON, virt_to_phys(ac83xx_secondary_startup), cpu, 0);	
#endif	

	ac83xx_cpu_powerdown(cpu);	//power down, so will start from address 0 when power on

    switch(cpu)
    {
	    case 1:
		case 2:
		case 3:
			//
			// write call back address for secondary cpu from trustzone
			ac83xx_reg_sync_writel(virt_to_phys(ac83xx_secondary_startup), REG_RW_SLAVE_START);
						
	        break;
    }

	ac83xx_reg_sync_writel(CORE1_MAGIC_NUM, REG_CORE1_MAGIC);
	ac83xx_cpu_powerup(cpu);
	printk("[TRUSTZONE], secondary cpu%d power on!\r\n", cpu);

	ac83xx_reg_sync_writel(0x0, BIM_BASE_VA + 0x118);

	//gic_send_sgi(cpu, 0);
    //arch_send_wakeup_ipi_mask(cpumask_of(cpu));
    //gic_raise_softirq(cpumask_of(cpu), 1);
    dsb_sev();

	

    return 0;
}

 #else
 int __cpuinit ac83xx_boot_secondary(unsigned int cpu, struct task_struct *idle)
 {
	 
	 printk("%s, cpu = %d\n",__FUNCTION__, cpu);
 
		#ifdef CONFIG_HOTPLUG_CPU
		  //spm_set_power(SPM_MODULE_CPU1,true);
			  ac83xx_cpu_powerup(cpu);
	  #endif    
 
	 switch(cpu)
	 {
	 case 1:
		 case 2:
		 case 3:
						 ac83xx_reg_sync_writel(CORE1_MAGIC_NUM, REG_CORE1_MAGIC);
			 break;
	 }
	 
		 ac83xx_reg_sync_writel(0x0, BIM_BASE_VA + 0x118);
 
		 gic_send_sgi(cpu, 0);
	 //arch_send_wakeup_ipi_mask(cpumask_of(cpu));
	 //gic_raise_softirq(cpumask_of(cpu), 1);
	 dsb_sev();
 
	 return 0;
 }

 #endif

int platform_cpu_disable(unsigned int cpu)
{
		printk("%s, cpu = %d\n",__FUNCTION__, cpu);
    return 0;
		
}


void platform_cpu_enable(unsigned int cpu)
{
	printk("%s, cpu = %d\n",__FUNCTION__, cpu);
}
int platform_cpu_kill(unsigned int cpu)
{
          uint32_t rval = 0;
		  int k;
	/* this function is running on another CPU than the offline target,
     * here we need wait for shutdown code in platform_cpu_die() to
     * finish before asking SoC-specific code to power off the CPU core.
     */

	/*switch(cpu)
    {
        case 1:
	   #ifdef CONFIG_HOTPLUG_CPU
	     spm_set_power(SPM_MODULE_CPU1,false);
	   #endif
	    
		case 2:
		case 3:
            break;
    }
	*/

	printk("%s, cpu = %d\n",__FUNCTION__, cpu);	

	for (k = 0; k < 1000; k++) 
	{
		if (cpumask_test_cpu(cpu, &dead_cpus))
		{
			 cpumask_clear_cpu(cpu, &dead_cpus);	
			 //spm_set_power(SPM_MODULE_CPU1,false);
			 ac83xx_cpu_powerdown(cpu);	
			 rval = __raw_readl(__io(BIM_BASE_VA + 0x118));
			 dsb();
			 rval |= 0x01;
			 ac83xx_reg_sync_writel(rval, BIM_BASE_VA + 0x118);

			 return 1;
		}

		mdelay(1);
	}

    return 0;
 

}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
 #ifdef CONFIG_TRUSTY	
 extern unsigned int trusty_get_resume_entry(void);
#endif
void platform_cpu_die(unsigned int cpu)
{
    uint32_t rval = 0;

	printk("%s, cpu = %d\n",__FUNCTION__, cpu);

#ifdef CONFIG_TRUSTY	
	long sw_resume_entry = 0;	
	sw_resume_entry = trusty_get_resume_entry();
	if( (sw_resume_entry<0) || (sw_resume_entry==0))
	{
		printk("ac83xx_pm_begin: cannot get sw_resume_entry\n");
		while(1);
	}
	else
	{
		unsigned int virt_addr = 0;
		
		virt_addr = ioremap(0x8, sizeof(unsigned int));
		printk("%s, virt_addr = 0x%x\n",__FUNCTION__, virt_addr);
		*(volatile unsigned int *)virt_addr = sw_resume_entry;
		iounmap(virt_addr);
		dsb();
	}

	smc(SMC_FC_CPU_OFF, 0, cpu, 0);
#endif

    dsb();
    flush_cache_all();

	rval = __raw_readl(__io(BIM_BASE_VA + 0x118));
	dsb();
	rval |= 0x02;
	ac83xx_reg_sync_writel(rval, BIM_BASE_VA + 0x118);

	dsb();
	cpumask_set_cpu(cpu, &dead_cpus);

    /* wait for SoC code in platform_cpu_kill() to shut off CPU core
     * power. CPU bring up starts from the reset vector.
     */
    while (1) {
        asm("wfi"
                :
                :
                : "memory", "cc");
    }
}


struct smp_operations ac83xx_smp_ops __initdata = {
	.smp_prepare_cpus	= ac83xx_smp_prepare_cpus,
	.smp_secondary_init	= ac83xx_secondary_init,
	.smp_boot_secondary	= ac83xx_boot_secondary,
	.smp_init_cpus = ac83xx_smp_init_cpus,

#ifdef CONFIG_HOTPLUG_CPU
	.cpu_kill		= platform_cpu_kill,
	.cpu_die		= platform_cpu_die,
#endif

};

CPU_METHOD_OF_DECLARE(atc_smp_ac83xx, "ac83xx,ac83xx-cpu-method",
			&ac83xx_smp_ops);

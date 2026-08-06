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
#include <linux/of.h>
#include <linux/smp.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <uapi/linux/psci.h>

#include <asm/compiler.h>
#include <asm/cpu_ops.h>
#include <asm/errno.h>
#include <asm/psci.h>
#include <asm/smp_plat.h>
#include <asm/suspend.h>
#include <asm/system_misc.h>
#include <asm/cacheflush.h>

#include "atc_pm.h"

/* Debugging */
#undef TAG
#define TAG	 "[smp] "

#define smp_err(fmt, args...)	   \
	pr_err(TAG"[ERROR]"fmt, ##args)
#define smp_warn(fmt, args...)	  \
	pr_warn(TAG"[WARNING]"fmt, ##args)
#define smp_info(fmt, args...)	  \
	pr_warn(TAG""fmt, ##args)
#define smp_dbg(fmt, args...)	   \
	pr_debug(TAG""fmt, ##args)

static phys_addr_t cpu_release_addr = 0x10008110;

static inline void delay_us(u32 us)
{
	udelay(us);
}
extern unsigned int cluster1_powerup_bitmap;
extern void secondary_holding_pen(void);

int spm_table_cpu_boot(unsigned int cpu)
{
    __le64 __iomem *release_addr;

	if (!cpu_release_addr)
		return -ENODEV;

	/*
	 * The cpu-release-addr may or may not be inside the linear mapping.
	 * As ioremap_cache will either give us a new mapping or reuse the
	 * existing linear mapping, we can use it to cover both cases. In
	 * either case the memory will be MT_NORMAL.
	 */
	release_addr = ioremap_cache(cpu_release_addr,
				     sizeof(*release_addr));
	if (!release_addr)
		return -ENOMEM;

	/*
	 * We write the release address as LE regardless of the native
	 * endianess of the kernel. Therefore, any boot-loaders that
	 * read this address need to convert this address to the
	 * boot-loader's endianess before jumping. This is mandated by
	 * the boot protocol.
	 */
	writeq_relaxed(__pa(secondary_holding_pen), release_addr);
	__flush_dcache_area((__force void *)release_addr,
			    sizeof(*release_addr));
	/*
	* ioremap for spm address, cci address, mcusys address.
	*/
	smp_info("spm_table_cpu_boot start cpu%d\n", cpu);

	if (cpu > 3) {
		/* cluster1 is power up first time. */
			clusterx_poweron(1, 0);
	}

	delay_us(10);
	corex_poweron(cpu);

	/* delay more time to make wakeup core into wfe in preloader */
	delay_us(5000);
	delay_us(5000);
	delay_us(5000);

	/*
	 * Send an event to wake up the secondary CPU.
	 */
	//sev();

	delay_us(5000);

	writeq_relaxed(0xFFFFFFFF, release_addr);
	__flush_dcache_area((__force void *)release_addr,
			    sizeof(*release_addr));

    smp_info("spm_table_cpu_boot end\n");

	return 0;
}


int spm_table_cpu_kill(unsigned int cpu)
{
	int k;

	/* this function is running on another CPU than the offline target,
	 * here we need wait for shutdown code in smp_spin_table_cpu_die()
	 * to finish before asking SoC-specific code to power off the CPU core.
	 */

	for (k = 0; k < 1000; k++) {
		corex_poweroff(cpu);
		delay_us(10);

		if (cpu > 3) {
			cluster1_powerup_bitmap &= (~(1 << cpu));
			/* cluster1 is power down. */
			if (!cluster1_powerup_bitmap)
				clusterx_poweroff(1, 0);
		}

		break;

		mdelay(1);
	}

	if (k >= 100) {
		smp_err("smp_spin_table_cpu_kill, polling dead_cpus is timeout, cpu(%d)", cpu);
		return 0;
	} else
		return 1;
}

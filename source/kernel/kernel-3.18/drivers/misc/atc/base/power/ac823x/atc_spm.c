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
#include <generated/atc_project.h>
#include "atc_pm.h"

#define __io(a)	((void __iomem *)(a))
#define SET_BIT(a, b)			((a) |= ((u32)1L<<(b)))
#define CLR_BIT(a, b)			((a) &= (~((u32)1L<<(b))))


/*
 For AC8237 Base Address
*/
#define SPM_REG_PBASE		    (0x10048000)
#define CCI_REG_PBASE			(0x13090000)
#define MCUSYS_REG_PBASE		(0x10055000)

/* Debugging */
#undef TAG
#define TAG	 "[spm] "

#define spm_err(fmt, args...)	   \
	pr_err(TAG"[ERROR]"fmt, ##args)
#define spm_warn(fmt, args...)	  \
	pr_warn(TAG"[WARNING]"fmt, ##args)
#define spm_info(fmt, args...)	  \
	pr_warn(TAG""fmt, ##args)
#define spm_dbg(fmt, args...)	   \
	pr_debug(TAG""fmt, ##args)


static DEFINE_SPINLOCK(spm_spin_lock);

/*
* cluster1 is power up already if cluster1_powerup_done is non-zero.
*/
unsigned int cluster1_powerup_bitmap = 0;

static void __iomem *spm_reg_vbase;
static void __iomem *cci_reg_vbase;
static void __iomem *mcusys_reg_vbase;



static inline u32 spm_readl(u32 offset)
{
	return __raw_readl(__io(spm_reg_vbase + offset));
}

static inline void spm_writel(u32 regval32, u32 offset)
{
	__raw_writel(regval32, __io(spm_reg_vbase + offset));
}

static u32 spm_reg_read(u32 reg_offset)
{
	u32 val;

	/* enable spm clock */
	spm_writel(0x02860001, 0);

	val = spm_readl(reg_offset);
	spm_writel(0x02860000, 0);

	return val;
}

static void spm_reg_write(u32 reg_offset, u32 val)
{
	/* enable spm clock */
	spm_writel(0x02860001, 0);

	val = val & 0x0000FFFF;
	val = val | (0x0286 << 16);
	spm_writel(val, reg_offset);

	spm_writel(0x02860000, 0);
}

static void spm_clear_onebit(u32 reg_offset, u32 bit_idx)
{
	u32 val;

	val = spm_reg_read(reg_offset);
	CLR_BIT(val, bit_idx);
	spm_reg_write(reg_offset, val);
}

static void spm_set_onebit(u32 reg_offset, u32 bit_idx)
{
	u32 val;

	val = spm_reg_read(reg_offset);
	SET_BIT(val, bit_idx);
	spm_reg_write(reg_offset, val);
}

static void spm_clear(u32 reg_offset, u32 width)
{
	u32 i;

	for (i = 0; i < width; i++) {
		spm_clear_onebit(reg_offset, i);
	}
}

static void spm_set(u32 reg_offset, u32 width)
{
	u32 i;

	for (i = 0; i < width; i++) {
		spm_set_onebit(reg_offset, i);
	}
}

/*
* l2c_from
*	0, Cluster<n> CPUSYS Power On withL2 Cache from Power off.
*	1, Cluster<n> CPUSYS Power On withL2 Cache from Dormant(Sleep).
*/
void clusterx_poweron(u32 cluster_id, u32 l2c_from)
{
	u32 val;
	u32 mpx_cpusys_reg_offset;
	u32 l2c_afifo_bit_offset;
	void __iomem * mpx_cfg0_addr;
	unsigned long flags;

	if (cluster_id == 0) {
		mpx_cpusys_reg_offset = 0xE4;
		l2c_afifo_bit_offset = 0;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x02C;
	}
	else if (cluster_id == 1) {
		mpx_cpusys_reg_offset = 0xE8;
		l2c_afifo_bit_offset = 4;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x22C;
	} else {
		spm_err("clusterx_poweron, cluster id is error.\n");
		return;
	}

	/*
	* Follow Cluster<n> Power on with L2 Cache from Power Off /Dormant sequence in
	* SPM Application Note
	*/
	spm_info("clusterx_poweron(%d) !!!!\n", cluster_id);
	spin_lock_irqsave(&spm_spin_lock, flags);
	/* Set mp<n>_cpusys_top_pwr_rst_en to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 7);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on_2nd to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 8);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Wait until mp<n>_cpusys_top_pwr_ack and mp<n>_cpusys_top_pwr_ack_2nd are high */
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 9)) == 0);
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 10)) == 0);

	/* Set mp<n>_cpusys_top_clamp to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 11);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	if (l2c_from == 0) {
		/* Set mp<n>_cpusys_top_mem_pd to low */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		CLR_BIT(val, 0);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_pd_ack is low */
		while (spm_reg_read(mpx_cpusys_reg_offset) & (1 << 3));

	} else {
		/* Set mp<n>_cpusys_top_mem_slpb to high */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		SET_BIT(val, 1);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_slpb_ack is high */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 4)) == 0);
	}
	/* Wait 1000ns */
	udelay(10);

	/* Set mp<n>_cpusys_top_mem_ckiso to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 2);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_rst_en to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set pwrdnreqn_mp<n>_l2c_afifo to high and wait pwrdnackn_mp<n>_l2c_afifo to high */
	val = spm_reg_read(0xF8);
	SET_BIT(val, l2c_afifo_bit_offset);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 2))) == 0);

	//Set pwrdnreqn_mp<n>_adb to high and wait pwrdnackn_mp<n>_adb to high
	val = spm_reg_read(0xF8);
	SET_BIT(val, l2c_afifo_bit_offset + 1);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 3))) == 0);

	spin_unlock_irqrestore(&spm_spin_lock, flags);
	/*
	* Program CCI400 to enable S4 (for MP0) or S3 (for MP1) interface
	* Snoop Control Registers bit0 to 1 if need snoop feature (0x13095000 for mp0, 0x13094000 for mp1)
	*
	* Bit1 is needed to set also b/c it enables DVM support.
	* SW needs to poll the regster write to make sure updates are already effective.
	*/
	if (cluster_id == 0) {
		val =__raw_readl(__io(cci_reg_vbase + 0x5000));
		val = val | 0x3;
		__raw_writel(val, __io(cci_reg_vbase + 0x5000));
	}
	else {
		val =__raw_readl(__io(cci_reg_vbase + 0x4000));
		val = val | 0x3;
		__raw_writel(val, __io(cci_reg_vbase + 0x4000));
	}

	while (__raw_readl(__io(cci_reg_vbase + 0xC)) & 0x1) {
		udelay(1);
	}

	/* Program MP<n>_AXI_CONFIG acinactm to 0
	* if need snoop feature (0x1005502C for MP0, 0x1005522c for MP1, bit 4)
	*/
	val =__raw_readl(__io(mpx_cfg0_addr));
	CLR_BIT(val, 4);
	__raw_writel(val, __io(mpx_cfg0_addr));

	/* Finish power on and reset sequences */
}

void corex_poweron(u32 core_id)
{
	u32 val;
	u32 mpx_corex_reg_offset;
	unsigned long flags;

	/*
	* core_id and cluster_id mappinng
	*
	*	 core_id		cluster_id	core
	*	0			0			0
	*	1			0			1
	*	2			0			2
	*	3			0			3
	*	4			1			0
	*	5			1			1
	*	6			1			2
	*	7			1			3
	*/

	spm_info("corex_poweron(%d) !!!!\n", core_id);

	if (core_id > 7) {
		spm_err("corex_poweron, core_id(%d) is error.\n", core_id);
		return;
	}
	else
		mpx_corex_reg_offset = 0xC4 + (core_id << 2);
	spin_lock_irqsave(&spm_spin_lock, flags);
	/* Set mpx_cpux_pwr_rst_en to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on_2nd to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_cpux_pwr_ack and mpx_cpux_pwr_ack_2nd are high */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 7)) == 0);
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 8)) == 0);

	/* Set mpx_cpux_clamp/mpx_cpux_pd_slpb_clamp to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 9);
	CLR_BIT(val, 10);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_pd to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 0);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_corex_mem_pd_ack are low */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 2)));

	/* Wait 1000ns for memory power ready (defined in memory model) */
	udelay(1);

	/* Set mpx_corex_mem_ckiso to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 1);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_rst_en to low to finish power on and reset sequences */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* finish power on and reset sequence */
	spin_unlock_irqrestore(&spm_spin_lock, flags);
}

/*
* l2c_into
*	0, Cluster<n> CPUSYS Power On withL2 Cache into Power off.
*	1, Cluster<n> CPUSYS Power On withL2 Cache into Dormant(Sleep).
*/
void clusterx_poweroff(u32 cluster_id, u32 l2c_into)
{
	u32 val;
	u32 mpx_cpusys_reg_offset;
	u32 l2c_afifo_bit_offset;
	void __iomem * mpx_cfg0_addr;
	void __iomem * mpx_misc_cfg_addr;
	unsigned long flags;

	if (cluster_id == 0) {
		mpx_cpusys_reg_offset = 0xE4;
		l2c_afifo_bit_offset = 0;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x02C;
		mpx_misc_cfg_addr = mcusys_reg_vbase + 0x064;
	}
	else if (cluster_id == 1) {
		mpx_cpusys_reg_offset = 0xE8;
		l2c_afifo_bit_offset = 4;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x22C;
		mpx_misc_cfg_addr = mcusys_reg_vbase + 0x264;
	} else {
		spm_err("clusterx_poweroff, cluster id is error.\n");
		return;
	}

	/*
	* Follow Cluster<n> Power off with L2 Cache into Power Off /Dormant sequence in
	* SPM Application Note
	*/

	/* program all cores of ClusterX into standby wait for interrupt or standby wait for event
	* mode. Then follow core power off sequence to power down each core in cluster.
	*/

	/*
	* Program CCI400 to disable S4 (for MP0) or S3 (for MP1) interface
	* Snoop Control Registers bit0 to 0 if need snoop feature (0x13095000 for mp0, 0x13094000 for mp1)
	*
	* Bit1 is needed to clear also b/c it disables DVM support.
	* SW needs to poll the regster write to make sure updates are already effective.
	*/
	if (cluster_id == 0) {
		val =__raw_readl(__io(cci_reg_vbase + 0x5000));
		val = val & (~0x3);
		__raw_writel(val, __io(cci_reg_vbase + 0x5000));
	}
	else {
		val =__raw_readl(__io(cci_reg_vbase + 0x4000));
		val = val & (~0x3);
		__raw_writel(val, __io(cci_reg_vbase + 0x4000));
	}

	while (__raw_readl(__io(cci_reg_vbase + 0xC)) & 0x1) {
		udelay(1);
	}

	/* Program MP<n>_AXI_CONFIG acinactm to 1
	* if need snoop feature (0x1005502C for MP0, 0x1005522c for MP1, bit 4)
	*/
	val =__raw_readl(__io(mpx_cfg0_addr));
	SET_BIT(val, 4);
	__raw_writel(val, __io(mpx_cfg0_addr));

	/* wait mpx_standbywfil2 to high */
	while ((__raw_readl(__io(mpx_misc_cfg_addr)) & (1 << 28)) == 0);

	spm_info("clusterx_poweroff(%d) !!!!\n", cluster_id);

	spin_lock_irqsave(&spm_spin_lock, flags);
	/* Set pwrdnreqn_mp<n>_l2c_afifo to low and wait pwrdnackn_mp<n>_l2c_afifo to low */
	val = spm_reg_read(0xF8);
	CLR_BIT(val, l2c_afifo_bit_offset);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 2))));

	/* Set pwrdnreqn_mp<n>_adb to low and wait pwrdnackn_mp<n>_adb to low */
	val = spm_reg_read(0xF8);
	CLR_BIT(val, l2c_afifo_bit_offset + 1);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 3))));

	/* Set mp<n>_cpusys_top_clamp to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 11);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_mem_ckiso to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 2);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	if (l2c_into == 0) {
		/* Set mp<n>_cpusys_top_mem_pd to high */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		SET_BIT(val, 0);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_pd_ack is high */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 3)) == 0);

	} else {
		/* wait 100ns */
		udelay(1);

		/* Set mp<n>_cpusys_top_mem_slpb to low */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		CLR_BIT(val, 1);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_slpb_ack is low */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 4)));
	}

	/* Set mp<n>_cpusys_top_pwr_rst_en to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 7);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on_2nd to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 8);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Wait until mp<n>_cpusys_top_pwr_ack and mp<n>_cpusys_top_pwr_ack_2nd are low */
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 9)));
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 10)));

	spin_unlock_irqrestore(&spm_spin_lock, flags);
	/* Finish power off sequences */
}

void corex_poweroff(u32 core_id)
{
	u32 val;
	u32 mpx_corex_reg_offset;
	unsigned long flags;

	/*
	* core_id and cluster_id mappinng
	*
	*	 core_id		cluster_id	core
	*	0			0			0
	*	1			0			1
	*	2			0			2
	*	3			0			3
	*	4			1			0
	*	5			1			1
	*	6			1			2
	*	7			1			3
	*/
	spm_info("corex_poweroff(%d) !!!!\n", core_id);

	if (core_id > 7) {
		spm_err(" corex_poweroff,core_id(%d) is error.\n", core_id);
		return;
	}
	else
		mpx_corex_reg_offset = 0xC4 + (core_id << 2);

	/* program corex into standby wait for interrupt or standby wait for event mode. */
	spin_lock_irqsave(&spm_spin_lock, flags);

	/* Set mpx_cpux_clamp/mpx_cpux_pd_slpb_clamp to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 9);
	SET_BIT(val, 10);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_ckiso to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 1);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_pd to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 0);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_rst_en to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 5);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on_2nd to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 6);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_cpux_pwr_ack and mpx_cpux_pwr_ack_2nd are low */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 7)));
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 8)));

	/* the power off sequence is finished. */
	spin_unlock_irqrestore(&spm_spin_lock, flags);
}

static void __spm_power_core(u32 cpu, u32 on_off)
{
	if (on_off) { //power on cpuX
		if (cpu > 3) {
			/* cluster1 is power up first time. */
			if (!cluster1_powerup_bitmap)
				clusterx_poweron(1, 0);
			cluster1_powerup_bitmap |= (1 << cpu);
		}

		udelay(10);
		corex_poweron(cpu);
	} else { //power off cpuX
		corex_poweroff(cpu);
		udelay(10);

		if (cpu > 3) {
			cluster1_powerup_bitmap &= (~(1 << cpu));

			/* cluster1 is power down. */
			if (!cluster1_powerup_bitmap)
				clusterx_poweroff(1, 0);
		}
	}
}


static void __spm_power_g3d(u32 sw_hw_sel, u32 on_off)
{
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&spm_spin_lock, flags);
	if (on_off) { //power on
		/* write mfg_hier_pwr_on(bit2) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 2);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack(bit22) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << 22)));

		udelay(1);

		/* write mfg_hier_pwr_on_s(bit3) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 3);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack_s(bit21) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << 21)));

		/* write mfg_hier_clock_dis(bit4) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 4);
		spm_reg_write(0x08, val);

		if (sw_hw_sel) { //power on with hardware control
			/* write mfg_hier_mem_pd_sel(bit6) = 1
			 * write mfg_hier_mem_pd_hw(bit5) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 5);
			SET_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* wait mfg_mem_pd_ack = 0 */
			while ((spm_reg_read(0x7C) & (1 << 20)));

			/* write mfg1_hier_mem_pd_sel(bit14) = 1
			 * write mfg1_hier_mem_pd_hw(bit13) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 13);
			SET_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* wait mfg1_mem_pd_ack = 0 */
			while ((spm_reg_read(0x7C) & (1 << 16)));

		} else { //power on with software control
			/* write mfg_hier_mem_pd_sel(bit6) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* write mfg_hier_sft_sram_pd_l = 0x0000
			 * write mfg_hier_sft_sram_pd_h = 0x000
			 */
			spm_clear(0x24, 16);
			spm_clear(0x28, 7);

			/* wait mfg_hier_sram_pd == 0x00000000 */
			while (spm_reg_read(0x98) != 0x0);

			/* write mfg1_hier_mem_pd_sel(bit14) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* write mfg1_hier_sft_sram_pd_l = 0x0000
			 */
			spm_clear(0x2C, 9);

			/* wait mfg_hier_pp1_sram_pd == 0x0 */
			while (spm_reg_read(0x9C) != 0x0);

		}

		udelay(1);

		/* write mfg_hier_pwr_iso(bit1) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 1);
		spm_reg_write(0x08, val);

		/* write mfg_hier_pwr_rst_(bit0) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 0);
		spm_reg_write(0x08, val);

		/* wait rg_mfg_mem_pd_ack == 0 */
		while (spm_reg_read(0x24) & (1 << 16));

		/* wait rg_mfg_mem_pd_ack_2nd == 0 */
		while (spm_reg_read(0x28) & (1 << 16));
	} else { //power off

		if (sw_hw_sel) { //power off with hardware control

			/* write mfg_hier_mem_pd_sel(bit6) = 1
			 * write mfg_hier_mem_pd_hw(bit5) = 1
			 */
			val = spm_reg_read(0x08);
			SET_BIT(val, 5);
			SET_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* wait mfg_mem_pd_ack = 1 */
			while (!(spm_reg_read(0x7C) & (1 << 20)));

			/* write mfg1_hier_mem_pd_sel(bit14) = 1
			 * write mfg1_hier_mem_pd_hw(bit13) = 1
			 */
			val = spm_reg_read(0x08);
			SET_BIT(val, 13);
			SET_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* wait mfg1_mem_pd_ack = 1 */
			while (!(spm_reg_read(0x7C) & (1 << 16)));

		} else { //power off with software control
			/* write mfg_hier_mem_pd_sel(bit6) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* write mfg_hier_sft_sram_pd_l = 0xFFFF
			 * write mfg_hier_sft_sram_pd_h(7bits) = 0x007F
			 */
			spm_set(0x24, 16);
			spm_set(0x28, 7);

			/* wait mfg_hier_sram_pd = 0x007FFFFF */
			while(spm_reg_read(0x98) != 0x007FFFFF);

			/* write mfg1_hier_mem_pd_sel(bit14) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* write mfg1_hier_sft_sram_pd_l(9bits) = 0x01FF
			 */
			spm_set(0x2C, 9);

			/* wait mfg_hier_pp1_sram_pd == 1FF(9bits) */
			while (spm_reg_read(0x9C) != 0x01FF);
		}

		/* wait rg_mfg_mem_pd_ack == 1 */
		while (!(spm_reg_read(0x24) & (1 << 16)));

		/* wait rg_mfg_mem_pd_ack_2nd == 1 */
		while (!(spm_reg_read(0x28) & (1 << 16)));

		udelay(1);

		/* write mfg_hier_pwr_iso(bit1/9) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 1);
		spm_reg_write(0x08, val);

		/* write mfg_hier_clock_dis(bit4/12) = 1 */
		/* write mfg_hier_pwr_rst_(bit0/8) = 0 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 4);
		CLR_BIT(val, 0);
		spm_reg_write(0x08, val);

		/* write mfg_hier_pwr_on(bit2/10) = 0 */
		/* write mfg_hier_pwr_on_s(bit3/11) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 2);
		CLR_BIT(val, 3);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack(bit22/18) == 0 */
		while ((spm_reg_read(0x7C) & (1 << 22)));

		/* wait mfg_pwr_on_ack_s(bit21/17) == 0 */
		while ((spm_reg_read(0x7C) & (1 << 21)));

	}

	spin_unlock_irqrestore(&spm_spin_lock, flags);
}

static void __spm_power_vdec(u32 corex, u32 sw_hw_sel, u32 on_off)
{
	u32 val;
	u32 ctrl_bit_offset;
	u32 ack_bit_offset;
	u32 mem_ctrl_offset;
	unsigned long flags;

	if (corex) {
		ctrl_bit_offset = 0;
		ack_bit_offset = 29;
		mem_ctrl_offset = 0x14;
	} else {
		ctrl_bit_offset = 8;
		ack_bit_offset = 25;
		mem_ctrl_offset = 0x1C;
	}

	spin_lock_irqsave(&spm_spin_lock, flags);
	if (on_off) { //power on
		/* write xx_pwr_on(bit10/2) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 2);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack(bit26/30) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << (ack_bit_offset + 1))));

		udelay(1);

		/* write xx_hier_pwr_on_s(bit11/3) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 3);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack_s(bit25/29) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << (ack_bit_offset))));

		/* write xx_hier_clock_dis(bit12/4) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 4);
		spm_reg_write(0x04, val);

		if (sw_hw_sel) { //power on with hardware control
			/* write xx_hier_mem_pd_sel(bit14/6) = 1
			 * write xx_hier_mem_pd_hw(bit13/5) = 0
			 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 5);
			SET_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);
		} else { //power on with software control
			/* write xx_hier_mem_pd_sel(bit14/6) = 0 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

			/* write xx_hier_sft_sram_pd_l = 0x0000
			 * write xx_hier_sft_sram_pd_h = 0x0000
			 */
			spm_clear(mem_ctrl_offset, 16);
			spm_clear(mem_ctrl_offset + 4, 16);
		}

		udelay(1);

		/* write xx_hier_pwr_iso(bit9/1) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 1);
		spm_reg_write(0x04, val);

		/* write mfg_hier_pwr_rst_(bit8/0) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset);
		spm_reg_write(0x04, val);
	} else { //power off

		if (sw_hw_sel) { //power off with hardware control

			/* write xx_hier_mem_pd_sel(bit14/6) = 1
			 * write xx_hier_mem_pd_hw(bit13/5) = 1
			 */
			val = spm_reg_read(0x04);
			SET_BIT(val, ctrl_bit_offset + 5);
			SET_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

		} else { //power off with software control
			/* write xx_hier_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

			/* write xx_hier_sft_sram_pd_l = 0xFFFF
			 * write xx_hier_sft_sram_pd_h = 0xFFFF
			 */
			spm_set(mem_ctrl_offset, 16);
			spm_set(mem_ctrl_offset + 4, 16);
		}

		udelay(1);

		/* write xx_hier_pwr_iso(bit9/1) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 1);
		spm_reg_write(0x04, val);

		/* write xx_hier_clock_dis(bit12/4) = 1 */
		/* write xx_hier_pwr_rst_(bit8/0) = 0 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 4);
		CLR_BIT(val, ctrl_bit_offset);
		spm_reg_write(0x04, val);

		/* write xx_hier_pwr_on(bit10/2) = 0 */
		/* write xx_hier_pwr_on_s(bit11/3) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 2);
		CLR_BIT(val, ctrl_bit_offset + 3);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack(bit26/30) == 0 */
		while ((spm_reg_read(0x7C) & (1 << (ack_bit_offset + 1))));

		/* wait xx_pwr_on_ack_s(bit25/29) == 0 */
		while ((spm_reg_read(0x7C) & (1 << (ack_bit_offset))));

	}
	spin_unlock_irqrestore(&spm_spin_lock, flags);

}

/*
* idx = 0, for arm9
* idx = 1, for msdc
* idx = 2, for usb20
* idx = 3, for ssusb
*/
static void __spm_power_sram(u32 idx, u32 sw_hw_sel, u32 on_off)
{
	u32 val;
	u32 ctrl_bit_offset;
	u32 mem_ctrl_offset;
	u32 pwr_ctrl_offset;
	unsigned long flags;

	if (idx == 0) { // for arm9
		ctrl_bit_offset = 13;
		mem_ctrl_offset = 0x34;
		pwr_ctrl_offset = 0x10;
	} else if (idx == 1) { // for msdc
		ctrl_bit_offset = 5;
		mem_ctrl_offset = 0x38;
		pwr_ctrl_offset = 0x10;
	} else if (idx == 2) { // for usb20
		ctrl_bit_offset = 13;
		mem_ctrl_offset = 0x3C;
		pwr_ctrl_offset = 0x0C;
	} else if (idx == 3) { // for ssusb
		ctrl_bit_offset = 5;
		mem_ctrl_offset = 0x40;
		pwr_ctrl_offset = 0x0C;
	}

	spin_lock_irqsave(&spm_spin_lock, flags);
	if (on_off) { //power on sram

		if (sw_hw_sel) { //power on with hardware control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 1
			 * write xx_mem_pd_hw(bit13/5) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			CLR_BIT(val, ctrl_bit_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);
		} else { // power on with software control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			CLR_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);

			/*
			 * write xx_hier_sft_sram_pd_l = 0x0000
			 */
			spm_clear(mem_ctrl_offset, 16);
		}

		udelay(1);

	} else { //power off sram

		if (sw_hw_sel) { //power off with hardware control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 1
			 * write xx_mem_pd_hw(bit13/5) = 1
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			SET_BIT(val, ctrl_bit_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);
		} else { // power off with software control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);

			/*
			 * write xx_hier_sft_sram_pd_l = 0xFFFF
			 */
			spm_set(mem_ctrl_offset, 16);

			///* wait arm9_sram_pd == 0xFFFF(16bits) */
			//while (SPM_READ32(0xAC) != 0xFFFF);
		}

		udelay(1);
	}

	spin_unlock_irqrestore(&spm_spin_lock, flags);
}

void spm_power_core(u32 cpu, u32 on_off)
{
	//on_off: 0: power off, 1: power on
	__spm_power_core(cpu, on_off);
}
EXPORT_SYMBOL(spm_power_core);

void spm_power_g3d(u32 sw_hw_sel, u32 on_off)
{
	// 0: sw sel, 1: hw sel
	// 0: power off, 1: power on
	u32 regval32;
    /*
    * mfg reset release.
    */
    regval32 = spm_reg_read(0xC8);
    regval32 |= (1 << 31);
    spm_reg_write(regval32, 0xC8);

    /*
    * mfg clock enable
    */
    regval32 = spm_reg_read(0xAC);
    regval32 |= (1 << 31);
    spm_reg_write(regval32, 0xAC);

    __spm_power_g3d(sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_g3d);

void spm_power_vdec(u32 sw_hw_sel, u32 on_off)
{
    //on_off: 0: power off, 1: power on
	__spm_power_vdec(0, sw_hw_sel, on_off);
    __spm_power_vdec(1, sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_vdec);

void spm_power_arm9(u32 sw_hw_sel, u32 on_off)
{
    //on_off: 0: power off, 1: power on
	__spm_power_sram(0, sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_arm9);

void spm_power_msdc(u32 sw_hw_sel, u32 on_off)
{
    //on_off: 0: power off, 1: power on
	__spm_power_sram(1, sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_msdc);

void spm_power_usb20(u32 sw_hw_sel, u32 on_off)
{
    //on_off: 0: power off, 1: power on
	__spm_power_sram(2, sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_usb20);

void spm_power_ssusb(u32 sw_hw_sel, u32 on_off)
{
    //on_off: 0: power off, 1: power on
	__spm_power_sram(3, sw_hw_sel, on_off);
}
EXPORT_SYMBOL(spm_power_ssusb);

void spm_power_all(u32 sw_hw_sel, u32 on_off)
{
	__spm_power_core(1, on_off);
	__spm_power_core(2, on_off);
	__spm_power_core(3, on_off);
	__spm_power_core(4, on_off);
	__spm_power_core(5, on_off);
	__spm_power_core(6, on_off);
	__spm_power_core(7, on_off);

	__spm_power_g3d(sw_hw_sel, on_off);
	__spm_power_vdec(0, sw_hw_sel, on_off);
	__spm_power_vdec(1, sw_hw_sel, on_off);

	__spm_power_sram(0, sw_hw_sel, on_off); //arm9
	__spm_power_sram(1, sw_hw_sel, on_off); //msdc
	__spm_power_sram(2, sw_hw_sel, on_off); //usb20
	__spm_power_sram(3, sw_hw_sel, on_off); //ssusb
}
EXPORT_SYMBOL(spm_power_all);

static int __init atc_spm_init(void)
{
	/*
	* ioremap for spm address, cci address, mcusys address.
	*/

	spm_reg_vbase = ioremap(SPM_REG_PBASE, 0x1000);
	if (!spm_reg_vbase) {
		spm_err("spm_reg_vbase ioremap error.\n");
		return -ENOMEM;
	}

	cci_reg_vbase = ioremap(CCI_REG_PBASE, 0x10000);
	if (!cci_reg_vbase) {
		spm_err("cci_reg_vbase ioremap error.\n");
		return -ENOMEM;
	}

	mcusys_reg_vbase = ioremap(MCUSYS_REG_PBASE, 0x1000);
	if (!mcusys_reg_vbase) {
		spm_err("cci_reg_vbase ioremap error.\n");
		return -ENOMEM;
	}

#ifdef CONFIG_ATC_PRJ_ac823x_adas
    spm_power_vdec(0, 0);
#endif

	return 0;

}

early_initcall(atc_spm_init);


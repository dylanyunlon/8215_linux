/*
 * Copyright 2009 Wolfson Microelectronics plc
 *
 * mt33xx CPUfreq Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/ac83xx.h>
#include <mach/ac83xx_basic.h>


#define SLEEP_FREQ (800 * 1000)

#define EFUSE_REG	0xFD054668
#define HP	(1<<13)
//#define SYS_ARMPLL			850000  //KHz
#define SYS_ARMPLL_1G			1001000  //KHz
static unsigned int cpufreq_cur = SYS_ARMPLL_1G;

#define ARMCLK_1G_1d1		(SYS_ARMPLL_1G)
#define ARMCLK_1G_3d4		((SYS_ARMPLL_1G*3)/4)
#define ARMCLK_1G_1d2		(SYS_ARMPLL_1G/2)
#define ARMCLK_1G_1d4		(SYS_ARMPLL_1G/4)
#define ARMCLK_1G_4d5		((SYS_ARMPLL_1G*4)/5)
#define ARMCLK_1G_3d5		((SYS_ARMPLL_1G*3)/5)
#define ARMCLK_1G_2d5		((SYS_ARMPLL_1G*2)/5)
#define ARMCLK_1G_1d5		(SYS_ARMPLL_1G/5)
#define ARMCLK_1G_5d6		((SYS_ARMPLL_1G*5)/6)
#define ARMCLK_1G_2d3		((SYS_ARMPLL_1G*2)/3)
#define ARMCLK_1G_1d3		(SYS_ARMPLL_1G/3)
#define ARMCLK_1G_1d6		(SYS_ARMPLL_1G/6)


#define SYS_ARMPLL_1G2			1188000  //KHz
#define ARMCLK_1G2_1d1		(SYS_ARMPLL_1G2)
#define ARMCLK_1G2_3d4		((SYS_ARMPLL_1G2*3)/4)
#define ARMCLK_1G2_1d2		(SYS_ARMPLL_1G2/2)
#define ARMCLK_1G2_1d4		(SYS_ARMPLL_1G2/4)
#define ARMCLK_1G2_4d5		((SYS_ARMPLL_1G2*4)/5)
#define ARMCLK_1G2_3d5		((SYS_ARMPLL_1G2*3)/5)
#define ARMCLK_1G2_2d5		((SYS_ARMPLL_1G2*2)/5)
#define ARMCLK_1G2_1d5		(SYS_ARMPLL_1G2/5)
#define ARMCLK_1G2_5d6		((SYS_ARMPLL_1G2*5)/6)
#define ARMCLK_1G2_2d3		((SYS_ARMPLL_1G2*2)/3)
#define ARMCLK_1G2_1d3		(SYS_ARMPLL_1G2/3)
#define ARMCLK_1G2_1d6		(SYS_ARMPLL_1G2/6)
#define ARMCLK_CKDIV_TOG	(1<<25)
#define ARMCLK_CKDIV_SEL_MASK	(0x1F << 20)
#define ARMCLK_CKDIV_SEL_SET(v)	((v) << 20)

static struct cpufreq_frequency_table mt33xx_1G_freq_table[] = {
	{ 0, 8,  ARMCLK_1G_1d1 },
	{ 0, 9,  ARMCLK_1G_3d4 },
	{ 0, 10,  ARMCLK_1G_1d2 },
	{ 0, 11,  ARMCLK_1G_1d4 },
	{ 0, 17,  ARMCLK_1G_4d5 },
	{ 0, 18,  ARMCLK_1G_3d5 },
	{ 0, 19,  ARMCLK_1G_2d5 },
	{ 0, 20,  ARMCLK_1G_1d5 },
	{ 0, 25,  ARMCLK_1G_5d6 },
	{ 0, 26,  ARMCLK_1G_2d3 },
	{ 0, 28,  ARMCLK_1G_1d3 },
	{ 0, 29,  ARMCLK_1G_1d6 },
	{ 0, 0, CPUFREQ_TABLE_END },
};

static struct cpufreq_frequency_table mt33xx_freq_table[20]={{0}};


static struct cpufreq_frequency_table mt33xx_1G2_freq_table[] = {
	{ 0, 8,  ARMCLK_1G2_1d1 },
	{ 0, 9,  ARMCLK_1G2_3d4 },
	{ 0, 10,  ARMCLK_1G2_1d2 },
	{ 0, 11,  ARMCLK_1G2_1d4 },
	{ 0, 17,  ARMCLK_1G2_4d5 },
	{ 0, 18,  ARMCLK_1G2_3d5 },
	{ 0, 19,  ARMCLK_1G2_2d5 },
	{ 0, 20,  ARMCLK_1G2_1d5 },
	{ 0, 25,  ARMCLK_1G2_5d6 },
	{ 0, 26,  ARMCLK_1G2_2d3 },
	{ 0, 28,  ARMCLK_1G2_1d3 },
	{ 0, 29,  ARMCLK_1G2_1d6 },
	{ 0, 0,  CPUFREQ_TABLE_END },
};

static struct freq_attr *mt33xx_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};


static int mt33xx_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	if (policy->cpu != 0)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, mt33xx_freq_table);
}

static unsigned int mt33xx_cpufreq_get_speed(unsigned int cpu)
{
	if (cpu != 0)
		return 0;

	//return clk_get_rate(armclk) / 1000;
	return cpufreq_cur;
}

static int mt33xx_cpufreq_set_target(struct cpufreq_policy *policy,
				      unsigned int index)
{
	unsigned int i = index;
	unsigned int regV;

	
	regV = __bim_readl(0x448);
	regV = (regV & (~ARMCLK_CKDIV_SEL_MASK));
	regV = regV | ARMCLK_CKDIV_SEL_SET(mt33xx_freq_table[i].flags);
	__bim_writel(regV,0x448);

	if(regV & ARMCLK_CKDIV_TOG)
	{
		regV &= ~ARMCLK_CKDIV_TOG;
	}
	else
	{
		regV |= ARMCLK_CKDIV_TOG;
	}
	__bim_writel(regV,0x448);

	return 0;
}



static int mt33xx_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret;
	//struct cpufreq_frequency_table *freq;

	/*
	* Cjie Sun: fix CNB0019188. 
	* Antutu.apk only show two cpu freq, others are sleep.
	*/
#if 0
	if (policy->cpu != 0)
		return -EINVAL;
#endif

	if ((IO_READ32(EFUSE_REG,0) & HP)) {
		pr_err("not hp efuse=%x\n", IO_READ32(EFUSE_REG,0));
		cpufreq_cur = SYS_ARMPLL_1G;
		memcpy(mt33xx_freq_table, mt33xx_1G_freq_table, sizeof(mt33xx_1G_freq_table));
	}else {
		pr_err("hp efuse=%x\n", IO_READ32(EFUSE_REG,0));
		cpufreq_cur = SYS_ARMPLL_1G2;
		memcpy(mt33xx_freq_table, mt33xx_1G2_freq_table, sizeof(mt33xx_1G2_freq_table));
	}
	if (mt33xx_freq_table == NULL) {
		pr_err("No frequency information for this CPU\n");
		return -ENODEV;
	}
	
        policy->suspend_freq = SLEEP_FREQ;
        ret = cpufreq_generic_init(policy, mt33xx_freq_table, 0);


	return ret;
}

static struct cpufreq_driver mt33xx_cpufreq_driver = {
	.flags          = 0,
	.verify		= mt33xx_cpufreq_verify_speed,
	.target_index   = mt33xx_cpufreq_set_target,
	.get		= mt33xx_cpufreq_get_speed,
	.init		= mt33xx_cpufreq_driver_init,
	.name		= "mt33xx",
	.attr		= mt33xx_cpufreq_attr,
};

static int __init mt33xx_cpufreq_init(void)
{
	return cpufreq_register_driver(&mt33xx_cpufreq_driver);
}
module_init(mt33xx_cpufreq_init);
MODULE_LICENSE("GPL");

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



#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/io.h>

/* Debugging */
#undef TAG
#define TAG	 "[cpufreq] "

#define cpufreq_err(fmt, args...)	   \
	pr_err(TAG"[ERROR]"fmt, ##args)
#define cpufreq_warn(fmt, args...)	  \
	pr_warn(TAG"[WARNING]"fmt, ##args)
#define cpufreq_info(fmt, args...)	  \
	pr_warn(TAG""fmt, ##args)
#define cpufreq_dbg(fmt, args...)	   \
	pr_debug(TAG""fmt, ##args)


#define SLEEP_FREQ (800 * 1000)
//#define EFUSE_REG	0xFD054668
//static unsigned int cpufreq_cur = SYS_ARMPLL_1G;

static struct cpufreq_frequency_table ac823x_freq_table[] = {
    { 0, 0x8,   1024},
    { 0, 0x19,  853 },
    { 0, 0x12,  614 },
    { 0, 0x9,   768 },
    { 0, 0xa,   512 },
    { 0, 0x13,  409 },
    { 0, 0x1c,  341 },
    { 0, 0xb,   256 },
    { 0, 0x14,  204 },
    { 0, 0x1d,  170 },
    { 0, 0, CPUFREQ_TABLE_END },
};

static struct freq_attr *ac823x_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};


static DEFINE_SPINLOCK(atc_cpufreq_lock);


extern u32 bim_get_cluster_freq(u32 cluster_id);
extern u32 bim_set_cluster_freq(u32 cluster_id, u32 drvdata);

static int ac823x_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	if (policy->cpu != 0)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, ac823x_freq_table);
}

static unsigned int ac823x_cpufreq_get_speed(unsigned int cpu)
{
    u32 cluster_id = 0, drvdata = 0, cluster_freq = 0;
	int i = 0;

    cpufreq_info("we will get core%d cpufreq from bim drv \n", cpu);	
	BUG_ON(cpu>7);
	
    if(cpu>3)
        cluster_id = 1;
	
	drvdata = bim_get_cluster_freq(cluster_id);

	for(i=0; i<sizeof(ac823x_freq_table)/sizeof(struct cpufreq_frequency_table); i++) {
		if(drvdata == ac823x_freq_table[i].driver_data) {
			cluster_freq = ac823x_freq_table[i].frequency;
			break;
		}
	}

	if(i == sizeof(ac823x_freq_table)/sizeof(struct cpufreq_frequency_table))
		cpufreq_err("wrong temperature \n");
	
	return cluster_freq;
}

static int ac823x_cpufreq_set_target(struct cpufreq_policy *policy,
				      unsigned int index)
{
    u32 cluster_id = 0, drvdata = 0;
	int ret = 0;

	cpufreq_info("we will set cluster cpufreq form bim drv \n");
	cpufreq_info("dump policy info \n");
	cpufreq_info("index: %d. cpu: %d. max: %d. min: %d \n. cur: %d \n",
		index,
		policy->cpu,
		policy->max,
		policy->min,
		policy->cur
		);
	
    BUG_ON(policy->cpu>7);

	if(policy->cpu>3)
		cluster_id = 1;

    drvdata = policy->freq_table[index].driver_data;

	ret = bim_set_cluster_freq(cluster_id, drvdata);
	if(ret) {
		cpufreq_err("cpufreq set error!! \n");
		return 1;
	}

	return 0;
}



static int ac823x_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret;

    policy->suspend_freq = SLEEP_FREQ;
    ret = cpufreq_generic_init(policy, ac823x_freq_table, 0);

	return ret;
}

static struct cpufreq_driver ac823x_cpufreq_driver = {
	.flags          = 0,
	.verify		= ac823x_cpufreq_verify_speed,
	.target_index   = ac823x_cpufreq_set_target,
	.get		= ac823x_cpufreq_get_speed,
	.init		= ac823x_cpufreq_driver_init,
	.name		= "ac823x",
	.attr		= ac823x_cpufreq_attr,
};

static int __init ac823x_cpufreq_init(void)
{
    cpufreq_info("All kthread of monitor and loading is created done \n");
	return cpufreq_register_driver(&ac823x_cpufreq_driver);
}
module_init(ac823x_cpufreq_init);
MODULE_LICENSE("GPL");

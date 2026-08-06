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
#include <linux/kernel_stat.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>


/* Debugging */
#undef TAG
#define TAG	 "[cputest] "

#define loading_err(fmt, args...)	   \
	pr_err(TAG"[ERROR]"fmt, ##args)
#define loading_warn(fmt, args...)	  \
	pr_warn(TAG"[WARNING]"fmt, ##args)
#define loading_info(fmt, args...)	  \
	pr_warn(TAG""fmt, ##args)
#define loading_dbg(fmt, args...)	   \
	pr_debug(TAG""fmt, ##args)

#define CPU_NRM  4

struct cpu_loading_data {
    u32 loading[CPU_NRM]; // the loading of  perCPU
    u32 loadavg;
	u32 samplerate;
    u32 weight[CPU_NRM];
	u32 ratio;

	u32 k_loading_flag;
    struct task_struct *k_loading; //

	u32 k_press_flag[CPU_NRM];
    struct task_struct *k_press[CPU_NRM];
	
	struct kobject kobj;
};

struct cpu_loading_attr {
	struct attribute attr;
	ssize_t (*show)(struct cpu_loading_data *, char *);
	ssize_t (*store)(struct cpu_loading_data *, const char *, size_t count);
};

static DECLARE_RWSEM(cpuloading_rwsem);

#define to_loading_data(k) container_of(k, struct cpu_loading_data, kobj)
#define to_loading_attr(a) container_of(a, struct cpu_loading_attr, attr)

#define cpu_loading_attr_rw(_name)		\
              static struct cpu_loading_attr _name =			\
              __ATTR(_name, 0644, show_##_name, store_##_name)

#define cpu_loading_attr_ro(_name)		\
              static struct cpu_loading_attr _name =			\
              __ATTR(_name, 0444, show_##_name, NULL)


#define per_cpu_loading_attr_rw(_name, cpu) \
	cpu_loading_attr_rw(_name##cpu)


static struct cpu_loading_data *cpu_loading_data;
static ssize_t show_loading(struct kobject *kobj, struct attribute *attr, char *buf, size_t size)
{
    struct cpu_loading_data *cld = to_loading_data(kobj);
	struct cpu_loading_attr *cla = to_loading_attr(attr);
	ssize_t ret;

	if (!down_read_trylock(&cpuloading_rwsem))
		return -EINVAL;	

    if(cla->show)
		ret = cla->show(cld,buf);
	else
		ret = -EIO;

	up_read(&cpuloading_rwsem);
	return ret;
}

static ssize_t store_loading(struct kobject *kobj, struct attribute *attr, const char *buf, size_t size)
{
    struct cpu_loading_data *cld = to_loading_data(kobj);
	struct cpu_loading_attr *cla = to_loading_attr(attr);
    ssize_t ret;

    down_write(&cpuloading_rwsem);

    if(cla->store)
		ret = cla->store(cld, buf, size);
	else
		ret = -EIO;

	up_write(&cpuloading_rwsem);
	
    return ret;
}


const struct sysfs_ops loading_ops = {
	.show = show_loading,
	.store = store_loading,
};

static void loading_release(struct kobject *kobj)
{
    struct cpu_loading_data *cld = to_loading_data(kobj);
	kfree(cld);
}

static struct kobj_type loading_ktype = {
	.release = loading_release,
	.sysfs_ops = &loading_ops,
};


static ssize_t show_loading_monitor(struct cpu_loading_data *cld, char *buf)
{	
    return sprintf(buf, "%d\n", cld->k_loading_flag);
}

static void report_cpuloading_thread(void *p) ;

static ssize_t store_loading_monitor(struct cpu_loading_data *cld, const char *buf, size_t size)
{
    int ret;
	u32 cmd;
    ret = sscanf(buf, "%u", &cmd);
	if(ret != 1)
		return -EINVAL;

	loading_info("recive data from user: %d \n", cmd);

    if(cmd == 1) {
        if(cld->k_loading_flag == 0) {
            cld->k_loading = kthread_run(report_cpuloading_thread, (void*)cld, "cpuloading_report");
            if(IS_ERR(cld->k_loading)) {
                loading_err("Create cpu loading thread failed \n");
            }
            else {
                loading_info("Create cpu loading thread success \n");
                cld->k_loading_flag = 1;
            }
        }
        else {
            loading_info("Monitor thread has already created \n");
        }
    }

    return ret?ret:size;
}


static unsigned int loop_process_stack(char *buf)
{
    struct task_struct *p = current;
    struct vm_area_struct *vm = NULL;
	unsigned int cnt = 0;
	const char *name = NULL;

	printk("pid: %d, name: %s kernel stack: %08lx  ", p->pid, p->comm, p->stack);
    if(p->mm) {
		vm = p->mm->mmap;
		printk("user stack: %08lx heap: brk - %08lx, start-brk - %08lx\n",  p->mm->start_stack, p->mm->brk, p->mm->start_brk);
    } else {
        printk("\n");
    }

	if(vm) {
		for(vm; vm != NULL;) {
			if(vm->vm_start <= p->mm->start_stack && vm->vm_end >= p->mm->start_stack) {
				name = arch_vma_name(vm);
			    printk("modulename: %s  %08lx-%08lx  [stack]\n", name, vm->vm_start, vm->vm_end);
				cnt = sprintf(buf, "%08lx-%08lx  [stack]\n", vm->vm_start, vm->vm_end);
			}
			else if(vm->vm_start >= p->mm->start_stack && vm->vm_end >= p->mm->start_stack) {
				printk("case2 happy ~~~~~~~~~~~~~~~~~~~~~~~~ \n");
			}
			vm=vm->vm_next;
		}		
	}

#if 0
	for_each_process(p) {
		printk("pid: %d, name: %s kernel stack: %16lx  ", p->pid, p->comm, p->stack);
		if(p->mm) {
			printk("user stack : %16lx \n",  p->mm->start_stack);
			vm = p->mm->mmap;
		}
		else {
			printk("\n");
		}
		if(vm) {
			for(vm; vm != NULL;) {
				if(vm->vm_start <= p->mm->start_stack && vm->vm_end >= p->mm->start_stack) {
				    printk("%08lx-%08lx  [stack] \n", vm->vm_start, vm->vm_end);
					cnt = sprintf(buf, "%08lx-%08lx  [stack] \n", vm->vm_start, vm->vm_end);
				}
				vm=vm->vm_next;
			}
		}
	}
#endif
	return cnt;
}

static ssize_t show_sample_rate(struct cpu_loading_data *cld, char *buf)
{
    
	return loop_process_stack(buf);
    //return sprintf(buf, "%d\n", cld->samplerate);
}

static ssize_t store_sample_rate(struct cpu_loading_data *cld, const char *buf, size_t size)
{
    int ret;

    ret = sscanf(buf, "%u", &cld->samplerate);

	loading_info("set samplerate as %u \n", cld->samplerate);
	
	return size;
}

static ssize_t show_load_detail(struct cpu_loading_data *cld, char *buf)
{
    int i = 0, ret = 0;

    i = num_online_cpus();

	switch(i) {
		case 1:
			ret = sprintf(buf, "%d \n", cld->loading[0]);
			break;
		case 2:
			ret = sprintf(buf, "%d\t %d\t %d\n", cld->loading[0], cld->loading[1], cld->loadavg);
			break;
		case 3:
			ret = sprintf(buf, "%d\t %d\t %d\t %d \n", cld->loading[0], cld->loading[1], cld->loading[2], cld->loadavg);
			break;
		case 4:
			ret = sprintf(buf, "%d\t %d\t %d\t %d\t %d \n", cld->loading[0], cld->loading[1], cld->loading[2], cld->loading[3], cld->loadavg);
			break;
		default:
			break;
	}
	return ret;
}


cpu_loading_attr_rw(loading_monitor);
cpu_loading_attr_rw(sample_rate);
cpu_loading_attr_ro(load_detail);

struct attribute *loading_monitor_attr[] = {
	&loading_monitor.attr,
	&sample_rate.attr,
	&load_detail.attr,
	NULL,	
};

static ssize_t show_loading_thread(struct cpu_loading_data *cld, char *buf, int cpu)
{
    return sprintf(buf, "cpu%d: %d\n", cpu, cld->k_press_flag[cpu]);
}

static void cpu_press_thread(void *p) ;
static ssize_t store_loading_thread(struct cpu_loading_data *cld, const char *buf, size_t size, int cpu)
{
    int ret, cmd = 0;
    char thread_name[32];

	ret = sscanf(buf, "%u", &cmd);

    memset(thread_name, 0, sizeof(thread_name));
	
    sprintf(thread_name, "cpu%d_press", cpu);

	if(cmd == 1) {
        if(cld->k_press_flag[cpu] == 0) {
            cld->k_press[cpu] = kthread_run(cpu_press_thread, (void*)&cpu, thread_name);
            if(IS_ERR(cld->k_press[cpu])) {
                loading_err("Create cpu%d press thread failed \n", cpu);
            }
            else {
                loading_info("Create cpu%d press thread %s success \n", cpu, thread_name);
				msleep(100);
                cld->k_press_flag[cpu] = 1;
            }
        }
        else {
            loading_info("cpu%d press thread has already created \n", cpu);
        }

	}
    return size;
}

static ssize_t show_loading_weight(struct cpu_loading_data *cld, char *buf, int cpu)
{
    return sprintf(buf, "cpu%d: %d\n", cpu, cld->weight[cpu]);
}

static ssize_t store_loading_weight(struct cpu_loading_data *cld, const char *buf, size_t size, int cpu)
{
    int ret;

	ret =  sscanf(buf, "%u", &cld->weight[cpu]);

	loading_info("debug for test weight: %d \n", cld->weight[cpu]);
    return size;
}

#define per_cpu_show(_name,n) \
static ssize_t show_##_name##n \
(struct cpu_loading_data *cld, char *buf) \
{  return show_##_name(cld, buf, n); \
}

#define per_cpu_store(_name,n) \
static ssize_t store_##_name##n \
(struct cpu_loading_data *cld, const char *buf, size_t size) \
{ return store_##_name(cld, buf, size, n); \
}

per_cpu_show(loading_thread,0);
per_cpu_store(loading_thread,0);
per_cpu_loading_attr_rw(loading_thread,0);

per_cpu_show(loading_thread,1);
per_cpu_store(loading_thread,1);
per_cpu_loading_attr_rw(loading_thread,1);


per_cpu_show(loading_thread,2);
per_cpu_store(loading_thread,2);
per_cpu_loading_attr_rw(loading_thread,2);

per_cpu_show(loading_thread,3);
per_cpu_store(loading_thread,3);
per_cpu_loading_attr_rw(loading_thread,3);

per_cpu_show(loading_weight,0);
per_cpu_store(loading_weight,0);
per_cpu_loading_attr_rw(loading_weight,0);

per_cpu_show(loading_weight,1);
per_cpu_store(loading_weight,1);
per_cpu_loading_attr_rw(loading_weight,1);

per_cpu_show(loading_weight,2);
per_cpu_store(loading_weight,2);
per_cpu_loading_attr_rw(loading_weight,2);

per_cpu_show(loading_weight,3);
per_cpu_store(loading_weight,3);
per_cpu_loading_attr_rw(loading_weight,3);


static struct attribute *cpu0_sub_attr[] = {
	&loading_thread0.attr,
	&loading_weight0.attr,
	NULL,
};

static struct attribute *cpu1_sub_attr[] = {
	&loading_thread1.attr,
	&loading_weight1.attr,
	NULL,
};

static struct attribute *cpu2_sub_attr[] = {
	&loading_thread2.attr,
	&loading_weight2.attr,
	NULL,
};

static struct attribute *cpu3_sub_attr[] = {
	&loading_thread3.attr,
	&loading_weight3.attr,
	NULL,
};

static struct attribute_group cpu_test_groups[] = {
    {
        .name = "cpu0",
        .attrs = cpu0_sub_attr,
    },
    
    {
        .name = "cpu1",
        .attrs = cpu1_sub_attr,
    },
    
    {
        .name = "cpu2",
        .attrs = cpu2_sub_attr,
    },
    
    {
        .name = "cpu3",
        .attrs = cpu3_sub_attr,
    },

	NULL,
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline u64 get_cpu_idle_time_from_jiffy(unsigned int cpu, u64 *wall)
{
	u64 idle_time;
	u64 cur_wall_time;
	u64 busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	busy_time = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];

	idle_time = cur_wall_time - busy_time;
	if (wall)
		*wall = cputime_to_usecs(cur_wall_time);

	return cputime_to_usecs(idle_time);
}


static void report_cpuloading_thread(void *p) 
{
	u32 cpu=0;
	static u64 pre_idle_time[4] = {0}, pre_wall_time[4] = {0}, pre_busy_time[4] = {0};

    struct cpu_loading_data *cld = (struct cpu_loading_data *)p;

	if(cld->samplerate == 0)
		cld->samplerate = 2;  // default 2s sample rate
	
	while(1) {
		u32 loadsum = 0;
		for_each_online_cpu(cpu) {
			u32 cur_wall_time, cur_idle_time;
			u64 idle_time, wall_time;
			u32 load;
			
			idle_time = get_cpu_idle_time_from_jiffy(cpu, &wall_time);
			
			cur_wall_time = wall_time - pre_wall_time[cpu];
			pre_wall_time[cpu] = wall_time;
			cur_idle_time = idle_time - pre_idle_time[cpu];
			pre_idle_time[cpu] = idle_time;
			
			load = ((cur_wall_time - cur_idle_time)*100) / cur_wall_time;

			cld->loading[cpu] = load;
			loadsum += load;
		}

        cld->loadavg = loadsum/4;
		
		msleep(cld->samplerate * 1000);

	}
	return ;
}


static void cpu_press_thread(void *p) 
{
	int i = 0;
	volatile int temp = 0;
	struct cpumask mask;
    int cpu = *(int *)p;

    loading_info("create press thread for cpu%d \n", cpu);	
	
	cpumask_clear( &mask );
	cpumask_set_cpu(cpu, &mask);

	if( sched_setaffinity(0, &mask ) == -1)
	{
		loading_err("sched_setaffinity 0 failed.\n");
	}

	while(1) {
		for(i=0; i<(cpu_loading_data->ratio*cpu_loading_data->weight[cpu]); i++) {
			temp = 0x55aa;
			barrier();
		}
		msleep(1);
	}
	
	return ;
}


static int __init cpu_loading_sysfs_init(void)
{
    int ret;
	u32 cpu = 0;
	
    cpu_loading_data = kmalloc(sizeof(struct cpu_loading_data), GFP_KERNEL);
	if(!cpu_loading_data) 
		return -ENOMEM;

	memset(cpu_loading_data, 0, sizeof(struct cpu_loading_data));
	memset(&cpu_loading_data->kobj, 0, sizeof(struct kobject));

    //&cpu_loading_data->kobj = kobject_create();
    cpu_loading_data->samplerate = 2;  // 2s as defalut;
    cpu_loading_data->ratio = 1024*512; // default ratio
    cpu_loading_data->weight[0] = 1; //default weight
	cpu_loading_data->weight[1] = 1;
	cpu_loading_data->weight[2] = 1;
	cpu_loading_data->weight[3] = 1;
	
    kobject_init_and_add(&cpu_loading_data->kobj, &loading_ktype, &cpu_subsys.dev_root->kobj, "cputest");

	ret = sysfs_create_files(&cpu_loading_data->kobj, (struct attribute **)&loading_monitor_attr);
	if(ret) {
		kobject_put(&cpu_loading_data->kobj);
		return ret;
	}

    for_each_online_cpu(cpu) {
		ret = sysfs_create_group(&cpu_loading_data->kobj, &cpu_test_groups[cpu]);
		if(ret) {
			kobject_put(&cpu_loading_data->kobj);
			loading_err("Create cpu%d groups failed \n", cpu);
			return ret;
		}
    }

	return 0;
}

static void __exit cpu_loading_sysfs_exit(void)
{
    kobject_del(&cpu_loading_data->kobj);
	kobject_put(&cpu_loading_data->kobj);

    kfree(cpu_loading_data);
	cpu_loading_data = NULL;

	return;
}


module_init(cpu_loading_sysfs_init);

MODULE_AUTHOR("Ke Xu<ke.xu@autochips.com>");
MODULE_DESCRIPTION("'cpu loading' - A driver to test cpu and export sysfs to user space");
MODULE_LICENSE("GPL");


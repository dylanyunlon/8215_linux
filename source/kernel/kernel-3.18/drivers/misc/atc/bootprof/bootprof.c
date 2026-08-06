#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <linux/utsname.h>
#include <linux/types.h>

#include <asm/uaccess.h>
#include <linux/moduleparam.h>
#include <generated/atc_project.h>

#ifdef NAND_SUPPORT
extern u32 nand_w_tt, nand_r_tt, nand_w_pgc, nand_r_pgc; 
#endif
#define SEQ_printf(m, x...)	    \
 do {			    \
    if (m)		    \
	seq_printf(m, x);	\
    else		    \
	printk(x);	    \
 } while (0)


#define MTSCHED_DEBUG_ENTRY(name) \
static int mt_##name##_open(struct inode *inode, struct file *file) \
{ \
    return single_open(file, mt_##name##_show, inode->i_private); \
} \
\
static const struct file_operations mt_##name##_fops = { \
    .open = mt_##name##_open, \
    .write = mt_##name##_write,\
    .read = seq_read, \
    .llseek = seq_lseek, \
    .release = single_release, \
}

#define BOOT_STR_SIZE 128
#define BOOT_LOG_NUM 64
struct boot_log_struct{
    u32 timestamp;
#ifdef NAND_SUPPORT
    u32 nand_r_pgc;
    u32 nand_r_tt;
    u32 nand_w_pgc;
    u32 nand_w_tt;
#endif
    char event[BOOT_STR_SIZE];
}mt_bootprof[BOOT_LOG_NUM];
int boot_log_count = 0;

static DEFINE_MUTEX(mt_bootprof_lock);
static int mt_bootprof_enabled = 0;
static int bootprof_pl_t = 0;
static int bootprof_lk_t = 0;

module_param_named(pl_t, bootprof_pl_t, int, S_IRUGO | S_IWUSR);
module_param_named(lk_t, bootprof_lk_t, int, S_IRUGO | S_IWUSR);

#if defined(CONFIG_ATC_PLATFORM_ac83xx)
#include "mach/ac83xx.h"
static u32 boot_time_ms(void)
{
	volatile u32 time = 0;
	/***
	* Register FD00814C, which was triggered by BootROM, 
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - IO_READ32(0xFD000000, 0x814C)) / 27000;
	return time;
}
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
static u32 boot_time_ms(void)
{
        void __iomem *io_vbase;
	volatile u32 time = 0;

	io_vbase = ioremap(0x10008000, 0x1000);
	if (!io_vbase) {
		pr_err("[boot_time_ms]: ioremap io_base error.\n");
		return -ENOMEM;
	}
	//time = (0xFFFFFFFF - __raw_readl((io_vbase+0x728))) / 27000;
        time = (__raw_readl((io_vbase+0x728))) / 27000;

        iounmap(0x10008000);
	return time;
}
#endif

void log_boot(char *str)
{
    u32 ts = 0;

    if( 0 == mt_bootprof_enabled)
	    return;
	
    ts = boot_time_ms();
	pr_debug("BOOTPROF:%10d : %s\n", ts, str);    

	if(boot_log_count >= BOOT_LOG_NUM){
	    pr_warn("[BOOTPROF] not enuough bootprof buffer\n");
	    return;
    }
    mutex_lock(&mt_bootprof_lock);
    mt_bootprof[boot_log_count].timestamp = ts;
    strcpy((char*)&mt_bootprof[boot_log_count].event, str);
#ifdef NAND_SUPPORT
    mt_bootprof[boot_log_count].nand_r_pgc = nand_r_pgc;
    mt_bootprof[boot_log_count].nand_w_pgc = nand_w_pgc;
    mt_bootprof[boot_log_count].nand_r_tt = nand_r_tt;
    mt_bootprof[boot_log_count].nand_w_tt = nand_w_tt;
#endif
    boot_log_count++;
    mutex_unlock(&mt_bootprof_lock);
}

EXPORT_SYMBOL(log_boot);
//extern void (*set_intact_mode)(void);
static void mt_bootprof_switch(int on)
{
    mutex_lock(&mt_bootprof_lock);
    if (mt_bootprof_enabled ^ on) {
    	if (on) {
    	    mt_bootprof_enabled = 1;
    	} else {
    	    mt_bootprof_enabled = 0;
    		/*
    	    if(set_intact_mode != NULL){
    		(*set_intact_mode)();
    		printk("set_intact_mode:0x%x\n", (unsigned int)set_intact_mode);
    	    }
    	    */
    	}
    }
    mutex_unlock(&mt_bootprof_lock);

}

static ssize_t mt_bootprof_write(struct file *filp, const char *ubuf,
	   size_t cnt, loff_t *data)
{
    char buf[BOOT_STR_SIZE];
    size_t copy_size = cnt;

    if (cnt >= sizeof(buf))
	copy_size = BOOT_STR_SIZE - 1;

    if (copy_from_user(&buf, ubuf, copy_size))
	return -EFAULT;

    if(cnt==1){
	if(buf[0] == '0')
	    mt_bootprof_switch(0);
	else if(buf[0] == '1')
	    mt_bootprof_switch(1);
    }

    buf[copy_size] = 0;
    log_boot(buf);

    return cnt;

}
static int mt_bootprof_show(struct seq_file *m, void *v)
{
    int i;
    SEQ_printf(m, "----------------------------------------\n");
    SEQ_printf(m, "%d	    BOOT PROF (unit:msec)\n", mt_bootprof_enabled);
    SEQ_printf(m, "----------------------------------------\n");

	SEQ_printf(m, "%10d : %s\n", bootprof_pl_t, "preloader");
	SEQ_printf(m, "%10d : %s\n", bootprof_lk_t, "uboot/lk");
	
	for(i = 0 ;i< boot_log_count;i ++){
#ifdef NAND_SUPPORT
		SEQ_printf(m, "%10d : %s <r:%d %d> <w:%d %d>\n", mt_bootprof[i].timestamp, mt_bootprof[i].event, 
                    mt_bootprof[i].nand_r_pgc, mt_bootprof[i].nand_r_tt, mt_bootprof[i].nand_w_pgc, mt_bootprof[i].nand_w_tt);
#else
		SEQ_printf(m, "%10d : %s\n", mt_bootprof[i].timestamp, mt_bootprof[i].event);
#endif
	}
    SEQ_printf(m, "----------------------------------------\n");
    return 0;
}
/*** Seq operation of mtprof ****/
//MTSCHED_DEBUG_ENTRY(bootprof); 
static int mt_bootprof_open(struct inode *inode, struct file *file) 
{ 
    return single_open(file, mt_bootprof_show, inode->i_private); 
} 

static const struct file_operations mt_bootprof_fops = { 
    .open = mt_bootprof_open, 
    .write = mt_bootprof_write,
    .read = seq_read, 
    .llseek = seq_lseek, 
    .release = single_release, 
};
static int __init init_boot_prof(void)
{
    struct proc_dir_entry *pe;

    pe = proc_create("bootprof", 0666, NULL, &mt_bootprof_fops);
    if (!pe)
	return -ENOMEM;
   // set_intact_mode = NULL;
    mt_bootprof_switch(1);
    return 0;
}
__initcall(init_boot_prof);

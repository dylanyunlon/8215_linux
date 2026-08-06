#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/autoconf.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/delay.h>  //Add for msleep
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <mach/power_loss_test.h>

#include <mach/ac83xx_gpio_pinmux.h>

#ifdef CONFIG_AC83XX_HD_CUT_ENABLE
#include <linux/gpio.h>
#endif

#define TAG                 "[MVG_TEST]:"
//#define PWR_LOSS_MT6575
//#define PWR_LOSS_MT6573
#define PWR_LOSS_MT6571

/* use software reset to do power loss test,
 * if not defined means to use hardware(ATE)
 * reset */
 //#define PWR_LOSS_RANDOM_SW_RESET (1)

//static struct wake_lock	spoh_wake_lock;

/* CONFIG_** macro will defined in linux .config file */
#ifdef CONFIG_PWR_LOSS_ATC_DEBUG
#define PWR_LOSS_DEBUG
#endif

#define PWR_LOSS_DEVNAME            "power_loss_test"  //name in /proc/devices & sysfs
#define PWR_LOSS_FIRST_MINOR         0
#define PWR_LOSS_MAX_MINOR_COUNT     1
#define PWR_LOSS_CBUF_LEN            128

#ifndef HZ
#define HZ	100
#endif


#define PWR_LOSS_SLEEP_MAX_TIME      (msecs_to_jiffies(60000))
#define PWR_LOSS_SLEEP_TIME          (msecs_to_jiffies(60000))    //60second

#ifdef WDT_NON_RST_REG2
int mvg_get_rgu_counter(void)
{
    int result = 0;
    return result;
}
int mvg_set_rgu_counter(int val)
{
    return val;
}
#else // Simulate by software
int mvg_get_rgu_counter(void)
{
    return mvg_get_counter();
}
int mvg_set_rgu_counter(int val)
{
    mvg_set_counter(val);
    return -1;
}
#endif

static dev_t sg_pwr_loss_devno;
static struct cdev*   sg_pwr_loss_dev       = NULL;
static struct class*  sg_pwr_loss_dev_class = NULL;
static struct device* sg_pwr_loss_dev_file  = NULL;

static wdt_reboot_info power_loss_info;
static char cmd_buf[256];

int pwr_loss_open(struct inode *inode, struct file *file)
{
#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test: Open operation !\n", TAG);
#endif
    return 0;
}

int pwr_loss_release(struct inode *inode, struct file *file)
{
#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test: Release operation !\n", TAG);
#endif
    return 0;
}

#if 1
	#define REG_RW_RESRV1 0x160
	#define REG_RW_WDT 0x008
	#define REG_RW_WDTSET 0x004
#endif

#ifdef CONFIG_AC83XX_WDT_ENABLE
void wdt_arch_reset(void)
{
	int u4Test;

	PDWNC_WRITE32(REG_RW_RESRV1, 0x33633363);
	PDWNC_WRITE32(REG_RW_WDT, 0xffffff00);
	for(u4Test = 0; u4Test < 10000; u4Test++)
	{
	}
	PDWNC_WRITE32(REG_RW_WDTSET, 1);
	while(1);
}
#else
void wdt_arch_reset(void)
{
	#ifdef CONFIG_ATC_PLATFORM
	printk(KERN_ERR "%s Power Loss Test: watchdog reset is not enabled in %s\n", TAG, CONFIG_ATC_PLATFORM);
	#else
	printk(KERN_ERR "%s Power Loss Test: watchdog reset is not enabled!\n", TAG);
	#endif
	
	return;
}
#endif

#ifdef CONFIG_AC83XX_HD_CUT_ENABLE
extern int syscheck_gpio_reserve(int pin, int dir, const char *name);

void hardware_reset(void)
{
	gpio_set_value(PIN_136_TS_D5, 1);
        printk(KERN_ERR "%s Power Loss Test: MCU reboot, GPIO has been set!\n", TAG);
//	syscheck_gpio_reserve(PIN_136_TS_D5, 0, "syscheck_gpio");
	PL_DELAY(MVG_CFG_VCC_DROP_TIME);
	return;
}
#else
void hardware_reset(void)
{
	#ifdef CONFIG_ATC_PLATFORM
		printk(KERN_ERR "%s Power Loss Test: hardware reset not enabled in %s\n", TAG, CONFIG_ATC_PLATFORM);
	#else
		printk(KERN_ERR "%s Power Loss Test: both hardware reset is not enabled!\n", TAG);
	#endif

	PL_DELAY(MVG_CFG_VCC_DROP_TIME);
	
	return;
}
#endif

extern void get_random_bytes(void *buf, int nbytes);

int pwr_loss_random_reset_thread(void *p)
{
    signed long ret = 0;
    int HZ_val = HZ;
    struct timespec current_time;
    long sec_time = 0;
    long nsec_time = 0;
    signed long sleep_time = 0;

    get_random_bytes(&sleep_time, sizeof(signed long));
    if (sleep_time < 0){
        sleep_time &= 0x7fffffff;
	}
    sleep_time %= PWR_LOSS_SLEEP_MAX_TIME;
#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test: sleep time =%d\n", TAG, sleep_time);
#endif

    while (1){
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, sleep_time);
		#endif
        set_current_state(TASK_UNINTERRUPTIBLE);
		if(sleep_time > 0){
        	ret = schedule_timeout(sleep_time);
		}
        down_read(&power_loss_info.rwsem);
        if(power_loss_info.wdt_reboot_support == WDT_REBOOT_OFF) {
            up_read(&power_loss_info.rwsem);
            msleep(1000);
			#ifdef PWR_LOSS_DEBUG
            printk(KERN_NOTICE "%s Power Loss Test: wdt reboot pause...!\n", TAG);
			#endif
            continue;
        }
        up_read(&power_loss_info.rwsem);
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: ret = %d, do reset now...\n", TAG, ret);
		#endif

		if (mvg_get_method()==MVG_METHOD_HW_CUT){
			hardware_reset();
    	}
		else if(mvg_get_method()==MVG_METHOD_SW_WDT){
			wdt_arch_reset();
		}
		else{
			printk(KERN_ERR "%s Power Loss Test: reset failed because reset method is not set!!!\n", TAG, ret);
		}

        while(1);
    }
}
//#else
int pwr_loss_reset_thread(void *p)
{
    signed long ret = 0;
    signed long l_val1 = 0;
    signed long l_val2 = 0;
    signed long l_count = 0;

	int sleep_time = (int) PWR_LOSS_SLEEP_TIME;
#ifdef PWR_LOSS_DEBUG
	printk(KERN_NOTICE "%s Power Loss Test: sleep time =%d\n", TAG, sleep_time);
#endif

    while (1){
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, sleep_time);
		#endif
        set_current_state(TASK_UNINTERRUPTIBLE);
        ret = schedule_timeout(PWR_LOSS_SLEEP_TIME);
        down_read(&power_loss_info.rwsem);
        if(power_loss_info.wdt_reboot_support == WDT_REBOOT_OFF) {
            up_read(&power_loss_info.rwsem);
			#ifdef PWR_LOSS_DEBUG
            printk(KERN_NOTICE "%s Power Loss Test: wdt reboot pause...!\n", TAG);
			#endif
            msleep(1000);
            continue;
        }
        up_read(&power_loss_info.rwsem);
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: ret = %d, do reset now...\n", TAG, ret);
		#endif

		if (mvg_get_method()==MVG_METHOD_HW_CUT){
			hardware_reset();
    	}
		else if(mvg_get_method()==MVG_METHOD_SW_WDT){
			wdt_arch_reset();
		}
		else{
			printk(KERN_ERR "%s Power Loss Test: reset failed because reset method is not set!!!\n", TAG, ret);
		}

        while(1);
    }
}

static int initiative_pwr_loss_reset_thread(void *p)
{
    signed long ret = 0;
    signed long l_val1 = 0;
    signed long l_val2 = 0;
    signed long l_count = 0;
	int sleep_time = 0;

	#ifdef CONFIG_AC83XX_RANDOM_PWR_CUT_INITIATIVE
	    get_random_bytes(&sleep_time, sizeof(int));

		if (sleep_time < 0){
    	    sleep_time &= 0x7fffffff;
		}
    	sleep_time %= PWR_LOSS_SLEEP_MAX_TIME;
	#else
		sleep_time = PWR_LOSS_SLEEP_TIME;
	#endif

#ifdef PWR_LOSS_DEBUG
	printk(KERN_NOTICE "%s Power Loss Test: sleep time =%d\n", TAG, sleep_time);
#endif

    while (1){
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: wait for initiative reset in %d mseconds...!\n", TAG, sleep_time);
		#endif
        set_current_state(TASK_UNINTERRUPTIBLE);
        ret = schedule_timeout(PWR_LOSS_SLEEP_TIME);
        down_read(&power_loss_info.rwsem);
        if(power_loss_info.wdt_reboot_support == WDT_REBOOT_OFF) {
            up_read(&power_loss_info.rwsem);
			#ifdef PWR_LOSS_DEBUG
            printk(KERN_NOTICE "%s Power Loss Test: wdt reboot pause...!\n", TAG);
			#endif
            msleep(1000);
            continue;
        }
        up_read(&power_loss_info.rwsem);
		#ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s Power Loss Test: ret = %d, do reset now...\n", TAG, ret);
		#endif

		#ifdef CONFIG_AC83XX_SW_PWR_CUT_INITIATIVE
			wdt_arch_reset();
		#else
			hardware_reset();
		#endif

        while(1);
    }
}

//#endif /* end of PWR_LOSS_RANDOM_SW_RESET */
//#endif /* end of PWR_LOSS_SW_RESET */

long pwr_loss_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    char l_buf[PWR_LOSS_CBUF_LEN] = {0};
    struct mvg_addr addr_entry;

    switch (cmd){
        case PRINT_REBOOT_TIMES:
            ret  = copy_from_user((int *)l_buf, (int *)arg, sizeof(int));
            if (ret != 0){
                printk(KERN_ERR "%s Power Loss Test: PRINT_REBOOT_TIMES cmd copy from user space failed!\n", TAG);
            }
            printk(KERN_ERR "%s Power Loss Test: -----------System Reboot Successfully Times= %d---------------!\n", TAG, ((int *)l_buf)[0]);
            break;
        case PRINT_DATA_COMPARE_ERR:
            printk(KERN_ERR "%s Power Loss Test: -----------Data Compare Error---------------!\n", TAG);
            break;
        case PRINT_FILE_OPERATION_ERR:
            printk(KERN_ERR "%s Power Loss Test: -----------File Operation Error---------------!\n", TAG);
            break;
        case PRINT_GENERAL_INFO:
            ret  = copy_from_user(l_buf,(int *)arg,(sizeof(l_buf[0])*(sizeof(l_buf))));
            if (ret != 0){
                printk(KERN_ERR "%s Power Loss Test: PRINT_GENERAL_INFO cmd copy from user space failed! \n", TAG);
            }

            l_buf[(sizeof(l_buf[0])*(sizeof(l_buf)))-1] = '\0';

            #ifdef PWR_LOSS_DEBUG
                printk(KERN_NOTICE "%s %s", TAG, l_buf);
            #endif
            break;
        case PRINT_RAW_RW_INFO:
            ret  = copy_from_user(l_buf,(int *)arg,(sizeof(l_buf[0])*(sizeof(l_buf))));
            if (ret != 0){
                printk(KERN_ERR "%s Power Loss Test: PRINT_RAW_RW_INFO cmd copy from user space failed! \n", TAG);
            }

            l_buf[(sizeof(l_buf[0])*(sizeof(l_buf)))-1] = '\0';


            #ifdef PWR_LOSS_DEBUG
                printk(KERN_NOTICE "%s %s\n", TAG, l_buf);
            #endif
            break;

        /*--------------------------------------------------------------------*/
        /* SPOH MVG power-loss ioctl items */
        /*--------------------------------------------------------------------*/
        case MVG_WDT_CONTROL:
            ret=(int)arg;
            switch(ret) {
                case MVG_WDT_DISABLE:
                case MVG_WDT_ENABLE:
                    mvg_set_wdt(arg);
                    break;
                case MVG_WDT_RESET_NOW:
                    mvg_wdt_reset();
                default:
                    break;
            }
            break;

		case MVG_ENABLE_SW_PWR_LOSS_CONFIG:
			mvg_set_trigger(MVG_TRIGGER_ALWAYS);
            mvg_set_wdt(MVG_WDT_ENABLE);
			mvg_set_method(MVG_METHOD_SW_WDT);
			power_loss_info.wdt_reboot_support = WDT_REBOOT_ON;
            break;

		case MVG_ENABLE_HW_PWR_LOSS_CONFIG:
			mvg_set_trigger(MVG_TRIGGER_ALWAYS);
			mvg_set_method(MVG_METHOD_HW_CUT);
			power_loss_info.wdt_reboot_support = WDT_REBOOT_ON;
			break;

		case MVG_COUNTER_SET:
            mvg_set_rgu_counter(arg);
            break;

		case MVG_DELAYED_PWR_RESET:
			mvg_delay_pwr_reset(arg);
			break;
			
		case MVG_RANDOM_DELAYED_PWR_RESET:
			mvg_random_delay_pwr_reset(arg);
			break;
			
        case MVG_COUNTER_GET:
            ret = mvg_get_rgu_counter();
            break;

        case MVG_ADDR_RANGE_ADD:
            ret  = copy_from_user(&addr_entry, (int *)arg, sizeof(struct mvg_addr));
            if (ret != 0){
                printk(KERN_ERR "%s: MVG_ADDR_RANGE_ADD cmd copy address from user space failed!\n", TAG);
                break;
            }
            mvg_addr_range_add(addr_entry.base, addr_entry.end);
            break;

        case MVG_ADDR_RANGE_DELETE:
            mvg_addr_range_del((u64)arg);
            break;

        case MVG_ADDR_RANGE_CHECK:
            ret=mvg_addr_range_check((u64)arg);
            break;

        case MVG_ADDR_RANGE_CLEAR:
            mvg_addr_range_clear();
            break;

        case MVG_SET_CURR_CASE_NAME:
            ret  = copy_from_user(l_buf,(int *)arg,(sizeof(l_buf[0])*(sizeof(l_buf))));
            if (ret != 0){
                printk(KERN_ERR "%s:MVG_SET_CURR_CASE_NAME cmd copy case name from user space failed!\n", TAG);
                break;
            }
            l_buf[(sizeof(l_buf[0])*(sizeof(l_buf)))-1] = '\0';
            mvg_set_curr_case_name(l_buf);
            break;

        case MVG_SET_CURR_GROUP_NAME:
            ret  = copy_from_user(l_buf,(int *)arg,(sizeof(l_buf[0])*(sizeof(l_buf))));
            if (ret != 0){
                printk(KERN_ERR "%s:MVG_SET_CURR_CASE_NAME cmd copy group name from user space failed!\n", TAG);
                break;
            }
            l_buf[(sizeof(l_buf[0])*(sizeof(l_buf)))-1] = '\0';
            mvg_set_curr_group_name(l_buf);
            break;

        case MVG_SET_TRIGGER:
            mvg_set_trigger(arg);
            break;

        case MVG_METHOD_SET:
            mvg_set_method(arg);
            break;

        case MVG_METHOD_GET:
            ret = mvg_get_method();
            break;

		case MVG_DELAYED_PWR_RESET_BACKGROUND:
			kthread_run(pwr_loss_reset_thread, NULL, "power_loss_thread");    //CLONE_KERNEL
			break;

		case MVG_RANDOM_DELAYED_PWR_RESET_BACKGROUND:
			kthread_run(pwr_loss_random_reset_thread, NULL, "power_loss_thread");    //CLONE_KERNEL
			break;

#if 1//#ifdef MTK_EMMC_SUPPORT
        case MVG_EMMC_TIME_DIVISOR_SET:
            mvg_emmc_set_reset_time_divisor(&power_loss_info, arg);
            break;

        case MVG_EMMC_TIME_DIVISOR_GET:
            ret = mvg_emmc_get_reset_time_divisor(&power_loss_info);
            break;

        case MVG_EMMC_RESET_MODE_SET:
            mvg_emmc_reset_mode_set(&power_loss_info, arg);
            break;

        case MVG_EMMC_RESET_TIME_MODE_SET:
            mvg_emmc_reset_time_mode_set(&power_loss_info, arg);
            break;

        case MVG_EMMC_SET_ERASE_GROUP_SIZE:
            mvg_emmc_set_erase_group_sector(&power_loss_info, arg);
            break;

        case MVG_EMMC_GET_DELAY_RESULT:
            ret = mvg_emmc_get_delay_result(&power_loss_info);
            break;

        case MVG_EMMC_SET_DELAY_TABLE:
            ret = mvg_emmc_get_set_delay_table(&power_loss_info, (unsigned char *)arg);
            break;
#endif

        default:
            ret=-1;
            printk(KERN_ERR "%s Power Loss Test: cmd code Error! %d\n", TAG, cmd);
            break;
    }

    return ret;
}

static int power_loss_info_init(void)
{
    memset(&power_loss_info, 0, sizeof(wdt_reboot_info));

    init_rwsem(&power_loss_info.rwsem);
    down_write(&power_loss_info.rwsem);

    INIT_LIST_HEAD(&power_loss_info.addr_list);

    #ifdef CONFIG_PWR_LOSS_ATC_SPOH
    power_loss_info.pl_counter = -1;
    power_loss_info.wdt_reboot_support = WDT_REBOOT_ON;
    #else
    power_loss_info.wdt_reboot_support = WDT_REBOOT_ON;
    #endif

    #if 1//#ifdef MTK_EMMC_SUPPORT
    power_loss_info.drv_priv=(void *)&mvg_spoh_emmc_priv;
    #endif

    up_write(&power_loss_info.rwsem);

    return 0;
}

static int power_loss_debug_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
    int ret;
    int wdt_reboot_support;

    if (count == 0) return -1;
    if (count > 255) count = 255;

    ret = copy_from_user(cmd_buf, buffer, count);
    if (ret < 0) return -1;

    cmd_buf[count] = '\0';

    if (1 == sscanf(cmd_buf, "%x", &wdt_reboot_support)) {
        if(wdt_reboot_support < 0){
            printk(KERN_ERR "%s [%s] : command format is error, please help to check!!!\n", TAG, __func__);
            return -1;
        }
        else {
            down_write(&power_loss_info.rwsem);
            power_loss_info.wdt_reboot_support = (wdt_reboot_support == WDT_REBOOT_OFF) ? WDT_REBOOT_OFF : WDT_REBOOT_ON;
            up_write(&power_loss_info.rwsem);
			#ifdef PWR_LOSS_DEBUG
            	printk(KERN_NOTICE "%s [****PWR_LOSS_DEBUG****]\n", TAG);
            	printk(KERN_NOTICE "%s WDT REBOOT\t", TAG);
				if(wdt_reboot_support == WDT_REBOOT_ON)
	            {
    	            printk(KERN_ERR "%s ON\n", TAG);
        	    }
            	else
                	printk(KERN_ERR "%s OFF\n", TAG);
			#endif
            
        }
    }else {
        printk(KERN_ERR "%s [%s] : command format is error, please help to check!!!\n", TAG, __func__);
        return -1;
    }

    return count;
}

static int power_loss_debug_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;
    int wdt_reboot_support = 0;

    down_read(&power_loss_info.rwsem);
    wdt_reboot_support = power_loss_info.wdt_reboot_support;
    up_read(&power_loss_info.rwsem);
    len = sprintf(page, "WDT REBOOT STATUS\t%d\n", wdt_reboot_support);

    *eof = 1;

    return len;
}

static const struct file_operations power_loss_debug_proc_fops = { 
    .write = power_loss_debug_write,
	.read = power_loss_debug_read,
};

static int power_loss_debug_init(void)
{
    struct proc_dir_entry *power_loss_debug;

    power_loss_debug = proc_create("power_loss_debug", 0660, 0, &power_loss_debug_proc_fops);

    if(power_loss_debug){
        #ifdef PWR_LOSS_DEBUG
        printk(KERN_NOTICE "%s [%s]: Successfully create /proc/power_loss_debug\n", TAG, __func__);
		#endif
    }else{
        printk(KERN_ERR "%s [%s]: Failed to create /proc/power_loss_debug\n", TAG, __func__);
        return -1;
    }

    return 0;
}

static struct file_operations pwr_loss_fops =
{
    .owner = THIS_MODULE,
    .open = pwr_loss_open,
    .release = pwr_loss_release,
    .unlocked_ioctl	= pwr_loss_ioctl,
};

static int __init power_loss_init(void)
{
    int err;
#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test Module Init\n", TAG);
#endif

    err = alloc_chrdev_region(&sg_pwr_loss_devno, PWR_LOSS_FIRST_MINOR, PWR_LOSS_MAX_MINOR_COUNT, PWR_LOSS_DEVNAME);
    if (err != 0){
        printk(KERN_ERR "%s Power Loss Test: alloc_chardev_region Failed!\n", TAG);
        return err;
    }

#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test: MAJOR =%d, MINOR=%d\n", TAG,
                      MAJOR(sg_pwr_loss_devno), MINOR(sg_pwr_loss_devno));
#endif

    sg_pwr_loss_dev = cdev_alloc();
    if (NULL == sg_pwr_loss_dev){
        printk(KERN_ERR "%s Power Loss Test: cdev_alloc Failed\n", TAG);
        goto out2;
    }

    sg_pwr_loss_dev->owner = THIS_MODULE;
    sg_pwr_loss_dev->ops   = &pwr_loss_fops;

    err = cdev_add(sg_pwr_loss_dev, sg_pwr_loss_devno, 1);
    if (err != 0){
        printk(KERN_ERR "%s Power Loss Test: cdev_add Failed!\n", TAG);
        goto out2;
    }

    sg_pwr_loss_dev_class = class_create(THIS_MODULE, PWR_LOSS_DEVNAME);
    if (NULL == sg_pwr_loss_dev_class){
        printk(KERN_ERR "%s Power Loss Test: class_create Failed!\n", TAG);
        goto out1;
    }

    sg_pwr_loss_dev_file = device_create(sg_pwr_loss_dev_class, NULL, sg_pwr_loss_devno, NULL, PWR_LOSS_DEVNAME);
    if (NULL == sg_pwr_loss_dev_file){
        printk(KERN_ERR "%s Power Loss Test: device_create Failed!\n", TAG);
        goto out;
    }

//#ifdef PWR_LOSS_SPOH
//    wake_lock_init(&spoh_wake_lock, WAKE_LOCK_SUSPEND, "spoh");
//    wake_lock(&spoh_wake_lock);
//#endif

    power_loss_info_init();
    err = power_loss_debug_init();
    if(err  < 0)
        goto out;

#ifdef CONFIG_AC83XX_PWR_CUT_INITIATIVE
	kthread_run(initiative_pwr_loss_reset_thread, NULL, "power_loss_thread");    //CLONE_KERNEL
	
	#ifdef PWR_LOSS_DEBUG
	printk(KERN_NOTICE "%s Power Loss Test: kernel thread create Successful!\n", TAG);
	#endif
#endif

    return 0;
out:
    class_destroy(sg_pwr_loss_dev_class);
out1:
    cdev_del(sg_pwr_loss_dev);
out2:
    unregister_chrdev_region(sg_pwr_loss_devno, PWR_LOSS_MAX_MINOR_COUNT);
//#ifdef PWR_LOSS_PROC
    remove_proc_entry("power_loss_debug", NULL);
//#endif

    return err;
}

static void __exit power_loss_exit(void)
{
#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test: Module Exit\n", TAG);
#endif

//#ifdef PWR_LOSS_SPOH
//    wake_lock_destroy(&spoh_wake_lock);
//#endif

    unregister_chrdev_region(sg_pwr_loss_devno, PWR_LOSS_MAX_MINOR_COUNT);
    cdev_del(sg_pwr_loss_dev);
    device_destroy(sg_pwr_loss_dev_class, sg_pwr_loss_devno);
    class_destroy(sg_pwr_loss_dev_class);
    remove_proc_entry("power_loss_debug", NULL);

#ifdef PWR_LOSS_DEBUG
    printk(KERN_NOTICE "%s Power Loss Test:module exit Successfully!\n", TAG);
#endif

}

/*----------------------------------------------------------------------------*/
/* Storage Power Outage Handling(SPOH) MVG test sw */
/*----------------------------------------------------------------------------*/
//#ifdef PWR_LOSS_SPOH

/* API for kenrel storage drivers */
int mvg_delay_pwr_reset(unsigned int val)
{
	int ret = 0;
	if(mvg_trigger() && (val >= 0) && (power_loss_info.wdt_reboot_support == WDT_REBOOT_ON))
	{
		switch (mvg_get_method()){
        	case MVG_METHOD_HW_CUT:
				#ifdef PWR_LOSS_DEBUG
					printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, val);
				#endif
				if(val){
					mdelay(val);
				}
				hardware_reset();
				break;
			case MVG_METHOD_SW_WDT:
				if(mvg_get_wdt()){
					#ifdef PWR_LOSS_DEBUG
						printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, val);
					#endif
					if(val){
						mdelay(val);
					}
					wdt_arch_reset();
				}
				else{
					printk(KERN_ERR "%s Power Loss Test Fail: watchdog reset is not enabled for sw power loss!\n", TAG);
					ret = -1;
				}
				break;
			default:
				printk(KERN_ERR "%s Power Loss Test Fail: power loss method is neither hardware cut nor watchdog reset!\n", TAG);
				ret = -1;
		}
	}
	else
	{
		printk(KERN_ERR "%s Power Loss Test Fail: power loss trigger is not trigged or reboot is not enabled!\n", TAG);
		ret = -1;
	}
	return ret;
}

int mvg_random_delay_pwr_reset(unsigned int val)
{
	int ret = 0;
	if(mvg_trigger() && (val >= 0) && (power_loss_info.wdt_reboot_support == WDT_REBOOT_ON))
	{
		unsigned int sleep_time = 0;
		if(val){
		    get_random_bytes(&sleep_time, sizeof(unsigned int));
			sleep_time %= val;
		}
		switch (mvg_get_method()){
        	case MVG_METHOD_HW_CUT:
				#ifdef PWR_LOSS_DEBUG
					printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, sleep_time);
				#endif
				if(sleep_time)
					mdelay(sleep_time);
				hardware_reset();
				break;
			case MVG_METHOD_SW_WDT:
				if(mvg_get_wdt()){
					#ifdef PWR_LOSS_DEBUG
						printk(KERN_NOTICE "%s Power Loss Test: wait for reset in %d mseconds...!\n", TAG, sleep_time);
					#endif
					if(sleep_time)
						mdelay(sleep_time);
					wdt_arch_reset();
				}
				else{
					printk(KERN_ERR "%s Power Loss Test Fail: watchdog reset is not enabled for sw power loss!\n", TAG);
					ret = -1;
				}
				break;
			default:
				printk(KERN_ERR "%s Power Loss Test Fail: power loss method is neither hardware cut nor watchdog reset!\n", TAG);
				ret = -1;
				break;
		}
	}
	else {
		printk(KERN_ERR "%s Power Loss Test Fail: power loss trigger is not trigged or reboot is not enabled!\n", TAG);
		ret = -1;
	}
	return ret;

}

int mvg_get_method(void)
{
    int ret;
    down_read(&power_loss_info.rwsem);
    ret=power_loss_info.pl_method;
    up_read(&power_loss_info.rwsem);
    return ret;
}

void mvg_set_method(int method)
{
    down_write(&power_loss_info.rwsem);
    power_loss_info.pl_method = method;
    up_write(&power_loss_info.rwsem);
	
	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set mehtod to %X\n", TAG, power_loss_info.pl_method);
	#endif
}

int mvg_get_wdt(void)
{
    int ret;
    down_read(&power_loss_info.rwsem);
    ret=power_loss_info.wdt_enable;
    up_read(&power_loss_info.rwsem);
	//printk(KERN_ERR "%s mvg_get_wdt: %d\n", TAG, ret);
    return ret;
}

void mvg_set_wdt(int enable)
{
    down_write(&power_loss_info.rwsem);
    power_loss_info.wdt_enable = enable;
    up_write(&power_loss_info.rwsem);
	
	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set wdt to %X\n", TAG, power_loss_info.wdt_enable);
	#endif
}

int mvg_get_counter(void)
{
    int ret;

    down_read(&power_loss_info.rwsem);
    ret=power_loss_info.pl_counter;
    up_read(&power_loss_info.rwsem);
    return ret;
}

void mvg_set_counter(int counter)
{
    down_write(&power_loss_info.rwsem);
    power_loss_info.pl_counter = counter;
    up_write(&power_loss_info.rwsem);
	
	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set counter to %X\n", TAG, power_loss_info.pl_counter);
	#endif
}

void mvg_set_trigger(int trigger)
{
    down_write(&power_loss_info.rwsem);
    power_loss_info.pl_trigger= trigger;
    up_write(&power_loss_info.rwsem);

	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set trigger to %X\n", TAG, power_loss_info.pl_trigger);
	#endif
}

void mvg_set_flag(u32 flag)
{
    down_write(&power_loss_info.rwsem);
    power_loss_info.flags = flag;
    up_write(&power_loss_info.rwsem);

	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set flag to %X\n", TAG, power_loss_info.flags);
	#endif
}

u32  mvg_get_flag(void)
{
    int ret;
    down_read(&power_loss_info.rwsem);
    ret=power_loss_info.flags;
    up_read(&power_loss_info.rwsem);
    return ret;
}

void mvg_set_curr_case_name(char *str)
{
    down_write(&power_loss_info.rwsem);
    strncpy(power_loss_info.current_case_name, str, MVG_NAME_LIMIT);
    up_write(&power_loss_info.rwsem);

	#ifdef PWR_LOSS_DEBUG	
    	printk(KERN_NOTICE "%s set case name to %s\n", TAG, power_loss_info.current_case_name);
	#endif
}

void mvg_set_curr_group_name(char *str)
{
    down_write(&power_loss_info.rwsem);
    strncpy(power_loss_info.current_group_name, str, MVG_NAME_LIMIT);
    up_write(&power_loss_info.rwsem);

	#ifdef PWR_LOSS_DEBUG	
    	 printk(KERN_NOTICE "%s set group name to %s\n", TAG, power_loss_info.current_group_name);
	#endif
}

void mvg_get_curr_case_name(char *str, int size)
{
    down_read(&power_loss_info.rwsem);
    strncpy(str, power_loss_info.current_case_name,
        (size>MVG_NAME_LIMIT) ? MVG_NAME_LIMIT : size);
    up_read(&power_loss_info.rwsem);
}

void mvg_get_curr_group_name(char *str, int size)
{
    down_read(&power_loss_info.rwsem);
    strncpy(str, power_loss_info.current_group_name,
        (size>MVG_NAME_LIMIT) ? MVG_NAME_LIMIT : size);
    up_read(&power_loss_info.rwsem);
}

int mvg_on_group_case(const char *group_name, const char *case_name)
{
    int s1, s2;
	//printk(KERN_ERR "%s mvg_on_group_case: %s ~ %s\n", TAG, group_name, case_name);
    down_read(&power_loss_info.rwsem);
    s1=strcmp(group_name, power_loss_info.current_group_name);
    s2=strcmp(case_name, power_loss_info.current_case_name);
    up_read(&power_loss_info.rwsem);

    if (s1==0 && s2==0) /* both group & case are match */
        return 1;
    else
        return 0;
}
int mvg_addr_range_add(u64 base, u64 end)
{
    struct list_head *list_p;
    struct mvg_addr_list_entry *entry;

	#ifdef PWR_LOSS_DEBUG	
    	 printk(KERN_NOTICE "%s mvg_addr_range_add: %llX ~ %llX\n", TAG, base, end);
	#endif

    if (end < base) {
        printk(KERN_ERR "%s mvg_addr_range_add: Invalid range\n", TAG);
        return 0;
    }

    entry = kzalloc(sizeof(struct mvg_addr_list_entry), GFP_NOFS);

    if (!entry) {
        return -ENOMEM;
    }

    entry->base = base;
    entry->end = end;

    down_write(&power_loss_info.rwsem);
    list_p = &power_loss_info.addr_list;
    INIT_LIST_HEAD( &(entry->list));
    list_add_tail(&(entry->list), list_p);
    up_write(&power_loss_info.rwsem);

    return 0;
}

void mvg_addr_range_del(u64 addr)
{
    struct mvg_addr_list_entry *ptr, *tmp;
    struct list_head *list_p;

    down_write(&power_loss_info.rwsem);
    list_p = &power_loss_info.addr_list;
    list_for_each_entry_safe(ptr, tmp, list_p, list) {
        if ((addr >= ptr->base) && (addr < ptr->end)) {
            list_del(&ptr->list);
            kfree(ptr);
        }
    }
    up_write(&power_loss_info.rwsem);
}

void mvg_addr_range_clear(void)
{
    struct mvg_addr_list_entry *ptr, *tmp;
    struct list_head *list_p;

    down_write(&power_loss_info.rwsem);
    list_p = &power_loss_info.addr_list;
    list_for_each_entry_safe(ptr, tmp, list_p, list) {
        list_del(&ptr->list);
        kfree(ptr);
    }
    up_write(&power_loss_info.rwsem);
}

// return false, if wdt is disabled
int mvg_addr_range_check(u64 addr)
{
    struct mvg_addr_list_entry *ptr;
    struct list_head *list_p;

    int hit=0;

    down_read(&power_loss_info.rwsem);
    if (power_loss_info.wdt_enable) {
        list_p = &power_loss_info.addr_list;
        list_for_each_entry(ptr, list_p, list) {
            if ((addr >= ptr->base) && (addr < ptr->end)) {
                hit = 1;
                break;
            }
        }
    }
    up_read(&power_loss_info.rwsem);

    return hit;
}

int mvg_trigger(void)
{
    int ret;

    down_read(&power_loss_info.rwsem);
    ret=power_loss_info.pl_trigger;
    up_read(&power_loss_info.rwsem);

    if (MVG_TRIGGER_ALWAYS == ret)
        return ret;

    if (MVG_TRIGGER_NEVER != ret)
        ret = !(sched_clock() & ret);

    return ret;
}

#ifdef CONFIG_MTK_MTD_NAND

void mvg_wdt_reset(void)
{
	return;
}

#else

void mvg_wdt_reset(void)
{
    wdt_arch_reset();
}
#endif

module_init(power_loss_init);
module_exit(power_loss_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chandler.Han <Chandler.Han@autochips.com>");
MODULE_DESCRIPTION(" This module is for power loss test");


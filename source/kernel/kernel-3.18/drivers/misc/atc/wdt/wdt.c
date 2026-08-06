#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <mach/ac83xx_system.h>
#define PROC_NAME "pdwnc_wdt"

#define WDT_REG_CTRL   0x004
#define WDT_REG_CNT    0x008

enum wdt_cmd {
    WDT_CMD_DISABLE = 0,
    WDT_CMD_ENABLE,
    WDT_CMD_STOP_FEED,
    WDT_CMD_RESET_5S,
    WDT_CMD_RESET_1S,
    WDT_CMD_RESET_NOW,
};

static const char *wdt_cmd_str[] = {
    [WDT_CMD_DISABLE]    = "disable",
    [WDT_CMD_ENABLE]     = "enable",
    [WDT_CMD_STOP_FEED]  = "stop_feed",
    [WDT_CMD_RESET_5S]   = "reset_5s",
    [WDT_CMD_RESET_1S]   = "reset_1s",
    [WDT_CMD_RESET_NOW]  = "reset_now",
};

static int wdt_enabled = 1;
static int wdt_feed_enabled = 1;
static int wdt_hw_enabled;

static struct task_struct *wdt_task;


static void wdt_hw_set_enable(int enable)
{
    if (enable) {
        printk(KERN_INFO "WDT: enable\n");
        PDWNC_WRITE32(WDT_REG_CTRL, 1);
    } else {
        printk(KERN_INFO "WDT: disable\n");
        PDWNC_WRITE32(WDT_REG_CTRL, 0);
    }

    wdt_hw_enabled = enable;
}

static void wdt_hw_set_timeout(unsigned int sec)
{
    unsigned int ticks = sec * 3000000; // 3MHz
    unsigned int val = 0xFFFFFFFF - ticks;

    PDWNC_WRITE32(WDT_REG_CNT, val);
}

static inline void wdt_hw_feed(void)
{
    wdt_hw_set_timeout(3);
}

static int wdt_kthread(void *data)
{
    printk(KERN_INFO "WDT: thread start\n");

    wdt_hw_set_enable(1);
    wdt_hw_set_timeout(3);
    while (!kthread_should_stop()) {

        if (wdt_enabled && wdt_feed_enabled)
            wdt_hw_feed();

        msleep(500);
    }

    printk(KERN_INFO "WDT: thread stop\n");
    return 0;
}


static ssize_t wdt_proc_read(struct file *file,
                             char __user *buf,
                             size_t count, loff_t *ppos)
{
    char tmp[128];
    int len;

    len = snprintf(tmp, sizeof(tmp),
        "enabled=%d\nfeed=%d\nhw=%d\n",
        wdt_enabled, wdt_feed_enabled, wdt_hw_enabled);

    return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static ssize_t wdt_proc_write(struct file *file,
                              const char __user *buffer,
                              size_t count, loff_t *pos)
{
    char buf[16] = {0};
    int val;

    if (count > sizeof(buf) - 1)
        return -EINVAL;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    if (val < 0 || val >= ARRAY_SIZE(wdt_cmd_str)) {
        printk(KERN_ERR "WDT: invalid cmd %d\n", val);
        return count;
    }

    printk(KERN_INFO "WDT cmd: %s\n", wdt_cmd_str[val]);

    switch (val) {

    case WDT_CMD_DISABLE:
        wdt_enabled = 0;
        wdt_feed_enabled = 0;
        wdt_hw_set_enable(0);
        break;

    case WDT_CMD_ENABLE:
        wdt_enabled = 1;
        wdt_feed_enabled = 1;
        wdt_hw_set_enable(1);
        break;

    case WDT_CMD_STOP_FEED:
    	wdt_hw_set_enable(1);
        wdt_feed_enabled = 0;
        break;

    case WDT_CMD_RESET_5S:
        wdt_enabled = 1;
        wdt_feed_enabled = 0;
        wdt_hw_set_enable(1);
        wdt_hw_set_timeout(5);
        break;

    case WDT_CMD_RESET_1S:
        wdt_enabled = 1;
        wdt_feed_enabled = 0;
        wdt_hw_set_enable(1);
        wdt_hw_set_timeout(1);
        break;

    case WDT_CMD_RESET_NOW:
        wdt_enabled = 1;
        wdt_feed_enabled = 0;
        wdt_hw_set_enable(1);
        PDWNC_WRITE32(WDT_REG_CNT, 0xFFFFFF00);
        break;
    }

    return count;
}

static const struct file_operations wdt_proc_fops = {
    .owner = THIS_MODULE,
    .read  = wdt_proc_read,
    .write = wdt_proc_write,
};

/* ================= init / exit ================= */

static int __init pdwnc_wdt_init(void)
{
    printk(KERN_INFO "WDT: init\n");


    wdt_task = kthread_run(wdt_kthread, NULL, "wdt_thread");
    if (IS_ERR(wdt_task)) {
        printk(KERN_ERR "WDT: thread failed\n");
        return PTR_ERR(wdt_task);
    }

    if (!proc_create(PROC_NAME, 0666, NULL, &wdt_proc_fops)) {
        printk(KERN_ERR "WDT: proc create failed\n");
        kthread_stop(wdt_task);
        return -ENOMEM;
    }

    return 0;
}

static void __exit pdwnc_wdt_exit(void)
{
    printk(KERN_INFO "WDT: exit\n");

    if (wdt_task)
        kthread_stop(wdt_task);

    remove_proc_entry(PROC_NAME, NULL);

    wdt_hw_set_enable(0);
}

module_init(pdwnc_wdt_init);
module_exit(pdwnc_wdt_exit);

MODULE_LICENSE("GPL");

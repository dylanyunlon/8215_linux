#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscore_ops.h>
#include <linux/gpio.h>
#include <linux/delay.h>

/*To enable CONFIG_ATC_PLATFORM_ac83xx, include below head file*/
//#include <generated/atc_project.h>
#if 0
#include <mach/ac83xx_system.h>
#endif

const static int ATE_ENABLED = 1;
const static int ATE_DISABLED = 0;

#if 0
#ifdef CONFIG_ATC_PLATFORM_ac83xx
void  set_pdwnc_gpio_value(uint32_t u4Pin, uint32_t u4value);
#endif
#endif

struct kobject *shutdown_ate_kobj;

static int ate_enabled = 0;

static void shutdown_ate_syscore_shutdown(void)
{
    if (!ate_enabled) return;
    pr_info("[ATE]Will pulldown gpio136.\n");
    gpio_set_value(136, 0);

#if 0
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    pr_err("[ATE]Will set wakeup_sts low.\n");
	set_pdwnc_gpio_value(0, 0);
#endif
#endif
    msleep(200);
}

static ssize_t trigger_show(struct kobject *kobj, struct kobj_attribute *attr,
                char *buf)
{
    return sprintf(buf, "%d\n", ate_enabled);;
}

static ssize_t trigger_store(struct kobject *kobj, struct kobj_attribute *attr,
                const char *buf, size_t n)
{
    char* p;
    int len;

    p = memchr(buf, '\n', n);
    len = p ? p - buf : n;

    if (len == 1 && !strncmp(buf, "1", len)) {
        ate_enabled = ATE_ENABLED;

        pr_err("[ATE]Will pullup gpio136.\n");
        gpio_set_value(136, 1);
    } else if (len == 1 && !strncmp(buf, "0", len)) {
        ate_enabled = ATE_DISABLED;
    } else {
        return -EINVAL;
    }

    return n;
}


static struct kobj_attribute trigger_attr = __ATTR(trigger,
    0644,
    trigger_show,
    trigger_store);

static struct syscore_ops shutdown_ate_syscore_ops = {
    .shutdown = shutdown_ate_syscore_shutdown,
};

static int __init initialization_func(void)
{
    //pr_info("[ATE]Shutdown Testing Support Module init.\n");
    int ret = 0;

    shutdown_ate_kobj = kobject_create_and_add("atc_ate", NULL);
    if (!shutdown_ate_kobj) {
        return -ENOMEM;
    }

    ret = sysfs_create_file(shutdown_ate_kobj, &trigger_attr.attr);
    if (ret) {
        pr_err("[ATE]Fail to create file /sys/atc_ate/trigger");
    }

    register_syscore_ops(&shutdown_ate_syscore_ops);
    return ret;
}
module_init(initialization_func);

static int __exit cleanup_func(void)
{
    pr_info("[ATE]Shutdown Testing Support Module exit.\n");
    unregister_syscore_ops(&shutdown_ate_syscore_ops);

    if (shutdown_ate_kobj) {
        kobject_del(shutdown_ate_kobj);
    }
    return 0;
}
module_exit(cleanup_func);

MODULE_AUTHOR("Qing Yu <qing.yu@autochips.com>");
MODULE_DESCRIPTION("Auto Test Equipment Support");
MODULE_LICENSE("GPL");

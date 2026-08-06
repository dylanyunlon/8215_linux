#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>

static int __init wlan_gen3_init(void)
{
	pr_info("%s start\n", __func__);
	pr_info("%s end\n", __func__);

	return 0;
}

static void __exit wlan_gen3_exit(void)
{
	pr_info("%s start\n", __func__);
	pr_info("%s end\n", __func__);
}

module_init(wlan_gen3_init);
module_exit(wlan_gen3_exit);

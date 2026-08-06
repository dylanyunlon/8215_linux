
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <linux/semaphore.h>
#include <linux/platform_device.h>

#include <linux/of.h>
#include <linux/of_address.h>

#include "x_typedef.h"
#include "oal.h"
#include "windev.h"
#include "hdcp2x_drv.h"
#include <x_ver.h>
#include "metazone.h"

#define ATC_KERNEL_LINUX_LICENSE     "Proprietary"

bool forceRxsence = FALSE;
struct semaphore mhdcp2x_sem;


int hdcp2x_open(struct inode *inode, struct file *file)
{
#if 0
    down(&mhlrx_sem);

	if (mhlrx_Single) {
		HDMI_LOG(HDMI_LOG_DEBUG, "mhlrx has opened");
		up(&mhlrx_sem);
		return -1;
	}

	mhlrx_Single = TRUE;
	up(&mhlrx_sem);

	HDMI_LOG(HDMI_LOG_INFO, "MHL open success");
#endif   
	return 0;
}
int hdcp2x_release(struct inode *inode, struct file *filp)
{
#if 0
	down(&mhlrx_sem);
	mhlrx_Single = FALSE;
	up(&mhlrx_sem);
#endif
	return 0;
}

static long hdcp2x_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    _i4DRV_HDCP2X_CmdHandle(cmd, arg);
    return 0;

}

const struct file_operations hdcp2x_fops = {
	.open           = hdcp2x_open,
	.release        = hdcp2x_release,
	.unlocked_ioctl = hdcp2x_ioctl,
};

static struct miscdevice hdcp2x_dev = {
	MISC_DYNAMIC_MINOR,
	"hdcp2xdev",
	&hdcp2x_fops
};




int __init hdcp2x_init(void)
{
	long bret = 0;

    misc_register(&hdcp2x_dev);
    
	return bret;
}

static void __exit hdcp2x_exit(void)
{
    misc_deregister(&hdcp2x_dev);
}

module_init(hdcp2x_init);
module_exit(hdcp2x_exit);

MODULE_AUTHOR("ATC Inc");
MODULE_DESCRIPTION("HDCP2X Driver");
MODULE_LICENSE("GPL");




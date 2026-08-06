/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>

#include "wch_log.h"
#include "wch_hal.h"
#include "wch_if.h"
#include "wch_drv.h"
#include "wch_priv.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define ATC_WCH_DRIVER_INFO	"ATC write channel driver"

#define MMISC_MODE_NAME              "WCH"
#define MMISC_VER_MAJOR              10
#define MMISC_VER_MINOR              00
#define MMISC_VER_REV                00
#define NO_IRQ (unsigned int)(-1)//for 8237

struct semaphore wchSem[WCH_NUM];

#ifndef __ARM2__
spinlock_t wchParamLock[WCH_NUM];
#endif

/*get register base from dts*/
void __iomem *wch_sysreg_base[WCH_NUM];
extern int _g_show_isr_interval;


/*get reserve memory base from dts*/
unsigned long wch_base_va = 0;
unsigned long wch_size = 0;
unsigned long wch_base_pa = 0;

static long wch_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_ioctl not be implemented \n");
	/*to do*/

	/*****/
	return 0;
}

struct file_operations const wch_fops = {
	.unlocked_ioctl = wch_ioctl,
};

static struct miscdevice wch_dev = {
	MISC_DYNAMIC_MINOR,
	"wch",
	&wch_fops
};


int wch_suspend(struct device *dev)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_suspend--->\n");

	WchSuspend();

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_suspend OK--->\n");
	return 0;
}

int wch_resume(struct device *dev)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_resume--->\n");
	WchResume();

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_resume OK--->\n");
	return 0;
}

static const struct dev_pm_ops wch_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(wch_suspend, wch_resume)
};


#if WCH_DUMP_BUFFER_ATTR
static ssize_t wch_dumpbuffer_show(struct device *dev, struct device_attribute *attr, char *buf)
{

/* ssize_t size; */
/* size = sprintf(buf, "Wch Log Level %d\n", (int); */

	return 0;
}

bool WchDumpBuffer(u32 u1WchId, unsigned long u4YAddr, unsigned long u4CAddr, u32 u4Width, u32 u4Hight)
{
	char szDumpFile[256];
	struct file *fp = NULL;
	mm_segment_t old_fs;
	loff_t pos;
	int ret = 0, ysize = 0, csize = 0;
	void *virt_addr = NULL;

	ysize = u4Width * u4Hight;
	csize = ysize / 2;
	
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	/* write y data begin */
	sprintf(szDumpFile, "/storage/ext_sdcard2/y%02d.raw", u4DumpBufIdxCnt[u1WchId]);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchDumpBuffer] Dump File:%s for wch%d\r\n", szDumpFile,
		(int)u1WchId);

	fp = filp_open(szDumpFile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchDumpBuffer] open file %s error wch%d return %d\n",
			szDumpFile, (int)u1WchId, ret);
		return false;
	}

	pos = 0;
	virt_addr = phys_to_virt((phys_addr_t)u4YAddr);
	ret = vfs_write(fp, (char __user *)virt_addr, ysize, &pos);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchDumpBuffer] write file addr 0x%lx, 0x%lx size %d, %d\r\n",
		u4YAddr, (unsigned long)virt_addr, (int)ysize, (int)pos);

	filp_close(fp, NULL);
	/* write y data end */

	/* write cbcr data begin */
	sprintf(szDumpFile, "/storage/ext_sdcard2/cbcr%02d.raw", u4DumpBufIdxCnt[u1WchId]);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchDumpBuffer] Dump File:%s for wch%d\r\n", szDumpFile,
		(int)u1WchId);

	fp = filp_open(szDumpFile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchDumpBuffer] open file %s error wch%d return %d\n",
			szDumpFile, (int)u1WchId, ret);
		return false;
	}

	pos = 0;
	virt_addr = phys_to_virt((phys_addr_t)u4CAddr);
	ret = vfs_write(fp, (char __user *)virt_addr, csize, &pos);
	WCH_LOG(WCH_LOG_LVL_INFO, "[WchDumpBuffer] write file addr 0x%lx, 0x%lx size %d, %d\r\n",
		u4CAddr, (unsigned long)virt_addr, (int)csize,	(int)pos);
	/* write cbcr data */

	filp_close(fp, NULL);
	set_fs(old_fs);

	return true;
}

static ssize_t wch_dumpbuffer_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	u32 u4BuffIdx = 0;
	u32 u4Width = 0, u4Hight = 0, u4FrameNum = 0, u4FrameCnt = 0;
	u32 u4WchIdDump = 0;
	int ret;
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_dumpbuffer_store enter!\n");

	ret = sscanf(buf, "%d %d %d %d", (int *)&u4WchIdDump, (int *)&u4Width, (int *)&u4Hight, (int *)&u4FrameNum);
	if (ret < 0) {
		pr_err("sscanf error!\n");
		return -EINVAL;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_dumpbuffer_store u4Width: %d, u4Hight: %d, u4FrameNum: %d!\n", 
		u4Width, u4Hight, u4FrameNum);
	for (u4FrameCnt = 0; u4FrameCnt < u4FrameNum; u4FrameCnt++) {
		if (u4DumpFrameCnt[u4WchIdDump]) {
			u4BuffIdx =
			    u4DumpBufIdxCnt[u4WchIdDump] % _gWchParam[u4WchIdDump].tWchBuf.u4BufCnt;
			WCH_LOG(WCH_LOG_LVL_DBG, "wch_dumpbuffer_store wch id %d, buffer id %d\r\n",
				(int)u4WchIdDump, (int)u4BuffIdx);
			/* dump begin */
			WCH_LOG(WCH_LOG_LVL_ERR, "wch_dumpbuffer_store dumpbuffer!\n");
			WchDumpBuffer(u4WchIdDump,
				      _gWchParam[u4WchIdDump].tWchBuf.u4YBuf[u4BuffIdx],
				      _gWchParam[u4WchIdDump].tWchBuf.u4CBuf[u4BuffIdx], u4Width, u4Hight);
			/* dump end */
			u4DumpBufIdxCnt[u4WchIdDump]++;
			u4DumpFrameCnt[u4WchIdDump]--;
		}
	}
	return count;
}

static DEVICE_ATTR(wchdumpbuffer, S_IWUSR | S_IRUGO, wch_dumpbuffer_show, wch_dumpbuffer_store);

#endif

static ssize_t wch_loglevel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;

	size = sprintf(buf, "Wch Log Level %d\n", (int)_u4WCH_DBG_LVL);

	return size;
}

static ssize_t wch_loglevel_store(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int ret;
	char *buffer = "done";

	ret = sscanf(buf, "%d %s", (int *)&_u4WCH_DBG_LVL, buffer);
	if (ret < 0) {
		pr_err("sscanf error!\n");
		return -EINVAL;
	}
	return (ssize_t)count;
}

static DEVICE_ATTR(wchloglevel, S_IWUSR | S_IRUGO, wch_loglevel_show, wch_loglevel_store);

static ssize_t wch_IntrruptInterval_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;

	size = sprintf(buf, "Wch interrupt interval %d\n", _g_show_isr_interval);

	return size;
}

static ssize_t wch_IntrruptInterval_store(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int ret;
	char *buffer = "done";

	ret = sscanf(buf, "%d %s", (int *)&_g_show_isr_interval, buffer);
	if (ret < 0) {
		pr_err("sscanf error!\n");
		return -EINVAL;
	}
	return (ssize_t)count;
}

static DEVICE_ATTR(wchInterruptInterval, S_IWUSR | S_IRUGO, wch_IntrruptInterval_show, wch_IntrruptInterval_store);



static int wch_probe(struct platform_device *pdev)
{
	unsigned int ret = -EINVAL;
	struct device_node *nd = pdev->dev.of_node;
	int i = 0;

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_probe--->\n");

	_IO_BASE_ = (unsigned long)ioremap(0x10000000, 0x100000);
	if (!_IO_BASE_) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get io base address failed = %p \r\n",
					(void *)_IO_BASE_);
		goto err;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get io base address _IO_BASE_ = %lx \r\n",_IO_BASE_);

	for (i = 0; i < WCH_NUM; i++) {
		wch_sysreg_base[i] = (void __iomem *)of_iomap(nd, i);
		if (!wch_sysreg_base[i]) {
			WCH_LOG(WCH_LOG_LVL_ERR, "get wch%d reg base address failed = %p \r\n",
				i + 1, wch_sysreg_base[i]);
			goto err;
		}
		WCH_LOG(WCH_LOG_LVL_INFO, "get wch%d reg base address = %p \r\n", i + 1, wch_sysreg_base[i]);
	}
	
    
	wchGetHwRegAddress();

	WCH_LOG(WCH_LOG_LVL_INFO, "get wch irq start \r\n");

	for (i = 0; i < WCH_NUM; i++) {
		wchirq[i] = (unsigned int)irq_of_parse_and_map(nd, i) - 32 + 104;
		if (wchirq[i] == NO_IRQ) {
			WCH_LOG(WCH_LOG_LVL_ERR, "get wch%d irq failed = %d \r\n", i + 1, wchirq[i]);
			goto err;
		}
		WCH_LOG(WCH_LOG_LVL_INFO, "get wch%d irq = %d \r\n", i + 1, wchirq[i]);
	}
	
	{
		struct device_node *node = NULL;
		int result = 0;
		u32 regs[4] = {0};

		node = of_find_compatible_node(NULL, NULL, "atc-wch");
		if (!node) {
			WCH_LOG(WCH_LOG_LVL_ERR, "of_find_compatible_node atc-wch Fail\r\n");
			goto err;
		}

		result = of_property_read_u32_array(node, "reg", regs, 4);
		if (0 != result) {
			WCH_LOG(WCH_LOG_LVL_ERR, "of_property_read_u32_array reg Fail\r\n");
			goto err;
		}

		WCH_LOG(WCH_LOG_LVL_INFO, " probe find reg(0x%08x, 0x%08x, 0x%08x, 0x%08x)!!\r\n",
			regs[0], regs[1], regs[2], regs[3]);
		
		wch_base_pa = (((__u64)regs[0]) << 32) | ((__u64)regs[1]);
		WCH_LOG(WCH_LOG_LVL_INFO, "get wch_base_pa = 0x%lx \r\n", wch_base_pa);

		wch_size = (((__u64)regs[2]) << 32) | ((__u64)regs[3]);
		WCH_LOG(WCH_LOG_LVL_INFO, "get wch_size = 0x%lx \r\n", wch_size);
	}

	if ((wch_base_pa == 0) || (wch_size == 0)) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get memory failed base %lx size %lx\r\n", wch_base_pa, wch_size);
		goto err;
	}
	
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_base_pa=%lx wch_base_va=%lx, wch_size=%lx\n",wch_base_pa, wch_base_va,wch_size);
	WchSetSourceBaseAddr(wch_base_pa);

	ret = misc_register(&wch_dev);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "wch_init: misc_register error %d\r\n", ret);

#if WCH_DUMP_BUFFER_ATTR
	ret = device_create_file(wch_dev.this_device, &dev_attr_wchdumpbuffer);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);
#endif

	/* Add wchloglevel variable in /sys/devices/virtual/misc/wch */
	ret = device_create_file(wch_dev.this_device, &dev_attr_wchloglevel);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);

	ret = device_create_file(wch_dev.this_device, &dev_attr_wchInterruptInterval);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);

	return ret;

err:
    for (i = 0; i < WCH_NUM; i++) {
		if (NULL != wch_sysreg_base[i]) {
			iounmap (wch_sysreg_base[i]);
			wch_sysreg_base[i] = NULL;
		}
	}
	
	return ret;
}

static int wch_remove(struct platform_device *pdev)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_remove<---\n");
#if WCH_DUMP_BUFFER_ATTR
	device_remove_file(wch_dev.this_device, &dev_attr_wchdumpbuffer);
#endif
	device_remove_file(wch_dev.this_device, &dev_attr_wchloglevel);

	device_remove_file(wch_dev.this_device, &dev_attr_wchInterruptInterval);

	misc_deregister(&wch_dev);

	return 0;
}

static void wch_shutdown(struct platform_device *pdev)
{
	int i = 0;

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_shutdown <---\n");
	
	for (i = 0; i < WCH_NUM; i++) {
		if (_gWchParam[i].u4Status == WCH_HW_START) {
			CloseWch(i, _gWchParam[i].eWchSrcId);
		}
	}
}

static const struct of_device_id wch_of_ids[] = {
	{.compatible = "Autochips,writechannel",},
	{}
};

static struct platform_driver wch_plt_drv = {
	.driver = {
		   .name = "Autochips-writechannel",
		   .owner = THIS_MODULE,
		   .pm = &wch_pm_ops,
		   .of_match_table = wch_of_ids,
		   },
	.probe = wch_probe,
	.remove = wch_remove,
	.shutdown = wch_shutdown,
};


static int __init wch_init(void)
{
	int ret = 0;
    unsigned char u1WchId = 0;

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_init--->\n");
	ret = platform_driver_register(&wch_plt_drv);
	if (ret){
		WCH_LOG(WCH_LOG_LVL_ERR, "[wch]: %s: register  driver failed\n", __func__);
    }

    for(u1WchId = WCH_1; u1WchId <= WCH_9; u1WchId++) {
	    init_MUTEX_LOCKED(&wchSem[u1WchId]);
        
        #ifndef __ARM2__
        spin_lock_init(&wchParamLock[u1WchId]); 
        #endif        
    }

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_init---> WchEventThreadInit\n");
	WchEventThreadInit();
    
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_init---> WchIsrInit\n");
	/*request IRQ*/
    for(u1WchId = WCH_1; u1WchId <= WCH_9; u1WchId++) {
	    WchIsrInit(u1WchId);
    }

	return ret;
}

static void __exit wch_exit(void)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_exit<---\n");

	//kthread_stop(hWchInst[WCH_5]);
    WchEventThreadDeinit();

	platform_driver_unregister(&wch_plt_drv);
}
module_init(wch_init);
module_exit(wch_exit);

MODULE_DESCRIPTION(ATC_WCH_DRIVER_INFO);
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);


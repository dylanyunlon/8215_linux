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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
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
#include <generated/atc_project.h>

#include "windows.h"
#include "oal.h"
#include "windev.h"
#include "x_ver.h"
#include "x_typedef.h"
#include "atc/wch_drv.h"
#include "atc/wch_if.h"
#include "hal/wch_log.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"

#define MMISC_MODE_NAME              "WCH"
#define MMISC_VER_MAJOR              01
#define MMISC_VER_MINOR              00
#define MMISC_VER_REV                00

static long wch_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 u4Ret = WCH_SUCCESS;

	if (!file)
		u4Ret = WCH_CONTEXT_NULL;

	switch (cmd) {
	case IOCTL_WCH_CONFIG :
	{
		PWCH_CTL_PARAM_T pWchCtlParam;

		if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(WCH_CTL_PARAM_T)))
			u4Ret = WCH_ACCESS_FAIL;

		if (copy_from_user((void *)pWchCtlParam, (void *)arg, sizeof(WCH_CTL_PARAM_T)))
			u4Ret = WCH_COPYFROMUSER_FAIL;

		u4Ret = ConfigWch(pWchCtlParam);
		break;
	}

	case IOCTL_WCH_START :
	{
		WCH_SRC_APP_ID_E eWchSrcId;	
		if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_ACCESS_FAIL;

		if (copy_from_user((void *)&eWchSrcId, (void *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_COPYFROMUSER_FAIL;

		u4Ret = StartWch(eWchSrcId);
		break;
	}

	case IOCTL_WCH_STOP :
	{
		WCH_SRC_APP_ID_E eWchSrcId;	
		if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_ACCESS_FAIL;
	
		if (copy_from_user((void *)&eWchSrcId, (void *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_COPYFROMUSER_FAIL;
	
		u4Ret = StopWch(eWchSrcId);
		break;
	}

	case IOCTL_WCH_CLOSE :
	{
		WCH_SRC_APP_ID_E eWchSrcId;	
		if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_ACCESS_FAIL;
	
		if (copy_from_user((void *)&eWchSrcId, (void *)arg, sizeof(WCH_SRC_APP_ID_E)))
			u4Ret = WCH_COPYFROMUSER_FAIL;
	
		u4Ret = CloseWch(eWchSrcId);
		break;
	}

	case IOCTL_WCH_GET_ADDR :
	{
		PWCH_BUF_T pWchGetBuf;	
		if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(WCH_BUF_T)))
			u4Ret = WCH_ACCESS_FAIL;
	
		if (copy_from_user((void *)pWchGetBuf, (void *)arg, sizeof(WCH_BUF_T)))
			u4Ret = WCH_COPYFROMUSER_FAIL;
	
		u4Ret = WchGetBufferAddress(pWchGetBuf);

		if (copy_to_user((void *)arg, (void *)pWchGetBuf, sizeof(WCH_BUF_T)))
			u4Ret = WCH_COPYFROMUSER_FAIL;
		break;
	}
	}	

	WCH_LOG(WCH_LOG_LVL_DBG, "wch_ioctl cmd = %x, return %x\r\n", cmd, (unsigned int)u4Ret);

	return (long)u4Ret;
}
/*
static long wch_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	WIN32_IOCTL_DATA win_ioctl;
	u32 u4Ret = WCH_SUCCESS;

	if (!file)
		u4Ret = WCH_CONTEXT_NULL;

	if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(win_ioctl)))
		u4Ret = WCH_ACCESS_FAIL;

	if (copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl)))
		u4Ret = WCH_COPYFROMUSER_FAIL;

	if (!access_ok(VERIFY_READ, (void __user *)(win_ioctl.pInBuf), sizeof(win_ioctl.InSize)))
		u4Ret = WCH_ACCESS_FAIL;

	if (!access_ok(VERIFY_WRITE, (void __user *)(win_ioctl.pOutBuf), sizeof(win_ioctl.OutSize)))
		u4Ret = WCH_ACCESS_FAIL;

	if (u4Ret == WCH_SUCCESS) {
		u4Ret =
		    WchIoControl((u32) file, (u32) cmd, (u8 *) win_ioctl.pInBuf,
				 (u32) win_ioctl.InSize, (u8 *) win_ioctl.pOutBuf,
				 (u32) win_ioctl.OutSize, (u32 *) win_ioctl.pBytesReturned);
	}

	WCH_LOG(WCH_LOG_LVL_DBG, "wch_ioctl cmd = %x, return %x\r\n", cmd, (unsigned int)u4Ret);

	return (long)u4Ret;
}
*/
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

/*
*	wch dump function begin
*
*/
#if WCH_DUMP_BUFFER_ATTR


static ssize_t wch_dumpbuffer_show(struct device *dev, struct device_attribute *attr, char *buf)
{

/* ssize_t size; */
/* size = sprintf(buf, "Wch Log Level %d\n", (int); */

	return 0;
}

void *WchIoremap(phys_addr_t phys_addr, size_t size)
{
	void *vir_addr = NULL;

	if (phys_addr == 0 || size == 0) {
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchIoremap] phys_addr:0x%x ,size:0x%x \r\n",phys_addr, size);
		return NULL;
	}

	vir_addr = ioremap(phys_addr, size);
	if (vir_addr == NULL) {
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchIoremap] ERROR \n");
		return NULL;
	}

	return vir_addr;
}

void WchIoUnmap(void *vir_addr)
{
	if (vir_addr == NULL) {
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchIoUnmap] no need to unmap the address! \n");
		return;
	}
	iounmap((void __iomem *)vir_addr);
}

enum {
	WCH_SD_SRC = 0,
	WCH_VGA_SRC,
	WCH_HDMI_SRC,
	WCH_DISP_SRC,
	WCH_DGI_SRC
};
bool WchDumpBuffer(u32 u1WchId, u32 u4YAddr, u32 u4CAddr, u32 u4Src)
{
	TCHAR szDumpFile[256];
	struct file *fp = NULL;
	mm_segment_t old_fs;
	loff_t pos;
	int ret = 0, ysize = 0, csize = 0;
	void *map_addr = NULL;

	switch (u4Src) {
	case WCH_SD_SRC:{
			ysize = (int)WCH_SD_YBUF_SIZE;
			csize = (int)WCH_SD_CBUF_SIZE;
			break;
		}
	case WCH_VGA_SRC:{
			ysize = (int)WCH_VGA_YBUF_SIZE;
			csize = (int)WCH_VGA_CBUF_SIZE;
			break;
		}
	case WCH_HDMI_SRC:{
			ysize = (int)WCH_HDMI_YBUF_SIZE;
			csize = (int)WCH_HDMI_CBUF_SIZE;
			break;
		}
	case WCH_DISP_SRC:{
			ysize = (int)WCH_DISP_YBUF_SIZE;
			csize = (int)WCH_DISP_CBUF_SIZE;
			break;
		}
	case WCH_DGI_SRC:{
			ysize = (int)WCH_SD_YBUF_SIZE;
			csize = (int)WCH_SD_CBUF_SIZE;
			break;
		}
	default:
		break;
	}
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	/* write y data begin */
#if defined(CONFIG_ATC_OS_android)
	sprintf(szDumpFile, "/storage/ext_sdcard2/y%02d.raw", (int)u4DumpBufIdxCnt[u1WchId]);
#elif defined(CONFIG_ATC_OS_linux)
	sprintf(szDumpFile, "/media/ext_sdcard2/y%02d.raw", (int)u4DumpBufIdxCnt[u1WchId]);
#endif
	WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] Dump File:%s for wch%d\r\n", szDumpFile,
		(int)u1WchId);

	fp = filp_open(szDumpFile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchDumpBuffer] open file %s error wch%d return %d\n",
			szDumpFile, (int)u1WchId, ret);
		return false;
	}

	pos = 0;
#if defined(CONFIG_ATC_OS_android)
	map_addr = WchIoremap(u4YAddr, ysize);
	ret = vfs_write(fp, (char __user *)map_addr, ysize, &pos);
	WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x, 0x%x size %d, %d\r\n",
		(unsigned int)u4YAddr, (unsigned int)map_addr, (int)ysize,
		(int)pos);
	WchIoUnmap(map_addr);
#elif defined(CONFIG_ATC_OS_linux)
    if (u4Src == WCH_SD_SRC) {
		map_addr = WchIoremap(u4YAddr, ysize);
		ret = vfs_write(fp, (char __user *)map_addr, ysize, &pos);
		WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x, 0x%x size %d, %d\r\n",
			(unsigned int)u4YAddr, (unsigned int)map_addr, (int)ysize,
			(int)pos);
		WchIoUnmap(map_addr);
	} else {
		ret = vfs_write(fp, (char __user *)u4YAddr, ysize, &pos);
		WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x	size %d, %d\r\n",
			(unsigned int)u4YAddr, (int)ysize,
			(int)pos);
	}
#endif
	filp_close(fp, NULL);
	/* write y data end */

	/* write cbcr data begin */
#if defined(CONFIG_ATC_OS_android)
	sprintf(szDumpFile, "/storage/ext_sdcard2/cbcr%02d.raw", (int)u4DumpBufIdxCnt[u1WchId]);
#elif defined(CONFIG_ATC_OS_linux)
	sprintf(szDumpFile, "/media/ext_sdcard2/cbcr%02d.raw", (int)u4DumpBufIdxCnt[u1WchId]);
#endif
	WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] Dump File:%s for wch%d\r\n", szDumpFile,
		(int)u1WchId);

	fp = filp_open(szDumpFile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		WCH_LOG(WCH_LOG_LVL_ERR, "[WchDumpBuffer] open file %s error wch%d return %d\n",
			szDumpFile, (int)u1WchId, ret);
		return false;
	}

	pos = 0;
#if defined(CONFIG_ATC_OS_android)
	map_addr = WchIoremap(u4CAddr, csize);
	ret = vfs_write(fp, (char __user *)map_addr, csize, &pos);
	WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x, 0x%x size %d, %d\r\n",
		(unsigned int)u4CAddr, (unsigned int)map_addr, (int)csize,
		(int)pos);
	WchIoUnmap(map_addr);
#elif defined(CONFIG_ATC_OS_linux)
    if (u4Src == WCH_SD_SRC) {
		map_addr = WchIoremap(u4CAddr, csize);
		ret = vfs_write(fp, (char __user *)map_addr, csize, &pos);
		WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x, 0x%x size %d, %d\r\n",
			(unsigned int)u4CAddr, (unsigned int)map_addr, (int)csize,
			(int)pos);
		WchIoUnmap(map_addr);
	} else {
		ret = vfs_write(fp, (char __user *)u4CAddr, csize, &pos);
		WCH_LOG(WCH_LOG_LVL_DBG, "[WchDumpBuffer] write file addr 0x%x, size %d, %d\r\n",
			(unsigned int)u4CAddr, (int)csize,
			(int)pos);
	}
#endif
	/* write cbcr data */

	u4DumpBufIdxCnt[u1WchId]++;
	filp_close(fp, NULL);
	set_fs(old_fs);

	return true;
}
#if defined(CONFIG_ATC_OS_linux)
extern WCH_BUFF_INFO_T _virtualaddr[3];
#endif
static ssize_t wch_dumpbuffer_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	u32 u4BuffIdx = 0;
	u32 u4Src, u4FrameNum, u4FrameCnt;
	u32 u4WchIdDump;
	int ret;

	ret = sscanf(buf, "%d %d %d", (int *)&u4WchIdDump, (int *)&u4Src, (int *)&u4FrameNum);
	if (ret < 0) {
		pr_err("sscanf error!\n");
		return -EINVAL;
	}
	for (u4FrameCnt = 0; u4FrameCnt < u4FrameNum; u4FrameCnt++) {
		if (u4DumpFrameCnt[u4WchIdDump]) {
#if defined(CONFIG_ATC_OS_android)
			if (0 ==_gWchParam[u4WchIdDump].tWchBuf.u4BufCnt) {
				WCH_LOG(WCH_LOG_LVL_ERR, "fail for _gWchParam[u4WchIdDump].tWchBuf.u4BufCnt == 0 u4DumpFrameCnt--:%d! \n",u4DumpFrameCnt[u4WchIdDump]);
				u4DumpFrameCnt[u4WchIdDump]--;
				continue;
			}
			u4BuffIdx =
			    u4DumpBufIdxCnt[u4WchIdDump] % _gWchParam[u4WchIdDump].tWchBuf.u4BufCnt;
#elif defined(CONFIG_ATC_OS_linux)
			if (u4Src == WCH_SD_SRC) {
				if (0 ==_gWchParam[u4WchIdDump].tWchBuf.u4BufCnt) {
					WCH_LOG(WCH_LOG_LVL_ERR, "fail for _gWchParam[u4WchIdDump].tWchBuf.u4BufCnt == 0 u4DumpFrameCnt--:%d! \n",u4DumpFrameCnt[u4WchIdDump]);
					u4DumpFrameCnt[u4WchIdDump]--;
					continue;
				}
				u4BuffIdx =
					u4DumpBufIdxCnt[u4WchIdDump] % _gWchParam[u4WchIdDump].tWchBuf.u4BufCnt;
			} else {
				if (0 ==_virtualaddr[u4WchIdDump].u4BufCnt) {
					WCH_LOG(WCH_LOG_LVL_ERR, "fail for _virtualaddr[u4WchIdDump].u4BufCnt == 0 u4DumpFrameCnt--:%d! \n",u4DumpFrameCnt[u4WchIdDump]);
					u4DumpFrameCnt[u4WchIdDump]--;
					continue;
				}
				u4BuffIdx =
				    u4DumpBufIdxCnt[u4WchIdDump] % _virtualaddr[u4WchIdDump].u4BufCnt;
			}
#endif
			WCH_LOG(WCH_LOG_LVL_DBG, "wch_dumpbuffer_store wch id %d, buffer id %d\r\n",
				(int)u4WchIdDump, (int)u4BuffIdx);
			/* dump begin */
#if defined(CONFIG_ATC_OS_android)
			WchDumpBuffer(u4WchIdDump,
				      _gWchParam[u4WchIdDump].tWchBuf.u4YBuf[u4BuffIdx],
				      _gWchParam[u4WchIdDump].tWchBuf.u4CBuf[u4BuffIdx], u4Src);
#elif defined(CONFIG_ATC_OS_linux)
			if (u4Src == WCH_SD_SRC) {
				WchDumpBuffer(u4WchIdDump,
						  _gWchParam[u4WchIdDump].tWchBuf.u4YBuf[u4BuffIdx],
						  _gWchParam[u4WchIdDump].tWchBuf.u4CBuf[u4BuffIdx], u4Src);
			} else {
				WchDumpBuffer(u4WchIdDump,
						  _virtualaddr[u4WchIdDump].u4YBuf[u4BuffIdx],
						  _virtualaddr[u4WchIdDump].u4CBuf[u4BuffIdx], u4Src);
			}
#endif
			/* dump end */
			u4DumpFrameCnt[u4WchIdDump]--;
		}
	}
	return count;
}

static DEVICE_ATTR(wchdumpbuffer, S_IWUSR | S_IRUGO, wch_dumpbuffer_show, wch_dumpbuffer_store);

#endif
/*
*	wch dump function end
*
*/

static ssize_t wch_capturescreen_show(struct device * dev,struct device_attribute * attr,char * buf)
{
/* ssize_t size; */
/* size = sprintf(buf, "Wch Log Level %d\n", (int); */

	return 0;
}

static ssize_t wch_capturescreen_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	s32 ret = 0, _random_num;

	ret = sscanf(buf, "%d", (int *)&_random_num);
	if (ret < 0) {
		WCH_LOG(WCH_LOG_LVL_ERR, "wch_capturescreen_store sscanf error!\n");
		return -EINVAL;
	}
	if (false == WchCaptureScreen()) {
		WCH_LOG(WCH_LOG_LVL_ERR, "capture screen error!\n");
	}

	return count;
}

static DEVICE_ATTR(wchcapturescreen, S_IWUSR | S_IRUGO, wch_capturescreen_show, wch_capturescreen_store);

struct file_operations const wch_fops = {
	.unlocked_ioctl = wch_ioctl,
};

static struct miscdevice wch_dev = {
	MISC_DYNAMIC_MINOR,
	"wch",
	&wch_fops
};

void __iomem *wch0_sysreg_base = NULL;
void __iomem *wch1_sysreg_base = NULL;
unsigned int wch0irq = 0;
unsigned int wch1irq = 0;
struct clk *clk_ac8317_wch0 = NULL;
struct clk *clk_ac8317_wch1 = NULL;
struct pinctrl *pinctrl_wch = NULL;

static int wch_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *nd = pdev->dev.of_node;
	struct reserved_mem *wch_reserve_mem = NULL;
	phys_addr_t wch_base_addr = 0;

	wch0_sysreg_base = (void __iomem *)of_iomap(nd, 0);
	if (!wch0_sysreg_base) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch0 reg base address failed = %p \r\n",
			wch0_sysreg_base);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch0 reg base address = %p \r\n", wch0_sysreg_base);

	wch1_sysreg_base = (void __iomem *)of_iomap(nd, 1);
	if (!wch1_sysreg_base) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch1 reg base address failed  = %p \r\n",
			wch1_sysreg_base);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch1 reg base address = %p \r\n", wch1_sysreg_base);

	wch0irq = (unsigned int)irq_of_parse_and_map((struct device_node *)nd, 0);
	if (wch0irq == NO_IRQ) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch0 irq failed = %d \r\n", wch0irq);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch0 irq = %d \r\n", wch0irq);

	wch1irq = (unsigned int)irq_of_parse_and_map((struct device_node *)nd, (int)1);
	if (wch1irq == NO_IRQ) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch1 irq failed = %d \r\n", wch1irq);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch1 irq = %d \r\n", wch1irq);

	wchGetHwRegAddress();

	clk_ac8317_wch0 = devm_clk_get(&pdev->dev, "wch0-device");
	if (!clk_ac8317_wch0) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch0 clk failed %p\r\n", clk_ac8317_wch0);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch0 clk success %p\r\n", clk_ac8317_wch0);

	clk_ac8317_wch1 = devm_clk_get(&pdev->dev, "wch1-device");
	if (!clk_ac8317_wch1) {
		WCH_LOG(WCH_LOG_LVL_ERR, "get wch1 clk failed %p\r\n", clk_ac8317_wch1);
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "get wch1 clk success %p\r\n", clk_ac8317_wch1);

	pinctrl_wch = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl_wch))
		/*dev_err(&pdev->dev, "wch get pinctrl error!\n");*/
		return -1;

	of_reserved_mem_device_init(&(pdev->dev));
	wch_reserve_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!wch_reserve_mem) {
		WCH_LOG(WCH_LOG_LVL_ERR, "reserve memory get error!\r\n");
		return -1;
	}
	WCH_LOG(WCH_LOG_LVL_INFO, "%s reserve memory base:0x%x, size:0x%x. \r\n",
		wch_reserve_mem->name, wch_reserve_mem->base, wch_reserve_mem->size);
	wch_base_addr = wch_reserve_mem->base;
	WchSetSourceBaseAddr(wch_base_addr);

	ret = misc_register(&wch_dev);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "wch_init: misc_register error %d\r\n", ret);

	ret = os_device_create_file(wch_dev.this_device, &dev_attr_wchcapturescreen);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);

	/* Add wchloglevel variable in /sys/devices/virtual/misc/wch */
	ret = os_device_create_file(wch_dev.this_device, &dev_attr_wchloglevel);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);
#if WCH_DUMP_BUFFER_ATTR
	ret = os_device_create_file(wch_dev.this_device, &dev_attr_wchdumpbuffer);
	if (ret)
		WCH_LOG(WCH_LOG_LVL_ERR, "cannot create dev file %d\r\n", ret);
#endif
	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

#if !NEW_WCH_EVENT_NAME
	if (!WchCreateEvent())
		ret = -1;
#endif

	return ret;
}

static int wch_remove(struct platform_device *pdev)
{
#if !NEW_WCH_EVENT_NAME
	WchCloseEvent();
#endif
	misc_deregister(&(wch_dev));
	os_device_remove_file(wch_dev.this_device, &dev_attr_wchloglevel);
#if WCH_DUMP_BUFFER_ATTR
	os_device_remove_file(wch_dev.this_device, &dev_attr_wchdumpbuffer);
#endif
	os_device_remove_file(wch_dev.this_device, &dev_attr_wchcapturescreen);

	return 0;
}

static const struct of_device_id wch_of_ids[] = {
	{.compatible = "Autochips,writechannel",},
	{}
};

static struct platform_driver wch_plt_drv = {
	.driver = {
		   .name = "Autochips-writechannel",
		   .owner = THIS_MODULE,
		   .of_match_table = wch_of_ids,
		   },
	.probe = wch_probe,
	.remove = wch_remove,
};

static int __init wch_init(void)
{
	int ret;

	WCH_LOG(WCH_LOG_LVL_INFO, "wch_init--->\n");
	ret = platform_driver_register(&wch_plt_drv);
	if (ret){
		WCH_LOG(WCH_LOG_LVL_ERR, "%s: register  driver failed\n", __func__);
	}
#ifndef __ARM2__
	WchEventThreadInit();
#endif

	return ret;
}

static void __exit wch_exit(void)
{
	WCH_LOG(WCH_LOG_LVL_INFO, "wch_exit--->\n");
#ifndef __ARM2__
	kthread_stop(hWchInst[0]);
	kthread_stop(hWchInst[1]);
	WchCloseThreadEvent();
#endif
	platform_driver_unregister(&wch_plt_drv);
}
module_init(wch_init);
module_exit(wch_exit);

MODULE_DESCRIPTION("ATC write channel driver");
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);

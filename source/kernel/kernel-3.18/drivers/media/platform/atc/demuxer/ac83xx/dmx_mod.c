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


#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/module.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/pinctrl/consumer.h>
#include <linux/errno.h>
#include <mach/base_regs.h>

#include "x_os.h"
#include "x_debug.h"
#include "windows.h"
#include "x_ckgen.h"
#include "oal.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/ose_mem.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */

#include "x_hal_ic.h"
#include "dmx_spt.h"
#include "dmx_spt_if.h"
#include "dmx_pbbuf.h"
#include "dmx_gau_if.h"
#include "dmx_stream.h"
#include "dmx_pvr.h"
#include "dmx_pvr_ddi.h"
#include "mmisc.h"
#include "dmx_inst.h"


#define MTK_KERNEL_LINUX_LICENSE	 "GPL"

EXTERN DMX_INST_LIST_T			g_rDmxInstList;

#define VALID_DX(state) (((state) == D0) || ((state) == D1) ||\
	((state) == D2) || ((state) == D3) || ((state) == D4))

#define pr_fmt(fmt) "[MM]["KBUILD_MODNAME"]" fmt

static int demuxer_remove(struct platform_device *pdev);
static int demuxer_probe(struct platform_device *pdev);
static int demuxer_shutdown(struct platform_device *pdev);


#ifdef CONFIG_COMPAT
EXTERN bool DMX_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg);

long demuxer_ioctl_compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	WIN32_IOCTL_DATA win32_ioctl;
	DMX_IOCTL_DATA dmx_ioctl;
	u32 dwContext = 0;
	void *pIn = NULL;
	void *pOut = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == file) {
		DMX_ASSERT(FALSE);
		pr_debug("[DMX] %s fail for struct file is NULL!\r\n", DMX_FUNC_NAME);
		return -1;
	}
	if (!file->f_op->unlocked_ioctl)
		return -1;

	if (0 == arg) {
		pr_err("[DMX] %s fail for arg is NULL!\r\n", DMX_FUNC_NAME);
	}
	
	dwContext = (uintptr_t) (file->private_data);

	if (_IOC_TYPE(cmd) == 'D') {
		if (!DMX_IOControl_Compat(file, cmd, arg))
			return -1;
	} else {
		pr_err("[DMX] %s fail for DMX cmd!\r\n", DMX_FUNC_NAME);
	}
	
	return 0;
}

EXPORT_SYMBOL(demuxer_ioctl_compat);

#endif     //CONFIG_COMPAT

int dmx_suspend(struct device *dev)
{
	DMX_PM_STATE eDmxPmState = D0;

	MRESULT mrRet = RET_DMX_OK;

	eDmxPmState = SplitterGetPowerState();

	pr_debug("[dmx] dmx_suspend -- eDmxPmState: %d!\r\n", eDmxPmState);

	if (D4 == eDmxPmState) {
		pr_debug("[dmx] dmx_suspend exit for Demux Hw Power State already = D4!\r\n");
		return 0;
	}

	mrRet = SplitterSetPowerState(D4);

	if (DMX_FAILED(mrRet)) {
		pr_debug("[dmx] dmx_pm suspend failed in SplitterSetPowerState(D4)!\r\n");
		return 0;
	}

	pr_debug("[dmx] dmx_suspend exit, success!\r\n");

	return 0;
}

int dmx_resume(struct device *dev)
{
	DMX_PM_STATE eDmxPmState = D0;
	MRESULT mrRet = RET_DMX_OK;

	eDmxPmState = SplitterGetPowerState();

	pr_debug("[dmx] dmx_resume -- eDmxPmState: %d!\r\n", eDmxPmState);

	if (D0 == eDmxPmState) {
		pr_debug("[dmx] dmx_resume exit for Demux Hw Power State already = D0!\r\n");
		return 0;
	}

	mrRet = SplitterSetPowerState(D0);

	if (DMX_FAILED(mrRet)) {
		pr_err("[dmx] dmx_resume failed in SplitterSetPowerState(D0)!\r\n");
		return 0;
	}

	pr_debug("[dmx] dmx_resume exit, success!\r\n");

	return 0;
}

static const struct dev_pm_ops dmx_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dmx_suspend, dmx_resume)
};
static int demuxer_open(struct inode *inode, struct file *flip)
{
	void *dwContext = NULL;

	pr_debug("[DMX] %s enter!\r\n", DMX_FUNC_NAME);
	dwContext = DMX_Open();
	if (NULL == dwContext) {
		pr_debug("[DMX] %s fail in DMX_Open()!\r\n", DMX_FUNC_NAME);
		return -1;
	}

	flip->private_data = (void *)dwContext;
	pr_debug("[DMX] %s success!\r\n", DMX_FUNC_NAME);
	return 0;
}

static int demuxer_release(struct inode *inode, struct file *file)
{
	void *dwContext = NULL;

	if (NULL == file) {
		DMX_ASSERT(FALSE);
		pr_debug("[DMX] %s fail for struct file is NULL!\r\n", DMX_FUNC_NAME);
		return -1;
	}

	dwContext = file->private_data;

	if (!DMX_Close(dwContext)) {
		DMX_ASSERT(FALSE);
		pr_debug("[DMX] %s fail in DMX_Close(context: 0x%p)!\r\n",
			 DMX_FUNC_NAME, dwContext);
		return -1;
	}

	file->private_data = NULL;

	pr_debug("[DMX] %s success!\r\n", DMX_FUNC_NAME);
	return 0;
}

long demuxer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void *dwContext = NULL;

	if (NULL == file) {
		DMX_ASSERT(FALSE);
		pr_debug("[DMX] %s fail for struct file is NULL!\r\n", DMX_FUNC_NAME);
		return -1;
	}
	if (!file->f_op->unlocked_ioctl)
		return -1;

	if (0 == arg) {
		pr_err("[DMX] %s fail for arg is NULL!\r\n", DMX_FUNC_NAME);
	}
	
	dwContext = (void *) (file->private_data);
	if (_IOC_TYPE(cmd) == 'D') {
		if (!DMX_IOControl(dwContext, cmd, arg, TRUE))
			return -1;
	} else {
		pr_err("[DMX] %s fail for DMX cmd!\r\n", DMX_FUNC_NAME);
	}
	
	return 0;
}

EXPORT_SYMBOL(demuxer_ioctl);

static int demuxer_mmap(struct file *fp, struct vm_area_struct *vma)
{
	/*printk("enter demuxer_mmap vma->vm_pgoff=0x%x\n", vma->vm_pgoff); */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	return 0;
}

const struct file_operations demuxer_fops = {
	.release = demuxer_release,
	.mmap = demuxer_mmap,
	.open = demuxer_open,
	.unlocked_ioctl = demuxer_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = demuxer_ioctl_compat
#endif
};

#define DEMUXER_DEVNAME	"demuxer"

struct dmx_dev_info *g_dmxdevinfo = NULL;

static const struct of_device_id dmx_of_ids[] = {
	{.compatible = "atc,demuxer",},
	{}
};

static struct platform_driver dmx_of_driver = {
	.driver = {.name = "ac83xx_demuxer",
		   .owner = THIS_MODULE,
		   .pm = &dmx_pm_ops,
		   .of_match_table = dmx_of_ids,
		   },
	.probe = demuxer_probe,
	.remove = demuxer_remove,
	.shutdown = demuxer_shutdown
};

static irqreturn_t ac83xx_dmx_ftup_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct dmx_dev_info *dmx_dev = platform_get_drvdata(pdev);

	if (NULL == dmx_dev) {
		pr_err("fail for invalid drv data!\r\n");
		return IRQ_NONE;
	}

	_DmxIrqHandler(dmx_dev->ftup_irq);

	return IRQ_HANDLED;
}

static irqreturn_t ac83xx_dmx_ddi_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct dmx_dev_info *dmx_dev = platform_get_drvdata(pdev);

	if (NULL == dmx_dev) {
		pr_err("fail for invalid drv data!\r\n");
		return IRQ_NONE;
	}

	_PVR_DDI_IrqHandler(dmx_dev->ddi_irq);

	return IRQ_HANDLED;
}

static int demuxer_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct dmx_dev_info *dmx_dev;
	int result = -1;

	pr_debug("demuxer_probe enter!!\r\n");

	if (!node) {
		pr_err("demuxer_probe fail for no demuxer device compatible dts node!\r\n");
		return -1;
	}

	dmx_dev = kzalloc(sizeof(struct dmx_dev_info), GFP_KERNEL);
	if (!dmx_dev) {
		result = -ENOMEM;
		goto err_free_mem;
	}

	g_dmxdevinfo = dmx_dev;

	dmx_dev->dmx_top_clk = devm_clk_get(&pdev->dev, "demuxer-topselclk");
	if (IS_ERR(dmx_dev->dmx_top_clk)) {
		pr_err("get dmx topsel clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_ts0_in_clk = devm_clk_get(&pdev->dev, "demuxer-ts0selclk");
	if (IS_ERR(dmx_dev->dmx_ts0_in_clk)) {
		pr_err("get dmx ts0 sel clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_ts1_in_clk = devm_clk_get(&pdev->dev, "demuxer-ts1selclk");
	if (IS_ERR(dmx_dev->dmx_ts1_in_clk)) {
		pr_err("get dmx ts0 sel clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_gateclk = devm_clk_get(&pdev->dev, "demuxer-clk");
	if (IS_ERR(dmx_dev->dmx_gateclk)) {
		pr_err("get dmx gate clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_27m_gateclk = devm_clk_get(&pdev->dev, "demuxer-27m-clk");
	if (IS_ERR(dmx_dev->dmx_27m_gateclk)) {
		pr_err("get dmx 27m gate clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_ts0_gateclk = devm_clk_get(&pdev->dev, "demuxer-ts0-clk");
	if (IS_ERR(dmx_dev->dmx_ts0_gateclk)) {
		pr_err("get dmx ts0 gate clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_ts1_gateclk = devm_clk_get(&pdev->dev, "demuxer-ts1-clk");
	if (IS_ERR(dmx_dev->dmx_ts1_gateclk)) {
		pr_err("get dmx ts1 gate clk error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->pinctrl_demuxer = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(dmx_dev->pinctrl_demuxer)) {
		pr_err("get demuxer pin control error!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dmx_base0_regs = of_iomap(node, 0);
	if (!dmx_dev->dmx_base0_regs) {
		pr_err("fail in unable to iomap base0 registers!!\r\n");
		goto err_free_mem;
	}
	dmx_dev->dmx_base1_regs = of_iomap(node, 1);
	if (!dmx_dev->dmx_base1_regs) {
		pr_err("fail in unable to iomap base1 registers!!\r\n");
		goto err_free_mem;
	}
	dmx_dev->dmx_base2_regs = of_iomap(node, 2);
	if (!dmx_dev->dmx_base2_regs) {
		pr_err("fail in unable to iomap base2 registers!!\r\n");
		goto err_free_mem;
	}
	dmx_dev->dmx_base8_regs = of_iomap(node, 3);
	if (!dmx_dev->dmx_base8_regs) {
		pr_err("fail in unable to iomap base8 registers!!\r\n");
		goto err_free_mem;
	}
	dmx_dev->dmx_ddi_regs = of_iomap(node, 4);
	if (!dmx_dev->dmx_ddi_regs) {
		pr_err("fail in unable to iomap ddi registers!!\r\n");
		goto err_free_mem;
	}

	pr_info("demuxer --> dmx_regs [0: 0x%08x] [1: 0x%08x] [2: 0x%08x] !!\r\n",
		(unsigned int)(dmx_dev->dmx_base0_regs),
		(unsigned int)(dmx_dev->dmx_base1_regs),
		(unsigned int)(dmx_dev->dmx_base2_regs));
	pr_info("demuxer --> dmx_regs [8: 0x%08x] [ddi: 0x%08x] !!\r\n",
		(unsigned int)(dmx_dev->dmx_base8_regs),
		(unsigned int)(dmx_dev->dmx_ddi_regs));

	dmx_dev->ftup_irq = irq_of_parse_and_map(node, 0);
	if (dmx_dev->ftup_irq == NO_IRQ) {
		pr_err("demuxer_probe fail in get dmx ftup irq id!\r\n");
		goto err_free_mem;
	}
	dmx_dev->ddi_irq = irq_of_parse_and_map(node, 1);
	if (dmx_dev->ddi_irq == NO_IRQ) {
		pr_err("demuxer_probe fail in get dmx ddi irq id!\r\n");
		goto err_free_mem;
	}

	dmx_dev->dev = &(pdev->dev);
	dmx_dev->cdev.name = DEMUXER_DEVNAME;
	dmx_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	dmx_dev->cdev.fops = &demuxer_fops;

	platform_set_drvdata(pdev, dmx_dev);

	if (0 == DMX_Init(NULL, 0)) {
		pr_err("demuxer_probe fail in DMX_Init!\r\n");
		result = -1;
		goto err_unset_drvdata;
	}

	result = misc_register(&(dmx_dev->cdev));

	if (result != 0) {
		pr_err("demuxer_probe fail in misc_register, error = %d\r\n", result);
		DMX_Deinit();
		goto err_unset_drvdata;
	}

	result = request_irq(dmx_dev->ddi_irq, ac83xx_dmx_ddi_interrupt,
		0, DEMUXER_DEVNAME, pdev);
	if (result) {
		pr_err("demuxer_probe fail in request ddi irq(id: %d)\r\n",
			dmx_dev->ddi_irq);
		DMX_Deinit();
		misc_deregister(&(dmx_dev->cdev));
		goto err_unset_drvdata;
	}
	disable_irq(dmx_dev->ddi_irq);

	result = request_irq(dmx_dev->ftup_irq, ac83xx_dmx_ftup_interrupt,
			     0, DEMUXER_DEVNAME, pdev);
	if (result) {
		pr_err("demuxer_probe fail in request ftup irq(id: %d)\r\n",
			dmx_dev->ftup_irq);
		free_irq(dmx_dev->ddi_irq, pdev);
		DMX_Deinit();
		misc_deregister(&(dmx_dev->cdev));
		goto err_unset_drvdata;
	}
	disable_irq(dmx_dev->ftup_irq);

	pr_info("demuxer device probe ok!!\r\n");

	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_mem:
	kfree(dmx_dev);

	g_dmxdevinfo = NULL;

	return result;
}

static int demuxer_remove(struct platform_device *pdev)
{
	struct dmx_dev_info *dmx_dev = platform_get_drvdata(pdev);

	free_irq(dmx_dev->ddi_irq, pdev);
	free_irq(dmx_dev->ftup_irq, pdev);

	if (!DMX_Deinit()) {
		pr_debug("remove demuxer device fail in DMX_Deinit!\r\n");
		return -1;
	}

	misc_deregister(&(dmx_dev->cdev));

	platform_set_drvdata(pdev, NULL);

	g_dmxdevinfo = NULL;

	kfree(dmx_dev);

	return 0;
}
static int demuxer_shutdown(struct platform_device *pdev)
{
	DMX_PM_STATE eDmxPmState = D0;

	pr_info("[dmx] -- demuxer_shutdown start --!\r\n");

	eDmxPmState = SplitterGetPowerState();
	if (D4 == eDmxPmState) {
		pr_info("[dmx] demuxer has been closed!\r\n");
		return 0;
	}

	pr_info("[dmx] -- demuxer_shutdown end --!\r\n");
	return 0;
}

static int __init demuxer_init(void)
{
	struct device_node *node = NULL;
	int result = 0;

	pr_debug("demuxer_init enter!!\r\n");

	node = of_find_compatible_node(NULL, NULL, "atc,demuxer");
	if (!node) {
		pr_debug("demuxer_init fail in get demuxer driver dts compatible node!!\r\n");
		result = -ENOMEM;
		goto err_node;
	}

	result = platform_driver_register(&dmx_of_driver);
	if (result) {
		pr_err("[DMX] demuxer_init fail in platform_driver_register, error = %d\r\n",
		       result);
		goto err_node;
	}

	pr_info("demuxer device init success!!\r\n");

	return 0;

err_node:

	return result;
}

static void __exit demuxer_exit(void)
{
	pr_debug("demuxer_exit enter!!\r\n");

	platform_driver_unregister(&dmx_of_driver);

	pr_debug("demuxer_exit exit!!\r\n");
}


module_init(demuxer_init);
module_exit(demuxer_exit);

MODULE_AUTHOR("Autochips");
MODULE_DESCRIPTION("ATC ac83xx Demuxer Driver");
MODULE_LICENSE("GPL");

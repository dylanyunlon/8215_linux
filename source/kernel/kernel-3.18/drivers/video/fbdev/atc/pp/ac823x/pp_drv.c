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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/pm.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <generated/atc_project.h>
#include "pp_log.h"
#include "pp_if.h"
#include "oal.h"

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define ATC_PP_DRIVER_INFO	"ATC post process driver"

struct clk *clk_ppf;
struct clk *clk_ppr;
void __iomem *ppf_reg;
void __iomem *ppr_reg;
unsigned int ppfREG[0x114 / 4] = {0};
unsigned int pprREG[0x114 / 4] = {0};

/*set post fmt begin*/
static ssize_t pp_pf_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_pf_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0, videoPath, pictureSize;

	PP_LOG(PP_LOG_LVL_INFO, "	Parameters: [VideoPath, PicSize]\n");
	PP_LOG(PP_LOG_LVL_INFO, "		VideoPath:\n");
	PP_LOG(PP_LOG_LVL_INFO, "			0:Front; 1:Rear\n");
	PP_LOG(PP_LOG_LVL_INFO, "		PicSize:\n");
	PP_LOG(PP_LOG_LVL_INFO, "			1:1080P; 2:720P; 3:1024x600; 4:800x480; 5:800x600; 6:480P 7:480I 8:576P 9:576I 10:1080I ");

	ret = sscanf(buf, "%d %d", (unsigned int *)&videoPath, (unsigned int *)&pictureSize);
	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	PostFmt(videoPath, pictureSize);

	return count;

}
static DEVICE_ATTR(pp4pf, S_IWUSR | S_IRUGO, pp_pf_show, pp_pf_store);
/*set post fmt end*/

/*Enable/disable Sharpness begin*/
static ssize_t pp_shp_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_shp_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0;
	unsigned char UiMin, UiMax, UiDft, UiCur;

	PP_LOG(PP_LOG_LVL_INFO, "	Usage: UiMin, UiMax, UiDft, UiCur\n\n");

	ret = sscanf(buf, "%d %d %d %d", (unsigned char *)&UiMin, (unsigned char *)&UiMax, (unsigned char *)&UiDft, (unsigned char *)&UiCur);
	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	PostSharpness (UiMin, UiMax, UiDft, UiCur);

	return count;

}
static DEVICE_ATTR(pp4shp, S_IWUSR | S_IRUGO, pp_shp_show, pp_shp_store);
/*Enable/disable Sharpness end*/

/*Set sharp band para begin*/
static ssize_t pp_ssbp_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_ssbp_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0;
	POST_SHN_BAND_PARA eBandPara;

	PP_LOG(PP_LOG_LVL_INFO, "	Usage: [Band], [coring], [gain], [limit_pos], [limit_neg], [clip_en], [clip_pos], [clip_neg]\n");
	PP_LOG(PP_LOG_LVL_INFO, "		   [Band]: 1: H1, 2: V, 3: X1, 4: X2, 5: H2, 6: H3\n");

	ret = sscanf(buf, "%d %d %d %d %d %d %d %d",
		(POST_SHN_BAND_T *)&eBandPara.eShnBand,
		(unsigned char *)&eBandPara.bCoring,
		(unsigned char *)&eBandPara.bGain,
		(unsigned char *)&eBandPara.bLimitPos,
		(unsigned char *)&eBandPara.bLimitNeg,
		(unsigned char *)&eBandPara.bClipEn,
		(unsigned char *)&eBandPara.bClipThPos,
		(unsigned char *)&eBandPara.bClipThNeg);

	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	PostSharpParaSet (eBandPara);

	return count;

}
static DEVICE_ATTR(pp4ssbp, S_IWUSR | S_IRUGO, pp_ssbp_show, pp_ssbp_store);
/*Set sharp band para end*/

/*Set sharp ctrl begin*/
static ssize_t pp_ssc_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_ssc_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0;
	POST_SHN_CTRL_PARA eCtrlPara;

	PP_LOG(PP_LOG_LVL_INFO, "	Usage: [Enable], [limit_all_pos], [limit_all_neg]\n");

	ret = sscanf(buf, "%d %d %d",
		(unsigned char *)&eCtrlPara.fgShnEn, 
		(unsigned short *)&eCtrlPara.bLimitAllPos, 
		(unsigned short *)&eCtrlPara.bLimitAllNeg);

	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	PostSharpCtrlSet(eCtrlPara);

	return count;

}
static DEVICE_ATTR(pp4ssc, S_IWUSR | S_IRUGO, pp_ssc_show, pp_ssc_store);
/*Set sharp ctrl end*/

/*Enable/disable CTI begin*/
static ssize_t pp_cti_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_cti_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0;
	unsigned char UiMin, UiMax, UiDft, UiCur;

	PP_LOG(PP_LOG_LVL_INFO, "	Usage: UiMin, UiMax, UiDft, UiCur\n\n");

	ret = sscanf(buf, "%d %d %d %d", (unsigned char *)&UiMin, (unsigned char *)&UiMax, (unsigned char *)&UiDft, (unsigned char *)&UiCur);
	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	PostCTI(UiMin, UiMax, UiDft, UiCur);

	return count;
}
static DEVICE_ATTR(pp4cti, S_IWUSR | S_IRUGO, pp_cti_show, pp_cti_store);
/*Enable/disable CTI end*/

/*Set CTI ctrl begin*/
static ssize_t pp_ctis_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return 0;
}

static ssize_t pp_ctis_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned int ret = 0;
	POST_CTI_CTRL_PARA eCtrlPara;

	PP_LOG(PP_LOG_LVL_INFO, "	Usage: ctis CTIStb CTILpfSel<0 or 1 or 2> CTIEn<0 or 1>\n");

	ret = sscanf(buf, "%d %d %d", 
		(unsigned short *)&eCtrlPara.bECTIStbSel,
		(unsigned char *)&eCtrlPara.bECTIFlpfSel,
		(unsigned char *)&eCtrlPara.fgCtiEn);
	if (ret < 0) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_pf_store sscanf error!\n");
		return -EINVAL;
	}

	CTICtrlSet(eCtrlPara);

	return count;
}
static DEVICE_ATTR(pp4ctis, S_IWUSR | S_IRUGO, pp_ctis_show, pp_ctis_store);
/*Set CTI ctrl end*/

static long pp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	PP_LOG(PP_LOG_LVL_INFO, "pp_ioctl not be implemented \n");
	/*to do*/

	/*****/
}

struct file_operations const pp_fops = {
	.unlocked_ioctl = pp_ioctl,
};

static struct miscdevice pp_dev = {
	MISC_DYNAMIC_MINOR,
	"pp",
	&pp_fops
};

extern unsigned long _IO_BASE_;
static int pp_probe(struct platform_device *pdev)
{
	unsigned int ret = -EINVAL;
	struct device_node *node;
	struct device_node *nd = pdev->dev.of_node;
	unsigned int property[4];
	unsigned long propertytmp;
	unsigned int size;
	unsigned long io_base = 0x10000000;
	unsigned int io_size = 0x100000;
	PP_LOG(PP_LOG_LVL_INFO, "pp_probe--->\n");

	_IO_BASE_ = ioremap(io_base, io_size);
	if (!_IO_BASE_) {
		PP_LOG(PP_LOG_LVL_ERR, "get io base address failed = %p \r\n",
					_IO_BASE_);
		goto err;
	}
	PP_LOG(PP_LOG_LVL_INFO, "get io base address _IO_BASE_ = %lx \r\n",_IO_BASE_);

	ppf_reg = of_iomap(nd, 0);
	if (!ppf_reg) {
		PP_LOG(PP_LOG_LVL_ERR, "get ppf reg base address failed = %x \r\n"
			, (unsigned int)ppf_reg);
		goto err;
	}

	ppr_reg = of_iomap(nd, 1);
	if (!ppr_reg) {
		PP_LOG(PP_LOG_LVL_ERR, "get ppr reg base address failed = %x \r\n"
			, (unsigned int)ppr_reg);
		goto err;
	}

	clk_ppf = devm_clk_get(&pdev->dev, "ppf-clock");
	if (!clk_ppf) {
		PP_LOG(PP_LOG_LVL_ERR, "get front pp clock failed %x\r\n", (unsigned int)clk_ppf);
		goto err;
	}

	clk_ppr = devm_clk_get(&pdev->dev, "ppr-clock");
	if (!clk_ppr) {
		PP_LOG(PP_LOG_LVL_ERR, "get rear pp clock failed %x\r\n", (unsigned int)clk_ppr);
		goto err;
	}
#ifndef CONFIG_ATC_PRJ_ac823x_adas
	clk_prepare_enable(clk_ppf);
	clk_prepare_enable(clk_ppr);
#endif
	ret = misc_register(&pp_dev);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "pp_probe: misc_register error %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4pf);
	if (ret)
		PP_LOG(PP_LOG_LVL_ERR, "cannot create pf dev file %d\r\n", ret);

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4shp);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "cannot create shp dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4ssbp);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "cannot create ssbp dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4ssc);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "cannot create ssc dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4cti);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "cannot create cti dev file %d\r\n", ret);
		goto err;
	}

	ret = os_device_create_file(pp_dev.this_device, &dev_attr_pp4ctis);
	if (ret) {
		PP_LOG(PP_LOG_LVL_ERR, "cannot create ctis dev file %d\r\n", ret);
		goto err;
	}
	//SetPostProcess(0);

err:
	return ret;

}

static int pp_remove(struct platform_device *pdev)
{
	PP_LOG(PP_LOG_LVL_INFO, "pp_remove<---\n");

	misc_deregister(&pp_dev);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4pf);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4shp);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4ssbp);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4ssc);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4cti);
	os_device_remove_file(pp_dev.this_device, &dev_attr_pp4ctis);

	return 0;
}

#ifdef CONFIG_PM
static int pp_suspend(struct device *dev)
{
        PP_LOG(PP_LOG_LVL_INFO, "pp_suspend--->\n");

	memcpy(ppfREG, (unsigned int*)ppf_reg, 0x114);
	memcpy(pprREG, (unsigned int*)ppr_reg, 0x114);

	clk_disable_unprepare(clk_ppf);
	clk_disable_unprepare(clk_ppr);

	return 0;
}

static int pp_resume(struct device *dev)
{
	unsigned int VideoPath = PP_FRONT;
	PP_DISPLAY_MODE_E eDisplayMode = RES_1024X600;
        PP_LOG(PP_LOG_LVL_INFO, "pp_resume--->\n");

	clk_prepare_enable(clk_ppf);
	clk_prepare_enable(clk_ppr);

	PpInit(VideoPath, eDisplayMode);
	memcpy((unsigned int*)ppf_reg, ppfREG, 0x114);
	memcpy((unsigned int*)ppr_reg, pprREG, 0x114);

	return 0;
}

#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops pp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(pp_suspend, pp_resume)
};
#endif

static const struct of_device_id pp_of_ids[] = {
	{.compatible = "Autochips,postprocess",},
	{}
};

static struct platform_driver pp_plt_drv = {
	.driver = {
		   .name = "Autochips-postprocess",
		   .owner = THIS_MODULE,
		   .of_match_table = pp_of_ids,
#ifdef CONFIG_PM
		   .pm = &pp_pm_ops,
#endif
		   },
	.probe = pp_probe,
	.remove = pp_remove,
};

static int __init pp_init(void)
{
	int ret;

	PP_LOG(PP_LOG_LVL_INFO, "[PP]:pp_init--->\n");
	ret = platform_driver_register(&pp_plt_drv);
	if (ret)
		PP_LOG(PP_LOG_LVL_ERR, "[PP]: %s: register  driver failed\n", __func__);
}

static void __exit pp_exit(void)
{
	PP_LOG(PP_LOG_LVL_INFO, "pp_exit<---\n");

	platform_driver_unregister(&pp_plt_drv);
}
module_init(pp_init);
module_exit(pp_exit);

MODULE_DESCRIPTION(ATC_PP_DRIVER_INFO);
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);


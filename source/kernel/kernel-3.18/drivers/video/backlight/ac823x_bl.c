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

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/types.h>
//#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
//#include <linux/gpio/consumer.h>
#include <linux/backlight.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
//#include <mach/cache_operation.h>
/* #include <asm/system.h> */
//#include <mach/ac83xx_pinmux_table.h>
//#include "x_bim_83xx.h"
#include "pwm.h"
#include <generated/atc_project.h>

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
struct bkl_device {
	struct miscdevice cdev;	/* Char device structure */
	uint32_t dwbklInst;
#ifdef CONFIG_PM
	struct device *dev;
#endif
};

static struct bkl_device *g_bkldev;
static DEFINE_MUTEX(bkl_mutex);

#define WriteREG(arg, val) ((*(volatile u32*)(IO_BASE_VA + (arg))) = val)
#define ReadREG(arg)       (*(volatile u32*)(IO_BASE_VA + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define DISPLAY_SET_BKL_INTENSITY               (0x00020008)
#define DISPLAY_GET_BKL_INTENSITY               (0x00020021)
#define DISPLAY_SET_BKL_SHUTDOWN								(0x00020027)

int vGetBacklightIntensity(void)
{
	int value = 0;
	/*to do...*/
	return value;
}

void vSetBacklightIntensity(__u32 u4Value);
/* #define PERFTEST */
static long atc_bkl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;

	mutex_lock(&bkl_mutex);

	switch (cmd) {
	case DISPLAY_SET_BKL_INTENSITY:
		vSetBacklightIntensity(arg);
		break;

	case DISPLAY_GET_BKL_INTENSITY:
		{
			__s32 tconvalue = 0;

			tconvalue = vGetBacklightIntensity();
			if (copy_to_user((void __user *)arg, &tconvalue, sizeof(__s32))) {
				err  = -EINVAL;
				pr_err("copy_to_user return %d iocode %d\n", err, cmd);
			}
			break;
		}

	case DISPLAY_SET_BKL_SHUTDOWN:
		if (arg == 1)
			vSetBacklightIntensity(0);
		break;

	default:
		err  = -EINVAL;
		pr_err("atc_bkl_ioctl do not support this cmd = 0x%x", cmd);
		break;
	}

	mutex_unlock(&bkl_mutex);

	return err;
}

static int atc_bkl_open(struct inode *inode, struct file *file)
{
	pr_info("bkl_open\n");
	pr_info("current pgd %p\n", (unsigned long *)((current->mm)->pgd));
	return 0;
}

static int atc_bkl_release(struct inode *inode, struct file *file)
{
	pr_info("bkl_release\n");
	return 0;
}

static const struct file_operations bkl_fops = {
	.owner = THIS_MODULE,
	.open = atc_bkl_open,
	.release = atc_bkl_release,
	.unlocked_ioctl = atc_bkl_ioctl,
};

static int ac83xx_backlight_update_status(struct backlight_device *bl)
{
	int brightness = bl->props.brightness;
	
	if (bl->props.power != FB_BLANK_UNBLANK ||
			bl->props.fb_blank != FB_BLANK_UNBLANK ||
	    bl->props.state & BL_CORE_FBBLANK)
		brightness = 0;

	if (brightness > 0)
        #ifdef CONFIG_ATC_OS_linux                
        vSetBacklightIntensity((__u32)brightness);
        #endif

    return 0;
}

static int ac83xx_backlight_check_fb(struct backlight_device *bl,
				  struct fb_info *info)
{
	return 0;
}

static const struct backlight_ops ac83xx_backlight_ops = {
	.update_status	= ac83xx_backlight_update_status,
	.check_fb	= ac83xx_backlight_check_fb,
};

static int __init bkl_init(void);
static int bkl_probe(struct platform_device *pdev)
{
	struct bkl_device *bkldev;
	struct backlight_properties props;
	struct backlight_device *bl;
	int result;
        pr_err("[xzr] flow __1\n");
	bkldev = kzalloc(sizeof(struct bkl_device), GFP_KERNEL);
	if (bkldev == NULL) {
		/* dev_err(&pdev->dev, "[bkl]: bkl_probe: malloc device failed\n"); */
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, bkldev);
	bkldev->cdev.name = "bkl";
	bkldev->cdev.minor = MISC_DYNAMIC_MINOR;
	bkldev->cdev.fops = &bkl_fops;
        pr_err("[xzr] flow __2\n");
#ifdef CONFIG_PM
	bkldev->dev = &(pdev->dev);

#endif
	bkldev->cdev.parent = &(pdev->dev);
	result = misc_register(&(bkldev->cdev));
	if (result == 0)
		pr_info("bkl init successes\n");
	else{
		pr_err("bkl misc device register error\n");
		kfree(bkldev);
		return result;
	}
	g_bkldev = bkldev;

#ifdef CONFIG_PM
	pm_runtime_enable(g_bkldev->dev);

#endif

        pr_err("[xzr] flow __3\n");
	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = 100;
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, NULL,
				       &ac83xx_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "[bkl]failed to register backlight\n");
		result = PTR_ERR(bl);
		return result;
	}

	bl->props.brightness = 64;//default for linux
	backlight_update_status(bl);

        pr_err("[xzr] flow __4\n");
	platform_set_drvdata(pdev, bl);

	/* bkl_init(); */
	return 0;
}


/* static int __devexit bkl_remove(struct platform_device *pdev) */
static int bkl_remove(struct platform_device *pdev)
{
	struct bkl_device *bkldev = platform_get_drvdata(pdev);
	struct backlight_device *bl = platform_get_drvdata(pdev);

	backlight_device_unregister(bl);

	if (bkldev == NULL) {
		pr_err("No device when bkl remove!\n");
		return -ENODEV;
	}
	misc_deregister(&(bkldev->cdev));
	kfree(g_bkldev);
	g_bkldev = NULL;

	return 0;
}


#ifdef CONFIG_PM
static int atc_bkl_suspend(struct device *dev)
{
	struct bkl_device *bkldev;
	struct platform_device *pdev = to_platform_device(dev);

	bkldev = platform_get_drvdata(pdev);
	if (bkldev == NULL)
		return -ENODEV;
	pr_info("[bkl]: %s\n", __func__);
	return 0;
}

//struct pinctrl *pinctrl_bkl = NULL;
//struct pinctrl_state *pins_bkl_set_gpio124 = NULL;
//struct gpio_desc *bklGPIO = NULL;
//const char * const bklgpiofct = "backlight";

static int atc_bkl_resume(struct device *dev)
{
	/*
	struct bkl_device *bkldev;
	struct platform_device *pdev = to_platform_device(dev);
	int ret = 0;

	bkldev = platform_get_drvdata(pdev);
	if (bkldev == NULL)
		return -ENODEV;

	pinctrl_bkl = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrl_bkl))
		dev_err(&pdev->dev, "wch get pinctrl error!\n");
		return -1;

	pins_bkl_set_gpio124 = pinctrl_lookup_state(pinctrl_bkl, "bkl_set_gpio124_out");
	if (IS_ERR(pins_bkl_set_gpio124))
		pr_err("atc_bkl_resume pin state gpio124 err %p\r\n",
			pins_bkl_set_gpio124);
	ret = pinctrl_select_state(pinctrl_bkl, pins_bkl_set_gpio124);
	if (ret)
		pr_err("WchHalSetPinmux pin select gpio178 err ret=%d\r\n",
			ret);

	bklGPIO = __gpiod_get(dev, bklgpiofct, GPIOD_ASIS);
	if (IS_ERR(bklGPIO)) {
		pr_err("backlight:can not get gpio %s\r\n", bklgpiofct);
		return -1;
	}

	ret = gpiod_direction_output(bklGPIO, 0);
	if (ret) {
		pr_err("can not look state backlight-gpios\r\n");
		return -1;
	}

	pr_info("set PIN_124_GPIO124 low!\r\n");
	*/
	pr_info("[bkl]: %s\n", __func__);
	return 0;
}


#ifdef CONFIG_PM_RUNTIME
static int atc_bkl_runtime_suspend(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}

static int atc_bkl_runtime_resume(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}

static int atc_bkl_runtime_idle(struct device *dev)
{
	/*
	struct platform_device *pdev = to_platform_device(dev);
	*/
	return 0;
}


#endif
#endif

#ifdef CONFIG_PM_SLEEP
static int atc_bkl_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
	pr_info("[bkl]: %s\n", __func__);
	return 0;
}

static int atc_bkl_legacy_resume(struct platform_device *dev)
{
	pr_info("[bkl]: %s\n", __func__);
	return 0;
}


#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops atc_bkl_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(atc_bkl_suspend, atc_bkl_resume)
#ifdef CONFIG_PM_RUNTIME
	SET_RUNTIME_PM_OPS(atc_bkl_runtime_suspend, atc_bkl_runtime_resume,
			       atc_bkl_runtime_idle)
#endif
};


#endif
static const struct of_device_id bkl_of_ids[] = {
	{.compatible = "Autochips,backlight",},
	{}
};

static void bkl_shutdown(struct platform_device *dev)
{
	vSetBacklightIntensity(0);
}

static struct platform_driver bkl_plt_drv = {
	.driver = {
		.name = "ac83xx-backlight",
		.owner = THIS_MODULE,
		.of_match_table = bkl_of_ids,
#ifdef CONFIG_PM
		.pm = &atc_bkl_dev_pm_ops,
#endif
		},
	.probe = bkl_probe,
	/* .remove = __devexit_p(bkl_remove), */
	.remove = bkl_remove,
#ifdef CONFIG_PM_SLEEP
	.suspend = atc_bkl_legacy_suspend,
	.resume = atc_bkl_legacy_resume,
#endif
	.shutdown = bkl_shutdown
};

void vSetPWM(u32 dty_cyc)
{

	u32 u4PwmRsn = 0xFFF;
	u32 u4PwmP = 0x4;
	u32 u4PwmH = 0x800;
	u32 u4PwmH_FB = 0x800;
	pwm_device *bl_en = NULL;
	pwm_device *bl_fb = NULL;
	pwm_config_code pwm_bl[2];

	memset(pwm_bl, 0, 2 * sizeof(pwm_config_code));

	/************************************************************
	*                   Step3:  Configure Backlight PWM                               *
	************************************************************/
	u4PwmRsn = (269) & 0xFFF;
	dty_cyc = (dty_cyc > 100) ? 100 : dty_cyc;
	u4PwmH = (dty_cyc * u4PwmRsn / 100) & 0xFFF;
	u4PwmH_FB = ((100 - dty_cyc) * u4PwmRsn / 100) & 0xFFF;

	bl_en = pwm_request(3, "bl_en");
	bl_fb = pwm_request(1, "bl_fb");

	/* pwm_bl en setting */
	pwm_bl[0].clk_id = CLKSRC_27M, pwm_bl[0].pin_id = 1;	/* GPIO 125 output en */
	pwm_bl[0].pwm_en = 1;
	pwm_bl[0].pwm_high = u4PwmH;
	pwm_bl[0].pwm_prescale = u4PwmP;
	pwm_bl[0].pwm_rsn = u4PwmRsn;
	pwm_bl[0].pwm_mode = SYNC_TRI_MODE;

	/* pwm_bl fb setting */
	pwm_bl[1].clk_id = CLKSRC_27M, pwm_bl[1].pin_id = 1;	/* GPIO 150 output en */
	pwm_bl[1].pwm_en = 1;
	pwm_bl[1].pwm_high = u4PwmH_FB;
	pwm_bl[1].pwm_prescale = u4PwmP;
	pwm_bl[1].pwm_rsn = u4PwmRsn;
	pwm_bl[1].pwm_mode = SYNC_TRI_MODE;
	memcpy(&(bl_en->pwm_cfg_code), &(pwm_bl[0]), sizeof(pwm_config_code));
	memcpy(&(bl_fb->pwm_cfg_code), &(pwm_bl[1]), sizeof(pwm_config_code));
	if (bl_en->running) {
		pwm_set_intensity(bl_en, dty_cyc);
		pwm_mode_switch(bl_en, SYNC_TRI_MODE);
	} else {
		pwm_config(bl_en, 0, 0);
	}
	if (bl_fb->running) {
		pwm_set_intensity(bl_fb, (100 - dty_cyc));
		pwm_mode_switch(bl_fb, SYNC_TRI_MODE);
	} else {
		pwm_config(bl_fb, 0, 0);
	}
	pr_info("[bkl_drv][wts] set bk light %d\n", (int)dty_cyc);
}

void vSetBacklightIntensity(__u32 u4Value)
{
        if(20 == u4Value) {
                vSetPWM(0);
        } else {
                vSetPWM(u4Value);
        }
        
}
EXPORT_SYMBOL(vSetBacklightIntensity);

static int __init bkl_init(void)
{
	int ret;

	pr_info("bkl_init--->\n");
	ret = platform_driver_register(&bkl_plt_drv);
	if (ret)
		pr_err("[bkl]: %s: register  driver failed\n", __func__);
	return ret;
}

module_init(bkl_init);
static void __exit bkl_exit(void)
{
	pr_info("bkl_exit--->\n");
	platform_driver_unregister(&bkl_plt_drv);
}
module_exit(bkl_exit);

MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Ziran Xu <ziran.xu@autochips.com>");

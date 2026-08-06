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


 *Project
 *    AC83xx
 *
 * Description
 *    AC83xx pwm driver interface for hal
 *
 * Author_Name
 *    Yunjie Ren
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
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/types.h>

#include "../../../misc/atc/inc/pwm.h"
#include "oal.h"
#include "x_ver.h"
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif
#define ATC_KERNEL_LINUX_LICENSE     "GPL"

static DEFINE_MUTEX(pwm_lock);
static LIST_HEAD(pwm_list);

bool pwm_clk_select(struct pwm_device *pwm, u32 clk_id)
{
    if ((pwm == NULL) || (pwm->pwm_id >= ATC_NR_PWMS) || (clk_id >= ATC_NR_PWMCLK)) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: incorrect pwm id, or clk id\n", __func__,__LINE__);
        return false;
    } else {
        return (pwm_hal_clk_select(pwm->pwm_id, clk_id));
    }
}
EXPORT_SYMBOL(pwm_clk_select);


void pwm_dump(u32 pwm_id)
{
    pwm_hal_dump(pwm_id);
}
EXPORT_SYMBOL(pwm_dump);

/**
    ac8317 pwm multifunction selection
    pwm0_sel[1:0]: 1. gpio162 2. gpio0 3. vb0
    pwm1_sel[1:0]: 1. gpio150 2. gpio1 3. vb1
    pwm2_sel[1:0]: 1. gpio124 2. gpio2 3. gpio42
    pwm3_sel[1:0]: 1. gpio125 2. de_in 3. vb3
*/
bool pwm_config_pin(struct pwm_device *pwm, s32 pin_id)
{
    if ( (pwm == NULL) || pwm->pwm_id >= ATC_NR_PWMS) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: incorrect pwm id\r\n", __func__,__LINE__);
        return false;
    } else {
        return pwm_hal_config_pin(pwm->pwm_id, pin_id);
    }
    
}
EXPORT_SYMBOL(pwm_config_pin);


void pwm_clk_enable(struct pwm_device *pwm)
{
    if (pwm == NULL || pwm->pwm_id >= ATC_NR_PWMS) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter is not correct\n", __func__,__LINE__);
        return;
    } else {
        pwm_hal_clk_enable(pwm->pwm_id);
    }
    
    
}
EXPORT_SYMBOL(pwm_clk_enable);


void pwm_clk_disable(struct pwm_device *pwm)
{
    if (pwm == NULL || pwm->pwm_id >= ATC_NR_PWMS) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter is not correct\n", __func__,__LINE__);
        return;
    } else {
        pwm_hal_clk_disable(pwm->pwm_id);
    } 
}
EXPORT_SYMBOL(pwm_clk_disable);

/**
* pwm_mode_switch ( pwm_mode = 0 isnormal mode , else pwm sync trigger mode)
*/
bool  pwm_mode_switch(struct pwm_device *pwm, u32 pwm_mode)
{
    if((pwm == NULL) || (pwm->pwm_id >= ATC_NR_PWMS)) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter error\n", __func__,__LINE__);
        return false;
    } else {
        if(pwm_hal_mode_switch(pwm->pwm_id, pwm_mode))
            /*save pwm_mod */
            pwm->pwm_cfg_code.pwm_mode = pwm_mode;
        else
            return false;
    }
    
    return true;
}
EXPORT_SYMBOL(pwm_mode_switch);



bool pwm_set_intensity(struct pwm_device *pwm, u32  intensity)
{
    if(( pwm == NULL) || (pwm->pwm_id > ATC_NR_PWMS) || (intensity > 100)) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter error\n", __func__,__LINE__);
        return false;
    } else {
        if (pwm_hal_set_intensity(pwm->pwm_id, intensity))
            /* save pwm_high*/
            pwm->pwm_cfg_code.pwm_high = PWM_GET_HIGH(pwm->pwm_id);
        else
            return false;
    }
    return true;
    
}
EXPORT_SYMBOL(pwm_set_intensity);


bool pwm_get_intensity(struct pwm_device *pwm, u32* pIntensity)
{
    
    if(( pwm == NULL) || (pwm->pwm_id > ATC_NR_PWMS)) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: incorrect pwm id\n", __func__,__LINE__);
        return false;
    }

    *pIntensity = pwm_hal_get_intensity(pwm->pwm_id);
        
    return true;
}
EXPORT_SYMBOL(pwm_get_intensity);


/**
* pwm_config ( dutys_ns & period_ns is not used)
*/
bool pwm_config(struct pwm_device *pwm, s32 duty_ns, s32 period_ns)
{

    if ((pwm == NULL) || (pwm->pwm_id > ATC_NR_PWMS)) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter is not correct\n", __func__,__LINE__);
        return false;
    }
   
    //if (!pwm->running) 
    {
        if (pwm_hal_config(pwm->pwm_id,&(pwm->pwm_cfg_code))) {
            if (pwm->pwm_cfg_code.pwm_en)
                pwm->running = true;
            //pwm_hal_dump(pwm->pwm_id);
        }
    } 
    
    return true;
}
EXPORT_SYMBOL(pwm_config);

bool pwm_enable(struct pwm_device *pwm)
{   
    if (pwm == NULL || pwm->pwm_id >=  ATC_NR_PWMS) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter is not correct\n", __func__,__LINE__);
        return false;
    }
        
    if (!pwm->running) {
        pwm->pwm_cfg_code.pwm_en = true;
        if (!pwm_hal_config(pwm->pwm_id,&(pwm->pwm_cfg_code))) {
            pr_err( "[PWM][pwm_oal.c][%s][%d]: pwm enable fail\n", __func__,__LINE__);
            return false;
        }
        pwm->running = true;
    } else {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: pwm[%d] is already running!\n", __func__,__LINE__, pwm->pwm_id);
    }

    return true;
}
EXPORT_SYMBOL(pwm_enable);


void pwm_disable(struct pwm_device *pwm)
{
    if (pwm == NULL || pwm->pwm_id >=  ATC_NR_PWMS)
    {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: input parameter is not correct\n", __func__,__LINE__);
        return;
    }

    if (pwm->running) {
        pwm->pwm_cfg_code.pwm_en = 0;
        if (!pwm_hal_config(pwm->pwm_id,&(pwm->pwm_cfg_code))) {
            pr_err( "[PWM][pwm_oal.c][%s][%d]: pwm disable fail\n", __func__,__LINE__);
            return;
        }
        pwm->running = false;
    } else {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: pwm[%d] is already stop!\n", __func__,__LINE__, pwm->pwm_id);
    }
    
}
EXPORT_SYMBOL(pwm_disable);

struct pwm_device *pwm_request(s32 pwm_id, const s8 *label)
{
    struct pwm_device *pwm;
    s32 found = 0;

    mutex_lock(&pwm_lock);

    list_for_each_entry(pwm, &pwm_list, node) {
        if (pwm->pwm_id == pwm_id) {
            found = 1;
            break;
        }
    }

    if (found) {
        pwm->use_count++;
        pwm->label = label;
    } else {
        pwm = NULL;
    }

    mutex_unlock(&pwm_lock);
    return pwm;
}
EXPORT_SYMBOL(pwm_request);

void pwm_free(struct pwm_device *pwm)
{
    mutex_lock(&pwm_lock);

    if (pwm->use_count) {
        pwm->use_count--;
        pwm->label = NULL;
    } else {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: PWM device already freed\n", __func__,__LINE__);
    }

    mutex_unlock(&pwm_lock);
}
EXPORT_SYMBOL(pwm_free);

static inline void __add_pwm(struct pwm_device *pwm)
{
    list_add_tail(&pwm->node, &pwm_list);
}

struct clk *clk_ac8317_pwm[4];
struct clk *clk_ac8317_select_pwm[4];

struct pinctrl *pinctrl_pwm = NULL;
// struct pinctrl_state *pins_gpio125_pwm3 = NULL;
struct pinctrl_state *pins_gpio150_pwm1 = NULL;
/*static int __devinit pwm_probe(struct platform_device *pdev)*/
#ifdef CONFIG_ATC_PLATFORM_ac823x
ulong PWM_VBASE_ADDR = NULL;
#endif

static int32_t pwm_probe(struct platform_device *pdev)
{
    struct pwm_device *pwms;
    int32_t ret = 0;
    int32_t i;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    struct device_node *node = NULL;
    pr_info("[PWM][wts] pwm_probe Start\n");

	node = of_find_compatible_node(NULL, NULL, "Autochips,pwm");	
    if (node) {		
        PWM_VBASE_ADDR = (ulong)of_iomap(node, 0);  
        pr_info("[PWM][wts] pwm_probe Start 4\n");
    if (PWM_VBASE_ADDR == 0) {            
        pr_err("[PWM][pwm_oal.c][%s][%d]:can't find io virtual base address", __func__,__LINE__);         
        return -1;      
        }   
        pr_info("[PWM]PWM_VBASE_ADDR=%lx\n", PWM_VBASE_ADDR);  
        }
        else {        pr_err("[PWM][pwm_oal.c][%s][%d]:can't find compatible node\n", __func__,__LINE__);    }
#endif
    pwms = kzalloc(sizeof(struct pwm_device) * ATC_NR_PWMS, GFP_KERNEL);
    if (pwms == NULL) {
        /*dev_err(&pdev->dev, "[pwm]: pwm_probe: failed to allocate memory\n");*/
        kfree(pwms);
        return -ENOMEM;
    }
	for (i = 0; i < ATC_NR_PWMS; i++) {
		clk_ac8317_pwm[i] = NULL;
		clk_ac8317_select_pwm[i] = NULL;
		}
	clk_ac8317_pwm[0] = devm_clk_get(&pdev->dev, "mux-pwm0");
    if (clk_ac8317_pwm[0] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%d]:get mux-pwm0 error! \n", __func__,__LINE__);
      return -1;
    }
    clk_ac8317_pwm[1] = devm_clk_get(&pdev->dev, "mux-pwm1");
    if (clk_ac8317_pwm[1] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get mux-pwm1 error! \n", __func__,__LINE__);
      return -1;
    }
	clk_ac8317_pwm[2] = devm_clk_get(&pdev->dev, "mux-pwm2");
    if (clk_ac8317_pwm[2] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get mux-pwm2 error! \n", __func__,__LINE__);
      return -1;
    }
    clk_ac8317_pwm[3] = devm_clk_get(&pdev->dev, "mux-pwm3");
    if (clk_ac8317_pwm[3] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get mux-pwm3 error! \n", __func__,__LINE__);
      return -1;
    }

    clk_ac8317_select_pwm[0] = devm_clk_get(&pdev->dev, "pwm0-select");
    if (clk_ac8317_select_pwm[0] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get pwm0-select error! \n", __func__,__LINE__);
      return -1;
    }
    clk_ac8317_select_pwm[1] = devm_clk_get(&pdev->dev, "pwm1-select");
    if (clk_ac8317_select_pwm[1] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get pwm1-select error! \n", __func__,__LINE__);
      return -1;
    }
    clk_ac8317_select_pwm[2] = devm_clk_get(&pdev->dev, "pwm2-select");
    if (clk_ac8317_select_pwm[2] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get pwm2-select error! \n", __func__,__LINE__);
      return -1;
    }
    clk_ac8317_select_pwm[3] = devm_clk_get(&pdev->dev, "pwm3-select");
    if (clk_ac8317_select_pwm[3] == NULL){
      pr_err("[PWM][pwm_oal.c][%s][%s]:get pwm3-select error! \n", __func__,__LINE__);
      return -1;
    }

    pinctrl_pwm = devm_pinctrl_get(&pdev->dev);
    if(IS_ERR(pinctrl_pwm))
	pr_err("[PWM][pwm_oal.c][%s][%s]:Backlight pinctrl_pwm error! \n", __func__,__LINE__);
	
    // pins_gpio125_pwm3 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio125_pwm3");
    // ret = pinctrl_select_state(pinctrl_pwm,pins_gpio125_pwm3);
    // if(ret)
    //   pr_err("[PWM][pwm_oal.c][%s][%s]:Backlight pins_gpio125_pwm3 error! \n", __func__,__LINE__);
	
    pins_gpio150_pwm1 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio150_pwm1");
    ret = pinctrl_select_state(pinctrl_pwm,pins_gpio150_pwm1);
    if(ret)
      pr_err("[PWM][pwm_oal.c][%s][%s]:Backlight pins_gpio150_pwm1 error! \n", __func__,__LINE__);

    for (i = 0; i < ATC_NR_PWMS; i++) {
        pwms[i].use_count = 0;
        pwms[i].pwm_id = i;
        pwms[i].pdev = pdev;
        pwms[i].running = false;
        memset(&(pwms[i].pwm_cfg_code), 0, sizeof(pwm_config_code));        
        pwms[i].pwm_cfg_code.clk_id = 0;    /*default 27M */
        pwms[i].pwm_cfg_code.pin_id = 1;/*default  output pin is GPIO*/
    }
    

    mutex_lock(&pwm_lock);
    for (i = 0; i < ATC_NR_PWMS; i++)
        __add_pwm(&pwms[i]);
    mutex_unlock(&pwm_lock);

    platform_set_drvdata(pdev, pwms);

    return ret;
}

/*static int __devexit pwm_remove(struct platform_device *pdev)*/
static s32 pwm_remove(struct platform_device *pdev)
{
    struct pwm_device *pwms;
    s32 ret = 0;
    s32 i;

    pwms = platform_get_drvdata(pdev);
    if (pwms == NULL)
        return -ENODEV;

    mutex_lock(&pwm_lock);
	
    // pins_gpio125_pwm3 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio125_gpio");
    // ret = pinctrl_select_state(pinctrl_pwm,pins_gpio125_pwm3);
    // if(ret)
    //   pr_err("[PWM][pwm_oal.c][%s][%s]:Backlight pins_gpio125_gpio error! \n", __func__,__LINE__);
	
    pins_gpio150_pwm1 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio150_gpio");
    ret = pinctrl_select_state(pinctrl_pwm,pins_gpio150_pwm1);
    if(ret)
      pr_err("[PWM][pwm_oal.c][%s][%s]:Backlight pins_gpio50_gpio error! \n", __func__,__LINE__);

    for (i = 0; i < ATC_NR_PWMS; i++)
        list_del(&pwms[i].node);
    
    mutex_unlock(&pwm_lock);
    
    kfree(pwms);
    
    return 0;
}

#ifdef CONFIG_PM
static s32 atc_pwm_suspend(struct device *dev)
{

    struct pwm_device *pwms;
    struct platform_device *pdev = to_platform_device(dev);
    s32 i;
    
    pwms = platform_get_drvdata(pdev);
    
    if (pwms == NULL)
        return -ENODEV;
    pwm_hal_clk_disable(1);
    pwm_hal_clk_disable(3);

    mutex_lock(&pwm_lock);

    for (i = 0; i < ATC_NR_PWMS; i++) {
        if (pwms[i].running)
            pwms[i].running = false;
     }
    
    mutex_unlock(&pwm_lock);
    pr_info( "[PWM][pwm_oal.c][%s][%d]\n", __func__,__LINE__);
    
    return 0;  
}

static s32 atc_pwm_resume(struct device *dev)
{
  /*
    struct platform_device *pdev = to_platform_device(dev);
  */
  int32_t ret = 0;
  
   //pins_gpio125_pwm3 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio125_pwm3");
    //ret = pinctrl_select_state(pinctrl_pwm,pins_gpio125_pwm3);
    //if(ret)
    //  pr_err("[PWM][pwm_oal.c][%s][%d]:Backlight pins_gpio125_pwm3 error! \n", __func__,__LINE__);
	
    pins_gpio150_pwm1 = pinctrl_lookup_state(pinctrl_pwm,"backlight_gpio150_pwm1");
    ret = pinctrl_select_state(pinctrl_pwm,pins_gpio150_pwm1);
    if(ret)
      pr_err("[PWM][pwm_oal.c][%s][%d]:Backlight pins_gpio150_pwm1 error! \n", __func__,__LINE__);
	
    pr_info( "[PWM][%s][%d]\n", __func__,__LINE__);
    
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static s32 atc_pwm_legacy_suspend(struct platform_device *dev, pm_message_t state)
{
    pr_info( "[PWM][%s][%d]\n", __func__,__LINE__);
    return 0;
}
static s32 atc_pwm_legacy_resume(struct platform_device *dev)
{
    pr_info( "[PWM][%s][%d]\n", __func__,__LINE__);
    return 0;
}
#endif

#ifdef CONFIG_PM_RUNTIME

static s32 atc_pwm_runtime_suspend(struct device *dev)
{
  /*
     struct platform_device *pdev = to_platform_device(dev);
  */
     return 0;
}
static s32 atc_pwm_runtime_resume(struct device *dev)
{
  /*
     struct platform_device *pdev = to_platform_device(dev);
  */
     return 0;
}

static s32 atc_pwm_runtime_idle(struct device *dev)
{ 
  /*
     struct platform_device *pdev = to_platform_device(dev);
  */
     return 0;
}
#endif

#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops atc_pwm_dev_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(atc_pwm_suspend, atc_pwm_resume)
#ifdef CONFIG_PM_RUNTIME        
    SET_RUNTIME_PM_OPS(atc_pwm_runtime_suspend, atc_pwm_runtime_resume, atc_pwm_runtime_idle)
#endif  
};
#endif

static const struct of_device_id pwm_of_ids[] = {
	{.compatible = "Autochips,pwm",},
	{}
};


static struct platform_driver atc_pwm_driver = {
    .driver = {
        .name = "ac83xx-pwm",
        .owner = THIS_MODULE,
        .of_match_table = pwm_of_ids,
#ifdef CONFIG_PM    
        .pm = &atc_pwm_dev_pm_ops,
#endif      
            },
    .probe  = pwm_probe,
    /*.remove = __devexit_p(pwm_remove),*/
    .remove = pwm_remove,
#ifdef CONFIG_PM_SLEEP
     .suspend = atc_pwm_legacy_suspend,
     .resume = atc_pwm_legacy_resume,
#endif
};

static s32 __init pwm_init(void)
{
    s32 ret;
    
    //ret = os_driver_register(&atc_pwm_driver);
    ret = platform_driver_register(&atc_pwm_driver);
    if (ret) {
        pr_err( "[PWM][pwm_oal.c][%s][%d]: register  driver failed\n", __func__,__LINE__);
        return ret;
    }   
    MOD_VERSION_INFO(PWM_MOD_NAME, PWM_VER_MAIN, PWM_VER_MINOR, PWM_VER_REV);  
    pr_info( "[PWM]: %s: register  driver OK\n", __func__);
    return ret;
}
module_init(pwm_init);

static void __exit pwm_exit(void)
{
    pr_info( "[PWM]: %s------->>\n", __func__);
    os_driver_unregister(&atc_pwm_driver);
}
module_exit(pwm_exit);


MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);
MODULE_AUTHOR("Yunjie Ren <yunjie.ren@autochips.com>");


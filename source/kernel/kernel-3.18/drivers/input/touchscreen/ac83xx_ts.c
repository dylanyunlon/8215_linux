#include <linux/init.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include "ac83xx_tsadc.h"
#include "ac83xx_ts.h"

#ifdef GET_LCM_RESOLUTION_RUNTIME
#include "display_lcm.h"
#endif

#include "timer.h"
#include "oal.h"
#define KERNEL_LINUX_LICENSE     "GPL"


#define TOUCH_FIRE_TIME 1  //5ms
#define TOUCH_SAMPLE_TIME   1  //1ms

//pointercal parm
//static s32 cal[7] = {60211224, -91506,2481 , -9467008, 1679, 99758, 65536};
//static s32 cal[7] = {54197456,  -68929,  -100,  -2589108,  -494, 73137, 65536};
static s32 cal[7] = {-4309248, 73791, 126, -6529336, 1001, 85874, 65536};



static struct ac83xx_ts *ts = NULL;

extern bool touch_init(void);
extern bool touch_deinit(void);
#ifdef GET_LCM_RESOLUTION_RUNTIME
extern void touch_panel_get_point(struct ac83xx_ts *ts, TOUCH_PRESS_STATE *pTipState, s32* pUnCalX,s32* pUnCalY);
extern void TransACD2Display(struct ac83xx_ts *ts, u16 *pX, u16 *pY);
#else
extern void touch_panel_get_point(TOUCH_PRESS_STATE *pTipState, s32* pUnCalX,s32* pUnCalY);
extern void TransACD2Display(u16 *pX, u16 *pY);
#endif
extern bool AdaptiveGPT_DeInit(void);

static void ac83xx_adc_start(void)
{
    s32 xp, yp;  
    TOUCH_PRESS_STATE press_state;

#ifdef GET_LCM_RESOLUTION_RUNTIME
    touch_panel_get_point(ts, &press_state, &xp, &yp);
    TransACD2Display(ts, (u16 *)&xp, (u16 *)&yp);
#else
    touch_panel_get_point(&press_state, &xp, &yp);
    TransACD2Display((u16 *)&xp, (u16 *)&yp);
#endif

    ts->xp = xp;
    ts->yp = yp;
    //ts.u2Z1 += u2Z1;
    //ts.u2Z2 += u2Z2;
    //ts->count++;
    ts->press_state = press_state;

    #ifdef TS_DEBUG
    printk(" ts xp %d ts yp %d ts press state %d\n", xp, yp, press_state);
    #endif
    return;
}

static void touch_timer_fire(u32 data);
static DEFINE_TIMER(touch_timer, touch_timer_fire, 0, 0);

static void touch_timer_fire(u32 data)
{
#ifdef TS_DEBUG
    printk("timer ts\n");
#endif
    ac83xx_adc_start();
#if 1

    if (TOUCH_PRESS_DOWN == ts->press_state) {
        ts->xp = ( cal[0] + cal[1]*ts->xp + cal[2]*ts->yp ) / cal[6];
        ts->yp = ( cal[3] + cal[4]*ts->xp + cal[5]*ts->yp ) / cal[6];
        //ts->xp =1024 -ts->xp;
        if(abs(ts->xp-ts->xp_old)>3||abs(ts->yp-ts->yp_old)>3)
        {
            input_report_abs(ts->input, ABS_X, ts->xp);
            input_report_abs(ts->input, ABS_Y, ts->yp);
            input_report_abs(ts->input, ABS_PRESSURE,1);
            input_report_key(ts->input, BTN_TOUCH, 1);
            input_sync(ts->input);
            ts->xp_old=ts->xp;
            ts->yp_old=ts->yp;
        }
#ifdef TS_DEBUG
        printk(KERN_ALERT "TOUCH_PRESS_DOWN  traslation x %d y %d!!!!!\n", ts->xp,  ts->yp);
#endif 

 //       mod_timer(&touch_timer, jiffies + TOUCH_SAMPLE_TIME * HZ/1000);

    } else if (TOUCH_PRESS_UP == ts->press_state) {
        ts->xp = 0;
        ts->yp = 0;
        ts->xp_old=-4;
        ts->yp_old=-4;
        ts->count = 0;
    #ifdef TS_DEBUG
        printk(KERN_ALERT "TOUCH_PRESS_UP!!!!!\n");
    #endif
        //input_report_abs(ts->input, ABS_X, ts->xp);
        //input_report_abs(ts->input, ABS_Y, ts->yp);
        input_report_abs(ts->input, ABS_PRESSURE,0);
        input_report_key(ts->input, BTN_TOUCH, 0);
        input_sync(ts->input);

        //writel(WAIT4INT | INT_DOWN, ts.io + S3C2410_ADCTSC);
    } else if (TOUCH_PRESS_IGNOR == ts->press_state) {
        //printk("TOUCH_PRESS_IGNOR!!!!\n");
        #ifdef TS_DEBUG
        printk(KERN_ALERT "TOUCH_PRESS_IGNOR!!!!\n");
        #endif        
//        mod_timer(&touch_timer, jiffies + TOUCH_SAMPLE_TIME * HZ/1000);
    }
#endif
//printk(" end low:%x  high:%x",T64B_GET_LOW(),T64B_GET_HIGH());

}

extern void ac83xx_mask_ack_bim_irq(uint32_t irq);

static irqreturn_t stylus_irq(s32 irq, void *dev_id)
{
    if (irq == VECTOR_TSI)
        mod_timer(&touch_timer, jiffies + TOUCH_FIRE_TIME * HZ/1000);
    
#if AUXADC_BIM_MODE
    if (irq == VECTOR_T2)
#else
    if (irq == VECTOR_PWNFB)
#endif
        mod_timer(&touch_timer, jiffies + TOUCH_SAMPLE_TIME * HZ/1000);

#ifdef TS_DEBUG
        printk(KERN_ALERT "stylus_irq!!!!!!!\n");
#endif        
    ac83xx_mask_ack_bim_irq(irq);
    return IRQ_HANDLED;
}


static ssize_t tsauxadc_props_show(struct device *dev, struct device_attribute *attr,
                                   char *buf)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(dev);
    ssize_t size;
    //s32 pressed;
    //s32 abs_x, abs_y, abs_pressure;
    
    if (!ts_dev)
    {
        printk("Failed in dev_get_drvdata function\n");
        return (0);
    }
#if 0   // TODO
    spin_lock(&(ts_dev->lock));
    pressed = ts_dev->prev_pressed;
    abs_x = ts_dev->prev_absx;
    abs_y = ts_dev->prev_absy;
    abs_pressure = ts_dev->prev_absp;
    spin_unlock(&(ts_dev->lock));
#endif    
    size = sprintf(buf, "%d %d %d %d %d %d %d",
                    cal[0], cal[1], cal[2], cal[3], 
                    cal[4], cal[5], cal[6]);
    printk(KERN_ALERT "cal[0]=%d\n",cal[0]);

    return (size);
}

static ssize_t tsauxadc_props_store(struct device *dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    s32 nr = sscanf(buf, "%d %d %d %d %d %d %d", &cal[0], &cal[1], &cal[2], &cal[3], 
                                    &cal[4], &cal[5], &cal[6]);
    printk(KERN_ALERT "nr=%d, count=%d,cal[0]=%d\n",nr,count,cal[0]);
    
    return (count);
}

static DEVICE_ATTR(props, S_IWUSR | S_IRUGO, tsauxadc_props_show, tsauxadc_props_store);

#ifdef GET_LCM_RESOLUTION_RUNTIME
static ssize_t tsauxadc_props_resx_store(struct device *dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(dev);

    printk(KERN_INFO "current resolution x %d\n", ts_dev->x_resolution);

    sscanf(buf, "%d",&ts_dev->x_resolution);

    printk(KERN_INFO "updated resolution x %d", ts_dev->x_resolution);
    
    return (count);
}

static ssize_t tsauxadc_props_resx_show(struct device *dev, struct device_attribute *attr,
                                   char *buf)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(dev);
    ssize_t size;

    size = sprintf(buf, "%d", ts_dev->x_resolution);
    printk(KERN_INFO "current resolution x %d\n", ts_dev->x_resolution);

    return (size);
}

static ssize_t tsauxadc_props_resy_store(struct device *dev, struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(dev);

    printk(KERN_INFO "current resolution y %d\n", ts_dev->y_resolution);

    sscanf(buf, "%d", &ts_dev->y_resolution);

    printk(KERN_INFO "updated resolution y %d\n", ts_dev->y_resolution);
    
    return (count);
}

static ssize_t tsauxadc_props_resy_show(struct device *dev, struct device_attribute *attr,
                                   char *buf)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(dev);
    ssize_t size;

    size = sprintf(buf, "%d", ts_dev->y_resolution);
    printk(KERN_INFO "current resolution y %d\n", ts_dev->y_resolution);

    return (size);
}
static DEVICE_ATTR(props_resx, S_IWUSR | S_IRUGO, tsauxadc_props_resx_show, tsauxadc_props_resx_store);
static DEVICE_ATTR(props_resy, S_IWUSR | S_IRUGO, tsauxadc_props_resy_show, tsauxadc_props_resy_store);
#endif

static s32 __devinit ac83xx_tsadcc_probe(struct platform_device *pdev)
{
    struct ac83xx_ts   *ts_dev;
    struct input_dev   *input_dev;
    //struct resource      *res;
    s32  err = 0; 
    //T64B_INIT();

    /* Allocate memory for device */
    ts_dev = kzalloc(sizeof(struct ac83xx_ts), GFP_KERNEL);
    if (!ts_dev) {
        dev_err(&pdev->dev, "failed to allocate memory.\n");
        return -ENOMEM;
    }

#ifdef GET_LCM_RESOLUTION_RUNTIME
    /* Assume that getting lcd resolution is alway okay */
    ts_dev->x_resolution =  LCD_GetScreenWidth();
    ts_dev->y_resolution =  LCD_GetScreenHeight();
#else
    ts_dev->x_resolution =  1023;
    ts_dev->y_resolution =  599;

#endif
    printk(KERN_INFO "Current screen width %d, screen height %d.\n", 
                          ts_dev->x_resolution, ts_dev->y_resolution);

    /* Allocate memory for device */
    platform_set_drvdata(pdev, ts_dev);

    input_dev = input_allocate_device();
    if (!input_dev) {
        dev_err(&pdev->dev, "failed to allocate input device.\n");
        err = -EBUSY;
        goto err_free_mem;
    }
    
    ts_dev->input = input_dev;
            
    input_dev->name = pdev->name;
    input_dev->dev.parent = &pdev->dev;
    
    input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS)|BIT_MASK(EV_SYN);
    input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
    
    input_set_abs_params(input_dev, ABS_X, 0, ts_dev->x_resolution, 0, 0);
    input_set_abs_params(input_dev, ABS_Y, 0, ts_dev->y_resolution, 0, 0);

    input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
    
    input_set_drvdata(input_dev, ts_dev);
    
    ts_dev->shift = 0;
    ts_dev->irq = VECTOR_TSI;   // TODO
    
#if AUXADC_BIM_MODE
    ts_dev->gpt_irq = VECTOR_T2;
#else
    ts_dev->gpt_irq = VECTOR_PWNFB;
#endif

    ts_dev->press_state = TOUCH_PRESS_IGNOR;
    touch_init(); 
    AuxEnablePenIrq();

    /* All went ok, so register to the input system */
    err = input_register_device(input_dev);
    if (err)
        goto err_tctimerirq;

    ts = ts_dev;
    ts->xp = 0;
        ts->yp = 0;
        ts->xp_old=-4;
        ts->yp_old=-4;
        
    err = os_device_create_file(&(input_dev->dev), &dev_attr_props);
    if (err)
        goto err_filecreate;
#ifdef GET_LCM_RESOLUTION_RUNTIME
    err = os_device_create_file(&(input_dev->dev), &dev_attr_props_resx);
    if (err)
        goto err_filecreate;
    err = os_device_create_file(&(input_dev->dev), &dev_attr_props_resy);
    if (err)
        goto err_filecreate;
#endif
    err = request_irq(ts_dev->irq, stylus_irq, 0, "ac83xx_ts", ts_dev->input);
    if (err) {
        printk("cannot get TC interrupt\n");
        goto err_free_dev;
    }
    

    err = request_irq(ts_dev->gpt_irq, stylus_irq, 0, "ac83xx_ts_timer", ts_dev->input);
    if (err) {
        printk("cannot get TC timer interrupt\n");
        goto err_tcirq;
    }
    printk("ac83xx ts probe done\n");
    return 0;
    
err_filecreate:
    input_unregister_device(input_dev);
err_tctimerirq:
    free_irq(ts_dev->gpt_irq, ts_dev->input);
err_tcirq:
    free_irq(ts_dev->irq, ts_dev->input);
err_free_dev:
    input_free_device(input_dev);
err_free_mem:
    ;
    return err;
}


static s32 __devexit ac83xx_tsadcc_remove(struct platform_device *pdev)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(&pdev->dev);
    //struct resource *res;
    AuxDisablePenIrq();
    free_irq(ts_dev->irq, ts_dev->input);

    input_unregister_device(ts_dev->input);
    
    os_device_remove_file(&pdev->dev, &dev_attr_props);
#ifdef GET_LCM_RESOLUTION_RUNTIME
    os_device_remove_file(&pdev->dev, &dev_attr_props_resx);
    os_device_remove_file(&pdev->dev, &dev_attr_props_resy);
#endif
    kfree(ts);
    ts = NULL;

    return 0;
}

static s32 ac83xx_tsadcc_resume(struct platform_device *device);
static s32 ac83xx_tsadcc_suspend(struct platform_device *device, pm_message_t state);

static struct platform_driver ac83xx_tsadcc_driver = {
    .probe      = ac83xx_tsadcc_probe,
    .remove     = __devexit_p(ac83xx_tsadcc_remove),
    .resume     = ac83xx_tsadcc_resume,
    .suspend    = ac83xx_tsadcc_suspend,
    .driver     = {
        .name   = "ac83xx_tsadcc",
        },
};

struct platform_device ac83xx_device_ts = {
    .name = "ac83xx_tsadcc",
    .id   = -1,
};

static s32 __init ac83xx_tsadcc_init(void)
{
     s32 ret = 0;
     ret  = platform_device_register(&ac83xx_device_ts);
     if (ret)
        printk(KERN_ERR "ac83xx_tsadcc: add device  failed: %d\n", ret);

    return os_driver_register(&ac83xx_tsadcc_driver);
}

static void __exit ac83xx_tsadcc_exit(void)
{
    os_driver_unregister(&ac83xx_tsadcc_driver);
    platform_device_unregister(&ac83xx_device_ts);
}

static s32 ac83xx_tsadcc_resume(struct platform_device *device)
{   
    struct ac83xx_ts *ts_dev = dev_get_drvdata(&device->dev);
    touch_init();
    AuxEnablePenIrq();
    request_irq(ts_dev->irq, stylus_irq, 0, "ac83xx_ts", ts_dev->input);
    request_irq(ts_dev->gpt_irq, stylus_irq, 0, "ac83xx_ts", ts_dev->input);
    return 0;
}
static s32 ac83xx_tsadcc_suspend(struct platform_device *device, pm_message_t state)
{
    struct ac83xx_ts *ts_dev = dev_get_drvdata(&device->dev);
    AuxDisablePenIrq();
    touch_deinit();
    free_irq(ts_dev->irq, ts_dev->input);
    free_irq(ts_dev->gpt_irq, ts_dev->input);
    return 0;
}

module_init(ac83xx_tsadcc_init);
module_exit(ac83xx_tsadcc_exit);

MODULE_AUTHOR("Autochips");
MODULE_LICENSE(KERNEL_LINUX_LICENSE );
MODULE_DESCRIPTION("AC83XX TouchScreen Driver");

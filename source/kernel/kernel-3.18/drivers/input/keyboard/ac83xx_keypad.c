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

#include <linux/init.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/printk.h>
#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <ac83xx_auxadc.h>

#include <generated/atc_project.h>

#include "ac83xx_keyadc.h"
#include "ac83xx_ir_drv.h"
#include "ac83xx_ir_regs.h"
#include "x_ver.h"

#include "boot_state.h"


#define KERNEL_LINUX_LICENSE     "GPL"
#define POWER_DOWN 5

//version info
#define KPAD_VER_NAME    "KPAD"
#define KPAD_VER_MAIN     01
#define KPAD_VER_MINOR  00
#define KPAD_VER_REV       00

typedef enum 
{
    PressValid = 1,
    PressInvalid, 
    PressIgnore   
} PressFlagValue;

struct ac83xx_keypad {
    struct input_dev   *input;
    s32                 adckey_irq;
    u32        adckey_count;
    u32        adckey_validflag;
    u32        adckey_value;
    u32        adckey_oldvalue;

#ifdef CONFIG_KNOB_AC83XX
    s32                 knob_irq_pressed;
    s32                 knob_irq_spined;
#endif

#ifdef CONFIG_IR_AC83XX
    struct task_struct *ircontrol_task;
    s32                 irthread_run;
#endif

};

static struct ac83xx_keypad ac83xx_key;
static spinlock_t  irq_lock;
static bool _fgKeypadEnabled = false;//true;

extern u32 key_value[16];
extern u32 key_voltage[16];

extern const u8  AC83xxScanCodeToIndexTable[SCAN_CODE_MAX] ;
extern const u8  MainScanCodeTable[MAIN_SCAN_CODE_TABLE_SIZE];
extern s32 gpio_inout_sel(unsigned gpio, s32 dir);
extern s32 gpio_get_value(unsigned gpio);
#ifdef CONFIG_AC83XX_POWER_KEY
extern s32  ac83xx_powerkey_init(struct input_dev   *keypad_input);
#endif

extern int AC_BoardType_Get(void);//cgx

#ifdef CONFIG_VIRTUAL_KEY_AC83XX

#define VIRTUAL_POWER_KEY_POLLING_TIME   (jiffies + 5*(HZ/1000))  /* 5ms */
void virtualpowerkey_timer_fire(u32 data)
{
    struct input_dev   *input = (struct input_dev *)data;
    pr_debug("[KP][ADCKEY]: virtual power key release\r\n");
    input_report_key(input,116,0);
    input_sync(input);
}

static DEFINE_TIMER(virtualpowerkey_timer,virtualpowerkey_timer_fire,0,0);
#endif

static s32 b_is_handle_power_key = 1;
s32 is_handle_power_key(void)
{
    return b_is_handle_power_key;
}

#ifndef CONFIG_PWRK_ATOMIC_PROTECT
#define POWER_KEY_HANDLE_TIME   (jiffies + 5*(HZ))  /* 5s */
void powerkey_handle_time(unsigned long data)
{
      b_is_handle_power_key = 1;
      pr_info("[KP][Keypad] : powerkey_handle_time\r\n");
}

static DEFINE_TIMER(powerkey_handle_timer,powerkey_handle_time,0,0);
#endif

#if 0
u32 simple_strtoul(const char *cp,char **endp,u32 base)
{
    u32 result = 0,value;

    if (*cp == '0') {
        cp++;
        if ((*cp == 'x') && isxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if (!base) {
            base = 8;
        }
    }
    if (!base) {
        base = 10;
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
        ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

#endif
static inline s32 str2long(char *p, u32*num, u32 base)
{
    char *endptr;

    *num = simple_strtoul(p, &endptr, base);
    return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

u32 AuxGetKeyPadDat(char channel)
{
    u32 dat;
    u32 reg;

    switch(channel) {
        case 0:
            reg = AUXADC_DAT0; 
            break;
        case 1:
            reg = AUXADC_DAT1; 
            break;
        case 2:
            reg = AUXADC_DAT2; 
            break;
        case 3:
            reg = AUXADC_DAT3; 
            break;
        case 4:
            reg = AUXADC_DAT4; 
            break;
		case 5:
			reg = AUXADC_DAT5;
			break;
        default:
            reg = AUXADC_DAT0; 
             break;
    };

    dat = ADC_READ32(reg); 
    /*
    #define AC83XX_KEY_TIMEOUT (msecs_to_jiffies(50))
    while(!(ADC_READ32(reg)& 0x1000)) {        
         //dat = ADC_READ32(reg);
         u32 timeout;
         timeout = jiffies + AC83XX_KEY_TIMEOUT;
         //dat = ADC_READ32(reg); 
         if (time_after(jiffies, timeout))
         {
                break;
         }
    }
    dat = ADC_READ32(reg); 
    */
    if (dat & 0x1000)
        dat = dat & 0xfff;
    else
    {
        dat = 0;
		//printk("ERROR! AUXADC Channel %d data is not ready!\n", channel);
    }
    return dat;
}
EXPORT_SYMBOL(AuxGetKeyPadDat);


void Keypad_Enable(bool fgEnable)
{
    _fgKeypadEnabled = fgEnable;

    if (fgEnable)
    {
        pr_info("[KP][ADCKEY]:  >>>>>>Keypad Enabled \r\n");
    }
    else
    {
        pr_info("[KP][ADCKEY]: >>>>>>Keypad Disabled \r\n");
    }
}
EXPORT_SYMBOL(Keypad_Enable);

#ifdef CONFIG_KNOB_AC83XX
#define SPIN_LEFT   15   //android volume_up
#define SPIN_RIGHT  114   //android volume_down
#define KNOB_PRESS  102   //android HOME

static irqreturn_t knob_pressed_irq_handler(s32 irq, void *dev_id)
{
    if (VECTOR_EXT2 != irq) {
        pr_info("[KP][ADCKEY]: >>>>>>knob_pressed_irq_handler call error \r\n");
        return IRQ_NONE;
    }

//down
    input_report_key(ac83xx_key.input, KNOB_PRESS, 1);
    input_sync(ac83xx_key.input);

//up
    input_report_key(ac83xx_key.input, KNOB_PRESS, 0);
    input_sync(ac83xx_key.input);
    return IRQ_HANDLED;

}

static irqreturn_t knob_spined_irq_handler(s32 irq, void *dev_id)
{
    u32 udir;

    static u32 knob_last_jiffies=0;
    u32 knob_cur_jiffies=jiffies*1000/HZ;

    gpio_inout_sel(124,0); 
    udir = gpio_get_value(124);

    if ((knob_cur_jiffies-knob_last_jiffies)>=50) {//about 50ms
        if (1 == udir) {
            //keybd_event(KNOB_RIGHT,0,0,0);
            input_report_key(ac83xx_key.input, SPIN_RIGHT, 1);
            input_sync(ac83xx_key.input);

            //up
            input_report_key(ac83xx_key.input, SPIN_RIGHT, 0);
            input_sync(ac83xx_key.input);

            pr_info("[KP]right!\r\n");
        } else {
            //keybd_event(KNOB_LEFT,0,0,0);
            input_report_key(ac83xx_key.input, SPIN_LEFT, 1);
            input_sync(ac83xx_key.input);

            input_report_key(ac83xx_key.input, SPIN_LEFT, 0);
            input_sync(ac83xx_key.input);
            pr_info("[KP]left!\r\n");
        }

        knob_last_jiffies=knob_cur_jiffies;
    }

    return IRQ_HANDLED;

}
#endif

#ifdef CONFIG_IR_AC83XX
static s32 ircontrol_fire(void *arg)
{
    u32 pu4Key = BTN_NONE;
    u32 pu4OldKey = BTN_NONE;

    do {

        IRRX_PollMtkIr(&pu4Key);
        if (pu4Key != BTN_NONE ) {
            if (0xFF != AC83xxScanCodeToIndexTable[pu4Key]) {
                pr_info("\n [KP]get key index: %x, %x, scancode: %x\r\n",pu4Key, AC83xxScanCodeToIndexTable[pu4Key],
                                                                     MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4Key]]);
                input_report_key(ac83xx_key.input, MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4Key]], 1);
                input_sync(ac83xx_key.input);
                pr_info("[KP][IR]ircontrol_fire down %d \r\n", MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4Key]]);
            }
            //pu4Key = BTN_NONE;
            pu4OldKey = pu4Key;
        }

       
        if( pu4Key == BTN_NONE && pu4OldKey != pu4Key) {

            if (0xFF != AC83xxScanCodeToIndexTable[pu4OldKey]) {
                pr_info("\n [KP]get key index: %x, %x, scancode: %x\r\n",pu4OldKey, 
                       AC83xxScanCodeToIndexTable[pu4OldKey],MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4OldKey]]);
                input_report_key(ac83xx_key.input, MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4OldKey]], 0);
                input_sync(ac83xx_key.input);

                pr_info("[KP][IR]ircontrol_fire  up %d \r\n", MainScanCodeTable[AC83xxScanCodeToIndexTable[pu4OldKey]]);
            }

            pu4OldKey = pu4Key;
        }
    }while(ac83xx_key.irthread_run);

    return 0;
}
#endif

void KeyValue(s32 value,bool Down)
{
    if(Down)
    {
        switch(value)
        {
            case 28:
                     pr_err("[KP]Enter key down\n");
                     break;
            case 102:
                     pr_err("[KP]Home key down\n");
                     break;
            case 103:
                     pr_err("[KP]Up key down\n");
                     break;
            case 105:
                     pr_err("[KP]Left key down\n");
                     break;     
            case 106:
                     pr_err("[KP]Right key down\n");
                     break;
            case 108:
                     pr_err("[KP]Down key down\n");
                     break;       
            case 114:
                     pr_err("[KP]Voice Down key down\n");
                     break;
            case 115:
                     pr_err("[KP]Voice Up  key down\n");
                     break;   
            case 158:
                     pr_err("[KP]Back  key down\n");
                     break;            
            case 229:
                     pr_err("[KP]Recent apps key down\n");
                     break;
           default:
                     break;        
        }
    }
    else
    {
        switch(value)
        {
            case 28:
                     pr_err("[KP]Enter key up\n");
                     break;
            case 102:
                     pr_err("[KP]Home  key up\n");
                     break;
            case 103:
                     pr_err("[KP]Up key up\n");
                     break;
            case 105:
                     pr_err("[KP]Left key up\n");
                     break;     
            case 106:
                     pr_err("[KP]Right key up\n");
                     break;
            case 108:
                     pr_err("[KP]Down key up\n");
                     break;       
            case 114:
                     pr_err("[KP]Voice Down key up\n");
                     break;
            case 115:
                     pr_err("[KP]Voice Up  key up\n");
                     break;   
            case 158:
                     pr_err("[KP]Back   key up\n");
                     break;            
            case 229:
                     pr_err("[KP]Recent apps key up\n");
                     break;
           default:
                     break;        
        }
    }
}
void ac83xx_mask_ack_bim_irq(u32 irq);
static irqreturn_t adckey_irq_handler(s32 irq, void *dev_id)
{
    u32 u4DataOrg[5] = {0, 0, 0, 0, 0};
    u32 channel = 0;
    static s32 count=0;
    static s32 number=0;
    static u32 perFlag = 0;
    static u32 Flag=0;
    //static u32 i=0;
    u32 temp = 0;
    u32 counter  = 0;
    unsigned int boardtype;//cgx

    boardtype = AC_BoardType_Get();//cgx

    //printk("[Keypad] adckey_irq_handler---enter-\n");
    if (!_fgKeypadEnabled)
    {
        ac83xx_mask_ack_bim_irq(irq);
        return (IRQ_HANDLED);
    }
    do{
        temp = ADC_READ32(AUXADC_CON3);
        counter++ ;
        if (counter > 1000000)
        {
            //pr_err("[ADCKEY] ERROR !!!KeyPad debug at line %d \n",__LINE__);
            break;
        }
    }while((temp&0x1));

    temp = 0;
    temp = ADC_READ32(AUXADC_KEY_PWM_IRQ_STA);
    temp = temp & 0XF ;
    if (temp == 0) {
        //pr_err("[ADCKEY] KeyPad debug at line %d IRQ_STA=0\n",__LINE__);
        return IRQ_NONE;
    }
    if (temp & 0x01) 
    {
        u4DataOrg[1] = AuxGetKeyPadDat(1);  
        if (u4DataOrg[1] < 20) {
            channel = 1;
        }
        //printk("sampled channel 1 at %d u4DataOrg = %d temp = %x\n",__LINE__,u4DataOrg[1],temp);
    }
    if (temp & 0x02)
    {
        u4DataOrg[2] = AuxGetKeyPadDat(2);  
        if (u4DataOrg[2] < 20) {
            channel = 2;
        }
        //printk("sampled channel 2 at %d u4DataOrg = %d temp = %x\n",__LINE__,u4DataOrg[2],temp);
    }
    if (temp & 0x04)
    {
        u4DataOrg[3] = AuxGetKeyPadDat(3);  
        if (u4DataOrg[3] < 20) {
            channel = 3;
        }
        //printk("sampled channel 3 at %d u4DataOrg = %d temp = %x\n",__LINE__,u4DataOrg[3],temp);
    }
    if (temp & 0x08)
    {
        u4DataOrg[4] = AuxGetKeyPadDat(4); 
        if (u4DataOrg[4] < 20) {
            channel = 4;
        }
        //printk("sampled channel 4 at %d u4DataOrg = %d temp = %x\n",__LINE__,u4DataOrg[4],temp);
    }
    {
        u4DataOrg[0] = AuxGetKeyPadDat(0);  
        if (u4DataOrg[0] < 20) {
            channel = 0;
        }
        //printk("sampled channel 0 at %d u4DataOrg = %d temp = %x\n",__LINE__,u4DataOrg[0],temp);
    }

    number++;
    if(Flag == 0&&perFlag==0)//&&number>100)
    {
        if(u4DataOrg[0] < 20 || u4DataOrg[1] < 20 || u4DataOrg[2] < 20
            || u4DataOrg[3] < 20 || u4DataOrg[4] < 20)
        {
            count++;
        }
        if(count>40)
        {
            if (boardtype == 0)
            {
                Flag =0;
                //printk("this is demo board\n");
            }else
            {
                Flag =1;
                //printk("this is EVB board\n");
            }
            //printk("[Keypad] KeyPad is ready\n");
        }
        if(number>150)
        {
            perFlag = 1;
            if(count<40)
            {
                printk("[KP] KeyPad is not ready\n");
            }
        }
               
    }

    ac83xx_key.adckey_value = GET_KEY(u4DataOrg[0]);
	//printk("[1]Flag %d perFlag %d temp %d  count %d number %d adckey_value %d", Flag, perFlag,temp, count, number,ac83xx_key.adckey_value);
	
    if ((ac83xx_key.adckey_value == ac83xx_key.adckey_oldvalue) &&
        (0xfff != ac83xx_key.adckey_value))
    {
       
        if(1 == Flag )
        {
            ac83xx_key.adckey_count++;
            if (3 == ac83xx_key.adckey_count)
            {
                input_report_key(ac83xx_key.input, ac83xx_key.adckey_value, 1);
                input_sync(ac83xx_key.input);

                ac83xx_key.adckey_count = 0;
                ac83xx_key.adckey_validflag = PressValid;
                KeyValue(ac83xx_key.adckey_value, 1);
                //pr_debug("[ADCKEY] u4DataOrg=%d\n",u4DataOrg[0]);
            }
        }
        
    }
    else
    {
        ac83xx_key.adckey_count  = 0;
              
        if (ac83xx_key.adckey_validflag == PressValid) {
            ac83xx_key.adckey_validflag = PressInvalid;

            input_report_key(ac83xx_key.input, ac83xx_key.adckey_oldvalue, 0);
            input_sync(ac83xx_key.input);
            KeyValue(ac83xx_key.adckey_oldvalue, 0);
        } else {
            ac83xx_key.adckey_validflag = PressIgnore;
        }

        ac83xx_key.adckey_oldvalue = ac83xx_key.adckey_value;
    }

    ac83xx_mask_ack_bim_irq(irq);

key_ignore:
    return IRQ_HANDLED;
}

static ssize_t adckey_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t adckey_enable_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t size)
{
    s32 val = 0;
    sscanf(buf, "%d", &val);
    if(val) {
        _fgKeypadEnabled = true;       
    } else {
        _fgKeypadEnabled = false;       
    }
    return size;
}

static ssize_t adckey_keycode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "[KP]0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
                        key_value[0], key_value[1], key_value[2], key_value[3],
                        key_value[4], key_value[5], key_value[6], key_value[7],
                        key_value[8], key_value[9], key_value[10], key_value[11],
                        key_value[12], key_value[13], key_value[14], key_value[15]);
}

static ssize_t adckey_keycode_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t size)
{
    s32 index, now;
    s32 i;
    char temp[33];
    u32 kc;

    if (buf != NULL) { 
        index = 0;
        now = 0;
        for (i = 0; i < MAX_KEYS; i++)
        {
           index = strcspn(buf + now, ":");
           memset(temp, 0, 32);
           strncpy(temp, (buf + now), index);
           str2long(temp, &kc, 16); 
           now += index  + 1;
           key_value[i] = kc;
           pr_err("[KP][ADC KEY] key_value[%d]=0x%x kc=0x%x\n", i, key_value[i], kc);
        }
        pr_err("[KP][ADC KEY] KeyCode is updated.\n");
   } else {
        pr_err("[KP][ADC KEY] Input is invalid.\n");
   }
    return size;
}

static ssize_t adckey_voltage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "[KP]0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
                         key_voltage[0], key_voltage[1], key_voltage[2], key_voltage[3],
                         key_voltage[4], key_voltage[5], key_voltage[6], key_voltage[7],
                         key_voltage[8], key_voltage[9], key_voltage[10], key_voltage[11],
                         key_voltage[12], key_voltage[13], key_voltage[14], key_voltage[15]);
}

static ssize_t adckey_voltage_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t size)
{
    s32 index, now;
    s32 i;
    char temp[33];
    u32 kv;

    if (buf != NULL) { 
        index = 0;
        now = 0;
        for (i = 0; i < MAX_KEYS; i++)
        {
           index = strcspn(buf + now, ":");
           memset(temp, 0, 32);
           strncpy(temp, (buf + now), index);
           str2long(temp, &kv, 16); 
           now += index  + 1;
           key_voltage[i] = kv;
           pr_err("[KP][ADC KEY] key_voltage[%d]=0x%x kv=0x%x\n", i, key_voltage[i], kv);
        }
        pr_err("[KP][ADC KEY] Key Voltage is updated.\n");
   } else {
       pr_err("[KP][ADC KEY] Input is invalid.\n");
   }
    return size;
}

static DEVICE_ATTR(adckey_keycode, (S_IRUGO | S_IWUSR | S_IWGRP),
                               adckey_keycode_show,
                               adckey_keycode_store);


static DEVICE_ATTR(adckey_voltage, (S_IRUGO | S_IWUSR | S_IWGRP),
                               adckey_voltage_show,
                               adckey_voltage_store);


static DEVICE_ATTR(adckey_enable, (S_IRUGO | S_IWUSR | S_IWGRP),
                               adckey_enable_show,
                               adckey_enable_store);


static struct attribute *adckey_attrs[] = {
    &dev_attr_adckey_enable.attr,
    &dev_attr_adckey_keycode.attr,
    &dev_attr_adckey_voltage.attr,
    NULL
};

static const struct attribute_group adckey_attr_grp = {
     .attrs = adckey_attrs,
};

static s32  ac83xx_keypad_probe(struct platform_device *pdev)
{
    struct input_dev   *input_dev;
    s32    err = 0;
	struct device_node *node = pdev->dev.of_node;
    s32     i;
    
    pr_info("[KP][ADCKEY]: ac83xx keypad probe start NAME: %s VECTOR_KPI = %d\n", pdev->name,VECTOR_KPI);    
    memset(&ac83xx_key, 0, sizeof(struct ac83xx_keypad));

    /* Allocate memory for device */
    platform_set_drvdata(pdev, &ac83xx_key);


    spin_lock_init(&irq_lock);
    input_dev = input_allocate_device();
    if (!input_dev) {
        pr_err("[KP]failed to allocate input device.\n");
        err = -EBUSY;
        goto err_free_mem;
    }
    input_dev->name = pdev->name;
    input_dev->dev.parent = &pdev->dev;

    set_bit(EV_KEY, input_dev->evbit);
    for(i = 0; i < MAX_KEYS; i++)
    {
        set_bit(key_value[i], input_dev->keybit);
    }

    for(i = 0; i < MAIN_SCAN_CODE_TABLE_SIZE; i++)
    {
        set_bit(MainScanCodeTable[i], input_dev->keybit);
    }

    ac83xx_key.input = input_dev;
    input_set_drvdata(input_dev, &ac83xx_key);

    ac83xx_key.adckey_count = 0;
    ac83xx_key.adckey_validflag = PressIgnore;
    ac83xx_key.adckey_value = 0;
    ac83xx_key.adckey_oldvalue = 0;
    ac83xx_key.adckey_irq = VECTOR_KPI;

    err = input_register_device(input_dev);
    if (err) {
        pr_err("[KP]cannot input_register_device\n");
        goto err_free_dev;
    }

#ifdef CONFIG_AC83XX_KEYADC
    AuxADCInitKeypad();
    if (node)
    {
        ac83xx_key.adckey_irq = irq_of_parse_and_map(node, 0);
        pr_info("[KP]keypad get irq is %d \n",ac83xx_key.adckey_irq);
    }

    err = request_irq(ac83xx_key.adckey_irq, adckey_irq_handler, 0, "ac83xx_adckey", ac83xx_key.input);
    if (err) {
        pr_err("[KP]cannot get adckey interrupt\n");
        goto err_adcirq;
    }
#endif

#ifdef CONFIG_AC83XX_POWER_KEY
    err = ac83xx_powerkey_init(ac83xx_key.input);
    if(err)
    {
        pr_err("[KP]cannot get powerkey interrupt\n");
    }
#endif

#ifdef CONFIG_KNOB_AC83XX
    //////set the bitkey
    set_bit(SPIN_LEFT, input_dev->keybit);
    set_bit(SPIN_RIGHT, input_dev->keybit);
    set_bit(KNOB_PRESS, input_dev->keybit);

    //////////
    ac83xx_key.knob_irq_pressed = VECTOR_EXT2;
    ac83xx_key.knob_irq_spined=VECTOR_EXT3;

    ac83xx_knob_init();

    err = request_irq(ac83xx_key.knob_irq_pressed, knob_pressed_irq_handler, 0, "ac83xx_knob_pressed", ac83xx_key.input);
    if (err) {
        pr_err("[KP][ADCKEY]: >>>>>>>>konb:can't get assigned irq of knob pressed\r\n");
        goto err_knobirq_pressed;
    }

    err = request_irq(ac83xx_key.knob_irq_spined, knob_spined_irq_handler, 0, "ac83xx_knob_spined", ac83xx_key.input);
    if (err) {
        pr_err("[KP][ADCKEY]: >>>>>>>>konb:can't get assigned irq of knob spined\r\n");
        goto err_knobirq_spined;
    }
#endif 

#ifdef CONFIG_IR_AC83XX
    IRRX_InitMtkIr(ac83xx_key.input);
    ac83xx_key.irthread_run = 1;
    ac83xx_key.ircontrol_task = kthread_create(ircontrol_fire, NULL, "ircontroltask");
    wake_up_process(ac83xx_key.ircontrol_task);
#endif
    
#ifdef CONFIG_VIRTUAL_KEY_AC83XX
    add_timer(&virtualpowerkey_timer);
    virtualpowerkey_timer.data = (u32)input_dev;
#endif

    device_init_wakeup(&pdev->dev, 1);

    sysfs_create_group(&pdev->dev.kobj, &adckey_attr_grp);
    pr_info("[KP][ADCKEY]: ac83xx keypad probe done NAME: %s\n", pdev->name);
    return 0;

#ifdef CONFIG_KNOB_AC83XX
err_knobirq_spined:
    free_irq(ac83xx_key.knob_irq_spined, ac83xx_key.input);
err_knobirq_pressed:
    free_irq(ac83xx_key.knob_irq_pressed, ac83xx_key.input);
#endif
    free_irq(ac83xx_key.adckey_irq, ac83xx_key.input);
err_adcirq:
    input_free_device(input_dev);
err_free_dev:
    if(input_dev)
        kfree(input_dev);
err_free_mem:
    ;
    return err;
}

static s32  ac83xx_keypad_remove(struct platform_device *pdev)
{
    struct ac83xx_keypad *keypad_dev = dev_get_drvdata(&pdev->dev);

    device_init_wakeup(&pdev->dev, 0);

    platform_set_drvdata(pdev, NULL);

    free_irq(keypad_dev->adckey_irq, keypad_dev->input);
#ifdef CONFIG_KNOB_AC83XX
    free_irq(keypad_dev->knob_irq_pressed, keypad_dev->input);
    free_irq(keypad_dev->knob_irq_spined, keypad_dev->input);
#endif

#ifdef CONFIG_IR_AC83XX
    IRRX_StopMtkIr(keypad_dev->input);
    ac83xx_key.irthread_run = 0;
    kthread_stop(ac83xx_key.ircontrol_task);
#endif

    sysfs_remove_group(&pdev->dev.kobj, &adckey_attr_grp);
    input_unregister_device(keypad_dev->input);
    pr_err("[KP][ADCKEY]: ac83xx keypad removed \n");
    return 0;
}


extern void enable_handle_power_key(s32 b_enable)
{    
#if 0
    u32 u4Tmp;
    
    u4Tmp = PDWNC_READ32(REG_RW_GPIOOUT);
    if ((u4Tmp&0x1) != 0)
    {    
        pr_err("REG_RW_GPIOOUT:0x%x\r\n", u4Tmp);
        b_is_handle_power_key = 1;
    }
    else
    {    
        b_is_handle_power_key = b_enable;
    }
    pr_err("\n enable_handle_power_key %d", b_is_handle_power_key);
    return;
#else

#ifndef  CONFIG_PWRK_ATOMIC_PROTECT

    if(b_enable == 0){
        mod_timer(&powerkey_handle_timer, POWER_KEY_HANDLE_TIME);
    }
#endif
    b_is_handle_power_key = b_enable;
#endif
}
static s32 ac83xx_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{

      unsigned long irqflags;
#ifdef CONFIG_IR_AC83XX
#endif
      pr_err("[KP]ac83xx keypad suspend\n");
      AuxADCDeInitKeypad(); 
      spin_lock_irqsave(&irq_lock, irqflags);
      disable_irq_nosync(ac83xx_key.adckey_irq);
      spin_unlock_irqrestore(&irq_lock, irqflags);
#ifdef CONFIG_KNOB_AC83XX
      /* Disable HW */
      /* Disable SW */             
#endif

#ifdef CONFIG_IR_AC83XX
      /* Disable HW */
      /* Disable SW */             
#endif

#ifndef CONFIG_PWRK_ATOMIC_PROTECT
    enable_handle_power_key(1);
#endif
    return 0;
}

static s32 ac83xx_keypad_resume(struct platform_device *pdev)
{

     unsigned long irqflags = 0;
     AuxADCInitKeypad();
     spin_lock_irqsave(&irq_lock, irqflags);
     enable_irq(ac83xx_key.adckey_irq);
     spin_unlock_irqrestore(&irq_lock, irqflags);
#ifdef CONFIG_VIRTUAL_KEY_AC83XX
     struct ac83xx_keypad *keypad_dev = dev_get_drvdata(&pdev->dev);
     input_report_key(keypad_dev->input,116,1);
     input_sync(keypad_dev->input);
     mod_timer(&virtualpowerkey_timer, VIRTUAL_POWER_KEY_POLLING_TIME);
#endif
     //enable_handle_power_key(1);

     //pr_err("ac83xx keypad resume\npower key enable: %d\n", b_is_handle_power_key);
       
#ifdef CONFIG_KNOB_AC83XX
     /* Enable SW */
     /* Enable HW */
#endif

#ifdef CONFIG_IR_AC83XX
     /* Enable SW */
     /* Enable HW */
#endif

      return 0;
}

static void  ac83xx_keypad_release(struct device *dev)
{
    ;
}

struct platform_device ac83xx_device_keypad = {
    .name = "ac83xx_keypad",
    .id   = -1,
    .dev = {
        .release  = ac83xx_keypad_release,
    }
};

#if 0
static struct platform_driver ac83xx_keypad_driver = {
    .probe     = ac83xx_keypad_probe,
    .remove    = ac83xx_keypad_remove,
    .driver    = {
        .name  = "ac83xx_keypad",
        .owner = THIS_MODULE,
    },
    .suspend = ac83xx_keypad_suspend,
    .resume  = ac83xx_keypad_resume,
};

#else 
static const struct of_device_id keypad_of_ids[] = {
        { .compatible = "Autochips,ac83xx-keypad", },
        {}
};

static struct platform_driver ac83xx_keypad_driver = {
    .probe        = ac83xx_keypad_probe,
    .remove        = ac83xx_keypad_remove,
    .suspend    = ac83xx_keypad_suspend,
    .resume     = ac83xx_keypad_resume,
    .driver        = {
        .name    = "ac83xx_keypad",
        .owner    = THIS_MODULE,
        .of_match_table = keypad_of_ids,
    },
};

#endif 

#ifdef CONFIG_PWRK_ATOMIC_PROTECT
static int input_bs_event(struct notifier_block *notifier,
			 unsigned long bs_event, void *unused)
{
    switch (bs_event) {
    case STATUS_BOOT_START:
        pr_info("[BS] input_bs_event STATUS_BOOT_START\n");
        break;
    case STATUS_BOOT_END:
        enable_handle_power_key(1);
        pr_info("[BS] input_bs_event STATUS_BOOT_END\n");
        break;
    case STATUS_SUSPEND_START:
        enable_handle_power_key(0);
        pr_info("[BS] input_bs_event STATUS_SUSPEND_START\n");
        break;
    case STATUS_SUSPEND_END:
        enable_handle_power_key(1);
        pr_info("[BS] input_bs_event STATUS_SUSPEND_END\n");
        break;
    case STATUS_RESUME_START:
        //enable_handle_power_key(0);
        pr_info("[BS] input_bs_event STATUS_RESUME_START\n");
        break;
    case STATUS_RESUME_END:
        enable_handle_power_key(1);
        pr_info("[BS] input_bs_event STATUS_RESUME_END\n");
        break;
    case STATUS_SHUTDOWN_START:
        enable_handle_power_key(0);
        pr_info("[BS] input_bs_event STATUS_SHUTDOWN_START\n");
        break;
    case STATUS_SHUTDOWN_END:
        enable_handle_power_key(1);
        pr_info("[BS] input_bs_event STATUS_SHUTDOWN_END\n");
        break;
    }

    return 0;
}

static struct notifier_block input_bs_notifier_block = {
    .notifier_call = input_bs_event,
};
#endif

static s32 __init ac83xx_keypad_init(void)
{
    s32 ret = 0;
    MOD_VERSION_INFO(KPAD_VER_NAME,KPAD_VER_MAIN,KPAD_VER_MINOR,KPAD_VER_REV);
    //ret  = platform_device_register(&ac83xx_device_keypad);
    if (ret)
        pr_err("[KP]ac83xx_keypad: add device  failed: %d\n", ret);

#ifdef CONFIG_PWRK_ATOMIC_PROTECT
    register_bs_notifer(&input_bs_notifier_block);
#endif

    return platform_driver_register(&ac83xx_keypad_driver);
}

static void __exit ac83xx_keypad_exit(void)
{
#ifdef CONFIG_PWRK_ATOMIC_PROTECT
    unregister_bs_notifer(&input_bs_notifier_block);
#endif

    platform_driver_unregister(&ac83xx_keypad_driver);
    //platform_device_unregister(&ac83xx_device_keypad);
    pr_err("[KP][ADCKEY]: ac83xx_keypad: driver removed\n");

}

module_init(ac83xx_keypad_init);
module_exit(ac83xx_keypad_exit);

MODULE_LICENSE(KERNEL_LINUX_LICENSE);
MODULE_DESCRIPTION("AC83XX Keypad Driver");


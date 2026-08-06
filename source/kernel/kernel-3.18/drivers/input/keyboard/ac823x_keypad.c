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
//#include <mach/base_regs.h>//cgx 823x
//#include <mach/ac83xx_basic.h>//cgx 823x
#include <linux/delay.h>
#include <linux/string.h>
#include <ac823x_auxadc.h>

#include <generated/atc_project.h>

#include "../../misc/atc/inc/x_ioopt.h"//cgx
#include "../../misc/atc/include/mach/mt3365_irqs_vector.h"//cgx

#include "ac823x_keyadc.h"
#include "ac823x_ir_drv.h"
#include "ac823x_ir_regs.h"
#include "x_ver.h"
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

struct ac823x_keypad {
    struct input_dev   *input;
    s32                 adckey_irq;
    u32        adckey_count;
    u32        adckey_validflag;
    u32        adckey_value;
    u32        adckey_oldvalue;

#ifdef CONFIG_KNOB_AC823x
    s32                 knob_irq_pressed;
    s32                 knob_irq_spined;
#endif

#ifdef CONFIG_IR_AC823x
    struct task_struct *ircontrol_task;
    s32                 irthread_run;
#endif

};

static struct ac823x_keypad ac823x_key;
static spinlock_t  irq_lock;
static bool _fgKeypadEnabled = false;//true;

extern u32 key_value[16];
extern u32 key_voltage[16];

extern const u8  AC823xScanCodeToIndexTable[SCAN_CODE_MAX] ;
extern const u8  MainScanCodeTable[MAIN_SCAN_CODE_TABLE_SIZE];
extern s32 gpio_inout_sel(unsigned gpio, s32 dir);
extern s32 gpio_get_value(unsigned gpio);
#ifdef CONFIG_AC823X_POWER_KEY
extern s32  ac823x_powerkey_init(struct input_dev   *keypad_input);
#endif

extern int AC_BoardType_Get(void);//cgx

ulong IO_UCV_BASE_FOR_KP = 0;//cgx
ulong BIM_UCV_BASE = 0;
//static void __iomem *bim_reg_base;

#ifdef CONFIG_VIRTUAL_KEY_AC823x

#define VIRTUAL_POWER_KEY_POLLING_TIME   (jiffies + 5*(HZ/1000))  /* 5ms */
void virtualpowerkey_timer_fire(u32 data)
{
    struct input_dev   *input = (struct input_dev *)data;
    pr_debug("[ADCKEY]: virtual power key release\r\n");
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
#define POWER_KEY_HANDLE_TIME   (jiffies + 10*(HZ))  /* 10s */
void powerkey_handle_time(unsigned long data)
{
      b_is_handle_power_key = 1;
      pr_warn("[Keypad] : powerkey_handle_time 10s, workaround QB wifi issue\r\n");
}

static DEFINE_TIMER(powerkey_handle_timer,powerkey_handle_time,0,0);

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
    //printk("[KP]--cgxreg-line=%d---ADC_READ32(reg)=0x%08x----\n",__LINE__,ADC_READ32(reg));
    /*
    #define AC823x_KEY_TIMEOUT (msecs_to_jiffies(50))
    while(!(ADC_READ32(reg)& 0x1000)) {        
         //dat = ADC_READ32(reg);
         u32 timeout;
         timeout = jiffies + AC823x_KEY_TIMEOUT;
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
        pr_info("[ADCKEY]:  >>>>>>Keypad Enabled \r\n");
    }
    else
    {
        pr_info("[ADCKEY]: >>>>>>Keypad Disabled \r\n");
    }
}
EXPORT_SYMBOL(Keypad_Enable);

#ifdef CONFIG_KNOB_AC823x
#define SPIN_LEFT   15   //android volume_up
#define SPIN_RIGHT  114   //android volume_down
#define KNOB_PRESS  102   //android HOME

static irqreturn_t knob_pressed_irq_handler(s32 irq, void *dev_id)
{
    if (VECTOR_EXT2 != irq) {
        pr_info("[ADCKEY]: >>>>>>knob_pressed_irq_handler call error \r\n");
        return IRQ_NONE;
    }

//down
    input_report_key(ac823x_key.input, KNOB_PRESS, 1);
    input_sync(ac823x_key.input);

//up
    input_report_key(ac823x_key.input, KNOB_PRESS, 0);
    input_sync(ac823x_key.input);
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
            input_report_key(ac823x_key.input, SPIN_RIGHT, 1);
            input_sync(ac823x_key.input);

            //up
            input_report_key(ac823x_key.input, SPIN_RIGHT, 0);
            input_sync(ac823x_key.input);

            pr_info("right!\r\n");
        } else {
            //keybd_event(KNOB_LEFT,0,0,0);
            input_report_key(ac823x_key.input, SPIN_LEFT, 1);
            input_sync(ac823x_key.input);

            input_report_key(ac823x_key.input, SPIN_LEFT, 0);
            input_sync(ac823x_key.input);
            pr_info("left!\r\n");
        }

        knob_last_jiffies=knob_cur_jiffies;
    }

    return IRQ_HANDLED;

}
#endif

#ifdef CONFIG_IR_AC823x
static s32 ircontrol_fire(void *arg)
{
    u32 pu4Key = BTN_NONE;
    u32 pu4OldKey = BTN_NONE;

    do {

        IRRX_PollMtkIr(&pu4Key);
        if (pu4Key != BTN_NONE ) {
            if (0xFF != AC823xScanCodeToIndexTable[pu4Key]) {
                pr_info("\n get key index: %x, %x, scancode: %x\r\n",pu4Key, AC823xScanCodeToIndexTable[pu4Key],
                                                                     MainScanCodeTable[AC823xScanCodeToIndexTable[pu4Key]]);
                input_report_key(ac823x_key.input, MainScanCodeTable[AC823xScanCodeToIndexTable[pu4Key]], 1);
                input_sync(ac823x_key.input);
                pr_info("[IR]ircontrol_fire down %d \r\n", MainScanCodeTable[AC823xScanCodeToIndexTable[pu4Key]]);
            }
            //pu4Key = BTN_NONE;
            pu4OldKey = pu4Key;
        }

       
        if( pu4Key == BTN_NONE && pu4OldKey != pu4Key) {

            if (0xFF != AC823xScanCodeToIndexTable[pu4OldKey]) {
                pr_info("\n get key index: %x, %x, scancode: %x\r\n",pu4OldKey, 
                       AC823xScanCodeToIndexTable[pu4OldKey],MainScanCodeTable[AC823xScanCodeToIndexTable[pu4OldKey]]);
                input_report_key(ac823x_key.input, MainScanCodeTable[AC823xScanCodeToIndexTable[pu4OldKey]], 0);
                input_sync(ac823x_key.input);

                pr_info("[IR]ircontrol_fire  up %d \r\n", MainScanCodeTable[AC823xScanCodeToIndexTable[pu4OldKey]]);
            }

            pu4OldKey = pu4Key;
        }
    }while(ac823x_key.irthread_run);

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
                     pr_err("Enter key down\n");
                     break;
            case 102:
                     pr_err("Home key down\n");
                     break;
            case 103:
                     pr_err("Up key down\n");
                     break;
            case 105:
                     pr_err("Left key down\n");
                     break;     
            case 106:
                     pr_err("Right key down\n");
                     break;
            case 108:
                     pr_err("Down key down\n");
                     break;       
            case 114:
                     pr_err("Voice Down key down\n");
                     break;
            case 115:
                     pr_err("Voice Up  key down\n");
                     break;   
            case 158:
                     pr_err("Back  key down\n");
                     break;            
            case 229:
                     pr_err("Recent apps key down\n");
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
                     pr_err("Enter key up\n");
                     break;
            case 102:
                     pr_err("Home  key up\n");
                     break;
            case 103:
                     pr_err("Up key up\n");
                     break;
            case 105:
                     pr_err("Left key up\n");
                     break;     
            case 106:
                     pr_err("Right key up\n");
                     break;
            case 108:
                     pr_err("Down key up\n");
                     break;       
            case 114:
                     pr_err("Voice Down key up\n");
                     break;
            case 115:
                     pr_err("Voice Up  key up\n");
                     break;   
            case 158:
                     pr_err("Back   key up\n");
                     break;            
            case 229:
                     pr_err("Recent apps key up\n");
                     break;
           default:
                     break;        
        }
    }
}
void mt33xx_mask_ack_bim_irq(u32 irq);//cgx
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

    //printk("[KP]---cgx--1-[Keypad] adckey_irq_handler---enter-\n");

    boardtype = AC_BoardType_Get();//cgx

    //printk("[KP]---cgx--2-[Keypad] adckey_irq_handler---enter-\n");
    if (!_fgKeypadEnabled)
    {
        mt33xx_mask_ack_bim_irq(irq);
        return (IRQ_HANDLED);
    }
    //printk("[KP]---cgx--3-[Keypad] adckey_irq_handler---enter-\n");
    do{
        temp = ADC_READ32(AUXADC_CON3);
        //printk("[KP]---cgxreg-line=%d---ADC_READ32(AUXADC_CON3)=0x%08x----\n",__LINE__,ADC_READ32(AUXADC_CON3));
        counter++ ;
        if (counter > 1000000)
        {
            //pr_err("--cgx--[ADCKEY] ERROR !!!KeyPad debug at line %d \n",__LINE__);
            break;
        }
    }while((temp&0x1));
    //printk("[KP]---cgx--4-[Keypad] adckey_irq_handler---enter-\n");

    temp = 0;
    temp = ADC_READ32(AUXADC_KEY_PWM_IRQ_STA);
    //printk("[KP]---cgxreg-line=%d---ADC_READ32(AUXADC_KEY_PWM_IRQ_STA)=0x%08x----\n",__LINE__,ADC_READ32(AUXADC_KEY_PWM_IRQ_STA));
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
                Flag =1;
                //printk("--cgx--this is demo board\n");
            }else
            {
                Flag =1;
                //printk("--cgx--this is EVB board\n");
            }
            //printk("[Keypad] KeyPad is ready\n");
        }
        if(number>150)
        {
            perFlag = 1;
            if(count<40)
            {
                //printk("[Keypad] KeyPad is not ready\n");
            }
        }
               
    }

    ac823x_key.adckey_value = GET_KEY(u4DataOrg[0]);//cgx 0
	//printk("--cgx--[1]Flag %d perFlag %d temp %d  count %d number %d adckey_value %d", Flag, perFlag,temp, count, number,ac823x_key.adckey_value);
	
    if ((ac823x_key.adckey_value == ac823x_key.adckey_oldvalue) &&
        (0xfff != ac823x_key.adckey_value))
    {
       
        if(1 == Flag )
        {
            ac823x_key.adckey_count++;
            if (3 == ac823x_key.adckey_count)
            {
                //printk("--cgx--input_report_key---1-down--\n");
                input_report_key(ac823x_key.input, ac823x_key.adckey_value, 1);
                input_sync(ac823x_key.input);

                ac823x_key.adckey_count = 0;
                ac823x_key.adckey_validflag = PressValid;
                KeyValue(ac823x_key.adckey_value, 1);
                //pr_debug("--cgx---[ADCKEY] u4DataOrg=%d\n",u4DataOrg[0]);//cgx 0
            }
        }
        
    }
    else
    {
        ac823x_key.adckey_count  = 0;
              
        if (ac823x_key.adckey_validflag == PressValid) {
            ac823x_key.adckey_validflag = PressInvalid;
            //printk("--cgx--input_report_key---0--up---\n");

            input_report_key(ac823x_key.input, ac823x_key.adckey_oldvalue, 0);
            input_sync(ac823x_key.input);
            KeyValue(ac823x_key.adckey_oldvalue, 0);
        } else {
            ac823x_key.adckey_validflag = PressIgnore;
        }

        ac823x_key.adckey_oldvalue = ac823x_key.adckey_value;
    }

    mt33xx_mask_ack_bim_irq(irq);

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
    return sprintf(buf, "0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
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
           pr_err("[ADC KEY] key_value[%d]=0x%x kc=0x%x\n", i, key_value[i], kc);
        }
        pr_err("[ADC KEY] KeyCode is updated.\n");
   } else {
        pr_err("[ADC KEY] Input is invalid.\n");
   }
    return size;
}

static ssize_t adckey_voltage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
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
           pr_err("[ADC KEY] key_voltage[%d]=0x%x kv=0x%x\n", i, key_voltage[i], kv);
        }
        pr_err("[ADC KEY] Key Voltage is updated.\n");
   } else {
       pr_err("[ADC KEY] Input is invalid.\n");
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

static s32  ac823x_keypad_probe(struct platform_device *pdev)
{
    struct input_dev   *input_dev;
    s32    err = 0;
	struct device_node *node = pdev->dev.of_node;
    s32     i;
	static struct device_node * node_x = NULL;//cgx
	
	pr_err("--cgx--1----[ADCKEY]: ac823x keypad probe start NAME: %s VECTOR_KP = %d\n", pdev->name,VECTOR_KP);    
//cgx
	node_x = of_find_compatible_node(NULL, NULL, "autochips,ac823x-gpio");
	if (node_x) {
		IO_UCV_BASE_FOR_KP = (ulong)of_iomap(node_x, 0);
		if (IO_UCV_BASE_FOR_KP == 0) {
            pr_err("[KP]can't find io virtual base address");
			return -1;
		}
		pr_info("IO_UCV_BASE_FOR_KP=%lx\n", IO_UCV_BASE_FOR_KP);
	}else {
        pr_err("[KP]can't find compatible node\n");
	}

	node_x =of_find_compatible_node(NULL,NULL,"mediatek,mt33xx-bim");
	if (node_x) {
		BIM_UCV_BASE = (ulong)of_iomap(node_x, 0);
		if (BIM_UCV_BASE == 0) {
            pr_err("[KP]can't find BIM virtual base address");
			return -1;
		}
		pr_info("BIM_UCV_BASE=%lx\n", BIM_UCV_BASE);
	}else {
        pr_err("[KP]can't find compatible node\n");
	}
    //node_x =of_find_compatible_node(NULL,NULL,"mediatek,mt33xx-bim");
    //        if(node_x){
    //                bim_reg_base = of_iomap(node_x, 0);
    //                printk("bim_reg_base start is 0x%p\n", bim_reg_base);
    //        }
    //        else {
    //                printk("failed to get bim node\n");
    //        }
    ////      bim_reg_base = phys_to_virt((unsigned long)(0x10045000));//ioremap(0x10045000, 0x1000);//of_iomap(pdev->dev.of_node, 0);
    //        if(bim_reg_base == 0){
    //                printk("dualarm probe is failed cause by get bim_reg_base\n");
    //               ret = -ENODEV;
    //        }
//cgx
    
    pr_err("--cgx--2----[ADCKEY]: ac823x keypad probe start NAME: %s VECTOR_KP = %d\n", pdev->name,VECTOR_KP);    
    memset(&ac823x_key, 0, sizeof(struct ac823x_keypad));

    /* Allocate memory for device */
    platform_set_drvdata(pdev, &ac823x_key);


    spin_lock_init(&irq_lock);
    input_dev = input_allocate_device();
    if (!input_dev) {
        pr_err("failed to allocate input device.\n");
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

    ac823x_key.input = input_dev;
    input_set_drvdata(input_dev, &ac823x_key);

    ac823x_key.adckey_count = 0;
    ac823x_key.adckey_validflag = PressIgnore;
    ac823x_key.adckey_value = 0;
    ac823x_key.adckey_oldvalue = 0;
    ac823x_key.adckey_irq = VECTOR_KP;//VECTOR_KPI;

    err = input_register_device(input_dev);
    if (err) {
        pr_err("cannot input_register_device\n");
        goto err_free_dev;
    }

#if 1//def CONFIG_AC823x_KEYADC   //CONFIG_AC823X_KEYADC//cgx
    printk("---cgx---[KP]---cgx---[Keypad] CONFIG_AC823x_KEYADC--AuxADCInitKeypad--\n");

    AuxADCInitKeypad();
    if (node)
    {
        ac823x_key.adckey_irq = 211;//irq_of_parse_and_map(node, 0);//cgx todo
        pr_err("--[KP]---cgx---keypad get irq is %d \n",ac823x_key.adckey_irq);
    }

    err = request_irq(ac823x_key.adckey_irq, adckey_irq_handler, 0, "ac823x_adckey", ac823x_key.input);
    if (err) {
        pr_err("cannot get adckey interrupt\n");
        goto err_adcirq;
    }
#endif

#ifdef CONFIG_AC823X_POWER_KEY
    pr_err("[KP][8237 PWRKEY]: >>>>>>>>init power key\r\n");
    err = ac823x_powerkey_init(ac823x_key.input);
    if(err)
    {
        pr_err("cannot get powerkey interrupt\n");
    }
#endif

#ifdef CONFIG_KNOB_AC823x
    //////set the bitkey
    set_bit(SPIN_LEFT, input_dev->keybit);
    set_bit(SPIN_RIGHT, input_dev->keybit);
    set_bit(KNOB_PRESS, input_dev->keybit);

    //////////
    ac823x_key.knob_irq_pressed = VECTOR_EXT2;
    ac823x_key.knob_irq_spined=VECTOR_EXT3;

    ac823x_knob_init();

    err = request_irq(ac823x_key.knob_irq_pressed, knob_pressed_irq_handler, 0, "ac823x_knob_pressed", ac823x_key.input);
    if (err) {
        pr_err("[ADCKEY]: >>>>>>>>konb:can't get assigned irq of knob pressed\r\n");
        goto err_knobirq_pressed;
    }

    err = request_irq(ac823x_key.knob_irq_spined, knob_spined_irq_handler, 0, "ac823x_knob_spined", ac823x_key.input);
    if (err) {
        pr_err("[ADCKEY]: >>>>>>>>konb:can't get assigned irq of knob spined\r\n");
        goto err_knobirq_spined;
    }
#endif 

#ifdef CONFIG_IR_AC823x
    IRRX_InitMtkIr(ac823x_key.input);
    ac823x_key.irthread_run = 1;
    ac823x_key.ircontrol_task = kthread_create(ircontrol_fire, NULL, "ircontroltask");
    wake_up_process(ac823x_key.ircontrol_task);
#endif
    
#ifdef CONFIG_VIRTUAL_KEY_AC823x
    add_timer(&virtualpowerkey_timer);
    virtualpowerkey_timer.data = (u32)input_dev;
#endif

    device_init_wakeup(&pdev->dev, 1);

    sysfs_create_group(&pdev->dev.kobj, &adckey_attr_grp);
    pr_err("[ADCKEY]: ac823x keypad probe done NAME: %s\n", pdev->name);
    return 0;

#ifdef CONFIG_KNOB_AC823x
err_knobirq_spined:
    free_irq(ac823x_key.knob_irq_spined, ac823x_key.input);
err_knobirq_pressed:
    free_irq(ac823x_key.knob_irq_pressed, ac823x_key.input);
#endif
    free_irq(ac823x_key.adckey_irq, ac823x_key.input);
err_adcirq:
    input_free_device(input_dev);
err_free_dev:
    if(input_dev)
        kfree(input_dev);
err_free_mem:
    ;
    return err;
}

static s32  ac823x_keypad_remove(struct platform_device *pdev)
{
    struct ac823x_keypad *keypad_dev = dev_get_drvdata(&pdev->dev);

    device_init_wakeup(&pdev->dev, 0);

    platform_set_drvdata(pdev, NULL);

    free_irq(keypad_dev->adckey_irq, keypad_dev->input);
#ifdef CONFIG_KNOB_AC823x
    free_irq(keypad_dev->knob_irq_pressed, keypad_dev->input);
    free_irq(keypad_dev->knob_irq_spined, keypad_dev->input);
#endif

#ifdef CONFIG_IR_AC823x
    IRRX_StopMtkIr(keypad_dev->input);
    ac823x_key.irthread_run = 0;
    kthread_stop(ac823x_key.ircontrol_task);
#endif

    sysfs_remove_group(&pdev->dev.kobj, &adckey_attr_grp);
    input_unregister_device(keypad_dev->input);
    pr_err("[ADCKEY]: ac823x keypad removed \n");
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
    if(b_enable == 0){
        mod_timer(&powerkey_handle_timer, POWER_KEY_HANDLE_TIME);
    }
    
    b_is_handle_power_key = b_enable;
#endif
}
static s32 ac823x_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{

      unsigned long irqflags;
#ifdef CONFIG_IR_AC823x
#endif
      pr_info("ac823x keypad suspend\n");
      //AuxADCDeInitKeypad(); //todo
      spin_lock_irqsave(&irq_lock, irqflags);
      disable_irq_nosync(ac823x_key.adckey_irq);
      spin_unlock_irqrestore(&irq_lock, irqflags);
#ifdef CONFIG_KNOB_AC823x
      /* Disable HW */
      /* Disable SW */             
#endif

#ifdef CONFIG_IR_AC823x
      /* Disable HW */
      /* Disable SW */             
#endif

      enable_handle_power_key(1);
      return 0;
}

static s32 ac823x_keypad_resume(struct platform_device *pdev)
{

     unsigned long irqflags = 0;
     AuxADCInitKeypad();
     spin_lock_irqsave(&irq_lock, irqflags);
     enable_irq(ac823x_key.adckey_irq);
     spin_unlock_irqrestore(&irq_lock, irqflags);
#ifdef CONFIG_VIRTUAL_KEY_AC823x
     struct ac823x_keypad *keypad_dev = dev_get_drvdata(&pdev->dev);
     input_report_key(keypad_dev->input,116,1);
     input_sync(keypad_dev->input);
     mod_timer(&virtualpowerkey_timer, VIRTUAL_POWER_KEY_POLLING_TIME);
#endif
     //enable_handle_power_key(1);

     //pr_err("ac823x keypad resume\npower key enable: %d\n", b_is_handle_power_key);
       
#ifdef CONFIG_KNOB_AC823x
     /* Enable SW */
     /* Enable HW */
#endif

#ifdef CONFIG_IR_AC823x
     /* Enable SW */
     /* Enable HW */
#endif

      return 0;
}

static void  ac823x_keypad_release(struct device *dev)
{
    ;
}

struct platform_device ac823x_device_keypad = {
    .name = "ac823x_keypad",
    .id   = -1,
    .dev = {
        .release  = ac823x_keypad_release,
    }
};

#if 0
static struct platform_driver ac823x_keypad_driver = {
    .probe     = ac823x_keypad_probe,
    .remove    = ac823x_keypad_remove,
    .driver    = {
        .name  = "ac823x_keypad",
        .owner = THIS_MODULE,
    },
    .suspend = ac823x_keypad_suspend,
    .resume  = ac823x_keypad_resume,
};

#else 
static const struct of_device_id keypad_of_ids[] = {
        { .compatible = "Autochips,ac823x-keypad", },
        {}
};

static struct platform_driver ac823x_keypad_driver = {
    .probe        = ac823x_keypad_probe,
    .remove        = ac823x_keypad_remove,
    .suspend    = ac823x_keypad_suspend,
    .resume     = ac823x_keypad_resume,
    .driver        = {
        .name    = "ac823x_keypad",
        .owner    = THIS_MODULE,
        .of_match_table = keypad_of_ids,
    },
};

#endif 

static s32 __init ac823x_keypad_init(void)
{
    s32 ret = 0;
	printk("[KP]---cgx--ac823x_keypad_init---1----\n");
    MOD_VERSION_INFO(KPAD_VER_NAME,KPAD_VER_MAIN,KPAD_VER_MINOR,KPAD_VER_REV);
    //ret  = platform_device_register(&ac823x_device_keypad);
    if (ret)
        pr_err("ac823x_keypad: add device  failed: %d\n", ret);
	
    printk("[KP]---cgx--ac823x_keypad_init---2----\n");
     return platform_driver_register(&ac823x_keypad_driver);
	printk("[KP]---cgx--ac823x_keypad_init---3----\n");
}

static void __exit ac823x_keypad_exit(void)
{
    platform_driver_unregister(&ac823x_keypad_driver);
    //platform_device_unregister(&ac823x_device_keypad);
    pr_err("[ADCKEY]: ac823x_keypad: driver removed\n");

}

module_init(ac823x_keypad_init);
module_exit(ac823x_keypad_exit);

MODULE_LICENSE(KERNEL_LINUX_LICENSE);
MODULE_DESCRIPTION("AC823x Keypad Driver");


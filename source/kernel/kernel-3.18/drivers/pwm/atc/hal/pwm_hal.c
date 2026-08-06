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
 *    AC83xx pwm hal layer
 *
 * Author_Name
 *    Yunjie Ren
 *
 */
#ifdef __ARM2__
#include "x_types.h"
#define KERN_ERR		"Error:"
#define KERN_ALERT		"Warning:"
#endif
#include "pwm_hal.h"

#ifdef __ARM2__
/*clk*/
static const u32 pwm_clk_sel_reg[ATC_NR_PWMS] ={e_CLK_SEL_PWM0,
                                                   e_CLK_SEL_PWM1,
                                                   e_CLK_SEL_PWM2,
                                                   e_CLK_SEL_PWM3,
#ifndef AC8317
                                                   e_CLK_SEL_PWM4,
                                                   e_CLK_SEL_PWM5
#endif
};

static const u32 pwm_clk_en_reg[ATC_NR_PWMS] ={e_CLK_PWM0,
                                                   e_CLK_PWM1,
                                                   e_CLK_PWM2,
                                                   e_CLK_PWM3,
#ifndef AC8317
                                                   e_CLK_PWM4,
                                                   e_CLK_PWM5
#endif
};
#endif

/*pin mux*/
#ifdef __ARM2__
static const u32 pwm_pin_sel_reg[ATC_NR_PWMS] ={PWM0_SEL,
                                                        PWM1_SEL,
                                                        PWM2_SEL,
                                                        PWM3_SEL,
#ifndef AC8317
                                                        PWM4_SEL,
                                                        PWM5_SEL
#endif
};
#endif

/*clk source frequency value*/
#ifdef __ARM2__
static const u32 pwm_source_clk[ATC_NR_PWMCLK] = {27, 216, 108, 294, 324, 418, 150, 270};
#else
static const char *pwm_clk_switch[ATC_NR_PWMCLK] = {"clk27m_ck", "syspll_d3", "syspll_d6", "apll2_ck", "syspll_d2", "armpll_d2", "dmpll_d2", "apll1_ck"};
#endif

#ifndef __ARM2__
struct clk *clk_ac8317_parent_pwm[4];
#endif

bool pwm_hal_clk_select(u32 pwm_id, u32 clk_id)
{
    s32 ret;
    if ((pwm_id >= ATC_NR_PWMS) || (clk_id >= ATC_NR_PWMCLK)) {
      #ifdef __ARM2__
		printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id, or clk id\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id, or clk id\n", __func__,__LINE__);
      #endif
      return 0;
    }
    #ifdef __ARM2__
      CKGEN_AgtSelClk(pwm_clk_sel_reg[pwm_id], clk_id);
    #else	
      clk_ac8317_parent_pwm[pwm_id] = clk_get(NULL, pwm_clk_switch[clk_id]);
	  
      ret = clk_set_parent(clk_ac8317_pwm[pwm_id], clk_ac8317_parent_pwm[pwm_id]);
	  
      clk_ac8317_parent_pwm[pwm_id] = clk_get_parent(clk_ac8317_pwm[pwm_id]);
	  
      if (clk_ac8317_parent_pwm[pwm_id] == NULL){
        pr_err("[PWM][pwm_hal.c][%s][%s]:get select-parent-pwm%d error! \n", __func__,__LINE__,pwm_id);
        return 0;
        }
    #endif
    return 1;
}



void pwm_hal_dump(u32 pwm_id)
{
    if (pwm_id >= ATC_NR_PWMS) {
      #ifdef __ARM2__
        printk( KERN_ERR"[PWM][pwm_hal.c][%s][%s]: pwm id [%d] is not incorrect\n", __func__,__LINE__, pwm_id);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: %s: pwm id [%d] is not incorrect\n", __func__,__LINE__, pwm_id);
      #endif
      return;
    }
    #ifdef __ARM2__
      printk(KERN_ALERT "[PWM][pwm_hal.c][%d][%s][%s]: Cfg Register: 0x%08x\n", pwm_id, __func__,__LINE__, (u32)PWM_READ32(pwm_id));
    #else
      pr_alert( "[PWM][%d][pwm_hal.c][%s][%s]: Cfg Register: 0x%08x\n", pwm_id, __func__,__LINE__, (u32)PWM_READ32(pwm_id));
    #endif
}



bool pwm_hal_config_pin(u32 pwm_id, s32 pin_id)
{
    if (pwm_id >= ATC_NR_PWMS) {
      #ifdef __ARM2__
     //   printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id\r\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id\r\n", __func__,__LINE__);
      #endif
        return 0;
    }
    if (pin_id > 0 && pin_id < 4)
    {
      #ifdef __ARM2__
      bsp_pinset(pwm_pin_sel_reg[pwm_id], pin_id);
      #endif
    }
    else {
      #ifdef __ARM2__
//        printk(KERN_ERR"[PWM][pwm_hal.c][%s][%s]: PWM[%d] Output pin setting error\n", __func__,__LINE__, pwm_id);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: PWM[%d] Output pin setting error\n", __func__,__LINE__, pwm_id);
      #endif
      return 0;
    }
    return 1;
}


void pwm_hal_clk_enable(u32 pwm_id)
{
    if (pwm_id >= ATC_NR_PWMS) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #endif
      return;
    }
    #ifdef __ARM2__
      CKGEN_AgtOnClk(pwm_clk_en_reg[pwm_id]);
    #else
      clk_prepare(clk_ac8317_select_pwm[pwm_id]);
      clk_enable(clk_ac8317_select_pwm[pwm_id]);
    #endif
}


void pwm_hal_clk_disable(u32 pwm_id)
{
    if (pwm_id >= ATC_NR_PWMS) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #endif
      return;
    }
    #ifdef __ARM2__
      CKGEN_AgtOffClk(pwm_clk_en_reg[pwm_id]); 	
    #else
      clk_disable(clk_ac8317_select_pwm[pwm_id]);
      clk_unprepare(clk_ac8317_select_pwm[pwm_id]);
    #endif
}


static bool pwm_hal_set(u32 pwm_id, bool pwm_en, u32 pwm_rsn, 
    u32 pwm_high, u32 pwm_prescale, u32 pwm_mode)
{   
    if (pwm_id >= ATC_NR_PWMS
        || (pwm_rsn == 0) 
        || (pwm_prescale > 0x3F) 
        || (pwm_high > 0xFFF) 
        || (pwm_rsn > 0xFFF)
        || (pwm_high > pwm_rsn)) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%d][%s][%s]: input parameter error, RSN[0,0xFFF]; High[0,0xFFF]; High <=RSN; Prescale[0,0x3F]\n",
            pwm_id, __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%d][%s][%s]: input parameter error, RSN[0,0xFFF]; High[0,0xFFF]; High <=RSN; Prescale[0,0x3F]\n",
            pwm_id, __func__,__LINE__);
      #endif
      return 0;
    }

    if (!pwm_en) 
        PWM_WRITE32(pwm_id,  0);      
    else
        PWM_WRITE32(pwm_id, 
            (PWM_RSN_SET(pwm_rsn) | PWM_HIGH_SET(pwm_high) | PWM_PERSCALE_SET(pwm_prescale) | PWM_EN));
        
    if (pwm_mode)  /* pwm sync trigger mode */
        PWM_WRITE32_MASK(PWM_CTRL, (0x3 << (pwm_id*2)), (0x3 << (pwm_id*2)));
    else /* normal mode */
        PWM_WRITE32_MASK(PWM_CTRL, (0x1 << (pwm_id*2)), (0x3 << (pwm_id*2)));

    return 1;
}


bool pwm_hal_config(u32 pwm_id, pwm_config_code *pwm_cfg_code)
{

    if ((pwm_id > ATC_NR_PWMS)) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: input parameter is not correct\n", __func__,__LINE__);
      #endif
        return 0;
    }

    pwm_hal_config_pin(pwm_id, pwm_cfg_code->pin_id);
    pwm_hal_clk_select(pwm_id, pwm_cfg_code->clk_id);
    pwm_hal_clk_enable(pwm_id);

    if (!pwm_hal_set(pwm_id, 
        pwm_cfg_code->pwm_en,
        pwm_cfg_code->pwm_rsn, 
        pwm_cfg_code->pwm_high,
        pwm_cfg_code->pwm_prescale,
        pwm_cfg_code->pwm_mode)
        ) {
        #ifdef __ARM2__
            printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: pwm config fail\n", __func__,__LINE__);
        #else
            pr_err( "[PWM][pwm_hal.c][%s][%s]: pwm config fail\n", __func__,__LINE__);
        #endif
            return 0;
        }

    return 1;
}


/**
* pwm_mode_switch ( pwm_mode = 0 isnormal mode , else pwm sync trigger mode)
*/
bool  pwm_hal_mode_switch(u32 pwm_id, u32 pwm_mode)
{
    if((pwm_id >= ATC_NR_PWMS)) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: input parameter error\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: input parameter error\n", __func__,__LINE__);
      #endif
        return 0;
    }
    
    
    if (pwm_mode) /* pwm sync trigger mode */
        PWM_WRITE32_MASK(PWM_CTRL, (0x3 << (pwm_id*2)), (0x3 << (pwm_id*2)));
    else /* normal mode */
        PWM_WRITE32_MASK(PWM_CTRL, (0x1 << (pwm_id*2)), (0x3 << (pwm_id*2)));

    return 1;
}



bool pwm_hal_set_intensity(u32 pwm_id, u32  intensity)
{
    u32 total = 0;
    u32 high = 0;
    u32 reg_value = 0;
    
    if((pwm_id > ATC_NR_PWMS) || (intensity > 100) ) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: input parameter error\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: input parameter error\n", __func__,__LINE__);
      #endif
        return 0;
    }

    reg_value = PWM_READ32(pwm_id);
    total = (reg_value & 0xFFF00000)>>20;
    high = (reg_value & 0x000FFF00)>>8;

    if (((reg_value & PWM_EN) == 0) || (total == 0)) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: read intensity error, reg value:0x%08x\n", __func__,__LINE__, reg_value);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: read intensity error, reg value:0x%08x\n", __func__,__LINE__, reg_value);
      #endif
        return 0;
    }
    /*add intensity is 0 process*/
    if (intensity == 0) {
        high = 0;
    }
    else 
    {
        high = (intensity*(total + 1))/100 - 1;
    }
    
    PWM_WRITE32_MASK(pwm_id, (PWM_HIGH_SET(high)), (PWM_HIGH_SET(0xFFF)));
    return 1;
    
}


u32 pwm_hal_get_intensity(u32 pwm_id)
{
    u32 total = 0;
    u32 high = 0;
    u32 reg_value = 0;
    
    if(pwm_id > ATC_NR_PWMS) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: incorrect pwm id\n", __func__,__LINE__);
      #endif
        return 0;
    }
    
    reg_value = PWM_READ32(pwm_id);
    
    total = (reg_value & 0xFFF00000) >> 20;
    high = (reg_value & 0x000FFF00) >> 8;
    
    if (((reg_value & PWM_EN) == 0) || (total == 0)) {
      #ifdef __ARM2__
        printk(KERN_ERR "[PWM][pwm_hal.c][%s][%s]: read intensity error\n", __func__,__LINE__);
      #else
        pr_err( "[PWM][pwm_hal.c][%s][%s]: read intensity error\n", __func__,__LINE__);
      #endif
        return 0;
    }

    return ((100*(high + 1))/(total + 1) + 1);
    
}






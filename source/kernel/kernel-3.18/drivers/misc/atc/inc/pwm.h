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

#ifndef __PWM_H__
#define __PWM_H__


#include "pwm_hal.h"

/**
Version Control
*/
#define PWM_MOD_NAME    "PWM"
#define PWM_VER_MAIN    1
#define PWM_VER_MINOR   0
#define PWM_VER_REV     0

typedef struct pwm_device {
        struct list_head node;
        struct platform_device *pdev;
        const s8 *label;
        u8 pwm_id; 
        u32 use_count;/* use to mark pwm use count @ internal use */
        pwm_config_code pwm_cfg_code;
        bool running;/* use to mark pwm state @ internal use */
}pwm_device;

/* pwm interface */
extern bool pwm_clk_select(struct pwm_device *pwm, u32 clk_id);
extern bool pwm_config_pin(struct pwm_device *pwm, s32 pin);
extern struct pwm_device *pwm_request(s32 pwm_id, const s8 *label);
extern bool pwm_config(struct pwm_device *pwm, s32 duty_ns, s32 period_ns);
extern bool pwm_set_intensity(struct pwm_device *pwm, u32  intensity);
extern bool pwm_mode_switch(struct pwm_device *pwm, u32 pwm_mode);
extern bool pwm_get_intensity(struct pwm_device *pwm, u32* pIntensity);
extern void pwm_dump(u32 id);
extern bool pwm_enable(struct pwm_device *pwm);
extern void pwm_disable(struct pwm_device *pwm);
extern void pwm_clk_enable(struct pwm_device *pwm);
extern void pwm_clk_disable(struct pwm_device *pwm);
extern void pwm_free(struct pwm_device *pwm);

#endif


#ifndef _AC83XX_TS_H
#define _AC83XX_TS_H
    
#include <linux/types.h>
#include <mach/ac83xx_basic.h>

#define AUXADC_BIM_MODE     0 
#define TS_DEBUG = 1
struct ac83xx_ts {
    struct input_dev   *input;
    s32                 irq;
    s32                 gpt_irq;
    u32        prev_absx;
    u32        prev_absy;
    u32            prev_absp;
    s32                 prev_pressed;
    TOUCH_PRESS_STATE press_state;
    s32                 xp;
    s32                 yp;
    s32                 xp_old,yp_old;
    s32                 count;
    s32                 shift;
    s32                 u2Z1,u2Z2;
    s32                 x_resolution;
    s32                 y_resolution;
};

#endif //_AC83XX_TS_H


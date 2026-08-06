#ifndef __AC83XX_TS_TIMER_H
#define __AC83XX_TS_TIMER_H

#include <linux/types.h>
#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <ac83xx_auxadc.h>

#define TOUCH_MS_PER_SAMPLE_LOW  8  //every 8 ms sample 1 points
#define TOUCH_MS_PER_SAMPLE_HIGH 1  //every 1 ms sample 1 points

#define COUNTE_PER_MSEC 1625

#define STANDARD_MOVEMENT 9.0 // = dX ^ 2 + dY ^ 2 (pixel ^ 2)

#define MIN_GPT_INTERVAL_MS TOUCH_MS_PER_SAMPLE_HIGH
#define MAX_GPT_INTERVAL_MS 15.0

bool AdaptiveGPT_Init(void);
bool AdaptiveGPT_DeInit(void);

void AdaptiveGPT_Start(void);
void AdaptiveGPT_Stop(void);
void AdaptiveGPT_AdjustInterval(u16 x, u16 y, bool bAdjusted);
void AdaptiveGPT_ChangInterval(u16 ms);


#endif /* __AC83XX_TS_TIMER_H */


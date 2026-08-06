#include <linux/kernel.h>
#include "ac83xx_tstimer.h"
#include "ac83xx_regtimer.h"
// GPT SETTING
//static GPT_CONFIG s_gptConfig;


static MT_TIMER_REG *s_pGptRegs;
extern u16 g_u2HighSampleRateFlag;
void AdaptiveGPT_Start()
{
    s_pGptRegs->TIMER_CTRL |= 0x10000;    //enable bit16
}

void AdaptiveGPT_Stop()
{
   
    s_pGptRegs->TIMER_CTRL &=  0xfffeffff;    //clear bit16
}

void AdaptiveGPT_AdjustInterval(u16 x, u16 y, bool bAdjusted)
{
    static u16 previousX = 0, previousY = 0;
    double dGptIntervalMs = TOUCH_MS_PER_SAMPLE_LOW;
    double dMovement;

    if (bAdjusted == false)
    {
        dGptIntervalMs = TOUCH_MS_PER_SAMPLE_LOW;
    }
    else
    {
        dMovement = (x - previousX) * (x - previousX) + (y - previousY) * (y - previousY);

        if (dMovement > STANDARD_MOVEMENT * 16)
        {
            dGptIntervalMs /= 4.0;
        }
        else if (dMovement > STANDARD_MOVEMENT * 9)
        {
            dGptIntervalMs /= 3.0;
        }
        else if (dMovement > STANDARD_MOVEMENT * 4)
        {
            dGptIntervalMs /= 2.0;
        }
        else if (dMovement > STANDARD_MOVEMENT)
        {
            dGptIntervalMs -= 1.0;
        }
        else if (dMovement < STANDARD_MOVEMENT)
        {
            dGptIntervalMs += 1.0;
        }

        if (dGptIntervalMs > MAX_GPT_INTERVAL_MS)
            dGptIntervalMs = MAX_GPT_INTERVAL_MS;
        else if (dGptIntervalMs < MIN_GPT_INTERVAL_MS)
            dGptIntervalMs = MIN_GPT_INTERVAL_MS;
    }

   // s_gptConfig.u4CompareL = (UINT32) (dGptIntervalMs * COUNTE_PER_MSEC);
    //GPT_SetCompareL32(s_pGptRegs, s_gptConfig.num, s_gptConfig.u4CompareL);
   // GPT_SetCompareL32(s_pGptRegs, s_gptConfig.num, TOUCH_MS_PER_SAMPLE_LOW * COUNTE_PER_MSEC);

    //RETAILMSG(1, (TEXT("u4CompareL %d\r\n"), s_gptConfig.u4CompareL));
    // save the previous point
    previousX = x;
    previousY = y;
}

bool AdaptiveGPT_Init()
{
    s_pGptRegs = (MT_TIMER_REG *)(BIM_UCV_BASE + REG_RW_TIMER0_LIM_OFFSET);
    if (s_pGptRegs == NULL)
        return false;


	printk(" s_pGptRegs->TIMER2_LIM = 0x%x\r\n", s_pGptRegs->TIMER2_LIM);
   // s_pGptRegs->TIMER2_LIM = CFG_CLOCK_PER_TICKS*1000;  //1ms

    s_pGptRegs->TIMER2_LIM = CFG_CLOCK_PER_TICKS*3;  //10ms
	printk(" s_pGptRegs->TIMER_CTRL = 0x%x\r\n", s_pGptRegs->TIMER_CTRL);
    /* Start timer irq */
    s_pGptRegs->TIMER_CTRL |= VAL_T2_AUTOLOAD ;


    return true;
}

bool AdaptiveGPT_DeInit()
{
    if (s_pGptRegs != NULL)
    {
        s_pGptRegs = NULL;    
    }
    return true;
}

void AdaptiveGPT_ChangInterval(u16 ms)
{
    if (ms == 0)
        return;


}

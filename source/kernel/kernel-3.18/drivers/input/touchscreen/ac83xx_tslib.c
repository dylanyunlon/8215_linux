#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>

#include "ac83xx_tsadc.h"
#include "ac83xx_ts.h"
#include "ac83xx_tstimer.h"

typedef struct
{
    s32 maxPointX;
    s32 minPointX;
    s32 maxPointY;
    s32 minPointY;
    s32 Point1stX;
    s32 Point1stY;
} TOUCH_MISC;
//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static TOUCH_MISC s_touchMisc;

// because the point from adc is reliable
// disable the dejitter algorithm.
#define DEJITTER_THRESHOLD_X 500
#define DEJITTER_THRESHOLD_Y 500

// to avoid to get the adc values that varaince is too big.
// need to tune it
//#define VARIANCE_ALGO
#define VARIANCE_THRESHOLD_X 230
#define VARIANCE_THRESHOLD_Y 250

#define DISPLAY_X 1024
#define DISPLAY_Y 600

static bool s_bPowerOff = true;

u32 g_u4TouchResistorThreshold = 120000;
s32 g_i4TouchResistorOffset = 0;
u16 g_u2HighSampleRateFlag = 0;

u16 g_u2ErrSampleCnt = 0;
u16 g_u2ADCSPL = 28;
u16 g_u2ADCPULLUP = 0;

#if AUXADC_BIM_MODE
u16         g_u2TouchSampleNum         = 16;
#endif

#define FILTER_ORDER_SHIFT_BIT 2
#define FILTER_ORDER (1 << FILTER_ORDER_SHIFT_BIT)

s32 bufferX[FILTER_ORDER];
s32 bufferY[FILTER_ORDER];
s32 bufferXIndex, bufferYIndex;
s32 bufferXSize,bufferYSize;
u32 sumX, sumY;


extern const u16 MAX_ACD_X;
extern const u16 MAX_ACD_Y;

extern const u16 MIN_ACD_X;
extern const u16 MIN_ACD_Y;

extern void AuxsetSPLDuration(int value);
extern void AuxsetPullUp(int value);

void FilterInit(void)
{
    memset(bufferX, 0, FILTER_ORDER * sizeof(s32));
    memset(bufferY, 0, FILTER_ORDER * sizeof(s32));
    bufferXIndex = bufferYIndex = 0;
    bufferXSize = bufferYSize = 0;
    sumX = sumY = 0;
}

s32 FilterX(s32 x)
{
    sumX += (u32)x;
    sumX -= (u32)bufferX[bufferXIndex];
    bufferX[bufferXIndex] = x;
    bufferXIndex++;

    bufferXIndex %= FILTER_ORDER;

    if (bufferXSize < FILTER_ORDER)
        return (s32)(sumX / ++bufferXSize);

    return (s32)(sumX >> FILTER_ORDER_SHIFT_BIT);
}

s32 FilterY(s32 y)
{
    sumY += (u32)y;
    sumY -= (u32)bufferY[bufferYIndex];
    bufferY[bufferYIndex] = y;
    bufferYIndex++;

    bufferYIndex %= FILTER_ORDER;

    if (bufferYSize < FILTER_ORDER)
        return (s32)(sumY / ++bufferYSize);

    return (s32)(sumY >> FILTER_ORDER_SHIFT_BIT);
}

#ifdef GET_LCM_RESOLUTION_RUNTIME
void TransACD2Display(struct ac83xx_ts *ts, u16 *pX, u16 *pY)
{
    if (*pX <= MIN_ACD_X)
        *pX = 0;
    else if (*pX >= MAX_ACD_X)
        *pX = ts->x_resolution - 1;
    else
        *pX = (*pX - MIN_ACD_X) * ts->x_resolution / (MAX_ACD_X - MIN_ACD_X);

    if (*pY <= MIN_ACD_Y)
        *pY = 0;
    else if (*pY >= MAX_ACD_Y)
        *pY = ts->y_resolution - 1;
    else
        *pY = (*pY - MIN_ACD_Y) * ts->y_resolution / (MAX_ACD_Y - MIN_ACD_Y);
}
#else
void TransACD2Display(u16 *pX, u16 *pY)
{
    if (*pX <= MIN_ACD_X)
        *pX = 0;
    else if (*pX >= MAX_ACD_X)
        *pX = DISPLAY_X - 1;
    else
        *pX = (*pX - MIN_ACD_X) * DISPLAY_X / (MAX_ACD_X - MIN_ACD_X);

    if (*pY <= MIN_ACD_Y)
        *pY = 0;
    else if (*pY >= MAX_ACD_Y)
        *pY = DISPLAY_Y - 1;
    else
        *pY = (*pY - MIN_ACD_Y) * DISPLAY_Y / (MAX_ACD_Y - MIN_ACD_Y);
}
#endif

bool touch_init(void)
{
    AuxADCInit();
#if AUXADC_BIM_MODE
    AdaptiveGPT_Init();
#endif
    AuxsetSPLDuration(g_u2ADCSPL);
    AuxsetPullUp(g_u2ADCPULLUP);
    g_u2ErrSampleCnt = 0;
    return true;
}

bool touch_deinit(void)
{
#if AUXADC_BIM_MODE
    AdaptiveGPT_DeInit();
#endif
    AuxADCCloseClock();
    return true;
}

bool touch_power_on(void)
{
    // Power On Touch
    if (s_bPowerOff == true) {
        //CKGEN_AgtOnClk(e_CLK_TOUCHPANEL);
        s_bPowerOff = false;
    }

    return true;
}

bool touch_power_off(void)
{
    // Power Off Touch
    if (s_bPowerOff == false) {
        /// CKGEN_AgtOffClk(e_CLK_TOUCHPANEL);
        s_bPowerOff = true;
    }

    return true;
}

#ifdef GET_LCM_RESOLUTION_RUNTIME
void touch_panel_get_point(struct ac83xx_ts *ts, TOUCH_PRESS_STATE *pTipState, s32* pUnCalX,s32* pUnCalY)
#else
void touch_panel_get_point(TOUCH_PRESS_STATE *pTipState, s32* pUnCalX,s32* pUnCalY)
#endif
{
    static u16 u2AcdPx = 0, u2AcdPy = 0; //Previous valid acd x, y
    u16 u2AcdCx, u2AcdCy;    //Current acd x, y
    static u16 u2SmoothX = 0, u2SmoothY = 0; //after filter, x,y
    u16 u2DisX, u2DisY;      //dispaly x, y
    static bool bPreTouchPressed = false;
    bool bTouchPressed = false;
    bool bRt;
    u32 u4Rtouch;

    bRt = AuxGetTSPressed(&bTouchPressed);

    if (bRt == false) {
    //printk("brt=false");
        *pTipState = TOUCH_PRESS_IGNOR;    
        //printk("DdsiTouchPanelGetPoint : Can't call AuxGetTSPressed\r\n");        
        return;    
    }

    if ( g_u2ErrSampleCnt > 10) {
        //printk(" ======== sample err = %d \r\n",g_u2ErrSampleCnt);
        bTouchPressed = false;
        g_u2ErrSampleCnt = 0;
    }

    if (bTouchPressed == true && bPreTouchPressed == false) {
        touch_power_on();

        // just the 1st touch need to call AuxGetSposXY to do hardware calibration
        bRt = AuxGetTSPosXY(&u2AcdCx, &u2AcdCy, false, &u4Rtouch);

        touch_power_off();

        // Init the filter
        FilterInit();

        if (bRt == false) {
            *pUnCalX = u2SmoothX;
            *pUnCalY = u2SmoothY;
            *pTipState = TOUCH_PRESS_IGNOR;    
            //printk("brt=false1");
            //Invalid point, regard it as non-pressed
            bTouchPressed = false;    
            //printk("==Down : Invalid Point\r\n");                                        
        } else {
            //printk("%d, %d\r\n", u2AcdCx, u2AcdCy);

            s_touchMisc.Point1stX = s_touchMisc.maxPointX = s_touchMisc.minPointX = u2AcdCx;
            s_touchMisc.Point1stY = s_touchMisc.maxPointY = s_touchMisc.minPointY = u2AcdCy;

            *pUnCalX = u2SmoothX =u2AcdCx;  // FilterX(u2AcdCx);
            *pUnCalY = u2SmoothY = u2AcdCy ;// FilterY(u2AcdCy);     
            *pTipState = TOUCH_PRESS_DOWN;              

            u2AcdPx = u2AcdCx;
            u2AcdPy = u2AcdCy;            

            u2DisX = u2AcdCx;
            u2DisY = u2AcdCy;

#ifdef GET_LCM_RESOLUTION_RUNTIME
            TransACD2Display(ts, &u2DisX, &u2DisY);
#else
            TransACD2Display(&u2DisX, &u2DisY);
#endif

            //printk("Down %d %d\r\n", u2SmoothX, u2SmoothY);
            //printk("==Down %d %d %d %d %d %d\r\n", u2SmoothX, u2SmoothY, u2AcdCx, u2AcdCy, u2DisX, u2DisY);
        #if AUXADC_BIM_MODE
        AdaptiveGPT_Start();
        #else
        AuxADC_Open_Batchtimer();
        #endif
        }

    } 
    else if (bTouchPressed == true && bPreTouchPressed == true) 
    {
#if AUXADC_BIM_MODE
        AdaptiveGPT_Stop();
#else
        AuxADC_Close_Batchtimer();
#endif
        touch_power_on();

        bRt = AuxGetTSPosXY(&u2AcdCx, &u2AcdCy, true,&u4Rtouch);

        touch_power_off();

        if (bRt == false)
        {
            *pUnCalX = u2SmoothX;
            *pUnCalY = u2SmoothY;
            *pTipState = TOUCH_PRESS_IGNOR;    
            //printk("brt=false2");
            //printk("==Drag : Invalid Point\r\n");                 
        }
        else 
        {
            if (abs(u2AcdCx - u2AcdPx) > DEJITTER_THRESHOLD_X 
                || abs(u2AcdCy - u2AcdPy) > DEJITTER_THRESHOLD_Y) 
            {
                *pUnCalX = u2SmoothX;
                *pUnCalY = u2SmoothY;     
                //printk("==Drag : dejitter\r\n");                
            }
            else 
            {
                *pUnCalX = u2SmoothX = FilterX(u2AcdCx);
                *pUnCalY = u2SmoothY = FilterY(u2AcdCy);  

                u2AcdPx = u2AcdCx;
                u2AcdPy = u2AcdCy;            

                if (u2AcdCx > s_touchMisc.maxPointX)
                    s_touchMisc.maxPointX = u2AcdCx;
                else if (u2AcdCx < s_touchMisc.minPointX)
                    s_touchMisc.minPointX = u2AcdCx;

                if (u2AcdCy > s_touchMisc.maxPointY)
                    s_touchMisc.maxPointY = u2AcdCy;
                else if (u2AcdCy < s_touchMisc.minPointY)
                    s_touchMisc.minPointY = u2AcdCy;
            }
            *pTipState = TOUCH_PRESS_DOWN;    

            u2DisX = u2AcdCx;
            u2DisY = u2AcdCy;
#ifdef GET_LCM_RESOLUTION_RUNTIME
            TransACD2Display(ts, &u2DisX, &u2DisY);
#else
            TransACD2Display(&u2DisX, &u2DisY);
#endif
            //printk("Drag %d %d\r\n", u2SmoothX, u2SmoothY);
            //printk("==Drag %d %d %d %d %d %d\r\n", u2SmoothX, u2SmoothY, u2AcdCx, u2AcdCy, u2DisX, u2DisY);
        }
#if AUXADC_BIM_MODE
        AdaptiveGPT_Start();
#else
        AuxADC_Open_Batchtimer();
#endif

    } 
    else 
    {
    
        *pUnCalX = u2SmoothX;
        *pUnCalY = u2SmoothY;

        if (bTouchPressed == false && bPreTouchPressed == true) 
        {
            *pTipState = TOUCH_PRESS_UP;    
            //printk("Up %d %d\r\n", *pUnCalX, *pUnCalY);
        } 
        else 
        {
            *pTipState = TOUCH_PRESS_IGNOR;    
            //printk("Touch Interrupt issued. But the pressed flag can't be detected\r\n");
        }    
#if AUXADC_BIM_MODE
        AdaptiveGPT_Stop();
#else
        AuxADC_Close_Batchtimer();
#endif
    }
    bPreTouchPressed = bTouchPressed;  
}



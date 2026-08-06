#ifndef _AC83XX_TS_ADC_H
#define _AC83XX_TS_ADC_H
    
#include <linux/types.h>
#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include "drv_polling.h"

#define IS_VALID_POSITION(x, y)     ((x) <= MIN_ACD_X || (x)>= MAX_ACD_X || (y) <= MIN_ACD_Y || (y) >= MAX_ACD_Y)
#define AUX_SAMPLE_ADDR(addr)       WR(AUXADC_TS_ADDR, addr&0x7)
#define AUX_SAMPLE_SET_TRIG()       SB(AUXADC_TS_CON0, 1)
#define AUX_SAMPLE_GET_TRIG()       RB(AUXADC_TS_CON0, 1, 0) 
#define AUX_DATA_GET()              RB(AUXADC_TS_DAT0,bitmask,0)

#if 0
#define AUX_GET_POS(pos, addr)  \
            do{  \
                AUX_SAMPLE_ADDR(addr); \
                AUX_SAMPLE_SET_TRIG();   \
                while(AUX_SAMPLE_GET_TRIG());  \
                pos = AUX_DATA_GET();   \
            }while(0)
#endif
#define AUX_GET_POS(pos, addr)  \
            do{  \
                AUX_SAMPLE_ADDR(addr); \
                AUX_SAMPLE_SET_TRIG();   \
                WAIT_FOR_ZERO(AUX_SAMPLE_GET_TRIG(),10,"[Auxadc]sample get trigger timeout \n"); \
                pos = AUX_DATA_GET();   \
            }while(0)


#define ADC_LOG_LVL_OFF                          0x00000000
#define ADC_LOG_LVL_ERR                          0x00000001
#define ADC_LOG_LVL_TP                           0x00000002
#define ADC_LOG_LVL_KEY                          0x00000004
#define ADC_LOG_LVL_HAL                          0x00000008

extern u32 g_u4ADCLogMask;
#define X_Printf(formatStr,...)  printk(formatStr,##__VA_ARGS__)


#define HAL_LOG(lvl, formatStr,...)\
           {           \
               if(lvl & g_u4ADCLogMask){\
                   X_Printf("ADC Debug:");  \
                   X_Printf(formatStr, ##__VA_ARGS__);} \
           }

#define TOUCH_LOG(lvl, formatStr,...)\
           {           \
               if(lvl & g_u4ADCLogMask){ \
              X_Printf("TP Debug:");  \
              X_Printf(formatStr, ##__VA_ARGS__); }\
           }

#define KEYPAD_LOG(lvl, formatStr,...)\
           {           \
               if(lvl & g_u4ADCLogMask){ \
              X_Printf("KeyPad Debug:");  \
              X_Printf(formatStr, ##__VA_ARGS__);} \
           }

// Types
//
typedef enum 
{
    COORD_DDR       = 0,
    COORD_ALL       = 1,
    COORD_XY        = 2,
    COORD_Z1Z2      = 3,
} FAV_COORD;

typedef enum _tag_TS_POS
{
    TS_POS_X,
    TS_POS_Y,
    TS_POS_Z1,
    TS_POS_Z2,
    TS_POS_NUM
} TS_POS;

typedef enum _TOUCH_PRESS_STATE
{
    TOUCH_PRESS_UP = 0,
    TOUCH_PRESS_DOWN,
    TOUCH_PRESS_IGNOR,
} TOUCH_PRESS_STATE;

//This function sets the debounce time of the touch screen
bool AuxSetTSDebt(u16 u2Time);

//This function gets the status whether the touch screen is pressed.
bool AuxGetTSPressed(bool *pbPressed);

//This function gets the X and Y position depended on the variable "pos"
bool AuxGetTSPos(TS_POS pos, u16* pu2data);

//This function gets X and Y positions with the calibration algorith
bool AuxGetTSPosXY(u16* pu2Xdata, u16* pu2Ydata, bool bDraged,u32* pu4Rtouch);

//This function enable the penirq
bool AuxEnablePenIrq(void);

//This function disable the penirq
bool AuxDisablePenIrq(void);
void AuxADCCloseClock(void);

bool AuxADCInit(void);

bool AuxADC_Open_Batchtimer(void);
bool AuxADC_Close_Batchtimer(void);

#endif //_AC83XX_TS_ADC_H


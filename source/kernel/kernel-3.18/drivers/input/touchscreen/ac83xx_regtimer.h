#ifndef _REG_TIMER_H
#define _REG_TIMER_H

#include <mach/base_regs.h>
 

/*************************************************************************/
/*   TIMER                                                             */
/*************************************************************************/
#define REG_TIMER_LIMIT     (*((volatile uint32_t*)(BIM_BASE + 0x60)))
#define REG_TIMER_CONTROL   (*((volatile uint32_t*)(BIM_BASE + 0x78)))

#define REG_RW_TIMER0_LIM_OFFSET    0x60
#define REG_RW_TIMER0_VALUE               0x64
#define REG_RW_TIMER_CTRL_OFFSET    0x78
#define REG_RW_64B_TIMER0_OFFSET    0x728  

#define VAL_T0_AUTOLOAD     0x2
#define VAL_T0_ENABLE           0x1
#define VAL_T1_AUTOLOAD     0x200
#define VAL_T1_ENABLE           0x100
#define VAL_T2_AUTOLOAD     0x20000
#define VAL_T2_ENABLE           0x10000

#define CONFIG_SYS_HZ          1000     // ticks every 1ms

#define CFG_HZ_CLOCK        324000000

#define CFG_CLOCK_PER_TICKS (CFG_HZ_CLOCK / CONFIG_SYS_HZ)
#define CFG_HZ_PER_USEC     (CFG_HZ_CLOCK / 1000000)

typedef volatile struct {
    uint32_t TIMER0_LIM;
    uint32_t TIMER0_VALUE;
    uint32_t TIMER1_LIM;
    uint32_t TIMER1_VALUE;
    uint32_t TIMER2_LIM;
    uint32_t TIMER2_VALUE;
    uint32_t TIMER_CTRL;
}MT_TIMER_REG;

typedef volatile struct {
    uint32_t TIMER0_64B_LO;
    uint32_t TIMER0_64B_HI;
    uint32_t TIMER0_64B_EN;
}MT_TIMER_64B_REG;

//PowerDown Watchdog
#define REG_WDT_EN       0x4
#define WDT_EN 0x1
#define WDT_MODE_00 0x0
#define REG_WDT_COUNTER  0x8
#define REG_WDT_LIMIT    0xc

#define WDT_CLOCK        3000000  //3Mhz

#endif


/*-----------------------------------------------------------------------------
 * Copyright (c) 2005, MediaTek Inc.
 * All rights reserved.
 *
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclosure of this information in whole or in part is prohibited.
 *---------------------------------------------------------------------------
 *
 * $Author: jianghong.lin $
 * $Date: 2015/07/01 $
 * $RCSfile: serial_mt53xx.h,v $
 * $Revision: #1 $
 *
 *---------------------------------------------------------------------------*/

 #ifndef _REG_TIMER_H
 #define _REG_TIMER_H

 
#define IO_BASE             0xf0000000
#define BIM_BASE            (IO_BASE + 0x08000)

/*************************************************************************/
/*   TIMER                                                             */
/*************************************************************************/
#define REG_TIMER_LIMIT		(*((volatile UINT32*)(BIM_BASE + 0x148)))
#define REG_TIMER_CONTROL	(*((volatile UINT32*)(BIM_BASE + 0x164)))

//#define REG_RW_TIMER0_LIM_OFFSET    0x60
//#define REG_RW_TIMER0_VALUE         0x64
//#define REG_RW_TIMER_CTRL_OFFSET    0x78
#define REG_RW_64B_TIMER0_OFFSET  0x728  

#define VAL_T0_AUTOLOAD     0x2
#define VAL_T0_ENABLE       0x1
#define VAL_T1_AUTOLOAD     0x200
#define VAL_T1_ENABLE       0x100
#define VAL_T2_AUTOLOAD     0x20000
#define VAL_T2_ENABLE       0x10000

#define CONFIG_SYS_HZ          1000     

#define CFG_HZ_CLOCK		27000000  



#define CFG_CLOCK_PER_TICKS (CFG_HZ_CLOCK / CONFIG_SYS_HZ)
#define CFG_HZ_PER_USEC     (CFG_HZ_CLOCK / 1000000)

typedef volatile struct {
    UINT32 TIMER0_LIM;
    UINT32 TIMER0_VALUE;
    UINT32 TIMER1_LIM;
    UINT32 TIMER1_VALUE;
    UINT32 TIMER2_LIM;
    UINT32 TIMER2_VALUE;
    UINT32 TIMER_CTRL;
}AC83XX_TIMER_REG;

//Add by yinsheng zhang for 64Bit timer
typedef volatile struct {
    UINT32 TIMER0_64B_LO;
    UINT32 TIMER0_64B_HI;
    UINT32 TIMER0_64B_EN;
    UINT32 TIMER1_64B_LO;
    UINT32 TIMER2_64B_HI;
    UINT32 TIMER2_64B_EN;
}AC83XX_TIMER_64B_REG;

#endif


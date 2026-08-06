#ifndef _ARM_MONITOR_H_
#define _ARM_MONITOR_H_


#include "x_typedef.h"

static inline void ConfigPerformanceMonitorControlReg(UINT32 value)
{
  asm volatile(
    "MCR     p15, 0, %0, c15, c12, 0\n"
  :
  : "r"(value)
  : "memory"
  ); 
}


static inline UINT32 ReadARMCycleCounter(void)
{
  UINT32 r;

  asm volatile(
    "MRC     p15, 0, %0, c15, c12, 1\n"
    :  "=r"(r)
    :
    : "memory"
  );

	return (r);
}


static inline BOOL IsARMCycleCounterOverflow(void)
{
  UINT32 r;
  UINT32 overflow_bit;

  asm volatile(
    "MRC     p15, 0, r, c15, c12, 0\n"
    :  "=r"(r)
    :
    : "memory"
  );
  
  overflow_bit = (r>>10) & 0x0001;
  return ((overflow_bit == 1));
  
}


#endif


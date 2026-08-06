#ifndef _ARM_CYCLE_COUNT_H_
#define _ARM_CYCLE_COUNT_H_

void ConfigPerformanceMonitorControlReg(UINT32 value);
UINT32 ReadARMCycleCounter(void);
BOOL IsARMCycleCounterOverflow(void);

#endif

#include "targetConfig.h"
#include "preloader_common.h"




#ifdef ARM_CORE_V6
//---------------------------------------------------------------------
void ConfigPerformanceMonitorControlReg(UINT32 value)
{
  asm volatile(
    "MCR     p15, 0, %0, c15, c12, 0\n"
    :
    :"r"(value)
    :"memory" 
  ); 
}


UINT32 ReadARMCycleCounter(void)
{
  UINT32 r;

  asm volatile(
    "MRC     p15, 0, %0, c15, c12, 1\n"
    :"=r"(r)
    :
    :"memory"
  ); 

	return (r);
}


BOOL IsARMCycleCounterOverflow(void)
{
  UINT32 r;
  UINT32 overflow_bit;

  asm volatile(
    "MRC     p15, 0, %0, c15, c12, 0\n"
    :"=r"(r)
    :
    :"memory"
  ); 

  overflow_bit = (r>>10) & 0x0001;
  return ((overflow_bit == 1));
  
}

#endif

#ifdef ARM_CORE_V7
//---------------------------------------------------------------------
void ConfigPerformanceMonitorControlReg(UINT32 value)
{
	
  asm volatile(
  	// disable PMCCNTR
  	"MOV     r0,#0x80000000\n"
    "MCR     p15, 0, r0, c9, c12, 2\n"
  	// Reset PMCCNTR to zero
  	"MOV     r0,#0x5\n"
    "MCR     p15, 0, r0, c9, c12, 0\n"
    // clear PMCCNTR overflow flag
    "MOV     r0,#0x80000000\n"                           
    "MCR     p15, 0, r0, c9, c12, 3\n"
    // enable PMCCNTR
    "MOV     r0,#0x80000000\n"                           
    "MCR     p15, 0, r0, c9, c12, 1\n"
    :
    :
    :
  ); 
}


UINT32 ReadARMCycleCounter(void)
{
  UINT32 r;

  asm volatile(
    "MRC     p15, 0, %0, c9, c13, 0\n"
    :"=r"(r)
    :
    :"memory"
  ); 

	return (r);
}


BOOL IsARMCycleCounterOverflow(void)
{
  UINT32 r;
  UINT32 overflow_bit;

  asm volatile(
    "MRC     p15, 0, %0, c9, c12, 3\n"
    :"=r"(r)
    :
    :"memory"
  ); 

  overflow_bit = (r>>31) & 0x0001;
  return ((overflow_bit == 1));
  
}
#endif






#ifndef _AP_SHOW_TEST_H_
#define _AP_SHOW_TEST_H_


#if(AP_SHOW_TEST_ENABLE)
extern void AP_Show(UINT32 cpu);
#endif

#if(MT3363_AP_SHOW_TEST_ENABLE)
extern void AP_Show_BAR_OR_PIC(UINT32 cpu);
#endif


#endif


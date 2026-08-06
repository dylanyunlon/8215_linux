#ifndef __TIMER_H_
#define __TIMER_H_


extern UINT32 TIM_CalcExpiredUS(UINT32 us);
extern UINT32 TIM_IsExpired(UINT32 cnt);
extern UINT32 TIM_DelayUS(UINT32 us);
extern UINT32 TIM_ReadValue(void);
extern void TIM_Start(void);
extern UINT32 TIM_StartTIM(UINT32 tid,UINT32 us);
extern UINT32 TIM_IsTIMExpired(UINT32 tid);


#endif

#include "x_typedef.h"
#include "targetConfig.h"
#include "x_bim_3363.h"
#include "x_bim.h"

#define TOLMT_REG  0x148
#define T0VAL_REG  0x14c
#define TCTL_REG   0x164
#define TCTL_T0_CTLR_MASK	0x3
#define TCTL_TO_ENABLE		0x1

#define TIM_MAX_INSTANCE      3

#define vWriteBIM(offset, value) BIM_WRITE32(offset, value)
#define u4ReadBIM(offset) BIM_READ32(offset)

UINT32 TIM_Instance[TIM_MAX_INSTANCE];

void TIM_Start(void)
{
    UINT32 tmpReg;
	vWriteBIM(TOLMT_REG,0xFFFFFFFF);
    tmpReg = u4ReadBIM(TCTL_REG);
    tmpReg &= (~TCTL_T0_CTLR_MASK);
    vWriteBIM(TCTL_REG,tmpReg | TCTL_TO_ENABLE);
}

UINT32 TIM_ReadValue(void)
{
	return u4ReadBIM(T0VAL_REG);
}

UINT32 TIM_DelayUS(UINT32 us)
{
	UINT32 clkdly;
	if(us > 159000000)
	{
		return 1;
	}


	clkdly = u4ReadBIM(T0VAL_REG) - (us << 5) + (us << 2) + us;

	while(u4ReadBIM(T0VAL_REG) > clkdly);
	return 0;
}

UINT32 TIM_IsExpired(UINT32 cnt)
{
	if(u4ReadBIM(T0VAL_REG) < cnt)
	{
		return 1;
	}
	return 0;
}

UINT32 TIM_CalcExpiredUS(UINT32 us)
{
	UINT32 clkdly;

	clkdly = u4ReadBIM(T0VAL_REG) - (us << 5) + (us << 2) + us;
	return clkdly;
}

UINT32 TIM_StartTIM(UINT32 tid,UINT32 us)
{

	TIM_Instance[tid] = TIM_CalcExpiredUS(us);
	return 0;
}

UINT32 TIM_IsTIMExpired(UINT32 tid)
{
	return TIM_IsExpired(TIM_Instance[tid]);
}


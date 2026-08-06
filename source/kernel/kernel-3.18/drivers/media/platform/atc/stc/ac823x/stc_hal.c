/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <linux/module.h>
#include "stc_hal.h"

#define AUD_BASE_REGS				stc_base_regs

#define STCREG_BASE_0				    (AUD_BASE_REGS + 0x208)
#define STCREG_BASE_1				    (AUD_BASE_REGS + 0x258)
#define STCREG_PCR_CTL          0
#define STCREG_STC_EXTENSION	  12
#define STCREG_STC_BASE				  13
/* bit 3    : speed up or slow down control, 0-speed up, 1-slow down */
/* bit 0~2: STC Speed control reigster, 000: 1x or 1x, 0001: 2x or 1/2x, ... */
#define STCREG_SPEED_CTRL_0     452
#define STCREG_SPEED_CTRL_1     433

volatile u32 *g_stc_base[MAX_OF_STC_DEV_CNT] = { NULL, NULL };

#define STC_READ32(idx, offset)							 \
		(*(g_stc_base[idx] + (offset)))

#define STC_WRITE32(idx, offset, value)			 \
do {	 \
		*(g_stc_base[idx] + (offset)) = (value);	 \
		mb();	 \
} while (0)

bool STC_HalInit(void)
{
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < MAX_OF_STC_DEV_CNT; u4Idx++) {
		if (0 == u4Idx)
			g_stc_base[u4Idx] = (u32 *) STCREG_BASE_0;
		else
			g_stc_base[u4Idx] = (u32 *) STCREG_BASE_1;

		if (NULL == g_stc_base[u4Idx]) {
			pr_err("%s:%s:%d. fail for invalid stc base %d register\r\n",
                FILE_ONLY, __func__, __LINE__, u4Idx);
			return false;
		}
	}

	return true;
}

void STC_HalDeinit(void)
{
}

bool STC_HalGetTime(u32 u4DevId, u64 *pu8Time)
{
	u32 u4StartStcL;
	u32 u4StartStcH;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	if (NULL == pu8Time) {
		pr_err("%s:%s:%d. fail for invalid args\r\n", FILE_ONLY, __func__, __LINE__);
		return false;
	}

	u4StartStcL = STC_READ32(u4DevId, STCREG_STC_EXTENSION);
	u4StartStcH = STC_READ32(u4DevId, STCREG_STC_BASE);

	*pu8Time = u4StartStcH;
	*pu8Time = (u64) ((*pu8Time << 1) | (u4StartStcL >> 31));

	return true;
}
EXPORT_SYMBOL(STC_HalGetTime);

bool STC_HalSetTime(u32 u4DevId, u64 u8Time)
{
	u32 u4StartStcL = 0;
	u32 u4StartStcH = 0;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n", FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	u4StartStcL = (u32) (u8Time & 0x1);
	u4StartStcH = (u32) ((u8Time >> 1) & 0xFFFFFFFF);

	STC_WRITE32(u4DevId, STCREG_STC_EXTENSION, u4StartStcL << 31);
	STC_WRITE32(u4DevId, STCREG_STC_BASE, u4StartStcH);

	return true;
}
EXPORT_SYMBOL(STC_HalSetTime);

bool STC_HalSetRate(u32 u4DevId, u32 u4Rate, bool is_slow_down)
{
	u32 u4Reg = 0;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n", FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	switch (u4Rate) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
		{
			u32 u4RateId = 0;

			if (0 == u4DevId)
				u4Reg = STC_READ32(u4DevId, STCREG_SPEED_CTRL_0);
			else
				u4Reg = STC_READ32(u4DevId, STCREG_SPEED_CTRL_1);

			u4Reg &= ~(1 << 3);
			if (is_slow_down)
				u4Reg |= ~(1 << 3);

			for (u4RateId = 0; u4RateId <= 5; u4RateId++) {
				if ((((u32) 1) << u4RateId) == u4Rate)
					break;
			}
			if (u4RateId > 5) {
				pr_err
				    ("%s:%s:%d. fail for invalid rate(%d), slow_down(%d), u4RateIdx(%d)\r\n",
				     FILE_ONLY, __func__, __LINE__, u4Rate, is_slow_down, u4RateId);
				return false;
			}
			u4Reg |= 1 << u4RateId;
			if (0 == u4DevId)
				STC_WRITE32(u4DevId, STCREG_SPEED_CTRL_0, u4Reg);
			else
				STC_WRITE32(u4DevId, STCREG_SPEED_CTRL_1, u4Reg);
		}

		break;
	default:
		pr_err("%s:%s:%d. fail for invalid rate(%d), slow_down(%d)\r\n",
		       FILE_ONLY, __func__, __LINE__, u4Rate, is_slow_down);
		return false;
	}

	return true;
}

bool STC_HalStart(u32 u4DevId)
{
	u32 u4RegSetValue = 0;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n", 
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	u4RegSetValue = STC_READ32(u4DevId, STCREG_PCR_CTL);
	u4RegSetValue &= (~(1 << 3));
	STC_WRITE32(u4DevId, STCREG_PCR_CTL, u4RegSetValue);

	return true;
}

bool STC_HalPause(u32 u4DevId)
{
	u32 u4RegSetValue = 0;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	u4RegSetValue = STC_READ32(u4DevId, STCREG_PCR_CTL);
	u4RegSetValue |= (1 << 3);
	STC_WRITE32(u4DevId, STCREG_PCR_CTL, u4RegSetValue);

	return true;
}

bool STC_HalGetStatus(u32 u4DevId, STC_STATUS_T *peStatus)
{
	u32 u4RegSetValue = 0;
	STC_STATUS_T eStatus = STC_PAUSE;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	if (NULL == peStatus) {
		pr_err("%s:%s:%d. fail for invalid args, stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	eStatus = STC_PAUSE;

	u4RegSetValue = STC_READ32(u4DevId, STCREG_PCR_CTL);
	if (u4RegSetValue & 0x8)
		eStatus = STC_PAUSE;
	else
		eStatus = STC_RUN;

	*peStatus = eStatus;

	return true;
}


bool STC_HalUpdate(u32 u4DevId, STC_UPDATE_IN_INFO_T *prIn, STC_UPDATE_OUT_INFO_T *prOut)
{
	u32 u4StartStcL;
	u32 u4StartStcH;
	s64 i8NewBaseTime;
	s64 i8NewSTCTime;

	if (MAX_OF_STC_DEV_CNT <= u4DevId) {
		pr_err("%s:%s:%d. fail for invalid stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	if ((NULL == prIn) || (NULL == prOut)) {
		pr_err("%s:%s:%d. fail for invalid args, stc device id(%d)\r\n",
            FILE_ONLY, __func__, __LINE__, u4DevId);
		return false;
	}

	i8NewSTCTime = prIn->u8StartMediaTime + prIn->i8BaseTime - prIn->i8StartTime;

	if (i8NewSTCTime < 0)
		i8NewSTCTime += 0x200000000LL;
	else if (i8NewSTCTime >= 0x200000000LL)
		i8NewSTCTime -= 0x200000000LL;

	i8NewBaseTime = prIn->i8BaseTime - i8NewSTCTime;
	u4StartStcL = (u32) (i8NewSTCTime & 0x1);
	u4StartStcH = (u32) ((i8NewSTCTime >> 1) & 0xFFFFFFFF);
	STC_WRITE32(u4DevId, STCREG_STC_EXTENSION, u4StartStcL << 31);
	STC_WRITE32(u4DevId, STCREG_STC_BASE, u4StartStcH);

	prOut->i8BaseTime = i8NewBaseTime;
	prOut->u8STCTime = (__u64) i8NewSTCTime;

	return true;
}

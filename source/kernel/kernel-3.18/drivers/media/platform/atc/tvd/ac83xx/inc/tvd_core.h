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

#ifndef TVD_CORE_HAL_H_
#define TVD_CORE_HAL_H_


#include "tvd_data_struct.h"
#include "tvd_internal_data_struct.h"

extern struct clk *clk_ac8317_tvd1;
extern struct clk *clk_ac8317_tvd2;

#if SUPPORT_CALIBRATE_BRIGHTNESS

#define   YGAIN_VALID_MASK       ((u8)1 << 0)
#define   YOFFSET_VALID_MASK     ((u8)1 << 1)
#define   UCOSGAIN_VALID_MASK    ((u8)1 << 2)
#define   VCOSGAIN_VALID_MASK    ((u8)1 << 3)
#define   USINGAIN_VALID_MASK    ((u8)1 << 4)
#define   VSINGAIN_VALID_MASK    ((u8)1 << 5)
#define   UOFFSET_VALID_MASK     ((u8)1 << 6)
#define   VOFFSET_VALID_MASK     ((u8)1 << 7)
#endif

enum TVD_MODE_E {
	AV_PAL_N,
	AV_PAL,
	AV_PAL_M,
	AV_NTSC,
	AV_SECAM,
	AV_PAL_60,
	AV_UNSTABLE,  /* Represent video signal is not stable yet! In Hardware, it's reserved*/
	AV_NTSC443,
	AV_NONE
};



void Tvd_ClkOnOff(bool fgOn);
void vCVBS_Init(void);
bool CVBS_By_Pass(u32 u4CH, u32 u4CVBSInP, u32 u4BypassChannel, u32 u4CfgType);
void Tvd_CHA_PowerOn(bool fgOnOff);
void Tvd_CHB_PowerOn(bool fgOnOff);
void Tvd_Ana_IO_PowerOn(bool fgOnOff);
u32 Tvd_Core_Init(void);
void Tvd_Core_DeInit(void);
void Tvd_Core_Config(TVD_CORE_PREVIEW_CFG_T *prPreviewCfg);
void Tvd_Core_IrqClear(u32 u4IrqStatus);
void Tvd_Core_IrqEnable(bool fgEnable);
u32 Tvd_Core_IrqStatus(void);
u32 Tvd_Core_GetMode(void);
void  Tvd_Core_SetMode(u32 u4TvdMode);
void Tvd_Core_SetUVSwap(bool fgUVSwap);
TVD_SIG_STATE_T tvd_core_get_signal_state(void);
void TVD_ATV_Mode_Set(u32 u4AtvMode);
void _TVD_Set_ManualSigMode(u32 u4SigMode);
void TVD_Disable_ManualMode(void);
void TVD_Enable_ManualMode(void);
void Tvd_Register_Rst(void);


#endif

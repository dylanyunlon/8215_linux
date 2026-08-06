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

#ifndef _WCH_HAL_H_
#define _WCH_HAL_H_

#define WCH_SHADOW_ENABLE       0

void WchHalSetPinmux(WCH_DATA_SRC_E eSrcType);
void WchHalSetSrcType(u8 u1WchId, WCH_DATA_SRC_E eSrcType);
void WchHalSetInput(u8 u1WchId, WCH_DATA_FMT_E eSrcFmt, u32 u4SrcWidth, u32 u4SrcHeight,
			u32 u4StartX, u32 u4StartYTop, u32 u4StartYBot, bool fgProgressive);
void WchHalSetOutput(u8 u1WchId, WCH_DATA_FMT_E eDstFmt, bool fgProgressive, u32 u4DstWidth, u32 u4DstHeight);
void WchHalSetPolarity(u8 u1WchId, bool VSyncPolarity, bool HSyncPolarity, bool BotFieldFirst);
void WchHalSetCtrlSignal(u8 u1WchId, bool fgProgressive);
void WchHalSetYCAddr(u8 u1WchId, u32 u4YAddr, u32 u4CAddr);
void WchHalSetUvYcSwap(u8 u1WchId, u8 u1Mask, u8 u1UVSwap, u8 u1YCSwap);
void WchHalSetYUVDelay(u8 u1WchId, u8 u1Mask, u8 u1YSel, u8 u1USel, u8 u1VSel);
void WchHalSetDEDelay(u8 u1WchId, u8 u4DeSel);
void WchHalSetMirror(u8 u1WchId, u32 u4Mirror);
void WchHalLineAverageEn(u8 u1WchId, bool fgOn);
void WchHalSetRegTouch(u8 u1WchId);
bool WchHalIsOn(u8 u1WchId);
void WchHalStart(u8 u1WchId);
void WchHalStop(u8 u1WchId);
void WchHalEnableFsm(u8 u1WchId);
void WchHalDisableFsm(u8 u1WchId);
void WchHalResetFsm(u8 u1WchId);
void WchHalInit(u8 u1WchId);
void WchHalDeinit(u8 u1WchId);
WCH_DATA_SRC_E WchHalGetSrcType(u8 u1WchId);

#endif

